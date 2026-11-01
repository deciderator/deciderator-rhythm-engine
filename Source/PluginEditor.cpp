// SPDX-License-Identifier: AGPL-3.0-only
#include "PluginEditor.h"
#include "BinaryData.h"

namespace
{
const juce::Colour background(0xff0d121b), panel(0xff17212e), ink(0xffedf3f5), muted(0xff8b9baa),
                   mint(0xffb5f268), blue(0xff73ccf6), track(0xff2c3a48);
}

DecideratorLook::DecideratorLook()
{
    setColour(juce::Slider::textBoxTextColourId, ink);
    setColour(juce::Slider::textBoxBackgroundColourId, background);
    setColour(juce::Slider::textBoxOutlineColourId, track);
    setColour(juce::Label::textColourId, ink);
    setColour(juce::TextButton::buttonColourId, panel);
    setColour(juce::TextButton::textColourOffId, ink);
    setColour(juce::ComboBox::backgroundColourId, panel);
    setColour(juce::ComboBox::textColourId, ink);
    setColour(juce::ComboBox::outlineColourId, track);
    setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, ink);
}

void DecideratorLook::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                      float position, float start, float end, juce::Slider& slider)
{
    auto area = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(width), static_cast<float>(height)).reduced(9.0f);
    const auto radius = juce::jmin(area.getWidth(), area.getHeight()) * 0.5f;
    const auto centre = area.getCentre();
    juce::Path base, arc;
    base.addCentredArc(centre.x, centre.y, radius, radius, 0, start, end, true);
    arc.addCentredArc(centre.x, centre.y, radius, radius, 0, start, start + position * (end - start), true);
    g.setColour(track); g.strokePath(base, juce::PathStrokeType(4.0f));
    g.setColour(slider.findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(arc, juce::PathStrokeType(4.0f));
    g.fillEllipse(centre.x - 3, centre.y - 3, 6, 6);
}

void DecideratorLook::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                      float sliderPosition, float, float, juce::Slider::SliderStyle,
                                      juce::Slider& slider)
{
    auto area = juce::Rectangle<float>(static_cast<float>(x + 3), static_cast<float>(y),
        static_cast<float>(width - 6), static_cast<float>(height));
    g.setColour(track.withAlpha(0.65f)); g.fillRoundedRectangle(area, 3);
    const auto top = juce::jlimit(area.getY(), area.getBottom(), sliderPosition);
    g.setColour(slider.findColour(juce::Slider::trackColourId).withAlpha(0.8f));
    g.fillRoundedRectangle(area.withTop(top), 3);
    g.setColour(ink); g.fillRect(area.getX(), top, area.getWidth(), 2.0f);
}

