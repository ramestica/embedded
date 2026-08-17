/** Would I ever have a declaration statement? 
 */

//
// System stuff
//
#include <stdbool.h>

//
// Local stuff
//
#include <Target/tm4c123gh6pm.h>

//
// application's entry point
//
extern int main(void);

//
// variables created by the linker script
//
extern unsigned long _ram_origin;
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;
extern unsigned long _bss;
extern unsigned long _ebss;
extern unsigned long _estack;
extern void (*__preinit_array_start[])(void);
extern void (*__preinit_array_end[])(void);
extern void (*__init_array_start[])(void);
extern void (*__init_array_end[])(void);

//
// function executed when the mcu resets:
//
//     1. copy data from flash to ram
//     2. zero bss
//     3. enable fpu unit (how to make it an option?)
//     4. execute static constructors
//     5. invoke 'main'
//
void ResetISR(void)
{
    unsigned i, cnt;

    //
    // Copy the data segment initializers from flash to SRAM. If text
    // was loaded into ram already then nothing to copy. That is, if
    // data starts at ram beginning then text is in flash and copying
    // initialized data is required.
    //
    if ( &_ram_origin == &_data )
    {
        for ( uint32_t *src = &_etext, *dest = &_data; dest < &_edata; ++src, ++dest )
        {
            *dest = *src;
        }
    }

    //
    // Zero fill the bss segment.
    //
    for ( uint32_t *i = &_bss; i < &_ebss; ++i )
    {
        *i = 0;
    }

    //
    // initialize board using our own function (see system_<processor>.c)
    //
    SystemInit();
    
    //
    // execute static constructors
    //
    // http://stackoverflow.com/questions/6343348/global-constructor-call-not-in-init-array-section
    //
    // It seems that __libc_init_array would do the same job as the two
    // for-loops below, see comment here:
    //
    // http://stackoverflow.com/questions/15265295/understanding-the-libc-init-array
    //
    cnt = __preinit_array_end - __preinit_array_start;
    for ( i = 0; i < cnt; i++ )
    {
        __preinit_array_start[i]();
    }
    cnt = __init_array_end - __init_array_start;
    for ( i = 0; i < cnt; i++ )
    {
        __init_array_start[i]();
    }

    //
    // call application's entry point
    //
    main();
}

static void defaultHandler(void)
 {
    while (1)
    {
        //
        // what can actually happen here?
        //
    }
}

