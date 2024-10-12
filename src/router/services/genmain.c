/*
 * genmain.c - generates main.c for service handling
 *
 * Copyright (C) 2020 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **syms;

int strc(char *s1, char *s2)
{
	int i;
	if (!strncmp(s1, "start_", 6))
		s1 = s1 + 6;
	if (!strncmp(s1, "restart_", 8))
		s1 = s1 + 8;
	if (!strncmp(s1, "stop_", 5))
		s1 = s1 + 5;
	if (!strncmp(s2, "start_", 6))
		s2 = s2 + 6;
	if (!strncmp(s2, "stop_", 5))
		s2 = s2 + 5;
	if (!strncmp(s2, "restart_", 8))
		s2 = s2 + 8;

	int len = strlen(s1);
	int len2 = strlen(s2);
	if (len2 < len)
		len = len2;
	for (i = 0; i < len; i++) {
		if (s1[i] != s2[i])
			return s1[i] - s2[i];
	}
	return 0;
}

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
			if (!strcmp(type, "T"))
				valid++;
		}
		if (valid) {
			fscanf(fp, "%s", sym);
			syms[cnt++] = strdup(sym);
			fprintf(stdout, "%s\n", sym);
			valid = 0;
		}
	}
	syms[cnt] = NULL;
	int i, a;
	for (a = 0; a < cnt; a++) {
		for (i = 0; i < cnt - 1; i++) {
			int r = strc(syms[i], syms[i + 1]);
			if (r > 0) {
				char *b = syms[i + 1];
				syms[i + 1] = syms[i];
				syms[i] = b;
			}
		}
	}
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

char **list = NULL;
int inlist(char *name)
{
	if (!list) {
		list = (char **)malloc(sizeof(char *) * 10000);
		memset(list, 0, sizeof(char *) * 10000);
	}
	int cnt = 0;
	while (list[cnt]) {
		if (!strcmp(list[cnt++], name))
			return cnt - 1;
	}
	list[cnt] = strdup(name);
	return -1;
}

int main(int argc, char *argv[])
{
	FILE *out = fopen("main.c", "wb");
	FILE *s = fopen("services.txt","wb");
	int i = 0;
	readsymbols();
	fprintf(out, "/* generated - do not edit */\n");
	fprintf(out, "#include <string.h>\n");
	while (syms[i]) {
		char copy[256];
		strcpy(copy, syms[i]);
		char *p;
		if (!strcmp(syms[i], "stop_process")) {
			i++;
			continue;
		}
		if (!strcmp(syms[i], "stop_process_timeout")) {
			i++;
			continue;
		}
		if (!strcmp(syms[i], "stop_process_hard")) {
			i++;
			continue;
		}
		if (!strncmp(syms[i], "start_", 6)) {
			char *name = syms[i] + 6;
			int deps = sym(name, NULL, "deps");
			int proc = sym(name, NULL, "proc");
			fprintf(out, "void start_%s(void);\n", name);
			fprintf(s,"DECLARE_SERVICE(%s);\n", name);
			if (deps)
				fprintf(out, "char *%s_deps(void);\n", name);
			if (proc)
				fprintf(out, "char *%s_proc(void);\n", name);

		} else if (!strncmp(syms[i], "stop_", 5)) {
			fprintf(out, "void stop_%s(void);\n", syms[i] + 5);
		} else if ((p = strstr(copy, "_main"))) {
			*p = 0;
			if (sym(copy, NULL, "main"))
				fprintf(out, "int %s_main(int argc,char *argv[]);\n", copy);
		} else if (!strncmp(syms[i], "restart_", 8)) {
			fprintf(out, "void restart_%s(void);\n", syms[i] + 8);
		}
		i++;
	}
	fprintf(out, "struct fn {\n");
	fprintf(out, "char *name;\n");
	fprintf(out, "void (*start)(void);\n");
	fprintf(out, "char  * (*deps)(void);\n");
	fprintf(out, "char  * (*proc)(void);\n");
	fprintf(out, "void (*stop)(void);\n");
	fprintf(out, "int  (*main)(int argc,char *argv[]);\n");
	fprintf(out, "void (*restart)(void);\n");
	fprintf(out, "};\n");
	fprintf(out, "struct fn functiontable[]={\n");
	i = 0;
	while (syms[i]) {
		char copy[256];
		strcpy(copy, syms[i]);
		char *p;
		if (!strncmp(syms[i], "start_", 6)) {
			if (inlist(syms[i] + 6) == -1) {
				fprintf(out, "{\"%s\"", syms[i] + 6);
				int stop = sym(syms[i] + 6, "stop", NULL);
				int main = sym(syms[i] + 6, NULL, "main");
				int proc = sym(syms[i] + 6, NULL, "proc");
				int deps = sym(syms[i] + 6, NULL, "deps");
				int restart = sym(syms[i] + 6, "restart", NULL);
				fprintf(out, ",\tstart_%s", syms[i] + 6);

				if (deps)
					fprintf(out, ",\t%s_deps", syms[i] + 6);
				else
					fprintf(out, ",\tNULL");
				if (proc)
					fprintf(out, ",\t%s_proc", syms[i] + 6);
				else
					fprintf(out, ",\tNULL");
				if (stop)
					fprintf(out, ",\tstop_%s", syms[i] + 6);
				else
					fprintf(out, ",\tNULL");
				if (main)
					fprintf(out, ",\t%s_main", syms[i] + 6);
				else
					fprintf(out, ",\tNULL");
				if (restart)
					fprintf(out, ",\trestart_%s", syms[i] + 6);
				else
					fprintf(out, ",\tNULL");
				fprintf(out, "},\n");
			}
		} else if (!strncmp(syms[i], "stop_", 5)) {
			if (!strcmp(syms[i], "stop_process")) {
				i++;
				continue;
			}
			if (!strcmp(syms[i], "stop_process_timeout")) {
				i++;
				continue;
			}
			if (!strcmp(syms[i], "stop_process_hard")) {
				i++;
				continue;
			}
			if (inlist(syms[i] + 5) == -1) {
				fprintf(out, "{\"%s\"", syms[i] + 5);

				int start = sym(syms[i] + 5, "start", NULL);
				int main = sym(syms[i] + 5, NULL, "main");
				int proc = sym(syms[i] + 5, NULL, "proc");
				int deps = sym(syms[i] + 5, NULL, "deps");
				int restart = sym(syms[i] + 5, "restart", NULL);
				if (start)
					fprintf(out, ",\tstart_%s", syms[i] + 5);
				else
					fprintf(out, ",\tNULL");
				if (deps)
					fprintf(out, ",\t%s_deps", syms[i] + 5);
				else
					fprintf(out, ",\tNULL");
				if (proc)
					fprintf(out, ",\t%s_proc", syms[i] + 5);
				else
					fprintf(out, ",\tNULL");

				fprintf(out, ",\tstop_%s", syms[i] + 5);
				if (main)
					fprintf(out, ",\t%s_main", syms[i] + 5);
				else
					fprintf(out, ",\tNULL");

				if (restart)
					fprintf(out, ",\trestart_%s", syms[i] + 5);
				else
					fprintf(out, ",\tNULL");
				fprintf(out, "},\n");
			}
		} else if ((p = strstr(copy, "_main"))) {
			*p = 0;
			if (sym(copy, NULL, "main")) {
				if (inlist(copy) == -1) {
					fprintf(out, "{\"%s\"", copy);
					int start = sym(copy, "start", NULL);
					int stop = sym(copy, "stop", NULL);
					int main = sym(copy, NULL, "main");
					int proc = sym(copy, NULL, "proc");
					int deps = sym(copy, NULL, "deps");
					int restart = sym(copy, "restart", NULL);
					if (start)
						fprintf(out, ",\tstart_%s", copy);
					else
						fprintf(out, ",\tNULL");
					if (deps)
						fprintf(out, ",\t%s_deps", copy);
					else
						fprintf(out, ",\tNULL");
					if (proc)
						fprintf(out, ",\t%s_proc", copy);
					else
						fprintf(out, ",\tNULL");
					if (stop)
						fprintf(out, ",\tstop_%s", copy);
					else
						fprintf(out, ",\tNULL");
					if (main)
						fprintf(out, ",\t%s_main", copy);
					else
						fprintf(out, ",\tNULL");
					if (restart)
						fprintf(out, ",\trestart_", copy);
					else
						fprintf(out, ",\tNULL");
					fprintf(out, "},\n");
				}
			}
		} else if (!strncmp(syms[i], "restart_", 8)) {
			if (!strcmp(syms[i], "stop_process")) {
				i++;
				continue;
			}
			if (!strcmp(syms[i], "stop_process_timeout")) {
				i++;
				continue;
			}
			if (!strcmp(syms[i], "stop_process_hard")) {
				i++;
				continue;
			}
			if (inlist(syms[i] + 8) == -1) {
				fprintf(out, "{\"%s\"", syms[i] + 8);

				int start = sym(syms[i] + 8, "start", NULL);
				int stop = sym(syms[i] + 8, "stop", NULL);
				int main = sym(syms[i] + 8, NULL, "main");
				int proc = sym(syms[i] + 8, NULL, "proc");
				int deps = sym(syms[i] + 8, NULL, "deps");
				if (start)
					fprintf(out, ",\tstart_%s", syms[i] + 8);
				else
					fprintf(out, ",\tNULL");
				if (deps)
					fprintf(out, ",\t%s_deps", syms[i] + 8);
				else
					fprintf(out, ",\tNULL");
				if (proc)
					fprintf(out, ",\t%s_proc", syms[i] + 8);
				else
					fprintf(out, ",\tNULL");

				if (stop)
					fprintf(out, ",\tstop_%s", syms[i] + 8);
				else
					fprintf(out, ",\tNULL");
				if (main)
					fprintf(out, ",\t%s_main", syms[i] + 8);
				else
					fprintf(out, ",\tNULL");

				fprintf(out, ",\trestart_%s", syms[i] + 8);
				fprintf(out, "},\n");
			}
		}
		i++;
	}
	fprintf(out, "};\n");

	fprintf(out, "#include \"genmain.h\"\n");

	fprintf(out, "int main(int argc,char *argv[]){\n");
	fprintf(out, "return check_arguments(argc, argv);\n");
