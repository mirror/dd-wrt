#ifndef __BSP_COMMON_BSP_IF_H__
#define __BSP_COMMON_BSP_IF_H__
//==========================================================================
//
//      bsp_if.h
//
//      BSP interface definitions.
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
// Purpose:      BSP interface definitions.
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <bsp/bsp.h>

/*
 *  Maximum number of interrupt controllers supported by
 *  this bsp.
 */
#define BSP_MAX_IRQ_CONTROLLERS 8

#ifndef __ASSEMBLER__


/*
 *  Interrupt controller abstraction.
 *  Each interrupt controller on a given board should be described using
 *  this data structure.
 */
struct bsp_irq_controller {
    /*
     * First and last irqs handled by this controller.
     */
    short	first;
    short	last;

    /* 
     * pointer to array of bsp_vec struct pointers. These are
     * the heads of the linked list of ISRs for each irq handled
     * by this controller.
     */
    bsp_vec_t	**vec_list;

    /*
     * Pointer to initialization routine which is run once at boot time.
     */
    void	(*init)(const struct bsp_irq_controller *__ic);

    /*
     * Pointer to routines used to disable and enable interrupts handled
     * by this controller.
     */
    int		(*disable)(const struct bsp_irq_controller *__ic,
			   int __irq_nr);
    void	(*enable)(const struct bsp_irq_controller *__ic,
			  int __irq_nr);
};


/*
 * Board specific code needs to provide at least one communication channel
 * for use as the debug and console (stdio) channel. For each channel,
 * there must be a set of function vectors for the common BSP code to
 * control the channel.
 */
struct bsp_comm_procs {
    /*
     * Implementation dependent data pointer passed to the following procs.
     */
    void *ch_data;

    /*
     * Write a buffer of the given length. All of buffer is sent before
     * the write call returns.
     */
    void (*__write)(void *ch_data, const char *buf, int len);

    /*
     * Fill a buffer with up to the given length. Returns the actual number
     * of characters read.
     */
    int  (*__read)(void *ch_data, char *buf, int len);

    /*
     * Send a single character.
     */
    void (*__putc)(void *ch_data, char ch);

    /*
     * Read a single character. If no character is immediately available, will
     * block until one becomes available.
     */
    int  (*__getc)(void *ch_data);

    /*
     * Catchall comm port control.
     */
    int  (*__control)(void *ch_data, int func, ...);

    /*
     * For serial ports, the control function may be used to set and get the
     * current baud rate. Usage:
     * 
     *   err = (*__control)(COMMCTL_SETBAUD, int bits_per_second);
     *     err => Zero if successful, -1 if error.
     *
     *   baud = (*__control)(COMMCTL_GETBAUD);
     *     baud => -1 if error, current baud otherwise.
     */
#define COMMCTL_SETBAUD 0
#define COMMCTL_GETBAUD 1

    /*
     * Install and remove debugger interrupt handlers. These are the receiver
     * interrupt routines which are used to change control from a running
     * program to the debugger stub.
     */
#define COMMCTL_INSTALL_DBG_ISR 2
#define COMMCTL_REMOVE_DBG_ISR  3

    /*
     * Disable comm port interrupt. Returns TRUE if interrupt was enabled,
     * FALSE otherwise.
     */
#define COMMCTL_IRQ_DISABLE 4
    /*
     * Enable comm port interrupt.
     */
#define COMMCTL_IRQ_ENABLE 5
};


/*
 * The board specific code uses this data structure to provide information
 * about and procedure vectors for each supported communication channel.
 * See _bsp_comm_list below.
 */
struct bsp_comm_channel {
    struct bsp_comm_info   info;
    struct bsp_comm_procs  procs;
};


/*
 * Number to place in the version field. If structure is changed
 * in a way which is not backwards compatible, this number should
 * be incremented.
 */
#define BSP_SHARED_DATA_VERSION 2

/*
 * Clients of this BSP will need to have access to BSP functions and
 * data structures. Because, the client and the BSP may not be linked
 * together, a structure of vectors is used to gain this access. A
 * pointer to this structure can be gotten via a syscall. This syscall
 * is made automatically from within the crt0.o file.
 */
typedef struct {
    int		version;	/* version number for future expansion */

    /*
     *  Pointer to the array of pointers to interrupt controller descriptors.
     */
    const struct bsp_irq_controller **__ictrl_table;

    /*
     *  Pointer to the array of exception vectors.
     */
    bsp_vec_t **__exc_table;

    /*
     * Pointer to debug handler vector.
     */
    bsp_handler_t *__dbg_vector;

    /*
     * User hook to catch debugger 'kill' command.
     */
    bsp_handler_t __kill_vector;

    /*
     * Vectored functions for console and debug i/o.
     */
    struct bsp_comm_procs *__console_procs;
    struct bsp_comm_procs *__debug_procs;

    /*
     * Vectored cache control functions.
     */
    void (*__flush_dcache)(void *__p, int __nbytes);
    void (*__flush_icache)(void *__p, int __nbytes);

    /*
     * Generic data pointers
     */
    void *__cpu_data;
    void *__board_data;

    /*
     * General BSP information access.
     * See bsp.h for details.
     */
    int  (*__sysinfo)(enum bsp_info_id __id, va_list __ap);

    /*
     * Set or get active debug and console channels.
     * Returns -1 if unsucessful.
     * If the passed in __comm_id is -1, then the id of the current channel
     * is returned.
     */
    int	 (*__set_debug_comm)(int __comm_id);
    int	 (*__set_console_comm)(int __comm_id);

    /*
     * Set or get the current baud rate of a serial comm channel.
     * Returns -1 on if unsuccessful.
     * If the given baud is -1, then the current baudrate is returned.
     */
    int  (*__set_serial_baud)(int __comm_id, int baud);

    /*
     * Debug agent data.
     */
    void *__dbg_data;

    /*
     * Reset function
     * We want to avoid calling this with a trap since
     * we may be calling it from SWI mode (in cygmon).
     * That is problematic, as nested SWI's are not
     * very good.
     */
    void (*__reset)(void);

    /*
     * TRUE if console interrupt detected during program output.
     */
    int  __console_interrupt_flag;

} bsp_shared_t;


