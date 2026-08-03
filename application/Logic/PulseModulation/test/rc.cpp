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

// F/0/1
RAM::Logic::PulseModulation::PWM::Encoder pwm0p(
    RAM::Target::PWM::GeneratorId::PWMGENID6,
    RAM::Target::PWM::SelectId::PWMBOTH,
    2500,
    1000,
    2000,
    0);

// F/2/3
RAM::Logic::PulseModulation::PWM::Encoder pwm1p(
    RAM::Target::PWM::GeneratorId::PWMGENID7,
    RAM::Target::PWM::SelectId::PWMBOTH,
    2500,
    1000,
    2000,
    0);

PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1> ppm(7, 18000, 2070);
float timings[7];

void setup()
{
}

void algorithm()
{
    static unsigned counter = 0;
    unsigned frame, resync;
    
    ppm.readout(frame, resync, timings);

    pwm0p.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMA,
        timings[0]);

    pwm0p.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMB,
        timings[1]);

    pwm1p.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMA,
        timings[2]);

    pwm1p.setDutyCycle(
        RAM::Target::PWM::SelectId::PWMB,
        timings[3]);

    if ( (counter % 200) == 0 )
    {
        print(frame);
        print("/");
        println(resync);
        println(timings[0]);
        println(timings[1]);
        println(timings[2]);
        println(timings[3]);
        println(timings[4]);
        println(timings[5]);
        println(timings[6]);
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
