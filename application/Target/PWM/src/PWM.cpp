/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Base stuff
//

//
// Target stuff
//
 
//
// System stuff
//
#include <initializer_list>
#include <math.h>

//
// Local stuff
//
#include <Target/PWM.h>

using namespace RAM::Target;

//
// PWM modules' addresses
//
static PWM0_Type *pwmp[static_cast<int>(PWM::ModuleId::IDN)] =
{
    PWM0,
    PWM1
};

static volatile RegType *xbctlp[static_cast<int>(PWM::GeneratorId::IDN)] =
{
    &PWM0->_0_CTL,
    &PWM0->_1_CTL,
    &PWM0->_2_CTL,
    &PWM0->_3_CTL,
    &PWM1->_0_CTL,
    &PWM1->_1_CTL,
    &PWM1->_2_CTL,
    &PWM1->_3_CTL
};

static volatile RegType *xctlp[static_cast<int>(PWM::GeneratorId::IDN)][2] =
{
    {&PWM0->_0_GENA, &PWM0->_0_GENB},
    {&PWM0->_1_GENA, &PWM0->_1_GENB},
    {&PWM0->_2_GENA, &PWM0->_2_GENB},
    {&PWM0->_3_GENA, &PWM0->_3_GENB},
    {&PWM1->_0_GENA, &PWM1->_0_GENB},
    {&PWM1->_1_GENA, &PWM1->_1_GENB},
    {&PWM1->_2_GENA, &PWM1->_2_GENB},
    {&PWM1->_3_GENA, &PWM1->_3_GENB}
};

static volatile RegType *xloadp[static_cast<int>(PWM::GeneratorId::IDN)] =
{
    &PWM0->_0_LOAD,
    &PWM0->_1_LOAD,
    &PWM0->_2_LOAD,
    &PWM0->_3_LOAD,
    &PWM1->_0_LOAD,
    &PWM1->_1_LOAD,
    &PWM1->_2_LOAD,
    &PWM1->_3_LOAD
};

static volatile RegType *xcompp[static_cast<int>(PWM::GeneratorId::IDN)][2] =
{
    {&PWM0->_0_CMPA, &PWM0->_0_CMPB},
    {&PWM0->_1_CMPA, &PWM0->_1_CMPB},
    {&PWM0->_2_CMPA, &PWM0->_2_CMPB},
    {&PWM0->_3_CMPA, &PWM0->_3_CMPB},
    {&PWM1->_0_CMPA, &PWM1->_0_CMPB},
    {&PWM1->_1_CMPA, &PWM1->_1_CMPB},
    {&PWM1->_2_CMPA, &PWM1->_2_CMPB},
    {&PWM1->_3_CMPA, &PWM1->_3_CMPB},
};

//-------------------------------------------------------------------------------
int PWM::configure(
    const GeneratorId gid,
    const SelectId sid,
    const unsigned periodus,
    Generator &generator)
{
    if ( gid == GeneratorId::IDN )
    {
        return 1;
    }

    if ( gid < GeneratorId::PWMGENID4 )
    {
        generator.m_module.m_id = ModuleId::PWMID0;
    }
    else
    {
        generator.m_module.m_id = ModuleId::PWMID1;
    }

    generator.m_sid  = sid;
    
    generator.m_module.m_index = unsigned(generator.m_module.m_id);
    
    generator.m_module.m_pointer =
        reinterpret_cast<intptr_t>(pwmp[generator.m_module.m_index]);

    generator.m_id = gid;
    
    generator.m_index = unsigned(generator.m_id);

    generator.m_periodus = periodus;
    
    //
    // divider to use is the smaller one that's able to accommodate the given
    // period
    //
    for ( unsigned i = 0; i <= 6; ++i )
    {
        generator.m_divider = (0x1UL << i);
        
        //
        // microseconds in a full 16 bit timer loop at system clock frequency
        // divided by the divisor. Dropping the decimal part does not matter
        // because the input parameter are round microseconds.
        //
        if ( generator.m_periodus
             <=
             (generator.m_divider * ((0x1UL << 16) - 1)) / F_CPU_CYCLES_PER_US )
        {
            break;
        }
        else if ( generator.m_divider == 64 )
        {
            //
            // if the given period is too long then complain
            //
            return 1;
        }
    }

    //
    // desired period is given in microseconds, if the cpu frequency combined
    // with the selected divisor does not allow for an integer period then error
    //
    if (  generator.m_divider
          *
          unsigned((generator.m_periodus * F_CPU_CYCLES_PER_US)
                   /
                   generator.m_divider)
         !=
         generator.m_periodus * F_CPU_CYCLES_PER_US )
    {
        return 1;
    }

    //
    // keep track of the actual load value
    //
    generator.m_load =
        (generator.m_periodus * F_CPU_CYCLES_PER_US) / generator.m_divider - 1;

    //
    // configure ??? signal(s) in associated pwm module
    //
    if ( GPIO::configure(generator.m_id, generator.m_sid, generator.m_pins) )
    {
        return 1;
    }

    return 0;
}

