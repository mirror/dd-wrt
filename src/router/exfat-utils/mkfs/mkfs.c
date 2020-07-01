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

	/* Fill exfat BIOS paramemter block */
	pbpb->jmp_boot[0] = 0xeb;
	pbpb->jmp_boot[1] = 0x76;
	pbpb->jmp_boot[2] = 0x90;
	memcpy(pbpb->oem_name, "EXFAT   ", 8);
	memset(pbpb->res_zero, 0, 53);

	/* Fill exfat extend BIOS paramemter block */
	pbsx->vol_offset = 0;
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
	memset(pbsx->reserved2, 0, 7);

	memset(ppbr->boot_code, 0, 390);
	ppbr->signature = cpu_to_le16(PBR_SIGNATURE);

	exfat_debug("Volume Length(sectors) : %" PRIu64 "\n",
		le64_to_cpu(pbsx->vol_length));
	exfat_debug("FAT Offset(sector offset) : %u\n",
		le32_to_cpu(pbsx->fat_offset));
	exfat_debug("FAT Length(sectors) : %u\n",
		le32_to_cpu(pbsx->fat_length));
	exfat_debug("Cluster Heap Offset (sector offset) : %u\n",
		le32_to_cpu(pbsx->clu_offset));
	exfat_debug("Cluster Count (sectors) : %u\n",
		le32_to_cpu(pbsx->clu_count));
	exfat_debug("Root Cluster (cluster offset) : %u\n",
		le32_to_cpu(pbsx->root_cluster));
	exfat_debug("Sector Size Bits : %u\n",
		pbsx->sect_size_bits);
	exfat_debug("Sector per Cluster bits : %u\n",
		pbsx->sect_per_clus_bits);
}

static int exfat_write_sector(struct exfat_blk_dev *bd, void *buf,
		unsigned int sec_off)
{
	int bytes;
	unsigned long long offset = sec_off * bd->sector_size;

	lseek(bd->dev_fd, offset, SEEK_SET);
	bytes = write(bd->dev_fd, buf, bd->sector_size);
	if (bytes != (int)bd->sector_size) {
		exfat_err("write failed, sec_off : %u, bytes : %d\n", sec_off,
			bytes);
		return -1;
	}
	return 0;
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

	ppbr = malloc(sizeof(struct pbr));
	if (!ppbr) {
		exfat_err("Cannot allocate pbr: out of memory\n");
		return -1;
	}
	memset(ppbr, 0, sizeof(struct pbr));

	exfat_setup_boot_sector(ppbr, bd, ui);

	/* write main boot sector */
	ret = exfat_write_sector(bd, ppbr, sec_idx);
	if (ret < 0) {
		exfat_err("main boot sector write failed\n");
		ret = -1;
		goto free_ppbr;
	}

	boot_calc_checksum((unsigned char *)ppbr, sizeof(struct pbr),
		true, checksum);

free_ppbr:
	free(ppbr);
	return ret;
}

