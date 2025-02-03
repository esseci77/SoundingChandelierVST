/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SoundingChandelierAudioProcessorEditor::SoundingChandelierAudioProcessorEditor (SoundingChandelierAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), audioProcessor (p)
{
    _scope = std::make_unique<Scope>(&audioProcessor);
    addAndMakeVisible(_scope.get());
    
    _parameterPanel = std::make_unique<ParameterPanel>(p.parameters());
    addAndMakeVisible(_parameterPanel.get());
    
    juce::String text("OSC port: ");
    text << kDefaultUDPPort;
    _portLabel = std::make_unique<juce::Label>("portLabel", text);
    addAndMakeVisible(_portLabel.get());
    
    _toggleOsc = std::make_unique<juce::ToggleButton>("OSC connect");
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 600);
    
    _toggleOsc->onClick = [this]()
    {
        auto& scp = dynamic_cast<SoundingChandelierAudioProcessor&>(audioProcessor);
        
        if (_toggleOsc->getToggleState())
        {
            scp.startOSC();
        }
        else
        {
            scp.stopOSC();
        }
    };
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
    auto strip = area.removeFromBottom(40);
    
    _toggleOsc->setBounds(strip.removeFromLeft(120));
    _portLabel->setBounds(strip.removeFromRight(120));
    
    _scope->setBounds(area.removeFromLeft(area.getWidth()/2));
    _parameterPanel->setBounds(area);
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
