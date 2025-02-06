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




class ParameterPanel : public juce::Component
{
    class FloatControl : public juce::Component
    {
      public:
        FloatControl(const juce::String& label,
                     juce::AudioParameterFloat& parameter,
                     const double min, const double max, const double step);
        void resized() override;
        
      private:
        std::unique_ptr<juce::Label> m_label;
        std::unique_ptr<juce::Slider> m_slider;
        std::unique_ptr<juce::SliderParameterAttachment> m_link;
    };
    // =========================================================================
    
    class SourcePanel : public juce::Component
    {
      public:
        SourcePanel(const int sourceIndex,
                    SourceParameters& parameters);
        void resized() override;
        
      private:
        std::unique_ptr<juce::Label>  m_sectionLabel;
        std::unique_ptr<FloatControl> m_xCtl;
        std::unique_ptr<FloatControl> m_yCtl;
        std::unique_ptr<FloatControl> m_zCtl;
        std::unique_ptr<FloatControl> m_gainCtl;
    };
    // =========================================================================
    
    class SourcesPanel : public juce::Component
    {
      public:
        SourcesPanel(SoundingChandelierParameters& params);
        void resized() override;
        
      private:
        juce::OwnedArray<SourcePanel> m_panels;
    };
    // =========================================================================
    
  public:
    ParameterPanel(SoundingChandelierParameters& params);
    
  private:
    void resized() override;

    std::unique_ptr<juce::Viewport> m_viewport;
};
