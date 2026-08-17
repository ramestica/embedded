#ifndef _Host_H_
#define _Host_H_

//
// System stuff
//
#include <type_traits> 

//
// Base stuff
//
#include <Support/Utils.h>
#include <Logic/Queue.h>

//
// Local stuff
//

namespace RAM
{
    namespace Logic
    {
        namespace Host
        {
            /** A data stream abstraction used to send and receive data, for
             ** different purposes like printing out or command/reply
             ** functionality.
             */
            class DataStream
            {
            public:
                DataStream():
                    m_rxq(8),
                    m_txq(8)
                {
                }

                DataStream(const unsigned rxSizeN, const unsigned txSizeN):
                    m_rxq(rxSizeN),
                    m_txq(txSizeN)
                {
                }

                /** Number of items in the receive queue available to be read.
                 */
                inline unsigned available()
                {
                    return m_rxq.queued();
                }

                inline unsigned read(char &c)
                {
                    return m_rxq.pop(&c, 1);
                }
                
                inline unsigned read(char *c, unsigned maxLength)
                {
                    return m_rxq.pop(c, maxLength);
                }

                unsigned write(const char *str, const unsigned length);

                template<typename T>
                unsigned write(const T &val)
                {
                    return write(reinterpret_cast<const char *>(&val), sizeof(T));
                }
                
                virtual bool isTransmitChannelEmpty() = 0;

                virtual void writeIntoChannel(const char c) = 0;

            protected:

                RAM::Logic::Queue m_rxq, m_txq;

                unsigned m_errorCountRx = 0;

                unsigned m_errorCountTx = 0;
            };

            /** Instantiate and configure serial communication channels through
             ** given UART modules.
             */
            void setCommunicationStreams(
                DataStream *consoleChannel_p,
                DataStream *commandReplyChannel_p);

            /** Get pointer to data stream instance used for printout purposes.
             */
            DataStream *getConsoleDataStream();

            /** Get pointer to data stream instance used for command/reply
             ** purposes.
             */
            DataStream *getCommandReplyDataStream();

            /** A basic and non compatible print and println implementation.
             */
            namespace Console
            {
                extern char buffer[];
            
                template <typename T>
                void print(const T val, int base = 10, int flen = 4)
                {
                    static DataStream *dsp = getConsoleDataStream();
                    
                    if ( std::is_floating_point<T>::value )
                    {
                        dsp->write(buffer, RAM::Support::Utils::ntoa(val, buffer, 10, flen));
                    }
                    else
                    {
                        dsp->write(buffer, RAM::Support::Utils::ntoa(val, buffer, base, 0));
                    }
                }

                template <typename T>
                void println(const T val, int base = 10, int flen = 4)
                {
                    static DataStream *dsp = getConsoleDataStream();

                    print<T>(val, base, flen);

                    dsp->write("\n\r", 2);
                }

                inline void print(const char c)
                {
                    static DataStream *dsp = getConsoleDataStream();

                    dsp->write(&c, 1);
                }

                inline void println(const char c)
                {
                    static DataStream *dsp = getConsoleDataStream();

                    dsp->write(&c, 1);

                    dsp->write("\n\r", 2);
                }

                inline void print(const char *str)
                {
                    static DataStream *dsp = getConsoleDataStream();

                    unsigned slen = 0;

                    while ( str[slen++] );

                    --slen;

                    dsp->write(str, slen);
                }

                inline void println(const char *str)
                {
                    static DataStream *dsp = getConsoleDataStream();

                    print(str);

                    dsp->write("\n\r", 2);
                }

                inline void print(const bool &val)
                {
                    if ( val )
                    {
                        print("true");
                    }
                    else
                    {
                        print("false");
                    }
                }

                inline void println(const bool &val)
                {
                    static DataStream *dsp = getConsoleDataStream();

                    print(val);

                    dsp->write("\n\r", 2);                
                }
            }

