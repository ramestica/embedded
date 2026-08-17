#ifndef _MainLoop_H_
#define _MainLoop_H_

//
// System stuff
//

//
// Target stuff
//
#include <Target/Timer.h>

//
// Library stuff
// 
#include <Support/ErrorCodes.h>
#include <Logic/Host.h>
#include <Logic/SampleStatistics.h>

namespace RAM
{
    namespace Logic
    {
        namespace MainLoop
        {
            /** Serial channels are hardcoded to either UARTID0 for both streams
             ** (console and monitor&control) or UARTID0 for console and UARTID1
             ** for monitor&control.
             */
            enum class SerialChannelSelector: unsigned
            {
                SAME_SERIAL_CHANNELS,
                INDIVIDUAL_SERIAL_CHANNELS
            };
                
            struct SerialCommandId
            {
                enum SerialCommandIdEnum
                {
                    SYSTICK = 0,
                    UPTIME,
                    STATS,
                    READ_SERIAL_TEST,
                    WRITE_SERIAL_TEST
                };
            };

            struct SerialReplyId
            {
                enum SerialReplyIdEnum
                {
                    INVALID_CODE = RAM::Logic::Host::CommandReply::ReplyId::OFFSET,
                    INVALID_STATISTICS_TYPE,
                    NULL_HANDLER
                };
            };

            class Executive
            {
            public:
                static Executive &instance();

                /** Setup main-loop parameters and setup system.
                 ** @param periodus system tick period.
                 */
                void setup(
                    void (*setupFun)(),
                    void (*algorithmFun)(),
                    const SerialChannelSelector serialChannelSelector,
                    const unsigned periodus,
                    const unsigned int loopJiffies = 1,
                    const unsigned long serialBaudRate = 115200);

                /** Execute loop: update timing stats, algorithm and serial
                 ** commands.
                 ** @param time at which the method was invoked.
                 */
                void execute();
                
            private:
                bool m_setupStatus, m_algorithmStatus;
                bool m_wasLoopTickAcknowledged = false;
                void (*m_algorithmFun_p)();
                RAM::Logic::SampleStatistics m_tDeltaAlgorithm, m_tDeltaExecute, m_tLoopPeriod;

                /** A private constructor makes a singleton.
                 */
                Executive():
                    m_setupStatus(false),
                    m_algorithmStatus(true),
                    m_algorithmFun_p(nullptr),
                    m_statsCmd(this)
                {
                }
                
                class SystemTickCmd: public RAM::Logic::Host::CommandReply::Command
                {
                public:
                    SystemTickCmd():
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MAIN_LOOP,
                            SerialCommandId::SYSTICK)
                    {;}

                    void execute();
                };
                SystemTickCmd m_sysTickCmd;
                
                class UptimeCmd: public RAM::Logic::Host::CommandReply::Command
                {
                public:
                    UptimeCmd():
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MAIN_LOOP,
                            SerialCommandId::UPTIME)
                    {;}

                    void execute();
                };
                UptimeCmd m_uptimeCmd;
                
                class StatsCmd: public RAM::Logic::Host::CommandReply::Command
                {
                public:
                    StatsCmd(Executive *ptr):
                        Command(
                            RAM::Logic::Host::CommandReply::ReceiverId::MAIN_LOOP,
                            SerialCommandId::STATS),
                        m_obj_p(ptr)
                    {;}

                    void execute();

                    Executive *m_obj_p;
                };
                StatsCmd m_statsCmd;
            };
        }
    }
}

#endif /* _MainLoop_H_ */

/*___oOo__*/
