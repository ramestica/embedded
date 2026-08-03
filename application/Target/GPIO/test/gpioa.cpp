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
    uint8_t mask = (0x1 << 5 | 0x1 << 4);
    //SYSCTL->RCGCGPIO |= (0x1 << 0);
    //SYSCTL->GPIOHBCTL |= (0x1 << 0);
    //GPIOA_AHB->LOCK = 0x4C4F434B;
    //GPIOA_AHB->CR |= mask;
    //GPIOA_AHB->AFSEL &= ~(mask);
    //GPIOA_AHB->DIR |= (mask);
    //GPIOA_AHB->DEN |= mask;
    configure(PortId::GPIOIDA, pin, 0, mask);
} 

//-------------------------------------------------------------------------------
void Main::loop()
{
    static unsigned i = 0;
    static bool first = true;
    
//    Digital::toggle(pin, 0x1 << 5);
//    Digital::toggle(pin, 0x1 << 4);
    if ( first )
    {
        GPIOA_AHB->DATA[0x1 << 5] = (0x1 << 5);
        GPIOA_AHB->DATA[0x1 << 5] = 0x0;
    }
    else
    {
        if ( Digital::read(pin) != 0 )
        {
            Digital::toggle(pin, 0x1 << 5);
        }
    }
    
    if ( i < 10000000 )
    {
        GPIOA_AHB->DATA[0x1 << 4] = (0x1 << 4);
        GPIOA_AHB->DATA[0x1 << 4] = 0x0;
    }
    else if ( first )
    {
        first = false;

        setDirection(pin, 0x1 << 4, DirectionMode::INPUT_PIN);
    }
    
    ++i;
    
    return;
} 
 
/*___oOo___*/
