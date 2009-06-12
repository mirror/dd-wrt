//========================================================================
//
//      mn10300_stub.c
//
//      Helper functions for mn10300 stub
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Red Hat, jskov
// Contributors:  Red Hat, jskov, dmoseley
// Date:          1998-11-06
// Purpose:       
// Description:   Helper functions for mn10300 stub
// Usage:         
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

/*----------------------------------------------------------------------
 * Asynchronous interrupt support
 */

typedef unsigned char t_inst;

static struct
{
  t_inst *targetAddr;
  t_inst savedInstr;
} asyncBuffer;

/* Called to asynchronously interrupt a running program.
   Must be passed address of instruction interrupted.
   This is typically called in response to a debug port
   receive interrupt.
*/

void
install_async_breakpoint(void *pc)
{
  asyncBuffer.targetAddr = pc;
  asyncBuffer.savedInstr = *(t_inst *)pc;
  *(t_inst *)pc = (t_inst)HAL_BREAKINST;
  __instruction_cache(CACHE_FLUSH);
  __data_cache(CACHE_FLUSH);
}

/*--------------------------------------------------------------------*/
/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    if (asyncBuffer.targetAddr != NULL)
    {
        /* BP installed by serial driver to stop running program */
        *asyncBuffer.targetAddr = asyncBuffer.savedInstr;
        __instruction_cache(CACHE_FLUSH);
        __data_cache(CACHE_FLUSH);
        asyncBuffer.targetAddr = NULL;
        return SIGINT;
    }
#ifdef SIGSYSCALL
    switch (trap_number)
      {
      case SIGSYSCALL:
        /* System call */
        return SIGSYSCALL;
      }
#endif
    return SIGTRAP;
}

/*--------------------------------------------------------------------*/
/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return _hal_registers->vector;
}

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
int __is_bsp_syscall(void) 
{
    return __get_trap_number() == SIGSYS;
}
#endif

/*--------------------------------------------------------------------*/
/* Set the currently-saved pc register value to PC. This also updates NPC
   as needed. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}


/*----------------------------------------------------------------------
 * Single-step support. Lifted from CygMon.
 */

#define NUM_SS_BPTS 2
static target_register_t break_mem [NUM_SS_BPTS] = {0, 0};
static unsigned char break_mem_data [NUM_SS_BPTS];

/* Set a single-step breakpoint at ADDR.  Up to two such breakpoints
   can be set; WHICH specifies which one to set (0 or 1).  */

static void
set_single_bp (int which, unsigned char *addr)
{
    if (0 == break_mem[which]) {
        break_mem[which] = (target_register_t) addr;
        break_mem_data[which] = *addr;
        *addr = HAL_BREAKINST;
    }
}

/* Clear any single-step breakpoint(s) that may have been set.  */

void __clear_single_step (void)
{
  int x;
  for (x = 0; x < NUM_SS_BPTS; x++)
    {
        unsigned char* addr = (unsigned char*) break_mem[x];
        if (addr) {
            *addr = break_mem_data[x];
            break_mem[x] = 0;
        }
    }
}

/* Read a 16-bit displacement from address 'p'. The
   value is stored little-endian.  */

static short
read_disp16(unsigned char *p)
{
    return (short)(p[0] | (p[1] << 8));
}

/* Read a 32-bit displacement from address 'p'. The
   value is stored little-endian.  */

static int
read_disp32(unsigned char *p)
{
    return (int)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}


/* Get the contents of An register.  */

static unsigned int
get_areg (int n)
{
  switch (n)
    {
    case 0:
      return get_register (A0);
    case 1:
      return get_register (A1);
    case 2:
      return get_register (A2);
    case 3:
      return get_register (A3);
    }
  return 0;
}


/* Table of instruction sizes, indexed by first byte of instruction,
   used to determine the address of the next instruction for single stepping.
   If an entry is zero, special code must handle the case (for example,
   branches or multi-byte opcodes).  */

static char opcode_size[256] =
{
     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f */
     /*------------------------------------------------*/
/* 0 */ 1, 3, 3, 3, 1, 3, 3, 3, 1, 3, 3, 3, 1, 3, 3, 3,
/* 1 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 2 */ 2, 2, 2, 2, 3, 3, 3, 3, 2, 2, 2, 2, 3, 3, 3, 3,
/* 3 */ 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 1, 1,
/* 4 */ 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2,
/* 5 */ 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
/* 6 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 7 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* 8 */ 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2,
/* 9 */ 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2,
/* a */ 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2,
/* b */ 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2,
/* c */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 2,
/* d */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* e */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
/* f */ 0, 2, 2, 2, 2, 2, 2, 1, 0, 3, 0, 4, 0, 6, 7, 1
};

/* Set breakpoint(s) to simulate a single step from the current PC.  */

