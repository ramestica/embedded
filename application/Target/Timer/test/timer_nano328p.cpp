/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <GPIO.h>

//
// System stuff
//
 
//
// Local stuff
//
#include "TargetTimer.h"

using namespace std;
using namespace RAM::Target;

//-------------------------------------------------------------------------------
void setup()
{
    unsigned periodus = 1000;
    float idealPrescaler;
    unsigned prescaler;
    
    Serial.begin(115200);
    
    if ( Timer::adaptPrescaler(
             Timer::Module::TIMIDX0,
             periodus,
             idealPrescaler,
             prescaler) )
    {
        Serial.println("failed to adapt prescaler");

        return;
    }        

    Timer::LoadType load = (F_CPU / 1e6) * (float(periodus) / prescaler);

    if ( Timer::setMode(
             Timer::Module::TIMIDX0,
             Timer::Select::TIMERA,
             Timer::Mode::CTC,
             load) )
    {
        Serial.println("failed to set timer mode");

        return;
    }

    if ( Timer::setCompareOutputMode(
             Timer::Module::TIMIDX0,
             Timer::Select::TIMERA,
             Timer::CompareOutputMode::TOGGLE_OUTPUT) )
    {
        Serial.println("failed to set compare output mode");

        return;
    }

    if ( Timer::setOutputCompareValue(
             Timer::Module::TIMIDX0,
             Timer::Select::TIMERA,
             load) )
    {
        Serial.println("failed to set output compare value");

        return;
    }

    if ( GPIO::enableDigitalOutput(
             Timer::Module::TIMIDX0,
             Timer::Select::TIMERA) )
    {
        Serial.println("failed to enable digital output");

        return;
    }

    if ( Timer::setPrescaler(
             Timer::Module::TIMIDX0,
             Timer::Select::TIMERA,
             prescaler) )
    {
        Serial.println("failed to set prescaler");

        return;
    }

    Timer::print(Timer::Module::TIMIDX0);
    Serial.print("started");Serial.print(" ");Serial.print(prescaler);Serial.print(" ");Serial.println((int)load);
}

void loop()
{
}

/*___oOo___*/
