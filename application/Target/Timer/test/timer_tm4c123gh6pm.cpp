/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Interrupts.h>
#include <GPIO.h>
#include <Main.h>

//
// System stuff
//
#include <string.h>
 
//
// Library stuff
//
#include <UART.h>

//
// Local stuff
//
#include "target/Timer.h"
#include "SystemTick.h"

using namespace std;
using namespace RAM::Target;
using namespace RAM::Logic::Host::Console;

GPIO::Pin beatA, beatB;
Timer::Module timer, ppm;
RAM::Logic::UART::Module<RAM::Target::UART::ModuleId::UARTID0> uart(115200);

unsigned jiffyA = 0;
void isrA()
{
    static RAM::Target::RegType mis;
    
    Timer::iclear(
        timer,
        Timer::SelectId::TIMERA,
        mis);

    if ( mis & GPTM_MIS_TATOMIS )
    {
        GPIO::Digital::toggle(beatA);

        ++jiffyA;
    }
}

void isrB()
{
    static RAM::Target::RegType mis;
    
    Timer::iclear(
        timer,
        Timer::SelectId::TIMERB,
        mis);

    if ( mis & GPTM_MIS_TBTOMIS )
    {
        GPIO::Digital::toggle(beatB);
    }
}

volatile unsigned resyncCount = 0;
volatile unsigned syncCode = 0;
volatile unsigned ppmCounter = 0;
volatile unsigned timings[5];
volatile unsigned timingso[5];
void isrPPM()
{
    volatile uint32_t now = SystemTick::micros();
    static volatile uint32_t before = 0;
    static volatile RAM::Target::RegType mis;
    
    Timer::iclear(
        ppm,
        Timer::SelectId::TIMERA,
        mis);
    
    if ( mis & GPTM_IMR_CAEIM )
    {
        if ( before == 0 )
        {
            before = now;

            return;
        }

        if ( syncCode == 0 )
        {
            if ( now - before > 5000 )
            {
                syncCode = 1;
            }
        }
        else
        {
            if ( syncCode == 1 )
            {
                syncCode = 2;

                ppmCounter = 0;
            }

            if ( ppmCounter % 6 != 5 )
            {
                timings[ppmCounter % 6] = now - before;

                if ( timings[ppmCounter % 6] > 5000 )
                {
                    syncCode = 0;

                    ++resyncCount;
                    
                    return;
                }                
            }
            
            ++ppmCounter;
        }
        
        before = now;
    }
}

//-------------------------------------------------------------------------------
void Main::setup()
{    
    unsigned periodus = 100;

    RAM::Logic::Host::setCommunicationStreams(&uart, &uart);

    if ( SystemTick::start(
             true,
             10,
             0,
             nullptr) )
    {
        println("systick failed to start");
        
        return;
    }
    
    if ( GPIO::configure(
             GPIO::PortId::GPIOIDF,
             beatA,
             0x0,
             (0x1 << 3)) )
    {
        return;
    }

    if ( GPIO::configure(
             GPIO::PortId::GPIOIDF,
             beatB,
             0x0,
             (0x1 << 1)) )
    {
        return;
    }

    if ( Timer::configure(
             Timer::ModuleId::TIMID0,
             Timer::Mode::Periodic,
             true,
             timer) )
    {
        println("failed to configure timer");

        return;
    }

    if ( Timer::start(
             timer,
             Timer::SelectId::TIMERA,
             periodus,
             isrA) )
    {
        println("failed to start timer");

        return;
    }

    if ( Timer::start(
             timer,
             Timer::SelectId::TIMERB,
             periodus/2,
             isrB) )
    {
        println("failed to start timer");

        return;
    }

    //
    // wait for timers output to be stable
    //
    SystemTick::delayus(5000);

    if ( Timer::configure(
             Timer::ModuleId::TIMID1,
             Timer::Mode::InputEdgeTime,
             true,
             ppm) )
    {
        println("failed to configure ppm timer");

        return;
    }

    if ( Timer::start(
             ppm,
             Timer::SelectId::TIMERA,
             10000,
             isrPPM) )
    {
        println("failed to start ppm timer");

        return;
    }

    SystemTick::mark();

    SystemTick::delayus(2800);
    
    println(unsigned(SystemTick::mark()));

    println(unsigned(SystemTick::micros()));
    
    print(timer.m_index);
    print("\t");
    print(timer.m_isSplit);
    print("\t");
    println(timer.m_bits);

    SystemTick::delayus(60000);
    
    println(ppmCounter);
    println(timingso[0]);
    println(timingso[1]);
    println(timingso[2]);
    println(timingso[3]);
    println(timingso[4]);

    syncCode = 0;

    SystemTick::delayus(120000);

    println(ppmCounter);
    println(timingso[0]);
    println(timingso[1]);
    println(timingso[2]);
    println(timingso[3]);
    println(timingso[4]);    
}

void Main::loop()
{
    unsigned cnt = ppmCounter;
    
    if ( (cnt % 500 == 0) )
    {
        memcpy((void *)timingso, (void *)timings, 5 * sizeof(unsigned));

        if ( cnt == ppmCounter )
        {
            println(cnt);
            println(resyncCount);
            println(timingso[0]);
            println(timingso[1]);
            println(timingso[2]);
            println(timingso[3]);
            println(timingso[4]);    
        }
    }
}

/*___oOo___*/
