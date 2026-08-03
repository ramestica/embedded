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
#include <PulseModulation.h>

//
// Local stuff
//
#include "PID.h"

using namespace std;
using namespace RAM::Library::Host::Console;
using namespace RAM::Library::PulseModulation;

// F/0/1 PWM motors
RAM::Library::PulseModulation::PWM::Encoder *pwm0p;

// RC input
PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1, 5> *ppm;
unsigned timings[5];

void setup()
{
    pwm = new RAM::Library::PulseModulation::PWM::Encoder(
        RAM::Target::PWM::GeneratorId::PWMGENID6,
        RAM::Target::PWM::SelectId::PWMBOTH,
        2500,
        1000,
        2000,
        0);

    ppm = new PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1, 5>(18000, 2070);
}

void algorithm()
{
    unsigned frame, resync;

    ppm->readout(frame, resync, timings);

    pwm->setDutyCycle(
        RAM::Target::PWM::SelectId::PWMA,
        timings[0] < 986
        ?
        0
        :
        timings[0] > 2066
        ?
        1.
        :
        (timings[0] - 986.) / (2066. - 986.));

    pwm->setDutyCycle(
        RAM::Target::PWM::SelectId::PWMB,
        timings[1] < 986
        ?
        0
        :
        timings[1] > 2066
        ?
        1.
        :
        (timings[1] - 986.) / (2066. - 986.));
}

void RAM::Target::Main::setup()
{
    RAM::Library::MainLoop::Executive::instance().
        setup(
            ::setup,
            ::algorithm,
            RAM::Library::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
            8000);
}

void RAM::Target::Main::loop()
{
    RAM::Library::MainLoop::Executive::instance().execute();
}

/*___oOo___*/
