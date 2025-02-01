/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Scope.h"

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

    void changeListenerCallback(juce::ChangeBroadcaster* cb);
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SoundingChandelierAudioProcessor& audioProcessor;
    
    std::unique_ptr<Scope>       _scope;
    std::unique_ptr<juce::Label> _portLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundingChandelierAudioProcessorEditor)
};
