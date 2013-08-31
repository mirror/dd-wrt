/*
 *  mtd.c - Dump control structures of the NAND
 *
 *  Copyright 2008-2010 Freescale Semiconductor, Inc.
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

#define _GNU_SOURCE
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "mtd.h"

#include "config.h"

int  rom_version = -1;

const struct mtd_config default_mtd_config = {
	.chip_count = 1,
	.chip_0_device_path = "/dev/mtd0",
	.chip_0_offset = 0,
	.chip_0_size = 0,
	.chip_1_device_path = NULL,
	.chip_1_offset = 0,
	.chip_1_size = 0,
	.search_exponent = 2,
	.data_setup_time = 80,
	.data_hold_time = 60,
	.address_setup_time = 25,
	.data_sample_time = 6,
	.row_address_size = 3,
	.column_address_size = 2,
	.read_command_code1 = 0x00,
	.read_command_code2 = 0x30,
	.boot_stream_major_version = 1,
	.boot_stream_minor_version = 0,
	.boot_stream_sub_version = 0,
	.ncb_version = 3,
	.boot_stream_1_address = 0,
	.boot_stream_2_address = 0,
};

static inline int multichip(struct mtd_data *md)
{
	return md->flags & F_MULTICHIP;
}

static int dev_attr_read_int(const char *filename)
{
	FILE *fp;
	char buf[BUFSIZ];
	int ret = -1;

	fp = fopen(filename, "ra");
	if (fp != NULL) {
		if (fgets(buf, sizeof(buf), fp) != NULL)
			ret = strtoul(buf, NULL, 0);
		fclose(fp);
	}

	return ret;
}

static int dev_attr_write_int(const char *filename, int val)
{
	FILE *fp;

	fp = fopen(filename, "wa");
	if (fp == NULL)
		return -1;	/* harmless */
	fprintf(fp, "%d", val);
	fclose(fp);

	return 0;
}

int mtd_isbad(struct mtd_data *md, int chip, loff_t ofs)
{
	int no;
	struct mtd_info_user *miu;

	/* outside the extends */
	if (ofs >= mtd_size(md))
		return -1;

	miu = &md->part[chip].info;

	if (ofs >= miu->size)
		return -1;

	/* bad block = write block, not erase block */
	no = ofs / miu->erasesize;
	return (md->part[chip].bad_blocks[no >> 5] >> (no & 31)) & 1;
}

/* force BAD */
int mtd_markbad(struct mtd_data *md, int chip, loff_t ofs)
{
	int no;
	struct mtd_info_user *miu;

	/* NOP if already bad */
	if (mtd_isbad(md, chip, ofs))
		return 0;

	/* outside the extends */
	if (ofs >= mtd_size(md))
		return -1;

	miu = &md->part[chip].info;

	if (ofs >= miu->size)
		return -1;

	/* bad block = write block, not erase block */
	no = ofs / miu->erasesize;

	md->part[chip].bad_blocks[no >> 5] |= 1 << (no & 31);
	md->part[chip].nrbad++;
	return 0;
}

int mtd_erase(struct mtd_data *md, int chip, loff_t ofs, size_t size)
{
	struct erase_info_user eiu;
	int r, chunk, nerase;

	nerase = 0;
	while (size > 0) {
		if (ofs + size > md->part[chip].info.size)
			chunk = md->part[chip].info.size - ofs;
		else
			chunk = size;

		eiu.start = ofs;
		eiu.length = chunk;

		if (md->flags & F_VERBOSE) {
			printf("mtd: erasing @%d:0x%llx-0x%llx\n",
					chip,
					(unsigned long long)eiu.start,
					(unsigned long long)eiu.length);
		}

		r = ioctl(md->part[chip].fd, MEMERASE, &eiu);
		if (r != 0) {
			fprintf(stderr, "mtd: device %d fails MEMERASE (0x%llx - 0x%x)\n",
					chip, (unsigned long long)ofs, chunk);
			return -1;
		}

		nerase += chunk;
		ofs += chunk;
		size -= chunk;
		if (ofs >= md->part[chip].info.size) {
			fprintf(stderr, "mtd: erase stepping bounds\n");
			return -1;
		}
	}

	return nerase;
}

/* block == erasesize */
int mtd_erase_block(struct mtd_data *md, int chip, loff_t ofs)
{
	int r;
	unsigned int search_area_sz, stride;
	const char *attrfile = "/sys/devices/platform/gpmi/ignorebad";
	int ignore = -1;

	/* for the NCB area turn ignorebad off */
	stride = PAGES_PER_STRIDE * mtd_writesize(md);
	search_area_sz = (1 << md->cfg.search_exponent) * stride;

	if (ofs < search_area_sz * 2 && (ignore = dev_attr_read_int(attrfile)) >= 0)
		dev_attr_write_int(attrfile, 1);

	r = mtd_erase(md, chip, ofs, mtd_erasesize(md));

	if (ofs < search_area_sz * 2 && ignore >= 0)
		dev_attr_write_int(attrfile, ignore);

	return r >= 0 ? 0 : -1;
}

int mtd_read(struct mtd_data *md, int chip, void *data, size_t size, loff_t ofs)
{
	int chunk, nread;
	int r;

	mtd_set_ecc_mode(md, 1);

	nread = 0;
	while (size > 0) {
		if (ofs + size > md->part[chip].info.size)
			chunk = md->part[chip].info.size - ofs;
		else
			chunk = size;

		do {
			r = pread(md->part[chip].fd, data, chunk, ofs);
		} while (r == -1 && (errno == EAGAIN || errno == EBUSY));

		if (r != chunk) {
			fprintf(stderr, "mtd: read failed\n");
			return -1;
		}

		nread += chunk;
		data += chunk;
		ofs += chunk;
		size -= chunk;
		if (ofs >= md->part[chip].info.size) {
			if (size == 0)	/* read to the end */
				break;
			fprintf(stderr, "mtd: read stepping bounds\n");
			return -1;
		}
	}

	return nread;
}

/* page == wrsize */
int mtd_read_page(struct mtd_data *md, int chip, loff_t ofs, int ecc)
{
	int size;
	int r;
	void *data;
	void *oobdata;

	mtd_set_ecc_mode(md, ecc);

	data = md->buf;
	oobdata = data + mtd_writesize(md);

	/* make sure it's aligned to a page */
	if ((ofs % mtd_writesize(md)) != 0)
		return -1;

	memset(md->buf, 0, mtd_writesize(md) + mtd_oobsize(md));

	size = mtd_writesize(md);

	do {
		r = pread(md->part[chip].fd, data, size, ofs);
	} while (r == -1 && (errno == EAGAIN || errno == EBUSY));

	/* end of partition? */
	if (r == 0)
		return 0;

	if (r != size) {
		fprintf(stderr, "mtd: %s failed\n", __func__);
		return -1;
	}

	if (ecc)
		return size;

	return size + mtd_oobsize(md);
}

int mtd_write(struct mtd_data *md, int chip, const void *data, size_t size, loff_t ofs)
{
	int chunk, nread;
	int r;

	mtd_set_ecc_mode(md, 1);

	nread = 0;
	while (size > 0) {
		if (ofs + size > md->part[chip].info.size)
			chunk = md->part[chip].info.size - ofs;
		else
			chunk = size;

		do {
			r = pwrite(md->part[chip].fd, data, chunk, ofs);
		} while (r == -1 && (errno == EAGAIN || errno == EBUSY));

		if (r != chunk) {
			fprintf(stderr, "mtd: %s failed\n", __func__);
			return -1;
		}

		nread += chunk;
		data += chunk;
		ofs += chunk;
		size -= chunk;
		if (ofs >= md->part[chip].info.size) {
			if (size == 0)	/* read to the end */
				break;
			fprintf(stderr, "mtd: %s stepping bounds\n", __func__);
			return -1;
		}
	}

	return nread;
}

/* page == wrsize */
int mtd_write_page(struct mtd_data *md, int chip, loff_t ofs, int ecc)
{
	int size;
	int r;
	const void *data;
	const void *oobdata;

	mtd_set_ecc_mode(md, ecc);

	data = md->buf;
	oobdata = data + mtd_oobsize(md);

	/* make sure it's aligned to a page */
	if ((ofs % mtd_writesize(md)) != 0) {
		fprintf(stderr, "mtd: %s failed\n", __func__);
		return -1;
	}

	size = mtd_writesize(md);

	do {
		r = pwrite(md->part[chip].fd, data, size, ofs);
	} while (r == -1 && (errno == EAGAIN || errno == EBUSY));

	/* end of partition? */
	if (r == 0) {
		fprintf(stderr, "mtd: %s written 0\n", __func__);
		return 0;
	}

	if (r != size) {
		fprintf(stderr, "mtd: %s failed\n", __func__);
		return -1;
	}

	if (ecc)
		return size;

	return size + mtd_oobsize(md);
}

void mtd_dump(struct mtd_data *md)
{
	struct mtd_part *mp;
	struct mtd_info_user *miu;
	int i, j, k;

	for (i = 0; i < 2; i++) {

		mp = &md->part[i];

		if (mp->fd == -1)
			continue;

		miu = &mp->info;

		fprintf(stderr, "mtd: partition #%d\n", i);
#undef P
#define P(x)	fprintf(stderr, "  %s = %d\n", #x, miu->x)
		P(type);
		P(flags);
		fprintf(stderr, "  %s = %lld\n", "size" , (unsigned long long)miu->size);
		P(erasesize);
		P(writesize);
		P(oobsize);
		fprintf(stderr, "  %s = %d\n", "blocks", mtd_bytes2blocks(md, miu->size));
#undef P

	}

	j = mtd_size(md) / mtd_erasesize(md);

	for (i = 0; i < 2; i++) {

		mp = &md->part[i];

		if (mp->fd == -1)
			continue;

		miu = &mp->info;

		if (mp->nrbad == 0)
			continue;

		fprintf(stderr, "  BAD:");
		for (k = 0; k < j; k++) {
			if (!mtd_isbad(md, i, k * mtd_erasesize(md)))
				continue;
			fprintf(stderr, " 0x%x", k * miu->erasesize);
		}
		fprintf(stderr, "\n");
	}

}

static struct nand_oobinfo none_oobinfo = { .useecc = MTD_NANDECC_OFF };

int parse_nfc_geometry(struct mtd_data *md)
{
	FILE               *node;
	static const char  *nfc_geometry_node_path = "/sys/bus/platform/devices/gpmi-nfc.0/nfc_geometry";
	static const int   buffer_size = 100;
	char               buffer[buffer_size];
	char               *p;
	char               *q;
	char               *name;
	char               *value_string;
	unsigned int       value;

	if (rom_version == ROM_Version_2)
		return 0;

	//----------------------------------------------------------------------
	// Attempt to open the NFC geometry node.
	//----------------------------------------------------------------------

	if (!(node = fopen(nfc_geometry_node_path, "r"))) {
		fprintf(stderr, "Cannot open NFC geometry node: \"%s\"", nfc_geometry_node_path);
		return !0;
	}

	//----------------------------------------------------------------------
	// Loop over lines from the node.
	//----------------------------------------------------------------------

	if (md->flags & F_VERBOSE) {
		printf("NFC Geometry\n");
	}


	while (fgets(buffer, buffer_size, node)) {

		if (md->flags & F_VERBOSE) {
			printf("  %s", buffer);
		}

		//--------------------------------------------------------------
		// Replace the newline with a null.
		//--------------------------------------------------------------

		buffer[strlen(buffer) - 1] = 0;

		//--------------------------------------------------------------
		// Find the colon (:)
		//--------------------------------------------------------------

		for (p = buffer; *p && (*p != ':'); p++);
		if (!p) goto failure;

		//--------------------------------------------------------------
		// Work backward from the colon to pick out the name.
		//--------------------------------------------------------------

		for (q = p - 1; *q == ' '; q--);
		q++;
		*q = 0;
		name = buffer;

		//--------------------------------------------------------------
		// Pick out the value.
		//--------------------------------------------------------------

		value_string = p + 2;

		//--------------------------------------------------------------
		// Now that we have clearly identified the name/value pair, we
		// can parse them. Begin by turning the value into a number.
		//--------------------------------------------------------------

		value = strtoul(value_string, 0, 0);

		//--------------------------------------------------------------
		// Figure out where to assign this value.
		//--------------------------------------------------------------

		if (!strcmp(name, "Page Size in Bytes"))
			md->nfc_geometry.page_size_in_bytes = value;
		else
		if (!strcmp(name, "Metadata Size in Bytes"))
			md->nfc_geometry.metadata_size_in_bytes = value;
		else
		if (!strcmp(name, "ECC Chunk Size in Bytes"))
			md->nfc_geometry.ecc_chunk_size_in_bytes = value;
		else
		if (!strcmp(name, "ECC Chunk Count"))
			md->nfc_geometry.ecc_chunk_count = value;
		else
		if (!strcmp(name, "Block Mark Byte Offset"))
			md->nfc_geometry.block_mark_byte_offset = value;
		else
		if (!strcmp(name, "Block Mark Bit Offset"))
			md->nfc_geometry.block_mark_bit_offset = value;

	}

	/* Return success. */

	return 0;

	/* Control arrives here when parsing has failed. */

failure:

	fprintf(stderr, "Could not parse the NFC geometry\n");

	return !0;

}

