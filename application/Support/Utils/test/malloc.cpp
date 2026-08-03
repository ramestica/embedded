/** Would I ever have a declaration statement? 
 */

//
// System stuff
//
#include <stdlib.h>

//
// Base stuff
//

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
#include <Host.h>
#include <MainLoop.h>

//
// Local stuff
//
#include "Utils.h"

using namespace std;
using namespace RAM::Support::Host::Console;

void setup()
{
}

void algorithm()
{
    //malloc(512);
}

void RAM::Target::Main::setup()
{
    RAM::Logic::MainLoop::Executive::instance().
        setup(
            ::setup,
            ::algorithm,
            RAM::Logic::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
            8000,
            1);
}

void RAM::Target::Main::loop()
{
    RAM::Logic::MainLoop::Executive::instance().execute();
}

/* ___oOo___ */
