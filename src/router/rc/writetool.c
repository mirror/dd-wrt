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
#define le16_to_cpu(x) bswap_16(x)
#define le32_to_cpu(x) bswap_32(x)
#define le64_to_cpu(x) bswap_64(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(x) (x)
#define cpu_to_le32(x) (x)
#define cpu_to_le64(x) (x)
#define le16_to_cpu(x) (x)
#define le32_to_cpu(x) (x)
#define le64_to_cpu(x) (x)
#else
#error unknown endianness!
#endif

#define swap(a, b)                     \
	do {                           \
		typeof(a) __tmp = (a); \
		(a) = (b);             \
		(b) = __tmp;           \
	} while (0)

#include <stdio.h>

#define MBR_ENTRY_MAX 4
#define MBR_DISK_SIGNATURE_OFFSET 440
#define MBR_PARTITION_ENTRY_OFFSET 446
#define MBR_BOOT_SIGNATURE_OFFSET 510
#define MBR_ENTRY_MAX 4

#define DISK_SECTOR_SIZE 512

#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X) bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X) (X)
#else
#error unkown endianness!
#endif

#define GPT_MAGIC 0x5452415020494645ULL

enum {
	LEGACY_GPT_TYPE = 0xee,
	GPT_MAX_PARTS = 256,
	GPT_MAX_PART_ENTRY_LEN = 4096,
	GUID_LEN = 16,
};

struct pte {
	uint8_t active;
	uint8_t chs_start[3];
	uint8_t type;
	uint8_t chs_end[3];
	uint32_t start;
	uint32_t length;
};

typedef struct {
	uint64_t magic;
	uint32_t revision;
	uint32_t hdr_size;
	uint32_t hdr_crc32;
	uint32_t reserved;
	uint64_t current_lba;
	uint64_t backup_lba;
	uint64_t first_usable_lba;
	uint64_t last_usable_lba;
	uint8_t disk_guid[GUID_LEN];
	uint64_t first_part_lba;
	uint32_t n_parts;
	uint32_t part_entry_len;
	uint32_t part_array_crc32;
} gpt_header;

typedef struct {
	uint8_t type_guid[GUID_LEN];
	uint8_t part_guid[GUID_LEN];
	uint64_t lba_start;
	uint64_t lba_end;
	uint64_t flags;
	uint16_t name36[36];
} gpt_partition;

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

static void copy(FILE *out, size_t inoff, size_t outoff, int len)
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

int main_mbr(int argc, char *argv[])
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
		fprintf(stderr, "p[%d]: start %d end %d active %X type %X\n", i, le32_to_cpu(presentlayout[i].start),
			le32_to_cpu(presentlayout[i].start) + le32_to_cpu(presentlayout[i].length) - 1, presentlayout[i].active,
			presentlayout[i].type);
	}
	fprintf(stderr, "new layout\n");
	for (i = 0; i < 4; i++) {
		fprintf(stderr, "p[%d]: start %d end %d active %X type %X\n", i, le32_to_cpu(newlayout[i].start),
			le32_to_cpu(newlayout[i].start) + le32_to_cpu(newlayout[i].length) - 1, newlayout[i].active,
			newlayout[i].type);
	}
	struct pte *nvram = &presentlayout[2];
	fseek(out, le32_to_cpu(nvram->start) * 512, SEEK_SET);
	uint32_t nvlen = nvram->length * 512;
	if (nvlen) {
		if (le32_to_cpu(nvram->start) == le32_to_cpu(newlayout[2].start)) {
			if (le32_to_cpu(nvram->length) > le32_to_cpu(newlayout[2].length)) {
				// if old nvram partition size is bigger than the new partition to be written, we keep the old partition entry as is
				memcpy(&newlayout[2], &nvram, sizeof(struct pte));
				fseek(out, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
				fseek(in, MBR_PARTITION_ENTRY_OFFSET, SEEK_SET);
				fwrite(&newlayout, sizeof(struct pte), MBR_ENTRY_MAX, in);
				fwrite(&newlayout, sizeof(struct pte), MBR_ENTRY_MAX, out);
			}
		} else {
			fprintf(stderr, "read nvram from old offset %d with len %d and write to offset %d\n",
				le32_to_cpu(nvram->start) * 512, nvlen, le32_to_cpu(newlayout[2].start) * 512);
			copy(out, le32_to_cpu(nvram->start) * 512, le32_to_cpu(newlayout[2].start) * 512, nvlen);
		}
	}
	fseek(in, 0, SEEK_END);
	uint32_t len = ftell(in);
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
	//	if (newlayout[2].start > 0) {
	//		copy(out, nvram->start * 512, newlayout[2].start * 512, nvlen);
	//	}
	fclose(out);
	fclose(in);
	return 0;
}

