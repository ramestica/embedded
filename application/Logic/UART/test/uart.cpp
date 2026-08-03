/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Target.h>
#include <Interrupts.h>
#include <GPIO.h>
#include <Main.h>

//
// System stuff
//
#include <stdlib.h>
#include <string.h>

//
// Library stuff
//
#include <UART.h>

//
// Local stuff
//

using namespace RAM::Library::Print;

RAM::Target::GPIO::Pin leds;
RAM::Library::UART<RAM::Target::UART::ModuleId::UARTID0> uart;

void RAM::Target::Main::setup()
{
    uart.configure(115200);
    
    RAM::Target::GPIO::configure(
        RAM::Target::GPIO::PortId::GPIOIDF,
        leds,
        (0x1 << 1 | 0x1 << 2 | 0x1 << 3),
        RAM::Target::GPIO::DirectionMode::OUTPUT_PIN);
    
    RAM::Library::Print::setUART(&uart);

    //
    // turn off leds
    //
    RAM::Target::GPIO::Digital::write(leds, 0);

    println(666);
    println(666.);

    uint8_t zxz = 255;

    println(zxz);
}

void RAM::Target::Main::loop()
{
    println("Type something and press enter:");
    print(123.1);
    print("\t");
    println(unsigned(SYSCTL->DID0), 16);
    char c;
    while ( uart.read(c) == 0 );
    println(c);
    switch ( c )
    {
    case 'r':
        RAM::Target::GPIO::Digital::write(leds, 1<<1);
        break;
    case 'b':
        RAM::Target::GPIO::Digital::write(leds, 1<<2);
        break;
    case 'g':
        RAM::Target::GPIO::Digital::write(leds, 1<<3);
        break;
    }
}

/*___oOo___*/
