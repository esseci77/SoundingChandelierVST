#pragma once

#include <JuceHeader.h>
#include "SourceControl.h"
#include "Scope.h"
#include "OscEncoder.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component,
                       public OscEncoder::DataSource
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    // =========================================================================
    juce::String getTargetAddress() override;
    int getTargetPortNumber() override;
    int getMessagePeriod() override;
    int getOSCParam(float& x, float& y, float& z,
                    float& g, float& r, float& a) override;

    OscEncoder* oscEncoder() { return m_oscEncoder.get(); }
    
private:
    //==============================================================================
    // Your private member variables go here...
    std::unique_ptr<juce::Label> m_sourceLabel;
    std::unique_ptr<juce::ComboBox> m_sourceSelector;
    
    std::unique_ptr<ParameterPanel::SourcePanel> m_sourceControls;
    
    std::unique_ptr<juce::Label> m_periodLabel;
    std::unique_ptr<juce::TextEditor> m_periodEditor;
    
    std::unique_ptr<juce::Label> m_hostLabel;
    std::unique_ptr<juce::TextEditor> m_hostEditor;
    
    std::unique_ptr<juce::Label> m_portLabel;
    std::unique_ptr<juce::TextEditor> m_portEditor;
    
    std::unique_ptr<juce::Label> m_messageLabel;
    std::unique_ptr<juce::TextEditor> m_messageEditor;
    
    std::unique_ptr<juce::ToggleButton> m_connectBtn;
    std::unique_ptr<juce::ToggleButton> m_sendBtn;

    std::unique_ptr<Scope> m_scope;

    std::unique_ptr<OscEncoder> m_oscEncoder;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
