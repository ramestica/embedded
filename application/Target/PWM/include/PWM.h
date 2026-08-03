#ifndef _TargetPWM_H_
#define _TargetPWM_H_

//
// System stuff
//

//
// Target stuff
//
#include <GPIO.h>

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
        namespace PWM
        {
            struct Module
            {
                ModuleId m_id;

                intptr_t m_pointer;

                unsigned m_index = static_cast<unsigned>(ModuleId::IDN);
            };
                
            struct Generator
            {
                Module m_module;
                
                GeneratorId m_id;

                SelectId m_sid;
                
                unsigned m_index = static_cast<unsigned>(GeneratorId::IDN);

                /** Divider to synthesize clock driving the generator from
                 ** system clock. A value of one means no divider, that is, same
                 ** as system clock.
                 */
                unsigned m_divider;
                
                unsigned m_periodus;

                unsigned m_load;

                RAM::Target::GPIO::Pin m_pins;
            };

            int configure(
                const GeneratorId gid,
                const SelectId sid,
                const unsigned periodus,
                Generator &generator);

            int start(
                const Generator &generator,
                float dutyCycle[2],
                void (*isr)());

            int setDutyCycle(
                const Generator &generator,
                const SelectId sid,
                float dutyCycle);
        }
    }
}

#endif /* _TargetPWM_H_ */

/*___oOo__*/
