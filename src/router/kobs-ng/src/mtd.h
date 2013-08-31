/*
*  mtd.c - Dump control structures of the NAND
*
*  Copyright 2008-2011 Freescale Semiconductor, Inc.
*  Copyright (c) 2008 by Embedded Alley Solution Inc.
*
*   Author: Pantelis Antoniou <pantelis@embeddedalley.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*
*/

#ifndef MTD_H
#define MTD_H

#include <mtd/mtd-user.h>
#include <endian.h>

#include "BootControlBlocks.h"
#include "rom_nand_hamming_code_ecc.h"

//------------------------------------------------------------------------------
// Re-definitions of true and false, because the standard ones aren't good
// enough?
//------------------------------------------------------------------------------

#define false 	0
#define true 	!false

//------------------------------------------------------------------------------
// Flag bit definitions for the md->flags member.
//------------------------------------------------------------------------------

#define F_VERBOSE		0x01
#define F_MULTICHIP		0x02
#define F_AUTO_MULTICHIP	0x04

#define vp(x, ...)	\
	do {		\
		if (((x)->flags) & F_VERBOSE)	\
			printf(__VA_ARGS__);\
	} while (0)

//------------------------------------------------------------------------------
// This structure contains configuration information. This information comes
// primarily from a .kobs file, but may be overidden from the command line or
// edited as kobs works.
//------------------------------------------------------------------------------

struct mtd_config {
	int chip_count;
	const char *chip_0_device_path;
	int chip_0_offset;
	int chip_0_size;
	const char *chip_1_device_path;
	int chip_1_offset;
	int chip_1_size;
	int search_exponent;
	int data_setup_time;
	int data_hold_time;
	int address_setup_time;
	int data_sample_time;
	int row_address_size;
	int column_address_size;
	int read_command_code1;
	int read_command_code2;
	int boot_stream_major_version;
	int boot_stream_minor_version;
	int boot_stream_sub_version;
	int ncb_version;
	int boot_stream_1_address;
	int boot_stream_2_address;

	/* for rom boot */
	int stride_size_in_bytes;
	int search_area_size_in_bytes;
	int search_area_size_in_pages;
};

extern const struct mtd_config default_mtd_config;

//------------------------------------------------------------------------------
// This structure represents an MTD device in which we will write boot
// information.
//------------------------------------------------------------------------------

struct mtd_part {

	// The path to the device node that represents this MTD.
	char *name;

	// The file descriptor that represents this MTD.
	int fd;
	struct mtd_info_user info;

	// A bit set where each bit corresponds to a block in a given MTD.
	uint32_t *bad_blocks;

	// The number of bad blocks appearing in this MTD.
	int nrbad;

        int oobinfochanged;
	struct nand_oobinfo old_oobinfo;
	int ecc;
};

/**
 * struct nfc_geometry - NFC geometry description.
 *
 * This structure describes the NFC's view of the medium geometry.
 *
 * @ecc_algorithm:            The human-readable name of the ECC algorithm
 *                            (e.g., "Reed-Solomon" or "BCH").
 * @ecc_strength:             A number that describes the strength of the ECC
 *                            algorithm.
 * @page_size_in_bytes:       The size, in bytes, of a physical page, including
 *                            both data and OOB.
 * @metadata_size_in_bytes:   The size, in bytes, of the metadata.
 * @ecc_chunk_size_in_bytes:  The size, in bytes, of a single ECC chunk. Note
 *                            the first chunk in the page includes both data and
 *                            metadata, so it's a bit larger than this value.
 * @ecc_chunk_count:          The number of ECC chunks in the page,
 * @block_mark_byte_offset:   The byte offset in the ECC-based page view at
 *                            which the underlying physical block mark appears.
 * @block_mark_bit_offset:    The bit offset into the ECC-based page view at
 *                            which the underlying physical block mark appears.
 */

struct nfc_geometry {
	unsigned int  gf_len;
	unsigned int  ecc_strength;
	unsigned int  page_size_in_bytes;
	unsigned int  metadata_size_in_bytes;
	unsigned int  ecc_chunk_size_in_bytes;
	unsigned int  ecc_chunk_count;
	unsigned int  block_mark_byte_offset;
	unsigned int  block_mark_bit_offset;
};

//------------------------------------------------------------------------------
// This structure collects information that controls the entire kobs process.
// Various functions contribute information to this structure, depending on the
// command line options and the resulting process flow.
//------------------------------------------------------------------------------

struct mtd_data {
	struct mtd_part part[2];
	int flags;
	struct mtd_config cfg;

	/* writesize + oobsize buffer */
	void *buf;

	/* NFC Geometry */
	struct nfc_geometry  nfc_geometry;

	/* NCBs */
	NCB_BootBlockStruct_t *curr_ncb;
	NCB_BootBlockStruct_t ncb[2];
	loff_t ncb_ofs[2];
	int ncb_version;	/* 0, 1, or 3. Negative means error */

	/* LDLBs */
	NCB_BootBlockStruct_t *curr_ldlb;
	NCB_BootBlockStruct_t ldlb[2];
	loff_t ldlb_ofs[2];

