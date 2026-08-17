#ifndef _PulseModulation_H_
#define _PulseModulation_H_

//
// System stuff
//
#include <string.h>

//
// Target stuff
//
#include <Target/Timer.h>
#include <Target/PWM.h>
#include <Target/SystemTick.h>

//
// Library stuff
// 
#include <Support/ErrorCodes.h>
#include <Logic/Host.h>

namespace RAM
{
    namespace Logic
    {
        namespace PulseModulation
        {
            namespace PPM
            {
                /** A not templated base class that makes possible keep a static
                 ** variable without the need for template parameters.
                 **
                 **          1  ...  N   gap pulse
                 **          ++ ++   ++  +----+           ++
                 **          || ||   ||  |    |           ||
                 **     ...__||_||___||__|    |___________||_...
                 **      
                 **                ->||<- pulse width
                 **      
                 **          |<-    ppm frame period    ->|
                 */
                class Base
                {
                public:
                    Base(const RAM::Target::Timer::ModuleId timerId,
                         const unsigned nchannels,
                         const unsigned framePeriodus,
                         const unsigned channelMaxWidthus,
                         void (*interruptHandler)());

                    void isr();

                    /** For all timeings to resync. It takes at least
                     ** one full PPM frame to be in sync again, it is
                     ** only when in sync that all readings are
                     ** guarantee to be correct.
                     */
                    inline void resync()
                    {
                        m_syncCode = 0;
                    }

                    /** Read all timings at once. The method iterates
                     ** reading until detecting that the ISR had not
                     ** updated them, it is a cheap way to implement
                     ** mutual exclusion. In practice it should not
                     ** iterate more than once, because ISR update are
                     ** not more frequent than many microseconds.
                     */
                    void readout(unsigned &frame, unsigned &resync, float *out);

                private:
                    /** Timer module structure.
                     */
                    RAM::Target::Timer::Module m_timer;

                    /** Pointer to array where to write actual timing
                     ** figures per channel and gap pulse. For N
                     ** channels there are actually N+1 pulses, period
                     ** figures are capture for each one of them.
                     */
                    volatile unsigned *m_timings_p;

                    /** Channel outputs always belong to same PPM
                     ** frame. To implement that functionality the ISR
                     ** updates this variable only after last channel
                     ** has been updated.
                     */
                    volatile unsigned *m_outputs_p;

                    /** Normalization parameters, which make the
                     ** output a floating point value between zero and
                     ** one. Basically a linear conversion of channel
                     ** readout timings into a fractional number.
                     */
                    float *m_params_p;
                    
                    /** Load value at which timer counter resets..
                     */
                    uint32_t m_load;
                    
                    /** Number of channels in PPM frame.
                     */
                    unsigned m_nchannels;

                    /** Time interval above which timing between two
                     ** consecutive rising edges imply a new frame.
                     **
                     ** Measurement made for Futaba FP-R127DF
                     ** Min = 0.992 ms
                     ** Max = 2.07 ms
                     */
                    unsigned m_frameDetectionThreshold;
                    
                    /** Channel or frame gap timing index. It is used
                     ** internally where in the array to write each
                     ** timing update once ready.
                     */
                    volatile unsigned m_count = 0;

                    /** PPM frame counter.
                     */
                    volatile unsigned m_nframes = 0;
                
                    /** Synchronization status.
                     **
                     ** 0: unsynchronized, searching for frame's first edge.
                     ** 1: rendezvous with first edge just happen, measuring now
                     **    width of first pulse in frame.
                     ** 2: second edge just capture, first pulse width just
                     **    measured.
                     **
                     ** The object remains in #2 (sync) unless a
                     ** measurement shows that the frame's phase has
                     ** been lost (big gap not where it should be). In
                     ** that case the synchronization status switches
                     ** to #0 (unsync) and the process restarts.
                     */
                    volatile unsigned m_syncCode = 0;

