#include <stdio.h>

#include "blobmsg.h"

/* chunks of 64KB to be added to blob-buffer */
#define BUFF_SIZE	0x10000
/* exceed maximum blob buff-length */
#define BUFF_CHUNKS	(((BLOB_ATTR_LEN_MASK + 1) / BUFF_SIZE) + 1)

int main(int argc, char **argv)
{
	int i;
	static struct blob_buf buf;
	blobmsg_buf_init(&buf);
	int prev_len = buf.buflen;

	for (i = 0; i < BUFF_CHUNKS; i++) {
		struct blob_attr *attr = blob_new(&buf, 0, BUFF_SIZE);
		if (!attr) {
			fprintf(stderr, "SUCCESS: failed to allocate attribute\n");
			break;
		}
		if (prev_len < buf.buflen) {
			prev_len = buf.buflen;
			continue;
		}
		fprintf(stderr, "ERROR: buffer length did not increase\n");
		return -1;
	}
	return 0;
}
