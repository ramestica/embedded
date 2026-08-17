#ifndef _TargetTimer_H_
#define _TargetTimer_H_

//
// System stuff
//
#include <inttypes.h>

//
// Target stuff
//
#include <Target/GPIO.h>

namespace RAM
{
    namespace Target
    {
        namespace Timer
        {
            struct Module
            {
                ModuleId m_id;
                intptr_t m_pointer;
                unsigned m_index = static_cast<unsigned>(ModuleId::IDN);
                IRQn_Type m_irqn[2];
                bool m_isSplit;
                unsigned m_bits;
                RAM::Target::GPIO::Pin m_pins;
            };

            /** Modes of operation.
             */
            enum class Mode: unsigned 
            {
                Periodic,     // or CTC (clear timer on compare match)
                InputEdgeTime,
                PWM
            };
            
            /** Type of output on compare match.
             */
            enum class CompareOutputMode: unsigned
            {
                DISCONECT_OUTPUT,
                TOGGLE_OUTPUT,
                NON_INVERTING_OUTPUT,
                INVERTING_OUTPUT
            };

            /** Load value type.
             */
#ifdef TARGET_nano328p
            typedef uint16_t LoadType;
#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
            typedef uint64_t LoadType;
#else
#error "unknown target" 
#endif

            unsigned int getNumberBits(const ModuleId tid);

            /** Ideal prescaler for a given period. Ideal means to have the
             ** exact floating point value that would clock at a frequency
             ** exactly to the requested period. After this macro is actually
             ** used in the code then 'ceil' would be used to compute the actual
             ** prescale value that the mcu can understand.
             */
            float getExactPrescaler(const uint32_t periodus, const unsigned nbits);
                
            /** Adapt a given prescaler value to closest (up) board's
             ** supported prescaler.
             */
            int adaptPrescaler(
                const ModuleId tid,
                const uint32_t periodus,
                float &ideal,
                unsigned int &out);

            int configure(
                const ModuleId tid,
                const Mode mode,
                const bool isSplit,
                Module &module);

            /** Start timer. In capture mode the timer is expected to
             ** timeout earlier than given period value. That is, to
             ** discern for how long the timer ran before the input
             ** event, then, the count should not reset before the
             ** maximum expected time between events.
             */
            int start(
                const Module &module,
                const SelectId sid,
                const unsigned periodus,
                void (*isr)());

            /** Clear all pending interrupts and returns the status of some of
             ** them.
             ** @param rx receive FIFO passing through level.
             ** @param rxto receive timeout.
             ** @param tx transmit FIFO passing through level.
             */
            inline void iclear(
                const Module &module,
                const SelectId sid,
                volatile RAM::Target::RegType &mis)
            {
                //
                // reads masked interrupt status
                //
                mis = reinterpret_cast<TIMER0_Type *>(module.m_pointer)->MIS;
                
                //
                // clear all A or B asserted interrupts
                //
                if ( sid == SelectId::TIMERA )
                {
                    reinterpret_cast<TIMER0_Type *>(module.m_pointer)->ICR =
                        (mis & 0x17);
                }
                else
                {
                    reinterpret_cast<TIMER0_Type *>(module.m_pointer)->ICR =
                        (mis & 0xf00);
                }
            }
            
            int getLoad(const Module &module, uint32_t &load);

            int getLoad(const Module &module, const SelectId sid, uint32_t &load);

            /** Inverting or not inverting logic. Positive logic means to
             ** set when start counting and to clear after a compare match.
             */
            int setCompareOutputMode(                    
                const ModuleId tid,
                const SelectId sid,
                const CompareOutputMode mode);

            int setOutputCompareValue(
                const ModuleId tid,
                const SelectId sid,
                const LoadType &dc);

            int setOutputCompareFromCounter(
                const ModuleId tid,
                const SelectId sid,
                LoadType &cnt);

            int interruptControl(
                const ModuleId tid,
                const SelectId sid,
                void (*isr)());

            /** Immediately trigger a timer's interrupt. 
             */
            void interruptTrigger(
                const ModuleId tid,
                const SelectId sid);

            /** Set prescaler code. Note that setting the prescaler
             ** automatically starts the timer.
             */
            int setPrescaler(
                const ModuleId tid,
                const SelectId sid,
                const uint32_t prescaler);

            LoadType getValue(
                const Module &module,
                const SelectId sid);

            LoadType getValueSnapshot(
                const Module &module,
                const SelectId sid);

            int setDutyCycle(
                const ModuleId tid,
                const SelectId sid,
                const float &dc);

            int acknowledgeOverflow(
                const ModuleId tid,
                const SelectId sid,
                const uint64_t value,
                bool &acknowledged);

            int acknowledgeCompareMatchFlag(
                const ModuleId tid,
                const SelectId sid,
                bool &acknowledged);

            void print(const Module &module);
        }
    }
}

#endif /* _BoardsTimer_H_ */

/*___oOo__*/
