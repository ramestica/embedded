//
// System stuff
//
#include <math.h>

//
//#include <timer.h>
//
#include <Target/Interrupts.h>
#include <Target/GPIO.h>

//
// Local stuff
//
#include <Target/Timer.h>

using namespace std;
using namespace RAM::Target;

//
// Timers' adresses
//
static TIMER0_Type *timerp[static_cast<int>(Timer::ModuleId::IDN)] =
{
    TIMER0,
    TIMER1,
    TIMER2,
    TIMER3,
    TIMER4,
    TIMER5,
    WTIMER0,
    WTIMER1,
    WTIMER2,
    WTIMER3,
    WTIMER4,
    WTIMER5
};

//
// IRQ numbers for each timer within each module (A and B timers)
//
static IRQn_Type irqNumber[static_cast<int>(Timer::ModuleId::IDN)][2] =
{
    { TIMER0A_IRQn, TIMER0B_IRQn},
    { TIMER1A_IRQn, TIMER1B_IRQn},
    { TIMER2A_IRQn, TIMER2B_IRQn},
    { TIMER3A_IRQn, TIMER3B_IRQn},
    { TIMER4A_IRQn, TIMER4B_IRQn},
    { TIMER5A_IRQn, TIMER5B_IRQn},
    { WTIMER0A_IRQn, WTIMER0B_IRQn},
    { WTIMER1A_IRQn, WTIMER1B_IRQn},
    { WTIMER2A_IRQn, WTIMER2B_IRQn},
    { WTIMER3A_IRQn, WTIMER3B_IRQn},
    { WTIMER4A_IRQn, WTIMER4B_IRQn},
    { WTIMER5A_IRQn, WTIMER5B_IRQn}
};

static volatile RegType *rcgcp[static_cast<int>(Timer::ModuleId::IDN)] =
{
    &SYSCTL->RCGCTIMER,
    &SYSCTL->RCGCTIMER,
    &SYSCTL->RCGCTIMER,
    &SYSCTL->RCGCTIMER,
    &SYSCTL->RCGCTIMER,
    &SYSCTL->RCGCTIMER,
    &SYSCTL->RCGCWTIMER,
    &SYSCTL->RCGCWTIMER,
    &SYSCTL->RCGCWTIMER,
    &SYSCTL->RCGCWTIMER,
    &SYSCTL->RCGCWTIMER,
    &SYSCTL->RCGCWTIMER
};

static volatile RegType *prp[static_cast<int>(Timer::ModuleId::IDN)] =
{
    &SYSCTL->PRTIMER,
    &SYSCTL->PRTIMER,
    &SYSCTL->PRTIMER,
    &SYSCTL->PRTIMER,
    &SYSCTL->PRTIMER,
    &SYSCTL->PRTIMER,
    &SYSCTL->PRWTIMER,
    &SYSCTL->PRWTIMER,
    &SYSCTL->PRWTIMER,
    &SYSCTL->PRWTIMER,
    &SYSCTL->PRWTIMER,
    &SYSCTL->PRWTIMER
};

