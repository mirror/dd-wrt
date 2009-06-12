#ifndef CYGONCE_HAL_HAL_IF_H
#define CYGONCE_HAL_HAL_IF_H

//=============================================================================
//
//      hal_if.h
//
//      HAL header for ROM/RAM calling interface.
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2002, 2003, 2004 Gary Thomas
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov, woehler
// Date:        2000-06-07
// Purpose:     HAL RAM/ROM calling interface
// Description: ROM/RAM calling interface table	definitions. The layout is a
//              combination of libbsp and vectors already in use by some
//              eCos platforms.
// Usage:       #include <cyg/hal/hal_if.h>
//                           
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>         // types & externC
#include <cyg/hal/dbg-threads-api.h>
#include <cyg/hal/dbg-thread-syscall.h>

#include <stdarg.h>

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

// Architecture/var/platform may override the accessor macros.
#include <cyg/hal/hal_arch.h>

// Special monitor locking procedures.  These are necessary when the monitor
// and eCos share facilities, e.g. the network hardware.
#if defined (CYGPKG_NET) || defined (CYGPKG_NET_LWIP)
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>            // cyg_drv_dsr_lock(), etc
#define _ENTER_MONITOR()                        \
    cyg_uint32 ints;                            \
    HAL_DISABLE_INTERRUPTS(ints);               \
    cyg_drv_dsr_lock()

#define _EXIT_MONITOR()                         \
    cyg_drv_dsr_unlock();                       \
    HAL_RESTORE_INTERRUPTS(ints)
#else // !CYGPKG_NET && !CYGPKG_NET_LWIP
#define _ENTER_MONITOR() CYG_EMPTY_STATEMENT
#define _EXIT_MONITOR()  CYG_EMPTY_STATEMENT
#endif

//--------------------------------------------------------------------------
#ifndef _BSP_HANDLER_T_DEFINED
#define _BSP_HANDLER_T_DEFINED
typedef int (*bsp_handler_t)(int __irq_nr, void *__regs);
#endif // _BSP_HANDLER_T_DEFINED

//--------------------------------------------------------------------------
// Communication interface table. CYGNUM_CALL_IF_CONSOLE_PROCS and
// CYGNUM_CALL_IF_DEBUG_PROCS point to instances (possibly the same)
// of this table.

typedef enum {
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
    __COMMCTL_SETBAUD=0,
    __COMMCTL_GETBAUD,

    /*
     * Install and remove debugger interrupt handlers. These are the receiver
     * interrupt routines which are used to change control from a running
     * program to the debugger stub.
     */
    __COMMCTL_INSTALL_DBG_ISR,
    __COMMCTL_REMOVE_DBG_ISR,

    /*
     * Disable comm port interrupt. Returns TRUE if interrupt was enabled,
     * FALSE otherwise.
     */
    __COMMCTL_IRQ_DISABLE,

    /*
     * Enable comm port interrupt.
     */
    __COMMCTL_IRQ_ENABLE,

    /*
     * Returns the number of the interrupt vector used by the debug
     * interrupt handler.
     */
    __COMMCTL_DBG_ISR_VECTOR,

    /*
     * Returns the current timeout value and sets a new timeout.
     * Timeout resolution is in milliseconds.
     *   old_timeout = (*__control)(__COMMCTL_SET_TIMEOUT, 
     *                              cyg_int32 new_timeout);
     */
    __COMMCTL_SET_TIMEOUT,

    /*
     * Forces driver to send all characters which may be buffered in
     * the driver. This only flushes the driver buffers, not necessarily
     * any hardware FIFO, etc.
     */
    __COMMCTL_FLUSH_OUTPUT,

    /*
     * Forces driver to enable or disable flushes when a newline is
     * seen in the output stream. Flushing at line boundaries occurs
     * in the driver, not necessarily any hardware FIFO, etc. Line
     * buffering is optional and may only be available in some drivers.
     */
    __COMMCTL_ENABLE_LINE_FLUSH,
    __COMMCTL_DISABLE_LINE_FLUSH,

} __comm_control_cmd_t;


