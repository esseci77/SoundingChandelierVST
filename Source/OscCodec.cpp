/*
  ==============================================================================

    OscCodec.cpp
    Created: 1 Feb 2025 3:53:01pm
    Author:  Simone Campanini

  ==============================================================================
*/

#include "OscCodec.h"
#include "global.h"

//TODO: OSC should write data to plugin parameters!!

OscCodec::OscCodec(unsigned int nsrce, const float ftime)
 : juce::OSCReceiver("LampaOscThread"),
   _oscqueue (64),
   _nsrce(nsrce),
   _ftime(ftime)
{
    resetState();
    
    if (connect(kDefaultUDPPort))
    {
        juce::OSCReceiver::addListener(this, "/source/line");
        juce::OSCReceiver::addListener(this, "/reset");
        juce::OSCReceiver::addListener(this, "/gain");
        juce::OSCReceiver::addListener(this, "/quit");
    }
    else
    {
        DBG("Cannot connect to UDP port.");
    }
}

void OscCodec::resetState()
{
    OSC_state* S = _oscstate;

    for (int i = 0; i < NSRCE; i++)
    {
        S->_count = 0;
        S->_flags = 0;
        S->_x = S->_y = S->_z = 0.0f;
        S->_g = -200.0f;
        S->_dline = new float [MAXDEL + 1];
        memset (S->_dline, 0, (MAXDEL + 1) * sizeof (float));
        S++;
    }
}

void OscCodec::setFtime(const double sampleRate, int const samplesPerBlock)
{
    _ftime = (float)samplesPerBlock/(float)sampleRate;
}

void OscCodec::getparams (OSC_state* processorState)
{
    unsigned int  i, k, n;
    OSC_param     *P;
    OSC_state     *S;
    bool staticSource = true;

    while (_oscqueue.rd_avail())
    {
        P = _oscqueue.rd_ptr ();
        k = P->_index;
        
        if (k)
        {
            S = _oscstate + (k - 1);
            
            if (P->_t > 0)
            {
                //S = processorState + (k - 1);
                n = ceilf (P->_t / _ftime);
                S->_dx = (P->_x - S->_x) / n;
                S->_dy = (P->_y - S->_y) / n;
                S->_dz = (P->_z - S->_z) / n;
                S->_dg = (P->_g - S->_g) / n;
                S->_count = n;
                
                staticSource = false;
            }
            else
            {
                S->_x = P->_x;
                S->_y = P->_y;
                S->_z = P->_z;
                S->_g = P->_g;
            }
        }
        else
        {
            for (i = 0; i < NSRCE; i++)
            {
                _oscstate [i]._g = -200.0f;
            }
        }
        _oscqueue.rd_commit ();
    }
    
    if (! staticSource)
    {
        S = _oscstate;
        
        for (i = 0; i < _nsrce; i++)
        {
            if (S->_count)
            {
                S->_x += S->_dx;
                S->_y += S->_dy;
                S->_z += S->_dz;
                S->_g += S->_dg;
                S->_count--;
            }
            if (S->_g < -150.0f)
            {
                S->_x = S->_y = S->_z = 0;
                S->_flags = 0;
            }
            else
            {
                S->_flags = 1;
            }
            S++;
        }
    }
    
    memcpy(processorState, _oscstate, NSRCE * sizeof(OSC_state));
}

void OscCodec::oscMessageReceived (const juce::OSCMessage& message)
{
    OSC_param* P;
    auto addr = message.getAddressPattern();

    if (addr.matches("/source/line"))
    {
        // format must be ",ifffff"
        const int32_t k = message[0].getInt32();

        if ((k < 1) || (k > (int)_nsrce))
        {
            return;
        }
        P = _oscqueue.wr_ptr ();
        P->_index = k;
        P->_flags = 0;
        P->_x = message[1].getFloat32();
        P->_y = message[2].getFloat32();
        P->_z = message[3].getFloat32();
        P->_g = message[4].getFloat32();
        P->_t = message[5].getFloat32();
        _oscqueue.wr_commit ();
    }
    else if (addr.matches("/reset"))
    {
        while (_oscqueue.wr_avail () == 0)
        {
            juce::Thread::sleep(100);
        }
        P = _oscqueue.wr_ptr ();
        P->_index = 0;
        P->_flags = 0;
        _oscqueue.wr_commit ();
    }
    else if (addr.matches("/gain"))
    {
        auto& arg = message[0];
        _mgain = arg.getFloat32();
    }
    else if (addr.matches("/quit"))
    {
        // Do nothing because a plugin should not quit!
        DBG("quit received");
    }
}
