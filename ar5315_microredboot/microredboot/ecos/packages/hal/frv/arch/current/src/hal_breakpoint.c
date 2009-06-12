//==========================================================================
//
//      hal_breakpoint.c
//
//      HAL breakpoint/watchpoint code for Fujitsu FR-V
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Red Hat, Inc.
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
// Author(s):    gthomas
// Contributors: gthomas
// Date:         2001-09-07
// Purpose:      HAL board support
// Description:  Hardware breakpoint/watchpoint support
//
//####DESCRIPTIONEND####
//
//========================================================================*/



// ------------------------------------------------------------------------
//
// Hardware breakpoint/watchpoint support
// ======================================
//
// Now follows a load of extreme unpleasantness to deal with the totally
// broken debug model of this device.
//
// To modify the special hardware debug registers, it is necessary to put
// the CPU into "debug mode".  This can only be done by executing a break
// instruction, or taking a special hardware break event as described by
// the special hardware debug registers.
//
// But once in debug mode, no break is taken, and break instructions are
// ignored, because we are in debug mode.
//
// So we must exit debug mode for normal running, which you can only do via
// a rett #1 instruction.  Because rett is for returning from traps, it
// halts the CPU if you do it with traps enabled.  So you have to mess
// about disabling traps before the rett.  Also, because rett #1 is for
// returning from a *debug* trap, you can only issue it from debug mode -
// or it halts the CPU.
//
// To be able to set and unset hardware debug breakpoints and watchpoints,
// we must enter debug mode (via a "break" instruction).  Fortunately, it
// is possible to return from a "break" remaining in debug mode, using a
// rett #0, so we can arrange that a break instruction just means "go to
// debug mode".
//
// So we can manipulate the special hardware debug registers by executing a
// "break", doing the work, then doing the magic sequence to rett #1.
// These are encapsulated in HAL_FRV_ENTER_DEBUG_MODE() and
// HAL_FRV_EXIT_DEBUG_MODE() from plf_stub.h
//
// So, we get into break_hander() for two reasons:
//   1) a break instruction.  Detect this and do nothing; return skipping
//      over the break instruction.  CPU remains in debug mode.
//   2) a hardware debug trap.  Continue just as for a normal exception;
//      GDB and the stubs will handle it.  But first, exit debug mode, or
//      stuff happening in the stubs will go wrong.
//
// In order to be certain that we are in debug mode, for performing (2)
// safely, vectors.S installs a special debug trap handler on vector #255.
// That's the reason for break_handler() existing as a separate routine.
// 
// Note that there is no need to define CYGSEM_HAL_FRV_HW_DEBUG for the
// FRV_FRVGEN target; while we do use Hardware Debug, we don't use *that*
// sort of hardware debug, specifically we do not use hardware single-step,
// because it breaks as soon as we exit debug mode, ie. whilst we are still
// within the stub.  So in fact defining CYGSEM_HAL_FRV_HW_DEBUG is bad; I
// guess it is mis-named.
//

// ------------------------------------------------------------------------
// First a load of ugly boilerplate for register access.
#include <cyg/hal/fr-v.h>
#include <cyg/hal/hal_stub.h>           // HAL_STUB_HW_STOP_NONE et al
#include <cyg/hal/frv_stub.h>           // register names PC, PSR et al
#include <cyg/hal/plf_stub.h>           // HAL_FRV_EXIT_DEBUG_MODE()

