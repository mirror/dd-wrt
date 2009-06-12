//=============================================================================
//
//      hal_if.c
//
//      ROM/RAM interfacing functions
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
// Copyright (C) 2003 Nick Garnett <nickg@calivar.com>
// Copyright (C) 2003 Jonathan Larmour <jlarmour@eCosCentric.com>
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
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov
// Contributors:jskov, woehler
// Date:        2000-06-07
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif

#include <cyg/infra/cyg_ass.h>          // assertions

#include <cyg/hal/hal_arch.h>           // set/restore GP

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // our interface

#include <cyg/hal/hal_diag.h>           // Diag IO
#include <cyg/hal/hal_misc.h>           // User break

#include <cyg/hal/hal_stub.h>           // stub functionality

#include <cyg/hal/hal_intr.h>           // hal_vsr_table and others

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
#include <redboot.h>
#include <flash_config.h>
#endif
#ifdef CYGOPT_REDBOOT_FIS
#include <fis.h>
#endif
#endif

//--------------------------------------------------------------------------

externC void patch_dbg_syscalls(void * vector);
externC void init_thread_syscall(void * vector);

//--------------------------------------------------------------------------
// Implementations and function wrappers for monitor services

// flash config state queries
#ifdef CYGSEM_REDBOOT_FLASH_CONFIG

static __call_if_flash_cfg_op_fn_t flash_config_op;

