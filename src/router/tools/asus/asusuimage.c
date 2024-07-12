// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 *  Copyright (C) 2024 OpenWrt.org
 *  Copyright (C) 2024 Oleg S <remittor@gmail.com>
 */

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <byteswap.h>
#include <zlib.h> /* crc32 */

/* Various defines picked from U-boot */

#define FDT_MAGIC	0xD00DFEED

#define IH_MAGIC	0x27051956

#define IH_OS_LINUX	5

#define IH_ARCH_ARM64	22

#define IH_TYPE_KERNEL	2
#define IH_TYPE_MULTI	4

#define IH_COMP_NONE	0

enum volume_t {
	VOL_KERNEL	= 0,
	VOL_RAMDISK,
	VOL_FLATDT,

	VOL_COUNT,
};

typedef struct {
	uint8_t     major;
	uint8_t     minor;
} __attribute__ ((packed)) version_t;

#define FS_OFFSET_PREFIX  (0xA9)

typedef struct {
	char		prod_name[23];
	uint8_t		unk0;        // version of rootfs ???
	uint32_t	fs_offset;   // 24 bit BE (first byte = 0xA9)
} __attribute__ ((packed)) trx1_t;

typedef struct {
	char		prod_name[12];
	uint16_t	sn;          // fw build no (example: 388)
	uint16_t	en;          // fw extended build no (example: 51234)
	uint8_t		dummy;       // likely random byte
	uint8_t		key;         // hash value from kernel and fs
	uint8_t		unk[6];      // likely random bytes
	uint32_t	fs_offset;   // 24 bit BE (first byte = 0xA9)
} __attribute__ ((packed)) trx2_t;	// hdr2

typedef struct {
	char		prod_name[23];
	uint8_t		unk0;
	uint32_t	unk1;        // ???  usualy: 0x003000
} __attribute__ ((packed)) trx3_t;

typedef struct image_header {
	uint32_t	ih_magic;
	uint32_t	ih_hcrc;
	uint32_t	ih_time;
	uint32_t	ih_size;     // content size
	uint32_t	ih_load;     // load addr
	uint32_t	ih_ep;       // entry point
	uint32_t	ih_dcrc;     // content hash
	uint8_t		ih_os;       // os type
	uint8_t		ih_arch;     // kernel arch
	uint8_t		ih_type;     // image type
	uint8_t		ih_comp;     // compression
	version_t	kernel_ver;  // usualy: 3.0
	version_t	fs_ver;      // usualy: 0.4
	union {
		trx1_t	trx1;
		trx2_t	trx2;
		trx3_t	trx3;
	} tail;
} __attribute__ ((packed)) image_header_t;

typedef struct {
	uint32_t	extendno;	// fw extended build no (example: 51234)
	uint16_t	buildno;	// fw build no (example: 388)
	uint16_t	r16;		// always 0 ???
	uint32_t	r32;		// always 0 ???
} __attribute__ ((packed)) tail_content_t;

#define DEF_ASUS_TAIL_MAGIC  0x2AFED414

typedef struct {
	uint8_t		flags: 4,	// always 0 ???
			type : 4;	// always 1 ???
	uint8_t		clen[3];	// content len (24bit BE)
	uint16_t	fcrc;		// crc for footer
	uint16_t	checksum;	// content hash
	uint32_t	magic;
} __attribute__ ((packed)) tail_footer_t;

typedef struct {
	int		show_info;
	char *		imagefn;
	char *		outfn;
	char		prod_name[128];
	int		trx_ver;
	version_t	kernel_ver;
	version_t	fs_ver;
	uint32_t	magic;
	uint32_t	type;
	uint32_t	flags;
	uint32_t	extendno;
	uint32_t	buildno;
	uint32_t	r16;
	uint32_t	r32;
} trx_opt_t;

trx_opt_t g_def = {0};
trx_opt_t g_opt = {0};

// =========================================================

#define ROUNDUP(x, n) (((x) + (n - 1)) & ~(n - 1))

// =========================================================

char * g_progname = "";
int g_debug = 0;

