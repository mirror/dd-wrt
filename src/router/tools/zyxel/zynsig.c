// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Copyright (C) 2024-2025 Andreas Boehler <dev@aboehler.at>
 *  Copyright (C) 2007-2008 OpenWrt.org
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg at openwrt.org>
 *
 *  zynsig.c is basically a stripped-down version of mkzynfw.c,
 *  it just adds the required "SIG" header to the given file.
 *  This file is then accepted as a valid BootExt block by BootBase, to mask
 *  rt-loader as BootExt.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <byteswap.h>
#include <unistd.h>
#include <libgen.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "zynos.h"

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define HOST_TO_LE16(x)	(x)
#  define HOST_TO_LE32(x)	(x)
#  define LE16_TO_HOST(x)	(x)
#  define LE32_TO_HOST(x)	(x)
#  define HOST_TO_BE16(x)	bswap_16(x)
#  define HOST_TO_BE32(x)	bswap_32(x)
#  define BE16_TO_HOST(x)	bswap_16(x)
#  define BE32_TO_HOST(x)	bswap_32(x)
#else
#  define HOST_TO_BE16(x)	(x)
#  define HOST_TO_BE32(x)	(x)
#  define BE16_TO_HOST(x)	(x)
#  define BE32_TO_HOST(x)	(x)
#  define HOST_TO_LE16(x)	bswap_16(x)
#  define HOST_TO_LE32(x)	bswap_32(x)
#  define LE16_TO_HOST(x)	bswap_16(x)
#  define LE32_TO_HOST(x)	bswap_32(x)
#endif

#define ALIGN(x,y)	(((x)+((y)-1)) & ~((y)-1))

/*
 * Message macros
 */
#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__ ); \
} while (0)


#define MAX_ARG_COUNT	32
#define MAX_ARG_LEN	1024

struct csum_state{
	int		odd;
	uint32_t	sum;
	uint32_t	tmp;
};

char *ofname = NULL;
char *ifname = NULL;

void *input_file = NULL;
char *progname;
uint32_t load_addr = 0x80100000;
uint32_t mmap_addr = 0xb40e0000;

/*
 * Helper routines
 */
void
usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream, "\nOptions:\n");
	fprintf(stream,
"  -i <file>\n"
"                  input file, e.g. rt-loader image\n"
"  -o <file>\n"
"                  write output to the file <file>\n"
"  -h              show this screen\n"
	);

	exit(status);
}

void
csum_init(struct csum_state *css)
{
	css->odd = 0;
	css->sum = 0;
	css->tmp = 0;
}

void
csum_update(void *data, uint32_t len, struct csum_state *css)
{
	uint8_t *p = data;

	if (len == 0)
		return;

	if (css->odd) {
		css->sum += (css->tmp << 8) + p[0];
		if (css->sum > 0xFFFF) {
			css->sum += 1;
			css->sum &= 0xFFFF;
		}
		css->odd = 0;
		len--;
		p++;
	}

	for ( ; len > 1; len -= 2, p +=2 ) {
		css->sum  += (p[0] << 8) + p[1];
		if (css->sum > 0xFFFF) {
			css->sum += 1;
			css->sum &= 0xFFFF;
		}
	}

	if (len == 1){
		css->tmp = p[0];
		css->odd = 1;
	}
}

uint16_t
csum_get(struct csum_state *css)
{
	char pad = 0;

	csum_update(&pad, 1, css);
	return css->sum;
}

uint16_t
csum_buf(uint8_t *p, uint32_t len)
{
	struct csum_state css;

	csum_init(&css);
	csum_update(p, len, &css);
	return csum_get(&css);

}

static void
*map_input(const char *name, size_t *len)
{
	struct stat stat;
	void *mapped;
	int fd;

	fd = open(name, O_RDONLY);
	if (fd < 0)
		return NULL;
	if (fstat(fd, &stat) < 0) {
		close(fd);
		return NULL;
	}
	*len = stat.st_size;
	mapped = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (close(fd) < 0) {
		(void) munmap(mapped, stat.st_size);
		return NULL;
	}
	return mapped;
}

