#ifndef _RioRand9DOF_H_
#define _RioRand9DOF_H_

//
// System stuff
//
#include <inttypes.h>

namespace RAM
{
    namespace IMU
    {
        class RioRand9DOF
        {
        public:
            RioRand9DOF();
            
            RioRand9DOF(const uint8_t gyroAddr, const uint8_t accelAddr, const uint8_t magnetAddr);

        private:
            uint8_t m_gyroAddr;
            uint8_t m_accelAddr;
            uint8_t m_magnetAddr
        };
    }
}

#endif /* _RioRand9DOF_H_ */

/*___oOo___*/
