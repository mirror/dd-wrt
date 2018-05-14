#ifndef BASE_FS_H
# define BASE_FS_H

# include "fsmap.h"
# include "hashmap.h"
# include "block_range.h"

struct basefs_entry {
	char *path;
	struct block_range *head;
	struct block_range *tail;
};

extern struct fsmap_format base_fs_format;

struct hashmap *basefs_parse(const char *file, const char *mountpoint);

#endif /* !BASE_FS_H */