static cyg_bool
flash_config_op(int op, struct cyg_fconfig *fc)
{
    cyg_bool res = false;

    CYGARC_HAL_SAVE_GP();

    switch (op) {
    case CYGNUM_CALL_IF_FLASH_CFG_GET:
        res = flash_get_config(fc->key, fc->val, fc->type);
        break;
    case CYGNUM_CALL_IF_FLASH_CFG_NEXT:
        res = flash_next_key(fc->key, fc->keylen, &fc->type, &fc->offset);
        break;
    case CYGNUM_CALL_IF_FLASH_CFG_SET:
        res = flash_set_config(fc->key, fc->val, fc->type);
        break;
    default:
        // nothing else supported yet - though it is expected that "set"
        // will fit the same set of arguments, potentially.
        break;
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}
#endif

#ifdef CYGOPT_REDBOOT_FIS

static __call_if_flash_fis_op_fn_t flash_fis_op;

static cyg_bool
flash_fis_op( int op, char *name, void *val)
{
    cyg_bool res = false;
    struct fis_image_desc *fis;
    int num;

    CYGARC_HAL_SAVE_GP();
    fis = fis_lookup(name, &num);
    if(fis != NULL)
    {
        switch ( op ) {
        case CYGNUM_CALL_IF_FLASH_FIS_GET_FLASH_BASE:
            *(CYG_ADDRESS *)val = fis->flash_base; 
            res = true;
            break;
        case CYGNUM_CALL_IF_FLASH_FIS_GET_SIZE:
            *(unsigned long *)val = fis->size;
            res = true;
            break;
        case CYGNUM_CALL_IF_FLASH_FIS_GET_MEM_BASE:
            *(CYG_ADDRESS *)val = fis->mem_base;
            res = true;
            break;
        case CYGNUM_CALL_IF_FLASH_FIS_GET_ENTRY_POINT:
            *(CYG_ADDRESS *)val = fis->entry_point;
            res = true;
            break;
        case CYGNUM_CALL_IF_FLASH_FIS_GET_DATA_LENGTH:
            *(unsigned long *)val = fis->data_length;
            res = true;
            break;
        case CYGNUM_CALL_IF_FLASH_FIS_GET_DESC_CKSUM:
            *(unsigned long *)val = fis->desc_cksum;
            res = true;
            break;
        case CYGNUM_CALL_IF_FLASH_FIS_GET_FILE_CKSUM:
            *(unsigned long *)val = fis->file_cksum;
            res = true;
            break;
        default:
            break;
        }
    }
    CYGARC_HAL_RESTORE_GP();
    return res;
}
#endif

//----------------------------
// Delay uS
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_DELAY_US

static __call_if_delay_us_t delay_us;

static void
delay_us(cyg_int32 usecs)
{
    CYGARC_HAL_SAVE_GP();
#ifdef CYGPKG_KERNEL
    {
        cyg_int32 start, elapsed, elapsed_usec;
        cyg_int32 slice;
        cyg_int32 usec_per_period = CYGNUM_HAL_RTC_NUMERATOR/CYGNUM_HAL_RTC_DENOMINATOR/1000;
        cyg_int32 ticks_per_usec = CYGNUM_KERNEL_COUNTERS_RTC_PERIOD/usec_per_period;
        
        do {
            // Spin in slices of 1/2 the RTC period. Allows interrupts
            // time to run without messing up the algorithm. If we
            // spun for 1 period (or more) of the RTC, there would also
            // be problems figuring out when the timer wrapped.  We
            // may lose a tick or two for each cycle but it shouldn't
            // matter much.

            // The tests against CYGNUM_KERNEL_COUNTERS_RTC_PERIOD
            // check for a value that would cause a 32 bit signed
            // multiply to overflow. But this also implies that just
            // multiplying by ticks_per_usec will yield a good
            // approximation.  Otherwise we need to do the full
            // multiply+divide to get sufficient accuracy. Note that
            // this test is actually constant, so the compiler will
            // eliminate it and only compile the branch that is
            // selected.
            
            if( usecs > usec_per_period/2 )
                slice = CYGNUM_KERNEL_COUNTERS_RTC_PERIOD/2;
            else if( CYGNUM_KERNEL_COUNTERS_RTC_PERIOD/2 >= 0x7FFFFFFF/usec_per_period )
                slice = usecs * ticks_per_usec;
            else
            {
                slice = usecs*CYGNUM_KERNEL_COUNTERS_RTC_PERIOD;
                slice /= usec_per_period;
            }
    
            HAL_CLOCK_READ(&start);
            do {
                HAL_CLOCK_READ(&elapsed);
                elapsed = (elapsed - start); // counts up!
                if (elapsed < 0)
                    elapsed += CYGNUM_KERNEL_COUNTERS_RTC_PERIOD;
            } while (elapsed < slice);
            
            // Adjust by elapsed, not slice, since an interrupt may
            // have been stalling us for some time.

            if( CYGNUM_KERNEL_COUNTERS_RTC_PERIOD >= 0x7FFFFFFF/usec_per_period )
                elapsed_usec = elapsed / ticks_per_usec;
            else
            {
                elapsed_usec = elapsed * usec_per_period;
                elapsed_usec = elapsed_usec / CYGNUM_KERNEL_COUNTERS_RTC_PERIOD;
            }

            // It is possible for elapsed_usec to end up zero in some
            // circumstances and we could end up looping indefinitely.
            // Avoid that by ensuring that we always decrement usec by
            // at least 1 each time.
            
            usecs -= elapsed_usec ? elapsed_usec : 1;
            
        } while (usecs > 0);
    }
#else // CYGPKG_KERNEL
#ifdef HAL_DELAY_US
    // Use a HAL feature if defined
    HAL_DELAY_US(usecs);
#else
    // If no accurate delay mechanism, just spin for a while. Having
    // an inaccurate delay is much better than no delay at all. The
    // count of 10 should mean the loop takes something resembling
    // 1us on most CPUs running between 30-100MHz [depends on how many
    // instructions this compiles to, how many dispatch units can be
    // used for the simple loop, actual CPU frequency, etc]
    while (usecs-- > 0) {
        int i;
        for (i = 0; i < 10; i++);
    }
#endif // HAL_DELAY_US
#endif // CYGPKG_KERNEL
    CYGARC_HAL_RESTORE_GP();
}
#endif // CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_DELAY_US

// Reset functions
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_RESET

static __call_if_reset_t reset;

static void
reset(void)
{
    CYGARC_HAL_SAVE_GP();
    // With luck, the platform defines some magic that will cause a hardware
    // reset.
#ifdef HAL_PLATFORM_RESET
    HAL_PLATFORM_RESET();
#endif

#ifdef HAL_PLATFORM_RESET_ENTRY
    // If that's not the case (above is an empty statement) there may
    // be defined an address we can jump to - and effectively
    // reinitialize the system. Not quite as good as a reset, but it
    // is often enough.
    goto *HAL_PLATFORM_RESET_ENTRY;

#else
#error " no RESET_ENTRY"
#endif
    CYG_FAIL("Reset failed");
    CYGARC_HAL_RESTORE_GP();
}

#endif

//------------------------------------
// NOP service
#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_INIT_WHOLE_TABLE) || \
    defined(CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_COMMS)
static int
nop_service(void)
{
    // This is the default service. It always returns false (0), and
    // _does not_ trigger any assertions. Clients must either cope
    // with the service failure or assert.
    return 0;
}
#endif

//----------------------------------
// Comm controls
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_COMMS

#ifdef CYGNUM_HAL_VIRTUAL_VECTOR_AUX_CHANNELS
#define CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS \
  (CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS+CYGNUM_HAL_VIRTUAL_VECTOR_AUX_CHANNELS)
#else
#define CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS \
  CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS
#endif

static hal_virtual_comm_table_t comm_channels[CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS+1];

static int
set_debug_comm(int __comm_id)
{
    static int __selected_id = CYGNUM_CALL_IF_SET_COMM_ID_EMPTY;
    hal_virtual_comm_table_t* __chan;
    int interrupt_state = 0;
    int res = 1, update = 0;
    CYGARC_HAL_SAVE_GP();

    CYG_ASSERT(__comm_id >= CYGNUM_CALL_IF_SET_COMM_ID_MANGLER
               && __comm_id < CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS,
               "Invalid channel");

    switch (__comm_id) {
    case CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT:
        if (__selected_id > 0)
            res = __selected_id-1;
        else if (__selected_id == 0)
            res = CYGNUM_CALL_IF_SET_COMM_ID_MANGLER;
        else 
            res = __selected_id;
        break;

    case CYGNUM_CALL_IF_SET_COMM_ID_EMPTY:
        CYGACC_CALL_IF_DEBUG_PROCS_SET(0);
        __selected_id = __comm_id;
        break;

    case CYGNUM_CALL_IF_SET_COMM_ID_MANGLER:
        __comm_id = 0;
        update = 1;
        break;

    default:
        __comm_id++;                    // skip mangler entry
        update = 1;
        break;
    }

    if (update) {
        // Find the interrupt state of the channel.
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();
        if (__chan)
            interrupt_state = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_DISABLE);

        __selected_id = __comm_id;
        CYGACC_CALL_IF_DEBUG_PROCS_SET(comm_channels[__comm_id]);

        // Set interrupt state on the new channel.
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();
        if (interrupt_state)
            CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_ENABLE);
        else
            CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_DISABLE);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static int
