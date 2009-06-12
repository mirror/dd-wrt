//=============================================================================
//
//      hal_stub.c
//
//      Helper functions for stub, specific to eCos HAL
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   jskov (based on powerpc/cogent hal_stub.c)
// Contributors:jskov, dmoseley
// Date:        1999-02-12
// Purpose:     Helper functions for stub, specific to eCos HAL
// Description: Parts of the GDB stub requirements are provided by
//              the eCos HAL, rather than target and/or board specific
//              code. 
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#ifdef CYGPKG_CYGMON
#include <pkgconf/cygmon.h>
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include <cyg/hal/hal_stub.h>           // Our header

#include <cyg/hal/hal_arch.h>           // HAL_BREAKINST
#include <cyg/hal/hal_cache.h>          // HAL_xCACHE_x
#include <cyg/hal/hal_intr.h>           // interrupt disable/restore

#include <cyg/hal/hal_if.h>             // ROM calling interface
#include <cyg/hal/hal_misc.h>           // Helper functions

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

#ifdef USE_LONG_NAMES_FOR_ENUM_REGNAMES
#ifndef PC
#define PC REG_PC
#endif
#ifndef SP
#define SP REG_SP
#endif
#endif

//-----------------------------------------------------------------------------
// Extra eCos data.

// Some architectures use registers of different sizes, so NUMREGS
// alone is not suffucient to size the register save area. For those
// architectures, HAL_STUB_REGISTERS_SIZE is defined as the number
// of target_register_t sized elements in the register save area.
#ifndef HAL_STUB_REGISTERS_SIZE
#define HAL_STUB_REGISTERS_SIZE NUMREGS
#endif

// Saved registers.
HAL_SavedRegisters *_hal_registers;
target_register_t registers[HAL_STUB_REGISTERS_SIZE];
target_register_t alt_registers[HAL_STUB_REGISTERS_SIZE] ;  // Thread or saved process state
target_register_t * _registers = registers;                 // Pointer to current set of registers
#ifndef CYGPKG_REDBOOT
target_register_t orig_registers[HAL_STUB_REGISTERS_SIZE];  // Registers to get back to original state
#endif

#if defined(HAL_STUB_HW_WATCHPOINT) || defined(HAL_STUB_HW_BREAKPOINT)
static int  _hw_stop_reason;   // Reason we were stopped by hw.

//#define HAL_STUB_HW_SEND_STOP_REASON_TEXT
#ifdef CYGINT_HAL_ARM_ARCH_XSCALE
#define HAL_STUB_HW_SEND_STOP_REASON_TEXT
#endif

#ifdef HAL_STUB_HW_SEND_STOP_REASON_TEXT
// strings indexed by hw stop reasons defined in hal_stub.h

// Not all GDBs understand this.
static const char * const _hw_stop_str[] = {
    "",
    "hbreak",
    "watch",
    "rwatch",
    "awatch"
};
#endif // HAL_STUB_HW_SEND_STOP_REASON_TEXT

static void *_watch_data_addr; // The data address if stopped by watchpoint
#endif // defined(HAL_STUB_HW_WATCHPOINT) || defined(HAL_STUB_HW_BREAKPOINT)

// Register validity checking
#ifdef CYGHWR_REGISTER_VALIDITY_CHECKING
int registers_valid[NUMREGS];
int *_registers_valid = registers_valid;
#endif

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT // this should go away
// Interrupt control.
static volatile __PFI __interruptible_control;
#endif

// Some architectures need extras regs reported in T packet
#ifndef HAL_STUB_ARCH_T_PACKET_EXTRAS
#define HAL_STUB_ARCH_T_PACKET_EXTRAS(x)
#endif

//-----------------------------------------------------------------------------
// Register access

#ifndef CYGARC_STUB_REGISTER_ACCESS_DEFINED
// Return the currently-saved value corresponding to register REG of
// the exception context.
target_register_t 
get_register (regnames_t reg)
{
    return _registers[reg];
}
#endif

