/** Would I ever have a declaration statement? 
 */

/*
  wiring.c - Partial implementation of the Wiring API for the ATmega8.
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id$
*/

#ifndef __cplusplus
#error this compilation unit requires c++
#endif

//
// System stuff
//
#include <inttypes.h>
#ifdef TARGET_nano328p
//#include <util/delay_basic.h>
#define __DELAY_BACKWARD_COMPATIBLE__ 
#include <util/delay.h>
#endif

//
// Local stuff
//
#include <Support/Utils.h>

#if defined(TARGET_tivac) || defined(TARGET_minim4) || defined(TARGET_discovery)
static void __attribute__((naked)) SysCtlDelay(uint32_t l)
{
    __asm(
        "    Loop:\n"
        "    subs    r0, #1\n"
        "    bne     Loop\n"
        "    bx      lr");
}
#endif

//-------------------------------------------------------------------------------
void RAM::Support::Utils::delayus(uint32_t us)
{
#ifdef TARGET_nano328p
#if F_CPU != 16000000L
#error "unsupported cpu frequency"
#endif

    while ( us >= 1000 )
    {
        _delay_ms(1);
        
        us -= 1000;
    }
    
    // for a one-microsecond delay, simply return.  the overhead
    // of the function call yields a delay of approximately 1 1/8 us.
    if (us == 0 || --us == 0)
    {
        return;
    }
    
    // the following loop takes a quarter of a microsecond (4 cycles)
    // per iteration, so execute it four times for each microsecond of
    // delay requested.
    us <<= 2;
    
    // account for the time taken in the preceeding commands.
    us -= 2;
    
    // busy wait
    __asm__ __volatile__ (
        "1: sbiw %0,1" "\n\t" // 2 cycles
        "brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
#else
    //
    // the assembly implementation takes 3 cycles per loop
    //
    SysCtlDelay((us * F_CPU_CYCLES_PER_US) / 3);
//    us = (us * (F_CPU / 1000000)) / 5;
//    __asm
//        ("Loop:;
//         subs	$0x1, %0;
//         bne	Loop;"
//         :
//         :"r"(us));

#endif
}

/*___oOo___*/
