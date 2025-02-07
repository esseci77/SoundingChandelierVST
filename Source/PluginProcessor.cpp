/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/
#include <cassert>

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "impdata.h"
#include "osx_utils.h"

//==============================================================================
SoundingChandelierAudioProcessor::SoundingChandelierAudioProcessor()
: juce::AudioProcessor(juce::AudioProcessor::BusesProperties()
                       .withInput("LampaSources",  juce::AudioChannelSet::ambisonic(7))//discreteChannels(NSRCE))
                       .withOutput("LampaOutputs", juce::AudioChannelSet::ambisonic(7))),//discreteChannels(NSPKR))),
    _state (INIT),
    _newst (NONE),
    _fsamp (0),
    _fsize (0),
    _nsrce (NSRCE), //nsrce), this can be a plugin parameter
    _mgain (0),
    _parameters(*this)
{
    _jprio = sched_get_priority_min(SCHED_OTHER);

    // Disable output channels used for the AMB system.
    for (int i = 0; i < 64; i++)
    {
        _array.set_active (i, _parameters.isSpeakerActive(i));
    }

    char ifp [512];
    char ofp [512];

#ifdef __APPLE__
    OSXUtils::findFilterPath("inputfilt1",  ifp, 511);
    OSXUtils::findFilterPath("outputfilt2", ofp, 511);
#else
    strncpy(ifp, SHARED"/lampadario/inputfilt1.ald",  511);
    strncpy(ofp, SHARED"/lampadario/outputfilt2.ald", 511);
#endif

    load_inpfilt (ifp);
    load_outfilt (ofp);
}

SoundingChandelierAudioProcessor::~SoundingChandelierAudioProcessor()
{
}

//==============================================================================
const juce::String SoundingChandelierAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SoundingChandelierAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SoundingChandelierAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SoundingChandelierAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SoundingChandelierAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SoundingChandelierAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SoundingChandelierAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SoundingChandelierAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SoundingChandelierAudioProcessor::getProgramName (int index)
{
    return {};
}

void SoundingChandelierAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SoundingChandelierAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    _fsamp = (unsigned int)sampleRate;
    _fsize = (unsigned int)samplesPerBlock;
    _ftime = (float)_fsize / (float)_fsamp;
    _wrind = 0;
#if DEBUG
    //_oscCodec->setFtime(sampleRate, samplesPerBlock);
    const auto ich  = getTotalNumInputChannels();
    const auto och = getTotalNumOutputChannels();
    DBG("Actually " + juce::String(ich) + " inputs, " + juce::String(och) + " outputs.");
#endif
    int i, j;
    OSC_state *S;
    OUT_param *Q;

    S = _oscstate;

    for (i = 0; i < NSRCE; i++)
    {
        S->_count = 0;
        S->_flags = 0;
        S->_x = S->_y = S->_z = 0.0f;
        S->_g = -200.0f;
        S->_dline = new float [MAXDEL + 1];
        memset (S->_dline, 0, (MAXDEL + 1) * sizeof (float));
        S++;
    }
    Q = _outparam;

    for (j = 0; j < NSPKR; j++)
    {
        for (i = 0; i < NSRCE; i++)
        {
            Q->_t [i] = REFDEL;
            Q->_g [i] = 0.0f;
        }
        Q++;
    }
    _inpconv.set_options (Convproc::OPT_FFTW_MEASURE);
    _outconv.set_options (Convproc::OPT_FFTW_MEASURE);
    _state = IDLE;
        
    const int periodMs = (int)(1000.0 * _parameters.paramPeriod()
                                        / sampleRate);
    startTimer(periodMs);
}

void SoundingChandelierAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

/*
#ifndef JucePlugin_PreferredChannelConfigurations
bool SoundingChandelierAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif*/

void SoundingChandelierAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                                     juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    unsigned int   h, i, j, k;
    float          t, tnew, dt, g, gnew, dg;
    float          a0, a1, c, d, r, x, y, z, dx, dy, dz;
    float          *p1, *p2, *q;
    const Sgroup   *G;
    OUT_param      *Q;
    OSC_state      *S;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    if (_newst > NONE)
    {
        _state = _newst;
        _newst = NONE;
    }

    if (_state < IDLE)
    {
        for (int i = 0; i < totalNumInputChannels; ++i)
        {
            buffer.clear (i, 0, buffer.getNumSamples());
        }
        return;
    }
    