#ifdef CYGHWR_REGISTER_VALIDITY_CHECKING
// Return the validity of register REG.
int
get_register_valid (regnames_t reg)
{
    return _registers_valid[reg];
}
#endif

#ifndef CYGARC_STUB_REGISTER_ACCESS_DEFINED
// Store VALUE in the register corresponding to WHICH in the exception
// context.
void 
put_register (regnames_t which, target_register_t value)
{
#ifdef CYGPKG_HAL_MIPS_VR4300
    // This is a rather nasty kludge to compensate for the fact that
    // the VR4300 GDB is rather old and does not support proper 64 bit
    // registers. The only time this really matters is when setting
    // the PC after loading an executable. So here we detect this case
    // and artificially sign extend it. 
    
    if( which == PC && (value & 0x0000000080000000ULL ) )
    {
        value |= 0xFFFFFFFF00000000ULL;
    }
#endif    
    _registers[which] = value;
}
#endif // CYGARC_STUB_REGISTER_ACCESS_DEFINED

//-----------------------------------------------------------------------------
// Serial stuff
#ifdef CYGPKG_CYGMON
extern void ecos_bsp_console_putc(char);
extern char ecos_bsp_console_getc(void);
#endif

// Write C to the current serial port.
void 
putDebugChar (int c)
{
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    __call_if_debug_procs_t __debug_procs = CYGACC_CALL_IF_DEBUG_PROCS();
    CYGACC_COMM_IF_PUTC(*__debug_procs, c);
#elif defined(CYGPKG_CYGMON)
    ecos_bsp_console_putc(c);
#else
    HAL_STUB_PLATFORM_PUT_CHAR(c);
#endif
}

// Read one character from the current serial port.
int 
getDebugChar (void)
{
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    __call_if_debug_procs_t __debug_procs = CYGACC_CALL_IF_DEBUG_PROCS();
    return CYGACC_COMM_IF_GETC(*__debug_procs);
#elif defined(CYGPKG_CYGMON)
    return ecos_bsp_console_getc();
#else
    return HAL_STUB_PLATFORM_GET_CHAR();
#endif
}

// Flush output channel
void
hal_flush_output(void)
{
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    __call_if_debug_procs_t __debug_procs = CYGACC_CALL_IF_DEBUG_PROCS();
    CYGACC_COMM_IF_CONTROL(*__debug_procs, __COMMCTL_FLUSH_OUTPUT);
#endif
}


// Set the baud rate for the current serial port.
void 
__set_baud_rate (int baud) 
{
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    __call_if_debug_procs_t __debug_procs = CYGACC_CALL_IF_DEBUG_PROCS();
    CYGACC_COMM_IF_CONTROL(*__debug_procs, __COMMCTL_SETBAUD, baud);
#elif defined(CYGPKG_CYGMON)
    // FIXME!
#else
    HAL_STUB_PLATFORM_SET_BAUD_RATE(baud);
#endif
}

//-----------------------------------------------------------------------------
// GDB interrupt (BREAK) support.

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT

#ifndef CYGPKG_HAL_ARM

#if (HAL_BREAKINST_SIZE == 1)
typedef cyg_uint8 t_inst;
#elif (HAL_BREAKINST_SIZE == 2)
typedef cyg_uint16 t_inst;
#elif (HAL_BREAKINST_SIZE == 4)
typedef cyg_uint32 t_inst;
#else
#error "Don't know how to handle that size"
#endif

typedef struct
{
  t_inst *targetAddr;
  t_inst savedInstr;
} instrBuffer;

static instrBuffer break_buffer;

volatile int cyg_hal_gdb_running_step = 0;

void 
cyg_hal_gdb_place_break (target_register_t pc)
{
    cyg_hal_gdb_interrupt( pc ); // Let's hope this becomes a drop-through:
}

