#ifndef _Sensor_H_
#define _Sensor_H_

//
// Arduino stuff
//
#include <Arduino.h>

//
// System stuff
//
#include <inttypes.h>

//
// RAM stuff
//
#include <Endianness.h>
#include <SampleStatistics.h>

namespace RAM
{
    namespace Library
    {
        namespace Sensor
        {
            template <typename InputType, typename OutputType, unsigned int DIMENSION>
            class Base
            {
            public:

                Base():
                    m_in_p(NULL),
                    m_out_p(NULL)
                {;}

                Base(InputType * const in, OutputType * const out, const bool isInputLittleEndian):
                    m_in_p(in),
                    m_out_p(out),
                    m_adjustInputEndianess(false)
                {
                    /** Input values need to be corrected for local processor
                     ** endianess whenever both do not match.
                     */
                    if ( RAM::Library::Endianess::isLittleEndian() != isInputLittleEndian )
                    {
                        m_adjustInputEndianess = true;
                    }

                    for ( unsigned int i = 0; i < DIMENSION; ++i )
                    {
                        m_scale[i] = float(1);
                    }
                }

                ~Base() {;}

                unsigned int getDimension()
                {
                    return DIMENSION;
                }

                /** Convert sensed value into a physical value by calibrating
                 ** the number by means of an offset and scale factor.
                 */
                void calibrate()
                {
                    InputType tmp;

                    for ( unsigned int i = 0; i < DIMENSION; ++i )
                    {
                        if ( m_adjustInputEndianess )
                        {
                            RAM::Library::Endianness::adapt(m_in_p[i], tmp);
                        }
                        else
                        {
                            tmp = m_in_p[i];
                        }

                        m_out_p[i] = (tmp - m_offset[i].m_mean) * m_scale[i];
                    }
                }

            protected:
                /** Input and output data. Allocation must be provided
                 ** externally.
                 */
                InputType *m_in_p;
                OutputType *m_out_p;

                /** Big-endian systems are systems in which the most significant
                 ** byte of the word is stored in the smallest address given and
                 ** the least significant byte is stored in the largest. In
                 ** contrast, little endian systems are those in which the least
                 ** significant byte is stored in the smallest address.
                 */
                bool m_isInputBigEndian;

                /** ADC counts to physical unit.
                 */
                float m_scale[DIMENSION];
                
                /** At rest output in counts.
                 */
                RAM::Library::SampleStatistics m_offset[DIMENSION];
            };

            typedef Base<uint16_t, float, 3> 3DSensor;
        }
    }
}

#endif /* _Sensor_H_ */

/*___oOo__*/
