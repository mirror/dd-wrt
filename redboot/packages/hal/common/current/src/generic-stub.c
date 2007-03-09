#include "board.h"

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

/* Eventually, this should default to ON */
#if USE_GDBSTUB_PROTOTYPES
#include "stub-tservice.h"
#include "generic-stub.h"
#else
// Function declarations (prevents compiler warnings)
int stubhex (unsigned char ch);
static void unlock_thread_scheduler (void);
static uint32 crc32 (target_addr_t mem, int len, uint32 crc);
#endif

#include "thread-pkts.h"
  /* Defines function macros if thread support is not selected in board.h */

#ifdef __ECOS__
char GDB_stubs_version[] CYGBLD_ATTRIB_WEAK = 
    "eCos GDB stubs - built " __DATE__ " / " __TIME__;
#endif

/****************************************************************************

                THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or it's performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Header: remcom.c,v 1.34 91/03/09 12:29:49 glenne Exp $
 *
 *  Module name: remcom.c $
 *  Revision: 1.34 $
 *  Date: 91/03/09 12:29:49 $
 *  Contributor:     Lake Stevens Instrument Division$
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works on target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *  Modified for SPARC by Stu Grossman, Red Hat.
 *  Modified for generic CygMON stub support by Bob Manson, Red Hat.
 *
 *  To enable debugger support, two things need to happen.  One, a
 *  call to set_debug_traps () is necessary in order to allow any breakpoints
 *  or error conditions to be properly intercepted and reported to gdb.
 *  Two, a breakpoint needs to be generated to begin communication.  This
 *  is most easily accomplished by a call to breakpoint ().  Breakpoint ()
 *  simulates a breakpoint by executing a trap #1.
 *
 *************
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 *    bBB..BB       Set baud rate to BB..BB                OK or BNN, then sets
 *                                                         baud rate
 *
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: < two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/

#ifdef __ECOS__

// We cannot share memcpy and memset with the rest of the system since
// the user may want to step through it.
static inline void*
_memcpy(void* dest, void* src, int size)
{
    unsigned char* __d = (unsigned char*) dest;
    unsigned char* __s = (unsigned char*) src;
    
    while(size--)
        *__d++ = *__s++;

    return dest;
}

static inline void*
_memset(void* s, int c, int size)
{
    unsigned char* __s = (unsigned char*) s;
    unsigned char __c = (unsigned char) c;
    
    while(size--)
        *__s++ = __c;

    return s;
}

#else
#include <string.h>
#include <signal.h>
#define _memcpy memcpy
#define _memset memset
#endif // __ECOS__

/************************************************************************/
/* BUFMAX defines the maximum number of characters in inbound/outbound buffers*/
/* at least NUMREGBYTES*2 are needed for register packets */
#ifdef __ECOS__
#ifdef NUMREGBYTES
#define BUFMAX (32 + (NUMREGBYTES*2))
#else
#define BUFMAX 2048
#endif
#else
#define BUFMAX 2048
#endif

static int initialized = 0;     /* !0 means we've been initialized */

static int process_exception (int sigval);
static void do_nothing (void); /* and do it gracefully */
static int syscall_do_nothing (int);

#ifdef CYGSEM_ECOS_SUPPORTS_PROGRAM_ARGS
void __free_program_args (void);
static char *__add_program_arg (int argnum, uint32 arglen);
#endif

volatile __PFI __process_exception_vec = process_exception;
volatile __PFV __process_exit_vec = do_nothing;
volatile __PFI __process_syscall_vec = syscall_do_nothing;
volatile __PFI __process_signal_vec = NULL;
volatile __PFV __init_vec = NULL;
volatile __PFV __cleanup_vec = NULL;

static const char hexchars[] = "0123456789abcdef";

static void process_query (char *pkt);
static void process_set   (char *pkt);

char
__tohex (int c)
{
  return hexchars [c & 15];
}

#define __tohex(c) hexchars[(c) & 15]

#ifndef NUMREGS_GDB
#define NUMREGS_GDB NUMREGS
#endif

/* One pushback character. */
int ungot_char = -1;

static int
readDebugChar (void)
{
  if (ungot_char > 0)
    {
      int result = ungot_char;
      ungot_char = -1;
      return result;
    }
  else
    return getDebugChar ();
}

/* Convert ch from a hex digit to an int. */

int
stubhex (ch)
     unsigned char ch;
{
  if (ch >= 'a' && ch <= 'f')
    return ch-'a'+10;
  if (ch >= '0' && ch <= '9')
    return ch-'0';
  if (ch >= 'A' && ch <= 'F')
    return ch-'A'+10;
  return -1;
}

void
__getpacket (buffer)
     char *buffer;
{
    struct gdb_packet packet;
    int res;

    packet.state = 0;
    packet.contents = buffer;
    packet.err = 0;
    while ((res = __add_char_to_packet (readDebugChar () & 0xff, &packet)) != 1) {
        if (res == -2) {
            putDebugChar ('-'); // Tell host packet was not processed
            // Reset for the next packet
            packet.state = 0;
            packet.err = 0;
        }
    }
}

int
__add_char_to_packet (ch, packet)
     unsigned int ch;
     struct gdb_packet *packet;
{
  if (packet->state == 0)
    {
      if (ch == '$')
        {
          packet->state = 1;
          packet->length = 0;
          packet->checksum = 0;
          packet->xmitcsum = -1;
        }
      return 0;
    }
  
  if (packet->state == 1)
    {
      if (packet->length == BUFMAX)
        {
          packet->state = 0;
          packet->err = 1;
        }
      else if (ch == '#')
        {
          packet->contents[packet->length] = 0;
          packet->state = 2;
        }
      else 
        {
          packet->checksum += ch;
          packet->contents[packet->length++] = ch;
        }
      return 0;
    }

  if (packet->state == 2)
    {
      packet->xmitcsum = stubhex (ch) << 4;
      packet->state = 3;
      return 0;
    }

  if (packet->state == 3)
    {
      packet->xmitcsum |= stubhex (ch);
      if (packet->err) {
          // Packet was too long - just tell the consumer
          return -2;
      }
      if ((packet->checksum & 255) != packet->xmitcsum)
        {
          putDebugChar ('-');   /* failed checksum */
          packet->state = 0;
          return -1;
        }
      else
        {
          putDebugChar ('+'); /* successful transfer */
          /* if a sequence char is present, reply the sequence ID */
          if (packet->contents[2] == ':')
            {
              uint32 count = packet->length;
              uint32 i;
              putDebugChar (packet->contents[0]);
              putDebugChar (packet->contents[1]);
              /* remove sequence chars from buffer */
              for (i=3; i <= count; i++)
                packet->contents[i-3] = packet->contents[i];
            }
          return 1;
        }
    }
  /* We should never get here. */
  packet->state = 0;
  return -1;
}

/* send the packet in buffer.  */

void
__putpacket (buffer)
     char *buffer;
{
  unsigned char checksum;
  uint32 count;
  unsigned char ch;

  /*  $<packet info>#<checksum>. */
  do
    {
      putDebugChar ('$');
      checksum = 0;
      count = 0;

      while ((ch = buffer[count]))
        {
          putDebugChar (ch);
          checksum += ch;
          count += 1;
        }

      putDebugChar ('#');
      putDebugChar (hexchars[(checksum >> 4) & 0xf]);
      putDebugChar (hexchars[checksum & 0xf]);

    }
  while ((readDebugChar () & 0x7f) != '+');
}

