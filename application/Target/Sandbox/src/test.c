/** Would I ever have a declaration statement? 
 */

#ifdef __cplusplus
#error this compilation unit is meant for c
#endif

//
// System stuff
//
#include <avr/io.h>
#include <avr/interrupt.h>
#define __DELAY_BACKWARD_COMPATIBLE__ 
#include <util/delay.h>

//
// Target stuff
//
 
//
// Local stuff
//

/* Delay for the given number of microseconds.  Assumes a 8 or 16 MHz clock. */
void delayus(uint32_t us)
{
#if F_CPU != 16000000L
#error "unsupported cpu frequency"
#endif

	while ( us >= 1000 )
	{
		_delay_ms(1);

		us -= 1000;
	}

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands.
	us -= 2;

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
}

inline void dl(double us)
{
    for ( unsigned int i = 0; i < us; ++i )
    {
        _delay_us(1);
    }
}

inline void isr()
{
    static int state = 1;

    if ( state == 1 )
    {
        PORTB &= ~(0b100000);
    }
    else
    {
        PORTB |= (0b100000);
    }

    state = !state;

    //_delay_us(1);
    //dl(20);
    //delayus(0);
}

//ISR(TIMER0_COMPA_vect)
//{
//    isr();
//}
 
//-----------------------------------------------------------------------------------
int main()
{
    sei();

    //
    // led pin is output pin
    //
    DDRB |= (0x1 << 5);

    //
    // turn led on
    //
    PORTB |= (0x1 << 5);

    while ( 1 )
    {
        isr();
    }

    return 0;
}
 
/*___oOo___*/