            namespace CommandReply
            {
                /** Command receiver identifiers. 
                 **
                 ** APPLICATION is special in the sense that its
                 ** meaning changes from application to
                 ** application. The application being the actual
                 ** binary loaded to mcu memory. This makes possible
                 ** customize how to inteact with different
                 ** applications. Note that this also means that each
                 ** application requires its own serial driver
                 ** (normally written in python). The actual command
                 ** code is normally implemented in the single cpp
                 ** file that contains the application loop.
                 */
                //
                // FIXME: what's the idea for an enum within a struct?
                //
                struct ReceiverId
                {
                    enum ReceiverIdEnum
                    {
                        MAIN_LOOP = 0,
                        I2C,
                        PWMENC0,PWMENC1,PWMENC2,PWMENC3,PWMENC4,PWMENC5,PWMENC6,PWMENC7,
                        PPMDEC,
                        MPU6050,
                        APPLICATION,
                        MAX
                    };
                };

                //
                // reply codes used by many receivers, each command
                // receiver must start its own list with the OFFSET
                // value of the present enumeration. In that way the
                // reply ids defined here work as base common set of
                // replies.
                //
                // FIXME: what's the idea for an enum within a struct?
                //
                struct ReplyId
                {
                    enum ReplyIdEnum
                    {
                        OK = 0,
                        UNKNOWN,
                        NULL_HANDLER,
                        DATA_TIMEOUT,
                        MISSING_ADDR_PARAM,
                        MISSING_REG_PARAM,
                        MISSING_LENGTH_PARAM,
                        MISSING_VALUE_PARAM,
                        MEMORY_ALLOCATION_ERROR,
                        RECEIVED_UNEXPECTED_VALUE,
                        OFFSET
                    };
                };

                /** The command/reply mechanism implemented here assumes the
                 ** following data flow:
                 **
                 ** receiver identifier received 
                 ** command identifier received
                 **    parameters received
                 **     executed command
                 ** send same receiver identifier
                 ** send same command identifier
                 ** send command execution status
                 **    send command results
                 **          finish
                 **
                 ** It is important to note that the execution status is sent before
                 ** any command results. This means that the command execution must
                 ** preserve its results until after the status code has been sent
                 ** and the command implementation is now ready to transmit those
                 ** results.
                 */
                struct Command
                {
                    uint8_t m_receiverId;
                    uint8_t m_commandId;
                    RAM::Logic::Host::DataStream *m_dataStream_p;

                    Command();
                    
                    Command(const int recvid, const int cmdid);

                    void reply(const int status);

                    void reply(const int status, void *data, const size_t size);

                    void reply(const int status, const uint8_t data);

                    bool read(void *out, const unsigned bytes, unsigned timeout = 2000);

                    bool write(void const *in, const unsigned bytes);

                    /** Execute is normally overloaded by derived class, but it
                     ** must be virtual to be able to operate on the derived
                     ** class 'execute' functionality from a Command. It is also
                     ** important not to be pure virtual (=0) because some
                     ** Command objects are instantiated out of the blue to
                     ** handle error replies.
                     */
                    virtual void execute()
                    {;}
                };

                class CommandList
                {
                public:

                    /** The list is a singleton.
                     */
                    static CommandList &instance();

                    /** Register a new command.
                     */
                    void setCommandHandler(Command *cmd);

                    /** Update all commands with current global data
                     ** stream. because some classes are perhaps instantiated
                     ** before the data stream is intantiated then we need to
                     ** allow for this call to happen later during the
                     ** application's initialization and instantiation process.
                     */ 
                    void updateDataStream();
                
                    /** Return number of command handlers registered to the list.
                     */
                    inline unsigned int getCommandsCount()
                    {
                        return m_ncommands;
                    }

                    /** Print receivers and their commands count.
                     */
                    void print();

                    /** Check for available characters in the serial interface and
                     ** if available then try to interpret as a command and execute
                     ** it. 
                     ** @return whether there was data or not available in the
                     ** serial interface. The actual success to match to a command
                     ** and the outcome of that command is not represented at all by
                     ** the return value.
                     */
                    bool execute();

                private:
                    static const unsigned int m_maxCommands = 20;
                    unsigned int m_ncommands;
                    Command *m_commands[m_maxCommands];

                    /** Singleton with a private constructor.
                     */
                    CommandList();

                };
            }
        }
    }
}

#endif /* _Host_H_ */

/*___oOo__*/
