/* i386_stub.c - helper functions for stub, generic to all i386 processors
 * 
 * Copyright (c) 1998,1999, 2001 Cygnus Solutions
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */

/*
- pjo, 29 sep 1999
- Copied from the ARM configuration and merged with an older GDB i386-stub.c.
*/

#include <stddef.h>

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#ifdef CYGPKG_HAL_I386_SIM
#error "GDB Stub support not implemented for i386 SIM"
#endif

#include <cyg/hal/hal_stub.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/i386_stub.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#ifdef CYGDBG_HAL_DEBUG_GDB_THREAD_SUPPORT
#include <cyg/hal/dbg-threads-api.h>    // dbg_currthread_id
#endif

// We need a local memcpy so we don't rely on libc.
static inline void*
memcpy(void* dest, void* src, int size)
{
    unsigned char* __d = (unsigned char*) dest;
    unsigned char* __s = (unsigned char*) src;
    
    while(size--)
        *__d++ = *__s++;

    return dest;
}


#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

/* Given a trap value TRAP, return the corresponding signal. */

int __computeSignal (unsigned int trap_number)
{
    switch (trap_number)
	{
		case CYGNUM_HAL_VECTOR_DIV0:
			/* This isn't quite accurrate: this is integer division only. */
			return SIGFPE ;

		case CYGNUM_HAL_VECTOR_DEBUG:
			return SIGTRAP ;

		case CYGNUM_HAL_VECTOR_NMI:
			return SIGINT ;

		case CYGNUM_HAL_VECTOR_BREAKPOINT:
			return SIGTRAP ;

		case CYGNUM_HAL_VECTOR_OVERFLOW:
		case CYGNUM_HAL_VECTOR_BOUND:
			return SIGSEGV ;

		case CYGNUM_HAL_VECTOR_OPCODE:
			return SIGILL ;

		case CYGNUM_HAL_VECTOR_NO_DEVICE:
		case CYGNUM_HAL_VECTOR_FPE:
			return SIGFPE ;

		case CYGNUM_HAL_VECTOR_DOUBLE_FAULT:
			return SIGTRAP ;

		case CYGNUM_HAL_VECTOR_INVALID_TSS:
		case CYGNUM_HAL_VECTOR_SEGV:
		case CYGNUM_HAL_VECTOR_STACK_FAULT:
		case CYGNUM_HAL_VECTOR_PROTECTION:
		case CYGNUM_HAL_VECTOR_PAGE:
		case CYGNUM_HAL_VECTOR_ALIGNMENT:
			return SIGSEGV ;

		default:
			return SIGTRAP;
    }
}


/* Return the trap number corresponding to the last-taken trap. */

int __get_trap_number (void)
{
#if 1
    // The vector is not not part of the GDB register set so get it
    // directly from the HAL saved context.
    return _hal_registers->vector;
#else
	extern int hal_pc_trap_number ;
    // The vector is not not part of the GDB register set so get it
    // directly from the save context.
    return hal_pc_trap_number ;
#endif        
}

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)
int __is_bsp_syscall(void) 
{
    return __get_trap_number() == 0x80;
}
#endif

/* Set the currently-saved pc register value to PC. */

void set_pc (target_register_t pc)
{
    put_register (PC, pc);
}

