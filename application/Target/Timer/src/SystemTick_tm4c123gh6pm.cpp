/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Interrupts.h>

//
// System stuff
//

//
// Local stuff
//
#include "SystemTick.h"
 
using namespace std;
using namespace RAM::Target;

//
// some local variables
//
static uint32_t m_periodus;
static uint32_t m_load;
static uint32_t m_micros = 0;
static volatile uint32_t m_jiffies = 0;
static uint32_t m_userJiffies;
static bool *m_userFlag_p = nullptr;

/** Pin used as a standard led to indicate some application specific state.
 */
#ifdef TARGET_tivac
static const GPIO::PortId heartBeatPortId = GPIO::PortId::GPIOIDF;
static const unsigned heartBeatBitIndex = 2;
#elif defined TARGET_minim4
static const GPIO::PortId heartBeatPortId = GPIO::PortId::GPIOIDC;
static const unsigned heartBeatBitIndex = 4;
#else
#error "unknown target board" 
#endif
static GPIO::Pin heartBeatPin;

//
// FIXME: why 0x80? (copied from Energia)
//
#define SYSTICK_INT_PRIORITY    0x80

//-------------------------------------------------------------------------------
static void isr()
{
    (*systemViewRecordEnterISR)();
    
    ++m_jiffies;

    if ( m_userFlag_p != nullptr
         &&
         (m_jiffies / m_userJiffies) * m_userJiffies == m_jiffies )
    {
        *m_userFlag_p = true;
    }

    if ( heartBeatPin.m_pointer != 0 )
    {
        static unsigned last = m_jiffies;

        if ( (m_jiffies - last) * m_periodus > 500000 )
        {
            last = m_jiffies;
            
            GPIO::Digital::toggle(heartBeatPin);
        }
    }

    (*systemViewRecordExitISR)();
}

//-------------------------------------------------------------------------------
int SystemTick::start(
    const bool enableHeartBeat,
    const uint32_t periodus,
    const uint32_t userJiffies,
    bool *userFlag)
{
    if ( enableHeartBeat
         &&
         GPIO::configure(
             heartBeatPortId,
             heartBeatPin,
             0x0,
             (0x1 << heartBeatBitIndex)) )
        {
            return 1;
        }
    
    //
    // the maximum count value is 2^24
    //
    if ( (((F_CPU_CYCLES_PER_US * 1ULL) * periodus) >> 24) )
    {
        return 1;
    }

    //
    // timers load value
    //
    m_load = F_CPU_CYCLES_PER_US * periodus;

    //
    // keep a record of the period value
    //
    m_periodus = periodus;

    //
    // ???
    //
    if ( userFlag != nullptr && ((m_userJiffies = userJiffies) == 0) )
    {
        return 1;
    }

    //
    // user flag used from isr
    //
    m_userFlag_p = userFlag;

    //
    // set interrupt handler to point to our local function
    //
    Interrupts::setHandler(SysTick_IRQn, isr);

    //
    // configure and start system tick by means of CMSIS function
    //
    SysTick_Config(m_load);

    //
    // switch it to the highest programmable priority; this must be done after
    // calling SysTick_Config because that function set its priority as well
    // (lowest in that case).
    //
    NVIC_SetPriority(SysTick_IRQn, 0);

    return 0;
}

//-------------------------------------------------------------------------------
uint32_t SystemTick::jiffies()
{
    return m_jiffies;
}

//-------------------------------------------------------------------------------
uint32_t SystemTick::micros()
{
    volatile uint32_t jiffies;
    volatile RegType cnt;

    //
    // a poor's man atomic read of two distinct variables
    //
    do
    {
        jiffies = m_jiffies;

        cnt = (SysTick->VAL & 0x00ffffffUL);
    }
    while ( jiffies != m_jiffies );
    
    return jiffies * m_periodus + (m_load - cnt) / F_CPU_CYCLES_PER_US;
}

//-------------------------------------------------------------------------------
uint32_t SystemTick::mark()
{
    uint32_t tmp = m_micros;

    m_micros = micros();

    return m_micros - tmp;
}

//-------------------------------------------------------------------------------
void SystemTick::delayus(const uint32_t us)
{
    uint32_t target = micros() + us;

    while ( micros() < target );
}

/*___oOo___*/
