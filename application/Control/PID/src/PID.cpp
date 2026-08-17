/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif
 
//
// System stuff
//
#include <initializer_list>

//
// Base stuff
//

//
// Target stuff
//

//
// Local stuff
//
#include <Control/PID.h>

using namespace RAM::Control;

//-------------------------------------------------------------------------------
PID::PID()
{
    setParams(1., 0., 0.);

    reset();
}

//-------------------------------------------------------------------------------
void PID::setParams(const float kp, const float ki, const float kd)
{
    Kp = kp;

    Ki = ki;

    Kd = kd;

    //
    // compute parameters without resetting the state
    //
    arm_pid_init_f32(this, 0);

    //
    // if the integration action is currently disabled then adjust new
    // A0 parameter
    //
    if ( !m_isIntegrationActionOn )
    {
        //
        // this is equivalent to making ki equal zero
        //
        A0 -= Ki;
    }
}

//-------------------------------------------------------------------------------
void PID::reset()
{
    if ( !m_isIntegrationActionOn )
    {
        //
        // reenable error integration action
        //
        A0 += Ki;

        m_isIntegrationActionOn = true;
    }

    arm_pid_reset_f32(this);
}

//-------------------------------------------------------------------------------
float PID::update(const float in, const float min, const float max)
{
    float out = arm_pid_f32(this, in);

    //
    // if no anti windup configured then return now
    //
    if ( min == max )
    {
        return out;
    }
    
    if ( out < min || out > max )
    {
        if ( m_isIntegrationActionOn )
        {
            //
            // this is equivalent to making ki equal zero
            //
            A0 -= Ki;

            m_isIntegrationActionOn = false;
        }

        return out < min ? min : max;
    }

    if ( !m_isIntegrationActionOn )
    {
        //
        // reenable error integration action
        //
        A0 += Ki;

        m_isIntegrationActionOn = true;
    }

    return out;
}

/*___oOo___*/