static target_register_t
reg_offset(regnames_t reg)
{
    switch(reg) {
      case EAX ... GS:
	return reg * 4;
#ifdef CYGHWR_HAL_I386_FPU
      case REG_FST0 ... REG_FST7:
	return (target_register_t)&((GDB_SavedRegisters *)0)->st0[0]
	    + ((reg - REG_FST0) * 10);
      case REG_FCTRL:
	return (target_register_t)&((GDB_SavedRegisters *)0)->fcw;
      case REG_FSTAT:
	return (target_register_t)&((GDB_SavedRegisters *)0)->fsw;
      case REG_FTAG:
	return (target_register_t)&((GDB_SavedRegisters *)0)->ftw;
      case REG_FISEG:
      case REG_FOP:
	// REG_FISEG is lsw, REG_FOP is msw
	return (target_register_t)&((GDB_SavedRegisters *)0)->cssel;
      case REG_FIOFF:
	return (target_register_t)&((GDB_SavedRegisters *)0)->ipoff;
      case REG_FOSEG:
	return (target_register_t)&((GDB_SavedRegisters *)0)->opsel;
      case REG_FOOFF:
	return (target_register_t)&((GDB_SavedRegisters *)0)->dataoff;
#endif
#if 0  // GDB never asks for MMX regs directly, but it did...
      case REG_MMX0 ... REG_MMX7:
	  {
	      target_register_t tos = (get_register (REG_FSTAT) >> 11) & 7;
	      return reg_offset((((8 - tos) + reg - REG_MMX0) & 7) + REG_FST0);
	  }
#endif
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
      case REG_XMM0 ... REG_XMM7:
	return (target_register_t)&((GDB_SavedRegisters *)0)->xmm0[0]
	    + ((reg - REG_XMM0) * 16);
      case REG_MXCSR:
	return (target_register_t)&((GDB_SavedRegisters *)0)->mxcsr;
#endif
#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
      case REG_CR0 ... REG_CR4:
	return (target_register_t)&((GDB_SavedRegisters *)0)->cr0
	    + ((reg - REG_CR0) * 4);
      case REG_GDT:
	return (target_register_t)&((GDB_SavedRegisters *)0)->gdtr[0];
      case REG_IDT:
	return (target_register_t)&((GDB_SavedRegisters *)0)->idtr[0];
#endif
      default:
	return -1;
    }
    return -1;
}


// Return the currently-saved value corresponding to register REG of
// the exception context.
target_register_t 
get_register (regnames_t reg)
{
    target_register_t val;
    target_register_t offset = reg_offset(reg);

    if (REGSIZE(reg) > sizeof(target_register_t) || offset == -1)
	return -1;

    val = _registers[offset/sizeof(target_register_t)];

#ifdef CYGHWR_HAL_I386_FPU
    if (reg == REG_FISEG)
        val &= 0xffff;
    else if (reg == REG_FOP)
        val = (val >> 16) & 0xffff;
#endif

    return val;
}

// Store VALUE in the register corresponding to WHICH in the exception
// context.
void 
put_register (regnames_t which, target_register_t value)
{
    target_register_t index;
    target_register_t offset = reg_offset(which);

    if (REGSIZE(which) > sizeof(target_register_t) || offset == -1)
	return;

    index = offset / sizeof(target_register_t);

    switch (which) {
#ifdef CYGHWR_HAL_I386_FPU
      case REG_FISEG:
	value = (_registers[index] & 0xffff0000) | (value & 0xffff);
	break;
      case REG_FOP:
	value = (_registers[index] & 0x0000ffff) | (value << 16);
	break;
#endif
#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
      case REG_CR0:
	value &= REG_CR0_MASK;
	break;
      case REG_CR2:
	value &= REG_CR2_MASK;
	break;
      case REG_CR3:
	value &= REG_CR3_MASK;
	break;
      case REG_CR4:
	value &= REG_CR4_MASK;
	break;
#endif
      default:
	break;
    }
    _registers[index] = value;
}

#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
// Handle the Model Specific Registers
static target_register_t _msrval[2];
static int _which_msr = 0;
static int _dummy_flag = 0;

extern void * volatile __mem_fault_handler;

static void
__do_read_msr (void)
{
    // _dummy_flag is always false but the goto is necessary to keep
    // some compilers from reordering stuff across the 'err' label.
    if (_dummy_flag)
	goto err;

    __mem_fault = 1;                      // Defaults to 'fail'. Is cleared
                                          // when the wrmsr completes.
    __mem_fault_handler = &&err;

    asm volatile ("movl %2,%%ecx\n"
		  "rdmsr\n"
		  "movl %%edx,%1\n"
		  "movl %%eax,%0\n"
		  : "=m" (_msrval[0]), "=m" (_msrval[1])
		  : "m" (_which_msr)
		  : "ecx", "ebx", "edx", "eax", "memory");

    __mem_fault = 0;

 err:
    __mem_fault_handler = (void *)0;
}

