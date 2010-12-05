#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      AR7240 Interrupt and clock support
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/ar7240_soc.h>

//--------------------------------------------------------------------------
// Interrupt vectors.

#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED
/*
 * Interrupt Vectors for AR7240.
 * The first 6 interrupts are wired directly to the interrupt field
 * in the Status and Cause Mips Registers.  They are also available in the
 * Global Interrupt Status register. One of these is the Miscellaneous interrupt
 * which is actually the OR of all the interrupts reported in the
 * Miscellaneous Interrupt Status Register.
 */

#define CYGNUM_HAL_INTERRUPT_PCI_VEC            0
#define CYGNUM_HAL_INTERRUPT_USB_VEC            1
#define CYGNUM_HAL_INTERRUPT_ETH0_VEC           2
#define CYGNUM_HAL_INTERRUPT_ETH1_VEC           3
#define CYGNUM_HAL_INTERRUPT_MISC_VEC           4
#define CYGNUM_HAL_INTERRUPT_INTCLK_VEC         5

#define CYGNUM_HAL_INTERRUPT_GENTIMER_VEC       6
#define CYGNUM_HAL_INTERRUPT_AHBPROC_ERR_VEC    7
#define CYGNUM_HAL_INTERRUPT_GPIO_VEC           8
#define CYGNUM_HAL_INTERRUPT_UART_VEC           9
#define CYGNUM_HAL_INTERRUPT_WATCHDOG_VEC      10
#define CYGNUM_HAL_INTERRUPT_PC_VEC            11
#define CYGNUM_HAL_INTERRUPT_MBOX_VEC          13
#define CYGNUM_HAL_INTERRUPT_TIMER1_VEC        14
#define CYGNUM_HAL_INTERRUPT_TIMER2_VEC        15
#define CYGNUM_HAL_INTERRUPT_TIMER3_VEC        16
#define CYGNUM_HAL_INTERRUPT_ETH_MAC_VEC       18




#if defined(CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN)

/*
 * XXXAdrian fixme
 */
// This overlaps with CYGNUM_HAL_INTERRUPT_EXTERNAL_BASE above but it
// doesn't matter. It's only used by the HAL to access the special
// chaining entry in the ISR tables.  All other attempted access to
// the ISR table will be redirected to this entry (courtesy of
// HAL_TRANSLATE_VECTOR). The other vector definitions are still
// valid, but only for enable/disable/config etc. (i.e., in chaining
// mode they have associated entries in the ISR tables).
#define CYGNUM_HAL_INTERRUPT_CHAINING           6

#define HAL_TRANSLATE_VECTOR(_vector_,_index_) \
    (_index_) = CYGNUM_HAL_INTERRUPT_CHAINING

// Min/Max ISR numbers
#define CYGNUM_HAL_ISR_MIN                 0
#define CYGNUM_HAL_ISR_MAX                 CYGNUM_HAL_INTERRUPT_ETH_MAC_VEC

#else

// Min/Max ISR numbers
#define CYGNUM_HAL_ISR_MIN                 0
#define CYGNUM_HAL_ISR_MAX                 CYGNUM_HAL_INTERRUPT_ETH_MAC_VEC
#endif // CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN

#define CYGNUM_HAL_ISR_COUNT               (CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC           CYGNUM_HAL_INTERRUPT_INTCLK_VEC

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

#endif // CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED


//--------------------------------------------------------------------------
// Control-C support.

#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

#define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_UART_VEC

externC cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

#define HAL_CTRLC_ISR hal_ctrlc_isr

#endif // CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT

#define HAL_INTERRUPT_MASK( _vector_ )                                      \
    CYG_MACRO_START                                                         \
    if( (_vector_) <= CYGNUM_HAL_INTERRUPT_INTCLK_VEC )                     \
    {                                                                       \
        asm volatile (                                                      \
            "mfc0   $3,$12\n"                                               \
            "la     $2,0x00000400\n"                                        \
            "sllv   $2,$2,%0\n"                                             \
            "nor    $2,$2,$0\n"                                             \
            "and    $3,$3,$2\n"                                             \
            "mtc0   $3,$12\n"                                               \
            "nop; nop; nop\n"                                               \
            :                                                               \
            : "r"(_vector_)                                                 \
            : "$2", "$3"                                                    \
            );                                                              \
        if ((_vector_) == CYGNUM_HAL_INTERRUPT_PCI_VEC)                     \
        {                                                                   \
            cyg_uint32 _mask_;                                              \
            HAL_READ_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_MASK), _mask_);      \
            _mask_ &= ~(AR7240_PCI_INT_A_L);                                            \
            HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_MASK), _mask_ );    \
            HAL_READ_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_STATUS), _mask_);    \
            _mask_ &= ~(AR7240_PCI_INT_A_L);                                            \
            HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_STATUS), _mask_ );  \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    if ( (_vector_) <= CYGNUM_HAL_INTERRUPT_ETH_MAC_VEC)                    \
    {   /* Miscellaneous Interrupt */                                       \
        cyg_uint32 _mask_;                                                  \
        cyg_uint32 _shift_ =                                                \
            (_vector_) - CYGNUM_HAL_INTERRUPT_GENTIMER_VEC;                 \
        HAL_READ_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_MISC_INT_MASK), _mask_ );   \
        _mask_ &= !(1 << _shift_);                                                 \
        HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_MISC_INT_MASK), _mask_ );  \
    }                                                                              \
    CYG_MACRO_END