void 
cyg_hal_gdb_interrupt (target_register_t pc)
{
    t_inst break_inst = HAL_BREAKINST;

    CYGARC_HAL_SAVE_GP();

    // Clear flag that we Continued instead of Stepping
    cyg_hal_gdb_running_step = 0;
    // and override existing break? So that a ^C takes effect...
    if (NULL != break_buffer.targetAddr)
        cyg_hal_gdb_remove_break( (target_register_t)break_buffer.targetAddr );

    if (NULL == break_buffer.targetAddr) {
	// Not always safe to read/write directly to program
	// memory due to possibly unaligned instruction, use the
	// provided memory functions instead.
	__read_mem_safe(&break_buffer.savedInstr, (t_inst*)pc, HAL_BREAKINST_SIZE);
	__write_mem_safe(&break_inst, (t_inst*)pc, HAL_BREAKINST_SIZE);

	// Save the PC where we put the break, so we can remove
	// it after the target takes the break.
	break_buffer.targetAddr = (t_inst*)pc;

        __data_cache(CACHE_FLUSH);
        __instruction_cache(CACHE_FLUSH);
    }

    CYGARC_HAL_RESTORE_GP();
}

int 
cyg_hal_gdb_remove_break (target_register_t pc)
{
    if ( cyg_hal_gdb_running_step )
        return 0;

    if ((t_inst*)pc == break_buffer.targetAddr) {

	__write_mem_safe(&break_buffer.savedInstr, (t_inst*)pc, HAL_BREAKINST_SIZE);
        break_buffer.targetAddr = NULL;

        __data_cache(CACHE_FLUSH);
        __instruction_cache(CACHE_FLUSH);
        return 1;
    }
    return 0;
}

int 
cyg_hal_gdb_break_is_set (void)
{
    if (NULL != break_buffer.targetAddr) {
        return 1;
    }
    return 0;
}

#endif // CYGPKG_HAL_ARM

#endif // CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT

// Use this function to disable serial interrupts whenever reply
// characters are expected from GDB.  The reason we want to control
// whether the target can be interrupted or not is simply that GDB on
// the host will be sending acknowledge characters/commands while the
// stub is running - if serial interrupts were still active, the
// characters would never reach the (polling) getDebugChar.
static void
interruptible(int state)
{
    static int __interrupts_suspended = 0;

    if (state) {
        __interrupts_suspended--;
        if (0 >= __interrupts_suspended) {
            __interrupts_suspended = 0;
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT // this _check_ should go away
            {
                hal_virtual_comm_table_t* __chan;
                __chan = CYGACC_CALL_IF_DEBUG_PROCS();
                CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_ENABLE);
            }
#else                
            if (__interruptible_control)
                __interruptible_control(1);
#endif
        }
    } else {
        __interrupts_suspended++;
        if (1 == __interrupts_suspended)
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT // this _check_ should go away
            {
                hal_virtual_comm_table_t* __chan;
                __chan = CYGACC_CALL_IF_DEBUG_PROCS();
                CYGACC_COMM_IF_CONTROL(*__chan, __COMMCTL_IRQ_DISABLE);
            }
#else                
            if (__interruptible_control)
                __interruptible_control(0);
#endif
    }
}

//-----------------------------------------------------------------------------
// eCos stub entry and exit magic.

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
int cyg_hal_gdb_break;
#endif

#ifdef CYGPKG_REDBOOT
// Trampoline for returning to RedBoot from exception/stub code
static void
return_from_stub(int exit_status)
{
    CYGACC_CALL_IF_MONITOR_RETURN(exit_status);
}
#endif

// Called at stub *kill*
static void 
handle_exception_exit( void )
{
#ifdef CYGPKG_REDBOOT
#ifdef CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    {   // Reset the timer to default and cancel any callback
	extern void sys_profile_reset(void);
	sys_profile_reset();
    }
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS_GPROF
    set_pc((target_register_t)return_from_stub);
