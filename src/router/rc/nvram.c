
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <utils.h>

static void usage(void)
{
	fprintf(stderr, "usage: nvram [get name] [set name=value] [unset name] [show] [backup filename] [restore filename]\n");
	exit(0);
}

/* 
 * NVRAM utility 
 */
int main(int argc, char **argv)
{
	char *name, *value, buf[NVRAM_SPACE];
	int size;

	/* 
	 * Skip program name 
	 */
	--argc;
	++argv;

	if (!*argv)
		usage();

	/* 
	 * Process the remaining arguments. 
	 */
	for (; *argv; argv++) {
		if (!strncmp(*argv, "get", 3)) {
			if (*++argv) {
				if ((value = nvram_get(*argv)))
					puts(value);
			}
		} else if (!strncmp(*argv, "set", 3)) {
			if (*++argv) {
				strncpy(value = buf, *argv, sizeof(buf));
				name = strsep(&value, "=");
				nvram_set(name, value);
			}
		} else if (!strncmp(*argv, "unset", 5)) {
			if (*++argv)
				nvram_unset(*argv);
		} else if (!strncmp(*argv, "commit", 5)) {
			nvram_commit();
		} else if (!strncmp(*argv, "show", 4)
			   || !strncmp(*argv, "getall", 6)) {
			nvram_getall(buf, sizeof(buf));
			for (name = buf; *name; name += strlen(name) + 1)
				puts(name);
			size = sizeof(struct nvram_header) + (long)name - (long)buf;
			fprintf(stderr, "size: %d bytes (%d left)\n", size, NVRAM_SPACE - size);
		} else if (!strncmp(*argv, "backup", 6)) {
			if (*++argv) {
				int ret = nvram_backup(*argv);
				if (ret < 0) {
					fprintf(stderr, "can't write %s\n", *argv);
				}
			}
		} else if (!strncmp(*argv, "restore", 7)) {
			if (*++argv) {
				int ret = nvram_restore(*argv);
				if (ret == -1) {
					fprintf(stderr, "can't write %s\n", *argv);
				}
				if (ret == -2) {
					fprintf(stderr, "file %s broken\n", *argv);
				}
			}
		}
		if (!*argv)
			break;
	}

	return 0;
}