DecideratorPanel::DecideratorPanel(DecideratorProcessor& p) : processor(p)
{
    setLookAndFeel(&look);
    const std::array<const char*, 9> ids { "gateDepth", "filterDepth", "cutoff", "drive", "echo", "feedback", "motion", "mix", "gainDb" };
    const std::array<const char*, 9> names { "PULSE", "TONE", "CEILING", "HEAT", "ECHO", "FEEDBACK", "ORBIT", "DRY / WET", "OUTPUT" };
    const std::array<const char*, 9> hints {
        "Profundidad del patron de volumen. 0 = sin gate.",
        "Profundidad del patron de filtro. 0 = filtro desactivado.",
        "Frecuencia maxima del filtro; el patron Tone lo oscurece.",
        "Saturacion suave para dar textura. Puede aumentar el volumen.",
        "Mezcla del eco ping-pong sincronizado.", "Repeticiones del eco; limitado a 0.7.",
        "Movimiento panoramico durante el ciclo de 16 pasos.",
        "0 = audio original; 1 = cadena completa. Output se aplica a ambos.",
        "Ganancia final de -60 a +12 dB. Doble clic = 0 dB." };
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& slider = knobs[i];
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 85, 24);
        slider.setColour(juce::Slider::rotarySliderFillColourId, i == 1 || i == 2 ? blue : mint);
        slider.setTooltip(hints[i]); slider.setTitle(names[i]);
        if (i == 2) slider.setTextValueSuffix(" Hz");
        if (i == 8) { slider.setTextValueSuffix(" dB"); slider.setDoubleClickReturnValue(true, 0); }
        addAndMakeVisible(slider);
        knobLabels[i].setText(names[i], juce::dontSendNotification);
        knobLabels[i].setJustificationType(juce::Justification::centred);
        knobLabels[i].setFont(juce::Font(juce::FontOptions(11.0f)));
        addAndMakeVisible(knobLabels[i]);
        knobAttachments[i] = std::make_unique<Attachment>(p.parameters, ids[i], slider);
        slider.setNumDecimalPlacesToDisplay(i == 2 ? 0 : 2);
        slider.textFromValueFunction = [places = i == 2 ? 0 : 2](double value) { return juce::String(value, places); };
        slider.updateText();
    }
    for (size_t i = 0; i < 16; ++i)
    {
        for (auto* slider : { &pulse[i], &tone[i] })
        {
            slider->setSliderStyle(juce::Slider::LinearVertical);
            slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
            slider->setDoubleClickReturnValue(true, 1);
            addAndMakeVisible(*slider);
        }
        pulse[i].setColour(juce::Slider::trackColourId, mint);
        tone[i].setColour(juce::Slider::trackColourId, blue);
        pulse[i].setTitle("Pulse paso " + juce::String(static_cast<int>(i + 1)));
        tone[i].setTitle("Tone paso " + juce::String(static_cast<int>(i + 1)));
        pulse[i].setTooltip("Arrastra para editar volumen del paso. Doble clic = maximo.");
        tone[i].setTooltip("Arrastra para editar apertura del filtro. Doble clic = maximo.");
        pulseAttachments[i] = std::make_unique<Attachment>(p.parameters, "pulse" + juce::String(static_cast<int>(i)), pulse[i]);
        toneAttachments[i] = std::make_unique<Attachment>(p.parameters, "tone" + juce::String(static_cast<int>(i)), tone[i]);
    }
    stepRate.addItemList({ "1/4", "1/8", "1/16", "1/32", "1/16T" }, 1);
    echoRate.addItemList({ "1/8", "1/8 dotted", "1/4" }, 1);
    stepRate.setTooltip("Duracion de cada paso. El ciclo contiene 16 pasos.");
    echoRate.setTooltip("Tiempo de eco sincronizado al tempo.");
    addAndMakeVisible(stepRate); addAndMakeVisible(echoRate);
    rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "rate", stepRate);
    echoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "echoRate", echoRate);
    tempo.setSliderStyle(juce::Slider::IncDecButtons);
    tempo.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 65, 24);
    tempo.setTooltip("Tempo de respaldo. Se usa cuando el host no entrega BPM.");
    addAndMakeVisible(tempo);
    tempoAttachment = std::make_unique<Attachment>(p.parameters, "tempo", tempo);
    tempo.setNumDecimalPlacesToDisplay(0);
    tempo.textFromValueFunction = [](double value) { return juce::String(value, 0); };
    tempo.updateText();
    for (auto* button : { &pulseScene, &shardsScene, &driftScene, &neutralScene, &generate, &undo }) addAndMakeVisible(*button);
    pulseScene.onClick = [&p] { p.applyScene(0); };
    shardsScene.onClick = [&p] { p.applyScene(1); };
    driftScene.onClick = [&p] { p.applyScene(2); };
    neutralScene.onClick = [&p] { p.applyScene(3); };
    generate.onClick = [&p] { p.generatePattern(); };
    undo.onClick = [&p] { p.undoScene(); };
    generate.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff37552b));
    generate.setTooltip("Genera patrones y cantidades de efectos. Conserva tempo, mezcla y salida.");
    undo.setTooltip("Alterna con el estado anterior a la ultima escena o generacion. No es historial de edicion.");
    const std::array<const char*, 10> fxIds { "spaceMix", "spaceSize", "spaceDecay", "spaceTone", "spaceMotion", "glitchMix", "glitchRate", "lofiMix", "lofiBits", "lofiRate" };
    const std::array<const char*, 10> fxNames { "SPACE", "SIZE", "DECAY", "AIR", "SPACE MOD", "STUTTER", "SLICE", "LO-FI", "BITS", "HOLD" };
    const std::array<const char*, 10> fxHints {
        "Mezcla de reverb FDN adaptada de Aquaton; ver CREDITOS.", "Tamano del espacio reverberante.",
        "Duracion de la cola, con feedback acotado.", "Brillo de la reverb.", "Modulacion de retardos, sin pitch-shifter de octava.",
        "Repite fragmentos anteriores en pasos pares (02, 04, ...).", "Duracion del fragmento capturado: 1/16, 1/32 o 1/64.",
        "Mezcla de degradacion digital.", "Resolucion de cuantizacion (4 a 16 bits).", "Mantiene cada muestra de 1 a 32 ciclos: reduce frecuencia efectiva." };
    for (size_t i = 0; i < textureKnobs.size(); ++i)
    {
        auto& slider = textureKnobs[i];
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 24);
        slider.setColour(juce::Slider::rotarySliderFillColourId, blue);
        slider.setTooltip(fxHints[i]); slider.setTitle(fxNames[i]);
        addChildComponent(slider);
        textureLabels[i].setText(fxNames[i], juce::dontSendNotification);
        textureLabels[i].setJustificationType(juce::Justification::centred);
        textureLabels[i].setFont(juce::Font(juce::FontOptions(11.0f)));
        addChildComponent(textureLabels[i]);
        textureAttachments[i] = std::make_unique<Attachment>(p.parameters, fxIds[i], slider);
        slider.textFromValueFunction = [i](double value)
        {
            if (i == 6) return juce::StringArray { "1/16", "1/32", "1/64" }[juce::jlimit(0, 2, juce::roundToInt(value))];
            return juce::String(value, i >= 8 ? 0 : 2);
        };
        slider.valueFromTextFunction = [i](const juce::String& text)
        {
            if (i == 6) return text.contains("64") ? 2.0 : (text.contains("32") ? 1.0 : 0.0);
            return text.getDoubleValue();
        };
        slider.updateText();
    }
    addAndMakeVisible(texturePage);
    texturePage.setClickingTogglesState(true);
    texturePage.onClick = [this] { showTextureControls(texturePage.getToggleState()); };
    addAndMakeVisible(timeLabButton); addAndMakeVisible(fxWindowButton);
    timeLabButton.onClick = [this] { if (onTimeLab) onTimeLab(); };
    fxWindowButton.onClick = [this] { if (onFxInspector) onFxInspector(); };
    setSize(1000, 580);
    startTimerHz(24);
}