int
parse_arg(char *arg, char *buf, char *argv[])
{
	int res = 0;
	size_t argl;
	char *tok;
	char **ap = &buf;
	int i;

	memset(argv, 0, MAX_ARG_COUNT * sizeof(void *));

	if ((arg == NULL)) {
		/* no arguments */
		return 0;
	}

	argl = strlen(arg);
	if (argl == 0) {
		/* no arguments */
		return 0;
	}

	if (argl >= MAX_ARG_LEN) {
		/* argument is too long */
		argl = MAX_ARG_LEN-1;
	}

	memcpy(buf, arg, argl);
	buf[argl] = '\0';

	for (i = 0; i < MAX_ARG_COUNT; i++) {
		tok = strsep(ap, ":");
		if (tok == NULL) {
			break;
		}
		argv[i] = tok;
		res++;
	}

	return res;
}

int
required_arg(char c, char *arg)
{
	if (arg == NULL || *arg != '-')
		return 0;

	ERR("option -%c requires an argument\n", c);
	return -1;
}

int
parse_opt_name(char ch, char *arg, char **dest)
{

	if (*dest != NULL) {
		ERR("only one input/output file allowed");
		return -1;
	}

	if (required_arg(ch, arg))
		return -1;

	*dest = arg;

	return 0;
}

int
is_empty_arg(char *arg)
{
	int ret = 1;
	if (arg != NULL) {
		if (*arg) ret = 0;
	};
	return ret;
}

int main(int argc, char *argv[]) {
	uint16_t csum;
	size_t file_len = 0;
	int optinvalid = 0;   /* flag for invalid option */
	int res = EXIT_FAILURE;
	int c;

	struct zyn_rombin_hdr bootext_hdr;

	FILE *outfile;

	progname=basename(argv[0]);

	opterr = 0;  /* could not print standard getopt error messages */
	while ( 1 ) {
		optinvalid = 0;

		c = getopt(argc, argv, "i:o:h");
		if (c == -1)
			break;

		switch (c) {
		case 'i':
			optinvalid = parse_opt_name(c,optarg,&ifname);
			break;
		case 'o':
			optinvalid = parse_opt_name(c,optarg,&ofname);
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			optinvalid = 1;
			break;
		}
		if (optinvalid != 0 ) {
			ERR("invalid option: -%c", optopt);
			goto out;
		}
	}

	if(!ifname) {
		ERR("input file is mandatory");
		goto out;
	}

	if(!ofname) {
		ERR("output file is mandatory");
		goto out;
	}

	input_file = map_input(ifname, &file_len);
	if(!input_file) {
		ERR("input file not found.");
		goto out;
	}
	csum = csum_buf((uint8_t*)input_file, file_len);

	memset(&bootext_hdr, 0, sizeof(bootext_hdr));
	bootext_hdr.addr = HOST_TO_BE32(load_addr);
	bootext_hdr.type = OBJECT_TYPE_BOOTEXT;

	memcpy(&bootext_hdr.sig, ROMBIN_SIGNATURE, ROMBIN_SIG_LEN);
	bootext_hdr.osize = HOST_TO_BE32(file_len);
	bootext_hdr.ocsum = HOST_TO_BE16(csum);
	bootext_hdr.mmap_addr = HOST_TO_BE32(mmap_addr);

	bootext_hdr.flags = ROMBIN_FLAG_OCSUM;
	bootext_hdr.csize = HOST_TO_BE32(0);
	bootext_hdr.ccsum = HOST_TO_BE16(0);

	outfile = fopen(ofname, "w");
	fwrite(&bootext_hdr, sizeof(bootext_hdr), 1, outfile);
	fwrite(input_file, file_len, 1, outfile);
	fflush(outfile);
	fclose(outfile);

	res = EXIT_SUCCESS;
out:
	if (res != EXIT_SUCCESS) {
		unlink(ofname);
	}
	if(input_file)
		munmap(input_file, file_len);
	return res;
}
