#include <stdio.h>

#define FILTER(a) if (!strncmp(&mem[i],a,strlen(a))) \
		{ \
		fwrite(&mem[i],strlen(a),1,fp); \
		i+=strlen(a); \
		while(i<l && mem[i]!=0xa) { \
		    putc(mem[i++],fp); \
		} \
		putc(mem[i],fp); \
		continue; \
		} \

#define FILTER_pre(a) if (!strncmp(&mem[i],a,strlen(a))) \
		{ \
		putc(0xa,fp); \
		fwrite(&mem[i],strlen(a),1,fp); \
		i+=strlen(a); \
		while(i<l && mem[i]!=0xa) { \
		    putc(mem[i++],fp); \
		} \
		putc(mem[i],fp); \
		continue; \
		} \


void main(int argc, char *argv[])
{
	FILE *fp;
	char *mem;
	fp = fopen(argv[1], "rb");
	fseek(fp, 0, SEEK_END);
	long l;
	l = ftell(fp);
	rewind(fp);
	mem = malloc(l + 16);
	fread(mem, l, 1, fp);
	fclose(fp);
	fp = fopen(argv[1], "wb");
	int i;
	for (i = 0; i < l; i++) {
		FILTER_pre("//<![CDATA[");
		FILTER_pre("//]]>");
//		if (!strncmp(&mem[i], "//", 2)) {
//			while (mem[i] != 0xa) {
//				i++;
//			}
//		}
//		if (!memcmp(&mem[i], "else\n", 5)) {
//			i += 5;
//			fprintf(fp, "else ");
//		}
//		if (!memcmp(&mem[i], "else\r\n", 6)) {
//			i += 6;
//			fprintf(fp, "else ");
//		}
		if (i>=l)
		    break;
		if (mem[i] != '\r' && mem[i] != '\n' && mem[i] != '\t' && mem[i] != '\f')
			putc(mem[i], fp);
	}
	fclose(fp);
}
