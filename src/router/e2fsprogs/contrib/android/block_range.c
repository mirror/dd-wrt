#define _GNU_SOURCE

#include "block_range.h"
#include <stdio.h>

struct block_range *new_block_range(blk64_t start, blk64_t end)
{
	struct block_range *range = malloc(sizeof(*range));
	range->start = start;
	range->end = end;
	range->next = NULL;
	return range;
}

void add_blocks_to_range(struct block_range **head, struct block_range **tail,
			 blk64_t blk_start, blk64_t blk_end)
{
	if (*head == NULL)
		*head = *tail = new_block_range(blk_start, blk_end);
	else if ((*tail)->end + 1 == blk_start)
		(*tail)->end += (blk_end - blk_start + 1);
	else {
		struct block_range *range = new_block_range(blk_start, blk_end);
		(*tail)->next = range;
		*tail = range;
	}
}

void delete_block_ranges(struct block_range *head)
{
	struct block_range *tmp;

	while (head) {
		tmp = head->next;
		free(head);
		head = tmp;
	}
}

int write_block_ranges(FILE *f, struct block_range *range,
				     char *sep)
{
	int len;
	char *buf;

	while (range) {
		if (range->start == range->end)
			len = asprintf(&buf, "%llu%s", range->start, sep);
		else
			len = asprintf(&buf, "%llu-%llu%s", range->start,
				       range->end, sep);
		if (fwrite(buf, 1, len, f) != (size_t)len) {
			free(buf);
			return -1;
		}
		free(buf);
		range = range->next;
	}

	len = strlen(sep);
	if (fseek(f, -len, SEEK_CUR) == -len)
		return -1;
	return 0;
}