//
// vector table
//
__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (void *)&_estack,        // The initial stack pointer, 0x20008000 32K
    ResetISR,                // The reset handler
    defaultHandler,       // The NMI handler
    defaultHandler,       // The hard fault handler
    defaultHandler,       // The MPU fault handler
    defaultHandler,       // The bus fault handler
    defaultHandler,       // The usage fault handler
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    defaultHandler,       // SVCall handler
    defaultHandler,       // Debug monitor handler
    0,                       // Reserved
    defaultHandler,       // The PendSV handler
    defaultHandler,       // The SysTick handler
    defaultHandler,       // GPIO Port A
    defaultHandler,       // GPIO Port B
    defaultHandler,       // GPIO Port C
    defaultHandler,       // GPIO Port D
    defaultHandler,       // GPIO Port E
    defaultHandler,       // UART0 Rx and Tx
    defaultHandler,       // UART1 Rx and Tx
    defaultHandler,       // SSI0 Rx and Tx
    defaultHandler,       // I2C0 Master and Slave
    defaultHandler,       // PWM Fault
    defaultHandler,       // PWM Generator 0
    defaultHandler,       // PWM Generator 1
    defaultHandler,       // PWM Generator 2
    defaultHandler,       // Quadrature Encoder 0
    defaultHandler,       // ADC Sequence 0
    defaultHandler,       // ADC Sequence 1
    defaultHandler,       // ADC Sequence 2
    defaultHandler,       // ADC Sequence 3
    defaultHandler,       // Watchdog timer
    defaultHandler,       // Timer 0 subtimer A
    defaultHandler,       // Timer 0 subtimer B
    defaultHandler,       // Timer 1 subtimer A
    defaultHandler,       // Timer 1 subtimer B
    defaultHandler,       // Timer 2 subtimer A
    defaultHandler,       // Timer 2 subtimer B
    defaultHandler,       // Analog Comparator 0
    defaultHandler,       // Analog Comparator 1
    defaultHandler,       // Analog Comparator 2
    defaultHandler,       // System Control (PLL, OSC, BO)
    defaultHandler,       // FLASH Control
    defaultHandler,       // GPIO Port F
    defaultHandler,       // GPIO Port G
    defaultHandler,       // GPIO Port H
    defaultHandler,       // UART2 Rx and Tx
    defaultHandler,       // SSI1 Rx and Tx
    defaultHandler,       // Timer 3 subtimer A
    defaultHandler,       // Timer 3 subtimer B
    defaultHandler,       // I2C1 Master and Slave
    defaultHandler,       // Quadrature Encoder 1
    defaultHandler,       // CAN0
    defaultHandler,       // CAN1
    defaultHandler,       // CAN2
    defaultHandler,       // Ethernet
    defaultHandler,       // Hibernate
    defaultHandler,       // USB0
    defaultHandler,       // PWM Generator 3
    defaultHandler,       // uDMA Software Transfer
    defaultHandler,       // uDMA Error
    defaultHandler,       // ADC1 Sequence 0
    defaultHandler,       // ADC1 Sequence 1
    defaultHandler,       // ADC1 Sequence 2
    defaultHandler,       // ADC1 Sequence 3
    defaultHandler,       // I2S0
    defaultHandler,       // External Bus Interface 0
    defaultHandler,       // GPIO Port J
    defaultHandler,       // GPIO Port K
    defaultHandler,       // GPIO Port L
    defaultHandler,       // SSI2 Rx and Tx
    defaultHandler,       // SSI3 Rx and Tx
    defaultHandler,       // UART3 Rx and Tx
    defaultHandler,       // UART4 Rx and Tx
    defaultHandler,       // UART5 Rx and Tx
    defaultHandler,       // UART6 Rx and Tx
    defaultHandler,       // UART7 Rx and Tx
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    defaultHandler,       // I2C2 Master and Slave
    defaultHandler,       // I2C3 Master and Slave
    defaultHandler,       // Timer 4 subtimer A
    defaultHandler,       // Timer 4 subtimer B
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    0,                       // Reserved
    defaultHandler,       // Timer 5 subtimer A
    defaultHandler,       // Timer 5 subtimer B
    defaultHandler,       // Wide Timer 0 subtimer A
    defaultHandler,       // Wide Timer 0 subtimer B
    defaultHandler,       // Wide Timer 1 subtimer A
    defaultHandler,       // Wide Timer 1 subtimer B
    defaultHandler,       // Wide Timer 2 subtimer A
    defaultHandler,       // Wide Timer 2 subtimer B
    defaultHandler,       // Wide Timer 3 subtimer A
    defaultHandler,       // Wide Timer 3 subtimer B
    defaultHandler,       // Wide Timer 4 subtimer A
    defaultHandler,       // Wide Timer 4 subtimer B
    defaultHandler,       // Wide Timer 5 subtimer A
    defaultHandler,       // Wide Timer 5 subtimer B
    defaultHandler,       // FPU
    defaultHandler,       // PECI 0
    defaultHandler,       // LPC 0
    defaultHandler,       // I2C4 Master and Slave
    defaultHandler,       // I2C5 Master and Slave
    defaultHandler,       // GPIO Port M
    defaultHandler,       // GPIO Port N
    defaultHandler,       // Quadrature Encoder 2
    defaultHandler,       // Fan 0
    0,                       // Reserved
    defaultHandler,       // GPIO Port P (Summary or P0)
    defaultHandler,       // GPIO Port P1
    defaultHandler,       // GPIO Port P2
    defaultHandler,       // GPIO Port P3
    defaultHandler,       // GPIO Port P4
    defaultHandler,       // GPIO Port P5
    defaultHandler,       // GPIO Port P6
    defaultHandler,       // GPIO Port P7
    defaultHandler,       // GPIO Port Q (Summary or Q0)
    defaultHandler,       // GPIO Port Q1
    defaultHandler,       // GPIO Port Q2
    defaultHandler,       // GPIO Port Q3
    defaultHandler,       // GPIO Port Q4
    defaultHandler,       // GPIO Port Q5
    defaultHandler,       // GPIO Port Q6
    defaultHandler,       // GPIO Port Q7
    defaultHandler,       // GPIO Port R
    defaultHandler,       // GPIO Port S
    defaultHandler,       // PWM 1 Generator 0
    defaultHandler,       // PWM 1 Generator 1
    defaultHandler,       // PWM 1 Generator 2
    defaultHandler,       // PWM 1 Generator 3
    defaultHandler        // PWM 1 Fault
};

/*___oOo___*/
