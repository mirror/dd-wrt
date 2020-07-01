/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 */

#ifndef _LIBEXFAT_H

#include <stdbool.h>
#include <sys/types.h>
#include <wchar.h>
#include <limits.h>

#define KB			(1024)
#define MB			(1024*1024)
#define GB			(1024UL*1024UL*1024UL)

#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))

#define MIN(a, b)	((a) < (b) ? (a) : (b))
#define MAX(a, b)	((a) > (b) ? (a) : (b))

#define DIV_ROUND_UP(__i, __d)	(((__i) + (__d) - 1) / (__d))

#define EXFAT_MIN_NUM_SEC_VOL		(2048)
#define EXFAT_MAX_NUM_SEC_VOL		((2 << 64) - 1)

#define EXFAT_MAX_NUM_CLUSTER		(0xFFFFFFF5)

#define DEFAULT_SECTOR_SIZE	(512)

#define VOLUME_LABEL_BUFFER_SIZE	(VOLUME_LABEL_MAX_LEN*MB_LEN_MAX+1)

/* Upcase tabel macro */
#define EXFAT_UPCASE_TABLE_SIZE		(5836)

enum {
	BOOT_SEC_IDX = 0,
	EXBOOT_SEC_IDX,
	EXBOOT_SEC_NUM = 8,
	OEM_SEC_IDX,
	RESERVED_SEC_IDX,
	CHECKSUM_SEC_IDX,
	BACKUP_BOOT_SEC_IDX,
};

struct exfat_blk_dev {
	int dev_fd;
	unsigned long long size;
	unsigned int sector_size;
	unsigned int sector_size_bits;
	unsigned long long num_sectors;
	unsigned int num_clusters;
};

struct exfat_user_input {
	char dev_name[255];
	bool writeable;
	unsigned int cluster_size;
	unsigned int sec_per_clu;
	bool quick;
	__u16 volume_label[VOLUME_LABEL_MAX_LEN];
	int volume_label_len;
};

void show_version(void);

void exfat_set_bit(struct exfat_blk_dev *bd, char *bitmap,
		unsigned int clu);
void exfat_clear_bit(struct exfat_blk_dev *bd, char *bitmap,
		unsigned int clu);
wchar_t exfat_bad_char(wchar_t w);
void boot_calc_checksum(unsigned char *sector, unsigned short size,
		bool is_boot_sec, __le32 *checksum);
void init_user_input(struct exfat_user_input *ui);
int exfat_get_blk_dev_info(struct exfat_user_input *ui,
		struct exfat_blk_dev *bd);
ssize_t exfat_read(int fd, void *buf, size_t size, off_t offset);
ssize_t exfat_write(int fd, void *buf, size_t size, off_t offset);

size_t exfat_utf16_len(const __le16 *str, size_t max_size);
ssize_t exfat_utf16_enc(const char *in_str, __u16 *out_str, size_t out_size);
ssize_t exfat_utf16_dec(const __u16 *in_str, size_t in_len,
			char *out_str, size_t out_size);

/*
 * Exfat Print
 */

extern unsigned int print_level;

#define EXFAT_ERROR	(1)
#define EXFAT_INFO	(2)
#define EXFAT_DEBUG	(3)

#define exfat_msg(level, dir, fmt, ...)					\
	do {								\
		if (print_level >= level) {				\
			fprintf(dir, fmt, ##__VA_ARGS__);		\
		}							\
	} while (0)							\

#define exfat_err(fmt, ...)	exfat_msg(EXFAT_ERROR, stderr,		\
					fmt, ##__VA_ARGS__)
#define exfat_info(fmt, ...)	exfat_msg(EXFAT_INFO, stdout,		\
					fmt, ##__VA_ARGS__)
#define exfat_debug(fmt, ...)	exfat_msg(EXFAT_DEBUG, stdout,		\
					"[%s:%4d] " fmt, __func__, 	\
					__LINE__, ##__VA_ARGS__)

#endif /* !_LIBEXFAT_H */