#else
    int i;

    for (i = 0; i < (sizeof(registers)/sizeof(registers[0])); i++)
	registers[i] = orig_registers[i];
#endif
}

// Called at stub *entry*
static void 
handle_exception_cleanup( void )
{
#ifndef CYGPKG_REDBOOT
    static int orig_registers_set = 0;
#endif

    interruptible(0);

    // Expand the HAL_SavedRegisters structure into the GDB register
    // array format.
    HAL_GET_GDB_REGISTERS(&registers[0], _hal_registers);
    _registers = &registers[0];

#ifndef CYGPKG_REDBOOT
    if (!orig_registers_set) {
	int i;
	for (i = 0; i < (sizeof(registers)/sizeof(registers[0])); i++)
	    orig_registers[i] = registers[i];
	_registers = &orig_registers[0];
	if (__is_breakpoint_function ())
	    __skipinst ();
	_registers = &registers[0];
	orig_registers_set = 1;
    }
#endif
	
#ifdef HAL_STUB_PLATFORM_STUBS_FIXUP
    // Some architectures may need to fix the PC in case of a partial
    // or fully executed trap instruction. GDB only takes correct action
    // when the PC is pointing to the breakpoint instruction it set.
    // 
    // Most architectures would leave PC pointing at the trap
    // instruction itself though, and so do not need to do anything
    // special.
    HAL_STUB_PLATFORM_STUBS_FIXUP();
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
    // If we continued instead of stepping, when there was a break set
    // ie. we were stepping within a critical region, clear the break, and
    // that flag.  If we stopped for some other reason, this has no effect.
    if ( cyg_hal_gdb_running_step ) {
        cyg_hal_gdb_running_step = 0;
        cyg_hal_gdb_remove_break(get_register (PC));
    }

    // FIXME: (there may be a better way to do this)
    // If we hit a breakpoint set by the gdb interrupt stub, make it
    // seem like an interrupt rather than having hit a breakpoint.
    cyg_hal_gdb_break = cyg_hal_gdb_remove_break(get_register (PC));
#endif

#if defined(HAL_STUB_HW_WATCHPOINT) || defined(HAL_STUB_HW_BREAKPOINT)
    // For HW watchpoint/breakpoint support, we need to know if we
    // stopped because of watchpoint or hw break. We do that here
    // before GDB has a chance to remove the watchpoints and save
    // the information for later use in building response packets.
    _hw_stop_reason = HAL_STUB_IS_STOPPED_BY_HARDWARE(_watch_data_addr);
#endif    
}

// Called at stub *exit*
static void 
handle_exception_init( void )
{
    // Compact register array again.
    HAL_SET_GDB_REGISTERS(_hal_registers, &registers[0]);

    interruptible(1);
}


//-----------------------------------------------------------------------------
// Initialization.

// Signal handler.
int 
cyg_hal_process_signal (int signal)
{
    // We don't care about the signal (atm).
    return 0;
}

// Install the standard set of trap handlers for the stub.
void 
__install_traps (void)
{
    // Set signal handling vector so we can treat 'C<signum>' as 'c'.
    __process_signal_vec = &cyg_hal_process_signal;
    __process_exit_vec = &handle_exception_exit;

    __cleanup_vec = &handle_exception_cleanup;
    __init_vec    = &handle_exception_init;

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT // this should go away
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
    // Control of GDB interrupts.
    __interruptible_control = HAL_STUB_PLATFORM_INTERRUPTIBLE;
#endif
#endif

    // Nothing further to do, handle_exception will be called when an
    // exception occurs.
}

// Initialize the hardware.
void 
initHardware (void) 
{
    static int initialized = 0;

    if (initialized)
        return;
    initialized = 1;

    // Get serial port initialized.
    HAL_STUB_PLATFORM_INIT_SERIAL();

#ifdef HAL_STUB_PLATFORM_INIT
    // If the platform defines any initialization code, call it here.
    HAL_STUB_PLATFORM_INIT();
#endif        

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT // this should go away
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
    // Get interrupt handler initialized.
    HAL_STUB_PLATFORM_INIT_BREAK_IRQ();
#endif
#endif // !CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
}

