//
// RAM stuff
//
#include <SampleStatistics.h>

//
// Local stuff
//
#include "L3G4200D-RAM.h"

//-------------------------------------------------------------------------------
RAM::Sensors::L3G4200D::L3G4200D(const uint8_t addr):
    ::L3G4200D(addr),
    m_addr(addr)
{
    m_offset[0] = 0;

    m_offset[1] = 0;

    m_offset[2] = 0;

    m_gain = 0.0f;
}

//-------------------------------------------------------------------------------
void RAM::Sensors::L3G4200D::initialize()
{
    uint16_t fullScale;

    //::L3G4200D::initialize();

    delay(100);

    I2Cdev::writeByte(m_addr, L3G4200D_RA_CTRL_REG1, 0b10001111);

    delay(5);

    I2Cdev::writeByte(m_addr, L3G4200D_RA_CTRL_REG5, 0b00000010);

    delay(5);

    I2Cdev::writeByte(m_addr, L3G4200D_RA_CTRL_REG4, 0b00110000);

    //setOutputDataRate(400);

    //setBandwidthCutOffMode(L3G4200D_BW_LOW);

    //setPowerOn(true);

    fullScale = ::L3G4200D::getFullScale();

    if ( fullScale == 250 )
    {
        m_gain = 0.00875f;
    }
    else if ( fullScale == 500 )
    {
        m_gain = 0.0175f;
    }
    else
    {
        m_gain = 0.07f;
    }
}

//-------------------------------------------------------------------------------
void RAM::Sensors::L3G4200D::calibrate()
{
    RAM::SampleStatistics stats[3];
    int16_t av[3];

    for ( unsigned int i = 0; i < 100; ++i )
    {
        //
        // access angular velocities from base class
        //
        ::L3G4200D::getAngularVelocity(&av[0], &av[1], &av[2]);

        for ( unsigned int j = 0; j < 3; ++j )
        {
            stats[j].addSample(av[j]);
        }

        delay(1);
    }

    for ( unsigned int j = 0; j < 3; ++j )
    {
        m_offset[j] = static_cast<int16_t>(stats[j].m_mean);
    }

    Serial.println("===== L3G4200D::calibrate =====");
    
    Serial.print("Gain  : "); Serial.print(m_gain); Serial.println(" [dps]");

    Serial.print("Offset: ");

    for ( unsigned int j = 0; j < 3; ++j )
    {
        Serial.print(m_offset[j]);
        Serial.print("/");
        Serial.print(sqrt(stats[j].m_var));
        Serial.print("\t");
    }

    Serial.println();
}

//-------------------------------------------------------------------------------
void RAM::Sensors::L3G4200D::getAngularVelocity(int16_t &avx, int16_t &avy, int16_t &avz)
{
    ::L3G4200D::getAngularVelocity(&avx, &avy, &avz);

    avx = (avx - m_offset[0]) * m_gain;

    avy = (avy - m_offset[1]) * m_gain;

    avz = (avz - m_offset[2]) * m_gain;
}

/*___oOo___*/
