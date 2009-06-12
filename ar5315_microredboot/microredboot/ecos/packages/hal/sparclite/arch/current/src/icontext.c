/*=============================================================================
//
//      icontext.c
//
//      SPARClite HAL context init function
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   hmt
// Contributors:        hmt
// Date:        1998-12-14
// Purpose:     HAL context initialization function
// Description: Initialize a HAL context for SPARClite; this is in C and out
//              of line because there is too much of it for a simple macro.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>           // HAL header

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/vectors.h>            // SAVE_REGS_SIZE, __WINSIZE, ...

/*---------------------------------------------------------------------------*/

/* We lay out the stack in the manner that the PCS demands:
 *        frame pointer -----> [top of stack] 
 *                             Argument spill area (6 words)
 *                             Return Arg pointer
 *                             Initial saved register window (i[8], l[8])
 *                                for use by program when it starts
 *                             [rest of] saved register object (various)
 *        stack pointer -----> saved register window (i[8], l[8])
 *                                to allow us to be interrupted.
 *
 * ie.    frame pointer ->
 *                          struct HAL_FrameStructure
 *        stack pointer ->  struct HAL_SavedRegisters
 *
 * and when the context "resumes" sp is incremented by 40 * 4, the size of
 * a  _struct HAL_SavedRegisters_  which points it at the extant but unused
 *  _struct HAL_FrameStructure_  as the PCS requires.  The frame pointer is
 * left pointing off the top of stack.
 *
 *
 * Thus the stack is the same if created from an already executing context:
 *
 *        frame pointer -----> 
 *                             [temporaries and locals]
 *                             [more arguments]
 *                             Argument spill area (6 words)
 *                             Return Arg pointer
 *        [sp at entry]------> Previous saved register window (i[8], l[8])
 *                                for use by program when it starts
 *                             [rest of] saved register object (various)
 *        stack pointer -----> saved register window (i[8], l[8])
 *                                to allow us to be interrupted.
 */

CYG_ADDRESS
hal_thread_init_context(  CYG_WORD sparg,
                          CYG_WORD thread,
                          CYG_WORD entry,
                          CYG_WORD id )
{
    register CYG_WORD fp = sparg;
    register CYG_WORD sp = 0;
    register HAL_SavedRegisters *regs;
    register HAL_FrameStructure *frame;
    int i;

    if ( 0 == (id & 0xffff0000) )
        id <<= 16;

    fp &= ~15;                          // round down to double alignment

    frame = (HAL_FrameStructure *)(
        fp - sizeof( HAL_FrameStructure ) );
    
    regs = (HAL_SavedRegisters *)(
        ((CYG_WORD)frame) - sizeof(HAL_SavedRegisters) );

    sp = (CYG_WORD)regs;
    
    for ( i = 0; i < 6; i++ ) {
        frame->spill_args[i] = id | 0xa0 | i;
    }    
    frame->composite_return_ptr = 0;

    for ( i = 0; i < 8; i++ ) {
        frame->li.i[i]   = id | ( 56 + i );
        frame->li.l[i]   = id | ( 48 + i );
        regs ->li.i[i]   = id | ( 24 + i );
        regs ->li.l[i]   = id | ( 16 + i );
        regs ->o[i]      = id | (  8 + i );
        regs ->g[i]      = id | (      i );
    }

    // first terminate the linked list on the stack in the initial
    // (already saved) register window:
    frame->li.i[6] = regs->li.i[6] = (cyg_uint32)fp; // frame pointer
    frame->li.i[7] = regs->li.i[7] = (cyg_uint32)0;  // no ret addr here

    // and set up other saved regs as if called from just before
    // the entry point:
    regs->o[7] = (entry - 8);
    regs->o[6] = sp;

    // this is the argument that the entry point is called with
    regs->o[0] = thread;

    // this is the initial CWP and interrupt state; CWP is quite arbitrary
    // really, the WIM is set up accordingly in hal_thread_load_context().

    regs->g[0] = 0x0e0 + __WIN_INIT; // PIL zero, ET, S, PS and CWP set.

    return (CYG_ADDRESS)sp;
}

// ---------------------------------------------------------------------------

//#define THREAD_DEBUG_SERIAL_VERBOSE

#ifdef THREAD_DEBUG_SERIAL_VERBOSE         // NOT INCLUDED

// This is unpleasant to try to debug, because these routines are called
// WHEN THE PROGRAM IS NOT RUNNING from the CygMon's GDB stubs - so you
// can't use any normal output: these little routines use the serial
// line directly, so are best used when debugging via Ethernet, so you
// just have a separate output stream to read.  Nasty...

#include <cyg/hal/hal_diag.h>

