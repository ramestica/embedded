/** Would I ever have a declaration statement? 
 */

//
// Target stuff
//

//
// System stuff
//
 
//
// Local stuff
//
#include "Target.h"

#if defined(TARGET_nano328p) || defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
#else
#error "unknown target" 
#endif

//
// FIXME: provide this to make possible to define virtual methods, it
// is not available by default.
//
extern "C" void __cxa_pure_virtual() { while (1); }

/*___oOo___*/
