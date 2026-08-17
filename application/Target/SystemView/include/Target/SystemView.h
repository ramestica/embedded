#ifndef _SystemView_H_
#define _SystemView_H_

extern void (*systemViewConf)();
extern void (*systemViewOnIdle)();
extern void (*systemViewRecordEnterISR)();
extern void (*systemViewRecordExitISR)();
extern void (*systemViewRecordVoid)(unsigned int EventId);
extern void (*systemViewRecordEndCall)(unsigned int EventId);

#endif /* _SystemView_H_ */

/*___oOo__*/
