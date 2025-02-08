#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    m_scope = std::make_unique<Scope>();
    addAndMakeVisible(m_scope.get());
    
    m_sourceLabel = std::make_unique<juce::Label>("sourceLabel", "Source:");
    m_sourceLabel->setJustificationType (juce::Justification::right);
    addAndMakeVisible(m_sourceLabel.get());
    
    juce::StringArray srcLabels = { "1", "2", "3", "4",
                                    "5", "6", "7", "8" };
    m_sourceSelector = std::make_unique<juce::ComboBox>("sourceSelector");
    m_sourceSelector->addItemList(srcLabels, 100);
    addAndMakeVisible(m_sourceSelector.get());
    
    m_sourceLabel->attachToComponent (m_sourceSelector.get(), true);
    
    m_sourceControls = std::make_unique<ParameterPanel::SourcePanel>();
    addAndMakeVisible(m_sourceControls.get());
    
    m_periodLabel = std::make_unique<juce::Label>("periodLabel", "Period [ms]:");
    m_periodLabel->setJustificationType (juce::Justification::right);
    addAndMakeVisible(m_periodLabel.get());
    
    m_periodEditor = std::make_unique<juce::TextEditor>("periodEditor");
    m_periodEditor->setJustification(juce::Justification(juce::Justification::Flags::right));
    m_periodEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(6, "0123456789"), true);
    addAndMakeVisible(m_periodEditor.get());
    
    m_periodLabel->attachToComponent (m_periodEditor.get(), true);

    m_hostLabel = std::make_unique<juce::Label>("hostLabel", "Target address:");
    m_hostLabel->setJustificationType (juce::Justification::right);
    addAndMakeVisible(m_hostLabel.get());
    
    m_hostEditor = std::make_unique<juce::TextEditor>("hostEditor");
    addAndMakeVisible(m_hostEditor.get());

    m_portLabel = std::make_unique<juce::Label>("portLabel", "OSC Port:");
    m_portLabel->setJustificationType (juce::Justification::right);
    addAndMakeVisible(m_portLabel.get());
    
    m_portEditor = std::make_unique<juce::TextEditor>("portEditor");
    m_portEditor->setJustification(juce::Justification(juce::Justification::Flags::right));
    m_portEditor->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(6, "0123456789"), true);
    addAndMakeVisible(m_portEditor.get());
    
    m_portLabel->attachToComponent (m_portEditor.get(), true);
        
    m_messageLabel = std::make_unique<juce::Label>("messageLabel", "Status:");
    addAndMakeVisible(m_messageLabel.get());
    
    m_messageEditor = std::make_unique<juce::TextEditor>("messageEditor");
    m_messageEditor->setReadOnly(true);
    addAndMakeVisible(m_messageEditor.get());
    
    m_connectBtn = std::make_unique<juce::ToggleButton>("OSC connect");
    addAndMakeVisible(m_connectBtn.get());
    
    m_sendBtn = std::make_unique<juce::ToggleButton>("Send");
    addAndMakeVisible(m_sendBtn.get());

    setSize (600, 600);
    
    m_sourceSelector->setSelectedItemIndex(0, juce::dontSendNotification);
    m_periodEditor->setText(juce::String(kDefaultPeriodMs));
    m_portEditor->setText(juce::String(kDefaultUDPPort));
    m_hostEditor->setText(juce::String(kDefaultHostName));
    m_messageEditor->setText("Disconnected");
    
    m_connectBtn->onClick = [this]()
    {
        try
        {
            if (m_connectBtn->getToggleState())
            {
                m_oscEncoder = std::make_unique<OscEncoder>(this);
            }
            else
            {
                m_oscEncoder = nullptr;
            }
        }
        catch(OscEncoder::Exception& e)
        {
            juce::String msg("Error: ");
            msg << e.what();
            m_messageEditor->setText(msg);
        }
    };
    
    m_sendBtn->onClick = [this]()
    {
        if (!m_oscEncoder)
        {
            return;
        }
        
        try
        {
            if (m_sendBtn->getToggleState())
            {
                m_oscEncoder->start();
            }
            else
            {
                m_oscEncoder->stop();
            }
        }
        catch(OscEncoder::Exception& e)
        {
            juce::String msg("Error: ");
            msg << e.what();
            m_messageEditor->setText(msg);
        }
    };
}

MainComponent::~MainComponent()
{
    
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setFont (juce::FontOptions (16.0f));
    //g.setColour (juce::Colours::white);
    //g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    auto area = getLocalBounds().reduced(4);
    
    m_scope->setBounds(area.removeFromLeft(area.getWidth()/2));
    
    const int wl = 100;
    const int  h = 36;
    const int  d = 3;
    
    auto slot = area.removeFromTop(h).reduced(d);
    m_sourceLabel->setBounds(slot.removeFromLeft(wl));
    m_sourceSelector->setBounds(slot);
    
    m_sourceControls->setBounds(area.removeFromTop(7 * h).reduced(d));
    
    slot = area.removeFromTop(h).reduced(d);
    m_periodLabel->setBounds(slot.removeFromLeft(wl));
    m_periodEditor->setBounds(slot);

    slot = area.removeFromTop(h).reduced(d);
    m_hostLabel->setBounds(slot.removeFromLeft(wl));
    m_hostEditor->setBounds(slot);

    slot = area.removeFromTop(h).reduced(d);
    m_portLabel->setBounds(slot.removeFromLeft(wl));
    m_portEditor->setBounds(slot);

    slot = area.removeFromTop(h).reduced(d);
    m_messageLabel->setBounds(slot.removeFromLeft(wl));
    m_messageEditor->setBounds(slot);

    area.removeFromTop(5);
    
    auto strip = area.removeFromBottom(h);
    m_connectBtn->setBounds(strip.removeFromLeft(100).reduced(d));
    m_sendBtn->setBounds(strip.removeFromRight(80).reduced(d));
}

juce::String MainComponent::getTargetAddress()
{
    return m_hostEditor->getText();
}

int MainComponent::getTargetPortNumber()
{
    auto s = m_portEditor->getText();
    return s.getIntValue();
}

int MainComponent::getMessagePeriod()
{
    auto s = m_periodEditor->getText();
    return s.getIntValue();
}

int MainComponent::getOSCParam(float& x, float& y, float& z,
                               float& g, float& r, float& a)
{
    auto idx = m_sourceSelector->getSelectedItemIndex() + 1;
    m_sourceControls->getParams(x, y, z, g, r, a);
    return idx;
}
