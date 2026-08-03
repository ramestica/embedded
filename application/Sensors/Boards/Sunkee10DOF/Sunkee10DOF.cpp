//
// RAM stuff
//
#include <SampleStatistics.h>

//
// Local stuff
//
#include "Sunkee10DOF.h"

//-------------------------------------------------------------------------------
RAM::IMU::Sunkee10DOF::Sunkee10DOF(
    const uint8_t gyroAddr,
    const uint8_t accelAddr,
    const uint8_t magnetAddr,
    const uint8_t ambientAddr):
    m_gyro(gyroAddr),
    m_accel(accelAddr),
    m_magnetometer(magnetAddr),
    m_ambient(ambientAddr)
{
}

//-------------------------------------------------------------------------------
bool RAM::IMU::Sunkee10DOF::init()
{
    m_gyro.initialize();

    if ( !m_gyro.testConnection() )
    {
        return false;
    }

    m_accel.initialize();

    if ( !m_accel.testConnection() )
    {
        return false;
    }

    m_magnetometer.initialize();

    if ( !m_magnetometer.testConnection() )
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------
void RAM::IMU::Sunkee10DOF::calibrate()
{
    m_gyro.calibrate();
}

//-------------------------------------------------------------------------------
void RAM::IMU::Sunkee10DOF::getAngularVelocity(int16_t &avx, int16_t &avy, int16_t &avz)
{
    m_gyro.getAngularVelocity(avx, avy, avz);
}

//-------------------------------------------------------------------------------
void RAM::IMU::Sunkee10DOF::getAcceleration(int16_t *ax, int16_t *ay, int16_t *az)
{
    m_accel.getAcceleration(ax, ay, az);
}

//-------------------------------------------------------------------------------
void RAM::IMU::Sunkee10DOF::getHeading(int16_t *x, int16_t *y, int16_t *z)
{
    m_magnetometer.getHeading(x, y, z);
}

/*___oOo___*/
