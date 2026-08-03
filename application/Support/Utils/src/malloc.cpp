/** Would I ever have a declaration statement? 
 */

//
// System stuff
//
#include <stdlib.h>
#include <errno.h>
#include <cstdint>

//
// Base stuff
//
#include <ErrorCodes.h>

//
// Target stuff
//
 
//
// Local stuff
//
#include "Utils.h"

//
// Global static variables
//
static bool isLocked = false;

#define CHARP(addr) static_cast<char *>(addr)

/* Register name faking - works in collusion with the linker.  */
register char * stack_ptr asm ("sp");
static char * heap_start;
static char * heap_end;
unsigned __heap_limit = 0xcafedead;

//
// Consult the following page for a rather useful description of
// malloc's implementation.
//
/** Original code copied from src/newlib/./libgloss/arm/syscalls.c
 */
extern "C" void *_sbrk (ptrdiff_t incr)
{
  extern char end asm ("end"); /* Defined by the linker.  */
  char * prev_heap_end;

  //
  // if not yet initialized then initialize heap start and end
  // pointers to end of allocated memory (end)
  //
  if ( heap_end == NULL )
  {
      heap_start = heap_end = &end;
  }
  
  prev_heap_end = heap_end;
  
  if ( (heap_end + incr > stack_ptr)
       /* Honour heap limit if it's valid.  */
       /* But I do not feel like enforcing a limit, therefore,
        * __heap_limit is left unchanged.
        */
       ||
       (__heap_limit != 0xcafedead && heap_end + incr > (char *)__heap_limit)
       ||
       /* whole purpose of my own sbrk */
       isLocked )
    {
        RAM::Support::Error::longjmp(MEMORY_ERR_ENOMEM);
    }
  
  heap_end += incr;

  return (void *) prev_heap_end;
}

// 
// reimplement new's allocator to use our own malloc
//
// FIXME: now that there is no 'own malloc' (reimplemented _sbrk
// instead), then, there is no reason for new/delete replacements.
//
void * operator new(std::size_t n)
{
    return malloc(n);
}
void * operator new[](std::size_t n)
{
    return ::operator new(n);
}
void operator delete(void * p)
{
    RAM::Support::Error::longjmp(MEMORY_ERR_DELETE_NOT_IMPLEMENTED);
}
void operator delete[](void *p)
{
    RAM::Support::Error::longjmp(MEMORY_ERR_DELETE_NOT_IMPLEMENTED);
}

//-----------------------------------------------------------------------------------
void RAM::Support::Utils::mlock()
{
    isLocked = true;
}

//-----------------------------------------------------------------------------------
size_t RAM::Support::Utils::musage()
{    
    return CHARP(heap_end) - CHARP(heap_start);
}

/*___oOo___*/
