/*
  ==============================================================================

    Scope.h
    Created: 28 Jan 2025 5:25:34pm
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "OscEncoder.h"

class Scope : public juce::Component,
              public juce::Timer
{
    /// A 2d view of the source space.
    class View : public juce::Component
    {
      public:
        /// Create a view.
        /// @param[in] name The component name
        /// @param[in] nsource The number of source to plot.
        /// @param[in] widthInMeters The space width in meters.
        /// @param[in] heightInMeters The space height in meters.
        View(const juce::String& name,
             const int nsources,
             const float widthInMeters,
             const float heightInMeters);
        
        /// Update view with new sources position
        void update(const std::vector<juce::Point<float>>& positions);
        
        /// Return current positions;
        void getPositions(std::vector<juce::Point<float>>& dest) const;
        
        void paint(juce::Graphics& g) override;
        
      private:
        juce::String m_name;
        std::vector<juce::Point<float>> m_positions;
        std::vector<juce::Colour>       m_palette;
        
        float m_width      =  4.0; // meters
        float m_height     =  4.0; // meters
        float m_sourceSize = 10.0f; // pixel
        
        juce::CriticalSection m_criticalSection;
    };
    // =========================================================================
    
  public:
    Scope();
  
    void resized() override;
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    
  private:
    int m_framerate = 30; // 30 fps
    
    std::unique_ptr<View> m_topView;
    std::unique_ptr<View> m_sideView;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Scope)
};
