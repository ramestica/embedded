#ifndef _Endianess_H_
#define _Endianess_H_

//
// System stuff
//
#include <inttypes.h>

namespace RAM
{
    namespace Logic
    {
        namespace Endianness
        {
            /** Tell endianess of architecture where this software has been
             ** compiled.
             */
            bool isLittleEndian()
            {
                uint16_t word = 0x0001;

                char *byte = (char *)&word;

                return byte[0];
            }

            /** Convert endianess of value. In and out can share the same
             ** address in memory.
             */
            template <typename ValueType>
            void adapt(const ValueType &in, ValueType &out)
            {
                int ilower = (sizeof(ValueType) >> 0x1) - 1;
                int iupper = ilower + 1;
                char tmp;
                
                while ( ilower >= 0 )
                {
                    tmp = reinterpret_cast<const char *>(&in)[ilower];
                    
                    reinterpret_cast<char *>(&out)[ilower] = reinterpret_cast<const char *>(&in)[iupper];
                    
                    reinterpret_cast<char *>(&out)[iupper] = tmp;
                    
                    --ilower;
                    
                    ++iupper;
                }
            }
        }
    }
}

#endif /* _Endianess_H_ */

/*___oOo__*/
