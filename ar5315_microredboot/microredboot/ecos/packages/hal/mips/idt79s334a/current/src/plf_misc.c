//==========================================================================
//
//      plf_misc.c
//
//      HAL platform miscellaneous functions
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
// Author(s):    tmichals
// Contributors: 
// Date:         2002-09-01
// Purpose:      HAL miscellaneous functions
// Description:  This file contains miscellaneous functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/system.h>

#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling

/* This is the Reference board configuration */							
#include <cyg/hal/idt79rc233x.h>

#include <cyg/io/pci_hw.h>
#include <cyg/io/pci.h>

void  hal_rc334PciInit (void); 					   
static void mmuInit (void);
static void sysDisableBusError (void) ;
static void sysEnableBusError(void);
void   ecosPciConfigOutByte(  int busNo, int devFnNo,int regOffset,unsigned char data );
void   ecosPciConfigOutHalfWord( int busNo,int devFnNo,int regOffset,unsigned short data );
void   ecosPciConfigOutWord( int busNo,int devFnNo,int regOffset,unsigned int data );
unsigned char ecosPciConfigInByte(int busNo, int devFnNo,int regOffset);
unsigned short ecosPciConfigInHalfWord( int busNo,int devFnNo,int regOffset);
unsigned int ecosPciConfigInWord( int busNo, int devFnNo,int regOffset);   							  
void displayLED(char *str, int count);
						   
/*------------------------------------------------------------------------*/

  /* this is called from the kernel */
void hal_platform_init(void)
{

    HAL_ICACHE_INVALIDATE_ALL();    
    HAL_ICACHE_ENABLE();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_ENABLE(); 
	
    displayLED("eCOS", 4);
    hal_if_init();
    mmuInit();
    hal_rc334PciInit();
}



/* PCI Configuration Registers */
#define  PCI_CFG_VENDORID         0x00
#define  PCI_CFG_DEVICEID         0x02
#define  PCI_CFG_COMMAND          0x04
#define  PCI_CFG_STATUS           0x06
#define  PCI_CFG_REVID            0x08 
#define  PCI_CFG_CLASS_CODE       0x09
#define  PCI_CFG_CACHELINE        0x0c
#define  PCI_CFG_LATENCY_TIMER    0x0d
#define  PCI_CFG_HEADER_TYPE      0x0e
#define  PCI_CFG_BIST             0x0f
#define  PCI_CFG_BAR0             0x10
#define  PCI_CFG_BAR1             0x14
#define  PCI_CFG_BAR2             0x18
#define  PCI_CFG_BAR3             0x1c
#define  PCI_CFG_BAR4             0x20
#define  PCI_CFG_BAR5             0x24
#define  PCI_CFG_CIS_POINTER      0x28
#define  PCI_CFG_SUB_VENDORID     0x2c
#define  PCI_CFG_SUB_SYSTEMID     0x2e
#define  PCI_CFG_EXP_ROM          0x30
#define  PCI_CFG_CAPABILITIES     0x34
#define  PCI_CFG_RESERVED1        0x35
#define  PCI_CFG_RESERVED2        0x38
#define  PCI_CFG_INT_LINE         0x3c
#define  PCI_CFG_INT_PIN          0x3d
#define  PCI_CFG_MIN_GRANT        0x3e
#define  PCI_CFG_MAX_LATENCY      0x3f
#define  PCI_CFG_TRDY_TIMEOUT     0x40
#define  PCI_CFG_RETRY_TIMEOUT    0x41

#define RC334_CONFIG0           0x80000000
/* Typical values used in this example */
#define RC334_PCI_CONFIG0 0x0204111D /* Device ID & Vendor ID */
#define RC334_PCI_CONFIG1       0x00200157   /* Command : MWINV, Enable bus master, 
			memory I/O access       */
#define RC334_PCI_CONFIG2       0x06800001   /* Class Code & Revision ID */
#define RC334_PCI_CONFIG3       0x0000ff04   /* BIST, Header Type, Master Latency,
	Cache line size */