DecideratorPanel::~DecideratorPanel() { stopTimer(); setLookAndFeel(nullptr); }
void DecideratorPanel::timerCallback() { repaint(); }

void DecideratorPanel::showTextureControls(bool show)
{
    textureVisible = show;
    texturePage.setToggleState(show, juce::dontSendNotification);
    texturePage.setButtonText(show ? "RHYTHM / FX" : "SPACE / FX");
    for (size_t i = 0; i < knobs.size(); ++i) { knobs[i].setVisible(!show); knobLabels[i].setVisible(!show); }
    for (size_t i = 0; i < textureKnobs.size(); ++i) { textureKnobs[i].setVisible(show); textureLabels[i].setVisible(show); }
    repaint();
}

void DecideratorPanel::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.setColour(mint); g.setFont(juce::FontOptions(30.0f));
    g.drawText("DECIDERATOR", 28, 18, 360, 42, juce::Justification::centredLeft);
    g.setColour(muted); g.setFont(juce::FontOptions(12.0f));
    g.drawText("RHYTHM ENGINE   /   CREATIVE AUDIO LAB   /   0.3.0", 30, 60, 580, 22, juce::Justification::centredLeft);
    g.setColour(ink); g.setFont(juce::FontOptions(16.0f));
    const auto clockText = juce::String(processor.currentBpm.load(), 1) + " BPM  /  " + (processor.hostClock.load() ? "HOST SYNC" : "FREE CLOCK");
    g.drawText(clockText, 620, 26, 350, 30, juce::Justification::centredRight);
    for (const auto y : { 145, 265, 405 })
    {
        g.setColour(panel); g.fillRoundedRectangle(20.0f, static_cast<float>(y), 960.0f, y == 405 ? 135.0f : 112.0f, 10.0f);
    }
    g.setFont(juce::FontOptions(15.0f)); g.setColour(mint);
    g.drawText("01 / PULSE", 36, 158, 150, 24, juce::Justification::centredLeft);
    g.setColour(blue); g.drawText("02 / TONE", 36, 278, 150, 24, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(11.0f)); g.setColour(muted);
    g.drawText("VOLUMEN POR PASO", 36, 184, 150, 30, juce::Justification::centredLeft);
    g.drawText("APERTURA DEL FILTRO", 36, 304, 150, 30, juce::Justification::centredLeft);
    g.drawText("TIME LAB: curva de tiempo y onda / FX WINDOW: controles ampliados", 360, 382, 610, 20, juce::Justification::centredLeft);
    g.drawText("STEP", 530, 74, 40, 20, juce::Justification::centredLeft);
    g.drawText("ECHO TIME", 665, 74, 70, 20, juce::Justification::centredLeft);
    g.drawText("BPM", 850, 74, 32, 20, juce::Justification::centredLeft);
    const auto active = processor.currentStep.load();
    for (int i = 0; i < 16; ++i)
    {
        const auto x = 194 + i * 48;
        g.setColour(i == active ? mint : muted);
        g.drawText(juce::String(i + 1).paddedLeft('0', 2), x, 236, 40, 18, juce::Justification::centred);
        if (i == active)
        {
            g.setColour(mint.withAlpha(0.16f)); g.fillRoundedRectangle(static_cast<float>(x), 163, 40, 70, 4);
            g.setColour(blue.withAlpha(0.16f)); g.fillRoundedRectangle(static_cast<float>(x), 283, 40, 70, 4);
        }
    }
    g.setColour(muted);
    g.drawText("PULSE > TONE > HEAT > REPEAT / LO-FI > TIME WARP > ECHO > SPACE > MIX", 30, 548, 760, 22, juce::Justification::centredLeft);
    g.setColour(mint); g.drawText("16 STEPS / STEREO", 800, 548, 170, 22, juce::Justification::centredRight);
}

