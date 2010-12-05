//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
//
//==========================================================================

#define CYGARC_HAL_COMMON_EXPORT_CPU_MACROS
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/mips-regs.h>          // FPU cause register definitions

#include <cyg/hal/ar7240_soc.h>
#if defined(CYGPKG_IO_PCI)
#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>
#endif /* defined(CYGPKG_IO_PCI) */

#if 0
#define CONSOLE_PRINTF(fmt, arg...)
#else
#define CONSOLE_PRINTF(fmt, arg...) do {                                                    \
                               diag_printf("%s[%4d]: "fmt"\n", __func__, __LINE__, ##arg);    \
                          } while(0)
#endif


typedef cyg_uint32 u32;
typedef cyg_uint16 u16;
typedef cyg_uint8  u8;

extern void hal_diag_init(void);
extern void hal_board_init(void);

void
hal_platform_init(void)
{
    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE();


    hal_if_init();
    // Board specific initialization

    hal_board_init();

    cyg_pci_init();
}



/*
 * We extract the pll divider, multiply it by the base freq 40.
 * The cpu and ahb are divided off of that.
 */
cyg_uint32 ar7240_ahb_freq = 0;
cyg_uint32 ar7240_cpu_freq = 0;
cyg_uint32 ar7240_ddr_freq = 0;

void
hal_ar7240_sys_frequency(void)
{
    cyg_uint32 pll, pll_div, ahb_div, ddr_div, freq, ref_div;

    if (ar7240_ahb_freq) {
        return;
    }

    pll = ar7240_reg_rd(AR7240_PLL_CONFIG);

    pll_div  = ((pll >> PLL_DIV_SHIFT) & PLL_DIV_MASK);
    ref_div  = (pll >> REF_DIV_SHIFT) & REF_DIV_MASK;
    ddr_div  = ((pll >> DDR_DIV_SHIFT) & DDR_DIV_MASK) + 1;
    ahb_div  = (((pll >> AHB_DIV_SHIFT) & AHB_DIV_MASK) + 1)*2;

    freq     = pll_div * ref_div * 5000000;

    ar7240_cpu_freq = freq;
    ar7240_ddr_freq = freq/ddr_div;
    ar7240_ahb_freq = ar7240_cpu_freq/ahb_div;

}


/*------------------------------------------------------------------------*/
/* Reset support                                                          */

void
hal_ar7240_reset(void)
{
    /* Check if this is correct */
    for(;;) {
        ar7240_reg_wr(AR7240_RESET, AR7240_RESET_FULL_CHIP);
    }
}

/*------------------------------------------------------------------------*/

#if defined(CYGPKG_IO_PCI)
void
ar7240_local_read_config(int where, int size, cyg_uint32 *value)
{
    cyg_uint32 addr, tval, mask;

    /* Make sure the address is aligned to natural boundary */
    switch (size) {
        case 1:
            addr = where & ~3;
            mask = 0xff000000 >> ((where % 4) * 8);
            tval = ar7240_reg_rd(AR7240_PCI_CRP + addr);
            tval = tval & ~mask;
            *value = (tval >> ((4 - (where % 4))*8));
            break;
        case 2:
            addr = where & ~3;
            mask = 0xffff0000 >> ((where % 4)*8);
            tval = ar7240_reg_rd(AR7240_PCI_CRP + addr);
            tval = tval & ~mask;
            *value = (tval >> ((4 - (where % 4))*8));
            break;
        case 4:
            *value = ar7240_reg_rd(AR7240_PCI_CRP + where);
            break;
        default:
            return;
    }
}

void
ar7240_local_write_config(int where, int size, cyg_uint32 value)
{
    cyg_uint32 addr, tval, mask;

    switch (size) {
        case 1:
            addr = (AR7240_PCI_CRP + where) & ~3;
            mask = 0xff000000 >> ((where % 4)*8);
            tval = ar7240_reg_rd(addr);
            tval = tval & ~mask;
            tval |= (value << ((4 - (where % 4))*8)) & mask;
            ar7240_reg_wr(addr, tval);
            break;
        case 2:
            addr = (AR7240_PCI_CRP + where) & ~3;
            mask = 0xffff0000 >> ((where % 4)*8);
            tval = ar7240_reg_rd(addr);
            tval = tval & ~mask;
            tval |= (value << ((4 - (where % 4))*8)) & mask;
            ar7240_reg_wr(addr, tval);
            break;
        case 4:
            ar7240_reg_wr((AR7240_PCI_CRP + where), value);
            break;
        default:
            return;
    }
}