#define RC334_PCI_CONFIG4       0xA0000008    /* Memory Base Address Reg, prefetchable  */
#define RC334_PCI_CONFIG5       0x60000000    /* Integrated Controller Reg, non-prefetchable    */
#define RC334_PCI_CONFIG6       0x00800001    /* IO Base Address Reg      */
#define RC334_PCI_CONFIG7       0x00000000    /* Unused BAR space, assign some address 
	that never gets generated on PCI Bus      */

/* Reserved  registers */
#define RC334_PCI_CONFIG8       0x00000000
#define RC334_PCI_CONFIG9       0x00000000
#define RC334_PCI_CONFIG10      0x00000000

/* Subsystem ID and the subsystem Vendor ID */
#define RC334_PCI_CONFIG11      0x00000000

/* Reserved registers */
#define RC334_PCI_CONFIG12      0x00000000
#define RC334_PCI_CONFIG13      0x00000000
#define RC334_PCI_CONFIG14      0x00000000

/* Max latency, Min Grant, Interrupt pin and interrupt line */
#define RC334_PCI_CONFIG15      0x38080101

/* Retry timeout value, TRDY timeout value. Set to default 0x80 */
#define RC334_PCI_CONFIG16      0x00008080

/* Rc32334 specific PCI registers                  */
#define RC334_PCI_REG_BASE       0xb8000000
#define RC334_CPUTOPCI_BASE_REG1  (RC334_PCI_REG_BASE + 0x20B0)
#define RC334_CPUTOPCI_BASE_REG2  (RC334_PCI_REG_BASE + 0x20B8)
#define RC334_CPUTOPCI_BASE_REG3  (RC334_PCI_REG_BASE + 0x20C0)
#define RC334_CPUTOPCI_BASE_REG4    (RC334_PCI_REG_BASE + 0x20C8)

#define RC334_PCI_ARB_REG   (RC334_PCI_REG_BASE + 0x20E0)
#define RC334_PCITOCPU__BASE_REG1  (RC334_PCI_REG_BASE + 0x20E8)
#define RC334_PCITOCPU__BASE_REG2   (RC334_PCI_REG_BASE + 0x20F4)
#define RC334_PCITOCPU__BASE_REG3   (RC334_PCI_REG_BASE + 0x2100)
#define RC334_PCITOCPU__BASE_REG4   (RC334_PCI_REG_BASE + 0x210C)

/* Considering a typical case  */
#define CPUTOPCI_BASE_REG1_VAL      0x40000001
#define CPUTOPCI_BASE_REG2_VAL      0x00000000
#define CPUTOPCI_BASE_REG3_VAL      0x00000000
#define CPUTOPCI_BASE_REG4_VAL      0x18800001 

//TCM#define PCITOCPU_BASE_REG3_VAL      0x00000000
//TCM#define PCITOCPU_BASE_REG4_VAL      0x18000051    /*    Size field set to 0x14 : 1MB size    */

#define  RC334_PCITOCPU_BASE_REG1           (RC334_PCI_REG_BASE+0x20E8)
#define  RC334_PCITOCPU_BASE_REG2           (RC334_PCI_REG_BASE+0x20F4)
#define  RC334_PCITOCPU_BASE_REG3           (RC334_PCI_REG_BASE+0x2100)
#define  RC334_PCITOCPU_BASE_REG4           (RC334_PCI_REG_BASE+0x210C)

#define  PCITOCPU_MEM_BASE(addr)  ( (addr & 0xFFFFFF)<<8)
#define  PCITOCPU_SIZE(i)                      ( ( i & 0x1F) << 2 )
#define  PCITOCPU_EN_SWAP                 1

#define  SIZE_1MB                                      0x14
#define  SIZE_64MB                                    0x1A

#define SYS_MEM_BASE                          0x0              /* local sdram starting address */
#define RC32334_INT_REG_BASE           0x18000000 /* Integrated controller's internal registers */