struct mtd_data *mtd_open(const struct mtd_config *cfg, int flags)
{
	struct mtd_data *md;
	struct mtd_part *mp;
	struct mtd_info_user *miu;
	const char *name;
	int i, k, j, r, no;
	loff_t ofs;
	unsigned int search_area_sz, stride;

	md = malloc(sizeof(*md));
	if (md == NULL)
		goto out;
	memset(md, 0, sizeof(*md));
	md->part[0].fd = md->part[1].fd = -1;
	md->flags = flags;

	md->ncb_version = -1;

	if (cfg == NULL)
		cfg = &default_mtd_config;
	md->cfg = *cfg;

	if (rom_version == ROM_Version_0) {

		// The i.MX23 always expects a boot area on chip 0, but will also expect
		// a boot area on chip 1, if it exists.

		if (dev_attr_read_int("/sys/bus/platform/devices/gpmi/numchips") == 2) {
			md->flags |= F_AUTO_MULTICHIP;
			if (md->flags & F_VERBOSE)
				printf("mtd: detected multichip NAND\n");
			if (!md->cfg.chip_1_device_path) {
				if (md->flags & F_VERBOSE)
					printf("mtd: WARNING - device node for chip 1 is not specified, using default one\n");
				md->cfg.chip_1_device_path = "/dev/mtd1";	/* late default */
			}
		}

	}
	else if (rom_version == ROM_Version_1) {

		// The i.MX28 expects a boot area only on chip 0.

		md->cfg.chip_1_device_path = 0;

	}

	for (i = 0; i < 2; i++) {

		name = i == 0 ?
			md->cfg.chip_0_device_path :
			md->cfg.chip_1_device_path;

		if (name == NULL)
			break;	/* only one */

		if (md->flags & F_VERBOSE)
			printf("mtd: opening: \"%s\"\n", name);

		mp = &md->part[i];

		mp->name = strdup(name);
		if (mp->name == NULL) {
			fprintf(stderr, "mtd: device %s can't allocate name\n", name);
			goto out;
		}

		mp->fd = open(name, O_RDWR);
		if (mp->fd == -1) {
			fprintf(stderr, "mtd: device \"%s\" can't be opened\n", mp->name);
			goto out;
		}

		miu = &mp->info;

		r = ioctl(mp->fd, MTDFILEMODE, (void *)MTD_FILE_MODE_NORMAL);
		if (r != 0 && r != -ENOTTY) {
			fprintf(stderr, "mtd: device %s can't switch to normal\n", mp->name);
			goto out;
		}

		/* keep original oobinfo */
		r = ioctl(mp->fd, MEMGETOOBSEL, &mp->old_oobinfo);
		if (r != 0) {
			fprintf(stderr, "mtd: device %s can't ioctl MEMGETOOBSEL\n", mp->name);
			goto out;
		}

		/* get info about the mtd device (partition) */
		r = ioctl(mp->fd, MEMGETINFO, miu);
		if (r != 0) {
			fprintf(stderr, "mtd: device %s fails MEMGETINFO\n", mp->name);
			goto out;
		}

		/* verify it's a nand */
		if (miu->type != MTD_NANDFLASH) {
			fprintf(stderr, "mtd: device %s not NAND\n", mp->name);
			goto out;
		}

		/* verify it's a supported geometry */
		if (miu->writesize + miu->oobsize != 2048 + 64 &&
			miu->writesize + miu->oobsize != 4096 + 128 &&
			miu->writesize + miu->oobsize != 4096 + 224 &&
			miu->writesize + miu->oobsize != 4096 + 218 &&
			miu->writesize + miu->oobsize != 8192 + 376 &&
			miu->writesize + miu->oobsize != 8192 + 512) {
			fprintf(stderr, "mtd: device %s; unsupported geometry (%d/%d)\n",
					mp->name, miu->writesize, miu->oobsize);
			goto out;
		}

		/* size in blocks */
		j = miu->size / miu->erasesize;

		/* number of 32 bit words */
		k = ((j + 31) / 32) * sizeof(uint32_t);
		mp->bad_blocks = malloc(k);
		if (mp->bad_blocks == NULL) {
			fprintf(stderr, "mtd: device %s; unable to allocate bad block table\n",
					mp->name);
			goto out;
		}
		memset(mp->bad_blocks, 0, k);

		stride = PAGES_PER_STRIDE * mtd_writesize(md);
		search_area_sz = (1 << md->cfg.search_exponent) * stride;

		/* probe for bad blocks */
		for (ofs = 0; ofs < miu->size; ofs += miu->erasesize) {

			/* skip the two NCB areas (where ECC does not apply) */
			if (ofs < search_area_sz * 2)
				continue;

			no = ofs / miu->erasesize;

			/* check if it's bad */
			r = ioctl(mp->fd, MEMGETBADBLOCK, &ofs);
			if (r < 0) {
				fprintf(stderr, "mtd: device %s; error checking bad block @0x%llu\n",
						mp->name, ofs);
				goto out;

			}

			/* not bad */
			if (r == 0)
				continue;

			/* calculate */
			mp->bad_blocks[no >> 5] |= 1 << (no & 31);
			mp->nrbad++;

			/* bad block */
			if (md->flags & F_VERBOSE)
				printf("mtd: '%s' bad block @ 0x%llx (MTD)\n", mp->name, ofs);
		}

		mp->ecc = 1;
	}

	if (md->part[1].fd >= 0 && md->part[2].fd >=0)
		md->flags |= F_MULTICHIP;

	/* if a second partition has been opened, verify that are compatible */
	if (md->part[1].fd != -1 &&
		(md->part[0].info.erasesize != md->part[1].info.erasesize ||
		 md->part[0].info.writesize != md->part[1].info.writesize ||
		 md->part[0].info.oobsize != md->part[1].info.oobsize ||
		 md->part[0].info.size != md->part[1].info.size)) {
			fprintf(stderr, "mtd: device %s / %s; incompatible\n",
					md->part[0].name, md->part[1].name);
	}

	md->buf = malloc(mtd_writesize(md) + mtd_oobsize(md));
	if (md->buf == NULL) {
		fprintf(stderr, "mtd: unable to allocate page buffer\n");
		goto out;
	}

	/* reset the boot structure info */
	md->curr_ncb = NULL;
	md->ncb_ofs[0] = md->ncb_ofs[1] = -1;

	md->curr_ldlb = NULL;
	md->ldlb_ofs[0] = md->ldlb_ofs[1] = -1;

	md->curr_dbbt = NULL;
	md->dbbt_ofs[0] = md->dbbt_ofs[1] = -1;

	/* Parse the NFC geometry. */

	if (parse_nfc_geometry(md)) {
		fprintf(stderr, "mtd: unable to parse NFC geometry\n");
		goto out;
	}

	/* Announce success. */

	if (md->flags & F_VERBOSE)
		printf("mtd: opened '%s' - '%s'\n",
				md->part[0].name,
				md->part[1].name);

	return md;
out:
	mtd_close(md);
	return NULL;
}

void mtd_close(struct mtd_data *md)
{
	struct mtd_part *mp;
	int i;

	if (md == NULL)
		return;

	for (i = 0; i < 2; i++) {

		mp = &md->part[i];

		if (mp->fd != -1) {
//			(void)ioctl(mp->fd, MEMSETOOBSEL,
//					&mp->old_oobinfo);
			close(mp->fd);
		}

		if (mp->bad_blocks)
			free(mp->bad_blocks);

		if (mp->name)
			free(mp->name);
	}
	if (md->buf)
		free(md->buf);

	if (md->bbtn[0])
		free(md->bbtn[0]);
	if (md->bbtn[1])
		free(md->bbtn[1]);

	free(md);
}

int mtd_set_ecc_mode(struct mtd_data *md, int ecc)
{
	struct mtd_part *mp;
	int i, r;

	/* correct ecc mode */
	for (i = 0; i < 2; i++) {

		mp = &md->part[i];
		if (mp->fd == -1)
			continue;

		if (mp->ecc == ecc)
			continue;

		if (ecc == 1) {
			r = ioctl(mp->fd, MTDFILEMODE, (void *)MTD_FILE_MODE_NORMAL);
			if (r != 0 && r != -ENOTTY) {
				fprintf(stderr, "mtd: device %s can't switch to normal\n", mp->name);
				continue;
			}

			if (r == -ENOTTY) {
//				r = ioctl(mp->fd, MEMSETOOBSEL, &mp->old_oobinfo);
//				if (r != 0) {
//					fprintf(stderr, "mtd: device %s can't ioctl MEMSETOOBSEL\n", mp->name);
					continue;
//				}
//				mp->oobinfochanged = 0;
			}
		} else {
			r = ioctl(mp->fd, MTDFILEMODE, (void *)MTD_FILE_MODE_RAW);
			if (r != 0 && r != -ENOTTY) {
				fprintf(stderr, "mtd: device %s can't switch to RAW\n", mp->name);
				continue;
			}

			if (r == -ENOTTY) {
//				r = ioctl(mp->fd, MEMSETOOBSEL, &none_oobinfo);
//				if (r != 0) {
//					fprintf(stderr, "mtd: device %s can't ioctl MEMSETOOBSEL\n", mp->name);
					continue;
//				}
//				mp->oobinfochanged = 1;
			} else
				mp->oobinfochanged = 2;
		}

		mp->ecc = ecc;
	}

	return 0;
}

/******************************************************/

/* static */ void dump(const void *data, int size)
{
	int i, j;
	const uint8_t *s;

	s = data;
	for (i = j = 0; i < size; i += 16) {
		if (i)
			printf("\n");
		printf("[%04x]", i);

		for (j = i; j < i + 16; j ++) {
			if (j < size)
				printf(" %02x", s[j]);
			else
				printf("   ");
			if (j == i + 8)
				printf(" ");
		}


		printf(" | ");

		for (j = i; j < i + 16; j ++) {
			if (j < size)
				printf("%c", isprint(s[j]) ? s[j] : '.');
			else
				printf(" ");
			if (j == i + 8)
				printf("-");
		}
	}
	printf("\n");
}

