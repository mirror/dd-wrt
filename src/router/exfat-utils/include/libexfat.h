/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  Copyright (C) 2019 Namjae Jeon <linkinjeon@kernel.org>
 */

#ifndef _LIBEXFAT_H

#include <stdbool.h>
#include <sys/types.h>
#include <wchar.h>
#include <limits.h>

typedef __u32 clus_t;

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

#define DEFAULT_BOUNDARY_ALIGNMENT	(1024*1024)

#define DEFAULT_SECTOR_SIZE	(512)

#define VOLUME_LABEL_BUFFER_SIZE	(VOLUME_LABEL_MAX_LEN*MB_LEN_MAX+1)

/* Upcase table macro */
#define EXFAT_UPCASE_TABLE_CHARS	(0x10000)
#define EXFAT_UPCASE_TABLE_SIZE		(5836)

/* Flags for tune.exfat and exfatlabel */
#define EXFAT_GET_VOLUME_LABEL		0x01
#define EXFAT_SET_VOLUME_LABEL		0x02
#define EXFAT_GET_VOLUME_SERIAL		0x03
#define EXFAT_SET_VOLUME_SERIAL		0x04
#define EXFAT_GET_VOLUME_GUID		0x05
#define EXFAT_SET_VOLUME_GUID		0x06

#define EXFAT_MAX_SECTOR_SIZE		4096

#define EXFAT_CLUSTER_SIZE(pbr) (1 << ((pbr)->bsx.sect_size_bits +	\
					(pbr)->bsx.sect_per_clus_bits))
#define EXFAT_SECTOR_SIZE(pbr) (1 << (pbr)->bsx.sect_size_bits)

#define EXFAT_MAX_HASH_COUNT		(UINT16_MAX + 1)

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
	unsigned long long offset;
	unsigned long long size;
	unsigned int sector_size;
	unsigned int sector_size_bits;
	unsigned long long num_sectors;
	unsigned int num_clusters;
	unsigned int cluster_size;
};

struct exfat_user_input {
	char dev_name[255];
	bool writeable;
	unsigned int sector_size;
	unsigned int cluster_size;
	unsigned int sec_per_clu;
	unsigned int boundary_align;
	bool pack_bitmap;
	bool quick;
	__u16 volume_label[VOLUME_LABEL_MAX_LEN];
	int volume_label_len;
	unsigned int volume_serial;
	const char *guid;
};

struct exfat;
struct exfat_inode;

#ifdef WORDS_BIGENDIAN
typedef __u8	bitmap_t;
#else
typedef __u32	bitmap_t;
#endif

#define BITS_PER	(sizeof(bitmap_t) * 8)
#define BIT_MASK(__c)	(1 << ((__c) % BITS_PER))
#define BIT_ENTRY(__c)	((__c) / BITS_PER)

#define EXFAT_BITMAP_SIZE(__c_count)	\
	(DIV_ROUND_UP(__c_count, BITS_PER) * sizeof(bitmap_t))

#define BITMAP_GET(bmap, bit)	\
	(((bitmap_t *)(bmap))[BIT_ENTRY(bit)] & BIT_MASK(bit))

#define BITMAP_SET(bmap, bit)	\
	(((bitmap_t *)(bmap))[BIT_ENTRY(bit)] |= BIT_MASK(bit))

#define BITMAP_CLEAR(bmap, bit)	\
	(((bitmap_t *)(bmap))[BIT_ENTRY(bit)] &= ~BIT_MASK(bit))

static inline bool exfat_bitmap_get(char *bmap, clus_t c)
{
	clus_t cc = c - EXFAT_FIRST_CLUSTER;

	return BITMAP_GET(bmap, cc);
}

static inline void exfat_bitmap_set(char *bmap, clus_t c)
{
	clus_t cc = c - EXFAT_FIRST_CLUSTER;

	BITMAP_SET(bmap, cc);
}

static inline void exfat_bitmap_clear(char *bmap, clus_t c)
{
	clus_t cc = c - EXFAT_FIRST_CLUSTER;
	(((bitmap_t *)(bmap))[BIT_ENTRY(cc)] &= ~BIT_MASK(cc));
}

void exfat_bitmap_set_range(struct exfat *exfat, char *bitmap,
			    clus_t start_clus, clus_t count);
int exfat_bitmap_find_zero(struct exfat *exfat, char *bmap,
			   clus_t start_clu, clus_t *next);
int exfat_bitmap_find_one(struct exfat *exfat, char *bmap,
			  clus_t start_clu, clus_t *next);

void show_version(void);

wchar_t exfat_bad_char(wchar_t w);
void boot_calc_checksum(unsigned char *sector, unsigned short size,
		bool is_boot_sec, __le32 *checksum);
void init_user_input(struct exfat_user_input *ui);
int exfat_get_blk_dev_info(struct exfat_user_input *ui,
		struct exfat_blk_dev *bd);
ssize_t exfat_read(int fd, void *buf, size_t size, off_t offset);
ssize_t exfat_write(int fd, void *buf, size_t size, off_t offset);
ssize_t exfat_write_zero(int fd, size_t size, off_t offset);

size_t exfat_utf16_len(const __le16 *str, size_t max_size);
ssize_t exfat_utf16_enc(const char *in_str, __u16 *out_str, size_t out_size);
ssize_t exfat_utf16_dec(const __u16 *in_str, size_t in_len,
			char *out_str, size_t out_size);
off_t exfat_get_root_entry_offset(struct exfat_blk_dev *bd);
int exfat_read_volume_label(struct exfat *exfat);
int exfat_set_volume_label(struct exfat *exfat, char *label_input);
int __exfat_set_volume_guid(struct exfat_dentry *dentry, const char *guid);
int exfat_read_volume_guid(struct exfat *exfat);
int exfat_set_volume_guid(struct exfat *exfat, const char *guid);
int exfat_read_sector(struct exfat_blk_dev *bd, void *buf,
		unsigned int sec_off);
int exfat_write_sector(struct exfat_blk_dev *bd, void *buf,
		unsigned int sec_off);
int exfat_write_checksum_sector(struct exfat_blk_dev *bd,
		unsigned int checksum, bool is_backup);
char *exfat_conv_volume_label(struct exfat_dentry *vol_entry);
int exfat_show_volume_serial(int fd);
int exfat_set_volume_serial(struct exfat_blk_dev *bd,
		struct exfat_user_input *ui);
unsigned int exfat_clus_to_blk_dev_off(struct exfat_blk_dev *bd,
		unsigned int clu_off, unsigned int clu);
int exfat_get_next_clus(struct exfat *exfat, clus_t clus, clus_t *next);
int exfat_get_inode_next_clus(struct exfat *exfat, struct exfat_inode *node,
			      clus_t clus, clus_t *next);
int exfat_set_fat(struct exfat *exfat, clus_t clus, clus_t next_clus);
off_t exfat_s2o(struct exfat *exfat, off_t sect);
off_t exfat_c2o(struct exfat *exfat, unsigned int clus);
int exfat_o2c(struct exfat *exfat, off_t device_offset,
	      unsigned int *clu, unsigned int *offset);
bool exfat_heap_clus(struct exfat *exfat, clus_t clus);
int exfat_root_clus_count(struct exfat *exfat);
int read_boot_sect(struct exfat_blk_dev *bdev, struct pbr **bs);
int exfat_parse_ulong(const char *s, unsigned long *out);

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
