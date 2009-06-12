//==========================================================================
//
//      var_misc.c
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
// Author(s):    jskov
// Contributors: jskov, jlarmour, nickg
// Date:         1999-04-03
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_printf

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_cache.h>          // HAL cache
#include <cyg/hal/hal_intr.h>           // HAL interrupts/exceptions

//---------------------------------------------------------------------------
// Initial cache enabling

#ifdef CYGHWR_HAL_SH_CACHE_MODE_P0_WRITE_BACK
# define CACHE_MODE_P0 0
#else
# define CACHE_MODE_P0 CYGARC_REG_CCR_WT
#endif

#ifdef CYGHWR_HAL_SH_CACHE_MODE_P1_WRITE_BACK
# define CACHE_MODE_P1 CYGARC_REG_CCR_CB
#else
# define CACHE_MODE_P1 0
#endif

externC void
cyg_var_enable_caches(void)
{
    // If relying on a ROM monitor do not invalidate the caches as the
    // ROM monitor may have (non-synced) state in the caches.
#if !defined(CYGSEM_HAL_USE_ROM_MONITOR)
    // Initialize cache.
    HAL_UCACHE_INVALIDATE_ALL();    

    // Set cache modes
    HAL_UCACHE_WRITE_MODE_SH(CACHE_MODE_P0|CACHE_MODE_P1);
#endif
#ifdef CYGHWR_HAL_SH_CACHE_ENABLE
    // Enable cache.
    HAL_UCACHE_ENABLE();
#endif
}

//---------------------------------------------------------------------------
void
hal_variant_init(void)
{
}

//---------------------------------------------------------------------------
// Interrupt function support

externC cyg_uint8 cyg_hal_ILVL_table[];
externC cyg_uint8 cyg_hal_IMASK_table[];

static void
hal_interrupt_update_level(int vector)
{
    cyg_uint16 iprX;                                                     
    int level;

    level = cyg_hal_IMASK_table[vector] ? cyg_hal_ILVL_table[vector] : 0;

    switch( (vector) ) {                                               
        /* IPRA */                                                           
    case CYGNUM_HAL_INTERRUPT_TMU0_TUNI0:                                
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);                          
        iprX &= ~CYGARC_REG_IPRA_TMU0_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRA_TMU0_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_TMU1_TUNI1:                                
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);                          
        iprX &= ~CYGARC_REG_IPRA_TMU1_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRA_TMU1_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_TMU2_TUNI2:                                
    case CYGNUM_HAL_INTERRUPT_TMU2_TICPI2:                               
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);                          
        iprX &= ~CYGARC_REG_IPRA_TMU2_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRA_TMU2_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_RTC_ATI:                                   
    case CYGNUM_HAL_INTERRUPT_RTC_PRI:                                   
    case CYGNUM_HAL_INTERRUPT_RTC_CUI:                                   
        HAL_READ_UINT16(CYGARC_REG_IPRA, iprX);                          
        iprX &= ~CYGARC_REG_IPRA_RTC_MASK;                               
        iprX |= (level)*CYGARC_REG_IPRA_RTC_PRI1;                      
        HAL_WRITE_UINT16(CYGARC_REG_IPRA, iprX);                         
        break;                                                           
                                                                         
        /* IPRB */                                                           
    case CYGNUM_HAL_INTERRUPT_SCI_ERI:                                   
    case CYGNUM_HAL_INTERRUPT_SCI_RXI:                                   
    case CYGNUM_HAL_INTERRUPT_SCI_TXI:                                   
    case CYGNUM_HAL_INTERRUPT_SCI_TEI:                                   
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);                          
        iprX &= ~CYGARC_REG_IPRB_SCI_MASK;                               
        iprX |= (level)*CYGARC_REG_IPRB_SCI_PRI1;                      
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_WDT_ITI:                                   
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);                          
        iprX &= ~CYGARC_REG_IPRB_WDT_MASK;                               
        iprX |= (level)*CYGARC_REG_IPRB_WDT_PRI1;                      
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_REF_RCMI:                                  
    case CYGNUM_HAL_INTERRUPT_REF_ROVI:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRB, iprX);                          
        iprX &= ~CYGARC_REG_IPRB_REF_MASK;                               
        iprX |= (level)*CYGARC_REG_IPRB_REF_PRI1;                      
        HAL_WRITE_UINT16(CYGARC_REG_IPRB, iprX);                         
        break;                                                           
                                                                         