void *mtd_load_boot_structure(struct mtd_data *md, int chip, loff_t *ofsp, loff_t end,
		uint32_t magic1, uint32_t magic2, uint32_t magic3, int use_ecc,
		int magic_offset)
{
	loff_t ofs;
	int r, stride, size;
	BootBlockStruct_t *bbs;

	stride = PAGES_PER_STRIDE * mtd_writesize(md);

	for (ofs = *ofsp; ofs < end; ofs += stride) {

		/* check if it's bad only when under ECC control */
		if (use_ecc && mtd_isbad(md, chip, ofs)) {
			fprintf(stderr, "mtd: skipping bad block @0x%llx\n", ofs);
			continue;
		}

		/* calculate size of page to read (if no_ecc, we read oob) */
		size = mtd_writesize(md);
		if (!use_ecc)
		      size += mtd_oobsize(md);

		/* read page */
		r = mtd_read_page(md, chip, ofs, use_ecc);
		if (r != size) {
			fprintf(stderr, "mtd: read failed @0x%llx (%d)\n", ofs, r);
			continue;
		}
		bbs = md->buf;

		/* fast test */
		if (bbs->m_u32FingerPrint1 == magic1 ||
		    bbs->m_u32FingerPrint2 == magic2 ||
		    bbs->m_u32FingerPrint3 == magic3)
			break;

		if (magic_offset > 0) {
			bbs = md->buf + magic_offset;
			if (bbs->m_u32FingerPrint1 == magic1 ||
			    bbs->m_u32FingerPrint2 == magic2 ||
			    bbs->m_u32FingerPrint3 == magic3)
				break;
		}

		fprintf(stderr, "mtd: fingerprints mismatch @%d:0x%llx\n", chip, ofs);
		// dump(bbs, sizeof(*bbs));
	}

	if (ofs >= end)
		return NULL;

	// dump(bbs, sizeof(*bbs));

	*ofsp = ofs;
	return md->buf;
}

/* single chip version */
int mtd_load_all_boot_structures(struct mtd_data *md)
{
	loff_t ofs, end;
	int search_area_sz, stride;
	int i, j, r, no, sz;
	void *buf;
	BadBlockTableNand_t *bbtn;
	struct mtd_part *mp;
	int chip;

	stride = PAGES_PER_STRIDE * mtd_writesize(md);
	search_area_sz = (1 << md->cfg.search_exponent) * stride;

	/* make sure it fits */
	if (search_area_sz * 6 > mtd_size(md)) {
		fprintf(stderr, "mtd: search areas too large\n");
		return -1;
	}

	/* NCBs are NCB1, NCB2 */
	for (i = 0; i < 2; i++) {

		if (multichip(md)) {
			ofs = 0;
			chip = i;
		} else {
			ofs = i * search_area_sz;
			chip = 0;
		}
		end = ofs + search_area_sz;
		md->curr_ncb = NULL;
		md->ncb_ofs[i] = -1;

		while (ofs < end) {
			buf = mtd_load_boot_structure(md, chip, &ofs, end,
					NCB_FINGERPRINT1,
					NCB_FINGERPRINT2,
					NCB_FINGERPRINT3,
					0, 12);
			if (buf == NULL) {
				ofs = end;
				break;
			}


			/* found, but we have to verify now */
			md->curr_ncb = NULL;
			md->ncb_version = ncb_get_version(buf, &md->curr_ncb);

			if (md->flags & F_VERBOSE)
				printf("mtd: found NCB%d candidate version %d @%d:0x%llx\n",
					i, md->ncb_version, chip, ofs);

			if (md->ncb_version >= 0)
				break;

			fprintf(stderr, "mtd: NCB fails check @%d:0x%llx\n", chip, ofs);
			// dump(md->buf, mtd_writesize(md));

			ofs += PAGES_PER_STRIDE * mtd_writesize(md);
		}
		if (md->curr_ncb == NULL) {
			fprintf(stderr, "mtd: NCB%d not found\n", i);
			continue;
		}

		if (md->flags & F_VERBOSE)
			printf("mtd: Valid NCB%d version %d found @%d:0x%llx\n",
					i, md->ncb_version, chip, ofs);

		md->ncb[i] = *md->curr_ncb;
		md->curr_ncb = NULL;
		md->ncb_ofs[i] = ofs;
	}

	if (md->ncb_ofs[0] == -1 && md->ncb_ofs[1] == -1) {
		fprintf(stderr, "mtd: neither NCB1 or NCB2 found ERROR\n");
		return -1;
	}

	if (md->ncb_ofs[0] != -1 && md->ncb_ofs[1] != -1) {
		if (memcmp(&md->ncb[0], &md->ncb[1], sizeof(md->ncb[0])) != 0)
			printf("mtd: warning NCB1 != NCB2, using NCB1\n");
	}

	/* prefer 0 */
	if (md->ncb_ofs[0] != -1)
		md->curr_ncb = &md->ncb[0];
	else
		md->curr_ncb = &md->ncb[1];

	/* LDLBs are right after the NCBs */
	for (i = 0; i < 2; i++) {

		if (multichip(md)) {
			ofs = 1 * search_area_sz;
			chip = i;
		} else {
			ofs = (2 + i) * search_area_sz;
			chip = 0;
		}

		end = ofs + search_area_sz;
		md->curr_ldlb = NULL;
		md->ldlb_ofs[i] = -1;

		buf = mtd_load_boot_structure(md, chip, &ofs, end,
				LDLB_FINGERPRINT1,
				LDLB_FINGERPRINT2,
				LDLB_FINGERPRINT3,
				1, 0);
		if (buf == NULL) {
			fprintf(stderr, "mtd: LDLB%d not found\n", i);
			continue;
		}
		md->curr_ldlb = buf;

		if (md->flags & F_VERBOSE)
			printf("mtd: Valid LDLB%d found @%d:0x%llx\n", i, chip, ofs);

		md->ldlb[i] = *md->curr_ldlb;
		md->curr_ldlb = NULL;
		md->ldlb_ofs[i] = ofs;
	}

	if (md->ldlb_ofs[0] != -1 && md->ldlb_ofs[1] != -1) {
		if (memcmp(&md->ldlb[0], &md->ldlb[1], sizeof(md->ldlb[0])) != 0)
			printf("mtd: warning LDLB1 != LDLB2, using LDLB2\n");
	}

	if (md->ldlb_ofs[0] == -1 && md->ldlb_ofs[1] == -1) {
		fprintf(stderr, "mtd: neither LDLB1 or LDLB2 found ERROR\n");
		return -1;
	}

	/* prefer 0 */
	if (md->ldlb_ofs[0] != -1)
		md->curr_ldlb = &md->ldlb[0];
	else
		md->curr_ldlb = &md->ldlb[1];

	/* DBBTs are right after the LDLBs */
	for (i = 0; i < 2; i++) {

		if (multichip(md)) {
			ofs = 2 * search_area_sz;
			chip = i;
		} else {
			ofs = (4 + i) * search_area_sz;
			chip = 0;
		}

		end = ofs + search_area_sz;
		md->curr_dbbt = NULL;
		md->dbbt_ofs[i] = -1;

		buf = mtd_load_boot_structure(md, chip, &ofs, end,
				DBBT_FINGERPRINT1,
				DBBT_FINGERPRINT2,
				DBBT_FINGERPRINT3,
				1, 0);
		if (buf == NULL) {
			fprintf(stderr, "mtd: DBBT%d not found\n", i);
			continue;
		}
		md->curr_dbbt = buf;

		if (md->flags & F_VERBOSE)
			printf("mtd: Valid DBBT%d found @%d:0x%llx\n", i, chip, ofs);

		md->dbbt[i] = *md->curr_dbbt;
		md->curr_dbbt = NULL;
		md->dbbt_ofs[i] = ofs;
	}

	if (md->dbbt_ofs[0] != -1 && md->dbbt_ofs[1] != -1) {
		if (memcmp(&md->dbbt[0], &md->dbbt[1], sizeof(md->dbbt[0])) != 0)
			printf("mtd: warning DBBT1 != DBBT2, using DBBT2\n");
	}

	if (md->dbbt_ofs[0] == -1 && md->dbbt_ofs[1] == -1) {
		fprintf(stderr, "mtd: neither DBBT1 or DBBT2 found ERROR\n");
		return -1;
	}

	/* prefer 0 */
	if (md->dbbt_ofs[0] != -1)
		md->curr_dbbt = &md->dbbt[0];
	else
		md->curr_dbbt = &md->dbbt[1];

	/* no bad blocks what-so-ever */
	if (md->curr_dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND0 == 0 &&
	    md->curr_dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND1 == 0)
		return 0;

	/* find DBBT to read from */
	if (md->curr_dbbt == &md->dbbt[0]) {
		ofs = md->dbbt_ofs[0];
		chip = 0;
	}
	else {
		ofs = md->dbbt_ofs[1];
		if (multichip(md))
			chip = 1;
	}

	/* BBTNs start here */
	ofs += 4 * 2048;
	for (j = 0; j < 2; j++, ofs += sz) {
		if (j == 0)
			sz = md->curr_dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND0;
		else
			sz = md->curr_dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND1;
		if (sz == 0)
			continue;
		sz *= 2048;
		md->bbtn[j] = malloc(sz);
		if (md->bbtn[j] == NULL) {
			printf("mtd: UNABLE to allocate %d bytes for BBTN%d\n", sz, j);
			continue;
		}
		r = mtd_read(md, chip, md->bbtn[j], sz, ofs);
		if (r != sz) {
			printf("mtd: UNABLE to read %d bytes for BBTN%d\n", sz, j);
			continue;
		}

	}

	/* update bad block table */
	for (j = 0; j < 2; j++) {

		bbtn = md->bbtn[j];
		if (bbtn == NULL)
			continue;

		mp = &md->part[j];

		if (bbtn->uNAND != j || bbtn->uNumberBB * mtd_erasesize(md) > mp->info.size) {
			printf("mtd: illegal BBTN#%d\n", j);
			continue;
		}

		for (i = 0; i < bbtn->uNumberBB; i++) {
			no = bbtn->u32BadBlock[i];
			/* already marked bad? */
			if ((mp->bad_blocks[no >> 5] & (1 << (no & 31))) != 0)
				continue;

			/* mark it as bad */
			mp->bad_blocks[no >> 5] |= (1 << (no & 31));
			mp->nrbad++;

			/* bad block */
			if (md->flags & F_VERBOSE)
				printf("mtd: '%s' bad block @ 0x%llx (DBBT)\n", mp->name, (loff_t)no * mtd_erasesize(md));
		}
	}
	return 0;
}

