#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OrbitTextureAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit OrbitTextureAudioProcessorEditor(OrbitTextureAudioProcessor&);
    ~OrbitTextureAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    OrbitTextureAudioProcessor& processor;
    juce::Label heading;
    std::array<juce::Slider, 5> sliders;
    std::array<juce::Label, 5> labels;
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::array<std::unique_ptr<Attachment>, 5> attachments;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitTextureAudioProcessorEditor)
};
