#include "seqnum.h"

/**
 * A trivial function that reads kernel seqnum from sysfs.
 *
 * Returns: Seqnum as read from sysfs
 */
int seqnum_get(event_seqnum_t *value) {
	char seqnum[64];
	FILE *fp;
	
	fp = fopen(SYSFS_SEQNUM_PATH, "r");
	if (fp == NULL)
		return -1;
	
	fread(seqnum, 1, 64, fp);
	fclose(fp);

	*value = strtoull(seqnum, NULL, 0);
	
	return 0;
}
