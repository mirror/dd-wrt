
#include "print_safe_buffer.h"
#include "../config.h"
#include "../radvd.h"

#include <stdio.h>

void print_safe_buffer(struct safe_buffer const *sb)
{
	char buf[4096];
	snprint_safe_buffer(buf, sizeof(buf), sb);
	printf("%s", buf);
}

size_t snprint_safe_buffer(char *s, size_t size, struct safe_buffer const *sb)
{
	size_t count = 0;

	count += snprintf((s + count), (size - count), "unsigned char expected[] = { /* sb.allocated = %ld, sb.used = %ld */", sb->allocated, sb->used);

	for (size_t i = 0; i < sb->used; ++i) {
		if (i % 8 == 0) {
			count += snprintf((s + count), (size - count), "\n\t0x%02x,", sb->buffer[i]);
		} else {
			count += snprintf((s + count), (size - count), " 0x%02x,", sb->buffer[i]);
		}
	}
	/* Do not remove the final byte's comma. Only JSON requires the comma is
	 * removed, and this is not JSON. */

	count += snprintf((s + count), (size - count), "\n};\n");
	return count;
}
