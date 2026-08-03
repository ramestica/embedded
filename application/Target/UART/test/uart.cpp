/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Target.h>
#include <Interrupts.h>
#include <UART.h>
#include <Main.h>

//
// System stuff
//
#include <stdlib.h>

//
// Local stuff
//
volatile char cg = '0';

/** A not templated base class that makes possible keep a static
 ** variable without the need for template parameters.
 */
class Base
{
public:
    static bool m_initialized;
    static volatile void * volatile m_instances_p[int(RAM::Target::UART::Module::UARTIDXN)];
};

template <RAM::Target::UART::Module UARTN>
class UART: Base
{
public:
    UART():
        m_uart(UARTN)
    {
        RAM::Target::Interrupts::setHandler(UART0_IRQn, isr);
    
        if ( RAM::Target::UART::configure(UARTN, 115200) )
        {
            return;
        }

        m_instances_p[int(UARTN)] = static_cast<void *>(this);
    }

    void write(const char c)
    {
        RAM::Target::UART::write(m_uart, c);
    }
    
    int write(char *ptr)
    {
        return RAM::Target::UART::write(m_uart, ptr);
    }
    
    RAM::Target::UART::Module m_uart;

    UART *m_obj;
    
    static void isr()
    {
        volatile UART * volatile obj =
            static_cast<volatile UART<UARTN> * volatile>(m_instances_p[int(UARTN)]);

        //
        // clear all asserted interrupts
        //
        UART0->ICR = UART0->MIS;
        
        cg = RAM::Target::UART::read(obj->m_uart);
    }
};

bool Base::m_initialized = false;
volatile void * volatile Base::m_instances_p[int(RAM::Target::UART::Module::UARTIDXN)];

UART<RAM::Target::UART::Module::UARTIDX0> uart;

void RAM::Target::Main::setup()
{
    // Configure LED pins
    SYSCTL->RCGCGPIO |= (1<<5); // enable clock on PortF
    GPIOF->DIR = (1<<1)|(1<<2)|(1<<3);  // make LED pins (PF1, PF2, and PF3) outputs
    GPIOF->DEN = (1<<1)|(1<<2)|(1<<3); // enable digital function on LED pins
    GPIOF->DATA &= ~((1<<1)|(1<<2)|(1<<3)); // turn off leds
}

void RAM::Target::Main::loop()
{
    uart.write("Type something and press enter: ");
    //char c = RAM::Target::UART::read(uart);
    char c;
    while ( (c = cg) == '0' );
    cg = '0';
    uart.write("\n\r");
    uart.write(c);
    uart.write("\n\r");
    switch ( c )
    {
    case 'r':
        GPIOF->DATA = (1<<1);
        break;
    case 'b':
        GPIOF->DATA = (1<<2);
        break;
    case 'g':
        GPIOF->DATA = (1<<3);
        break;
    }
}

/*___oOo___*/
