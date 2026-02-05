#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class SamplePad : public juce::Component,
                  public juce::FileDragAndDropTarget
{
public:
    SamplePad(int index);

    void setPadName(const juce::String& name);
    void setOnLoad(std::function<void(int)> callback);
    void setOnFileDropped(std::function<void(int, const juce::File&)> callback);
    void setOnSelect(std::function<void(int)> callback);
    void setOnPlay(std::function<void(int)> callback);
    void setSelected(bool shouldSelect);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

private:
    int padIndex = 0;
    juce::Label nameLabel;
    juce::DrawableButton browseButton { "Browse", juce::DrawableButton::ImageOnButtonBackground };
    juce::DrawableButton playButton { "Play", juce::DrawableButton::ImageOnButtonBackground };
    std::function<void(int)> onLoad;
    std::function<void(int, const juce::File&)> onFileDropped;
    std::function<void(int)> onSelect;
    std::function<void(int)> onPlay;
    bool selected = false;
}; 
