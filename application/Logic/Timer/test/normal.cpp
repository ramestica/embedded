//
// RAM stuff
//
#include <GPIO.h>
#include <SysTick.h>

//
// Local stuff
//
#include "Timer.h"

using namespace std;

RAM::Library::Timer::Periodic::CTC *tp;

volatile unsigned long cnt, cntp = 0;
#ifdef TARGET_nano328p
RAM::Target::GPIO::Pin pin = {RAM::Target::GPIO::PORTx::F_IDX, 0x8};
#elif defined(TARGET_tivac) || defined(TARGET_minim4)
RAM::Target::GPIO::Pin pin = {0, RAM::Target::GPIO::PORTx::F_IDX, 0x8};
#else
#error "unknown target" 
#endif

void setup()
{
    if ( SET_ERROR_JUMP_MARK() == 0 )
    {
        Serial.begin(115200);

        Serial.println("starting");

        Serial.println(F_CPU);

        RAM::Target::SysTick::start(
            RAM::Library::Timer::ModuleId::TIMIDX0,
            true,
            16000,
            1,
            NULL);
            
        Serial.println("SysTick started");

        tp = new RAM::Library::Timer::Periodic::CTC(
            RAM::Library::Timer::ModuleId::TIMIDX2,
            RAM::Library::Timer::SelectId::TIMERA,
            1000);

        tp->print();

        RAM::Target::GPIO::enable(pin);

        RAM::Target::GPIO::setDirection(pin, RAM::Target::GPIO::DirectionMode::OUTPUT_PIN);
        
        RAM::Target::GPIO::StatusLED::enable();

        RAM::Target::GPIO::StatusLED::on();
    }
    else
    {
        Serial.println("error");

        Serial.println(RAM::Library::Error::longjmpCode, HEX);
    }
}

void loop()
{
    if ( tp->acknowledge() )
    {
        RAM::Target::GPIO::Digital::toggle(pin);
    }
    else
    {
        RAM::Target::GPIO::Digital::toggle(pin);
    }
}

/*___oOo___*/