void DecideratorPanel::resized()
{
    pulseScene.setBounds(28, 104, 140, 28); shardsScene.setBounds(176, 104, 140, 28);
    driftScene.setBounds(324, 104, 140, 28); neutralScene.setBounds(472, 104, 110, 28);
    generate.setBounds(710, 104, 124, 28); undo.setBounds(844, 104, 128, 28);
    texturePage.setBounds(590, 104, 112, 28);
    timeLabButton.setBounds(30, 379, 150, 22); fxWindowButton.setBounds(190, 379, 150, 22);
    stepRate.setBounds(570, 70, 85, 28); echoRate.setBounds(735, 70, 100, 28);
    tempo.setBounds(884, 70, 88, 28);
    for (int i = 0; i < 16; ++i)
    {
        pulse[static_cast<size_t>(i)].setBounds(194 + i * 48, 163, 40, 70);
        tone[static_cast<size_t>(i)].setBounds(194 + i * 48, 283, 40, 70);
    }
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const auto x = 30 + static_cast<int>(i) * 104;
        knobLabels[i].setBounds(x, 410, 98, 22);
        knobs[i].setBounds(x, 433, 98, 98);
    }
    for (size_t i = 0; i < textureKnobs.size(); ++i)
    {
        const auto x = 30 + static_cast<int>(i) * 94;
        textureLabels[i].setBounds(x, 410, 90, 22);
        textureKnobs[i].setBounds(x, 433, 90, 98);
    }
}

