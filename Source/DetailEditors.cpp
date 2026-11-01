// SPDX-License-Identifier: AGPL-3.0-only
#include "DetailEditors.h"

namespace
{
const juce::Colour bg(0xff0d121b), card(0xff17212e), grid(0xff2c3a48), ink(0xffedf3f5),
    dim(0xff8b9baa), orange(0xffffb56b), blue(0xff73ccf6);
}

TimeGraph::TimeGraph(DecideratorProcessor& p) : processor(p)
{
    for (int i = 0; i < 33; ++i) points[static_cast<size_t>(i)] = p.parameters.getParameter("timePoint" + juce::String(i));
    setMouseCursor(juce::MouseCursor::CrosshairCursor);
}
TimeGraph::~TimeGraph() { endEdit(); }
juce::Rectangle<float> TimeGraph::plot() const
{
    return getLocalBounds().toFloat().withTrimmedLeft(60).withTrimmedRight(18).withTrimmedTop(16).withTrimmedBottom(36);
}
void TimeGraph::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    const auto area = plot();
    g.setColour(card); g.fillRoundedRectangle(area.expanded(1), 5);
    g.setColour(grid);
    for (int i = 0; i <= 16; ++i)
        g.drawVerticalLine(juce::roundToInt(area.getX() + area.getWidth() * i / 16.0f), area.getY(), area.getBottom());
    for (int i = 0; i <= 8; ++i)
        g.drawHorizontalLine(juce::roundToInt(area.getY() + area.getHeight() * i / 8.0f), area.getX(), area.getRight());
    g.setColour(dim); g.setFont(juce::FontOptions(11));
    g.drawText("LIVE", 0, juce::roundToInt(area.getY()) - 6, 54, 18, juce::Justification::centredRight);
    g.drawText("-1 ciclo", 0, juce::roundToInt(area.getCentreY()) - 8, 54, 18, juce::Justification::centredRight);
    g.drawText("-2 ciclos", 0, juce::roundToInt(area.getBottom()) - 10, 54, 18, juce::Justification::centredRight);
    for (int i = 0; i <= 4; ++i)
        g.drawText(juce::String(i * 25) + "%", juce::roundToInt(area.getX() + area.getWidth() * i / 4.0f) - 20,
            juce::roundToInt(area.getBottom()) + 8, 40, 18, juce::Justification::centred);
    const auto& engine = processor.getTimeWarp();
    const auto head = engine.waveHead.load();
    juce::Path wave;
    for (int i = 0; i < 256; ++i)
    {
        const auto peak = engine.waveform[static_cast<size_t>((head + i) % 256)].load();
        const auto x = area.getX() + area.getWidth() * i / 255.0f;
        wave.startNewSubPath(x, area.getCentreY() - peak * area.getHeight() * 0.35f);
        wave.lineTo(x, area.getCentreY() + peak * area.getHeight() * 0.35f);
    }
    g.setColour(blue.withAlpha(0.25f)); g.strokePath(wave, juce::PathStrokeType(1));
    const bool steps = processor.parameters.getRawParameterValue("timeShape")->load() > 0.5f;
    juce::Path curve;
    float previousY = area.getY();
    for (int i = 0; i < 33; ++i)
    {
        const auto value = points[static_cast<size_t>(i)]->convertFrom0to1(points[static_cast<size_t>(i)]->getValue());
        const auto x = area.getX() + area.getWidth() * i / 32.0f;
        const auto y = area.getY() + value * area.getHeight();
        if (i == 0) curve.startNewSubPath(x, y);
        else { if (steps) curve.lineTo(x, previousY); curve.lineTo(x, y); }
        previousY = y;
    }
    g.setColour(orange); g.strokePath(curve, juce::PathStrokeType(2));
    for (int i = 0; i < 33; ++i)
    {
        const auto y = area.getY() + points[static_cast<size_t>(i)]->getValue() * area.getHeight();
        const auto x = area.getX() + area.getWidth() * i / 32.0f;
        const auto radius = i == selected ? 5.0f : 3.0f;
        g.fillEllipse(x - radius, y - radius, radius * 2, radius * 2);
    }
    g.setColour(ink.withAlpha(0.65f));
    const auto cursorX = area.getX() + area.getWidth() * engine.phaseDisplay.load();
    g.drawLine(cursorX, area.getY(), cursorX, area.getBottom(), 1);
}

