// SPDX-License-Identifier: GPL-2.0-only

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <byteswap.h>
#include <endian.h>
#include <getopt.h>


#if !defined(__BYTE_ORDER)
#error "Unknown byte order"
#endif

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_be16(x)  (x)
#define cpu_to_be32(x)  (x)
#define be16_to_cpu(x)  (x)
#define be32_to_cpu(x)  (x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be16(x)  bswap_16(x)
#define cpu_to_be32(x)  bswap_32(x)
#define be16_to_cpu(x)  bswap_16(x)
#define be32_to_cpu(x)  bswap_32(x)
#else
#error "Unsupported endianness"
#endif


#define FAT_PTR_FLAGS_GET(x)     ((be32_to_cpu(x) & 0xff000000) >> 24)
#define FAT_PTR_FLAGS_SET(x, y)  (x = cpu_to_be32((be32_to_cpu(x) & 0x00ffffff) | ((y & 0x000000ff) << 24)))

#define FAT_PTR_VAL_GET(x)     (be32_to_cpu(x) & 0x00ffffff)
#define FAT_PTR_VAL_SET(x, y)  (x = cpu_to_be32((be32_to_cpu(x) & 0xff000000) | (y & 0x00ffffff)))


struct fat_entry {
	/* first byte contains flags */
	uint32_t previous;

	/* first byte is reserved */
	uint32_t next;
} __attribute__ ((packed));

struct file_entry {
	uint8_t flags;

	uint8_t res0[5];

	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	uint8_t res1[3];

	uint32_t length;

	uint32_t parent_block;
	uint16_t parent_index;

	uint8_t res2[2];

	uint32_t data_block;

	char name[96];
} __attribute__ ((packed));


#define ERASEBLOCK_SIZE   0x10000
#define BLOCK_SIZE        0x400

#define BLOCKS_PER_ERASEBLOCK   (ERASEBLOCK_SIZE / BLOCK_SIZE)
#define FILE_ENTRIES_PER_BLOCK  (BLOCK_SIZE / sizeof(struct file_entry))


#define FLAG_FREE       0x80
#define FLAG_VALID      0x40
#define FLAG_INVALID    0x20
#define FLAG_HIDE       0x10
#define FLAG_DIRECTORY  0x02
#define FLAG_READONLY   0x01


static FILE *f;
static size_t file_size = 0;

static int dir_block = 1;
static int dir_count = 0;

static int next_data_block = 2;


static inline size_t fat_entry_offset(int block) {
	return ERASEBLOCK_SIZE * (block / (BLOCKS_PER_ERASEBLOCK-1))
		+ sizeof(struct fat_entry) * (block % (BLOCKS_PER_ERASEBLOCK-1));
}

static inline size_t block_offset(int block) {
	return ERASEBLOCK_SIZE * (block / (BLOCKS_PER_ERASEBLOCK-1))
		+ BLOCK_SIZE * (1 + (block % (BLOCKS_PER_ERASEBLOCK-1)));
}

static int init_eraseblock(size_t offset) {
	size_t end = offset - (offset % ERASEBLOCK_SIZE) + ERASEBLOCK_SIZE;
	char *fill = "\xff";
	int i;

	while (file_size < end) {
		if (fseek(f, file_size, SEEK_SET)) {
			fprintf(stderr, "failed to seek to end\n");
			return -1;
		}

		for (i = 0; i < ERASEBLOCK_SIZE; i++) {
			if (fwrite(fill, 1, 1, f) != 1) {
				fprintf(stderr, "failed to write eraseblock\n");
				return -1;
			}
		}

		file_size += ERASEBLOCK_SIZE;
	}

	return 0;
}

static inline void init_fat_entry(struct fat_entry *out) {
	memset(out, '\xff', sizeof(struct fat_entry));
}

static int read_fat_entry(struct fat_entry *out, int block) {
	size_t offset = fat_entry_offset(block);

	if (init_eraseblock(offset)) {
		return -1;
	}

	if (fseek(f, offset, SEEK_SET)) {
		fprintf(stderr, "failed to seek to fat entry\n");
		return -1;
	}

	if (fread(out, sizeof(struct fat_entry), 1, f) != 1) {
		fprintf(stderr, "failed to read fat entry\n");
		return -1;
	}

	return 0;
}

static int write_fat_entry(struct fat_entry *in, int block) {
	size_t offset = fat_entry_offset(block);

	if (init_eraseblock(offset)) {
		return -1;
	}

	if (fseek(f, offset, SEEK_SET)) {
		fprintf(stderr, "failed to seek to fat entry\n");
		return -1;
	}

	if (fwrite(in, sizeof(struct fat_entry), 1, f) != 1) {
		fprintf(stderr, "failed to write fat entry\n");
		return -1;
	}

	return 0;
}

static inline void init_file_entry(struct file_entry *out) {
	memset(out, '\xff', sizeof(struct file_entry));
}

static int write_file_entry(struct file_entry *in, int block, int index) {
	size_t offset = block_offset(block) + sizeof(struct file_entry) * index;

	if (init_eraseblock(offset)) {
		return -1;
	}

	if (fseek(f, offset, SEEK_SET)) {
		fprintf(stderr, "failed to seek to file entry\n");
		return -1;
	}

	if (fwrite(in, sizeof(struct file_entry), 1, f) != 1) {
		fprintf(stderr, "failed to write file entry\n");
		return -1;
	}

	return 0;
}

