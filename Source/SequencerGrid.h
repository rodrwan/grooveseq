#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class SequencerGrid : public juce::Component,
                      private juce::Timer
{
public:
    struct DataProvider
    {
        virtual ~DataProvider() = default;
        virtual bool getStepState(int pad, int step) const = 0;
        virtual void setStepState(int pad, int step, bool enabled) = 0;
        virtual int getCurrentStep() const = 0;
        virtual int getPadCount() const = 0;
        virtual int getStepCount() const = 0;
    };

    explicit SequencerGrid(DataProvider& provider);

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void resized() override;

private:
    void timerCallback() override;

    DataProvider& data;
    int lastStep = -1;
};
