/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SoundingChandelierAudioProcessorEditor::SoundingChandelierAudioProcessorEditor (SoundingChandelierAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    _scope = std::make_unique<Scope>(&audioProcessor);
    addAndMakeVisible(_scope.get());
    
    juce::String text("UDP port: ");
    text << kDefaultUDPPort;
    _portLabel = std::make_unique<juce::Label>("portLabel", text);
    addAndMakeVisible(_portLabel.get());
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 400);
}

SoundingChandelierAudioProcessorEditor::~SoundingChandelierAudioProcessorEditor()
{
}

//==============================================================================
void SoundingChandelierAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SoundingChandelierAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();
    auto strip = area.removeFromBottom(30);
    _portLabel->setBounds(strip.removeFromLeft(120));
    _scope->setBounds(area);
}

void SoundingChandelierAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* cb)
{
    auto lcb = dynamic_cast<LampaChangeBroadcaster*>(cb);
    
    if (lcb)
    {
        auto msg = lcb->getChangeBroadcasterMessage();
        
        if (msg.messageID == LampaChangeBroadcaster::Message::kUpdateUI)
        {
            _scope->repaint();
        }
    }
}
