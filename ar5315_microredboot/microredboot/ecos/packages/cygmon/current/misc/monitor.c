//==========================================================================
//
//      monitor.c
//
//      Monitor shell and main routines for CygMON the Wonder Monitor
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
// Contributors: gthomas, dmoseley
// Date:         1999-10-20
// Purpose:      Monitor shell and main routines for CygMON the Wonder Monitor
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================

/* Platform-independent code for cygmon */

#include <stdlib.h>
#include <unistd.h>
#ifdef HAVE_BSP
#include <bsp/bsp.h>
#include <bsp/cpu.h>
#include <bsp/hex-utils.h>
#endif
#include <monitor.h>
#ifdef HAVE_BSP
#include "cpu_info.h"
#endif
#include <ledit.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#if USE_CYGMON_PROTOTYPES
/* Use common prototypes */
/* Some of the composed board.h files compose these
   prototypes redundently, but if they dont,
   these are the common definitions */
#include "fmt_util.h"   /* Interface to string formatting utilities */
#include "tservice.h"   /* Interface to target specific services */
#include "generic-stub.h" /* from libstub */
#endif /* USE_CYGMON_PROTOTYPES */

static int  cygmon_handle_exception (int sigval);
static void monitor_take_control (void);

#if CYGMON_SYSTEM_SERVICES
extern int process_syscall (int syscall_num);
#elif defined (USE_ECOS_HAL_EXCEPTIONS)
static int cygmon_process_syscall (int sigval);
#endif

int stub_is_active = 0;
mem_addr_t last_pc;

#ifdef HAVE_BSP

#if !defined(USE_ECOS_HAL_EXCEPTIONS)
static int mon_dbg_handler(int exc_nr, void *regs);
static int mon_kill_handler(int exc_nr, void *regs);
#endif // USE_ECOS_HAL_EXCEPTIONS

#ifndef USE_ECOS_HAL_BREAKPOINTS
#define __is_breakpoint_function() (get_pc() == (target_register_t)bsp_breakinsn)
#endif

#ifdef __ECOS__
/*
 * This global flag is used by generic-stub.c to communicate to us that we
 * are processing the breakpoint function within Cygmon itself.
 */
extern int processing_breakpoint_function;
#endif

/* Global pointer to current set of saved registers. */
void *mon_saved_regs;

/* Original BSP debug vector replaced by monitor. */
#if !defined(USE_ECOS_HAL_EXCEPTIONS)
static bsp_handler_t old_dbg_vec;
#endif // !defined(USE_ECOS_HAL_EXCEPTIONS)

#else
static int handle_signal (int signal_val);

__PFI user_signal_handler = NULL;
#endif

#if defined(__ECOS__)
#include <cyg/hal/hal_stub.h>
#endif

#if defined(__ECOS__) && !defined(PROCESS_EXCEPTION_VEC_PROTOTYPE_EXISTS)
#define PROCESS_EXCEPTION_VEC_PROTOTYPE_EXISTS
extern volatile __PFI (*__process_exception_vec)(int);
#endif


#ifdef MONITOR_CONTROL_INTERRUPTS
/* This is set if the user wants interrupts enabled in the monitor. */
static int monitor_interrupts_enabled;
#endif


