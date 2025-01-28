/*
  ==============================================================================

    Scope.h
    Created: 28 Jan 2025 5:25:34pm
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class SoundingChandelierAudioProcessor;

class Scope : public juce::Component,
              public juce::Timer
{
  public:
    Scope(SoundingChandelierAudioProcessor* processor);
  
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    
  private:
    SoundingChandelierAudioProcessor* m_processor;
    int m_framerate = 30; // 30 fps
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Scope)
};