/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

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
#include "PulseModulation.h"

using namespace std;
using namespace RAM::Logic::Host::Console;
using namespace RAM::Logic::PulseModulation;

#define NCHANNELS 7

PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1> *ppm;
float timings[NCHANNELS];
unsigned minmax[NCHANNELS][2];

void setup()
{
    ppm = new PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1>(7, 18000, 2060);

    for ( unsigned i = 0; i < NCHANNELS; ++i )
    {
        minmax[i][0] = 99999;
        
        minmax[i][1] = 0;
    }    
}

void algorithm()
{
    static unsigned counter = 0;
    unsigned frame, resync;

    if ( (counter % 200) == 0 )
    {
        ppm->readout(frame, resync, timings);

        print(frame);
        print("/");
        println(resync);
        for ( unsigned i = 0; i < NCHANNELS; ++i )
        {
            if ( minmax[i][0] > timings[i] )
            {
                minmax[i][0] = timings[i];
            }

            if ( minmax[i][1] < timings[i] )
            {
                minmax[i][1] = timings[i];
            }
            
            println(timings[i]);
            //print("/");
            //print(minmax[i][0]);
            //print("/");
            //println(minmax[i][1]);
        }
    }

    ++counter;
}

void RAM::Target::Main::setup()
{
    RAM::Logic::MainLoop::Executive::instance().
        setup(
            ::setup,
            ::algorithm,
            RAM::Logic::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
            8000);
}

void RAM::Target::Main::loop()
{
    RAM::Logic::MainLoop::Executive::instance().execute();
}

/* ___oOo___ */