#define CYGNUM_COMM_IF_CH_DATA                    0
#define CYGNUM_COMM_IF_WRITE                      1
#define CYGNUM_COMM_IF_READ                       2
#define CYGNUM_COMM_IF_PUTC                       3
#define CYGNUM_COMM_IF_GETC                       4
#define CYGNUM_COMM_IF_CONTROL                    5
#define CYGNUM_COMM_IF_DBG_ISR                    6
#define CYGNUM_COMM_IF_GETC_TIMEOUT               7

#define CYGNUM_COMM_IF_TABLE_SIZE                 8

typedef volatile CYG_ADDRWORD hal_virtual_comm_table_t[CYGNUM_COMM_IF_TABLE_SIZE];

// The below is a (messy) attempt at adding some type safety to the
// above array. At the same time, the accessors allow the
// implementation to be easily changed in the future (both tag->data
// table and structure implementations have been suggested).

typedef void* __comm_if_ch_data_t;
typedef void (*__comm_if_write_t)(void* __ch_data, const cyg_uint8* __buf,
                                  cyg_uint32 __len);
typedef int (*__comm_if_read_t)(void* __ch_data, cyg_uint8* __buf,
                                cyg_uint32 __len);
typedef void (*__comm_if_putc_t)(void* __ch_data, cyg_uint8 __ch);
typedef cyg_uint8 (*__comm_if_getc_t)(void* __ch_data);
typedef int (*__comm_if_control_t)(void *__ch_data, 
                                   __comm_control_cmd_t __func, ...);
typedef int (*__comm_if_dbg_isr_t)(void *__ch_data, 
                               int* __ctrlc, CYG_ADDRWORD __vector,
                               CYG_ADDRWORD __data);
typedef cyg_bool (*__comm_if_getc_timeout_t)(void* __ch_data, cyg_uint8* __ch);