//-------------------------------------------------------------------------------
int RAM::Target::Timer::configure(
    const ModuleId moduleId,
    const Mode mode,
    const bool isSplit,
    Module &module)
{
    if ( moduleId == ModuleId::IDN )
    {
        return 1;
    }

    module.m_id = moduleId;
    
    module.m_index = unsigned(moduleId);
    
    module.m_pointer = reinterpret_cast<intptr_t>(timerp[module.m_index]);
    
    module.m_irqn[0] = irqNumber[module.m_index][0];

    module.m_irqn[1] = irqNumber[module.m_index][1];

    module.m_isSplit = isSplit;

    if ( module.m_isSplit )
    {
        module.m_bits =
            module.m_id <= ModuleId::TIMID5
            ?
            16
            :
            32;
    }
    else
    {
        module.m_bits =
            module.m_id <= ModuleId::TIMID5
            ?
            32
            :
            64;
    }
    
    //
    // enable module (RCGC(W)TIMER register see page 338 and 357)
    //
    unsigned rbit =
        module.m_id <= ModuleId::TIMID5
        ?
        (0x1 << module.m_index)
        :
        (0x1 << (module.m_index - unsigned(ModuleId::TIMID6)));
    if ( (*rcgcp[module.m_index] & rbit) == 0 )
    {
        *rcgcp[module.m_index] |= rbit; 
        
        //
        // wait until module ready
        //
        while ( (*prp[module.m_index] & rbit) == 0 );
    }
    else
    {
        return 1;
    }
    
    //
    // while configuring timers keep them disabled and then apply
    // configuration mode 16/32 or 32/64 bits
    //
    if ( module.m_isSplit )
    {
        timerp[module.m_index]->CTL &= ~(GPTM_CTL_TAEN | GPTM_CTL_TBEN);
        
        timerp[module.m_index]->CFG = 0x4;
    }
    else
    {
        timerp[module.m_index]->CTL &= ~(GPTM_CTL_TAEN | GPTM_CTL_TBEN);

        timerp[module.m_index]->CFG = 0x0;
    }

    switch ( mode )
    {
    case Mode::Periodic:
        //
        // enable interrupts, in case of a full counter is timer A the one to
        // address. Count up.
        //
        if ( module.m_isSplit )
        {
            timerp[module.m_index]->IMR |=
                GPTM_IMR_TATOIM | GPTM_IMR_TBTOIM;
            
            timerp[module.m_index]->TAMR =
                GPTM_TAMR_TACDIR | 0x2;

            timerp[module.m_index]->TBMR =
                GPTM_TBMR_TBCDIR | 0x2;
        }
        else
        {
            timerp[module.m_index]->IMR |= GPTM_IMR_TATOIM;

            timerp[module.m_index]->TAMR =
                GPTM_TAMR_TACDIR | 0x2;
        }
        
        break;

    case Mode::InputEdgeTime:
        //
        // edge detection must be split, see Table
        // 11-3. General-Purpose Timer Capabilities
        //
        if ( !module.m_isSplit )
        {
            return 1;
        }

        //
        // configure gpio pins (no gpio + input)
        //
        if ( GPIO::configure(module.m_id, SelectId::TIMERBOTH, module.m_pins) )
        {
            return 1;
        }

        //
        // enable gpio
        //
        if ( GPIO::enable(module.m_pins) )
        {
            return 1;
        }

        //
        // enable capture mode event interrupt
        //
        timerp[module.m_index]->IMR |= GPTM_IMR_CAEIM | GPTM_IMR_CBEIM;

        //
        // capture mode is edge-time, direction always up
        //
        timerp[module.m_index]->TAMR = GPTM_TAMR_TACMR | GPTM_TAMR_TACDIR | 0x3;
        timerp[module.m_index]->TBMR = GPTM_TBMR_TBCMR | GPTM_TBMR_TBCDIR | 0x3;

        //
        // triger on rising edges
        //
        timerp[module.m_index]->CTL |= GPTM_CTL_TAEVENT | GPTM_CTL_TBEVENT;

        break;
        
    case Mode::PWM:
        //
        // TM4C123GH6PM only supports splitted timers in pwm
        //
        if ( !module.m_isSplit )
        {
            return 1;
        }

        if ( GPIO::configure(module.m_id, SelectId::TIMERBOTH, module.m_pins) )
        {
            return 1;
        }

        if ( GPIO::enable(module.m_pins) )
        {
            return 1;
        }

        //
        // PWM interrupts enabled
        //
        timerp[module.m_index]->TAMR = GPTM_TAMR_TAPWMIE;
        timerp[module.m_index]->TBMR = GPTM_TBMR_TBPWMIE;

        break;
    }

    return 0;
}

