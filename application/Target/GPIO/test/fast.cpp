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
#include <Main.h>
#include <target/SystemTick.h>

//
// Local stuff
//
#include "GPIO.h"

using namespace std;
using namespace RAM::Target;
using namespace RAM::Target::GPIO;

Pin pin;
bool isOn = true;

//-------------------------------------------------------------------------------
void Main::setup()
{
    uint8_t mask = (0x1 << 1 | 0x1 << 3);

    configure(PortId::GPIOIDF, pin, 0, mask);

    Digital::write(pin, ~mask);

    if ( SystemTick::start(
             true,
             1000,
             0,
             nullptr) )
    {
        return;
    }
} 

//-------------------------------------------------------------------------------
void Main::loop()
{
    volatile unsigned long delay;
    
    //
    // with such a local loop the frequency is 3.3 MHz, without is 1.8 MHz
    //
    while ( 1 )
    {
        Digital::toggle(pin, 0x1 << 1);
     
        Digital::toggle(pin, 0x1 << 3);
    }

    //
    // to measure clock frequency check on the scope for how long the pin is up
    //
loop:
    //GPIOF_AHB->DATA[0x8] = (0x8);
//    delay = SYSCTL->RCGCGPIO;
//    SystemTick::delayus(3);
    //GPIOF_AHB->DATA[0x8] = 0x0;
//    GPIOF_AHB->DATA[0x2] = (0x2);
//    GPIOF_AHB->DATA[0x2] = 0x0;

    goto loop;
} 
 
/*___oOo___*/
