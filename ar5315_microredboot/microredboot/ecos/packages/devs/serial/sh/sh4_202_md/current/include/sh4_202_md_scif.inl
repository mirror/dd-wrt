#ifndef CYGONCE_DEVS_SH_SH4_202_MD_SCIF_H
#define CYGONCE_DEVS_SH_SH4_202_MD_SCIF_H

//==========================================================================
//
//      devs/serial/sh/sh4_202_md_scif.inl
//
//      Serial I/O specification for SuperH SH4-202 MicroDev development board
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Nick Garnett
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors:
// Date:         2003-09-18
// Purpose:      Specifies serial resources for the platform.
// Description:  This file can be include from either SCI or SCIF/IRDA driver
//               sources and should specify driver information as required
//               for the platform. This file was derived from the se77x9
//               driver.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/io_serial_sh_sh4_202_md.h>

#ifdef CYGPKG_IO_SERIAL_SH_SH4_202_MD_SERIAL1
static sh_scif_info sh4_202_md_serial_info1 = {
    er_int_num : CYGNUM_HAL_INTERRUPT_SCIF_ERI,
    rx_int_num : CYGNUM_HAL_INTERRUPT_SCIF_RXI,
    br_int_num : CYGNUM_HAL_INTERRUPT_SCIF_BRI,
    tx_int_num : CYGNUM_HAL_INTERRUPT_SCIF_TXI,
    ctrl_base  : CYGARC_REG_SCIF_SCSMR2,
#ifdef CYGINT_IO_SERIAL_SH_SCIF_DMA
# ifdef CYGSEM_IO_SERIAL_SH_SH4_202_MD_SERIAL1_DMA
    dma_enable : true,// we want DMA for this channel
    dma_xmt_cr_flags : CYGARC_REG_CHCR_RS_SCIF_TX
# else
    dma_enable : false // No DMA
# endif
#endif
};

#if CYGNUM_IO_SERIAL_SH_SH4_202_MD_SERIAL1_BUFSIZE > 0
static unsigned char sh4_202_md_serial_out_buf1[CYGNUM_IO_SERIAL_SH_SH4_202_MD_SERIAL1_BUFSIZE];
static unsigned char sh4_202_md_serial_in_buf1[CYGNUM_IO_SERIAL_SH_SH4_202_MD_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(sh4_202_md_serial_channel1,
                                       sh_scif_funs, 
                                       sh4_202_md_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SH_SH4_202_MD_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &sh4_202_md_serial_out_buf1[0], 
                                       sizeof(sh4_202_md_serial_out_buf1),
                                       &sh4_202_md_serial_in_buf1[0],  
                                       sizeof(sh4_202_md_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(sh4_202_md_serial_channel1,
                      sh_scif_funs, 
                      sh4_202_md_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_SH_SH4_202_MD_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(sh_serial_io1,
             CYGDAT_IO_SERIAL_SH_SH4_202_MD_SERIAL1_NAME,
             0,                 // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             sh_scif_init, 
             sh_scif_lookup,          // Serial driver may need initializing
             &sh4_202_md_serial_channel1
    );
#endif // CYGPKG_IO_SERIAL_SH_SH4_202_MD_SERIAL1

#endif // CYGONCE_DEVS_SH_SH4_202_MD_SCIF_H

// EOF sh4_202_md_scif.inl
