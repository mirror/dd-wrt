/*
* Copyright (C) 2010-2011 Freescale Semiconductor, Inc. All Rights Reserved.
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef BOOTCONTROLBLOCKS_H_
#define BOOTCONTROLBLOCKS_H_

#define NCB_FINGERPRINT1	0x504d5453	//!< 'STMP'
#define NCB_FINGERPRINT2	0x2042434e	//!< 'NCB<space>' - NAND Control Block
#define NCB_FINGERPRINT3	0x4e494252	//!< 'RBIN' - ROM Boot Image Block - N

#define FCB_FINGERPRINT		0x20424346	//!< 'FCB '
#define FCB_VERSION_1		0x01000000

#define LDLB_FINGERPRINT1	0x504d5453	//!< 'STMP'
#define LDLB_FINGERPRINT2	0x424c444c	//!< 'LDLB' - Logical Device Layout Block
#define LDLB_FINGERPRINT3	0x4c494252	//!< 'RBIL' - ROM Boot Image Block - L

#define DBBT_FINGERPRINT1	0x504d5453	//!< 'STMP'
#define DBBT_FINGERPRINT2_V2	0x44424254	//!< 'DBBT' - Discovered Bad Block Table.
#define DBBT_FINGERPRINT2	0x54424244	//!< 'DBBT' - Discovered Bad Block Table.
#define DBBT_FINGERPRINT3	0x44494252	//!< 'RBID' - ROM Boot Image Block - D
#define DBBT_VERSION_1		0x01000000

#define LDLB_VERSION_MAJOR   0x0001
#define LDLB_VERSION_MINOR   0x0000
#define LDLB_VERSION_SUB     0x0000

#define TYPICAL_NAND_READ_SIZE              2048

#define BCB_MAGIC_OFFSET	12

//==============================================================================

//! \brief NAND Timing structure for setting up the GPMI timing.
//!
//! This structure holds the timing for the NAND.  This data is used by
//! rom_nand_hal_GpmiSetNandTiming to setup the GPMI hardware registers.

typedef struct _NAND_Timing {
	uint8_t m_u8DataSetup;
	uint8_t m_u8DataHold;
	uint8_t m_u8AddressSetup;
	uint8_t m_u8DSAMPLE_TIME;
} NCB_NAND_Timing_t;

//==============================================================================

//! \brief Structure defining where NCB and LDLB parameters are located.
//!
//! This structure defines the basic fingerprint template for both the Nand
//! Control Block (NCB) and the Logical Drive Layout Block (LDLB).  This
//! template is used to determine if the sector read is a Boot Control Block.
//! This structure defines the NAND Control Block (NCB).  This block
//! contains information describing the timing for the NAND, the number of
//! NANDs in the system, the block size of the NAND, the page size of the NAND,
//! and other criteria for the NAND.  This is information that is
//! required just to successfully communicate with the NAND.
//! This structure also defines the Logical Drive Layout Block (LDLB).  This
//! block contains information describing the version as well as the layout of
//! the code and data on the NAND Media.  For the ROM, we're only concerned
//! with the boot firmware start.  Additional information may be stored in
//! the Reserved3 area.  This area will be of interest to the SDK.
//! This structure also defines the Discovered Bad Block Table (DBBT) header.
//! This block contains the information used for parsing the bad block tables
//! which are stored in subsequent 2K sectors.  The DBBT header is 8K, followed
//! by the first NANDs entries, then the 2nd NANDs entries on a subsequent 2K
//! page (determined by how many 2K pages the first nand requires), and so on.

typedef struct _NCB_BootBlockStruct_t {
	uint32_t m_u32FingerPrint1;	//!< First fingerprint in first byte.
	union {
		struct {
			NCB_NAND_Timing_t m_NANDTiming;	//!< Optimum timing parameters for Tas, Tds, Tdh in nsec.
			uint32_t m_u32DataPageSize;	//!< 2048 for 2K pages, 4096 for 4K pages.
			uint32_t m_u32TotalPageSize;	//!< 2112 for 2K pages, 4314 for 4K pages.
			uint32_t m_u32SectorsPerBlock;	//!< Number of 2K sections per block.
			uint32_t m_u32SectorInPageMask;	//!< Mask for handling pages > 2K.
			uint32_t m_u32SectorToPageShift;	//!< Address shift for handling pages > 2K.
			uint32_t m_u32NumberOfNANDs;	//!< Total Number of NANDs - not used by ROM.
		} NCB_Block1;
		struct {
			struct {
				uint16_t m_u16Major;
				uint16_t m_u16Minor;
				uint16_t m_u16Sub;
				uint16_t m_u16Reserved;
			} LDLB_Version;	//!< LDLB version - not used by ROM.
			uint32_t m_u32NANDBitmap;	//!< bit 0 == NAND 0, bit 1 == NAND 1, bit 2 = NAND 2, bit 3 = NAND3
		} LDLB_Block1;
		struct {
			uint32_t m_u32NumberBB_NAND0;	//!< # Bad Blocks stored in this table for NAND0.
			uint32_t m_u32NumberBB_NAND1;	//!< # Bad Blocks stored in this table for NAND1.
			uint32_t m_u32NumberBB_NAND2;	//!< # Bad Blocks stored in this table for NAND2.
			uint32_t m_u32NumberBB_NAND3;	//!< # Bad Blocks stored in this table for NAND3.
			uint32_t m_u32Number2KPagesBB_NAND0;	//!< Bad Blocks for NAND0 consume this # of 2K pages.
			uint32_t m_u32Number2KPagesBB_NAND1;	//!< Bad Blocks for NAND1 consume this # of 2K pages.
			uint32_t m_u32Number2KPagesBB_NAND2;	//!< Bad Blocks for NAND2 consume this # of 2K pages.
			uint32_t m_u32Number2KPagesBB_NAND3;	//!< Bad Blocks for NAND3 consume this # of 2K pages.
		} DBBT_Block1;
		// This one just forces the spacing.
		uint32_t m_Reserved1[10];
	};
	uint32_t m_u32FingerPrint2;	//!< 2nd fingerprint at byte 10.
	union {
		struct {
			uint32_t m_u32NumRowBytes;	//!< Number of row bytes in read/write transactions.
			uint32_t m_u32NumColumnBytes;	//!< Number of row bytes in read/write transactions.
			uint32_t m_u32TotalInternalDie;	//!< Number of separate chips in this NAND.
			uint32_t m_u32InternalPlanesPerDie;	//!< Number of internal planes - treat like separate chips.
			uint32_t m_u32CellType;	//!< MLC or SLC.
			uint32_t m_u32ECCType;	//!< 4 symbol or 8 symbol ECC?

/**********************************/
			uint32_t m_u32EccBlock0Size;	//!< Number of bytes for Block0 - BCH
			uint32_t m_u32EccBlockNSize;	//!< Block size in bytes for all blocks other than Block0 - BCH
			uint32_t m_u32EccBlock0EccLevel;	//!< Ecc level for Block 0 - BCH
			uint32_t m_u32NumEccBlocksPerPage;	//!< Number of blocks per page - BCH
			uint32_t m_u32MetadataBytes;	//!< Metadata size - BCH
			uint32_t m_u32EraseThreshold;	//!< To set into BCH_MODE register.
