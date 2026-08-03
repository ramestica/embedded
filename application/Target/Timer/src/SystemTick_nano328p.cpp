/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
#include <util/atomic.h> 

//
// Target stuff
//
#include <Target.h>

//
// Local stuff
//
#include "SysTick.h"
 
using namespace std;
using namespace RAM::Target;

//
// no heart bit pin
//
const RAM::Target::GPIO::Pin m_heartBeatPin = {RAM::Target::GPIO::PORTx::IDX_N, 0};

//
// some local variables
//
static Timer::Module m_systickModule; 
static uint32_t m_periodus;
static Timer::LoadType m_load;
static float m_resolutionns;
static unsigned m_nbits;
static float m_idealPresecaler;
static unsigned m_prescaler;
static uint32_t m_micros = 0;
static volatile uint32_t m_jiffies;
static uint32_t m_userJiffies;
static bool *m_userFlag_p = nullptr;
static volatile bool m_isDelayDone;
static unsigned int m_delaySysTickMatch;
static unsigned int m_delayCountMatch;
static uint32_t m_delayMicros;
static Timer::LoadType m_delayCount0;

//-------------------------------------------------------------------------------
static void isra()
{
    ++m_jiffies;
    
    if ( m_userFlag_p != nullptr
         &&
         (m_jiffies / m_userJiffies) * m_userJiffies == m_jiffies )
    {
        *m_userFlag_p = true;
    }
}

//-------------------------------------------------------------------------------
static void isrb()
{
    if ( m_delayMicros != 0 )
    {
        m_delaySysTickMatch = m_delayMicros / m_periodus;

        m_delayCountMatch =
            (m_delayCount0
             +
             static_cast<unsigned int>(
                 (m_delayMicros * static_cast<uint32_t>(1000))
                 /
                 m_resolutionns))
            %
            m_load;

        //
        // disarm reentering to this initialization scope
        //
        m_delayMicros = 0;

        Timer::setOutputCompareValue(
            m_systickModule,
            Timer::Select::TIMERB,
            m_delayCountMatch);

        Timer::LoadType cnt;
        
        Timer::getCounter(m_systickModule, Timer::Select::TIMERA, cnt);
        
        if ( m_delaySysTickMatch > 0 )
        {
            if ( m_delayCountMatch >= m_delayCount0 )
            {
                if ( cnt >= m_delayCount0 )
                {
                    if ( cnt >= m_delayCountMatch )
                    {
                        --m_delaySysTickMatch;
                    }
                }
                else
                {
                    --m_delaySysTickMatch;
                }
            }
            else
            {
                if ( cnt >= m_delayCountMatch )
                {
                    //--m_delaySysTickMatch;
                }
            }
        }

        return;
    }

    if ( m_delaySysTickMatch == 0 )
    {
        Timer::interruptControl(
            m_systickModule, 
            Timer::Select::TIMERB,
            nullptr);

        m_isDelayDone = true;

        return;
    }

    --m_delaySysTickMatch;

    return;
}

//-------------------------------------------------------------------------------
int SysTick::start(
    const Timer::Module tid,
    const bool enableHeartBeat,
    const uint32_t periodus,
    const uint32_t userJiffies,
    bool *userFlag)
{
    m_systickModule = tid;

    m_periodus = periodus;

    if ( userFlag != nullptr && ((m_userJiffies = userJiffies) == 0) )
    {
        return 1;
    }

    m_userFlag_p = userFlag;

    //
    // record in class variable the timer's number of bits
    //
    m_nbits = getNumberBits(m_systickModule);

    //
    // possible prescaler values are board specific
    //
    if ( Timer::adaptPrescaler(
             m_systickModule,
             m_periodus,
             m_idealPresecaler,
             m_prescaler) )
    {
        return 1;
    }

    //
    // timer's top count, when counting up from zero, to match at the requested
    // frequency
    //
    m_load = (F_CPU / 1e6) * (float(m_periodus) / m_prescaler);

    m_resolutionns = (m_prescaler * float(1000000000)) / F_CPU;

    if ( Timer::setMode(
             m_systickModule,
             Timer::Select::TIMERFULL,
             Timer::Mode::CTC,
             m_periodus) )
    {
        return 1;
    }

    if ( Timer::setCompareOutputMode(
             m_systickModule,
             Timer::Select::TIMERFULL,
             Timer::CompareOutputMode::TOGGLE_OUTPUT) )
    {
        return 1;
    }

    if ( Timer::setOutputCompareValue(
             m_systickModule,
             Timer::Select::TIMERFULL,
             m_load) )
    {
        return 1;
    }

    GPIO::enableDigitalOutput(m_systickModule, Timer::Select::TIMERA);

    Timer::interruptControl(m_systickModule, Timer::Select::TIMERA, isra);
    
    //
    // start timer with given prescaler
    //
    if ( Timer::setPrescaler(
             m_systickModule,
             Timer::Select::TIMERA,
             m_prescaler) )
    {
        return 1;
    }

    return 0;
}

//-------------------------------------------------------------------------------
uint32_t SysTick::micros()
{
    volatile uint32_t jiffies;
    Timer::LoadType cnt;

    //
    // jiffies is a 32 bits variable, disable interrupts before reading it
    //
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        Timer::getCounter(m_systickModule, Timer::Select::TIMERA, cnt);

        jiffies = m_jiffies;
    }

    return jiffies * m_periodus + ((uint32_t)cnt * m_periodus) / m_load;
}

//-------------------------------------------------------------------------------
uint32_t SysTick::mark()
{
    uint32_t tmp = micros();

    m_micros = tmp;

    return tmp;
}

//-------------------------------------------------------------------------------
uint32_t SysTick::deltaus()
{
    return micros() - m_micros; 
}

//-------------------------------------------------------------------------------
void SysTick::delayus(const uint32_t us)
{
    m_isDelayDone = false;

    m_delayMicros = us;

    if ( setOutputCompareFromCounter(
             m_systickModule,
             Timer::Select::TIMERB,
             m_delayCount0) )
    {
        //
        // FIXME: what to do on error?
        //
    }
    
    Timer::interruptControl(m_systickModule, Timer::Select::TIMERB, isrb);

    Timer::interruptTrigger(m_systickModule, Timer::Select::TIMERB);

    while ( !m_isDelayDone );
}

/*___oOo___*/
