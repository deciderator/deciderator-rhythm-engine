// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "PluginProcessor.h"

class TimeGraph final : public juce::Component
{
public:
    explicit TimeGraph(DecideratorProcessor&);
    ~TimeGraph() override;
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseDoubleClick(const juce::MouseEvent&) override;
    std::function<void(int)> onSelection;
private:
    juce::Rectangle<float> plot() const;
    void edit(const juce::MouseEvent&, bool interpolate);
    void setPoint(int, float);
    void endEdit();
    DecideratorProcessor& processor;
    std::array<juce::RangedAudioParameter*, 33> points {};
    std::array<bool, 33> gestures {};
    int selected = 0, lastIndex = -1;
    float lastValue = 0;
};

class TimeLab final : public juce::Component, private juce::Timer
{
public:
    explicit TimeLab(DecideratorProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override { if (isShowing()) repaint(); }
    DecideratorProcessor& processor;
    TimeGraph graph;
    std::array<juce::TextButton, 5> shapes;
    juce::Slider mix, pointValue;
    juce::ComboBox cycle, interpolation;
    juce::ToggleButton snap { "SNAP" };
    juce::Label pointLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment, pointAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> cycleAttachment, interpolationAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> snapAttachment;
};

class FxInspector final : public juce::Component
{
public:
    explicit FxInspector(DecideratorProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    std::array<juce::Slider, 10> knobs;
    std::array<juce::Label, 10> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 10> attachments;
};

class DetailWindow final : public juce::DocumentWindow
{
public:
    DetailWindow(const juce::String&, juce::Component*, int width, int height);
    ~DetailWindow() override;
    void closeButtonPressed() override { setVisible(false); }
private:
    juce::LookAndFeel_V4 appearance;
};
