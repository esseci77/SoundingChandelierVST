/*
  ==============================================================================

    PluginParameters.cpp
    Created: 1 Feb 2025 10:09:14pm
    Author:  Simone Campanini

  ==============================================================================
*/

#include "PluginParameters.h"
#include "global.h"

// ---------------------------------------------------------------[ esseci ]----

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout(juce::AudioProcessor* p)
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int i = 0; i < p->getTotalNumInputChannels(); ++i)
    {
        juce::String paramId("XPos");
        juce::String label("X position");
        paramId << i;
        label   << i + 1;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(paramId,     // 0
                                                                     label,
                                                                       0.0f,
                                                                     100.0f,
                                                                      50.0f));
        paramId = "YPos";
        label   = "Y position";
        paramId << i;
        label   << i + 1;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(paramId,     // 1
                                                                     label,
                                                                       0.0f,
                                                                     100.0f,
                                                                      50.0f));
        paramId = "ZPos";
        label   = "Z position";
        paramId << i;
        label   << i + 1;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(paramId,     // 2
                                                                     label,
                                                                       0.0f,
                                                                     100.0f,
                                                                      50.0f));
        paramId = "Gain";
        label   = "Gain";
        paramId << i;
        label   << i + 1;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(paramId,     // 3
                                                                     label,
                                                                       0.0f,
                                                                     100.0f,
                                                                      50.0f));

    }
    return { params.begin(), params.end() };
}

SoundingChandelierParameters::SoundingChandelierParameters(juce::AudioProcessor& p)
  : m_valueTreeState(p, nullptr, "LampaParameters", createParameterLayout(&p))
{
    m_nSources = NSRCE; //p.getTotalNumInputChannels();
    m_srcParams.resize(m_nSources);
    
    for (int i = 0; i < m_nSources; ++i)
    {
        juce::String paramId("XPos");
        paramId << i;
        m_srcParams[i].xpos  = (juce::AudioParameterFloat*)m_valueTreeState.getParameter(paramId);
        paramId = "YPos";
        paramId << i;
        m_srcParams[i].ypos  = (juce::AudioParameterFloat*)m_valueTreeState.getParameter(paramId);
        paramId = "ZPos";
        paramId << i;
        m_srcParams[i].zpos  = (juce::AudioParameterFloat*)m_valueTreeState.getParameter(paramId);
        paramId = "Gain";
        paramId << i;
        m_srcParams[i].zpos  = (juce::AudioParameterFloat*)m_valueTreeState.getParameter(paramId);
    }
}


juce::Value SoundingChandelierParameters::getAsValue(const juce::Identifier &name,
                                                     bool shouldUpdateSynchronously)
{
    return m_valueTreeState.state.getPropertyAsValue(name, nullptr,
                                                     shouldUpdateSynchronously);
}
