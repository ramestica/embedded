/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
 
//
// System stuff
//
 
//
// Local stuff
//
#include <Logic/Host.h>

using namespace RAM::Logic::Host;

//-------------------------------------------------------------------------------
unsigned DataStream::write(const char *str, const unsigned length)
{
    char c;
    unsigned count, slen = length;
                
    //
    // go pushing only when there is something to push
    //
    while ( slen )
    {
        //
        // push into queue as many as possible.
        //
        count = m_txq.push(str, slen);

        //
        // if the fifo became empty while pushing into the queue
        // then make sure that the new data will be transmitted by
        // priming the fifo with one character.
        //
        if ( isTransmitChannelEmpty() )
        {
            if ( m_txq.pop(&c, 1) )
            {
                writeIntoChannel(c);
            }
        }

        str += count;
                    
        slen -= count;
    }

    return length;
}
 
/*___oOo___*/
