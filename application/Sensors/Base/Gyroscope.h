#ifndef _Gyroscope_H_
#define _Gyroscope_H_

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
        /** A gyroscope senses three-axis angular velocity. Readout methods
         ** produce either uncalibrated counts as a signed integers or floating
         ** point degrees-per-second angular velocities.
         */
        class Gyroscope: public RAM::Sensor::Base<3>
        {
        public:
            Gyroscope():
                Base<3>()
            {
            }
            
            Gyroscope(const uint8_t i2cAddr,
                      const uint8_t i2cDevIdReg,
                      const uint8_t i2cDevIdValue):
                Base<3>(3, i2cAddr, i2cDevIdReg, i2cDevIdValue)
            {
            }

            ~Gyroscope() {;}
        };
    }
}
#endif /* _Gyroscope_H_ */

/*___oOo__*/
