#include "coldplug2.h"

/**
 * Function supplementing 'echo add > uevent'
 *
 * @1 File to be written
 *
 * Returns: 0 on success, -1 on failure.
 */
static int coldplug2_uevent(const char *filename, const char *event) {
	FILE *fp;
	size_t event_s;
	size_t written;

	event_s = strlen(event);

	fp = fopen(filename, "w");
	if (fp == NULL)
		return -1;
	written = fwrite(event, event_s, 1, fp);
	fclose(fp);

	return (written == 1) ? 0 : -1;
}

/**
 *
 */
static int 

/**
 * Recurrent attempt to find all 'uevent' files. Does not
 * follow symlinks.
 *
 * @1 Base directory
 * 
 * Returns: 0 on success, -1 on failure.
 */
static int coldplug2_uevent_lookup(const char *basepath) {
}

int main(int argc, char *argv[]) {
	char basedir[PATH_MAX];
	int i;
	size_t sysfs_path_len;

	if (argc < 2) {
		/*
		 * No arguments? Then scan entire sysfs.
		 */
		coldplug2_uevent_lookup(SYSFS_PATH);
	} else {
		/* If user asked for help, provide it. */
		if (strcmp(argv[1], "-h") == 0) {
			printf("Usage: %s [<directory1> [<directory2> [...]]]\n\n", argv[0]);
			printf("Execution without any directories specified will scan entire /sys.\n");
			printf("You may specify list of directories, such as 'bus/pci' and 'block' instead.\n");
			return 1;
		}

		/*
		 * Specification of what to actually query was given.
		 */
		sysfs_path_len = strlen(SYSFS_PATH);
		if (sysfs_path_len > PATH_MAX)
			return -1;
		strcpy(basedir, SYSFS_PATH);
		strncat(basedir, "/", PATH_MAX-sysfs_path_len);

		for (i = 1; i < argc; i++) {
			strncat(basedir, argv[i], PATH_MAX-sysfs_path_len-1);
			coldplug2_uevent_lookup(SYSFS_PATH);
			basedir[sysfs_path_len+1] = '\0';
		}
	}

	return 0;
}
