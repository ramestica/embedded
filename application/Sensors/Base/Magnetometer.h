#ifndef _Magnetometer_H_
#define _Magnetometer_H_

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
        /** A compass senses three-axis magnetic field.
         */
        class Magnetometer: public RAM::Sensor::Base<3>
        {
        public:
            Magnetometer():
                Base<3>()
            {
            }

            Magnetometer(const uint8_t i2cAddr,
                         const uint8_t i2cDevIdReg,
                         const uint8_t i2cDevIdValue):
                Base<3>(3, i2cAddr, i2cDevIdReg, i2cDevIdValue)
            {
            }

            ~Magnetometer() {;}
        };
    }
}
#endif /* _Magnetometer_H_ */

/*___oOo__*/
