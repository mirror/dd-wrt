//==========================================================================
//
//      mipsfp.c
//
//      HAL miscellaneous functions
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
// Author(s):    jlarmour
// Contributors: jlarmour
// Date:         1999-07-13
// Purpose:      Emulate unimplemented FP operations on MIPS architectures
// Description:  This catches the unimplemented operation excetion only,
//               and if possible deals with it so processing can continue
//               as if the MIPS had a proper IEEE FPU
//               
//
//####DESCRIPTIONEND####
//
//========================================================================*/

// CONFIGURATION

#include <pkgconf/hal.h>

#ifdef CYGHWR_HAL_MIPS_FPU

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Standard eCos types
#include <cyg/infra/cyg_ass.h>     // Standard eCos assertion support
#include <cyg/infra/cyg_trac.h>    // Standard eCos tracing support
#include <cyg/hal/hal_intr.h>      // HAL interrupt vectors
#include <cyg/hal/hal_arch.h>      // Architecture types such as
                                   // HAL_SavedRegisters
#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <cyg/hal/mips-regs.h>     // MIPS register and bitmask definitions

// TYPES

// The following types were taken from <sys/ieeefp.h> from libm.

#if (CYG_BYTEORDER == CYG_MSBFIRST) // Big endian

typedef union 
{
    cyg_int32 asi32[2];

    cyg_int64 asi64;
    
    double value;
    
    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST)
        unsigned int sign : 1;
        unsigned int exponent: 11;
        unsigned int fraction0:4;
        unsigned int fraction1:16;
        unsigned int fraction2:16;
        unsigned int fraction3:16;
#else
        unsigned int fraction2:16;
        unsigned int fraction3:16;
        unsigned int sign : 1;
        unsigned int exponent: 11;
        unsigned int fraction0:4;
        unsigned int fraction1:16;
#endif        
    } number;
    
    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST)
        unsigned int sign : 1;
        unsigned int exponent: 11;
        unsigned int quiet:1;
        unsigned int function0:3;
        unsigned int function1:16;
        unsigned int function2:16;
        unsigned int function3:16;
#else
        unsigned int function2:16;
        unsigned int function3:16;
        unsigned int sign : 1;
        unsigned int exponent: 11;
        unsigned int quiet:1;
        unsigned int function0:3;
        unsigned int function1:16;
#endif
    } nan;
    
    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST)
        cyg_uint32 msw;
        cyg_uint32 lsw;
#else
        cyg_uint32 lsw;
        cyg_uint32 msw;
#endif
    } parts;

    
} Cyg_libm_ieee_double_shape_type;


typedef union
{
    cyg_int32 asi32;
    
    float value;

    struct 
    {
        unsigned int sign : 1;
        unsigned int exponent: 8;
        unsigned int fraction0: 7;
        unsigned int fraction1: 16;
    } number;

    struct 
    {
        unsigned int sign:1;
        unsigned int exponent:8;
        unsigned int quiet:1;
        unsigned int function0:6;
        unsigned int function1:16;
    } nan;
    
} Cyg_libm_ieee_float_shape_type;


#else // Little endian

typedef union 
{
    cyg_int32 asi32[2];

    cyg_int64 asi64;
    
    double value;

    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST) // Big endian
        unsigned int fraction1:16;
        unsigned int fraction0: 4;
        unsigned int exponent :11;
        unsigned int sign     : 1;
        unsigned int fraction3:16;
        unsigned int fraction2:16;
#else
        unsigned int fraction3:16;
        unsigned int fraction2:16;
        unsigned int fraction1:16;
        unsigned int fraction0: 4;
        unsigned int exponent :11;
        unsigned int sign     : 1;
#endif
    } number;

    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST) // Big endian
        unsigned int function1:16;
        unsigned int function0:3;
        unsigned int quiet:1;
        unsigned int exponent: 11;
        unsigned int sign : 1;
        unsigned int function3:16;
        unsigned int function2:16;
#else
        unsigned int function3:16;
        unsigned int function2:16;
        unsigned int function1:16;
        unsigned int function0:3;
        unsigned int quiet:1;
        unsigned int exponent: 11;
        unsigned int sign : 1;
#endif
    } nan;

    struct 
    {
#if (CYG_DOUBLE_BYTEORDER == CYG_MSBFIRST) // Big endian
        cyg_uint32 msw;
        cyg_uint32 lsw;
#else
        cyg_uint32 lsw;
        cyg_uint32 msw;
#endif
    } parts;
    
} Cyg_libm_ieee_double_shape_type;