char __remcomInBuffer[BUFMAX];
char __remcomOutBuffer[BUFMAX];

/* Indicate to caller of mem2hex or hex2mem that there has been an
   error.  */
volatile int __mem_fault = 0;


#ifndef TARGET_HAS_OWN_MEM_FUNCS
/*
 * _target_readmem_hook / _target_writemem_hook:
 * Allow target to get involved in reading/writing memory.
 *
 * If these hooks are defined by the target, they will be
 * called for each user program memory access.  Otherwise, the stub
 * will simply dereference a pointer to access user program memory.
 */

unsigned char (*_target_readmem_hook)  (unsigned char* addr);
void          (*_target_writemem_hook) (unsigned char* addr, 
                                        unsigned char value);

static unsigned char
get_target_byte (volatile unsigned char *address)
{
  if (_target_readmem_hook)     /* target needs to control memory access */
    return _target_readmem_hook ((unsigned char *) address);
  else
    return *address;
}

static void
put_target_byte (volatile unsigned char *address, unsigned char value)
{
  if (_target_writemem_hook)    /* target needs to control memory access */
    _target_writemem_hook ((unsigned char *) address, value);
  else
    *address = value;
}

/* These are the "arguments" to __do_read_mem and __do_write_mem, 
   which are passed as globals to avoid squeezing them thru
   __set_mem_fault_trap.  */

static volatile target_register_t memCount;
static volatile unsigned char    *memSrc,  *memDst;

/*
 * __do_read_mem:
 * Copy from target memory to trusted memory.
 */

static void
__do_read_mem (void)
{
  __mem_fault = 0;
  while (memCount)
    {
      unsigned char ch = get_target_byte (memSrc++);

      if (__mem_fault)
        return;
      *memDst++ = ch;
      memCount--;
    }
}

/*
 * __do_write_mem:
 * Copy from trusted memory to target memory.
 */

static void
__do_write_mem (void)
{
  __mem_fault = 0;
  while (memCount)
    {
      unsigned char ch = *memSrc++;

      put_target_byte (memDst++, ch);
      if (__mem_fault)
        return;
      memCount--;
    }
}

/*
 * __read_mem_safe:
 * Get contents of target memory, abort on error.
 */

int
__read_mem_safe (void *dst, target_register_t src, int count)
{
  memCount = count;
  memSrc   = (unsigned char *) src;
  memDst   = (unsigned char *) dst;
  __set_mem_fault_trap (__do_read_mem);
  return count - memCount;      /* return number of bytes successfully read */
}

/*
 * __write_mem_safe:
 * Set contents of target memory, abort on error.
 */

int
__write_mem_safe (unsigned char *src, target_register_t dst, int count)
{
  memCount = count;
  memSrc   = (unsigned char *) src;
  memDst   = (unsigned char *) dst;
  __set_mem_fault_trap (__do_write_mem);
  return count - memCount;      /* return number of bytes successfully read */
}

#endif /* TARGET_HAS_OWN_MEM_FUNCS */

/* These are the "arguments" to __mem2hex_helper and __hex2mem_helper, 
   which are passed as globals to avoid squeezing them thru
   __set_mem_fault_trap.  */

static int   hexMemCount;
static char *hexMemSrc, *hexMemDst;
static int   may_fault_mode;
#ifdef TARGET_HAS_HARVARD_MEMORY
static int   progMem;
#endif

/* Hamburger helper? */
static void
__mem2hex_helper (void)
{
    union {
        unsigned long  long_val;
        unsigned char  bytes[sizeof(long)];
    } val;
    int len, i;
    unsigned char ch;
    __mem_fault = 0;
    while (hexMemCount > 0) {
        if (may_fault_mode) {
            if ((hexMemCount >= sizeof(long)) &&
                (((target_register_t)hexMemSrc & (sizeof(long)-1)) == 0)) {
                // Should be safe to access via a long
                len = sizeof(long);
            } else if ((hexMemCount >= sizeof(short)) &&
                       (((target_register_t)hexMemSrc & (sizeof(short)-1)) == 0)) {
                // Should be safe to access via a short
                len = sizeof(short);
            } else {
                len = 1;
            }
#ifdef TARGET_HAS_HARVARD_MEMORY
	    if (progMem)
		__read_progmem_safe(&val.bytes[0], hexMemSrc, len);
	    else
#endif
            __read_mem_safe(&val.bytes[0], hexMemSrc, len);
        } else {
            len = 1;
            val.bytes[0] = *hexMemSrc;
        }
        if (__mem_fault)
            return;

        for (i = 0;  i < len;  i++) {
            ch = val.bytes[i];
            *(hexMemDst++) = hexchars[(ch >> 4) & 0xf];
            if (__mem_fault)
                return;
            *(hexMemDst++) = hexchars[ch & 0xf];
            if (__mem_fault)
                return;
        }
        hexMemCount -= len;
        hexMemSrc += len;
    }
}

/* Convert the memory pointed to by MEM into HEX, placing result in BUF.
 * Return a pointer to the last char put in buf (NUL). In case of a memory
 * fault, return 0.
 * If MAY_FAULT is non-zero, then we will handle memory faults by returning
 * a 0 (and assume that MEM is a pointer into the user program), else we 
 * treat a fault like any other fault in the stub (and assume that MEM is
 * a pointer into the stub's memory).
 */

char *
__mem2hex (mem, buf, count, may_fault)
     char *mem;
     char *buf;
     int count;
     int may_fault;
{
  hexMemDst      = (unsigned char *) buf;
  hexMemSrc      = (unsigned char *) mem;
  hexMemCount    = count;
  may_fault_mode = may_fault;
#ifdef TARGET_HAS_HARVARD_MEMORY
  progMem = 0;
#endif
  
  if (may_fault)
    {
      if (__set_mem_fault_trap (__mem2hex_helper))
        return 0;
    }
  else
    __mem2hex_helper ();

  *hexMemDst = 0;

  return (char *) hexMemDst;
}

/* Convert the target memory identified by MEM into HEX, placing result in BUF.
 * Return a pointer to the last char put in buf (NUL). In case of a memory
 * fault, return 0.
 */

static char *
__mem2hex_safe (target_addr_t mem, char *buf, int count)
{
  hexMemDst      = (unsigned char *) buf;
  hexMemSrc      = (unsigned char *) TARGET_ADDR_TO_PTR(mem);
  hexMemCount    = count;
  may_fault_mode = 1;
#ifdef TARGET_HAS_HARVARD_MEMORY
  progMem = TARGET_ADDR_IS_PROGMEM(mem);
#endif
  
  if (__set_mem_fault_trap (__mem2hex_helper))
    return 0;

  *hexMemDst = 0;

  return (char *) hexMemDst;
}



