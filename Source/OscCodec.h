/*
  ==============================================================================

    OscCodec.h
    Created: 1 Feb 2025 3:53:01pm
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "global.h"

class OscCodec : public juce::OSCReceiver,
                 private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
  public:
    OscCodec(unsigned int nsrce, const float ftime);

    /// Original plugin methods.
    //OSC_state* oscstate (void) { return _oscstate; }
    void  getparams (OSC_state* processorState);
    
    void setFtime(const double sampleRate, int const samplesPerBlock);
    
    /// OSC message callback
    void oscMessageReceived (const juce::OSCMessage& message) override;
    
    OSC_queue  _oscqueue;
    //OSC_state  _oscstate [NSRCE]; // moved to processor
    
    unsigned int    _nsrce;
    float           _ftime;
    float           _mgain;
};