set_console_comm(int __comm_id)
{
    static int __selected_id = CYGNUM_CALL_IF_SET_COMM_ID_EMPTY;
    int res = 1, update = 0;
    CYGARC_HAL_SAVE_GP();

    CYG_ASSERT(__comm_id >= CYGNUM_CALL_IF_SET_COMM_ID_MANGLER
               && __comm_id < CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS,
               "Invalid channel");

    switch (__comm_id) {
    case CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT:
        if (__selected_id > 0)
            res = __selected_id-1;
        else if (__selected_id == 0)
            res = CYGNUM_CALL_IF_SET_COMM_ID_MANGLER;
        else
            res = __selected_id;
        break;

    case CYGNUM_CALL_IF_SET_COMM_ID_EMPTY:
        CYGACC_CALL_IF_CONSOLE_PROCS_SET(0);
        __selected_id = __comm_id;
        break;

    case CYGNUM_CALL_IF_SET_COMM_ID_MANGLER:
        __comm_id = 0;
        update = 1;
        break;

    default:
        __comm_id++;                    // skip mangler entry
        update = 1;
        break;
    }
    
    if (update) {
        __selected_id = __comm_id;
    
        CYGACC_CALL_IF_CONSOLE_PROCS_SET(comm_channels[__comm_id]);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}
#endif

//----------------------------------
// Cache functions
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_CACHE

static void
flush_icache(void *__p, int __nbytes)
{
    CYGARC_HAL_SAVE_GP();
#ifdef HAL_ICACHE_FLUSH
    HAL_ICACHE_FLUSH( __p , __nbytes );
#elif defined(HAL_ICACHE_INVALIDATE)
    HAL_ICACHE_INVALIDATE();
#endif
    CYGARC_HAL_RESTORE_GP();
}

static void
flush_dcache(void *__p, int __nbytes)
{
    CYGARC_HAL_SAVE_GP();
#ifdef HAL_DCACHE_FLUSH
    HAL_DCACHE_FLUSH( __p , __nbytes );
#elif defined(HAL_DCACHE_INVALIDATE)
    HAL_DCACHE_INVALIDATE();
#endif
    CYGARC_HAL_RESTORE_GP();
}
#endif

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_DIAG)
//-----------------------------------------------------------------------------
// GDB console output mangler (O-packetizer)
// COMMS init function at end.

