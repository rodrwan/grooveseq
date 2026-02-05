#include "PluginEditor.h"

GrooveSeqAudioProcessorEditor::GrooveSeqAudioProcessorEditor(GrooveSeqAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
    , sequencerGrid(*this)
{
    setSize(820, 620);

    generateButton.onClick = [this]
    {
        processor.generatePattern();
        sequencerGrid.repaint();
    };

    browseButton.onClick = [this]
    {
        fileBrowserVisible = !fileBrowserVisible;
        fileBrowser.setVisible(fileBrowserVisible);
        resized();
    };

    auto setupSlider = [](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    };

    setupSlider(swingSlider);
    setupSlider(humanizeSlider);
    setupSlider(fillsSlider);
    setupSlider(densitySlider);
    setupSlider(velocitySlider);
    setupSlider(attackSlider);
    setupSlider(decaySlider);
    setupSlider(sustainSlider);
    setupSlider(releaseSlider);

    attackSlider.setRange(0.0, 100.0, 0.1);
    decaySlider.setRange(0.0, 800.0, 0.1);
    sustainSlider.setRange(0.0, 1.0, 0.001);
    releaseSlider.setRange(0.0, 1500.0, 0.1);

    swingLabel.setJustificationType(juce::Justification::centred);
    humanizeLabel.setJustificationType(juce::Justification::centred);
    fillsLabel.setJustificationType(juce::Justification::centred);
    densityLabel.setJustificationType(juce::Justification::centred);
    velocityLabel.setJustificationType(juce::Justification::centred);
    attackLabel.setJustificationType(juce::Justification::centred);
    decayLabel.setJustificationType(juce::Justification::centred);
    sustainLabel.setJustificationType(juce::Justification::centred);
    releaseLabel.setJustificationType(juce::Justification::centred);
    helpLabel.setJustificationType(juce::Justification::centredLeft);
    helpLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));
    selectedLabel.setJustificationType(juce::Justification::centredLeft);
    selectedLabel.setColour(juce::Label::textColourId, juce::Colour(0xff9aa0a6));

    swingLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    humanizeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    fillsLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    densityLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    velocityLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    attackLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    decayLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    sustainLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));
    releaseLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e0e0));

    swingAttachment = std::make_unique<SliderAttachment>(processor.getValueTreeState(), "swing", swingSlider);
    humanizeAttachment = std::make_unique<SliderAttachment>(processor.getValueTreeState(), "humanize", humanizeSlider);
    fillsAttachment = std::make_unique<SliderAttachment>(processor.getValueTreeState(), "fills", fillsSlider);
    densityAttachment = std::make_unique<SliderAttachment>(processor.getValueTreeState(), "density", densitySlider);
    velocityAttachment = std::make_unique<SliderAttachment>(processor.getValueTreeState(), "velocity", velocitySlider);
    attackSlider.onValueChange = [this]
    {
        auto params = processor.getPadAdsr(selectedPad);
        params.attack = static_cast<float>(attackSlider.getValue() / 1000.0);
        processor.setPadAdsr(selectedPad, params);
    };
    decaySlider.onValueChange = [this]
    {
        auto params = processor.getPadAdsr(selectedPad);
        params.decay = static_cast<float>(decaySlider.getValue() / 1000.0);
        processor.setPadAdsr(selectedPad, params);
    };
    sustainSlider.onValueChange = [this]
    {
        auto params = processor.getPadAdsr(selectedPad);
        params.sustain = static_cast<float>(sustainSlider.getValue());
        processor.setPadAdsr(selectedPad, params);
    };
    releaseSlider.onValueChange = [this]
    {
        auto params = processor.getPadAdsr(selectedPad);
        params.release = static_cast<float>(releaseSlider.getValue() / 1000.0);
        processor.setPadAdsr(selectedPad, params);
    };

    addAndMakeVisible(generateButton);
    addAndMakeVisible(browseButton);
    addAndMakeVisible(helpLabel);
    addAndMakeVisible(selectedLabel);
    addAndMakeVisible(sequencerGrid);

    addAndMakeVisible(swingSlider);
    addAndMakeVisible(humanizeSlider);
    addAndMakeVisible(fillsSlider);
    addAndMakeVisible(densitySlider);
    addAndMakeVisible(velocitySlider);
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(sustainSlider);
    addAndMakeVisible(releaseSlider);

    addAndMakeVisible(swingLabel);
    addAndMakeVisible(humanizeLabel);
    addAndMakeVisible(fillsLabel);
    addAndMakeVisible(densityLabel);
    addAndMakeVisible(velocityLabel);
    addAndMakeVisible(attackLabel);
    addAndMakeVisible(decayLabel);
    addAndMakeVisible(sustainLabel);
    addAndMakeVisible(releaseLabel);

    pads.reserve(Sequencer::kPads);
    for (int i = 0; i < Sequencer::kPads; ++i)
    {
        auto pad = std::make_unique<SamplePad>(i);
        pad->setOnLoad([this](int padIndex) { handleLoadSample(padIndex); });
        pad->setOnFileDropped([this](int padIndex, const juce::File& file)
        {
            if (processor.loadSample(padIndex, file))
                updatePadLabels();
        });
        pad->setOnSelect([this](int padIndex)
        {
            selectPad(padIndex);
        });
        pad->setOnPlay([this](int padIndex)
        {
            processor.triggerPadPreview(padIndex);
        });
        pads.push_back(std::move(pad));
        addAndMakeVisible(pads.back().get());
    }

    updatePadLabels();
    selectPad(0);

    fileBrowser.addListener(this);
    fileBrowser.setVisible(false);
    addAndMakeVisible(fileBrowser);

    resized();
}

