// SPDX-License-Identifier: AGPL-3.0-only
#include "../Source/PluginProcessor.h"
#include "../Source/DetailEditors.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace
{
void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
std::array<float, 33> values(const DecideratorProcessor& p)
{
    std::array<float, 33> out {};
    for (int i = 0; i < 33; ++i) out[static_cast<size_t>(i)] = p.parameters.getRawParameterValue("timePoint" + juce::String(i))->load();
    return out;
}
}

void runTimeWarpTests()
{
    std::array<float, 33> curve {}, direct {};
    for (int i = 0; i < 33; ++i) curve[static_cast<size_t>(i)] = static_cast<float>(i) / 32;
    check(std::abs(TimeWarpEngine::curveValue(curve, 0.2, false) - 0.2f) < 1.0e-6f, "Time curve linear interpolation");
    check(std::abs(TimeWarpEngine::curveValue(curve, 0.2, true) - 6.0f / 32) < 1.0e-6f, "Time curve stepped interpolation");
    TimeWarpEngine warp;
    warp.prepare(48000);
    for (int i = 0; i < 40000; ++i)
    {
        const auto sample = static_cast<float>(std::sin(i * 0.01));
        const auto out = warp.process({ sample, -sample }, (i % 4000) / 4000.0, 4000, direct, false, 1);
        check(out[0] == sample && out[1] == -sample, "Flat time curve transparent");
    }
    // Independent expected audio: reverse source traversal, then the specified DC filter.
    float previousInput = static_cast<float>(std::sin(40000 * 0.01)), previousOutput = 0, largestError = 0;
    const auto coefficient = static_cast<float>(std::exp(-6.283185307179586 * 5.0 / 48000));
    for (int i = 0; i < 4000; ++i)
    {
        const auto input = static_cast<float>(std::sin((40000 + i) * 0.01));
        const auto out = warp.process({ input, -input }, i / 4000.0, 4000, curve, false, 1);
        const auto raw = static_cast<float>(std::sin((40000 - i) * 0.01));
        const auto expected = i == 0 ? raw : raw - previousInput + coefficient * previousOutput;
        previousInput = raw; previousOutput = i == 0 ? 0 : expected;
        largestError = std::max(largestError, std::abs(out[0] - expected));
        check(std::abs(out[1] + out[0]) < 1.0e-5f, "Time warp stereo linkage");
    }
    check(largestError < 0.0002f, "Reverse reads prior audio backwards");
    for (int i = 0; i < 1000; ++i)
    {
        const auto input = static_cast<float>(std::cos(i * 0.13));
        const auto out = warp.process({ input, -input }, (i % 31) / 31.0, 24000, curve, true, 0);
        check(out[0] == input && out[1] == -input, "Time mix zero transparent");
    }
    DecideratorProcessor p;
    p.applyTimeShape(1);
    check(values(p)[8] > values(p)[0] && p.parameters.getRawParameterValue("timeShape")->load() == 1, "Chop template retriggers");
    p.applyTimeShape(3);
    const auto stopped = values(p);
    check(std::abs(stopped[32] - 0.25f) < 1.0e-6f && stopped[16] < stopped[32] * 0.4f, "Tape stop deceleration curve");
    p.applyTimeShape(4); const auto scratch = values(p);
    check(scratch[3] > scratch[8] && scratch[8] < scratch[14], "Scratch reverses delay slope");
    juce::MemoryBlock state; p.getStateInformation(state);
    DecideratorProcessor restored; restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
    check(values(restored) == scratch, "Curve state roundtrip");
    p.applyTimeShape(0); check(p.parameters.getRawParameterValue("timeMix")->load() == 0, "Direct template bypasses warp");
    p.undoScene(); check(values(p) == scratch, "Time template undo");
    {
        TimeLab lab(restored); lab.setSize(1000, 540);
        FxInspector inspector(restored); inspector.setSize(760, 390);
        check(lab.getWidth() == 1000 && inspector.getHeight() == 390, "Independent editor contents");
    }
    juce::MidiBuffer midi;
    for (const auto sr : { 44100.0, 48000.0, 96000.0 })
    {
        DecideratorProcessor stress; stress.prepareToPlay(sr, 257);
        juce::AudioBuffer<float> block(2, 257);
        for (int j = 0; j < 600; ++j)
        {
            if (j % 120 == 0) stress.applyTimeShape((j / 120) % 5);
            for (int i = 0; i < 257; ++i)
            {
                block.setSample(0, i, 0.4f * static_cast<float>(std::sin((j * 257 + i) * 0.09)));
                block.setSample(1, i, -block.getSample(0, i));
            }
            stress.processBlock(block, midi);
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 257; ++i) check(std::isfinite(block.getSample(c, i)) && std::abs(block.getSample(c, i)) < 4, "Time warp finite under pattern changes");
        }
    }
    std::cout << "PASS: curve interpolation, direct/dry transparency, reverse audio reference, stereo linkage, templates/state/undo, independent editor contents, time warp stress\n";
}