/* single chip version */
int mtd_dump_structure(struct mtd_data *md)
{
	int i, j, k;
	BadBlockTableNand_t *bbtn;

	if(rom_version == ROM_Version_0){
		// dump(md->curr_ncb, sizeof(*md->curr_ncb));
		printf("NCB\n");
#undef P0
#define P0(x)	printf("  %s = %d\n", #x, md->curr_ncb->NCB_Block1.x)
		P0(m_NANDTiming.m_u8DataSetup);
		P0(m_NANDTiming.m_u8DataHold);
		P0(m_NANDTiming.m_u8AddressSetup);
		P0(m_NANDTiming.m_u8DSAMPLE_TIME);
		P0(m_u32DataPageSize);
		P0(m_u32TotalPageSize);
		P0(m_u32SectorsPerBlock);
		P0(m_u32SectorInPageMask);
		P0(m_u32SectorToPageShift);
		P0(m_u32NumberOfNANDs);
#undef P0
#define P0(x)	printf("  %s = %d\n", #x, md->curr_ncb->NCB_Block2.x)
		P0(m_u32NumRowBytes);
		P0(m_u32NumColumnBytes);
		P0(m_u32TotalInternalDie);
		P0(m_u32InternalPlanesPerDie);
		P0(m_u32CellType);
		P0(m_u32ECCType);
		P0(m_u32EccBlock0Size);
		P0(m_u32EccBlockNSize);
		P0(m_u32EccBlock0EccLevel);
		P0(m_u32NumEccBlocksPerPage);
		P0(m_u32MetadataBytes);
		P0(m_u32EraseThreshold);
		P0(m_u32Read1stCode);
		P0(m_u32Read2ndCode);
		P0(m_u32BootPatch);
		P0(m_u32PatchSectors);
		P0(m_u32Firmware_startingNAND2);
#undef P0

		printf("LDLB\n");
		// dump(md->curr_ldlb, sizeof(*md->curr_ldlb));
#undef P0
#define P0(x)	printf("  %s = %d\n", #x, md->curr_ldlb->LDLB_Block1.x)
		P0(LDLB_Version.m_u16Major);
		P0(LDLB_Version.m_u16Minor);
		P0(LDLB_Version.m_u16Sub);
		P0(m_u32NANDBitmap);
#undef P0
#define P0(x)	printf("  %s = %d\n", #x, md->curr_ldlb->LDLB_Block2.x)
		P0(m_u32Firmware_startingNAND);
		P0(m_u32Firmware_startingSector);
		P0(m_u32Firmware_sectorStride);
		P0(m_uSectorsInFirmware);
		P0(m_u32Firmware_startingNAND2);
		P0(m_u32Firmware_startingSector2);
		P0(m_u32Firmware_sectorStride2);
		P0(m_uSectorsInFirmware2);
		P0(FirmwareVersion.m_u16Major);
		P0(FirmwareVersion.m_u16Minor);
		P0(FirmwareVersion.m_u16Sub);
		P0(FirmwareVersion.m_u16Reserved);
		P0(m_u32DiscoveredBBTableSector);
		P0(m_u32DiscoveredBBTableSector2);
#undef P0

		printf("DBBT\n");
		// dump(md->curr_ldlb, sizeof(*md->curr_ldlb));
#undef P0
#define P0(x)	printf("  %s = %d\n", #x, md->curr_dbbt->DBBT_Block1.x)
		P0(m_u32NumberBB_NAND0);
		P0(m_u32NumberBB_NAND1);
		P0(m_u32NumberBB_NAND2);
		P0(m_u32NumberBB_NAND3);
		P0(m_u32Number2KPagesBB_NAND0);
		P0(m_u32Number2KPagesBB_NAND1);
		P0(m_u32Number2KPagesBB_NAND2);
		P0(m_u32Number2KPagesBB_NAND3);

		for (k = 0; k < 2; k++) {
			bbtn = md->bbtn[k];
			if (bbtn == NULL)
				continue;

			printf("BBTN#%d\n", k);
#undef P0
#define P0(x)	printf("  %s = %d\n", #x, bbtn->x)
			P0(uNAND);
			P0(uNumberBB);
#undef P0
			if (bbtn->uNumberBB > 0) {
				printf("  BADBLOCKS:");
				for (i = 0, j = 0; i < bbtn->uNumberBB; i++) {
					if (j == 0)
						printf("\n    ");
					printf(" 0x%x", bbtn->u32BadBlock[i]);
					if (++j > 16)
						j = 0;
				}
				if (j > 0)
					printf("\n");
			}
		}

		printf("Firmware: image #0 @ 0x%x size 0x%x - available 0x%x\n",
			md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector * 2048,
			md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware * 2048,
			( md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2 -
			 md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector) * 2048);

		printf("Firmware: image #1 @ 0x%x size 0x%x - available 0x%x\n",
			md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2 * 2048,
			md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware2 * 2048,
			mtd_size(md) - md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2 * 2048);
	}else if(rom_version == ROM_Version_1){
		int page_size = md->fcb.FCB_Block.m_u32DataPageSize;
#undef P1
#define P1(x)	printf("  %s = %d\n", #x, md->fcb.x)
		P1(m_u32Checksum);
		P1(m_u32FingerPrint);
		P1(m_u32Version);
		printf("FCB\n");
#undef P1
#define P1(x)	printf("  %s = %d\n", #x, md->fcb.FCB_Block.x)
		P1(m_NANDTiming.m_u8DataSetup);
		P1(m_NANDTiming.m_u8DataHold);
		P1(m_NANDTiming.m_u8AddressSetup);
		P1(m_NANDTiming.m_u8DSAMPLE_TIME);
		P1(m_u32DataPageSize);
		P1(m_u32TotalPageSize);
		P1(m_u32SectorsPerBlock);
		P1(m_u32NumberOfNANDs);
		P1(m_u32TotalInternalDie);
		P1(m_u32CellType);
		P1(m_u32EccBlockNEccType);
		P1(m_u32EccBlock0Size);
		P1(m_u32EccBlockNSize);
		P1(m_u32EccBlock0EccType);
		P1(m_u32MetadataBytes);
		P1(m_u32NumEccBlocksPerPage);
		P1(m_u32EccBlockNEccLevelSDK);
		P1(m_u32EccBlock0SizeSDK);
		P1(m_u32EccBlockNSizeSDK);
		P1(m_u32EccBlock0EccLevelSDK);
		P1(m_u32NumEccBlocksPerPageSDK);
		P1(m_u32MetadataBytesSDK);
		P1(m_u32EraseThreshold);
		P1(m_u32BootPatch);
		P1(m_u32PatchSectors);
		P1(m_u32Firmware1_startingSector);
		P1(m_u32Firmware2_startingSector);
		P1(m_u32SectorsInFirmware1);
		P1(m_u32SectorsInFirmware2);
		P1(m_u32DBBTSearchAreaStartAddress);
		P1(m_u32BadBlockMarkerByte);
		P1(m_u32BadBlockMarkerStartBit);
		P1(m_u32BBMarkerPhysicalOffset);
#undef P1
#define P1(x)	printf("  %s = %d\n", #x, md->fcb.DBBT_Block.x)
		P1(m_u32NumberBB);
		P1(m_u32Number2KPagesBB);

		printf("Firmware: image #0 @ 0x%x size 0x%x - available 0x%x\n",
			md->fcb.FCB_Block.m_u32Firmware1_startingSector * page_size,
			md->fcb.FCB_Block.m_u32SectorsInFirmware1 * page_size,
			(md->fcb.FCB_Block.m_u32Firmware2_startingSector -
			 md->fcb.FCB_Block.m_u32Firmware1_startingSector) * page_size);

		printf("Firmware: image #1 @ 0x%x size 0x%x - available 0x%x\n",
			md->fcb.FCB_Block.m_u32Firmware2_startingSector * page_size,
			md->fcb.FCB_Block.m_u32SectorsInFirmware2 * page_size,
			mtd_size(md) - md->fcb.FCB_Block.m_u32Firmware2_startingSector * page_size);
	}else{
		printf("unsupported ROM version \n");
	}

	return 0;
}

