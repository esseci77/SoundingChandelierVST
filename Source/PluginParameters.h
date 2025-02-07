/*
  ==============================================================================

    PluginParameters.h
    Created: 1 Feb 2025 10:09:14pm
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "global.h"

struct SourceParameters
{
    juce::AudioParameterFloat*   xpos  = nullptr; // x position
    juce::AudioParameterFloat*   ypos  = nullptr; // y position
    juce::AudioParameterFloat*   zpos  = nullptr; // z position
    juce::AudioParameterFloat*   gain  = nullptr; // gain
};

class SoundingChandelierParameters
{
  public:    
    /// The constructor
    SoundingChandelierParameters(juce::AudioProcessor& p);
    
    /// Restore default values.
    void resetAudioParameters();
    
    juce::AudioProcessorValueTreeState& valueTree() { return m_valueTreeState; }

    int numberOfSources() const { return m_nSources; }
    int oscPort() const;
    void setOscPort(const int port);

    SourceParameters& operator[](const int idx) { return m_srcParams[idx]; }
    
    
    /// Copy parameter value to state array.
    void copyTo(OSC_state* stateArray) const;

    /// Copy value from state array to parameter.
    void copyFrom(const OSC_state* stateArray);

    /**
     * @brief Update parameters
     * @param dt Time interval in seconds
     */
    void update(const double dt);
        
    void setParamPeriod(const float pp) { m_paramPeriod = (int)pp; }
    int paramPeriod() const { return m_paramPeriod; }

    void setParamOffset(const float po) { m_paramOffset = (int)po; }
    int paramOffset() const { return m_paramOffset; }

    juce::Value getAsValue(const juce::Identifier &name,
                           bool shouldUpdateSynchronously = true);
    
    /// Return speaker status (10 channels are dedicated to AMB system).
    bool isSpeakerActive(const int channel) const;
    
    /// Apparently some speakers has inverted cables, so a proper gain must be
    /// provided!
    float* inversionGains() const { return (float*)m_inversionGains; }

    /// Return default path.
    static juce::File defaultPath();
    
    /// Save current sources status to default path
    bool save();
        
    /// Load sources status from default path.
    bool load();

    /// Save current sources status.
    bool save(const juce::File& file);
        
    /// Load sources status.
    bool load(const juce::File& file);
    
   private:
     juce::AudioProcessorValueTreeState m_valueTreeState;
    
    std::vector<SourceParameters> m_srcParams;
    int   m_nSources    = 0;
    int   m_oscPort     = kDefaultUDPPort;
    int   m_paramPeriod = 2048; // this can be modified only on startup!
    int   m_paramOffset = 0;
    float m_maxdist     = 2.0;  // m

    char m_config52 [64] =
    {
        0, 0, 1, 0, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 1, 0, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 1, 0, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 1, 0, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1
    };
    
    /// Some speakers has inverted cables, so proper gain must be provided!
    float m_inversionGains [64] =
    {
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, //  0 -  7
       -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, //  8 - 15
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // 16 - 23
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // 24 - 31
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // 32 - 39
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // 40 - 47
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // 48 - 55
        1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // 56 - 63
    };

};