/* PCI Target Control Register is provided in the RC32334 to utilize
eager prefetches and reduce target disconnects and retries. In the
following example, an optimized value is picked that enables eager
prefetch for all BAR's, enables Memory Write and Memory Write and
Invalidate (MWMWI), uses threshold for target write FIFO of 8 words,
and sets disconnect and retry timer to 40 PCI clocks */

#define  PCI_TARGET_CONTROL_REG           0xB80020A4
#define  PCI_TARGET_CONTROL_REG_VAL       0x7EF02828

/* BAR1 is selected as memory base register with 64 Mbyte address
range starting at physical address 0x0000_0000, allowing external PCI
masters to access the local SDRAM for data read and write. This
register setting works with the BAR1 register in the PCI configuration
register in the PCI bridge of the RC32334 which, in this example, has
been set to 0xA000_0000. With the given settings, the external PCI
masters can access addresses in the range 0xA000_0000 through
0xA3FF_FFFF using BAR1 which gets translated to address range
0x0000_0000 through 0x03FF_FFFF on the local CPU bus owing to the
PCITOCPU_BASE_REG1 settings. */

#define  PCITOCPU_BASE_REG1_VAL   ((PCITOCPU_MEM_BASE(SYS_MEM_BASE)) | \
                                              (PCITOCPU_SIZE(SIZE_64MB) ) | \
                                              (PCITOCPU_EN_SWAP) )

/* BAR2 is selected as memory base register with 1 Mbyte range
starting at the physical address 0x1800_0000. This maps to the RC32334
internal registers allowing external PCI masters to read/modify the
RC32334 registers. In this example, the BAR2 register in the PCI
configuration register of the RC32334 PCI bridge has been set to
0xB800_0000 (note that this address is in the PCI space and should not
be confused with the CPU address map). External PCI masters can access
memory range 0xB800_0000 through 0xB80F_FFFF, sufficient enough to
access all the RC32334 internal registers. The PCITOCPU_BASE_REG2
settings map all PCI cycles falling in the above range to physical
address range 0x1800_0000 — 0x180F_FFFF on the local CPU bus.  */

#define  PCITOCPU_BASE_REG2_VAL   ((PCITOCPU_MEM_BASE(RC32334_INT_REG_BASE)) | \
                                              (PCITOCPU_SIZE(SIZE_1MB) ) | \
                                              (PCITOCPU_EN_SWAP) )

/* BAR3 is selected as IO base register with 1 Mbyte range mapped to
address 0x1800_0000, providing another window for accessing the
Integrated controller registers. In this example, the value for the
BAR3 has been picked as 0x0000_0000 (address range 0x0000_00000
through 0x000F_FFFF). Any PCI IO cycles to this address range would
get translated to local CPU address range of 0x1800_0000 through
0x180F_FFFF using this register settings. */

#define  PCITOCPU_BASE_REG3_VAL   (        \
                                              (PCITOCPU_MEM_BASE(RC32334_INT_REG_BASE)) | \
                                              (PCITOCPU_SIZE(SIZE_1MB) ) | \
                                              (PCITOCPU_EN_SWAP) )

/* BAR4 register is not used. Therefore, it can be disabled by selecting the SIZE value 1-7 */
#define  PCITOCPU_BASE_REG4_VAL     ( PCITOCPU_SIZE( 1 ) )

/* Arbitration register value:
   Target Ready, internal arbiter, fixed priority
*/
#define PCI_ARB_REG_VAL           0x00000001

/*  Rc32334 config address/data definitions  */
#define PCI_CONFIG_ADDR_REG      0xb8002cf8
#define PCI_CONFIG_DATA_REG      0xb8002cfc

/* BYTE SWAP macros */
#define HALF_WORD_SWAP(x) \
             (  ( ( x  << 8 ) & 0xff00)    |  \
                 ( (x  >> 8 ) & 0x00ff )  )

#define WORD_SWAP(x)\
             ( ( ( x << 24 )  & 0xff000000 ) |  \
                ( (x << 8   )  & 0x00ff0000 ) |  \
                ( (x >> 8   )  & 0x0000ff00 ) |  \
                ( (x >> 24 )  & 0x000000ff )    )

