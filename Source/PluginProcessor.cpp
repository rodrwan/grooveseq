#include "PluginProcessor.h"
#include "PluginEditor.h"

GrooveSeqAudioProcessor::GrooveSeqAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), false)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    formatManager.registerBasicFormats();

    synth.setNoteStealingEnabled(true);
    for (int i = 0; i < 32; ++i)
        synth.addVoice(new juce::SamplerVoice());

    std::array<bool, Sequencer::kPads> defaultActivePads;
    defaultActivePads.fill(true);
    sequencer.generate(0.6f,
                       0.15f,
                       juce::Random::getSystemRandom().nextInt(),
                       defaultActivePads);

    const juce::ADSR::Parameters defaultAdsr { 0.002f, 0.12f, 0.7f, 0.12f };
    padAdsr.fill(defaultAdsr);
}

GrooveSeqAudioProcessor::~GrooveSeqAudioProcessor() = default;

const juce::String GrooveSeqAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GrooveSeqAudioProcessor::acceptsMidi() const
{
    return true;
}

bool GrooveSeqAudioProcessor::producesMidi() const
{
    return false;
}

bool GrooveSeqAudioProcessor::isMidiEffect() const
{
    return false;
}

double GrooveSeqAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GrooveSeqAudioProcessor::getNumPrograms() { return 1; }
int GrooveSeqAudioProcessor::getCurrentProgram() { return 0; }
void GrooveSeqAudioProcessor::setCurrentProgram(int) {}
const juce::String GrooveSeqAudioProcessor::getProgramName(int) { return {}; }
void GrooveSeqAudioProcessor::changeProgramName(int, const juce::String&) {}

void GrooveSeqAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    synth.setCurrentPlaybackSampleRate(sampleRate);
}

void GrooveSeqAudioProcessor::releaseResources() {}

bool GrooveSeqAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& outSet = layouts.getMainOutputChannelSet();
    if (outSet != juce::AudioChannelSet::mono()
        && outSet != juce::AudioChannelSet::stereo())
        return false;

    const auto& inSet = layouts.getMainInputChannelSet();
    if (!inSet.isDisabled())
        return false;

    return true;
}

void GrooveSeqAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    buffer.clear();

    juce::MidiBuffer midiOut;
    midiOut.addEvents(midiMessages, 0, numSamples, 0);

    {
        const juce::SpinLock::ScopedLockType lock(previewLock);
        midiOut.addEvents(previewMidi, 0, numSamples, 0);
        previewMidi.clear();
    }

    auto* playHead = getPlayHead();
    juce::AudioPlayHead::CurrentPositionInfo posInfo;

    bool canPlay = false;
    if (playHead != nullptr && playHead->getCurrentPosition(posInfo))
        canPlay = posInfo.isPlaying;

    if (canPlay)
    {
        const double bpm = (posInfo.bpm > 0.0) ? posInfo.bpm : 120.0;
        const double sampleRate = getSampleRate();
        const double samplesPerQuarter = sampleRate * 60.0 / bpm;
        const double stepPpq = 0.25; // 16th note
        const double barLengthPpq = (posInfo.timeSigDenominator > 0)
            ? (posInfo.timeSigNumerator * 4.0 / posInfo.timeSigDenominator)
            : 4.0;
        const double cycleLengthPpq = barLengthPpq * 2.0;

        const double startPpq = posInfo.ppqPosition;
        const double endPpq = startPpq + (static_cast<double>(numSamples) / samplesPerQuarter);

        const double cycleStartPpq = std::floor(startPpq / cycleLengthPpq) * cycleLengthPpq;

        const float swingPercent = parameters.getRawParameterValue("swing")->load();
        const float humanizeMs = parameters.getRawParameterValue("humanize")->load();
        const float velocityRand = parameters.getRawParameterValue("velocity")->load() / 100.0f;

        const double stepSamples = samplesPerQuarter / 4.0;
        const double swingSamples = stepSamples * (swingPercent / 100.0) * 0.5;
        const double humanizeSamples = (humanizeMs / 1000.0) * sampleRate;

        std::array<std::array<bool, Sequencer::kSteps>, Sequencer::kPads> patternSnapshot;
        {
            const juce::SpinLock::ScopedLockType lock(sequenceLock);
            patternSnapshot = sequencer.getPattern();
        }

        const int startStep = static_cast<int>(std::floor(startPpq / stepPpq));
        const int endStep = static_cast<int>(std::floor(endPpq / stepPpq));

        for (int step = startStep; step <= endStep; ++step)
        {
            const double stepTimePpq = step * stepPpq;
            const double offsetSamples = (stepTimePpq - startPpq) * samplesPerQuarter;
            if (offsetSamples < 0.0 || offsetSamples >= numSamples)
                continue;

            const int stepInCycle = static_cast<int>(std::floor((stepTimePpq - cycleStartPpq) / stepPpq))
                % Sequencer::kSteps;

            if (stepInCycle < 0)
                continue;

            if (step == startStep)
                currentStep.store(stepInCycle, std::memory_order_relaxed);

            double swungOffset = offsetSamples;
            if (stepInCycle % 2 == 1)
                swungOffset += swingSamples;

            for (int pad = 0; pad < Sequencer::kPads; ++pad)
            {
                if (!patternSnapshot[pad][stepInCycle])
                    continue;

                double eventOffset = swungOffset;

                if (humanizeSamples > 0.0)
                {
                    const double r = random.nextFloat() * 2.0 - 1.0;
                    eventOffset += r * humanizeSamples;
                }

                const int sampleOffset = juce::jlimit(0, numSamples - 1, static_cast<int>(std::round(eventOffset)));
                const int midiNote = 36 + pad;

                const float baseVelocity = 0.78f;
                const float randSpan = velocityRand * 0.5f;
                const float v = juce::jlimit(0.05f, 1.0f,
                                             baseVelocity + ((random.nextFloat() * 2.0f - 1.0f) * randSpan));
                const auto velocity = static_cast<juce::uint8>(juce::roundToInt(v * 127.0f));

                midiOut.addEvent(juce::MidiMessage::noteOn(1, midiNote, velocity), sampleOffset);
            }
        }
    }

    {
        const juce::SpinLock::ScopedLockType lock(synthLock);
        synth.renderNextBlock(buffer, midiOut, 0, numSamples);
    }
}

