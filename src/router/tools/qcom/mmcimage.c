#include <stdio.h>
#include <string.h>

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

	memset(devname, 0, sizeof(devname));
	sprintf(devname, argv[1]);
	fwrite(devname, sizeof(devname), 1, out);
	putc(0, out);
	putc(0, out);
	putc(0, out);
	putc(0, out);
	putc(2, out); // num of partitions
	fseek(in, 0, SEEK_END);
	unsigned long long len, len2;
	len = ftell(in);
	int i;
	rewind(in);
	memset(name, 0, sizeof(name));
	sprintf(name, argv[2]);
	fwrite(name, sizeof(name), 1, out);
	fwrite(&len, 8, 1, out);

	fseek(in2, 0, SEEK_END);
	len2 = ftell(in2);
	rewind(in2);
	memset(name, 0, sizeof(name));
	sprintf(name, argv[4]);
	fwrite(name, sizeof(name), 1, out);

	fwrite(&len2, 8, 1, out);

	for (i = 0; i < len; i++)
		putc(getc(in), out);
	for (i = 0; i < len2; i++)
		putc(getc(in2), out);
	fclose(in);
	fclose(in2);
	fclose(out);
	return 0;
}
