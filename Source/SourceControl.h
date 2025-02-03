/*
  ==============================================================================

    SourceControl.h
    Created: 1 Feb 2025 3:53:15pm
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "PluginParameters.h"

class ParameterPanel : public juce::Component,
                       public juce::Value::Listener
{
  public:
    ParameterPanel(SoundingChandelierParameters& params);
    
  private:
    void resized() override;
    void valueChanged(juce::Value& value) override;
    
    SoundingChandelierParameters&        m_params;
    std::unique_ptr<juce::PropertyPanel> m_panel;
    
    juce::Array<juce::Identifier>        m_ids;
    juce::Array<juce::Value>             m_values;
};
