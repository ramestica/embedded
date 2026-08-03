#ifndef _Device_H_
#define _Device_H_

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
#include <Sensor.h>

namespace RAM
{
    namespace Library
    {
        namespace Device
        {
            class Base
            {
            public:
                enum SensorId
                {
                    SENSOR0
                };

                unsigned int addSensor(Sensor::Base *sensor);

                Sensor::Base * operator [const SensorId id] const;
                
                virtual void initialize() = 0;

                virtual void initialize(const SensorId sid) = 0;

                virtual void calibrate(const SensorId sid) = 0;

                virtual void sample() = 0;

                virtual bool checkPresence() const = 0;

            private:
                Sensor::Base *m_sensors_p;
                
            };
            
            class I2C: public Base
            {
            public:
                I2C(const uint8_t i2cAddr,
                    const uint8_t i2cDevIdReg,
                    const uint8_t i2cDevIdValue);
                
                bool checkPresence() const;
                {
                    if ( (m_i2cDevIdReg != 0x0 || m_i2cDevIdValue != 0x0)
                         &&
                         RAM::Library::I2C::read(m_i2cAddr, m_i2cDevIdReg) != m_i2cDevIdValue )
                    {
                        return false;
                    }
                    
                    return true;
                }
                
            private:
                uint8_t m_i2cAddr;
                
                uint8_t m_i2cDevIdReg;
                
                uint8_t m_i2cDevIdValue;
                
            };
            
            class SPI: public Base
            {
            };
        }
    }
}

#endif /* _Device_H_ */

/*___oOo__*/