// First a load of glue
static inline unsigned get_bpsr(void) {
    unsigned retval;
    asm volatile ( "movsg   bpsr,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_bpsr(unsigned val) {
    asm volatile ( "movgs   %0,bpsr\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dcr(void) {
    unsigned retval;
    asm volatile ( "movsg   dcr,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dcr(unsigned val) {
    asm volatile ( "movgs   %0,dcr\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_brr(void) {
    unsigned retval;
    asm volatile ( "movsg   brr,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_brr(unsigned val) {
    asm volatile ( "movgs   %0,brr\n" : /* no outputs */  : "r" (val) );}

// Four Instruction Break Address Registers
static inline unsigned get_ibar0(void) {
    unsigned retval;
    asm volatile ( "movsg   ibar0,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_ibar0(unsigned val) {
    asm volatile ( "movgs   %0,ibar0\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_ibar1(void) {
    unsigned retval;
    asm volatile ( "movsg   ibar1,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_ibar1(unsigned val){
    asm volatile ( "movgs   %0,ibar1\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_ibar2(void) {
    unsigned retval;
    asm volatile ( "movsg   ibar2,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_ibar2(unsigned val) {
    asm volatile ( "movgs   %0,ibar2\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_ibar3(void) {
    unsigned retval;
    asm volatile ( "movsg   ibar3,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_ibar3(unsigned val){
    asm volatile ( "movgs   %0,ibar3\n" : /* no outputs */  : "r" (val) );}

// Two Data Break Address Registers
static inline unsigned get_dbar0(void) {
    unsigned retval;
    asm volatile ( "movsg   dbar0,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbar0(unsigned val){
    asm volatile ( "movgs   %0,dbar0\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dbar1(void){
    unsigned retval;
    asm volatile ( "movsg   dbar1,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbar1(unsigned val){
    asm volatile ( "movgs   %0,dbar1\n" : /* no outputs */  : "r" (val) );}

// Two times two Data Break Data Registers
static inline unsigned get_dbdr00(void){
    unsigned retval;
    asm volatile ( "movsg   dbdr00,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbdr00(unsigned val){
    asm volatile ( "movgs   %0,dbdr00\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dbdr01(void){
    unsigned retval;
    asm volatile ( "movsg   dbdr01,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbdr01(unsigned val){
    asm volatile ( "movgs   %0,dbdr01\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dbdr10(void){
    unsigned retval;
    asm volatile ( "movsg   dbdr10,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbdr10(unsigned val){
    asm volatile ( "movgs   %0,dbdr10\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dbdr11(void){
    unsigned retval;
    asm volatile ( "movsg   dbdr11,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbdr11(unsigned val){
    asm volatile ( "movgs   %0,dbdr11\n" : /* no outputs */  : "r" (val) );}

// Two times two Data Break Mask Registers
static inline unsigned get_dbmr00(void){
    unsigned retval;
    asm volatile ( "movsg   dbmr00,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbmr00(unsigned val){
    asm volatile ( "movgs   %0,dbmr00\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dbmr01(void){
    unsigned retval;
    asm volatile ( "movsg   dbmr01,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbmr01(unsigned val){
    asm volatile ( "movgs   %0,dbmr01\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dbmr10(void){
    unsigned retval;
    asm volatile ( "movsg   dbmr10,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbmr10(unsigned val){
    asm volatile ( "movgs   %0,dbmr10\n" : /* no outputs */  : "r" (val) );}

static inline unsigned get_dbmr11(void){
    unsigned retval;
    asm volatile ( "movsg   dbmr11,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_dbmr11(unsigned val){
    asm volatile ( "movgs   %0,dbmr11\n" : /* no outputs */  : "r" (val) );}

// and here's the prototype.  Which compiles, believe it or not.
static inline unsigned get_XXXX(void){
    unsigned retval;
    asm volatile ( "movsg   XXXX,%0\n" : "=r" (retval) : /* no inputs */  );
    return retval;}
static inline void set_XXXX(unsigned val){
    asm volatile ( "movgs   %0,XXXX\n" : /* no outputs */  : "r" (val) );}

// ------------------------------------------------------------------------
// This is called in the same manner as exception_handler() in hal_misc.c
// Comments compare and contrast what we do here.

static unsigned int saved_brr = 0;

void
break_handler(HAL_SavedRegisters *regs)
{
    unsigned int i, old_bpsr;

    // See if it an actual "break" instruction.
    i = get_brr();
    saved_brr |= i; // do not lose previous state
    // Acknowledge the trap, clear the "factor" (== cause)
    set_brr( 0 );

    // Now leave debug mode so that it's safe to run the stub code.

    // Unfortunately, leaving debug mode isn't a self-contained
    // operation.  The only means of doing it is with a "rett #1"
    // instruction, which will also restore the previous values of
    // the ET and S status flags.  We can massage the BPSR
    // register so that the flags keep their current values, but
    // we need to save the old one first.
    i = old_bpsr = get_bpsr ();
    i |= _BPSR_BS; // Stay in supervisor mode
    i &= ~_BPSR_BET; // Keep traps disabled
    set_bpsr (i);
    HAL_FRV_EXIT_DEBUG_MODE();

    // Only perturb this variable if stopping, not
    // just for a break instruction.
    _hal_registers = regs; 

    // Continue with the standard mechanism:
    __handle_exception();

    // Go back into debug mode.
    HAL_FRV_ENTER_DEBUG_MODE();
    // Restore the original BPSR register.
    set_bpsr (old_bpsr);
    return;
}

// ------------------------------------------------------------------------

// Now the routines to manipulate said hardware break and watchpoints.

int cyg_hal_plf_hw_breakpoint(int setflag, void *vaddr, int len)
{
    unsigned int addr = (unsigned)vaddr;
    unsigned int dcr;
    unsigned int retcode = 0;

    HAL_FRV_ENTER_DEBUG_MODE();
    dcr = get_dcr();

    // GDB manual suggests that idempotency is required, so first remove
    // any identical BP in residence.  Implements remove arm anyway.
    if ( 0 != (dcr & (_DCR_IBE0 | _DCR_IBCE0)) &&
         get_ibar0() == addr                       )
        dcr &=~(_DCR_IBE0 | _DCR_IBCE0);
    else if ( 0 != (dcr & (_DCR_IBE1 | _DCR_IBCE1)) &&
              get_ibar1() == addr                       )
        dcr &=~(_DCR_IBE1 | _DCR_IBCE1);
    else if ( 0 != (dcr & (_DCR_IBE2 | _DCR_IBCE2)) &&
              get_ibar2() == addr                       )
        dcr &=~(_DCR_IBE2 | _DCR_IBCE2);
    else if ( 0 != (dcr & (_DCR_IBE3 | _DCR_IBCE3)) &&
              get_ibar3() == addr                       )
        dcr &=~(_DCR_IBE3 | _DCR_IBCE3);
    else
        retcode = -1;
    
    if (setflag) {
        retcode = 0; // it is OK really
        if ( 0 == (dcr & (_DCR_IBE0 | _DCR_IBCE0)) ) {
	    set_ibar0(addr);
            dcr |= _DCR_IBE0;
        }
	else if ( 0 == (dcr & (_DCR_IBE1 | _DCR_IBCE1)) ) {
	    set_ibar1(addr);
            dcr |= _DCR_IBE1;
        }
	else if ( 0 == (dcr & (_DCR_IBE2 | _DCR_IBCE2)) ) {
	    set_ibar2(addr);
            dcr |= _DCR_IBE2;
        }
	else if ( 0 == (dcr & (_DCR_IBE3 | _DCR_IBCE3)) ) {
	    set_ibar3(addr);
            dcr |= _DCR_IBE3;
        }
	else
	    retcode = -1;
    }

    if ( 0 == retcode )
        set_dcr(dcr);
    HAL_FRV_EXIT_DEBUG_MODE();
    return retcode;
}

int cyg_hal_plf_hw_watchpoint(int setflag, void *vaddr, int len, int type)
{
    unsigned int addr = (unsigned)vaddr;
    unsigned int mode;
    unsigned int dcr;
    unsigned int retcode = 0;
    unsigned long long mask;
    unsigned int mask0, mask1;
    int i;

    // Check the length fits within one block.
    if ( ((~7) & (addr + len - 1)) != ((~7) & addr) )
        return -1;

    // Assuming big-endian like the platform seems to be...

    // Get masks for the 8-byte span.  00 means enabled, ff means ignore a
    // byte, which is why this looks funny at first glance.
    mask = 0x00ffffffffffffffULL >> ((len - 1) << 3);
    for (i = 0; i < (addr & 7); i++) {
        mask >>= 8;
        mask |= 0xff00000000000000ULL;
    }

    mask0 = mask >> 32;
    mask1 = mask & 0xffffffffULL;

    addr &=~7; // round to 8-byte block

    HAL_FRV_ENTER_DEBUG_MODE();
    dcr = get_dcr();

    // GDB manual suggests that idempotency is required, so first remove
    // any identical WP in residence.  Implements remove arm anyway.
    if (      0 != (dcr & (7 * _DCR_DBASE0)) &&
              get_dbar0() == addr            &&
              get_dbmr00() == mask0 && get_dbmr01() == mask1 )
        dcr &=~(7 * _DCR_DBASE0);
    else if ( 0 != (dcr & (7 * _DCR_DBASE1)) &&
              get_dbar1() == addr&&
              get_dbmr10() == mask0 && get_dbmr11() == mask1 )
        dcr &=~(7 * _DCR_DBASE1);
    else
        retcode = -1;

    if (setflag) {
        retcode = 0; // it is OK really
        if      (type == 2)       mode = 2; // break on write
        else if (type == 3)       mode = 4; // break on read
        else if (type == 4)       mode = 6; // break on any access
        else {
            mode = 0; // actually add no enable at all.
            retcode = -1;
        }
        if ( 0 == (dcr & (7 * _DCR_DBASE0)) ) {
            set_dbar0(addr);
            // Data and Mask 0,1 to zero (mask no bits/bytes)
            set_dbdr00(0); set_dbdr01(0); set_dbmr00(mask0); set_dbmr01(mask1);
            mode *= _DCR_DBASE0;
            dcr |= mode;
        }
        else if ( 0 == (dcr & (7 * _DCR_DBASE1)) ) {
            set_dbar1(addr);
            set_dbdr10(0); set_dbdr11(0); set_dbmr10(mask0); set_dbmr11(mask1);
            mode *= _DCR_DBASE1;
            dcr |= mode;
        }
        else
            retcode = -1;
    }

    if ( 0 == retcode )
        set_dcr(dcr);
    HAL_FRV_EXIT_DEBUG_MODE();
    return retcode;
}

// Return indication of whether or not we stopped because of a
// watchpoint or hardware breakpoint. If stopped by a watchpoint,
// also set '*data_addr_p' to the data address which triggered the
// watchpoint.
int cyg_hal_plf_is_stopped_by_hardware(void **data_addr_p)
{
    unsigned int brr;
    int retcode = HAL_STUB_HW_STOP_NONE;
    unsigned long long mask;

    // There was a debug event. Check the BRR for details
    brr = saved_brr;
    saved_brr = 0;

    if ( brr & (_BRR_IB0 | _BRR_IB1 | _BRR_IB2 | _BRR_IB3) ) {
        // then it was an instruction break
        retcode = HAL_STUB_HW_STOP_BREAK;
    }
    else if ( brr & (_BRR_DB0 | _BRR_DB1) ) {
        unsigned int addr, kind;
        kind = get_dcr();
        if ( brr & (_BRR_DB0) ) {
            addr = get_dbar0();
            kind &= 7 * _DCR_DBASE0;
            kind /= _DCR_DBASE0;
            mask = (((unsigned long long)get_dbmr00())<<32) | (unsigned long long)get_dbmr01();
        } else {
            addr = get_dbar1();
            kind &= 7 * _DCR_DBASE1;
            kind /= _DCR_DBASE1;
            mask = (((unsigned long long)get_dbmr10())<<32) | (unsigned long long)get_dbmr11();
        }

        if ( data_addr_p ) {
            // Scan for a zero byte in the mask - this gives the true address.
            //              0123456789abcdef
            while ( 0 != (0xff00000000000000LLU & mask) ) {
                mask <<= 8;
                addr++;
            }
            *data_addr_p = (void *)addr;
        }

        // Inverse of the mapping above in the "set" code.
        if      (kind == 2)       retcode = HAL_STUB_HW_STOP_WATCH;
        else if (kind == 6)       retcode = HAL_STUB_HW_STOP_AWATCH;
        else if (kind == 4)       retcode = HAL_STUB_HW_STOP_RWATCH;
    }
    return retcode;
}

// ------------------------------------------------------------------------

