// SPDX-License-Identifier: AGPL-3.0-only
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <charconv>

namespace
{
const std::array<const char*, 19> ids { "gainDb", "gateDepth", "filterDepth", "cutoff", "drive", "echo", "feedback", "motion", "mix",
    "spaceMix", "spaceSize", "spaceDecay", "spaceTone", "spaceMotion", "glitchMix", "lofiMix", "lofiBits", "lofiRate", "timeMix" };
constexpr std::array<double, 5> stepBeats { 1.0, 0.5, 0.25, 0.125, 1.0 / 6.0 };
constexpr std::array<double, 3> echoBeats { 0.5, 0.75, 1.0 };
}

juce::AudioProcessorValueTreeState::ParameterLayout DecideratorProcessor::makeParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    auto add = [&](const char* id, const char* name, float lo, float hi, float initial, const char* unit = "")
    {
        auto range = juce::NormalisableRange<float>(lo, hi, 0.001f);
        if (juce::String(id) == "lofiBits" || juce::String(id) == "lofiRate") range.interval = 1.0f;
        if (juce::String(id) == "cutoff") range.setSkewForCentre(2000.0f);
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { id, 1 }, name, range, initial,
            juce::AudioParameterFloatAttributes().withLabel(unit)));
    };
    // Keep the original gain ID and range for existing sessions.
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "gainDb", 1 }, "Gain",
        juce::NormalisableRange<float> { -60.0f, 12.0f, 0.01f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));
    add("gateDepth", "Pulse depth", 0, 1, 0);
    add("filterDepth", "Tone depth", 0, 1, 0);
    add("cutoff", "Tone ceiling", 150, 18000, 9000, "Hz");
    add("drive", "Heat", 0, 1, 0);
    add("echo", "Echo mix", 0, 0.7f, 0);
    add("feedback", "Echo feedback", 0, 0.7f, 0.35f);
    add("motion", "Orbit", 0, 1, 0);
    add("mix", "Dry Wet", 0, 1, 1);
    add("tempo", "Fallback tempo", 40, 240, 120, "BPM");
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "rate", 1 }, "Step rate",
        juce::StringArray { "1/4", "1/8", "1/16", "1/32", "1/16T" }, 2));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "echoRate", 1 }, "Echo time",
        juce::StringArray { "1/8", "1/8 dotted", "1/4" }, 1));
    for (int i = 0; i < 16; ++i)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "pulse" + juce::String(i), 1 },
            "Pulse " + juce::String(i + 1), 0.0f, 1.0f, 1.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "tone" + juce::String(i), 1 },
            "Tone " + juce::String(i + 1), 0.0f, 1.0f, 1.0f));
    }
    // Append new IDs after the old parameters to preserve the existing layout.
    add("spaceMix", "Space mix", 0, 0.65f, 0);
    add("spaceSize", "Space size", 0, 1, 0.45f);
    add("spaceDecay", "Space decay", 0, 1, 0.5f);
    add("spaceTone", "Space tone", 0, 1, 0.6f);
    add("spaceMotion", "Space motion", 0, 1, 0.2f);
    add("glitchMix", "Glitch mix", 0, 1, 0);
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "glitchRate", 1 }, "Glitch slice",
        juce::StringArray { "1/16", "1/32", "1/64" }, 1));
    add("lofiMix", "Lo Fi mix", 0, 1, 0);
    add("lofiBits", "Lo Fi bits", 4, 16, 10);
    add("lofiRate", "Lo Fi hold", 1, 32, 4);
    add("timeMix", "Time Warp mix", 0, 1, 0);
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "timeCycle", 1 }, "Time Warp cycle",
        juce::StringArray { "1 beat", "2 beats", "4 beats" }, 2));
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "timeShape", 1 }, "Time Warp interpolation",
        juce::StringArray { "Linear", "Steps" }, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID { "timeSnap", 1 }, "Time grid snap", true));
    for (int i = 0; i < 33; ++i)
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID { "timePoint" + juce::String(i), 1 },
            "Time point " + juce::String(i + 1), 0.0f, 1.0f, 0.0f));
    return layout;
}

