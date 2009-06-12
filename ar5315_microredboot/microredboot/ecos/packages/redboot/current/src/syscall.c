/*==========================================================================
//
//      syscall.c
//
//      Redboot syscall handling for GNUPro bsp support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
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
// Author(s):    msalter
// Contributors: msalter
// Date:         1999-02-20
// Purpose:      Temporary support for gnupro bsp
//
//####DESCRIPTIONEND####
//
//=========================================================================*/

#include <redboot.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_stub.h>

#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS

#define NEWLIB_EIO 5              /* I/O error */
#define NEWLIB_ENOSYS 88          /* Syscall not supported */

/*
 * Clients of this BSP will need to have access to BSP functions and
 * data structures. Because, the client and the BSP may not be linked
 * together, a structure of vectors is used to gain this access. A
 * pointer to this structure can be gotten via a syscall. This syscall
 * is made automatically from within the crt0.o file.
 */
typedef struct {
    int         version;        /* version number for future expansion */
    const void **__ictrl_table;
    void **__exc_table;
    void *__dbg_vector;
    void *__kill_vector;
    void *__console_procs;
    void *__debug_procs;
    void (*__flush_dcache)(void *__p, int __nbytes);
    void (*__flush_icache)(void *__p, int __nbytes);
    void *__cpu_data;
    void *__board_data;
    void *__sysinfo;
    int  (*__set_debug_comm)(int __comm_id);
    int  (*__set_console_comm)(int __comm_id);
    int  (*__set_serial_baud)(int __comm_id, int baud);
    void *__dbg_data;
    void (*__reset)(void);
    int  __console_interrupt_flag;
} __shared_t;

static __shared_t __shared_data = { 2 };

// this is used by newlib's mode_t so we should match it
#ifdef __GNUC__
#define _ST_INT32 __attribute__ ((__mode__ (__SI__)))
#else
#define _ST_INT32
#endif
typedef unsigned int    newlib_mode_t _ST_INT32;
typedef short           newlib_dev_t;
typedef unsigned short  newlib_ino_t;
typedef unsigned short  newlib_nlink_t;
typedef long            newlib_off_t;
typedef unsigned short  newlib_uid_t;
typedef unsigned short  newlib_gid_t;
typedef long            newlib_time_t;
typedef long            newlib_long_t;

struct newlib_stat 
{
    newlib_dev_t     st_dev;
    newlib_ino_t     st_ino;
    newlib_mode_t    st_mode;
    newlib_nlink_t   st_nlink;
    newlib_uid_t     st_uid;
    newlib_gid_t     st_gid;
    newlib_dev_t     st_rdev;
    newlib_off_t     st_size;
    // We assume we've been compiled with the same flags as newlib here
#if defined(__svr4__) && !defined(__PPC__) && !defined(__sun__)
    newlib_time_t    st_atime;
    newlib_time_t    st_mtime;
    newlib_time_t    st_ctime;
#else
    newlib_time_t    st_atime;
    newlib_long_t    st_spare1;
    newlib_time_t    st_mtime;
    newlib_long_t    st_spare2;
    newlib_time_t    st_ctime;
    newlib_long_t    st_spare3;
    newlib_long_t    st_blksize;
    newlib_long_t    st_blocks;
    newlib_long_t    st_spare4[2];
#endif
};
#define NEWLIB_S_IFCHR 0020000 // character special file

static inline char __getc(void)
{
    char c;
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    
    if (__chan)
        c = CYGACC_COMM_IF_GETC(*__chan);
    else {
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();
        c = CYGACC_COMM_IF_GETC(*__chan);
    }
    return c;
}

static inline void __putc(char c)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_CONSOLE_PROCS();
    if (__chan)
        CYGACC_COMM_IF_PUTC(*__chan, c);
    else {
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();
        CYGACC_COMM_IF_PUTC(*__chan, c);
    }
}


static inline void __flush(void)
{
    hal_virtual_comm_table_t* __chan = CYGACC_CALL_IF_CONSOLE_PROCS();

    if (__chan == NULL)
        __chan = CYGACC_CALL_IF_DEBUG_PROCS();

    CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_FLUSH_OUTPUT);
}