GrooveSeqAudioProcessorEditor::~GrooveSeqAudioProcessorEditor() = default;

void GrooveSeqAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1b1b1f));
    g.setColour(juce::Colour(0xffd7d7d7));
    g.setFont(juce::Font(16.0f, juce::Font::bold));
    g.drawText("GrooveSeq", getLocalBounds().removeFromTop(30), juce::Justification::centred);
}

void GrooveSeqAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12);
    auto header = area.removeFromTop(210);

    auto headerTop = header.removeFromTop(34);
    generateButton.setBounds(headerTop.removeFromLeft(140).reduced(6, 2));
    browseButton.setBounds(headerTop.removeFromLeft(140).reduced(6, 2));
    selectedLabel.setBounds(headerTop.removeFromLeft(180).reduced(6, 2));
    helpLabel.setBounds(headerTop.reduced(6, 2));

    if (fileBrowserVisible)
    {
        auto browserArea = area.removeFromBottom(180);
        fileBrowser.setBounds(browserArea.reduced(6));
    }

    auto gridArea = area.removeFromTop(160);
    sequencerGrid.setBounds(gridArea.reduced(4, 0));

    auto sliderArea = header.reduced(0, 6);
    auto topRow = sliderArea.removeFromTop(sliderArea.getHeight() / 2);
    auto bottomRow = sliderArea;
    const int topWidth = topRow.getWidth() / 5;
    const int bottomWidth = bottomRow.getWidth() / 4;

    auto placeSlider = [](juce::Rectangle<int> area, juce::Slider& slider, juce::Label& label)
    {
        auto slot = area.reduced(6);
        label.setBounds(slot.removeFromTop(18));
        slider.setBounds(slot);
    };

    placeSlider(topRow.removeFromLeft(topWidth), swingSlider, swingLabel);
    placeSlider(topRow.removeFromLeft(topWidth), humanizeSlider, humanizeLabel);
    placeSlider(topRow.removeFromLeft(topWidth), fillsSlider, fillsLabel);
    placeSlider(topRow.removeFromLeft(topWidth), densitySlider, densityLabel);
    placeSlider(topRow.removeFromLeft(topWidth), velocitySlider, velocityLabel);

    placeSlider(bottomRow.removeFromLeft(bottomWidth), attackSlider, attackLabel);
    placeSlider(bottomRow.removeFromLeft(bottomWidth), decaySlider, decayLabel);
    placeSlider(bottomRow.removeFromLeft(bottomWidth), sustainSlider, sustainLabel);
    placeSlider(bottomRow.removeFromLeft(bottomWidth), releaseSlider, releaseLabel);

    juce::Grid grid;
    grid.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                             juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                             juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                             juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    grid.templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                          juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                          juce::Grid::TrackInfo(juce::Grid::Fr(1)),
                          juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    grid.columnGap = juce::Grid::Px(8);
    grid.rowGap = juce::Grid::Px(8);

    for (auto& pad : pads)
        grid.items.add(juce::GridItem(*pad));

    grid.performLayout(area);
}

