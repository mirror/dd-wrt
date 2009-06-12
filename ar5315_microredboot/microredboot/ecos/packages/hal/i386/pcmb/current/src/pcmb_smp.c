//==========================================================================
//
//      pcmb_smp.c
//
//      HAL SMP implementation
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
// Author(s):    nickg
// Contributors: nickg
// Date:         2001-08-03
// Purpose:      HAL SMP implementation
// Description:  This file contains SMP support functions.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/hal_i386.h>
#include <pkgconf/hal_i386_pcmb.h>

#ifdef CYGPKG_HAL_SMP_SUPPORT

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_smp.h>

// ------------------------------------------------------------------------
// Debugging/diagnostic controls

// Setting this to 1 causes the parsing of the MP structures and the
// initialization of the APIC and IOAPIC to be reported on the
// diagnostic channel.
#define SHOW_DIAGNOSTICS 0

// Setting this to 1 causes various things to be displayed on the PC
// screen during normal operation. These are mostly for my own use,
// and may be somewhat obscure to anyone else.
#define SCREEN_DIAGNOSTICS 0

#if SCREEN_DIAGNOSTICS==0
#undef PC_WRITE_SCREEN
#undef PC_WRITE_SCREEN_8
#undef PC_WRITE_SCREEN_16
#undef PC_WRITE_SCREEN_32
#define PC_WRITE_SCREEN( pos, ch )
#define PC_WRITE_SCREEN_8( pos, val )
#define PC_WRITE_SCREEN_16( pos, val )
#define PC_WRITE_SCREEN_32( pos, val )
#endif

// ------------------------------------------------------------------------

static int streq( const char *s1, const char *s2 )
{
    while( *s1 == *s2 && *s1 && *s2 ) s1++,s2++;

    return !(*s2-*s1);
}

/*------------------------------------------------------------------------*/
// MP FPS structure:

#define MP_FPS_SIGNATURE         0
#define MP_FPS_MPCT              4
#define MP_FPS_LENGTH            8
#define MP_FPS_REV               9
#define MP_FPS_CSUM             10
#define MP_FPS_FEATURE1         11
#define MP_FPS_FEATURE2         12
#define MP_FPS_FEATURE3         13
#define MP_FPS_FEATURE4         14
#define MP_FPS_FEATURE5         15
#define MP_FPS_SIZE             16

#define MP_FPS_SIGNATURE_VALUE  0x5f504d5f

/*------------------------------------------------------------------------*/
// MP config table structure

// Header structure:

#define MPCT_HDR_SIGNATURE      0
#define MPCT_HDR_LENGTH         4
#define MPCT_HDR_REV            6
#define MPCT_HDR_CHECKSUM       7
#define MPCT_HDR_OEM_ID         8
#define MPCT_HDR_PROD_ID        16
#define MPCT_HDR_OEM_TAB        28
#define MPCT_HDR_OEM_TAB_SIZE   32
#define MPCT_HDR_ENTRY_COUNT    34
#define MPCT_HDR_LOCAL_APIC     36
#define MPCT_HDR_XTAB_LENGTH    40
#define MPCT_HDR_XTAB_CHECKSUM  42
#define MPCT_HDR_SIZE           44

#define MPCT_HDR_SIGNATURE_VAL  0x504d4350

#define MPCT_ENTRY_TYPE_PROCESSOR       0
#define MPCT_ENTRY_TYPE_BUS             1
#define MPCT_ENTRY_TYPE_IOAPIC          2
#define MPCT_ENTRY_TYPE_INTERRUPT_IO    3
#define MPCT_ENTRY_TYPE_INTERRUPT_LOCAL 4

#define MPCT_ENTRY_PROC_TYPE            0
#define MPCT_ENTRY_PROC_APIC_ID         1
#define MPCT_ENTRY_PROC_APIC_VER        2
#define MPCT_ENTRY_PROC_CPU_FLAGS       3
#define MPCT_ENTRY_PROC_CPU_SIGNATURE   4
#define MPCT_ENTRY_PROC_FEATURE_FLAGS   8
#define MPCT_ENTRY_PROC_RESERVED0       12
#define MPCT_ENTRY_PROC_RESERVED1       16
#define MPCT_ENTRY_PROC_SIZE            20

#define MPCT_ENTRY_BUS_TYPE             0
#define MPCT_ENTRY_BUS_ID               1
#define MPCT_ENTRY_BUS_TYPE_STRING      2
#define MPCT_ENTRY_BUS_SIZE             8

#define MPCT_ENTRY_IOAPIC_TYPE          0
#define MPCT_ENTRY_IOAPIC_ID            1
#define MPCT_ENTRY_IOAPIC_VER           2
#define MPCT_ENTRY_IOAPIC_FLAGS         3
#define MPCT_ENTRY_IOAPIC_ADDRESS       4
#define MPCT_ENTRY_IOAPIC_SIZE          8

#define MPCT_ENTRY_IOINT_TYPE           0
#define MPCT_ENTRY_IOINT_INT_TYPE       1
#define MPCT_ENTRY_IOINT_FLAGS          2
#define MPCT_ENTRY_IOINT_SOURCE_BUS     4
#define MPCT_ENTRY_IOINT_SOURCE_IRQ     5
#define MPCT_ENTRY_IOINT_DEST_APIC      6
#define MPCT_ENTRY_IOINT_DEST_INT       7
#define MPCT_ENTRY_IOINT_SIZE           8

#define MPCT_ENTRY_LOCINT_TYPE          0
#define MPCT_ENTRY_LOCINT_INT_TYPE      1
#define MPCT_ENTRY_LOCINT_FLAGS         2
#define MPCT_ENTRY_LOCINT_SOURCE_BUS    4
#define MPCT_ENTRY_LOCINT_SOURCE_IRQ    5
#define MPCT_ENTRY_LOCINT_DEST_APIC     6
#define MPCT_ENTRY_LOCINT_DEST_INT      7
#define MPCT_ENTRY_LOCINT_SIZE          8

/*------------------------------------------------------------------------*/
// Exported SMP configuration

CYG_ADDRESS cyg_hal_smp_local_apic;

CYG_ADDRESS cyg_hal_smp_io_apic;