// Timer support

static cyg_handle_t  sys_timer_handle;
static cyg_interrupt sys_timer_interrupt;
static cyg_uint64    sys_timer_ticks = 0;

#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF

static unsigned int set_period = CYGNUM_HAL_RTC_PERIOD; // The default

typedef void *callback_func( char *pc, char *sp );
static callback_func *timer_callback = 0;

#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF

static void
sys_timer_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    // do nothing
}


static cyg_uint32
sys_timer_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    ++sys_timer_ticks;

#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    HAL_CLOCK_RESET(CYGNUM_HAL_INTERRUPT_RTC, set_period);
#else
    HAL_CLOCK_RESET(CYGNUM_HAL_INTERRUPT_RTC, CYGNUM_HAL_RTC_PERIOD);
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF

    cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_RTC);

#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    if ( timer_callback ) {
        char *intrpc = (char *)0;
        char *intrsp = (char *)0;

        // There may be a number of ways to get the PC and (optional) SP
        // information out of the HAL.  Hence this is conditioned.  In some
        // configurations, a register-set pointer is available as
        // (invisible) argument 3 to this ISR call.

#ifdef HAL_GET_PROFILE_INFO
        HAL_GET_PROFILE_INFO( intrpc, intrsp );
#endif // HAL_GET_PROFILE_INFO available

        CYGARC_HAL_SAVE_GP();
        timer_callback( intrpc, intrsp );
        CYGARC_HAL_RESTORE_GP();
    }
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    return CYG_ISR_HANDLED;
}


static void sys_timer_init(void)
{
#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    HAL_CLOCK_INITIALIZE(set_period);
#else
    HAL_CLOCK_INITIALIZE(CYGNUM_HAL_RTC_PERIOD);
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    
    cyg_drv_interrupt_create(
        CYGNUM_HAL_INTERRUPT_RTC,
        0,                      // Priority - unused
        (CYG_ADDRWORD)0,        // Data item passed to ISR & DSR
        sys_timer_isr,          // ISR
        sys_timer_dsr,          // DSR
        &sys_timer_handle,      // handle to intr obj
        &sys_timer_interrupt ); // space for int obj

    cyg_drv_interrupt_attach(sys_timer_handle);

    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_RTC);
}


//
// read  -- read bytes from the serial port. Ignore fd, since
//          we only have stdin.
static int
sys_read(int fd, char *buf, int nbytes)
{
    int i = 0;

    for (i = 0; i < nbytes; i++) {
        *(buf + i) = __getc();
        if ((*(buf + i) == '\n') || (*(buf + i) == '\r')) {
            (*(buf + i + 1)) = 0;
            break;
        }
    }
    return (i);
}


//
// write -- write bytes to the serial port. Ignore fd, since
//          stdout and stderr are the same. Since we have no filesystem,
//          open will only return an error.
//
static int
sys_write(int fd, char *buf, int nbytes)
{
#define WBUFSIZE  256
    int  tosend;

    tosend = nbytes;

    while (tosend > 0) {
        if (*buf == '\n')
            __putc('\r');
        __putc(*buf++);
        tosend--;
    }
    __flush();

    return (nbytes);
}


//
// open -- open a file descriptor. We don't have a filesystem, so
//         we return an error.
//
static int
sys_open (const char *buf, int flags, int mode)
{
    return (-NEWLIB_EIO);
}

//
// close -- We don't need to do anything, but pretend we did.
//
static int
sys_close(int fd)
{
    return (0);
}


//
// lseek --  Since a serial port is non-seekable, we return an error.
//
static int
sys_lseek(int fd,  int offset, int whence)
{
    return (-NEWLIB_EIO);
}


#define NS_PER_TICK    (CYGNUM_HAL_RTC_NUMERATOR/CYGNUM_HAL_RTC_DENOMINATOR)
#define TICKS_PER_SEC  (1000000000ULL / NS_PER_TICK)

// This needs to match newlib HZ which is normally 60.
#define HZ (60ULL)

