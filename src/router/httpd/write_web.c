#include <stdio.h>
typedef struct {
	char *path;		/* Web page URL path */
	unsigned int offset;	/* Web page data */
	int size;		/* Size of web page in bytes */
//  int csize;                  /* Size of web page in bytes */
} websRomPageIndexType;

#define WEBS_PAGE_ROM
#include "html.c"

int main(int argc, char *argv[])
{
	FILE *out;
	out = fopen(argv[1], "wb");
	fwrite(pages, sizeof(pages), 1, out);
	fclose(out);
}