CYG_WORD32 cyg_hal_smp_cpu_count = 0;

CYG_BYTE cyg_hal_smp_cpu_flags[HAL_SMP_CPU_MAX];

CYG_BYTE cyg_hal_isa_bus_id = 0xff;
CYG_BYTE cyg_hal_isa_bus_irq[16];

CYG_BYTE cyg_hal_pci_bus_id = 0xff;
CYG_BYTE cyg_hal_pci_bus_irq[4];

HAL_SPINLOCK_TYPE cyg_hal_ioapic_lock;

/*------------------------------------------------------------------------*/

// Statics

static CYG_ADDRESS     mp_fps;
static CYG_ADDRESS     mpct;


/*------------------------------------------------------------------------*/

static CYG_WORD32 mp_checksum(CYG_BYTE *p, CYG_WORD32 len)
{
	CYG_WORD32 sum = 0;

	while (len--) sum += *p++;

	return sum & 0xFF;
}

/*------------------------------------------------------------------------*/

static cyg_bool cyg_hal_scan_smp_config( CYG_ADDRESS base, CYG_ADDRWORD size )
{

#if SHOW_DIAGNOSTICS                        
    diag_printf("Scan for MP struct: %08x..%08x\n",base,base+size);
#endif    
    
    while( size > 0 )
    {
        CYG_WORD32 sig;

        HAL_READMEM_UINT32( base, sig );
        
        if( sig == MP_FPS_SIGNATURE_VALUE )
        {
            CYG_BYTE val8;
            // We have a candidate for the floating structure

#if SHOW_DIAGNOSTICS                                
            diag_printf("MP_FPS candidate found at %08x\n",base);
#endif
 
            HAL_READMEM_UINT8( base+MP_FPS_LENGTH, val8 );

            if( val8 != 1 )
                break;

            HAL_READMEM_UINT8( base+MP_FPS_REV, val8 );

            if( val8 != 1 && val8 != 4 )
                break;

            if( mp_checksum( (CYG_BYTE *)base, MP_FPS_SIZE ) != 0 )
                break;

            mp_fps = base;

            HAL_READMEM_UINT32( base+MP_FPS_MPCT, mpct );

#if SHOW_DIAGNOSTICS                                
            diag_printf("MPCT at %08x\n",mpct);
#endif
            return 1; 
        }

        base += 16;
        size -= 16;
    }

    return 0;
}

/*------------------------------------------------------------------------*/

static cyg_bool cyg_hal_find_smp_config( void )
{
    // check bottom 1k
    if( cyg_hal_scan_smp_config(0x00000, 0x00400 ) )
        return 1;

    // check top 1k of RAM
    if( cyg_hal_scan_smp_config(0x9fc00, 0x00400 ) )
        return 1;

    // check BIOS ROM
    if( cyg_hal_scan_smp_config(0xf0000, 0x10000 ) )
        return 1;    
}

/*------------------------------------------------------------------------*/