bool GrooveSeqAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GrooveSeqAudioProcessor::createEditor()
{
    return new GrooveSeqAudioProcessorEditor(*this);
}

void GrooveSeqAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GrooveSeqAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

bool GrooveSeqAudioProcessor::loadSample(int padIndex, const juce::File& file)
{
    if (padIndex < 0 || padIndex >= Sequencer::kPads)
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
        return false;


    juce::BigInteger range;
    const int midiNote = 36 + padIndex;
    range.setBit(midiNote);

    auto* sound = new juce::SamplerSound(
        file.getFileNameWithoutExtension(),
        *reader,
        range,
        midiNote,
        0.0,   // attack
        0.1,   // release
        10.0   // max length
    );
    sound->setEnvelopeParameters(getPadAdsr(padIndex));

    {
        const juce::SpinLock::ScopedLockType lock(synthLock);
        removePadSound(padIndex);
        synth.addSound(sound);
        padSounds[padIndex] = sound;
    }
    padNames[padIndex] = file.getFileNameWithoutExtension();

    return true;
}

void GrooveSeqAudioProcessor::removePadSound(int padIndex)
{
    auto* target = padSounds[padIndex];
    if (target == nullptr)
        return;

    for (int i = synth.getNumSounds(); --i >= 0;)
    {
        if (synth.getSound(i).get() == target)
        {
            synth.removeSound(i);
            break;
        }
    }

    padSounds[padIndex] = nullptr;
    padNames[padIndex].clear();
}

juce::String GrooveSeqAudioProcessor::getPadName(int padIndex) const
{
    if (padIndex < 0 || padIndex >= Sequencer::kPads)
        return {};

    return padNames[padIndex];
}

void GrooveSeqAudioProcessor::generatePattern()
{
    const float density = parameters.getRawParameterValue("density")->load() / 100.0f;
    const float fills = parameters.getRawParameterValue("fills")->load() / 100.0f;

    std::array<bool, Sequencer::kPads> activePads{};
    bool anyPadHasSample = false;
    {
        const juce::SpinLock::ScopedLockType lock(synthLock);
        for (int i = 0; i < Sequencer::kPads; ++i)
        {
            const bool active = padSounds[static_cast<size_t>(i)] != nullptr;
            activePads[static_cast<size_t>(i)] = active;
            anyPadHasSample |= active;
        }
    }

    if (!anyPadHasSample)
        activePads.fill(true);

    const juce::SpinLock::ScopedLockType lock(sequenceLock);
    sequencer.generate(density,
                       fills,
                       juce::Random::getSystemRandom().nextInt(),
                       activePads);
}

bool GrooveSeqAudioProcessor::getStepState(int pad, int step) const
{
    const juce::SpinLock::ScopedLockType lock(sequenceLock);
    return sequencer.isStepActive(pad, step);
}

void GrooveSeqAudioProcessor::setStepState(int pad, int step, bool enabled)
{
    const juce::SpinLock::ScopedLockType lock(sequenceLock);
    sequencer.setStepActive(pad, step, enabled);
}

int GrooveSeqAudioProcessor::getCurrentStep() const
{
    return currentStep.load(std::memory_order_relaxed);
}

juce::AudioProcessorValueTreeState::ParameterLayout GrooveSeqAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "swing", "Swing", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "humanize", "Humanize", juce::NormalisableRange<float>(0.0f, 50.0f, 0.1f), 8.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "fills", "Fills", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 15.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "density", "Density", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 60.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "velocity", "Velocity Random", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 20.0f));

    return { params.begin(), params.end() };
}

juce::ADSR::Parameters GrooveSeqAudioProcessor::getPadAdsr(int padIndex) const
{
    if (padIndex < 0 || padIndex >= Sequencer::kPads)
        return juce::ADSR::Parameters { 0.002f, 0.12f, 0.7f, 0.12f };

    return padAdsr[static_cast<size_t>(padIndex)];
}

void GrooveSeqAudioProcessor::setPadAdsr(int padIndex, const juce::ADSR::Parameters& params)
{
    if (padIndex < 0 || padIndex >= Sequencer::kPads)
        return;

    padAdsr[static_cast<size_t>(padIndex)] = params;

    const juce::SpinLock::ScopedLockType lock(synthLock);
    if (auto* sound = padSounds[static_cast<size_t>(padIndex)])
        sound->setEnvelopeParameters(params);
}

void GrooveSeqAudioProcessor::triggerPadPreview(int padIndex)
{
    if (padIndex < 0 || padIndex >= Sequencer::kPads)
        return;

    {
        const juce::SpinLock::ScopedLockType lock(synthLock);
        if (padSounds[static_cast<size_t>(padIndex)] == nullptr)
            return;
    }

    const int midiNote = 36 + padIndex;
    const auto velocity = static_cast<juce::uint8>(juce::roundToInt(0.9f * 127.0f));
    juce::MidiMessage noteOn = juce::MidiMessage::noteOn(1, midiNote, velocity);

    const juce::SpinLock::ScopedLockType lock(previewLock);
    previewMidi.addEvent(noteOn, 0);
}
