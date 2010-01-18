/*
 *  linux/include/asm-arm/arch-sl2312/system.h
 *
 *  Copyright (C) 1999 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *  Copyright (C) 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef __ASM_ARCH_IPI_H
#define __ASM_ARCH_IPI_H
#include <asm/io.h>

//#define spin_lock(x)	spin_lock_dt(x)
//#define spin_unlock(x)	spin_unlock_dt(x)

#define SWAP_OFFSET							0x400000
#define SWAP_SIZE							0x400000

#define SHARE_MEM_ADDR						0x2000000
#define SHARE_MEM_SIZE						1024*1024


//--> Add by jason for IPI testing
// memory layout for maste & slave bin
#define MASTERTEXT      0x8000
#define SLAVETEXT		0x108000
#define SHARESIZE		0x4000
#define SHAREADDR		SHARE_MEM_ADDR // starting 8M

// CPU1 reset release
#define GLOBAL_BASE		IO_ADDRESS(0x40000000)
#define GLOBAL_SOFTRESET	(GLOBAL_BASE + 0x0C)
#define CPU1_RESET_BIT_MASK     0x40000000

// IPI , need to redefine the folliwing,  bug
#define CPU0_STATUS			(GLOBAL_BASE + 0x0038)
#define CPU1_STATUS			(GLOBAL_BASE + 0x003C)
#define CPU_IPI_BIT_MASK    0x80000000

/* Your basic SMP spinlocks, allowing only a single CPU anywhere
*/
typedef struct {
       volatile unsigned int lock;
} spinlock_dt;


#define         MASTER_BIT      0x01
#define         SLAVE_BIT       0x02
#define         HEART_BIT       0x04
#define         IPI0_IRQ_BIT    0x08
#define         IPI0_FIQ_BIT    0x10
#define         IPI1_IRQ_BIT    0x20
#define         IPI1_FIQ_BIT    0x40

#define IRQ     0
#define FIQ     1
#define DONE    0xff

#define         CPU0            0x0
#define         CPU1            0x1

#define         MAXCHAR         128*1024
typedef struct  {
       int flag;
       int uart_flag;
       int cnt;
       spinlock_dt lk;
       char message[MAXCHAR];
}s_mailbox;

// JScale proj definition
typedef struct {
	u16		type;				// message Type
	u16		length;				// message length, including message header
} IPC_MSG_HDR_T;

typedef struct{
	IPC_MSG_HDR_T	hdr;
	u32				input_location;
	u32				input_size;
	u32				output_location;
	u16  			ScaledImageWidth;
	u16  			ScaledImageHeight;
	u8  			ScaledImageQuality;
	u8  			MaintainResultionRatio;
	u8  			TwoStepScaling;
	u8  			InputFormat;
	u8				verbose;
	u8				reserved[3];
} JSCALE_REQ_T;

typedef struct{
	IPC_MSG_HDR_T	hdr;
	u32				status;
	u32				code;
	u32				output_size;
} JSCALE_RSP_T;

#define IPC_JSCALE_REQ_MSG			0	// JScale request from CPU-0 to CPU-1
#define IPC_JSCALE_RSP_MSG			1	// JScale response from CPU-1 to CPU-0

enum {
	JSCALE_STATUS_OK = 0,
	JSCALE_UNKNOWN_MSG_TYPE,
	JSCALE_FAILED_FILE_SIZE,
	JSCALE_FAILED_MALLOC,
	JSCALE_FAILED_FORMAT,
	JSCALE_DECODE_ERROR,
	JSCALE_BUSY,
};
// <-- JScale

#define GEMINI_IPI_IOCTL_BASE	'Z'
#define GEMINI_IPI_JSCALE_REQ		_IOW (GEMINI_IPI_IOCTL_BASE,0,JSCALE_REQ_T)
#define GEMINI_IPI_JSCALE_STAT		_IOR (GEMINI_IPI_IOCTL_BASE,1,JSCALE_RSP_T)


/*
* Simple spin lock operations.
*
*/

#define spin_is_locked_dt(x)((x)->lock != 0)

static inline int test_and_set_dt(spinlock_dt *lock)
{
unsigned long tmp;
__asm__ __volatile__(
"swp     %0, %2, [%1]\n"
: "=&r" (tmp)
: "r" (&lock->lock), "r" (1)
: "cc", "memory");

return tmp;
}

static inline void spin_lock_dt(spinlock_dt *lock)
{

unsigned long tmp;
__asm__ __volatile__(
"1:     ldr   %0, [%1]\n"
"teq     %0, #0\n"
"swpeq   %0, %2, [%1]\n"
"       teqeq   %0, #0\n"
"       bne     1b"
       : "=&r" (tmp)
       : "r" (&lock->lock), "r" (1)
       : "cc", "memory");
}

static inline void spin_unlock_dt(spinlock_dt *lock)
{
       __asm__ __volatile__(
"       str     %1, [%0]"
       :
       : "r" (&lock->lock), "r" (0)
       : "cc", "memory");
}

static inline int getcpuid(void)
{
       int cpuid;

      __asm__(
"mrc p8, 0, r0, c0, c0, 0\n"
"mov %0, r0"
       :"=r"(cpuid)
       :
       :"r0");
       return (cpuid & 0x07);
}



#endif