// Reset the board.
void 
__reset (void)
{
#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    __call_if_reset_t *__rom_reset = CYGACC_CALL_IF_RESET_GET();
    if (__rom_reset)
        (*__rom_reset)();
#else
    HAL_PLATFORM_RESET();
#endif
}

//-----------------------------------------------------------------------------
// Breakpoint support.

#ifndef CYGPKG_HAL_ARM
// This function will generate a breakpoint exception.  It is used at
// the beginning of a program to sync up with a debugger and can be
// used otherwise as a quick means to stop program execution and
// "break" into the debugger.
void
breakpoint()
{
    HAL_BREAKPOINT(_breakinst);
}

// This function returns the opcode for a 'trap' instruction.
unsigned long 
__break_opcode ()
{
  return HAL_BREAKINST;
}
#endif

//-----------------------------------------------------------------------------
// Write the 'T' packet in BUFFER. SIGVAL is the signal the program received.
void 
__build_t_packet (int sigval, char *buf)
{
    target_register_t addr;
    char *ptr = buf;
    target_register_t extend_val = 0;

    *ptr++ = 'T';
    *ptr++ = __tohex (sigval >> 4);
    *ptr++ = __tohex (sigval);

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
    // Include thread ID if thread manipulation is required.
    {
        int id = dbg_currthread_id ();

        if (id != 0) {
	    *ptr++ = 't';
	    *ptr++ = 'h';
	    *ptr++ = 'r';
	    *ptr++ = 'e';
	    *ptr++ = 'a';
	    *ptr++ = 'd';
	    *ptr++ = ':';

#if (CYG_BYTEORDER == CYG_LSBFIRST)
	    // FIXME: Temporary workaround for PR 18903. Thread ID must be
	    // big-endian in the T packet.
	    {
		unsigned char* bep = (unsigned char*)&id;
		int be_id;

		be_id = id;
		*bep++ = (be_id >> 24) & 0xff ;
		*bep++ = (be_id >> 16) & 0xff ;
		*bep++ = (be_id >> 8) & 0xff ;
		*bep++ = (be_id & 0xff) ;
	    }
#endif
	    ptr = __mem2hex((char *)&id, ptr, sizeof(id), 0);
	    *ptr++ = ';';
	}
    }
#endif

#ifdef HAL_STUB_HW_WATCHPOINT
    switch(_hw_stop_reason) {
      case HAL_STUB_HW_STOP_WATCH:
      case HAL_STUB_HW_STOP_RWATCH:
      case HAL_STUB_HW_STOP_AWATCH:
#ifdef HAL_STUB_HW_SEND_STOP_REASON_TEXT
        // Not all GDBs understand this.
	strcpy(ptr, _hw_stop_str[_hw_stop_reason]);
	ptr += strlen(_hw_stop_str[_hw_stop_reason]);
#endif
	*ptr++ = ':';
	// Send address MSB first
	ptr += __intToHex(ptr, (target_register_t)_watch_data_addr,
			  sizeof(_watch_data_addr) * 8);
	*ptr++ = ';';
	break;
      default:
	break;
    }
#endif

    *ptr++ = __tohex (PC >> 4);
    *ptr++ = __tohex (PC);
    *ptr++ = ':';
    addr = get_register (PC);
    if (sizeof(addr) < REGSIZE(PC))
    {
        // GDB is expecting REGSIZE(PC) number of bytes.
        // We only have sizeof(addr) number.  Let's fill
        // the appropriate number of bytes intelligently.
#ifdef CYGARC_SIGN_EXTEND_REGISTERS
        {
            unsigned long bits_in_addr = (sizeof(addr) << 3);  // ie Size in bytes * 8
            target_register_t sign_bit_mask = (1 << (bits_in_addr - 1));
            if ((addr & sign_bit_mask) == sign_bit_mask)
                extend_val = ~0;
        }
#endif
    }
#if (CYG_BYTEORDER == CYG_MSBFIRST)
    ptr = __mem2hex((char *)&extend_val, ptr, REGSIZE(PC) - sizeof(addr), 0);
#endif
    ptr = __mem2hex((char *)&addr, ptr, sizeof(addr), 0);
#if (CYG_BYTEORDER == CYG_LSBFIRST)
    ptr = __mem2hex((char *)&extend_val, ptr, REGSIZE(PC) - sizeof(addr), 0);
#endif
    *ptr++ = ';';

    *ptr++ = __tohex (SP >> 4);
    *ptr++ = __tohex (SP);
    *ptr++ = ':';
    addr = (target_register_t) get_register (SP);
    if (sizeof(addr) < REGSIZE(SP))
    {
        // GDB is expecting REGSIZE(SP) number of bytes.
        // We only have sizeof(addr) number.  Let's fill
        // the appropriate number of bytes intelligently.
        extend_val = 0;
#ifdef CYGARC_SIGN_EXTEND_REGISTERS
        {
            unsigned long bits_in_addr = (sizeof(addr) << 3);  // ie Size in bytes * 8
            target_register_t sign_bit_mask = (1 << (bits_in_addr - 1));
            if ((addr & sign_bit_mask) == sign_bit_mask)
                extend_val = ~0;
        }
#endif
        ptr = __mem2hex((char *)&extend_val, ptr, REGSIZE(SP) - sizeof(addr), 0);
    }
    ptr = __mem2hex((char *)&addr, ptr, sizeof(addr), 0);
    *ptr++ = ';';

    HAL_STUB_ARCH_T_PACKET_EXTRAS(ptr);
    
    *ptr++ = 0;
}


