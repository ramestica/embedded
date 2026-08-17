//
// System stuff
//
#include <stdlib.h>

//
// Support stuff
//
#include <Support/Utils.h>

//
// Target stuff
//
#include <Target/SystemTick.h>
#include <Target/GPIO.h>

//
// Library stuff
//
#include <Support/ErrorCodes.h>
#include <Logic/UART.h>

//
// Local stuff
//
#include <Logic/I2C.h>
#include <Logic/MainLoop.h>

//
// identify possible arguments to STATS command
//
#define STATS_TYPE_ALL       0
#define STATS_TYPE_LOOP      1
#define STATS_TYPE_ALGORITHM 2
#define STATS_TYPE_EXEC      3

//
// data streamers use for printout and command/reply purposes
//
RAM::Logic::Host::DataStream *consoleDataStream_p;
RAM::Logic::Host::DataStream *commandDataStream_p;

using namespace RAM::Target;
using namespace RAM::Logic::Host::CommandReply;
using namespace RAM::Logic::MainLoop;

extern unsigned long _text;
extern unsigned long _etext;
extern unsigned long _data;
extern unsigned long _edata;

//-------------------------------------------------------------------------------
Executive &Executive::instance()
{
    static Executive instance;
    
    return instance;
}

//-------------------------------------------------------------------------------
void Executive::setup(
    void (*setupFun)(),
    void (*algorithmFun)(),
    const SerialChannelSelector serialChannelSelector,
    const unsigned periodus,
    const unsigned int loopJiffies,
    const unsigned long serialBaudRate)
{
    if ( SET_ERROR_JUMP_MARK() == 0 )
    {
        m_algorithmFun_p = algorithmFun;

        consoleDataStream_p =
            new RAM::Logic::UART::Module<RAM::Target::UART::ModuleId::UARTID0>
            (serialBaudRate);

        if ( serialChannelSelector
             ==
             SerialChannelSelector::SAME_SERIAL_CHANNELS )
        {
            commandDataStream_p = consoleDataStream_p;
        }
        else
        {
            commandDataStream_p =
                new RAM::Logic::UART::Module<RAM::Target::UART::ModuleId::UARTID1>
                (serialBaudRate);
        }
        
        RAM::Logic::Host::setCommunicationStreams(
            consoleDataStream_p, commandDataStream_p);

        //
        // start the system tick
        //
        if ( SystemTick::start(
                 true,
                 periodus,
                 loopJiffies,
                 &m_wasLoopTickAcknowledged) )
        {
            RAM::Support::Error::longjmp(MAINLOOP_ERR_SYSTICK);
        }

        m_statsCmd.m_obj_p = this;
        
        CommandList::instance().setCommandHandler(&m_sysTickCmd);

        CommandList::instance().setCommandHandler(&m_uptimeCmd);

        CommandList::instance().setCommandHandler(&m_statsCmd);

        if ( setupFun != nullptr )
        {
            setupFun();
        }

        CommandList::instance().updateDataStream();

        RAM::Logic::Host::Console::print(SystemCoreClock);
        RAM::Logic::Host::Console::println("Hz");
        
        RAM::Logic::Host::Console::print("serial commands: ");

        RAM::Logic::Host::Console::println(CommandList::instance().getCommandsCount());

        CommandList::instance().print();

        GPIO::StatusLED::enable();

        GPIO::StatusLED::on();

        //
        // no more dynamic memory allocation after this point
        //
        //RAM::Support::Utils::mlock();

        RAM::Logic::Host::Console::print("text @ 0x");

        RAM::Logic::Host::Console::println((unsigned)&_text, 16);
        RAM::Logic::Host::Console::println((unsigned)&_etext, 16);
        RAM::Logic::Host::Console::println((unsigned)&_data, 16);
        RAM::Logic::Host::Console::println((unsigned)&_edata, 16);

        RAM::Logic::Host::Console::print("dynamic memory: ");

        RAM::Logic::Host::Console::println(RAM::Support::Utils::musage());
        
        m_setupStatus = true;
    }
}