/**************** above is NCBv2 */

			uint32_t m_u32Read1stCode;	//!< First value sent to initiate a NAND Read sequence.
			uint32_t m_u32Read2ndCode;	//!< Second value sent to initiate a NAND Read sequence.
			uint32_t m_u32BootPatch;
			uint32_t m_u32PatchSectors;
			uint32_t m_u32Firmware_startingNAND2;
		} NCB_Block2;
		struct {
			uint32_t m_u32Firmware_startingNAND;	//!< Firmware image starts on this NAND.
			uint32_t m_u32Firmware_startingSector;	//!< Firmware image starts on this sector.
			uint32_t m_u32Firmware_sectorStride;	//!< Amount to jump between sectors - unused in ROM.
			uint32_t m_uSectorsInFirmware;	//!< Number of sectors in firmware image.
			uint32_t m_u32Firmware_startingNAND2;	//!< Secondary FW Image starting NAND.
			uint32_t m_u32Firmware_startingSector2;	//!< Secondary FW Image starting Sector.
			uint32_t m_u32Firmware_sectorStride2;	//!< Secondary FW Image stride - unused in ROM.
			uint32_t m_uSectorsInFirmware2;	//!< Number of sector in secondary FW image.
			struct {
				uint16_t m_u16Major;
				uint16_t m_u16Minor;
				uint16_t m_u16Sub;
				uint16_t m_u16Reserved;
			} FirmwareVersion;
			uint32_t m_u32DiscoveredBBTableSector;	//!< Location of Discovered Bad Block Table (DBBT).
			uint32_t m_u32DiscoveredBBTableSector2;	//!< Location of backup DBBT
		} LDLB_Block2;
		// This one just forces the spacing.
		uint32_t m_Reserved2[20];
	};
	uint32_t m_u32FingerPrint3;	//!< 3rd fingerprint at byte 30.
	union {
		struct {
			unsigned char ncb_unknown[12];
		};
	};
} NCB_BootBlockStruct_t;