int v0_rom_mtd_init(struct mtd_data *md, FILE *fp)
{
	BootBlockStruct_t *ncb;
	BootBlockStruct_t *ldlb;
	BootBlockStruct_t *dbbt;
	BadBlockTableNand_t *bbtn;
	int search_area_sz, stride;
	unsigned int max_bootstream_sz;
	unsigned int bootstream_sz, bootstream1_pos, bootstream2_pos, bootstream_sectors;
	int i, j, k, badmax, thisbad, currbad;
	struct mtd_part *mp;

	stride = PAGES_PER_STRIDE * mtd_writesize(md);
	search_area_sz = (1 << md->cfg.search_exponent) * stride;

	if (search_area_sz * 6 > mtd_size(md)) {
		fprintf(stderr, "mtd: mtd size too small\n");
		return -1;
	}

	fseek(fp, 0, SEEK_END);
	bootstream_sz = ftell(fp);
	rewind(fp);

	max_bootstream_sz = (mtd_size(md) - search_area_sz * 6) / 2;

	if (md->flags & F_VERBOSE) {
		printf("mtd: max_bootstream_sz = %d\n", max_bootstream_sz);
		printf("mtd: bootstream_sz = %d\n", bootstream_sz);
	}

	if (bootstream_sz >= max_bootstream_sz) {
		fprintf(stderr, "mtd: bootstream too large\n");
		return -1;
	}
	bootstream1_pos = 6 * search_area_sz;
	bootstream2_pos = bootstream1_pos + max_bootstream_sz;
	bootstream_sectors = (bootstream_sz + 2047) / 2048;

	if (md->flags & F_VERBOSE) {
		printf("mtd: #1 0x%08x - 0x%08x (0x%08x)\n",
				bootstream1_pos, bootstream1_pos + max_bootstream_sz,
				bootstream1_pos + bootstream_sz);
		printf("mtd: #2 0x%08x - 0x%08x (0x%08x)\n",
				bootstream2_pos, bootstream2_pos + max_bootstream_sz,
				bootstream2_pos + bootstream_sz);
	}

	md->curr_ncb = &md->ncb[0];
	ncb = md->curr_ncb;

	md->curr_ldlb = &md->ldlb[0];
	ldlb = md->curr_ldlb;

	md->curr_dbbt = &md->dbbt[0];
	dbbt = md->curr_dbbt;

	/* clean BBTNs */
	if (md->bbtn[0] != NULL) {
		free(md->bbtn[0]);
		md->bbtn[0] = NULL;
	}
	if (md->bbtn[1] != NULL) {
		free(md->bbtn[1]);
		md->bbtn[0] = NULL;
	}

	memset(ncb, 0, sizeof(*ncb));
	memset(ldlb, 0, sizeof(*ldlb));
	memset(dbbt, 0, sizeof(*ldlb));

	ncb->m_u32FingerPrint1                        = NCB_FINGERPRINT1;

	ncb->NCB_Block1.m_NANDTiming.m_u8DataSetup    = md->cfg.data_setup_time;
	ncb->NCB_Block1.m_NANDTiming.m_u8DataHold     = md->cfg.data_hold_time;
	ncb->NCB_Block1.m_NANDTiming.m_u8AddressSetup = md->cfg.address_setup_time;
	ncb->NCB_Block1.m_NANDTiming.m_u8DSAMPLE_TIME = md->cfg.data_sample_time;

	ncb->NCB_Block1.m_u32DataPageSize             = mtd_writesize(md);
	ncb->NCB_Block1.m_u32TotalPageSize            = mtd_writesize(md) + mtd_oobsize(md);

	if (mtd_writesize(md) == 2048) {
		ncb->NCB_Block1.m_u32SectorsPerBlock          = mtd_erasesize(md) / mtd_writesize(md);
		ncb->NCB_Block1.m_u32SectorInPageMask         = 0;
		ncb->NCB_Block1.m_u32SectorToPageShift        = 0;
		ncb->NCB_Block2.m_u32ECCType                  = BCH_Ecc_8bit;
		ncb->NCB_Block2.m_u32EccBlock0EccLevel        = BCH_Ecc_8bit;
                ncb->NCB_Block2.m_u32EccBlock0Size            = 512;
                ncb->NCB_Block2.m_u32EccBlockNSize            = 512;
                ncb->NCB_Block2.m_u32NumEccBlocksPerPage      = mtd_writesize(md) / 512 - 1;
                ncb->NCB_Block2.m_u32MetadataBytes            = 10;

	} else if (mtd_writesize(md) == 4096) {
		ncb->NCB_Block1.m_u32SectorsPerBlock          = (mtd_erasesize(md) / mtd_writesize(md)) * 2;
		ncb->NCB_Block1.m_u32SectorInPageMask         = 1;
		ncb->NCB_Block1.m_u32SectorToPageShift        = 1;
		ncb->NCB_Block2.m_u32EccBlock0Size            = 512;
		ncb->NCB_Block2.m_u32EccBlockNSize	      = 512;
		ncb->NCB_Block2.m_u32NumEccBlocksPerPage      = (mtd_writesize(md) / 512) - 1;
		ncb->NCB_Block2.m_u32MetadataBytes            = 10;
		if (mtd_oobsize(md) == 218 || mtd_oobsize(md) == 224) {
			ncb->NCB_Block2.m_u32ECCType           = BCH_Ecc_16bit;
			ncb->NCB_Block2.m_u32EccBlock0EccLevel = BCH_Ecc_16bit;
		}  else if ((mtd_oobsize(md) == 128)){
			ncb->NCB_Block2.m_u32ECCType           = BCH_Ecc_8bit;
                        ncb->NCB_Block2.m_u32EccBlock0EccLevel = BCH_Ecc_8bit;
		}
	} else {
		fprintf(stderr, "Illegal page size %d\n", mtd_writesize(md));
	}

	ncb->NCB_Block1.m_u32NumberOfNANDs            = 1;

	ncb->m_u32FingerPrint2                        = NCB_FINGERPRINT2;

	ncb->NCB_Block2.m_u32NumRowBytes              = md->cfg.row_address_size;
	ncb->NCB_Block2.m_u32NumColumnBytes           = md->cfg.column_address_size;
	ncb->NCB_Block2.m_u32TotalInternalDie         = 1; // DontCare;
	ncb->NCB_Block2.m_u32InternalPlanesPerDie     = 1; // DontCare;
	ncb->NCB_Block2.m_u32CellType                 = 0; // ??? DontCare;
	ncb->NCB_Block2.m_u32Read1stCode              = md->cfg.read_command_code1;
	ncb->NCB_Block2.m_u32Read2ndCode              = md->cfg.read_command_code2;

	ncb->m_u32FingerPrint3                        = NCB_FINGERPRINT3;

	memcpy(&md->ncb[1], &md->ncb[0], sizeof(md->ncb[0]));

	ldlb->m_u32FingerPrint1                         = LDLB_FINGERPRINT1;

	ldlb->LDLB_Block1.LDLB_Version.m_u16Major       = LDLB_VERSION_MAJOR;
	ldlb->LDLB_Block1.LDLB_Version.m_u16Minor       = LDLB_VERSION_MINOR;
	ldlb->LDLB_Block1.LDLB_Version.m_u16Sub         = LDLB_VERSION_SUB;
	ldlb->LDLB_Block1.LDLB_Version.m_u16Reserved    = 0;
	ldlb->LDLB_Block1.m_u32NANDBitmap               = 0;

	ldlb->m_u32FingerPrint2                         = LDLB_FINGERPRINT2;

	ldlb->LDLB_Block2.m_u32Firmware_startingNAND    = 0;
	ldlb->LDLB_Block2.m_u32Firmware_startingSector  = bootstream1_pos / 2048; // BootStream1BasePageNumber;
	ldlb->LDLB_Block2.m_u32Firmware_sectorStride    = 0;
	ldlb->LDLB_Block2.m_uSectorsInFirmware          = bootstream_sectors; // BootStreamSizeIn2KSectors;
	ldlb->LDLB_Block2.m_u32Firmware_startingNAND2   = 0;	/* (ChipCount.Get() == 1) ? 0 : 1; */
	ldlb->LDLB_Block2.m_u32Firmware_startingSector2 = bootstream2_pos / 2048; // BootStream2BasePageNumber;
	ldlb->LDLB_Block2.m_u32Firmware_sectorStride2   = 0;
	ldlb->LDLB_Block2.m_uSectorsInFirmware2         = bootstream_sectors; // BootStreamSizeIn2KSectors;

	ldlb->LDLB_Block2.FirmwareVersion.m_u16Major    = md->cfg.boot_stream_major_version;
	ldlb->LDLB_Block2.FirmwareVersion.m_u16Minor    = md->cfg.boot_stream_minor_version;
	ldlb->LDLB_Block2.FirmwareVersion.m_u16Sub      = md->cfg.boot_stream_sub_version;
	ldlb->LDLB_Block2.FirmwareVersion.m_u16Reserved = 0;

	ldlb->LDLB_Block2.m_u32DiscoveredBBTableSector  = (search_area_sz * 4) / 2048; // DBBT1SearchAreaBasePageNumber;
	ldlb->LDLB_Block2.m_u32DiscoveredBBTableSector2 = (search_area_sz * 5) / 2048; // DBBT2SearchAreaBasePageNumber;

	ldlb->m_u32FingerPrint3                         = LDLB_FINGERPRINT3;

	memcpy(&md->ldlb[1], &md->ldlb[0], sizeof(md->ldlb[0]));

	dbbt->m_u32FingerPrint1                         = DBBT_FINGERPRINT1;
	dbbt->DBBT_Block1.m_u32NumberBB_NAND0		= 0;
	dbbt->DBBT_Block1.m_u32NumberBB_NAND1		= 0;
	dbbt->DBBT_Block1.m_u32NumberBB_NAND2		= 0;
	dbbt->DBBT_Block1.m_u32NumberBB_NAND3		= 0;
	dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND0	= 0;
	dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND1	= 0;
	dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND2	= 0;
	dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND3	= 0;
	dbbt->m_u32FingerPrint2                         = DBBT_FINGERPRINT2;
	dbbt->m_u32FingerPrint3                         = DBBT_FINGERPRINT3;

	/* maximum number of bad blocks that ROM supports */
	for (i = 0; i < 2; i++) {
		mp = &md->part[i];
		if (mp->nrbad == 0)
			continue;
		md->bbtn[i] = malloc(2048);	/* single page */
		if (md->bbtn[i] == NULL) {
			fprintf(stderr, "mtd: failed to allocate BBTN#%d\n", 2048);
			return -1;
		}
		bbtn = md->bbtn[i];

		memset(bbtn, 0, sizeof(*bbtn));

		badmax = ARRAY_SIZE(bbtn->u32BadBlock);
		thisbad = mp->nrbad;
		if (thisbad > badmax)
			thisbad = badmax;

		if (i == 0) {
			dbbt->DBBT_Block1.m_u32NumberBB_NAND0 = thisbad;
			dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND0 = 1;	/* one page */
		} else {
			dbbt->DBBT_Block1.m_u32NumberBB_NAND1 = thisbad;
			dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND1 = 1;	/* one page */
		}
		bbtn->uNAND = i;
		bbtn->uNumberBB = thisbad;

		/* fill in BBTN */
		j = mtd_size(md) / mtd_erasesize(md);
		currbad = 0;
		for (k = 0; k < j && currbad < thisbad; k++) {
			if ((mp->bad_blocks[k >> 5] & (1 << (k & 31))) == 0)
				continue;
			bbtn->u32BadBlock[currbad++] = k;
		}
	}

	memcpy(&md->dbbt[1], &md->dbbt[0], sizeof(md->dbbt[0]));

	return 0;
}

int v1_rom_mtd_init(struct mtd_data *md, FILE *fp)
{
	unsigned int  stride_size_in_bytes;
	unsigned int  search_area_size_in_bytes;
	unsigned int  search_area_size_in_pages;
	unsigned int  max_boot_stream_size_in_bytes;
	unsigned int  boot_stream_size_in_bytes;
	unsigned int  boot_stream_size_in_pages;
	unsigned int  boot_stream1_pos;
	unsigned int  boot_stream2_pos;
	V1_ROM_BootBlockStruct_t  *fcb;
	V1_ROM_BootBlockStruct_t  *dbbt;

	//----------------------------------------------------------------------
	// Compute the geometry of a search area.
	//----------------------------------------------------------------------

	stride_size_in_bytes = PAGES_PER_STRIDE * mtd_writesize(md);
	search_area_size_in_bytes = (1 << md->cfg.search_exponent) * stride_size_in_bytes;
	search_area_size_in_pages = (1 << md->cfg.search_exponent) * PAGES_PER_STRIDE;

	//----------------------------------------------------------------------
	// Check if the target MTD is too small to even contain the necessary
	// search areas.
	//
	// Recall that the boot area for the i.MX28 appears at the beginning of
	// the first chip and contains two search areas: one each for the FCB
	// and DBBT.
	//----------------------------------------------------------------------

	if ((search_area_size_in_bytes * 2) > mtd_size(md)) {
		fprintf(stderr, "mtd: mtd size too small\n");
		return -1;
	}

	//----------------------------------------------------------------------
	// Figure out how large a boot stream the target MTD could possibly
	// hold.
	//
	// The boot area will contain both search areas and two copies of the
	// boot stream.
	//----------------------------------------------------------------------

	max_boot_stream_size_in_bytes =

		(mtd_size(md) - search_area_size_in_bytes * 2) /
		//--------------------------------------------//
					2;

	//----------------------------------------------------------------------
	// Figure out how large the boot stream is.
	//----------------------------------------------------------------------

	fseek(fp, 0, SEEK_END);
	boot_stream_size_in_bytes = ftell(fp);
	rewind(fp);

	boot_stream_size_in_pages =

		(boot_stream_size_in_bytes + (mtd_writesize(md) - 1)) /
		//---------------------------------------------------//
				mtd_writesize(md);

	if (md->flags & F_VERBOSE) {
		printf("mtd: max_boot_stream_size_in_bytes = %d\n", max_boot_stream_size_in_bytes);
		printf("mtd: boot_stream_size_in_bytes = %d\n", boot_stream_size_in_bytes);
	}

	//----------------------------------------------------------------------
	// Check if the boot stream will fit.
	//----------------------------------------------------------------------

	if (boot_stream_size_in_bytes >= max_boot_stream_size_in_bytes) {
		fprintf(stderr, "mtd: bootstream too large\n");
		return -1;
	}

	//----------------------------------------------------------------------
	// Compute the positions of the boot stream copies.
	//----------------------------------------------------------------------

	boot_stream1_pos = 2 * search_area_size_in_bytes;
	boot_stream2_pos = boot_stream1_pos + max_boot_stream_size_in_bytes;

	if (md->flags & F_VERBOSE) {
		printf("mtd: #1 0x%08x - 0x%08x (0x%08x)\n",
				boot_stream1_pos, boot_stream1_pos + max_boot_stream_size_in_bytes,
				boot_stream1_pos + boot_stream_size_in_bytes);
		printf("mtd: #2 0x%08x - 0x%08x (0x%08x)\n",
				boot_stream2_pos, boot_stream2_pos + max_boot_stream_size_in_bytes,
				boot_stream2_pos + boot_stream_size_in_bytes);
	}

	//----------------------------------------------------------------------
	// Fill in the FCB.
	//----------------------------------------------------------------------

	fcb = &(md->fcb);
	memset(fcb, 0, sizeof(*fcb));

	fcb->m_u32FingerPrint                        = FCB_FINGERPRINT;
	fcb->m_u32Version                            = 0x01000000;

	fcb->FCB_Block.m_NANDTiming.m_u8DataSetup    = md->cfg.data_setup_time;
	fcb->FCB_Block.m_NANDTiming.m_u8DataHold     = md->cfg.data_hold_time;
	fcb->FCB_Block.m_NANDTiming.m_u8AddressSetup = md->cfg.address_setup_time;
	fcb->FCB_Block.m_NANDTiming.m_u8DSAMPLE_TIME = md->cfg.data_sample_time;

	fcb->FCB_Block.m_u32DataPageSize             = mtd_writesize(md);
	fcb->FCB_Block.m_u32TotalPageSize            = mtd_writesize(md) + mtd_oobsize(md);
	fcb->FCB_Block.m_u32SectorsPerBlock          = mtd_erasesize(md) / mtd_writesize(md);

	if (mtd_writesize(md) == 2048) {
                fcb->FCB_Block.m_u32NumEccBlocksPerPage      = mtd_writesize(md) / 512 - 1;
                fcb->FCB_Block.m_u32MetadataBytes            = 10;
                fcb->FCB_Block.m_u32EccBlock0Size            = 512;
                fcb->FCB_Block.m_u32EccBlockNSize            = 512;
		fcb->FCB_Block.m_u32EccBlock0EccType         = V1_ROM_BCH_Ecc_8bit;
		fcb->FCB_Block.m_u32EccBlockNEccType         = V1_ROM_BCH_Ecc_8bit;

	} else if (mtd_writesize(md) == 4096) {
		fcb->FCB_Block.m_u32NumEccBlocksPerPage      = (mtd_writesize(md) / 512) - 1;
		fcb->FCB_Block.m_u32MetadataBytes            = 10;
		fcb->FCB_Block.m_u32EccBlock0Size            = 512;
		fcb->FCB_Block.m_u32EccBlockNSize            = 512;
		if (mtd_oobsize(md) == 218) {
			fcb->FCB_Block.m_u32EccBlock0EccType = V1_ROM_BCH_Ecc_16bit;
			fcb->FCB_Block.m_u32EccBlockNEccType = V1_ROM_BCH_Ecc_16bit;
		} else if ((mtd_oobsize(md) == 128)){
			fcb->FCB_Block.m_u32EccBlock0EccType = V1_ROM_BCH_Ecc_8bit;
			fcb->FCB_Block.m_u32EccBlockNEccType = V1_ROM_BCH_Ecc_8bit;
		}
	} else {
		fprintf(stderr, "Illegal page size %d\n", mtd_writesize(md));
	}

	fcb->FCB_Block.m_u32BootPatch                  = 0; // Normal boot.

	fcb->FCB_Block.m_u32Firmware1_startingSector   = boot_stream1_pos / mtd_writesize(md);
	fcb->FCB_Block.m_u32Firmware2_startingSector   = boot_stream2_pos / mtd_writesize(md);
	fcb->FCB_Block.m_u32SectorsInFirmware1         = boot_stream_size_in_pages;
	fcb->FCB_Block.m_u32SectorsInFirmware2         = boot_stream_size_in_pages;
	fcb->FCB_Block.m_u32DBBTSearchAreaStartAddress = search_area_size_in_pages;
	fcb->FCB_Block.m_u32BadBlockMarkerByte         = md->nfc_geometry.block_mark_byte_offset;
	fcb->FCB_Block.m_u32BadBlockMarkerStartBit     = md->nfc_geometry.block_mark_bit_offset;
	fcb->FCB_Block.m_u32BBMarkerPhysicalOffset     = mtd_writesize(md);

	//----------------------------------------------------------------------
	// Fill in the DBBT.
	//----------------------------------------------------------------------

	dbbt = &(md->dbbt28);
	memset(dbbt, 0, sizeof(*dbbt));

	dbbt->m_u32FingerPrint                = DBBT_FINGERPRINT2;
	dbbt->m_u32Version                    = 1;

	dbbt->DBBT_Block.m_u32NumberBB        = 0;
	dbbt->DBBT_Block.m_u32Number2KPagesBB = 0;

	return 0;

}