static cyg_bool cyg_hal_parse_smp_config( void )
{
    CYG_WORD32 val32;
    CYG_WORD16 val16;
    CYG_BYTE val8;
    int i;
    
#if SHOW_DIAGNOSTICS

    {
        
        diag_printf("FPS address:         %08x\n",mp_fps);
        HAL_READMEM_UINT32( mp_fps+MP_FPS_SIGNATURE, val32);
        diag_printf("FPS signature:       %08x\n",val32);
        HAL_READMEM_UINT32( mp_fps+MP_FPS_MPCT, val32);
        diag_printf("FPS MPCT address:    %08x\n",val32);
        HAL_READMEM_UINT8( mp_fps+MP_FPS_LENGTH, val8);
        diag_printf("FPS length:          %02x\n",val8);
        HAL_READMEM_UINT8( mp_fps+MP_FPS_REV, val8);
        diag_printf("FPS spec rev:        %02x\n",val8);
        HAL_READMEM_UINT8( mp_fps+MP_FPS_CSUM, val8);
        diag_printf("FPS checksum:        %02x\n",val8);

        for( i = 0; i < 5; i++ )
        {
            HAL_READMEM_UINT8( mp_fps+MP_FPS_FEATURE1, val8);
            diag_printf("FPS feature byte %d:  %02x\n",i,val8);
        }
    }

#endif
    
    if( mpct )
    {

        HAL_READMEM_UINT32( mpct+MPCT_HDR_SIGNATURE, val32);

        if( val32 != MPCT_HDR_SIGNATURE_VAL )
            return 0;

        HAL_READMEM_UINT16( mpct+MPCT_HDR_LENGTH, val16);
        if( mp_checksum( (CYG_BYTE *)mpct, val16 ) != 0 )
            return 0;
        
#if SHOW_DIAGNOSTICS
        {
            char id[13];

            diag_printf("MPCT address:                 %08x\n",mpct);
            HAL_READMEM_UINT32( mpct+MPCT_HDR_SIGNATURE, val32);
            diag_printf("MPCT signature:               %08x\n",val32);

            HAL_READMEM_UINT16( mpct+MPCT_HDR_LENGTH, val16);
            diag_printf("MPCT length:                  %04x\n",val16);
            HAL_READMEM_UINT8( mpct+MPCT_HDR_REV, val8);
            diag_printf("MPCT spec rev:                %02x\n",val8);
            HAL_READMEM_UINT8( mpct+MPCT_HDR_CHECKSUM, val8);
            diag_printf("MPCT checksum:                %02x\n",val8);

            for( i = 0; i < 8; i++ )
            {
                HAL_READMEM_UINT8( mpct+MPCT_HDR_OEM_ID+i, val8);
                id[i] = val8;
            }
            id[i] = 0;
            diag_printf("MPCT OEM Id:                  %s\n",id);        

            for( i = 0; i < 12; i++ )
            {
                HAL_READMEM_UINT8( mpct+MPCT_HDR_PROD_ID+i, val8);
                id[i] = val8;
            }
            id[i] = 0;
            diag_printf("MPCT Product Id:              %s\n",id);

            HAL_READMEM_UINT32( mpct+MPCT_HDR_OEM_TAB, val32);
            diag_printf("MPCT OEM table:               %08x\n",val32);
            HAL_READMEM_UINT16( mpct+MPCT_HDR_OEM_TAB_SIZE, val16);
            diag_printf("MPCT OEM table size:          %04x\n",val16);

            HAL_READMEM_UINT16( mpct+MPCT_HDR_ENTRY_COUNT, val16);
            diag_printf("MPCT entry count:             %04x\n",val16);

            HAL_READMEM_UINT32( mpct+MPCT_HDR_LOCAL_APIC, val32);
            diag_printf("MPCT local APIC:              %08x\n",val32);

            HAL_READMEM_UINT16( mpct+MPCT_HDR_XTAB_LENGTH, val16);
            diag_printf("MPCT extended table length:   %04x\n",val16);
            HAL_READMEM_UINT8( mpct+MPCT_HDR_XTAB_CHECKSUM, val8);
            diag_printf("MPCT extended table checksum: %02x\n",val8);
        }
#endif    

        // Extract the data we need from the structure

        HAL_READMEM_UINT32( mpct+MPCT_HDR_LOCAL_APIC, cyg_hal_smp_local_apic);
        
        // now parse the base table, looking for processor and IOAPIC entries.

        {
            CYG_WORD16 entries;
            CYG_ADDRESS entry = mpct + MPCT_HDR_SIZE;
        
            HAL_READMEM_UINT16( mpct+MPCT_HDR_ENTRY_COUNT, entries);

#if SHOW_DIAGNOSTICS            
            diag_printf("\nBase table:\n");
#endif
            
            while( entries-- )
            {
                CYG_BYTE type;

                HAL_READMEM_UINT8( entry, type );

                switch( type )
                {
                case MPCT_ENTRY_TYPE_PROCESSOR:
#if SHOW_DIAGNOSTICS                    
                    diag_printf("        Processor\n");
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_PROC_APIC_ID, val8 );
                    diag_printf("                APIC ID:                  %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_PROC_APIC_VER, val8 );
                    diag_printf("                APIC Version:             %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_PROC_CPU_FLAGS, val8 );
                    diag_printf("                CPU Flags:                %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_PROC_CPU_SIGNATURE, val32 );
                    diag_printf("                CPU Signature:            %08x\n",val32);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_PROC_FEATURE_FLAGS, val32 );
                    diag_printf("                Feature flags:            %08x\n",val32);
#endif
                    {
                        CYG_BYTE cpuid;

                        // Index CPUs by their APIC IDs
                        HAL_READMEM_UINT8( entry+MPCT_ENTRY_PROC_APIC_ID, cpuid );

                        // Get flags for this CPU.
                        HAL_READMEM_UINT8(entry+MPCT_ENTRY_PROC_CPU_FLAGS, cyg_hal_smp_cpu_flags[cpuid]);
                        
                        cyg_hal_smp_cpu_count++;      // count another CPU
                    }
                    
                    entry += MPCT_ENTRY_PROC_SIZE;
                    break;

                case MPCT_ENTRY_TYPE_BUS:
                    {
                        CYG_BYTE id;
                        char bustype[7];
                        HAL_READMEM_UINT8( entry+MPCT_ENTRY_BUS_ID, id );
#if SHOW_DIAGNOSTICS                    
                        diag_printf("        Bus\n");
                        diag_printf("                ID:                       %02x\n",id);
#endif
                        {
                            int i;
                            for( i = 0; i < 6; i++ )
                            {
                                HAL_READMEM_UINT8( entry+MPCT_ENTRY_BUS_TYPE_STRING+i, val8 );
                                bustype[i] = val8;
                            }
                            bustype[i] = 0;
#if SHOW_DIAGNOSTICS
                            diag_printf("                Type:                     %s\n",&bustype[0]);
#endif                            
                        }

                        if( streq( bustype, "ISA   " ) )
                        {
                            cyg_hal_isa_bus_id = id;
                            for( i = 0; i < 16; i++ )
                                cyg_hal_isa_bus_irq[i] = 100;
                        }
                        if( streq( bustype, "PCI   " ) )
                        {
                            cyg_hal_pci_bus_id = id;
                        }
                    }

                    entry += MPCT_ENTRY_BUS_SIZE;
                    break;

                case MPCT_ENTRY_TYPE_IOAPIC:
#if SHOW_DIAGNOSTICS                    
                    diag_printf("        I/O APIC\n");
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOAPIC_ID, val8 );
                    diag_printf("                ID:                       %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOAPIC_VER, val8 );
                    diag_printf("                Version:                  %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOAPIC_FLAGS, val8 );
                    diag_printf("                Flags:                    %02x\n",val8);
                    HAL_READMEM_UINT32( entry+MPCT_ENTRY_IOAPIC_ADDRESS, val32 );
                    diag_printf("                Address:                  %08x\n",val32);
#endif

                    HAL_READMEM_UINT32( entry+MPCT_ENTRY_IOAPIC_ADDRESS, cyg_hal_smp_io_apic );
                    entry += MPCT_ENTRY_IOAPIC_SIZE;                
                    break;

                case MPCT_ENTRY_TYPE_INTERRUPT_IO:
                    {
                        CYG_BYTE bus, irq, dst;
                        HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_SOURCE_BUS, bus );
                        HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_SOURCE_IRQ, irq );
                        HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_DEST_INT, dst );
#if SHOW_DIAGNOSTICS                    
                        diag_printf("        I/O interrupt assignment\n");
                        HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_TYPE, val8 );
                        diag_printf("                Type:                     %02x\n",val8);
                        HAL_READMEM_UINT16( entry+MPCT_ENTRY_IOINT_TYPE, val16 );
                        diag_printf("                Flags:                    %04x\n",val16);
                        diag_printf("                Source bus:               %02x\n",bus);
                        diag_printf("                Source IRQ:               %02x\n",irq);
                        HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_DEST_APIC, val8 );
                        diag_printf("                Dest APIC:                %02x\n",val8);
                        diag_printf("                Dest Interrupt:           %02x\n",dst);
#endif

                        if( bus == cyg_hal_isa_bus_id )
                            cyg_hal_isa_bus_irq[irq] = dst;
//                        if( bus == cyg_hal_pci_bus_id )
//                            cyg_hal_pci_bus_irq[irq] = dst;
                    
                    }
                    entry += MPCT_ENTRY_IOINT_SIZE;
                    break;

                case MPCT_ENTRY_TYPE_INTERRUPT_LOCAL:
#if SHOW_DIAGNOSTICS                    
                    diag_printf("        Local interrupt assignment\n");
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_TYPE, val8 );
                    diag_printf("                Type:                     %02x\n",val8);
                    HAL_READMEM_UINT16( entry+MPCT_ENTRY_IOINT_TYPE, val16 );
                    diag_printf("                Flags:                    %04x\n",val16);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_SOURCE_BUS, val8 );
                    diag_printf("                Source bus:               %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_SOURCE_IRQ, val8 );
                    diag_printf("                Source IRQ:               %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_DEST_APIC, val8 );
                    diag_printf("                Dest APIC:                %02x\n",val8);
                    HAL_READMEM_UINT8( entry+MPCT_ENTRY_IOINT_DEST_INT, val8 );
                    diag_printf("                Dest Interrupt:           %02x\n",val8);
#endif
                    entry += MPCT_ENTRY_LOCINT_SIZE;
                    break;
                
                default:
#if SHOW_DIAGNOSTICS                    
                    diag_printf("        MPCT Entry: unknown type %02x\n",type);
#endif                    
                    entry += 8;
                    break;
                }
            
            }
        
        }
    }

