//
// System stuff
//
#include <math.h>

//
// Target stuff
//
#include <GPIO.h>

//
// Local stuff
//
#include <Timer.h>

namespace GPIO = RAM::Target::GPIO;
namespace TargetTimer = RAM::Target::Timer;
using namespace RAM::Library::Print;
using namespace RAM::Library::Timer;

//
// from duty cycle in [0,1] range to Output Compare register value
//
#define DUTYCYCLE(dc)                                               \
    ((((m_dc100us - m_dc0us) * dc + m_dc0us) / float(m_periodus))   \
     *                                                              \
     m_outputCompareMax)

//
// static variables
//
Module *Module::m_modulesInUse[] = {};

//-------------------------------------------------------------------------------
Module::Module(
    const ModuleId tid,
    const uint32_t periodus):
    m_tid(tid),
    m_periodus(periodus)
{
    if ( m_modulesInUse[int(tid)] != NULL )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_MODULE_IN_USE);
    }
     
    m_modulesInUse[int(tid)] = this;

    float cpupns = 1e9 / float(F_CPU);

    //
    // record in class variable the timer's number of bits
    //
    m_nbits = getNumberBits(m_tid);

    //
    // if the period is not exactly an integer multiple of the cpu's period then
    // there is no way the requested period could be met.
    //
    if ( fmod(m_periodus * 1e3, cpupns) != 0 )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_INVALID_FREQUENCY);
    }

    //
    // possible prescaler values are board specific
    //
    if ( TargetTimer::adaptPrescaler(
             m_tid,
             m_periodus,
             m_idealPresecaler,
             m_prescaler) )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_INVALID_PRESCALER);
    }

    //
    // timer's top count, when counting up from zero, to match at the requested
    // frequency
    //
    m_load = (F_CPU / 1e6) * (float(m_periodus) / m_prescaler);

    m_resolutionns = (m_prescaler * float(1000000000)) / F_CPU;
}

//-------------------------------------------------------------------------------
void Module::print()
{
    Print::print("load="); Print::println((unsigned int)m_load);

    Print::print("nbits="); Print::println((unsigned int)m_nbits);

    Print::print("ideal prescaler="); Print::println(m_idealPresecaler);

    Print::print("prescaler="); Print::println((unsigned int)m_prescaler);

    Print::print("periodus="); Print::println(m_periodus);

    Print::print("resolutionus="); Print::println(m_resolutionns / 1000);

    TargetTimer::print(m_module);
}

//-------------------------------------------------------------------------------
void Module::configure(
    const SelectId sid,
    const TargetTimer::Mode mode,
    const TargetTimer::CompareOutputMode compareOutputMode)
{
    //
    // FIXME:: what if it is already started?
    //
    m_sid = sid;

    if ( TargetTimer::setMode(m_tid, m_sid, mode, m_load) )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_SET_MODE);
    }

    if ( TargetTimer::setCompareOutputMode(
             m_tid,
             m_sid,
             compareOutputMode) )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_SET_COMPARE_OUTPUT_MODE);
    }
}

//-------------------------------------------------------------------------------
void Module::start()
{
    //
    // start timer with given prescaler
    //
    if ( TargetTimer::setPrescaler(m_tid, m_sid, m_prescaler) )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_SET_PRESCALER);
    }
}

//-------------------------------------------------------------------------------
PWM::Encoder::Encoder(
    const ModuleId tid,
    const SelectId sid,
    const uint32_t periodus,
    const uint32_t dc0us,
    const uint32_t dc100us,
    const float initialDutyCycle):
    Module(tid, periodus),
    m_dc0us(dc0us),
    m_dc100us(dc100us)
{
    //
    // in pwm the period must match the full length of the timer, which
    // implicitly means that the requested prescaler should have not been
    // adapted
    //
    if ( m_prescaler != m_idealPresecaler )
    {
        RAM::Library::Error::longjmp(PWM_ERR_INVALID_PERIOD);
    }

    if ( m_dc0us >= m_dc100us || m_dc100us > m_periodus )
    {
        RAM::Library::Error::longjmp(PWM_ERR_INVALID_DC_PARAMS);
    }

    //m_outputCompareMax = (1ULL << m_nbits) - 1;
    m_outputCompareMax = m_load;

    configure(
        sid,
        TargetTimer::Mode::FastPWM,
        TargetTimer::CompareOutputMode::NON_INVERTING_OUTPUT);

    setDutyCycle(sid, initialDutyCycle);
    
#ifdef TARGET_nano328p
    GPIO::enableDigitalOutput(m_tid, m_sid);
#elif defined(TARGET_tivac) || defined(TARGET_minim4)
#else
#error "unknown target" 
#endif
    
    start();
}

//-------------------------------------------------------------------------------
void PWM::Encoder::setDutyCycle(
    const SelectId sid,
    const float &dc)
{
    if ( sid != m_sid && m_sid != SelectId::TIMERFULL )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_INVALID_SELECT_ID);
    }

    if ( TargetTimer::setOutputCompareValue(m_tid, sid, DUTYCYCLE(dc)) )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_SET_OUTPUT_COMPARE_VALUE);
    }
}

//-------------------------------------------------------------------------------
void PWM::Encoder::SetDutyCycleCmd::execute()
{
    uint8_t module, timer;
    float dc;

    //
    // if read fails to read the parameter then itself has sent an error reply
    //
    if ( !read(module, 1) )
    {
        return;
    }

    //
    // if read fails to read the parameter then itself has sent an error reply
    //
    if ( !read(timer, 1) )
    {
        return;
    }

    //
    // if read fails to read the parameter then itself has sent an error reply
    //
    if ( !read(dc, 1) )
    {
        return;
    }

    if ( Module::m_modulesInUse[module] == NULL )
    {
        reply(SerialReplyId::UNINITIALIZED_MODULE);

        return;
    }

    Module::m_modulesInUse[module]->setDutyCycle(
        (timer == 0)
        ?
        TargetTimer::Select::TIMERA
        :
        (timer == 1)
        ?
        TargetTimer::Select::TIMERB
        :
        TargetTimer::Select::TIMERFULL,
        dc);

    reply(SerialCommands::ReplyId::OK);
}

//-------------------------------------------------------------------------------
Periodic::Normal::Normal(
    const ModuleId tid,
    const SelectId sid,
    const uint32_t periodus):
    Module(tid, periodus)
{
    configure(
        sid,
        TargetTimer::Mode::Normal,
        TargetTimer::CompareOutputMode::DISCONECT_OUTPUT);

    start();
}

//-------------------------------------------------------------------------------
bool Periodic::Normal::acknowledge() const
{
    bool res;

    if ( acknowledgeOverflow(m_tid, m_sid, m_load, res) )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_ACKNOWLEDGE_OVERFLOW);
    }

    return res;
}

//-------------------------------------------------------------------------------
Periodic::CTC::CTC(
    const ModuleId tid,
    const SelectId sid,
    const uint32_t periodus):
    Module(tid, periodus)
{
    configure(
        sid,
        TargetTimer::Mode::CTC,
        TargetTimer::CompareOutputMode::DISCONECT_OUTPUT);

    start();
}

//-------------------------------------------------------------------------------
bool Periodic::CTC::acknowledge() const
{
    bool res;

    if ( acknowledgeCompareMatchFlag(m_tid, m_sid, res) )
    {
        RAM::Library::Error::longjmp(TIMER_ERR_ACKNOWLEDGE_MATCH);
    }

    return res;
}

/*___oOo___*/
