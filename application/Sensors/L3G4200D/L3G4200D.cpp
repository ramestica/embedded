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
#include "L3G4200D.h"

using namespace std;

//-------------------------------------------------------------------------------
RAM::Sensor::L3G4200D::L3G4200D(const uint8_t addr):
    Gyroscope(addr, L3G4200D_RA_WHO_AM_I, L3G4200D_DEVID)
{
    setOutputRegisters(0x80 | L3G4200D_RA_OUT_X_L, true);
}

//-------------------------------------------------------------------------------
void RAM::Sensor::L3G4200D::initialize()
{
    uint16_t fullScale;

    //
    // is it really me out there?
    //
    if ( !checkDeviceId() )
    {
        RAM::Library::Error::longjmp(L3G4200D_ERR_INVALID_ID);
    }
    
    delay(100);

    //
    // CTRL_REG1   400Hz ODR, 20hz filter, run!
    //
    RAM::Library::I2C::write(m_i2cAddr,
                             L3G4200D_RA_CTRL_REG1,
                             L3G4200D_RATE_400
                             |
                             L3G4200D_BW_LOW
                             |
                             0x1 << L3G4200D_PD_BIT
                             |
                             0x1 << L3G4200D_XEN_BIT
                             |
                             0x1 << L3G4200D_YEN_BIT
                             |
                             0x1 << L3G4200D_ZEN_BIT);
    
    delay(5);

    //
    // CTRL_REG5   low pass filter enable
    //
    RAM::Library::I2C::write(m_i2cAddr, L3G4200D_RA_CTRL_REG5, 0x02);

    delay(5);

    //
    // CTRL_REG4 Select 2000 dps
    //
    RAM::Library::I2C::write(m_i2cAddr, L3G4200D_RA_CTRL_REG4, 0x30);

    fullScale = 2000;

    if ( fullScale == 250 )
    {
        m_scale[CoordinateAxes::X] = 0.00875f;

        m_scale[CoordinateAxes::Y] = 0.00875f;

        m_scale[CoordinateAxes::Z] = 0.00875f;
    }
    else if ( fullScale == 500 )
    {
        m_scale[CoordinateAxes::X] = 0.0175f;

        m_scale[CoordinateAxes::Y] = 0.0175f;

        m_scale[CoordinateAxes::Z] = 0.0175f;
    }
    else
    {
        m_scale[CoordinateAxes::X] = 0.07f;

        m_scale[CoordinateAxes::Y] = 0.07f;

        m_scale[CoordinateAxes::Z] = 0.07f;
    }
}

//-------------------------------------------------------------------------------
void RAM::Sensor::L3G4200D::calibrate()
{
    int16_t min[3], max[3];

    sampler(512, 3 /*ms*/, m_offset, min, max, m_offsetSigma);

    Serial.println("===== L3G4200D::calibrate =====");
    
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
