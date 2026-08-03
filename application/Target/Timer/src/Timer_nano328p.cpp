//
// System stuff
//
#include <math.h>

//
// Target stuff
//
#include <Target.h>
#include <GPIO.h>

//
// Local stuff
//
#include <TargetTimer.h>

using namespace std;
using namespace RAM::Target;

#define setRegister(reg, mask, value)                   \
    *reg = (((*reg) & ~(mask)) | ((value) & (mask)))

/** All timers use the same bits in TCCR1B for prescaler.
 */
static const uint8_t PrescalerBits = 0b00000111;

/** Type taken for include/avr/sfr_defs.h (_MMIO_BYTE)
 */
typedef volatile uint8_t *RegAddr;

/** TIMSKx registers.
 */
RegAddr TIMSKx[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    &TIMSK0,
    &TIMSK1,
    &TIMSK2
};

/** TIMSKx bits.
 */
enum TIMSKxBits
{
    TOIEx  = 0,
    OCIExA = 1,
    OCIExB = 2,
    ICIEx  = 5
};

RegAddr TCCRxA[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    &TCCR0A,
    &TCCR1A,
    &TCCR2A
};

/** TCCRxA bits.
 */
enum TCCRxABits
{
    WGMx0 = 0,
    WGMx1 = 1,
    COMxB0 = 4,
    COMxB1 = 5,
    COMxA0 = 6,
    COMxA1 = 7
};

RegAddr TCCRxB[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    &TCCR0B,
    &TCCR1B,
    &TCCR2B
};

/** TCCRxB bits (8 bits timer).
 */
enum class TCCRxBBits8
{
    CSx0 = 0,
    CSx1 = 1,
    CSx2 = 2,
    WGMx2 = 3,
    FOCxB = 6,
    FOCxA = 7
};

/** TCCRxB bits (16 bits timer).
 */
enum class TCCRxBBits16
{
    CSx0 = 0,
    CSx1 = 1,
    CSx2 = 2,
    WGMx2 = 3,
    WGMx3 = 4,
    ICESx = 6,
    ICNCx = 7
};

RegAddr TIFRx[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    &TIFR0,
    &TIFR1,
    &TIFR2
};

/** TIFRx bits.
 */
enum TIFRxBits
{
    TOVx  = 0,
    OCFxA = 1,
    OCFxB = 2,
    ICFx  = 5
};

RegAddr OCRxA[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    &OCR0A,
    &OCR1AL,
    &OCR2A
};

RegAddr OCRxB[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    &OCR0B,
    &OCR1BL,
    &OCR2B
};

RegAddr TCNTx[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    &TCNT0,
    &TCNT1L,
    &TCNT2 
};

//
// available prescalers per timer
//
//    1 8     64       256 1024
//    1 8     64       256 1024
//    1 8 32  64  128  256 1024
//    -------------------------
//    0 3  5   6    7    8   10 power of two
//
static uint16_t Prescaler[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    0b0000010101001001,
    0b0000010101001001,
    0b0000010111101001
};

//
// timers' size
//
static uint8_t BitsN[static_cast<int>(Timer::Module::TIMIDXN)] =
{
    8,
    16,
    8
};

//
// a global array of pointers to timer isr, the vector would call the actual
// function through their pointer in this global.
//
static void (*isr_global[int(Timer::Module::TIMIDXN)][2])();

//
// use avr ISR macro to define an interrupt
//
#define GENERATE_ISR(vect, tid, sid)                                    \
    ISR(vect)                                                           \
    {                                                                   \
        void (*f)() = isr_global[int(tid)][int(sid)];                   \
        (*f)();                                                         \
    }

//
// all supported interrupts
//
GENERATE_ISR(
    TIMER0_COMPA_vect,
    Timer::Module::TIMIDX0,
    Timer::Select::TIMERA);
GENERATE_ISR(
    TIMER0_COMPB_vect,
    Timer::Module::TIMIDX0,
    Timer::Select::TIMERB);
GENERATE_ISR(
    TIMER1_COMPA_vect,
    Timer::Module::TIMIDX1,
    Timer::Select::TIMERA);
GENERATE_ISR(
    TIMER1_COMPB_vect,
    Timer::Module::TIMIDX1,
    Timer::Select::TIMERB);
GENERATE_ISR(
    TIMER2_COMPA_vect,
    Timer::Module::TIMIDX2,
    Timer::Select::TIMERA);
GENERATE_ISR(
    TIMER2_COMPB_vect,
    Timer::Module::TIMIDX2,
    Timer::Select::TIMERB);

