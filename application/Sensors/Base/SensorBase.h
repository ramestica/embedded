#ifndef _SensorBase_H_
#define _SensorBase_H_

//
// Arduino stuff
//
#include <Arduino.h>

//
// System stuff
//
#include <inttypes.h>

//
// RAM stuff
//
#include <I2C.h>
#include <SampleStatistics.h>

namespace RAM
{
    namespace Sensor
    {
        template <unsigned int DIMENSION>
        class Base
        {
        public:
            Base():
                m_dimension(0),
                m_i2cAddr(0x0),
                m_i2cDevIdReg(0x0),
                m_i2cDevIdValue(0x0),
                m_isCalibrated(false)
            {;}

            Base(const uint8_t dimension,
                 const uint8_t i2cAddr,
                 const uint8_t i2cDevIdReg,
                 const uint8_t i2cDevIdValue):
                m_dimension(DIMENSION),
                m_i2cAddr(i2cAddr),
                m_i2cDevIdReg(i2cDevIdReg),
                m_i2cDevIdValue(i2cDevIdValue),
                m_isCalibrated(false)
            {
                for ( unsigned int i = 0; i < DIMENSION; ++i )
                {
                    m_scale[i] = float(1);
                    m_offset[i] = 0.0f;
                    m_offsetSigma[i] = 0.0f;
                    m_outAddrLowNibble[i] = 0;
                    m_outAddrHighNibble[i] = 0;
                }
            }

            ~Base() {;}

            bool checkDeviceId()
            {
                if ( (m_i2cDevIdReg != 0x0 || m_i2cDevIdValue != 0x0)
                     &&
                     RAM::Library::I2C::read(m_i2cAddr, m_i2cDevIdReg) != m_i2cDevIdValue )
                {
                    Serial.print("failed to verify i2c device id");
                    
                    return false;
                }
    
                return true;
            }

            virtual void initialize() {;}

            virtual void calibrate() {;}

            void sampler(const unsigned int repeat, 
                         const unsigned int updatePeriod,
                         int16_t (&out)[DIMENSION],
                         int16_t (&min)[DIMENSION],
                         int16_t (&max)[DIMENSION],
                         float (&rms)[DIMENSION])
            {
                RAM::Library::SampleStatistics stats[DIMENSION];
                int16_t data[DIMENSION];

                for ( unsigned int i = 0; i < repeat; ++i )
                {
                    readout(data);

                    for ( unsigned int j = 0; j < m_dimension; ++j )
                    {
                        stats[j].addSample(data[j]);
                    }
                    
                    delay(updatePeriod + 1);
                }

                for ( unsigned int i = 0; i < m_dimension; ++i )
                {
                    out[i] = static_cast<int16_t>(stats[i].m_mean);
                    
                    min[i] = static_cast<int16_t>(stats[i].m_min);
                    
                    max[i] = static_cast<int16_t>(stats[i].m_max);
                    
                    rms[i] = sqrt(stats[i].m_var);
                }
            }

            void setOutputRegisters(const uint8_t reg, const bool isLSBFirst)
            {
                uint8_t lreg = reg;
                
                for ( unsigned int i = 0; i < m_dimension; ++i )
                {
                    m_outAddrLowNibble[i] = lreg++;
                    
                    m_outAddrHighNibble[i] = lreg++;
                }
                
                if ( isLSBFirst )
                {
                    m_readoutByteLSB = 0;
                    
                    m_readoutByteMSB = 1;
                }
                else
                {
                    m_readoutByteLSB = 1;
                    
                    m_readoutByteMSB = 0;
                }
                
                m_areOutputRegistersConsecutive = true;
            }
            
            void readout(int16_t (&data)[DIMENSION])
            {
                uint8_t *ptr = reinterpret_cast<uint8_t *>(data);

                if ( m_areOutputRegistersConsecutive )
                {
                    RAM::Library::I2C::read(m_i2cAddr,
                                            m_outAddrLowNibble[0],
                                            reinterpret_cast<void *>(ptr),
                                            m_dimension * 2);
                }
                else
                {
                    for ( unsigned int i = 0; i < m_dimension; ++i, ptr += 2 )
                    {
                        ptr[0] = RAM::Library::I2C::read(m_i2cAddr, m_outAddrLowNibble[i]);
                        
                        ptr[1] = RAM::Library::I2C::read(m_i2cAddr, m_outAddrHighNibble[i]);
                    }
                }

                //
                // reset pointer to starting address
                //
                ptr = reinterpret_cast<uint8_t *>(data);
                
                //
                // LSB in first axis register
                //
                for ( unsigned int i = 0; i < m_dimension; ++i, ptr += 2 )
                {
                    data[i] = ((ptr[m_readoutByteMSB]<<8) | ptr[m_readoutByteLSB]);
                }
            }

            void readout(float (&data)[DIMENSION])
            {
                int16_t out[DIMENSION];
    
                readout(out);

                for ( unsigned int i = 0; i < DIMENSION; ++i )
                {
                    data[i] = (out[i] - m_offset[i]) * m_scale[i];
                }
            }

        protected:
            uint8_t m_dimension;

            uint8_t m_i2cAddr;

            uint8_t m_i2cDevIdReg;

            uint8_t m_i2cDevIdValue;

            bool m_areOutputRegistersConsecutive;

            bool m_isCalibrated;

            /** Readout of 16 bits integers data happens 
             */
            uint8_t m_readoutByteLSB;
            uint8_t m_readoutByteMSB;

            /** Count to physical unit.
             */
            float m_scale[DIMENSION];

            /** At rest output in counts.
             */
            int16_t m_offset[DIMENSION];

            /** RMS of those samples used to compute the offset at rest.
             */
            float m_offsetSigma[DIMENSION];

            /** Registers from where to read axes data.
             */
            uint8_t m_outAddrLowNibble[DIMENSION];
            uint8_t m_outAddrHighNibble[DIMENSION];
        };
    }
}

#endif /* _SensorBase_H_ */

/*___oOo__*/
