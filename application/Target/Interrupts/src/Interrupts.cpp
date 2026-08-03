/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
 
//
// System stuff
//
 
//
// Local stuff
//
#include "Interrupts.h"

#ifdef TARGET_nano328p
#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
#define SYSTEM_IRQN 16
#define USER_IRQN 139
static __attribute__((section("vtable")))
void (*vectorTable[SYSTEM_IRQN + USER_IRQN])(void) __attribute__((aligned(1024)));
#else
#error "unknown target" 
#endif

using namespace RAM::Target;

//-------------------------------------------------------------------------------
#ifdef TARGET_nano328p
int Interrupts::setHandler(const int irqn, void (*handler)())
{
    return 1;
}
#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
int Interrupts::setHandler(const IRQn_Type irqn, void (*handler)())
{
    if ( ((uint32_t)vectorTable & 0x000003ff) )
    {
        return 1;
    }

    //
    // if not yet relocated then relocate now
    //
    if ( SCB->VTOR != reinterpret_cast<uintptr_t>(vectorTable) )
    {
        //
        // copy current vector table content to new address
        //
        
        for ( unsigned i = 0; i < SYSTEM_IRQN + USER_IRQN; ++i )
        {
            vectorTable[i] = reinterpret_cast<void (*)()>(SCB->VTOR)[i];
        }

        //
        // swap pointer to vector table
        //
        SCB->VTOR = reinterpret_cast<uintptr_t>(vectorTable);
    }

    //
    // store pointer to handler function in the table
    //
    vectorTable[irqn + 16] = handler;

    return 0;
}

#else
#error "unknown target" 
#endif


/*___oOo___*/