//
// macro to set Waveform Generation Mode bits. WGM in timers 0 and 2 has three
// bits. WGM in timer 2 has four bits. In both cases the two LSB bits are in
// TCCRnA (stating at LSB) and the rest in TCCRBnB starting at bit three
// (counting from zero).
//
#define SET_WGM(tidx, value)                                            \
    do                                                                  \
    {                                                                   \
        setRegister(                                                    \
            TCCRxA[static_cast<int>(tidx)],                             \
            (1 << TCCRxABits::WGMx1) | (1 << TCCRxABits::WGMx0),        \
            (0b11 & value) << TCCRxABits::WGMx0);                       \
                                                                        \
        if ( BitsN[static_cast<int>(tidx)] == 16 )                      \
        {                                                               \
            setRegister(                                                \
                TCCRxB[static_cast<int>(tidx)],                         \
                (1 << int(TCCRxBBits16::WGMx3)) | (1 << int(TCCRxBBits16::WGMx2)), \
                ((0b1100 & value) >> 2) << int(TCCRxBBits16::WGMx2));   \
        }                                                               \
        else                                                            \
        {                                                               \
            setRegister(                                                \
                TCCRxB[static_cast<int>(tidx)],                         \
                (1 << int(TCCRxBBits8::WGMx2)),                         \
                ((0b100 & value) >> 2) << int(TCCRxBBits8::WGMx2));     \
        }                                                               \
    }                                                                   \
    while ( 0 )

//-------------------------------------------------------------------------------
unsigned int Timer::getNumberBits(const Module tidx)
{
    return BitsN[static_cast<int>(tidx)];
}

//-------------------------------------------------------------------------------
int Timer::adaptPrescaler(
    const Module tidx,
    const uint32_t periodus,
    float &ideal,
    unsigned int &out)
{
    //
    // if the period is not exactly an integer multiple of the cpu's period then
    // there is no way the requested period could be met.
    //
    if ( fmod(periodus * 1e3, 1e9 / float(F_CPU)) != 0 )
    {
        return 1;
    }

    ideal = getExactPrescaler(periodus, BitsN[static_cast<int>(tidx)]);

    unsigned int top = ceil(ideal);

    for ( unsigned int i = 0; i < sizeof(uint16_t) * 8; ++i )
    {
        //
        // a prescaler is available only if the bit is set
        //
        if ( !(Prescaler[static_cast<int>(tidx)] & (1UL << i)) )
        {
            continue;
        }

        if ( (Prescaler[static_cast<int>(tidx)] & (1UL << i)) >= top )
        {
            out = (1UL << i);

            return 0;
        }
    }

    return 1;
}

//-------------------------------------------------------------------------------
int Timer::setMode(
    const Module tid,
    const Select sid,
    const Mode mode,
    const LoadType period)
{
    //
    // make sure to start from a clean state
    //
    *(TCCRxA[static_cast<int>(tid)]) = 0;
    *(TCCRxB[static_cast<int>(tid)]) = 0;

    //
    // timer 2 has an option to use an external oscillator but we want I/O clk
    //
    if ( tid == Module::TIMIDX2 )
    {
        //
        // use I/O clk for timer2
        //
        ASSR &= ~(1 << AS2);
    }

    //
    // make sure this specific timer is not interrupting
    //
    *(TIMSKx[static_cast<int>(tid)]) &= ~(0x1 << TOIEx);

    switch ( mode )
    {
    case Mode::Normal:
        //
        // Configure in normal mode (pure counting, no PWM etc.)
        //
        SET_WGM(tid, 0b0);

        break;
    case Mode::CTC:
        //
        // Configure in CTC mode. Top is OCRA.
        //
        if ( BitsN[static_cast<int>(tid)] == 8 )
        {
            SET_WGM(tid, 0b010);
        }
        else
        {
            SET_WGM(tid, 0b100);
        }

        if ( setOutputCompareValue(tid, sid, period) )
        {
            return 1;
        }
        
        break;
    case Mode::FastPWM:
        //
        // timer 1 is different than 0 and 2
        //
        if ( tid == Module::TIMIDX1 )
        {
            //
            // set fast pwm mode and use ICR1 as TOP
            //
            SET_WGM(tid, 0b1110);
            
            //
            // load 16 bits ICR1 register with counting maximum
            //
            ICR1 = 0xFFFF;
        }
        else
        {
            //
            // set fast pwm mode and use 0xff as TOP
            //
            SET_WGM(tid, 0b11);
        }
        break;
    }

    return 0;
}

