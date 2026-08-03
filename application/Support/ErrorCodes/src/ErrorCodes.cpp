//
// Local stuff
//
#include "ErrorCodes.h"

using namespace std;

//
// Error name space global variables
//
jmp_buf RAM::Support::Error::errorJump;
int RAM::Support::Error::longjmpCode;

//-------------------------------------------------------------------------------
void RAM::Support::Error::longjmp(const uint16_t err)
{
    ::longjmp(errorJump, (longjmpCode = err));
}

/*___oOo___*/
