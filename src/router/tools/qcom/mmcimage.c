#include <stdio.h>
#include <string.h>
#include "../../shared/mmcimage.h"
/*
typedef struct FWPART {
  char name[32]; // name of partition. must be 0 terminated
  unsigned long long partsize; // partition size
} fwpart;
typedef struct FWHEADER {
    char devname[64]; // devicename. must be 0 terminated
    unsigned int flags; // flags (not used yet);
    unsigned int partnum; // number of partitions;
    fwpart partitions[];
} fwheader;
*/

int main(int argc, char *argv[])
{
	FILE *in, *in2, *out;
	in = fopen(argv[3], "rb");
	if (!in) {
		fprintf(stderr, "%s not found\n", argv[3]);
		return -1;
	}
	in2 = fopen(argv[5], "rb");
	if (!in2) {
		fprintf(stderr, "%s not found\n", argv[5]);
		return -1;
	}
	out = fopen(argv[6], "wb");
	if (!out) {
		fprintf(stderr, "%s not found\n", argv[6]);
		return -1;
	}
	char name[32];
	char devname[64];

	fwheader header;
	memset(&header, 0, sizeof(header));
	sprintf(header.devname, argv[1]);
	header.partnum = 2;
	fseek(in, 0, SEEK_END);
	unsigned long long len, len2;
	len = ftell(in);
	int i;
	rewind(in);
	sprintf(header.partitions[0].name, argv[2]);
	header.partitions[0].partsize = len;
	fseek(in2, 0, SEEK_END);
	len2 = ftell(in2);
	rewind(in2);
	sprintf(header.partitions[1].name, argv[4]);
	header.partitions[1].partsize=len;
	fwrite(&header, sizeof(header), 1, out);

	for (i = 0; i < len; i++)
		putc(getc(in), out);
	for (i = 0; i < len2; i++)
		putc(getc(in2), out);
	fclose(in);
	fclose(in2);
	fclose(out);
	return 0;
}
