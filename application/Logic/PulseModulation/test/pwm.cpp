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
#include <stdlib.h>

//
// Library stuff
//
#include <UART.h>
#include <MainLoop.h>

//
// Local stuff
//
#include "PulseModulation.h"

using namespace std;
using namespace RAM::Logic::Host::Console;

// F/0/1
RAM::Logic::PulseModulation::PWM::Encoder pwm0(
    RAM::Target::PWM::GeneratorId::PWMGENID6,
    RAM::Target::PWM::SelectId::PWMBOTH,
    2500,
    1000,
    2000,
    0.5);

// F/2/3
RAM::Logic::PulseModulation::PWM::Encoder pwm1(
    RAM::Target::PWM::GeneratorId::PWMGENID7,
    RAM::Target::PWM::SelectId::PWMBOTH,
    2500,
    1000,
    2000,
    0);

void setup()
{
    pwm0.setDutyCycle(RAM::Target::PWM::SelectId::PWMA, .1);
        
    pwm0.setDutyCycle(RAM::Target::PWM::SelectId::PWMB, .9);
        
    pwm1.setDutyCycle(RAM::Target::PWM::SelectId::PWMA, .01);
        
    pwm1.setDutyCycle(RAM::Target::PWM::SelectId::PWMB, .99);
}

void RAM::Target::Main::setup()
{
    RAM::Logic::MainLoop::Executive::instance().
        setup(
            ::setup,
            nullptr,
            RAM::Logic::MainLoop::SerialChannelSelector::SAME_SERIAL_CHANNELS,
            8000);

}

void RAM::Target::Main::loop()
{
    RAM::Logic::MainLoop::Executive::instance().execute();
}

/*___oOo___*/