                    /** Every time the isr looses track of the frame's
                     ** phase the synchronization status is forced to
                     ** #0 (unsync) and this variable is incremented
                     ** to keep a record of how many times a resync
                     ** has been forced to happen. A number different
                     ** than zero could mean a bad PPM signal
                     ** reception.
                     */
                    volatile unsigned m_resyncCount = 0;
                };

                /** Template parameters select an interrupt pin and
                 ** the number of PPM channels to decode from the
                 ** signal in that pin. The static method which is
                 ** attached to the interrupt is specific to a given
                 ** template parameter, and given that the constructor
                 ** checks whether the pin is in use already, then,
                 ** the isr static method is actually unique for a
                 ** given pin.
                 */
                template <RAM::Target::Timer::ModuleId MODULEID>
                class Decoder: public Base
                {
                public:
                    Decoder(
                        const unsigned nchannels,
                        const unsigned framePeriodus,
                        const unsigned channelMaxWidthus):
                        Base(
                            MODULEID,
                            nchannels,
                            framePeriodus,
                            channelMaxWidthus,
                            isr)
                    {
                        m_obj_p = this;
                    }

                private:
                    /** Pointer to base object that implements the
                     ** actual work to be done everytime an interrupts
                     ** occurs. The variable must be static because
                     ** the ISR is a static method. Note that this
                     ** variable is static to the specific timer
                     ** module class, therefore, there is a version of
                     ** this variable to every possible decoder. That
                     ** is, there is no problem if two decoders are
                     ** instantiated at the same time on two different
                     ** timer modules.
                     */
                    static Base *m_obj_p;

                    /** ISR function to assign within interrupt
                     ** vector. It should simply call base class ISR,
                     ** which implements the real work.
                     */
                    static void isr()
                    {
                        m_obj_p->isr();
                    }
                };

                //
                // initialize static variable
                //
                template <RAM::Target::Timer::ModuleId MODULEID>
                Base *Decoder<MODULEID>::m_obj_p = nullptr;
            }

            namespace PWM
            {
                class Encoder
                {
                public:
                    enum class SerialCommandId: unsigned
                    {
                        SETDUTY = 0
                    };

                    enum class SerialReplyId: unsigned
                    {
                        UNINITIALIZED_MODULE = RAM::Logic::Host::CommandReply::ReplyId::OFFSET
                    };

                    /** Encode [0,1] input parameter into a PWM signal. The PWM
                     ** frequency (1/periodus) must match one of the 'natural'
                     ** frequencies of the timer, that is, the one derived from
                     ** prescaler and maximum counting value. That is to, this
                     ** implementation does not support custom TOP through
                     ** Corina to instead concentrate on always making available
                     ** two PWM signals per timer. It happens also that those
                     ** two PWM signal will necessarily have the same frequency.
                     ** @param dc0us pulse length ON when 0% duty cycle. 
                     ** @param dc100us pulse length ON when 100% duty cycle. 
                     */
                    Encoder(
                        const RAM::Target::PWM::GeneratorId gid,
                        const RAM::Target::PWM::SelectId sid,
                        const uint32_t periodus,
                        const uint32_t dc0us,
                        const uint32_t dc100us,
                        const float initialDutyCycle);
                    
                    void setDutyCycle(
                        const RAM::Target::PWM::SelectId sid,
                        const float &dc);

                private:
                    /** PWM generator data structure.
                     */
                    RAM::Target::PWM::Generator m_generator;
                
                    /** Duty cycle duration in micro-seconds at 0% and 100%.
                     */
                    uint32_t m_dc0us;
                    uint32_t m_dc100us;

                    struct SetDutyCycleCmd:
                        public RAM::Logic::Host::CommandReply::Command
                    {
                        SetDutyCycleCmd(
                            const RAM::Target::PWM::GeneratorId gid,
                            PWM::Encoder *obj);

                        void execute();

                        Encoder *m_obj_p;
                    };
                    
                    SetDutyCycleCmd m_setDutyCycleCmd;
                    
                };
            }
        }
    }
}

#endif /* _PulseModulation_H_ */

/*___oOo__*/