typedef union
{
    cyg_int32 asi32;
  
    float value;

    struct 
    {
        unsigned int fraction0: 7;
        unsigned int fraction1: 16;
        unsigned int exponent: 8;
        unsigned int sign : 1;
    } number;

    struct 
    {
        unsigned int function1:16;
        unsigned int function0:6;
        unsigned int quiet:1;
        unsigned int exponent:8;
        unsigned int sign:1;
    } nan;

} Cyg_libm_ieee_float_shape_type;

#endif // little-endian

typedef enum {
    ADD_INSN=0,       // 0
    SUB_INSN,
    MUL_INSN,
    DIV_INSN,
    SQRT_INSN,
    ABS_INSN,         // 5
    MOV_INSN,
    NEG_INSN,
    ROUNDL_INSN,
    TRUNCL_INSN,
    CEILL_INSN,       // 10
    FLOORL_INSN,
    ROUNDW_INSN,
    TRUNCW_INSN,
    CEILW_INSN,
    FLOORW_INSN,      // 15
    // ...
    CVTS_INSN=32,
    CVTD_INSN,
    // ...
    CVTW_INSN=36,
    CVTL_INSN,
    // ...
    // 
    // 48-63 are floating point compare - treated separately
    
} fp_operation;

typedef enum {
    S_FORMAT=16,
    D_FORMAT=17,
    W_FORMAT=20,
    L_FORMAT=21
} fp_format;

// FUNCTIONS

#define issubnormal(_x_) ((_x_).number.exponent == 0)

// These functions convert between single precision floating point numbers
// represented in register or union form. This is required because endian-ness
// matters when a 32-bit float is in a 64-bit register.

static __inline__ void
reg2flt( CYG_HAL_FPU_REG *fpu_reg_p, Cyg_libm_ieee_float_shape_type *flt)
{
#if defined(CYGHWR_HAL_MIPS_FPU_32BIT) || (CYG_BYTEORDER == CYG_LSBFIRST)
    flt->asi32 = *(cyg_int32 *)fpu_reg_p;
#else
    flt->asi32 = *((cyg_int32 *)fpu_reg_p + 1);
# endif
} // reg2flt()

static __inline__ void
flt2reg( Cyg_libm_ieee_float_shape_type *flt, CYG_HAL_FPU_REG *fpu_reg_p )
{
#if defined(CYGHWR_HAL_MIPS_FPU_32BIT) || (CYG_BYTEORDER == CYG_LSBFIRST)
    *(cyg_int32 *)fpu_reg_p = flt->asi32;
#else
    *((cyg_int32 *)fpu_reg_p + 1) = flt->asi32;
# endif
} // flt2reg()

static __inline__ void
reg2dbl( CYG_HAL_FPU_REG *fpu_reg_p, Cyg_libm_ieee_double_shape_type *flt)
{
    flt->asi64 = *(cyg_int64 *)fpu_reg_p;
} // reg2dbl()

static __inline__ void
dbl2reg( Cyg_libm_ieee_double_shape_type *flt, CYG_HAL_FPU_REG *fpu_reg_p )
{
    *(cyg_uint64*)fpu_reg_p = flt->asi64;
} // dbl2reg()


// This function returns non-zero if the exception has been handled
// successfully.

// FIXME: Arguably we should raise underflow exceptions in some of the cases
// below e.g. sqrt(subnormal). And perhaps we should round appropriately to
// +/- 2^^Emin if round to +/- infinity is enabled, as per the FS bit. Not sure.

