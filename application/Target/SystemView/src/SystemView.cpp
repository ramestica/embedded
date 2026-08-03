/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
 
//
// Support stuff
//

//
// Target stuff
//

//
// Logic stuff
// 

//
// Local stuff
//
#include "SEGGER_SYSVIEW.h"
#include "SystemView.h"

//-------------------------------------------------------------------------------
void (*systemViewConf)() = SEGGER_SYSVIEW_Conf;
void (*systemViewOnIdle)() = SEGGER_SYSVIEW_OnIdle;
void (*systemViewRecordEnterISR)() = SEGGER_SYSVIEW_RecordEnterISR;
void (*systemViewRecordExitISR)() = SEGGER_SYSVIEW_RecordExitISR;
void (*systemViewRecordVoid)(unsigned int EventId) = SEGGER_SYSVIEW_RecordVoid;
void (*systemViewRecordEndCall)(unsigned int EventId) = SEGGER_SYSVIEW_RecordEndCall;

/*___oOo___*/