#if JACK_BASED_CLIENT
    for (i = 0; i < _nsrce; i++)
    {
        _inpp [i] = (float *) jack_port_get_buffer (_jack_inpports [i], nframes);
    }
    
    for (i = 0; i <  NSPKR; i++)
    {
        _outp [i] = _jack_outports [i] ? (float *) jack_port_get_buffer (_jack_outports [i], nframes)
                                       : nullptr;
    }
#else
    for (i = 0; i < _nsrce; ++i) //totalNumInputChannels
    {
        _inpp [i] = buffer.getReadPointer (i);
    }

    for (i = 0; i < NSPKR; ++i) // totalNumOutputChannels
    {
        _outp [i] = buffer.getWritePointer (i);
    }

    const size_t nframes = buffer.getNumSamples();
#endif
    
    if (_oscCodec)
    {
        _oscCodec->getparams (_oscstate);
    }
    else
    {
        _parameters.copyTo(_oscstate);
    }
    
    if (_state != PROC)
    {
        for (i = 0; i < NSPKR; i++)
        {
            if (_outp [i])
            {
                memset (_outp [i], 0, nframes * sizeof (float));
            }
        }
        return;
    }
    
    for (i = 0; i < _nsrce; i++)
    {
        memcpy (_inpconv.inpdata (i), _inpp [i], nframes * sizeof (float));
    }
    _inpconv.process ();

    S = _oscstate;
    
    for (i = 0; i < _nsrce; i++)
    {
        p1 = _inpconv.outdata (2 * i);
        p2 = _inpconv.outdata (2 * i + 1);
        q  = S->_dline + _wrind;
        
        if (_wrind == 0)
        {
            S->_dline [MAXDEL] = *p1;
        }
        
        for (k = 0; k < nframes; k++)
        {
            *q++ = *p1++;
            *q++ = *p2++;
        }
        g = S->_g + _mgain;
        S->_glin = 0.2f * ((g < -120.0) ? 0
                                        : std::powf (10.0f, 0.05 * g));
        S++;
    }
    
    r = 2.0f * _fsamp / 340.0f;
    Q = _outparam;
    
    for (j = 0; j < NSPKR; j++)
    {
        G = _array.sgroup (j);
        
        if (! G->_active) continue;

        q = _outconv.inpdata (j);
    //	q = _outp [j];
        x = G->_x;
        y = G->_y;
        z = G->_z;
        memset (q, 0, nframes * sizeof (float));
        S = _oscstate;
        
        for (i = 0; i < _nsrce; i++)
        {
            dx = x - S->_x;
            dy = y - S->_y;
            dz = z - S->_z;
            d = sqrtf (dx * dx + dy * dy + dz * dz);
            c = (dx * G->_x + dy * G->_y + dz * G->_z) / (d * G->_d);
            
            if (c < 0) c = 0;
            {
                tnew = (d - G->_d) * r + REFDEL;
            }
            gnew = c * S->_glin;

            t = Q->_t [i];
            g = Q->_g [i];
            Q->_t [i] = tnew;
            Q->_g [i] = gnew;
            dt = (tnew - t) / nframes + 1e-30f;
            dg = (gnew - g) / nframes + 1e-30f;

            h = (int)(ceilf (t));
            a1 = h - t;
            a0 = 1.0f - a1;
            h = _wrind - h;
            p1 = S->_dline;
            
            for (k = 0; k < nframes; k++)
            {
                g += dg;
                h += 2;
                a0 += dt;
                a1 -= dt;
                
                if (a0 < 0)
                {
                    a0 += 1.0f;
                    a1 -= 1.0f;
                    h++;
                }
                else if (a1 < 0)
                {
                    a0 -= 1.0f;
                    a1 += 1.0f;
                    h--;
                }
                h &= DLMASK;
                q [k] += g * (a0 * p1 [h] + a1 * p1 [h + 1]);
            }
            S++;
        }
        Q++;
    }

    _outconv.process ();
    
#ifndef NO_INVERSION
    const auto igain = _parameters.inversionGains();
#endif
    
    for (i = 0; i < NSPKR; i++)
    {
        if (_outp [i])
        {
#ifdef NO_INVERSION
            memcpy (_outp [i], _outconv.outdata (i), nframes * sizeof (float));
#else
            for (j = 0; j < nframes; ++j)
            {
                _outp[i][j] = igain[i] * _outconv.outdata(i)[j];
            }
#endif
        }
    }
    
    _wrind += 2 * nframes;
    
    if (_wrind == MAXDEL)
    {
        _wrind = 0;
    }
}

