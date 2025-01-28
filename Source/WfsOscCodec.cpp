/*
  ==============================================================================

    WfsOscCodec.cpp
    Created: 10 Feb 2021 10:39:57am
    Author:  Simone Campanini

  ==============================================================================
*/
#include <JuceHeader.h>

#ifdef __APPLE__
#  include <sys/types.h>
#  include <sys/socket.h>     /* socket(), setsockopt(), bind(), recvfrom(), sendto() */
#  include <errno.h>          /* perror() */
#  include <netinet/in.h>     /* IPPROTO_IP, sockaddr_in, htons(), htonl() */
#  include <arpa/inet.h>

#elif defined _MSC_VER
#  include <winsock.h>

#endif

#include "PluginParameters.h"
#include "WfsOscCodec.h"

#define MSG_SIZE 2048

WfsOscCodec::Message::Message (int size) : _size (size)
{
    _data.reset(new char [size]);
}

//------------------------------------------------------------------------------------
WfsOscCodec::Message& WfsOscCodec::Message::operator=(WfsOscCodec::Message& m)
{
    if (_data.get() != m._data.get())
    {
        if (_size != m._size)
        {
            if (_data)
            {
                _data.reset(new char [m._size]);
            }
        }
        memcpy(_data.get(), m._data.get(), m._size);
        _size  = m._size;
        _dlen  = m._dlen;
        _iform = m._iform;
        _inext = m._inext;
        _pcomm = m._pcomm;
        _pform = m._pform;
    }
    return *this;
}



int WfsOscCodec::Message::putAddress (const char *v)
{
    unsigned int k;

    if (*v == 0) return -1;
    k = (unsigned int)strlen (v);
    if (k >= _size) return -1;
    _pcomm = _data.get();
    strcpy (_pcomm, v);
    _pform = 0;
    _iform = 0;
    _inext = k + 1;
    while (_inext & 3) _data [_inext++] = 0;
    return 0;
}


int WfsOscCodec::Message::putFormat (const char *v)
{
    unsigned int k;

    if (*v != ',') return -1;
    k = (unsigned int)strlen (v);
    if (k + _inext >= _size) return -1;
    _pform = _data.get() + _inext;
    strcpy (_pform, v);
    _iform = 1;
    _inext += k + 1;
    while (_inext & 3) _data [_inext++] = 0;
    return 0;
}


int WfsOscCodec::Message::putInt (int32_t v)
{
    if (!_pform || (_pform [_iform] != 'i')) return -1;
    if (4 + _inext > _size) return -1;
    *((int32_t *)(_data.get() + _inext)) = htonl (v);
    _iform++;
    _inext += 4;
    return 0;
}


int WfsOscCodec::Message::putFloat (float v)
{
    union { int32_t i; float f; } u;
    
    if (!_pform || (_pform [_iform] != 'f')) return -1;
    if (4 + _inext > _size) return -1;
    u.f = v;
    *((int *)(_data.get() + _inext)) = htonl (u.i);
    _iform++;
    _inext += 4;
    return 0;
}


int WfsOscCodec::Message::putDouble (double v)
{
    union { int32_t i [2]; double d; } u;
    
    if (!_pform || (_pform [_iform] != 'd')) return -1;
    if (4 + _inext > _size) return -1;
    u.d = v;
    ((int32_t *)(_data.get() + _inext))[0] = htonl (u.i [1]);
    ((int32_t *)(_data.get() + _inext))[1] = htonl (u.i [0]);
    _iform++;
    _inext += 8;
    return 0;
}


int WfsOscCodec::Message::putString (const char *p)
{
    unsigned int k;

    if (!_pform || (_pform [_iform] != 's')) return -1;
    k = (unsigned int)strlen (p);
    if (k + _inext >= _size) return -1;
    strcpy (_data.get() + _inext, p);
    _iform++;
    _inext += k + 1;
    while (_inext & 3) _data [_inext++] = 0;
    return 0;
}


