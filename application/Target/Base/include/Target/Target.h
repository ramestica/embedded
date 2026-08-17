#ifndef _Base_H_
#define _Base_H_

//
// System stuff
//
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#ifdef TARGET_nano328p
#include <avr/io.h>
#include <avr/interrupt.h>
#elif defined(TARGET_tivac) || defined(TARGET_minim4)
#endif

//
// Target stuff
//
#ifdef TARGET_nano328p
#include <HardwareSerial.h>
//#include <Arduino.h>
#elif defined(TARGET_tivac) || defined(TARGET_minim4)
#include <Target/tm4c123gh6pm.h>
#include <Target/SystemView.h>
#elif defined(TARGET_discovery)
#include <stm32f407vgt6.h>
#include <Target/SystemView.h>
#endif

//
// RAM stuff
// 

//
// Local stuff
//
namespace RAM
{
    namespace Target
    {
#ifdef TARGET_nano328p
        typedef uint8_t RegType;
#elif defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
        typedef uint32_t RegType;
#endif
        typedef volatile RegType * RegAddrType;

        namespace GPIO
        {
#if defined(TARGET_tivac) || defined(TARGET_minim4)
            typedef GPIOA_Type TypeDef; 
#elif defined(TARGET_discovery)
            typedef GPIO_TypeDef TypeDef;
#endif

            enum class PortId: unsigned
            {
                GPIOIDA = 0,
                GPIOIDB,
                GPIOIDC,
                GPIOIDD,
                GPIOIDE,
                GPIOIDF,
#ifdef TARGET_discovery
                GPIOIDG,
                GPIOIDH,
                GPIOIDI,
#endif
                IDN
            };
        }

        namespace UART
        {
            enum class ModuleId: unsigned
            {
                UARTID0 = 0,
                UARTID1,
#ifndef TARGET_dicovery
                UARTID2,
                UARTID3,
                UARTID4,
                UARTID5,
                UARTID6,
                UARTID7,
#endif
                IDN
            };
        }
        
        namespace Timer
        {
            /** Timer modules.
             */
            enum class ModuleId: unsigned
            {
                TIMID0 = 0,
                TIMID1,
                TIMID2,
#if defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
                TIMID3,
                TIMID4,
                TIMID5,
                TIMID6,
                TIMID7,
                TIMID8,
                TIMID9,
                TIMID10,
                TIMID11,
#endif
#ifndef TARGET_dicovery
                TIMID12,
                TIMID13,
#endif
                IDN
             };
            
            /** Timer within a timer module.
             */
            enum class SelectId: unsigned
            {
                TIMERA = 0,
                TIMERB,
                TIMERBOTH
            };
        }

        namespace PWM
        {
            /** PWM modules.
             */
            enum class ModuleId: unsigned
            {
                PWMID0 = 0,
                PWMID1,
                IDN
            };

            /** PWM generators.
             */
            enum class GeneratorId: unsigned
            {
                PWMGENID0 = 0,
                PWMGENID1,
                PWMGENID2,
                PWMGENID3,
                PWMGENID4,
                PWMGENID5,
                PWMGENID6,
                PWMGENID7,
                IDN
            };

            /** PWM within a PWM generator (they share same timer).
             */
            enum class SelectId: unsigned
            {
                PWMA = 0,
                PWMB,
                PWMBOTH
            };
        }

        namespace I2C
        {
            /** I2C modules.
             */
            enum class ModuleId: unsigned
            {
                I2CID0 = 0,
                I2CID1,
                I2CID2,
                I2CID3,
                IDN
             };
        }        
    }
}

#endif /* _Base_H_ */

/*___oOo__*/
