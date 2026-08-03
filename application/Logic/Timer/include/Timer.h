#ifndef _Timer_H_
#define _Timer_H_

//
// System stuff
//
#include <inttypes.h>

//
// Target stuff
//
#include <Target.h>
#include <TargetTimer.h>
#include <SystemTick.h>

//
// RAM stuff
// 
#include <ErrorCodes.h>
#include <SerialCommands.h>

namespace RAM
{
    namespace Library
    {
        namespace Timer
        {
            /** Avoid the user to refer to Boards' specific types.
             */
            typedef RAM::Target::Timer::ModuleId ModuleId;
            typedef RAM::Target::Timer::SelectId SelectId;

            /** Modes of operation.
             */
            enum class OperationMode
            {
                Normal,
                    CTC,
                    FastPWM,
                    PhaseCorrectPWM,
                    PhaseFreqCorrectPWM
                    };

            struct Module
            {
                Module():
                    m_tid(ModuleId::TIMID0),
                    m_sid(SelectId::TIMERA),
                    m_periodus(0),
                    m_prescaler(0)
                {}

                Module(
                    const ModuleId tid,
                    const uint32_t periodus);

                void configure(
                    const SelectId sid,
                    const RAM::Target::Timer::Mode mode,
                    const RAM::Target::Timer::CompareOutputMode compareOutputMode);

                //
                // start timer by setting prescaler
                //
                void start();

                virtual void setDutyCycle(const SelectId sid, const float &dc) {;}

                void print();

                /** Keep track of timer modules in use.
                 */
                static Module *m_modulesInUse[int(ModuleId::TIMIDN)];

                /** Timer module.
                 */
                Module m_module;

                /** Timer within module (normally A or B or BOTH). Once started
                 ** this setting remains fixed and further invocations to other
                 ** method will check that the requested timer overlaps.
                 */
                SelectId m_sid;

                /** Timer period in microseconds.
                 */
                uint32_t m_periodus;

                /** Nanoseconds per count.
                 */
                float m_resolutionns;

                /** Prescaler value without rounding.
                 */
                float m_idealPresecaler;

                /** Prescaler use to clock the timer's control logic.
                 */
                unsigned int m_prescaler;

                /** A counter value used as a start up count for up/down
                 ** counting in normal mode or a timeout value for
                 ** clear-timer-on-compare (CTC) mode.
                 */
                RAM::Target::Timer::LoadType m_load;
                    
                /** Timer's bits length.
                 */
                unsigned int m_nbits;
            };

            namespace PWM
            {
                struct Encoder: public Module
                {
                    struct SerialCommandId
                    {
                        enum SerialCommandIdEnum
                        {
                            SETDUTY = 0
                        };
                    };

                    struct SerialReplyId
                    {
                        enum SerialReplyIdEnum
                        {
                            UNINITIALIZED_MODULE = RAM::Library::SerialCommands::ReplyId::OFFSET
                        };
                    };

                    /** A PWM mode defines what happens on counting up
                     ** compare-match events. Note that by linking the mode to
                     ** the counting up matches then the enumeration can be
                     ** applied to single slope timers as well (only assumption
                     ** is that single slope implies up counting.
                     */
                    enum class Mode
                    {
                        Toggle,
                        Set,
                        Clear
                    };

                    /** Duty cycle duration in micro-seconds at 0% and 100%.
                     */
                    uint32_t m_dc0us;
                    uint32_t m_dc100us;

                    /** Output compare register has a range of [0,max], where
                     ** max is dependent on the actual timer and mode. The
                     ** following variable records max for its repetitive usage
                     ** every time a new duty cycle is to be set.
                     */
                    RAM::Target::Timer::LoadType m_outputCompareMax;

                    Encoder() {;}
                    
                    /** Encode [0,1] input parameter into a PWM signal. The PWM
                     ** frequency (1/periodus) must match one of the 'natural'
                     ** frequencies of the timer, that is, the one derived from
                     ** prescaler and maximum counting value. That is to, this
                     ** implementation does not support custom TOP through OCRnA
                     ** to instead concentrate on always making available two
                     ** PWM signals per timer. It happens also that those two
                     ** PWM signal will necessarily have the same frequency.
                     ** @param dc0us pulse length ON when 0% duty cycle. 
                     ** @param dc100us pulse length ON when 100% duty cycle. 
                     */
                    Encoder(
                        const ModuleId tid,
                        const SelectId sid,
                        const uint32_t periodus,
                        const uint32_t dc0us,
                        const uint32_t dc100us,
                        const float initialDutyCycle);
                    
                    void setDutyCycle(const SelectId sid, const float &dc);

                    struct SetDutyCycleCmd:
                        public RAM::Library::SerialCommands::Command
                    {
                        SetDutyCycleCmd():
                            Command(
                                RAM::Library::SerialCommands::ReceiverId::PWMENC,
                                SerialCommandId::SETDUTY)
                        {;}

                        void execute();
                    };
                };
            }

            namespace Periodic
            {
                struct Normal: public Module 
                {
                    Normal() {;}
                    
                    Normal(
                        const ModuleId tid,
                        const SelectId sid,
                        const uint32_t periodus);
                    
                    bool acknowledge() const;
                };

                struct CTC: public Module 
                {
                    CTC() {;}
                    
                    CTC(
                        const ModuleId tid,
                        const SelectId sid,
                        const uint32_t periodus);
                    
                    bool acknowledge() const;
                };
            }
        }
    }
}

#endif /* _Timer_H_ */

/*___oOo__*/
