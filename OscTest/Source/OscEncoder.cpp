/*
  ==============================================================================

    OscCodec.cpp
    Created: 8 Feb 2025 11:32:27am
    Author:  Simone Campanini

  ==============================================================================
*/

#include "OscEncoder.h"

OscEncoder::OscEncoder(DataSource* ds)
 : m_dataSource(ds)
{
    jassert(ds);
    memset(&m_param, 0, sizeof(OSC_param));
    
    auto host = m_dataSource->getTargetAddress();
    auto port = m_dataSource->getTargetPortNumber();
    
    if (! connect(host, port))
    {
        throw(Exception("Connection error."));
    }
}

OscEncoder::~OscEncoder()
{
    disconnect();
}

static inline void unwrapAngle(float& angle)
{
    const float a = std::fabs(angle / juce::MathConstants<float>::twoPi);
    angle = angle < 0 ? juce::MathConstants<float>::twoPi * (std::floor(a) - a)
                      : juce::MathConstants<float>::twoPi * (a - std::floor(a));
}

void OscEncoder::timerCallback()
{
    float x, y, z, g, r, a;
    auto index = m_dataSource->getOSCParam(x, y, z, g, r, a);
    
    if (a == 0)
    {
        m_param._index = (int32_t)index;
        m_param._flags = 0;
        m_param._x = x;
        m_param._y = y;
        m_param._z = z;
        m_param._g = g;
        m_param._t = 0;
    }
    else
    {
        m_angularPosition += m_period * a;
        unwrapAngle(m_angularPosition);
        
        m_param._index = (int32_t)index;
        m_param._flags = 0;
        m_param._x = x + r * std::cos(m_angularPosition);
        m_param._y = y + r * std::sin(m_angularPosition);
        m_param._z = z;
        m_param._g = g;
        m_param._t += m_period;
    }
    juce::OSCMessage msg(juce::OSCAddressPattern("/source/line"));
    
    // format must be ",ifffff" -> ixyzgt
    msg.addInt32(m_param._index);
    msg.addFloat32(m_param._x);
    msg.addFloat32(m_param._y);
    msg.addFloat32(m_param._z);
    msg.addFloat32(m_param._g);
    msg.addFloat32(m_param._t);
    
    send(msg);
}

void OscEncoder::start()
{
    auto ms = m_dataSource->getMessagePeriod();
    m_period = 0.001f * ms;
    m_param._t = 0;
    m_angularPosition = 0;

    startTimer(ms);
}

void OscEncoder::stop()
{
    stopTimer();
}

void OscEncoder::getParam(OSC_param& param)
{
    memcpy(&param, &m_param, sizeof(OSC_param));
}