//-------------------------------------------------------------------------------
int PWM::start(
    const Generator &generator,
    float dutyCycle[2],
    void (*isr)())
{
    if ( generator.m_sid != SelectId::PWMBOTH )
    {
        return 1;
    }
        
    if ( dutyCycle[0] < 0 || dutyCycle[0] > 1.
         ||
         dutyCycle[1] < 0 || dutyCycle[1] > 1. )
    {
        return 1;
    }
    
    //
    // enable module (RCGCPWM register see page 354)
    //
    if ( (SYSCTL->RCGCPWM & (0x1 << generator.m_module.m_index)) == 0 )
    {
        SYSCTL->RCGCPWM |= (0x1 << generator.m_module.m_index); 

        //
        // wait until module ready
        //
        while ( (SYSCTL->PRPWM & (0x1 << generator.m_module.m_index)) == 0 );
    }

    if ( GPIO::enable(generator.m_pins, generator.m_sid) )
    {
        return 1;
    }
    
    //
    // Tiva TM4C123 parts seem to share the same clock source on both PWM
    // modules, hence the configuration through SYSCTL and not through RCC on
    // each module.
    //
    if ( generator.m_divider == 1 )
    {
        SYSCTL->RCC &= ~SYSCTL_RCC_USEPWMDIV;
        
        //
        // do not care about the divisor setting
        //
    }
    else
    {
        SYSCTL->RCC |= SYSCTL_RCC_USEPWMDIV;
        
        SYSCTL->RCC &= ~SYSCTL_RCC_PWMDIV_M;

        SYSCTL->RCC |=
            ((unsigned(log2(generator.m_divider)) - 1) << SYSCTL_RCC_PWMDIV_B);
    }

    //
    // avoid touching PWMnCTL, in default mode the timer counts down
    //
    
    //
    // disable the generator block while setting up
    //
    *xbctlp[generator.m_index] = 0x0;

    //
    // write load value
    //
    *xloadp[generator.m_index] = generator.m_load;

    //
    // set duty cycle value and enable individual side A or B generator(s)
    //
    *xcompp[generator.m_index][0] = (1. - dutyCycle[0]) * generator.m_load;
    *xcompp[generator.m_index][1] = (1. - dutyCycle[1]) * generator.m_load;

    //
    // ACTCMPAD=2 Action for Comparator A Down -> Drive pwmA Low
    // ACTLOAD=3  Action for Counter=LOAD -> Drive pwmA High
    //
    *xctlp[generator.m_index][0] = 0x08c;
    *xctlp[generator.m_index][1] = 0x80c;

    //
    // enable the generator block
    //
    *xbctlp[generator.m_index] |= PWM_n_CTL_ENABLE;

    //
    // enable the output
    //
    reinterpret_cast<PWM0_Type *>(generator.m_module.m_pointer)->ENABLE
        |=
        (generator.m_id < GeneratorId::PWMGENID4)
        ?
        0x3 << (generator.m_index * 2)
        :
        0x3 << ((generator.m_index - unsigned(GeneratorId::PWMGENID4)) * 2);
    
    return 0;
}

//-------------------------------------------------------------------------------
int PWM::setDutyCycle(
    const Generator &generator,
    const SelectId sid,
    float dutyCycle)
{
    if ( generator.m_sid != SelectId::PWMBOTH
         &&
         generator.m_sid != sid )
    {
        return 1;
    }
    
    if ( dutyCycle < 0 || dutyCycle > 1. )
    {
        return 1;
    }

    //
    // set duty cycle value
    //
    if ( sid == SelectId::PWMA || sid == SelectId::PWMBOTH )
    {
        *xcompp[generator.m_index][0] =
            (1. - dutyCycle) * generator.m_load;
    }
    if ( sid == SelectId::PWMB || sid == SelectId::PWMBOTH )
    {
        *xcompp[generator.m_index][1] =
            (1. - dutyCycle) * generator.m_load;
    }

    return 0;
}

/*___oOo___*/
