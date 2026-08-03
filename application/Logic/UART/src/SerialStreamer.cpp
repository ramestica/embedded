/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Local stuff
//
#include "UART.h"

using namespace std;
using namespace RAM::Logic::UART;

//-------------------------------------------------------------------------------
SerialStreamer::SerialStreamer():
    RAM::Logic::Host::DataStream(10, 10)
{
}
 
/*___oOo___*/