void __single_step (void)
{
  unsigned char *pc = (unsigned char *) get_register (PC);
  unsigned int opcode;
  int          displ;

  opcode = *pc;

  /* Check the table for the simple cases.  */
  displ = opcode_size[opcode];
  if (displ != 0)
    {
      set_single_bp (0, pc + displ);
      return;
    }

  /* Handle the more complicated cases.  */
  switch (opcode)
    {
    case 0xc0:
    case 0xc1:
    case 0xc2:
    case 0xc3:
    case 0xc4:
    case 0xc5:
    case 0xc6:
    case 0xc7:
    case 0xc8:
    case 0xc9:
    case 0xca:
      /*
       *  bxx (d8,PC)
       */
      displ = *((signed char *)pc + 1);
      set_single_bp (0, pc + 2);
      if (displ < 0 || displ > 2)
        set_single_bp (1, pc + displ);
      break;

    case 0xd0:
    case 0xd1:
    case 0xd2:
    case 0xd3:
    case 0xd4:
    case 0xd5:
    case 0xd6:
    case 0xd7:
    case 0xd8:
    case 0xd9:
    case 0xda:
      /*
       *  lxx (d8,PC)
       */
      if (pc != (unsigned char*) get_register (LAR))
        set_single_bp (0, (unsigned char *) get_register (LAR));
      set_single_bp (1, pc + 1);
      break;

    case 0xdb:
      /*
       * setlb requires special attention. It loads the next four instruction
       * bytes into the LIR register, so we can't insert a breakpoint in any
       * of those locations.
       */
      set_single_bp (0, pc + 5);
      break;

    case 0xcc:
    case 0xcd:
      /*
       * jmp (d16,PC) or call (d16,PC)
       */
      displ = read_disp16((char *)(pc + 1));
      set_single_bp (0, pc + displ);
      break;

    case 0xdc:
    case 0xdd:
      /*
       * jmp (d32,PC) or call (d32,PC)
       */
      displ = read_disp32((char *)(pc + 1));
      set_single_bp (0, pc + displ);
      break;

    case 0xde:
      /*
       *  retf
       */
      set_single_bp (0, (unsigned char *) get_register (MDR));
      break;

    case 0xdf:
      /*
       *  ret
       */
      displ = *((char *)pc + 2);
      set_single_bp (0, (unsigned char *) read_disp32 ((unsigned char *)
                     get_register (SP) + displ));
      break;

    case 0xf0:
      /*
       *  Some branching 2-byte instructions.
       */
      opcode = *(pc + 1);
      if (opcode >= 0xf0 && opcode <= 0xf7)
        {
          /* jmp (An) / calls (An) */
          set_single_bp (0, (unsigned char *) get_areg (opcode & 3));

        }
      else if (opcode == 0xfc)
        {
          /* rets */
          set_single_bp (0, (unsigned char *) read_disp32 ((unsigned char *)
                         get_register (SP)));
    
        }
      else if (opcode == 0xfd)
        {
          /* rti */
          set_single_bp (0, (unsigned char *) read_disp32 ((unsigned char *)
                         get_register (SP) + 4));

        }
      else 
        set_single_bp (0, pc + 2);

      break;

    case 0xf8:
      /*
       *  Some branching 3-byte instructions.
       */
      opcode = *(pc + 1);
      if (opcode >= 0xe8 && opcode <= 0xeb)
        {
          displ = *((signed char *)pc + 2);
          set_single_bp (0, pc + 3);
          if (displ < 0 || displ > 3)
              set_single_bp (1, pc + displ);
    
        }
      else
        set_single_bp (0, pc + 3);
      break;

    case 0xfa:
      opcode = *(pc + 1);
      if (opcode == 0xff)
        {
          /* calls (d16,PC) */
          displ = read_disp16((char *)(pc + 2));
          set_single_bp (0, pc + displ);
        }
      else
        set_single_bp (0, pc + 4);
      break;

    case 0xfc:
      opcode = *(pc + 1);
      if (opcode == 0xff)
        {
          /* calls (d32,PC) */
          displ = read_disp32((char *)(pc + 2));
          set_single_bp (0, pc + displ);
        }
      else
        set_single_bp (0, pc + 6);
      break;

  }
}

void __install_breakpoints (void)
{
    /* NOP since single-step HW exceptions are used instead of
       breakpoints. */

  /* Install the breakpoints in the breakpoint list */
  __install_breakpoint_list();
}

void __clear_breakpoints (void)
{
  __clear_breakpoint_list();
}


/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */

int
__is_breakpoint_function ()
{
    return get_register (PC) == (target_register_t)&CYG_LABEL_NAME(_breakinst);
}


/* Skip the current instruction. */

void __skipinst (void)
{
    unsigned char *pc = (char *) get_register (PC);

    switch (*pc)
    {
    case 0xff:                          // breakpoint instruction
        pc++;
        break;
    case 0xf0:                          // Assume syscall trap (0xf0, 0x20)
        pc += 2;
        break;
    default:
        pc++;                           // Assume all other instructions 
        break;                          // are one byte
    }

  put_register (PC, (target_register_t) pc);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