void
cyg_hal_plf_pci_init(void)
{

    cyg_uint32 cmd;

    CONSOLE_PRINTF("begin");
    if (((ar7240_reg_rd(AR7240_PCI_LCL_RESET)) & 0x1) == 0x0) {
        CONSOLE_PRINTF("***** Warning *****: PCIe WLAN H/W not found !!!\n");
        return;
    }

    if ((is_ar7241() || is_ar7242())) {
        ar7240_reg_wr(AR7240_PCI_LCL_APP, (ar7240_reg_rd(AR7240_PCI_LCL_APP) | (0x1 << 16)));
    
    }

    cmd = CYG_PCI_CFG_COMMAND_MEMORY |
          CYG_PCI_CFG_COMMAND_MASTER |
          CYG_PCI_CFG_COMMAND_INVALIDATE |
          CYG_PCI_CFG_COMMAND_PARITY|
          CYG_PCI_CFG_COMMAND_SERR |
          CYG_PCI_CFG_COMMAND_FAST_BACK;

    ar7240_local_write_config(CYG_PCI_CFG_COMMAND, 4, cmd);

    asm volatile (                   \
        "mfc0   $3,$12\n"            \
        "la     $2,0x00000400\n"     \
        "or     $3,$3,$2\n"          \
        "mtc0   $3,$12\n"            \
        "nop; nop; nop\n"            \
        :                            \
        :                            \
        : "$2", "$3"                 \
    );                               \

    CONSOLE_PRINTF("end");
}


cyg_uint32 cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus,
                                           cyg_uint32 devfn,
                                           cyg_uint32 offset)
{
    cyg_uint32 tval;

    tval = ar7240_reg_rd(AR7240_PCI_DEV_CFGBASE + offset);
    if (is_ar7240()) {
                /*
                 * WAR for BAR issue - We are unable to access the PCI device spac
                 * if we set the BAR with proper base address
                 */
        if (offset == 0x10) {
            ar7240_reg_wr((AR7240_PCI_DEV_CFGBASE + offset),0xffff);
        }
    }
    return tval;
}

cyg_uint16 cyg_hal_plf_pci_cfg_read_word (cyg_uint32 bus,
                                          cyg_uint32 devfn,
                                          cyg_uint32 offset)
{
    cyg_uint32 addr, tval, mask;

    addr = offset & ~3;
    mask = 0x0000ffff << ((offset % 4)*8);
    tval = ar7240_reg_rd(AR7240_PCI_DEV_CFGBASE + addr);
    tval = tval & mask;

    return (cyg_uint16)(tval >> ((offset % 4))*8);
}

cyg_uint8 cyg_hal_plf_pci_cfg_read_byte (cyg_uint32 bus,
                                         cyg_uint32 devfn,
                                         cyg_uint32 offset)
{
    cyg_uint32 addr, tval, mask;
    

    addr = offset & ~3;
    mask = 0x000000ff << ((offset % 4)*8);
    tval = ar7240_reg_rd(AR7240_PCI_DEV_CFGBASE + addr);
    tval = tval & mask;
    return (cyg_uint8)(tval >> ((offset % 4))*8);
}

void cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
                                      cyg_uint32 devfn,
                                      cyg_uint32 offset,
                                      cyg_uint32 value)
{
    ar7240_reg_wr((AR7240_PCI_DEV_CFGBASE + offset), value);
}

void cyg_hal_plf_pci_cfg_write_word (cyg_uint32 bus,
                                     cyg_uint32 devfn,
                                     cyg_uint32 offset,
                                     cyg_uint16 value)
{
    cyg_uint32 addr, tval, mask;

    addr = (AR7240_PCI_DEV_CFGBASE + offset) & ~3;
    mask = 0x0000ffff << ((offset % 4)*8);
    tval = ar7240_reg_rd(addr);
    tval = tval & ~mask;
    tval |= (value << ((offset % 4))*8) & mask;
    ar7240_reg_wr(addr, tval);
}

void cyg_hal_plf_pci_cfg_write_byte (cyg_uint32 bus,
                                     cyg_uint32 devfn,
                                     cyg_uint32 offset,
                                     cyg_uint8  value)
{
    cyg_uint32 addr, tval, mask;

    addr = (AR7240_PCI_DEV_CFGBASE + offset) & ~3;
    mask = 0x000000ff << ((offset % 4)*8);
    tval = ar7240_reg_rd(addr);
    tval = tval & ~mask;
    tval |= (value << ((offset % 4))*8) & mask;
    ar7240_reg_wr(addr, tval);
}

#endif /* defined(CYGPKG_IO_PCI) */
