#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef __ASSEMBLER__
#else
extern cyg_uint32 cyg_hal_plf_pci_cfg_read_dword (cyg_uint32 bus,
                                                  cyg_uint32 devfn,
                                                  cyg_uint32 offset);
extern cyg_uint16 cyg_hal_plf_pci_cfg_read_word  (cyg_uint32 bus,
                                                  cyg_uint32 devfn,
                                                  cyg_uint32 offset);
extern cyg_uint8 cyg_hal_plf_pci_cfg_read_byte   (cyg_uint32 bus,
                                                  cyg_uint32 devfn,
                                                  cyg_uint32 offset);
extern void cyg_hal_plf_pci_cfg_write_dword (cyg_uint32 bus,
                                             cyg_uint32 devfn,
                                             cyg_uint32 offset,
                                             cyg_uint32 val);
extern void cyg_hal_plf_pci_cfg_write_word  (cyg_uint32 bus,
                                             cyg_uint32 devfn,
                                             cyg_uint32 offset,
                                             cyg_uint16 val);
extern void cyg_hal_plf_pci_cfg_write_byte   (cyg_uint32 bus,
                                              cyg_uint32 devfn,
                                              cyg_uint32 offset,
                                              cyg_uint8 val);

// Initialize the PCI bus.
externC void cyg_hal_plf_pci_init(void);
#define HAL_PCI_INIT() cyg_hal_plf_pci_init()


// Map PCI device resources starting from these addresses in PCI space.
#define HAL_PCI_ALLOC_BASE_MEMORY 0x10000000
#define HAL_PCI_ALLOC_BASE_IO     0x00000000

// This is where the PCI spaces are mapped in the CPU's address space.
//
#define HAL_PCI_PHYSICAL_MEMORY_BASE 0xA0000000
#define HAL_PCI_PHYSICAL_IO_BASE     0xA0000000

// Read a value from the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_READ_UINT8( __bus, __devfn, __offset, __val )  \
    __val = cyg_hal_plf_pci_cfg_read_byte((__bus),  (__devfn), (__offset))

#define HAL_PCI_CFG_READ_UINT16( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_word((__bus),  (__devfn), (__offset))

#define HAL_PCI_CFG_READ_UINT32( __bus, __devfn, __offset, __val ) \
    __val = cyg_hal_plf_pci_cfg_read_dword((__bus),  (__devfn), (__offset))

// Write a value to the PCI configuration space of the appropriate
// size at an address composed from the bus, devfn and offset.
#define HAL_PCI_CFG_WRITE_UINT8( __bus, __devfn, __offset, __val )  \
    cyg_hal_plf_pci_cfg_write_byte((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT16( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_word((__bus),  (__devfn), (__offset), (__val))

#define HAL_PCI_CFG_WRITE_UINT32( __bus, __devfn, __offset, __val ) \
    cyg_hal_plf_pci_cfg_write_dword((__bus),  (__devfn), (__offset), (__val))


#ifndef CYGNUM_HAL_INTERRUPT_PCI_VEC
#define CYGNUM_HAL_INTERRUPT_PCI_VEC            0
#endif


// Translate the PCI interrupt requested by the device (INTA#, INTB#,
// INTC# or INTD#) to the associated CPU interrupt (i.e., HAL vector).
#define HAL_PCI_TRANSLATE_INTERRUPT( __bus, __devfn, __vec, __valid)          \
    CYG_MACRO_START                                                           \
    __vec = CYGNUM_HAL_INTERRUPT_PCI_VEC;                                     \
    __valid = true;                                                           \
    CYG_MACRO_END



void ar7240_gpio_drive_low(cyg_uint32 mask);
void ar7240_gpio_drive_high(cyg_uint32 mask);
cyg_uint32 ar7240_gpio_float_high_test(cyg_uint32 mask);
void ar7240_gpio_enable_slic(void);
void ar7240_gpio_enable_uart(void);
void ar7240_gpio_enable_stereo(void);
void ar7240_gpio_enable_spi_cs1_cs0(void);
void ar7240_gpio_enable_i2c_on_gpio_0_1(void);

#endif /* __ASSEMBLER__ */




// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
