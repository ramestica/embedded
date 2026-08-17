/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// Target stuff
//
#include <Target/GPIO.h>

//
// System stuff
//
 
//
// Local stuff
//
#include <Target/Timer.h>

using namespace RAM::Target;

//-------------------------------------------------------------------------------
float Timer::getExactPrescaler(
    const uint32_t periodus,
    const unsigned nbits)
{
    float ret = periodus * (F_CPU / 1e6) / (1ULL << nbits);

    return (ret < 1 ? 1 : ret);
}

/*___oOo___*/