#if (CYGARC_SH_MOD_INTC >= 2)
#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
        /* IPRC */                                                           
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ0:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);                          
        iprX &= ~CYGARC_REG_IPRC_IRQ0_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRC_IRQ0_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ1:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);                          
        iprX &= ~CYGARC_REG_IPRC_IRQ1_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRC_IRQ1_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ2:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);                          
        iprX &= ~CYGARC_REG_IPRC_IRQ2_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRC_IRQ2_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ3:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRC, iprX);                          
        iprX &= ~CYGARC_REG_IPRC_IRQ3_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRC_IRQ3_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRC, iprX);                         
        break;                                                           
#endif
        /* IPRD */                                                           
    case CYGNUM_HAL_INTERRUPT_PINT_PINT07:                               
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);                          
        iprX &= ~CYGARC_REG_IPRD_PINT07_MASK;                            
        iprX |= (level)*CYGARC_REG_IPRD_PINT07_PRI1;                   
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_PINT_PINT8F:                               
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);                          
        iprX &= ~CYGARC_REG_IPRD_PINT8F_MASK;                            
        iprX |= (level)*CYGARC_REG_IPRD_PINT8F_PRI1;                   
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ5:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);                          
        iprX &= ~CYGARC_REG_IPRD_IRQ5_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRD_IRQ5_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_IRQ_IRQ4:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRD, iprX);                          
        iprX &= ~CYGARC_REG_IPRD_IRQ4_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRD_IRQ4_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRD, iprX);                         
        break;                                                           
                                                                         
        /* IPRE */                                                           
    case CYGNUM_HAL_INTERRUPT_DMAC_DEI0:                                 
    case CYGNUM_HAL_INTERRUPT_DMAC_DEI1:                                 
    case CYGNUM_HAL_INTERRUPT_DMAC_DEI2:                                 
    case CYGNUM_HAL_INTERRUPT_DMAC_DEI3:                                 
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);                          
        iprX &= ~CYGARC_REG_IPRE_DMAC_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRE_DMAC_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_IRDA_ERI1:                                 
    case CYGNUM_HAL_INTERRUPT_IRDA_RXI1:                                 
    case CYGNUM_HAL_INTERRUPT_IRDA_BRI1:                                 
    case CYGNUM_HAL_INTERRUPT_IRDA_TXI1:                                 
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);                          
        iprX &= ~CYGARC_REG_IPRE_IRDA_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRE_IRDA_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_SCIF_ERI2:                                 
    case CYGNUM_HAL_INTERRUPT_SCIF_RXI2:                                 
    case CYGNUM_HAL_INTERRUPT_SCIF_BRI2:                                 
    case CYGNUM_HAL_INTERRUPT_SCIF_TXI2:                                 
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);                          
        iprX &= ~CYGARC_REG_IPRE_SCIF_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRE_SCIF_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_ADC_ADI:                                   
        HAL_READ_UINT16(CYGARC_REG_IPRE, iprX);                          
        iprX &= ~CYGARC_REG_IPRE_ADC_MASK;                               
        iprX |= (level)*CYGARC_REG_IPRE_ADC_PRI1;                      
        HAL_WRITE_UINT16(CYGARC_REG_IPRE, iprX);                         
        break;                                                           
#endif // (CYGARC_SH_MOD_INTC >= 2)

