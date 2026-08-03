#ifndef _CoordinateAxes_H_
#define _CoordinateAxes_H_

//
// Arduino stuff
//
#include <Arduino.h>

//
// System stuff
//
#include <inttypes.h>

namespace RAM
{
    namespace Control
    {
        namespace CoordinateAxes
        {
            /** Cartesian system fix to some reference frame.
             */
            template <typename T = float, unsigned int XIDX = 0, unsigned int YIDX = 1, unsigned int ZIDX = 2>
            class Cartesian
            {
            public:
                enum AxisIndex
                {
                    X = XIDX,
                    Y = YIDX,
                    Z = ZIDX
                };
                
                const static char *AxisName[3];
                
                Cartesian():
                    m_values_p(NULL)
                {
                }
                
                Cartesian(T * const ptr):
                    m_values_p((XIDX < 0 || XIDX > 2
                                ||
                                YIDX < 0 || YIDX > 2 
                                ||
                                ZIDX < 0 || ZIDX > 2
                                ?
                                NULL
                                :
                                ptr))
                {
                    if ( m_values_p == NULL )
                    {
                        RAM::Library::Error::longjmp(COORDINATEAXES_ERR_CARTESIAN_INDEX_OUT);
                    }
                }
                
                T & operator [](const AxisIndex axis) const
                {
                    if ( m_values_p == NULL )
                    {
                        RAM::Library::Error::longjmp(COORDINATEAXES_ERR_CARTESIAN_NULL);
                    }
                    
                    return const_cast<T &>(m_values_p[axis]);
                }
                
                T (&(operator() () const))[3]
                {
                    if ( m_values_p == NULL )
                    {
                        RAM::Library::Error::longjmp(COORDINATEAXES_ERR_CARTESIAN_NULL);
                    }
                    
                    return *(T (*)[3])m_values_p;
                }
                
            private:
                T *const m_values_p;
            };
            
            /** Aircraft principal axes.
             */
            template <typename T, unsigned int ROLLIDX, unsigned int PITCHIDX, unsigned int YAWIDX>
            class AircraftPrincipalAxes
            {
                enum AxisIndex
                {
                    X = ROLLIDX,
                    Y = PITCHIDX,
                    Z = YAWIDX
                };
                
                const static char *AxisName[3];
            };
        }
    }
}

template <typename T, unsigned int XIDX, unsigned int YIDX, unsigned int ZIDX>
const char *RAM::Control::CoordinateAxes::Cartesian<T, XIDX, YIDX, ZIDX>::AxisName[3] =
{
    "X",
    "Y",
    "Z"
};

template <typename T, unsigned int ROLLIDX, unsigned int PITCHIDX, unsigned int YAWIDX>
const char *RAM::Control::CoordinateAxes::AircraftPrincipalAxes<T, ROLLIDX, PITCHIDX, YAWIDX>::AxisName[3] =
{
    "ROLL",
    "PITCH",
    "YAW"
};

#endif /* _CoordinateAxes_H_ */

/*___oOo__*/