//-----------------------------------------------------------------------------
// Cache functions.

// Perform the specified operation on the instruction cache. 
// Returns 1 if the cache is enabled, 0 otherwise.
int 
__instruction_cache (cache_control_t request)
{
    int state = 1;

    switch (request) {
    case CACHE_ENABLE:
        HAL_ICACHE_ENABLE();
        break;
    case CACHE_DISABLE:
        HAL_ICACHE_DISABLE();
        state = 0;
        break;
    case CACHE_FLUSH:
        HAL_ICACHE_SYNC();
        break;
    case CACHE_NOOP:
        /* fall through */
    default:
        break;
    }

#ifdef HAL_ICACHE_IS_ENABLED
    HAL_ICACHE_IS_ENABLED(state);
#endif

    return state;
}

// Perform the specified operation on the data cache. 
// Returns 1 if the cache is enabled, 0 otherwise.
int 
__data_cache (cache_control_t request)
{
    int state = 1;

    switch (request) {
    case CACHE_ENABLE:
        HAL_DCACHE_ENABLE();
        break;
    case CACHE_DISABLE:
        HAL_DCACHE_DISABLE();
        state = 0;
        break;
    case CACHE_FLUSH:
        HAL_DCACHE_SYNC();
        break;
    case CACHE_NOOP:
        /* fall through */
    default:
        break;
    }
#ifdef HAL_DCACHE_IS_ENABLED
    HAL_DCACHE_IS_ENABLED(state);
#endif

    return state;
}

//-----------------------------------------------------------------------------
// Memory accessor functions.

// The __mem_fault_handler pointer is volatile since it is only
// set/cleared by the function below - which does not rely on any
// other functions, so the compiler may decide to not bother updating
// the pointer at all. If any of the memory accesses cause an
// exception, the pointer must be set to ensure the exception handler
// can make use of it.

void* volatile __mem_fault_handler = (void *)0;

/* These are the "arguments" to __do_read_mem and __do_write_mem, 
   which are passed as globals to avoid squeezing them thru
   __set_mem_fault_trap.  */

