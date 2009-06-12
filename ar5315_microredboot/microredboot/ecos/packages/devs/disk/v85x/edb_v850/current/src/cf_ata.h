#ifndef CYGONCE_CF_ATA_H
#define CYGONCE_CF_ATA_H
//==========================================================================
//
//      cf_ata.h
//
//      CompactFlash ATA interface definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Savin Zlobec 
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
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    savin 
// Contributors: 
// Date:         2003-08-21
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

// -------------------------------------------------------------------------
// CF Error Register bits 
//
#define CF_EREG_BBK   (1<<7) // bad block detected
#define CF_EREG_UNC   (1<<6) // uncorrectable error
#define CF_EREG_IDNF  (1<<4) // req sector ID is in error or cannot be found
#define CF_EREG_ABORT (1<<2) // command aborted
#define CF_EREG_AMNF  (1<<0) // general error

// -------------------------------------------------------------------------
// CF Status Register bits 
//
#define CF_SREG_BUSY (1<<7) // busy (no other bits in this register are valid)
#define CF_SREG_RDY  (1<<6) // ready to accept a command
#define CF_SREG_DWF  (1<<5) // write fault has occurred
#define CF_SREG_DSC  (1<<4) // card is ready
#define CF_SREG_DRQ  (1<<3) // information req transfer to/from the host 
                            // through the Data register
#define CF_SREG_CORR (1<<2) // correctable data error (data corrected) 
#define CF_SREG_IDX  (1<<1) // always 0
#define CF_SREG_ERR  (1<<0) // command has ended in error (see error reg)

// -------------------------------------------------------------------------
// CF Device Control Register bits 
//
#define CF_DREG_SWRST (1<<2) // soft reset
#define CF_DREG_IEN   (1<<1) // interrupt enable (0 - enabled, 1 - disabled) 

// -------------------------------------------------------------------------
// CF ATA Command Set
//
// FR  - Features register
// SC  - Sector count register
// SN  - Sector number register
// CY  - Cylinder registers
// DH  - Card/Drive/Head register
// LBA - Logical Block Address Mode Supported
// 
// Y - The register contains a valid parameter for this command
//     For the Drive/Head Register Y means both the CF Card and head
//     parameters are used
// D - Only the CompactFlash Card parameter is valid and not the head parameter
//
//                                                 Note  FR  SC  SN  CY  DH  LBA
#define CF_ATA_CHECK_POWER_MODE_CMD        0xE5 //   -   -   -   -   -   D   -
#define CF_ATA_EXE_DRIVE_DIAG_CMD          0x90 //   -   -   -   -   -   D   -
#define CF_ATA_ERASE_SECTORS_CMD           0xC0 //   2   -   Y   Y   Y   Y   Y
#define CF_ATA_FORMAT_TRACK_CMD            0x50 //   -   -   Y   -   Y   Y   Y 
#define CF_ATA_IDENTIFY_DRIVE_CMD          0xEC //   -   -   -   -   -   D   -
#define CF_ATA_IDLE_CMD                    0xE3 //   -   -   Y   -   -   D   -
#define CF_ATA_IDLE_IMMEDIATE_CMD          0xE1 //   -   -   -   -   -   D   -
#define CF_ATA_INIT_DRIVE_PARAMS_CMD       0x91 //   -   -   Y   -   -   Y   -
#define CF_ATA_READ_BUFFER_CMD             0xE4 //   -   -   -   -   -   D   -
#define CF_ATA_READ_MULTIPLE_CMD           0xC4 //   -   -   Y   Y   Y   Y   Y
#define CF_ATA_READ_LONG_SECTOR_CMD        0x22 //   -   -   -   Y   Y   Y   Y
#define CF_ATA_READ_SECTORS_CMD            0x20 //   -   -   Y   Y   Y   Y   Y
#define CF_ATA_READ_VERIFY_SECTORS_CMD     0x40 //   -   -   Y   Y   Y   Y   Y
#define CF_ATA_RECALIBRATE_CMD             0x10 //   -   -   -   -   -   D   -
#define CF_ATA_REQUEST_SENSE_CMD           0x03 //   1   -   -   -   -   D   -
#define CF_ATA_SEEK_CMD                    0x70 //   -   -   -   Y   Y   Y   Y
#define CF_ATA_SET_FEATURES_CMD            0xEF //   -   Y   -   -   -   D   -
#define CF_ATA_SET_MULTIPLE_MODE_CMD       0xC6 //   -   -   Y   -   -   D   -
#define CF_ATA_SET_SLEEP_MODE_CMD          0xE6 //   -   -   -   -   -   D   -
#define CF_ATA_STAND_BY_CMD                0xE2 //   -   -   -   -   -   D   -
#define CF_ATA_STAND_BY_IMMEDIATE_CMD      0xE0 //   -   -   -   -   -   D   -  
#define CF_ATA_TRANSLATE_SECTOR_CMD        0x87 //   1   -   Y   Y   Y   Y   Y
#define CF_ATA_WEAR_LEVEL_CMD              0xF5 //   1   -   -   -   -   Y   -
#define CF_ATA_WRITE_BUFFER_CMD            0xE8 //   -   -   -   -   -   D   -
#define CF_ATA_WRITE_LONG_SECTOR_CMD       0x32 //   -   -   -   Y   Y   Y   Y
#define CF_ATA_WRITE_MULTIPLE_CMD          0xC5 //   -   -   Y   Y   Y   Y   Y
#define CF_ATA_WRITE_MULTIPLE_WO_ERASE_CMD 0xCD //   2   -   Y   Y   Y   Y   Y
#define CF_ATA_WRITE_SECTORS_CMD           0x30 //   -   -   Y   Y   Y   Y   Y
#define CF_ATA_WRITE_SECTORS_WO_ERASE_CMD  0x38 //   2   -   Y   Y   Y   Y   Y
#define CF_ATA_WRITE_VERIFY_SECTORS_CMD    0x3C //   -   -   Y   Y   Y   Y   Y
//
// Note 1 : These commands are not standard PC Card ATA commands but 
//          provide additional functionality
// Note 2 : These commands are not standard PC Card ATA commands and
//          these features are no longer supported with the
//          introduction of 256 Mbit flash technology. If one
//          of these commands is issued, the sectors will be 
//          erased but there will be no net gain in write
//          performance when using the Write Without Erase command.          

