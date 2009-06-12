#ifndef __BSP_BSP_H__
#define __BSP_BSP_H__
//==========================================================================
//
//      bsp.h
//
//      Public interface to Red Hat BSP.
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Public interface to Red Hat BSP.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#ifndef __ASSEMBLER__

/* needed for _bsp_vsprintf() */
#include <stdarg.h>

/*
 * Exception and interrupt handler type.
 */
#ifndef _BSP_HANDLER_T_DEFINED
#define _BSP_HANDLER_T_DEFINED
typedef int (*bsp_handler_t)(int __irq_nr, void *__regs);
#endif // _BSP_HANDLER_T_DEFINED

/*
 *  Vector descriptor. This is needed for chaining vectors. The interfaces use
 *  bsp_vec structure pointers instead of direct pointers to handlers. This
 *  puts the responsibility for allocating the bsp_vec structures on the
 *  caller, rather than the BSP code.
 */
typedef struct bsp_vec {
    bsp_handler_t  handler;	/* pointer to actual ISR */
    struct bsp_vec *next;	/* for chaining */
} bsp_vec_t;

/*
 *  Valid op values for vector install routines.
 */
#define BSP_VEC_REPLACE     0
#define BSP_VEC_CHAIN_FIRST 1
#define BSP_VEC_CHAIN_LAST  2

/*
 *  Valid kinds of vectors supported by vector install and remove
 *  routines.
 */
#define BSP_VEC_EXCEPTION 0
#define BSP_VEC_INTERRUPT 1

/*
 * Routine to cause a breakpoint exception.
 */
extern void bsp_breakpoint(void);

/*
 * Dummy function whose address is the address of
 * the breakpoint caused by calling bsp_breakpoint().
 */
extern void bsp_breakinsn(void);

/*
 * Enable given irq.
 */
extern void bsp_enable_irq(int __irq_nr);

/*
 * Disable given irq. Returns true if irq was enabled.
 */
extern int  bsp_disable_irq(int __irq_nr);

/*
 * Remove given vector from vector chain.
 */
extern void bsp_remove_vec(int __vec_kind,
			   int __vec_nr,
			   bsp_vec_t *__vec);

/*
 * Install a vector chain.
 *
 * vec_kind may be BSP_VEC_EXCEPTION or BSP_VEC_INTERRUPT.
 * vec_nr is the exception or interrupt number.
 * op may be one of:
 *     BSP_VEC_REPLACE - replace existing chain.
 *     BSP_VEC_CHAIN_FIRST - install at head of chain.
 *     BSP_VEC_CHAIN_LAST  - install at tail of chain.
 *
 */
extern bsp_vec_t *bsp_install_vec(int __vec_kind,
				  int __vec_nr,
				  int __op,
				  bsp_vec_t *__vec);

/*
 * Install a debug handler.
 * Returns old handler being replaced.
 */
extern bsp_handler_t bsp_install_dbg_handler(bsp_handler_t __new_handler);

/*
 * Sometimes it is desireable to call the debug handler directly. This routine
 * accomplishes that. It is the responsibility of the caller to insure that
 * interrupts are disabled before calling this routine.
 */
extern void bsp_invoke_dbg_handler(int __exc_nr, void *__regs);

/*
 * Install a 'kill' handler. This handler is called when debugger
 * issues a kill command.
 * Returns old handler being replaced.
 */
extern bsp_handler_t bsp_install_kill_handler(bsp_handler_t __new_handler);

/*
 * Architecure specific routine to prepare CPU to execute
 * a single machine instruction.
 */
#ifndef USE_ECOS_HAL_SINGLESTEP
extern void bsp_singlestep_setup(void *__saved_regs);
#endif /* USE_ECOS_HAL_SINGLESTEP */

/*
 * Architecure specific routine to cleanup after a single-step
 * completes.
 */
#ifndef USE_ECOS_HAL_SINGLESTEP
extern void bsp_singlestep_cleanup(void *__saved_regs);
#endif /* USE_ECOS_HAL_SINGLESTEP */

/*
 * Architecture specific routine to skip past the current machine instruction.
 */
