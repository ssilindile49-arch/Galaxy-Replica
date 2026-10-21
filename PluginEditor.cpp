#include "PluginEditor.h"

OrbitTextureAudioProcessorEditor::OrbitTextureAudioProcessorEditor(OrbitTextureAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(620, 360);
    heading.setText("ORBIT TEXTURE", juce::dontSendNotification);
    heading.setJustificationType(juce::Justification::centred);
    heading.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    addAndMakeVisible(heading);

    const std::array<const char*, 5> ids{"ORBIT", "SPREAD", "COLOR", "NOVA", "TAIL"};
    for (size_t i = 0; i < sliders.size(); ++i) {
        sliders[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        sliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 22);
        sliders[i].setRange(0.0, 1.0, 0.001);
        addAndMakeVisible(sliders[i]);
        labels[i].setText(ids[i], juce::dontSendNotification);
        labels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(labels[i]);
        attachments[i] = std::make_unique<Attachment>(processor.parameters, ids[i], sliders[i]);
    }
}

void OrbitTextureAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff10131b));
    g.setColour(juce::Colour(0xff171e2a));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(14.0f), 18.0f);
    g.setColour(juce::Colour(0xff7be0d6));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(14.0f), 18.0f, 1.5f);
    g.setColour(juce::Colours::white);
    g.setFont(13.0f);
    g.drawFittedText("Dual-layer MIDI texture instrument  •  Host tempo aware",
                     30, 62, getWidth()-60, 24, juce::Justification::centred, 1);
}

void OrbitTextureAudioProcessorEditor::resized()
{
    heading.setBounds(20, 22, getWidth()-40, 40);
    const int width = (getWidth()-60) / 5;
    for (int i=0; i<5; ++i) {
        const int x = 30 + i*width;
        labels[(size_t)i].setBounds(x, 125, width, 24);
        sliders[(size_t)i].setBounds(x+4, 150, width-8, 150);
    }
}
