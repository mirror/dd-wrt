// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012 Red Hat, Inc.
 * All Rights Reserved.
 */
#ifndef XFS_SPACEMAN_SPACE_H_
#define XFS_SPACEMAN_SPACE_H_

struct fileio {
	struct xfs_fd	xfd;		/* XFS runtime support context */
	struct fs_path	fs_path;	/* XFS path information */
	char		*name;		/* file name at time of open */
};

extern struct fileio	*filetable;	/* open file table */
extern int		filecount;	/* number of open files */
extern struct fileio	*file;		/* active file in file table */

extern int	openfile(char *path, struct xfs_fd *xfd,
			 struct fs_path *fs_path);
extern int	addfile(char *path, struct xfs_fd *xfd,
			struct fs_path *fs_path);

extern void	print_init(void);
extern void	help_init(void);
extern void	prealloc_init(void);
extern void	quit_init(void);
extern void	trim_init(void);
#ifdef HAVE_GETFSMAP
extern void	freesp_init(void);
#else
# define freesp_init()	do { } while (0)
#endif
extern void	info_init(void);
extern void	health_init(void);

#endif /* XFS_SPACEMAN_SPACE_H_ */