/* PCI Functions */
unsigned int   pciConfigInWord( int busNo, int devNo, int funcNo, int regOffset);
unsigned short pciConfigInHalfWord( int busNo, int devNo, int funcNo, int regOffset);
unsigned char pciConfigInByte( int busNo, int devNo, int funcNo, int regOffset);
void   pciConfigOutWord ( int busNo, int devNo, int funcNo, int regOffset, unsigned int data );
void   pciConfigOutHalfWord ( int busNo, int devNo, int funcNo, int regOffset, unsigned short data );
void   pciConfigOutChar ( int busNo, int devNo, int funcNo, int regOffset, unsigned char data );

/* Rc32334 Bus error register. The bit7 of this register can be used
to enable or disable the BusError. The bus error is disabled briefly
at the time of pci Scanning and enabled thereafter. */

#define  RC334_BUS_ERR_CNTL_REG  0xb8000010

/* 
    Function name       : hal_rc334PciInit
    Parameters passed  : none
    return value       : none
    The function initialises the configuration registers of    Rc32334 PCI interface controller.
*/
void hal_rc334PciInit ( ) 
{

        unsigned int pciConfigData[17];
        int index                     ;
        volatile unsigned int *configAddrReg   ;
        volatile unsigned int *configDataReg   ;
        volatile unsigned int  *regPointer     ;

        configAddrReg  = (volatile unsigned int*) PCI_CONFIG_ADDR_REG;
        configDataReg  = (volatile unsigned int*) PCI_CONFIG_DATA_REG;

        pciConfigData[0]    = RC334_PCI_CONFIG0;
        pciConfigData[1]    = RC334_PCI_CONFIG1;
        pciConfigData[2]    = RC334_PCI_CONFIG2;
        pciConfigData[3]    = RC334_PCI_CONFIG3;
        pciConfigData[4]    = RC334_PCI_CONFIG4;
        pciConfigData[5]    = RC334_PCI_CONFIG5;
        pciConfigData[6]    = RC334_PCI_CONFIG6;
        pciConfigData[7]    = RC334_PCI_CONFIG7;
        pciConfigData[8]    = RC334_PCI_CONFIG8;
        pciConfigData[9]    = RC334_PCI_CONFIG9;
        pciConfigData[10]  = RC334_PCI_CONFIG10;
        pciConfigData[11]  = RC334_PCI_CONFIG11;
        pciConfigData[12]  = RC334_PCI_CONFIG12;
        pciConfigData[13]  = RC334_PCI_CONFIG13;
        pciConfigData[14]  = RC334_PCI_CONFIG14;
        pciConfigData[15]  = RC334_PCI_CONFIG15;
        pciConfigData[16]  = RC334_PCI_CONFIG16;

        *configAddrReg = (unsigned int)RC334_CONFIG0 ;
/*   This example writes to all the configuration registers. Some of
the PCI configuration registers (such as Device ID, Vendor ID, Class
Code, Revision ID, BIST, Header Type, Subsystem Vendor ID, Maximum
Latency, Minimum Grant, Interrupt Pin) need not be initialized */

        for (index =0; index <17; index++ )
                {
                *configDataReg = pciConfigData[index];
                *configAddrReg = *configAddrReg + 4;
                }

       /* Park the Address Register */
       configAddrReg = ( volatile unsigned int*)0x0 ; 

/* Set Rc32334 specific registers */

regPointer = ( volatile unsigned int*)(RC334_CPUTOPCI_BASE_REG1) ;
*regPointer = (unsigned int)(CPUTOPCI_BASE_REG1_VAL );

regPointer = (volatile unsigned int*)(RC334_CPUTOPCI_BASE_REG2) ;
*regPointer = (unsigned int)(CPUTOPCI_BASE_REG2_VAL );

regPointer = (volatile unsigned int*)(RC334_CPUTOPCI_BASE_REG3) ;
*regPointer = (unsigned int)(CPUTOPCI_BASE_REG3_VAL );

regPointer = (volatile unsigned int*)(RC334_CPUTOPCI_BASE_REG4) ;
*regPointer = (unsigned int)(CPUTOPCI_BASE_REG4_VAL );

regPointer = (volatile unsigned int*)(RC334_PCITOCPU_BASE_REG1) ;
*regPointer = (unsigned int)(PCITOCPU_BASE_REG1_VAL ); 

regPointer = ( volatile unsigned int*)(RC334_PCITOCPU_BASE_REG2) ;
*regPointer = (unsigned int)(PCITOCPU_BASE_REG2_VAL );

regPointer = (volatile unsigned int*)(RC334_PCITOCPU_BASE_REG3) ;
*regPointer = (unsigned int)(PCITOCPU_BASE_REG3_VAL ); 

regPointer = (volatile unsigned int*)(RC334_PCITOCPU_BASE_REG4) ;
*regPointer = (unsigned int)(PCITOCPU_BASE_REG4_VAL );

regPointer = (volatile unsigned int*)PCI_TARGET_CONTROL_REG ;
*regPointer = (unsigned int)PCI_TARGET_CONTROL_REG_VAL ;

regPointer = (volatile unsigned int*)(RC334_PCI_ARB_REG);
*regPointer = (unsigned int)(PCI_ARB_REG_VAL);

}

