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
#include "HMC5883L.h"

using namespace std;

const char *RAM::Sensor::HMC5883L::AxisName[] =
{
    "X",
    "Z",
    "Y"
};

//-------------------------------------------------------------------------------
RAM::Sensor::HMC5883L::HMC5883L(const uint8_t addr):
    Magnetometer(addr, HMC5883L_RA_WHO_AM_I, HMC5883L_DEVID)
{
    setOutputRegisters(HMC5883L_RA_OUT_X_L, false);
}

//-------------------------------------------------------------------------------
void RAM::Sensor::HMC5883L::initialize()
{
    //
    // is it really me out there?
    //
    if ( !checkDeviceId() )
    {
        RAM::Library::Error::longjmp(HMC5883L_ERR_INVALID_ID);
    }

    //
    // Reg A DOR 15 Hz + MS1,MS0 set to normal (no bias)
    //
    RAM::Library::I2C::write(m_i2cAddr,
                             uint8_t(HMC5883L_RA_CONFA),
                             uint8_t(HMC5843L_MA_8
                                     |
                                     HMC5883L_DOR_75
                                     |
                                     HMC5883L_MEASUREMENT_MODE_NORMAL));
    
    //
    // Note that the very first measurement after a gain change maintains the
    // same gain as the previous setting. The new gain setting is effective
    // from the second measurement and on.
    //

    //
    // set the Gain
    //
    RAM::Library::I2C::write(m_i2cAddr, HMC5883L_RA_CONFB, HMC5883L_DIGITAL_RESOLUTION_1p22); 

    //
    // operating mode: continuous
    //
    RAM::Library::I2C::write(m_i2cAddr, HMC5883L_RA_MODE, HMC5883L_OPERATING_MODE_CONTINUOUS);

    delay(100);

    //
    // get one sample, and discard it (read above)
    //
    int16_t data[3];
    readout(data);
}

