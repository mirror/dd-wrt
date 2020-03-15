/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _DIR_H
#define _DIR_H

#include <apfs/types.h>

extern void make_special_dir_dentry(u64 ino, char *name,
				    struct apfs_kvloc **next_toc,
				    void *key_area, void **key,
				    void *val_area_end, void **val_end);
extern void make_special_dir_inode(u64 ino, char *name,
				   struct apfs_kvloc **next_toc,
				   void *key_area, void **key,
				   void *val_area_end, void **val_end);

#endif	/* _DIR_H */
