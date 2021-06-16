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

#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)		bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)		(X)
#else
#error unkown endianness!
#endif

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

static void copy(FILE * out, size_t inoff, size_t outoff, int len)
{
	size_t i;
	char *mem = malloc(65536);
	fprintf(stderr, "\n");
	if (inoff >= outoff) {
		for (i = 0; i < (len / 65536); i++) {
			fprintf(stderr, "copy from %zu to %zu\r", inoff + (i * 65536), outoff + (i * 65536));
			fseek(out, inoff + (i * 65536), SEEK_SET);
			fread(mem, 65536, 1, out);
			fseek(out, outoff + (i * 65536), SEEK_SET);
			fwrite(mem, 65536, 1, out);
		}
		if (len % 65536) {
			fseek(out, inoff + (i * 65536), SEEK_SET);
			fread(mem, len % 65536, 1, out);
			fseek(out, outoff + (i * 65536), SEEK_SET);
			fwrite(mem, len % 65536, 1, out);
		}
	} else {
		long o = len - 65536;
		for (i = 0; i < (len / 65536); i++) {
			fprintf(stderr, "copy from %lu to %lu\r", inoff + o, outoff + o);
			fseek(out, inoff + o, SEEK_SET);
			fread(mem, 65536, 1, out);
			fseek(out, outoff + o, SEEK_SET);
			fwrite(mem, 65536, 1, out);
			o -= 65536;
			if (o < 0)
				break;
		}
		if (len % 65536) {
			fseek(out, inoff, SEEK_SET);
			fread(mem, len % 65536, 1, out);
			fseek(out, outoff, SEEK_SET);
			fwrite(mem, len % 65536, 1, out);
		}
	}
	fprintf(stderr, "\n");
	free(mem);
}

int main(int argc, char *argv[])
{
	FILE *in = fopen(argv[1], "r+b");
	FILE *out = fopen(argv[2], "r+b");
	fseek(out, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
	fseek(in, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
// read old mbr
	struct pte presentlayout[4];
	struct pte newlayout[4];
	fread(&presentlayout, sizeof(struct pte), MBR_ENTRY_MAX, out);
	fread(&newlayout, sizeof(struct pte), MBR_ENTRY_MAX, in);

	int i;
	fprintf(stderr, "old layout\n");
	for (i = 0; i < 4; i++) {
		presentlayout[i].start = STORE32_LE(presentlayout[i].start);
		presentlayout[i].length = STORE32_LE(presentlayout[i].length);
		fprintf(stderr, "p[%d]: start %d end %d active %X type %X\n", i, presentlayout[i].start, presentlayout[i].start + presentlayout[i].length - 1, presentlayout[i].active, presentlayout[i].type);
	}
	fprintf(stderr, "new layout\n");
	for (i = 0; i < 4; i++) {
		newlayout[i].start = STORE32_LE(newlayout[i].start);
		newlayout[i].length = STORE32_LE(newlayout[i].length);
		fprintf(stderr, "p[%d]: start %d end %d active %X type %X\n", i, newlayout[i].start, newlayout[i].start + newlayout[i].length - 1, newlayout[i].active, newlayout[i].type);
	}
	struct pte *nvram = &presentlayout[2];
	fseek(out, nvram->start * 512, SEEK_SET);
	uint32_t len = nvram->length * 512;
	if (len) {
		if (nvram->start == newlayout[2].start) {
			if (nvram->length > newlayout[2].length) {
				// if old nvram partition size is bigger than the new partition to be written, we keep the old partition entry as is
				memcpy(&newlayout[2], &nvram, sizeof(struct pte));
				fseek(out, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
				fseek(in, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
				for (i = 0; i < 4; i++) {
					newlayout[i].start = STORE32_LE(newlayout[i].start);
					newlayout[i].length = STORE32_LE(newlayout[i].length);
				}
				fwrite(&newlayout, sizeof(struct pte), MBR_ENTRY_MAX, in);
				fwrite(&newlayout, sizeof(struct pte), MBR_ENTRY_MAX, out);
				for (i = 0; i < 4; i++) {
					newlayout[i].start = STORE32_LE(newlayout[i].start);
					newlayout[i].length = STORE32_LE(newlayout[i].length);
				}
			}
		} else {
			fprintf(stderr, "read nvram from old offset %d with len %d\n", nvram->start * 512, len);
			copy(out, nvram->start * 512, newlayout[2].start * 512, len);
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
	if (newlayout[2].start > 0) {
		copy(out, nvram->start * 512, newlayout[2].start * 512, len);
	}
	fclose(out);
	fclose(in);
}
