//
// Arduino stuff
//
#include <Arduino.h>

//
// RAM stuff
//
#include <I2C.h>

//
// Local stuff
//
#include "ADXL345.h"

using namespace std;

//-------------------------------------------------------------------------------
RAM::Sensor::ADXL345::ADXL345(const uint8_t addr):
    Accelerometer(addr, ADXL345_RA_WHO_AM_I, ADXL345_DEVID)
{
    setOutputRegisters(ADXL345_RA_OUT_X_L, true);
}

//-------------------------------------------------------------------------------
void RAM::Sensor::ADXL345::initialize()
{
    //
    // is it really me out there?
    //
    if ( !checkDeviceId() )
    {
        RAM::Library::Error::longjmp(ADXL345_ERR_INVALID_ID);
    }
    
    delay(10);

    //
    // Power CTRL: set measure bit 3 on
    //
    RAM::Library::I2C::write(m_i2cAddr,
                             ADXL345_RA_POWER_CTRL,
                             1 << ADXL345_POWER_CTRL_MEASURE_BIT);
    
    //
    // DATA_FORMAT: set bits 3(full range) and 1 0 on (+/- 16g-range)
    //
    RAM::Library::I2C::write(m_i2cAddr,
                             ADXL345_RA_DATA_FORMAT,
                             ADXL345_DATA_FORMAT_RANGE_16g
                             |
                             1 << ADXL345_DATA_FORMAT_FULLRES_BIT);
    
    //
    // BW_RATE: rate=50hz, bw=25hz
    //
    RAM::Library::I2C::write(m_i2cAddr, ADXL345_RA_BW_RATE, ADXL345_BW_RATE_50odr_25bw);

    //
    // full resolution
    //
    m_scale[CoordinateAxes::X] = 0.004f;
    m_scale[CoordinateAxes::Y] = 0.004f;
    m_scale[CoordinateAxes::Z] = 0.004f;
}

//-------------------------------------------------------------------------------
void RAM::Sensor::ADXL345::calibrate()
{
    int16_t min[3], max[3];

    sampler(512, 20 /*ms*/, m_offset, min, max, m_offsetSigma);

    m_offset[2] = 0.0f;

    Serial.println("===== ADXL345::calibrate =====");
    
    Serial.print("Gain: ");

    for ( unsigned int j = 0; j < 3; ++j )
    {
        Serial.print(m_scale[j]);
        Serial.print("\t");
    }

    Serial.println();

    Serial.print("Offset: ");

    for ( unsigned int j = 0; j < 3; ++j )
    {
        Serial.print(m_offset[j]);
        Serial.print("/");
        Serial.print(m_offsetSigma[j]);
        Serial.print("\t");
    }

    Serial.println();
}

/*___oOo___*/
