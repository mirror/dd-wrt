#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *in, *out;
	size_t len;
	int i;
	int datalen = atoi(argv[1]);
	for (i = 2; i < argc; i++) {
		in = fopen(argv[i], "rb");
		fseek(in, 0, SEEK_END);
		len = ftell(in);
		rewind(in);
		unsigned char *mem = malloc(len);
		fread(mem, len, 1, in);
		fclose(in);
		out = fopen(argv[i], "wb");
		int a;
		int b;
		for (b = 0; b < datalen; b++) {
			for (a = 0; a < len; a += (datalen * 2)) {
				putc(mem[a + b], out);
				putc(mem[a + datalen + b], out);
			}
		}
		fclose(out);
		free(mem);
	}
}
