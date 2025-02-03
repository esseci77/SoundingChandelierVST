/*
  ==============================================================================

    SourceControl.cpp
    Created: 1 Feb 2025 3:53:15pm
    Author:  Simone Campanini

  ==============================================================================
*/
#include "SourceControl.h"
#include "global.h"

ParameterPanel::ParameterPanel(SoundingChandelierParameters& params)
 : m_params(params)
{
    m_panel = std::make_unique<juce::PropertyPanel>();
    addAndMakeVisible(m_panel.get());
    
    int k = 0;
    
    for (int i = 0; i < m_params.numberOfSources(); ++i)
    {
        juce::String sectionName("Source ");
        sectionName << (i+1);
        
        juce::Array<juce::PropertyComponent* > controls;
        
        juce::String paramId("XPos");
        paramId << i;
        m_ids.add(paramId);
        m_values.add(m_params.getAsValue( m_ids[k]));
        controls.add(new juce::SliderPropertyComponent(m_values.getReference(k++),
                                                       "X", kXmin, kXmax, 0.01));
        paramId = "YPos";
        paramId << i;
        m_ids.add(paramId);
        m_values.add(m_params.getAsValue( m_ids[k]));
        controls.add(new juce::SliderPropertyComponent(m_values.getReference(k++),
                                                       "Y", kYmin, kYmax, 0.01));
        paramId = "ZPos";
        paramId << i;
        m_ids.add(paramId);
        m_values.add(m_params.getAsValue( m_ids[k]));
        controls.add(new juce::SliderPropertyComponent(m_values.getReference(k++),
                                                       "Z", kZmin, kZmax, 0.01));
        paramId = "Gain";
        paramId << i;
        m_ids.add(paramId);
        m_values.add(m_params.getAsValue( m_ids[k]));
        controls.add(new juce::SliderPropertyComponent(m_values.getReference(k++),
                                                       "Gain", kGainMin, kGainMax, 0.1));
        m_panel->addSection(sectionName, controls);
    }
}

void ParameterPanel::resized()
{
    auto area =  getLocalBounds();
    m_panel->setBounds(area);
}

void ParameterPanel::valueChanged(juce::Value& value)
{
    // trigger Plot.
}
