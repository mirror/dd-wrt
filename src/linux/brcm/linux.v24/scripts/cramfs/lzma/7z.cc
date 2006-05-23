#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "7z.h"
#include "cmdline.h"

int main()
{
char *in_buf, *out_buf;
unsigned in_size, out_size;

in_buf = strdup("Busybox");
in_size = strlen(in_buf);
out_size = 4096;
out_buf = (char *)calloc(1, out_size);

if (!compress_lzma_7z(1, 1 << 20, 32, in_buf, in_size, out_buf, &out_size))
	{
	fprintf(stderr, "error\n");
	abort();
	}
{
FILE *f = fopen("foo", "w");
fwrite(out_buf, 1, out_size, f);
fclose(f);
}

return 0;
}


#if 0
int main(int argc, char** argv) {
	gengetopt_args_info ai;
	unsigned int lzma_algo = 1;
	unsigned int lzma_dictsize = 1 << 20;
	unsigned int lzma_fastbytes = 32;

	//if (cmdline_parser(argc, argv, &ai) != 0) exit(1);

	if (ai.dictsize_given) {
		lzma_dictsize = 1<<ai.dictsize_arg;
	}

	if (ai.fastbytes_given) {
		lzma_fastbytes = ai.fastbytes_arg;
		if (lzma_fastbytes > 255) {
			fprintf(stderr, "fastbytes has to be <= 255");
			exit(1);
		}
	}

	if (ai.high_given) {
		lzma_algo = 2;
	}
#if 0
	if (ai.decode_given)
	{

		if (!decompress_lzma_7z())
		{
			fprintf(stderr, "error\n");
		}
	}
	else
#endif
	{
		if (!compress_lzma_7z(lzma_algo, lzma_dictsize, lzma_fastbytes))
		{
			fprintf(stderr, "error\n");
		}
	}
}
#endif