typedef enum _nand_ecc_type {
    RS_Ecc_4bit = 0,
    RS_Ecc_8bit,
    BCH_Ecc_0bit,
    BCH_Ecc_2bit,
    BCH_Ecc_4bit,
    BCH_Ecc_6bit,
    BCH_Ecc_8bit,
    BCH_Ecc_10bit,
    BCH_Ecc_12bit,
    BCH_Ecc_14bit,
    BCH_Ecc_16bit,
    BCH_Ecc_18bit,
    BCH_Ecc_20bit
} nand_ecc_type_t;

typedef enum {
    ROM_BCH_Ecc_0bit = 0,
    ROM_BCH_Ecc_2bit,
    ROM_BCH_Ecc_4bit,
    ROM_BCH_Ecc_6bit,
    ROM_BCH_Ecc_8bit,
    ROM_BCH_Ecc_10bit,
    ROM_BCH_Ecc_12bit,
    ROM_BCH_Ecc_14bit,
    ROM_BCH_Ecc_16bit,
    ROM_BCH_Ecc_18bit,
    ROM_BCH_Ecc_20bit
} rom_ecc_type_t;

//==============================================================================

//! \brief Structure of the Bad Block Entry Table in NAND.
//!
//! This structure defines the Discovered Bad Block Table (DBBT) entries.  This
//! block contains a word holding the NAND number then a word describing the number
//! of Bad Blocks on the NAND and an array containing these bad blocks.  The ROM
//! will use these entries in the Bad Block table to correctly index to the next
//! sector (skip over bad blocks) while reading from the NAND.
//! Blocks are not guaranteed to be sorted in this table.
typedef struct _BadBlockTableNand_t {
	uint32_t uNAND;		//!< Which NAND this table is for.
	uint32_t uNumberBB;	//!< Number of Bad Blocks in this NAND.
	// Divide by 4 because of 32 bit words.  Subtract 2 because of the 2 above
	// 32 bit words.
	uint32_t u32BadBlock[(TYPICAL_NAND_READ_SIZE / 4) - 2];	//!< Table of the Bad Blocks.
} BadBlockTableNand_t;

//==============================================================================

//! \brief NAND Timing structure for setting up the GPMI timing.
//!
//! This structure holds the timing for the NAND.  This data is used by
//! rom_nand_hal_GpmiSetNandTiming to setup the GPMI hardware registers.
typedef struct {
	uint8_t m_u8DataSetup;
	uint8_t m_u8DataHold;
	uint8_t m_u8AddressSetup;
	uint8_t m_u8DSAMPLE_TIME;
	/* These are for application use only and not for ROM. */
	uint8_t m_u8NandTimingState;
	uint8_t m_u8REA;
	uint8_t m_u8RLOH;
	uint8_t m_u8RHOH;
} FCB_ROM_NAND_Timing_t;

typedef struct {
	uint32_t	m_u32TMTiming2_ReadLatency;
	uint32_t	m_u32TMTiming2_PreambleDelay;
	uint32_t	m_u32TMTiming2_CEDelay;
	uint32_t	m_u32TMTiming2_PostambleDelay;
	uint32_t	m_u32TMTiming2_CmdAddPause;
	uint32_t	m_u32TMTiming2_DataPause;
	uint32_t	m_u32TMSpeed;
	uint32_t	m_u32TMTiming1_BusyTimeout;
} FCB_ROM_NAND_TM_Timing_t;