static int write_block(void *in, size_t len, int block) {
	size_t offset = block_offset(block);

	if (init_eraseblock(offset)) {
		return -1;
	}

	if (fseek(f, offset, SEEK_SET)) {
		fprintf(stderr, "failed to seek to block\n");
		return -1;
	}

	if (fwrite(in, len, 1, f) != 1) {
		fprintf(stderr, "failed to write block\n");
		return -1;
	}

	return 0;
}

static int create_root_directory() {
	struct fat_entry fat;
	struct file_entry file;

	/* write format flag / FAT entry for block 0 (contains root file entry) */
	init_fat_entry(&fat);
	fat.previous = cpu_to_be32((ERASEBLOCK_SIZE << 12) | BLOCK_SIZE);
	if (write_fat_entry(&fat, 0)) {
		return -1;
	}

	/* write root file entry */
	init_file_entry(&file);
	file.flags = ~(FLAG_FREE | FLAG_VALID) & 0xff;
	file.parent_block = cpu_to_be32(0);
	file.data_block = cpu_to_be32(1);
	if (write_file_entry(&file, 0, 0)) {
		return -1;
	}

	/* write FAT entry for block 1 (contains first file entries of root directory) */
	init_fat_entry(&fat);
	FAT_PTR_FLAGS_SET(fat.previous, ~(FLAG_FREE | FLAG_VALID));
	if (write_fat_entry(&fat, 1)) {
		return -1;
	}

	return 0;
}

static int write_file(char *name, char *path) {
	int ret = -1;
	struct fat_entry fat;
	struct file_entry file;
	FILE *fin;
	char buf[BLOCK_SIZE];
	size_t len;
	size_t total = 0;
	int first_data_block = next_data_block;
	int data_block = 0;

	fin = fopen(path, "r");
	if (fin == NULL) {
		fprintf(stderr, "failed to open input file\n");
		return ret;
	}

	while ((len = fread(buf, 1, BLOCK_SIZE, fin)) != 0 || !data_block) {
		total += len;

		/* update next pointer of previous FAT entry */
		if (data_block) {
			if (read_fat_entry(&fat, data_block)) {
				goto err;
			}
			FAT_PTR_VAL_SET(fat.next, next_data_block);
			if (write_fat_entry(&fat, data_block)) {
				goto err;
			}
		}

		/* write FAT entry for new block */
		init_fat_entry(&fat);
		FAT_PTR_FLAGS_SET(fat.previous, ~(FLAG_FREE | FLAG_VALID));
		if (data_block) {
			FAT_PTR_VAL_SET(fat.previous, data_block);
		}
		if (write_fat_entry(&fat, next_data_block)) {
			goto err;
		}

		/* write data block */
		if (write_block(buf, len, next_data_block)) {
			goto err;
		}

		data_block = next_data_block;
		next_data_block++;
	}

	/* create new file entries block if necessary */
	if (dir_count == FILE_ENTRIES_PER_BLOCK) {
		/* update next pointer of previous FAT entry */
		if (read_fat_entry(&fat, dir_block)) {
			goto err;
		}
		FAT_PTR_VAL_SET(fat.next, next_data_block);
		if (write_fat_entry(&fat, dir_block)) {
			goto err;
		}

		/* write FAT entry for new block */
		init_fat_entry(&fat);
		FAT_PTR_FLAGS_SET(fat.previous, ~(FLAG_FREE | FLAG_VALID));
		FAT_PTR_VAL_SET(fat.previous, dir_block);
		if (write_fat_entry(&fat, next_data_block)) {
			goto err;
		}

		dir_block = next_data_block;
		dir_count = 0;
		next_data_block++;
	}

	/* write file entry */
	init_file_entry(&file);

	file.flags = ~(FLAG_FREE | FLAG_VALID) & 0xff;

	file.year = cpu_to_be16(1970);
	file.month = 1;
	file.day = 1;
	file.hour = 0;
	file.minute = 0;
	file.second = 0;

	file.length = cpu_to_be32(total);

	file.parent_block = cpu_to_be32(0);
	file.parent_index = cpu_to_be16(0);

	file.data_block = cpu_to_be32(first_data_block);

	snprintf(file.name, sizeof(file.name), "%s", name);

	if (write_file_entry(&file, dir_block, dir_count)) {
		goto err;
	}

	dir_count++;

	ret = 0;
err:
	fclose(fin);
	return ret;
}

static void usage(char* argv[]) {
	printf("Usage: %s [OPTIONS...]\n"
	       "\n"
	       "Options:\n"
	       "  -f <filename>  filename in image\n"
	       "  -i <file>      input filename\n"
	       "  -o <file>      output filename\n"
	       , argv[0]);
}

int main(int argc, char* argv[]) {
	int ret = EXIT_FAILURE;

	static char *filename = NULL;
	static char *input_filename = NULL;
	static char *output_filename = NULL;

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "f:i:o:");
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			filename = optarg;
			break;
		case 'i':
			input_filename = optarg;
			break;
		case 'o':
			output_filename = optarg;
			break;
		default:
			usage(argv);
			goto err;
		}
	}

	if (!filename || strlen(filename) == 0 ||
		!input_filename || strlen(input_filename) == 0 ||
		!output_filename || strlen(output_filename) == 0) {

		usage(argv);
		goto err;
	}

	f = fopen(output_filename, "w+");
	if (f == NULL) {
		fprintf(stderr, "failed to open output file\n");
		goto err;
	}

	if (create_root_directory()) {
		goto err_close;
	}

	if (write_file(filename, input_filename)) {
		goto err_close;
	}

	ret = EXIT_SUCCESS;

err_close:
	fclose(f);
err:
	return ret;
}
