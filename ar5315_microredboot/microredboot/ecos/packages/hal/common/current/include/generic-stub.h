//========================================================================
//
//      generic-stub.h
//
//      Definitions for the generic GDB remote stub
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
// Author(s):     Red Hat, nickg
// Contributors:  Red Hat, nickg, dmoseley
// Date:          1998-06-08
// Purpose:       
// Description:   Definitions for the generic GDB remote stub
// Usage:         This header is not to be included by user code, and is
//                only placed in a publically accessible directory so
//                that platform stub packages are able to include it
//                if required.
//
//####DESCRIPTIONEND####
//
//========================================================================


#ifndef GENERIC_STUB_H
#define GENERIC_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Three typedefs must be provided before including this file.
   target_register_t should correspond to the largest register value on
   this target processor.
   regnames_t corresponds to a type representing a register number. 
   uint32 must correspond to at least a 32-bit unsigned integer type
   (it may be larger). */

#ifndef ASM

#ifndef __ECOS__
#include "dbg-serial.h"
#endif // __ECOS__

struct gdb_packet
{
  /* The checksum calculated so far. */
  uint32 checksum;
  /* The checksum we've received from the remote side. */
  uint32 xmitcsum;
  /* Contents of the accumulated packet. */
  char *contents;
  /* Number of characters received. */
  uint32 length;
  /*
   * state is the current state:
   *  0 = looking for start of packet
   *  1 = got start of packet, looking for # terminator
   *  2 = looking for first byte of checksum
   *  3 = looking for second byte of checksum (indicating end of packet)
   */
  char state;
  char err;  // This is set if the packet should be tossed because of overflow
};

/* Return the currently-saved value corresponding to register REG. */
extern target_register_t get_register (regnames_t reg);

/* Write the contents of register WHICH into VALUE as raw bytes */
extern int get_register_as_bytes (regnames_t which, char *value);

#ifdef CYGHWR_REGISTER_VALIDITY_CHECKING
// Return the validity of register REG.
extern int get_register_valid (regnames_t reg);
#endif

/* Store VALUE in the register corresponding to WHICH. */
extern void put_register (regnames_t which, target_register_t value);

extern int put_register_as_bytes (regnames_t which, char *value);

/* Set the next instruction to be executed when the user program resumes
   execution to PC. */
#if !defined(SET_PC_PROTOTYPE_EXISTS) && !defined(set_pc)
#define SET_PC_PROTOTYPE_EXISTS
extern void set_pc (target_register_t pc);
#endif

/* Set the return value of the currently-pending syscall to VALUE. */
extern void set_syscall_value (target_register_t value);

/* Return the Nth argument to the currently-pending syscall (starting from
   0). */
extern target_register_t get_syscall_arg (int n);

/* Initialize the stub. This will also install any trap handlers needed by
   the stub. */
extern void initialize_stub (void);

/* Initialize the hardware. */
extern void initHardware (void);

/* Skip the current instruction. */
extern void __skipinst (void);

/* If the address in the PC register corresponds to the breakpoint()
   instruction, return a non-zero value. */
#ifndef __is_breakpoint_function
extern int __is_breakpoint_function (void);
#endif

/* Execute a breakpoint instruction. Restarting will cause the instruction
   to be skipped. */
#ifndef breakpoint
extern void breakpoint (void);
#endif

/* Return the syscall # corresponding to this system call. */
extern int __get_syscall_num (void);

/* Transfer exception event processing to the stub. */
extern void __switch_to_stub (void);

/* Send an exit packet containing the specified status. */
extern void __send_exit_status (int status);

/* Copy COUNT bytes of memory from ADDR to BUF.  
   ADDR is assumed to live in the user program's space. 
   Returns number of bytes successfully read 
   (caller must check to see if less than requested).  */
extern int __read_mem_safe (void *buf, 
                            void *addr, 
                            int count);

extern int __read_progmem_safe (void *buf, 
				void *addr, 
				int count);

/* Copy COUNT bytes of memory from BUF to ADDR. 
   ADDR is assumed to live in the user program's space. 
   Returns number of bytes successfully read 
   (caller must check to see if less than requested).  */
#ifndef __write_mem_safe
extern int __write_mem_safe (void *buf, 
                             void *addr, 
                             int count);
#endif

extern int __write_progmem_safe (void *buf, 
				 void *addr, 
				 int count);

/* Set to a non-zero value if a memory fault occurs while 
   __set_mem_fault_trap () is running. */
extern volatile int __mem_fault;

#ifndef __ECOS__
#if 1
#include "stub-tservice.h"  /* target dependent stub services */
#else
/* Flush the instruction cache. */
extern void flush_i_cache (void);

/* Flush the data cache. */
extern void __flush_d_cache (void);

typedef enum {
  CACHE_NOOP, CACHE_ENABLE, CACHE_DISABLE, CACHE_FLUSH
} cache_control_t;

/* Perform the specified operation on the instruction cache. 
   Returns 1 if the cache is enabled, 0 otherwise. */
extern int __instruction_cache (cache_control_t request);
/* Perform the specified operation on the data cache. 
   Returns 1 if the cache is enabled, 0 otherwise. */
extern int __data_cache (cache_control_t request);
#endif
#endif // __ECOS__

/* Write the 'T' packet in BUFFER. SIGVAL is the signal the program
   received. */
extern void __build_t_packet (int sigval, char *buffer);

/* Return 1 when a complete packet has been received, 0 if the packet
   is not yet complete, or -1 if an erroneous packet was NAKed. */
int __add_char_to_packet (unsigned character, struct gdb_packet *packet);

typedef int (*__PFI)(int);
typedef void (*__PFV)(void);