// This gets called via the virtual vector console comms entry and
// handles O-packetization. The debug comms entries are used for the
// actual device IO.
static cyg_uint8
cyg_hal_diag_mangler_gdb_getc(void* __ch_data)
{
    cyg_uint8 __ch;
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();
    CYGARC_HAL_SAVE_GP();

    __ch = CYGACC_COMM_IF_GETC(*__chan);

    CYGARC_HAL_RESTORE_GP();

    return __ch;
}

static char __mangler_line[100];
static int  __mangler_pos = 0;

static void
cyg_hal_diag_mangler_gdb_flush(void* __ch_data)
{
    CYG_INTERRUPT_STATE old;
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();

    // Nothing to do if mangler buffer is empty.
    if (__mangler_pos == 0)
        return;

    // Disable interrupts. This prevents GDB trying to interrupt us
    // while we are in the middle of sending a packet. The serial
    // receive interrupt will be seen when we re-enable interrupts
    // later.
#if defined(CYG_HAL_STARTUP_ROM) \
    || !defined(CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION)
    HAL_DISABLE_INTERRUPTS(old);
#else
    CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#endif
        
#if CYGNUM_HAL_DEBUG_GDB_PROTOCOL_RETRIES != 0
    // Only wait 500ms for data to arrive - avoid "stuck" connections
    CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_SET_TIMEOUT, CYGNUM_HAL_DEBUG_GDB_PROTOCOL_TIMEOUT);
#endif

    while(1)
    {
	static const char hex[] = "0123456789ABCDEF";
	cyg_uint8 csum = 0, c1;
	int i;
        
	CYGACC_COMM_IF_PUTC(*__chan, '$');
	CYGACC_COMM_IF_PUTC(*__chan, 'O');
	csum += 'O';
	for( i = 0; i < __mangler_pos; i++ )
        {
	    char ch = __mangler_line[i];
	    char h = hex[(ch>>4)&0xF];
	    char l = hex[ch&0xF];
	    CYGACC_COMM_IF_PUTC(*__chan, h);
	    CYGACC_COMM_IF_PUTC(*__chan, l);
	    csum += h;
	    csum += l;
	}
	CYGACC_COMM_IF_PUTC(*__chan, '#');
	CYGACC_COMM_IF_PUTC(*__chan, hex[(csum>>4)&0xF]);
	CYGACC_COMM_IF_PUTC(*__chan, hex[csum&0xF]);

    nak:
#if CYGNUM_HAL_DEBUG_GDB_PROTOCOL_RETRIES != 0
	if (CYGACC_COMM_IF_GETC_TIMEOUT(*__chan, &c1) == 0) {
	    c1 = '-';
	    if (tries && (--tries == 0)) c1 = '+';
	}
#else
	c1 = CYGACC_COMM_IF_GETC(*__chan);
#endif

	if( c1 == '+' ) break;

	if( cyg_hal_is_break( &c1 , 1 ) ) {
	    // Caller's responsibility to react on this.
	    CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG_SET(1);
	    break;
	}
	if( c1 != '-' ) goto nak;
    }

    __mangler_pos = 0;
    // And re-enable interrupts
