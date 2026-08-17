#ifndef _SystemTick_H_
#define _SystemTick_H_

//
// System stuff
//

//
// Target stuff
//
#include <Target/Target.h>
#include <Target/GPIO.h>

//
// Local stuff
//

namespace RAM
{
    namespace Target
    {
        namespace SystemTick
        {
            /** Pin used as a heart beat.
             */
            static const RAM::Target::GPIO::Pin m_heartBeatPin = {};

            /** Start system tick. Every so many jiffies set given user flag.
             ** Every userJiffies ticks the user flags is set to true.
             ** @param userJiffies how many periods after setting user flag.
             ** @param userFlag boolean flag pointer.
             */
            int start(
                const bool enableHeartBeat,
                const uint32_t periodus,
                const uint32_t userJiffies,
                bool *userFlag);

            uint32_t jiffies();
            
            uint32_t micros();

            /** Mark current time in microseconds and its value minus
             * previous mark.
             */
            uint32_t mark();

            void delayus(const uint32_t us);
        }
    }
}

#endif /* _SystemTick_H_ */

/*___oOo__*/