/*     Function name :   sysDisableBusError
                         Disables the Bus Error prior to pciScan.
*/
static void sysDisableBusError (  ) {
        unsigned int*  regPointer ;
        unsigned int    data      ;
        regPointer = (unsigned int*) ( RC334_BUS_ERR_CNTL_REG);
        data       = *regPointer  ;
        /* Set bit7 to disable busError */
        data   =  data |  0x00000080 ;
        *regPointer = data ;
}

/*  Function name :   sysEnableBusError
                      Enables the Bus Error after pciScan
*/
static void sysEnableBusError (  ) {
        unsigned int*  regPointer ;
        unsigned int    data      ;
        regPointer = (unsigned int*) ( RC334_BUS_ERR_CNTL_REG);
        data       = *regPointer  ;
        /* Reset bit7 to enable busError */
        data   =  data & 0xffffff7f;
        *regPointer = data ;
}




#define TLB_HI_MASK                          0xffffe000
#define TLB_LO_MASK                         0x3fffffff
#define PAGEMASK_SHIFT                        13
#define TLB_LO_SHIFT                                 6
#define PCI_PAGE_SIZE                          0x01000000    /* 16 Mbyte */
#define MMU_PAGE_UNCACHED        0x00000010
#define MMU_PAGE_DIRTY                  0x00000004
#define MMU_PAGE_VALID                  0x00000002
#define MMU_PAGE_GLOBAL              0x00000001
#define PCI_MMU_PAGEMASK             0x00000fff
#define PCI_MMU_PAGEATTRIB    (MMU_PAGE_UNCACHED|MMU_PAGE_DIRTY| MMU_PAGE_VALID|MMU_PAGE_GLOBAL)
#define PCI_MEMORY_SPACE1           0x40000000
#define PCI_MEMORY_SPACE2           0x60000000
#define PCI_IO_SPACE                           0x18000000
/*
   Function name : mmuInit
                  Tlb Initialisation for the PCI  memory/IO windows.
*/
static void mmuInit (  ) {
        unsigned int Tlb_Attrib ;
        unsigned int Tlb_Hi     ;
        unsigned int Tlb_Lo0    ;
        unsigned int Tlb_Lo1    ;
        unsigned int Page_Size  ;
        unsigned int pageFrame  ;
        unsigned int Tlb_Inx    ;
       
        /* Uncached, dirty, global and valid MMU page */
          Tlb_Attrib = PCI_MMU_PAGEATTRIB ;

        Page_Size = PCI_MMU_PAGEMASK   ;
        Page_Size = (Page_Size << (PAGEMASK_SHIFT));
        hal_setPageSize(Page_Size);

/*
 * MMU mapping for PCI_MEMORY_SPACE1
 * Map 16MB pages
 * Virtual 0x40000000-0x40ffffff to Physical 0x40000000 - 0x40ffffff
 * Virtual 0x41000000-0x41ffffff to Physical 0x41000000 - 0x41ffffff
 */

        Tlb_Hi     = PCI_MEMORY_SPACE1     ; /* VPN2:VirtualPageframeNumber%2 */
        Tlb_Hi     = (Tlb_Hi & TLB_HI_MASK)  ;

        pageFrame  = PCI_MEMORY_SPACE1       ;
                                                                                  /* Even PFN:Page Frame Number */
        pageFrame  = pageFrame >> TLB_LO_SHIFT;
        Tlb_Lo0     = pageFrame ;
        Tlb_Lo0    = ( Tlb_Lo0 | Tlb_Attrib) ;
        Tlb_Lo0    = ( Tlb_Lo0 & TLB_LO_MASK);

        pageFrame  = (PCI_MEMORY_SPACE1 | PCI_PAGE_SIZE) ;
                                                                                /* Odd PFN:Page Frame Number*/
        pageFrame  = pageFrame >> TLB_LO_SHIFT ; 
        Tlb_Lo1    = pageFrame               ;
        Tlb_Lo1    = ( Tlb_Lo1 | Tlb_Attrib) ;
        Tlb_Lo1    = ( Tlb_Lo1 & TLB_LO_MASK);
        Tlb_Inx    =           0             ;
        hal_setTlbEntry(Tlb_Inx, Tlb_Hi, Tlb_Lo0, Tlb_Lo1);

/*
 * MMU mapping for PCI_MEMORY_SPACE2
 * Virtual 0x60000000-0x60ffffff to Physical 0x60000000 - 0x60ffffff
 * Virtual 0x61000000-0x61ffffff to Physical 0x61000000 - 0x61ffffff
 */
        Tlb_Hi     = PCI_MEMORY_SPACE2       ;   /* VPN2    */
        Tlb_Hi     = ( Tlb_Hi & TLB_HI_MASK );

        pageFrame  = PCI_MEMORY_SPACE2       ;
        pageFrame  = pageFrame >> TLB_LO_SHIFT ; /*Even PFN */
        Tlb_Lo0    = pageFrame               ;

        Tlb_Lo0    = ( Tlb_Lo0 | Tlb_Attrib) ;
        Tlb_Lo0    = ( Tlb_Lo0 & TLB_LO_MASK);

        pageFrame  = ( PCI_MEMORY_SPACE2 | PCI_PAGE_SIZE ) ;
        pageFrame  = pageFrame >> TLB_LO_SHIFT ; /* Odd PFN */
        Tlb_Lo1    = pageFrame               ;
        Tlb_Lo1    = ( Tlb_Lo1 | Tlb_Attrib) ;
        Tlb_Lo1    = ( Tlb_Lo1 & TLB_LO_MASK);
        Tlb_Inx    =           1             ;
        hal_setTlbEntry(Tlb_Inx, Tlb_Hi, Tlb_Lo0, Tlb_Lo1);

/*
 * MMU mapping PCI IO space
 * Virtual 0x18000000-0x18ffffff to Physical 0x18000000 - 0x18ffffff
 * Virtual 0x19000000-0x19ffffff to Physical 0x19000000 - 0x19ffffff
 */
        Tlb_Hi     = PCI_IO_SPACE            ;   /* VPN2   */
        Tlb_Hi     = ( Tlb_Hi & TLB_HI_MASK );

        pageFrame  = PCI_IO_SPACE            ;

        pageFrame  = pageFrame >> TLB_LO_SHIFT ; /* Even PFN */
        Tlb_Lo0    = pageFrame               ;
        Tlb_Lo0    = ( Tlb_Lo0 | Tlb_Attrib) ;
        Tlb_Lo0    = ( Tlb_Lo0 & TLB_LO_MASK);

        pageFrame  = (PCI_IO_SPACE | PCI_PAGE_SIZE) ;
        pageFrame  = pageFrame >> TLB_LO_SHIFT ; /* Odd PFN  */
        Tlb_Lo1    = pageFrame               ;
        Tlb_Lo1    = ( Tlb_Lo1 | Tlb_Attrib) ;
        Tlb_Lo1    = ( Tlb_Lo1 & TLB_LO_MASK);
        Tlb_Inx    =           2             ;
        hal_setTlbEntry(Tlb_Inx, Tlb_Hi, Tlb_Lo0, Tlb_Lo1);
    }




