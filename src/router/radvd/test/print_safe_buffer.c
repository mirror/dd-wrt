
#include "../config.h"
#include "../radvd.h"
#include "print_safe_buffer.h"

#include <stdio.h>

void print_safe_buffer(struct safe_buffer * sb)
{
	printf("unsigned char expected[] = {");

	for (size_t i = 0; i < sb->used; ++i) {
		if (i % 8 == 0) {
			printf("\n\t0x%02x,", sb->buffer[i]);
		} else {
			printf(" 0x%02x,", sb->buffer[i]);
		}
	}

	printf("\n};\n");
}

