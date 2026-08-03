#ifndef _Eeprom_H_
#define _Eeprom_H_

//
// System stuff
//
#include <stddef.h>
#include <inttypes.h>

//
// RAM stuff
// 
#include <ErrorCodes.h>

namespace RAM
{
    namespace Library
    {
        namespace EEPROM
        {
            int enable();

            int disable();
            
            template <typename DataType>
            void read(uint8_t id, DataType *data, size_t size)
            {
                unsigned int loc = 0;

                while ( EEPROM::read(loc) == magic && loc < capacity )
                {
                    ++loc;

                    if ( EEPROM::read(loc++) == id )
                    {
                        break;
                    }

                    loc += EEPROM::read(loc);
                }

                if ( loc >= capacity )
                {
                    Error::longjmp(EEPROM_ERR_ID_NOT_FOUND);
                }

                if ( EEPROM::read(loc++) != size * sizeof(DataType) )
                {
                    Error::longjmp(EEPROM_ERR_SIZE_MISMATCH);
                }

                for ( unsigned int i = 0; i < size; ++i )
                {
                    for ( unsigned int j = 0; j < sizeof(DataType); ++j )
                    {
                        reinterpret_cast<uint8_t *>(&data[i])[j] = EEPROM::read(loc++);
                    }
                }
            }

            void write(unisgned id, void *data, size_t size)
            {   
            }

            void erase(unsigned int id)
            {
            }
        }
    }
}

#endif /* _Eeprom_H_ */

/*___oOo__*/
