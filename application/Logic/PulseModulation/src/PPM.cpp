/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
#include <stdlib.h>

//
// Support stuff
//
#include <Utils.h>

//
// Target stuff
//
#include <GPIO.h>

//
// Logic stuff
//
#include <Host.h>

//
// Local stuff
//
#include "PulseModulation.h"

using namespace std;
using namespace RAM::Support::Error;
using namespace RAM::Target;
using namespace RAM::Logic::PulseModulation;

//-------------------------------------------------------------------------------
PPM::Base::Base(
    const RAM::Target::Timer::ModuleId timerId,
    const unsigned nchannels,
    const unsigned framePeriodus,
    const unsigned channelMaxWidthus,
    void (*interruptHandler)()):
    m_timings_p(reinterpret_cast<volatile unsigned *>(malloc((nchannels + 1) * sizeof(unsigned)))),
    m_outputs_p(reinterpret_cast<volatile unsigned *>(malloc(2 * nchannels * sizeof(unsigned)))),
    m_params_p(reinterpret_cast<float *>(malloc(2 * nchannels * sizeof(float)))),
    m_nchannels(nchannels),
    m_frameDetectionThreshold(channelMaxWidthus + (channelMaxWidthus >> 0x1)),
    m_count(0)
{
    //
    // Futaba FP-T7UAF, FrSky D8R - XP. Measured on May 6th, 2018. 
    //
    //     1142/587/1642
    //     931/527/1579
    //     852/601/1549
    //     984/572/1640
    //     1667/578/1667
    //     579/578/1667
    //     578/578/1667
    //
    m_params_p[0 * 2 + 0] = 587. / (587 - 1642);
    m_params_p[0 * 2 + 1] = -1. / (587 - 1642);
    m_params_p[1 * 2 + 0] = 527. / (527 - 1579);
    m_params_p[1 * 2 + 1] = -1. / (527 - 1579);
    m_params_p[2 * 2 + 0] = 601. / (601 - 1549);
    m_params_p[2 * 2 + 1] = -1. / (601 - 1549);
    m_params_p[3 * 2 + 0] = 572. / (572 - 1640);
    m_params_p[3 * 2 + 1] = -1. / (572 - 1640);
    m_params_p[4 * 2 + 0] = 578. / (578 - 1667);
    m_params_p[4 * 2 + 1] = -1. / (578 - 1667);
    m_params_p[5 * 2 + 0] = 578. / (578 - 1667);
    m_params_p[5 * 2 + 1] = -1. / (578 - 1667);
    m_params_p[6 * 2 + 0] = 578. / (578 - 1667);
    m_params_p[6 * 2 + 1] = -1. / (578 - 1667);

    //
    // for a safe detection the space within a frame must be at least bigger
    // than the maximum channel width
    //
    if ( framePeriodus <= (nchannels + 1) * channelMaxWidthus )
    {
        longjmp(PPM_ERR_FRAME_PERIOD);
    }

    //
    // input edge capture supported in split mode only
    //
    if ( Timer::configure(
             timerId,
             Timer::Mode::InputEdgeTime,
             true,
             m_timer) )
    {
        longjmp(PPM_ERR_TIMER_CONFIG);
    }

    //
    // using framePeriodus here guaranties that between any to edge
    // detections the timer would not timeout
    //
    if ( Timer::start(
             m_timer,
             Timer::SelectId::TIMERA,
             framePeriodus,
             interruptHandler) )
    {
        longjmp(PPM_ERR_TIMER_START);
    }

    if ( Timer::getLoad(m_timer, Timer::SelectId::TIMERA, m_load) )
    {
        longjmp(PPM_ERR_TIMER_GET_LOAD);
    }
}