DecideratorEditor::DecideratorEditor(DecideratorProcessor& p)
    : AudioProcessorEditor(p), processor(p), panel(p)
{
    const auto savedWidth = juce::jlimit(750, 1500, p.editorWidth.load());
    const auto savedHeight = juce::jlimit(467, 902, p.editorHeight.load());
    setLookAndFeel(&look);
    addAndMakeVisible(panel);
    panel.onTimeLab = [this] { openTimeLab(); };
    panel.onFxInspector = [this] { openFxInspector(); };
    zoom.addItem("75% / compacto", 75);
    zoom.addItem("80%", 80);
    zoom.addItem("90%", 90);
    zoom.addItem("100%", 100);
    zoom.addItem("125%", 125);
    zoom.addItem("150% / grande", 150);
    zoom.setTextWhenNothingSelected("Personalizado");
    zoom.setTooltip("Escala toda la interfaz y sus controles. Tambien puedes arrastrar la esquina inferior derecha.");
    zoom.onChange = [this] { if (zoom.getSelectedId() > 0) setZoomPercent(zoom.getSelectedId()); };
    addAndMakeVisible(zoom);
    addAndMakeVisible(presetList);
    for (auto* button : { &savePresetButton, &loadPresetButton, &previousPreset, &nextPreset, &legalButton }) addAndMakeVisible(*button);
    presetList.setTextWhenNothingSelected("Presets / selecciona...");
    presetList.onChange = [this] { selectPreset(); };
    refreshPresets();
    savePresetButton.onClick = [this] { choosePreset(true); };
    loadPresetButton.onClick = [this] { choosePreset(false); };
    previousPreset.onClick = [this] { presetList.setSelectedItemIndex((presetList.getSelectedItemIndex() + presetList.getNumItems() - 1) % presetList.getNumItems(), juce::sendNotificationSync); };
    nextPreset.onClick = [this] { presetList.setSelectedItemIndex((presetList.getSelectedItemIndex() + 1) % presetList.getNumItems(), juce::sendNotificationSync); };
    legalButton.onClick = [this] { showLegal(); };
    setResizable(true, true);
    setResizeLimits(750, 467, 1500, 902);
    setSize(savedWidth, savedHeight);
}

DecideratorEditor::~DecideratorEditor()
{
    chooser.reset(); timeWindow.reset(); fxWindow.reset(); legalWindow.reset();
    setLookAndFeel(nullptr);
}

void DecideratorEditor::paint(juce::Graphics& g)
{
    g.fillAll(background);
    g.setColour(muted);
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("ZOOM", getWidth() - 224, 3, 45, 26, juce::Justification::centredLeft);
    g.setColour(track);
    g.drawHorizontalLine(31, 0, static_cast<float>(getWidth()));
}

void DecideratorEditor::setZoomPercent(int percent)
{
    const auto scale = juce::jlimit(0.75, 1.5, percent / 100.0);
    setSize(juce::roundToInt(1000.0 * scale), 32 + juce::roundToInt(580.0 * scale));
}

void DecideratorEditor::resized()
{
    zoom.setBounds(getWidth() - 178, 3, 160, 25);
    presetList.setBounds(12, 3, getWidth() - 572, 25);
    auto x = getWidth() - 552;
    savePresetButton.setBounds(x, 3, 68, 25); loadPresetButton.setBounds(x + 74, 3, 68, 25);
    previousPreset.setBounds(x + 148, 3, 28, 25); nextPreset.setBounds(x + 180, 3, 28, 25);
    legalButton.setBounds(x + 214, 3, 100, 25);
    const auto scale = juce::jmin(getWidth() / 1000.0f, (getHeight() - 32) / 580.0f);
    panel.setBounds(0, 0, 1000, 580);
    const auto offsetX = (getWidth() - 1000.0f * scale) * 0.5f;
    const auto y = 32.0f + (getHeight() - 32.0f - 580.0f * scale) * 0.5f;
    panel.setTransform(juce::AffineTransform::scale(scale).translated(offsetX, y));
    int preset = 0;
    for (const auto percent : { 75, 80, 90, 100, 125, 150 })
        if (getWidth() == percent * 10 && getHeight() == 32 + juce::roundToInt(580.0 * percent / 100.0)) preset = percent;
    zoom.setSelectedId(preset, juce::dontSendNotification);
    processor.editorWidth.store(getWidth());
    processor.editorHeight.store(getHeight());
}

void DecideratorEditor::refreshPresets()
{
    presetList.clear(juce::dontSendNotification);
    presetList.addItem("Factory / Pulse", 1); presetList.addItem("Factory / Shards", 2);
    presetList.addItem("Factory / Drift", 3); presetList.addItem("Factory / Neutral", 4);
    presetFiles = DecideratorProcessor::presetDirectory().findChildFiles(juce::File::findFiles, false, "*.decipreset");
    std::sort(presetFiles.begin(), presetFiles.end(), [](const juce::File& a, const juce::File& b)
    { return a.getFileName().compareNatural(b.getFileName()) < 0; });
    if (!presetFiles.isEmpty()) presetList.addSeparator();
    for (int i = 0; i < presetFiles.size(); ++i) presetList.addItem(presetFiles[i].getFileNameWithoutExtension(), 100 + i);
}