// -------------------------------------------------------------------------
// CF ATA Identify drive command response
//
typedef struct cf_ata_identify_data_t 
{        
    cyg_uint16 general_conf;         // 00    : general configuration   
    cyg_uint16 num_cylinders;        // 01    : number of cylinders (default CHS trans) 
    cyg_uint16 reserved0;            // 02    : reserved 
    cyg_uint16 num_heads;            // 03    : number of heads (default CHS trans) 
    cyg_uint16 num_ub_per_track;     // 04    : number of unformatted bytes per track 
    cyg_uint16 num_ub_per_sector;    // 05    : number of unformatted bytes per sector 
    cyg_uint16 num_sectors;          // 06    : number of sectors per track (default CHS trans) 
    cyg_uint16 num_card_sectors[2];  // 07-08 : number of sectors per card 
    cyg_uint16 reserved1;            // 09    : reserved 
    cyg_uint16 serial[10];           // 10-19 : serial number (string) 
    cyg_uint16 buffer_type;          // 20    : buffer type (dual ported) 
    cyg_uint16 buffer_size;          // 21    : buffer size in 512 increments 
    cyg_uint16 num_ECC_bytes;        // 22    : number of ECC bytes passed on R/W Long cmds 
    cyg_uint16 firmware_rev[4];      // 23-26 : firmware revision (string)
    cyg_uint16 model_num[20];        // 27-46 : model number (string)
    cyg_uint16 rw_mult_support;      // 47    : max number of sectors on R/W multiple cmds
    cyg_uint16 reserved2;            // 48    : reserved 
    cyg_uint16 capabilities;         // 49    : LBA, DMA, IORDY support indicator 
    cyg_uint16 reserved3;            // 50    : reserved 
    cyg_uint16 pio_xferc_timing;     // 51    : PIO data transfer cycle timing mode 
    cyg_uint16 dma_xferc_timing;     // 52    : single word DMA data transfer cycle timing mode 
    cyg_uint16 cur_field_validity;   // 53    : words 54-58 validity (0 == not valid) 
    cyg_uint16 cur_cylinders;        // 54    : number of current cylinders 
    cyg_uint16 cur_heads;            // 55    : number of current heads 
    cyg_uint16 cur_spt;              // 56    : number of current sectors per track 
    cyg_uint16 cur_capacity[2];      // 57-58 : current capacity in sectors 
    cyg_uint16 mult_sectors;         // 59    : multiple sector setting 
    cyg_uint16 lba_total_sectors[2]; // 60-61 : total sectors in LBA mode 
    cyg_uint16 sw_dma;               // 62    : single word DMA support 
    cyg_uint16 mw_dma;               // 63    : multi word DMA support 
    cyg_uint16 apio_modes;           // 64    : advanced PIO transfer mode supported 
    cyg_uint16 min_dma_timing;       // 65    : minimum multiword DMA transfer cycle 
    cyg_uint16 rec_dma_timing;       // 66    : recommended multiword DMA cycle 
    cyg_uint16 min_pio_timing;       // 67    : min PIO transfer time without flow control 
    cyg_uint16 min_pio_iordy_timing; // 68    : min PIO transfer time with IORDY flow control 
//  cyg_uint16 reserved4[187];       // 69-255: reserved 
} cf_ata_identify_data_t;

#endif // CYGONCE_CF_ATA_H

// -------------------------------------------------------------------------
// EOF cf_ata.h
