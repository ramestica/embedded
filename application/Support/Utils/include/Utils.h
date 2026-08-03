#ifndef _Utils_H_
#define _Utils_H_

//
// System stuff
//
#include <stdio.h>
#include <cstdint>

//
// Target stuff
//

//
// RAM stuff
// 

//
// Local stuff
//

namespace RAM
{
    namespace Support
    {
        namespace Utils
        {
            /** String variable on which ntoa works out its transformation to
             ** string.
             **
             ** FIXME: to allow some degree of reentrancy the string should be
             ** protected, or there should a bunch of different instances of the
             ** string buffer used by instances that do guarantee no reentrancy on
             ** their ntoa usage.
             */
            extern const char ntoa_digits [];

            /** Given a number produce a string representation in the character
             ** buffer passed as parameter.
             ** @param value a number to stringify.
             ** @param buffer string buffer, it must be enough to hold as many
             ** digits in the number value.
             ** @param base a two's power.
             ** @param flen length of the fractional part after the decimal comma.
             */
            template <typename T>
            size_t ntoa(T value, char *buffer, int base, int flen = 0)
            {
                int sign = 0;
                int quot, rem;
                char *ptr = buffer;
                char tmpc;
                unsigned dpart, fpart, precision = 1;

                if ( (base < 2) || (base > 16) )
                {
                    *buffer = '\0';

                    return 0;
                }

                //
                // precision digits
                //
                if ( flen )
                {
                    for ( rem = 0; rem < flen; ++rem )
                    {
                        precision *= 10;
                    }
                }

                //
                // the algorithm works on a positive number, record its sign here
                //
                if ( value < 0 )
                {
                    dpart = (unsigned)(-value);

                    fpart = (unsigned)((-value - dpart) * precision);

                    sign = 1;
                }
                else
                {
                    dpart = (value);

                    fpart = ((value - dpart) * precision);
                }

                //
                // if there is a fractional part then do that first
                //
                if ( flen != 0 )
                {
                    buffer[flen] = '.';

                    do
                    {
                        quot = fpart / base;

                        rem = fpart % base;

                        *ptr++  = ntoa_digits[rem];

                    }
                    while ( (fpart = quot) );

                    while ( ptr != buffer + flen )
                    {
                        *ptr++ = ntoa_digits[0];
                    }

                    ++ptr;
                }

                //
                // decimal part now
                //
                do
                {
                    quot = dpart / base;

                    rem = dpart % base;

                    *ptr++ = ntoa_digits[rem];

                }
                while ( (dpart = quot) );

                if ( sign )
                {
                    *ptr++ = '-';
                }

                size_t ret = ptr - buffer;

                *ptr-- = '\0';

                for ( ; buffer < ptr; ++buffer, --ptr )
                {
                    tmpc = *buffer;

                    *buffer = *ptr;

                    *ptr = tmpc;
                }

                return ret;
            }

            /** In val find bit index (zero based) to the given index-th bit which
             ** is set. Return 8 in case no so many bits are actually set.
             */
            uint8_t getBitIndex(uint8_t index, uint8_t val);
        
            /** Lock dynamic memory allocations (_sbrk reimplemented)
             ** and report currently allocated memory.
             */
            void mlock();
            size_t musage();

            /** A ubiquitous delay function, which simply counts a number of cpu
             ** cycles until completing.
             */
            void delayus(uint32_t us);
        }
    }
}

#endif /* _Utils_H_ */

/*___oOo__*/
