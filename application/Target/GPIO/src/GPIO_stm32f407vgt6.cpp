/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Utils.h>
#include <Interrupts.h>

//
// System stuff
//
#include <string.h>

//
// Local stuff
//
#include "GPIO.h"

using namespace std;
using namespace RAM::Target;

//
// Ports' addresses
//
static GPIO::TypeDef *gpiop[static_cast<int>(GPIO::PortId::IDN)] =
{
    GPIOA_AHB,
    GPIOB_AHB,
    GPIOC_AHB,
    GPIOD_AHB,
    GPIOE_AHB,
    GPIOF_AHB 
};

//
// IRQ numbers for each GPIO port
//
static IRQn_Type irqNumber[static_cast<int>(GPIO::PortId::IDN)] =
{
    GPIOA_IRQn, 
    GPIOB_IRQn, 
    GPIOC_IRQn, 
    GPIOD_IRQn, 
    GPIOE_IRQn, 
    GPIOF_IRQn 
};

//
// GPIO port associated to each UART module
//
static GPIO::PortId uartPort[static_cast<int>(UART::ModuleId::IDN)] =
{
    GPIO::PortId::GPIOIDA,
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDD,
    GPIO::PortId::GPIOIDC,
    GPIO::PortId::GPIOIDC,
    GPIO::PortId::GPIOIDE,
    GPIO::PortId::GPIOIDD,
    GPIO::PortId::GPIOIDE 
};

//
// GPIO pin bits (rx/tx) within each GPIO port associated to each UART module.
//
static uint8_t uartPortBits[static_cast<int>(UART::ModuleId::IDN)][2] =
{
    {0, 1},
    {0, 1},
    {6, 7},
    {6, 7},
    {4, 5},
    {4, 5},
    {4, 5},
    {0, 1}
};

//
// GPIO port associated to each Timer module. T0, T1 and T2 could be routed to
// GPIO F as well. Hardcoded to port B here.
//
static GPIO::PortId timerPort[static_cast<int>(Timer::ModuleId::IDN)] =
{
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDC,
    GPIO::PortId::GPIOIDC,
    GPIO::PortId::GPIOIDC,
    GPIO::PortId::GPIOIDC,
    GPIO::PortId::GPIOIDD,
    GPIO::PortId::GPIOIDD,
    GPIO::PortId::GPIOIDD,
    GPIO::PortId::GPIOIDD
};

//
// GPIO pin bits (A/B) within each GPIO port associated to each Timer module.
//
static uint8_t timerPortBits[static_cast<int>(Timer::ModuleId::IDN)][2] =
{
    {6, 7},
    {4, 5},
    {0, 1},
    {2, 3},
    {0, 1},
    {2, 3},
    {4, 5},
    {6, 7},
    {0, 1},
    {2, 3},
    {4, 5},
    {6, 7}
};

//
// GPIO port associated to each Timer module. T0, T1 and T2 could be routed to
// GPIO F as well. Hardcoded to port B here.
//
static GPIO::PortId pwmGeneratorPort[static_cast<int>(PWM::GeneratorId::IDN)] =
{
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDE,
    GPIO::PortId::GPIOIDC, /* it could also be D */
    GPIO::PortId::GPIOIDD,
    GPIO::PortId::GPIOIDA, /* it could also be E */    
    GPIO::PortId::GPIOIDF,
    GPIO::PortId::GPIOIDF,
};

//
// GPIO pin bits (A/B) within each GPIO port associated to each Timer module.
//
static uint8_t pwmGeneratorPortBits[static_cast<int>(PWM::GeneratorId::IDN)][2] =
{
    {6, 7},
    {4, 5},
    {4, 5},
    {4, 5},
    {0, 1},
    {6, 7},
    {0, 1},
    {2, 3}
};

//
// GPIO port associated to each I2C module
//
static GPIO::PortId i2cPort[static_cast<int>(I2C::ModuleId::IDN)] =
{
    GPIO::PortId::GPIOIDB,
    GPIO::PortId::GPIOIDA,
    GPIO::PortId::GPIOIDE,
    GPIO::PortId::GPIOIDD
};

//
// GPIO pin bits (scl/sda) within each GPIO port associated to each I2C module.
//
static uint8_t i2cPortBits[static_cast<int>(I2C::ModuleId::IDN)][2] =
{
    {2, 3},
    {6, 7},
    {4, 5},
    {0, 1}
};

