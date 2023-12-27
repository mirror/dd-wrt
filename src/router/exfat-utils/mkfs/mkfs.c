// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <locale.h>
#include <time.h>

#include "exfat_ondisk.h"
#include "libexfat.h"
#include "mkfs.h"

struct exfat_mkfs_info finfo;

/* random serial generator based on current time */
static unsigned int get_new_serial(void)
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts)) {
		/* set 0000-0000 on error */
		ts.tv_sec = 0;
		ts.tv_nsec = 0;
	}

	return (unsigned int)(ts.tv_nsec << 12 | ts.tv_sec);
}

static void exfat_setup_boot_sector(struct pbr *ppbr,
		struct exfat_blk_dev *bd, struct exfat_user_input *ui)
{
	struct bpb64 *pbpb = &ppbr->bpb;
	struct bsx64 *pbsx = &ppbr->bsx;
	unsigned int i;

	/* Fill exfat BIOS parameter block */
	pbpb->jmp_boot[0] = 0xeb;
	pbpb->jmp_boot[1] = 0x76;
	pbpb->jmp_boot[2] = 0x90;
	memcpy(pbpb->oem_name, "EXFAT   ", 8);
	memset(pbpb->res_zero, 0, 53);

	/* Fill exfat extend BIOS parameter block */
	pbsx->vol_offset = cpu_to_le64(bd->offset / bd->sector_size);
	pbsx->vol_length = cpu_to_le64(bd->size / bd->sector_size);
	pbsx->fat_offset = cpu_to_le32(finfo.fat_byte_off / bd->sector_size);
	pbsx->fat_length = cpu_to_le32(finfo.fat_byte_len / bd->sector_size);
	pbsx->clu_offset = cpu_to_le32(finfo.clu_byte_off / bd->sector_size);
	pbsx->clu_count = cpu_to_le32(finfo.total_clu_cnt);
	pbsx->root_cluster = cpu_to_le32(finfo.root_start_clu);
	pbsx->vol_serial = cpu_to_le32(finfo.volume_serial);
	pbsx->vol_flags = 0;
	pbsx->sect_size_bits = bd->sector_size_bits;
	pbsx->sect_per_clus_bits = 0;
	/* Compute base 2 logarithm of ui->cluster_size / bd->sector_size */
	for (i = ui->cluster_size / bd->sector_size; i > 1; i /= 2)
		pbsx->sect_per_clus_bits++;
	pbsx->num_fats = 1;
	/* fs_version[0] : minor and fs_version[1] : major */
	pbsx->fs_version[0] = 0;
	pbsx->fs_version[1] = 1;
	pbsx->phy_drv_no = 0x80;
	memset(pbsx->reserved2, 0, 7);

	memset(ppbr->boot_code, 0, 390);
	ppbr->signature = cpu_to_le16(PBR_SIGNATURE);

	exfat_debug("Volume Offset(sectors) : %" PRIu64 "\n",
		le64_to_cpu(pbsx->vol_offset));
	exfat_debug("Volume Length(sectors) : %" PRIu64 "\n",
		le64_to_cpu(pbsx->vol_length));
	exfat_debug("FAT Offset(sector offset) : %u\n",
		le32_to_cpu(pbsx->fat_offset));
	exfat_debug("FAT Length(sectors) : %u\n",
		le32_to_cpu(pbsx->fat_length));
	exfat_debug("Cluster Heap Offset (sector offset) : %u\n",
		le32_to_cpu(pbsx->clu_offset));
	exfat_debug("Cluster Count : %u\n",
		le32_to_cpu(pbsx->clu_count));
	exfat_debug("Root Cluster (cluster offset) : %u\n",
		le32_to_cpu(pbsx->root_cluster));
	exfat_debug("Volume Serial : 0x%x\n", le32_to_cpu(pbsx->vol_serial));
	exfat_debug("Sector Size Bits : %u\n",
		pbsx->sect_size_bits);
	exfat_debug("Sector per Cluster bits : %u\n",
		pbsx->sect_per_clus_bits);
}

