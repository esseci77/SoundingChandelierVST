/*
  ==============================================================================

    SourceControl.cpp
    Created: 1 Feb 2025 3:53:15pm
    Author:  Simone Campanini

  ==============================================================================
*/
#include "SourceControl.h"
#include "global.h"

ParameterPanel::FloatControl::FloatControl(const juce::String& label,
                                           juce::AudioParameterFloat& parameter,
                                           const double min, const double max, const double step)
{
    m_label = std::make_unique<juce::Label>("paramLabel", label);
    addAndMakeVisible(m_label.get());
    
    m_slider = std::make_unique<juce::Slider>("paramLabel");
    m_slider->setSliderStyle(juce::Slider::LinearBar);
    m_slider->setRange(min, max, step);
    addAndMakeVisible(m_slider.get());
    
    m_link = std::make_unique<juce::SliderParameterAttachment>(parameter, *m_slider);
}

void ParameterPanel::FloatControl::resized()
{
    auto area = getLocalBounds();
    m_label->setBounds(area.removeFromLeft(80));
    m_slider->setBounds(area);
}

void ParameterPanel::FloatControl::enable(const bool onOff)
{
    m_slider->setEnabled(onOff);
}

ParameterPanel::SourcePanel::SourcePanel(const int sourceIndex,
                                         SourceParameters& parameters)
{
    juce::String label("Source ");
    label << sourceIndex + 1;
    m_sectionLabel = std::make_unique<juce::Label>("sectionLabel", label);
    addAndMakeVisible(m_sectionLabel.get());
    
    m_xCtl = std::make_unique<FloatControl>("X:", *parameters.xpos, kXmin, kXmax, 0.01);
    addAndMakeVisible(m_xCtl.get());

    m_yCtl = std::make_unique<FloatControl>("Y:", *parameters.ypos, kYmin, kYmax, 0.01);
    addAndMakeVisible(m_yCtl.get());

    m_zCtl = std::make_unique<FloatControl>("Z:", *parameters.zpos, kZmin, kZmax, 0.01);
    addAndMakeVisible(m_zCtl.get());

    m_gainCtl = std::make_unique<FloatControl>("Gain:", *parameters.gain, kGainMin, kGainMax, 0.01);
    addAndMakeVisible(m_gainCtl.get());
}

void ParameterPanel::SourcePanel::resized()
{
    auto area = getLocalBounds();
    const auto h = area.getHeight() / 5;
    
    m_sectionLabel->setBounds(area.removeFromTop(h));
    m_xCtl->setBounds(area.removeFromTop(h));
    m_yCtl->setBounds(area.removeFromTop(h));
    m_zCtl->setBounds(area.removeFromTop(h));
    m_gainCtl->setBounds(area.removeFromTop(h));
}

void ParameterPanel::SourcePanel::enable(const bool onOff)
{
    m_xCtl->enable(onOff);
    m_yCtl->enable(onOff);
    m_zCtl->enable(onOff);
    m_gainCtl->enable(onOff);
}

ParameterPanel::SourcesPanel::SourcesPanel(SoundingChandelierParameters& params)
{
    for (int i = 0; i < params.numberOfSources(); ++i)
    {
        m_panels.add(new SourcePanel(i, params[i]));
        addAndMakeVisible(m_panels[i]);
    }
    
    setSize(290, 5*30*params.numberOfSources());
}

void ParameterPanel::SourcesPanel::resized()
{
    auto area = getLocalBounds();
    const auto nSect = m_panels.size();
    const auto h = area.getHeight() / nSect;
    
    for (auto i = 0; i < nSect; ++i)
    {
        m_panels[i]->setBounds(area.removeFromTop(h));
    }
}

void ParameterPanel::SourcesPanel::enable(const bool onOff)
{
    const auto nSect = m_panels.size();
    
    for (auto i = 0; i < nSect; ++i)
    {
        m_panels[i]->enable(onOff);
    }
}

ParameterPanel::ParameterPanel(SoundingChandelierParameters& params)
{
    m_viewport = std::make_unique<juce::Viewport>();
    addAndMakeVisible(m_viewport.get());
    
    m_viewport->setViewedComponent(new SourcesPanel(params), true);
}

void ParameterPanel::resized()
{
    auto area =  getLocalBounds();
    m_viewport->setBounds(area);
}

void ParameterPanel::enable(const bool onOff)
{
    auto* sp = dynamic_cast<SourcePanel*>(m_viewport->getViewedComponent());
    sp->enable(onOff);
}

