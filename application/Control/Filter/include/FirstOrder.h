#ifndef _FirstOrder_H_
#define _FirstOrder_H_

//
// System stuff
//
#include <math.h>

namespace RAM
{
    namespace Control
    {
        namespace Filter
        {
            template <unsigned int DIMENSION = 1>
            class FirstOrder
            {
            public:
                FirstOrder():
                    m_alpha(1.0f)
                {
                    for ( unsigned int i = 0; i < DIMENSION; ++i )
                    {
                        m_output_memory[i] = 0.0f;
                    }
                }

                FirstOrder(const float &fsHz, const float &fcHz):
                    m_alpha(fcHz / (fcHz + fsHz / (2.0f * M_PI)))
                {
                    for ( unsigned int i = 0; i < DIMENSION; ++i )
                    {
                        m_output_memory[i] = 0.0f;
                    }
                }

                void update(const float (&x)[DIMENSION], float (&y)[DIMENSION]) 
                {
                    if ( !m_isInitialized )
                    {
                        for ( unsigned int i = 0; i < DIMENSION; ++i )
                        {
                            m_output_memory[i] = x[i];
                        }
                         
                        m_isInitialized = true;
                    }

                    for ( unsigned int i = 0; i < DIMENSION; ++i )
                    {
                        y[i] = m_output_memory[i] = m_alpha * x[i] + (1.0f - m_alpha) * m_output_memory[i];
                    }
                }

            private:
                bool m_isInitialized = false;
                float m_alpha;
                float m_output_memory[DIMENSION];
            };
        }
    }
}

#endif /* _FirstOrder_H_ */

/*___oOo___*/
