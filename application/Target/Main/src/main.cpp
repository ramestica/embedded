/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Target/Target.h>

//
// Local stuff
//
#include <Target/Main.h>

using namespace RAM::Target;

//-------------------------------------------------------------------------------
int main(void)
{
    Main::setup();

    (*systemViewConf)();

    (*systemViewOnIdle)();
    
    while ( true )
    {
         Main::loop();
    }

    return 0;
}

/*___oOo___*/