static void
__hex2mem_helper (void)
{
    union {
        unsigned long  long_val;
        unsigned char  bytes[sizeof(long)];
    } val;
    int len, i;
    unsigned char ch = '\0';

    __mem_fault = 0;
    while (hexMemCount > 0 && *hexMemSrc) {
        if (may_fault_mode) {
            if ((hexMemCount >= sizeof(long)) &&
                (((target_register_t)hexMemDst & (sizeof(long)-1)) == 0)) {
                len = sizeof(long);
            } else if ((hexMemCount >= sizeof(short)) &&
                       (((target_register_t)hexMemDst & (sizeof(short)-1)) == 0)) {
                len = sizeof(short);
            } else {
                len = 1;
            }
        } else {
            len = 1;
        }

        for (i = 0;  i < len;  i++) {
            // Check for short data?
            ch = stubhex (*(hexMemSrc++)) << 4;
            if (__mem_fault)
                return;
            ch |= stubhex (*(hexMemSrc++));
            if (__mem_fault)
                return;
            val.bytes[i] = ch;
        }

        if (may_fault_mode) {
#ifdef TARGET_HAS_HARVARD_MEMORY
	    if (progMem)
		__write_progmem_safe (&val.bytes[0], hexMemDst, len);
	    else
#endif
            __write_mem_safe (&val.bytes[0], hexMemDst, len);
        } else
            *hexMemDst = ch;

        if (__mem_fault)
            return;
        hexMemCount -= len;
        hexMemDst += len;
    }
}

/* Convert COUNT bytes of the hex array pointed to by BUF into binary
   to be placed in MEM.  Return a pointer to the character AFTER the
   last byte written.

   If MAY_FAULT is set, we will return a non-zero value if a memory
   fault occurs (and we assume that MEM is a pointer into the user
   program). Otherwise, we will take a trap just like any other memory
   fault (and assume that MEM points into the stub's memory). */

char *
__hex2mem (buf, mem, count, may_fault)
     char *buf;
     char *mem;
     int count;
     int may_fault;
{
  hexMemSrc      = (unsigned char *) buf;
  hexMemDst      = (unsigned char *) mem;
  hexMemCount    = count;
  may_fault_mode = may_fault;
#ifdef TARGET_HAS_HARVARD_MEMORY
  progMem = 0;
#endif

  if (may_fault)
    {
      if (__set_mem_fault_trap (__hex2mem_helper))
        return 0;
    }
  else
    __hex2mem_helper ();

  return (char *) hexMemDst;
}

/* Convert COUNT bytes of the hex array pointed to by BUF into binary
   to be placed in target MEM.  Return a pointer to the character AFTER
   the last byte written.
*/
char *
__hex2mem_safe (char *buf, target_addr_t mem, int count)
{
  hexMemSrc      = (unsigned char *) buf;
  hexMemDst      = (unsigned char *) TARGET_ADDR_TO_PTR(mem);
  hexMemCount    = count;
  may_fault_mode = 1;
#ifdef TARGET_HAS_HARVARD_MEMORY
  progMem = TARGET_ADDR_IS_PROGMEM(mem);
#endif

  if (__set_mem_fault_trap (__hex2mem_helper))
    return 0;

  return (char *) hexMemDst;
}