#if SHOW_DIAGNOSTICS

    diag_printf("Exported configuration:\n");
    diag_printf("        Local APIC: %08x\n", cyg_hal_smp_local_apic );
    diag_printf("        I/O APIC:   %08x\n", cyg_hal_smp_io_apic );
    diag_printf("        CPU count:  %d\n", cyg_hal_smp_cpu_count );

    for( i = 0; i < cyg_hal_smp_cpu_count; i++ )
    {
        diag_printf("            CPU %d %sactive %s\n",i,
                    ((cyg_hal_smp_cpu_flags[i]&1)?"":"in"),
                    ((cyg_hal_smp_cpu_flags[i]&2)?"master":"slave")
            );
    }

    diag_printf("        ISA IRQ map:\n");

    for( i = 0; i < 16; i++ )
    {
        diag_printf("            IRQ %2d -> IOAPIC INT %2d\n",i,cyg_hal_isa_bus_irq[i]);
    }
    
#endif    
    
    return 1;
}

/*------------------------------------------------------------------------*/
// Init local APIC

static cyg_bool cyg_hal_smp_init_apic(void)
{
    
    cyg_uint32 maxlvt;
    cyg_uint32 val;
    HAL_SMP_CPU_TYPE cpu;
    
    HAL_APIC_READ(HAL_APIC_ID, val );
    cpu = val>>24;
    
#if SHOW_DIAGNOSTICS
    diag_printf("Local APIC: %08x\n",cyg_hal_smp_local_apic);
    diag_printf("        ID:             %08x\n",val);
#endif    
    // get max local vector table entry offset
    HAL_APIC_READ(HAL_APIC_VER, maxlvt );
#if SHOW_DIAGNOSTICS
    diag_printf("        VERSION:        %08x\n",maxlvt);
#endif
    maxlvt >>= 16;
    maxlvt &= 0xFF;

    
#if SHOW_DIAGNOSTICS
    diag_printf("maxlvt = %d\n",maxlvt);
#endif

    // Start by ensuring that all interrupt sources are disabled. The
    // following code ensures that this happens cleanly.
    
    // Local timer vector
    HAL_APIC_READ( HAL_APIC_LVT_TIMER, val );
    val |= HAL_APIC_LVT_MASK ;
    HAL_APIC_WRITE( HAL_APIC_LVT_TIMER, val );

#if SHOW_DIAGNOSTICS
    diag_printf("        APIC_LVT_TIMER: %08x\n",val);
#endif
    
    // Local interrupt vectors
    HAL_APIC_READ( HAL_APIC_LVT_INT0, val );
    val |= HAL_APIC_LVT_MASK ;
    HAL_APIC_WRITE( HAL_APIC_LVT_INT0, val );

#if SHOW_DIAGNOSTICS
    diag_printf("        APIC_LVT_INT0:  %08x\n",val);
#endif

    HAL_APIC_READ( HAL_APIC_LVT_INT1, val );
    val |= HAL_APIC_LVT_MASK ;
    HAL_APIC_WRITE( HAL_APIC_LVT_INT1, val );

#if SHOW_DIAGNOSTICS
    diag_printf("        APIC_LVT_INT1:  %08x\n",val);
#endif
 
    if (maxlvt >= 3 )
    {
        HAL_APIC_READ( HAL_APIC_LVT_ERROR, val );
        val |= HAL_APIC_LVT_MASK ;
        HAL_APIC_WRITE( HAL_APIC_LVT_ERROR, val );
#if SHOW_DIAGNOSTICS
        diag_printf("        APIC_LVT_ERROR: %08x\n",val);
#endif 
    }
    if (maxlvt >= 4 )
    {
        HAL_APIC_READ( HAL_APIC_LVT_PC, val );
        val |= HAL_APIC_LVT_MASK ;
        HAL_APIC_WRITE( HAL_APIC_LVT_PC, val );
#if SHOW_DIAGNOSTICS
        diag_printf("        APIC_LVT_PC:    %08x\n",val);
#endif 
    }

    // Now initialize the local vector table.
    
    HAL_APIC_WRITE( HAL_APIC_LVT_TIMER, HAL_APIC_LVT_MASK );
    HAL_APIC_WRITE( HAL_APIC_LVT_INT0, HAL_APIC_LVT_MASK );
    HAL_APIC_WRITE( HAL_APIC_LVT_INT1, HAL_APIC_LVT_MASK );
    if( maxlvt >= 3 )
        HAL_APIC_WRITE( HAL_APIC_LVT_ERROR, HAL_APIC_LVT_MASK );
    if( maxlvt >= 4 )
        HAL_APIC_WRITE( HAL_APIC_LVT_PC, HAL_APIC_LVT_MASK );


    // Set up DFR to flat delivery mode.
    HAL_APIC_WRITE( HAL_APIC_DFR, 0xffffffff );

    // Set up logical destination id. We set bit 1<<cpuid in the LDR
    // register.

    HAL_APIC_READ( HAL_APIC_LDR, val );
    val |= 1<<(cpu+24);
    HAL_APIC_WRITE( HAL_APIC_LDR, val );

    // Set TPR register to accept all.
    HAL_APIC_WRITE( HAL_APIC_TPR, 0 );

    // Enable APIC in SPIV
    HAL_APIC_WRITE( HAL_APIC_SPIV, 0x00000100 );

    
    if( cyg_hal_smp_cpu_flags[HAL_SMP_CPU_THIS()] & 2 )
    {
        // This is the boot CPU, switch its PIC into APIC mode
        // Non-boot CPUs are already in APIC mode.
        
        HAL_WRITE_UINT8( 0x22, 0x70 );
        HAL_WRITE_UINT8( 0x23, 0x01 );
    }
    
    return 1;
}