//==============================================================================
bool SoundingChandelierAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SoundingChandelierAudioProcessor::createEditor()
{
    return new SoundingChandelierAudioProcessorEditor (*this);
}

//==============================================================================
void SoundingChandelierAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SoundingChandelierAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoundingChandelierAudioProcessor();
}

void SoundingChandelierAudioProcessor::timerCallback()
{
    if (_oscCodec)
    {
        _parameters.copyFrom(_oscstate);
    }
    
    //getChangeBroadcasterMessage().messageID = LampaChangeBroadcaster::Message::Type::kUpdateUI;
    //sendChangeMessage();
}

void SoundingChandelierAudioProcessor::startOSC()
{
    if (!_oscCodec)
    {
        _oscCodec = std::make_unique<OscCodec>(_nsrce, _ftime);
        jassert(_oscCodec);
        
        auto editor = (SoundingChandelierAudioProcessorEditor*)getActiveEditor();
        editor->enable(false);
    }
}

void SoundingChandelierAudioProcessor::stopOSC()
{
    _oscCodec = nullptr;
    
    auto editor = (SoundingChandelierAudioProcessorEditor*)getActiveEditor();
    editor->enable(true);
}

void SoundingChandelierAudioProcessor::resetSourceParameters()
{
    if (_oscCodec)
    {
        _oscCodec->resetState();
    }
    else
    {
        parameters().resetAudioParameters();
    }
}

// --- Original lampa methods --------------------------------------------------

int SoundingChandelierAudioProcessor::load_inpfilt (const char *name)
{
    unsigned int  i, size, minp, maxp;
    void          *p;
    float         *input, *outp1, *outp2;
    Impdata       D;

    if (_inpconv.state () != Convproc::ST_IDLE)
    {
        fprintf (stderr, "input convolver is not idle.\n");
        return 1;
    }
    D.open_read (name);

    if (D.mode () != Impdata::MODE_READ)
    {
        fprintf (stderr, "Can't read '%s'.\n", name);
        return 2;
    }
    if (D.n_chan () != 1)
    {
        fprintf (stderr, "File '%s' has wrong format.\n", name);
        return 3;
    }

    if (D.n_fram () > 4096)
    {
        fprintf (stderr, "Input filter '%s' it too long.\n", name);
        return 3;
    }

    size = 128;
    minp = 2048;
    maxp = 4096;
    while (size < D.n_fram ()) size *= 2;
    if (minp > size) minp = size;
    if (minp < _fsize) minp = _fsize;
    if (maxp < _fsize) maxp = _fsize;

    if (_inpconv.configure (_nsrce, 2 * _nsrce, size, _fsize, minp, maxp, 0.0f))
    {
        fprintf (stderr, "Configuration of input convolution failed.\n");
        return 4;
    }

    posix_memalign (&p, 16, size * sizeof (float));
    input = (float *) p;
    posix_memalign (&p, 16, size * sizeof (float));
    outp1 = (float *) p;
    posix_memalign (&p, 16, size * sizeof (float));
    outp2 = (float *) p;

    D.read_ext (size, input);
    D.close ();
    upsample (size, input, outp1, outp2);

    _inpconv.impdata_create (0, 0, 1, outp1, 0, size);
    _inpconv.impdata_create (0, 1, 1, outp2, 0, size);
    for (i = 0; i < _nsrce; i++)
    {
        _inpconv.impdata_copy (0, 0, i, 2 * i);
        _inpconv.impdata_copy (0, 1, i, 2 * i + 1);
    }

    free (input);
    free (outp1);
    free (outp2);

    _inpconv.start_process (_jprio, SCHED_OTHER);

    return 0;
}