//-------------------------------------------------------------------------------
void RAM::Sensor::HMC5883L::calibrate()
{
    uint8_t confa, confb;
    unsigned int updatePeriod;
    uint8_t gains[8];
    int16_t out[3];
    int16_t min[3], max[3];
    float rms[3];

    //
    // read current configuration A register (to be reinstated after calibration)
    //
    confa = RAM::Library::I2C::read(m_i2cAddr, HMC5883L_RA_CONFA);

    //
    // read current configuration B register (to be reinstated after calibration)
    //
    confb = RAM::Library::I2C::read(m_i2cAddr, HMC5883L_RA_CONFB);

    //
    // data is updated every so many milliseconds
    //
    updatePeriod = 1000.0f / getDataOutputRate(confa);

    //
    // gains to exercise
    //
    gains[0] = HMC5883L_DIGITAL_RESOLUTION_0p73;
    gains[1] = HMC5883L_DIGITAL_RESOLUTION_0p92;
    gains[2] = HMC5883L_DIGITAL_RESOLUTION_1p22;
    gains[3] = HMC5883L_DIGITAL_RESOLUTION_1p52;
    gains[4] = HMC5883L_DIGITAL_RESOLUTION_2p27; 
    gains[5] = HMC5883L_DIGITAL_RESOLUTION_2p56; 
    gains[6] = HMC5883L_DIGITAL_RESOLUTION_3p03; 
    gains[7] = HMC5883L_DIGITAL_RESOLUTION_4p35; 

    Serial.print("sampling period: ");
    Serial.print(updatePeriod);
    Serial.print(" [ms] (");
    Serial.print(1000 / updatePeriod);
    Serial.println(" [hz])");

    //
    // iterate over different gain settings
    //
    for ( uint8_t gi = 0, g = gains[gi]; gi < 8; ++gi, g = gains[gi] )
    {
        Serial.print("digital resolution index: ");
        Serial.print(g >> 5);
        Serial.println((HMC5883L_DIGITAL_RESOLUTION_BITS & confb) == g ? " (*)" : "");
        //
        // change gain (we are assuming continuous mode)
        //
        RAM::Library::I2C::write(m_i2cAddr, HMC5883L_RA_CONFB, g); 

        //
        // switch between positive an negative bias
        //
        for ( int bias = 0; bias < 2; ++bias )
        {
            //
            // activate positive or negative bias
            //
            RAM::Library::I2C::write(m_i2cAddr,
                                     HMC5883L_RA_CONFA,
                                     (confa & ~HMC5883L_MEASUREMENT_MODE_BITS)
                                     |
                                     (bias == 0 ? HMC5883L_MEASUREMENT_MODE_POSITIVE_BIAS : HMC5883L_MEASUREMENT_MODE_NEGATIVE_BIAS));
            
            //
            // without this long delay sometimes the output still remains using the
            // previous gain
            //
            delay(150);

            //
            // after changing the gain we must discard one measurement
            //
            readout(out);
            
            //
            // collect a few samples, scale update period by two because in test
            // mode there are actually two samples done
            //
            sampler(1, updatePeriod * 2, out, min, max, rms);

            //
            // compute gain associated to operational resolution in use
            //
            if ( (HMC5883L_DIGITAL_RESOLUTION_BITS & confb) == g )
            {
                if ( bias == 0 )
                {
                    for ( unsigned int i = 0; i < m_dimension; ++i )
                    {
                        m_scale[i] = out[i];
                    }
                }
                else
                {
                    for ( unsigned int i = 0; i < m_dimension; ++i )
                    {
                        m_scale[i] = (m_scale[i] - out[i]) / 2.0f;

                        if ( i == X )
                        {
                            m_scale[i] = HMC5883L_MEASUREMENT_MODE_X_BIAS_GAUSS / m_scale[i];
                        }
                        else if ( i == Y )
                        {
                            m_scale[i] = HMC5883L_MEASUREMENT_MODE_Y_BIAS_GAUSS / m_scale[i];
                        }
                        else
                        {
                            m_scale[i] = HMC5883L_MEASUREMENT_MODE_Z_BIAS_GAUSS / m_scale[i];
                        }
                    }
                }
            }
            
            for ( unsigned int axis = 0; axis < 3; ++axis )
            {
                Serial.print(AxisName[axis]);
                Serial.print(bias == 0 ? " (+): " : " (-): ");
                Serial.print(out[axis]);
                Serial.print("/");
                Serial.print(min[axis]);
                Serial.print("/");
                Serial.print(max[axis]);
                Serial.print("/");
                Serial.println(rms[axis]);
            }
        }
    }

    Serial.print("X gain: ");Serial.print(1000*m_scale[X]);Serial.println(" [Gauss/LSb]");
    Serial.print("Y gain: ");Serial.print(1000*m_scale[Y]);Serial.println(" [Gauss/LSb]");
    Serial.print("Z gain: ");Serial.print(1000*m_scale[Z]);Serial.println(" [Gauss/LSb]");

    //
    // reinstate A and B configuration registers
    //
    RAM::Library::I2C::write(m_i2cAddr, HMC5883L_RA_CONFA, confa);
    RAM::Library::I2C::write(m_i2cAddr, HMC5883L_RA_CONFB, confb);
        
    //
    // after changing the gain we must discard one measurement
    //
    delay(updatePeriod);
    readout(out);
}

//-------------------------------------------------------------------------------
float RAM::Sensor::HMC5883L::getDataOutputRate(const uint8_t confa)
{
    switch ( confa & HMC5883L_DOR_BITS)
    {
    case HMC5883L_DOR_0p75:
        return 0.75f;
    case HMC5883L_DOR_1p5:
        return 1.5f;
    case HMC5883L_DOR_3:
        return 3.0f;
    case HMC5883L_DOR_7p5:
        return 7.5f;
    case HMC5883L_DOR_15:
        return 15.0f;
    case HMC5883L_DOR_30:
        return 30.0f;
    case HMC5883L_DOR_75:
        return 75.0f;
    default:
        return 1.0f;
    }
}

/*___oOo___*/