#if (CYGARC_SH_MOD_INTC >= 3)
        /* IPRF */                                                           
    case CYGNUM_HAL_INTERRUPT_LCDC_LCDI:                                 
        HAL_READ_UINT16(CYGARC_REG_IPRF, iprX);                          
        iprX &= ~CYGARC_REG_IPRF_LCDI_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRF_LCDI_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRF, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_PCC_PCC0:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRF, iprX);                          
        iprX &= ~CYGARC_REG_IPRF_PCC0_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRF_PCC0_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRF, iprX);                         
        break;                                                           
    case CYGNUM_HAL_INTERRUPT_PCC_PCC1:                                  
        HAL_READ_UINT16(CYGARC_REG_IPRF, iprX);                          
        iprX &= ~CYGARC_REG_IPRF_PCC1_MASK;                              
        iprX |= (level)*CYGARC_REG_IPRF_PCC1_PRI1;                     
        HAL_WRITE_UINT16(CYGARC_REG_IPRF, iprX);                         
        break;                                                           
#endif // (CYGARC_SH_MOD_INTC >= 3)

    case CYGNUM_HAL_INTERRUPT_RESERVED_1E0:                              
    case CYGNUM_HAL_INTERRUPT_RESERVED_3E0:                              
        /* Do nothing for these reserved vectors. */                     
        break;                                                           

    // Platform extensions
    CYGPRI_HAL_INTERRUPT_UPDATE_LEVEL_PLF(vector, level)

    default:
        CYG_FAIL("Unknown interrupt vector");                             
        break;
    }                                                                    
}

void
hal_interrupt_set_level(int vector, int level)
{
    CYG_ASSERT((0 <= (level) && 15 >= (level)), "Illegal level");    
    CYG_ASSERT((CYGNUM_HAL_ISR_MIN <= (vector)                         
                && CYGNUM_HAL_ISR_MAX >= (vector)), "Illegal vector"); 
                                                                         
    cyg_hal_ILVL_table[vector] = level;

    hal_interrupt_update_level(vector);
}

void
hal_interrupt_mask(int vector)                                    
{
    switch( (vector) ) {                                                
    case CYGNUM_HAL_INTERRUPT_NMI:                                        
        /* fall through */                                                
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL14:
        /* Normally can only be masked by fiddling Imask in SR,
           but some platforms use external interrupt controller,
           so allow regular handling. */
        // fall through
    case CYGNUM_HAL_INTERRUPT_TMU0_TUNI0 ... CYGNUM_HAL_ISR_MAX:            
        cyg_hal_IMASK_table[vector] = 0;
        hal_interrupt_update_level(vector);
        break;                                                            
    case CYGNUM_HAL_INTERRUPT_RESERVED_1E0:                               
    case CYGNUM_HAL_INTERRUPT_RESERVED_3E0:                               
        /* Do nothing for these reserved vectors. */                      
        break;                                                            
    default:                                                              
        CYG_FAIL("Unknown interrupt vector");                             
        break;                                                            
    }                                                                     
}

void
hal_interrupt_unmask(int vector)                                  
{
    switch( (vector) ) {                                                
    case CYGNUM_HAL_INTERRUPT_NMI:                                        
        /* fall through */                                                
    case CYGNUM_HAL_INTERRUPT_LVL0 ... CYGNUM_HAL_INTERRUPT_LVL14:          
        /* Normally can only be masked by fiddling Imask in SR,
           but some platforms use external interrupt controller,
           so allow regular handling. */
        // fall through
    case CYGNUM_HAL_INTERRUPT_TMU0_TUNI0 ... CYGNUM_HAL_ISR_MAX:            
        cyg_hal_IMASK_table[vector] = 1;
        hal_interrupt_update_level(vector);
        break;                                                            
    case CYGNUM_HAL_INTERRUPT_RESERVED_1E0:                               
    case CYGNUM_HAL_INTERRUPT_RESERVED_3E0:                               
        /* Do nothing for these reserved vectors. */                      
        break;                                                            
    default:                                                              
        CYG_FAIL("Unknown interrupt vector");                             
        break;                                                            
    }                                                                     
}

