//==========================================================================
//
//      redboot_linux_boot.h
//
//      RedBoot interfaces with Linux kernel
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Red Hat, Inc.
// Copyright (C) 2002, 2003 Gary Thomas
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
//####OTHERCOPYRIGHTBEGIN####
//
//  The structure definitions below are taken from include/asm-/redboot.h in
//  the Linux kernel, Copyright (c) 2002, 2003 Gary Thomas, Copyright (c) 1997 Dan Malek. 
//  Their presence here is for the express purpose of communication with the Linux 
//  kernel being booted and is considered 'fair use' by the original author and
//  are included with their permission.
//
//####OTHERCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: 
// Date:         2003-08-28
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

//=========================================================================
// Imported from Linux kernel include/asm-ppc/redboot.h
//   Copyright (c) 2002, 2003 Gary Thomas (<gary@mlbassoc.com>
//   Copyright (c) 1997 Dan Malek (dmalek@jlc.net)
//   Used with permission of author(s).


/* A Board Information structure that is given to a program when
 * RedBoot starts it up.  Note: not all fields make sense for all
 * architectures and it's up to the platform specific code to fill
 * in the details.
 */
typedef struct bd_info {
    unsigned int   bi_tag;        /* Should be 0x42444944 "BDID" */
    unsigned int   bi_size;       /* Size of this structure */
    unsigned int   bi_revision;   /* revision of this structure */
    unsigned int   bi_bdate;      /* bootstrap date, i.e. 0x11061997 */
    unsigned int   bi_memstart;   /* Memory start address */
    unsigned int   bi_memsize;    /* Memory (end) size in bytes */
    unsigned int   bi_intfreq;    /* Internal Freq, in Hz */
    unsigned int   bi_busfreq;    /* Bus Freq, in Hz */
    unsigned int   bi_cpmfreq;    /* CPM Freq, in Hz */
    unsigned int   bi_brgfreq;    /* BRG Freq, in Hz */
    unsigned int   bi_vco;        /* VCO Out from PLL */
    unsigned int   bi_pci_freq;   /* PCI Freq, in Hz */
    unsigned int   bi_baudrate;   /* Default console baud rate */
    unsigned int   bi_immr;       /* IMMR when called from boot rom */
    unsigned char  bi_enetaddr[6];
    unsigned int   bi_flashbase;  /* Physical address of FLASH memory */
    unsigned int   bi_flashsize;  /* Length of FLASH memory */
    int            bi_flashwidth; /* Width (8,16,32,64) */
    unsigned char *bi_cmdline;    /* Pointer to command line */
} bd_t;

externC void plf_redboot_linux_exec(bd_t *info);

