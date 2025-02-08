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

#include "OscCodec.h"
#include "PluginParameters.h"

class LampaChangeBroadcaster : public juce::ChangeBroadcaster
{
  public:
    struct Message
    {
        enum Type
        {
            kNull,
            kUpdateUI,
            kLoadConfiguration
        };
        
        Type messageID = kNull;
        juce::String   contents;
    };

    Message& getChangeBroadcasterMessage() { return m_message; }
    
  private:
    Message m_message;
};
// =============================================================================

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
                                          public juce::Timer,
                                          public LampaChangeBroadcaster
{
public:
    //==============================================================================
    SoundingChandelierAudioProcessor();
    ~SoundingChandelierAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   //#ifndef JucePlugin_PreferredChannelConfigurations
    //bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   //#endif

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
    
    void timerCallback() override;
    
    /// move parameters value to current state.
    void startOSC();
    void stopOSC();
    void resetSourceParameters();
    SoundingChandelierParameters& parameters() { return _parameters; }
    
    
    OSC_state* oscstate (void) { return _oscstate; }
    int   load_inpfilt (const char *name);
    int   load_outfilt (const char *name);
    int   upsample (unsigned int size, const float *input, float *outp1, float *outp2);
    float sinc (float x) const;
    float rcos (float x, float p) const;

    // Rendering engine state
    enum { NONE = -1, INIT = 0, IDLE = 1, PROC = 2 };
    
private:
    void findFilterPath(const char* name, char* path, const size_t maxlen);
    
    int             _jprio;
    int             _state;
    int             _newst;
    int             _wrind;
    unsigned int    _fsamp;
    unsigned int    _fsize;
    unsigned int    _nsrce;
    SPK_array       _array;
    //OSC_queue       _oscqueue;
    OSC_state       _oscstate [NSRCE];
    OUT_param       _outparam [NSPKR];
    float           _ftime;
    float           _mgain;
    const float*    _inpp [NSRCE];
    float*          _outp [NSPKR];
    Convproc        _inpconv;
    Convproc        _outconv;
    
    char            _ifp [512];
    char            _ofp [512];

    std::unique_ptr<OscCodec>    _oscCodec;
    SoundingChandelierParameters _parameters;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundingChandelierAudioProcessor)
};
