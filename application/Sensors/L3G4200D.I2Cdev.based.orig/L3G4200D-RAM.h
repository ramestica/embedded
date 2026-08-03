#ifndef _L3G4200D_RAM_H_
#define _L3G4200D_RAM_H_

//
// Contributed stuff
//
#include <L3G4200D.h>

namespace RAM
{
    namespace Sensors
    {
        class L3G4200D: public ::L3G4200D
        {
        public:
            L3G4200D()
            {;}
            
            L3G4200D(const uint8_t addr);

            void initialize();

            void calibrate();

            void getAngularVelocity(int16_t &avx, int16_t &avy, int16_t &avz);

        private:
            uint8_t m_addr;

            int16_t m_offset[3];

            float m_gain;
        };
    }
}

#endif /* _L3G4200D-RAM_H_ */

/*___oOo___*/
