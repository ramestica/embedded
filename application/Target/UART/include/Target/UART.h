#ifndef _TargetUART_H_
#define _TargetUART_H_

//
// System stuff
//

//
// Target stuff
//
#include <Target/Target.h>
#include <Target/GPIO.h>

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
        namespace UART
        {
            struct Module
            {
                ModuleId m_id;
                intptr_t m_pointer;
                unsigned m_index = static_cast<unsigned>(ModuleId::IDN);
                IRQn_Type m_irqn;
                RAM::Target::GPIO::Pin m_pins;
            };
            
            /** Configure UART module with default parameters and given baud
             ** rate.
             */
            int configure(
                const ModuleId moduleId,
                Module &module,
                const unsigned baud,
                void (*isr)());

            inline void flush()
            {
            }
            
            /** Clear all pending interrupts and returns the status of some of
             ** them.
             ** @param rx receive FIFO passing through level.
             ** @param rxto receive timeout.
             ** @param tx transmit FIFO passing through level.
             */
            inline void iclear(const Module &module, bool &rx, bool &rxto, bool &tx)
            {
                //
                // reads masked interrupt status
                //
                RAM::Target::RegType mis =
                    reinterpret_cast<UART0_Type *>(module.m_pointer)->MIS;
                
                //
                // clear all asserted interrupts
                //
                reinterpret_cast<UART0_Type *>(module.m_pointer)->ICR = mis;

                rx = (mis & UART_MIS_RXMIS);

                rxto = (mis & UART_MIS_RTMIS);
                
                tx = (mis & UART_MIS_TXMIS);
            }

            inline char read(const Module &module)
            {
                while ( (reinterpret_cast<UART0_Type *>(module.m_pointer)->FR
                         &
                         UART_FR_RXFE) ); 

                return reinterpret_cast<UART0_Type *>(module.m_pointer)->DR;
            }

            /** Read many data as available in the fifo, but never more than the
             ** size of the fifo. The idea of the limit is to enforce a maximum
             ** blocking time. The assumption is also that while actually
             ** reading the fifo new inserts to it could be happening
             ** concurrently.
             */
            inline int read(const Module &module, char *p, const unsigned max)
            {
                unsigned i = 0;

                //
                // while not max already read and fifo not empty the keep reading
                //
                while ( i != max
                        &&
                        !(reinterpret_cast<UART0_Type *>(module.m_pointer)->FR
                          &
                          UART_FR_RXFE) )
                {
                    p[i++] = reinterpret_cast<UART0_Type *>(module.m_pointer)->DR;
                }
                
                return i;
            }

            inline void write(const Module &module, const char c)
            {
                while( (reinterpret_cast<UART0_Type *>(module.m_pointer)->FR
                        &
                        (1 << 5)) );
                
                reinterpret_cast<UART0_Type *>(module.m_pointer)->DR = c;
            }

            inline void write(
                const Module &module,
                const char *c,
                const unsigned count,
                const bool disableInterrupts = false)
            {
                if ( disableInterrupts )
                {
                    Interrupts::disableIRQ(module.m_irqn);
                }
                
                for ( unsigned i = 0; i < count; ++i )
                {
                    reinterpret_cast<UART0_Type *>(module.m_pointer)->DR = c[i];
                }

                if ( disableInterrupts )
                {
                    Interrupts::enableIRQ(module.m_irqn);
                }
            }
            
            inline int write(const Module &module, char *str)
            {
                while ( *str )
                {
                    write(module, *str++);
                }
    
                return 0;
            }

            inline bool isTxFifoEmpty(const Module &module)
            {
                return
                    (reinterpret_cast<UART0_Type *>(module.m_pointer)->FR & UART_FR_TXFE) == UART_FR_TXFE ? true : false;
            }
        }
    }
}

#endif /* _TargetUART_H_ */

/*___oOo__*/