void
set_debug_traps (void)
{
  __install_traps ();
  initialized = 1;    /* FIXME: Change this to dbg_stub_initialized */
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */

unsigned int
__hexToInt (char **ptr, target_register_t *intValue)
{
  int numChars = 0;
  int hexValue;

  *intValue = 0;

  while (**ptr)
    {
      hexValue = stubhex (**ptr);
      if (hexValue < 0)
        break;

      *intValue = (*intValue << 4) | hexValue;
      numChars ++;

      (*ptr)++;
    }

  return (numChars);
}

/*
 * While we find nice hex chars, build a target memory address.
 * Return number of chars processed.
 */

unsigned int
__hexToAddr (char **ptr, target_addr_t *val)
{
  int numChars = 0;
  int hexValue;

  *val = 0;

  while (**ptr)
    {
      hexValue = stubhex (**ptr);
      if (hexValue < 0)
        break;

      *val = (*val << 4) | hexValue;
      numChars ++;

      (*ptr)++;
    }

  return (numChars);
}


/* 
 * Complement of __hexToInt: take an int of size "numBits", 
 * convert it to a hex string.  Return length of (unterminated) output.
 */

unsigned int
__intToHex (char *ptr, target_register_t intValue, int numBits)
{
  int numChars = 0;

  if (intValue == 0)
    {
      *(ptr++) = '0';
      *(ptr++) = '0';
      return 2;
    }

  numBits = (numBits + 7) / 8;
  while (numBits)
    {
      int v = (intValue >> ((numBits - 1) * 8));
      if (v || (numBits == 1))
        {
          v = v & 255;
          *(ptr++) = __tohex ((v / 16) & 15);
          *(ptr++) = __tohex (v & 15);
          numChars += 2;
        }
      numBits--;
    }

  return (numChars);
}

#if DEBUG_THREADS 
/*
 * Kernel Thread Control
 *
 * If the current thread is set to other than zero (or minus one),
 * then ask the kernel to lock it's scheduler so that only that thread
 * can run.
 */

static unsigned char did_lock_scheduler = 0;
static unsigned char did_disable_interrupts = 0;

/* Pointer to "kernel call" for scheduler control */
static int (*schedlock_fn) (int, int, long) = stub_lock_scheduler;

/* Pointer to target stub call for disabling interrupts.
   Target stub will initialize this if it can.  */
int (*__disable_interrupts_hook) (int); /* don't initialize here! */
#endif

static void
lock_thread_scheduler (int kind)        /* "step" or "continue" */
{
#if DEBUG_THREADS 
  int ret = 0;

  /* GDB will signal its desire to run a single thread
     by setting _gdb_cont_thread to non-zero / non-negative.  */
  if (_gdb_cont_thread <= 0)
    return;

  if (schedlock_fn)                     /* kernel call */
    ret = (*schedlock_fn) (1, kind, _gdb_cont_thread);

  if (ret == 1)
    {
      did_lock_scheduler = 1;
      return;
    }

  if (schedlock_fn == 0 ||              /* no kernel scheduler call */
      ret == -1)                        /* kernel asks stub to handle it */
    if (__disable_interrupts_hook)      /* target stub has capability */
      if ((*__disable_interrupts_hook) (1))
        {
          did_disable_interrupts = 1;
          return;
        }
#endif /* DEBUG_THREADS */
}

static void
unlock_thread_scheduler ()
{
#if DEBUG_THREADS
  if (did_lock_scheduler)
    if (schedlock_fn)                   /* kernel call */
      {
        (*schedlock_fn) (0, 0, _gdb_cont_thread);
        /* I could check the return value, but 
           what would I do if it failed???  */
        did_lock_scheduler = 0;
      }
  if (did_disable_interrupts)
    if (__disable_interrupts_hook)      /* target stub call */
      {
        (*__disable_interrupts_hook) (0);
        /* Again, I could check the return value, but 
           what would I do if it failed???  */
        did_disable_interrupts = 0;
      }
#endif /* DEBUG_THREADS */
}

#ifdef CYGPKG_CYGMON
int processing_breakpoint_function = 0;
#endif

void
__handle_exception (void)
{
  int sigval = 0;

#ifdef TARGET_HAS_NEXT_STEP
  if (! __next_step_done ())
    {
      __clear_breakpoints ();
      __install_breakpoints ();
      __single_step ();
      return;
    }
#endif

#ifdef __ECOS__
  // We need to unpack the registers before they are accessed.
  if (__cleanup_vec != NULL)
    __cleanup_vec ();

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
  // Temporary support for gnupro bsp SWIs
  if (__is_bsp_syscall())
  {
      sigval = hal_syscall_handler();
      if (sigval <= 0)
      {
	  if (sigval < 0)
	      __process_exit_vec ();

	  if (__init_vec != NULL)
              __init_vec ();
	  return;
      }
  }
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
  // Special case for GDB BREAKs. This flag is set by cyg_stub_cleanup.
  if (cyg_hal_gdb_break) {
      cyg_hal_gdb_break = 0;
      sigval = SIGINT;
  }
#endif
   
  // Only compute sigval if it wasn't already computed (in
  // hal_syscall_handler or as a result of a GDB async break)
  if (0 == sigval)
      sigval = __computeSignal (__get_trap_number ());

#else  // __ECOS__
  /* reply to host that an exception has occurred */
  sigval = __computeSignal (__get_trap_number ());
#endif // __ECOS__

  if (__is_breakpoint_function ())
  {
#ifdef CYGPKG_CYGMON
    processing_breakpoint_function = 1;
#endif
    __skipinst ();
  } else {
#ifdef CYGPKG_CYGMON
    processing_breakpoint_function = 0;
#endif
  }

#ifndef __ECOS__
  if (__cleanup_vec != NULL)
    __cleanup_vec ();
#endif // !__ECOS__

  __clear_breakpoints (); 

  /* Undo effect of previous single step.  */
  unlock_thread_scheduler ();
  __clear_single_step ();

#ifdef __ECOS__
      /* Need to flush the data and instruction cache here, as we may have
         removed a breakpoint in __single_step - and we may be sharing
         some code with the application! */

        __data_cache (CACHE_FLUSH) ;
        __instruction_cache (CACHE_FLUSH) ;
#endif

#ifdef SIGSYSCALL
  if (sigval == SIGSYSCALL)
    {
      int val;
      /* Do the skipinst FIRST. */
#ifndef SYSCALL_PC_AFTER_INST
      __skipinst ();
#endif
      val =  __process_syscall_vec (__get_syscall_num ());
      if (val < 0)
        sigval = -val;
      else
        sigval = 0;
    }

#endif

  /* Indirect function call to stub, cygmon monitor or other */
  if (sigval != 0)
    {
      while (__process_exception_vec (sigval))
        {
          /* Empty! */
        }
    }

  __install_breakpoints ();

  if (__init_vec != NULL)
    __init_vec ();
}

/*
 * _get_trace_register_hook:
 * This function pointer will be non-zero if the trace component
 * wants to intercept requests for register values.
 * 
 * FIXME: evidently I need a new hook for large registers...
 */

int   (*_get_trace_register_hook) (regnames_t, target_register_t *);

void
stub_format_registers(char *packet, char *ptr)
{
    int regnum;
    int sr = 0, er = NUMREGS_GDB;

    if (packet[0] == 'p')
      {
	 target_register_t regno;
         char *p = &packet[1];
	 if (__hexToInt (&p, &regno))
	   {
	     sr = regno;
	     er = regno + 1;
	   }
	 else
	   {
	     strcpy (ptr, "INVALID");
	     return;
	   }
      }

    for (regnum = sr; regnum < er; regnum++)
      {
        /* We need to compensate for the value offset within the
           register. */
        char dummyDat[32];
        target_register_t addr;
        char *vptr;
        int  reg_valid = 1;

#ifdef TARGET_HAS_LARGE_REGISTERS
        if (sizeof (target_register_t) < REGSIZE (regnum)) {
            get_register_as_bytes (regnum, dummyDat);
            vptr = dummyDat;
        } else
#endif
        {
            if (_get_trace_register_hook)
                reg_valid = _get_trace_register_hook (regnum, &addr);
            else
            {
                addr = get_register (regnum);
#ifdef CYGHWR_REGISTER_VALIDITY_CHECKING
                reg_valid = get_register_valid (regnum);
#endif
            }
            vptr = ((char *) &addr);
            if (sizeof (addr) > REGSIZE(regnum)) {
                /* May need to cope with endian-ness */

#if !defined(__LITTLE_ENDIAN__) && !defined(_LITTLE_ENDIAN)
                vptr += sizeof (addr) - REGSIZE (regnum);
#endif
            } else if (sizeof (addr) < REGSIZE (regnum)) {
                int off = REGSIZE (regnum) - sizeof (addr);
                int x;
                char extend_val = 0;

#ifdef CYGARC_SIGN_EXTEND_REGISTERS
                {
                    unsigned long bits_in_addr = (sizeof(addr) << 3);  // ie Size in bytes * 8
                    target_register_t sign_bit_mask = (1 << (bits_in_addr - 1));
                    if ((addr & sign_bit_mask) == sign_bit_mask)
                        extend_val = ~0;
                }
#endif

#if defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN)
                for (x = 0; x < off; x++)
                    dummyDat[x + sizeof(addr)] = extend_val;
                _memcpy (dummyDat, &addr, sizeof (addr));
#else
                for (x = 0; x < off; x++)
                    dummyDat[x] = extend_val;
                _memcpy (dummyDat + off, &addr, sizeof (addr));
#endif
                vptr = dummyDat;
            }
        }
        if (reg_valid) {      /* we have a valid reg value */
            ptr = __mem2hex (vptr, ptr, REGSIZE (regnum), 0);
        } else {
            /* Trace component returned a failure code.
               This means that the register value is not available.
               We'll fill it with 'x's, and GDB will understand.  */
            _memset (ptr, 'x', 2 * REGSIZE (regnum));
            ptr += 2 * REGSIZE (regnum);
        }
    }
}

void
stub_update_registers(char *in_ptr, char *out_ptr)
{
    char *ptr = &in_ptr[1];
    int x;
    int sr = 0, er = NUMREGS_GDB;

    if (*in_ptr == 'P') {
        target_register_t regno;

        if (__hexToInt (&ptr, &regno) && (*ptr++ == '=')) {

            sr = regno;
            er = regno + 1;
        } else {
            strcpy (out_ptr, "P01");
            return;
        }
    }

    for (x = sr; x < er; x++) {
        target_register_t value = 0;
        char *vptr;

#ifdef TARGET_HAS_LARGE_REGISTERS
        if (sizeof (target_register_t) < REGSIZE (x)) {
            char dummyDat [32];

            __hex2mem (ptr, dummyDat, REGSIZE (x), 0);
            put_register_as_bytes (x, dummyDat);
        } else 
#endif
        {
            vptr = ((char *) &value);
#if !defined(__LITTLE_ENDIAN__) && !defined(_LITTLE_ENDIAN)
            vptr += sizeof (value) - REGSIZE (x);
#endif
            __hex2mem (ptr, vptr, REGSIZE (x), 0);
            put_register (x, value);
        }
        ptr += REGSIZE (x) * 2;
    }

    strcpy (out_ptr, "OK");
}

int
__process_packet (char *packet)
{
  int  is_binary = 0;
#if defined(CYGNUM_HAL_BREAKPOINT_LIST_SIZE)
  int is_Z = 0;
#endif

  __remcomOutBuffer[0] = 0;
  switch (packet[0])
    {
    case '?':
      {
        int sigval = __computeSignal (__get_trap_number ());
        __remcomOutBuffer[0] = 'S';
        __remcomOutBuffer[1] = hexchars[(sigval >> 4) & 0xf];
        __remcomOutBuffer[2] = hexchars[sigval & 0xf];
        __remcomOutBuffer[3] = 0;
        break;
      }

#ifdef __ECOS__
#if !defined(CYG_HAL_STARTUP_RAM)    // Only for ROM based stubs
#if 0 // Disable to avoid conflict with stub-breakpoint z/Z-packets
    case 'z':
        /* report IO buffer sizes so download can achieve optimal
           download speed */
    {
        int i;
        i = __intToHex (__remcomOutBuffer, BUFMAX, 32);
        __remcomOutBuffer[i] = 0;
        break;
    }
#endif
    case 'd':
      /* toggle debug flag */
      strcpy(__remcomOutBuffer, GDB_stubs_version);
      break;
#endif
#endif // __ECOS__

    case 'q':
      /* general query packet */
      process_query (&packet[1]);
      break;

    case 'Q':
      /* general set packet */
      process_set (&packet[1]);
      break;

    case 'p':		/* return the value of  a single CPU register */
    case 'g':           /* return the value of the CPU registers */
      {
        stub_format_registers(&packet[0], __remcomOutBuffer);
        break;
      }

    case 'A': /* set program arguments */
      {
#ifdef CYGSEM_ECOS_SUPPORTS_PROGRAM_ARGS
        if (packet[1] == '\0')
          {
            __free_program_args ();
            strcpy (__remcomOutBuffer, "OK");
          }
        else 
          {
            target_register_t arglen, argnum;
            char *ptr = &packet[1];

            while (1)
              {
                if (__hexToInt (&ptr, &arglen)
                    && (*ptr++ == ',')
                    && __hexToInt (&ptr, &argnum)
                    && (*ptr++ == ','))
                  {
                    if (arglen > 0)
                      {
                        char *s = __add_program_arg (argnum, arglen);
                        if (s != NULL)
                          {
                            __hex2mem (ptr, s, arglen, 0);
                          }
                        ptr += arglen * 2;
                      }

                    if (*ptr == ',')
                      ptr++;
                    else
                      break;
                  }
                else
                  break;
              }
            if (*ptr == '\0')
              strcpy (__remcomOutBuffer, "OK");
            else
              strcpy (__remcomOutBuffer, "E01");
          }
#else
        strcpy (__remcomOutBuffer, "E01");
#endif
      }
      break;

    case 'P':
    case 'G':      /* set the value of the CPU registers - return OK */
      {
        char *in_ptr = &packet[0];
        char *out_ptr = __remcomOutBuffer;
        stub_update_registers(in_ptr, out_ptr);
        break;
      }

    case 'm':     /* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
      /* Try to read %x,%x.  */
      {
        target_register_t length;
        char *ptr = &packet[1];
        target_addr_t addr;

        if (__hexToAddr (&ptr, &addr)
            && *ptr++ == ','
            && __hexToInt (&ptr, &length))
          {
	    if (__mem2hex_safe (addr, __remcomOutBuffer, length))
              break;

            strcpy (__remcomOutBuffer, "E03");
          }
        else
          strcpy (__remcomOutBuffer, "E01");
        break;
      }

    case 'X':
      /* XAA..AA,LLLL: Write LLLL escaped binary bytes at address AA.AA */
      is_binary = 1;
      /* fall through */
    case 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
      /* Try to read '%x,%x:'.  */
      {
        target_register_t length;
        char *ptr = &packet[1], buf[128];
        int  i;
        target_addr_t addr;

        if (__hexToAddr (&ptr, &addr)
	    && *ptr++ == ','
            && __hexToInt (&ptr, &length)
            && *ptr++ == ':')
          {
            /* GDB sometimes sends an impossible length */
            if (length < 0 || length >= BUFMAX)
              strcpy (__remcomOutBuffer, "E01");
                
            else if (is_binary)
              {
                while (length > 0)
                  {
                    for (i = 0; i < sizeof(buf) && i < length; i++)
                      if ((buf[i] = *ptr++) == 0x7d)
                        buf[i] = 0x20 | (*ptr++ & 0xff);

#ifdef TARGET_HAS_HARVARD_MEMORY
		    if (TARGET_ADDR_IS_PROGMEM(addr)) {
		      if (__write_progmem_safe (buf, (void *)TARGET_ADDR_TO_PTR(addr), i) != i)
                        break;
		    } else
#endif
		      if (__write_mem_safe (buf, (void *)TARGET_ADDR_TO_PTR(addr), i) != i)
			break;


                    length -= i;
                    addr += i;
                  }
                if (length <= 0)
                  strcpy (__remcomOutBuffer, "OK");
                else
                  strcpy (__remcomOutBuffer, "E03");
              }
            else
              {
                if (__hex2mem_safe (ptr, addr, length) != NULL)
                  strcpy (__remcomOutBuffer, "OK");
                else
                  strcpy (__remcomOutBuffer, "E03");
              }
          }
        else
          strcpy (__remcomOutBuffer, "E02");
        break;
      }

    case 'S':
    case 's':    /* sAA..AA    Step from address AA..AA (optional) */
    case 'C':
    case 'c':    /* cAA..AA    Continue at address AA..AA (optional) */
      /* try to read optional parameter, pc unchanged if no parm */

      {
        char *ptr = &packet[1];
        target_addr_t addr;
        target_register_t sigval = 0;

        if (packet[0] == 'C' || packet[0] == 'S')
          {
            __hexToInt (&ptr, &sigval);
            if (*ptr == ';')
              ptr++;
          }

        if (__hexToAddr (&ptr, &addr))
          set_pc ((target_register_t)TARGET_ADDR_TO_PTR(addr));

      /* Need to flush the instruction cache here, as we may have
         deposited a breakpoint, and the icache probably has no way of
         knowing that a data ref to some location may have changed
         something that is in the instruction cache.  */

#ifdef __ECOS__
        __data_cache (CACHE_FLUSH) ;
#endif
        __instruction_cache (CACHE_FLUSH) ;

        /* If we have a function to handle signals, call it. */
        if (sigval != 0 && __process_signal_vec != NULL)
          {
            /* If 0 is returned, we either ignored the signal or invoked a user
               handler. Otherwise, the user program should die. */
            if (! __process_signal_vec (sigval))
              sigval = 0;
          }

        if (sigval != 0)
          {
            sigval = SIGKILL; /* Always nuke the program */
            __kill_program (sigval);
            return 0;
          }

#ifdef __ECOS__
        // CASE 102327 - watchpoints fight with output, so do not step
        // through $O packet output routines.
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
        if ( cyg_hal_gdb_break_is_set() ) {
            packet[0] = 'c'; // Force it to be a "continue" instead of step.
            cyg_hal_gdb_running_step = 1; // And tell the hal_stub...
        }
#endif
#endif

        /* Set machine state to force a single step.  */
        if (packet[0] == 's' || packet[0] == 'S')
          {
            lock_thread_scheduler (0);  /* 0 == single-step */
#ifdef __ECOS__
            // PR 19845 workaround:
            // Make sure the single-step magic affects the correct registers.
            _registers = &registers[0];
#endif
            __single_step ();
          }
        else
          {
            lock_thread_scheduler (1);  /* 1 == continue */
          }

#ifdef __ECOS__
      /* Need to flush the data and instruction cache here, as we may have
         deposited a breakpoint in __single_step. */

        __data_cache (CACHE_FLUSH) ;
        __instruction_cache (CACHE_FLUSH) ;
	hal_flush_output();
#endif

        return -1;
      }

    case 'D' :     /* detach */
      __putpacket (__remcomOutBuffer);
      /* fall through */
    case 'k' :      /* kill the program */
#ifdef __ECOS__
      hal_flush_output();
#endif
      __process_exit_vec ();
      return -1;

    case 'r':           /* Reset */
      /* With the next 'k' packet, reset the board */
      __process_exit_vec = &__reset;
      break;

    case 'H':
      STUB_PKT_CHANGETHREAD (packet+1, __remcomOutBuffer, 300) ;
      break ;
    case 'T' :
      STUB_PKT_THREAD_ALIVE (packet+1, __remcomOutBuffer, 300) ;
      break ;
    case 'B':
      /* breakpoint */
      {
        target_register_t addr;
        char mode;
        char *ptr = &packet[1];
        if (__hexToInt (&ptr, &addr) && *(ptr++) == ',')
          {
            mode = *(ptr++);
            if (mode == 'C')
              __remove_breakpoint (addr,0);
            else
              __set_breakpoint (addr,0);
            strcpy (__remcomOutBuffer, "OK");
          }
        else
          {
            strcpy (__remcomOutBuffer, "E01");
          }
        break;
      }

      case 'b':   /* bBB...  Set baud rate to BB... */
      {
        target_register_t baudrate;

        char *ptr = &packet[1];
        if (!__hexToInt (&ptr, &baudrate))
          {
            strcpy (__remcomOutBuffer, "B01");
            break;
          }

        __putpacket ("OK");     /* Ack before changing speed */
        __set_baud_rate (baudrate);
        break;
      }

#if defined(CYGNUM_HAL_BREAKPOINT_LIST_SIZE) && (CYGNUM_HAL_BREAKPOINT_LIST_SIZE > 0)
    case 'Z':
      is_Z = 1;
    case 'z':
      {
	char *ptr = &packet[1];
	target_register_t ztype, addr, length;
	int err;
	target_addr_t taddr;

	if (__hexToInt (&ptr, &ztype) && *(ptr++) == ',')
	  {
	    if (__hexToAddr (&ptr, &taddr))
	      {
		if (*(ptr++) == ',')
		  {
		      /* When there is a comma, there must be a length */
		      if  (!__hexToInt (&ptr, &length))
			{
			  strcpy (__remcomOutBuffer, "E02");
			  break;
			}
		  }
		else
		  length = 0;

		addr = (target_register_t)TARGET_ADDR_TO_PTR(taddr);

		switch (ztype)
		  {
		    case ZTYPE_SW_BREAKPOINT:
		      /* sw breakpoint */
		      if (is_Z)
			err = __set_breakpoint(addr,length);
		      else
			err = __remove_breakpoint(addr,length);
		      if (!err)
			strcpy (__remcomOutBuffer, "OK");
		      else
			strcpy (__remcomOutBuffer, "E02");
		      break;
		    case ZTYPE_HW_BREAKPOINT:
#if defined(HAL_STUB_HW_BREAKPOINT_LIST_SIZE) && (HAL_STUB_HW_BREAKPOINT_LIST_SIZE > 0)
		      if (is_Z)
			err = __set_hw_breakpoint(addr, length);
		      else
			err = __remove_hw_breakpoint(addr, length);
		      if (!err)
			strcpy (__remcomOutBuffer, "OK");
		      else
#endif
			strcpy (__remcomOutBuffer, "E02");
		      break;
		    case ZTYPE_HW_WATCHPOINT_WRITE:
		    case ZTYPE_HW_WATCHPOINT_READ:
		    case ZTYPE_HW_WATCHPOINT_ACCESS:
#if defined(HAL_STUB_HW_WATCHPOINT_LIST_SIZE) && (HAL_STUB_HW_WATCHPOINT_LIST_SIZE > 0)
		      if (is_Z)
			err = __set_hw_watchpoint(addr, length, ztype);
		      else
			err = __remove_hw_watchpoint(addr, length, ztype);
		      if (!err)
			strcpy (__remcomOutBuffer, "OK");
		      else
#endif
			strcpy (__remcomOutBuffer, "E02");
		      break;
		  }
	      }
	  }
	break;
      }
#endif // Z packet support
#ifdef CYGPKG_HAL_GDB_FILEIO // File I/O over the GDB remote protocol
    case 'F':
    {
        extern void cyg_hal_gdbfileio_process_F_packet( char *, char *);
        cyg_hal_gdbfileio_process_F_packet( packet, __remcomOutBuffer );
        return -1;
    }
#endif
    default:
      __process_target_packet (packet, __remcomOutBuffer, 300);
      break;
    }

  /* reply to the request */
  __putpacket (__remcomOutBuffer);
  return 0;
}

static void
send_t_packet (int sigval)
{
  __build_t_packet (sigval, __remcomOutBuffer);
  __putpacket (__remcomOutBuffer);
}

/*
 * This function does all command procesing for interfacing to gdb.
 */

static int
process_exception (int sigval)
{
  int status;

  /* Nasty. */
  if (ungot_char < 0)
    send_t_packet (sigval);

  do {
    __getpacket (__remcomInBuffer);
    status = __process_packet (__remcomInBuffer);
  } while (status == 0);

  if (status < 0)
    return 0;
  else
    return 1;
}

void
__send_exit_status (int status)
{
  __remcomOutBuffer[0] = 'W';
  __remcomOutBuffer[1] = hexchars[(status >> 4) & 0xf];
  __remcomOutBuffer[2] = hexchars[status & 0xf];
  __remcomOutBuffer[3] = 0;
  __putpacket (__remcomOutBuffer);
}

/* Read up to MAXLEN bytes from the remote GDB client, and store in DEST
   (which is a pointer in the user program). BLOCK indicates what mode
   is being used; if it is set, we will wait for MAXLEN bytes to be
   entered. Otherwise, the function will return immediately with whatever
   bytes are waiting to be read.

   The value returned is the number of bytes read. A -1 indicates that an
   error of some sort occurred. */

int
__get_gdb_input (target_register_t dest, int maxlen, int block)
{
  char buf[4];
  int len, i;
  char d;

  buf[0] = 'I';
  buf[1] = '0';
  buf[2] = block ? '0' : '1';
  buf[3] = 0;
  __putpacket (buf);
  __getpacket (__remcomInBuffer);
  if (__remcomInBuffer[0] != 'I')
    return -1;
  len = stubhex (__remcomInBuffer[1]) * 16 + stubhex (__remcomInBuffer[2]);
  for (i = 0; i < len; i++)
    {
      d = stubhex (__remcomInBuffer[3 + i * 2]) * 16;
      d |=  stubhex (__remcomInBuffer[3 + i * 2 + 1]);
      __write_mem_safe (&d, (void *)(dest + i), 1);
    }
  /* Write the trailing \0. */
  d = '\0';
  __write_mem_safe (&d, (void *)(dest + i), 1);
  return len;
}

void
__output_hex_value (target_register_t i)
{
  char buf[32], *ptr=buf+31;
  unsigned int x;

  *ptr = 0;
  for (x = 0; x < (sizeof (i) * 2); x++)
    {
      *(--ptr) = hexchars[i & 15];
      i = i >> 4;
    }
  while (*ptr)
    {
      putDebugChar (*(ptr++));
    }
}

/* Write the C-style string pointed to by STR to the GDB comm port. */
void
__putDebugStr (char *str)
{
  while (*str)
    {
      putDebugChar (*str);
      str++;
    }
}

/* Send STRING_LEN bytes of STR to GDB, using 'O' packets.
   STR is assumed to be in the program being debugged. */

int
__output_gdb_string (target_register_t str, int string_len)
{
  /* We will arbitrarily limit output packets to less than 400 bytes. */
  static char buf[400];
  int x;
  int len;

  if (string_len == 0)
    {
      /* We can't do strlen on a user pointer. */
      return -1;
    }

  len = string_len;
  while (len > 0)
    {
      int packetlen = ((len < 175) ? len : 175);
      buf[0] = 'O';
      for (x = 0; x < packetlen; x++) 
        {
          char c;

          __read_mem_safe (&c, (void *)(str + x), 1);
          buf[x*2+1] = hexchars[(c >> 4) & 0xf];
          buf[x*2+2] = hexchars[c % 16];
        }
      str += x;
      len -= x;
      buf[x*2+1] = 0;
      __putpacket (buf);
    }
  return string_len;
}

static void
do_nothing (void)
{
  /* mmmm */
}

static int
syscall_do_nothing (int junk)
{
  return 0;
}

/* Start the stub running. */
void
__switch_to_stub (void)
{
  __process_exception_vec = process_exception;
#ifdef CYGPKG_CYGMON
  // Cygmon will have consumed the '$' character for this packet.
  // Let's put one in the unget buffer.
  // Actually, Cygmon does an unget, but since it uses different
  // unget handling, we need to do this here.
  ungetDebugChar('$');
#endif
}

#if ! defined(BOARD_SPECIFIC_STUB_INIT)
void
initialize_stub (void)
{
  set_debug_traps ();
  /* FIXME: This function should be renamed to specifically init the
     hardware required by debug operations. If initHardware is implemented at
     all, it should be called before main ().
     */
  initHardware () ;
  /* This acks any stale packets , NOT an effective solution */
  putDebugChar ('+');
}
#endif

void
ungetDebugChar (int c)
{
  ungot_char = c;
}

void
__kill_program (int sigval)
{
  __remcomOutBuffer[0] = 'X';
  __remcomOutBuffer[1] = hexchars[(sigval >> 4) & 15];
  __remcomOutBuffer[2] = hexchars[sigval & 15];
  __remcomOutBuffer[3] = 0;
  __putpacket (__remcomOutBuffer);
}

#ifdef CYGSEM_ECOS_SUPPORTS_PROGRAM_ARGS
#define MAX_ARG_COUNT 20
#define MAX_ARGDATA 128

static char *program_argv [MAX_ARG_COUNT];
static int program_argc;
static int last_program_arg;
static char program_argstr [MAX_ARGDATA], *argptr;
static int args_initted = 0;

void
__free_program_args (void)
{
  last_program_arg = -1;
  program_argc = 0;
  program_argv [0] = NULL;
  argptr = program_argstr;
  args_initted = 1;
}

static char *
__add_program_arg (int argc, uint32 len)
{
  char *res;

  if (! args_initted)
    {
      __free_program_args ();
    }

  if ((argc >= (MAX_ARG_COUNT - 1))
      || ((argptr - program_argstr + len) > MAX_ARGDATA))
    {
      return NULL;
    }

  if (argc != last_program_arg)
    {
      if (argc >= program_argc)
        {
          program_argc = argc + 1;
          program_argv [program_argc] = NULL;
        }
      program_argv [argc] = argptr;
      last_program_arg = argc;
    }

  res = argptr;
  argptr += len;

  return res;
}

void
__set_program_args (int argc, char **argv)
{
  int x;

  __free_program_args ();
  if (argc)
    {
      for (x = 0; x < argc; x++)
        {
          uint32 len = strlen (argv[x])+1;
          char *s = __add_program_arg (x, len);

          if (s == NULL)
            return;

          _memcpy (s, argv[x], len);
        }
    }
}

char **
__get_program_args (target_register_t argcPtr)
{
  if (!args_initted)
    {
      __free_program_args ();
    }
  __write_mem_safe ((char *) &program_argc, (void *)argcPtr, sizeof (program_argc));
  return program_argv;
}
#endif

/* Table used by the crc32 function to calcuate the checksum. */
#ifdef CYGDBG_HAL_CRCTABLE_LOCATION_RAM
static uint32 crc32_table[256];
static int tableInit = 0;
#else
static const uint32 crc32_table[256]={
	0x00000000,0x04c11db7,0x09823b6e,0x0d4326d9,0x130476dc,0x17c56b6b,0x1a864db2,0x1e475005,
	0x2608edb8,0x22c9f00f,0x2f8ad6d6,0x2b4bcb61,0x350c9b64,0x31cd86d3,0x3c8ea00a,0x384fbdbd,
	0x4c11db70,0x48d0c6c7,0x4593e01e,0x4152fda9,0x5f15adac,0x5bd4b01b,0x569796c2,0x52568b75,
	0x6a1936c8,0x6ed82b7f,0x639b0da6,0x675a1011,0x791d4014,0x7ddc5da3,0x709f7b7a,0x745e66cd,
	0x9823b6e0,0x9ce2ab57,0x91a18d8e,0x95609039,0x8b27c03c,0x8fe6dd8b,0x82a5fb52,0x8664e6e5,
	0xbe2b5b58,0xbaea46ef,0xb7a96036,0xb3687d81,0xad2f2d84,0xa9ee3033,0xa4ad16ea,0xa06c0b5d,
	0xd4326d90,0xd0f37027,0xddb056fe,0xd9714b49,0xc7361b4c,0xc3f706fb,0xceb42022,0xca753d95,
	0xf23a8028,0xf6fb9d9f,0xfbb8bb46,0xff79a6f1,0xe13ef6f4,0xe5ffeb43,0xe8bccd9a,0xec7dd02d,
	0x34867077,0x30476dc0,0x3d044b19,0x39c556ae,0x278206ab,0x23431b1c,0x2e003dc5,0x2ac12072,
	0x128e9dcf,0x164f8078,0x1b0ca6a1,0x1fcdbb16,0x018aeb13,0x054bf6a4,0x0808d07d,0x0cc9cdca,
	0x7897ab07,0x7c56b6b0,0x71159069,0x75d48dde,0x6b93dddb,0x6f52c06c,0x6211e6b5,0x66d0fb02,
	0x5e9f46bf,0x5a5e5b08,0x571d7dd1,0x53dc6066,0x4d9b3063,0x495a2dd4,0x44190b0d,0x40d816ba,
	0xaca5c697,0xa864db20,0xa527fdf9,0xa1e6e04e,0xbfa1b04b,0xbb60adfc,0xb6238b25,0xb2e29692,
	0x8aad2b2f,0x8e6c3698,0x832f1041,0x87ee0df6,0x99a95df3,0x9d684044,0x902b669d,0x94ea7b2a,
	0xe0b41de7,0xe4750050,0xe9362689,0xedf73b3e,0xf3b06b3b,0xf771768c,0xfa325055,0xfef34de2,
	0xc6bcf05f,0xc27dede8,0xcf3ecb31,0xcbffd686,0xd5b88683,0xd1799b34,0xdc3abded,0xd8fba05a,
	0x690ce0ee,0x6dcdfd59,0x608edb80,0x644fc637,0x7a089632,0x7ec98b85,0x738aad5c,0x774bb0eb,
	0x4f040d56,0x4bc510e1,0x46863638,0x42472b8f,0x5c007b8a,0x58c1663d,0x558240e4,0x51435d53,
	0x251d3b9e,0x21dc2629,0x2c9f00f0,0x285e1d47,0x36194d42,0x32d850f5,0x3f9b762c,0x3b5a6b9b,
	0x0315d626,0x07d4cb91,0x0a97ed48,0x0e56f0ff,0x1011a0fa,0x14d0bd4d,0x19939b94,0x1d528623,
	0xf12f560e,0xf5ee4bb9,0xf8ad6d60,0xfc6c70d7,0xe22b20d2,0xe6ea3d65,0xeba91bbc,0xef68060b,
	0xd727bbb6,0xd3e6a601,0xdea580d8,0xda649d6f,0xc423cd6a,0xc0e2d0dd,0xcda1f604,0xc960ebb3,
	0xbd3e8d7e,0xb9ff90c9,0xb4bcb610,0xb07daba7,0xae3afba2,0xaafbe615,0xa7b8c0cc,0xa379dd7b,
	0x9b3660c6,0x9ff77d71,0x92b45ba8,0x9675461f,0x8832161a,0x8cf30bad,0x81b02d74,0x857130c3,
	0x5d8a9099,0x594b8d2e,0x5408abf7,0x50c9b640,0x4e8ee645,0x4a4ffbf2,0x470cdd2b,0x43cdc09c,
	0x7b827d21,0x7f436096,0x7200464f,0x76c15bf8,0x68860bfd,0x6c47164a,0x61043093,0x65c52d24,
	0x119b4be9,0x155a565e,0x18197087,0x1cd86d30,0x029f3d35,0x065e2082,0x0b1d065b,0x0fdc1bec,
	0x3793a651,0x3352bbe6,0x3e119d3f,0x3ad08088,0x2497d08d,0x2056cd3a,0x2d15ebe3,0x29d4f654,
	0xc5a92679,0xc1683bce,0xcc2b1d17,0xc8ea00a0,0xd6ad50a5,0xd26c4d12,0xdf2f6bcb,0xdbee767c,
	0xe3a1cbc1,0xe760d676,0xea23f0af,0xeee2ed18,0xf0a5bd1d,0xf464a0aa,0xf9278673,0xfde69bc4,
	0x89b8fd09,0x8d79e0be,0x803ac667,0x84fbdbd0,0x9abc8bd5,0x9e7d9662,0x933eb0bb,0x97ffad0c,
	0xafb010b1,0xab710d06,0xa6322bdf,0xa2f33668,0xbcb4666d,0xb8757bda,0xb5365d03,0xb1f740b4,
};
#endif

/* 
   Calculate a CRC-32 using LEN bytes of PTR. CRC is the initial CRC
   value.
   PTR is assumed to be a pointer in the user program. */

static uint32
crc32 (target_addr_t mem, int len, uint32 crc)
{
  unsigned char *ptr = (unsigned char *)TARGET_ADDR_TO_PTR(mem);
#ifdef TARGET_HAS_HARVARD_MEMORY
  int  is_progmem = TARGET_ADDR_IS_PROGMEM(mem);
#endif

#ifdef CYGDBG_HAL_CRCTABLE_LOCATION_RAM
  if (! tableInit)
    {
      /* Initialize the CRC table and the decoding table. */
      uint32 i, j;
      uint32 c;

      tableInit = 1;
      for (i = 0; i < 256; i++)
        {
          for (c = i << 24, j = 8; j > 0; --j)
            c = c & 0x80000000 ? (c << 1) ^ 0x04c11db7 : (c << 1);
          crc32_table[i] = c;
        }
    }
#endif

  __mem_fault = 0;
  while (len--)
    {
      unsigned char ch;

#ifdef TARGET_HAS_HARVARD_MEMORY
      if (is_progmem)
	  __read_progmem_safe (&ch, (void *)ptr, 1);
      else
#endif
      __read_mem_safe (&ch, (void *)ptr, 1);
      if (__mem_fault)
        {
          break;
        }
      crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ ch) & 255];
      ptr++;
    }
  return crc;
}