static void
__do_write_msr (void)
{
    // _dummy_flag is always false but the goto is necessary to keep
    // some compilers from reordering stuff across the 'err' label.
    if (_dummy_flag)
	goto err;

    __mem_fault = 1;                      // Defaults to 'fail'. Is cleared
                                          // when the wrmsr completes.
    __mem_fault_handler = &&err;

    asm volatile ("movl %1,%%edx\n"
		  "movl %0,%%eax\n"
		  "movl %2,%%ecx\n"
		  "wrmsr\n"
		  : /* no outputs */
		  : "m" (_msrval[0]), "m" (_msrval[1]), "m" (_which_msr)
		  : "ecx", "ebx", "edx", "eax", "memory");

    __mem_fault = 0;

 err:
    __mem_fault_handler = (void *)0;
}

static int
rdmsr (int msrnum, target_register_t *msrval)
{
    _which_msr = msrnum;
    __set_mem_fault_trap (__do_read_msr);
    if (__mem_fault)
	return 0;

    msrval[0] = _msrval[0];
    msrval[1] = _msrval[1];
    return 1;
}

static int
wrmsr (int msrnum, target_register_t *msrval)
{
    _which_msr = msrnum;
    _msrval[0] = msrval[0];
    _msrval[1] = msrval[1];

    __set_mem_fault_trap (__do_write_msr);
    if (__mem_fault)
	return 0;

    return 1;
}

int
cyg_hal_stub_process_query (char *pkt, char *buf, int bufsize)
{
    unsigned long val1, val2, val3, val4;
    int i = 0, max_input = 0;

    if ('C' == pkt[0] &&
	'P' == pkt[1] &&
	'U' == pkt[2] &&
	'I' == pkt[3] &&
	'D' == pkt[4]) {

	for (i = 0; i <= max_input; i++) {

	    asm volatile ("movl %4,%%eax\n"
			  "cpuid\n"
			  "movl %%eax,%0\n"
			  "movl %%ebx,%1\n"
			  "movl %%ecx,%2\n"
			  "movl %%edx,%3\n"
			  : "=m" (val1), "=m" (val2), "=m" (val3), "=m" (val4)
			  : "m" (i)
			  : "eax", "ebx", "ecx", "edx", "memory");

	    /*
	     * get the max value to use to get all the CPUID info.
	     */
	    if (i == 0)
		max_input = val1;

	    /*
	     * Swap the bytes around to handle endianness conversion:
	     * ie 12345678 --> 78563412
	     */
	    val1 = (((val1 & 0x000000ff) << 24) | ((val1 & 0x0000ff00) <<  8) |
		    ((val1 & 0x00ff0000) >>  8) | ((val1 & 0xff000000) >> 24));
	    val2 = (((val2 & 0x000000ff) << 24) | ((val2 & 0x0000ff00) <<  8) |
		    ((val2 & 0x00ff0000) >>  8) | ((val2 & 0xff000000) >> 24));
	    val3 = (((val3 & 0x000000ff) << 24) | ((val3 & 0x0000ff00) <<  8) |
		    ((val3 & 0x00ff0000) >>  8) | ((val3 & 0xff000000) >> 24));
	    val4 = (((val4 & 0x000000ff) << 24) | ((val4 & 0x0000ff00) <<  8) |
		    ((val4 & 0x00ff0000) >>  8) | ((val4 & 0xff000000) >> 24));

	    /*
	     * Generate the packet
	     */
	    __mem2hex ((char *)&val1, buf, 8, 0);  buf[8] = ',';  buf += 9;
	    __mem2hex ((char *)&val2, buf, 8, 0);  buf[8] = ',';  buf += 9;
	    __mem2hex ((char *)&val3, buf, 8, 0);  buf[8] = ',';  buf += 9;
	    __mem2hex ((char *)&val4, buf, 8, 0);  buf[8] = ';';  buf += 9;
        }

	/*
	 * The packet is complete.  buf points just past the final semicolon.
	 * Remove that semicolon and properly terminate the packet.
	 */
	*(buf - 1) = '\0';

	return 1;
    }

    if ('M' == pkt[0] &&
	'S' == pkt[1] &&
	'R' == pkt[2]) {

	pkt += 4;
	if (__hexToInt (&pkt, &val1)) {
	    target_register_t msrval[2];

	    // rdmsr implicitly sets _which_msr for subsequent REG_MSR read/write.
	    if (rdmsr(val1, msrval))
		__mem2hex ((char*)msrval, buf, 8, 0);
	    else
		memcpy (buf, "INVALID", 8);
        }
	return 1;
    }

    return 0;
}
#endif // CYGHWR_HAL_I386_PENTIUM_GDB_REGS

