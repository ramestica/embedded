#ifndef _HMC5883L_H_
#define _HMC5883L_H_

//
// RAM stuff
//
#include <Magnetometer.h>

#define HMC5883L_DEFAULT_ADDRESS   0x1E
#define HMC5883L_DEVID             0b01001000

#define HMC5883L_RA_WHO_AM_I       0x0A
#define HMC5883L_RA_CONFA          0x00
#define HMC5883L_RA_CONFB          0x01
#define HMC5883L_RA_MODE           0x02
#define HMC5883L_RA_OUT_X_L        0x03
#define HMC5883L_RA_OUT_X_H        0x04
#define HMC5883L_RA_OUT_Z_L        0x05
#define HMC5883L_RA_OUT_Z_H        0x06
#define HMC5883L_RA_OUT_Y_L        0x07
#define HMC5883L_RA_OUT_Y_H        0x08

#define HMC5883L_DOR_BITS          0b00011100
#define HMC5883L_DOR_0p75          0b00000000 
#define HMC5883L_DOR_1p5           0b00000100 
#define HMC5883L_DOR_3             0b00001000 
#define HMC5883L_DOR_7p5           0b00001100 
#define HMC5883L_DOR_15            0b00010000 
#define HMC5883L_DOR_30            0b00010100 
#define HMC5883L_DOR_75            0b00011000 
#define HMC5883L_MA_BITS           0b01100000
#define HMC5843L_MA_1              0b00000000
#define HMC5843L_MA_2              0x00100000
#define HMC5843L_MA_4              0x01000000
#define HMC5843L_MA_8              0x01100000
#define HMC5883L_MEASUREMENT_MODE_BITS          0b00000011
#define HMC5883L_MEASUREMENT_MODE_NORMAL        0b00000000
#define HMC5883L_MEASUREMENT_MODE_POSITIVE_BIAS 0b00000001
#define HMC5883L_MEASUREMENT_MODE_NEGATIVE_BIAS 0b00000010
#define HMC5883L_DIGITAL_RESOLUTION_BITS        0b11100000 
#define HMC5883L_DIGITAL_RESOLUTION_0p73        0b00000000 
#define HMC5883L_DIGITAL_RESOLUTION_0p92        0b00100000 
#define HMC5883L_DIGITAL_RESOLUTION_1p22        0b01000000 
#define HMC5883L_DIGITAL_RESOLUTION_1p52        0b01100000 
#define HMC5883L_DIGITAL_RESOLUTION_2p27        0b10000000 
#define HMC5883L_DIGITAL_RESOLUTION_2p56        0b10100000 
#define HMC5883L_DIGITAL_RESOLUTION_3p03        0b11000000 
#define HMC5883L_DIGITAL_RESOLUTION_4p35        0b11100000 
#define HMC5883L_OPERATING_MODE_CONTINUOUS      0x0
#define HMC5883L_OPERATING_MODE_SINGLE          0x1
#define HMC5883L_OPERATING_MODE_IDLE1           0x2
#define HMC5883L_OPERATING_MODE_IDEL2           0x3

#define HMC5883L_MEASUREMENT_MODE_X_BIAS_GAUSS 1.16f
#define HMC5883L_MEASUREMENT_MODE_Y_BIAS_GAUSS 1.16f
#define HMC5883L_MEASUREMENT_MODE_Z_BIAS_GAUSS 1.08f

namespace RAM
{
    namespace Sensor
    {
        class HMC5883L: public RAM::Sensor::Magnetometer
        {
        public:
            /** Coordinate type in which the magnetic field vector is measured.
             */
            typedef RAM::Control::CoordinateAxes::Cartesian<float, 0, 2, 1> CoordinateAxes;

            /** The HMC5883L has a different axis registers convention.
             */
            enum AxisIndex
            {
                X = 0,
                Z,
                Y
            };

            const static char *AxisName[];

            HMC5883L()
             {;}
            
            HMC5883L(const uint8_t addr);

            void initialize();

            void calibrate();

        private:
            float getDataOutputRate(const uint8_t confa);

         };
    }
}

#endif /* _HMC5883L_H_ */

/*___oOo___*/
