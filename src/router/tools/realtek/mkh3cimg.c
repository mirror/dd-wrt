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
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_be16(x)  bswap_16(x)
#define cpu_to_be32(x)  bswap_32(x)
#else
#error "Unsupported endianness"
#endif


#define IMAGE_VERSION            1
#define FILE_VERSION             1
#define FILE_DESCRIPTION         "DD-WRT"
#define FILE_TYPE_MASK           0x1


#define PACKAGE_FLAG             2

#define FILE_TYPE_APPLICATION    0x04000000

#define VERSION_OFFSET_INVALID   0xffffffff

#define COMPRESSION_TYPE_NONE    0xffffffff
#define COMPRESSION_TYPE_7Z      0x00000002


struct file_header {
	uint8_t res1[4];

	uint32_t header_crc;

	uint32_t file_type;
	uint32_t version;
	uint32_t product_id;
	uint32_t device_id;

	uint32_t length_unpadded;

	uint32_t version_offset;

	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t res2[1];
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	uint8_t res3[64];

	char description[224];

	uint32_t length;

	uint32_t file_crc;

	uint32_t compression_type;
} __attribute__ ((packed));

struct file_desc {
	uint32_t file_type;
	uint32_t offset;
	uint32_t length;
	uint32_t file_crc;
	uint32_t version;
	uint32_t type_mask;
} __attribute__ ((packed));

struct image_header {
	uint32_t version;

	uint32_t file_count;

	uint32_t product_id;
	uint32_t device_id;

	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t res1[1];
	uint8_t hour;
	uint8_t minute;
	uint8_t second;

	uint16_t package_crc;
	uint16_t package_flag;

	uint32_t length;

	struct file_desc files[128];

	/* RSA signature, not required */
	uint8_t res2[3072];

	uint32_t header_crc;
} __attribute__ ((packed));


static void *buf;
static size_t buflen;

static size_t length_unpadded;
static size_t length;


static uint32_t crc16_xmodem(char *buf, size_t len) {
	uint32_t poly = 0x1021;
	uint32_t crc = 0;
	char b;
	int i, j;

	for (i = 0; i < len; i++) {
		b = buf[i];
		crc = crc ^ (b << 8);

		for (j = 0; j < 8; j++) {
			crc = crc << 1;
			if (crc & 0x10000) {
				crc = (crc ^ poly) & 0xffff;
			}
		}
	}

	return crc;
}

static int create_buffer_and_read_file(char *filename) {
	FILE *f;

	f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "failed to open input file\n");
		goto err;
	}

	fseek(f, 0L, SEEK_END);
	length_unpadded = ftell(f);
	rewind(f);

	length = length_unpadded;
	if (length_unpadded % 8 != 0) {
		length += 8 - length_unpadded % 8;
	}

	buflen = sizeof(struct file_header) + sizeof(struct image_header) + length;
	buf = malloc(buflen);
	if (!buf) {
		fprintf(stderr, "failed to allocate buffer\n");
		goto err_close;
	}

	memset(buf, 0, buflen);

	if (fread(buf + sizeof(struct file_header) + sizeof(struct image_header), length_unpadded, 1, f) != 1) {
		fprintf(stderr, "failed to read input file\n");
		goto err_free;
	}

	fclose(f);
	return 0;

err_free:
	free(buf);
err_close:
	fclose(f);
err:
	return -1;
}

static void build_file_header(uint32_t product_id, uint32_t device_id, uint32_t compression_type) {
	struct file_header *header = buf + sizeof(struct image_header);
	uint32_t crc;

	header->file_type = cpu_to_be32(FILE_TYPE_APPLICATION);

	header->version = cpu_to_be32(FILE_VERSION);

	header->product_id = cpu_to_be32(product_id);
	header->device_id = cpu_to_be32(device_id);

	header->length_unpadded = cpu_to_be32(length_unpadded);

	header->version_offset = cpu_to_be32(VERSION_OFFSET_INVALID);

	header->year = cpu_to_be16(1970);
	header->month = 1;
	header->day = 1;
	header->hour = 0;
	header->minute = 0;
	header->second = 0;

	snprintf(header->description, sizeof(header->description), "%s", FILE_DESCRIPTION);

	header->length = cpu_to_be32(length);

	crc = crc16_xmodem(buf + sizeof(struct image_header) + sizeof(struct file_header), length);
	header->file_crc = cpu_to_be32(crc);

	header->compression_type = cpu_to_be32(compression_type);

	crc = crc16_xmodem((char *)header + sizeof(header->res1) + sizeof(header->header_crc),
		sizeof(struct file_header) - sizeof(header->res1) - sizeof(header->header_crc));
	header->header_crc = cpu_to_be32(crc);
}