#undef  HAL_DIAG_WRITE_CHAR
#define HAL_DIAG_WRITE_CHAR(_c_) CYG_MACRO_START                    \
    SLEB_LED = (_c_);                                               \
    HAL_DIAG_WRITE_CHAR_DIRECT( _c_ );                              \
CYG_MACRO_END

static void swritec( char c )
{
    HAL_DIAG_WRITE_CHAR( c );
}

static void swrites( char *s )
{
    char c;
    while ( 0 != (c = *s++) )
        HAL_DIAG_WRITE_CHAR( c );
}

static void swritex( cyg_uint32 x )
{
    int i;
    swrites( "0x" );
    for ( i = 28; i >= 0; i-= 4 ) {
        char c = "0123456789abcdef"[ 0xf & (x >> i) ];
        HAL_DIAG_WRITE_CHAR( c );
    }
}

#define newline() swrites( "\n\r" )

static void x8( char *s, unsigned long *xp )
{
    int i;
    for ( i = 0; i < 8; i++ ) {
        swrites( s );
        swritec( '0' + i );
        swrites( " = " );
        swritex( xp[i] );
        if ( 3 == (i & 3) )
            newline();
        else
            swrites( "    " );
    }
}

#endif // THREAD_DEBUG_SERIAL_VERBOSE ... NOT INCLUDED

// ---------------------------------------------------------------------------
// Routines in icontext.c used here because they're quite large for
// the SPARClite (note param order); these are used in talking to GDB.

enum regnames {G0 = 0, G1, G2, G3, G4, G5, G6, G7,
               O0, O1, O2, O3, O4, O5, SP, O7,
               L0, L1, L2, L3, L4, L5, L6, L7,
               I0, I1, I2, I3, I4, I5, FP, I7,

               F0, F1, F2, F3, F4, F5, F6, F7,
               F8, F9, F10, F11, F12, F13, F14, F15,
               F16, F17, F18, F19, F20, F21, F22, F23,
               F24, F25, F26, F27, F28, F29, F30, F31,
               Y, PSR, WIM, TBR, PC, NPC, FPSR, CPSR};

typedef unsigned long target_register_t;