//-------------------------------------------------------------------------------
void PPM::Base::readout(unsigned &nframes, unsigned &resync, float *out)
{
    do
    {
        nframes = m_nframes;
        
        resync = m_resyncCount;

        for ( unsigned i = 0; i < m_nchannels; ++i )
        {
            out[i] = m_params_p[2 * i + 1] * m_timings_p[i] + m_params_p[2 * i];

            if ( out[i] < 0 )
            {
                out[i] = 0;
            }

            if ( out[i] > 1. )
            {
                out[i] = 1.;
            }
        }
        
    } while ( nframes != m_nframes );
}

//-------------------------------------------------------------------------------
void PPM::Base::isr()
{
    //
    // FIXME: what would happen if there are two PPM instances? Static within
    // this method are shared by all instances.
    //
    static volatile uint32_t beforePosEdge = 0, beforeNegEdge = 0;
    volatile RAM::Target::RegType mis;
    volatile uint32_t now, deltaus;
    
    RAM::Target::Timer::iclear(
        m_timer,
        RAM::Target::Timer::SelectId::TIMERA,
        mis);
    
    if ( mis & GPTM_IMR_CAEIM )
    {
        now = Timer::getValueSnapshot(m_timer, Timer::SelectId::TIMERA);

        //
        // if this was falling-edge event than just record when it happened
        //
        if ( GPIO::Digital::read(m_timer.m_pins) == 0x0 )
        {
            beforeNegEdge = now;

            return;
        }
        
        //
        // the algorithm starts after detecting two positive
        // interruts, which implies that at this moment beforeNegEdge
        // is not zero
        //
        if ( beforePosEdge == 0 )
        {
            beforePosEdge = now;

            return;
        }

        //
        // handle timer counter wraparound
        //
        if ( now < beforePosEdge )
        {
            //
            // first record the off lapse, which should be subtracted
            // from the total time between positive edges. Signal
            // codes on positive pulse states, not pulse period.
            //
            if ( now < beforeNegEdge )
            {
                deltaus = now + m_load - beforeNegEdge;
            }
            else
            {
                deltaus = now - beforeNegEdge;
            }

            //
            // if last negative edge happened earlier than last
            // positive edge than nothing is making real sense, force
            // a reset
            //
            if ( now - beforePosEdge < deltaus )
            {
                beforePosEdge = 0;
                
                ++m_resyncCount;
                
                return;
            }
        
            now += m_load;
        }
        else
        {
            deltaus = now - beforeNegEdge;
        }

        deltaus = (now - beforePosEdge - deltaus) / F_CPU_CYCLES_PER_US;

        //
        // proceed based on the current synchronization status
        //
        if ( m_syncCode == 0 )
        {
            //
            // wait for the spacing interval such that we know the current
            // interrupt corresponds to the first channel starting edge
            //
            if ( deltaus > m_frameDetectionThreshold )
            {
                m_syncCode = 1;
            }
        }
        else
        {
            //
            // acknowledge detection of first's channel interval
            //
            if ( m_syncCode == 1 )
            {
                m_syncCode = 2;

                m_count = 0;
            }

            m_timings_p[m_count] = deltaus;

            //
            // detect error conditions and if detected then force reset
            //
            if ( m_timings_p[m_count] > m_frameDetectionThreshold )
            {
                if ( m_count != m_nchannels )
                {
                    beforePosEdge = 0;
                        
                    ++m_resyncCount;

                    return;
                }

                m_count = 0;
            }
            else
            {
                if ( m_count == m_nchannels )
                {
                    beforePosEdge = 0;
                        
                    ++m_resyncCount;

                    return;
                }

                ++m_count;
            }

            //
            // update output timings once all channels had been
            // updated, that is, readout values always belong to the
            // same frame.
            //
            if ( m_count == m_nchannels )
            {
                memcpy((void *)m_outputs_p, (void *)m_timings_p, m_nchannels * sizeof(unsigned));

                //
                // event though the frame is not finished yet, then
                // still update frames counter, the important thing is
                // to update the count as soon as all channels had
                // been updated.
                //
                ++m_nframes;
            }
        }
        
        beforePosEdge = (now > m_load ? now - m_load : now);
    }
}

/*___oOo___*/
