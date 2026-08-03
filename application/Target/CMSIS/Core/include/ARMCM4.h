/** CMSIS modules like DSP seem to assume that this file exists. Provided here
 ** to just include the expected core header. It seems also to be a good idea
 ** never to refer from application code to the specific header and instead let
 ** this file pick the correct one.
 */
#if defined(TARGET_tivac) || defined(TARGET_minim4)
#include <TM4C123GH6PM.h>
#else
#error "unknown target" 
#endif

/*___oOo__*/