void
cyg_hal_sparc_get_gdb_regs( void *gdb_regset,
                            HAL_SavedRegisters *eCos_regset )
{
    target_register_t *gdb = (target_register_t *)gdb_regset;
    int reg;
    cyg_uint32 scratch = 0;
    cyg_uint32 *sptrap;
    HAL_SavedWindow *trapli, *ctxli;

    if ( 0 == eCos_regset->g[0]             ||
         0xc0 == (0xe0 & eCos_regset->g[0])    ) {
        // Then it's an interrupt stack saved state:
        // (either minimal, or a saved PSR with traps disabled)
        // The saved register set is pretty minimal, so we have to grub
        // around in the stack to find out some truth...
        sptrap = (cyg_uint32 *)eCos_regset; // point to the IL save area for
        sptrap -= 24;                   // the trap handler, for PC, NPC
        trapli = (HAL_SavedWindow *)sptrap; // Get at those regs

        ctxli = (HAL_SavedWindow *)(trapli->i[6]); // (the frame pointer)
                                        // and get at the interruptee's regs

        // Pick up interruptee's registers from all over the stack:
        for ( reg = 0; reg < 8 ; reg++ ) {
            gdb[ G0 + reg ] = eCos_regset->g[reg];
            gdb[ O0 + reg ] = trapli->i[reg];
            gdb[ L0 + reg ] = ctxli->l[reg];
            gdb[ I0 + reg ] = ctxli->i[reg];
        }
    
        // Clear out G0 which is always 0 (but abused in eCos_regset)
        // and the FP regs which we do not have:
        gdb[ G0 ] = 0;
        for ( reg = F0; reg <= F31; reg++ )
            gdb[ reg ] = 0;
    
        // In the save context _of the trap handler_ registers are as follows:
        // %l0 = psr (with this CWP/window-level in it)
        // %l1 = pc
        // %l2 = npc
        // %l3 = vector number (1-15 for interrupts)
        // %l4 = Y register preserved
        gdb[ Y ]    = trapli->l[4];
        
        scratch = trapli->l[0];         // the PSR in the trap handler
#if 8 == __WINSIZE
        scratch++;                      // back to interupted thread's window
        scratch &=~ 0x38;               // clear ET and any __WINSIZE overflow
        gdb[ PSR ]  = scratch;
        gdb[ WIM ]  = 1 << ((__WINBITS & (1 + scratch)));
#else  // 6 or 7 windows only
        reg = (int)(scratch & __WINBITS);
        scratch &=~ (__WINBITS_MAXIMAL | 0x20); // clear ET and  CWP
        if ( __WINSIZE <= ++reg ) reg = 0; // back to intr'd window
        gdb[ PSR ]  = scratch | reg;
        if ( __WINSIZE <= ++reg ) reg = 0; // good index for WIM
        gdb[ WIM ]  = 1 << reg;
#endif // __WINSIZE
        
        // Read _a_ TBR value and ignore the current trap details:
        asm volatile ( "rd %%tbr, %0" : "=r"(scratch) : );
        gdb[ TBR ]  = (scratch &~ 0xfff);
        
        gdb[ PC ]   = trapli->l[1];
        gdb[ NPC ]  = trapli->l[2];
    
        gdb[ FPSR ] = 0;
        gdb[ CPSR ] = 0;

#ifdef THREAD_DEBUG_SERIAL_VERBOSE
        newline();
        swrites( "-----------------------------------------------------" ); newline();
        swrites( "-------------- INTERRUPT STACK GET ------------------" ); newline();
        swrites( "eCos regset at " ); swritex( eCos_regset ); newline();
        swrites( "        trapli " ); swritex( trapli      ); newline();
        swrites( "         ctxli " ); swritex(  ctxli      ); newline();
        x8( "global ", &(gdb[G0]) );
        x8( "    in ", &(gdb[I0]) );
        x8( " local ", &(gdb[L0]) );
        x8( "   out ", &(gdb[O0]) );
        swrites( "gdb  PC = " ); swritex( gdb[  PC ] ); newline();
        swrites( "gdb NPC = " ); swritex( gdb[ NPC ] ); newline();
        swrites( "gdb PSR = " ); swritex( gdb[ PSR ] ); newline();
#endif

    }
    else {
        // It's a synchronous context switch that led to this object.
        // Pick up interruptee's registers from the saved context:
        for ( reg = 0; reg < 8 ; reg++ ) {
#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
            gdb[ G0 + reg ] = 0;
            gdb[ O0 + reg ] = 0;
#else
            gdb[ G0 + reg ] = eCos_regset->g[reg];
            gdb[ O0 + reg ] = eCos_regset->o[reg];
#endif
            gdb[ L0 + reg ] = eCos_regset->li.l[reg];
            gdb[ I0 + reg ] = eCos_regset->li.i[reg];
        }

#ifdef CYGDBG_HAL_COMMON_CONTEXT_SAVE_MINIMUM
        // Set up the stack pointer by arithmetic and the return address
        gdb[ SP ] = ((cyg_uint32)(eCos_regset));
        gdb[ O7 ] = eCos_regset->o[ 7 ];
#else
        // Clear out G0 which is always 0 (but abused in eCos_regset)
        gdb[ G0 ] = 0;
#endif
        // and clear the FP regs which we do not have:
        for ( reg = F0; reg <= F31; reg++ )
            gdb[ reg ] = 0;
    
        gdb[ Y ]    = 0;                // it's not preserved.
        
        scratch = eCos_regset->g[ 0 ];  // the PSR in the saved context
        gdb[ PSR ]  = scratch;          // return it verbatim.
#if 8 == __WINSIZE
        gdb[ WIM ]  = 1 << ((__WINBITS & (1 + scratch)));
#else  // 6 or 7 windows only
        reg = (int)(scratch & __WINBITS);
        if ( __WINSIZE <= ++reg ) reg = 0; // good index for WIM
        gdb[ WIM ]  = 1 << reg;
#endif // __WINSIZE
    
        // Read _a_ TBR value and ignore the current trap details:
        asm volatile ( "rd %%tbr, %0" : "=r"(scratch) : );
        gdb[ TBR ]  = (scratch &~ 0xfff);
        
        gdb[ PC ]   = eCos_regset->o[ 7 ]; // the return address
        gdb[ NPC ]  = 4 + gdb[ PC ];
    
        gdb[ FPSR ] = 0;
        gdb[ CPSR ] = 0;

#ifdef THREAD_DEBUG_SERIAL_VERBOSE
        newline();
        swrites( "-----------------------------------------------------" ); newline();
        swrites( "-------------- SYNCHRONOUS SWITCH GET----------------" ); newline();
        swrites( "eCos regset at " ); swritex( eCos_regset ); newline();
        x8( "global ", &(gdb[G0]) );
        x8( "    in ", &(gdb[I0]) );
        x8( " local ", &(gdb[L0]) );
        x8( "   out ", &(gdb[O0]) );
        swrites( "gdb  PC = " ); swritex( gdb[  PC ] ); newline();
        swrites( "gdb NPC = " ); swritex( gdb[ NPC ] ); newline();
        swrites( "gdb PSR = " ); swritex( gdb[ PSR ] ); newline();
#endif
    }

}

// ---------------------------------------------------------------------------