#if 0
	fprintf(out, "int function;\n");
	fprintf(out, "check_arguments(argc, argv, &function);\n");
	i = 0;
	fprintf(out, "if (!strcmp(argv[2],\"start\")) {\n");
	while (syms[i]) {
		if (!strncmp(syms[i], "start_", 6)) {
			fprintf(stdout, "process %s\n", syms[i]);
			char *name = syms[i] + 6;
			int deps = sym(name, NULL, "deps");
			int proc = sym(name, NULL, "proc");
			if (deps && proc)
				fprintf(out, "HANDLE_START_DEPS_PROC(%d,%s);\n", inlist(name), name);
			else if (deps)
				fprintf(out, "HANDLE_START_DEPS(%d,%s);\n", inlist(name), name);
			else if (proc)
				fprintf(out, "HANDLE_START_PROC(%d,%s);\n", inlist(name), name);
			else
				fprintf(out, "HANDLE_START(%d,%s);\n", inlist(name), name);
		}
		i++;
	}
	i = 0;
	fprintf(out, "}\n");
	fprintf(out, "if (!strcmp(argv[2],\"stop\")) {\n");
	while (syms[i]) {
		fprintf(stdout, "process stop %s\n", syms[i]);
		if (!strcmp(syms[i], "stop_process")) {
			i++;
			continue;
		}
		if (!strcmp(syms[i], "stop_process_timeout")) {
			i++;
			continue;
		}
		if (!strcmp(syms[i], "stop_process_hard")) {
			i++;
			continue;
		}
		if (!strncmp(syms[i], "stop_", 5)) {
			fprintf(out, "HANDLE_STOP(%d,%s);\n", inlist(syms[i] + 5), syms[i] + 5);
		}
		i++;
	}
	i = 0;
	fprintf(out, "}\n");
	fprintf(out, "if (!strcmp(argv[2],\"restart\")) {\n");
	while (syms[i]) {
		fprintf(stdout, "process restart %s\n", syms[i]);
		if (!strcmp(syms[i], "stop_process")) {
			i++;
			continue;
		}
		if (!strcmp(syms[i], "stop_process_timeout")) {
			i++;
			continue;
		}
		if (!strcmp(syms[i], "stop_process_hard")) {
			i++;
			continue;
		}
		if (!strncmp(syms[i], "stop_", 5) && sym(syms[i] + 5, "start", NULL)) {
			fprintf(out, "HANDLE_RESTART(%d,%s);\n", inlist(syms[i] + 5), syms[i] + 5);
		}
		i++;
	}
	i = 0;
	fprintf(out, "}\n");
	fprintf(out, "if (!strcmp(argv[2],\"main\")) {\n");
	fprintf(out, "char **args = buildargs(argc,argv);\n");
	while (syms[i]) {
		char copy[256];
		strcpy(copy, syms[i]);
		char *p = strstr(copy, "_main");
		if (p) {
			*p = 0;
			if (sym(copy, NULL, "main"))
				fprintf(out, "HANDLE_MAIN(%d, %s);\n", inlist(copy), copy);
		}
		i++;
	}
	fprintf(out, "}\n");
	fprintf(out, "end(argv);\n");
#endif
	fprintf(out, "}\n");
	fclose(out);
}