/*------------------------------------------------------------------------*/
// Initialize I/O APIC

static cyg_bool cyg_hal_smp_init_ioapic(void)
{
    CYG_WORD32 val;
    cyg_uint32 tabsize = 0;
    int i;
    HAL_SMP_CPU_TYPE cpu_this = HAL_SMP_CPU_THIS();

    HAL_SPINLOCK_CLEAR( cyg_hal_ioapic_lock );
    HAL_SPINLOCK_SPIN( cyg_hal_ioapic_lock );
    
    HAL_IOAPIC_READ( HAL_IOAPIC_REG_APICVER, val );
    tabsize = (val>>16)&0xFF;

    // Set up ISA interrupts
    for( i = 0; i < 16; i++ )
    {
        if( cyg_hal_isa_bus_irq[i] != 100 )
        {
            CYG_WORD32 tehi = 0, telo = 0x00010000;

            tehi |= cpu_this<<24;

            telo |= CYGNUM_HAL_ISR_MIN+i;
            
            HAL_IOAPIC_WRITE( HAL_IOAPIC_REG_REDIR_LO(cyg_hal_isa_bus_irq[i]), telo );
            HAL_IOAPIC_WRITE( HAL_IOAPIC_REG_REDIR_HI(cyg_hal_isa_bus_irq[i]), tehi );
        }
    }

    
#if SHOW_DIAGNOSTICS    
    diag_printf("I/O APIC: %08x\n",cyg_hal_smp_io_apic);

    HAL_IOAPIC_READ( HAL_IOAPIC_REG_APICID, val );
    diag_printf("        ID: %08x\n",val);

    HAL_IOAPIC_READ( HAL_IOAPIC_REG_APICVER, val );
    diag_printf("        VER: %08x\n",val);

    HAL_IOAPIC_READ( HAL_IOAPIC_REG_APICARB, val );
    diag_printf("        ARB: %08x\n",val);

    diag_printf("        Redirection Table:\n");
    for( i = 0; i < tabsize; i++ )
    {
        CYG_WORD32 tehi, telo;

        HAL_IOAPIC_READ( HAL_IOAPIC_REG_REDIR_LO(i), telo );
        HAL_IOAPIC_READ( HAL_IOAPIC_REG_REDIR_HI(i), tehi );
        diag_printf("            %02d: %08x %08x\n",i,tehi,telo);
    }
#endif

    HAL_SPINLOCK_CLEAR( cyg_hal_ioapic_lock );

    return 1;
}

/*------------------------------------------------------------------------*/

static volatile CYG_WORD32 init_deasserted;

__externC volatile CYG_WORD32 cyg_hal_smp_cpu_sync_flag[HAL_SMP_CPU_MAX];
__externC volatile CYG_WORD32 cyg_hal_smp_cpu_sync[HAL_SMP_CPU_MAX];
__externC volatile void (*cyg_hal_smp_cpu_entry[HAL_SMP_CPU_MAX])(void);
__externC volatile CYG_WORD32 cyg_hal_smp_vsr_sync_flag;
__externC volatile CYG_WORD32 cyg_hal_smp_cpu_running[HAL_SMP_CPU_MAX];

/*------------------------------------------------------------------------*/

__externC void cyg_hal_smp_start(void);

__externC CYG_BYTE cyg_hal_slave_trampoline[];
__externC CYG_BYTE cyg_hal_slave_trampoline_end[];

#define HAL_SLAVE_START_ADDRESS 0x00002000

__externC void cyg_hal_cpu_start( HAL_SMP_CPU_TYPE cpu )
{
#if 1
    CYG_WORD32 icrlo, icrhi;
    CYG_BYTE *p = &cyg_hal_slave_trampoline[0];
    CYG_BYTE *q = (CYG_BYTE *)HAL_SLAVE_START_ADDRESS;
    CYG_BYTE old_cmos;
    int i;
    
    // Copy the trampoline over...
    do
    {
        *q++ = *p++;
    } while( p != &cyg_hal_slave_trampoline_end[0]);

    // Init synchronization spinlock to locked to halt slave CPU in
    // cyg_hal_smp_startup().

    init_deasserted = 0;
    
    // We now have to execute the grungy and unpleasant AP startup
    // sequence to get the cpu running. I'm sure that not all of this
    // is strictly necessary, but it works and it is too much effort
    // to work out the minimal subset.

    // Write warm-reset code into CMOS RAM and write the address of
    // the trampoline entry point into location 40:67.
    HAL_READ_CMOS( 0x0f, old_cmos );
    HAL_WRITE_CMOS( 0x0f, 0x0a );
    HAL_WRITEMEM_UINT16( 0x467, HAL_SLAVE_START_ADDRESS & 0xf );
    HAL_WRITEMEM_UINT16( 0x469, HAL_SLAVE_START_ADDRESS>>4 );
    
    
    // Send an INIT interrupt to the dest CPU
    icrhi = cpu<<24;
    icrlo = 0x0000C500;

    HAL_APIC_WRITE( HAL_APIC_ICR_HI, icrhi );
    HAL_APIC_WRITE( HAL_APIC_ICR_LO, icrlo );

    hal_delay_us( 10 * 1000 );  // Wait 10ms

    // Wait for the ICR to become inactive
    do {
        HAL_APIC_READ( HAL_APIC_ICR_LO, icrlo );
    } while( (icrlo & 0x00001000) != 0 );

    // Now de-assert INIT

    icrlo = 0x00008500;

    HAL_APIC_WRITE( HAL_APIC_ICR_HI, icrhi );    
    HAL_APIC_WRITE( HAL_APIC_ICR_LO, icrlo );

    init_deasserted = 1;

    // Now we send two STARTUP IPIs

    for( i = 0; i < 2; i++ )
    {
        icrlo = 0x00000600 | (HAL_SLAVE_START_ADDRESS>>12);

        // Send the STARTUP IPI
        HAL_APIC_WRITE( HAL_APIC_ICR_HI, icrhi );
        HAL_APIC_WRITE( HAL_APIC_ICR_LO, icrlo );

        hal_delay_us( 300 );
        
        // Wait for the ICR to become inactive
        do {
            HAL_APIC_READ( HAL_APIC_ICR_LO, icrlo );
        } while( (icrlo & 0x00001000) != 0 );

        hal_delay_us( 300 );
    }
    
    HAL_WRITE_CMOS( 0x0f, old_cmos );

//    PC_WRITE_SCREEN( PC_SCREEN_LINE(5)+0, '!' );    

    hal_delay_us( 300 );
    
//    PC_WRITE_SCREEN( PC_SCREEN_LINE(5)+1, '!' );    

#endif    
}