//-------------------------------------------------------------------------------
int Timer::setCompareOutputMode(                    
    const Module tidx,
    const Select timer,
    const CompareOutputMode mode)
{
    uint8_t bits =
        (mode == CompareOutputMode::DISCONECT_OUTPUT
         ?
         0
         :
         mode == CompareOutputMode::TOGGLE_OUTPUT
         ?
         1
         :
         mode == CompareOutputMode::NON_INVERTING_OUTPUT
         ?
         2
         :
         3);

    if ( timer == Select::TIMERA || timer == Select::TIMERFULL )
    {
        *(TCCRxA[static_cast<int>(tidx)]) |= (bits << TCCRxABits::COMxA0);
    }

    if ( timer == Select::TIMERB || timer == Select::TIMERFULL )
    {
        *(TCCRxA[static_cast<int>(tidx)]) |= (bits << TCCRxABits::COMxB0);
    }

    return 0;
}

//-------------------------------------------------------------------------------
int Timer::setOutputCompareValue(
    const Module tidx,
    const Select timer,
    const LoadType &dc)
{
    if ( timer == Select::TIMERA || timer == Select::TIMERFULL )
    {
        if ( tidx == Module::TIMIDX1 )
        {
            *reinterpret_cast<volatile LoadType *>
                (OCRxA[static_cast<int>(tidx)]) = dc;
        }
        else
        {
            *OCRxA[static_cast<int>(tidx)] = dc;
        }
    }

    if ( timer == Select::TIMERB || timer == Select::TIMERFULL )
    {
        if ( tidx == Module::TIMIDX1 )
        {
            *reinterpret_cast<volatile LoadType *>
                (OCRxB[static_cast<int>(tidx)]) = dc;
        }
        else
        {
            *OCRxB[static_cast<int>(tidx)] = dc;
        }
    }

    return 0;
}

//-------------------------------------------------------------------------------
int Timer::setOutputCompareFromCounter(
    const Module tid,
    const Select sid,
    LoadType &cnt)
{
    if ( BitsN[int(tid)] == 8 )
    {
        cnt = *TCNTx[int(tid)];
    }
    else
    {
        cnt = *reinterpret_cast<volatile LoadType *>(TCNTx[static_cast<int>(tid)]);
    }
            
    if ( sid == Select::TIMERA || sid == Select::TIMERFULL )
    {
        if ( tid == Module::TIMIDX1 )
        {
            *reinterpret_cast<volatile LoadType *>
                (OCRxA[static_cast<int>(tid)]) = cnt;
        }
        else
        {
            *OCRxA[static_cast<int>(tid)] = cnt;
        }
    }

    if ( sid == Select::TIMERB || sid == Select::TIMERFULL )
    {
        if ( tid == Module::TIMIDX1 )
        {
            *reinterpret_cast<volatile LoadType *>
                (OCRxB[static_cast<int>(tid)]) = cnt;
        }
        else
        {
            *OCRxB[static_cast<int>(tid)] = cnt;
        }
    }

    return 0;
}

//-------------------------------------------------------------------------------
int Timer::interruptControl(
    const Module tid,
    const Select sid,
    void (*isr)())
{
    isr_global[int(tid)][int(sid)] = isr;

    if ( sid == Select::TIMERA || sid == Select::TIMERFULL )
    {
        if ( isr == nullptr )
        {
            *(TIMSKx[static_cast<int>(tid)]) &= ~(1 << TIMSKxBits::OCIExA);
        }
        else
        {
            *(TIMSKx[static_cast<int>(tid)]) |= (1 << TIMSKxBits::OCIExA);
        }
    }
 
    if ( sid == Select::TIMERB || sid == Select::TIMERFULL )
    {
        if ( isr == nullptr )
        {
            *(TIMSKx[static_cast<int>(tid)]) &= ~(1 << TIMSKxBits::OCIExB);
        }
        else
        {
            *(TIMSKx[static_cast<int>(tid)]) |= (1 << TIMSKxBits::OCIExB);
        }
    }

    return 0;
}

//-------------------------------------------------------------------------------
void Timer::interruptTrigger(
    const Module tid,
    const Select sid)
{
    if ( sid == Select::TIMERA || sid == Select::TIMERFULL )
    {
        *(TIFRx[static_cast<int>(tid)]) |= (1 << TIFRxBits::OCFxA);
    }
 
    if ( sid == Select::TIMERB || sid == Select::TIMERFULL )
    {
        *(TIFRx[static_cast<int>(tid)]) |= (1 << TIFRxBits::OCFxB);
    }
}