#if defined(CYG_HAL_STARTUP_ROM) \
    || !defined(CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION)
    HAL_RESTORE_INTERRUPTS(old);
#else
    CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#endif
}

static void
cyg_hal_diag_mangler_gdb_putc(void* __ch_data, cyg_uint8 c)
{
#if CYGNUM_HAL_DEBUG_GDB_PROTOCOL_RETRIES != 0
    int tries = CYGNUM_HAL_DEBUG_GDB_PROTOCOL_RETRIES;
#endif

    // No need to send CRs
    if( c == '\r' ) return;

    CYGARC_HAL_SAVE_GP();

    __mangler_line[__mangler_pos++] = c;

    if( c == '\n' || __mangler_pos == sizeof(__mangler_line) )
	cyg_hal_diag_mangler_gdb_flush(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_diag_mangler_gdb_write(void* __ch_data,
                               const cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_diag_mangler_gdb_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_diag_mangler_gdb_read(void* __ch_data, 
                              cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_diag_mangler_gdb_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

static int
cyg_hal_diag_mangler_gdb_control(void *__ch_data, 
                                 __comm_control_cmd_t __func, ...)
{
    CYGARC_HAL_SAVE_GP();

    if (__func == __COMMCTL_FLUSH_OUTPUT)
	cyg_hal_diag_mangler_gdb_flush(__ch_data);

    CYGARC_HAL_RESTORE_GP();
    return 0;
}

// This is the COMMS init function. It gets called both by the stubs
// and diag init code to initialize the COMMS mangler channel table -
// that's all. The callers have to decide whether to actually use this
// channel.
void
cyg_hal_diag_mangler_gdb_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Initialize mangler procs
    CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_MANGLER);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_diag_mangler_gdb_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_diag_mangler_gdb_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_diag_mangler_gdb_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_diag_mangler_gdb_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_diag_mangler_gdb_control);
    
    // Restore the original console channel.
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

//-----------------------------------------------------------------------------
// Null console output mangler
// COMMS init function at end.

// This gets called via the virtual vector console comms entry and
// just forwards IO to the debug comms entries.
// This differs from setting the console channel to the same as the
// debug channel in that console output will go to the debug channel
// even if the debug channel is changed.
static cyg_uint8
cyg_hal_diag_mangler_null_getc(void* __ch_data)
{
    cyg_uint8 __ch;
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();
    CYGARC_HAL_SAVE_GP();

    __ch = CYGACC_COMM_IF_GETC(*__chan);

    CYGARC_HAL_RESTORE_GP();

    return __ch;
}


static void
cyg_hal_diag_mangler_null_putc(void* __ch_data, cyg_uint8 c)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();

    CYGARC_HAL_SAVE_GP();

    CYGACC_COMM_IF_PUTC(*__chan, c);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_diag_mangler_null_write(void* __ch_data,
                                const cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_diag_mangler_null_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_diag_mangler_null_read(void* __ch_data, 
                               cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_diag_mangler_null_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

static int
cyg_hal_diag_mangler_null_control(void *__ch_data, 
                                  __comm_control_cmd_t __func, ...)
{
    // Do nothing (yet).
    return 0;
}

// This is the COMMS init function. It gets called both by the stubs
// and diag init code to initialize the COMMS mangler channel table -
// that's all. The callers have to decide whether to actually use this
// channel.
void
cyg_hal_diag_mangler_null_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Initialize mangler procs
    CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_MANGLER);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_diag_mangler_null_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_diag_mangler_null_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_diag_mangler_null_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_diag_mangler_null_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_diag_mangler_null_control);
    
    // Restore the original console channel.
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

//-----------------------------------------------------------------------------
// Console IO functions that adhere to the virtual vector table semantics in
// order to ensure proper debug agent mangling when required.
//
externC void cyg_hal_plf_comms_init(void);

void
hal_if_diag_init(void)
{
    // This function may be called from various places and the code
    // should only run once.
    static cyg_uint8 called = 0;
    if (called) return;
    called = 1;

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_INHERIT_CONSOLE

#if defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
    // Use the mangler channel, which in turn uses the debug channel.
    CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_MANGLER);

    // Initialize the mangler channel.
#if defined(CYGSEM_HAL_DIAG_MANGLER_GDB)
    cyg_hal_diag_mangler_gdb_init();
#elif defined(CYGSEM_HAL_DIAG_MANGLER_None)
    cyg_hal_diag_mangler_null_init();
#endif

#else // CYGDBG_HAL_DIAG_TO_DEBUG_CHAN

    // Use an actual (raw) IO channel
    CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL);

#endif // CYGDBG_HAL_DIAG_TO_DEBUG_CHAN

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_INHERIT_CONSOLE
}

