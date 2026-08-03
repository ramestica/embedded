//
// Target stuff
//
#include <GPIO.h>
#include <Main.h>

//
// System stuff
//
#include <stdint.h>

//
// Local stuff
//

using namespace std;
using namespace RAM::Target;

static GPIO::Pin pin;

//-------------------------------------------------------------------------------
void RAM::Target::Main::setup()
{
    if ( GPIO::configure(
             GPIO::PortId::GPIOIDF,
             pin,
             0x0,
             (0x3 << 2)) ) // F2 and F3
    {
        return;
    }
}

//-------------------------------------------------------------------------------
void RAM::Target::Main::loop()
{
    __IO uint32_t *datap = &reinterpret_cast<GPIOA_Type *>(pin.m_pointer)->DATA[pin.m_outMask];

    *datap = 0x0;

    //
    // this a very tight loop, with very little number of
    // instructions, the idea is to check on the scope how much time
    // (clock cycles) all this is taking to happen. At 80 MHz each
    // clock cycle takes only 12.5 nanoseconds. Note that by not
    // returning from this function we are effectively bypassing the
    // loop implemented in main's function.
    //
    while ( true )
    {
        //*datap = ~*datap;
        //*datap ^= (0x3 << 2);
        *datap ^= (0x1 << 2);
        *datap ^= (0x1 << 3);
    }
}

/*___oOo___*/
