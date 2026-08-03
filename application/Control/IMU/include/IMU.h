#ifndef _IMU_H_
#define _IMU_H_

//
// RAM stuff
//
#include <FirstOrder.h>
#include <Complementary.h>

namespace RAM
{
    namespace Control
    {
        /** Inertial measurement unit.
         */
        namespace IMU
        {
            /** Algorithm type.
             */
            enum class AlgorithmType
            {
                COMPLEMENTARY,
                KALMAN
            };
            
            /** A nine axes (accelerometer + gyroscope +
             ** magenetometer) IMU based on the complementary filter.
             */
            class Device
            {
            public:
                Device():
                    m_gyro_p(nullptr),
                    m_accel_p(nullptr),
                    m_magneto_p(nullptr),
                    m_output_p(nullptr)
                {;}

                Device(const AlgorithmType type,
                       const float fsHz,
                       const float fcHz,
                       float const * const gyro,
                       float const * const accl,
                       float const * const magneto,
                       float * const output);
                
                void update();
                
            private:
                float m_dt; 
                float const * const m_gyro_p;
                float const * const m_accel_p;
                float const * const m_magneto_p;
                RAM::Control::Filter::FirstOrder<3> m_acclLowPassFilter;
                RAM::Control::Filter::Complementary * m_filters_p;
                float * const m_output_p; // roll, pitch and yaw
            };
        }
    }
}

#endif /* _L3G4200D_H_ */

/*___oOo___*/