#if NOMAIN
int monitor_main (int argc, char **argv) /* Suppress default main() junk */
#else
int main (int argc, char **argv)     
#endif
{
#ifdef HAVE_BSP
  int  cur_port;
  struct bsp_comm_info comm_info;
#else
  /* Set up exception handling traps */
  initialize_stub ();
#endif

  initialize_mon ();

#ifdef HAVE_BSP
  /* get info on debug channel */
  cur_port = bsp_set_debug_comm(-1);
  bsp_sysinfo(BSP_INFO_COMM, cur_port, &comm_info);

  /*
   * If we're using a network interface, don't install
   * the cygmon vectors. Currently, we only support stub
   * mode over a network connection.
   */
  if (comm_info.kind != BSP_COMM_ENET)
    {
      xprintf("\n");
      version ();

      /* replace original BSP debug and kill handler with ours */
#if !defined(USE_ECOS_HAL_EXCEPTIONS)
      old_dbg_vec = bsp_install_dbg_handler(mon_dbg_handler);
      (void)bsp_install_kill_handler(mon_kill_handler);
#else
      /* replace original BSP debug and kill handler with ours using eCos stuff */
      __process_exception_vec = (__PFI)cygmon_handle_exception;
      __process_syscall_vec = cygmon_process_syscall;
      __process_exit_vec = monitor_take_control;
#endif // __ECOS__
    }
  else
    {
      /* This forces the console to use the gdb channel. */
      bsp_set_console_comm(cur_port);
    }
#else
  xprintf("\n");
  version ();

  __process_exception_vec = cygmon_handle_exception;
  __process_exit_vec = monitor_take_control;
#if CYGMON_SYSTEM_SERVICES
  __process_syscall_vec = process_syscall;
#endif  
  __process_signal_vec = handle_signal;
  __init_vec = install_breakpoints;
  __cleanup_vec = clear_breakpoints;
#endif

#if 0
#ifdef __ECOS__
  __process_exception_vec = cygmon_handle_exception;
#endif
#endif

  while (1)
    {
      breakpoint ();
      if (switch_to_stub_flag)
        {
	  xprintf("Switching to stub\n");
	  switch_to_stub_flag = 0;
	}
    }

  /* never reached */
  exit (0);
}


/* Transfer control to gdb stub */
 
int 
transfer_to_stub ()
{
  /* Return back to the exception handler, but the exception handler
     should invoke the stub's exception handler instead of ours. */
#if defined(HAVE_BSP) && !defined(USE_ECOS_HAL_EXCEPTIONS)
  (void)bsp_install_dbg_handler(old_dbg_vec);
#else
  __switch_to_stub ();
#endif

  /* The stub is now active. */
  stub_is_active = 1;
  return -1;
}

void
clear_user_state (void)
{
#ifdef HAS_TIMER
  if (__timer_enabled ())
    __settimer (0, 0);
#endif

  clear_breakpoints ();

#ifndef HAVE_BSP
  user_signal_handler = NULL;
#endif

  __clear_single_step ();
}

static void
monitor_take_control (void)
{
  stub_is_active = 0;
  switch_to_stub_flag = 0;

  // Flush the unget state.  This is because the ecos stub and Cygmon track this
  // stuff separately.
  bsp_debug_ungetc('\0');

#ifdef INITIALIZE_MON_EACH_TIME
  // Call the per-stop initialization routine if it is defined.
  INITIALIZE_MON_EACH_TIME();
#endif

#if defined(HAVE_BSP) && !defined(USE_ECOS_HAL_EXCEPTIONS)
  /* replace original BSP debug trap handler with ours */
  (void)bsp_install_dbg_handler(mon_dbg_handler);
#else
  clear_user_state ();
  __process_exception_vec = cygmon_handle_exception;
#endif
}


#ifdef MONITOR_CONTROL_INTERRUPTS
void
monitor_enable_interrupts (void)
{
  monitor_interrupts_enabled = 1;
  enable_interrupts ();
}

void
monitor_disable_interrupts (void)
{
  monitor_interrupts_enabled = 0;
  disable_interrupts ();
}

int
monitor_interrupt_state (void)
{
  return monitor_interrupts_enabled;
}
#endif

#if defined(USE_ECOS_HAL_EXCEPTIONS)
externC HAL_SavedRegisters *_hal_registers;
extern int machine_syscall(HAL_SavedRegisters *regs);
static int
cygmon_process_syscall (int sigval)
{
    return machine_syscall(_hal_registers);
}
#endif