#ifndef USE_ECOS_HAL_SINGLESTEP
extern void bsp_skip_instruction(void *__saved_regs);
#endif /* USE_ECOS_HAL_SINGLESTEP */

/*
 * Return byte offset within the saved register area of the
 * given register.
 */
extern int bsp_regbyte(int __regno);

/*
 * Return size in bytes of given register.
 */
extern int bsp_regsize(int __regno);

/*
 * Setup the saved registered to establish the given Program Counter.
 */
#ifndef bsp_set_pc
extern void bsp_set_pc(unsigned long __pc, void *__saved_regs);
#endif

/*
 * Get the current Program Counter from the saved registers.
 */
#ifndef bsp_get_pc
unsigned long bsp_get_pc(void *__saved_regs);
#endif

extern int bsp_memory_read(void *__addr,    /* start addr of memory to read */
			   int  __asid,     /* address space id */
			   int  __rsize,    /* size of individual read ops */
			   int  __nreads,   /* number of read operations */
			   void *__buf);    /* result buffer */

extern int bsp_memory_write(void *__addr,   /* start addr of memory to write */
			    int  __asid,    /* address space id */
			    int  __wsize,   /* size of individual write ops */
			    int  __nwrites, /* number of write operations */
			    void *__buf);   /* source buffer for write data */

/*
 * Architecture specific routines to read and write CPU registers.
 */
extern void bsp_set_register(int __regno, void *__saved_regs, void *__val);
extern void bsp_get_register(int __regno, void *__saved_regs, void *__val);


/*
 * Architecture specific conversion of raw exception info into
 * a signal value.
 */
#ifndef bsp_get_signal
extern int  bsp_get_signal(int __exc_nr, void *__saved_regs);
#endif

/* light-weight bsp printf to console port */
extern void bsp_printf(const char *__fmt, ...);

/* light-weight bsp printf to debug port */
extern void bsp_dprintf(const char *__fmt, ...);

/* bsp vsprintf */
extern int bsp_vsprintf(char *__str, const char *__fmt, va_list __ap);

/* bsp vprintf to console port */
extern void bsp_vprintf(const char *__fmt, va_list __ap);

/* bsp vprintf to debug port */
extern void bsp_dvprintf(const char *__fmt, va_list __ap);

/* bsp sprintf */
extern void bsp_sprintf(char *str, const char *fmt, ...);

#ifdef NDEBUG
#define BSP_ASSERT(e)  ((void)0)
#else /* NDEBUG */
extern void _bsp_assert(const char *, const int, const char *);
#define BSP_ASSERT(e)  ((e) ? (void)0 : _bsp_assert(__FILE__, __LINE__, #e))
#endif /* NDEBUG */

/*
 * Functions for low-level console and debug i/o.
 */
extern void bsp_console_write(const char *__p, int __len);
extern void bsp_console_putc(char __ch);
extern int  bsp_console_read(char *__p, int __len);
extern int  bsp_console_getc(void);
extern void bsp_console_ungetc(char ch);
extern void bsp_debug_write(const char *__p, int __len);
extern int  bsp_debug_read(char *__p, int __len);
extern void bsp_debug_putc(char __ch);
extern int  bsp_debug_getc(void);
extern void bsp_debug_ungetc(char ch);

/*
 * Disable interrupts for debug comm channel.
 * Returns true if interrupts were previously enabled,
 * false if interrupts were already disabled.
 */
extern int  bsp_debug_irq_disable(void);
extern void bsp_debug_irq_enable(void);

/*
 * Cache control functions. May be noops for architectures not
 * supporting caches.
 *
 * The icache flush simply invalidates _at_least_ the range of
 * addresses specified.
 *
 * The dcache flush writes back (if write-back cache) and invalidates
 * _at_least_ the range of addresses specified.
 *
 */
extern void bsp_flush_dcache(void *__p, int __nbytes);
extern void bsp_flush_icache(void *__p, int __nbytes);

/*
 * Reset function. May be noops for architectures not
 * supporting software reset.
 */
