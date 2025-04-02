#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *in, *out;
	size_t len;
	int i;
	for (i = 1; i < argc; i++) {
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
		for (b = 0; b < 16; b++)
			for (a = 0; a < len; a += 8) {
				putc(mem[a + b], out);
				putc(mem[a + 16 + b], out);
			}
		fclose(out);
		free(mem);
	}
}