static int exfat_write_boot_sector(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui, unsigned int *checksum,
		bool is_backup)
{
	struct pbr *ppbr;
	unsigned int sec_idx = BOOT_SEC_IDX;
	int ret = 0;

	if (is_backup)
		sec_idx += BACKUP_BOOT_SEC_IDX;

	ppbr = malloc(bd->sector_size);
	if (!ppbr) {
		exfat_err("Cannot allocate pbr: out of memory\n");
		return -1;
	}
	memset(ppbr, 0, bd->sector_size);

	exfat_setup_boot_sector(ppbr, bd, ui);

	/* write main boot sector */
	ret = exfat_write_sector(bd, ppbr, sec_idx);
	if (ret < 0) {
		exfat_err("main boot sector write failed\n");
		ret = -1;
		goto free_ppbr;
	}

	boot_calc_checksum((unsigned char *)ppbr, bd->sector_size,
		true, checksum);

free_ppbr:
	free(ppbr);
	return ret;
}

static int exfat_write_extended_boot_sectors(struct exfat_blk_dev *bd,
		unsigned int *checksum, bool is_backup)
{
	char *peb;
	__le16 *peb_signature;
	int ret = 0;
	int i;
	unsigned int sec_idx = EXBOOT_SEC_IDX;

	peb = malloc(bd->sector_size);
	if (!peb)
		return -1;

	if (is_backup)
		sec_idx += BACKUP_BOOT_SEC_IDX;

	memset(peb, 0, bd->sector_size);
	peb_signature = (__le16*) (peb + bd->sector_size - 2);
	*peb_signature = cpu_to_le16(PBR_SIGNATURE);
	for (i = 0; i < EXBOOT_SEC_NUM; i++) {
		if (exfat_write_sector(bd, peb, sec_idx++)) {
			exfat_err("extended boot sector write failed\n");
			ret = -1;
			goto free_peb;
		}

		boot_calc_checksum((unsigned char *) peb, bd->sector_size,
			false, checksum);
	}

free_peb:
	free(peb);
	return ret;
}

static int exfat_write_oem_sector(struct exfat_blk_dev *bd,
		unsigned int *checksum, bool is_backup)
{
	char *oem;
	int ret = 0;
	unsigned int sec_idx = OEM_SEC_IDX;

	oem = malloc(bd->sector_size);
	if (!oem)
		return -1;

	if (is_backup)
		sec_idx += BACKUP_BOOT_SEC_IDX;

	memset(oem, 0xFF, bd->sector_size);
	ret = exfat_write_sector(bd, oem, sec_idx);
	if (ret) {
		exfat_err("oem sector write failed\n");
		ret = -1;
		goto free_oem;
	}

	boot_calc_checksum((unsigned char *)oem, bd->sector_size, false,
		checksum);

	/* Zero out reserved sector */
	memset(oem, 0, bd->sector_size);
	ret = exfat_write_sector(bd, oem, sec_idx + 1);
	if (ret) {
		exfat_err("reserved sector write failed\n");
		ret = -1;
		goto free_oem;
	}

	boot_calc_checksum((unsigned char *)oem, bd->sector_size, false,
		checksum);

free_oem:
	free(oem);
	return ret;
}

static int exfat_create_volume_boot_record(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui, bool is_backup)
{
	unsigned int checksum = 0;
	int ret;

	ret = exfat_write_boot_sector(bd, ui, &checksum, is_backup);
	if (ret)
		return ret;
	ret = exfat_write_extended_boot_sectors(bd, &checksum, is_backup);
	if (ret)
		return ret;
	ret = exfat_write_oem_sector(bd, &checksum, is_backup);
	if (ret)
		return ret;

	return exfat_write_checksum_sector(bd, checksum, is_backup);
}

static int write_fat_entry(int fd, __le32 clu,
		unsigned long long offset)
{
	int nbyte;
	off_t fat_entry_offset = finfo.fat_byte_off + (offset * sizeof(__le32));

	nbyte = pwrite(fd, (__u8 *) &clu, sizeof(__le32), fat_entry_offset);
	if (nbyte != sizeof(int)) {
		exfat_err("write failed, offset : %llu, clu : %x\n",
			offset, clu);
		return -1;
	}

	return 0;
}