DecideratorProcessor::DecideratorProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "DECIDERATOR_STATE", makeParameters())
{
    for (size_t i = 0; i < controls.size(); ++i) controls[i] = parameters.getRawParameterValue(ids[i]);
    for (int i = 0; i < 16; ++i)
    {
        gateSteps[static_cast<size_t>(i)] = parameters.getRawParameterValue("pulse" + juce::String(i));
        toneSteps[static_cast<size_t>(i)] = parameters.getRawParameterValue("tone" + juce::String(i));
    }
    rate = parameters.getRawParameterValue("rate");
    tempo = parameters.getRawParameterValue("tempo");
    echoRate = parameters.getRawParameterValue("echoRate");
    glitchRate = parameters.getRawParameterValue("glitchRate");
    timeCycle = parameters.getRawParameterValue("timeCycle");
    timeShape = parameters.getRawParameterValue("timeShape");
    for (int i = 0; i < 33; ++i) timePoints[static_cast<size_t>(i)] = parameters.getRawParameterValue("timePoint" + juce::String(i));
}

bool DecideratorProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    return layout.inputBuses.size() == 1 && layout.outputBuses.size() == 1
        && layout.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layout.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

double DecideratorProcessor::getTailLengthSeconds() const
{
    if (controls[mix]->load() < 0.00001f) return 0;
    const auto bpm = juce::jlimit(20.0, 400.0, static_cast<double>(currentBpm.load()));
    double seconds = 0;
    if (controls[timeMix]->load() > 0.00001f)
        seconds += 120.0 / bpm * std::pow(2.0, juce::jlimit(0, 2, static_cast<int>(timeCycle->load())));
    if (controls[echo]->load() > 0.00001f)
    {
        const auto fb = static_cast<double>(controls[feedback]->load());
        seconds += 60.0 / bpm * echoBeats[static_cast<size_t>(juce::jlimit(0, 2, static_cast<int>(echoRate->load())))]
            * (1.0 + (fb > 0.001 ? std::log(0.0001) / std::log(fb) : 0.0));
    }
    if (controls[spaceMix]->load() > 0.00001f)
    {
        const auto fb = 0.45 + 0.49 * controls[spaceDecay]->load();
        seconds += 1733.0 / 44100.0 * (1.0 + 5.0 * controls[spaceSize]->load())
            * (1.0 + std::log(0.0001) / std::log(fb));
    }
    return juce::jlimit(0.0, 180.0, seconds);
}

void DecideratorProcessor::prepareToPlay(double sampleRate, int)
{
    sampleRateHz = juce::jmax(8000.0, sampleRate);
    for (size_t i = 0; i < smooth.size(); ++i)
    {
        smooth[i].reset(sampleRateHz, 0.02);
        const auto value = controls[i]->load();
        smooth[i].setCurrentAndTargetValue(i == output ? juce::Decibels::decibelsToGain(value) : value);
    }
    for (auto& channel : delay) channel.assign(static_cast<size_t>(sampleRateHz * 6.0) + 4, 0.0f);
    envelopeCoefficient = static_cast<float>(1.0 - std::exp(-1.0 / (sampleRateHz * 0.003)));
    delaySamples = sampleRateHz * 60.0 / tempo->load() * 0.75;
    space.prepare(sampleRateHz);
    texture.prepare(sampleRateHz);
    timeWarp.prepare(sampleRateHz);
    reset();
}

void DecideratorProcessor::reset()
{
    for (auto& channel : delay) std::fill(channel.begin(), channel.end(), 0.0f);
    lowpass.fill(0.0f);
    writePosition = 0;
    freePpq = 0.0;
    gateEnvelope = toneEnvelope = 1.0f;
    currentStep.store(0);
    space.reset();
    texture.reset();
    timeWarp.reset();
}

void DecideratorProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    midi.clear();
    if (buffer.getNumChannels() < 2 || delay[0].empty()) return;
    for (int channel = 2; channel < buffer.getNumChannels(); ++channel) buffer.clear(channel, 0, buffer.getNumSamples());
    for (size_t i = 0; i < smooth.size(); ++i)
    {
        const auto value = controls[i]->load();
        smooth[i].setTargetValue(i == output ? juce::Decibels::decibelsToGain(value) : value);
    }
    std::array<float, 16> pulse {}, tone {};
    std::array<float, 33> curve {};
    for (size_t i = 0; i < curve.size(); ++i) curve[i] = timePoints[i]->load();
    for (size_t i = 0; i < 16; ++i) { pulse[i] = gateSteps[i]->load(); tone[i] = toneSteps[i]->load(); }
    double bpm = tempo->load(), ppq = freePpq;
    bool running = true, synced = false;
    if (auto* playhead = getPlayHead())
        if (const auto position = playhead->getPosition())
        {
            running = position->getIsPlaying();
            if (const auto hostBpm = position->getBpm())
                if (std::isfinite(*hostBpm) && *hostBpm > 0.0) bpm = *hostBpm;
            if (const auto hostPpq = position->getPpqPosition())
                if (std::isfinite(*hostPpq)) { ppq = *hostPpq; synced = true; }
        }
    bpm = juce::jlimit(20.0, 400.0, bpm);
    currentBpm.store(static_cast<float>(bpm));
    hostClock.store(synced);
    const auto beatsPerStep = stepBeats[static_cast<size_t>(juce::jlimit(0, 4, static_cast<int>(rate->load())))];
    const auto targetDelay = juce::jlimit(1.0, static_cast<double>(delay[0].size() - 2),
        sampleRateHz * 60.0 / bpm * echoBeats[static_cast<size_t>(juce::jlimit(0, 2, static_cast<int>(echoRate->load())))]);
    const auto beatIncrement = running ? bpm / (60.0 * sampleRateHz) : 0.0;
    constexpr std::array<double, 3> cycleBeats { 1.0, 2.0, 4.0 };
    const auto beatsPerCycle = cycleBeats[static_cast<size_t>(juce::jlimit(0, 2, static_cast<int>(timeCycle->load())))];
    const auto cycleSamples = sampleRateHz * 60.0 / bpm * beatsPerCycle;
    const auto steppedTime = timeShape->load() > 0.5f;
    constexpr std::array<double, 3> slices { 0.25, 0.125, 0.0625 };
    const auto sliceSamples = static_cast<int>(sampleRateHz * 60.0 / bpm
        * slices[static_cast<size_t>(juce::jlimit(0, 2, static_cast<int>(glitchRate->load())))]);
    auto* left = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const auto delaySize = static_cast<int>(delay[0].size());
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        std::array<float, numControls> value {};
        for (size_t i = 0; i < smooth.size(); ++i) value[i] = smooth[i].getNextValue();
        const auto stepPosition = ppq / beatsPerStep + 1.0e-9; // Stable exact beat boundaries across block sizes.
        const auto wrapped = stepPosition - std::floor(stepPosition / 16.0) * 16.0;
        const auto step = static_cast<size_t>(juce::jlimit(0, 15, static_cast<int>(wrapped)));
        currentStep.store(static_cast<int>(step), std::memory_order_relaxed);
        gateEnvelope += envelopeCoefficient * (pulse[step] - gateEnvelope);
        toneEnvelope += envelopeCoefficient * (tone[step] - toneEnvelope);
        const auto amplitude = 1.0f - value[gateDepth] + value[gateDepth] * gateEnvelope;
        const auto frequency = juce::jlimit(40.0f, static_cast<float>(sampleRateHz * 0.45),
            value[cutoff] * std::pow(2.0f, -5.0f * value[filterDepth] * (1.0f - toneEnvelope)));
        const auto coefficient = static_cast<float>(1.0 - std::exp(-juce::MathConstants<double>::twoPi * frequency / sampleRateHz));
        const std::array<float, 2> dry { left[sample], right[sample] };
        std::array<float, 2> wet {};
        delaySamples += 0.0005 * (targetDelay - delaySamples);
        auto readPosition = static_cast<double>(writePosition) - delaySamples;
        if (readPosition < 0) readPosition += delaySize;
        const auto readIndex = static_cast<int>(readPosition);
        const auto fraction = static_cast<float>(readPosition - readIndex);
        std::array<float, 2> delayed {};
        for (size_t ch = 0; ch < 2; ++ch)
        {
            auto signal = dry[ch] * amplitude;
            lowpass[ch] += coefficient * (signal - lowpass[ch]);
            signal += value[filterDepth] * (lowpass[ch] - signal);
            if (value[drive] > 0.00001f)
            {
                const auto heat = 1.0f + value[drive] * 14.0f;
                const auto saturated = std::tanh(signal * heat) / std::tanh(heat);
                signal += value[drive] * (saturated - signal);
            }
            const auto a = delay[ch][static_cast<size_t>(readIndex)];
            const auto b = delay[ch][static_cast<size_t>((readIndex + 1) % delaySize)];
            delayed[ch] = a + fraction * (b - a);
            wet[ch] = signal;
        }
        wet = texture.process(wet, static_cast<int>(step), sliceSamples, value[glitchMix], value[lofiMix], value[lofiBits], value[lofiRate]);
        const auto timePosition = ppq / beatsPerCycle + 1.0e-10;
        const auto timePhase = timePosition - std::floor(timePosition);
        wet = timeWarp.process(wet, timePhase, cycleSamples, curve, steppedTime, value[timeMix]);
        for (size_t ch = 0; ch < 2; ++ch)
        {
            delay[ch][static_cast<size_t>(writePosition)] = std::tanh(wet[ch] + delayed[1 - ch] * value[feedback]);
            wet[ch] = (1.0f - value[echo]) * wet[ch] + value[echo] * delayed[ch];
        }
        // Stop feeding the reverb when its mix is zero; process existing tails.
        const auto send = value[spaceMix] > 0.00001f ? 1.0f : 0.0f;
        const auto room = space.process(wet[0] * send, wet[1] * send, value[spaceSize], value[spaceDecay], value[spaceTone], value[spaceMotion]);
        for (size_t ch = 0; ch < 2; ++ch) wet[ch] += value[spaceMix] * (room[ch] - wet[ch]);
        const auto pan = value[motion] * static_cast<float>(std::sin(wrapped / 16.0 * juce::MathConstants<double>::twoPi));
        wet[0] *= 1.0f - juce::jmax(0.0f, pan);
        wet[1] *= 1.0f + juce::jmin(0.0f, pan);
        left[sample] = (dry[0] + value[mix] * (wet[0] - dry[0])) * value[output];
        right[sample] = (dry[1] + value[mix] * (wet[1] - dry[1])) * value[output];
        writePosition = (writePosition + 1) % delaySize;
        ppq += beatIncrement;
    }
    freePpq = ppq;
}

void DecideratorProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto state = parameters.copyState();
    state.setProperty("editorWidth", editorWidth.load(), nullptr);
    state.setProperty("editorHeight", editorHeight.load(), nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, destination);
}

void DecideratorProcessor::setStateInformation(const void* data, int size)
{
    if (data == nullptr || size <= 0) return;
    if (auto xml = getXmlFromBinary(data, size))
        if (xml->hasTagName(parameters.state.getType()))
        {
            const auto incoming = juce::ValueTree::fromXml(*xml);
            // Merge onto defaults so a v0.0.1 gain-only session stays neutral.
            auto merged = juce::ValueTree("DECIDERATOR_STATE");
            for (auto* parameter : getParameters())
                if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter))
                {
                    juce::ValueTree child("PARAM");
                    child.setProperty("id", ranged->paramID, nullptr);
                    child.setProperty("value", ranged->convertFrom0to1(ranged->getDefaultValue()), nullptr);
                    for (const auto& item : incoming)
                        if (item["id"].toString() == ranged->paramID)
                        {
                            const auto v = static_cast<float>(item["value"]);
                            if (std::isfinite(v))
                            {
                                const auto& range = ranged->getNormalisableRange();
                                child.setProperty("value", ranged->convertFrom0to1(ranged->convertTo0to1(juce::jlimit(range.start, range.end, v))), nullptr);
                            }
                        }
                    merged.addChild(child, -1, nullptr);
                }
            parameters.replaceState(merged);
            editorWidth.store(juce::jlimit(750, 1500, static_cast<int>(incoming.getProperty("editorWidth", 800))));
            editorHeight.store(juce::jlimit(467, 902, static_cast<int>(incoming.getProperty("editorHeight", 496))));
        }
}