int main_gpt(int argc, char *argv[])
{
	gpt_header header;
	gpt_header header2;
	gpt_partition *part;
	unsigned char *buf;
	unsigned char *buf2;
	FILE *in = fopen(argv[1], "r+b");
	FILE *out = fopen(argv[2], "r+b");
	fseek(in, 512, SEEK_SET);
	fseek(out, 512, SEEK_SET);
	fread(&header, sizeof(header), 1, in);
	fread(&header2, sizeof(header), 1, out);
	fseek(in, 512 * le64_to_cpu(header.first_part_lba), SEEK_SET);
	fseek(out, 512 * le64_to_cpu(header2.first_part_lba), SEEK_SET);
	uint32_t n_parts;
	uint32_t part_entry_len;

	buf = malloc(le32_to_cpu(header.n_parts) * le32_to_cpu(header.part_entry_len));
	fread(buf, le32_to_cpu(header.n_parts) * le32_to_cpu(header.part_entry_len), 1, in);

	buf2 = malloc(le32_to_cpu(header2.n_parts) * le32_to_cpu(header2.part_entry_len));
	fread(buf2, le32_to_cpu(header2.n_parts) * le32_to_cpu(header2.part_entry_len), 1, out);

	fprintf(stderr, "%d\n", le32_to_cpu(header2.part_entry_len));
	int i;
	fprintf(stderr, "old layout\n");
	for (i = 0; i < 4; i++) {
		part = (gpt_partition *)&buf2[i * le32_to_cpu(header2.part_entry_len)];
		fprintf(stderr, "p[%d]: start %lld end %lld flags %llX name %s\n", i, le64_to_cpu(part->lba_start),
			le64_to_cpu(part->lba_end), le64_to_cpu(part->flags), part->name36);
	}
	fprintf(stderr, "new layout\n");
	for (i = 0; i < 4; i++) {
		part = (gpt_partition *)&buf[i * le32_to_cpu(header.part_entry_len)];
		fprintf(stderr, "p[%d]: start %lld end %lld flags %llX name %s\n", i, le64_to_cpu(part->lba_start),
			le64_to_cpu(part->lba_end), le64_to_cpu(part->flags), part->name36);
	}
	gpt_partition *nvram = (gpt_partition *)&buf2[2 * le32_to_cpu(header2.part_entry_len)];
	gpt_partition *newnvram = (gpt_partition *)&buf[2 * le32_to_cpu(header.part_entry_len)];
	fseek(out, le64_to_cpu(nvram->lba_start) * 512, SEEK_SET);
	int64_t nvlen = (le64_to_cpu(nvram->lba_end) - le64_to_cpu(nvram->lba_start)) * 512;
	if (nvlen > 0) {
		if (nvram->lba_start == newnvram->lba_start) {
			if (le64_to_cpu(nvram->lba_end) > le64_to_cpu(newnvram->lba_end)) {
				// if old nvram partition size is bigger than the new partition to be written, we keep the old partition entry as is
				memcpy(newnvram, nvram, le32_to_cpu(header.part_entry_len));
				fseek(out, le32_to_cpu(header.part_entry_len) * 2, SEEK_SET);
				fwrite(newnvram, le32_to_cpu(header.part_entry_len), 1, out);
			}
		} else {
			fprintf(stderr, "read nvram from old offset %lld with len %lld and write to offset %lld\n",
				le64_to_cpu(nvram->lba_start) * 512, nvlen, le64_to_cpu(newnvram->lba_start) * 512);
			copy(out, le64_to_cpu(nvram->lba_start) * 512, le64_to_cpu(newnvram->lba_start) * 512, nvlen);
		}
	}
	fseek(in, 0, SEEK_END);
	uint32_t len = ftell(in);
	fseek(in, 0, SEEK_SET);
	fseek(out, 0, SEEK_SET);
	char *sbuf = malloc(65536);
	int count = len / 65536;
	fprintf(stderr, "write image len = %d\n", len);
	for (i = 0; i < count; i++) {
		fread(sbuf, 65536, 1, in);
		fwrite(sbuf, 65536, 1, out);
	}
	fread(sbuf, len % 65536, 1, in);
	fwrite(sbuf, len % 65536, 1, out);
	free(sbuf);
	fclose(out);
	fclose(in);
	return 0;
}
int main(int argc, char *argv[])
{
	gpt_header header;
	FILE *in = fopen(argv[1], "r+b");
	fseek(in, 512, SEEK_SET);
	fread(&header, sizeof(header), 1, in);
	fclose(in);
	if (header.magic == le64_to_cpu(GPT_MAGIC))
		return main_gpt(argc, argv);
	else
		return main_mbr(argc, argv);
}