int SoundingChandelierAudioProcessor::load_outfilt (const char *name)
{
    unsigned int  i, size, minp, maxp;
    Impdata       D;

    if (_outconv.state () != Convproc::ST_IDLE)
    {
        fprintf (stderr, "Output convolver is not idle.\n");
        return 1;
    }
    D.open_read (name);

    if (D.mode () != Impdata::MODE_READ)
    {
        fprintf (stderr, "Can't read '%s'.\n", name);
        return 2;
    }
    if ((D.n_chan () != 1) || (D.n_sect () != NSPKR))
    {
        fprintf (stderr, "File '%s' has wrong format.\n", name);
        return 3;
    }

    if (D.n_fram () > 1024)
    {
        fprintf (stderr, "Output filter '%s' it too long.\n", name);
        return 3;
    }

    size = 128;
    minp = 1024;
    maxp = 4096;

    while (size < D.n_fram ())
    {
        size *= 2;
    }
    if (minp > size) minp = size;
    if (minp < _fsize) minp = _fsize;
    if (maxp < _fsize) maxp = _fsize;

    //_outconv.set_density (1.0f / 64);
    const float density = 1.0f / 64;

    if (_outconv.configure (NSPKR, NSPKR, D.n_fram (), _fsize, minp, maxp, density))
    {
        fprintf (stderr, "Configuration of output convolution failed.\n");
        return 4;
    }
    D.alloc ();

    for (i = 0; i < NSPKR; i++)
    {
        if (_array.sgroup (i)->_active)
        {
            D.read_sect (i);
            _outconv.impdata_create (i, i, 1, D.data (0), 0, size);
        }
    }

    D.close ();
    D.deall ();

    _outconv.start_process (_jprio, SCHED_OTHER);
    return 0;
}

int SoundingChandelierAudioProcessor::upsample (unsigned int size, const float *input, float *outp1, float *outp2)
{
    int            i;
    unsigned int   j;
    float          *p, v, x, y;
    fftwf_complex  *F0, *F1, *F2;
    fftwf_plan     PF, PT;

    jassert (!(size & (size - 1)));
    jassert (size >= 256);

    F0 = (fftwf_complex *) fftwf_malloc ((size / 2 + 1) * sizeof (fftwf_complex));
    F1 = (fftwf_complex *) fftwf_malloc ((size / 2 + 1) * sizeof (fftwf_complex));
    F2 = (fftwf_complex *) fftwf_malloc ((size / 2 + 1) * sizeof (fftwf_complex));
    PF  = fftwf_plan_dft_r2c_1d (size, outp1, F1, FFTW_ESTIMATE);
    PT  = fftwf_plan_dft_c2r_1d (size, F1, outp1, FFTW_ESTIMATE);

    // Compute interpolator in T domain, transform to
    // F-domain. Result in F0.
    memset (outp1, 0, size * sizeof (float));

    for (i = -10; i < 10; i++)
    {
        outp1 [i & (size - 1)] = sinc (i + 0.5f) * rcos ((i + 0.5f) / 10, 0.8f);
    }
    fftwf_execute_dft_r2c (PF, outp1, F0);

    // Shift impulse by 10 samples, fade out the end and
    // transform to F domain. Result in F1;
    memset (outp1, 0, 10 * sizeof (float));
    memcpy (outp1 + 10, input, (size - 10) * sizeof (float));
    p = outp1 + size - 20;

    for (i = 0; i < 20; i++)
    {
        p [i] *= rcos (i / 10.0f, 1.0f);
    }
    fftwf_execute_dft_r2c (PF, outp1, F1);

    // F2 = F1 * F0               --> halve a sample delay.
    // F1 = F1 * magnitude of F0  --> same freq resp as F2.
    for (i = 0; i <= (int)(size / 2); i++)
    {
        x = F1 [i][0];
        y = F1 [i][1];
        v = hypotf (F0 [i][0], F0 [i][1]);
        F1 [i][0] = x * v;
        F1 [i][1] = y * v;
        F2 [i][0] = x * F0 [i][0] - y * F0 [i][1];
        F2 [i][1] = x * F0 [i][1] + y * F0 [i][0];
    }

    // Transform back to T-domain.
    fftwf_execute_dft_c2r (PT, F1, outp1);
    fftwf_execute_dft_c2r (PT, F2, outp2);

    for (j = 0; j < size; j++)
    {
        outp1 [j] /= size;
        outp2 [j] /= size;
    }

    fftwf_destroy_plan (PF);
    fftwf_destroy_plan (PT);
    fftwf_free (F0);
    fftwf_free (F1);
    fftwf_free (F2);

    return 0;
}

float SoundingChandelierAudioProcessor::sinc (float x) const
{
    x = std::fabsf (x);
    if (x < 1e-6f) return 1.0f;
    x *= (float) M_PI;
    return std::sinf (x) / x;
}

float SoundingChandelierAudioProcessor::rcos (float x, const float p) const
{
    x = std::fabsf (x);
    if (x <= 0.0f) return 1.0f;
    if (x >= 1.0f) return 0.0f;
    return std::powf (0.5f * (1.0f + std::cosf (x * (float) M_PI)), p);
}
