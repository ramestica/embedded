/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Library stuff
#include <Support/ErrorCodes.h>

//
// Local stuff
//
#include <Logic/Host.h>

static RAM::Logic::Host::DataStream *dataStream_p = nullptr;

using namespace RAM::Logic::Host::CommandReply;

//-------------------------------------------------------------------------------
Command::Command():
    m_receiverId(static_cast<uint8_t>(0)),
    m_commandId(static_cast<uint8_t>(0)),
    m_dataStream_p(nullptr)
{
}

//-------------------------------------------------------------------------------
Command::Command(const int recvid, const int cmdid):
    m_receiverId(static_cast<uint8_t>(recvid)),
    m_commandId(static_cast<uint8_t>(cmdid)),
    m_dataStream_p(RAM::Logic::Host::getCommandReplyDataStream())
{
}

//-------------------------------------------------------------------------------
void Command::reply(const int status)
{
    m_dataStream_p->write(m_receiverId);

    m_dataStream_p->write(m_commandId);

    m_dataStream_p->write(static_cast<uint8_t>(status));
}

//-------------------------------------------------------------------------------
void Command::reply(const int status, void *data, const size_t size)
{
    reply(status);

    m_dataStream_p->write(reinterpret_cast<char *>(data), size);
}

//-------------------------------------------------------------------------------
void Command::reply(const int status, const uint8_t data)
{
    reply(
        status,
        reinterpret_cast<void *>(const_cast<uint8_t *>(&data)),
        sizeof(uint8_t));
}

//-------------------------------------------------------------------------------
bool Command::read(void *out, const unsigned bytes, unsigned timeout)
{
    char *ccurrent = reinterpret_cast<char *>(out);
    char *cend = ccurrent + bytes;
    unsigned int available;

    while ( ccurrent != cend && timeout != 0 )
    {
        if ( (available =
              RAM::Logic::Host::getCommandReplyDataStream()->
              available())
             == 0 )
        {
            RAM::Support::Utils::delayus(1);

            --timeout;

            continue;
        }

        for ( unsigned int i = 0;
              i != available && ccurrent != cend;
              ++i, ++ccurrent )
        {
            RAM::Logic::Host::getCommandReplyDataStream()->
                read(*ccurrent);
        }
    }

    if ( ccurrent != cend )
    {
        reply(ReplyId::DATA_TIMEOUT);
                        
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------------
bool Command::write(void const *in, const unsigned bytes)
{
    RAM::Logic::Host::getCommandReplyDataStream()->write(
        reinterpret_cast<char const *>(in),
        bytes);

    return true;
}

//-------------------------------------------------------------------------------
CommandList &CommandList::instance()
{
    static CommandList instance;
    
    return instance;
}

//-------------------------------------------------------------------------------
CommandList::CommandList():
    m_ncommands(0)
{
    dataStream_p = RAM::Logic::Host::getCommandReplyDataStream();
}

//-------------------------------------------------------------------------------
void CommandList::setCommandHandler(Command *cmd)
{
    //if ( dataStream_p == nullptr )
    //{
    //    RAM::Support::Error::longjmp(SERIAL_COMMANDS_ERR_DATA_STREAM_NULL);
    //}
    
    if ( m_ncommands == m_maxCommands )
    {
        RAM::Support::Error::longjmp(SERIAL_COMMANDS_ERR_CMD_TOO_MANY);
    }
    
    for ( unsigned int i = 0; i < m_ncommands; ++i )
    {
        if ( m_commands[i]->m_receiverId == cmd->m_receiverId
             &&
             m_commands[i]->m_commandId == cmd->m_commandId )
        {
            RAM::Support::Error::longjmp(SERIAL_COMMANDS_ERR_CMD_BUSY);
        }
    }

    m_commands[m_ncommands] = cmd;

    ++m_ncommands;
}

//-------------------------------------------------------------------------------
void CommandList::updateDataStream()
{
    if ( (dataStream_p = RAM::Logic::Host::getCommandReplyDataStream())
         == nullptr )
    {
        RAM::Support::Error::longjmp(SERIAL_COMMANDS_ERR_DATA_STREAM_NULL);
    }
    
    for ( unsigned int i = 0; i < m_ncommands; ++i )
    {
        m_commands[i]->m_dataStream_p =
            RAM::Logic::Host::getCommandReplyDataStream();
    }
}

//-------------------------------------------------------------------------------
void CommandList::print()
{
    for ( unsigned int i = 0; i < m_ncommands; ++i )
    {
        RAM::Logic::Host::Console::print(m_commands[i]->m_receiverId);
        
        RAM::Logic::Host::Console::print(":");

        RAM::Logic::Host::Console::println(m_commands[i]->m_commandId);
    }
}

//-------------------------------------------------------------------------------
bool CommandList::execute()
{
    if ( dataStream_p->available() == 0 )
    {
        return false;
    }

    uint8_t recvid, cmdid;
    unsigned int cmd;

    if ( dataStream_p->read(reinterpret_cast<char &>(recvid)) == 0 )
    {
        return false;
    }
    
    if ( dataStream_p->read(reinterpret_cast<char &>(cmdid)) == 0 )
    {
        return false;
    }
    
    for ( cmd = 0; cmd < m_ncommands; ++cmd )
    {
        if ( m_commands[cmd]->m_receiverId == recvid
             &&
             m_commands[cmd]->m_commandId == cmdid )
        {
            break;
        }
    }
    
    if ( cmd == m_ncommands )
    {
        Command(recvid, cmdid).reply(ReplyId::UNKNOWN);
        
        return false;
    }
    
    if ( m_commands[cmd] == NULL )
    {
        Command(recvid, cmdid).reply(ReplyId::NULL_HANDLER);
        
        return false;
    }
    
    //
    // execute concrete implementation through base class pointer
    //
    m_commands[cmd]->execute();
    
    return true;
}

/*___oOo__*/