extern bsp_shared_t *bsp_shared_data;

/*
 * Platform info which may be overriden/modified by arch/board specific code.
 */
extern struct bsp_platform_info _bsp_platform_info;

/*
 * Cache info which may be overriden/modified by arch/board specific code.
 */
extern struct bsp_cachesize_info _bsp_dcache_info;
extern struct bsp_cachesize_info _bsp_icache_info;
extern struct bsp_cachesize_info _bsp_scache_info;

/*
 * Array of comm channel descriptors which must be provided by board specific
 * code.
 */
extern struct bsp_comm_channel _bsp_comm_list[];

/*
 * Number of comm channel descriptors which must be provided by board specific
 * code.
 */
extern int _bsp_num_comms;


/*
 * Array of memory region descriptors which must be provided by board specific
 * code.
 */
extern struct bsp_mem_info _bsp_memory_list[];

/*
 * Number of memory region descriptors which must be provided by board specific
 * code.
 */
extern int _bsp_num_mem_regions;

/*
 * In order to construct the above _bsp_memory_list, some board specific
 * code may have to size RAM regions. To do this easily and reliably,
 * the code needs to run from ROM before .bss and .data sections are
 * initialized. This leads to the problem of where to store the results
 * of the memory sizing tests. In this case, the _bsp_init_stack routine
 * which sizes memory and sets up the stack will place the board-specific
 * information on the stack and return with the stack pointer pointing to
 * a pointer to the information. That is, addr_of_info = *(void **)sp.
 * The architecture specific code will then copy that pointer to the
 * _bsp_ram_info_ptr variable after initializing the .data and .bss sections.
 */
extern void *_bsp_ram_info_ptr;

/*
 * Generic bsp initialization. Called by low level startup code
 */
extern void _bsp_init(void);

/*
 *  Initialize board communication in polling mode. This enables
 *  debugging printf for later initializations. Interrupts for
 *  comm channels may be set up later in _bsp_board_init().
 */
extern void _bsp_init_board_comm(void);

/*
 * Make generic BSP aware of CPU/MCU specific interrupt controllers.
 */
extern void _bsp_install_cpu_irq_controllers(void);

/*
 * Make generic BSP aware of board specific interrupt controllers.
 */
extern void _bsp_install_board_irq_controllers(void);

/*
 * Callback used by above two routines to install a single
 * interrupt controller.
 */
extern void _bsp_install_irq_controller(const struct bsp_irq_controller *__ic);

/*
 *  Generic exception dispatch routine. Usually called from asm-level
 *  exception handler to call vectors in vector chain for the given
 *  exception number. Stops traversing vector chain when a called
 *  vector returns a non-zero value. If no vector returns non-zero,
 *  a default error message and register dump is printed.
 */
extern int _bsp_exc_dispatch(int __exc_number, void *__regs);


/*
 * Architecture specific routine to dump register values.
 */
extern void _bsp_dump_regs(void *__regs);


/*
 * Generic syscall handler called by architecture specific handler.
 * Returns non-zero if given 'func' number was handled by the generic
 * code, zero otherwise. If handled, the syscall error is returned
 * via the err_ptr.
 */
extern int  _bsp_do_syscall(int __func,
			    long __arg1, long __arg2, long __arg3, long __arg4,
			    int *__err_ptr);


extern void _bsp_cpu_init(void);
extern void _bsp_board_init(void);


/*
 * General interface for getting certain BSP parameters.
 * See bsp.h for details.
 */
extern int  _bsp_sysinfo(enum bsp_info_id __id, va_list __ap);

/*
 * Called from comm channel when a connection to host is closed.
 */
extern void _bsp_dbg_connect_abort(void);
  

/*
 * Pointer to a network channel. NULL if no network channel
 * exists.
 */
extern struct bsp_comm_channel *_bsp_net_channel;


/*
 * Formatted output primitive.
 */
extern void __vprintf(void (*putc_func)(char c), const char *fmt0, va_list ap);



#endif /* !__ASSEMBLER__ */

/*
 * SYSCALL number to use to get pointer to above bsp_shared_t structure.
 */
#define BSP_GET_SHARED  0xbaad

#endif // __BSP_COMMON_BSP_IF_H__