static volatile target_register_t memCount;

static void
__do_copy_mem (unsigned char* src, unsigned char* dst)
{
    unsigned long *long_dst;
    unsigned long *long_src;
    unsigned short *short_dst;
    unsigned short *short_src;

    // Zero memCount is not really an error, but the goto is necessary to
    // keep some compilers from reordering stuff across the 'err' label.
    if (memCount == 0) goto err;

    __mem_fault = 1;                      /* Defaults to 'fail'. Is cleared */
                                          /* when the copy loop completes.  */
    __mem_fault_handler = &&err;

    // See if it's safe to do multi-byte, aligned operations
    while (memCount) {
        if ((memCount >= sizeof(long)) &&
            (((target_register_t)dst & (sizeof(long)-1)) == 0) &&
            (((target_register_t)src & (sizeof(long)-1)) == 0)) {
        
            long_dst = (unsigned long *)dst;
            long_src = (unsigned long *)src;

            *long_dst++ = *long_src++;
            memCount -= sizeof(long);

            dst = (unsigned char *)long_dst;
            src = (unsigned char *)long_src;
        } else if ((memCount >= sizeof(short)) &&
                   (((target_register_t)dst & (sizeof(short)-1)) == 0) &&
                   (((target_register_t)src & (sizeof(short)-1)) == 0)) {
            
            short_dst = (unsigned short *)dst;
            short_src = (unsigned short *)src;

            *short_dst++ = *short_src++;
            memCount -= sizeof(short);

            dst = (unsigned char *)short_dst;
            src = (unsigned char *)short_src;
        } else {
            *dst++ = *src++;
            memCount--;
        }
    }

    __mem_fault = 0;

 err:
    __mem_fault_handler = (void *)0;
}

/*
 * __read_mem_safe:
 * Get contents of target memory, abort on error.
 */

int
__read_mem_safe (void *dst, void *src, int count)
{
  if( !CYG_HAL_STUB_PERMIT_DATA_READ( src, count ) )
    return 0;

  memCount = count;
  __do_copy_mem((unsigned char*) src, (unsigned char*) dst);
  return count - memCount;      // return number of bytes successfully read
}

/*
 * __write_mem_safe:
 * Set contents of target memory, abort on error.
 */

int
__write_mem_safe (void *src, void *dst, int count)
{
  if( !CYG_HAL_STUB_PERMIT_DATA_READ( dst, count ) )
    return 0;

  memCount = count;
  __do_copy_mem((unsigned char*) src, (unsigned char*) dst);
  return count - memCount;      // return number of bytes successfully written
}

#ifdef TARGET_HAS_HARVARD_MEMORY
static void
__do_copy_from_progmem (unsigned char* src, unsigned char* dst)
{
    unsigned long *long_dst;
    unsigned long *long_src;
    unsigned short *short_dst;
    unsigned short *short_src;

    // Zero memCount is not really an error, but the goto is necessary to
    // keep some compilers from reordering stuff across the 'err' label.
    if (memCount == 0) goto err;

    __mem_fault = 1;                      /* Defaults to 'fail'. Is cleared */
                                          /* when the copy loop completes.  */
    __mem_fault_handler = &&err;

    // See if it's safe to do multi-byte, aligned operations
    while (memCount) {
        if ((memCount >= sizeof(long)) &&
            (((target_register_t)dst & (sizeof(long)-1)) == 0) &&
            (((target_register_t)src & (sizeof(long)-1)) == 0)) {
        
            long_dst = (unsigned long *)dst;
            long_src = (unsigned long *)src;

            *long_dst++ = __read_prog_uint32(long_src++);
            memCount -= sizeof(long);

            dst = (unsigned char *)long_dst;
            src = (unsigned char *)long_src;
        } else if ((memCount >= sizeof(short)) &&
                   (((target_register_t)dst & (sizeof(short)-1)) == 0) &&
                   (((target_register_t)src & (sizeof(short)-1)) == 0)) {
            
            short_dst = (unsigned short *)dst;
            short_src = (unsigned short *)src;

            *short_dst++ = __read_prog_uint16(short_src++);
            memCount -= sizeof(short);

            dst = (unsigned char *)short_dst;
            src = (unsigned char *)short_src;
        } else {
            *dst++ = __read_prog_uint8(src++);
            memCount--;
        }
    }

    __mem_fault = 0;

 err:
    __mem_fault_handler = (void *)0;
}

