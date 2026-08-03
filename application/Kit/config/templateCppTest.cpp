/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Target.h>
#include <SysTick.h>
#include <Main.h>

//
// System stuff
//

//
// Library stuff
//
#include <UART.h>

//
// Local stuff
//
#include "MainLoop.h"

using namespace std;

RAM::Library::UART::Module<RAM::Target::UART::ModuleId::UARTID0> uart;

void mySetup()
{
}

void algorithm()
{
    RAM::Target::SysTick::delayus(500);
}

void RAM::Target::Main::setup()
{
    uart.configure(115200);

    RAM::Library::Host::setCommunicationStreams(&uart, &uart);

    RAM::Library::MainLoop::Executive::instance().setup(
        mySetup,
        algorithm,
        RAM::Target::Timer::ModuleId::TIMID1,
        8000,
        1);
}

void RAM::Target::Main::loop()
{
    RAM::Library::MainLoop::Executive::instance().execute();
}

/*___oOo___*/
