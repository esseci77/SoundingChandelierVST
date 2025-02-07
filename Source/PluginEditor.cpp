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
    
    _portLabel = std::make_unique<juce::Label>("portLabel", "OSC port:");
    addAndMakeVisible(_portLabel.get());
    
    _portEditor = std::make_unique<juce::TextEditor>("portEditor");
    _portEditor->setJustification(juce::Justification(juce::Justification::Flags::right));
    _portEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(6, "0123456789"), true);
    _portEditor->setText(juce::String(p.parameters().oscPort()));
    addAndMakeVisible(_portEditor.get());
    
    _toggleOsc = std::make_unique<juce::ToggleButton>("OSC connect");
    addAndMakeVisible(_toggleOsc.get());
    
    _resetBtn = std::make_unique<juce::TextButton>("Reset sources");
    addAndMakeVisible(_resetBtn.get());
    
    _saveBtn = std::make_unique<juce::TextButton>("Save");
    addAndMakeVisible(_saveBtn.get());
    
    _portEditor->setEnabled(false); // TODO: enable and get data from params.
    
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
    
    _resetBtn->onClick = [this]()
    {
        auto& scp = dynamic_cast<SoundingChandelierAudioProcessor&>(audioProcessor);
        scp.resetSourceParameters();
    };
    
    _saveBtn->onClick = [this]()
    {
        auto& scp = dynamic_cast<SoundingChandelierAudioProcessor&>(audioProcessor);
        scp.parameters().save();
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

    //g.setColour (juce::Colours::white);
    //g.setFont (juce::FontOptions (15.0f));
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void SoundingChandelierAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();
    auto strip = area.removeFromBottom(40);
    
    _toggleOsc->setBounds(strip.removeFromLeft(120).reduced(3));
    _portEditor->setBounds(strip.removeFromRight(60).reduced(3));
    _portLabel->setBounds(strip.removeFromRight(60).reduced(3));
    
    _scope->setBounds(area.removeFromLeft(area.getWidth()/2));
    strip = area.removeFromBottom(40);
    _saveBtn->setBounds(strip.removeFromRight(80).reduced(4));
    _resetBtn->setBounds(strip.removeFromLeft(100).reduced(4));
    
    _parameterPanel->setBounds(area);
}

void SoundingChandelierAudioProcessorEditor::enable(const bool onOff)
{
    _parameterPanel->enable(onOff);
    _saveBtn->setEnabled(onOff);
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
