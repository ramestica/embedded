/** Would I ever have a declaration statement? 
 */

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
#include <stdlib.h>
#include <string.h>

//
// Support stuff
//
#include <Support/Utils.h>

//
// Local stuff
//
#include <Logic/Queue.h>

#define QUEUE_DEFAULT_NSIZE 8

#define MODULUS(x) ((x) - (((x) >> m_n) << m_n))

using namespace RAM::Logic;

//-------------------------------------------------------------------------------
Queue::Queue():
    m_n(QUEUE_DEFAULT_NSIZE),
    m_size(1 << m_n)
{
    if ( (m_buffer = malloc(m_size)) == nullptr )
    {
    }
}

//-------------------------------------------------------------------------------
Queue::Queue(const unsigned n):
    m_n(n),
    m_size(1 << m_n)
{
    m_buffer = malloc(m_size);
}

//-------------------------------------------------------------------------------
size_t Queue::push(const void *ptr, const size_t n)
{
    size_t available, toCopy;
    unsigned copied = 0;
    unsigned dx;

    //
    // enqueue until all them had been copied into the buffer
    //
    while ( copied != n )
    {
        //
        // current dequeue index (index of last dequeued item)
        //
        dx = m_dequeuex;

        //
        // number of available (free) bytes to write onto. To avoid
        // inconsistencies between enqueuing and dequeuing indexes every
        // enqueuing operation should never fill the queue up to N but at most
        // up to N-1 only.

        //
        if ( (available = distance(m_enqueuex, dx)) == 0 )
        {
            available = m_size - 1;
        }
        else
        {
            --available;
        }

        //
        // if no space available to enqueue then break to return actual
        // number of bits so far enqueued
        // FIXME: would it make any sense to wait for space available?
        //
        if ( available == 0 )
        {
            break;
        }

        //
        // bytes to copy into buffer
        //
        toCopy = (available >= (n - copied) ? (n - copied) : available);

        //
        // copy data from input parameter into the queue
        //
        if ( m_enqueuex + toCopy >= m_size )
        {
            memcpy(
                static_cast<uint8_t *>(m_buffer) + m_enqueuex + 1,
                static_cast<const uint8_t *>(ptr) + copied,
                m_size - (m_enqueuex + 1));

            memcpy(
                static_cast<uint8_t *>(m_buffer),
                static_cast<const uint8_t *>(ptr) + copied + m_size - (m_enqueuex + 1),
                toCopy - (m_size - (m_enqueuex + 1)));
        }
        else
        {
            memcpy(
                static_cast<uint8_t *>(m_buffer) + m_enqueuex + 1,
                static_cast<const uint8_t *>(ptr) + copied,
                toCopy);
        }

        //
        // update enqueued index to represent index where to enqueue next
        //
        m_enqueuex = MODULUS(m_enqueuex + toCopy);

        copied += toCopy;
    }

    return copied;
}

//-------------------------------------------------------------------------------
size_t Queue::pop(void *ptr, const size_t max)
{
    size_t available, toCopy;
    unsigned copied = 0;
    unsigned ex;

    //
    // dequeue items until one of the following conditions becomes true:
    //
    //    1. no more available items (queue empty)
    //    2. maximum number of items to dequeue reached
    //
    while ( (ex = m_enqueuex) != m_dequeuex && copied != max )
    {
        //
        // available items to dequeue
        //
        available = distance(m_dequeuex, ex);

        //
        // do not copy more than max
        //
        toCopy = (max - copied < available) ? max - copied : available;

        //
        // copy data from queue into input parameter
        //
        if ( m_dequeuex + toCopy >= m_size )
        {
            memcpy(
                static_cast<uint8_t *>(ptr) + copied,
                static_cast<uint8_t *>(m_buffer) + m_dequeuex + 1,
                m_size - (m_dequeuex + 1));

            memcpy(
                static_cast<uint8_t *>(ptr) + copied + m_size - (m_dequeuex + 1),
                static_cast<uint8_t *>(m_buffer),
                toCopy - (m_size - (m_dequeuex + 1)));
        }
        else
        {
            memcpy(
                static_cast<uint8_t *>(ptr) + copied,
                static_cast<uint8_t *>(m_buffer) + m_dequeuex + 1,
                toCopy);
        }

        //
        // update dequeue index to represent index of last dequeued item
        //
        m_dequeuex = MODULUS(m_dequeuex + toCopy);

        copied += toCopy;            
    }

    return copied;
}

//-------------------------------------------------------------------------------
unsigned Queue::queued()
{
    unsigned ex = m_enqueuex;

    return distance(m_dequeuex, ex);
}

//-------------------------------------------------------------------------------
size_t Queue::distance(const unsigned &tail, const unsigned &head)
{
    if ( tail <= head )
    {
        return head - tail;
    }

    return head + m_size - tail;
}
 
/*___oOo___*/