v2_rom_mtd_init(struct mtd_data *md, FILE *fp)
{
	unsigned int  stride_size_in_bytes;
	unsigned int  search_area_size_in_bytes;
	unsigned int  search_area_size_in_pages;
	unsigned int  max_boot_stream_size_in_bytes;
	unsigned int  boot_stream_size_in_bytes;
	unsigned int  boot_stream_size_in_pages;
	unsigned int  boot_stream1_pos;
	unsigned int  boot_stream2_pos;
	V1_ROM_BootBlockStruct_t  *fcb;
	V1_ROM_BootBlockStruct_t  *dbbt;
	struct mtd_part *mp;
	int j, k , thisbad, badmax,currbad;
	BadBlockTableNand_t *bbtn;

	//----------------------------------------------------------------------
	// Compute the geometry of a search area.
	//----------------------------------------------------------------------

	stride_size_in_bytes = mtd_erasesize;
	search_area_size_in_bytes = 4 * stride_size_in_bytes;
	search_area_size_in_pages = search_area_size_in_bytes / mtd_writesize(md);

	//----------------------------------------------------------------------
	// Check if the target MTD is too small to even contain the necessary
	// search areas.
	//
	// the first chip and contains two search areas: one each for the FCB
	// and DBBT.
	//----------------------------------------------------------------------

	if ((search_area_size_in_bytes * 2) > mtd_size(md)) {
		fprintf(stderr, "mtd: mtd size too small\n");
		return -1;
	}

	//----------------------------------------------------------------------
	// Figure out how large a boot stream the target MTD could possibly
	// hold.
	//
	// The boot area will contain both search areas and two copies of the
	// boot stream.
	//----------------------------------------------------------------------

	max_boot_stream_size_in_bytes =

		(mtd_size(md) - search_area_size_in_bytes * 2) /
		//--------------------------------------------//
					2;

	//----------------------------------------------------------------------
	// Figure out how large the boot stream is.
	//----------------------------------------------------------------------

	fseek(fp, 0, SEEK_END);
	boot_stream_size_in_bytes = ftell(fp);
	rewind(fp);

	boot_stream_size_in_pages =

		(boot_stream_size_in_bytes + (mtd_writesize(md) - 1)) /
		//---------------------------------------------------//
				mtd_writesize(md);

	if (md->flags & F_VERBOSE) {
		printf("mtd: max_boot_stream_size_in_bytes = %d\n", max_boot_stream_size_in_bytes);
		printf("mtd: boot_stream_size_in_bytes = %d\n", boot_stream_size_in_bytes);
	}

	//----------------------------------------------------------------------
	// Check if the boot stream will fit.
	//----------------------------------------------------------------------

	if (boot_stream_size_in_bytes >= max_boot_stream_size_in_bytes) {
		fprintf(stderr, "mtd: bootstream too large\n");
		return -1;
	}

	//----------------------------------------------------------------------
	// Compute the positions of the boot stream copies.
	//----------------------------------------------------------------------

	boot_stream1_pos = 2 * search_area_size_in_bytes;
	boot_stream2_pos = boot_stream1_pos + max_boot_stream_size_in_bytes;

	if (md->flags & F_VERBOSE) {
		printf("mtd: #1 0x%08x - 0x%08x (0x%08x)\n",
				boot_stream1_pos, boot_stream1_pos + max_boot_stream_size_in_bytes,
				boot_stream1_pos + boot_stream_size_in_bytes);
		printf("mtd: #2 0x%08x - 0x%08x (0x%08x)\n",
				boot_stream2_pos, boot_stream2_pos + max_boot_stream_size_in_bytes,
				boot_stream2_pos + boot_stream_size_in_bytes);
	}

	//----------------------------------------------------------------------
	// Fill in the FCB.
	//----------------------------------------------------------------------

	fcb = &(md->fcb);
	memset(fcb, 0, sizeof(*fcb));

	fcb->m_u32FingerPrint                        = FCB_FINGERPRINT;
	fcb->m_u32Version                            = 0x00000001;

	fcb->FCB_Block.m_u32Firmware1_startingSector   = boot_stream1_pos / mtd_writesize(md);
	fcb->FCB_Block.m_u32Firmware2_startingSector   = boot_stream2_pos / mtd_writesize(md);
	fcb->FCB_Block.m_u32SectorsInFirmware1         = boot_stream_size_in_pages;
	fcb->FCB_Block.m_u32SectorsInFirmware2         = boot_stream_size_in_pages;
	fcb->FCB_Block.m_u32DBBTSearchAreaStartAddress = search_area_size_in_pages;


	//----------------------------------------------------------------------
	// Fill in the DBBT.
	//----------------------------------------------------------------------

	dbbt = &(md->dbbt28);
	memset(dbbt, 0, sizeof(*dbbt));

	dbbt->m_u32FingerPrint                = DBBT_FINGERPRINT2;
	dbbt->m_u32Version                    = 1;

	/* Only check boot partition that ROM support */

	mp = &md->part[0];
	if (mp->nrbad == 0)
		return 0;

	md->bbtn[0] = malloc(2048); /* single page */
	if (md->bbtn[0] == NULL) {
		fprintf(stderr, "mtd: failed to allocate BBTN#%d\n", 2048);
		return -1;
	}

	bbtn = md->bbtn[0];
	memset(bbtn, 0, sizeof(*bbtn));

	badmax = ARRAY_SIZE(bbtn->u32BadBlock);
	thisbad = mp->nrbad;
	if (thisbad > badmax)
		thisbad = badmax;


	dbbt->DBBT_Block.m_u32NumberBB = thisbad;
	dbbt->DBBT_Block.m_u32Number2KPagesBB = 1; /* one page should be enough*/

	bbtn->uNumberBB = thisbad;

	/* fill in BBTN */
	j = mtd_size(md) / mtd_erasesize(md);
	currbad = 0;
	for (k = 0; k < j && currbad < thisbad; k++) {
		if ((mp->bad_blocks[k >> 5] & (1 << (k & 31))) == 0)
			continue;
		bbtn->u32BadBlock[currbad++] = k;
	}

	return 0;

}


//------------------------------------------------------------------------------
// This function writes the search areas for a given BCB. It will write *two*
// search areas for a given BCB. If there are multiple chips, it will write one
// search area on each chip. If there is one chip, it will write two search
// areas on the first chip.
//
// md         A pointer to the current struct mtd_data.
// bcb_name   A pointer to a human-readable string that indicates what kind of
//            BCB we're writing. This string will only be used in log messages.
// ofs1       If there is one chips, the index of the
// ofs2
// ofs_mchip  If there are multiple chips, the index of the search area to write
//            on both chips.
// end        The number of consecutive search areas to be written.
// size       The size of the BCB data to be written.
// ecc        Indicates whether or not to use hardware ECC.
//------------------------------------------------------------------------------

int mtd_commit_bcb(struct mtd_data *md, char *bcb_name,
		   loff_t ofs1, loff_t ofs2, loff_t ofs_mchip,
		   loff_t end, size_t size, int ecc)
{
	int chip;
	loff_t end_index, search_area_indices[2], o;
	int err = 0, r;
	int i;
	int j;
	unsigned stride_size_in_bytes;
	unsigned search_area_size_in_strides;
	unsigned search_area_size_in_bytes;

	//----------------------------------------------------------------------
	// Compute some important facts about geometry.
	//----------------------------------------------------------------------
	if (rom_version == ROM_Version_2) {

		stride_size_in_bytes        = mtd_erasesize(md);
		search_area_size_in_strides = 4;
		search_area_size_in_bytes   = search_area_size_in_strides * stride_size_in_bytes;

	} else {
		stride_size_in_bytes        = PAGES_PER_STRIDE * mtd_writesize(md);
		search_area_size_in_strides = 1 << md->cfg.search_exponent;
		search_area_size_in_bytes   = search_area_size_in_strides * stride_size_in_bytes;
	}

	//----------------------------------------------------------------------
	// Check whether there are multiple chips and set up the two search area
	// indices accordingly.
	//----------------------------------------------------------------------

	if (multichip(md))
		search_area_indices[0] = search_area_indices[1] = ofs_mchip;
	else {
		search_area_indices[0] = ofs1;
		search_area_indices[1] = ofs2;
	}

	//----------------------------------------------------------------------
	// Loop over search areas for this BCB.
	//----------------------------------------------------------------------

	for (i = 0; !err && i < 2; i++) {

		//--------------------------------------------------------------
		// Compute the search area index that marks the end of the
		// writing on this chip.
		//--------------------------------------------------------------

		end_index = search_area_indices[i] + end;

		//--------------------------------------------------------------
		// Figure out which chip we're writing.
		//--------------------------------------------------------------

		chip = multichip(md) ? i : 0;

		//--------------------------------------------------------------
		// Loop over consecutive search areas to write.
		//--------------------------------------------------------------

		for (; search_area_indices[i] < end_index; search_area_indices[i]++) {

			//------------------------------------------------------
			// Compute the byte offset of the beginning of this
			// search area.
			//------------------------------------------------------

			o = search_area_indices[i] * search_area_size_in_bytes;

			//------------------------------------------------------
			// Loop over strides in this search area.
			//------------------------------------------------------

			for (j = 0; j < search_area_size_in_strides; j++, o += stride_size_in_bytes) {

				//----------------------------------------------
				// If we're crossing into a new block, erase it
				// first.
				//----------------------------------------------

				if ((o % mtd_erasesize(md)) == 0) {
					r = mtd_erase_block(md, chip, o);
					if (r < 0) {
						fprintf(stderr, "mtd: Failed to erase block @0x%llx\n", o);
						err++;
						continue;
					}
				}

				//----------------------------------------------
				// Write the page.
				//----------------------------------------------

				if (md->flags & F_VERBOSE)
					printf("mtd: Writing %s%d @%d:0x%llx(%x)\n",
								bcb_name, j, chip, o, size);

				r = mtd_write_page(md, chip, o, ecc);
				if (r != size) {
					fprintf(stderr, "mtd: Failed to write %s @%d: 0x%llx (%d)\n",
						bcb_name, chip, o, r);
					err ++;
				}

			}

		}

	}

	if (md->flags & F_VERBOSE)
		printf("%s(%s): status %d\n", __func__, bcb_name, err);

	return err;
}