//-------------------------------------------------------------------------------
int Timer::setPrescaler(
    const Module tidx,
    const Select timer, 
    const uint32_t prescaler)
{
    uint8_t prescalerCode = 1;

    for ( unsigned int i = 0; i < sizeof(uint16_t) * 8; ++i )
    {
        //
        // a prescaler is available only if the bit is set
        //
        if ( !(Prescaler[static_cast<int>(tidx)] & (1UL << i)) )
        {
            continue;
        }

        if ( (Prescaler[static_cast<int>(tidx)] & (1UL << i)) == prescaler )
        {
            //
            // select specific prescaler in control register, which
            // automatically states the timer
            //
            setRegister(
                TCCRxB[static_cast<int>(tidx)],
                PrescalerBits,
                prescalerCode);

            return 0;
        }

        ++prescalerCode;
    }

    return 1;
}

//-------------------------------------------------------------------------------
int Timer::getCounter(
    const Module tid,
    const Select sid,
    LoadType &cnt)
{
    //
    // atmega328p does support just one counter per timer module
    //
    if ( sid != Select::TIMERA )
    {
        return 1;
    }

    if ( BitsN[int(tid)] == 8 )
    {
        cnt = *TCNTx[int(tid)];
    }
    else
    {
        cnt = *reinterpret_cast<volatile LoadType *>(TCNTx[static_cast<int>(tid)]);
    }
            
    return 0;
}

//-------------------------------------------------------------------------------
int Timer::acknowledgeOverflow(
    const Module tidx,
    const Select timer,
    const uint64_t value,
    bool &acknowledged)
{
    acknowledged = false;

    //
    // if overflow flag not set then we are still waiting
    //
    if ( (*(TIFRx[static_cast<int>(tidx)]) & (1 << TIFRxBits::TOVx)) == false )
    {
        return 0;
    }

    //
    // reset the overflow interrupt by writing one
    //
    *(TIFRx[static_cast<int>(tidx)]) |= (1 << TIFRxBits::TOVx);
    
    //
    // reset counter to its starting count
    //
    //if ( setLoad(tidx, timer, value) )
    //{
    //    return 1;
    //}

    acknowledged = true;

    return 0;
}

//-------------------------------------------------------------------------------
int Timer::acknowledgeCompareMatchFlag(
    const Module tidx,
    const Select timer,
    bool &acknowledged)
{
    uint8_t bitmask =
        (1 << (timer == Select::TIMERA ? TIFRxBits::OCFxA : TIFRxBits::OCFxB));

    acknowledged = false;

    //
    // check compare flag and if set reset it (writing one resets)
    //
    if ( *(TIFRx[static_cast<int>(tidx)]) & bitmask )
    {
        *(TIFRx[static_cast<int>(tidx)]) |= bitmask;

        acknowledged = true;
    }

    return 0;
}

//-------------------------------------------------------------------------------
void Timer::print(const Module tidx)
{
    Serial.print("TCCR");Serial.print(static_cast<int>(tidx));Serial.print("A=");
    Serial.println(*(TCCRxA[static_cast<int>(tidx)]), BIN);

    Serial.print("TCCR");Serial.print(static_cast<int>(tidx));Serial.print("B=");
    Serial.println((*TCCRxB[static_cast<int>(tidx)]), BIN);

    Serial.print("TCNT");Serial.print(static_cast<int>(tidx));Serial.print("=");
    Serial.println((*TCNTx[static_cast<int>(tidx)]), BIN);

    Serial.print("OCR");Serial.print(static_cast<int>(tidx));Serial.print("A=");
    if ( BitsN[static_cast<int>(tidx)] == 8 )
    {
        Serial.println((*OCRxA[static_cast<int>(tidx)]));
    }
    else
    {
        Serial.println(*reinterpret_cast<volatile uint16_t *>(OCRxA[static_cast<int>(tidx)]));
    }

    Serial.print("OCR");Serial.print(static_cast<int>(tidx));Serial.print("B=");
    if ( BitsN[static_cast<int>(tidx)] == 8 )
    {
        Serial.println((*OCRxB[static_cast<int>(tidx)]));
    }
    else
    {
        Serial.println(*reinterpret_cast<volatile uint16_t *>(OCRxB[static_cast<int>(tidx)]));
    }

    Serial.print("TIMSK");Serial.print(static_cast<int>(tidx));Serial.print("=");
    Serial.println((*TIMSKx[static_cast<int>(tidx)]), BIN);

    Serial.print("TIFR");Serial.print(static_cast<int>(tidx));Serial.print("=");
    Serial.println((*TIFRx[static_cast<int>(tidx)]), BIN);

    Serial.print("PRR=");
    Serial.println(PRR, BIN);

    Serial.print("ASSR=");
    Serial.println(ASSR, BIN);

    Serial.print("GTCCR=");
    Serial.println(GTCCR, BIN);
}

/*___oOo___*/