/*------------------------------------------------------------------------*/

__externC void cyg_hal_smp_start(void);
__externC void cyg_hal_smp_startup(void);

__externC void cyg_hal_cpu_release( HAL_SMP_CPU_TYPE cpu )
{
//    PC_WRITE_SCREEN( PC_SCREEN_LINE(13), '!' );            
//    PC_WRITE_SCREEN_8( PC_SCREEN_LINE(13), cpu );
    
    cyg_hal_smp_cpu_entry[cpu] = cyg_hal_smp_start;

    while( cyg_hal_smp_cpu_entry[cpu] != 0 )
    {
//        PC_WRITE_SCREEN_32( PC_SCREEN_LINE(13)+4, cyg_hal_smp_cpu_entry[cpu] );
        hal_delay_us( 100 );
         continue;
    }
}

/*------------------------------------------------------------------------*/

__externC void cyg_hal_smp_startup(void)
{
    HAL_SMP_CPU_TYPE cpu;

//    PC_WRITE_SCREEN( PC_SCREEN_LINE(2)+0, '!' );

#ifndef CYG_HAL_STARTUP_RAM 
    // Wait for INIT interrupt to be deasserted
    while( !init_deasserted )
        continue;
#endif
    
//    PC_WRITE_SCREEN( PC_SCREEN_LINE(2)+1, '!' );
    
    cpu  = HAL_SMP_CPU_THIS();
    
//    PC_WRITE_SCREEN_8( PC_SCREEN_LINE(2)+6, cpu );
    
#ifndef CYG_HAL_STARTUP_RAM 
    // Wait 1s for the world to settle
    hal_delay_us( 1000000 );

//    PC_WRITE_SCREEN( PC_SCREEN_LINE(2)+2, '!' );        
 
    // Setup our APIC
    cyg_hal_smp_init_apic();
#endif
    
//    PC_WRITE_SCREEN( PC_SCREEN_LINE(2)+3, '!' );        

#ifdef CYGPKG_KERNEL_SMP_SUPPORT			
    cyg_hal_smp_cpu_running[cpu] = 1;
    cyg_kernel_smp_startup();
#else 
    for(;;)
    {
        void (*entry)(void);

        while( (entry = cyg_hal_smp_cpu_entry[cpu]) == 0 )
        {
#if 0 //SCREEN_DIAGNOSTICS                
            static int n;
            PC_WRITE_SCREEN_8( PC_SCREEN_LINE(2)+10, n );
            PC_WRITE_SCREEN_8( PC_SCREEN_LINE(2)+15, cyg_hal_smp_cpu_sync[cpu] );
            PC_WRITE_SCREEN_8( PC_SCREEN_LINE(2)+30, cyg_hal_smp_cpu_sync_flag[0] );
            PC_WRITE_SCREEN_8( PC_SCREEN_LINE(2)+35, cyg_hal_smp_cpu_sync_flag[1] );
            PC_WRITE_SCREEN_8( PC_SCREEN_LINE(2)+40, cyg_hal_smp_vsr_sync_flag );
            n++;
#endif
            hal_delay_us( 100 );            
        }

//        PC_WRITE_SCREEN( PC_SCREEN_LINE(2)+4, '!' );

        cyg_hal_smp_cpu_entry[cpu] = 0; 
 
//        PC_WRITE_SCREEN_32( PC_SCREEN_LINE(2)+20, entry );  
 
        if( entry != NULL )
        {
            cyg_hal_smp_cpu_running[cpu] = 1;
            entry();
        }
    }
#endif     
}

/*------------------------------------------------------------------------*/

__externC void cyg_hal_smp_init(void)
{
    if( !cyg_hal_find_smp_config() )
        return;

    if( !cyg_hal_parse_smp_config() )
        return;

    if( !cyg_hal_smp_init_apic() )
        return;

    if( !cyg_hal_smp_init_ioapic() )
        return;
}

/*------------------------------------------------------------------------*/

__externC void cyg_hal_smp_cpu_start_all(void)
{
    HAL_SMP_CPU_TYPE cpu;

    for( cpu = 0; cpu < HAL_SMP_CPU_COUNT(); cpu++ )
    {
        cyg_hal_smp_cpu_sync[cpu] = 0;
        cyg_hal_smp_cpu_sync_flag[cpu] = 0;
        cyg_hal_smp_cpu_running[cpu] = 0;
        cyg_hal_smp_cpu_entry[cpu] = 0;
        
        if( cpu != HAL_SMP_CPU_THIS() )
            cyg_hal_cpu_start( cpu );
        else cyg_hal_smp_cpu_running[cpu] = 1;
    }
}

/*------------------------------------------------------------------------*/
// SMP message buffers.
// SMP CPUs pass messages to eachother via a small circular buffer
// protected by a spinlock. Each message is a single 32 bit word with
// a type code in the top 4 bits and any argument in the remaining
// 28 bits.

