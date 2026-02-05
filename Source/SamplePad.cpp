#include "SamplePad.h"

namespace
{
std::unique_ptr<juce::Drawable> createMagnifierIcon(juce::Colour colour)
{
    auto drawable = std::make_unique<juce::DrawablePath>();
    juce::Path icon;
    icon.addEllipse(3.0f, 3.0f, 18.0f, 18.0f);

    juce::Path handle;
    handle.addRectangle(17.0f, 17.0f, 8.0f, 3.0f);
    handle.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::pi / 4.0f, 18.0f, 18.0f));
    icon.addPath(handle);

    drawable->setPath(icon);
    drawable->setFill(juce::FillType(colour));
    return drawable;
}

std::unique_ptr<juce::Drawable> createPlayIcon(juce::Colour colour)
{
    auto drawable = std::make_unique<juce::DrawablePath>();
    juce::Path icon;
    icon.addTriangle({ 7.0f, 5.0f }, { 7.0f, 23.0f }, { 23.0f, 14.0f });
    drawable->setPath(icon);
    drawable->setFill(juce::FillType(colour));
    return drawable;
}

void configureIconButton(juce::DrawableButton& button,
                         const std::function<std::unique_ptr<juce::Drawable>(juce::Colour)>& factory)
{
    const auto base = juce::Colour(0xffd7d7d7);
    button.setImages(factory(base).release(),
                     factory(base.brighter(0.25f)).release(),
                     factory(base.brighter(0.5f)).release(),
                     nullptr,
                     nullptr,
                     nullptr,
                     nullptr,
                     nullptr);
    button.setColour(juce::DrawableButton::backgroundColourId, juce::Colour(0xff2c2c33));
    button.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colour(0xff35353c));
    button.setClickingTogglesState(false);
}
} // namespace

SamplePad::SamplePad(int index)
    : padIndex(index)
{
    nameLabel.setText("Pad " + juce::String(padIndex + 1), juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff1f1f1));

    configureIconButton(browseButton, createMagnifierIcon);
    configureIconButton(playButton, createPlayIcon);

    browseButton.setTooltip("Buscar sample (abre el explorador)");
    playButton.setTooltip("Reproducir sample cargado");
    playButton.setEnabled(false);
    playButton.setAlpha(0.4f);

    browseButton.onClick = [this]
    {
        if (onLoad)
            onLoad(padIndex);
    };

    playButton.onClick = [this]
    {
        if (onPlay)
            onPlay(padIndex);
    };

    addAndMakeVisible(nameLabel);
    addAndMakeVisible(browseButton);
    addAndMakeVisible(playButton);
}

void SamplePad::setPadName(const juce::String& name)
{
    nameLabel.setText(name.isEmpty() ? ("Pad " + juce::String(padIndex + 1)) : name,
                      juce::dontSendNotification);

    const bool hasSample = !name.isEmpty();
    playButton.setEnabled(hasSample);
    playButton.setAlpha(hasSample ? 1.0f : 0.4f);
}

void SamplePad::setOnLoad(std::function<void(int)> callback)
{
    onLoad = std::move(callback);
}

void SamplePad::setOnFileDropped(std::function<void(int, const juce::File&)> callback)
{
    onFileDropped = std::move(callback);
}

void SamplePad::setOnSelect(std::function<void(int)> callback)
{
    onSelect = std::move(callback);
}

void SamplePad::setOnPlay(std::function<void(int)> callback)
{
    onPlay = std::move(callback);
}

void SamplePad::setSelected(bool shouldSelect)
{
    selected = shouldSelect;
    repaint();
}

void SamplePad::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);
    auto bg = selected ? juce::Colour(0xff2f3c54) : juce::Colour(0xff232329);
    auto border = selected ? juce::Colour(0xff5aa9ff) : juce::Colour(0xff3f3f46);

    g.setColour(bg);
    g.fillRoundedRectangle(bounds, 8.0f);
    g.setColour(border);
    g.drawRoundedRectangle(bounds, 8.0f, 2.0f);
}

void SamplePad::resized()
{
    auto area = getLocalBounds().reduced(6);
    nameLabel.setBounds(area.removeFromTop(24));

    auto buttonRow = area.removeFromTop(34);
    auto left = buttonRow.removeFromLeft(buttonRow.getWidth() / 2).reduced(4, 2);
    auto right = buttonRow.reduced(4, 2);

    playButton.setBounds(left);
    browseButton.setBounds(right);
}

void SamplePad::mouseDown(const juce::MouseEvent&)
{
    if (onSelect)
        onSelect(padIndex);
}

bool SamplePad::isInterestedInFileDrag(const juce::StringArray& files)
{
    if (files.isEmpty())
        return false;

    juce::File file(files[0]);
    return file.hasFileExtension("wav;wave;aiff;aif;flac");
}

void SamplePad::filesDropped(const juce::StringArray& files, int, int)
{
    if (files.isEmpty() || !onFileDropped)
        return;

    juce::File file(files[0]);
    if (file.existsAsFile())
        onFileDropped(padIndex, file);
}