int v0_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags)
{
	int startpage, start, size;
	unsigned int search_area_sz, stride;
	int i, j, r, sz, chunk;
	loff_t ofs, end;
	int chip = 0;

	stride = PAGES_PER_STRIDE * mtd_writesize(md);
	search_area_sz = (1 << md->cfg.search_exponent) * stride;

	for (i = 0; i < 2; i++) {

		if (fp == NULL || (flags & UPDATE_BS(i)) == 0)
			continue;

		if (i == 0) {
			startpage = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector;
			size = md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware;
			end = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2;
			chip = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingNAND;
		} else {
			startpage = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingSector2;
			size = md->curr_ldlb->LDLB_Block2.m_uSectorsInFirmware2 ;
			end = mtd_size(md) / 2048;
			chip = md->curr_ldlb->LDLB_Block2.m_u32Firmware_startingNAND2;
		}

		start = startpage * 2048;
		size = size * 2048;
		end = end * 2048;

		if (md->flags & F_VERBOSE)
			printf("mtd: Writting firmware image #%d @%d: 0x%08x - 0x%08x\n", i,
					chip, start, start + size);

		rewind(fp);

		ofs = start;
		while (ofs < end && size > 0) {

			/* skip bad */
			while (mtd_isbad(md, chip, ofs) == 1) {
				if (md->flags & F_VERBOSE)
					printf("mtd: Skipping bad block at 0x%llx\n", ofs);
				ofs += mtd_erasesize(md);
			}

			chunk = size;

			/* entered new erase block, nuke */
			if ((ofs % mtd_erasesize(md)) == 0) {
				r = mtd_erase_block(md, chip, ofs);
				if (r < 0) {
					fprintf(stderr, "mtd: Failed to erase block @0x%llx\n", ofs);
					ofs += mtd_erasesize(md);
					continue;
				}
			}

			if (chunk > mtd_writesize(md))
				chunk = mtd_writesize(md);

			r = fread(md->buf, 1, chunk, fp);
			if (r < 0) {
				fprintf(stderr, "mtd: Failed %d (fread %d)\n", r, chunk);
				return -1;
			}
			if (r < chunk)
				memset(md->buf + r, 0, chunk - r);

			r = mtd_write_page(md, chip, ofs, 1);
			if (r != mtd_writesize(md)) {
				fprintf(stderr, "mtd: Failed to write BS @0x%llx (%d)\n", ofs, r);
			}
			ofs += mtd_writesize(md);
			size -= chunk;
		}

		if (ofs >= end) {
			fprintf(stderr, "mtd: Failed to write BS#%d\n", i);
			return -1;
		}
	}

	if (flags & UPDATE_NCB) {

		size = mtd_writesize(md) + mtd_oobsize(md);
		memset(md->buf, 0xff, size);

		if (md->flags & F_VERBOSE) {
			if (md->ncb_version != md->cfg.ncb_version)
				printf("NCB versions differ, %d is used.\n", md->cfg.ncb_version);
		}

		r = ncb_encrypt(md->curr_ncb, md->buf, size, md->cfg.ncb_version);
		if (r < 0)
			return r;

		mtd_commit_bcb(md, "NCB", 0, 1, 0, 1, size, false);
	}

	if (flags & UPDATE_LDLB) {

		/* LDLBs are right after the NCBs */
		memset(md->buf, 0, mtd_writesize(md));
		memcpy(md->buf, md->curr_ldlb, sizeof(*md->curr_ldlb));

		mtd_commit_bcb(md, "LDLB", 2, 3, 1, 1, mtd_writesize(md), true);
	}

	if (flags & UPDATE_DBBT) {

		/* DBBTs are right after the LDLB */
		memset(md->buf, 0, mtd_writesize(md));
		memcpy(md->buf, md->curr_dbbt, sizeof(*md->curr_dbbt));

		mtd_commit_bcb(md, "DBBT", 4, 5, 2, 1, mtd_writesize(md), true);
		for (i = 0; i < 2; i++) {
			for (j = 0; j < 2; j++) {
				if (md->flags & F_MULTICHIP) {
					chip = i;
					ofs = 2 * search_area_sz;
				} else {
					chip = 0;
					ofs = (4 + i) * search_area_sz;
				}
				if (j == 0)
					sz = md->curr_dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND0;
				else
					sz = md->curr_dbbt->DBBT_Block1.m_u32Number2KPagesBB_NAND1;
				if (sz > 0 && md->bbtn[j] != NULL) {
					memset(md->buf, 0, mtd_writesize(md));
					memcpy(md->buf, md->bbtn[j], sizeof(*md->bbtn[j]));

					if (md->flags & F_VERBOSE)
						printf("mtd: PUTTING down DBBT%d BBTN%d @0x%llx (0x%x)\n", i, j,
							ofs + (4 + j) * 2048, mtd_writesize(md));

					r = mtd_write_page(md, chip, ofs + (4 + j) * 2048, 1);
					if (r != mtd_writesize(md)) {
						fprintf(stderr, "mtd: Failed to write BBTN @0x%llx (%d)\n", ofs, r);
					}
				}
			}
		}
	}

	return 0;
}

int v1_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags)
{
	int startpage, start, size;
	unsigned int search_area_size_in_bytes, stride_size_in_bytes;
	int i, r, chunk;
	loff_t ofs, end;
	int chip = 0;

	//----------------------------------------------------------------------
	// Compute some important facts about geometry.
	//----------------------------------------------------------------------

	stride_size_in_bytes = PAGES_PER_STRIDE * mtd_writesize(md);
	search_area_size_in_bytes = (1 << md->cfg.search_exponent) * stride_size_in_bytes;

	//----------------------------------------------------------------------
	// Construct the ECC decorations and such for the FCB.
	//----------------------------------------------------------------------

	size = mtd_writesize(md) + mtd_oobsize(md);

	if (md->flags & F_VERBOSE) {
		if (md->ncb_version != md->cfg.ncb_version)
			printf("NCB versions differ, %d is used.\n", md->cfg.ncb_version);
	}

	r = fcb_encrypt(&md->fcb, md->buf, size, 1);
	if (r < 0)
		return r;

	//----------------------------------------------------------------------
	// Write the FCB search area.
	//----------------------------------------------------------------------

	mtd_commit_bcb(md, "FCB", 0, 0, 0, 1, size, false);

	//----------------------------------------------------------------------
	// Write the DBBT search area.
	//----------------------------------------------------------------------

	memset(md->buf, 0, mtd_writesize(md));
	memcpy(md->buf, &(md->dbbt28), sizeof(md->dbbt28));

	mtd_commit_bcb(md, "DBBT", 1, 1, 1, 1, mtd_writesize(md), true);

	//----------------------------------------------------------------------
	// Loop over the two boot streams.
	//----------------------------------------------------------------------

	for (i = 0; i < 2; i++) {

		//--------------------------------------------------------------
		// Check if we're actually supposed to write this boot stream.
		//--------------------------------------------------------------

		if (fp == NULL || (flags & UPDATE_BS(i)) == 0)
			continue;

		//--------------------------------------------------------------
		// Figure out where to put the current boot stream.
		//--------------------------------------------------------------

		if (i == 0) {
			startpage = md->fcb.FCB_Block.m_u32Firmware1_startingSector;
			size      = md->fcb.FCB_Block.m_u32SectorsInFirmware1;
			end       = md->fcb.FCB_Block.m_u32Firmware2_startingSector;
		} else {
			startpage = md->fcb.FCB_Block.m_u32Firmware2_startingSector;
			size      = md->fcb.FCB_Block.m_u32SectorsInFirmware2;
			end       = mtd_size(md) / mtd_writesize(md);
		}

		//--------------------------------------------------------------
		// Compute the byte addresses corresponding to the page
		// addresses.
		//--------------------------------------------------------------

		start = startpage * mtd_writesize(md);
		size  = size      * mtd_writesize(md);
		end   = end       * mtd_writesize(md);

		if (md->flags & F_VERBOSE)
			printf("mtd: Writting firmware image #%d @%d: 0x%08x - 0x%08x\n", i,
					chip, start, start + size);

		rewind(fp);

		//--------------------------------------------------------------
		// Loop over pages as we write them.
		//--------------------------------------------------------------

		ofs = start;
		while (ofs < end && size > 0) {

			//------------------------------------------------------
			// Check if the current block is bad.
			//------------------------------------------------------

			while (mtd_isbad(md, chip, ofs) == 1) {
				if (md->flags & F_VERBOSE)
					printf("mtd: Skipping bad block at 0x%llx\n", ofs);
				ofs += mtd_erasesize(md);
			}

			chunk = size;

			//------------------------------------------------------
			// Check if we've entered a new block and, if so, erase
			// it before beginning to write it.
			//------------------------------------------------------

			if ((ofs % mtd_erasesize(md)) == 0) {
				r = mtd_erase_block(md, chip, ofs);
				if (r < 0) {
					fprintf(stderr, "mtd: Failed to erase block @0x%llx\n", ofs);
					ofs += mtd_erasesize(md);
					continue;
				}
			}

			//------------------------------------------------------
			// Read the current chunk from the boot stream file.
			//------------------------------------------------------

			if (chunk > mtd_writesize(md))
				chunk = mtd_writesize(md);

			r = fread(md->buf, 1, chunk, fp);
			if (r < 0) {
				fprintf(stderr, "mtd: Failed %d (fread %d)\n", r, chunk);
				return -1;
			}
			if (r < chunk)
				memset(md->buf + r, 0, chunk - r);

			//------------------------------------------------------
			// Write the current chunk to the medium.
			//------------------------------------------------------

			r = mtd_write_page(md, chip, ofs, 1);
			if (r != mtd_writesize(md)) {
				fprintf(stderr, "mtd: Failed to write BS @0x%llx (%d)\n", ofs, r);
			}
			ofs += mtd_writesize(md);
			size -= chunk;

		}

		//--------------------------------------------------------------
		// Check if we ran out of room.
		//--------------------------------------------------------------

		if (ofs >= end) {
			fprintf(stderr, "mtd: Failed to write BS#%d\n", i);
			return -1;
		}

	}

	return 0;

}