int WfsOscCodec::Message::putBinary (const void *p, int n)
{
    if (!_pform || (_pform [_iform] != 'b')) return -1;
    if (4 + n + _inext >= _size) return -1;
    *((int32_t *)(_data.get() + _inext)) = htonl (n);
    _inext += 4;
    memcpy (_data.get() + _inext, p, n);
    _iform++;
    _inext += n;
    while (_inext & 3) _data [_inext++] = 0;
    return 0;
}


int WfsOscCodec::Message::done (void)
{
    if (!_pcomm || (_pform &&  _pform [_iform])) return -1;
    _dlen = _inext;
    return 0;
}


//------------------------------------------------------------------------------------


int WfsOscCodec::Message::init (void)
{
    unsigned int i, j;

    _pcomm = 0;
    _pform = 0;
    _iform = 0;
    _inext = 0;
    
    if (_dlen > _size) return -1;
    j = 0;
    while ((j < _size) && _data [j]) j++;
    if (j == _size) return -1;
    _pcomm = _data.get();
    j++;
    while (j & 3) j++;

    i = j;
    while ((j < _size) && _data [j]) j++;
    if (j == _size) return 0;
    _pform = _data.get() + i;
    j++;
    while (j & 3) j++;

    if (*_pform != ',') return -1;
    _inext = j;
    _iform = 1;

    return 0;
}


int WfsOscCodec::Message::getInt (int32_t *v)
{
    if (!_pform || (_pform [_iform] != 'i')) return -1;
    if (4 + _inext > _size) return -1;
    *v = ntohl (*((int32_t *)(_data.get() + _inext)));
    _iform++;
    _inext += 4;
    return 0;
}


int WfsOscCodec::Message::getFloat (float *v)
{
    union { int32_t i; float f; } u;

    if (!_pform || (_pform [_iform] != 'f')) return -1;
    if (4 + _inext > _size) return -1;
    u.i = ntohl (*((int32_t *)(_data.get() + _inext)));
    *v = u.f;
    _iform++;
    _inext += 4;
    return 0;
}


int WfsOscCodec::Message::getDouble (double *v)
{
    union { int32_t i [2]; double d; } u;

    if (!_pform || (_pform [_iform] != 'd')) return -1;
    if (8 + _inext > _size) return -1;
    u.i [0] = ntohl (((int32_t *)(_data.get() + _inext))[1]);
    u.i [1] = ntohl (((int32_t *)(_data.get() + _inext))[0]);
    *v = u.d;
    _iform++;
    _inext += 8;
    return 0;
}


int WfsOscCodec::Message::getString (const char **p, int *n)
{
    unsigned int k;

    if (!_pform || (_pform [_iform] != 's')) return -1;
    k = _inext;
    while ((k < _size) && _data [k]) k++;
    if (k == _size) return -1;
    *p = _data.get() + _inext;
    *n = k - _inext;
    k++;
    while (k & 3) k++;
    _iform++;
    _inext = k;
    return 0;
}


int WfsOscCodec::Message::getBinary (const void **p, int *n)
{
    unsigned int k;

    if (!_pform || (_pform [_iform] != 'b')) return -1;
    if (4 + _inext > _size) return -1;
    k = ntohl (*((int *)(_data.get() + _inext)));
    _inext += 4;
    if (k + _inext > _size) return -1;
    *p = _data.get() + _inext;
    *n = k;
    while (k & 3) k++;
    _iform++;
    _inext += k;
    return 0;
}


//------------------------------------------------------------------------------------


int WfsOscCodec::Message::write (FILE *F)
{
    uint32_t  k;

    if (done ()) return -1;
    k = htonl (_dlen);
    if (   (fwrite (&k, 1, 4, F) != 4)
        || (fwrite (_data.get(), 1, _dlen, F) != _dlen)) return -2;
    return 0;
}


