#ifndef _SAMPLE_STATISTICS_H
#define _SAMPLE_STATISTICS_H

//
// System stuff
//
#include <stddef.h>
#include <inttypes.h>

#define SS_NUMS_PTR (reinterpret_cast<unsigned int *>(m_data))
#define SS_STATS_PTR (reinterpret_cast<float *>(m_data + 3 * sizeof(unsigned int)))

namespace RAM
{
    namespace Logic
    {
        /** Accepts a sample value and updates the number of times a sample has
         ** been added, its mean, minimum and maximum values across through all
         ** those samples. Minimum and maximum figures go together with the
         ** sample count at which they did happen, respectively. Data is
         ** internally held in an array with the following structure:
         **
         ** | float | float | float | float | uint32  | uint32  | uint32  |
         ** +-------+-------+-------+-------+---------+---------+---------+
         ** | mean  | min   | max   | var   | cnt     | nmin    | nmax    |
         **
         ** Providing everything in a contiguous buffer should ease data
         ** transference to host application.
         */
        struct SampleStatistics
        {
            const static size_t m_dataSize=
                4 * sizeof(float) + 3 * sizeof(uint32_t);
 
            //
            // Contructor just simply sets number of samples to zero.
            //
            SampleStatistics();

            //
            // variables for holding timing stats.
            //
            uint8_t m_data[m_dataSize];
            float *m_mean_p;
            float *m_min_p;
            float *m_max_p;
            float *m_var_p;
            uint32_t *m_n_p;
            uint32_t *m_nmin_p;
            uint32_t *m_nmax_p;
            
            //
            // compute mean and variance as a running value over all available
            // samples. A recursive implementation is used here, therefore,
            // passed parameters must contain already computed values in the
            // previous iteration. That's normally true except for n=0, at which
            // moment mean and rms are intialized accordingly.
            //
            // http://mathworld.wolfram.com/SampleVarianceComputation.html
            //
            void addSample(const float &sample);
        };
    }
}

#endif /* _SAMPLE_STATISTICS_H */

/*___oOo___*/
