/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Target.h>

//
// System stuff
//
#include <util/atomic.h> 

//
// Local stuff
//
#include "GPIO.h"

using namespace std;
using namespace RAM::Target;

//
// set constants
//
RegAddrType PORTx
    [static_cast<int>(GPIO::PORTx::IDX_N)] =
{
    &PORTB,
    &PORTC,
    &PORTD
};
RegAddrType DDRx
[static_cast<int>(GPIO::PORTx::IDX_N)] =
{
    &DDRB,
    &DDRC,
    &DDRD
};
RegAddrType PINx
[static_cast<int>(GPIO::PORTx::IDX_N)] =
{
    &PINB,
    &PINC,
    &PIND
};
 
//-------------------------------------------------------------------------------
int GPIO::enable(const Pin &pin)
{
    // FIXME: really nothing?
    
    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::setDirection(
    const Pin &pin,
    const DirectionMode dir)
{
    if ( dir == DirectionMode::INPUT_PIN )
    {
        *DDRx[int(pin.m_port)] &= ~pin.m_bit;
    }
    else
    {
        *DDRx[int(pin.m_port)] |= pin.m_bit;
    }

    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::getPortPin(
    const Timer::Module mid,
    const Timer::Select sid,
    Pin &pin)
{
    pin.m_bit = 0;

    if ( mid == Timer::Module::TIMIDX0 )
    {
        pin.m_port = PORTx::D_IDX;

        if ( sid == Timer::Select::TIMERA
             ||
             sid == Timer::Select::TIMERFULL )
        {
            pin.m_bit |= (0x1 << 6);
        }

        if ( sid == Timer::Select::TIMERB
             ||
             sid == Timer::Select::TIMERFULL )
        {
            pin.m_bit |= (0x1 << 5);
        }
    }
    else if ( mid == Timer::Module::TIMIDX1 )
    {
        pin.m_port = PORTx::B_IDX;

        if ( sid == Timer::Select::TIMERA
             ||
             sid == Timer::Select::TIMERFULL )
        {
            pin.m_bit |= (0x1 << 1);
        }

        if ( sid == Timer::Select::TIMERB
             ||
             sid == Timer::Select::TIMERFULL )
        {
            pin.m_bit |= (0x1 << 2);
        }
    }
    else
    {
        //
        // timer 2 has its OC outputs through two different ports
        //
        if ( sid == Timer::Select::TIMERFULL )
        {
            return 1;
        }

        if ( sid == Timer::Select::TIMERA )
        {
            pin.m_port = PORTx::B_IDX;
        }
        else 
        {
            pin.m_port = PORTx::D_IDX;
        }

        pin.m_bit |= (0x1 << 3);
    }

    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::enableDigitalOutput(
    const Timer::Module mid,
    const Timer::Select timer)
{
    GPIO::Pin pin;

    //
    // in the nano328p timer2 has its two timers connected to two different
    // ports, therefore, it is less of a hassle to always interrogate here for
    // the actual timer and enable one by one.
    //
    if ( timer == Timer::Select::TIMERA || timer == Timer::Select::TIMERFULL )
    {
        if ( GPIO::getPortPin(mid, Timer::Select::TIMERA, pin) )
        {
            return 1;
        }
        
        if ( GPIO::setDirection(pin, GPIO::DirectionMode::OUTPUT_PIN) )
        {
            return 1;
        }
    }
    
    if ( timer == Timer::Select::TIMERB || timer == Timer::Select::TIMERFULL )
    {
        if ( GPIO::getPortPin(mid, Timer::Select::TIMERB, pin) )
        {
            return 1;
        }
    
        if ( GPIO::setDirection(pin, GPIO::DirectionMode::OUTPUT_PIN) )
        {
            return 1;
        }
    }
    
    return 0;
}

//-------------------------------------------------------------------------------
RegType GPIO::Digital::read(const Pin &pin)
{
    return *::PINx[int(pin.m_port)];
}

//-------------------------------------------------------------------------------
void GPIO::Digital::write(
    const Pin &pin,
    const bool value)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if ( value )
        {
            *::PORTx[int(pin.m_port)] |= pin.m_bit;
        }
        else
        {
            *::PORTx[int(pin.m_port)] &= ~pin.m_bit;
        }
    }
}

//-------------------------------------------------------------------------------
void GPIO::Digital::toggle(const Pin &pin)
{
    // FIXME: why is that tm4c123 does not need to disable interrupts to do the
    // read and write in a safe way.
#ifdef TARGET_nano328p
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
#elif defined TARGET_tm4c123gxl
    {    
#else
#error "unknown target" 
#endif
        *::PINx[int(pin.m_port)] |= pin.m_bit;
    }
}

//-------------------------------------------------------------------------------
void GPIO::StatusLED::enable()
{
    GPIO::enable(m_statusLED);

    GPIO::setDirection(m_statusLED, GPIO::DirectionMode::OUTPUT_PIN);
}

/*___oOo___*/