void 
hal_if_diag_write_char(char c)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_CONSOLE_PROCS();

    if (__chan)
        CYGACC_COMM_IF_PUTC(*__chan, c);
    else {
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();

        // FIXME: What should be done if assertions are not enabled?
        // This is a bad bad situation - we have no means for diag
        // output; we want to hit a breakpoint to alert the developer
        // or something like that.
        CYG_ASSERT(__chan, "No valid channel set");

        CYGACC_COMM_IF_PUTC(*__chan, c);
    }

    // Check interrupt flag
    if (CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG()) {
        CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG_SET(0);
        cyg_hal_user_break(0);
    }
}

void 
hal_if_diag_read_char(char *c)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    
    if (__chan)
        *c = CYGACC_COMM_IF_GETC(*__chan);
    else {
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();

        // FIXME: What should be done if assertions are not enabled?
        // This is a bad bad situation - we have no means for diag
        // output; we want to hit a breakpoint to alert the developer
        // or something like that.
        CYG_ASSERT(__chan, "No valid channel set");

        *c = CYGACC_COMM_IF_GETC(*__chan);
    }
}
#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

//=============================================================================
// CtrlC support
//=============================================================================

#if defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT) \
    || defined(CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT)

struct Hal_SavedRegisters *hal_saved_interrupt_state;

void
hal_ctrlc_isr_init(void)
{
    // A ROM monitor never enables the interrupt itself. This is left
    // to the (RAM) application.
#ifndef CYGSEM_HAL_ROM_MONITOR
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();

#if 1 // Prevents crash on older stubs
    int v_m;
    // Allow only ctrl-c interrupt enabling when version in table is
    // below legal max and above the necessary service, and _not_
    // the value we set it to below.
    v_m = CYGACC_CALL_IF_VERSION() & CYGNUM_CALL_IF_TABLE_VERSION_CALL_MASK;
    if (v_m >= CYGNUM_CALL_IF_TABLE_VERSION_CALL_MAX 
        || v_m < CYGNUM_CALL_IF_SET_DEBUG_COMM
        || v_m == CYGNUM_CALL_IF_TABLE_VERSION_CALL_HACK)
        return;

    // Now trash that value - otherwise downloading an image with
    // builtin stubs on a board with older stubs (which will cause the
    // version to be set to VERSION_CALL) may cause all subsequent
    // runs to (wrongly) fall through to the below code.  If there is
    // a new stub on the board, it will reinitialize the version field
    // on reset.  Yes, this is a gross hack!
    CYGACC_CALL_IF_VERSION_SET(CYGNUM_CALL_IF_TABLE_VERSION_CALL_HACK);
#endif

    // We can only enable interrupts on a valid debug channel.
    if (__chan)
        CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_ENABLE);
