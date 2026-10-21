#pragma once
#include <JuceHeader.h>
#include <array>

class OrbitTextureAudioProcessor final : public juce::AudioProcessor
{
public:
    OrbitTextureAudioProcessor();
    ~OrbitTextureAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    struct Voice {
        bool active = false, releasing = false;
        int note = 60;
        float velocity = 0.0f, env = 0.0f;
        double phase = 0.0, loopPhase = 0.0;
        float releaseStep = 0.001f;
    };
    static constexpr int voiceCount = 24;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void noteOn(int note, float velocity);
    void noteOff(int note);
    std::array<Voice, voiceCount> voices {};
    double sr = 44100.0;
    float bpm = 120.0f;
    juce::LinearSmoothedValue<float> orbit, spread, color, nova, tail;
    juce::dsp::StateVariableTPTFilter<float> filterL, filterR;
    juce::dsp::Reverb reverb;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitTextureAudioProcessor)
};
