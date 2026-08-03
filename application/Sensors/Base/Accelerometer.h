#ifndef _Accelerometer_H_
#define _Accelerometer_H_

//
// RAM stuff
//
#include <CoordinateAxes.h>

//
// Local stuff
//
#include "SensorBase.h"

namespace RAM
{
    namespace Sensor
    {
        /** An accelerometer senses proper acceleration in three orthogonal axes.
         */
        class Accelerometer: public RAM::Sensor::Base<3>
        {
        public:
            Accelerometer():
                Base<3>()
            {
            }

            Accelerometer(const uint8_t i2cAddr,
                          const uint8_t i2cDevIdReg,
                          const uint8_t i2cDevIdValue):
                Base<3>(3, i2cAddr, i2cDevIdReg, i2cDevIdValue)
            {
            }

            ~Accelerometer() {;}
        };
    }
}
#endif /* _Accelerometer_H_ */

/*___oOo__*/