int WfsOscCodec::Message::read (FILE *F)
{
    uint32_t  k;

    if (fread (&k, 1, 4, F) != 4) return -2;
    _dlen = ntohl (k);
    if (k > _size) return -1;
    if (fread (_data.get(), 1, _dlen, F) != _dlen) return -2;
    return init ();
}


//------------------------------------------------------------------------------------
WfsOscCodec::WfsOscCodec()
  : juce::Thread("oscCodecThread" + juce::String(RENDERER_ID))
{ }

WfsOscCodec::~WfsOscCodec()
{
    disconnect();
}

void WfsOscCodec::disconnect()
{
    signalThreadShouldExit();
    waitForThreadToExit(-1);
    
    DBG("OSC Thread exited [" + juce::String(RENDERER_ID) + "]");
    
    if (m_socket)
    {
        m_socket->shutdown();
    }
}

// -----------------------------------------------------------------------------------
WfsOscCodecBroadcaster::WfsOscCodecBroadcaster()
{
    m_sendConfigFilePath = false;
    memset(m_configFilePath, 0, 256);
}

WfsOscCodecBroadcaster::WfsOscCodecBroadcaster(const int localPort,
                                               const juce::String& multicastAddress)
{
    m_sendConfigFilePath = false;
    memset(m_configFilePath, 0, 256);
    connect(localPort, multicastAddress);
}

bool WfsOscCodecBroadcaster::connect(const int localPort,
                                     const juce::String& multicastAddress)
{
    m_socket.reset(new juce::DatagramSocket(false));
    
    m_port = localPort;
    m_multicastAddress = multicastAddress;
    startThread();
    
    return true;
}

void WfsOscCodecBroadcaster::setParameters(WFSRendererParameters* parameters)
{
    juce::ScopedWriteLock wl(m_rwLock);
    
    memset(m_dataFormat, 0, 256);
    strcat(m_dataFormat, ",isf");
    
    for (int i = 0; i < parameters->numberOfSources(); ++i)
    {
        strcat(m_dataFormat, "fffii");
    }
    strcat(m_dataFormat, "i"); // the port number closes the package
    
    m_parameters = parameters;
    jassert(m_parameters);
}

void WfsOscCodecBroadcaster::removeParameters()
{
    juce::ScopedWriteLock wl(m_rwLock);    
    m_parameters = nullptr;
}

void WfsOscCodecBroadcaster::sendConfigFilePath(const juce::String& cfp)
{
    strncpy(m_configFilePath, cfp.toRawUTF8(), 255);
    m_sendConfigFilePath = true;
}

void WfsOscCodecBroadcaster::run()
{
    m_active = true;
    WfsOscCodec::Message m(MSG_SIZE);
    
    while (! threadShouldExit())
    {
        if (m_sendConfigFilePath.load())
        {
            m.init();
            m.putAddress("/wfs_load");
            m.putFormat(",is");
            m.putInt(RENDERER_ID);
            m.putString(m_configFilePath);
            m.done();
            
            m_sendConfigFilePath = false;
        }
        else if (m_parameters)
        {
            juce::ScopedWriteLock wl(m_rwLock);
            
            m.init();
            m.putAddress("/wfs_rend");
            m.putFormat(m_dataFormat);
            m.putInt(RENDERER_ID);
            m.putString("test");
            m.putFloat(m_parameters->maxDist()); // not used
            
            for (int i = 0; i < m_parameters->numberOfSources(); ++i)
            {
                m.putFloat((float)(*m_parameters)[i].x);
                m.putFloat((float)(*m_parameters)[i].y);
                m.putFloat((float)*(*m_parameters)[i].gain);
                m.putInt((int32_t)*(*m_parameters)[i].mute);
                m.putInt((int32_t)*(*m_parameters)[i].solo);
            }
            m.putInt(m_port);
            m.done();
        }
        else
        {
            m.init();
            m.putAddress("/wfs_idle");
            m.done();
        }
        
        m_socket->write(m_multicastAddress, m_port, (void*)m.data(), m.dlen());
        sleep(m_periodMs);
    }
    m_active = false;
    DBG("WfsOscCodecBroadcaster exiting thread");
}