/* When an exception occurs, __process_exception_vec will be invoked with
   the signal number corresponding to the trap/exception. The function
   should return zero if it wishes execution to resume from the saved
   register values; a non-zero value indicates that the exception handler
   should be reinvoked. */
#if !defined(PROCESS_EXCEPTION_VEC_PROTOTYPE_EXISTS)
#define PROCESS_EXCEPTION_VEC_PROTOTYPE_EXISTS
extern volatile __PFI __process_exception_vec;
#endif

/* __process_exit_vec is invoked when a 'k' kill packet is received
   from GDB. */
extern volatile __PFV __process_exit_vec;

/* If SIGSYSCALL is defined, and such a signal value is returned from 
   __computeSignal (), the function pointed to by this vector will
   be invoked.

   If the return value is negative, the user program is assumed to
   have received the corresponding positive signal value, and an
   exception will be processed.  Otherwise, the user program is
   restarted from the next instruction. */
extern volatile __PFI __process_syscall_vec;

/* A continue packet was received from GDB with a signal value. The function
   pointed to by __process_signal_vec will be invoked with this signal
   value. 

   If a zero value is returned, we will ignore the signal, and proceed
   with the continue request. Otherwise, the program will be killed
   with the signal. */
extern volatile __PFI __process_signal_vec;

/* If non-NULL, __init_vec is called right before the user program is
   resumed. */
extern volatile __PFV __init_vec;
/* if non-NULL, __cleanup_vec is called after the user program takes
   an exception. */
extern volatile __PFV __cleanup_vec;

/* Send an 'O' packet to GDB containing STR. */
extern int __output_gdb_string (target_register_t addr, int string_len);

/* Request MAXLEN bytes of input from GDB to be stored in DEST. If BLOCK
   is set, GDB should block until MAXLEN bytes are available to be
   read; otherwise, it will return immediately with whatever data is
   available. 
   The return value is the number of bytes written into DEST. */
extern int __get_gdb_input (target_register_t dest, int maxlen, int block);

/* Return the ASCII equivalent of C (C>=0 && C<=15). The result will be 
   lower-case. */
extern char __tohex (int c);

/* Convert COUNT bytes of the memory region in MEM to a hexadecimal
   string in DEST.
   The resulting string will contain 2*COUNT characters.
   If MAY_FAULT is non-zero, memory faults are trapped; if a fault occurs,
   a NULL value will be returned.
   The value returned is one byte past the end of the string written. */
extern char *__mem2hex (char *mem, char *dest, int count, int may_fault);

/* Given a hexadecimal string in MEM, write the equivalent bytes to DEST.
   The string is assumed to contain 2*COUNT characters.
   If MAY_FAULT is non-zero, memory faults are trapped; if a fault occurs,
   a NULL value will be returned.
   Otherwise, the value returned is one byte past the last byte written. */
extern char *__hex2mem (char *buf, char *mem, int count, int may_fault);

#ifdef CYGSEM_ECOS_SUPPORTS_PROGRAM_ARGS
/* Set the program arguments passed into the user program's main */
extern void __set_program_args (int argc, char **argv);

/* Return the user program arguments passed in from GDB (via an 'A'
   packet). argcPtr is a pointer into the user program, which will hold
   the number of arguments; the strings are returned. */
extern char **__get_program_args (target_register_t argcPtr);
#endif

/* Encode PACKET as a remote protocol packet and send it to GDB; this takes
   care of sending the initial '$' character, as well as the trailing '#'
   and checksum, and also waits for an ACK from the remote side, resending
   as necessary. */
extern void __putpacket (char *packet);

/* Retrieve the next remote protocol packet from GDB, taking care of verifying
   the checksum and sending an ACK when necessary. */
extern void __getpacket (char *buffer);

/* Convert the hexadecimal string pointed to by *PTR into an integer,
   and store it in the value pointed to by INTVALUE. The number of
   characters read from *PTR will be returned; *PTR will point to the first
   non-hexadecmial character encountered. */
extern unsigned int __hexToInt (char **ptr, target_register_t *intValue);

/* Convert the value in INTVALUE into a string of hexadecimal
   characters stored in PTR. NUMBITS are the number of bits to use
   in INTVALUE. The number of characters written to PTR will be returned. */
extern unsigned int __intToHex (char *ptr, 
                                target_register_t intValue, 
                                int numBits);

/* Handle an exception, usually some sort of hardware or software trap.
   This is responsible for communicating the exception to GDB. */
extern void __handle_exception (void);

/* Send a 'X' packet with signal SIGVAL to GDB. */
extern void __kill_program (int sigval);

/* Given a packet pointed to by PACKETCONTENTS, decode it and respond to
   GDB appropriately. */
extern int __process_packet (char *packetContents);

/* Write the C-style string pointed to by STR to the GDB comm port.
   Used for printing debug messages. */
extern void __putDebugStr (char *str);

#if defined(NO_MALLOC) && !defined(MAX_BP_NUM)
#define MAX_BP_NUM 64 /* Maximum allowed # of breakpoints */
#endif

extern int hal_syscall_handler(void);
extern int __is_bsp_syscall(void);

extern void __install_breakpoint_list (void);
extern void __clear_breakpoint_list (void);
extern int __display_breakpoint_list (void (*print_func)(target_register_t));

/* 'Z' packet types */
#define ZTYPE_SW_BREAKPOINT        0
#define ZTYPE_HW_BREAKPOINT        1
#define ZTYPE_HW_WATCHPOINT_WRITE  2
#define ZTYPE_HW_WATCHPOINT_READ   3
#define ZTYPE_HW_WATCHPOINT_ACCESS 4

#endif /* ASM */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* GENERIC_STUB_H */
