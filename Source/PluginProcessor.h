// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <array>
#include <vector>
#include "CreativeDSP.h"
#include "TimeWarp.h"

class DecideratorProcessor final : public juce::AudioProcessor
{
public:
    DecideratorProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "DECIDERATOR Rhythm Engine"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override;
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    void applyScene(int index);
    void generatePattern();
    void undoScene();
    juce::Result savePreset(const juce::File&);
    juce::Result loadPreset(const juce::File&);
    static juce::File presetDirectory();
    void applyTimeShape(int index);
    const TimeWarpEngine& getTimeWarp() const { return timeWarp; }
    std::atomic<int> currentStep { 0 };
    std::atomic<float> currentBpm { 120.0f };
    std::atomic<bool> hostClock { false };
    std::atomic<int> editorWidth { 800 }, editorHeight { 496 };
    juce::AudioProcessorValueTreeState parameters;
private:
    enum Control { output, gateDepth, filterDepth, cutoff, drive, echo, feedback, motion, mix,
                   spaceMix, spaceSize, spaceDecay, spaceTone, spaceMotion, glitchMix, lofiMix, lofiBits, lofiRate, timeMix, numControls };
    static juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();
    void setParameter(const juce::String&, float);
    void setTimeShape(int index);
    std::array<std::atomic<float>*, numControls> controls {};
    std::array<juce::SmoothedValue<float>, numControls> smooth;
    std::array<std::atomic<float>*, 16> gateSteps {}, toneSteps {};
    std::atomic<float>* rate = nullptr;
    std::atomic<float>* tempo = nullptr;
    std::atomic<float>* echoRate = nullptr;
    std::atomic<float>* glitchRate = nullptr;
    std::atomic<float>* timeCycle = nullptr;
    std::atomic<float>* timeShape = nullptr;
    std::array<std::atomic<float>*, 33> timePoints {};
    TimeWarpEngine timeWarp;
    SpaceFDN space;
    TextureEngine texture;
    std::array<std::vector<float>, 2> delay;
    std::array<float, 2> lowpass {};
    int writePosition = 0;
    double sampleRateHz = 48000.0, freePpq = 0.0;
    float gateEnvelope = 1.0f, toneEnvelope = 1.0f, envelopeCoefficient = 0.0f;
    double delaySamples = 18000.0;
    juce::ValueTree previousScene;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DecideratorProcessor)
};