/* ecos PCI functions */


 void   ecosPciConfigOutByte
        (  int busNo, 
           int devFnNo,
           int regOffset,
           unsigned char data ){

          unsigned int    address ;

          address   = (  ( (busNo << 16) & 0x00ff0000 ) |
                         ( ( devFnNo << 8 )  & 0x0000ff00 )
                            );
          address = ( address | 0x80000000 | (regOffset ) );
          hal_sysConfigOutByte(address, data, (regOffset & 0x3) );
}

void   ecosPciConfigOutHalfWord
       ( int busNo,
         int devFnNo,
         int regOffset,
         unsigned short data ){
        
         unsigned int       address ;

         address   = (  ( (busNo << 16) & 0x00ff0000 ) |
                        ( ( devFnNo << 8 ) & 0x0000ff00) 
                            );
         address = ( address | 0x80000000 | (regOffset ) );
         hal_sysConfigOutHalfWord(address, data, (regOffset & 0x3) );
}


void   ecosPciConfigOutWord
        ( int busNo,
          int devFnNo,
          int regOffset,
          unsigned int data ){
         
          unsigned int    address ;
          address   = (  ( (busNo << 16) & 0x00ff0000 ) |
                         ( ( devFnNo << 8 ) & 0x0000ff00) 
                            );
          address = ( address | 0x80000000 | (regOffset ) );

          hal_sysConfigOutWord(address, data);
}

