/*
  ==============================================================================

    WfsOscCodec.h
    Created: 10 Feb 2021 10:39:57am
    Author:  Simone Campanini

  ==============================================================================
*/

#pragma once

#include "engine/atimeref.h"

/**
 * @brief A custom unidirectonal codec for Fons's wfs messages capable of multicasting (juce OSC classes aren't).
 * To establish local multicast communication, a proper route has to be added with the command
 * sudo route -n add 224.4.0.0/24 127.0.0.1
 */
class WfsOscCodec : public juce::Thread
{
  public:
    // Rendering command:
    // /wfs_rend <id> <name> <distance> <s1_xpos> <s1_ypos> <s1_xmov> <s1_ymov> <s1_spd> \
    //                                  <s2_xpos> <s2_ypos> <s2_xmov> <s2_ymov> <s2_spd> ... \
    //                              ... <sn_xpos> <sn_ypos> <sn_xmov> <sn_ymov> <sn_spd>
    
    // Change configuration command for local installations
    // /wfs_load <id> <filename>
    
    
    /**
     * @brief This is Fons Adriaensen's \c Osc_format class from its \c beta-oscutils library
     * renamed in various parts to. Basically it stores a OSC message and it works, so why
     * I should write a new one?
     */
    class Message
    {
    public:

        Message (int size);
        ~Message (void) { }

        Message& operator=(Message& m);
        
        void set(const char* data, const int len);
        
        int putAddress (const char *v);
        int putFormat (const char *v);
        int putInt (int32_t v);
        int putFloat (float v);
        int putDouble (double v);
        int putString (const char *p);
        int putBinary (const void *p, int n);
        int done (void);

        int init (void);
        const char *getAddress (void) { return _pcomm; }
        const char *getFormat (void) { return _pform; }
        int getInt (int32_t *v);
        int getFloat (float *v);
        int getDouble (double *v);
        int getString (const char **v, int *n);
        int getBinary (const void **v, int *n);

        int read (FILE *F);
        int write (FILE *F);

        void *data (void) const { return (void *) _data.get(); }
        unsigned int size (void) const { return _size; }
        unsigned int dlen (void) const { return _dlen; }
        void setDlen (unsigned int k) { _dlen = k; }

    private:
        std::unique_ptr<char[]> _data;
        
        unsigned int _size  = 0;
        unsigned int _dlen  = 0;
        unsigned int _iform = 0;
        unsigned int _inext = 0;
        char *       _pcomm = nullptr;
        char *       _pform = nullptr;
    };
    
    /**
     * @brief This class encapsulates the callback that will be called every time a osc message arrives.
     */
    class Listener
    {
      public:
        virtual void oscMessageDispatched(WfsOscCodec::Message& message) = 0;
        virtual ~Listener() { }
    };
    
    /**
     *  @brief The constructor
     */
    WfsOscCodec();
    ~WfsOscCodec();
    
    /**
     * @brief Activate the datagram broadcast socket.
     * @param localPort The port that will be bound with the socket
     * @param multicastAddress The multicast address...
     */
    virtual bool connect(const int localPort,
                         const juce::String& multicastAddress) = 0;
    
    /**
     * @brief Close the current connection
     */
    void disconnect();
    
    /**
     * @brief If false, check for error messages...
     */
    bool isReady() const { return m_socket != nullptr; }
    
    /**
     * @brief returns true if the thread loop is running.
     */
    bool isActive() const { return m_active; }
    
    /**
     * @brief Set the period for send/receive operations.
     */
    void setPeriod(const int ms) { m_periodMs = ms; }
    
    /**
     * @brief Get error messages.
     */
    const juce::String& getErrorMessage() const { return m_errorMsg; }
    
protected:
    std::unique_ptr<juce::DatagramSocket> m_socket;
    int           m_periodMs = 100; // time interval between send/receive operations
    int           m_port = 0;
    juce::String  m_multicastAddress;
    juce::String  m_errorMsg;
    
    bool m_active = false;
    
    juce::ReadWriteLock m_rwLock;
};

/**
 * @brief Osc Broadcaster class: it sends at fixed time interval the wfs sources position to the receivers
 */
class WfsOscCodecBroadcaster : public WfsOscCodec
{
public:
    WfsOscCodecBroadcaster();
    WfsOscCodecBroadcaster(const int localPort,
                           const juce::String& multicastAddress);

    /**
     * @brief Activate the datagram broadcast socket.
     * @param localPort The port that will be bound with the socket
     * @param multicastAddress The multicast address...
     */
    bool connect(const int localPort,
                 const juce::String& multicastAddress) override;

    void run() override;
    
    /**
     * @brief Only for broadcaster instances: set pointer to parameters class
     */
    void setParameters(WFSRendererParameters* parameters);

    /**
     * @brief Only for broadcaster instances: set pointer to parameters class
     */
    void removeParameters();
    
    /**
     * @brief Schedule a wfs_load message to be sent.
     */
    void sendConfigFilePath(const juce::String& cfp);
    
    /**
      * @brief The timer used to sinchronize the renderer process.
     */
    Atimetx& timer() { return m_timetx; }

protected:
    WFSRendererParameters* m_parameters = nullptr;
    char m_dataFormat [256];
    
    std::atomic<bool> m_sendConfigFilePath;
    char m_configFilePath [256];
    
    Atimetx m_timetx; ///< The timer used to synchronize the slave renderer
};


/**
 * @brief It receive the wfs sources positions sent from the broadcaster
 */

class WfsOscCodecReceiver : public WfsOscCodec
{
  public:
    WfsOscCodecReceiver();
    WfsOscCodecReceiver(const int localPort,
                        const juce::String& multicastAddress);
    
    /**
     * @brief Activate the datagram broadcast socket.
     * @param localPort The port that will be bound with the socket
     * @param multicastAddress The multicast address...
     */
    bool connect(const int localPort,
                 const juce::String& multicastAddress) override;

    void run() override;
    
    /**
     * @brief Only for listener instances: a callback will be called every time a message will be received.
     */
    void addListener(WfsOscCodec::Listener* listener);
    
    /**
     * @brief Only for listener istances. Remove the callback.
     */
    void removeListener();
    
  protected:
    WfsOscCodec::Listener* m_listener = nullptr;
};