void TimeGraph::setPoint(int index, float value)
{
    const auto i = static_cast<size_t>(juce::jlimit(0, 32, index));
    if (!gestures[i]) { points[i]->beginChangeGesture(); gestures[i] = true; }
    points[i]->setValueNotifyingHost(juce::jlimit(0.0f, 1.0f, value));
}
void TimeGraph::endEdit()
{
    for (size_t i = 0; i < gestures.size(); ++i)
        if (gestures[i]) { points[i]->endChangeGesture(); gestures[i] = false; }
    lastIndex = -1;
}
void TimeGraph::edit(const juce::MouseEvent& event, bool interpolate)
{
    const auto area = plot();
    const auto index = juce::jlimit(0, 32, juce::roundToInt((event.position.x - area.getX()) / area.getWidth() * 32));
    auto value = juce::jlimit(0.0f, 1.0f, (event.position.y - area.getY()) / area.getHeight());
    if (event.mods.isRightButtonDown()) value = 0;
    else if (processor.parameters.getRawParameterValue("timeSnap")->load() > 0.5f && !event.mods.isShiftDown())
        value = std::round(value * 16.0f) / 16.0f;
    if (interpolate && lastIndex >= 0 && lastIndex != index)
    {
        const auto lo = juce::jmin(lastIndex, index), hi = juce::jmax(lastIndex, index);
        for (int i = lo; i <= hi; ++i)
        {
            const auto fraction = static_cast<float>(i - lastIndex) / static_cast<float>(index - lastIndex);
            auto intermediate = lastValue + fraction * (value - lastValue);
            if (processor.parameters.getRawParameterValue("timeSnap")->load() > 0.5f && !event.mods.isShiftDown())
                intermediate = std::round(intermediate * 16.0f) / 16.0f;
            setPoint(i, intermediate);
        }
    }
    else setPoint(index, value);
    selected = index; lastIndex = index; lastValue = value;
    if (onSelection) onSelection(index);
    repaint();
}
void TimeGraph::mouseDown(const juce::MouseEvent& event) { endEdit(); edit(event, false); }
void TimeGraph::mouseDrag(const juce::MouseEvent& event) { edit(event, true); }
void TimeGraph::mouseUp(const juce::MouseEvent&) { endEdit(); }
void TimeGraph::mouseDoubleClick(const juce::MouseEvent& event)
{
    endEdit();
    const auto index = juce::jlimit(0, 32, juce::roundToInt((event.position.x - plot().getX()) / plot().getWidth() * 32));
    setPoint(index, 0); endEdit(); repaint();
}

