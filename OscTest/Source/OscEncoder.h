/*
  ==============================================================================

    OscCodec.h
    Created: 8 Feb 2025 11:32:27am
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once
#include <exception>

#include <JuceHeader.h>
#include <global.h>

#define kDefaultHostName "localhost"
#define kDefaultHostAddr "127.0.0.1"
#define kDefaultPeriodMs         100

class OscEncoder : public juce::OSCSender,
                   public juce::Timer
{
  public:
    // MARK: class data source -
    class DataSource
    {
      public:
        virtual ~DataSource() = default;
        
        virtual juce::String getTargetAddress() = 0;
        virtual int  getTargetPortNumber() = 0;
        virtual int  getMessagePeriod() = 0;
        virtual int  getOSCParam(float& x, float& y, float& z,
                                 float& g, float& r, float& a) = 0;
    };
    
    // MARK: class custom exception -
    class Exception : public std::exception
    {
      public:
        Exception(const juce::String& msg) : m_message(msg) { }
        
        const char* what() const throw() { return m_message.toRawUTF8(); }
        
      private:
        juce::String m_message;
    };
    
    // MARK: class body -
    OscEncoder(DataSource* ds);
    ~OscEncoder();
    
    void start();
    void stop();
    void timerCallback() override;
    
    void getParam(OSC_param& param);
    
  private:
    DataSource* m_dataSource;
    
    OSC_param m_param;
    float m_period = 0.1; // s
    float m_angularPosition = 0;
};
