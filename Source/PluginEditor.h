#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <vector>

#include "PluginProcessor.h"
#include "SamplePad.h"
#include "SequencerGrid.h"

class GrooveSeqAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      public juce::FileBrowserListener,
                                      public SequencerGrid::DataProvider
{
public:
    explicit GrooveSeqAudioProcessorEditor(GrooveSeqAudioProcessor&);
    ~GrooveSeqAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void selectionChanged() override;
    void fileClicked(const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked(const juce::File& file) override;
    void browserRootChanged(const juce::File& newRoot) override;
    bool getStepState(int pad, int step) const override;
    void setStepState(int pad, int step, bool enabled) override;
    int getCurrentStep() const override;
    int getPadCount() const override;
    int getStepCount() const override;

private:
    void handleLoadSample(int padIndex);
    void updatePadLabels();
    void selectPad(int padIndex);
    void tryLoadFileToSelectedPad(const juce::File& file);

    GrooveSeqAudioProcessor& processor;

    juce::TextButton generateButton { "Generate" };
    juce::TextButton browseButton { "Browse" };
    juce::Label helpLabel { {}, "Click Load or drop a sample onto a pad" };
    juce::Label selectedLabel { {}, "Selected Pad: 1" };
    SequencerGrid sequencerGrid;

    juce::Slider swingSlider;
    juce::Slider humanizeSlider;
    juce::Slider fillsSlider;
    juce::Slider densitySlider;
    juce::Slider velocitySlider;
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;

    juce::Label swingLabel { {}, "Swing" };
    juce::Label humanizeLabel { {}, "Humanize" };
    juce::Label fillsLabel { {}, "Fills" };
    juce::Label densityLabel { {}, "Density" };
    juce::Label velocityLabel { {}, "Velocity" };
    juce::Label attackLabel { {}, "Attack" };
    juce::Label decayLabel { {}, "Decay" };
    juce::Label sustainLabel { {}, "Sustain" };
    juce::Label releaseLabel { {}, "Release" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    std::unique_ptr<SliderAttachment> swingAttachment;
    std::unique_ptr<SliderAttachment> humanizeAttachment;
    std::unique_ptr<SliderAttachment> fillsAttachment;
    std::unique_ptr<SliderAttachment> densityAttachment;
    std::unique_ptr<SliderAttachment> velocityAttachment;

    std::vector<std::unique_ptr<SamplePad>> pads;
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::WildcardFileFilter sampleFilter { "*.wav;*.wave;*.aiff;*.aif;*.flac", "*", "Samples" };
    juce::FileBrowserComponent fileBrowser { juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                             juce::File::getSpecialLocation(juce::File::userHomeDirectory),
                                             &sampleFilter,
                                             nullptr };
    bool fileBrowserVisible = false;
    int selectedPad = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSeqAudioProcessorEditor)
};