static void
__do_copy_to_progmem (unsigned char* src, unsigned char* dst)
{
    unsigned long *long_dst;
    unsigned long *long_src;
    unsigned short *short_dst;
    unsigned short *short_src;

    // Zero memCount is not really an error, but the goto is necessary to
    // keep some compilers from reordering stuff across the 'err' label.
    if (memCount == 0)	goto err;

    __mem_fault = 1;                      /* Defaults to 'fail'. Is cleared */
                                          /* when the copy loop completes.  */
    __mem_fault_handler = &&err;

    // See if it's safe to do multi-byte, aligned operations
    while (memCount) {
        if ((memCount >= sizeof(long)) &&
            (((target_register_t)dst & (sizeof(long)-1)) == 0) &&
            (((target_register_t)src & (sizeof(long)-1)) == 0)) {
        
            long_dst = (unsigned long *)dst;
            long_src = (unsigned long *)src;

            __write_prog_uint32(long_dst++, *long_src++);
            memCount -= sizeof(long);

            dst = (unsigned char *)long_dst;
            src = (unsigned char *)long_src;
        } else if ((memCount >= sizeof(short)) &&
                   (((target_register_t)dst & (sizeof(short)-1)) == 0) &&
                   (((target_register_t)src & (sizeof(short)-1)) == 0)) {
            
            short_dst = (unsigned short *)dst;
            short_src = (unsigned short *)src;

            __write_prog_uint16(short_dst++, *short_src++);
            memCount -= sizeof(short);

            dst = (unsigned char *)short_dst;
            src = (unsigned char *)short_src;
        } else {
            __write_prog_uint8(dst++, *src++);
            memCount--;
        }
    }

    __mem_fault = 0;

 err:
    __mem_fault_handler = (void *)0;
}

/*
 * __read_progmem_safe:
 * Get contents of target memory, abort on error.
 */

int
__read_progmem_safe (void *dst, void *src, int count)
{
  if( !CYG_HAL_STUB_PERMIT_CODE_READ( src, count ) )
    return 0;

  memCount = count;
  __do_copy_from_progmem((unsigned char*) src, (unsigned char*) dst);
  return count - memCount;      // return number of bytes successfully read
}

/*
 * __write_progmem_safe:
 * Set contents of target memory, abort on error.
 */

int
__write_progmem_safe (void *src, void *dst, int count)
{
  if( !CYG_HAL_STUB_PERMIT_CODE_WRITE( dst, count ) )
    return 0;

  memCount = count;
  __do_copy_to_progmem((unsigned char*) src, (unsigned char*) dst);
  return count - memCount;      // return number of bytes successfully written
}
#endif

//-----------------------------------------------------------------------------
// Target extras?!
int 
__process_target_query(char * pkt, char * out, int maxOut)
{ return 0 ; }
int 
__process_target_set(char * pkt, char * out, int maxout)
{ return 0 ; }
int 
__process_target_packet(char * pkt, char * out, int maxout)
{ return 0 ; }

// GDB string output, making sure interrupts are disabled.
// This function gets used by some diag output functions.
void 
hal_output_gdb_string(target_register_t str, int string_len)
{
    unsigned long __state;
    HAL_DISABLE_INTERRUPTS(__state);
    __output_gdb_string(str, string_len);
    HAL_RESTORE_INTERRUPTS(__state);
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
