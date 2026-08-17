/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Local stuff
//
#include <Target/SystemView.h>

static inline void systemViewNop() { static_assert(true, "nopzxz"); }
static inline void systemViewUnsignedNop(unsigned int) { static_assert(true, "nopzxz"); }

//-------------------------------------------------------------------------------
void (*systemViewConf)() = systemViewNop;
void (*systemViewOnIdle)() = systemViewNop;
void (*systemViewRecordEnterISR)() = systemViewNop;
void (*systemViewRecordExitISR)() = systemViewNop;
void (*systemViewRecordVoid)(unsigned int EventId) = systemViewUnsignedNop;
void (*systemViewRecordEndCall)(unsigned int EventId) = systemViewUnsignedNop;

/*___oOo___*/