static int write_fat_entries(struct exfat_user_input *ui, int fd,
		unsigned int clu, unsigned int length)
{
	int ret;
	unsigned int count;

	count = clu + round_up(length, ui->cluster_size) / ui->cluster_size;

	for (; clu < count - 1; clu++) {
		ret = write_fat_entry(fd, cpu_to_le32(clu + 1), clu);
		if (ret)
			return ret;
	}

	ret = write_fat_entry(fd, cpu_to_le32(EXFAT_EOF_CLUSTER), clu);
	if (ret)
		return ret;

	return clu;
}

static int exfat_create_fat_table(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui)
{
	int ret, clu;

	/* fat entry 0 should be media type field(0xF8) */
	ret = write_fat_entry(bd->dev_fd, cpu_to_le32(0xfffffff8), 0);
	if (ret) {
		exfat_err("fat 0 entry write failed\n");
		return ret;
	}

	/* fat entry 1 is historical precedence(0xFFFFFFFF) */
	ret = write_fat_entry(bd->dev_fd, cpu_to_le32(0xffffffff), 1);
	if (ret) {
		exfat_err("fat 1 entry write failed\n");
		return ret;
	}

	/* write bitmap entries */
	clu = write_fat_entries(ui, bd->dev_fd, EXFAT_FIRST_CLUSTER,
		finfo.bitmap_byte_len);
	if (clu < 0)
		return ret;

	/* write upcase table entries */
	clu = write_fat_entries(ui, bd->dev_fd, clu + 1, finfo.ut_byte_len);
	if (clu < 0)
		return ret;

	/* write root directory entries */
	clu = write_fat_entries(ui, bd->dev_fd, clu + 1, finfo.root_byte_len);
	if (clu < 0)
		return ret;

	finfo.used_clu_cnt = clu + 1;
	exfat_debug("Total used cluster count : %d\n", finfo.used_clu_cnt);

	return ret;
}

static int exfat_create_bitmap(struct exfat_blk_dev *bd)
{
	char *bitmap;
	unsigned int i, nbytes;

	bitmap = calloc(round_up(finfo.bitmap_byte_len, sizeof(bitmap_t)),
			sizeof(*bitmap));
	if (!bitmap)
		return -1;

	for (i = EXFAT_FIRST_CLUSTER; i < finfo.used_clu_cnt; i++)
		exfat_bitmap_set(bitmap, i);

	nbytes = pwrite(bd->dev_fd, bitmap, finfo.bitmap_byte_len, finfo.bitmap_byte_off);
	if (nbytes != finfo.bitmap_byte_len) {
		exfat_err("write failed, nbytes : %d, bitmap_len : %d\n",
			nbytes, finfo.bitmap_byte_len);
		free(bitmap);
		return -1;
	}

	free(bitmap);
	return 0;
}

static int exfat_create_root_dir(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui)
{
	struct exfat_dentry ed[4] = {0};
	int dentries_len = sizeof(ed);
	int nbytes;

	/* Set volume label entry */
	ed[0].type = EXFAT_VOLUME;
	memset(ed[0].vol_label, 0, 22);
	memcpy(ed[0].vol_label, ui->volume_label, ui->volume_label_len);
	ed[0].vol_char_cnt = ui->volume_label_len/2;

	/* Set volume GUID entry */
	if (ui->guid) {
		if (__exfat_set_volume_guid(&ed[1], ui->guid))
			return -1;
	} else {
		/*
		 * Since a single empty entry cannot be allocated for a
		 * file, this can reserve the entry for volume GUID.
		 */
		ed[1].type = EXFAT_GUID & ~EXFAT_INVAL;
	}

	/* Set bitmap entry */
	ed[2].type = EXFAT_BITMAP;
	ed[2].bitmap_flags = 0;
	ed[2].bitmap_start_clu = cpu_to_le32(EXFAT_FIRST_CLUSTER);
	ed[2].bitmap_size = cpu_to_le64(finfo.bitmap_byte_len);

	/* Set upcase table entry */
	ed[3].type = EXFAT_UPCASE;
	ed[3].upcase_checksum = cpu_to_le32(0xe619d30d);
	ed[3].upcase_start_clu = cpu_to_le32(finfo.ut_start_clu);
	ed[3].upcase_size = cpu_to_le64(EXFAT_UPCASE_TABLE_SIZE);

	nbytes = pwrite(bd->dev_fd, ed, dentries_len, finfo.root_byte_off);
	if (nbytes != dentries_len) {
		exfat_err("write failed, nbytes : %d, dentries_len : %d\n",
			nbytes, dentries_len);
		return -1;
	}

	return 0;
}

