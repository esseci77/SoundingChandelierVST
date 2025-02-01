/*
  ==============================================================================

    PluginParameters.h
    Created: 1 Feb 2025 10:09:14pm
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct SourceParameters
{
    juce::AudioParameterFloat*   xpos  = nullptr; // x position
    juce::AudioParameterFloat*   ypos  = nullptr; // y position
    juce::AudioParameterFloat*   zpos  = nullptr; // z position
};

class SoundingChandelierParameters
{
  public:    
    /// The constructor
    SoundingChandelierParameters(juce::AudioProcessor& p);
    
    juce::AudioProcessorValueTreeState& valueTree() { return m_valueTreeState; }

    int numberOfSources() const { return m_nSources; }
    SourceParameters& operator[](const int idx) { return m_srcParams[idx]; }
    
    bool thereAreSolos() const;
    
    /**
     * @brief Update parameters
     * @param dt Time interval in seconds
     */
    void update(const double dt);
        
    void setParamPeriod(const float pp) { m_paramPeriod = (int)pp; }
    int paramPeriod() const { return m_paramPeriod; }

    void setParamOffset(const float po) { m_paramOffset = (int)po; }
    int paramOffset() const { return m_paramOffset; }

        
   private:    
     juce::AudioProcessorValueTreeState m_valueTreeState;
    
    std::vector<SourceParameters> m_srcParams;
    int   m_nSources    = 0;
    int   m_paramPeriod = 2048; // this can be modified only on startup!
    int   m_paramOffset = 0;
    float m_maxdist     = 2.0;  // m

};