// Write the contents of register WHICH into VALUE as raw bytes. We
// only support this for the MMX, FPU, and SSE registers.
// Return true if it is a valid register.
int
get_register_as_bytes (regnames_t which, char *value)
{
    target_register_t offset;

#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
    // Read the currently selected MSR
    if (which == REG_MSR) {
	if (rdmsr(_which_msr, _msrval)) {
	    memcpy (value, _msrval, REGSIZE(which));
	    return 1;
	}
	return 0;
    }
    // We can't actually read the LDT or TR in software, so just return invalid.
    if (which == REG_LDT || which == REG_TR)
	return 0;

    if (which == REG_GDT || which == REG_IDT) {
        // GDB requires these to be sent base first though the CPU stores them
	// limit first in 6 bytes. Weird.
        offset = reg_offset(which);
        memcpy (value, (char *)_registers + offset + 2, 4);
        memcpy (value + 4, (char *)_registers + offset, 2);
        return 1;
    }
#endif

    offset = reg_offset(which);
    if (offset != -1) {
	memcpy (value, (char *)_registers + offset, REGSIZE(which));
	return 1;
    }
    return 0;
}

// Alter the contents of saved register WHICH to contain VALUE.  We only
// support this for the MMX, FPU, and SSE registers.
int
put_register_as_bytes (regnames_t which, char *value)
{
    target_register_t offset;

#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
    // Write the currently selected MSR
    if (which == REG_MSR)
	return wrmsr(_which_msr, (target_register_t*)value);

    // We can't actually write the LDT OR TR in software, so just return invalid.
    if (which == REG_LDT || which == REG_TR)
	return 0;
    if (which == REG_GDT || which == REG_IDT) {
        // GDB sends these base first, though the CPU stores them
	// limit first. Weird.
        offset = reg_offset(which);
        memcpy ((char *)_registers + offset + 2, value, 4);
        memcpy ((char *)_registers + offset, value + 4, 2);
        return 1;
    }
#endif

    offset = reg_offset(which);
    if (offset != -1) {
	memcpy ((char *)_registers + offset, value, REGSIZE(which));
	return 1;
    }
    return 0;
}

/*----------------------------------------------------------------------
 * Single-step support
 */

/* Set things up so that the next user resume will execute one instruction.
   This may be done by setting breakpoints or setting a single step flag
   in the saved user registers, for example. */


/* We just turn on the trap bit in the flags register for the particular
	thread.  When it resumes, we'll get another debugger trap.
*/
void __single_step (void)
{	put_register(PS, get_register(PS) | PS_T) ;
}

/* Clear the single-step state. */

void __clear_single_step (void)
{	put_register(PS, get_register(PS) & ~PS_T) ;
}

void __install_breakpoints (void)
{
//    FIXME();
}

void __clear_breakpoints (void)
{
    __clear_breakpoint_list();
}

/* If the breakpoint we hit is in the breakpoint() instruction, return a
   non-zero value. */

int isBreakpointFunction ;

int
__is_breakpoint_function ()
{
    isBreakpointFunction = (get_register (PC) == (target_register_t)&_breakinst);
    return isBreakpointFunction ;
}


/* Skip the current instruction.  Since this is only called by the
   stub when the PC points to a breakpoint or trap instruction,
   we can safely just skip 4. */

