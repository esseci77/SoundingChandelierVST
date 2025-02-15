/*
  ==============================================================================

    PluginParameters.cpp
    Created: 1 Feb 2025 10:09:14pm
    Author:  Simone Campanini

  ==============================================================================
*/

#include "PluginParameters.h"
#include "global.h"

static const juce::Array<juce::var> s_activeSpkMatrix =
{ 0, 0, 1, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 1, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 1, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 1, 0, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1 };

static const juce::Array<juce::var> s_gainSpkMatrix =
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
                                                                     kXmin,
                                                                     kXmax,
                                                                     kXdefault));
        paramId = "YPos";
        label   = "Y position";
        paramId << i;
        label   << i + 1;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(paramId,     // 1
                                                                     label,
                                                                     kYmin,
                                                                     kYmax,
                                                                     kYdefault));
        paramId = "ZPos";
        label   = "Z position";
        paramId << i;
        label   << i + 1;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(paramId,     // 2
                                                                     label,
                                                                     kZmin,
                                                                     kZmax,
                                                                     kZdefault));
        paramId = "Gain";
        label   = "Gain";
        paramId << i;
        label   << i + 1;
        params.push_back(std::make_unique<juce::AudioParameterFloat>(paramId,     // 3
                                                                     label,
                                                                     kGainMin,
                                                                     kGainMax,
                                                                     kGainDefault));

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
        m_srcParams[i].gain  = (juce::AudioParameterFloat*)m_valueTreeState.getParameter(paramId);
    }
    
    
    m_valueTreeState.state.setProperty(propertyOscPortNumberId,         juce::var(kDefaultUDPPort), nullptr)
                          .setProperty(propertyParameterRefreshRatioId, juce::var(4), nullptr)
                          .setProperty(propertyActiveSpeakerMatrixId,   juce::var(s_activeSpkMatrix), nullptr)
                          .setProperty(propertySpeakerGainsMatrixId,    juce::var(s_gainSpkMatrix), nullptr);
    
    fillMatrices();
}

void SoundingChandelierParameters::resetAudioParameters()
{
    for (int i = 0; i < m_nSources; ++i)
    {
        *m_srcParams[i].xpos = kXdefault;
        *m_srcParams[i].ypos = kYdefault;
        *m_srcParams[i].zpos = kZdefault;
        *m_srcParams[i].gain = kGainDefault;
    }
}

int SoundingChandelierParameters::oscPort() const
{
    const auto opn = m_valueTreeState.state.getProperty(propertyOscPortNumberId, juce::var(kDefaultUDPPort));
    return (int)opn;
}

void SoundingChandelierParameters::setOscPort(const int port)
{
    m_valueTreeState.state.setProperty(propertyOscPortNumberId, juce::var(port), nullptr);
}

int SoundingChandelierParameters::refreshRatio()
{
    const auto prr = m_valueTreeState.state.getProperty(propertyParameterRefreshRatioId, juce::var(4));
    return (int)prr;
}

void SoundingChandelierParameters::setRefreshRatio(const int ratio)
{
    m_valueTreeState.state.setProperty(propertyParameterRefreshRatioId, juce::var(ratio), nullptr);
}

juce::Value SoundingChandelierParameters::getAsValue(const juce::Identifier &name,
                                                     bool shouldUpdateSynchronously)
{
    return m_valueTreeState.state.getPropertyAsValue(name, nullptr,
                                                     shouldUpdateSynchronously);
}

bool SoundingChandelierParameters::isSpeakerActive(const int channel) const
{
    return (m_config52[channel] == 1);
}

void SoundingChandelierParameters::copyTo(OSC_state* stateArray) const
{
    for (int i = 0; i < m_nSources; ++i)
    {
        stateArray[i]._x = m_srcParams[i].xpos->get();
        stateArray[i]._y = m_srcParams[i].ypos->get();
        stateArray[i]._z = m_srcParams[i].zpos->get();
        stateArray[i]._g = m_srcParams[i].gain->get();
        stateArray[i]._dx = 0.0f;
        stateArray[i]._dy = 0.0f;
        stateArray[i]._dz = 0.0f;
        stateArray[i]._dg = 0.0f;
    }
}

void SoundingChandelierParameters::copyFrom(const OSC_state* stateArray)
{
    for (int i = 0; i < m_nSources; ++i)
    {
        *(m_srcParams[i].xpos) = stateArray[i]._x;
        *(m_srcParams[i].ypos) = stateArray[i]._y;
        *(m_srcParams[i].zpos) = stateArray[i]._z;
        *(m_srcParams[i].gain) = stateArray[i]._g;
    }
}

juce::File SoundingChandelierParameters::defaultPath()
{
    juce::String dir = juce::File::getSpecialLocation (juce::File:: userApplicationDataDirectory).getFullPathName(); // ~/Library
    dir << juce::File::getSeparatorChar() << "Application Support"
        << juce::File::getSeparatorChar() << "cds"
        << juce::File::getSeparatorChar() << "Lampa";
    
    juce::File fdir (dir);
    
    if (! fdir.isDirectory())
    {
        fdir.createDirectory();
    }
    dir << juce::File::getSeparatorChar() << "Lampa.settings";
    return juce::File(dir);
}

bool SoundingChandelierParameters::save()
{
    return save(defaultPath());
}
    
bool SoundingChandelierParameters::load()
{
    return load(defaultPath());
}

bool SoundingChandelierParameters::save(const juce::File& file)
{
    std::unique_ptr<juce::XmlElement> xml(m_valueTreeState.copyState().createXml());
    return xml->writeTo(file);
}

bool SoundingChandelierParameters::load(const juce::File& file)
{
    if (! file.existsAsFile())
    {
        return false;
    }
    juce::XmlDocument doc(file);
    std::unique_ptr<juce::XmlElement> xml(doc.getDocumentElement());
    
    if (! xml)
    {
        return false;
    }
    m_valueTreeState.replaceState(juce::ValueTree::fromXml(*xml));
    fillMatrices();
    
    return true;
}

void SoundingChandelierParameters::fillMatrices()
{
    juce::var value = m_valueTreeState.state.getProperty(propertyActiveSpeakerMatrixId, juce::var(s_activeSpkMatrix));
    auto aspm = value.getArray();
    
    for (int i = 0; i < NSPKR; ++i)
    {
        const auto flag = (int)(*aspm)[i];
        m_config52[i] = (char)flag;
    }
    
    value = m_valueTreeState.state.getProperty(propertySpeakerGainsMatrixId, juce::var(s_gainSpkMatrix));
    auto spgm = value.getArray();
    
    for (int i = 0; i < NSPKR; ++i)
    {
        m_inversionGains[i] = (float)(*spgm)[i];
    }
}