struct fcb_block {
	FCB_ROM_NAND_Timing_t  m_NANDTiming;             //!< Optimum timing parameters for Tas, Tds, Tdh in nsec.
	uint32_t        m_u32PageDataSize;              //!< 2048 for 2K pages, 4096 for 4K pages.
	uint32_t        m_u32TotalPageSize;             //!< 2112 for 2K pages, 4314 for 4K pages.
	uint32_t        m_u32SectorsPerBlock;           //!< Number of 2K sections per block.
	uint32_t        m_u32NumberOfNANDs;             //!< Total Number of NANDs - not used by ROM.
	uint32_t        m_u32TotalInternalDie;          //!< Number of separate chips in this NAND.
	uint32_t        m_u32CellType;                  //!< MLC or SLC.
	uint32_t        m_u32EccBlockNEccType;          //!< Type of ECC, can be one of BCH-0-20
	uint32_t        m_u32EccBlock0Size;             //!< Number of bytes for Block0 - BCH
	uint32_t        m_u32EccBlockNSize;             //!< Block size in bytes for all blocks other than Block0 - BCH
	uint32_t        m_u32EccBlock0EccType;          //!< Ecc level for Block 0 - BCH
	uint32_t        m_u32MetadataBytes;             //!< Metadata size - BCH
	uint32_t        m_u32NumEccBlocksPerPage;       //!< Number of blocks per page for ROM use - BCH
	uint32_t        m_u32EccBlockNEccLevelSDK;      //!< Type of ECC, can be one of BCH-0-20
	uint32_t        m_u32EccBlock0SizeSDK;          //!< Number of bytes for Block0 - BCH
	uint32_t        m_u32EccBlockNSizeSDK;          //!< Block size in bytes for all blocks other than Block0 - BCH
	uint32_t        m_u32EccBlock0EccLevelSDK;      //!< Ecc level for Block 0 - BCH
	uint32_t        m_u32NumEccBlocksPerPageSDK;    //!< Number of blocks per page for SDK use - BCH
	uint32_t        m_u32MetadataBytesSDK;          //!< Metadata size - BCH
	uint32_t        m_u32EraseThreshold;            //!< To set into BCH_MODE register.
	uint32_t        m_u32BootPatch;                 //!< 0 for normal boot and 1 to load patch starting next to FCB.
	uint32_t        m_u32PatchSectors;              //!< Size of patch in sectors.
	uint32_t        m_u32Firmware1_startingPage;  //!< Firmware image starts on this sector.
	uint32_t        m_u32Firmware2_startingPage;  //!< Secondary FW Image starting Sector.
	uint32_t        m_u32PagesInFirmware1;        //!< Number of sectors in firmware image.
	uint32_t        m_u32PagesInFirmware2;        //!< Number of sector in secondary FW image.
	uint32_t        m_u32DBBTSearchAreaStartAddress;//!< Page address where dbbt search area begins
	uint32_t        m_u32BadBlockMarkerByte;        //!< Byte in page data that have manufacturer marked bad block marker, this will
							//!< bw swapped with metadata[0] to complete page data.
	uint32_t        m_u32BadBlockMarkerStartBit;    //!< For BCH ECC sizes other than 8 and 16 the bad block marker does not start
							//!< at 0th bit of m_u32BadBlockMarkerByte. This field is used to get to the
							//!< start bit of bad block marker byte with in m_u32BadBlockMarkerByte.
	uint32_t        m_u32BBMarkerPhysicalOffset;    //!< FCB value that gives byte offset for bad block marker on physical NAND page.
	uint32_t	m_u32BCHType;
	FCB_ROM_NAND_TM_Timing_t m_NANDTMTiming;
	uint32_t	m_u32DISBBM;	/* the flag to enable (1)/disable(0) bi swap */
	uint32_t	m_u32BBMarkerPhysicalOffsetInSpareData; /* The swap position of main area in spare area */
};

//==============================================================================

//! \brief Structure defining where FCB and DBBT parameters are located.
//!
//! This structure defines the basic fingerprint template for both the Firmware
//! Control Block (FCB) and the Discovered Bad Block Table (DBBT).  This
//! template is used to determine if the sector read is a Boot Control Block.
//! This structure defines the Firmware Control Block (FCB).  This block
//! contains information describing the timing for the NAND, the number of
//! NANDs in the system, the block size of the NAND, the page size of the NAND,
//! and other criteria for the NAND.  This is information that is
//! required just to successfully communicate with the NAND.
//! This block contains information describing the version as well as the layout of
//! the code and data on the NAND Media.  For the ROM, we're only concerned
//! with the boot firmware start.  Additional information may be stored in
//! the Reserved area.  This area will be of interest to the SDK.
//! This structure also defines the Discovered Bad Block Table (DBBT) header.
//! This block contains the information used for parsing the bad block tables
//! which are stored in subsequent 2K sectors.  The DBBT header is 8K, followed
//! by the first NANDs entries on a subsequent 2K page (determined by how many
//! 2K pages the first nand requires)
typedef struct {
	uint32_t    m_u32Checksum;         //!< First fingerprint in first byte.
	uint32_t    m_u32FingerPrint;      //!< 2nd fingerprint at byte 4.
	uint32_t    m_u32Version;          //!< 3rd fingerprint at byte 8.
	union {
		struct fcb_block FCB_Block;
		union {
			struct {
				uint32_t	m_u32NumberBB;		//!< # Bad Blocks stored in this table for NAND0.
				uint32_t	m_u32Number2KPagesBB;	//!< Bad Blocks for NAND0 consume this # of 2K pages.
			} v2;
			struct {
				uint32_t	m_u32res;
				uint32_t	m_u32DBBTNumOfPages;
			} v3;
		} DBBT_Block;
	};
} BCB_ROM_BootBlockStruct_t;

#endif	/*BOOTCONTROLBLOCKS_H_ */