static void build_image_header(uint32_t product_id, uint32_t device_id) {
	struct image_header *header = buf;
	struct file_header *file_header = buf + sizeof(struct image_header);
	uint32_t crc;

	header->version = cpu_to_be32(IMAGE_VERSION);

	header->file_count = cpu_to_be32(1);

	header->product_id = cpu_to_be32(product_id);
	header->device_id = cpu_to_be32(device_id);

	header->year = cpu_to_be16(1970);
	header->month = 1;
	header->day = 1;
	header->hour = 0;
	header->minute = 0;
	header->second = 0;

	crc = crc16_xmodem(buf + sizeof(struct file_header), buflen - sizeof(struct file_header));
	header->package_crc = cpu_to_be16(crc);
	header->package_flag = cpu_to_be16(PACKAGE_FLAG);

	header->length = cpu_to_be32(buflen - sizeof(struct image_header));

	header->files[0].file_type = file_header->file_type;
	header->files[0].offset = cpu_to_be32(sizeof(struct image_header));
	header->files[0].length = cpu_to_be32(sizeof(struct file_header) + length);
	header->files[0].file_crc = file_header->file_crc;
	header->files[0].version = file_header->version;
	header->files[0].type_mask = cpu_to_be32(FILE_TYPE_MASK);

	crc = crc16_xmodem((char *)header, sizeof(struct image_header) - sizeof(header->header_crc));
	header->header_crc = cpu_to_be32(crc);
}

static int write_output_file(char *filename) {
	int ret = 0;
	FILE *f;

	f = fopen(filename, "w");
	if (f == NULL) {
		fprintf(stderr, "failed to open output file\n");
		ret = -1;
		goto err;
	}

	if (fwrite(buf, buflen, 1, f) != 1) {
		fprintf(stderr, "failed to write output file\n");
		ret = -1;
	}

	fclose(f);

err:
	return ret;
}

static void usage(char* argv[]) {
	printf("Usage: %s [OPTIONS...]\n"
	       "\n"
	       "Options:\n"
	       "  -p <product id>   product id (32-bit unsigned integer)\n"
	       "  -d <device id>    device id (32-bit unsigned integer)\n"
	       "  -c <compression>  compression type of the input file (7z or none)\n"
	       "                    (in case of 7z only LZMA compression is allowed)\n"
	       "  -i <file>         input filename\n"
	       "  -o <file>         output filename\n"
	       , argv[0]);
}

int main(int argc, char* argv[]) {
	int ret = EXIT_FAILURE;

	static uint32_t product_id = 0;
	static uint32_t device_id = 0;
	static uint32_t compression_type = COMPRESSION_TYPE_NONE;
	static char *input_filename = NULL;
	static char *output_filename = NULL;

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "p:d:c:i:o:");
		if (c == -1)
			break;

		switch (c) {
		case 'p':
			product_id = strtoul(optarg, NULL, 0);
			break;
		case 'd':
			device_id = strtoul(optarg, NULL, 0);
			break;
		case 'c':
			if (strcmp(optarg, "none") == 0) {
				compression_type = COMPRESSION_TYPE_NONE;
			} else if (strcmp(optarg, "7z") == 0) {
				compression_type = COMPRESSION_TYPE_7Z;
			} else {
				usage(argv);
				return EXIT_FAILURE;
			}
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

	if (!product_id || !device_id ||
		!input_filename || strlen(input_filename) == 0 ||
		!output_filename || strlen(output_filename) == 0) {

		usage(argv);
		goto err;
	}

	if (create_buffer_and_read_file(input_filename)) {
		goto err;
	}

	build_file_header(product_id, device_id, compression_type);

	build_image_header(product_id, device_id);

	if (write_output_file(output_filename)) {
		goto err_free;
	}

	ret = EXIT_SUCCESS;

err_free:
	free(buf);
err:
	return ret;
}