void __skipinst (void)
{
//	FIXME() ;
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

//----------------------------------------------------------------------
// We apparently need these function even when no stubinclude
// for Thread Debug purposes


void hal_get_gdb_registers(CYG_ADDRWORD *dest, HAL_SavedRegisters * s)
{
    GDB_SavedRegisters *d = (GDB_SavedRegisters *)dest;

    d->eax = s->eax;
    d->ebx = s->ebx;
    d->ecx = s->ecx;
    d->edx = s->edx;
    d->ebp = s->ebp;
    d->esp = s->esp;
    d->edi = s->edi;
    d->esi = s->esi;
    d->pc = s->pc;
    d->cs = s->cs;
    d->ps = s->eflags;

    d->ss = 0;
    asm volatile ("movw %%ss,%0\n" :"=m" (d->ss));
    d->ds = 0;
    asm volatile ("movw %%ds,%0\n" :"=m" (d->ds));
    d->es = 0;
    asm volatile ("movw %%es,%0\n" :"=m" (d->es));
    d->fs = 0;
    asm volatile ("movw %%fs,%0\n" :"=m" (d->fs));
    d->gs = 0;
    asm volatile ("movw %%gs,%0\n" :"=m" (d->gs));

#ifdef CYGHWR_HAL_I386_FPU
#ifdef CYGHWR_HAL_I386_FPU_SWITCH_LAZY
    asm volatile ("fnop\n");  // force state save
    memcpy(&d->fcw, &s->fpucontext->fpstate[0], sizeof(s->fpucontext->fpstate));
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
    memcpy(&d->xmm0[0], &s->fpucontext->xmm0[0], (16 * 8) + 4);
#endif
#else
    memcpy(&d->fcw, &s->fpucontext.fpstate[0], sizeof(s->fpucontext.fpstate));
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
    memcpy(&d->xmm0[0], &s->fpucontext.xmm0[0], (16 * 8) + 4);
#endif
#endif
#endif

#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
    {
	unsigned long val;

	asm volatile ("movl %%cr0,%0\n"
		      "movl %0,%1\n"
		      : "=r" (val), "=m" (d->cr0));
	asm volatile ("movl %%cr2,%0\n"
		      "movl %0,%1\n"
		      : "=r" (val), "=m" (d->cr2));
	asm volatile ("movl %%cr3,%0\n"
		      "movl %0,%1\n"
		      : "=r" (val), "=m" (d->cr3));
	asm volatile ("movl %%cr4,%0\n"
		      "movl %0,%1\n"
		      : "=r" (val), "=m" (d->cr4));
	asm volatile ("sgdt %0\n" :"=m" (d->gdtr));
	asm volatile ("sidt %0\n" :"=m" (d->idtr));
    }
#endif // CYGHWR_HAL_I386_PENTIUM_GDB_REGS
}

void hal_set_gdb_registers(HAL_SavedRegisters * d, CYG_ADDRWORD * src)
{
    GDB_SavedRegisters *s = (GDB_SavedRegisters *)src;

    d->eax = s->eax;
    d->ebx = s->ebx;
    d->ecx = s->ecx;
    d->edx = s->edx;
    d->ebp = s->ebp;
    d->esp = s->esp;
    d->edi = s->edi;
    d->esi = s->esi;
    d->pc = s->pc;
    d->cs = s->cs;
    d->eflags = s->ps;
#ifdef CYGHWR_HAL_I386_FPU
#ifdef CYGHWR_HAL_I386_FPU_SWITCH_LAZY
    memcpy(&d->fpucontext->fpstate[0], &s->fcw, sizeof(d->fpucontext->fpstate));
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
    memcpy(&d->fpucontext->xmm0[0], &s->xmm0[0], (16 * 8) + 4);
#endif
#else
    memcpy(&d->fpucontext.fpstate[0], &s->fcw, sizeof(d->fpucontext.fpstate));
#ifdef CYGHWR_HAL_I386_PENTIUM_SSE
    memcpy(&d->fpucontext.xmm0[0], &s->xmm0[0], (16 * 8) + 4);
#endif
#endif
#endif
#ifdef CYGHWR_HAL_I386_PENTIUM_GDB_REGS
    {
	unsigned long val;

	val = s->cr0;
	asm volatile ("movl %0,%%cr0\n" : : "r" (val));
	val = s->cr2;
	asm volatile ("movl %0,%%cr2\n" : : "r" (val));
	val = s->cr3;
	asm volatile ("movl %0,%%cr3\n" : : "r" (val));
	val = s->cr4;
	asm volatile ("movl %0,%%cr4\n" : : "r" (val));
    }
#endif // CYGHWR_HAL_I386_PENTIUM_GDB_REGS
}


//----------------------------------------------------------------------
// End
