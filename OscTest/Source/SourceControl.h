/*
  ==============================================================================

    SourceControl.h
    Created: 1 Feb 2025 3:53:15pm
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class ParameterPanel : public juce::Component
{
  public:
    class FloatControl : public juce::Component
    {
      public:
        FloatControl(const juce::String& label,
                     const double min, const double max, const double step);
        void resized() override;
        
        float getValue() const;
        
        void enable(const bool onOff);
        
      private:
        std::unique_ptr<juce::Label> m_label;
        std::unique_ptr<juce::Slider> m_slider;
    };
    // =========================================================================
    
    class SourcePanel : public juce::Component
    {
      public:
        SourcePanel(const int sourceIndex = 0);
        void resized() override;
        
        void getParams(float& x, float& y, float& z,
                       float& g, float& r, float& a);
        
        void enable(const bool onOff);
        
      private:
        std::unique_ptr<juce::Label>  m_sectionLabel;
        std::unique_ptr<FloatControl> m_xCtl;
        std::unique_ptr<FloatControl> m_yCtl;
        std::unique_ptr<FloatControl> m_zCtl;
        std::unique_ptr<FloatControl> m_gainCtl;
        std::unique_ptr<FloatControl> m_radiusCtl;
        std::unique_ptr<FloatControl> m_aspeedCtl; 
    };
    // =========================================================================
    
    class SourcesPanel : public juce::Component
    {
      public:
        SourcesPanel(const int nsrce);
        void resized() override;
        
        void enable(const bool onOff);
      private:
        juce::OwnedArray<SourcePanel> m_panels;
    };
    // =========================================================================
    
  public:
    ParameterPanel();
    
    void enable(const bool onOff);
    
  private:
    void resized() override;

    std::unique_ptr<juce::Viewport> m_viewport;
};
