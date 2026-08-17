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
CascadePID::CascadePID():
    m_inner(),
    m_outer()
{
}

//-------------------------------------------------------------------------------
void CascadePID::setParams(const float kp, const float ki, const float kd, const CONTROLLER ctrl)
{
    if ( ctrl == CONTROLLER::BOTH || ctrl == CONTROLLER::INNER )
    {
        m_inner.setParams(kp, ki, kd);
    }

    if ( ctrl == CONTROLLER::BOTH || ctrl == CONTROLLER::OUTER )
    {
        m_outer.setParams(kp, ki, kd);
    }
}

//-------------------------------------------------------------------------------
void CascadePID::reset(const CONTROLLER ctrl)
{
    if ( ctrl == CONTROLLER::BOTH || ctrl == CONTROLLER::INNER )
    {
        m_inner.reset();
    }

    if ( ctrl == CONTROLLER::BOTH || ctrl == CONTROLLER::OUTER )
    {
        m_outer.reset();
    }
}

//-------------------------------------------------------------------------------
float CascadePID::update(
    const float32_t outerErr,
    const float32_t innerValue,
    const float min,
    const float max)
{
    return m_inner.update(m_outer.update(outerErr) - innerValue, min, max);
}

/*___oOo___*/