static void usage(void)
{
	fputs("Usage: mkfs.exfat\n"
		"\t-L | --volume-label=label                              Set volume label\n"
		"\t-U | --volume-guid=guid                                Set volume GUID\n"
		"\t-c | --cluster-size=size(or suffixed by 'K' or 'M')    Specify cluster size\n"
		"\t-b | --boundary-align=size(or suffixed by 'K' or 'M')  Specify boundary alignment\n"
		"\t     --pack-bitmap                                     Move bitmap into FAT segment\n"
		"\t-f | --full-format                                     Full format\n"
		"\t-V | --version                                         Show version\n"
		"\t-q | --quiet                                           Print only errors\n"
		"\t-v | --verbose                                         Print debug\n"
		"\t-h | --help                                            Show help\n",
		stderr);

	exit(EXIT_FAILURE);
}

#define PACK_BITMAP (CHAR_MAX + 1)

static const struct option opts[] = {
	{"volume-label",	required_argument,	NULL,	'L' },
	{"volume-guid",		required_argument,	NULL,	'U' },
	{"cluster-size",	required_argument,	NULL,	'c' },
	{"boundary-align",	required_argument,	NULL,	'b' },
	{"pack-bitmap",		no_argument,		NULL,	PACK_BITMAP },
	{"full-format",		no_argument,		NULL,	'f' },
	{"version",		no_argument,		NULL,	'V' },
	{"quiet",		no_argument,		NULL,	'q' },
	{"verbose",		no_argument,		NULL,	'v' },
	{"help",		no_argument,		NULL,	'h' },
	{"?",			no_argument,		NULL,	'?' },
	{NULL,			0,			NULL,	 0  }
};

/*
 * Moves the bitmap to just before the alignment boundary if there is space
 * between the boundary and the end of the FAT. This may allow the FAT and the
 * bitmap to share the same allocation unit on flash media, thereby improving
 * performance and endurance.
 */
static int exfat_pack_bitmap(const struct exfat_user_input *ui)
{
	unsigned int fat_byte_end = finfo.fat_byte_off + finfo.fat_byte_len,
		bitmap_byte_len = finfo.bitmap_byte_len,
		bitmap_clu_len = round_up(bitmap_byte_len, ui->cluster_size),
		bitmap_clu_cnt, total_clu_cnt, new_bitmap_clu_len;

	for (;;) {
		bitmap_clu_cnt = bitmap_clu_len / ui->cluster_size;
		if (finfo.clu_byte_off - bitmap_clu_len < fat_byte_end ||
				finfo.total_clu_cnt > EXFAT_MAX_NUM_CLUSTER -
					bitmap_clu_cnt)
			return -1;
		total_clu_cnt = finfo.total_clu_cnt + bitmap_clu_cnt;
		bitmap_byte_len = round_up(total_clu_cnt, 8) / 8;
		new_bitmap_clu_len = round_up(bitmap_byte_len, ui->cluster_size);
		if (new_bitmap_clu_len == bitmap_clu_len) {
			finfo.clu_byte_off -= bitmap_clu_len;
			finfo.total_clu_cnt = total_clu_cnt;
			finfo.bitmap_byte_off -= bitmap_clu_len;
			finfo.bitmap_byte_len = bitmap_byte_len;
			return 0;
		}
		bitmap_clu_len = new_bitmap_clu_len;
	}
}