#define __call_COMM0(_n_,_rt_,_t_)                              \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t)                   \
{                                                               \
    _rt_ res;                                                   \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    res = ((_t_)(t[CYGNUM_COMM_##_n_]))(dp);                    \
    _EXIT_MONITOR();                                            \
    return res;                                                 \
}

#define __call_voidCOMM(_n_,_rt_,_t_)                           \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t)                   \
{                                                               \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    ((_t_)(t[CYGNUM_COMM_##_n_]))(dp);                          \
    _EXIT_MONITOR();                                            \
}

#define __call_COMM1(_n_,_rt_,_t_,_t1_)                         \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t, _t1_ _p1_)        \
{                                                               \
    _rt_ res;                                                   \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    res = ((_t_)(t[CYGNUM_COMM_##_n_]))(dp, _p1_);              \
    _EXIT_MONITOR();                                            \
    return res;                                                 \
}

#define __call_voidCOMM1(_n_,_rt_,_t_,_t1_)                     \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t, _t1_ _p1_)        \
{                                                               \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    ((_t_)(t[CYGNUM_COMM_##_n_]))(dp, _p1_);                    \
    _EXIT_MONITOR();                                            \
}

#define __call_COMM2(_n_,_rt_,_t_,_t1_,_t2_)                    \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t, _t1_ _p1_, _t2_ _p2_)        \
{                                                               \
    _rt_ res;                                                   \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    res = ((_t_)(t[CYGNUM_COMM_##_n_]))(dp, _p1_, _p2_);        \
    _EXIT_MONITOR();                                            \
    return res;                                                 \
}

#define __call_voidCOMM2(_n_,_rt_,_t_,_t1_,_t2_)                \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t, _t1_ _p1_, _t2_ _p2_)        \
{                                                               \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    ((_t_)(t[CYGNUM_COMM_##_n_]))(dp, _p1_, _p2_);              \
    _EXIT_MONITOR();                                            \
}

#define __call_COMM3(_n_,_rt_,_t_,_t1_,_t2_,_t3_)               \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t, _t1_ _p1_, _t2_ _p2_, _t3_ _p3_)        \
{                                                               \
    _rt_ res;                                                   \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    res = ((_t_)(t[CYGNUM_COMM_##_n_]))(dp, _p1_, _p2_, _p3_);              \
    _EXIT_MONITOR();                                            \
    return res;                                                 \
}

#define __call_voidCOMM3(_n_,_rt_,_t_,_t1_,_t2_,_t3_)           \
static __inline__ _rt_                                          \
__call_COMM_##_n_(hal_virtual_comm_table_t t, _t1_ _p1_, _t2_ _p2_, _t3_ _p3_)        \
{                                                               \
    void *dp = (__comm_if_ch_data_t)t[CYGNUM_COMM_IF_CH_DATA];  \
    _ENTER_MONITOR();                                           \
    ((_t_)(t[CYGNUM_COMM_##_n_]))(dp, _p1_, _p2_, _p3_);        \
    _EXIT_MONITOR();                                            \
}

#ifndef CYGACC_COMM_IF_DEFINED

#define CYGACC_COMM_IF_CH_DATA(_t_) \
 ((__comm_if_ch_data_t)((_t_)[CYGNUM_COMM_IF_CH_DATA]))
#define CYGACC_COMM_IF_CH_DATA_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_CH_DATA]=(CYG_ADDRWORD)(_x_)

__call_voidCOMM2(IF_WRITE, void, __comm_if_write_t, const cyg_uint8 *, cyg_uint32)
#define CYGACC_COMM_IF_WRITE(_t_, _b_, _l_) \
 __call_COMM_IF_WRITE(_t_, _b_, _l_)
#define CYGACC_COMM_IF_WRITE_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_WRITE]=(CYG_ADDRWORD)(_x_)

__call_voidCOMM2(IF_READ, void, __comm_if_read_t, cyg_uint8 *, cyg_uint32)
#define CYGACC_COMM_IF_READ(_t_, _b_, _l_) \
 __call_COMM_IF_READ(_t_, _b_, _l_)
#define CYGACC_COMM_IF_READ_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_READ]=(CYG_ADDRWORD)(_x_)

__call_voidCOMM1(IF_PUTC, void, __comm_if_putc_t, cyg_uint8)
#define CYGACC_COMM_IF_PUTC(_t_, _c_) \
 __call_COMM_IF_PUTC(_t_,_c_)
#define CYGACC_COMM_IF_PUTC_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_PUTC]=(CYG_ADDRWORD)(_x_)

__call_COMM0(IF_GETC, cyg_uint8, __comm_if_getc_t)
#define CYGACC_COMM_IF_GETC(_t_) \
 __call_COMM_IF_GETC(_t_)
#define CYGACC_COMM_IF_GETC_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_GETC]=(CYG_ADDRWORD)(_x_)

// This macro has not been changed to use inline functions like the
// others, simply because it uses variable arguments, and the change
// would break binary compatibility.
#define CYGACC_COMM_IF_CONTROL(_t_, args...)                                                            \
 ({ int res;                                                                                            \
    _ENTER_MONITOR();                                                                                   \
    res = ((__comm_if_control_t)((_t_)[CYGNUM_COMM_IF_CONTROL]))(CYGACC_COMM_IF_CH_DATA(_t_), args);    \
    _EXIT_MONITOR();                                                                                    \
    res;})
#define CYGACC_COMM_IF_CONTROL_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_CONTROL]=(CYG_ADDRWORD)(_x_)

__call_COMM3(IF_DBG_ISR, int, __comm_if_dbg_isr_t, int *, CYG_ADDRWORD, CYG_ADDRWORD)
#define CYGACC_COMM_IF_DBG_ISR(_t_, _c_, _v_, _d_) \
 __call_COMM_IF_DBG_ISR(_t_, _c_, _v_, _d_)
#define CYGACC_COMM_IF_DBG_ISR_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_DBG_ISR]=(CYG_ADDRWORD)(_x_)

__call_COMM1(IF_GETC_TIMEOUT, cyg_bool, __comm_if_getc_timeout_t, cyg_uint8 *)
#define CYGACC_COMM_IF_GETC_TIMEOUT(_t_, _c_) \
 __call_COMM_IF_GETC_TIMEOUT(_t_, _c_)
#define CYGACC_COMM_IF_GETC_TIMEOUT_SET(_t_, _x_) \
 (_t_)[CYGNUM_COMM_IF_GETC_TIMEOUT]=(CYG_ADDRWORD)(_x_)

#endif // CYGACC_COMM_IF_DEFINED

//--------------------------------------------------------------------------
// Main calling interface table. Will be assigned a location by the 
// linker script. Both ROM and RAM startup applications will know about
// the location.
#define CYGNUM_CALL_IF_VERSION                    0
#define CYGNUM_CALL_IF_available_1                1
#define CYGNUM_CALL_IF_available_2                2
#define CYGNUM_CALL_IF_available_3                3
#define CYGNUM_CALL_IF_KILL_VECTOR                4
#define CYGNUM_CALL_IF_CONSOLE_PROCS              5
#define CYGNUM_CALL_IF_DEBUG_PROCS                6
#define CYGNUM_CALL_IF_FLUSH_DCACHE               7
#define CYGNUM_CALL_IF_FLUSH_ICACHE               8
#define CYGNUM_CALL_IF_available_9                9
#define CYGNUM_CALL_IF_available_10               10
#define CYGNUM_CALL_IF_available_11               11
#define CYGNUM_CALL_IF_SET_DEBUG_COMM             12
#define CYGNUM_CALL_IF_SET_CONSOLE_COMM           13
#define CYGNUM_CALL_IF_MONITOR_VERSION            14
#define CYGNUM_CALL_IF_DBG_SYSCALL                15
#define CYGNUM_CALL_IF_RESET                      16
#define CYGNUM_CALL_IF_CONSOLE_INTERRUPT_FLAG     17
#define CYGNUM_CALL_IF_DELAY_US                   18
#define CYGNUM_CALL_IF_DBG_DATA                   19
#define CYGNUM_CALL_IF_FLASH_CFG_OP               20
#define CYGNUM_CALL_IF_MONITOR_RETURN             21
#define CYGNUM_CALL_IF_FLASH_FIS_OP               22

#define CYGNUM_CALL_IF_LAST_ENTRY                 CYGNUM_CALL_IF_FLASH_FIS_OP

#define CYGNUM_CALL_IF_INSTALL_BPT_FN             35

#define CYGNUM_CALL_IF_TABLE_SIZE                 64

externC volatile CYG_ADDRWORD hal_virtual_vector_table[CYGNUM_CALL_IF_TABLE_SIZE];

// Table version contains version information for both the CALL table
// itself (the number of the last active entry in the table), and the
// COMM table (the size of the table).
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL         CYGNUM_CALL_IF_LAST_ENTRY
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL_HACK    (CYGNUM_CALL_IF_TABLE_SIZE+1)
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL_MAX     CYGNUM_CALL_IF_TABLE_SIZE
#define CYGNUM_CALL_IF_TABLE_VERSION_COMM         CYGNUM_COMM_IF_TABLE_SIZE
#define CYGNUM_CALL_IF_TABLE_VERSION_CALL_MASK    0x0000ffff
#define CYGNUM_CALL_IF_TABLE_VERSION_COMM_MASK    0xffff0000
#define CYGNUM_CALL_IF_TABLE_VERSION_COMM_shift   16


// These are special debug/console procs IDs
// QUERY_CURRENT will cause the ID of the currently selected proc ID to be
//               returned.
// EMPTY         this is the ID used for an empty procs table (i.e, NULL
//               pointer)
// MANGLER       selects the procs space reserved for the console mangler
//               allowing the application to temporarily disable mangling
//               or temporarily switch in different console procs.
#define CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT -1
#define CYGNUM_CALL_IF_SET_COMM_ID_EMPTY         -2
#define CYGNUM_CALL_IF_SET_COMM_ID_MANGLER       -3

// The below is a (messy) attempt at adding some type safety to the
// above array. At the same time, the accessors allow the
// implementation to be easily changed in the future (both tag->data
// table and structure implementations have been suggested).

typedef int __call_if_version_t;
typedef void* __call_if_ictrl_table_t;
typedef void* __call_if_exc_table_t;
typedef bsp_handler_t *__call_if_dbg_vector_t;
typedef bsp_handler_t __call_if_kill_vector_t;
typedef hal_virtual_comm_table_t *__call_if_console_procs_t;
typedef hal_virtual_comm_table_t *__call_if_debug_procs_t;
typedef void (__call_if_flush_dcache_t)(void *__p, int __nbytes);
typedef void (__call_if_flush_icache_t)(void *__p, int __nbytes);
typedef int (__call_if_set_debug_comm_t)(int __comm_id);
typedef int (__call_if_set_console_comm_t)(int __comm_id);
typedef void* __call_if_dbg_data_t;
typedef int (__call_if_dbg_syscall_t) (enum dbg_syscall_ids id,
                                        union dbg_thread_syscall_parms  *p );
typedef void (__call_if_reset_t)(void);
typedef int __call_if_console_interrupt_flag_t;
typedef void (__call_if_delay_us_t)(cyg_int32 usecs);
typedef void (__call_if_install_bpt_fn_t)(void *__epc);
typedef char *__call_if_monitor_version_t;
typedef void (__call_if_monitor_return_t)(int status);
typedef cyg_bool (__call_if_flash_fis_op_fn_t)(int __oper, char *__name, void *__val);
//
// This structure is used to pass parameters to/from the fconfig routines.
// This allows a single virtual vector interface, with widely varying functionality
//
struct cyg_fconfig {
    char *key;      // Datum 'key'
    int   keylen;   // Length of key
    void *val;      // Pointer to data
    int   type;     // Type of datum
    int   offset;   // Offset within data (used by _NEXT)
};
typedef cyg_bool (__call_if_flash_cfg_op_fn_t)(int __oper, struct cyg_fconfig *__data);

#ifndef CYGACC_CALL_IF_DEFINED

#define __data_VV(_n_,_tt_)                             \
static __inline__ _tt_                                  \
__call_vv_##_n_(void)                                   \
{                                                       \
    return ((_tt_)hal_virtual_vector_table[_n_]);       \
}

#define __call_VV0(_n_,_tt_,_rt_)                                       \
static __inline__ _rt_                                                  \
__call_vv_##_n_(void)                                                   \
{                                                                       \
    _rt_ res;                                                           \
    _ENTER_MONITOR();                                                   \
    res = ((_tt_ *)hal_virtual_vector_table[_n_])();                    \
    _EXIT_MONITOR();                                                    \
    return res;                                                         \
}

#define __call_voidVV0(_n_,_tt_,_rt_)                                   \
static __inline__ _rt_                                                  \
__call_vv_##_n_(void)                                                   \
{                                                                       \
    _ENTER_MONITOR();                                                   \
    ((_tt_ *)hal_virtual_vector_table[_n_])();                          \
    _EXIT_MONITOR();                                                    \
}

#define __call_VV1(_n_,_tt_,_rt_,_t1_)                                  \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_)                                              \
{                                                                       \
    _rt_ res;                                                           \
    _ENTER_MONITOR();                                                   \
    res = ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_);                \
    _EXIT_MONITOR();                                                    \
    return res;                                                         \
}

#define __call_voidVV1(_n_,_tt_,_rt_,_t1_)                              \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_)                                              \
{                                                                       \
    _ENTER_MONITOR();                                                   \
    ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_);                      \
    _EXIT_MONITOR();                                                    \
}

#define __call_VV2(_n_,_tt_,_rt_,_t1_,_t2_)                             \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_, _t2_ _p2_)                                   \
{                                                                       \
    _rt_ res;                                                           \
    _ENTER_MONITOR();                                                   \
    res = ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_,_p2_);           \
    _EXIT_MONITOR();                                                    \
    return res;                                                         \
}

#define __call_voidVV2(_n_,_tt_,_rt_,_t1_,_t2_)                         \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_, _t2_ _p2_)                                   \
{                                                                       \
    _ENTER_MONITOR();                                                   \
    ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_,_p2_);                 \
    _EXIT_MONITOR();                                                    \
}

#define __call_VV3(_n_,_tt_,_rt_,_t1_,_t2_,_t3_)                        \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_, _t2_ _p2_, _t3_ _p3_)                        \
{                                                                       \
    _rt_ res;                                                           \
    _ENTER_MONITOR();                                                   \
    res = ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_,_p2_,_p3_);      \
    _EXIT_MONITOR();                                                    \
    return res;                                                         \
}

#define __call_voidVV3(_n_,_tt_,_rt_,_t1_,_t2_,_t3_)                    \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_, _t2_ _p2_, _t3_ _p3_)                        \
{                                                                       \
    _ENTER_MONITOR();                                                   \
    ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_,_p2_,_p3_);            \
    _EXIT_MONITOR();                                                    \
}

#define __call_VV4(_n_,_tt_,_rt_,_t1_,_t2_,_t3_,_t4_)                   \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_, _t2_ _p2_, _t3_ _p3_, _t4_ _p4_)             \
{                                                                       \
    _rt_ res;                                                           \
    _ENTER_MONITOR();                                                   \
    res = ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_,_p2_,_p3_,_p4_); \
    _EXIT_MONITOR();                                                    \
    return res;                                                         \
}

#define __call_voidVV4(_n_,_tt_,_rt_,_t1_,_t2_,_t3_,_t4_)               \
static __inline__ _rt_                                                  \
__call_vv_##_n_(_t1_ _p1_, _t2_ _p2_, _t3_ _p3_, _t4_ _p4_)             \
{                                                                       \
    _ENTER_MONITOR();                                                   \
    ((_tt_ *)hal_virtual_vector_table[_n_])(_p1_,_p2_,_p3_,_p4_);       \
    _EXIT_MONITOR();                                                    \
}


#define CYGACC_DATA_VV(t,e)              __call_vv_##e()
#define CYGACC_CALL_VV0(t,e)             __call_vv_##e
#define CYGACC_CALL_VV1(t,e,p1)          __call_vv_##e((p1))
#define CYGACC_CALL_VV2(t,e,p1,p2)       __call_vv_##e((p1),(p2))
#define CYGACC_CALL_VV3(t,e,p1,p2,p3)    __call_vv_##e((p1),(p2),(p3))
#define CYGACC_CALL_VV4(t,e,p1,p2,p3,p4) __call_vv_##e((p1),(p2),(p3),(p4))

#define CYGACC_CALL_IF_VERSION() \
 CYGACC_DATA_VV(__call_if_version_t, CYGNUM_CALL_IF_VERSION)
__data_VV(CYGNUM_CALL_IF_VERSION, __call_if_version_t)
#define CYGACC_CALL_IF_VERSION_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_VERSION]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_KILL_VECTOR() \
 CYGACC_DATA_VV(__call_if_kill_vector_t, CYGNUM_CALL_IF_KILL_VECTOR)
__data_VV(CYGNUM_CALL_IF_KILL_VECTOR, __call_if_kill_vector_t)
#define CYGACC_CALL_IF_KILL_VECTOR_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_KILL_VECTOR]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_CONSOLE_PROCS() \
 CYGACC_DATA_VV(__call_if_console_procs_t, CYGNUM_CALL_IF_CONSOLE_PROCS)
__data_VV(CYGNUM_CALL_IF_CONSOLE_PROCS, __call_if_console_procs_t)
#define CYGACC_CALL_IF_CONSOLE_PROCS_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_CONSOLE_PROCS]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_FLUSH_DCACHE(_p_, _n_) \
 ((__call_if_flush_dcache_t*)hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_DCACHE])((_p_), (_n_))
#define CYGACC_CALL_IF_FLUSH_DCACHE_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_DCACHE]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_FLUSH_ICACHE(_p_, _n_) \
 ((__call_if_flush_icache_t*)hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_ICACHE])((_p_), (_n_))
#define CYGACC_CALL_IF_FLUSH_ICACHE_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_FLUSH_ICACHE]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DEBUG_PROCS() \
 CYGACC_DATA_VV(__call_if_debug_procs_t, CYGNUM_CALL_IF_DEBUG_PROCS)
__data_VV(CYGNUM_CALL_IF_DEBUG_PROCS, __call_if_debug_procs_t)
#define CYGACC_CALL_IF_DEBUG_PROCS_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DEBUG_PROCS]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_SET_DEBUG_COMM(_i_) \
 CYGACC_CALL_VV1(__call_if_set_debug_comm_t*, CYGNUM_CALL_IF_SET_DEBUG_COMM, (_i_))
__call_VV1(CYGNUM_CALL_IF_SET_DEBUG_COMM, __call_if_set_debug_comm_t, int, int)
#define CYGACC_CALL_IF_SET_DEBUG_COMM_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_SET_DEBUG_COMM]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_SET_CONSOLE_COMM(_i_) \
 CYGACC_CALL_VV1(__call_if_set_console_comm_t*, CYGNUM_CALL_IF_SET_CONSOLE_COMM, (_i_))
__call_VV1(CYGNUM_CALL_IF_SET_CONSOLE_COMM, __call_if_set_console_comm_t, int, int)
#define CYGACC_CALL_IF_SET_CONSOLE_COMM_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_SET_CONSOLE_COMM]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DBG_DATA() \
 CYGACC_DATA_VV(__call_if_dbg_data_t, CYGNUM_CALL_IF_DBG_DATA)
__data_VV(CYGNUM_CALL_IF_DBG_DATA, __call_if_dbg_data_t)
#define CYGACC_CALL_IF_DBG_DATA_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_DATA]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DBG_SYSCALL(_id_,_p_) \
 CYGACC_CALL_VV2(__call_if_dbg_syscall_t, CYGNUM_CALL_IF_DBG_SYSCALL, _id_, _p_)
__call_VV2(CYGNUM_CALL_IF_DBG_SYSCALL, __call_if_dbg_syscall_t, int, enum dbg_syscall_ids ,  union dbg_thread_syscall_parms  *)
#define CYGACC_CALL_IF_DBG_SYSCALL_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_SYSCALL]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_RESET() \
 CYGACC_CALL_VV0(__call_if_reset_t*, CYGNUM_CALL_IF_RESET)()
__call_voidVV0(CYGNUM_CALL_IF_RESET, __call_if_reset_t, void)
#define CYGACC_CALL_IF_RESET_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_RESET]=(CYG_ADDRWORD)(_x_)
#define CYGACC_CALL_IF_RESET_GET() \
 ((__call_if_reset_t*)hal_virtual_vector_table[CYGNUM_CALL_IF_RESET])

#define CYGACC_CALL_IF_MONITOR_VERSION() \
 CYGACC_DATA_VV(__call_if_monitor_version_t, CYGNUM_CALL_IF_MONITOR_VERSION)
__data_VV(CYGNUM_CALL_IF_MONITOR_VERSION, __call_if_monitor_version_t)
#define CYGACC_CALL_IF_MONITOR_VERSION_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_MONITOR_VERSION]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG() \
 CYGACC_DATA_VV(__call_if_console_interrupt_flag_t, CYGNUM_CALL_IF_CONSOLE_INTERRUPT_FLAG)
__data_VV(CYGNUM_CALL_IF_CONSOLE_INTERRUPT_FLAG, __call_if_console_interrupt_flag_t)
#define CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_CONSOLE_INTERRUPT_FLAG]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_DELAY_US(_u_) \
 CYGACC_CALL_VV1(__call_if_delay_us_t*, CYGNUM_CALL_IF_DELAY_US, (_u_))
__call_voidVV1(CYGNUM_CALL_IF_DELAY_US, __call_if_delay_us_t, void, cyg_int32)
#define CYGACC_CALL_IF_DELAY_US_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_DELAY_US]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_INSTALL_BPT_FN(_e_) \
 CYGACC_CALL_VV1(__call_if_install_bpt_fn_t*, CYGNUM_CALL_IF_INSTALL_BPT_FN, (_e_))
__call_voidVV1(CYGNUM_CALL_IF_INSTALL_BPT_FN, __call_if_install_bpt_fn_t, void, void *)
#define CYGACC_CALL_IF_INSTALL_BPT_FN_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_INSTALL_BPT_FN]=(CYG_ADDRWORD)(_x_)

//
// Access persistent data store - kept in FLASH or EEPROM by RedBoot
//
#define CYGNUM_CALL_IF_FLASH_CFG_GET  (0)     // Get a particular fconfig key
#define CYGNUM_CALL_IF_FLASH_CFG_NEXT (1)     // Enumerate keys (get the next one)
#define CYGNUM_CALL_IF_FLASH_CFG_SET  (2)     // Update particular fconfig key
#define CYGACC_CALL_IF_FLASH_CFG_OP2(_o_,_d_) \
 CYGACC_CALL_VV2(__call_if_flash_cfg_op_fn_t*, CYGNUM_CALL_IF_FLASH_CFG_OP, (_o_),(_d_))
__call_VV2(CYGNUM_CALL_IF_FLASH_CFG_OP, __call_if_flash_cfg_op_fn_t, cyg_bool, int, struct cyg_fconfig *)

static __inline__ cyg_bool
__call_if_flash_cfg_op(int op, char *key, void *data, int type)
{
    struct cyg_fconfig info;
    info.key = key;
    info.val = data;
    info.type = type;
    info.offset = 0;
    return CYGACC_CALL_IF_FLASH_CFG_OP2(op, &info);
}
#define CYGACC_CALL_IF_FLASH_CFG_OP(_o_,_k_,_d_,_t_) \
  __call_if_flash_cfg_op(_o_,_k_,_d_,_t_)
#define CYGACC_CALL_IF_FLASH_CFG_OP_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_FLASH_CFG_OP]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_MONITOR_RETURN(_u_) \
 CYGACC_CALL_VV1(__call_if_monitor_return_t*, CYGNUM_CALL_IF_MONITOR_RETURN, (_u_))
__call_voidVV1(CYGNUM_CALL_IF_MONITOR_RETURN, __call_if_monitor_return_t, void, int)
#define CYGACC_CALL_IF_MONITOR_RETURN_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_MONITOR_RETURN]=(CYG_ADDRWORD)(_x_)

#define CYGACC_CALL_IF_FLASH_FIS_OP(_o_,_k_,_d_) \
 CYGACC_CALL_VV3(__call_if_flash_fis_op_fn_t*, CYGNUM_CALL_IF_FLASH_FIS_OP, (_o_),(_k_),(_d_))
__call_VV3(CYGNUM_CALL_IF_FLASH_FIS_OP, __call_if_flash_fis_op_fn_t, cyg_bool, int, char *, void *)
#define CYGACC_CALL_IF_FLASH_FIS_OP_SET(_x_) \
 hal_virtual_vector_table[CYGNUM_CALL_IF_FLASH_FIS_OP]=(CYG_ADDRWORD)(_x_)
#define CYGNUM_CALL_IF_FLASH_FIS_GET_FLASH_BASE  (0)
#define CYGNUM_CALL_IF_FLASH_FIS_GET_SIZE        (1)
#define CYGNUM_CALL_IF_FLASH_FIS_GET_MEM_BASE    (2)
#define CYGNUM_CALL_IF_FLASH_FIS_GET_ENTRY_POINT (3)
#define CYGNUM_CALL_IF_FLASH_FIS_GET_DATA_LENGTH (4)
#define CYGNUM_CALL_IF_FLASH_FIS_GET_DESC_CKSUM  (5)
#define CYGNUM_CALL_IF_FLASH_FIS_GET_FILE_CKSUM  (6)


// These need to be kept uptodate with the (unadorned) masters
// in RedBoot's flash_config.h:
#define CYGNUM_FLASH_CFG_TYPE_CONFIG_EMPTY   0
#define CYGNUM_FLASH_CFG_TYPE_CONFIG_BOOL    1
#define CYGNUM_FLASH_CFG_TYPE_CONFIG_INT     2
#define CYGNUM_FLASH_CFG_TYPE_CONFIG_STRING  3
#define CYGNUM_FLASH_CFG_TYPE_CONFIG_SCRIPT  4
#define CYGNUM_FLASH_CFG_TYPE_CONFIG_IP      5
#define CYGNUM_FLASH_CFG_TYPE_CONFIG_ESA     6

#endif // CYGACC_CALL_IF_DEFINED

//--------------------------------------------------------------------------
// Diag wrappers.
externC void hal_if_diag_init(void);
externC void hal_if_diag_write_char(char c);
externC void hal_if_diag_read_char(char *c);

//--------------------------------------------------------------------------
// Ctrl-c support.
externC cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);
externC cyg_bool   hal_ctrlc_check(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_CTRLC_ISR hal_ctrlc_isr
#define HAL_CTRLC_CHECK hal_ctrlc_check

#else

#if defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)
// Then other code might invoke this macro
#define HAL_CTRLC_CHECK(a1,a2) (0) // Nothing, no CTRLC here
#endif

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

//--------------------------------------------------------------------------
// Functions provided by the HAL interface.
externC void hal_if_init(void);
#if 0 != CYGINT_HAL_PLF_IF_INIT
externC void plf_if_init(void);
#endif

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_HAL_IF_H
// End of hal_if.h