/* Handle the 'q' request */

static void
process_query (char *pkt)
{
  __remcomOutBuffer[0] = '\0';
#ifdef __ECOS__
  if ('C' == pkt[0] &&
      'R' == pkt[1] &&
      'C' == pkt[2] &&
      ':' == pkt[3])
#else  // __ECOS__
  if (strncmp (pkt, "CRC:", 4) == 0)
#endif // __ECOS__
    {
      target_addr_t startmem;
      target_register_t length;
      uint32 our_crc;

      pkt += 4;
      if (__hexToAddr (&pkt, &startmem)
          && *(pkt++) == ','
          && __hexToInt (&pkt, &length))
        {
          our_crc = crc32 (startmem, length, 0xffffffff);
          if (__mem_fault)
            {
              strcpy (__remcomOutBuffer, "E01");
            }
          else
            {
              int numb = __intToHex (__remcomOutBuffer + 1, our_crc, 32);
              __remcomOutBuffer[0] = 'C';
              __remcomOutBuffer[numb + 1] = 0;
            }
        }
      return;
    }
#ifdef CYG_HAL_STUB_PROCESS_QUERY
  else if (CYG_HAL_STUB_PROCESS_QUERY (pkt, __remcomOutBuffer, sizeof(__remcomOutBuffer)))
    return;
#endif
  else
    {
      char ch ;
      char * subpkt ;
      ch = *pkt ;
      subpkt = pkt + 1 ;
      switch (ch)
        {
        case 'L' : /* threadlistquery */
          STUB_PKT_GETTHREADLIST (subpkt, __remcomOutBuffer, 300);
          break ;
        case 'P' : /* Thread or process information request */
          STUB_PKT_GETTHREADINFO (subpkt, __remcomOutBuffer, 300);
          break ;
        case 'C' : /* current thread query */
          STUB_PKT_CURRTHREAD(subpkt, __remcomOutBuffer, sizeof(__remcomOutBuffer));
          break;
        default:
          __process_target_query (pkt, __remcomOutBuffer, 300);
          break ;
        }
    }
}

/* Handle the 'Q' request */

static void
process_set (char *pkt)
{
  char ch ;
  
#ifdef CYG_HAL_STUB_PROCESS_SET
  if (CYG_HAL_STUB_PROCESS_SET (pkt, __remcomOutBuffer, sizeof(__remcomOutBuffer)))
    return;
#endif

  ch = *pkt ;
  switch (ch)
    {
    case 'p' : /* Set current process or thread */
      /* reserve the packet id even if support is not present */
      /* Dont strip the 'p' off the header, there are several variations of
         this packet */
      STUB_PKT_CHANGETHREAD (pkt, __remcomOutBuffer, 300) ;
      break ;
    default:
      __process_target_set (pkt, __remcomOutBuffer, 300);
      break ;
    }
}
#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
