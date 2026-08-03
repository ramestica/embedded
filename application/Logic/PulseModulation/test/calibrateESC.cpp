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
RAM::Logic::PulseModulation::PWM::Encoder *pwm0p;

// F/2/3
RAM::Logic::PulseModulation::PWM::Encoder *pwm1p;

PPM::Decoder<RAM::Target::Timer::ModuleId::TIMID1> ppm(7, 18000, 2070);
float timings[7];

void setup()
{
    pwm0p = new RAM::Logic::PulseModulation::PWM::Encoder(
        RAM::Target::PWM::GeneratorId::PWMGENID6,
        RAM::Target::PWM::SelectId::PWMBOTH,
        2500,
        1000,
        2000,
        1);

    pwm1p = new RAM::Logic::PulseModulation::PWM::Encoder(
        RAM::Target::PWM::GeneratorId::PWMGENID7,
        RAM::Target::PWM::SelectId::PWMBOTH,
        2500,
        1000,
        2000,
        1);
    
    pwm0p->setDutyCycle(RAM::Target::PWM::SelectId::PWMA, 1.);
 
    pwm0p->setDutyCycle(RAM::Target::PWM::SelectId::PWMB, 1.);
 
    pwm1p->setDutyCycle(RAM::Target::PWM::SelectId::PWMA, 1.);
 
    pwm1p->setDutyCycle(RAM::Target::PWM::SelectId::PWMB, 1.);

    RAM::Target::delayus(4000000);

    pwm0p->setDutyCycle(RAM::Target::PWM::SelectId::PWMA, 0.);
 
    pwm0p->setDutyCycle(RAM::Target::PWM::SelectId::PWMB, 0.);
 
    pwm1p->setDutyCycle(RAM::Target::PWM::SelectId::PWMA, 0.);
 
    pwm1p->setDutyCycle(RAM::Target::PWM::SelectId::PWMB, 0.);
}

void algorithm()
{
    static unsigned counter = 0;
    
    if ( counter == 800 )
    {
        pwm0p->setDutyCycle(RAM::Target::PWM::SelectId::PWMA, 0.2);
         
        pwm0p->setDutyCycle(RAM::Target::PWM::SelectId::PWMB, 0.2);
         
        pwm1p->setDutyCycle(RAM::Target::PWM::SelectId::PWMA, 0.2);
         
        pwm1p->setDutyCycle(RAM::Target::PWM::SelectId::PWMB, 0.2);

        println("done");
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
