#ifndef _PID_H_
#define _PID_H_

//
// System stuff
//

//
// Target stuff
//
#include <Target/Target.h>

//
// CMSIS stuff
//
#include <arm_math.h>

//
// RAM stuff
// 

//
// Local stuff
//

namespace RAM
{
    namespace Control
    {
        /** Add anti-windup feature to CMSIS controller.
         */
        struct PID: public arm_pid_instance_f32
        {
            PID();

            /** Set parameters without resetting the state. If the
             ** integration action is currently disabled (anti-windup)
             ** then take care of modifying the new parameters
             ** accordingly. That is, integration action remains
             ** enabled or disabled as it was before calling this
             ** method.
             */
            void setParams(const float kp, const float ki, const float kd);

            /** Reset state, including the anti-windup feature
             ** implemented by the class itself. That is, integration
             ** action is enabled by this method.
             */
            void reset();

            /** Update controller action based on given error
             ** value. Integral action is disabled while the controller
             ** action output is ourside the given [min, max] range.
             */
            float update(const float32_t in, const float min = 0., const float max = 0.);

            bool m_isIntegrationActionOn = true;
        };

        /** Two PID controllers in cascade configuration. There is
         ** only one process action signal and two process values to
         ** control. Integral anti-windup applied to PID2 only.
         ** 
         **                +---------+            +---------+             +-----------+
         **     r1         |         |   r2       |         |             |           |
         **   ------------>|  PID1   |----------->|  PID2   |------------>|  Plant    |
         **                |         |            |         |             |           |
         **                +---------+            +---------+             |           |
         **                      ^                      ^                 |           |
         **                      |                      |                 |           |
         **                      |                      +-----------------|           |
         **                      |                         pv2            |           |
         **                      |                                        |           |
         **                      +----------------------------------------|           |
         **                           pv1                                 |           |
         **                                                               +-----------+
         */
        class CascadePID
        {
        public:
            enum class CONTROLLER
            {
                INNER = 0, OUTER = 1, BOTH
            };

            /** Default controller initializes to a proportional
             ** controller with Kp=1.
             */
            CascadePID();

            /** Set controller(s) parameters and initialize without
             ** resetting the state.
             */
            void setParams(const float kp, const float ki, const float kd, const CONTROLLER ctrl);

            /** Reset controller(s) state.
             */
            void reset(const CONTROLLER ctrl);

            /** Integral anti-windup limits apply to inner controller
             ** only. The inner controller output is the only action
             ** applied to the process. See diagram above.
             */
            float update(
                const float32_t outerErr,
                const float32_t innerValue,
                const float min = 0.,
                const float max = 0.);
            
        private:
            PID m_inner, m_outer;
        };
    }
}

#endif /* _PID_H_ */

/*___oOo__*/