static int exfat_build_mkfs_info(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui)
{
	unsigned long long total_clu_cnt;
	int clu_len;

	if (ui->cluster_size < bd->sector_size) {
		exfat_err("cluster size (%u bytes) is smaller than sector size (%u bytes)\n",
			  ui->cluster_size, bd->sector_size);
		return -1;
	}
	if (ui->boundary_align < bd->sector_size) {
		exfat_err("boundary alignment is too small (min %d)\n",
				bd->sector_size);
		return -1;
	}
	finfo.fat_byte_off = round_up(bd->offset + 24 * bd->sector_size,
			ui->boundary_align) - bd->offset;
	/* Prevent integer overflow when computing the FAT length */
	if (bd->num_clusters > UINT32_MAX / 4) {
		exfat_err("cluster size (%u bytes) is too small\n", ui->cluster_size);
		return -1;
	}
	finfo.fat_byte_len = round_up((bd->num_clusters * 4), ui->cluster_size);
	finfo.clu_byte_off = round_up(bd->offset + finfo.fat_byte_off +
		finfo.fat_byte_len, ui->boundary_align) - bd->offset;
	if (bd->size <= finfo.clu_byte_off) {
		exfat_err("boundary alignment is too big\n");
		return -1;
	}
	total_clu_cnt = (bd->size - finfo.clu_byte_off) / ui->cluster_size;
	if (total_clu_cnt > EXFAT_MAX_NUM_CLUSTER) {
		exfat_err("cluster size is too small\n");
		return -1;
	}
	finfo.total_clu_cnt = (unsigned int) total_clu_cnt;

	finfo.bitmap_byte_off = finfo.clu_byte_off;
	finfo.bitmap_byte_len = round_up(finfo.total_clu_cnt, 8) / 8;
	if (ui->pack_bitmap)
		exfat_pack_bitmap(ui);
	clu_len = round_up(finfo.bitmap_byte_len, ui->cluster_size);

	finfo.ut_start_clu = EXFAT_FIRST_CLUSTER + clu_len / ui->cluster_size;
	finfo.ut_byte_off = finfo.bitmap_byte_off + clu_len;
	finfo.ut_byte_len = EXFAT_UPCASE_TABLE_SIZE;
	clu_len = round_up(finfo.ut_byte_len, ui->cluster_size);

	finfo.root_start_clu = finfo.ut_start_clu + clu_len / ui->cluster_size;
	finfo.root_byte_off = finfo.ut_byte_off + clu_len;
	finfo.root_byte_len = sizeof(struct exfat_dentry) * 3;
	finfo.volume_serial = get_new_serial();

	return 0;
}

static int exfat_zero_out_disk(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui)
{
	int nbytes;
	unsigned long long total_written = 0;
	char *buf;
	unsigned int chunk_size = ui->cluster_size;
	unsigned long long size;

	if (ui->quick)
		size = finfo.root_byte_off + chunk_size;
	else
		size = bd->size;

	buf = malloc(chunk_size);
	if (!buf)
		return -1;

	memset(buf, 0, chunk_size);
	lseek(bd->dev_fd, 0, SEEK_SET);
	do {

		nbytes = write(bd->dev_fd, buf, chunk_size);
		if (nbytes <= 0) {
			if (nbytes < 0)
				exfat_err("write failed(errno : %d)\n", errno);
			break;
		}
		total_written += nbytes;
	} while (total_written < size);

	free(buf);
	exfat_debug("zero out written size : %llu, disk size : %llu\n",
		total_written, bd->size);
	return 0;
}

static int make_exfat(struct exfat_blk_dev *bd, struct exfat_user_input *ui)
{
	int ret;

	exfat_info("Creating exFAT filesystem(%s, cluster size=%u)\n\n",
		ui->dev_name, ui->cluster_size);

	exfat_info("Writing volume boot record: ");
	ret = exfat_create_volume_boot_record(bd, ui, 0);
	exfat_info("%s\n", ret ? "failed" : "done");
	if (ret)
		return ret;

	exfat_info("Writing backup volume boot record: ");
	/* backup sector */
	ret = exfat_create_volume_boot_record(bd, ui, 1);
	exfat_info("%s\n", ret ? "failed" : "done");
	if (ret)
		return ret;

	exfat_info("Fat table creation: ");
	ret = exfat_create_fat_table(bd, ui);
	exfat_info("%s\n", ret ? "failed" : "done");
	if (ret)
		return ret;

	exfat_info("Allocation bitmap creation: ");
	ret = exfat_create_bitmap(bd);
	exfat_info("%s\n", ret ? "failed" : "done");
	if (ret)
		return ret;

	exfat_info("Upcase table creation: ");
	ret = exfat_create_upcase_table(bd);
	exfat_info("%s\n", ret ? "failed" : "done");
	if (ret)
		return ret;

	exfat_info("Writing root directory entry: ");
	ret = exfat_create_root_dir(bd, ui);
	exfat_info("%s\n", ret ? "failed" : "done");
	if (ret)
		return ret;

	return 0;
}