void
cyg_hal_sparc_set_gdb_regs( HAL_SavedRegisters *eCos_regset,
                            void *gdb_regset )
{
    target_register_t *gdb = (target_register_t *)gdb_regset;
    int reg;
    cyg_uint32 scratch = 0;
    cyg_uint32 *sptrap;
    HAL_SavedWindow *trapli, *ctxli;

    // Guess where the eCos register set really is:
    if ( 0 == eCos_regset->g[0]             ||
         0xc0 == (0xe0 & eCos_regset->g[0])    ) {
        // Then it's an interrupt stack saved state:
        // (either minimal, or a saved PSR with traps disabled)
        // The saved register set is pretty minimal, so we have to grub
        // around in the stack to find out some truth...
        sptrap = (cyg_uint32 *)eCos_regset; // point to the IL save area for
        sptrap -= 24;                   // the trap handler, for PC, NPC
        trapli = (HAL_SavedWindow *)sptrap; // Get at those regs

        ctxli = (HAL_SavedWindow *)(trapli->i[6]); // (the frame pointer)
                                        // and get at the interruptee's regs

        scratch = eCos_regset->g[0];

        // Put back interruptee's registers all over the stack:
        for ( reg = 0; reg < 8 ; reg++ ) {
            eCos_regset->g[reg] = gdb[ G0 + reg ] ;
            trapli->i[reg]      = gdb[ O0 + reg ] ;
            ctxli->l[reg]       = gdb[ L0 + reg ] ;
            ctxli->i[reg]       = gdb[ I0 + reg ] ;
        }
    
        // Put back the eCos G0 which is always 0 (but abused in eCos_regset)
        eCos_regset->g[0] = scratch;

        // In the save context _of the trap handler_ registers are as follows:
        // %l0 = psr (with this CWP/window-level in it)
        // %l1 = pc
        // %l2 = npc
        // %l3 = vector number (1-15 for interrupts)
        // %l4 = Y register preserved
        trapli->l[4] = gdb[ Y ];
        
        // I am *not* interfering with the saved PSR, nor the TBR nor WIM.
        
        // put back return PC and NPC
        trapli->l[1] = gdb[ PC ] ;
        trapli->l[2] = gdb[ NPC ];
    
#ifdef THREAD_DEBUG_SERIAL_VERBOSE
        newline();
        swrites( "-----------------------------------------------------" ); newline();
        swrites( "-------------- INTERRUPT STACK SET ------------------" ); newline();
        swrites( "eCos regset at " ); swritex( eCos_regset ); newline();
        swrites( "        trapli " ); swritex( trapli      ); newline();
        swrites( "         ctxli " ); swritex(  ctxli      ); newline();
        x8( "global ", &(gdb[G0]) );
        x8( "    in ", &(gdb[I0]) );
        x8( " local ", &(gdb[L0]) );
        x8( "   out ", &(gdb[O0]) );
        swrites( "gdb  PC = " ); swritex( gdb[  PC ] ); newline();
        swrites( "gdb NPC = " ); swritex( gdb[ NPC ] ); newline();
        swrites( "gdb PSR = " ); swritex( gdb[ PSR ] ); newline();
#endif

    }
    else {
        // It's a synchronous context switch that led to this object.
        // Pick up interruptee's registers from the saved context:

        scratch = eCos_regset->g[0];

        for ( reg = 0; reg < 8 ; reg++ ) {
            eCos_regset->g[reg]    = gdb[ G0 + reg ];
            eCos_regset->o[reg]    = gdb[ O0 + reg ];
            eCos_regset->li.l[reg] = gdb[ L0 + reg ];
            eCos_regset->li.i[reg] = gdb[ I0 + reg ];
        }

        // Put back the eCos G0 which is always 0 (but abused in eCos_regset)
        eCos_regset->g[0] = scratch;
        
        // I am *not* interfering with the saved PSR, nor the TBR nor WIM.

        // The PC is in o7, altering it via GDB's PC is not on.
        // Setting the NPC in a voluntary context is meaningless.

#ifdef THREAD_DEBUG_SERIAL_VERBOSE
        newline();
        swrites( "-----------------------------------------------------" ); newline();
        swrites( "-------------- SYNCHRONOUS SWITCH SET ---------------" ); newline();
        swrites( "eCos regset at " ); swritex( eCos_regset ); newline();
        x8( "global ", &(gdb[G0]) );
        x8( "    in ", &(gdb[I0]) );
        x8( " local ", &(gdb[L0]) );
        x8( "   out ", &(gdb[O0]) );
        swrites( "gdb  PC = " ); swritex( gdb[  PC ] ); newline();
        swrites( "gdb NPC = " ); swritex( gdb[ NPC ] ); newline();
        swrites( "gdb PSR = " ); swritex( gdb[ PSR ] ); newline();
#endif
    }

}

/*---------------------------------------------------------------------------*/
// EOF icontext.c
