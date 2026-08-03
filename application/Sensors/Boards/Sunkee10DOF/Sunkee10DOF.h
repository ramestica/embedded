#ifndef _Sunkee10DOF_H_
#define _Sunkee10DOF_H_

//
// Contributed stuff
//
#include <L3G4200D-RAM.h>
#include <ADXL345.h>
#include <HMC5883L.h>
#include <BMP085.h>

namespace RAM
{
    namespace IMU
    {
        class Sunkee10DOF
        {
        public:
            Sunkee10DOF()
            {;}
            
            Sunkee10DOF(const uint8_t gyroAddr,
                        const uint8_t accelAddr,
                        const uint8_t magnetAddr,
                        const uint8_t ambientAddr);

            bool init();

            void calibrate();

            void getAngularVelocity(int16_t &avx, int16_t &avy, int16_t &avz);

            void getAcceleration(int16_t *ax, int16_t *ay, int16_t *az);

            void getHeading(int16_t *x, int16_t *y, int16_t *z);

        private:
            RAM::Sensors::L3G4200D m_gyro;
            ADXL345 m_accel;
            HMC5883L m_magnetometer;
            BMP085 m_ambient;
        };
    }
}

#endif /* _Sunkee10DOF_H_ */

/*___oOo___*/
