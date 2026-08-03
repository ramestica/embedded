/** Would I ever have a declaration statement? 
 */
 
//
// System stuff
//

//
// Target stuff
//
#ifdef TARGET_nano328p
#include <avr/interrupt.h>
#include <util/atomic.h> 
#elif defined TARGET_tm4c123gxl
#endif
#include <Interrupts.h>

//
// Local stuff
//
#include "Main.h"

//-------------------------------------------------------------------------------
void RAM::Target::Main::init()
{
#ifdef TARGET_nano328p
    //
    // enable interrupts
    //
    sei();

    //
    // timer zero used by arduino delay and micros/millis
    //
    TCCR0A |= (0x1 << WGM01) | (0x1 << WGM00);
    TCCR0B |= (0x1 << CS01) | (0x1 << CS00); 

#elif defined TARGET_tm4c123gxl
    //
    // relocate vector table
    //
    
    //
    // enable interrupts
    //
    __enable_irq();

    //
    // use Advanced High-Performance Bus (AHB) bus for GPIO
    //
//    SYSCTL->GPIOHBCTL |= 0b111111;
    
    //
    // Unlock and commit NMI pins PD7 and PF0
    // (See 5.2.3.1 NMI Pin, page 218)
    //
    //GPIOF->LOCK = 0x4C4F434B;
    //GPIOF->CR = 0x1;
    //GPIOD->LOCK = 0x4C4F434B;
    //GPIOD->CR= 0x80;
#endif
}

/*___oOo___*/
