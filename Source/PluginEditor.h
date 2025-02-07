/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Scope.h"
#include "SourceControl.h"

//==============================================================================
/**
*/
class SoundingChandelierAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                public juce::ChangeListener
{
public:
    SoundingChandelierAudioProcessorEditor (SoundingChandelierAudioProcessor&);
    ~SoundingChandelierAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void enable(const bool onOff);    
    void changeListenerCallback(juce::ChangeBroadcaster* cb) override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SoundingChandelierAudioProcessor& audioProcessor;
    
    std::unique_ptr<Scope>              _scope;
    std::unique_ptr<ParameterPanel>     _parameterPanel;
    std::unique_ptr<juce::Label>        _portLabel;
    std::unique_ptr<juce::TextEditor>   _portEditor;
    std::unique_ptr<juce::ToggleButton> _toggleOsc;
    std::unique_ptr<juce::TextButton>   _resetBtn;
    std::unique_ptr<juce::TextButton>   _saveBtn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundingChandelierAudioProcessorEditor)
};