#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
static unsigned int set_freq   = TICKS_PER_SEC; // The default
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
static int
sys_times(unsigned long *p)
{
    static int inited = 0;

    if (!inited) {
        inited = 1;
        sys_timer_init();
    }

    /* target clock runs at CLOCKS_PER_SEC. Convert to HZ */
    if (p)
#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
        *p = (sys_timer_ticks * HZ) / (cyg_uint64)set_freq;
#else
        *p = (sys_timer_ticks * HZ) / TICKS_PER_SEC;
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF

    return 0;
}

#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF

static void sys_profile_call_back( char *func, char **previous_call_back )
{
    if ( previous_call_back )
        *previous_call_back = (char *)timer_callback;

    timer_callback = (callback_func *)func;

    // Ensure the timer is started
    (void)sys_times( (unsigned long *)0 );
} 

static void sys_profile_frequency( int freq, int *previous_freq )
{
// Requested HZ:
// 0         => tell me the current value (no change, IMPLEMENTED HERE)
// - 1       => tell me the slowest (no change)
// - 2       => tell me the default (no change, IMPLEMENTED HERE)
// -nnn      => tell me what you would choose for nnn (no change)
// MIN_INT   => tell me the fastest (no change)
//        
// 1         => tell me the slowest (sets the clock)
// MAX_INT   => tell me the fastest (sets the clock)

    // Ensure the timer is started
    (void)sys_times( (unsigned long *)0 );

    if ( -2 == freq )
        freq = TICKS_PER_SEC; // default value
    else if ( 0 == freq )
        freq = set_freq; // collect current value
    else {
        int do_set_freq = (freq > 0);
        unsigned int period = CYGNUM_HAL_RTC_PERIOD;

        if ( 0 == (freq ^ -freq) ) // Then it's MIN_INT in local size
            freq++; // just so that it will negate correctly

        // Then set the timer to that fast - or pass on the enquiry
#ifdef HAL_CLOCK_REINITIALIZE
        // Give the HAL enough info to do the division sum relative to
        // the default setup, in period and TICKS_PER_SEC.
        HAL_CLOCK_REINITIALIZE( freq, period, TICKS_PER_SEC );
#else
        freq = TICKS_PER_SEC; // the only choice
#endif
        if ( do_set_freq ) { // update the global variables
            unsigned int orig = set_freq;
            set_freq = freq;
            set_period = period;
            // We must "correct" sys_timer_ticks for the new scale factor.
            sys_timer_ticks = sys_timer_ticks * set_freq / orig;
        }
    }

    if ( previous_freq ) // Return the current value (new value)
        *previous_freq = freq;
}

void sys_profile_reset( void )
{
    timer_callback = NULL;
// Want to preserve the frequency between runs, for clever GDB users!
//    sys_profile_frequency( TICKS_PER_SEC, NULL );
}

#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF

//
//  Generic syscall handler.
//
//  Returns 0 if syscall number is not handled by this
//  module, 1 otherwise. This allows applications to
//  extend the syscall handler by using exception chaining.
//
CYG_ADDRWORD
__do_syscall(CYG_ADDRWORD func,                 // syscall function number
             CYG_ADDRWORD arg1, CYG_ADDRWORD arg2,      // up to four args.
             CYG_ADDRWORD arg3, CYG_ADDRWORD arg4,
             CYG_ADDRWORD *retval, CYG_ADDRWORD *sig)   // syscall return value
{
    int err = 0;
    *sig = 0;

    switch (func) {

      case SYS_open:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_open( const char *name, int flags, 
                                                int mode, int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_open((const char *)arg1, (int)arg2, (int)arg3,
					   (int *)sig);
	  else
#endif
	      err = sys_open((const char *)arg1, (int)arg2, (int)arg3);
          break;
      }
      case SYS_read:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_read( int fd, void *buf, size_t count,
                                                int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_read((int)arg1, (void *)arg2, (size_t)arg3,
					   (int *)sig);
	  else
#endif
	      err = sys_read((int)arg1, (char *)arg2, (int)arg3);
          break;
      }
      case SYS_write:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_write( int fd, const void *buf,
                                                 size_t count, int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_write((int)arg1, (const void *)arg2,
					    (size_t)arg3, (int *)sig);
	  else
#endif
	      err = sys_write((int)arg1, (char *)arg2, (int)arg3);
          break;
      }
      case SYS_close:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_close( int fd, int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_close((int)arg1, (int *)sig);
	  else
#endif
	      err = sys_close((int)arg1);
          break;
      }
      case SYS_lseek:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_lseek( int fd, long offset,
                                                 int whence, int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_lseek((int)arg1, (long)arg2, (int)arg3,
					    (int *)sig);
	  else
#endif
	      err = sys_lseek((int)arg1, (int)arg2, (int)arg3);
          break;
      }
      case SYS_stat:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_stat( const char *pathname,
                                                void *statbuf, int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_stat((const char *)arg1, (void *)arg2,
					   (int *)sig);
	  else
#endif
	      err = -NEWLIB_ENOSYS;
          break;
      }
      case SYS_fstat:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_fstat( int fd, void *statbuf,
                                                 int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_fstat((int)arg1, (void *)arg2,
					    (int *)sig);
	  else
#endif
	  {
	      struct newlib_stat *st = (struct newlib_stat *)arg2;
	      st->st_mode = NEWLIB_S_IFCHR;
	      st->st_blksize = 4096;
	      err = 0;
	  }
          break;
      }
      case SYS_rename:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_rename( const char *oldpath,
                                                  const char *newpath,
                                                  int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_rename((const char *)arg1, (const char *)arg2,
					     (int *)sig);
	  else
#endif
	      err = -NEWLIB_ENOSYS;
          break;
      }
      case SYS_unlink:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_unlink( const char *pathname,
                                                  int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_unlink((const char *)arg1, (int *)sig);
	  else
#endif
	      err = -NEWLIB_ENOSYS;
          break;
      }
      case SYS_isatty:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_isatty( int fd, int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_isatty((int)arg1, (int *)sig);
	  else
#endif
	      err = 1;
          break;
      }
      case SYS_system:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_system( const char *command,
                                                  int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_system((const char *)arg1, (int *)sig);
	  else
#endif
	      err = -1;
          break;
      }
      case SYS_gettimeofday:
      {
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
          __externC int cyg_hal_gdbfileio_gettimeofday( void *tv, void *tz,
                                                        int *sig );
	  if (gdb_active)
	      err = cyg_hal_gdbfileio_gettimeofday((void *)arg1, (void *)arg2,
						   (int *)sig);
	  else
#endif
	      err = 0;
          break;
      }
      case SYS_utime:
        // FIXME. Some libglosses depend on this behavior.
        err = sys_times((unsigned long *)arg1);
        break;

      case SYS_times:
        err = sys_times((unsigned long *)arg1);
        break;

      case SYS_meminfo:
        err = 1;
        *(unsigned long *)arg1 = (unsigned long)(ram_end-ram_start);
        *(unsigned long *)arg2 = (unsigned long)ram_end;
        break;
#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    case SYS_timer_call_back:
        sys_profile_call_back( (char *)arg1, (char **)arg2 );
        break;

    case SYS_timer_frequency:
        sys_profile_frequency( (int)arg1, (unsigned int *)arg2 );
        break;

    case SYS_timer_reset:
        sys_profile_reset();
        break;

#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
      case __GET_SHARED:
        *(__shared_t **)arg1 = &__shared_data;
        break;

      case SYS_exit:
	*sig = -1;    // signal exit
	err = arg1;

	if (gdb_active) {
#ifdef CYGOPT_REDBOOT_BSP_SYSCALLS_EXIT_WITHOUT_TRAP
	    __send_exit_status((int)arg1);
#else
	    *sig = SIGTRAP;
	    err = func;
#endif // CYGOPT_REDBOOT_BSP_SYSCALLS_EXIT_WITHOUT_TRAP
	}
	break;

      default:
        return 0;
    }    

    *retval = err;
    return 1;
}

#endif
