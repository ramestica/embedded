#ifndef _ADXL345_H_
#define _ADXL345_H_

//
// RAM stuff
//
#include <Accelerometer.h>

#define ADXL345_DEFAULT_ADDRESS   0x53
#define ADXL345_DEVID             0xE5

#define ADXL345_RA_WHO_AM_I       0x00
#define ADXL345_RA_BW_RATE        0x2C
#define ADXL345_RA_POWER_CTRL     0x2D
#define ADXL345_RA_DATA_FORMAT    0x31
#define ADXL345_RA_OUT_X_L        0x32
#define ADXL345_RA_OUT_X_H        0x33
#define ADXL345_RA_OUT_Y_L        0x34
#define ADXL345_RA_OUT_Y_H        0x35
#define ADXL345_RA_OUT_Z_L        0x36
#define ADXL345_RA_OUT_Z_H        0x37

#define ADXL345_POWER_CTRL_MEASURE_BIT  3
#define ADXL345_DATA_FORMAT_FULLRES_BIT 3

#define ADXL345_DATA_FORMAT_RANGE_2g  0x0
#define ADXL345_DATA_FORMAT_RANGE_4g  0x1
#define ADXL345_DATA_FORMAT_RANGE_8g  0x2
#define ADXL345_DATA_FORMAT_RANGE_16g 0x3

#define ADXL345_BW_RATE_400odr_200bw   0b1100
#define ADXL345_BW_RATE_200odr_100bw   0b1011
#define ADXL345_BW_RATE_100odr_50bw    0b1010
#define ADXL345_BW_RATE_50odr_25bw     0b1001
#define ADXL345_BW_RATE_25odr_12p5bw   0b1000
#define ADXL345_BW_RATE_12p5odr_6p25bw 0b0111

namespace RAM
{
    namespace Sensor
    {
        class ADXL345: public RAM::Sensor::Accelerometer
        {
        public:
            typedef RAM::Control::CoordinateAxes::Cartesian<float, 0, 1, 2> CoordinateAxes;

            ADXL345()
             {;}
            
            ADXL345(const uint8_t addr);

            void initialize();

            void calibrate();
        };
    }
}

#endif /* _ADXL345_H_ */

/*___oOo___*/