void DecideratorProcessor::setParameter(const juce::String& id, float value)
{
    if (auto* parameter = parameters.getParameter(id))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

void DecideratorProcessor::applyScene(int index)
{
    previousScene = parameters.copyState();
    setTimeShape(index == 1 ? 1 : 0);
    setParameter("spaceMix", index == 2 ? 0.32f : (index == 3 ? 0.0f : 0.12f));
    setParameter("spaceSize", index == 2 ? 0.75f : 0.45f);
    setParameter("spaceDecay", index == 2 ? 0.8f : 0.5f);
    setParameter("spaceTone", 0.6f); setParameter("spaceMotion", 0.2f);
    setParameter("glitchMix", index == 1 ? 0.5f : 0.0f); setParameter("glitchRate", 1);
    setParameter("lofiMix", index == 1 ? 0.25f : 0.0f);
    setParameter("lofiBits", 10); setParameter("lofiRate", 4);
    setParameter("gainDb", -3.0f);
    setParameter("rate", 2.0f);
    setParameter("echoRate", 1.0f);
    setParameter("mix", 0.85f);
    setParameter("feedback", 0.35f);
    setParameter("cutoff", 9000.0f);
    for (int i = 0; i < 16; ++i)
    {
        float pulse = 1.0f, tone = 1.0f;
        if (index == 0) { pulse = i % 2 == 0 ? 1.0f : 0.08f; tone = i % 4 == 0 ? 1.0f : 0.35f; }
        if (index == 1) { pulse = i % 3 == 0 ? 1.0f : (i % 2 == 0 ? 0.45f : 0.0f); tone = static_cast<float>((i * 7) % 16) / 15.0f; }
        if (index == 2) { pulse = 0.65f + 0.35f * static_cast<float>(std::sin(i * 0.4)); tone = static_cast<float>(i) / 15.0f; }
        setParameter("pulse" + juce::String(i), pulse);
        setParameter("tone" + juce::String(i), tone);
    }
    setParameter("gateDepth", index == 3 ? 0.0f : (index == 2 ? 0.4f : 0.95f));
    setParameter("filterDepth", index == 3 ? 0.0f : (index == 2 ? 0.8f : 0.6f));
    setParameter("drive", index == 3 ? 0.0f : (index == 1 ? 0.3f : 0.08f));
    setParameter("echo", index == 3 ? 0.0f : (index == 2 ? 0.4f : 0.2f));
    setParameter("motion", index == 3 ? 0.0f : (index == 2 ? 0.7f : 0.25f));
    if (index == 3) { setParameter("gainDb", 0); setParameter("mix", 1); }
}

void DecideratorProcessor::generatePattern()
{
    previousScene = parameters.copyState();
    auto& random = juce::Random::getSystemRandom();
    for (int i = 0; i < 16; ++i)
    {
        const auto pulse = i % 4 == 0 ? 1.0f : (random.nextFloat() < 0.4f ? 0.0f : 0.3f + random.nextFloat() * 0.7f);
        setParameter("pulse" + juce::String(i), pulse);
        setParameter("tone" + juce::String(i), 0.1f + random.nextFloat() * 0.9f);
    }
    // Generate musical ranges, never randomise output level or tempo.
    setParameter("gateDepth", 0.65f + random.nextFloat() * 0.35f);
    setParameter("filterDepth", 0.35f + random.nextFloat() * 0.5f);
    setParameter("drive", random.nextFloat() * 0.25f);
    setParameter("echo", 0.1f + random.nextFloat() * 0.25f);
    setParameter("motion", random.nextFloat() * 0.6f);
    setParameter("spaceMix", 0.08f + random.nextFloat() * 0.3f);
    setParameter("glitchMix", random.nextFloat() * 0.5f);
    setParameter("lofiMix", random.nextFloat() * 0.3f);
    setTimeShape(1 + random.nextInt(4));
}

void DecideratorProcessor::applyTimeShape(int index)
{
    previousScene = parameters.copyState();
    setTimeShape(index);
}

void DecideratorProcessor::setTimeShape(int index)
{
    setParameter("timeMix", index == 0 ? 0.0f : 1.0f);
    setParameter("timeCycle", 2);
    setParameter("timeShape", index == 1 ? 1.0f : 0.0f);
    for (int i = 0; i < 33; ++i)
    {
        const auto x = static_cast<float>(i) / 32.0f;
        float value = 0;
        if (index == 1) value = std::floor(x * 16.0f) / 32.0f;
        if (index == 2) value = x;
        if (index == 3) value = 0.25f * x * x;
        if (index == 4) value = 0.35f + 0.28f * std::sin(x * juce::MathConstants<float>::twoPi * 3.0f);
        setParameter("timePoint" + juce::String(i), value);
    }
}

juce::File DecideratorProcessor::presetDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("DECIDERATOR/Presets");
}