// -----------------------------------------------------------------------------------
WfsOscCodecReceiver::WfsOscCodecReceiver()
{ }

WfsOscCodecReceiver::WfsOscCodecReceiver(const int localPort,
                                         const juce::String& multicastAddress)
{
    connect(localPort, multicastAddress);
}

bool WfsOscCodecReceiver::connect(const int localPort,
                                  const juce::String& multicastAddress)
{
    m_socket.reset(new juce::DatagramSocket(false));
    
#ifdef __APPLE__
    struct sockaddr_in mcast_group;
    struct ip_mreq mreq;
    memset(&mcast_group, 0, sizeof(mcast_group));
    
    int fd = m_socket->getRawSocketHandle();
    
    mcast_group.sin_family      = AF_INET;
    mcast_group.sin_addr.s_addr = inet_addr(multicastAddress.toRawUTF8());
    mcast_group.sin_port        = htons(localPort);

    if (bind(fd, (struct sockaddr *) &mcast_group, sizeof(mcast_group)) < 0)
    {
        m_errorMsg = "Binding error: maybe another socket is bound to the same port?";
        m_socket = nullptr;
        return false;
    }
    
    // join the multicast group
    //mreq.imr_multiaddr.s_addr = inet_addr(server_ip);
    mreq.imr_multiaddr        = mcast_group.sin_addr;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
      
    if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
    {
        m_errorMsg = "Cannot join multicast address " + multicastAddress;
        m_socket = nullptr;
        return false;
    }
    
    // set socket as non blocking
    struct timeval tmOut;
    tmOut.tv_sec  = 0;
    tmOut.tv_usec = 100000;
    
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tmOut, sizeof(tmOut)) < 0)
    {
        m_errorMsg = "Cannot set socket non blocking."  + juce::String(strerror(errno));
        m_socket = nullptr;
        return false;
    }
    
#else
    if (! m_socket->bindToPort(localPort, multicastAddress))
    {
        m_errorMsg = "Binding error: maybe another socket is bound to the same port?";
        m_socket = nullptr;
        return false;
    }
    
    if (! m_socket->joinMulticast(multicastAddress))
    {
        m_errorMsg = "Cannot join multicast address " + multicastAddress;
        m_socket = nullptr;
        return false;
    }
#endif
    
    m_port = localPort;
    m_multicastAddress = multicastAddress;
    startThread();
    
    return true;
}

void WfsOscCodecReceiver::addListener(WfsOscCodec::Listener* listener)
{
    juce::ScopedWriteLock wl(m_rwLock);
    m_listener = listener;
}

void WfsOscCodecReceiver::removeListener()
{
    juce::ScopedWriteLock wl(m_rwLock);
    m_listener = nullptr;
}

void WfsOscCodecReceiver::run()
{
    m_active = true;
    WfsOscCodec::Message m(MSG_SIZE);
    
#ifdef __APPLE__
    int fd = m_socket->getRawSocketHandle();
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(addr);
#endif
    
    while (! threadShouldExit())
    {
        m_socket->waitUntilReady(true, m_periodMs);
#ifdef __APPLE__        
        auto readed = recvfrom(fd, (void*)m.data(), MSG_SIZE, 0,
                               (struct sockaddr *) &addr, &addr_size);
#else
        int readed = m_socket->read((void*)m.data(), MSG_SIZE, false);
#endif
        if (readed > 0 && m_listener)
        {
            juce::ScopedWriteLock wl(m_rwLock);
            m.setDlen((unsigned int)readed);
            m.init();
            m_listener->oscMessageDispatched(m);
        }
    }
    m_active = false;
    DBG("WfsOscCodecReceiver exiting thread");
}
