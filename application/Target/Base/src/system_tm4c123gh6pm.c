/** Would I ever have a declaration statement? 
 */

//
// System stuff
//
#include <stdbool.h>
#include <stdint.h>

//
// TivaWare stuff
//
#include <driverlib/sysctl.h>

#include <Target/tm4c123gh6pm.h>

uint32_t SystemCoreClock = 0;
uint32_t SystemCoreClockCyclesPerMicrosecond;

void SystemInit (void)
{
    //
    // even though this is the default, enable lazy stacking feature
    // (skipping the stacking of floating-point registers)
    //
    FPU->FPCCR |= 0x80000000 | 0x40000000;

    SCB->CPACR |= ((3UL << 10*2) |                /* set CP10 Full Access */
                   (3UL << 11*2) );               /* set CP11 Full Access */

    //
    // use TivaWare to configure board's clock
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    //
    // use TivaWare function to compute the clock frequency
    // RCC =0x14E1540 
    // RCC2=0xC1004000
    //
    SystemCoreClock = SysCtlClockGet();
    SystemCoreClockCyclesPerMicrosecond = SystemCoreClock / 1000000;
}

/*___oOo___*/