externC cyg_uint8
cyg_hal_mips_process_fpe( HAL_SavedRegisters *regs )
{
    CYG_WORD insn;
    CYG_HAL_FPU_REG *srcreg1, *srcreg2, *dstreg;
    cyg_uint8 handled=0;
    fp_format format;
    cyg_uint8 fp64bit=0;             // true if format is 64bit, false if 32bit
    cyg_uint8 fixedpoint=0;          // true if format is fixed point, false if
                                     // floating point
    cyg_uint8 computational_insn=1;  // computational FP instruction
    cyg_bool delay_slot;             // did it happen in a delay slot

    CYG_REPORT_FUNCNAMETYPE("cyg_hal_mips_process_fpe", "returning %d");

    CYG_CHECK_DATA_PTR( regs,
                    "cyg_hal_mips_process_fpe() called with invalid regs ptr");

    CYG_PRECONDITION( (regs->vector>>2) == CYGNUM_HAL_VECTOR_FPE,
                      "Asked to process non-FPE exception");

    // First of all, we only handle the unimplemented operation exception
    // here, so if we don't have that, we just exit
    if ((regs->fcr31 & FCR31_CAUSE_E) == 0) {
        CYG_REPORT_RETVAL(0);
        return 0;
    }

    // Get the contents of the instruction that caused the exception. This
    // may have been in a branch delay slot however, so we have to check
    // the BD bit in the cause register first.
    if (regs->cause & CAUSE_BD) {
        insn = *(((CYG_WORD *) regs->pc) + 1);
        delay_slot = true;
    } else {
        insn = *(CYG_WORD *) regs->pc;
        delay_slot = false;
    }

    CYG_TRACE2(true, "exception at pc %08x containing %08x", regs->pc, insn);

    CYG_ASSERT( (insn>>26) == 0x11,
                "Instruction at pc doesn't have expected opcode COP1");

    // Determine the format
    format = (insn >> 21) & 0x1f;

    switch (format)
    {
    case S_FORMAT:
        break;
    case D_FORMAT:
        fp64bit++;
        break;
    case W_FORMAT:
        fixedpoint++;
        break;
    case L_FORMAT:
        fixedpoint++;
        fp64bit++;
        break;
    default:
        computational_insn=0;
        break;
    } // switch

    // This module only emulates computational floating point instructions
    if (computational_insn && !fixedpoint) {

        // Decode the registers used
        dstreg  = &regs->f[ (insn >>  6) & 0x1f ];
        srcreg1 = &regs->f[ (insn >> 11) & 0x1f ];
        srcreg2 = &regs->f[ (insn >> 16) & 0x1f ];
    
        // Determine the operation requested
        switch (insn & 0x3f)
        {
        case ADD_INSN:
        case SUB_INSN:
        case MUL_INSN:
        case DIV_INSN:

            if (fp64bit) {
                Cyg_libm_ieee_double_shape_type s1, s2;

                reg2dbl( srcreg1, &s1 );
                reg2dbl( srcreg2, &s2 );

                if ( issubnormal( s1 ) ) {  // flush to 0 and restart
                    // but preserve sign
                    if (s1.number.sign)
                        s1.value = -0.0;
                    else
                        s1.value = 0.0;
                    dbl2reg( &s1, srcreg1 );
                    handled++;
                }

                // We could try flushing both to 0 at the same time, but
                // that's inadvisable if both numbers are very small.
                // Particularly if this is DIV_INSN, when we could therefore
                // get 0/0 even when the program explicitly checked for
                // denominator != 0. That's also why we check s1 first.

                else if (  issubnormal( s2 ) ) {  // flush to 0 and restart
                    // but preserve sign
                    if (s2.number.sign)
                        s2.value = -0.0;
                    else
                        s2.value = 0.0;
                    dbl2reg( &s2, srcreg2 );
                    handled++;
                }

            } else { // 32-bit
                Cyg_libm_ieee_float_shape_type s1, s2;

                reg2flt( srcreg1, &s1 );
                reg2flt( srcreg2, &s2 );

                if ( issubnormal( s1 )) {  // flush to 0 and restart
                    // but preserve sign
                    if (s1.number.sign)
                        s1.value = -0.0;
                    else
                        s1.value = 0.0;
                    flt2reg( &s1, srcreg1 );
                    handled++;
                }
                else if ( issubnormal( s2 ) ) {  // flush to 0 and restart
                    // but preserve sign
                    if (s2.number.sign)
                        s2.value = -0.0;
                    else
                        s2.value = 0.0;
                    flt2reg( &s2, srcreg2 );
                    handled++;
                }
            }
            break;

        case SQRT_INSN:
            if ( fp64bit ) {
                Cyg_libm_ieee_double_shape_type d, s;

                reg2dbl( dstreg, &d );
                reg2dbl( srcreg1, &s );

                if ( issubnormal( s ) ) {  // Sqrt of something tiny is 0
                    // if this is a delay slot, we can't restart properly
                    // so if it is subnormal, clear the source register instead
                    if ( delay_slot ) {
                        // but preserve sign
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        dbl2reg( &s, srcreg1 );
                    } else {
                        // but preserve sign
                        if (s.number.sign)
                            d.value = -0.0;
                        else
                            d.value = 0.0;
                        dbl2reg( &d, dstreg );
                        regs->pc += 4; // We've dealt with this so move on
                    }
                    handled++;
                }

            } else { // 32-bit
                Cyg_libm_ieee_float_shape_type d, s;

                reg2flt( dstreg, &d );
                reg2flt( srcreg1, &s );

                if ( issubnormal( s ) ) {  // Sqrt of something tiny is 0
                    // if this is a delay slot, we can't restart properly
                    // so if it is subnormal, clear the source register instead
                    if ( delay_slot ) {
                        // but preserve sign
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        flt2reg( &s, srcreg1 );
                    } else {
                        // but preserve sign
                        if (s.number.sign)
                            d.value = -0.0;
                        else
                            d.value = 0.0;
                        flt2reg( &d, dstreg );
                        regs->pc += 4; // We've dealt with this so move on
                    }
                    handled++;
                }
            }
            break;

        case ABS_INSN:
            // We may as well do this right if we can
            if ( fp64bit ) {
                Cyg_libm_ieee_double_shape_type d, s;

                reg2dbl( dstreg, &d );
                reg2dbl( srcreg1, &s );

                // if this is a delay slot, we can't restart properly
                // so if it is subnormal, clear the source register instead
                if ( delay_slot ) {
                    if ( issubnormal( s ) ) {
                        // The sign is still important for abs in case
                        // there are any further operations on the same
                        // register
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        dbl2reg( &s, srcreg1 );
                        handled++;
                    }
                } else {
                    d.asi64 = s.asi64;
                    d.number.sign = 0;
                    dbl2reg( &d, dstreg );
                    regs->pc += 4;
                    handled++;
                }
            } else { // 32-bit
                Cyg_libm_ieee_float_shape_type d, s;

                reg2flt( dstreg, &d );
                reg2flt( srcreg1, &s );

                // if this is a delay slot, we can't restart properly
                // so if it is subnormal, clear the source register instead
                if ( delay_slot ) {
                    if ( issubnormal( s ) ) {
                        // The sign is still important for abs in case
                        // there are any further operations on the same
                        // register
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        flt2reg( &s, srcreg1 );
                        handled++;
                    }
                } else {
                    d.asi32 = s.asi32;
                    d.number.sign = 0;
                    flt2reg( &d, dstreg );
                    regs->pc += 4;
                    handled++;
                }
            }
            break;

        case MOV_INSN:
            // We may as well do this right if we can
            if ( fp64bit ) {
                Cyg_libm_ieee_double_shape_type d, s;

                reg2dbl( dstreg, &d );
                reg2dbl( srcreg1, &s );

                // if this is a delay slot, we can't restart properly
                // so if it is subnormal, clear the source register instead
                if ( delay_slot ) {
                    if ( issubnormal( s ) ) {
                        // but preserve sign
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        dbl2reg( &s, srcreg1 );
                        handled++;
                    }
                } else {
                    d.asi64 = s.asi64;
                    dbl2reg( &d, dstreg );
                    regs->pc += 4;
                    handled++;
                }
            } else { // 32-bit
                Cyg_libm_ieee_float_shape_type d, s;

                reg2flt( dstreg, &d );
                reg2flt( srcreg1, &s );

                // if this is a delay slot, we can't restart properly
                // so if it is subnormal, clear the source register instead
                if ( delay_slot ) {
                    if ( issubnormal( s ) ) {
                        // The sign is still important for abs in case
                        // there are any further operations on the same
                        // register
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        flt2reg( &s, srcreg1 );
                        handled++;
                    }
                } else {
                    d.asi32 = s.asi32;
                    flt2reg( &d, dstreg );
                    regs->pc += 4;
                    handled++;
                }
            }
            break;

        case NEG_INSN:
            // We may as well do this right if we can
            if ( fp64bit ) {
                Cyg_libm_ieee_double_shape_type d, s;

                reg2dbl( dstreg, &d );
                reg2dbl( srcreg1, &s );

                // if this is a delay slot, we can't restart properly
                // so if it is subnormal, clear the source register instead
                if ( delay_slot ) {
                    if ( issubnormal( s ) ) {
                        // but preserve sign
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        dbl2reg( &s, srcreg1 );
                        handled++;
                    }
                } else {
                    d.asi64 = s.asi64;
                    d.number.sign = s.number.sign ? 0 : 1;
                    dbl2reg( &d, dstreg );
                    regs->pc += 4;
                    handled++;
                }
            } else { // 32-bit
                Cyg_libm_ieee_float_shape_type d, s;

                reg2flt( dstreg, &d );
                reg2flt( srcreg1, &s );

                // if this is a delay slot, we can't restart properly
                // so if it is subnormal, clear the source register instead
                if ( delay_slot ) {
                    if ( issubnormal( s ) ) {
                        // but preserve sign
                        if (s.number.sign)
                            s.value = -0.0;
                        else
                            s.value = 0.0;
                        flt2reg( &s, srcreg1 );
                        handled++;
                    }
                } else {
                    d.asi32 = s.asi32;
                    d.number.sign = s.number.sign ? 0 : 1;
                    flt2reg( &d, dstreg );
                    regs->pc += 4;
                    handled++;
                }
            }
            break;

        // We can't do much about floating-point to fixed-point arithmetic
        // without emulating the FPU here ourselves!
        // So simply zero denormalized numbers
        case ROUNDL_INSN:
        case TRUNCL_INSN:
        case CEILL_INSN:
        case FLOORL_INSN:
        case ROUNDW_INSN:
        case TRUNCW_INSN:
        case CEILW_INSN:
        case FLOORW_INSN:
        case CVTS_INSN:
        case CVTD_INSN:
        case CVTW_INSN:
        case CVTL_INSN:
            
            if ( fp64bit ) {
                Cyg_libm_ieee_double_shape_type s;

                reg2dbl( srcreg1, &s );

                // just try and 0 the source register if it is subnormal
                if ( issubnormal( s ) ) {
                    // but preserve sign
                    if (s.number.sign)
                        s.value = -0.0;
                    else
                        s.value = 0.0;
                    dbl2reg( &s, srcreg1 );
                    handled++;
                }
            } else { // 32-bit
                Cyg_libm_ieee_float_shape_type s;

                reg2flt( srcreg1, &s );

                // just try and 0 the source register if it is subnormal
                if ( issubnormal( s ) ) {
                    // but preserve sign
                    if (s.number.sign)
                        s.value = -0.0;
                    else
                        s.value = 0.0;
                    flt2reg( &s, srcreg1 );
                    handled++;
                }
            }
            break;

        default:
            // check for floating-point compare (C.cond.fmt)
            if ( (insn & 0x30) == 0x30 ) {
                if (fp64bit) {
                    Cyg_libm_ieee_double_shape_type s1, s2;
                    
                    reg2dbl( srcreg1, &s1 );
                    reg2dbl( srcreg2, &s2 );
                    
                    if ( issubnormal( s1 ) ) {  // flush to 0 and restart
                        // but preserve sign
                        if (s1.number.sign)
                            s1.value = -0.0;
                        else
                            s1.value = 0.0;
                        dbl2reg( &s1, srcreg1 );
                        handled++;
                    }
                    
                    if (  issubnormal( s2 ) ) {  // flush to 0 and restart
                        // but preserve sign
                        if (s2.number.sign)
                            s2.value = -0.0;
                        else
                            s2.value = 0.0;
                        dbl2reg( &s2, srcreg2 );
                        handled++;
                    }
                    
                } else { // 32-bit
                    Cyg_libm_ieee_float_shape_type s1, s2;
                    
                    reg2flt( srcreg1, &s1 );
                    reg2flt( srcreg2, &s2 );
                    
                    if ( issubnormal( s1 )) {  // flush to 0 and restart
                        // but preserve sign
                        if (s1.number.sign)
                            s1.value = -0.0;
                        else
                            s1.value = 0.0;
                        flt2reg( &s1, srcreg1 );
                        handled++;
                    }
                    if ( issubnormal( s2 ) ) {  // flush to 0 and restart
                        // but preserve sign
                        if (s2.number.sign)
                            s2.value = -0.0;
                        else
                            s2.value = 0.0;
                        flt2reg( &s2, srcreg2 );
                        handled++;
                    }
                } // else 
            } // if
            break;
        } // switch
    } // if (computational_insn && !fixedpoint)
    
    if ( handled != 0) {
        // We must clear the cause and flag bits before restoring FPCR31
        regs->fcr31 &= ~(FCR31_CAUSE_E | FCR31_CAUSE_V | FCR31_CAUSE_Z |
                         FCR31_CAUSE_O | FCR31_CAUSE_U | FCR31_CAUSE_I |
                         FCR31_FLAGS_V | FCR31_FLAGS_Z |
                         FCR31_FLAGS_O | FCR31_FLAGS_U | FCR31_FLAGS_I);

    }

    CYG_REPORT_RETVAL( handled );
    return handled;
} // cyg_hal_mips_process_fpe()

#endif // ifdef CYGHWR_HAL_MIPS_FPU

// EOF mipsfp.c
