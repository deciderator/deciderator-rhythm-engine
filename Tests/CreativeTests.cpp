// SPDX-License-Identifier: AGPL-3.0-only
#include "../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace
{
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void set(DecideratorProcessor& p, const juce::String& id, float value)
{
    auto* parameter = p.parameters.getParameter(id);
    check(parameter != nullptr, "Missing parameter");
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}
class Clock final : public juce::AudioPlayHead
{
public:
    PositionInfo position;
    Clock() { position.setBpm(120.0); position.setPpqPosition(0.0); position.setIsPlaying(true); }
    juce::Optional<PositionInfo> getPosition() const override { return position; }
};
float energy(const juce::AudioBuffer<float>& b)
{
    float sum = 0;
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < b.getNumSamples(); ++i) sum += b.getSample(c, i) * b.getSample(c, i);
    return sum;
}
void signal(juce::AudioBuffer<float>& b, int offset = 0)
{
    for (int i = 0; i < b.getNumSamples(); ++i)
    {
        b.setSample(0, i, 0.2f * static_cast<float>(std::sin((i + offset) * 0.07)));
        b.setSample(1, i, 0.15f * static_cast<float>(std::cos((i + offset) * 0.11)));
    }
}
}

void runCreativeTests()
{
    juce::MidiBuffer midi;
    // Host tempo, seek, loop and stopped transport.
    Clock clock;
    DecideratorProcessor p;
    p.setPlayHead(&clock);
    p.prepareToPlay(48000, 64);
    juce::AudioBuffer<float> block(2, 64);
    for (const auto ppq : { 0.0, 0.25, 1.75, -0.25, 0.0 })
    {
        clock.position.setPpqPosition(ppq);
        signal(block); p.processBlock(block, midi);
        const auto expected = static_cast<int>(std::floor(ppq / 0.25) + 16) % 16;
        check(p.currentStep.load() == expected, "Host seek/loop step");
    }
    clock.position.setBpm(90.0); clock.position.setPpqPosition(0.25); clock.position.setIsPlaying(false);
    p.processBlock(block, midi);
    check(p.currentStep.load() == 1 && p.currentBpm.load() == 90.0f && p.hostClock.load(), "Stopped/tempo clock");

    // Same host timeline must yield the same result across block partitioning.
    std::cout << "CHECK: creative clock passed; block partitioning\n";
    DecideratorProcessor a, b;
    Clock ca, cb;
    a.setPlayHead(&ca); b.setPlayHead(&cb);
    a.applyScene(1); b.applyScene(1);
    a.prepareToPlay(48000, 24000); b.prepareToPlay(48000, 257);
    juce::AudioBuffer<float> whole(2, 24000); signal(whole); a.processBlock(whole, midi);
    int offset = 0;
    float maxDifference = 0;
    while (offset < 24000)
    {
        const auto count = juce::jmin(257, 24000 - offset);
        juce::AudioBuffer<float> part(2, count); signal(part, offset);
        cb.position.setPpqPosition(offset / 24000.0);
        b.processBlock(part, midi);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < count; ++i)
                maxDifference = juce::jmax(maxDifference, std::abs(part.getSample(c, i) - whole.getSample(c, offset + i)));
        offset += count;
    }
    check(maxDifference < 1.0e-5f, "Block-size independent host rhythm");

    // Dry mix is transparent even when every effect is enabled.
    std::cout << "CHECK: partitioning passed; dry mix\n";
    DecideratorProcessor dry;
    dry.applyScene(1); set(dry, "mix", 0); set(dry, "gainDb", 0);
    dry.prepareToPlay(48000, 512);
    juce::AudioBuffer<float> dryBlock(2, 512); signal(dryBlock);
    juce::AudioBuffer<float> original; original.makeCopyOf(dryBlock);
    dry.processBlock(dryBlock, midi);
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < 512; ++i) check(dryBlock.getSample(c, i) == original.getSample(c, i), "Dry mix transparency");

    // Silence a sequence; filter should attenuate high frequency energy.
    std::cout << "CHECK: dry mix passed; gate/filter/texture\n";
    DecideratorProcessor gate;
    set(gate, "gateDepth", 1);
    for (int i = 0; i < 16; ++i) set(gate, "pulse" + juce::String(i), 0);
    gate.prepareToPlay(48000, 4096);
    juce::AudioBuffer<float> muted(2, 4096); signal(muted); gate.processBlock(muted, midi);
    check(std::abs(muted.getSample(0, 4095)) < 1.0e-6f, "Gate silence after slew");
    DecideratorProcessor filter;
    set(filter, "filterDepth", 1); set(filter, "cutoff", 150);
    filter.prepareToPlay(48000, 4096);
    signal(muted); const auto before = energy(muted); filter.processBlock(muted, midi);
    check(energy(muted) < before * 0.2f, "Filter attenuation");

    DecideratorProcessor heat;
    set(heat, "drive", 0.5f); heat.prepareToPlay(48000, 512);
    signal(block); const auto cleanEnergy = energy(block); heat.processBlock(block, midi);
    check(energy(block) > cleanEnergy * 1.2f, "Heat changes audio");
    DecideratorProcessor orbit;
    Clock orbitClock; orbitClock.position.setPpqPosition(1.0); orbitClock.position.setIsPlaying(false);
    orbit.setPlayHead(&orbitClock); set(orbit, "motion", 1); orbit.prepareToPlay(48000, 64);
    signal(block); orbit.processBlock(block, midi);
    check(block.getMagnitude(0, 0, 64) < 1.0e-6f && block.getMagnitude(1, 0, 64) > 0.05f, "Orbit stereo balance");
    DecideratorProcessor fallback;
    set(fallback, "tempo", 120); fallback.prepareToPlay(48000, 6001);
    juce::AudioBuffer<float> freeRun(2, 6001); freeRun.clear(); fallback.processBlock(freeRun, midi);
    check(fallback.currentStep.load() == 1 && !fallback.hostClock.load(), "Fallback clock progression");

    // Echo repeats an impulse, crosses channels, and resets cleanly.
    std::cout << "CHECK: modulation passed; echo\n";
    DecideratorProcessor echo;
    set(echo, "echo", 0.7f); set(echo, "feedback", 0.5f);
    echo.prepareToPlay(48000, 512);
    juce::AudioBuffer<float> impulse(2, 48000); impulse.clear(); impulse.setSample(0, 0, 0.5f);
    echo.processBlock(impulse, midi);
    check(std::abs(impulse.getSample(0, 18000)) > 0.1f, "Echo first repeat");
    check(std::abs(impulse.getSample(1, 36000)) > 0.05f, "Ping-pong repeat");
    echo.reset(); impulse.clear(); echo.processBlock(impulse, midi);
    check(energy(impulse) == 0, "Reset clears echo");

    // Full state, including step data, survives roundtrip. Old gain-only states stay neutral.
    std::cout << "CHECK: echo passed; state\n";
    DecideratorProcessor state;
    state.applyScene(2); state.generatePattern();
    juce::MemoryBlock data; state.getStateInformation(data);
    DecideratorProcessor restored; restored.setStateInformation(data.getData(), static_cast<int>(data.getSize()));
    for (auto* parameter : state.getParameters())
    {
        auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter);
        check(ranged != nullptr, "Ranged parameter");
        check(std::abs(ranged->getValue() - restored.parameters.getParameter(ranged->paramID)->getValue()) < 1.0e-5f, "Full state roundtrip");
    }
    const auto oldState = juce::String("<DECIDERATOR_STATE><PARAM id=\"gainDb\" value=\"-9\"/></DECIDERATOR_STATE>");
    auto xml = juce::XmlDocument::parse(oldState); juce::MemoryBlock legacy;
    juce::AudioProcessor::copyXmlToBinary(*xml, legacy);
    restored.setStateInformation(legacy.getData(), static_cast<int>(legacy.getSize()));
    check(std::abs(restored.parameters.getRawParameterValue("gainDb")->load() + 9.0f) < 0.001f, "Legacy gain");
    for (const auto* id : { "gateDepth", "filterDepth", "drive", "echo", "motion" })
        check(restored.parameters.getRawParameterValue(id)->load() == 0, "Legacy neutral effects");

    // Generator preserves output/tempo; undo restores the prior scene exactly.
    std::cout << "CHECK: state passed; generator\n";
    set(state, "gainDb", -7); set(state, "tempo", 137);
    const auto oldStep = state.parameters.getRawParameterValue("pulse1")->load();
    state.generatePattern();
    check(std::abs(state.parameters.getRawParameterValue("gainDb")->load() + 7.0f) < 0.001f && state.parameters.getRawParameterValue("tempo")->load() == 137, "Generator preserves level/tempo");
    state.undoScene();
    check(state.parameters.getRawParameterValue("pulse1")->load() == oldStep, "Generator undo");

    // Stress all modules at several rates, changing parameters, silence and hot input.
    std::cout << "CHECK: generator passed; stress\n";
    for (const auto sampleRate : { 44100.0, 48000.0, 96000.0 })
    {
        DecideratorProcessor stress;
        stress.applyScene(1); set(stress, "feedback", 0.7f); set(stress, "drive", 1);
        stress.prepareToPlay(sampleRate, 257);
        juce::AudioBuffer<float> audio(2, 257);
        for (int iteration = 0; iteration < 400; ++iteration)
        {
            if (iteration % 100 == 0) stress.generatePattern();
            signal(audio, iteration * 257);
            if (iteration > 300) audio.clear();
            else audio.applyGain(4.0f);
            stress.processBlock(audio, midi);
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 257; ++i)
                    check(std::isfinite(audio.getSample(c, i)) && std::abs(audio.getSample(c, i)) < 8, "Finite bounded creative processing");
        }
    }
    std::cout << "PASS: host tempo/seek/loop/stop, block partitioning, dry mix, gate, filter, ping-pong/reset, full/legacy state, generator/undo, 3-rate stress\n";
}