TimeLab::TimeLab(DecideratorProcessor& p) : processor(p), graph(p)
{
    const std::array<const char*, 5> names { "DIRECTO", "CHOP 16", "REVERSE", "TAPE STOP", "SCRATCH" };
    for (size_t i = 0; i < shapes.size(); ++i)
    {
        shapes[i].setButtonText(names[i]); shapes[i].onClick = [&p, i] { p.applyTimeShape(static_cast<int>(i)); };
        addAndMakeVisible(shapes[i]);
    }
    mix.setSliderStyle(juce::Slider::LinearHorizontal);
    mix.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 26);
    addAndMakeVisible(mix);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "timeMix", mix);
    mix.textFromValueFunction = [](double value) { return juce::String(value * 100.0, 0) + "%"; };
    mix.valueFromTextFunction = [](const juce::String& text) { return text.getDoubleValue() / 100.0; }; mix.updateText();
    cycle.addItemList({ "1 beat", "2 beats", "4 beats" }, 1);
    interpolation.addItemList({ "LINEA", "PASOS" }, 1);
    addAndMakeVisible(cycle); addAndMakeVisible(interpolation); addAndMakeVisible(snap);
    cycleAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "timeCycle", cycle);
    interpolationAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters, "timeShape", interpolation);
    snapAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters, "timeSnap", snap);
    pointValue.setSliderStyle(juce::Slider::IncDecButtons); pointValue.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 95, 26);
    pointValue.setTooltip("Retardo del punto, de 0 a 2 ciclos. Shift al dibujar desactiva snap.");
    addAndMakeVisible(pointValue); addAndMakeVisible(pointLabel); addAndMakeVisible(graph);
    graph.onSelection = [this, &p](int index)
    {
        pointLabel.setText("PUNTO " + juce::String(index + 1), juce::dontSendNotification);
        pointAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "timePoint" + juce::String(index), pointValue);
        pointValue.textFromValueFunction = [](double value) { return juce::String(value * 2, 4); };
        pointValue.valueFromTextFunction = [](const juce::String& text) { return text.getDoubleValue() * 0.5; }; pointValue.updateText();
    };
    graph.onSelection(0);
    setSize(1000, 540); startTimerHz(24);
}
void TimeLab::paint(juce::Graphics& g)
{
    g.fillAll(bg); g.setColour(ink); g.setFont(juce::FontOptions(12));
    g.drawText("TIME MIX", getWidth() - 310, 14, 75, 28, juce::Justification::centredLeft);
    g.drawText("CICLO", 18, 60, 50, 26, juce::Justification::centredLeft);
    g.drawText("CURVA", 195, 60, 55, 26, juce::Justification::centredLeft);
    const auto ready = juce::roundToInt(processor.getTimeWarp().readiness.load() * 100);
    g.setColour(dim); g.drawText("BUFFER " + juce::String(ready) + "%", getWidth() - 140, 86, 120, 16, juce::Justification::centredRight);
    g.drawText("Arrastra puntos / dibuja. Shift: fino. Doble clic o boton derecho: punto a directo.", 20, getHeight() - 34, getWidth() - 40, 22, juce::Justification::centredLeft);
    g.setColour(orange); g.drawText("Eje X: ciclo musical. Eje Y: retardo hacia el pasado. La onda azul es la entrada reciente.", 20, getHeight() - 56, getWidth() - 40, 20, juce::Justification::centredLeft);
}
void TimeLab::resized()
{
    const auto buttonWidth = juce::jmin(112, (getWidth() - 360) / 5 - 4);
    for (size_t i = 0; i < shapes.size(); ++i) shapes[i].setBounds(18 + static_cast<int>(i) * (buttonWidth + 4), 14, buttonWidth, 28);
    mix.setBounds(getWidth() - 236, 14, 216, 28);
    cycle.setBounds(72, 60, 110, 26); interpolation.setBounds(250, 60, 108, 26); snap.setBounds(372, 60, 80, 26);
    pointLabel.setBounds(465, 60, 90, 26); pointValue.setBounds(560, 60, 150, 26);
    graph.setBounds(10, 102, getWidth() - 20, getHeight() - 164);
}

FxInspector::FxInspector(DecideratorProcessor& p)
{
    const std::array<const char*, 10> ids { "spaceMix", "spaceSize", "spaceDecay", "spaceTone", "spaceMotion", "glitchMix", "glitchRate", "lofiMix", "lofiBits", "lofiRate" };
    const std::array<const char*, 10> names { "SPACE", "SIZE", "DECAY", "AIR", "SPACE MOD", "REPEAT MIX", "REPEAT SLICE", "LO-FI", "BITS", "HOLD" };
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        knobs[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knobs[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 28);
        knobs[i].setColour(juce::Slider::rotarySliderFillColourId, blue);
        labels[i].setText(names[i], juce::dontSendNotification); labels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(knobs[i]); addAndMakeVisible(labels[i]);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, ids[i], knobs[i]);
    }
    setSize(760, 390);
}
void FxInspector::paint(juce::Graphics& g) { g.fillAll(bg); }
void FxInspector::resized()
{
    const auto width = (getWidth() - 30) / 5, height = (getHeight() - 20) / 2;
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const auto x = 15 + static_cast<int>(i % 5) * width, y = 10 + static_cast<int>(i / 5) * height;
        labels[i].setBounds(x, y, width - 10, 24); knobs[i].setBounds(x, y + 24, width - 10, height - 32);
    }
}

DetailWindow::DetailWindow(const juce::String& title, juce::Component* content, int width, int height)
    : DocumentWindow(title, bg, DocumentWindow::allButtons)
{
    appearance.setColour(juce::TextButton::buttonColourId, card);
    appearance.setColour(juce::TextButton::textColourOffId, ink);
    appearance.setColour(juce::ComboBox::backgroundColourId, card);
    appearance.setColour(juce::ComboBox::textColourId, ink);
    appearance.setColour(juce::Label::textColourId, ink);
    appearance.setColour(juce::Slider::textBoxTextColourId, ink);
    appearance.setColour(juce::Slider::textBoxBackgroundColourId, card);
    setLookAndFeel(&appearance); setUsingNativeTitleBar(true); setContentOwned(content, true);
    setResizable(true, false); setResizeLimits(760, 390, 1800, 1200);
    centreWithSize(width, height); setVisible(true);
}
DetailWindow::~DetailWindow() { clearContentComponent(); setLookAndFeel(nullptr); }
