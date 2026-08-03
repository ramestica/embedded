//
// RAM stuff
//
#include <Timer.h>
#include <MainLoop.h>

using namespace std;

RAM::Library::Timer::PWM::Encoder *pwm[4];
RAM::Library::Timer::ModuleId mids[4] = {
    RAM::Library::Timer::ModuleId::TIMIDX6,
    RAM::Library::Timer::ModuleId::TIMIDX7,
    RAM::Library::Timer::ModuleId::TIMIDX8,
    RAM::Library::Timer::ModuleId::TIMIDX9};

void appSetup()
{
    //
    // avr 8 bits timer available periods [us]:
    // 16, 128, 512, 1024, 2048, 4096, 16384
    //
    // pwm1 = new RAM::Library::Timer::PWM::Encoder(
    //     RAM::Library::Timer::ModuleId::TIMIDX1,
    //     RAM::Library::Timer::SelectId::TIMERBOTH,
    //     4096,
    //     1000,
    //     2000,
    //     float(0.));

    for ( unsigned i = 0; i < 4; ++i )
    {
        pwm[i] = new RAM::Library::Timer::PWM::Encoder(
            mids[i],
            RAM::Library::Timer::SelectId::TIMERA,
            3000,
            1000,
            2000,
            float(0));
    }
 
    RAM::Library::SerialCommands::CommandList::instance().
        setCommandHandler(
            new RAM::Library::Timer::PWM::Encoder::SetDutyCycleCmd());

//    pwmp->print();
// 
//    pwmp->setDutyCycle(RAM::Library::Timer::SelectId::TIMERA, 0.25);
// 
//    pwmp->setDutyCycle(RAM::Library::Timer::SelectId::TIMERB, 0.75);
}

void setup()
{
    RAM::Library::MainLoop::Executive::instance().setup(
        appSetup,
        NULL,
        RAM::Library::Timer::ModuleId::TIMIDX1,
        16000,
        1);
}

void loop()     
{
    RAM::Library::MainLoop::Executive::instance().execute();
}

/*___oOo___*/