static long long parse_size(const char *size)
{
	char *data_unit;
	unsigned long long byte_size = strtoull(size, &data_unit, 0);

	switch (*data_unit) {
	case 'M':
	case 'm':
		byte_size <<= 20;
		break;
	case 'K':
	case 'k':
		byte_size <<= 10;
		break;
	case '\0':
		break;
	default:
		exfat_err("Wrong unit input('%c') for size\n",
				*data_unit);
		return -EINVAL;
	}

	return byte_size;
}

int main(int argc, char *argv[])
{
	int c;
	int ret = EXIT_FAILURE;
	struct exfat_blk_dev bd;
	struct exfat_user_input ui;
	bool version_only = false;
	bool quiet = false;

	init_user_input(&ui);

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "n:L:U:c:b:fVqvh", opts, NULL)) != EOF)
		switch (c) {
		/*
		 * Make 'n' option fallthrough to 'L' option for for backward
		 * compatibility with old utils.
		 */
		case 'n':
		case 'L':
		{
			ret = exfat_utf16_enc(optarg,
				ui.volume_label, sizeof(ui.volume_label));
			if (ret < 0)
				goto out;

			ui.volume_label_len = ret;
			break;
		}
		case 'U':
			if (*optarg != '\0' && *optarg != '\r')
				ui.guid = optarg;
			break;
		case 'c':
			ret = parse_size(optarg);
			if (ret < 0)
				goto out;
			else if (ret & (ret - 1)) {
				exfat_err("cluster size(%d) is not a power of 2)\n",
					ret);
				goto out;
			} else if (ret > EXFAT_MAX_CLUSTER_SIZE) {
				exfat_err("cluster size(%d) exceeds max cluster size(%d)\n",
					ui.cluster_size, EXFAT_MAX_CLUSTER_SIZE);
				goto out;
			}
			ui.cluster_size = ret;
			break;
		case 'b':
			ret = parse_size(optarg);
			if (ret < 0)
				goto out;
			else if (ret & (ret - 1)) {
				exfat_err("boundary align(%d) is not a power of 2)\n",
					ret);
				goto out;
			}
			ui.boundary_align = ret;
			break;
		case PACK_BITMAP:
			ui.pack_bitmap = true;
			break;
		case 'f':
			ui.quick = false;
			break;
		case 'V':
			version_only = true;
			break;
		case 'q':
			print_level = EXFAT_ERROR;
			quiet = true;
			break;
		case 'v':
			print_level = EXFAT_DEBUG;
			break;
		case '?':
		case 'h':
		default:
			usage();
	}

	if (version_only) {
		show_version();
		exit(EXIT_FAILURE);
	} else if (!quiet) {
		show_version();
	}

	if (argc - optind != 1) {
		usage();
	}

	memset(ui.dev_name, 0, sizeof(ui.dev_name));
	snprintf(ui.dev_name, sizeof(ui.dev_name), "%s", argv[optind]);

	ret = exfat_get_blk_dev_info(&ui, &bd);
	if (ret < 0)
		goto out;

	ret = exfat_build_mkfs_info(&bd, &ui);
	if (ret)
		goto close;

	ret = exfat_zero_out_disk(&bd, &ui);
	if (ret)
		goto close;

	ret = make_exfat(&bd, &ui);
	if (ret)
		goto close;

	exfat_info("Synchronizing...\n");
	ret = fsync(bd.dev_fd);
close:
	close(bd.dev_fd);
out:
	if (!ret)
		exfat_info("\nexFAT format complete!\n");
	else
		exfat_err("\nexFAT format fail!\n");
	return ret;
}