extern void bsp_reset(void);

/*
 * Generic data (board and CPU specific) handling
 */
extern void *bsp_cpu_data(void);
extern void *bsp_board_data(void);

/*
 *  List of board characteristics which can be read queried by BSP clients. These
 *  information IDs are passed to:
 *
 *      int bsp_sysinfo(enum bsp_info_id id, ...);
 *
 *  Some pieces of information may have more than one instance. For example, the
 *  BSP will likely have information on multiple memory regions. In those cases,
 *  a particular instance may be accessed using a small integer index argument.
 *
 *  The following comments indicate what additional arguments are needed
 *  to access the specific information.
 */
enum bsp_info_id {
    /*
     * CPU and board names.
     *
     * err = bsp_sysinfo(BSP_INFO_PLATFORM,
     *                   struct bsp_platform_info *result)
     *
     *     err => zero if successful, -1 if unsupported.
     */
    BSP_INFO_PLATFORM,

    /*
     * Data, instruction, and secondary caches.
     *
     * err = bsp_sysinfo(BSP_INFO_[DIS]CACHE,
     *                   struct bsp_cache_info *result)
     *
     *     err => zero if successful, -1 if unsupported.
     *
     */
    BSP_INFO_DCACHE,
    BSP_INFO_ICACHE,
    BSP_INFO_SCACHE,

    /*
     * Memory region info.
     *
     * err = bsp_sysinfo(BSP_INFO_MEMORY,
     *                   int index,
     *                   struct bsp_mem_info *result)
     *
     *     err => zero if successful, -1 if invalid index.
     *
     * Caller should start index at zero, then increment index for subsequent
     * calls until error return indicates no more memory regions.
     */
    BSP_INFO_MEMORY,

    /*
     * Communication channel info.
     *
     * err = bsp_sysinfo(BSP_INFO_COMM,
     *                   int index,
     *                   struct bsp_comm_info *result)
     *
     *     err => zero if successful, -1 if invalid index.
     *
     * Caller should start index at zero, then increment index for subsequent
     * calls until error return indicates no more comm channels.
     */
    BSP_INFO_COMM
};


/*
 * Platform info.
 */
struct bsp_platform_info {
    const char *cpu;	/* CPU name*/
    const char *board;  /* board name */ 
    const char *extra;  /* extra info */
};


/*
 * Cache size info.
 */
struct bsp_cachesize_info {
    int	  size;		/* total size in bytes */
    short linesize;	/* width of cacheline in bytes */
    short ways;		/* number of ways per line */
};


/*
 * Memory region info.
 * The BSP may describe multiple memory regions. For example,
 * DRAM may be comprised of several non-contiguous regions.
 * ROM and FLASH regions may also be described.
 *
 */
struct bsp_mem_info {
    void *phys_start;	/* physical start address */
    void *virt_start;   /* virtual start address */
    int  virt_asid;	/* some architectures also use an address space id */
    long nbytes;	/* length of region in bytes */
    int  kind;          /* kind of memory */
#define BSP_MEM_RAM    1
#define BSP_MEM_FLASH  2
#define BSP_MEM_ROM    3
};


struct bsp_comm_info {
    char    *name;
    short   kind;
#define BSP_COMM_SERIAL 1
#define BSP_COMM_ENET   2
    short   protocol;
#define BSP_PROTO_NONE  0
#define BSP_PROTO_UDP   1
#define BSP_PROTO_TCP   2
};


extern int bsp_sysinfo(enum bsp_info_id __id, ...);

/*
 * Set or get active debug and console channels.
 * Returns -1 if unsucessful.
 * If the passed in __comm_id is -1, then the id of the current channel
 * is returned.
 */
extern int bsp_set_debug_comm(int __comm_id);
extern int bsp_set_console_comm(int __comm_id);

/*
 * Set or get the current baud rate of a serial comm channel.
 * Returns -1 on if unsuccessful.
 * If the given baud is -1, then the current baudrate is returned.
 */
extern int bsp_set_serial_baud(int __comm_id, int baud);

#endif

#endif // __BSP_BSP_H__