	/* DBBTs */
	NCB_BootBlockStruct_t *curr_dbbt;
	NCB_BootBlockStruct_t dbbt[2];
	loff_t dbbt_ofs[2];
	/* the 2 NANDs */
	BadBlockTableNand_t *bbtn[2];

	/* In fact, we can reuse the boot block
	 * struct for mx53 on mx28, it's compatible
	 */

	/* i.MX28 FCB */
	BCB_ROM_BootBlockStruct_t  fcb;

	/* i.MX28 DBBT */
	BCB_ROM_BootBlockStruct_t  dbbt28;
	/* i.MX50 DBBT */
	BCB_ROM_BootBlockStruct_t  dbbt50;

	void *private;
};

#define PAGES_PER_STRIDE	64
#define ROM_MAX_BAD_BLOCKS	425	/* WTF? */

//------------------------------------------------------------------------------
// This variable tells us which version of the ROM we're working with.
//------------------------------------------------------------------------------

enum {
	ROM_Version_Unknown = -1,
	ROM_Version_0       =  0, /* e.g., i.MX23 */
	ROM_Version_1       =  1, /* e.g., i.MX28 */
	ROM_Version_2       =  2, /* e.g., i.MX53 */
	ROM_Version_3	    =  3, /* e.g., i.MX50 */
};

static inline int mtd_pages_per_block(struct mtd_data *md)
{
	return md->part[0].info.erasesize / md->part[0].info.writesize;
}

static inline unsigned int mtd_bytes2blocks(struct mtd_data *md, unsigned int bytes)
{
	return bytes / md->part[0].info.erasesize;
}

static inline unsigned int mtd_bytes2pages(struct mtd_data *md, unsigned int bytes)
{
	return bytes / md->part[0].info.writesize;
}

static inline unsigned int mtd_size(struct mtd_data *md)
{
	return md->part[0].info.size;
}

static inline unsigned int mtd_erasesize(struct mtd_data *md)
{
	return md->part[0].info.erasesize;
}

static inline unsigned int mtd_writesize(struct mtd_data *md)
{
	return md->part[0].info.writesize;
}

static inline unsigned int mtd_oobsize(struct mtd_data *md)
{
	return md->part[0].info.oobsize;
}

static inline int mtd_nrbad(struct mtd_data *md, int chip)
{
	return md->part[chip].nrbad;
}

int mtd_isbad(struct mtd_data *md, int chip, loff_t ofs);
int mtd_erase(struct mtd_data *md, int chip, loff_t ofs, size_t size);
int mtd_erase_block(struct mtd_data *md, int chip, loff_t ofs);
int mtd_read(struct mtd_data *md, int chip, void *data, size_t size, loff_t ofs);
int mtd_read_page(struct mtd_data *md, int chip, loff_t ofs, int ecc);
int mtd_write(struct mtd_data *md, int chip, const void *data, size_t size, loff_t ofs);
int mtd_write_page(struct mtd_data *md, int chip, loff_t ofs, int ecc);
void mtd_dump(struct mtd_data *md);
struct mtd_data *mtd_open(const struct mtd_config *cfg, int flags);
void mtd_close(struct mtd_data *md);
void *mtd_load_boot_structure(struct mtd_data *md, int chip, loff_t *ofsp, loff_t end,
		uint32_t magic1, uint32_t magic2, uint32_t magic3, int use_ecc,
		int magic_offset);
int mtd_load_all_boot_structures(struct mtd_data *md);
int mtd_dump_structure(struct mtd_data *md);

int v0_rom_mtd_init(struct mtd_data *md, FILE *fp);
int v1_rom_mtd_init(struct mtd_data *md, FILE *fp);
int v2_rom_mtd_init(struct mtd_data *md, FILE *fp);
int v3_rom_mtd_init(struct mtd_data *md, FILE *fp);
int v4_rom_mtd_init(struct mtd_data *md, FILE *fp);

int mtd_markbad(struct mtd_data *md, int chip, loff_t ofs);

#define UPDATE_NCB	0x01
#define UPDATE_LDLB	0x02
#define UPDATE_DBBT	0x04
#define UPDATE_BS0	0x08
#define UPDATE_BS1	0x10
#define UPDATE_BS(x)	(0x08 << ((x) & 1))
#define UPDATE_ALL	(UPDATE_NCB | UPDATE_LDLB | UPDATE_DBBT | UPDATE_BS0 | UPDATE_BS1)

int v0_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags);
int v1_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags);
int v2_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags);
int v3_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags);
int v4_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags);

int mtd_set_ecc_mode(struct mtd_data *md, int ecc);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

void mtd_parse_args(struct mtd_config *cfg, int argc, char **argv);
void mtd_parse_kobs(struct mtd_config *cfg, const char *name, int verbose);
void mtd_cfg_dump(struct mtd_config *cfg);

int ncb_get_version(void *ncb_candidate, NCB_BootBlockStruct_t **result);
int ncb_encrypt(NCB_BootBlockStruct_t *ncb, void *target, size_t size, int version);
int fcb_encrypt(BCB_ROM_BootBlockStruct_t *fcb, void *target, size_t size, int version);

#endif