v2_rom_mtd_commit_structures(struct mtd_data *md, FILE *fp, int flags)
{
	int startpage, start, size;
	unsigned int search_area_size_in_bytes, stride_size_in_bytes;
	int i, r, chunk;
	loff_t ofs, end;
	int chip = 0;
	const char *attrfile = "/sys/devices/platform/mxc_nandv2_flash.0/disable_bi_swap";

	/* For MX53 TO1, ROM does not support bi_swap */
	dev_attr_write_int(attrfile, 1);

	//----------------------------------------------------------------------
	// Compute some important facts about geometry.
	//----------------------------------------------------------------------

	stride_size_in_bytes = mtd_erasesize(md);
	search_area_size_in_bytes = 4 * stride_size_in_bytes;

	//----------------------------------------------------------------------
	// Construct the ECC decorations and such for the FCB.
	//----------------------------------------------------------------------

	size = mtd_writesize(md) + mtd_oobsize(md);

	if (md->flags & F_VERBOSE) {
		if (md->ncb_version != md->cfg.ncb_version)
			printf("NCB versions differ, %d is used.\n", md->cfg.ncb_version);
	}

	//----------------------------------------------------------------------
	// Write the FCB search area.
	//----------------------------------------------------------------------

	memset(md->buf, 0, mtd_writesize(md));
	memcpy(md->buf, &(md->fcb), sizeof(md->fcb));

	mtd_commit_bcb(md, "FCB", 0, 0, 0, 1, mtd_writesize(md), true);

	//----------------------------------------------------------------------
	// Write the DBBT search area.
	//----------------------------------------------------------------------

	memset(md->buf, 0, mtd_writesize(md));
	memcpy(md->buf, &(md->dbbt28), sizeof(md->dbbt28));

	mtd_commit_bcb(md, "DBBT", 1, 1, 1, 1, mtd_writesize(md), true);

	//----------------------------------------------------------------------
	// Write the DBBT table area.
	//----------------------------------------------------------------------

	memset(md->buf, 0, mtd_writesize(md));

	if (md->dbbt28.DBBT_Block.m_u32Number2KPagesBB> 0 && md->bbtn[0] != NULL) {
		memcpy(md->buf, md->bbtn[0], sizeof(*md->bbtn[0]));

		ofs = search_area_size_in_bytes;

		for (i=0; i < 4; i++, ofs += stride_size_in_bytes) {

			if (md->flags & F_VERBOSE)
				printf("mtd: PUTTING down DBBT%d BBTN%d @0x%llx (0x%x)\n", i, 0,
					ofs + 4 * mtd_writesize(md), mtd_writesize(md));

			r = mtd_write_page(md, chip, ofs + 4 * mtd_writesize(md), 1);
			if (r != mtd_writesize(md)) {
				fprintf(stderr, "mtd: Failed to write BBTN @0x%llx (%d)\n", ofs, r);
			}
		}
	}

	//----------------------------------------------------------------------
	// Loop over the two boot streams.
	//----------------------------------------------------------------------

	for (i = 0; i < 1; i++) {

		//--------------------------------------------------------------
		// Check if we're actually supposed to write this boot stream.
		//--------------------------------------------------------------

		if (fp == NULL || (flags & UPDATE_BS(i)) == 0)
			continue;

		//--------------------------------------------------------------
		// Figure out where to put the current boot stream.
		//--------------------------------------------------------------

		startpage = md->fcb.FCB_Block.m_u32Firmware1_startingSector;
		size      = md->fcb.FCB_Block.m_u32SectorsInFirmware1;
		end       = md->fcb.FCB_Block.m_u32Firmware2_startingSector;

		//--------------------------------------------------------------
		// Compute the byte addresses corresponding to the page
		// addresses.
		//--------------------------------------------------------------

		start = startpage * mtd_writesize(md);
		size  = size      * mtd_writesize(md);
		end   = end       * mtd_writesize(md);

		if (md->flags & F_VERBOSE)
			printf("mtd: Writting firmware image #%d @%d: 0x%08x - 0x%08x\n", i,
					chip, start, start + size);

		rewind(fp);

		//--------------------------------------------------------------
		// Loop over pages as we write them.
		//--------------------------------------------------------------

		ofs = start;
		while (ofs < end && size > 0) {

			//------------------------------------------------------
			// Check if the current block is bad.
			//------------------------------------------------------

			while (mtd_isbad(md, chip, ofs) == 1) {
				if (md->flags & F_VERBOSE)
					printf("mtd: Skipping bad block at 0x%llx\n", ofs);
				ofs += mtd_erasesize(md);
			}

			chunk = size;

			//------------------------------------------------------
			// Check if we've entered a new block and, if so, erase
			// it before beginning to write it.
			//------------------------------------------------------

			if ((ofs % mtd_erasesize(md)) == 0) {
				r = mtd_erase_block(md, chip, ofs);
				if (r < 0) {
					fprintf(stderr, "mtd: Failed to erase block @0x%llx\n", ofs);
					ofs += mtd_erasesize(md);
					continue;
				}
			}

			//------------------------------------------------------
			// Read the current chunk from the boot stream file.
			//------------------------------------------------------

			if (chunk > mtd_writesize(md))
				chunk = mtd_writesize(md);

			r = fread(md->buf, 1, chunk, fp);
			if (r < 0) {
				fprintf(stderr, "mtd: Failed %d (fread %d)\n", r, chunk);
				dev_attr_write_int(attrfile, 0);
				return -1;
			}
			if (r < chunk)
				memset(md->buf + r, 0, chunk - r);

			//------------------------------------------------------
			// Write the current chunk to the medium.
			//------------------------------------------------------

			r = mtd_write_page(md, chip, ofs, 1);
			if (r != mtd_writesize(md)) {
				fprintf(stderr, "mtd: Failed to write BS @0x%llx (%d)\n", ofs, r);
			}
			ofs += mtd_writesize(md);
			size -= chunk;

		}

		//--------------------------------------------------------------
		// Check if we ran out of room.
		//--------------------------------------------------------------

		if (ofs >= end) {
			fprintf(stderr, "mtd: Failed to write BS#%d\n", i);
			dev_attr_write_int(attrfile, 0);
			return -1;
		}

	}

	dev_attr_write_int(attrfile, 0);

	return 0;

}


#undef ARG
#define ARG(x) { .name = #x , .offset = offsetof(struct mtd_config, x), .ignore = false, }
#define ARG_IGNORE(x) { .name = #x , .offset = offsetof(struct mtd_config, x), .ignore = true, }

static const struct {
	const char *name;
	int offset;
	int ignore;
} mtd_int_args[] = {
	ARG_IGNORE(chip_count),
	ARG_IGNORE(chip_0_offset),
	ARG_IGNORE(chip_0_size),
	ARG_IGNORE(chip_1_offset),
	ARG_IGNORE(chip_1_size),
	ARG(search_exponent),
	ARG(data_setup_time),
	ARG(data_hold_time),
	ARG(address_setup_time),
	ARG(data_sample_time),
	ARG(row_address_size),
	ARG(column_address_size),
	ARG(read_command_code1),
	ARG(read_command_code2),
	ARG(boot_stream_major_version),
	ARG(boot_stream_minor_version),
	ARG(ncb_version),
	ARG(boot_stream_1_address),
	ARG(boot_stream_2_address),
}, mtd_charp_args[] = {
	ARG(chip_0_device_path),
	ARG(chip_1_device_path),
};

#undef ARG

void mtd_parse_kobs(struct mtd_config *cfg, const char *name, int verbose)
{
	int j, lineno;
	FILE *fp;
	char line[BUFSIZ];
	char *p, *s;
	char *arg, *val;
	int *ip;
	char c;
	char **cp;

	fp = fopen(name, "ra");
	if (fp == NULL)
		return;

	lineno = 0;
	while (fgets(line, sizeof(line), fp)) {

		lineno++;

		/* remove trailing '\r', or '\n' */
		s = line + strlen(line);
		while (s > line && (s[-1] == '\r' || s[-1] == '\n'))
			*--s = '\0';

		/* remove comments */
		s = strchr(line, '#');
		if (s != NULL)
			*s = '\0';

		p = line;
		while (isspace(*p))
			p++;

		if (*p =='\0')
			continue;

		arg = p;
		while (!isspace(*p) && *p != '=')
			p++;

		c = *p;
		*p++ = '\0';
		if (c != '=') {
			s = strchr(p, '=');
			if (s == NULL) {
				fprintf(stderr, "line %d: syntax error\n", lineno);
				return;
			}
			p = s + 1;
		}
		while (isspace(*p))
			p++;
		val = p;

		/* possible, check for each */
		for (j = 0; j < ARRAY_SIZE(mtd_int_args); j++) {
			if (strcmp(mtd_int_args[j].name, arg) == 0)
				break;
		}

		if (j < ARRAY_SIZE(mtd_int_args)) {
			if (mtd_int_args[j].ignore) {
				fprintf(stderr, "WARNING: Parameter '%s' is no longer used, ignoring\n",
						mtd_int_args[j].name);
				continue;
			}
			ip = (int *)((void *)cfg + mtd_int_args[j].offset);
			*ip = strtoul(val, NULL, 0);

			if (verbose)
				printf("%s -> %d (decimal), 0x%x (hex)\n", mtd_int_args[j].name, *ip, *ip);

			continue;

		}

		/* possible, check for each */
		for (j = 0; j < ARRAY_SIZE(mtd_charp_args); j++) {
			if (strcmp(mtd_charp_args[j].name, arg) == 0)
				break;
		}

		if (j < ARRAY_SIZE(mtd_charp_args)) {
			if (mtd_int_args[j].ignore) {
				fprintf(stderr, "WARNING: Parameter '%s' is no longer used, ignoring\n",
						mtd_int_args[j].name);
				continue;
			}
			cp = (char **)((void *)cfg + mtd_charp_args[j].offset);
			*cp = strdup(val);	/* XXX yes, I know, memory leak */

			if (verbose)
				printf("%s -> \"%s\" (char *)\n", mtd_charp_args[j].name, *cp);
			continue;

		}

		fprintf(stderr, "Unknown arg '%s' at line %d\n", arg, lineno);
		return;
	}
}

void mtd_parse_args(struct mtd_config *cfg, int argc, char **argv)
{
	int i, j;
	char *p, *s;
	int *ip;
	char **cp;

	for (i = 1; i < argc; i++) {

		/* check for --argument= */
		if (argv[i][0] != '-' || argv[i][1] != '-')
			continue;

		p = &argv[i][2];
		s = strchr(p, '=');
		if (s == NULL)
			continue;

		/* possible, check for each */
		for (j = 0; j < ARRAY_SIZE(mtd_int_args); j++) {
			if (strlen(mtd_int_args[j].name) == (s - p) &&
			    memcmp(mtd_int_args[j].name, p, s - p) == 0)
				break;
		}

		if (j < ARRAY_SIZE(mtd_int_args)) {
			ip = (int *)((void *)cfg + mtd_int_args[j].offset);
			*ip = strtoul(s + 1, NULL, 0);

			/* printf("%s -> %d (int)\n", mtd_int_args[j].name, *ip); */
			continue;
		}

		/* possible, check for each */
		for (j = 0; j < ARRAY_SIZE(mtd_charp_args); j++) {
			if (strlen(mtd_charp_args[j].name) == (s - p) &&
			    memcmp(mtd_charp_args[j].name, p, s - p) == 0)
				break;
		}

		if (j < ARRAY_SIZE(mtd_charp_args)) {
			cp = (char **)((void *)cfg + mtd_charp_args[j].offset);
			*cp = s + 1;

			/* printf("%s -> \"%s\" (char *)\n", mtd_charp_args[j].name, *cp); */
			continue;

		}

		fprintf(stderr, "Unknown argument '%s'\n", argv[i]);
		break;
	}
}

void mtd_cfg_dump(struct mtd_config *cfg)
{
	printf("MTD CONFIG:\n");

#undef Pd
#undef Ps
#define Pd(x)	printf("  %s = %d\n", #x, cfg->x)
#define Ps(x)	printf("  %s = \"%s\"\n", #x, cfg->x)

//	Pd(chip_count);
	Ps(chip_0_device_path);
//	Pd(chip_0_offset);
//	Pd(chip_0_size);
	Ps(chip_1_device_path);
//	Pd(chip_1_offset);
//	Pd(chip_1_size);
	Pd(search_exponent);
	Pd(data_setup_time);
	Pd(data_hold_time);
	Pd(address_setup_time);
	Pd(data_sample_time);
	Pd(row_address_size);
	Pd(column_address_size);
	Pd(read_command_code1);
	Pd(read_command_code2);
	Pd(boot_stream_major_version);
	Pd(boot_stream_minor_version);
	Pd(boot_stream_sub_version);
	Pd(ncb_version);
	Pd(boot_stream_1_address);
	Pd(boot_stream_2_address);
#undef Pd
#undef Ps
}

