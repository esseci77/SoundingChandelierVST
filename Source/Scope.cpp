/*
  ==============================================================================

    Scope.cpp
    Created: 28 Jan 2025 5:25:34pm
    Author:  Simone Campanini

  ==============================================================================
*/

#include "Scope.h"
#include "PluginProcessor.h"
Scope::Scope(SoundingChandelierAudioProcessor* processor)
 : juce::Component("SoundSourcesScope"),
   m_processor(processor)
{
    const float period = 1.0f / m_framerate;
    startTimer((int)std::round(1000.0f * period));
}

void Scope::paint(juce::Graphics& g) 
{
    auto area = getLocalBounds();
        
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::grey);
    
    // TODO: draw grids for top and lateral views
    
    int i;
    const OSC_state* S;
        
    for (i = 0, S = m_processor->oscstate(); i < NSRCE; i++, S++)
    {
        if (! (S->_flags)) continue;
        //x = 100 + (int)((40 * S->_x + 0.5f));
        //y = 100 - (int)((40 * S->_y + 0.5f));
        //z = 100 - (int)((40 * S->_z + 0.5f));
        // TODO: draw cross using S->_x, S->_y, S->z in top and lateral view.
    }
}

void Scope::timerCallback()
{
    repaint();
}
