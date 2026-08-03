/** Would I ever have a declaration statement? 
 */

//
// Target stuff
//
#include <Main.h>

//
// System stuff
//
#include <stdlib.h>

//
// Logic stuff
//
#include <Host.h>

//
// Local stuff
//
#include "MainLoop.h"

using namespace std;
using namespace RAM::Target;
using namespace RAM::Logic;

extern void (* const g_pfnVectors[])(void);
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _estack;
extern unsigned long _ram_origin;

uint16_t counter = 0;
GPIO::Pin pin;

void mySetup()
{
    GPIO::configure(GPIO::PortId::GPIOIDF, pin, 0, 0x4);
}

void algorithm()
{
    GPIO::Digital::toggle(pin);
    GPIO::Digital::toggle(pin);
}

//-------------------------------------------------------------------------------
void RAM::Target::Main::setup()
{
    RAM::Logic::MainLoop::Executive::instance().setup(
        mySetup,
        algorithm,
        RAM::Logic::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
        2000,
        1);

    RAM::Logic::Host::Console::println((unsigned)g_pfnVectors[1], 16);
    RAM::Logic::Host::Console::println(reinterpret_cast<unsigned *>(SCB->VTOR)[15], 16);
    RAM::Logic::Host::Console::println((unsigned)&_etext, 16);
    RAM::Logic::Host::Console::println((unsigned)&_data, 16);
    RAM::Logic::Host::Console::println((unsigned)&_estack, 16);
    RAM::Logic::Host::Console::println((unsigned)&_ram_origin, 16);
    RAM::Logic::Host::Console::println(SYSCTL->RCC, 16);
    RAM::Logic::Host::Console::println(SYSCTL->RCC2, 16);
}

//-------------------------------------------------------------------------------
void RAM::Target::Main::loop()
{
    RAM::Logic::MainLoop::Executive::instance().execute();
}

/*___oOo___*/
