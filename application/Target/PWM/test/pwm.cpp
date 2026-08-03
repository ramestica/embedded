/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <target/Target.h>
#include <target/SysTick.h>
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
#include <target/PWM.h>

using namespace std;
using namespace RAM::Target;
using namespace RAM::Library::Host::Console;

GPIO::Pin interruptPin, togglingPin;
PWM::Generator pwm;
RAM::Library::UART::Module<RAM::Target::UART::ModuleId::UARTID0> uart(115200);

void isr()
{
    unsigned status =
        reinterpret_cast<GPIOA_Type *>(interruptPin.m_pointer)->MIS;

    reinterpret_cast<GPIOA_Type *>(interruptPin.m_pointer)->ICR = status;

    if ( status & interruptPin.m_mask )
    {
        GPIOF_AHB->DATA[0x2] = (0x2);
        GPIOF_AHB->DATA[0x2] = 0x0;

        //GPIO::Digital::toggle(togglingPin);

        //GPIO::Digital::write(togglingPin, 0x1);
        
        //GPIO::Digital::write(togglingPin, 0x0);
    }
}

//-------------------------------------------------------------------------------
void Main::setup()
{    
    unsigned periodus = 1/*2500*/;
    float dc[2] = {0.5, 0.5};

    RAM::Library::Host::setCommunicationStreams(&uart, &uart);

    if ( SysTick::start(
             true,
             8000,
             0,
             nullptr) )
    {
        println("systick failed to start");
        
        return;
    }

    configure(
        GPIO::PortId::GPIOIDF,
        togglingPin,
        0x2,
        GPIO::DirectionMode::OUTPUT_PIN);

    //
    // capture interrupts on ...
    //
    GPIO::configure(
        GPIO::PortId::GPIOIDD,
        interruptPin,
        0x1 << 6,
        GPIO::DirectionMode::INPUT_PIN,
        isr);

    if ( PWM::configure(
             PWM::GeneratorId::PWMGENID7,
             PWM::SelectId::PWMBOTH,
             periodus,
             pwm) )
    {
        println("failed to configure pwm generator");

        return;
    }

    if ( PWM::start(
             pwm,
             dc,
             nullptr) )
    {
        println("failed to start pwm generator A");

        return;
    }

    PWM::setDutyCycle(pwm, PWM::SelectId::PWMA, 0.1);

    PWM::setDutyCycle(pwm, PWM::SelectId::PWMB, 0.9);
    
    println("started");
}

void Main::loop()
{
}

/*___oOo___*/
