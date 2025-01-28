/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <zita-convolver.h>

#include "spkarray.h"
#include "global.h"

class OUT_param
{
  public:

    float  _t [NSRCE];
    float  _g [NSRCE];
};

//==============================================================================
/**
*/
class SoundingChandelierAudioProcessor  : public juce::AudioProcessor,
                                          private juce::OSCReceiver,
                                          private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    SoundingChandelierAudioProcessor();
    ~SoundingChandelierAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    /// OSC message callback
    void oscMessageReceived (const juce::OSCMessage& message) override;

    /// Original plugin method.
    void  getparams (void);
    int   load_inpfilt (const char *name);
    int   load_outfilt (const char *name);

    // Rendering engine state
    enum { NONE = -1, INIT = 0, IDLE = 1, PROC = 2 };
    
private:
    enum { MAXDEL = 8192, DLMASK = MAXDEL - 1, REFDEL = MAXDEL - 2048 };
    
    int             _state;
    int             _newst;
    int             _wrind;
    unsigned int    _fsamp;
    unsigned int    _fsize;
    unsigned int    _nsrce;
    SPK_array       _array;
    OSC_queue       _oscqueue;
    OSC_state       _oscstate [NSRCE];
    OUT_param       _outparam [NSPKR];
    float           _ftime;
    float           _mgain;
    float          *_inpp [NSRCE];
    float          *_outp [NSPKR];
    Convproc        _inpconv;
    Convproc        _outconv;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundingChandelierAudioProcessor)
};
