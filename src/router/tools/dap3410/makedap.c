/* short hacked code for DAP3310/DAP3410 firmware tool by Sebastian Gottschall (DD-WRT.COM) */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <byteswap.h>
#include <endian.h>		/* for __BYTE_ORDER */

#if defined(__CYGWIN__)
#  include <byteswap.h>
#endif

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define HOST_TO_BE32(x)	bswap_32(x)
#else
#  define HOST_TO_BE32(x)	(x)
#endif

/*
 * Code to compute the CRC-32 table. Borrowed from
 * gzip-1.0.3/makecrc.c.
 */

static uint32_t crc_32_tab[256];

void init_crc_table(void)
{
	/* Not copyrighted 1990 Mark Adler      */

	uint32_t c;		/* crc shift register */
	uint32_t e;		/* polynomial exclusive-or pattern */
	int i;			/* counter for all possible eight bit values */
	int k;			/* byte being shifted into crc apparatus */

	/* terms of polynomial defining this crc (except x^32): */
	static const int p[] = { 0, 1, 2, 4, 5, 7, 8, 10, 11, 12, 16, 22, 23, 26 };

	/* Make exclusive-or pattern from polynomial */
	e = 0;
	for (i = 0; i < sizeof(p) / sizeof(int); i++)
		e |= 1L << (31 - p[i]);

	crc_32_tab[0] = 0;

	for (i = 1; i < 256; i++) {
		c = 0;
		for (k = i | 256; k != 1; k >>= 1) {
			c = c & 1 ? (c >> 1) ^ e : c >> 1;
			if (k & 1)
				c ^= e;
		}
		crc_32_tab[i] = c;
	}
}

void update_crc(uint8_t * p, uint32_t len, uint32_t * crc)
{
	uint32_t t;

	t = *crc ^ 0xFFFFFFFFUL;
	while (len--) {
		t = crc_32_tab[(t ^ *p++) & 0xff] ^ (t >> 8);
	}
	*crc = t ^ 0xFFFFFFFFUL;
}

uint32_t get_crc(uint8_t * p, uint32_t len)
{
	uint32_t crc;

	crc = 0;
	update_crc(p, len, &crc);
	return crc;
}

struct header {
	char ident[8];		//CENRAMD for filesys, CENUIMG for kernel
	unsigned int size;
	unsigned int ver;
	unsigned int crc;
	unsigned char data[];
};

int main(int argc, char *argv[])
{
	FILE *in;
	FILE *out;
	if (argc < 5) {
		fprintf(stderr, "usage: makedap [kernel] [filesys] [dapfirmware] [type]\ntype: DAP3410 or DAP3310\n");
		exit(-1);
	}
	unsigned int ver;
	if (!strcmp(argv[4], "DAP3410"))
		ver = HOST_TO_BE32(0x08050004);
	else if (!strcmp(argv[4], "DAP3310"))
		ver = HOST_TO_BE32(0x08050102);
	else {
		fprintf(stderr, "wrong device type. must be DAP3310 or DAP3410\n");
		exit(-1);
	}
	in = fopen(argv[1], "rb");
	long len;
	fseek(in, 0, SEEK_END);
	len = ftell(in);
	if (len > 1024 * 1024)
		fprintf(stderr, "kernel too big (1024kb max size)\n");
	rewind(in);
	struct header *kernel;
	kernel = malloc(len + 20);
	strcpy(kernel->ident, "CENUIMG");
	kernel->size = HOST_TO_BE32(len);
	kernel->ver = ver;
	fread(kernel->data, 1, len, in);
	fclose(in);
	init_crc_table();
	update_crc(kernel->data, len, &kernel->crc);
	kernel->crc = HOST_TO_BE32(kernel->crc);

	in = fopen(argv[2], "rb");
	fseek(in, 0, SEEK_END);
	len = ftell(in);
	if (len > 6144 * 1024)
		fprintf(stderr, "filesys too big (6144kb max size)\n");
	rewind(in);
	struct header *filesys;
	filesys = malloc(len + 20);
	strcpy(filesys->ident, "CENRAMD");
	filesys->size = HOST_TO_BE32(len);
	filesys->ver = ver;
	fread(filesys->data, 1, len, in);
	fclose(in);
	init_crc_table();
	update_crc(filesys->data, len, &filesys->crc);
	filesys->crc = HOST_TO_BE32(filesys->crc);
	out = fopen(argv[3], "wb");
	fwrite(filesys, 1, HOST_TO_BE32(filesys->size) + 20, out);
	fwrite(kernel, 1, HOST_TO_BE32(kernel->size) + 20, out);
	fclose(out);
}