unsigned char ecosPciConfigInByte
        (int busNo, 
         int devFnNo,
         int regOffset
		  ){
        
          unsigned int    address ;
          unsigned char  retVal   ;

         address   = (  ( (busNo << 16) & 0x00ff0000 ) |
                        ( ( devFnNo << 8 ) & 0x0000ff00) 
                     );
         address = ( address | 0x80000000 | (regOffset ) );
		 sysDisableBusError( );
         retVal  = (unsigned char)(hal_sysConfigInByte(address)); 
		 sysEnableBusError( );
         return ( retVal );
}

unsigned short ecosPciConfigInHalfWord
        ( int busNo,
          int devFnNo,
          int regOffset
		  ){
         
          unsigned int            address;
          unsigned short           retVal;

          address   = (  ( (busNo << 16) & 0x00ff0000 ) |
                         ( ( devFnNo << 8 ) & 0x0000ff00) 
                       );
         address = ( address | 0x80000000 | (regOffset ) );
		 sysDisableBusError( );
         retVal = (unsigned short)hal_sysConfigInHalfWord(address); 
		 sysEnableBusError( );		 
		 return retVal;

}
unsigned int ecosPciConfigInWord
        ( int busNo, 
          int devFnNo,
          int regOffset
		  ){
         
          unsigned int            address;
          unsigned int           retVal;

          address   = (  ( (busNo << 16) & 0x00ff0000 ) |
                              ( ( devFnNo << 8 ) & 0x0000ff00) 
                       );
         address = ( address | 0x80000000 | regOffset );
		 sysDisableBusError( );
         retVal  = hal_sysConfigInWord(address);
		 sysEnableBusError( );
		 return retVal;
}


void displayLED(char *str, int count)
{
	char *pChar = (char *)0xB4000000;
	
	char temp;

	/* clear */
	temp = pChar[0x400];

	if (count)
		pChar[0xf]= str[0];
	else
		return;

	if (--count)
		pChar[0xb]= str[1];
	else
		return;

	if (--count)
		pChar[0x7]= str[2];
	else
		return;

	if (--count)
		pChar[0x3]= str[3];
	else
		return;

}

/*------------------------------------------------------------------------*/
/* End of plf_misc.c                                                      */