#define SMP_MSGBUF_SIZE 4

static struct smp_msg_t
{
    HAL_SPINLOCK_TYPE           lock;           // protecting spinlock
    volatile CYG_WORD32         msgs[SMP_MSGBUF_SIZE]; // message buffer
    volatile CYG_WORD32         head;           // head of list
    volatile CYG_WORD32         tail;           // tail of list
    volatile CYG_WORD32         reschedule;     // reschedule request
    volatile CYG_WORD32         timeslice;      // timeslice request
} smp_msg[HAL_SMP_CPU_MAX];

/*------------------------------------------------------------------------*/
// Pass a message to another CPU.

#if SCREEN_DIAGNOSTICS
static int res_msgs[2], tms_msgs[2];
#endif

__externC void cyg_hal_cpu_message( HAL_SMP_CPU_TYPE cpu,
                                    CYG_WORD32 msg,
                                    CYG_WORD32 arg,
                                    CYG_WORD32 wait)
{
#if 1
    CYG_INTERRUPT_STATE istate;    
    struct smp_msg_t *m = &smp_msg[cpu];
    int i;
    HAL_SMP_CPU_TYPE me = HAL_SMP_CPU_THIS();
 
    HAL_DISABLE_INTERRUPTS( istate );
    
    // Get access to the message buffer for the selected CPU
    HAL_SPINLOCK_SPIN( m->lock );

#if 0 //SCREEN_DIAGNOSTICS    
    if( msg == HAL_SMP_MESSAGE_RESCHEDULE )
        res_msgs[me]++;
    else if( msg == HAL_SMP_MESSAGE_TIMESLICE )
        tms_msgs[me]++;
    PC_WRITE_SCREEN_8( PC_SCREEN_LINE(18+me), me);     
    PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+40, res_msgs[me]); 
    PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+45, tms_msgs[me]); 
#endif
 
    if( msg == HAL_SMP_MESSAGE_RESCHEDULE )
        m->reschedule = true;
    else if( msg == HAL_SMP_MESSAGE_TIMESLICE )
        m->timeslice = true;
    else
    {
        CYG_WORD32 next = (m->tail + 1) & (SMP_MSGBUF_SIZE-1);

        // If the buffer is full, wait for space to appear in it.
        // This should only need to be done very rarely.
    
        while( next == m->head )
        {
            HAL_SPINLOCK_CLEAR( m->lock );
            for( i = 0; i < 1000; i++ );
            HAL_SPINLOCK_SPIN( m->lock );        
        }

        m->msgs[m->tail] = msg | arg;

        m->tail = next;
    }
    
    // Now send an interrupt to the CPU.
    
//    PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+50, cyg_hal_smp_cpu_running[cpu] );
    
    if( cyg_hal_smp_cpu_running[cpu] )
    {
        CYG_WORD32 icrlo, icrhi;

        // Set the ICR fields we want to write. Most fields are zero
        // except the destination in the high word and the vector
        // number in the low.
        icrhi = cpu<<24;
        icrlo = CYGNUM_HAL_SMP_CPU_INTERRUPT_VECTOR( cpu );

        // Write the ICR register. The interrupt will be raised when
        // the low word is written.
        HAL_APIC_WRITE( HAL_APIC_ICR_HI, icrhi );
        HAL_APIC_WRITE( HAL_APIC_ICR_LO, icrlo );

        // Wait for the ICR to become inactive
        do {
#if 0 //SCREEN_DIAGNOSTICS            
            static int n;                
            PC_WRITE_SCREEN_8( PC_SCREEN_LINE(18+me)+55, n );
            n++;
#endif            
            HAL_APIC_READ( HAL_APIC_ICR_LO, icrlo );
        } while( (icrlo & 0x00001000) != 0 );        
    }

    HAL_SPINLOCK_CLEAR( m->lock );

    // If we are expected to wait for the command to complete, then
    // spin here until it does. We actually wait for the destination
    // CPU to empty its input buffer. So we might wait for messages
    // from other CPUs as well. But this is benign.
    
    while(wait)
    {
        for( i = 0; i < 1000; i++ );
        
        HAL_SPINLOCK_SPIN( m->lock );

        if( m->head == m->tail )
            wait = false;
        
        HAL_SPINLOCK_CLEAR( m->lock );

    } 

    HAL_RESTORE_INTERRUPTS( istate );
#endif    
}

/*------------------------------------------------------------------------*/

#if SCREEN_DIAGNOSTICS
static int isrs[2];
static int dsrs[2];
#endif

__externC CYG_WORD32 cyg_hal_cpu_message_isr( CYG_WORD32 vector, CYG_ADDRWORD data )
{
    HAL_SMP_CPU_TYPE me = HAL_SMP_CPU_THIS();
    struct smp_msg_t *m = &smp_msg[me];
    CYG_WORD32 ret = 1;
    CYG_INTERRUPT_STATE istate;
    
    HAL_DISABLE_INTERRUPTS( istate );

    HAL_SPINLOCK_SPIN( m->lock );

    // First, acknowledge the interrupt.
    
    HAL_INTERRUPT_ACKNOWLEDGE( vector );

#if SCREEN_DIAGNOSTICS
    isrs[me]++;    
    PC_WRITE_SCREEN_8( PC_SCREEN_LINE(18+me), me); 
    PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+5, isrs[me]); 
