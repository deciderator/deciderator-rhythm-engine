// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#include "PluginProcessor.h"

class DecideratorLook final : public juce::LookAndFeel_V4
{
public:
    DecideratorLook();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float,
                          juce::Slider::SliderStyle, juce::Slider&) override;
};

class DecideratorPanel final : public juce::Component, private juce::Timer
{
public:
    explicit DecideratorPanel(DecideratorProcessor&);
    ~DecideratorPanel() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void showTextureControls(bool);
private:
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    void timerCallback() override;
    DecideratorProcessor& processor;
    DecideratorLook look;
    std::array<juce::Slider, 16> pulse, tone;
    std::array<std::unique_ptr<Attachment>, 16> pulseAttachments, toneAttachments;
    std::array<juce::Slider, 9> knobs;
    std::array<juce::Label, 9> knobLabels;
    std::array<std::unique_ptr<Attachment>, 9> knobAttachments;
    std::array<juce::Slider, 10> textureKnobs;
    std::array<juce::Label, 10> textureLabels;
    std::array<std::unique_ptr<Attachment>, 10> textureAttachments;
    juce::TextButton texturePage { "SPACE / FX" };
    bool textureVisible = false;
    juce::ComboBox stepRate, echoRate;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAttachment, echoAttachment;
    juce::Slider tempo;
    std::unique_ptr<Attachment> tempoAttachment;
    juce::TextButton pulseScene { "01 / PULSE" }, shardsScene { "02 / SHARDS" }, driftScene { "03 / DRIFT" },
                     neutralScene { "NEUTRAL" }, generate { "GENERATE" }, undo { "UNDO / REDO" };
    juce::TooltipWindow tooltips { this, 600 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DecideratorPanel)
};

class DecideratorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DecideratorEditor(DecideratorProcessor&);
    ~DecideratorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void setZoomPercent(int percent);
    juce::Rectangle<int> getCanvasBounds() const { return panel.getBoundsInParent(); }
    void showTextureControls(bool show) { panel.showTextureControls(show); }
private:
    void choosePreset(bool save);
    void refreshPresets();
    void selectPreset();
    void showLegal();
    DecideratorProcessor& processor;
    DecideratorLook look;
    DecideratorPanel panel;
    juce::ComboBox zoom;
    juce::ComboBox presetList;
    juce::TextButton savePresetButton { "GUARDAR" }, loadPresetButton { "CARGAR" },
                     previousPreset { "<" }, nextPreset { ">" }, legalButton { "CREDITOS" };
    std::unique_ptr<juce::FileChooser> chooser;
    std::unique_ptr<juce::DocumentWindow> legalWindow;
    juce::Array<juce::File> presetFiles;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DecideratorEditor)
};