#endif
}

cyg_uint32
hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();
    int isr_ret = 0, ctrlc = 0;

    if (__chan) {
        isr_ret = CYGACC_COMM_IF_DBG_ISR(*__chan, &ctrlc, vector, data);
        if (ctrlc)
            cyg_hal_user_break( (CYG_ADDRWORD *)hal_saved_interrupt_state );
    }
    return isr_ret;
}

cyg_bool
hal_ctrlc_check(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_DEBUG_PROCS();
    int gdb_vector = vector-1;
    int isr_ret, ctrlc = 0;

    // This check only to avoid crash on older stubs in case of unhandled
    // interrupts. It is a bit messy, but required in a transition period.
    if (__chan && 
        (CYGNUM_CALL_IF_TABLE_VERSION_CALL_HACK == 
         (CYGACC_CALL_IF_VERSION() & CYGNUM_CALL_IF_TABLE_VERSION_CALL_MASK))){
        gdb_vector = CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_DBG_ISR_VECTOR);
    }
    if (vector == gdb_vector) {
        isr_ret = CYGACC_COMM_IF_DBG_ISR(*__chan, &ctrlc, vector, data);
        if (ctrlc) {
            cyg_hal_user_break( (CYG_ADDRWORD *)hal_saved_interrupt_state );
            return true;
        }
    }
    return false;
}
#endif // CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT || CYGDBG_HAL_DEBUG_GDB_CTRLC_SUPPORT