void
hal_interrupt_acknowledge(int vector)
{
#if (CYGARC_SH_MOD_INTC >= 2)
    if ( (vector) >= CYGNUM_HAL_INTERRUPT_IRQ_IRQ0                      
         && (vector) <= CYGNUM_HAL_INTERRUPT_IRQ_IRQ5) {                
                                                                          
        cyg_uint8 irr0;                                                   
                                                                          
        HAL_READ_UINT8(CYGARC_REG_IRR0, irr0);       
        switch ( vector ) {                          
#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL                 
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ0:          
            irr0 &= ~CYGARC_REG_IRR0_IRQ0;           
            break;                                   
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ1:          
            irr0 &= ~CYGARC_REG_IRR0_IRQ1;           
            break;                                   
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ2:          
            irr0 &= ~CYGARC_REG_IRR0_IRQ2;           
            break;                                   
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ3:          
            irr0 &= ~CYGARC_REG_IRR0_IRQ3;           
            break;                                   
#endif                                               
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ4:          
            irr0 &= ~CYGARC_REG_IRR0_IRQ4;           
            break;                                   
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ5:          
            irr0 &= ~CYGARC_REG_IRR0_IRQ5;           
            break;                                   
        default:                                     
            CYG_FAIL("Unhandled interrupt vector");  
        }                                            
        HAL_WRITE_UINT8(CYGARC_REG_IRR0, irr0);      
    }                                                                     
#endif

    CYGPRI_HAL_INTERRUPT_ACKNOWLEDGE_PLF(vector);
}

// Note: The PINTs can be masked and configured individually, even
// though there are only two vectors. Maybe add some fake vectors just
// for masking/configuring?
void 
hal_interrupt_configure(int vector, int level, int up)
{
#if (CYGARC_SH_MOD_INTC >= 2)
    if ( (vector) >= CYGNUM_HAL_INTERRUPT_IRQ_IRQ0                     
         && (vector) <= CYGNUM_HAL_INTERRUPT_IRQ_IRQ5) {               
                                                                         
        cyg_uint16 icr1, ss, mask;                                       
        ss = 0;                                                          
        mask = CYGARC_REG_ICR1_SENSE_UP|CYGARC_REG_ICR1_SENSE_LEVEL;     
        if (up) ss |= CYGARC_REG_ICR1_SENSE_UP;                        
        if (level) ss |= CYGARC_REG_ICR1_SENSE_LEVEL;                  
        CYG_ASSERT(!(up && level), "Cannot trigger on high level!"); 
                                                                         
        switch( (vector) ) {                                           
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ5:                              
            ss <<= CYGARC_REG_ICR1_SENSE_IRQ5_shift;                     
            mask <<= CYGARC_REG_ICR1_SENSE_IRQ5_shift;                   
            break;                                                       
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ4:                              
            ss <<= CYGARC_REG_ICR1_SENSE_IRQ4_shift;                     
            mask <<= CYGARC_REG_ICR1_SENSE_IRQ4_shift;                   
            break;                                                       
#ifndef CYGHWR_HAL_SH_IRQ_USE_IRQLVL
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ3:                              
            ss <<= CYGARC_REG_ICR1_SENSE_IRQ3_shift;                     
            mask <<= CYGARC_REG_ICR1_SENSE_IRQ3_shift;                   
            break;                                                       
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ2:                              
            ss <<= CYGARC_REG_ICR1_SENSE_IRQ2_shift;                     
            mask <<= CYGARC_REG_ICR1_SENSE_IRQ2_shift;                   
            break;                                                       
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ1:                              
            ss <<= CYGARC_REG_ICR1_SENSE_IRQ1_shift;                     
            mask <<= CYGARC_REG_ICR1_SENSE_IRQ1_shift;                   
            break;                                                       
        case CYGNUM_HAL_INTERRUPT_IRQ_IRQ0:                              
            ss <<= CYGARC_REG_ICR1_SENSE_IRQ0_shift;                     
            mask <<= CYGARC_REG_ICR1_SENSE_IRQ0_shift;                   
            break;                                                       
#endif
        default:
            CYG_FAIL("Unhandled interrupt vector");
        }
                                                                         
        HAL_READ_UINT16(CYGARC_REG_ICR1, icr1);                          
        icr1 &= ~mask;                                                   
        icr1 |= ss;                                                      
        HAL_WRITE_UINT16(CYGARC_REG_ICR1, icr1);                         
    }                                                                    
#endif

    CYGPRI_HAL_INTERRUPT_CONFIGURE_PLF(vector, level, up);
}

//---------------------------------------------------------------------------
// End of hal_misc.c
