// SPDX-License-Identifier: AGPL-3.0-only
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#if JUCE_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

static void require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}
static void setGain(DecideratorProcessor& p, float db)
{
    auto* parameter = p.parameters.getParameter("gainDb");
    parameter->setValueNotifyingHost(parameter->convertTo0to1(db));
}
static void fill(juce::AudioBuffer<float>& b)
{
    for (int i = 0; i < b.getNumSamples(); ++i)
    {
        b.setSample(0, i, 0.125f);
        b.setSample(1, i, -0.25f);
    }
}
void runCreativeTests();
void runExtensionTests();
int main(int argc, char** argv)
{
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
#if JUCE_WINDOWS
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif
    juce::ScopedJuceInitialiser_GUI init;
    try
    {
        DecideratorProcessor p;
        auto layout = p.getBusesLayout();
        require(p.isBusesLayoutSupported(layout), "Stereo rejected");
        layout.inputBuses.set(0, juce::AudioChannelSet::mono());
        layout.outputBuses.set(0, juce::AudioChannelSet::mono());
        require(!p.isBusesLayoutSupported(layout), "Mono accepted");
        juce::MidiBuffer midi;
        for (const auto rate : { 44100.0, 48000.0, 96000.0 })
        for (const auto size : { 1, 64, 512, 1024 })
        for (const auto db : { -60.0f, -6.0f, 0.0f, 12.0f })
        {
            setGain(p, db);
            p.prepareToPlay(rate, size);
            juce::AudioBuffer<float> buffer(2, size);
            fill(buffer);
            p.processBlock(buffer, midi);
            const auto expected = std::pow(10.0f, db / 20.0f);
            for (int i = 0; i < size; ++i)
            {
                require(std::abs(buffer.getSample(0, i) - 0.125f * expected) < 1.0e-6f, "Left gain");
                require(std::abs(buffer.getSample(1, i) + 0.25f * expected) < 1.0e-6f, "Right gain");
            }
        }
        setGain(p, 0.0f);
        p.prepareToPlay(48000.0, 1024);
        setGain(p, -60.0f);
        juce::AudioBuffer<float> ramp(2, 1024);
        fill(ramp);
        p.processBlock(ramp, midi);
        require(ramp.getSample(0, 0) < 0.125f && ramp.getSample(0, 0) > 0.12f, "Ramp start");
        for (int i = 1; i < 1024; ++i)
            require(ramp.getSample(0, i) <= ramp.getSample(0, i - 1), "Ramp monotonicity");
        require(std::abs(ramp.getSample(0, 1023) - 0.000125f) < 1.0e-6f, "Ramp completion");
        setGain(p, -17.25f);
        juce::MemoryBlock state;
        p.getStateInformation(state);
        DecideratorProcessor restored;
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        require(std::abs(restored.parameters.getRawParameterValue("gainDb")->load() + 17.25f) < 0.001f, "State roundtrip");
        restored.setStateInformation(nullptr, 0);
        const char invalid[] = "invalid state";
        restored.setStateInformation(invalid, sizeof(invalid));
        require(std::abs(restored.parameters.getRawParameterValue("gainDb")->load() + 17.25f) < 0.001f, "Invalid state changed gain");
        juce::AudioBuffer<float> empty(2, 0);
        restored.prepareToPlay(48000.0, 512);
        restored.processBlock(empty, midi);
        std::cout << "PASS: layouts, stereo gain, 48 configurations, smoothing, state, invalid state, empty block\n";
        runCreativeTests();
        runExtensionTests();
        {
            DecideratorProcessor ui;
            std::unique_ptr<juce::AudioProcessorEditor> editor(ui.createEditor());
            auto* scalable = dynamic_cast<DecideratorEditor*>(editor.get());
            require(scalable != nullptr, "Scalable editor");
            require(editor->getWidth() == 800 && editor->getHeight() == 496, "Compact initial window");
            for (const auto percent : { 75, 80, 90, 100, 125, 150 })
            {
                scalable->setZoomPercent(percent);
                require(editor->getWidth() == percent * 10, "Zoom width");
                require(editor->getLocalBounds().withTrimmedTop(32).expanded(1).contains(scalable->getCanvasBounds()), "Zoom canvas fits");
            }
            editor->setSize(913, 543);
            require(editor->getLocalBounds().withTrimmedTop(32).expanded(1).contains(scalable->getCanvasBounds()), "Manual resize canvas fits");
            juce::MemoryBlock uiState; ui.getStateInformation(uiState);
            DecideratorProcessor reopened;
            reopened.setStateInformation(uiState.getData(), static_cast<int>(uiState.getSize()));
            std::unique_ptr<juce::AudioProcessorEditor> reopenedEditor(reopened.createEditor());
            require(reopenedEditor->getWidth() == 913 && reopenedEditor->getHeight() == 543, "Editor size persists");
            std::cout << "PASS: six zoom sizes, custom resize, compact default, editor size persistence\n";
        }
        if (argc >= 3 && juce::String(argv[1]) == "--preview")
        {
            DecideratorProcessor preview;
            preview.applyScene(1);
            std::unique_ptr<juce::AudioProcessorEditor> editor(preview.createEditor());
            if (argc >= 4)
                if (auto* scalable = dynamic_cast<DecideratorEditor*>(editor.get())) scalable->setZoomPercent(juce::String(argv[3]).getIntValue());
            if (argc >= 5 && juce::String(argv[4]) == "fx")
                if (auto* scalable = dynamic_cast<DecideratorEditor*>(editor.get())) scalable->showTextureControls(true);
            auto snapshot = editor->createComponentSnapshot(editor->getLocalBounds());
            juce::File file(juce::String::fromUTF8(argv[2]));
            auto stream = file.createOutputStream();
            require(stream != nullptr, "Preview file");
            require(stream->setPosition(0), "Preview rewind");
            require(stream->truncate().wasOk(), "Preview truncate");
            require(juce::PNGImageFormat().writeImageToStream(snapshot, *stream), "Preview render");
            std::cout << "PASS: editor snapshot rendered\n";
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