//--------------------------------------------------------------------------
// Init function. It should be called from the platform initialization code.
// For monitor configurations it will initialize the calling interface table,
// for client configurations it will patch the existing table as per
// configuration.
void
hal_if_init(void)
{
    //**********************************************************************
    //
    // Note that if your RAM application is configured to initialize
    // the whole table _or_ the communication channels, you _cannot_
    // step through this function with the debugger. If your channel
    // configurations are set to the default, you should be able to
    // simply step over this function though (or use 'finish' once you
    // have entered this function if that GDB command works).
    // 
    // If you really do need to debug this code, the best approach is
    // to have a working RedBoot / GDB stub in ROM and then change the
    // hal_virtual_vector_table to reside at some other address in the
    // RAM configuration than that used by the ROM monitor.  Then
    // you'll be able to use the ROM monitor to debug the below code
    // and check that it does the right thing.
    //
    // Note that if you have a ROM monitor in ROM/flash which does
    // support virtual vectors, you should be able to disable the
    // option CYGSEM_HAL_VIRTUAL_VECTOR_INIT_WHOLE_TABLE. On some
    // targets (which predate the introduction of virtual vectors)
    // that option is enabled per default and needs to be explicitly
    // disabled when you have an updated ROM monitor.
    //
    //**********************************************************************

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_INIT_WHOLE_TABLE
    {
        int i;

        // Initialize tables with the NOP service.
        // This should only be done for service routine entries - data
        // pointers should be NULLed.
        for (i = 0; i < CYGNUM_CALL_IF_TABLE_SIZE; i++)
            hal_virtual_vector_table[i] = (CYG_ADDRWORD) &nop_service;
        
        // Version number
        CYGACC_CALL_IF_VERSION_SET(CYGNUM_CALL_IF_TABLE_VERSION_CALL
            |((CYG_ADDRWORD)CYGNUM_CALL_IF_TABLE_VERSION_COMM<<CYGNUM_CALL_IF_TABLE_VERSION_COMM_shift));
    }
#endif

    // Miscellaneous services with wrappers in this file.
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_RESET
    CYGACC_CALL_IF_RESET_SET(reset);
#endif
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_DELAY_US
    CYGACC_CALL_IF_DELAY_US_SET(delay_us);
#endif

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_CACHE
    // Cache functions
    CYGACC_CALL_IF_FLUSH_ICACHE_SET(flush_icache);
    CYGACC_CALL_IF_FLUSH_DCACHE_SET(flush_dcache);
#endif

#ifdef CYGSEM_REDBOOT_FLASH_CONFIG
    CYGACC_CALL_IF_FLASH_CFG_OP_SET(flash_config_op);
#endif

#ifdef CYGOPT_REDBOOT_FIS
    CYGACC_CALL_IF_FLASH_FIS_OP_SET(flash_fis_op);
#endif

    // Data entries not currently supported in eCos
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_DATA
    CYGACC_CALL_IF_DBG_DATA_SET(0);
#endif

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_VERSION
    CYGACC_CALL_IF_MONITOR_VERSION_SET(0);
#endif

    // Comm controls
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_CLAIM_COMMS
    {
        int i, j;

        // Clear out tables with safe dummy function.
        for (j = 0; j < CYGNUM_HAL_VIRTUAL_VECTOR_NUM_CHANNELS+1; j++)
            for (i = 0; i < CYGNUM_COMM_IF_TABLE_SIZE; i++)
                comm_channels[j][i] = (CYG_ADDRWORD) &nop_service;

        // Set accessor functions
        CYGACC_CALL_IF_SET_DEBUG_COMM_SET(set_debug_comm);
        CYGACC_CALL_IF_SET_CONSOLE_COMM_SET(set_console_comm);

        // Initialize console/debug procs. Note that these _must_
        // be set to empty before the comms init call.
        set_debug_comm(CYGNUM_CALL_IF_SET_COMM_ID_EMPTY);
        set_console_comm(CYGNUM_CALL_IF_SET_COMM_ID_EMPTY);

        // Initialize channels. This used to be done in
        // hal_diag_init() and the stub initHardware() functions, but
        // it makes more sense to have here.
        cyg_hal_plf_comms_init();

        // Always set the debug channel. If stubs are included, it is
        // necessary. If no stubs are included it does not hurt and is
        // likely to be required by the hal_if_diag_init code anyway
        // as it may rely on it if using a mangler.
        set_debug_comm(CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL);
        // Set console channel to a safe default. hal_if_diag_init
        // will override with console channel or mangler if necessary.
        set_console_comm(CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL);
    }

    // Reset console interrupt flag.
    CYGACC_CALL_IF_CONSOLE_INTERRUPT_FLAG_SET(0);
#endif

    // Set up services provided by clients
#if defined(CYGFUN_HAL_COMMON_KERNEL_SUPPORT)   &&  \
    ( defined(CYGSEM_HAL_USE_ROM_MONITOR_GDB_stubs) \
      || defined(CYGSEM_HAL_USE_ROM_MONITOR_CygMon))

    patch_dbg_syscalls( (void *)(hal_virtual_vector_table) );
#endif

    // Init client services
#if !defined(CYGPKG_KERNEL) && defined(CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT)
    // Only include this code if we do not have a kernel. Otherwise
    // the kernel supplies the functionality for the app we are linked
    // with.

    // Prepare for application installation of thread info function in
    // vector table.
    init_thread_syscall( (void *)&hal_virtual_vector_table[CYGNUM_CALL_IF_DBG_SYSCALL] );
#endif

    // Finally, install async breakpoint handler if it is configured in.
    // FIXME: this should probably check for STUBS instead (but code is
    //        conditional on BREAK for now)
#if defined(CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT)
    // Install async breakpoint handler into vector table.
    CYGACC_CALL_IF_INSTALL_BPT_FN_SET(&cyg_hal_gdb_interrupt);
#endif

#if 0 != CYGINT_HAL_PLF_IF_INIT
    // Call platform specific initializations - should only be used
    // to augment what has already been set up, etc.
    plf_if_init();
#endif
}