#define DBG(...) do { if (g_debug) printf(__VA_ARGS__); } while(0)
#define _attr_fmt_err_   __attribute__ ((format (printf, 1, 2)))

static _attr_fmt_err_
void fatal_error(const char * fmtstr, ...)
{
	va_list ap;
	fflush(0);
	fprintf(stderr, "%s: ERROR: ", g_progname);
	va_start(ap, fmtstr);
	vfprintf(stderr, fmtstr, ap);
	va_end (ap);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

#define ERR(fmtstr, ...) fatal_error(fmtstr, ## __VA_ARGS__)

static
uint16_t asus_hash16(const void * data, size_t length)
{
	uint16_t * current = (uint16_t *) data;
	length = length / sizeof(uint16_t);
	uint16_t hash = 0;

	while (length--) {
		hash ^= *current++;
	}
	return ~hash; // same as hash ^ 0xFFFF
}

void update_iheader_crc(image_header_t * hdr, const void * data, size_t data_size)
{
	if (data == NULL)
		data = (const void *)((char *)hdr + sizeof(image_header_t));

	// Calculate payload checksum
	hdr->ih_dcrc = htobe32(crc32(0, data, data_size));
	hdr->ih_size = htobe32(data_size);

	// Calculate header checksum
	hdr->ih_hcrc = 0;
	hdr->ih_hcrc = htobe32(crc32(0, (const void *)hdr, sizeof(image_header_t)));
}

static
void init_opt(void)
{
	memset(&g_def, 0, sizeof(g_def));
	g_def.show_info = 0;
	g_def.trx_ver = 3;
	g_def.magic = DEF_ASUS_TAIL_MAGIC;
	g_def.type = 1;
	g_def.flags = 0;
	memcpy(&g_opt, &g_def, sizeof(g_def));
}

static
void usage(int status)
{
	FILE * fp = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(fp, "Usage: %s -i <image> [OPTIONS...]\n", g_progname);
	fprintf(fp, "\n");
	fprintf(fp, "Options:\n");
	fprintf(fp, "    -i <filename>  input image filename \n");
	fprintf(fp, "    -o <filename>  output image filename \n");
	fprintf(fp, "    -x             show only image info \n");
	fprintf(fp, "    -n <name>      product name \n");
	fprintf(fp, "    -v <number>    TRX version: 2 or 3 (def: %d) \n", g_def.trx_ver);
	fprintf(fp, "    -K <#>.<#>     kernel version (def: \"%d.%d\") \n", g_def.kernel_ver.major, g_def.kernel_ver.minor);
	fprintf(fp, "    -F <#>.<#>     filesys version (def: \"%d.%d\") \n", g_def.fs_ver.major, g_def.fs_ver.minor);
	fprintf(fp, "    -m <signature> tail HEX signature (def: %08X) \n", g_def.magic);
	fprintf(fp, "    -t <number>    tail type (def: %X) \n", g_def.type);
	fprintf(fp, "    -f <number>    tail flags (def: %X) \n", g_def.flags);
	fprintf(fp, "    -e <number>    tail ext no (def: %u) \n", g_def.extendno);
	fprintf(fp, "    -b <number>    tail build no (def: %u) \n", g_def.buildno);
	fprintf(fp, "    -h             show this screen \n");
	exit(status);
}

static
int parse_args(int argc, char ** argv)
{
	char *str, *end;
	int opt;

	while ((opt = getopt(argc, argv, "Dxi:o:n:K:F:v:m:t:f:e:b:h?")) != -1) {
		switch (opt) {
		case 'i':
			g_opt.imagefn = optarg;
			break;
		case 'o':
			g_opt.outfn = optarg;
			break;
		case 'x':
			g_opt.show_info = 1;
			g_debug = 1;
			break;
		case 'D':
			g_debug = 1;
			break;
		case 'n':
			strncpy(g_opt.prod_name, optarg, sizeof(g_opt.prod_name) - 1);
			break;
		case 'v':
			g_opt.trx_ver = strtoul(optarg, &end, 0);
			if (end == optarg)
				ERR("Incorrect -v argument!");
			break;
		case 'K':
			g_opt.kernel_ver.major = (uint8_t) strtoul(optarg, &end, 10);
			if (end == optarg || end[0] != '.')
				ERR("Incorrect -K argument!");

			str = end + 1;
			g_opt.kernel_ver.minor = (uint8_t) strtoul(str, &end, 10);
			if (end == str)
				ERR("Incorrect -K argument!");
			break;
		case 'F':
			g_opt.fs_ver.major = (uint8_t) strtoul(optarg, &end, 10);
			if (end == optarg || end[0] != '.')
				ERR("Incorrect -F argument!");

			str = end + 1;
			g_opt.fs_ver.minor = (uint8_t) strtoul(str, &end, 10);
			if (end == str)
				ERR("Incorrect -F argument!");
			break;
		case 'm':
			g_opt.magic = strtoul(optarg, &end, 16);
			if (end == optarg)
				ERR("Incorrect -m argument!");
			break;
		case 't':
			g_opt.type = strtoul(optarg, &end, 0);
			if (end == optarg)
				ERR("Incorrect -t argument!");
			break;
		case 'f':
			g_opt.flags = strtoul(optarg, &end, 0);
			if (end == optarg)
				ERR("Incorrect -f argument!");
			break;
		case 'e':
			g_opt.extendno = strtoul(optarg, &end, 0);
			if (end == optarg)
				ERR("Incorrect -e argument!");
			break;
		case 'b':
			g_opt.buildno = strtoul(optarg, &end, 0);
			if (end == optarg)
				ERR("Incorrect -b argument!");
			break;
		case 'h':
		default:
			usage(EXIT_FAILURE);
		}
	}
	if (g_opt.imagefn == NULL || g_opt.imagefn[0] == 0)
		usage(EXIT_FAILURE); // Required input image filename!

	if (g_opt.show_info == 0)
		if (g_opt.outfn == NULL || g_opt.outfn[0] == 0)
			usage(EXIT_FAILURE); // Required output image filename!

	if (g_opt.trx_ver < 2 || g_opt.trx_ver > 3)
		usage(EXIT_FAILURE);

    return 0;
}

static
char * load_image(size_t pad_size, size_t * psize)
{
	uint32_t file_sz;
	size_t readed;
	void * buf;
	FILE *fp;

	fp = fopen(g_opt.imagefn, "rb");
	if (!fp)
		ERR("Can't open %s: %s", g_opt.imagefn, strerror(errno));

	rewind(fp);
	fseek(fp, 0, SEEK_END);
	file_sz = ftell(fp);
	rewind(fp);

	if ((int32_t)file_sz <= 0) {
		fclose(fp);
		ERR("Error getting filesize: %s", g_opt.imagefn);
	}

	if (file_sz <= sizeof(image_header_t)) {
		fclose(fp);
		ERR("Bad size: \"%s\" is no valid image", g_opt.imagefn);
	}

	buf = malloc(file_sz + pad_size);
	if (!buf) {
		fclose(fp);
		ERR("Out of memory!");
	}
	memset(buf, 0, file_sz + pad_size);

	readed = fread(buf, 1, file_sz, fp);
	fclose(fp);
	if (readed != (size_t)file_sz)
		ERR("Error reading file %s", g_opt.imagefn);

	*psize = file_sz;

	return (char *)buf;
}

static
uint32_t get_timestamp(void)
{
	char * env = getenv("SOURCE_DATE_EPOCH");
	time_t fixed_timestamp = -1;
	char * endptr = env;

	if (env && *env) {
		errno = 0;
		fixed_timestamp = (time_t) strtoull(env, &endptr, 10);

		if (errno || (endptr && *endptr != '\0')) {
			fprintf(stderr, "ERROR: Invalid SOURCE_DATE_EPOCH \n");
			fixed_timestamp = -1;
		}
	}

	if (fixed_timestamp == -1)
		time(&fixed_timestamp);

	DBG("timestamp: %u \n", (uint32_t)fixed_timestamp);
	return (uint32_t)fixed_timestamp;
}

static int show_info(char *img, size_t img_size)
{
	uint32_t data_size, fdt_size, fs_size, fs_offset = 0;
	uint16_t fcrc, fcrc_c, checksum_c;
	uint8_t fs_key, kernel_key, key;
	uint32_t sn, en, xx = 0;
	size_t buf_size = 12;
	tail_footer_t * foot;
	tail_content_t *cont;
	image_header_t *hdr;
	uint32_t cont_len;
	uint32_t *fs_data;
	uint8_t *buf;
	trx2_t *trx;
	int i;

	/* Assume valid, already validated early in process_image */
	hdr = (image_header_t *)img;
	foot = (tail_footer_t *)(img + img_size - sizeof(tail_footer_t));

	if (be32toh(hdr->ih_magic) != IH_MAGIC) {
		free(img);
		ERR("Incorrect image: \"%s\" magic must be %08X", g_opt.imagefn, IH_MAGIC);
	}

	g_opt.trx_ver = 0;
	if (be32toh(foot->magic) == g_opt.magic)
		g_opt.trx_ver = 3;  /* tail with magic = DEF_ASUS_TAIL_MAGIC */

	if (be32toh(hdr->tail.trx2.fs_offset) >> 24 == FS_OFFSET_PREFIX) {
		g_opt.trx_ver = 1;  /* hdr1 */

		for (i = 0; i < sizeof(hdr->tail.trx1.prod_name); i++) {
			if (hdr->tail.trx1.prod_name[i] >= 0x7F)
				g_opt.trx_ver = 2;  /* hdr2 */
		}

		if (hdr->tail.trx2.sn >= 380 && hdr->tail.trx2.sn <= 490)
			g_opt.trx_ver = 2;  /* hdr2 */
	}

	DBG("detect trx version = %d \n", g_opt.trx_ver);
	switch(g_opt.trx_ver) {
	case 1:
		free(img);
		ERR("Formart HDR1 currently not supported");
		break;
	case 2:
		trx = &hdr->tail.trx2;

		data_size = (uint32_t)be32toh(hdr->ih_size);
		sn = htole16(trx->sn);
		en = htole16(trx->en);

		if (en < 20000 && sn >= 386)
			en += 0x10000;

		DBG("hdr2.sn: %u (0x%04X) \n", sn, sn);
		DBG("hdr2.en: %u (0x%04X) \n", en, htole16(trx->en));
		DBG("hdr2.key: 0x%02X \n", trx->key);
		buf = trx->unk;
		for (size_t i = 0; i < buf_size; i += 2) {
			if (buf[0] == FS_OFFSET_PREFIX && (buf[3] & 3) == 0) {
				xx = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | (xx && 0xFF);
				fs_offset = be32toh(xx);
				buf += 2;
			}
			buf += 2;
		}
		DBG("fs_offset: 0x%08X \n", fs_offset);
		if (fs_offset + 128 > img_size)
			ERR("Incorrect fs_offset!");

		fs_data = (uint32_t *)(img + fs_offset);
		DBG("fs_data: %08X %08X \n", be32toh(fs_data[0]), be32toh(fs_data[1]));
		fs_size = sizeof(image_header_t) + data_size - fs_offset;
		DBG("fs_size: 0x%X bytes \n", fs_size);
		fs_key = img[fs_offset + fs_size / 2];
		kernel_key = img[fs_offset / 2];
		DBG("fs_key: 0x%02X   kernel_key: 0x%02X \n", fs_key, kernel_key);
		if (fs_key) {
			key = kernel_key + ~fs_key;
		} else {
			key = (kernel_key % 3) - 3;
		}
		DBG("key = 0x%02X \n", key);
		if (be32toh(fs_data[0]) == FDT_MAGIC) {
			DBG("fdt_offset: 0x%08X \n", fs_offset);
			fdt_size = be32toh(fs_data[1]);
			DBG("fdt_size: 0x%X bytes \n", fdt_size);
			fs_offset += ROUNDUP(fdt_size, 64);
			DBG("fs_offset: 0x%08X \n", fs_offset);
			fs_size -= ROUNDUP(fdt_size, 64);
			DBG("fs_size: 0x%X bytes \n", fs_size);
		}
		break;
	case 3:
		DBG("tail: footer size = 0x%lX  (%lu) \n", sizeof(tail_footer_t), sizeof(tail_footer_t));
		DBG("tail: footer magic: 0x%X \n", be32toh(foot->magic));

		cont_len = foot->clen[0] << 24 | foot->clen[1] << 16 | foot->clen[2];
		DBG("tail: type = %X, flags = %X, content len = 0x%06X \n", foot->type, foot->flags, cont_len);

		fcrc = foot->fcrc;
		foot->fcrc = 0;
		fcrc_c = asus_hash16(foot, sizeof(*foot));
		DBG("tail: fcrc = %04X  (%04X) \n", be16toh(fcrc), fcrc_c);

		cont = (tail_content_t *)((char *)foot - cont_len);
		checksum_c = asus_hash16(cont, sizeof(*cont));
		DBG("cont: checksum = %04X  (%04X) \n", be16toh(foot->checksum), checksum_c);

		DBG("cont: buildno: %u, extendno: %u \n", be16toh(cont->buildno), be32toh(cont->extendno));
		DBG("cont: r16: 0x%08X, r32: 0x%08X \n", be16toh(cont->r16), be32toh(cont->r32));
		break;
	default:
		free(img);
		ERR("Input image is not compatible with AsusWRT");
	}

	free(img);
	return 0;
}

static
int process_image(void)
{
	const uint32_t hsz = sizeof(image_header_t);
	uint32_t vol_offset[VOL_COUNT + 1] = { 0 };
	uint32_t i, data_size, data_crc_c, fs_offset = 0, vol_count = 0,
	*fs_data, fs_size, fdt_size, *vol_size, xoffset, *fdt,
	cur_fdt_offset, new_fdt_offset, new_fs_offset, pad,
	hsqs_offset, hsqs_size, *hsqs_data, cont_len;
	uint32_t __attribute__ ((unused)) fdt_offset;
	size_t img_size = 0, max_prod_len, new_img_size,
	  new_data_size, wlen;
	char *img, *img_end, *prod_name;
	uint8_t fs_key, kernel_key, key;
	const char *prod_name_str;
	tail_content_t *cont;
	image_header_t *hdr;
	tail_footer_t *foot;
	trx2_t *trx;
	FILE *fp;

	img = load_image(1024, &img_size);
	if (!img)
		ERR("Can't load file %s", g_opt.imagefn);

	if (g_opt.show_info)
		return show_info(img, img_size);

	hdr = (image_header_t *)img;
	if (be32toh(hdr->ih_magic) != IH_MAGIC) {
		memmove(img + hsz, img, img_size);
		memset(hdr, 0, hsz);
		hdr->ih_magic = htobe32(IH_MAGIC);
		hdr->ih_time = htobe32(get_timestamp());
		hdr->ih_size = htobe32(img_size);
		hdr->ih_load = 0;
		hdr->ih_ep   = 0;
		hdr->ih_os   = IH_OS_LINUX;
		hdr->ih_arch = IH_ARCH_ARM64;
		hdr->ih_type = IH_TYPE_KERNEL;
		hdr->ih_comp = IH_COMP_NONE;
		img_size += hsz;
	}
	data_size = (uint32_t)be32toh(hdr->ih_size);
	DBG("data: size = 0x%08X  (%u bytes) \n", data_size, data_size);
	if (data_size + hsz > img_size)
		ERR("Bad size: \"%s\" is no valid content size", g_opt.imagefn);

	data_crc_c = crc32(0, (const unsigned char *)(img + hsz), data_size);
	DBG("data: crc = %08X  (%08X) \n", be32toh(hdr->ih_dcrc), data_crc_c);

	DBG("image type: %d \n", (int)hdr->ih_type);

	img_end = img + img_size;

	memset(&hdr->tail.trx1, 0, sizeof(hdr->tail.trx1));
	switch(g_opt.trx_ver) {
	case 2:
		prod_name = hdr->tail.trx2.prod_name;
		max_prod_len = sizeof(hdr->tail.trx2.prod_name);
		break;
	case 3:
		prod_name = hdr->tail.trx3.prod_name;
		max_prod_len = sizeof(hdr->tail.trx3.prod_name);
	}

	prod_name_str = (const char *)&hdr->kernel_ver;
	if (g_opt.prod_name[0])
		prod_name_str = g_opt.prod_name;

	strncpy(prod_name, prod_name_str, max_prod_len);
	hdr->kernel_ver = g_opt.kernel_ver;
	hdr->fs_ver = g_opt.fs_ver;

	switch(g_opt.trx_ver) {
	case 2:
		trx = &hdr->tail.trx2;

		if (hdr->ih_type == IH_TYPE_MULTI) {
			DBG("detect image with type: IH_TYPE_MULTI \n");
			vol_size = (uint32_t *)(img + hsz);
			if (vol_size[0] == 0) {
				free(img);
				ERR("Multi image does not contain volumes");
			}

			for (uint32_t i = 0; i <= VOL_COUNT; i++) {
				if (vol_size[i] == 0)
			   		break;
				vol_count++;
			}
			DBG("Multi image: volumes count = %u \n", vol_count);

			if (vol_count > VOL_COUNT) {
				free(img);
				ERR("Multi image contains too many volumes");
			}

			xoffset = hsz + sizeof(uint32_t) * (vol_count + 1);
			for (i = 0; i < vol_count; i++) {
				xoffset = ROUNDUP(xoffset, 4);
				vol_offset[i] = xoffset;
				DBG("Multi image: volume %u has offset = 0x%08X \n", i, xoffset);
				if (be32toh(vol_size[i]) > 0x4FFFFFF) {
					free(img);
			    		ERR("Multi image contain volume %u with huge size", i);
				}

				xoffset += be32toh(vol_size[i]);
			}
			if (xoffset > img_size) {
				free(img);
				ERR("Multi image contain incorrect img-size header");
			}

			fdt = (uint32_t *)(img + vol_offset[VOL_FLATDT]);
			if (vol_offset[VOL_FLATDT] && be32toh(fdt[0]) == FDT_MAGIC) {
				if (hdr->ih_arch == IH_ARCH_ARM64 && (vol_offset[VOL_FLATDT] & 7) != 0) {
					// for ARM64 offset of FlatDT must be 8-bytes align
					cur_fdt_offset = vol_offset[VOL_FLATDT];
					new_fdt_offset = vol_offset[VOL_FLATDT] + 4;
					memmove(img + new_fdt_offset, img + cur_fdt_offset, img_size - cur_fdt_offset);
					memset(img + cur_fdt_offset, 0, 4);
					img_size += 4;
					data_size += 4;
					vol_offset[VOL_FLATDT] = new_fdt_offset;
					vol_size[VOL_RAMDISK] = htobe32( be32toh(vol_size[VOL_RAMDISK]) + 4 );
					DBG("Multi image: volume %u size increased by 4 bytes \n", VOL_RAMDISK);
					DBG("Multi image: volume %u has offset = 0x%08X (patched) \n", VOL_FLATDT, new_fdt_offset);
				}
			}
			fs_offset = vol_offset[VOL_RAMDISK];
			if (fs_offset == 0) {
				//ERR("Multi image does not contain rootfs volume");
				fs_offset = hsz + data_size;
			}
		} else {
			fs_offset = hsz + data_size;
			if (fs_offset & 3) {
				//ERR("kernel size must be align to 4 bytes");
				new_fs_offset = ROUNDUP(fs_offset, 4);
				memmove(img + new_fs_offset, img + fs_offset, img_size - fs_offset);
				pad = new_fs_offset - fs_offset;
				memset(img + fs_offset, 0, pad);
				img_size += pad;
				data_size += pad;
				fs_offset = new_fs_offset;
			}
		}
		DBG("fs_offset: 0x%08X \n", fs_offset);
		fs_data = (uint32_t *)(img + fs_offset);
		DBG("fs_data: %08X %08X \n", be32toh(fs_data[0]), be32toh(fs_data[1]));
		fs_size = img_size - fs_offset;
		fdt_offset = 0;
		fdt_size = 0;
		hsqs_offset = fs_offset;
		hsqs_size = fs_size;
		if (be32toh(fs_data[0]) == FDT_MAGIC && !vol_count) {
			fdt_offset = fs_offset;
			DBG("fdt_offset: 0x%08X \n", fs_offset);
			fdt_size = be32toh(fs_data[1]);
			DBG("fdt_size: 0x%X bytes \n", fdt_size);
			hsqs_offset += ROUNDUP(fdt_size, 64);
			hsqs_size -= ROUNDUP(fdt_size, 64);
		}
		DBG("hsqs_offset: 0x%08X \n", hsqs_offset);
		DBG("hsqs_size: 0x%X bytes \n", hsqs_size);
		hsqs_data = (uint32_t *)(img + hsqs_offset);
		DBG("hsqs_data: %08X %08X \n", be32toh(hsqs_data[0]), be32toh(hsqs_data[1]));
		kernel_key = img[fs_offset / 2];
		fs_key = img[fs_offset + fs_size / 2];
		DBG("fs_key: 0x%02X   kernel_key: 0x%02X \n", fs_key, kernel_key);
		key = fs_key ? kernel_key + ~fs_key : (kernel_key % 3) - 3;
		DBG("key = 0x%02X \n", key);
		trx->sn = htole16((uint16_t)g_opt.buildno);
		trx->en = htole16((uint16_t)g_opt.extendno);
		trx->key = key;
		if (fs_offset >= 0xFFFFFF) {
			free(img);
			ERR("kernel image size is too big (max size: 16MiB)");
		}

		trx->fs_offset = htobe32((FS_OFFSET_PREFIX << 24) + fs_offset);
		update_iheader_crc(hdr, NULL, img_size - hsz);
		break;
	case 3:
		cont_len = 0;
		cont = NULL;
		foot = NULL;

		hdr->tail.trx3.unk1 = htobe32(0x3000);  // unknown value

		cont_len = img_size - hsz - data_size + sizeof(tail_content_t);
		cont = (tail_content_t *)img_end;
		cont->extendno = htobe32(g_opt.extendno);
		cont->buildno = htobe16(g_opt.buildno);
		cont->r16 = htobe16(g_opt.r16);
		cont->r32 = htobe32(g_opt.r32);

		foot = (tail_footer_t *)(img_end + sizeof(tail_content_t));
		char * cont_ptr = img + hsz + data_size;
		foot->checksum = htobe16(asus_hash16(cont_ptr, cont_len));

		if (cont_len >= (1UL << 24)) {
			free(img);
			ERR("Content length is too long (more than 0x%lX bytes)", 1UL << 24);
		}

		foot->clen[0] = (cont_len >> 16) & 0xFF;  // 24bit BigEndian
		foot->clen[1] = (cont_len >> 8) & 0xFF;
		foot->clen[2] = cont_len & 0xFF;

		foot->magic = htobe32(g_opt.magic);
		foot->type = g_opt.type;
		foot->flags = g_opt.flags;
		foot->fcrc = 0;
		foot->fcrc = htobe16(asus_hash16(foot, sizeof(*foot)));

		new_img_size = (size_t)((char *)foot + sizeof(tail_footer_t) - img);
		new_data_size = new_img_size - hsz;
		update_iheader_crc(hdr, NULL, new_data_size);

		img_size = hsz + data_size + cont_len + sizeof(tail_footer_t);
	}

	fp = fopen(g_opt.outfn, "wb");
	if (!fp) {
		free(img);
		ERR("Can't open %s for writing: %s", g_opt.outfn, strerror(errno));
	}

	wlen = fwrite(img, img_size, 1, fp);
	fclose(fp);
	if (wlen != 1) {
		free(img);
		ERR("Failed to write: %s", g_opt.outfn);
	}

	DBG("New TRX-image file created: \"%s\" \n", g_opt.outfn);
	free(img);

	return 0; // OK
}

int main(int argc, char ** argv)
{
	g_progname = argv[0];

	init_opt();
	parse_args(argc, argv);

	int rc = process_image();

	return rc;
}