static int
cygmon_handle_exception (int sigval)
{
  target_register_t pc;

#ifdef MONITOR_CONTROL_INTERRUPTS
  if (monitor_interrupts_enabled)
    {
      if (! __in_interrupt)
	enable_interrupts ();
    }
#endif

#ifdef TARGET_EXCEPTION_CODE
  TARGET_EXCEPTION_CODE
#endif

#ifndef HAVE_BSP
  if (sigval != SIGKILL)
    if (handle_signal (sigval) == 0)
      return 0;
#endif

  clear_user_state ();

  /* We may want to tweak the PC to point at the faulting instruction,
     for example. (breakpoints on x86). */
#ifdef TARGET_ADJUST_PC
  TARGET_ADJUST_PC
#endif

  pc = get_pc();
  MAKE_STD_ADDR (pc, &last_pc);

#ifdef __ECOS__
  if ((sigval == SIGTRAP) && (__is_breakpoint_function() || processing_breakpoint_function))
#else
  if ((sigval == SIGTRAP) && __is_breakpoint_function())
#endif
    {
      /*
       * This is the initial breakpoint inserted by the BSP
       * Don't print anything for this as it is confusing
       */
    }
  else
    {
      if (sigval == SIGTRAP)
          xprintf ("Hit breakpoint");
      else
          xprintf ("Got signal %d", sigval);

      xprintf (" at 0x%s\n", int2str (pc, 16, sizeof (target_register_t) * 2));

#ifdef DISASSEMBLER
      if (!__is_breakpoint_function ())
	{
	  do_dis (&last_pc);
	  flush_dis ();
	}
#endif
    }

  monitor_take_control ();

  return monitor_loop ();
}

#ifndef HAVE_BSP
/* Returns 0 if the program should restart at the point at which the
   signal was received, -1 otherwise. */
static int
handle_signal (int signal)
{
  if (signal == SIGKILL)
    return -1;

  if (user_signal_handler != NULL)
    {
      int result = user_signal_handler (signal);
      switch (result)
	{
	  /* Don't ignore potential hardware signals. */
	case 3:
	  if (signal == SIGSEGV || signal == SIGBUS || signal == SIGFPE
	      || signal == SIGTRAP || signal == SIGILL)
	    return -1;

	case 0:
	  return 0;

	default:
	case 1:
	case 2:
	  return -1;
	}
    }
    return -1;
}
#endif


void
version (void)
{
#ifdef HAVE_BSP
  struct bsp_platform_info platform;
  struct bsp_mem_info      mem;
  int                      i;
  unsigned long            u, totmem, topmem;
#endif
  extern char *build_date;

  xprintf ("Cygmon, the Cygnus ROM monitor.\n");
  xprintf ("Copyright(c) 1997, 1998, 1999, 2000 Red Hat\n\n");
  xprintf ("Version: %s\nThis image was built on %s\n\n",
	   VERSION, build_date);

#ifdef HAVE_BSP
  bsp_sysinfo(BSP_INFO_PLATFORM, &platform);

  totmem = topmem = 0;
  i = 0;
  while (bsp_sysinfo(BSP_INFO_MEMORY, i++, &mem) == 0)
    {
      if (mem.kind == BSP_MEM_RAM)
        {
          totmem += mem.nbytes;
          u = (unsigned long)mem.virt_start + mem.nbytes;
	  if (u > topmem)
	    topmem = u;
        }
    }

  xprintf("CPU: %s\n", platform.cpu);
  xprintf("Board: %s\n", platform.board);
  if (*(platform.extra))
      xprintf("%s\n", platform.extra);
  xprintf("Total RAM: %d bytes\n", totmem);
  xprintf("Top of RAM: 0x%x\n", topmem);
#endif
}


#ifdef HAVE_BSP
#if !defined(USE_ECOS_HAL_EXCEPTIONS)
static int
mon_kill_handler(int exc_nr, void *regs)
{
    monitor_take_control();
    return 1;
}
#endif // !defined(USE_ECOS_HAL_EXCEPTIONS)

#if !defined(USE_ECOS_HAL_EXCEPTIONS)
static int
mon_dbg_handler(int exc_nr, void *regs)
{
    int sig;
    unsigned long cur_pc;

    mon_saved_regs = regs;
    sig = bsp_get_signal(exc_nr, regs);

    cygmon_handle_exception(sig);

    cur_pc = bsp_get_pc(regs);
    if (cur_pc == (unsigned long)bsp_breakinsn)
	bsp_skip_instruction(regs);

    if (!stub_is_active)
	install_breakpoints();

    return 1;
}
#endif // !defined(USE_ECOS_HAL_EXCEPTIONS)

#endif
