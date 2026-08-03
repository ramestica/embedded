#ifndef _GPIO_H_
#define _GPIO_H_

//
// System stuff
//
#include <inttypes.h>

//
// Target stuff
//
#include <target/Target.h>

namespace RAM
{
    namespace Target
    {
        namespace GPIO
        {
            /** Each port presents a DATA register, the number of actual bits or
             ** pins is represented by the following type.
             */
            typedef uint8_t DataType;
            
            struct Port
            {
                PortId m_portId;
                intptr_t m_pointer = 0;
                unsigned m_index = static_cast<unsigned>(PortId::IDN);
                IRQn_Type m_irqn;
            };

            enum class DirectionMode: unsigned
            {
                INPUT_PIN,
                OUTPUT_PIN
            };

            enum class FunctionId: unsigned
            {
                GPIO_FUNC = 0,
                UART_FUNC,
                TIMER_FUNC,
                PWM_FUNC,
                I2C_FUNC
            };
            
            /** Structure to encapsulate a specific pin or pins in a given
             ** port. Nothing prevents m_pin to include more than one bit ON,
             ** therefore, many bits can be addressed at the same time.
             */
            struct Pin: public Port
            {
                FunctionId m_functionId = FunctionId::GPIO_FUNC;

                /** If the GPIO pins are used as input/output of some other
                 ** module (timer, uart, i2c, etc.) then that function needs to
                 ** be communicated to the the microcontroller. This variable
                 ** record the actual value used for a Pin instance in
                 ** particular.
                 */
                uint8_t m_functionCode;

                /** A pin structure can address multiple pins in a GPIO module,
                 ** these masks record which bits go together.
                 */
                DataType m_inMask = 0;
                DataType m_outMask = 0;

                /** At configuration time some specific types might request
                 ** setting specific associated pins in open-drain mode.
                 */
                DataType m_openDrainMask = 0;
            };

            /** Configure pin as a gpio.
             */
            int configure(
                const PortId portId,
                Pin &pin,
                const DataType inMask,
                const DataType outMask,
                void (*isr)() = nullptr);

            /** Configure UART digital function pins.
             */
            int configure(
                const RAM::Target::UART::ModuleId uartId,
                Pin &pin);

            /** Configure Timer digital function pins.
             */
            int configure(
                const RAM::Target::Timer::ModuleId mid,
                const RAM::Target::Timer::SelectId selectId,
                Pin &pin);

            /** Configure PWM digital function pins.
             ** @param selectId if A or B the populate only one bit in the mask.
             */
            int configure(
                const PWM::GeneratorId generatorId,
                const PWM::SelectId selectId,
                Pin &pin);

            /** Configure I2C digital function pins.
             */
            int configure(
                const RAM::Target::I2C::ModuleId mid,
                Pin &pin);

            /** Enable pin based on settings from configuration.
             */
            int enable(const Pin &pin);

            /** Enable pin based on settings from configuration plus the
             ** specific pwm generator in the second parameter.
             */
            int enable(const Pin &pin, const PWM::SelectId selectId);
            
            /** Return pin to reset configuration.
             */
            int disable(const Pin &pin);

            /** Set input/output mode.
             */
            int setDirection(Pin &pin, const GPIO::DataType mask, const DirectionMode dir);

            /** Get pin number for a given timer identifier.
             */
            int getPortPin(
                const RAM::Target::Timer::ModuleId mid,
                const RAM::Target::Timer::SelectId sid,
                Pin &pin);

            namespace Digital
            {
                //
                // all pins can be read, no matter their in/out configuration.
                //
                inline DataType read(const Pin &pin)
                {
                    return
                        (0xff
                         &
                         reinterpret_cast<GPIO::TypeDef *>(pin.m_pointer)->
                         DATA[pin.m_inMask | pin.m_outMask]);
                }

                //
                // if value contains bits not on the output mask,
                // then, this method silently discards them.
                //
                inline void write(const Pin &pin, const DataType value)
                {
                    reinterpret_cast<GPIO::TypeDef *>(pin.m_pointer)->
                        DATA[pin.m_outMask] = (pin.m_outMask & value);
                }

                //
                // if value contains bits not on the output mask,
                // then, this method silently discards them.
                //
                inline void toggle(const Pin &pin, const DataType value = 0xff)
                {
                    reinterpret_cast<GPIO::TypeDef *>(pin.m_pointer)->DATA[pin.m_outMask & value] =
                        ~(reinterpret_cast<GPIO::TypeDef *>(pin.m_pointer)->DATA[pin.m_outMask & value]);
                }
            }

            namespace StatusLED
            {
                void enable();

                void on();

                void off();

                void toggle();
            }
        }
    }
}
                    
#endif /* _GPIO_H_ */

/*___oOo__*/
