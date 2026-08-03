#ifndef _TargetI2C_H_
#define _TargetI2C_H_

//
// System stuff
//
#include <stddef.h>
#include <inttypes.h>

//
// Target stuff
//
#include <GPIO.h>

namespace RAM
{
    namespace Target
    {
        namespace I2C
        {
            struct Module
            {
                ModuleId m_id;
                intptr_t m_pointer;
                unsigned m_index = static_cast<unsigned>(ModuleId::IDN);
                IRQn_Type m_irqn;
                RAM::Target::GPIO::Pin m_pins;
            };

            int configure(
                const ModuleId moduleId,
                const unsigned kbps,
                const bool enableInternalPullup,
                Module &module);

            size_t read(
                const Module &module,
                const uint8_t addr,
                const uint8_t reg,
                void *buf,
                const size_t size);
            
            size_t write(
                const Module &module,
                const uint8_t addr,
                const uint8_t reg,
                const void *buf,
                const size_t size);

            void read(
                const Module &module,
                const bool start,
                const uint8_t addr,
                void *buf,
                const size_t size);

            void write(
                const Module &module,
                const bool start,
                const uint8_t addr,
                const void *buf,
                const size_t size);

            bool ping(const Module &module, uint8_t addr);

            bool wait(const Module &module);

            void stop(const Module &module);
        }
    }
}

#endif /* _TargetI2C_H_ */

/*___oOo__*/
