/*
 * Copyright (c) 2012 Red Hat, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef XFS_SPACEMAN_SPACE_H_
#define XFS_SPACEMAN_SPACE_H_

typedef struct fileio {
	xfs_fsop_geom_t	geom;		/* XFS filesystem geometry */
	struct fs_path	fs_path;	/* XFS path information */
	char		*name;		/* file name at time of open */
	int		fd;		/* open file descriptor */
} fileio_t;

extern fileio_t		*filetable;	/* open file table */
extern int		filecount;	/* number of open files */
extern fileio_t		*file;		/* active file in file table */

extern int	openfile(char *, xfs_fsop_geom_t *, struct fs_path *);
extern int	addfile(char *, int , xfs_fsop_geom_t *, struct fs_path *);

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

#endif /* XFS_SPACEMAN_SPACE_H_ */
