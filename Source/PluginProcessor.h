#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include <atomic>

#include "Sequencer.h"

class GrooveSeqAudioProcessor : public juce::AudioProcessor
{
public:
    GrooveSeqAudioProcessor();
    ~GrooveSeqAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool loadSample(int padIndex, const juce::File& file);
    juce::String getPadName(int padIndex) const;

    void generatePattern();
    bool getStepState(int pad, int step) const;
    void setStepState(int pad, int step, bool enabled);
    int getCurrentStep() const;
    juce::ADSR::Parameters getPadAdsr(int padIndex) const;
    void setPadAdsr(int padIndex, const juce::ADSR::Parameters& params);
    void triggerPadPreview(int padIndex);

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void removePadSound(int padIndex);

    juce::AudioProcessorValueTreeState parameters;
    juce::AudioFormatManager formatManager;
    juce::Synthesiser synth;
    juce::SpinLock synthLock;
    juce::SpinLock previewLock;

    Sequencer sequencer;
    mutable juce::SpinLock sequenceLock;
    juce::Random random;
    std::atomic<int> currentStep { -1 };

    std::array<juce::SamplerSound*, Sequencer::kPads> padSounds{};
    std::array<juce::String, Sequencer::kPads> padNames{};
    std::array<juce::ADSR::Parameters, Sequencer::kPads> padAdsr;
    juce::MidiBuffer previewMidi;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GrooveSeqAudioProcessor)
};
