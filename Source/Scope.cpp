/*
  ==============================================================================

    Scope.cpp
    Created: 28 Jan 2025 5:25:34pm
    Author:  Simone Campanini

  ==============================================================================
*/

#include "Scope.h"
#include "PluginProcessor.h"


Scope::View::View(const juce::String& name,
                  const int nsources,
                  const float widthInMeters,
                  const float heightInMeters)
: juce::Component(m_name),
  m_positions(nsources, { 0.0f, 0.0f }),
  m_width(widthInMeters),
  m_height(heightInMeters)
{
    m_palette = { juce::Colour(0xffff0000), // red
                  juce::Colour(0xff0000ff), // blue
                  juce::Colour(0xff00ff00), // green
                  juce::Colour(0xff00ffff), // cyan
                  juce::Colour(0xffff00ff), // magenta
                  juce::Colour(0xffffff00), // yellow
                  juce::Colour(0xffffffff), // white
                  juce::Colour(0xffe0e0e0),
                  juce::Colour(0xffc0c0c0),
                  juce::Colour(0xffa0a0a0),
                  juce::Colour(0xff909090),
                  juce::Colour(0xff707070),
                  juce::Colour(0xff505050),
                  juce::Colour(0xff404040),
                  juce::Colour(0xff303030),
                  juce::Colour(0xff202020) };
}

void Scope::View::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    const float Rx = area.getWidth()  / m_width;
    const float Ry = area.getHeight() / m_height;
    
    g.fillAll(juce::Colours::black);
    
    // draw plot name;
    g.setFont(11.0f);
    g.setColour(juce::Colours::white);
    
    g.drawText(m_name, area.removeFromTop(12.0f),
               juce::Justification(juce::Justification::Flags::topLeft));
    
    // draw grid with tick every 0.5 m
    g.setColour(juce::Colours::lightgrey);
    
    const float w_2 = area.getWidth() / 2.0f;
    const float h_2 = area.getHeight() / 2.0f;
    
    g.drawLine(0.0f, h_2, area.getWidth(), h_2);
    g.drawLine(w_2, 0.0, w_2, area.getHeight());
    
    float x = 0.5f;
    float y = 0.5f;
    
    for ( ; x < 0.5f * m_width; x += 0.5f)
    {
        const float ty0 = h_2 + 3.0f;
        const float ty1 = ty0 - 6.0f;
        const float txp = w_2 + x * Rx;
        const float txn = w_2 - x * Rx;
        
        g.drawLine(txp, ty0, txp, ty1);
        g.drawLine(txn, ty0, txp, ty1);
    }
    
    for ( ; y < 0.5f * m_height; y += 0.5f)
    {
        const float tx0 = w_2 - 3.0f;
        const float tx1 = tx0 + 6.0f;
        const float typ = h_2 - y * Ry;
        const float tyn = h_2 + y * Ry;
        
        g.drawLine(tx0, typ, tx1, typ);
        g.drawLine(tx0, tyn, tx1, tyn);
    }
    
    const float d = 0.5f * m_sourceSize;
    {
        juce::ScopedLock lock(m_criticalSection);
        
        int i = 0;
        // draw sources
        for (auto& pos : m_positions)
        {
            g.setColour(m_palette[i++]);
            const juce::Point<float> screenPt(w_2 + pos.x * Rx,
                                              h_2 - pos.y * Ry);
            
            g.drawLine(screenPt.x - d, screenPt.y,      screenPt.x + d, screenPt.y);
            g.drawLine(screenPt.x,     screenPt.y - d , screenPt.x,     screenPt.y + d);
        }
    }
}

void Scope::View::update(const std::vector<juce::Point<float>>& positions)
{
    {
        juce::ScopedLock lock(m_criticalSection);        
        m_positions = positions;
    }
    repaint();
}

void Scope::View::getPositions(std::vector<juce::Point<float>>& dest) const
{
    dest = m_positions;
}
// =============================================================================

Scope::Scope(SoundingChandelierAudioProcessor* processor)
 : juce::Component("SoundSourcesScope"),
   m_processor(processor)
{
    m_topView = std::make_unique<View>("Top", NSRCE, 2.0f, 2.0f);
    addAndMakeVisible(m_topView.get());
    
    m_sideView = std::make_unique<View>("Side", NSRCE, 2.0f, 2.0f);
    addAndMakeVisible(m_sideView.get());

    const float period = 1.0f / m_framerate;
    startTimer((int)std::round(1000.0f * period));
}

void Scope::paint(juce::Graphics& g) 
{
    g.fillAll(juce::Colours::black);
}

void Scope::timerCallback()
{
    std::vector<juce::Point<float>> topPositions;
    m_topView->getPositions(topPositions);
    
    std::vector<juce::Point<float>> sidePositions;
    m_sideView->getPositions(sidePositions);
    
    int i = 0;
    const OSC_state* S;
        
    for (i = 0, S = m_processor->oscstate(); i < NSRCE; i++, S++)
    {
        if (! (S->_flags))
        {
            continue;
        }
        //x = 100 + (int)((40 * S->_x + 0.5f));
        //y = 100 - (int)((40 * S->_y + 0.5f));
        //z = 100 - (int)((40 * S->_z + 0.5f));
        topPositions[i].x = S->_x;
        topPositions[i].y = S->_y;
        
        sidePositions[i].x = S->_x;
        sidePositions[i].y = S->_z;
    }

    m_topView->update(topPositions);
    m_sideView->update(sidePositions);
}