/** General enable function used by more specific and exported functions.
 ** @param pin
 ** @param mask sub-set of bits to enable.
 ** @param dir in case of GPIO functionality then set in/out based on dir.
 */
static int enable(
    const GPIO::Pin &pin,
    const GPIO::DataType inMask,
    const GPIO::DataType outMask,
    void (*isr)())
{
    if ( pin.m_portId == GPIO::PortId::IDN )
    {
        return 1;
    }

    //
    // if the given masks are empty or if they overlap, or their bits
    // do not match those in the pin structure then something is wrong
    //
    if ( (inMask == 0 && outMask == 0)
         ||
         (inMask & outMask)
         ||
         ((inMask | outMask) & ~(pin.m_inMask | pin.m_outMask)) )
    {
        return 1;
    }
    
    //
    // if the associated GPIO module is not enabled then enable it now
    // (see page 340) and set it to use AHB
    //
    if ( (SYSCTL->RCGCGPIO & (0x1 << pin.m_index)) == 0 )
    {
        SYSCTL->RCGCGPIO |= (0x1 << pin.m_index);

        //
        // wait until module ready
        //
        while ( (SYSCTL->PRGPIO & (0x1 << pin.m_index)) == 0 );

        //
        // AHB instead of APB (only if the used address opening is ahb)
        //
        if ( gpiop[pin.m_index] >= reinterpret_cast<GPIOA_Type *>(GPIOA_AHB_BASE) )
        {
            SYSCTL->GPIOHBCTL |= (0x1 << pin.m_index);
        }
    }

    //
    // PA[1:0], PA[5:2] and PF[0] have Special Considerations (see
    // table 10.1), if required then unlock it and set commit bit
    //
    if ( ((pin.m_portId == GPIO::PortId::GPIOIDA) && ((inMask | outMask) & 0x3f))
         ||
         ((pin.m_portId == GPIO::PortId::GPIOIDF) && ((inMask | outMask) & 0x1)) )
    {
        //
        // see magic number on page 684
        //
        gpiop[pin.m_index]->LOCK = 0x4C4F434B;

        gpiop[pin.m_index]->CR |=
            (pin.m_portId == GPIO::PortId::GPIOIDA
             ?
             ((inMask | outMask) & 0x3f)
             :
             ((inMask | outMask) & 0x1));
    }

    //
    // if the pins are already enable then complain, it looks like the pin(s) is
    // used two times
    //
    if ( gpiop[pin.m_index]->DEN & (inMask | outMask) )
    {
        return 1;
    }
    
    //
    // pin function coding when gpio and not gpio
    //
    if ( pin.m_functionId == GPIO::FunctionId::GPIO_FUNC )
    {
        //
        // disable alternate function selector, the pin is a gpio
        //
        gpiop[pin.m_index]->AFSEL &= ~(inMask | outMask);

        //
        // set input direction
        //
        gpiop[pin.m_index]->DIR &= ~inMask;

        //
        // set output direction
        //
        gpiop[pin.m_index]->DIR |= outMask;

        if ( isr != nullptr )
        {
            // From TM4C123GH6PM datasheet
            //
            // Note: To prevent false interrupts, the following
            // steps should be taken when re-configuring GPIO edge
            // and interrupt sense registers:
            //
            // 1. Mask the corresponding port by clearing the IME
            //    field in the GPIOIM register.
            // 2. Configure the IS field in the GPIOIS register
            //    and the IBE field in the GPIOIBE register.
            // 3. Clear the GPIORIS register.
            // 4. Unmask the port by setting the IME field in the
            //    GPIOIM register.
    
            //
            // mask interrupt before proceeding (avoid spurious trigger
            // while accessing interrupt related registers)
            //
            gpiop[pin.m_index]->IM &= ~(inMask | outMask);
                    
            Interrupts::setHandler(pin.m_irqn, isr);

            Interrupts::enableIRQ(pin.m_irqn);

            gpiop[pin.m_index]->IEV |= (inMask | outMask);

            gpiop[pin.m_index]->IM |= (inMask | outMask);
        }
    }
    else
    {
        RegType pctl = 0;
        uint8_t bidx;

        //
        // note that for altenate functionality only input pins matter
        //
        
        //
        // set as many pin configuration codes as many bits in the mask
        //
        for ( unsigned i = 0; i < sizeof(GPIO::DataType) * 8; ++i )
        {
            if ( (bidx = RAM::Support::Utils::getBitIndex(i, inMask)) == 8 )
            {
                break;
            }

            pctl |= (pin.m_functionCode << (4 * bidx));
        }
    
        //
        // apply pin function coding
        //
        gpiop[pin.m_index]->PCTL |= pctl;

        //
        // GPIO pins used by a module must be set as input 
        //
        gpiop[pin.m_index]->DIR &= ~(inMask);

        //
        // set alternate function bits on
        //
        gpiop[pin.m_index]->AFSEL |= inMask;
    }
    
    //
    // 2mA drive (it is the default actually)
    //
    gpiop[pin.m_index]->DR2R |= (inMask | outMask);

    //
    // open-drain mode as set during configuration
    //
    gpiop[pin.m_index]->ODR |= pin.m_openDrainMask;
    
    //
    // enable digital function of each pin
    //
    gpiop[pin.m_index]->DEN |= (inMask | outMask);
    
    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::configure(
    const PortId portId,
    Pin &pin,
    const uint8_t inMask,
    const uint8_t outMask,
    void (*isr)())
{
    if ( portId == PortId::IDN )
    {
        return 1;
    }

    pin.m_portId = portId;

    pin.m_index = unsigned(portId);
    
    pin.m_pointer = reinterpret_cast<intptr_t>(gpiop[pin.m_index]);
    
    pin.m_irqn = irqNumber[pin.m_index];

    pin.m_inMask = inMask;

    pin.m_outMask = outMask;

    return ::enable(pin, pin.m_inMask, pin.m_outMask, isr);
}

//-------------------------------------------------------------------------------
int GPIO::configure(
    const RAM::Target::UART::ModuleId uartId,
    Pin &pin)
{
    if ( uartId == UART::ModuleId::IDN )
    {
        return 1;
    }
    
    pin.m_portId = uartPort[unsigned(uartId)];
    
    pin.m_index = unsigned(pin.m_portId);
    
    pin.m_pointer = reinterpret_cast<intptr_t>(gpiop[pin.m_index]);
    
    pin.m_irqn = irqNumber[pin.m_index];

    pin.m_inMask =
        (1 << uartPortBits[unsigned(uartId)][0])
        |
        (1 << uartPortBits[unsigned(uartId)][1]);
    
    pin.m_functionId = FunctionId::UART_FUNC;
    
    pin.m_functionCode = 0x1;

    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::configure(
    const Timer::ModuleId moduleId,
    const Timer::SelectId selectId,
    Pin &pin)
{
    // https://sites.google.com/site/narasimhaweb/projects/pwm-on-stellaris-launchpad

    if ( moduleId == Timer::ModuleId::IDN )
    {
        return 1;
    }
    
    pin.m_portId = timerPort[unsigned(moduleId)];
    
    pin.m_index = unsigned(pin.m_portId);
    
    pin.m_pointer = reinterpret_cast<intptr_t>(gpiop[pin.m_index]);
    
    pin.m_irqn = irqNumber[pin.m_index];

    if ( selectId == Timer::SelectId::TIMERA || selectId == Timer::SelectId::TIMERBOTH )
    {
        pin.m_inMask |=
            (1 << timerPortBits[unsigned(moduleId)][0]);
    }
    if ( selectId == Timer::SelectId::TIMERB || selectId == Timer::SelectId::TIMERBOTH )
    {
        pin.m_inMask |=
            (1 << timerPortBits[unsigned(moduleId)][1]);
    }

    pin.m_functionId = FunctionId::TIMER_FUNC;
    
    pin.m_functionCode = 0x7;
    
    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::configure(
    const PWM::GeneratorId generatorId,
    const PWM::SelectId selectId,
    Pin &pin)
{
    if ( generatorId == PWM::GeneratorId::IDN )
    {
        return 1;
    }

    pin.m_portId = pwmGeneratorPort[unsigned(generatorId)];
    
    pin.m_index = unsigned(pin.m_portId);
    
    pin.m_pointer = reinterpret_cast<intptr_t>(gpiop[pin.m_index]);
    
    pin.m_irqn = irqNumber[pin.m_index];

    if ( selectId == PWM::SelectId::PWMA || selectId == PWM::SelectId::PWMBOTH )
    {
        pin.m_inMask |=
            (1 << pwmGeneratorPortBits[unsigned(generatorId)][0]);
    }
    if ( selectId == PWM::SelectId::PWMB || selectId == PWM::SelectId::PWMBOTH )
    {
        pin.m_inMask |=
            (1 << pwmGeneratorPortBits[unsigned(generatorId)][1]);
    }
    
    pin.m_functionId = FunctionId::PWM_FUNC;

    if ( generatorId < PWM::GeneratorId::PWMGENID4 )
    {
        pin.m_functionCode = 4;
    }
    else
    {
        pin.m_functionCode = 5;
    }
        
    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::configure(
    const RAM::Target::I2C::ModuleId moduleId,
    Pin &pin)
{
    if ( moduleId == I2C::ModuleId::IDN )
    {
        return 1;
    }
    
    pin.m_portId = i2cPort[unsigned(moduleId)];
    
    pin.m_index = unsigned(pin.m_portId);
    
    pin.m_pointer = reinterpret_cast<intptr_t>(gpiop[pin.m_index]);
    
    pin.m_irqn = irqNumber[pin.m_index];

    pin.m_inMask =
        (1 << i2cPortBits[unsigned(moduleId)][0])
        |
        (1 << i2cPortBits[unsigned(moduleId)][1]);

    pin.m_openDrainMask = (0x1 << i2cPortBits[unsigned(moduleId)][1]);
        
    pin.m_functionId = FunctionId::I2C_FUNC;
    
    pin.m_functionCode = 0x3;

    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::enable(const Pin &pin)
{
    //
    // so far the only type of functionality that cannot use this method is pwm
    //
    if ( pin.m_functionId == FunctionId::PWM_FUNC )
    {
        return 1;
    }

    //
    // pins that are enable by this method use the full mask set at
    // configuration
    //
    return ::enable(pin, pin.m_inMask, pin.m_outMask, nullptr); 
}

//-------------------------------------------------------------------------------
int GPIO::enable(const Pin &pin, const PWM::SelectId selectId)
{
    if ( pin.m_functionId != FunctionId::PWM_FUNC )
    {
        return 1;
    }
    
    //
    // actual pins mask to configure
    //
    RegType mask = 0;

    //
    // configure pwm function for requested pins
    //
    if ( selectId == PWM::SelectId::PWMA
         ||
         selectId == PWM::SelectId::PWMBOTH )
    {
        mask |= (0x1 << RAM::Support::Utils::getBitIndex(0, pin.m_inMask));
    }
    if ( selectId == PWM::SelectId::PWMB
         ||
         selectId == PWM::SelectId::PWMBOTH )
    {
        mask |= (0x1 << RAM::Support::Utils::getBitIndex(1, pin.m_inMask));
    }

    return ::enable(pin, mask, 0, nullptr);
}

//-------------------------------------------------------------------------------
int GPIO::disable(const Pin &pin)
{
    //
    // disable digital function of each pin
    //
    gpiop[pin.m_index]->DEN &= ~(pin.m_inMask | pin.m_outMask);

    //
    // PA[1:0], PA[5:2] and PF[0] have Special Considerations (see
    // table 10.1), if required then unlock it and set commit bit
    //
    if ( ((pin.m_portId == GPIO::PortId::GPIOIDA) && ((pin.m_inMask | pin.m_outMask) & 0x3f))
         ||
         ((pin.m_portId == GPIO::PortId::GPIOIDF) && ((pin.m_inMask | pin.m_outMask) & 0x1)) )
    {
        //
        // uncommit
        //
        gpiop[pin.m_index]->CR &= ~(pin.m_portId == GPIO::PortId::GPIOIDA ? ((pin.m_inMask | pin.m_outMask) & 0x3f) : ((pin.m_inMask | pin.m_outMask) & 0x1));

        //
        // lock it
        //
        gpiop[pin.m_index]->LOCK = 0x0;
    }

    return 0;
}

//-------------------------------------------------------------------------------
int GPIO::setDirection(Pin &pin, const GPIO::DataType mask, const DirectionMode dir)
{
    if ( ((pin.m_inMask | pin.m_outMask) | mask) != (pin.m_inMask | pin.m_outMask) )
    {
        return 1;
    }

    if ( dir == GPIO::DirectionMode::INPUT_PIN )
    {
        gpiop[pin.m_index]->DIR &= ~(mask);

        pin.m_inMask |= mask;

        pin.m_outMask &= ~mask;
    }
    else
    {
        gpiop[pin.m_index]->DIR |= mask;

        pin.m_inMask &= ~mask;

        pin.m_outMask |= mask;
    }

    return 0;
}

/*___oOo___*/
