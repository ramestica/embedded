#ifndef _UtilsQueue_H_
#define _UtilsQueue_H_

//
// System stuff
//
#include <inttypes.h>

namespace RAM
{
    namespace Logic
    {
        /** A single-producer and single-consumer lock free and
         ** bounded queue.  The implementation is based on two
         ** indexes, one to point to the last dequeued item and one to
         ** point to the last enqueued item. Dequeuing and enqueuing
         ** operations have write access to their associated index
         ** only. By making a copy of the other index at the beginning
         ** of an operation then that operation can always progress
         ** without attending activity on the other side.
         **
         ** Methods for pushing and popping data in and out the queue
         ** are based on a loop that will finish when all the provided
         ** or requested data has been completed. That means that it
         ** is assumed that progress is happening concurrently in the
         ** inverse direction. That is, when pushing then popping is
         ** happening from some other context; which for a bare-metal
         ** application it means that while the main loop is pushing
         ** or popping then at the same time from interrupt level an
         ** isr is popping or pushing, respectively.
         **
         ** Note the difference between the verb dequeue (remove from a
         ** queue) and the noun deque (double-ended queue).
         */
        class Queue
        {
        public:
            /** Default constructor assumes a given buffer capacity. Use
             ** malloc to allocate.
             */
            Queue();

            /** Constructor receives size in bytes to use as buffer
             ** capacity. Use malloc to allocate.
             */
            Queue(const unsigned n);

            /** Add items to queue. If at any moment the queue is detected
             ** full then ???
             */
            size_t push(const void *ptr, const size_t n);

            /** Helper push method useful when the variable to push in is a
             ** temporary output of some method.
             */
            inline size_t push(const char &c, const size_t n)
            {
                return push(reinterpret_cast<const void *>(&c), n);
            }

            /** Remove a maximum number of items from the queue.
             ** @param ptr pointer to buffer where to write extracted data.
             ** @param max maximum number of items to dequeue.
             ** @return actual number of actually dequeued items.
             */
            size_t pop(void *ptr, const size_t max);

            /** Items in buffer waiting to be removed. The method works as
             ** 'pop' in the sense that it makes a copy of the enqueued
             ** index before making the calculation.
             */
            unsigned queued();

        private:
            
            /** A helper method to calculate the distance between two given
             ** indexes to the buffer. Head and tail are indexes to
             ** positions in the queue. The method returns N such that
             ** tail+N equals head.
             */
            size_t distance(const unsigned &tail, const unsigned &head);

            /** Queue's size is forced to be a power of two.
             */
            unsigned m_n;

            /** Buffer size in bytes.
             */
            size_t m_size;

            /** Slot index of last enqueued item.
             */
            unsigned m_enqueuex = 0;

            /** Slot index of last dequeued item.
             */
            unsigned m_dequeuex = 0;

            /** Pointer to buffer.
             */
            void *m_buffer = nullptr;
        };
    }
}

#endif /* _UtilsQueue_H_ */

/*___oOo__*/
