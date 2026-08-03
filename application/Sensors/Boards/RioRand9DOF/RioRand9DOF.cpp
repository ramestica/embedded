//
// Local stuff
//
#include "RioRand9DOF.h"

//-------------------------------------------------------------------------------
RioRand9DOF::RioRand9DOF():
    m_gyroAddr(0x0),
    m_accelAddr(0x0),
    m_magnetAddr(0x0)
{
}

//-------------------------------------------------------------------------------
RioRand9DOF::RioRand9DOF(
    const uint8_t gyroAddr,
    const uint8_t accelAddr,
    const uint8_t magnetAddr):
    m_gyroAddr(gyroAddr),
    m_accelAddr(accelAddr),
    m_magnetAddr(magnetAddr)
{
}

/*___oOo___*/