//-------------------------------------------------------------------------------
int Timer::start(
    const Module &module,
    const SelectId sid,
    const unsigned periodus,
    void (*isr)())
{
    uint64_t full = uint64_t(periodus) * F_CPU_CYCLES_PER_US;
    uint16_t prescaler = (full >> module.m_bits);
    uint64_t load = full;

    if ( !module.m_isSplit && sid == SelectId::TIMERB )
    {
        return 1;
    }

    Interrupts::setHandler(
        module.m_irqn[sid == SelectId::TIMERB ? 1 : 0],
        isr);

    Interrupts::enableIRQ(
        module.m_irqn[sid == SelectId::TIMERB ? 1 : 0]);

    //
    // Excerpt from MCU data sheet:
    //
    // "Note that when counting down in one-shot or periodic modes, the
    // prescaler acts as a true prescaler and contains the least-significant
    // bits of the count. When counting up in one-shot or periodic modes, the
    // prescaler acts as a timer extension and holds the most-significant bits
    // of the count. In input edge count, input edge time and PWM mode, the
    // prescaler always acts as a timer extension, regardless of the count
    // direction."
    //
    // ==> it means that my implementation here is for timer extensions instead
    // of prescaler. I'm not counting down.
    // 
    // Prescaler is available only in split timers mode:
    //
    // 16/32 GPTM implies 8 bit scaler => 16+8=24 bit effective counter
    // 32/64 GPTM implies 16 bit scaler => 32+16=48 bit effective counter
    //
    if ( module.m_id <= ModuleId::TIMID5 )
    {
        if ( ((uint64_t(periodus) * F_CPU_CYCLES_PER_US)
              >>
              (module.m_bits + (module.m_isSplit ? 8 : 0)))
             != 0 )
        {
            return 1;
        }
        
        if ( module.m_isSplit )
        {
            load &= 0xffffUL;            
            
            if ( sid == SelectId::TIMERB )
            {
                timerp[module.m_index]->TBPR = prescaler;
            
                timerp[module.m_index]->TBILR = load;
            }
            else
            {
                timerp[module.m_index]->TAPR = prescaler;
            
                timerp[module.m_index]->TAILR = load;
            }
        }
        else
        {
            timerp[module.m_index]->TAILR = (full & 0xffffUL);            

            timerp[module.m_index]->TBILR = ((full & 0xffff0000ULL) >> 16);
        }
    }
    else
    {
        if ( ((uint64_t(periodus) * F_CPU_CYCLES_PER_US)
              >>
              (module.m_bits + (module.m_isSplit ? 16 : 0)))
             != 0 )
        {
            return 1;
        }
        
        if ( module.m_isSplit )
        {
            load &= 0xffffffffUL;            

            if ( sid == SelectId::TIMERB )
            {
                timerp[module.m_index]->TBPR = prescaler;
            
                timerp[module.m_index]->TBILR = load;
            }
            else
            {
                timerp[module.m_index]->TAPR = prescaler;
            
                timerp[module.m_index]->TAILR = load;
            }
        }
        else
        {
            timerp[module.m_index]->TAILR = (full & 0xffffffffUL);            

            timerp[module.m_index]->TBILR = ((full & 0xffffffff00000000ULL) >> 32);
        }
    }

    //
    // enable the timer(s)
    //
    if ( sid == SelectId::TIMERB )
    {
        timerp[module.m_index]->CTL |= (GPTM_CTL_TBEN);
    }
    else
    {
        timerp[module.m_index]->CTL |= (GPTM_CTL_TAEN);
    }
    
    return 0;
}

//-------------------------------------------------------------------------------
int Timer::getLoad(const Module &module, uint32_t &load)
{
    //
    // if in split mode then trigger an error
    //
    if ( (timerp[module.m_index]->CFG & 0x7) == 0x4 )
    {
        return 1;
    }

    //
    // if not a 32-bit timer then error
    //
    if ( module.m_id > ModuleId::TIMID5 )
    {
        return 1;
    }

    load = timerp[module.m_index]->TAILR;

    return 0;
}

//-------------------------------------------------------------------------------
int Timer::getLoad(const Module &module, const SelectId sid, uint32_t &load)
{
    //
    // if not in split mode then trigger an error
    //
    if ( (timerp[module.m_index]->CFG & 0x7) != 0x4 )
    {
        return 1;
    }

    //
    // if not a 32-bit timer then error
    //
    if ( module.m_id > ModuleId::TIMID5 )
    {
        return 1;
    }

    if ( sid == SelectId::TIMERA )
    {
        load =
            (timerp[module.m_index]->TAPR << 0x10)
            |
            (timerp[module.m_index]->TAILR & 0xffffUL);
    }
    else
    {
        load =
            (timerp[module.m_index]->TBPR << 0x10)
            |
            (timerp[module.m_index]->TBILR & 0xffffUL);
    }

    return 0;
}

//
// 11.3.6 Accessing Concatenated 16/32-Bit GPTM Register Values
//

// see 11.3.7 Accessing Concatenated 32/64-Bit Wide GPTM Register Values

//-------------------------------------------------------------------------------
Timer::LoadType Timer::getValue(
    const Module &module,
    const SelectId sid)
{
    if ( sid == SelectId::TIMERA )
    {
        return timerp[module.m_index]->TAV;
    }
    else if ( sid == SelectId::TIMERB )
    {
        return timerp[module.m_index]->TBV;
    }

    return 0;
}

//-------------------------------------------------------------------------------
Timer::LoadType Timer::getValueSnapshot(
    const Module &module,
    const SelectId sid)
{
    if ( sid == SelectId::TIMERA )
    {
        return timerp[module.m_index]->TAR;
    }
    else if ( sid == SelectId::TIMERB )
    {
        return timerp[module.m_index]->TBR;
    }

    return 0;
}

/*___oOo___*/
