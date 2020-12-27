/*
this stupid and bad coded tool generates a symbol list for used ej function out of the webfiles and the symbol table.
the generated header allows us to strip out unused code from the web code

(c) 2020 - Sebastian Gottschall / NewMedia-NET GmbH
*/

#include  <stdio.h>
#include  <malloc.h>
#include  <string.h>

int tlen = 0;
unsigned char *table[10000];
int checktable(char *name)
{
	int i;
	for (i = 0; i < tlen; i++) {
		if (!strcmp(table[i], name))
			return 1;
	}
	table[tlen] = strdup(name);
	tlen++;
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *in;
	in = fopen("www.renew", "rb");
	if (!in)
	    in = fopen("www", "rb");
	fseek(in, 0, SEEK_END);
	size_t len = ftell(in);
	rewind(in);
	char *mem = malloc(len);
	fread(mem, 1, len, in);
	fclose(in);
	in = fopen("ejsyms", "rb");
	fseek(in, 0, SEEK_END);
	size_t ejlen = ftell(in);
	rewind(in);
	char *syms = malloc(ejlen);
	fread(syms, 1, ejlen, in);
	fclose(in);
	int i;
	for (i = 0; i < len; i++) {
		if (mem[i] == '<' && mem[i + 1] == '%' && mem[i + 2] == ' ') {
			int a = 0;
			for (a = i + 3; a < i + 64; a++) {
				if (!strncmp(&mem[a], " %>", 3)) {
					char name[64];
					char *cut = strstr(&mem[i + 3], "(");
					char *p = &mem[i + 3];
					strncpy(name, &mem[i + 3], cut - p);
					name[cut - p] = 0;
					char ejname[64];
					sprintf(ejname, "ej_%s", name);
					if (!strstr(syms, ejname))
						fprintf(stderr, "%s is missing, we ignore it\n", ejname);
					if (!checktable(name) && strstr(syms, ejname))
						printf("{\"%s\",&ej_%s},\n", name, name);
					goto next;
				}
			}
		}

	      next:;
	}
}