#endif
    
    if( m->reschedule || m->timeslice )
        ret |= 2;               // Ask for the DSR to be called.
    
    // Now pick messages out of the buffer and handle them
    
    while( m->head != m->tail )
    {
        CYG_WORD32 msg = m->msgs[m->head];

        switch( msg & HAL_SMP_MESSAGE_TYPE )
        {
        case HAL_SMP_MESSAGE_RESCHEDULE:
            ret |= 2;           // Ask for the DSR to be called.
            break;
        case HAL_SMP_MESSAGE_MASK:
            // Mask the supplied vector
//            cyg_hal_interrupt_set_mask( msg&HAL_SMP_MESSAGE_ARG, false );
            break;
        case HAL_SMP_MESSAGE_UNMASK:
            // Unmask the supplied vector
//            cyg_hal_interrupt_set_mask( msg&HAL_SMP_MESSAGE_ARG, true );
            break;
        case HAL_SMP_MESSAGE_REVECTOR:
            // Deal with a change of CPU assignment for a vector. We
            // only actually worry about what happens when the vector
            // is changed to some other CPU. We just mask the
            // interrupt locally.
//            if( hal_interrupt_cpu[msg&HAL_SMP_MESSAGE_ARG] != me )
//                cyg_hal_interrupt_set_mask( msg&HAL_SMP_MESSAGE_ARG, false );
            break;
        }

        // Update the head pointer after handling the message, so that
        // the wait in cyg_hal_cpu_message() completes after the action
        // requested.
        
        m->head = (m->head + 1) & (SMP_MSGBUF_SIZE-1);
    }

    HAL_SPINLOCK_CLEAR( m->lock );    

    HAL_RESTORE_INTERRUPTS( istate );

    return ret;
}

/*------------------------------------------------------------------------*/
// CPU message DSR.
// This is only executed if the message was
// HAL_SMP_MESSAGE_RESCHEDULE. It calls up into the kernel to effect a
// reschedule.

__externC void cyg_scheduler_set_need_reschedule(void);
__externC void cyg_scheduler_timeslice_cpu(void);

#if SCREEN_DIAGNOSTICS
__externC int cyg_scheduler_sched_lock;
static int rescheds[2];
static int timeslices[2];
#endif

__externC CYG_WORD32 cyg_hal_cpu_message_dsr( CYG_WORD32 vector, CYG_ADDRWORD data )
{
    HAL_SMP_CPU_TYPE me = HAL_SMP_CPU_THIS();
    struct smp_msg_t *m = &smp_msg[me];
    CYG_INTERRUPT_STATE istate;
    CYG_WORD32 reschedule, timeslice;
    
    HAL_DISABLE_INTERRUPTS( istate );
    HAL_SPINLOCK_SPIN( m->lock );

#if SCREEN_DIAGNOSTICS    
    dsrs[me]++;    
    PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+10, dsrs[me]);
    PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+15, cyg_scheduler_sched_lock);  
#endif
    
    reschedule = m->reschedule;
    timeslice = m->timeslice;
    m->reschedule = m->timeslice = false;

    HAL_SPINLOCK_CLEAR( m->lock );    
    HAL_RESTORE_INTERRUPTS( istate );
        
    if( reschedule )
    {
#if SCREEN_DIAGNOSTICS        
        rescheds[me]++;
        PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+20, rescheds[me]);
#endif        
        cyg_scheduler_set_need_reschedule();
    }
    if( timeslice )
    {
#if SCREEN_DIAGNOSTICS
        timeslices[me]++;
        PC_WRITE_SCREEN_16( PC_SCREEN_LINE(18+me)+25, timeslices[me]);
#endif        
        cyg_scheduler_timeslice_cpu();
    }

    return 0;
    
}

/*------------------------------------------------------------------------*/

#if SCREEN_DIAGNOSTICS
static int x = 0;
#endif

__externC void cyg_hal_smp_halt_other_cpus(void)
{
    int i;
    HAL_SMP_CPU_TYPE me = HAL_SMP_CPU_THIS();
    
//    PC_WRITE_SCREEN_8( PC_SCREEN_LINE(6+me), me );
  
    for( i = 0 ; i < HAL_SMP_CPU_COUNT(); i++ )
    {
        if( i != me && cyg_hal_smp_cpu_running[i] )
        {
            CYG_WORD32 icrhi, icrlo;
            CYG_WORD32 oldsync;
            
//            PC_WRITE_SCREEN_8( PC_SCREEN_LINE(6+me)+40, i );

            oldsync = cyg_hal_smp_cpu_sync_flag[i]; 
            cyg_hal_smp_cpu_sync[i] = 0;


            icrhi = i<<24;
            icrlo = CYGNUM_HAL_VECTOR_NMI;  // not really used
            icrlo |= 0x00000400;    // Delivery = NMI
            //icrlo |= 0x000C0000;    // Dest = all excluding self

            // Write the ICR register. The interrupt will be raised when
            // the low word is written.
            HAL_APIC_WRITE( HAL_APIC_ICR_HI, icrhi );
            HAL_APIC_WRITE( HAL_APIC_ICR_LO, icrlo );

            // Wait for the ICR to become inactive
            do {
#if 0 //SCREEN_DIAGNOSTICS
                static int n;                
                PC_WRITE_SCREEN_8( PC_SCREEN_LINE(6+me)+45, n );
                n++;
#endif                
                HAL_APIC_READ( HAL_APIC_ICR_LO, icrlo );
            } while( (icrlo & 0x00001000) != 0 );

            // Wait for CPU to halt
            while( cyg_hal_smp_cpu_sync_flag[i] == oldsync )
            {
#if 0 //SCREEN_DIAGNOSTICS                
                PC_WRITE_SCREEN_8( PC_SCREEN_LINE(6+me)+4, x ); x++;
                PC_WRITE_SCREEN_8( PC_SCREEN_LINE(6+me)+10+(i*8), cyg_hal_smp_cpu_sync_flag[i] );
                PC_WRITE_SCREEN_8( PC_SCREEN_LINE(6+me)+10+(i*8)+4, oldsync );
#endif           
                hal_delay_us( 100 );
            }
            
        }
    }
    
}

__externC void cyg_hal_smp_release_other_cpus(void)
{
    int i;
    for( i = 0 ; i < HAL_SMP_CPU_COUNT(); i++ )
    {
        if( i != HAL_SMP_CPU_THIS() && cyg_hal_smp_cpu_running[i] )
        {
            CYG_WORD32 oldsync = cyg_hal_smp_cpu_sync_flag[i];        
            cyg_hal_smp_cpu_sync[i] = 1;
            while( cyg_hal_smp_cpu_sync_flag[i] == oldsync )
                continue;
            cyg_hal_smp_cpu_sync[i] = 0;
        }
    }
}

#endif // CYGPKG_HAL_SMP_SUPPORT

/*------------------------------------------------------------------------*/
/* End of pcmb_smp.c                                                      */
