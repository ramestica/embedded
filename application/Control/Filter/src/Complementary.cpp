/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
#include <math.h>
 
//
// Library stuff
//
#include <ErrorCodes.h>
#include <Utils.h>

//
// Local stuff
//
#include "Complementary.h"

using namespace RAM::Control::Filter;

//-------------------------------------------------------------------------------
Complementary::Complementary():
    m_alpha(1.0f),
    m_output_memory(0.0f)
{
}

//-------------------------------------------------------------------------------
Complementary::Complementary(const float dt, const float tau):
    m_dt(dt),
    m_alpha(tau / (tau + m_dt)),
    m_output_memory(0.0f)
{
}

//-------------------------------------------------------------------------------
void Complementary::update(float &out, const float &in, const float &din) 
{
    if ( !m_isInitialized )
    {
        m_output_memory = in;
    
        m_isInitialized = true;
    }
    
    out = m_output_memory =
        m_alpha * (m_output_memory + din * m_dt) + (1.0f - m_alpha) * in;
}

/*___oOo___*/