void DecideratorEditor::selectPreset()
{
    const auto id = presetList.getSelectedId();
    if (id >= 1 && id <= 4) processor.applyScene(id - 1);
    else if (id >= 100 && id - 100 < presetFiles.size())
    {
        const auto result = processor.loadPreset(presetFiles[id - 100]);
        if (result.failed()) juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Preset", result.getErrorMessage());
    }
}

void DecideratorEditor::choosePreset(bool save)
{
    const auto directory = DecideratorProcessor::presetDirectory();
    if (save)
    {
        const auto result = directory.createDirectory();
        if (result.failed())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Preset", result.getErrorMessage());
            return;
        }
    }
    chooser = std::make_unique<juce::FileChooser>(save ? "Guardar preset DECIDERATOR" : "Cargar preset DECIDERATOR",
        save ? directory.getChildFile("My Patch.decipreset") : directory, "*.decipreset");
    const auto chooserFlags = juce::FileBrowserComponent::canSelectFiles | (save
        ? juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting
        : juce::FileBrowserComponent::openMode);
    chooser->launchAsync(chooserFlags, [safe = juce::Component::SafePointer<DecideratorEditor>(this), save](const juce::FileChooser& selected)
    {
        if (safe == nullptr) return;
        const auto file = selected.getResult();
        if (file == juce::File{}) return;
        const auto result = save ? safe->processor.savePreset(file) : safe->processor.loadPreset(file);
        if (result.failed())
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Preset", result.getErrorMessage());
        else
        {
            safe->refreshPresets();
            safe->presetList.setText(file.getFileNameWithoutExtension(), juce::dontSendNotification);
        }
    });
}

namespace
{
class LegalViewer final : public juce::DocumentWindow
{
public:
    explicit LegalViewer(const juce::String& text)
        : DocumentWindow("DECIDERATOR / Creditos y licencias", background, DocumentWindow::closeButton)
    {
        auto* content = new juce::TextEditor();
        content->setMultiLine(true); content->setReadOnly(true); content->setScrollbarsShown(true);
        content->setColour(juce::TextEditor::backgroundColourId, background);
        content->setColour(juce::TextEditor::textColourId, ink);
        content->setText(text, false); content->setSize(680, 470);
        setUsingNativeTitleBar(true); setContentOwned(content, true); setResizable(true, false);
        centreWithSize(680, 500); setVisible(true);
    }
    void closeButtonPressed() override { setVisible(false); }
};
}

void DecideratorEditor::showLegal()
{
    if (legalWindow == nullptr)
    {
        juce::String text = "DECIDERATOR Rhythm Engine 0.3.0\nAGPL-3.0-only. Sin garantia.\n"
            "Puedes modificar y redistribuir bajo esta licencia.\n"
            "SpaceFDN adapta Aquaton Reverb de aquanode (Aqua Node).\n"
            "Fuentes y scripts de compilacion incluidos en el paquete de distribucion.\n\n";
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            int size = 0;
            if (const auto* data = BinaryData::getNamedResource(BinaryData::namedResourceList[i], size))
                text += "\n----- " + juce::String(BinaryData::namedResourceList[i]) + " -----\n" + juce::String::fromUTF8(data, size);
        }
        legalWindow = std::make_unique<LegalViewer>(text);
    }
    legalWindow->setVisible(true); legalWindow->toFront(true);
}

void DecideratorEditor::openTimeLab()
{
    if (!timeWindow) timeWindow = std::make_unique<DetailWindow>("DECIDERATOR / TIME LAB", new TimeLab(processor), 1000, 570);
    timeWindow->setVisible(true); timeWindow->toFront(true);
}
void DecideratorEditor::openFxInspector()
{
    if (!fxWindow) fxWindow = std::make_unique<DetailWindow>("DECIDERATOR / FX INSPECTOR", new FxInspector(processor), 760, 420);
    fxWindow->setVisible(true); fxWindow->toFront(true);
}