void GrooveSeqAudioProcessorEditor::handleLoadSample(int padIndex)
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select a sample",
        juce::File{},
        "*.wav;*.aiff;*.aif;*.flac",
        false,
        false,
        this);

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

    fileChooser->launchAsync(flags, [this, padIndex](const juce::FileChooser& chooser)
    {
        const auto file = chooser.getResult();
        if (file.existsAsFile())
        {
            if (processor.loadSample(padIndex, file))
                updatePadLabels();
        }
    });
}

void GrooveSeqAudioProcessorEditor::updatePadLabels()
{
    for (int i = 0; i < static_cast<int>(pads.size()); ++i)
        pads[static_cast<size_t>(i)]->setPadName(processor.getPadName(i));
}

void GrooveSeqAudioProcessorEditor::selectPad(int padIndex)
{
    if (padIndex < 0 || padIndex >= static_cast<int>(pads.size()))
        return;

    selectedPad = padIndex;
    for (int i = 0; i < static_cast<int>(pads.size()); ++i)
        pads[static_cast<size_t>(i)]->setSelected(i == padIndex);

    selectedLabel.setText("Selected Pad: " + juce::String(selectedPad + 1), juce::dontSendNotification);

    const auto params = processor.getPadAdsr(selectedPad);
    attackSlider.setValue(params.attack * 1000.0, juce::dontSendNotification);
    decaySlider.setValue(params.decay * 1000.0, juce::dontSendNotification);
    sustainSlider.setValue(params.sustain, juce::dontSendNotification);
    releaseSlider.setValue(params.release * 1000.0, juce::dontSendNotification);
}

void GrooveSeqAudioProcessorEditor::tryLoadFileToSelectedPad(const juce::File& file)
{
    if (!file.existsAsFile())
        return;

    if (processor.loadSample(selectedPad, file))
        updatePadLabels();
}

void GrooveSeqAudioProcessorEditor::selectionChanged()
{
    auto file = fileBrowser.getSelectedFile(0);
    if (file.existsAsFile())
        tryLoadFileToSelectedPad(file);
}

void GrooveSeqAudioProcessorEditor::fileClicked(const juce::File& file, const juce::MouseEvent&)
{
    if (file.existsAsFile())
        tryLoadFileToSelectedPad(file);
}

void GrooveSeqAudioProcessorEditor::fileDoubleClicked(const juce::File& file)
{
    if (file.existsAsFile())
        tryLoadFileToSelectedPad(file);
}

void GrooveSeqAudioProcessorEditor::browserRootChanged(const juce::File&)
{
}

bool GrooveSeqAudioProcessorEditor::getStepState(int pad, int step) const
{
    return processor.getStepState(pad, step);
}

void GrooveSeqAudioProcessorEditor::setStepState(int pad, int step, bool enabled)
{
    processor.setStepState(pad, step, enabled);
}

int GrooveSeqAudioProcessorEditor::getCurrentStep() const
{
    return processor.getCurrentStep();
}

int GrooveSeqAudioProcessorEditor::getPadCount() const
{
    return Sequencer::kPads;
}

int GrooveSeqAudioProcessorEditor::getStepCount() const
{
    return Sequencer::kSteps;
}
