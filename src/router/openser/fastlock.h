/*
 * $Id: fastlock.h,v 1.2 2005/06/16 11:37:45 miconda Exp $
 *
 * fast architecture specific locking
 *
 * Copyright (C) 2001-2003 FhG Fokus
 *
 * This file is part of openser, a free SIP server.
 *
 * openser is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * openser is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *History:
 *--------
 *  2002-02-05  created by andrei
 *  2003-01-16  added PPC locking code contributed by Dinos Dorkofikis
 *               <kdor@intranet.gr>
 *  2004-09-12  added MIPS locking for ISA>=2 (>r3000)  (andrei)
 *  2004-12-16  for now use the same locking code for sparc32 as for sparc64
 *               (it will work only if NOSMP is defined) (andrei)
 *
 *  2005-04-27  added alpha locking code (andrei)
 *  2005-05-25  PPC locking code enabled for PPC64; added a lwsync to
 *               the tsl part and replaced the sync with a lwsync for the
 *               unlock part (andrei)
 */


#ifndef fastlock_h
#define fastlock_h

#ifdef HAVE_SCHED_YIELD
#include <sched.h>
#else
#include <unistd.h>
	/* fake sched_yield */
	#define sched_yield()	sleep(0)
#endif


typedef  volatile int fl_lock_t;



#define init_lock( l ) (l)=0



/*test and set lock, ret 1 if lock held by someone else, 0 otherwise*/
inline static int tsl(fl_lock_t* lock)
{
	int val;

#if defined(__CPU_i386) || defined(__CPU_x86_64)

#ifdef NOSMP
	val=0;
	asm volatile(
		" btsl $0, %1 \n\t"
		" adcl $0, %0 \n\t"
		: "=q" (val), "=m" (*lock) : "0"(val) : "memory", "cc" /* "cc" */
	);
#else
	val=1;
	asm volatile( 
		" xchg %b1, %0" : "=q" (val), "=m" (*lock) : "0" (val) : "memory"
	);
#endif /*NOSMP*/
#elif defined(__CPU_sparc64) || defined(__CPU_sparc)
	asm volatile(
			"ldstub [%1], %0 \n\t"
#ifndef NOSMP
			"membar #StoreStore | #StoreLoad \n\t"
#endif
			: "=r"(val) : "r"(lock):"memory"
	);
	
#elif defined (__CPU_arm)
	asm volatile(
			".arm \n\t"
			"# here \n\t"
			"swpb %0, %1, [%2] \n\t"
//			: "=r" (val)
			: "=&r" (val)
			: "r"(1), "r" (lock) : "memory"
	);

#elif defined(__CPU_armeb)
	asm volatile(
			".arm \n\t"
			"# here \n\t"
			"swpb %0, %1, [%2] \n\t"
			: "=r" (val)
			: "r"(1), "r" (lock) : "memory"
	);

#elif defined(__CPU_ppc) || defined(__CPU_ppc64)
	asm volatile(
			"1: lwarx  %0, 0, %2\n\t"
			"   cmpwi  %0, 0\n\t"
			"   bne    0f\n\t"
			"   stwcx. %1, 0, %2\n\t"
			"   bne-   1b\n\t"
			"   lwsync\n\t" /* lwsync or isync, lwsync is faster
							   and should work, see
							   [ IBM Programming environments Manual, D.4.1.1]
							 */
			"0:\n\t"
			: "=r" (val)
			: "r"(1), "b" (lock) :
			"memory", "cc"
        );
#elif defined __CPU_mips2
	long tmp;
	tmp=1; /* just to kill a gcc 2.95 warning */
	
	asm volatile(
		".set noreorder\n\t"
		"1:  ll %1, %2   \n\t"
		"    li %0, 1 \n\t"
		"    sc %0, %2  \n\t"
		"    beqz %0, 1b \n\t"
		"    nop \n\t"
		".set reorder\n\t"
		: "=&r" (tmp), "=&r" (val), "=m" (*lock) 
		: "0" (tmp), "2" (*lock) 
		: "cc"
	);
#elif defined __CPU_alpha
	long tmp;
	tmp=0;
	/* lock low bit set to 1 when the lock is hold and to 0 otherwise */
	asm volatile(
		"1:  ldl %0, %1   \n\t"
		"    blbs %0, 2f  \n\t"  /* optimization if locked */
		"    ldl_l %0, %1 \n\t"
		"    blbs %0, 2f  \n\t" 
		"    lda %2, 1    \n\t"  /* or: or $31, 1, %2 ??? */
		"    stl_c %2, %1 \n\t"
		"    beq %2, 1b   \n\t"
		"    mb           \n\t"
		"2:               \n\t"
		:"=&r" (val), "=m"(*lock), "=r"(tmp)
		:"1"(*lock)  /* warning on gcc 3.4: replace it with m or remove
						it and use +m in the input line ? */
		: "memory"
	);
#else
#error "unknown architecture"
#endif
	return val;
}



inline static void get_lock(fl_lock_t* lock)
{
#ifdef ADAPTIVE_WAIT
	int i=ADAPTIVE_WAIT_LOOPS;
#endif
	
	while(tsl(lock)){
#ifdef BUSY_WAIT
#elif defined ADAPTIVE_WAIT
		if (i>0) i--;
		else sched_yield();
#else
		sched_yield();
#endif
	}
}



inline static void release_lock(fl_lock_t* lock)
{
#if defined(__CPU_i386) || defined(__CPU_x86_64)
	char val;
	val=0;
	asm volatile(
		" movb $0, (%0)" : /*no output*/ : "r"(lock): "memory"
		/*" xchg %b0, %1" : "=q" (val), "=m" (*lock) : "0" (val) : "memory"*/
	); 
#elif defined(__CPU_sparc64) || defined(__CPU_sparc)
	asm volatile(
#ifndef NOSMP
			"membar #LoadStore | #StoreStore \n\t" /*is this really needed?*/
#endif
			"stb %%g0, [%0] \n\t"
			: /*no output*/
			: "r" (lock)
			: "memory"
	);
#elif defined __CPU_arm
	asm volatile(
		" str %0, [%1] \n\r" 
		: /*no outputs*/ 
		: "r"(0), "r"(lock)
		: "memory"
	);
#elif defined __CPU_armeb
	asm volatile(
		" str %0, [%1] \n\r" 
		: /*no outputs*/ 
		: "r"(0), "r"(lock)
		: "memory"
	);
#elif defined(__CPU_ppc) || defined(__CPU_ppc64)
	asm volatile(
			/* "sync\n\t"  lwsync is faster and will work
			 *             here too
			 *             [IBM Prgramming Environments Manual, D.4.2.2]
			 */
			"lwsync\n\t"
			"stw %0, 0(%1)\n\t"
			: /* no output */
			: "r"(0), "b" (lock)
			: "memory"
    );
	*lock = 0;
#elif defined __CPU_mips2
	asm volatile(
		".set noreorder \n\t"
		"    sync \n\t"
		"    sw $0, %0 \n\t"
		".set reorder \n\t"
		: /*no output*/  : "m" (*lock) : "memory"
	);
#elif defined __CPU_alpha
	asm volatile(
		"    mb          \n\t"
		"    stl $31, %0 \n\t"
		: "=m"(*lock) :/* no input*/ : "memory"  /* because of the mb */
	);  
#else
#error "unknown architecture"
#endif
}



#endif