static int exfat_write_extended_boot_sectors(struct exfat_blk_dev *bd,
		unsigned int *checksum, bool is_backup)
{
	struct exbs eb;
	int i;
	unsigned int sec_idx = EXBOOT_SEC_IDX;

	if (is_backup)
		sec_idx += BACKUP_BOOT_SEC_IDX;

	memset(&eb, 0, sizeof(struct exbs));
	eb.signature = cpu_to_le16(PBR_SIGNATURE);
	for (i = 0; i < EXBOOT_SEC_NUM; i++) {
		if (exfat_write_sector(bd, &eb, sec_idx++)) {
			exfat_err("extended boot sector write failed\n");
			return -1;
		}

		boot_calc_checksum((unsigned char *) &eb, sizeof(struct exbs),
			false, checksum);
	}

	return 0;
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

static int exfat_write_checksum_sector(struct exfat_blk_dev *bd,
		unsigned int checksum, bool is_backup)
{
	__le32 *checksum_buf;
	int ret = 0;
	unsigned int i;
	unsigned int sec_idx = CHECKSUM_SEC_IDX;

	checksum_buf = malloc(bd->sector_size);
	if (!checksum_buf)
		return -1;

	if (is_backup)
		sec_idx += BACKUP_BOOT_SEC_IDX;

	for (i = 0; i < bd->sector_size / sizeof(int); i++)
		checksum_buf[i] = cpu_to_le32(checksum);

	ret = exfat_write_sector(bd, checksum_buf, sec_idx);
	if (ret) {
		exfat_err("checksum sector write failed\n");
		goto free;
	}

free:
	free(checksum_buf);
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

	lseek(fd, finfo.fat_byte_off + (offset * sizeof(__le32)), SEEK_SET);
	nbyte = write(fd, (__u8 *) &clu, sizeof(__le32));
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

	bitmap = calloc(finfo.bitmap_byte_len, sizeof(*bitmap));
	if (!bitmap)
		return -1;

	for (i = 0; i < finfo.used_clu_cnt - EXFAT_FIRST_CLUSTER; i++)
		exfat_set_bit(bd, bitmap, i);

	lseek(bd->dev_fd, finfo.bitmap_byte_off, SEEK_SET);
	nbytes = write(bd->dev_fd, bitmap, finfo.bitmap_byte_len);
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
	struct exfat_dentry ed[3];
	int dentries_len = sizeof(struct exfat_dentry) * 3;
	int nbytes;

	/* Set volume label entry */
	ed[0].type = EXFAT_VOLUME;
	memset(ed[0].vol_label, 0, 22);
	memcpy(ed[0].vol_label, ui->volume_label, ui->volume_label_len);
	ed[0].vol_char_cnt = ui->volume_label_len/2;

	/* Set bitmap entry */
	ed[1].type = EXFAT_BITMAP;
	ed[1].bitmap_flags = 0;
	ed[1].bitmap_start_clu = cpu_to_le32(EXFAT_FIRST_CLUSTER);
	ed[1].bitmap_size = cpu_to_le64(finfo.bitmap_byte_len);

	/* Set upcase table entry */
	ed[2].type = EXFAT_UPCASE;
	ed[2].upcase_checksum = cpu_to_le32(0xe619d30d);
	ed[2].upcase_start_clu = cpu_to_le32(finfo.ut_start_clu);
	ed[2].upcase_size = cpu_to_le64(EXFAT_UPCASE_TABLE_SIZE);

	lseek(bd->dev_fd, finfo.root_byte_off, SEEK_SET);
	nbytes = write(bd->dev_fd, ed, dentries_len);
	if (nbytes != dentries_len) {
		exfat_err("write failed, nbytes : %d, dentries_len : %d\n",
			nbytes, dentries_len);
		return -1;
	}

	return 0;
}

static void usage(void)
{
	fprintf(stderr, "Usage: mkfs.exfat\n");
	fprintf(stderr, "\t-L | --volume-label=label                              Set volume label\n");
	fprintf(stderr, "\t-c | --cluster-size=size(or suffixed by 'K' or 'M')    Specify cluster size\n");
	fprintf(stderr, "\t-f | --full-format                                     Full format\n");
	fprintf(stderr, "\t-V | --version                                         Show version\n");
	fprintf(stderr, "\t-v | --verbose                                         Print debug\n");
	fprintf(stderr, "\t-h | --help                                            Show help\n");

	exit(EXIT_FAILURE);
}

static struct option opts[] = {
	{"volume-label",	required_argument,	NULL,	'L' },
	{"cluster-size",	required_argument,	NULL,	'c' },
	{"full-format",		no_argument,		NULL,	'f' },
	{"version",		no_argument,		NULL,	'V' },
	{"verbose",		no_argument,		NULL,	'v' },
	{"help",		no_argument,		NULL,	'h' },
	{"?",			no_argument,		NULL,	'?' },
	{NULL,			0,			NULL,	 0  }
};

static int exfat_build_mkfs_info(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui)
{
	if (ui->cluster_size > DEFAULT_BOUNDARY_ALIGNMENT)
		finfo.fat_byte_off = ui->cluster_size;
	else
		finfo.fat_byte_off = DEFAULT_BOUNDARY_ALIGNMENT;
	finfo.fat_byte_len = round_up((bd->num_clusters * sizeof(int)),
		ui->cluster_size);
	finfo.clu_byte_off = round_up(finfo.fat_byte_off + finfo.fat_byte_len,
		DEFAULT_BOUNDARY_ALIGNMENT);
	finfo.total_clu_cnt = (bd->size - finfo.clu_byte_off) /
		ui->cluster_size;
	if (finfo.total_clu_cnt > EXFAT_MAX_NUM_CLUSTER) {
		exfat_err("cluster size is too small\n");
		return -1;
	}

	finfo.bitmap_byte_off = finfo.clu_byte_off;
	finfo.bitmap_byte_len = round_up(finfo.total_clu_cnt, 8) / 8;
	finfo.ut_start_clu = round_up(EXFAT_REVERVED_CLUSTERS *
		ui->cluster_size + finfo.bitmap_byte_len, ui->cluster_size) /
		ui->cluster_size;
	finfo.ut_byte_off = round_up(finfo.bitmap_byte_off +
		finfo.bitmap_byte_len, ui->cluster_size);
	finfo.ut_byte_len = EXFAT_UPCASE_TABLE_SIZE;
	finfo.root_start_clu = round_up(finfo.ut_start_clu * ui->cluster_size
		+ finfo.ut_byte_len, ui->cluster_size) / ui->cluster_size;
	finfo.root_byte_off = round_up(finfo.ut_byte_off + finfo.ut_byte_len,
		ui->cluster_size);
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

static long long parse_cluster_size(const char *size)
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
	default:
		exfat_err("Wrong unit input('%c') for cluster size\n",
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

	init_user_input(&ui);

	if (!setlocale(LC_CTYPE, ""))
		exfat_err("failed to init locale/codeset\n");

	opterr = 0;
	while ((c = getopt_long(argc, argv, "n:L:c:fVvh", opts, NULL)) != EOF)
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
		case 'c':
			ret = parse_cluster_size(optarg);
			if (ret < 0)
				goto out;
			else if (ret > EXFAT_MAX_CLUSTER_SIZE) {
				exfat_err("cluster size(%d) exceeds max cluster size(%d)\n",
					ui.cluster_size, EXFAT_MAX_CLUSTER_SIZE);
				goto out;
			}
			ui.cluster_size = ret;
			break;
		case 'f':
			ui.quick = false;
			break;
		case 'V':
			version_only = true;
			break;
		case 'v':
			print_level = EXFAT_DEBUG;
			break;
		case '?':
		case 'h':
		default:
			usage();
	}

	show_version();
	if (version_only)
		exit(EXIT_FAILURE);

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
		exfat_info("\nexFAT format fail!\n");
	return ret;
}
