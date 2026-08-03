#ifndef _Complementary_H_
#define _Complementary_H_

namespace RAM
{
    namespace Control
    {
        namespace Filter
        {
            /** Complementary filter implementation.
             */
            class Complementary
            {
            public:
                Complementary();

                /** Time constant background information:
                 **
                 ** https://drive.google.com/file/d/0B9rLLz1XQKmaLVJLSkRwMTU0b0E/view
                 **
                 ** @param dt sampling period in seconds
                 ** @param tau filter's time constant in seconds
                 */
                Complementary(const float dt, const float tau);

                /** Update filtered value.
                 ** @param in filtered value.
                 ** @param in measured value 
                 ** @param in first derivative of measured value.
                 */
                void update(float &out, const float &in, const float &din);

            private:
                bool m_isInitialized = false;
                float m_dt;
                float m_alpha;
                float m_output_memory;
            };
        }
    }
}

#endif /* _Complementary_H_ */

/*___oOo___*/
