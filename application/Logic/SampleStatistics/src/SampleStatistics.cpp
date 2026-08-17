//
// System stuff
//
#include <math.h>
#include <string.h> /* memset */

//
// Local stuff
//
#include <Logic/SampleStatistics.h>

using namespace std;
using namespace RAM::Logic;

//----------------------------------------------------------------------------------
SampleStatistics::SampleStatistics()
{
    memset(m_data, 0, m_dataSize);

    m_mean_p = reinterpret_cast<float *>(m_data);
    m_min_p = m_mean_p + 1;
    m_max_p = m_min_p + 1;
    m_var_p = m_max_p + 1;

    m_n_p = reinterpret_cast<uint32_t *>(m_var_p + 1);
    m_nmin_p = m_n_p + 1;
    m_nmax_p = m_nmin_p + 1;
}

//----------------------------------------------------------------------------------
void SampleStatistics::addSample(const float &sample)
{
    double mean_n = *m_mean_p;

    if ( *m_n_p == 0 )
    {
        *m_n_p = 1;

	*m_mean_p = *m_min_p = *m_max_p = sample;

	*m_var_p = 0;	

	return;
    }

    if ( sample > *m_max_p )
    { 
	*m_max_p = sample;

        //
        // sample index not count
        //
        *m_nmax_p = *m_n_p;
    }

    if ( sample < *m_min_p )
    {
	*m_min_p = sample;

        //
        // sample index not count
        //
        *m_nmin_p = *m_n_p;
    }

    *m_mean_p += (sample - *m_mean_p) / double(*m_n_p + 1);

    *m_var_p =
	(double(1) - double(1) / double(*m_n_p)) * (*m_var_p)
        +
	(double(1) + double(*m_n_p)) * (*m_mean_p - mean_n) * (*m_mean_p - mean_n);

    ++(*m_n_p);

    return;
}

/*___oOo___*/
