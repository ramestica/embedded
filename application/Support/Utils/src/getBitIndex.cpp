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
#include <string.h>

//
// Local stuff
//
#include <Support/Utils.h>

//
// Globals
//
const char RAM::Support::Utils::ntoa_digits [] = "0123456789ABCDEF";

//-------------------------------------------------------------------------------
uint8_t RAM::Support::Utils::getBitIndex(uint8_t index, uint8_t val)
{
    uint8_t ret = 0, pos;
    
    if ( index >= __builtin_popcount(val) )
    {
        return sizeof(val) * 8;
    }
    
    for ( unsigned i = 0; i <= index; ++i )
    {
        pos = ffs(val);
                
        val >>= pos;
        
        ret += pos;
    }
    
    return ret - 1;
}
 
/*___oOo___*/
