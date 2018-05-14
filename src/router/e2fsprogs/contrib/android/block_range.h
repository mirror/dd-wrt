#ifndef BLOCK_RANGE_H
# define BLOCK_RANGE_H

# include <sys/types.h>
# include <ext2fs/ext2fs.h>

struct block_range {
	blk64_t start;
	blk64_t end;
	struct block_range *next;
};

void add_blocks_to_range(struct block_range **head, struct block_range **tail,
			 blk64_t blk_start, blk64_t blk_end);
void delete_block_ranges(struct block_range *head);
int write_block_ranges(FILE *f, struct block_range *range, char *sep);

#endif /* !BLOCK_RANGE_H */
