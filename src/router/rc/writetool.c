#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stdint.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le16(x) bswap_16(x)
#define cpu_to_le32(x) bswap_32(x)
#define cpu_to_le64(x) bswap_64(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)
#else
#error unknown endianness!
#endif

#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0)

#include <stdio.h>

#define MBR_ENTRY_MAX           4
#define MBR_DISK_SIGNATURE_OFFSET  440
#define MBR_PARTITION_ENTRY_OFFSET 446
#define MBR_BOOT_SIGNATURE_OFFSET  510
#define MBR_ENTRY_MAX 		4

#define DISK_SECTOR_SIZE        512

struct pte {
	uint8_t active;
	uint8_t chs_start[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint32_t start;
	uint32_t length;
};

int sectors = 63;
int heads = 16;

static void to_chs(long sect, unsigned char chs[3])
{
	int c, h, s;

	s = (sect % sectors) + 1;
	sect = sect / sectors;
	h = sect % heads;
	sect = sect / heads;
	c = sect;

	chs[0] = h;
	chs[1] = s | ((c >> 2) & 0xC0);
	chs[2] = c & 0xFF;

	return;
}

int main(int argc, char *argv[])
{
	FILE *in = fopen(argv[1], "rb");
	FILE *out = fopen(argv[2], "r+b");
	fseek(out, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
	fseek(in, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
// read old mbr
	struct pte p[4];
	struct pte old_p[4];
	fread(&p, sizeof(struct pte), MBR_ENTRY_MAX, out);
	fread(&old_p, sizeof(struct pte), MBR_ENTRY_MAX, in);

	int i;
	fprintf(stderr, "old layout\n");
	for (i = 0; i < 4; i++) {
		fprintf(stderr, "p[%d]: start %d end %d active %X type %X\n", i, p[i].start, p[i].start + p[i].length - 1, p[i].active, p[i].type);
	}
	fprintf(stderr, "new layout\n");
	for (i = 0; i < 4; i++) {
		fprintf(stderr, "p[%d]: start %d end %d active %X type %X\n", i, old_p[i].start, old_p[i].start + old_p[i].length - 1, old_p[i].active, old_p[i].type);
	}
	struct pte *nvram = &p[2];
	fseek(out, nvram->start * 512, SEEK_SET);
	uint32_t len = nvram->length * 512;
	char *mem = NULL;
	if (len) {
		fprintf(stderr, "read nvram from old offset %d\n", nvram->start * 512);
		mem = malloc(len);
		if (fread(mem, len, 1, out) == 1) {
			fprintf(stderr, "write nvram from mew offset %d\n", old_p[2].start * 512);
			fseek(out, old_p[2].start * 512, SEEK_SET);
			fwrite(mem, len, 1, out);
		}
	}
	fseek(in, 0, SEEK_END);
	len = ftell(in);
	fseek(in, 0, SEEK_SET);
	fseek(out, 0, SEEK_SET);
	char *buf = malloc(65536);
	int count = len / 65536;
	fprintf(stderr, "write image len = %d\n", len);
	for (i = 0; i < count; i++) {
		fread(buf, 65536, 1, in);
		fwrite(buf, 65536, 1, out);
	}
	fread(buf, len % 65536, 1, in);
	fwrite(buf, len % 65536, 1, out);
	free(buf);
	if (mem && old_p[2].start > 0) {
		fseek(out, old_p[2].start * 512, SEEK_SET);
		fwrite(mem, len, 1, out);
	}
	fclose(out);
	fclose(in);
}
