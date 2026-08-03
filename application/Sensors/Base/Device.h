#ifndef _SensorBase_H_
#define _SensorBase_H_

//
// Arduino stuff
//
#include <Arduino.h>

//
// System stuff
//
#include <inttypes.h>

//
// RAM stuff
//
#include <I2C.h>
#include <SampleStatistics.h>

namespace RAM
{
    namespace Device
    {
        class Base
        {
        };

        class I2C: public Base
        {
        };

        class SPI: public Base
        {
        };
    }
}

#endif /* _SensorBase_H_ */

/*___oOo__*/
