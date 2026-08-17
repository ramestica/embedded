/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
 
//
// Support stuff
//

//
// Target stuff
//

//
// Logic stuff
// 

//
// Local stuff
//
#include <Control/IMU.h>

using namespace RAM::Control::IMU;

//-------------------------------------------------------------------------------
Device::Device(const AlgorithmType type,
       const float fsHz,
       const float fcHz,
       float const * const gyro,
       float const * const accel,
       float const * const magneto,
       float * const output):
    m_dt(1.0f / fsHz),
    m_gyro_p(gyro),
    m_accel_p(accel),
    m_magneto_p(magneto),
    m_acclLowPassFilter(fsHz, fcHz),
    m_output_p(output)
{
    if ( type == AlgorithmType::COMPLEMENTARY )
    {
        m_filters_p = new RAM::Control::Filter::Complementary[m_magneto_p == nullptr ? 2 : 3];
    }

    m_filters_p[0] = RAM::Control::Filter::Complementary(m_dt, 0.1);

    m_filters_p[1] = RAM::Control::Filter::Complementary(m_dt, 0.1);
}

//-------------------------------------------------------------------------------
void Device::update()
{
    float angle;
    
    //
    // Tilt Sensing Using a Three-Axis Accelerometer. In particular,
    // note the following:
    //
    // 'The convention used in the aerospace sequence is that the roll
    // angle can range between -180° to +180° but the pitch angle is
    // restricted to -90° to +90°.'
    //
    // https://www.nxp.com/docs/en/application-note/AN3461.pdf
    //

    //
    // The Balance Filter (Shane Colton)
    // https://drive.google.com/file/d/0B9rLLz1XQKmaLVJLSkRwMTU0b0E/view
    //

    //
    // MPU6050 is mounted such that rolls on Y axis and pitches on X
    // angle
    //

    //
    // AN3461 eq. 26, minus sign on Gpx removed because our g vector
    // points down
    //
    angle = atan2(m_accel_p[0], sqrt(m_accel_p[1] * m_accel_p[1] + m_accel_p[2] * m_accel_p[2]));

    m_filters_p[0].update(m_output_p[0], angle, m_gyro_p[1]);

    //
    // AN3461 eq. 25
    //
    // FIXME: this should be improved with eq. 38 in AN3461
    //
    angle = atan2(m_accel_p[1], m_accel_p[2]);

    //
    // if in arctan2(y, x) x is negative then the function is
    // discontinued through y equal zero. Correct for that explicitly.
    //
    if ( m_accel_p[2] < 0 )
    {
        if ( m_accel_p[1] >= 0. )
        {
            angle -= 3.1415927410125732421875f;
        }
        else
        {
            angle += 3.1415927410125732421875f;
        }
    }        

    m_filters_p[1].update(m_output_p[1], angle, m_gyro_p[0]);
    
    //m_acclLowPassFilter.update(accl, acclLowPass);
}

/*___oOo___*/
