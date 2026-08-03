#ifndef _Interrupts_H_
#define _Interrupts_H_

//
// System stuff
//
#include <inttypes.h>
#ifdef TARGET_nano328p
#include <avr/interrupt.h>
#endif

//
// Target stuff
//
#include <target/Target.h>

//
// RAM stuff
// 

//
// Local stuff
//

namespace RAM
{
    namespace Target
    {
        namespace Interrupts
        {
            /** Write handler address in vector table. This function causes the
             ** vector table to be relocated to RAM if not there already.
             */
#ifdef TARGET_nano328p
            int setHandler(const int irq, void (*handler)());            
#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
            int setHandler(const IRQn_Type irq, void (*handler)());            
#else
#error "unknown target" 
#endif

            /** Enable interrupt request line.
             */
#ifdef TARGET_nano328p
            int enableIRQ(const int irqn);            
#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
            inline void enableIRQ(const IRQn_Type irqn)
            {
                //
                // systick is running at the highest programmable priority (0),
                // switch any one else enabled by the application to run at
                // lower priority.
                //
                NVIC_SetPriority(irqn, 1);

                NVIC_EnableIRQ(irqn);
            }
            inline void disableIRQ(const IRQn_Type irqn)
            {
                NVIC_DisableIRQ(irqn);
            }
#else
#error "unknown target" 
#endif
            
            /** Enable global interrupts.
             */
            inline void enable(const RegType status)
            {
#ifdef TARGET_nano328p
                asm volatile("" : : : "memory");

                SREG = status;

#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
#endif         
            }

            /** Disable global interrupts.
             */
            inline RegType disable()
            {
#ifdef TARGET_nano328p
                RegisterType status = SREG;

                cli();

                asm volatile("" : : : "memory");

                return status;

#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
                return 0;
#endif         
            }
        }
    }
}

#endif /* _Interrupts_H_ */

/*___oOo__*/