//-------------------------------------------------------------------------------
void Executive::execute()
{
    static uint32_t t0Loop = 0;

    uint32_t t0 = SystemTick::micros();

    if ( !m_setupStatus )
    {
        RAM::Logic::Host::Console::print("setup error 0x");

        RAM::Logic::Host::Console::println(RAM::Support::Error::longjmpCode, 16);

        RAM::Support::Utils::delayus(1000000);

        return;
    }

    if ( !m_algorithmStatus )
    {
        RAM::Logic::Host::Console::print("algorithm error 0x");

        RAM::Logic::Host::Console::println(RAM::Support::Error::longjmpCode, 16);

        RAM::Support::Utils::delayus(1000000);

        return;
    }

    if ( !m_wasLoopTickAcknowledged )
    {
        return;
    }

    //(*systemViewRecordVoid)(33);

    m_wasLoopTickAcknowledged = false;

    if ( t0Loop != 0 )
    {
        m_tLoopPeriod.addSample(t0 - t0Loop);
    }
    
    t0Loop = t0;
    
    SystemTick::mark();

    if ( SET_ERROR_JUMP_MARK() == 0 )
    {
        if ( m_algorithmFun_p != nullptr )
        {
            (*systemViewRecordVoid)(34);

            m_algorithmFun_p();

            (*systemViewRecordEndCall)(34);
        }
    }
    else
    {
        m_algorithmStatus = false;
    }

    m_tDeltaAlgorithm.addSample(SystemTick::mark());

    SystemTick::mark();
    
    //
    // process any command coming through the serial line, if any then toggle
    // the LED to feedback commanding acknowledgment
    //
    if ( CommandList::instance().execute() )
    {
        m_tDeltaExecute.addSample(SystemTick::mark());

        GPIO::StatusLED::toggle();
    }

    //(*systemViewRecordEndCall)(33);
}

//-------------------------------------------------------------------------------
void Executive::SystemTickCmd::execute()
{
    RAM::Logic::Host::Console::print("systick: ");

    // what to print

    RAM::Logic::Host::Console::println("");
}

//-------------------------------------------------------------------------------
void Executive::UptimeCmd::execute()
{
    float m = SystemTick::micros() / 1000000.0f;

    reply(ReplyId::OK, &m, sizeof(m));
}

//-------------------------------------------------------------------------------
void Executive::StatsCmd::execute()
{
    uint8_t statsType;

    if ( !read(&statsType, sizeof(statsType)) )
    {
        return;
    }

    if ( statsType > STATS_TYPE_EXEC )
    {
        reply(SerialReplyId::INVALID_STATISTICS_TYPE);

        return;
    }

    reply(ReplyId::OK);

    m_dataStream_p->write(RAM::Support::Utils::musage());

    if ( statsType == STATS_TYPE_LOOP || statsType == STATS_TYPE_ALL )
    {
        m_dataStream_p->write(
            reinterpret_cast<const char *>(m_obj_p->m_tLoopPeriod.m_data),
            RAM::Logic::SampleStatistics::m_dataSize);
    }

    if ( statsType == STATS_TYPE_ALGORITHM || statsType == STATS_TYPE_ALL )
    {
        m_dataStream_p->write(
            reinterpret_cast<const char *>(m_obj_p->m_tDeltaAlgorithm.m_data),
            RAM::Logic::SampleStatistics::m_dataSize);
    }
  
    if ( statsType == STATS_TYPE_EXEC || statsType == STATS_TYPE_ALL )
    {
        m_dataStream_p->write(
            reinterpret_cast<const char *>(m_obj_p->m_tDeltaExecute.m_data),
            RAM::Logic::SampleStatistics::m_dataSize);
    }

    return;
}

/*___oOo__*/
