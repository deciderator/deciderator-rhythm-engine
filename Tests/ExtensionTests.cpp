// SPDX-License-Identifier: AGPL-3.0-only
#include "../Source/PluginProcessor.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace
{
void check(bool test, const char* message) { if (!test) throw std::runtime_error(message); }
void set(DecideratorProcessor& p, const char* id, float value)
{
    auto* parameter = p.parameters.getParameter(id);
    check(parameter != nullptr, "New parameter missing");
    parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}
}

void runExtensionTests()
{
    // Nontrivial reverb impulse and decay; no feedback blow-up at maximum settings.
    SpaceFDN space;
    space.prepare(48000);
    double early = 0, late = 0;
    for (int i = 0; i < 144000; ++i)
    {
        const auto result = space.process(i == 0 ? 1.0f : 0.0f, 0, 0.45f, 0.4f, 0.6f, 0.2f);
        for (auto sample : result)
        {
            check(std::isfinite(sample) && std::abs(sample) < 4, "Reverb impulse finite");
            if (i < 48000) early += sample * sample;
            if (i >= 96000) late += sample * sample;
        }
    }
    check(early > 0.001 && late < early * 0.3, "Reverb produces decaying tail");
    space.reset();
    for (int i = 0; i < 2048; ++i)
    {
        const auto result = space.process(0, 0, 1, 1, 1, 1);
        check(result[0] == 0 && result[1] == 0, "Reverb reset silence");
    }

    TextureEngine texture;
    texture.prepare(48000);
    for (int i = 0; i < 2048; ++i) texture.process({ i * 0.0001f, i * -0.0001f }, 0, 64, 0, 0, 8, 4);
    std::array<float, 2200> repeated {};
    for (int i = 0; i < 2200; ++i) repeated[static_cast<size_t>(i)] = texture.process({ -0.4f, 0.4f }, 1, 64, 1, 0, 8, 4)[0];
    for (int i = 2000; i < 2100; ++i)
        check(std::abs(repeated[static_cast<size_t>(i)] - repeated[static_cast<size_t>(i + 64)]) < 1.0e-5f, "Stutter slice periodicity");
    texture.reset();
    float held = 0;
    for (int i = 0; i < 512; ++i)
    {
        const auto out = texture.process({ static_cast<float>(std::sin(i * 0.09)), 0 }, 0, 64, 0, 1, 4, 8)[0];
        check(std::abs(out * 8 - std::round(out * 8)) < 1.0e-6f, "Lo-fi bit quantisation");
        if (i % 8 == 0) held = out;
        else check(out == held, "Lo-fi sample hold");
    }

    DecideratorProcessor p;
    p.applyScene(2);
    set(p, "spaceMix", 0.42f); set(p, "glitchMix", 0.7f); set(p, "lofiBits", 7);
    p.editorWidth.store(913); p.editorHeight.store(543);
    const auto directory = juce::File::getCurrentWorkingDirectory().getChildFile("deciderator-preset-tests-" + juce::Uuid().toString());
    check(directory.createDirectory().wasOk(), "Preset test directory");
    const auto preset = directory.getChildFile("Roundtrip.decipreset");
    check(p.savePreset(preset).wasOk(), "Preset save");
    DecideratorProcessor restored;
    check(restored.loadPreset(preset).wasOk(), "Preset load");
    for (auto* parameter : p.getParameters())
    {
        auto* ranged = dynamic_cast<juce::RangedAudioParameter*>(parameter);
        check(ranged != nullptr, "Preset ranged parameter");
        check(std::abs(ranged->getValue() - restored.parameters.getParameter(ranged->paramID)->getValue()) < 1.0e-5f, "Preset all-parameter roundtrip");
    }
    check(restored.editorWidth.load() == 800, "Preset preserves current UI size");
    const auto invalid = directory.getChildFile("Invalid.decipreset");
    const auto initial = restored.parameters.getRawParameterValue("spaceMix")->load();
    for (const auto* bad : {
        "<wrong/>",
        "<DECIDERATOR_PRESET schema=\"2\"><DECIDERATOR_STATE/></DECIDERATOR_PRESET>",
        "<DECIDERATOR_PRESET schema=\"1\"><DECIDERATOR_STATE><PARAM id=\"spaceMix\" value=\"nan\"/></DECIDERATOR_STATE></DECIDERATOR_PRESET>",
        "<DECIDERATOR_PRESET schema=\"1\"><DECIDERATOR_STATE><PARAM id=\"spaceMix\" value=\"999\"/></DECIDERATOR_STATE></DECIDERATOR_PRESET>",
        "<DECIDERATOR_PRESET schema=\"1\"><DECIDERATOR_STATE><PARAM id=\"spaceMix\" value=\"oops\"/></DECIDERATOR_STATE></DECIDERATOR_PRESET>",
        "<DECIDERATOR_PRESET schema=\"1\"><DECIDERATOR_STATE><PARAM id=\"spaceMix\" value=\"0.2\"/><PARAM id=\"spaceMix\" value=\"0.3\"/></DECIDERATOR_STATE></DECIDERATOR_PRESET>" })
    {
        check(invalid.replaceWithText(bad), "Invalid fixture write");
        check(restored.loadPreset(invalid).failed(), "Reject malformed preset");
        check(restored.parameters.getRawParameterValue("spaceMix")->load() == initial, "Invalid preset leaves state intact");
    }
    check(p.savePreset(directory.getChildFile("Bad.txt")).failed(), "Reject wrong save extension");
    check(p.savePreset(directory.getChildFile("missing/Bad.decipreset")).failed(), "Report save failure");
    invalid.deleteFile(); preset.deleteFile(); directory.deleteFile();

    // Legacy host state has no new FX parameters: restore neutral values.
    const auto legacyXml = juce::XmlDocument::parse("<DECIDERATOR_STATE><PARAM id=\"gainDb\" value=\"-6\"/></DECIDERATOR_STATE>");
    juce::MemoryBlock legacy;
    juce::AudioProcessor::copyXmlToBinary(*legacyXml, legacy);
    restored.setStateInformation(legacy.getData(), static_cast<int>(legacy.getSize()));
    for (const auto* id : { "spaceMix", "glitchMix", "lofiMix" }) check(restored.parameters.getRawParameterValue(id)->load() == 0, "Legacy new FX neutral");

    juce::MidiBuffer midi;
    for (const auto sr : { 44100.0, 48000.0, 96000.0 })
    {
        DecideratorProcessor stress;
        stress.applyScene(1);
        for (const auto* id : { "spaceSize", "spaceDecay", "spaceTone", "spaceMotion", "glitchMix", "lofiMix" }) set(stress, id, 1);
        set(stress, "spaceMix", 0.65f); set(stress, "lofiBits", 4); set(stress, "lofiRate", 32);
        stress.prepareToPlay(sr, 257);
        juce::AudioBuffer<float> buffer(2, 257);
        for (int block = 0; block < 400; ++block)
        {
            for (int i = 0; i < 257; ++i)
            {
                buffer.setSample(0, i, block < 200 ? 2.0f * static_cast<float>(std::sin((block * 257 + i) * 0.11)) : 0);
                buffer.setSample(1, i, block < 200 ? 2.0f * static_cast<float>(std::cos((block * 257 + i) * 0.07)) : 0);
            }
            stress.processBlock(buffer, midi);
            for (int ch = 0; ch < 2; ++ch)
                for (int i = 0; i < 257; ++i) check(std::isfinite(buffer.getSample(ch, i)) && std::abs(buffer.getSample(ch, i)) < 16, "Extended effects stress");
        }
    }
    std::cout << "PASS: reverb impulse/decay/reset, stutter repeat, digital lo-fi, preset roundtrip/rejection/I-O errors, legacy FX migration, extended 3-rate stress\n";
}
