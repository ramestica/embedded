/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//

//
// Target stuff
//
#include <Target.h>
#include <GPIO.h>
#include <TargetTimer.h>
#include <Main.h>

//
// Local stuff
//

using namespace std;
using namespace RAM::Target;
using namespace RAM::Target::GPIO;

Timer::Module timer;

void isr()
{
    static RAM::Target::RegType mis;
    
    Timer::iclear(
        timer,
        Timer::SelectId::TIMERA,
        mis);
}

//-------------------------------------------------------------------------------
void Main::setup()
{
    StatusLED::enable();

    StatusLED::on();

    for ( int i = 0; i < 1; ++i )
    {
//        delayus(500000);

        StatusLED::off();

//        delayus(500000);

        StatusLED::on();
    }

    if ( Timer::configure(
             Timer::ModuleId::TIMID0,
             Timer::Mode::Periodic,
             true,
             timer) )
    {
        return;
    }

    if ( Timer::start(
             timer,
             Timer::SelectId::TIMERA,
             1200,
             isr) )
    {
        return;
    }
} 

//-------------------------------------------------------------------------------
void Main::loop()
{
    StatusLED::toggle();
     
    delayus(150);
     
    return;
} 
 
/*___oOo___*/
