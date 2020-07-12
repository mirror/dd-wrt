#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **syms;
void readsymbols(void)
{
	char addr[64];
	char type[64];
	char sym[256];
	syms = (char **)malloc(sizeof(char *) * 10000);
	FILE *fp = fopen("symbols", "rb");
	int cnt = 0;
	int valid = 0;
	while (!feof(fp)) {
		if (!valid) {
		    fscanf(fp, "%s", type);
		    if (!strcmp(type,"T"))
			valid++;
		}
		if (valid) {
		    fscanf(fp,"%s", sym);
			syms[cnt++] = strdup(sym);
			fprintf(stdout, "%s\n", sym);
			valid=0;
		}
	}
	syms[cnt] = NULL;
}

int sym(char *name, char *prefix, char *postfix)
{
	int i = 0;
	char check[256];
	sprintf(check, "%s%s%s%s%s", prefix ? prefix : "", prefix ? "_" : "", name, postfix ? "_" : "", postfix ? postfix : "");
	while (syms[i]) {
		if (!strcmp(syms[i++], check))
			return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	FILE *out = fopen("main.c", "wb");
	int i=0;
	readsymbols();
	while (syms[i]) {
		if (!strncmp(syms[i],"start_", 6)) {
			fprintf(stdout, "process %s\n",syms[i]);
			char *name = syms[i] + 6;
			int deps = sym(name, NULL, "deps");
			int proc = sym(name, NULL, "proc");
			fprintf(out,"void start_%s(void);\n",name);
			if (deps)
			fprintf(out,"char *%s_deps(void);\n",name);
			if (proc)
			fprintf(out,"char *%s_proc(void);\n",name);
			
		} else if (!strncmp(syms[i],"stop_", 5)) {
			fprintf(out,"void stop_%s(void);\n",syms[i]+5);
		} else {
			char copy[256];
			strcpy(copy, syms[i]);
			char *p = strstr(copy, "_main");
			if (p) {
				*p = 0;
				if (sym(copy, NULL, "main"))
					fprintf(out,"int %s_main(int argc,char *argv[]);\n", copy);
			}
		}
		i++;
	}
	fprintf(out, "#include <string.h>\n");
	fprintf(out, "#include \"genmain.h\"\n");
	fprintf(out, "int main(int argc,char *argv[]){\n");
	fprintf(out, "int force;\n");
	fprintf(out, "if (argc>3 && !strcmp(argv[3],\"-f\")) force = 1;\n");
	
	i=0;
	fprintf(out, "if (!strcmp(argv[2],\"start\")) {\n");
	while (syms[i]) {
		if (!strncmp(syms[i],"start_", 6)) {
			fprintf(stdout, "process %s\n",syms[i]);
			char *name = syms[i] + 6;
			int deps = sym(name, NULL, "deps");
			int proc = sym(name, NULL, "proc");
			if (deps && proc)
				fprintf(out, "HANDLE_START_DEPS_PROC(\"%s\",%s);\n",name,name);
			else if (deps)
				fprintf(out, "HANDLE_START_DEPS(\"%s\",%s);\n",name,name);
			else if (proc)
				fprintf(out, "HANDLE_START_PROC(\"%s\",%s);\n",name,name);
			else
				fprintf(out, "HANDLE_START(\"%s\",%s);\n",name,name);
		}
		i++;
	}
	i=0;
	fprintf(out, "}\n");
	fprintf(out, "if (!strcmp(argv[2],\"stop\")) {\n");
	while (syms[i]) {
		fprintf(stdout, "process %s\n",syms[i]);
		if (!strcmp(syms[i],"stop_process")) {
			i++;
			continue;
		}
		if (!strncmp(syms[i],"stop_", 5)) {
			fprintf(out, "HANDLE_STOP(\"%s\",%s);\n",syms[i]+5,syms[i]+5);
		} 
		i++;
	}
	i=0;
	fprintf(out, "}\n");
	fprintf(out, "if (!strcmp(argv[2],\"main\")) {\n");
	fprintf(out, "char **args = buildargs(argc,argv);\n");
	while (syms[i]) {
			char copy[256];
			strcpy(copy, syms[i]);
			char *p = strstr(copy, "_main");
			if (p) {
				*p = 0;
				printf("check %s\n",copy);
				if (sym(copy, NULL, "main"))
					fprintf(out, "HANDLE_MAIN(\"%s\", %s);\n", copy, copy);
			}
		i++;
	}
	fprintf(out, "}\n");
	fprintf(out, "}\n");
	fclose(out);
}