#define HAL_INTERRUPT_UNMASK( _vector_ )                                    \
    CYG_MACRO_START                                                         \
    if( (_vector_) <= CYGNUM_HAL_INTERRUPT_INTCLK_VEC )                     \
    {                                                                       \
        asm volatile (                                                      \
            "mfc0   $3,$12\n"                                               \
            "la     $2,0x00000400\n"                                        \
            "sllv   $2,$2,%0\n"                                             \
            "or     $3,$3,$2\n"                                             \
            "mtc0   $3,$12\n"                                               \
            "nop; nop; nop\n"                                               \
            :                                                               \
            : "r"(_vector_)                                                 \
            : "$2", "$3"                                                    \
            );                                                              \
        if ((_vector_) == CYGNUM_HAL_INTERRUPT_PCI_VEC)                     \
        {                                                                   \
            cyg_uint32 _mask_;                                              \
            HAL_READ_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_MASK), _mask_);      \
            _mask_ |= AR7240_PCI_INT_A_L;                                               \
            HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_MASK), _mask_ );    \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    if ( (_vector_) <= CYGNUM_HAL_INTERRUPT_ETH_MAC_VEC)                    \
    {   /* Miscellaneous Interrupt */                                       \
        cyg_uint32 _mask_;                                                  \
        cyg_uint32 _shift_ =                                                \
            (_vector_) - CYGNUM_HAL_INTERRUPT_GENTIMER_VEC;                 \
        HAL_READ_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_MISC_INT_MASK), _mask_ );     \
        _mask_ |= (1 << _shift_);                                                    \
        HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_MISC_INT_MASK), _mask_ );    \
        asm volatile (                                                      \
            "mfc0   $3,$12\n"                                               \
            "la     $2,0x00000400\n"                                        \
            "sllv   $2,$2,%0\n"                                             \
            "or     $3,$3,$2\n"                                             \
            "mtc0   $3,$12\n"                                               \
            "nop; nop; nop\n"                                               \
            :                                                               \
            : "r"(CYGNUM_HAL_INTERRUPT_MISC_VEC)                            \
            : "$2", "$3"                                                    \
            );                                                              \
    }                                                                       \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )                               \
    CYG_MACRO_START                                                         \
    cyg_uint32 _srvector_ = _vector_;                                       \
    if ((_vector_) >= CYGNUM_HAL_INTERRUPT_GENTIMER_VEC) {                  \
        cyg_uint32 _mask_;                                                  \
        cyg_uint32 _shift_ =                                                \
            (_vector_) - CYGNUM_HAL_INTERRUPT_GENTIMER_VEC;                 \
        _mask_ = !(1 << _shift_);                                           \
        HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_MISC_INT_STATUS), _mask_ );     \
        _srvector_ = CYGNUM_HAL_INTERRUPT_MISC_VEC;                         \
    }                                                                       \
    else if ((_vector_) == CYGNUM_HAL_INTERRUPT_PCI_VEC)                    \
    {                                                                       \
        cyg_uint32 _mask_;                                                  \
        HAL_READ_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_STATUS), _mask_);    \
        _mask_ &= ~(AR7240_PCI_INT_A_L);                                            \
        HAL_WRITE_UINT32(CYGARC_UNCACHED_ADDRESS(AR7240_PCI_INT_STATUS), _mask_ );  \
    }                                                                       \
    asm volatile (                                                          \
        "mfc0   $3,$13\n"                                                   \
        "la     $2,0x00000400\n"                                            \
        "sllv   $2,$2,%0\n"                                                 \
        "nor    $2,$2,$0\n"                                                 \
        "and    $3,$3,$2\n"                                                 \
        "mtc0   $3,$13\n"                                                   \
        "nop; nop; nop\n"                                                   \
        :                                                                   \
        : "r"(_srvector_)                                                   \
        : "$2", "$3"                                                        \
    );                                                                      \
    CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )

#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

//----------------------------------------------------------------------------
// Reset.
#ifndef CYGHWR_HAL_RESET_DEFINED
extern void hal_ar7240_reset( void );
#define CYGHWR_HAL_RESET_DEFINED
#define HAL_PLATFORM_RESET()            hal_ar7240_reset() 

#define HAL_PLATFORM_RESET_ENTRY        0xbf000000

#endif // CYGHWR_HAL_RESET_DEFINED

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
