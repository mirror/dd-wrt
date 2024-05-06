#include "usr/command/common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "usr/log.h"

int load_pkt(char *filename, unsigned char **result, size_t *result_len)
{
	unsigned char *pkt;
	size_t pkt_len;
	FILE *file;
	int bytes_read;

	file = fopen(filename, "rb");
	if (!file) {
		pr_err("Could not open the file %s.", filename);
		return -EINVAL;
	}

	fseek(file, 0, SEEK_END);
	pkt_len = ftell(file);
	rewind(file);

	pkt = malloc(pkt_len);
	if (!pkt) {
		pr_err("Could not allocate the packet.");
		fclose(file);
		return -ENOMEM;
	}

	bytes_read = fread(pkt, 1, pkt_len, file);
	fclose(file);

	if (bytes_read != pkt_len) {
		pr_err("Reading error.");
		free(pkt);
		return -EINVAL;
	}

	*result = pkt;
	*result_len = pkt_len;
	return 0;
}