juce::Result DecideratorProcessor::savePreset(const juce::File& file)
{
    if (!file.hasFileExtension("decipreset")) return juce::Result::fail("Usa la extension .decipreset");
    auto root = std::make_unique<juce::XmlElement>("DECIDERATOR_PRESET");
    root->setAttribute("schema", 1);
    root->setAttribute("pluginVersion", "0.3.0");
    root->addChildElement(parameters.copyState().createXml().release());
    juce::TemporaryFile temporary(file);
    if (!root->writeTo(temporary.getFile()) || !temporary.overwriteTargetFileWithTemporary())
        return juce::Result::fail("No se pudo escribir el preset. Comprueba carpeta y permisos.");
    return juce::Result::ok();
}

juce::Result DecideratorProcessor::loadPreset(const juce::File& file)
{
    if (!file.existsAsFile() || file.getSize() > 2 * 1024 * 1024) return juce::Result::fail("Archivo inexistente o demasiado grande.");
    auto root = juce::XmlDocument::parse(file);
    if (!root || !root->hasTagName("DECIDERATOR_PRESET") || root->getIntAttribute("schema") != 1)
        return juce::Result::fail("No es un preset DECIDERATOR compatible.");
    auto* state = root->getChildByName("DECIDERATOR_STATE");
    if (state == nullptr) return juce::Result::fail("Preset sin parametros.");
    juce::StringArray seen;
    int recognised = 0;
    for (auto* item : state->getChildIterator())
    {
        const auto id = item->getStringAttribute("id");
        if (seen.contains(id)) return juce::Result::fail("Preset con parametros duplicados.");
        seen.add(id);
        if (auto* parameter = parameters.getParameter(id))
        {
            const auto text = item->getStringAttribute("value").trim();
            const auto* first = text.toRawUTF8();
            const auto* end = first + text.getNumBytesAsUTF8();
            float value = 0;
            const auto parsed = std::from_chars(first, end, value);
            const auto& range = parameter->getNormalisableRange();
            if (parsed.ec != std::errc{} || parsed.ptr != end || !std::isfinite(value) || value < range.start || value > range.end)
                return juce::Result::fail("Valor de parametro invalido: " + id);
            ++recognised;
        }
    }
    if (recognised == 0) return juce::Result::fail("Preset sin parametros reconocidos.");
    previousScene = parameters.copyState();
    state->setAttribute("editorWidth", editorWidth.load());
    state->setAttribute("editorHeight", editorHeight.load());
    juce::MemoryBlock binary; copyXmlToBinary(*state, binary);
    setStateInformation(binary.getData(), static_cast<int>(binary.getSize()));
    return juce::Result::ok();
}

void DecideratorProcessor::undoScene()
{
    if (previousScene.isValid())
    {
        auto current = parameters.copyState();
        parameters.replaceState(previousScene);
        previousScene = current;
    }
}

juce::AudioProcessorEditor* DecideratorProcessor::createEditor() { return new DecideratorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DecideratorProcessor(); }
