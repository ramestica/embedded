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

//
// Library stuff
//
#include <Support/ErrorCodes.h>

//
// Local stuff
//
#include <Logic/Host.h>

//
// global variables
//
static RAM::Logic::Host::DataStream *m_consoleChannel_p = nullptr;
static RAM::Logic::Host::DataStream *m_commandReplyChannel_p = nullptr;

//
// global used to format numbers into strings before printing out.
//
// FIXME: to give some concurrency lee way the buffer variable would need to be
// protected, or different instances given to different participants that know
// they do not print more than once at the same time.
//
char RAM::Logic::Host::Console::buffer[256];

//-------------------------------------------------------------------------------
void RAM::Logic::Host::setCommunicationStreams(
    RAM::Logic::Host::DataStream *consoleChannel_p,
    RAM::Logic::Host::DataStream *commandReplyChannel_p)
{
    m_consoleChannel_p = consoleChannel_p;
        
    m_commandReplyChannel_p = commandReplyChannel_p;
}

//-------------------------------------------------------------------------------
RAM::Logic::Host::DataStream *RAM::Logic::Host::getConsoleDataStream()
{
    //if ( m_printChannel_p == nullptr )
    //{
    //    RAM::Support::Error::longjmp(HOST_ERR_NULL_PRINTOUT_DATA_STREAM);
    //}
    
    return m_consoleChannel_p;
}

//-------------------------------------------------------------------------------
RAM::Logic::Host::DataStream *RAM::Logic::Host::getCommandReplyDataStream()
{
    //if ( m_commandReplyChannel_p == nullptr )
    //{
    //    RAM::Support::Error::longjmp(HOST_ERR_NULL_CMD_REPLY_DATA_STREAM);
    //}
    
    return m_commandReplyChannel_p;
}

/*___oOo___*/
