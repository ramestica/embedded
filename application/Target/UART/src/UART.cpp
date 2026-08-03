/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Interrupts.h>

//
// System stuff
//
 
//
// Local stuff
//
#include "TargetUART.h"

using namespace RAM::Target;

static UART0_Type *uartp[static_cast<int>(UART::ModuleId::IDN)] =
{
    UART0,
    UART1,
    UART2,
    UART3,
    UART4,
    UART5,
    UART6,
    UART7
};

static IRQn_Type irqNumber[static_cast<int>(UART::ModuleId::IDN)] =
{
    UART0_IRQn, 
    UART1_IRQn, 
    UART2_IRQn, 
    UART3_IRQn, 
    UART4_IRQn, 
    UART5_IRQn, 
    UART6_IRQn, 
    UART7_IRQn
};

//-------------------------------------------------------------------------------
int UART::configure(
    const UART::ModuleId moduleId,
    UART::Module &module,
    const unsigned baud,
    void (*isr)())
{
    if ( moduleId == ModuleId::IDN )
    {
        return 1;
    }

    module.m_id = moduleId;
    
    module.m_index = unsigned(moduleId);
    
    module.m_pointer = reinterpret_cast<intptr_t>(uartp[module.m_index]);
    
    module.m_irqn = irqNumber[module.m_index];
    
    //
    // enable module (RCGCUART register see page 344)
    //
    if ( (SYSCTL->RCGCUART & (0x1 << module.m_index)) == 0 )
    {
        SYSCTL->RCGCUART |= (0x1 << module.m_index); 

        //
        // wait until module ready
        //
        while ( (SYSCTL->PRUART & (0x1 << module.m_index)) == 0 );
    }
    else
    {
        return 1;
    }

    //
    // configure rx and tx signals in associated gpio module
    //
    if ( GPIO::configure(moduleId, module.m_pins) )
    {
        return 1;
    }

    //
    // enable rx and tx signals in associated gpio module
    //
    if ( GPIO::enable(module.m_pins) )
    {
        return 1;
    }

    //
    // a couple of examples to compute the baud rate divisor. The factor 16
    // implies High-Speed (HSE) disabled, which is the default state.
    //
    // BRD = 16,000,000 / (16 * 9600) = 104.16666666666666666666666666666666666666666
    // UARTFBRD[DIVFRAC] = integer(0.166667 * 64 + 0.5) = 11
    //
    // BRD = 80,000,000 / (16 * 9600) = 520.8333333333334
    // UARTFBRD[DIVFRAC] = integer(0.8333333333334 * 64 + 0.5) = 53
    //
    float brd = float(F_CPU) / (16 * baud);
    unsigned ibrd = unsigned(brd);
    unsigned fbrd = unsigned((brd - ibrd) * 64 + 0.5);

    //
    // disable the UART by clearing the UARTEN bit in the UARTCTL register
    // FIXME: why is this step needed?
    //
    reinterpret_cast<UART0_Type *>(module.m_pointer)->CTL &= ~(1<<0);

    //
    // fifo interrupts when tx almost empty (TX FIFO ≤ ⅞ empty) and rx whenever
    // a character is received (RX FIFO ≥ ⅛ full)
    //
    reinterpret_cast<UART0_Type *>(module.m_pointer)->IFLS = 0x0;

    //
    // flush all
    //
    flush();

    //
    // disable all uart interrupts
    //
    reinterpret_cast<UART0_Type *>(module.m_pointer)->IM = 0x0;

    //
    // if irs provided then enable interrupts
    //
    if ( isr != nullptr )
    {
        //
        // enable rx timeout interrupt and rx interrupt triggered by RIS bit 
        //
        reinterpret_cast<UART0_Type *>(module.m_pointer)->IM
            |= (UART_MIS_RXMIS | UART_MIS_RTMIS | UART_MIS_TXMIS);

        //
        // set isr handler pointer
        //
        Interrupts::setHandler(module.m_irqn, isr);

        //
        // enable uart module interrupts
        //
        Interrupts::enableIRQ(module.m_irqn);
    }
    
    //
    // write baud rate params in the followind order:
    //
    //     integer portion of the BRD to UARTIBRD
    //     fractional portion of the BRD to UARTFBRD
    //     line control to UARTLCRH (8-bit, no parity, 1-stop bit, fifos enabled)
    //
    reinterpret_cast<UART0_Type *>(module.m_pointer)->IBRD = ibrd;
    reinterpret_cast<UART0_Type *>(module.m_pointer)->FBRD = fbrd; 
    reinterpret_cast<UART0_Type *>(module.m_pointer)->LCRH = (0x3 << 5) | ( 1 << 4);

    //
    // clocked with system clock
    //
    reinterpret_cast<UART0_Type *>(module.m_pointer)->CC = 0x0;          

    //
    // optionally Micro Direct Memory Access (UARTDMACTL)
    //

    //
    // receive and transmit modes and transmit EOT
    //
    reinterpret_cast<UART0_Type *>(module.m_pointer)->CTL
        = (1 << 4) | (1 << 8) | (1 << 9);

    //
    // enable uart module
    //
    reinterpret_cast<UART0_Type *>(module.m_pointer)->CTL |= 0x1;

    return 0;
}

/*___oOo___*/
