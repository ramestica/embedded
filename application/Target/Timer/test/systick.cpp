/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Main.h>

//
// System stuff
//
 
//
// Library stuff
//
#include <UART.h>
#include <GPIO.h>

//
// Local stuff
//
#include "SystemTick.h"

using namespace std;
using namespace RAM::Target;
using namespace RAM::Logic;
using namespace RAM::Logic::Host::Console;

using namespace std;

GPIO::Pin pin;
RAM::Logic::UART::Module<RAM::Target::UART::ModuleId::UARTID0> uart(115200);

//extern "C" uint32_t SysCtlClockGet(void);

void RAM::Target::Main::setup()
{
    Host::setCommunicationStreams(&uart, &uart);

    if ( SystemTick::start(
             true,
             500,
             0,
             nullptr) )
    {
        println("systick failed to start");
        
        return;
    }

    GPIO::configure(GPIO::PortId::GPIOIDF, pin, 0, 0x1);
        
    volatile uint32_t micros0, micros1, delta;

    for ( unsigned i = 200+33; i < 4500; i+=100 )
    {
        SystemTick::mark();

        micros0 = SystemTick::micros();

        SystemTick::delayus(i);
        
        micros1 = SystemTick::micros();

        delta = SystemTick::mark();
        
        //
        // make sure not to saturate uart's transmit queue
        //
        SystemTick::delayus(1000);

        print(i);
        print(" -> ");
        print(float(micros1));
        print(" ");
        print(float(micros1) - float(micros0));
        print(" ");
        print(delta);
        print(" ");
        println(float(micros1) - float(micros0) - i, 1);
    }

    // println(SysCtlClockGet());
}

void RAM::Target::Main::loop()     
{
    GPIO::Digital::toggle(pin);
}

/*___oOo___*/
