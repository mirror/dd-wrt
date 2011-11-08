/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
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

/*
 * Read/write patterns (default is always "forward")
 */
#define IO_RANDOM	( 0)
#define IO_FORWARD	( 1)
#define IO_BACKWARD	(-1)

/*
 * File descriptor options
 */
#define IO_READONLY	(1<<0)
#define IO_DIRECT	(1<<1)
#define IO_REALTIME	(1<<2)
#define IO_APPEND	(1<<3)
#define IO_OSYNC	(1<<4)
#define IO_CREAT	(1<<5)
#define IO_TRUNC	(1<<6)
#define IO_FOREIGN	(1<<7)
#define IO_NONBLOCK	(1<<8)

/*
 * Regular file I/O control
 */
typedef struct fileio {
	int		fd;		/* open file descriptor */
	int		flags;		/* flags describing file state */
	char		*name;		/* file name at time of open */
	xfs_fsop_geom_t	geom;		/* XFS filesystem geometry */
} fileio_t;

extern fileio_t		*filetable;	/* open file table */
extern int		filecount;	/* number of open files */
extern fileio_t		*file;		/* active file in file table */
extern int filelist_f(void);

/*
 * Memory mapped file regions
 */
typedef struct mmap_region {
	void		*addr;		/* address of start of mapping */
	size_t		length;		/* length of mapping */
	off64_t		offset;		/* start offset into backing file */
	int		prot;		/* protection mode of the mapping */
	char		*name;		/* name of backing file */
} mmap_region_t;

extern mmap_region_t	*maptable;	/* mmap'd region array */
extern int		mapcount;	/* #entries in the mapping table */
extern mmap_region_t	*mapping;	/* active mapping table entry */
extern int maplist_f(void);
extern void *check_mapping_range(mmap_region_t *, off64_t, size_t, int);

/*
 * Various xfs_io helper routines/globals
 */

extern off64_t		filesize(void);
extern int		openfile(char *, xfs_fsop_geom_t *, int, mode_t);
extern int		addfile(char *, int , xfs_fsop_geom_t *, int);
extern void		printxattr(uint, int, int, const char *, int, int);

extern unsigned int	recurse_all;
extern unsigned int	recurse_dir;

extern void		*buffer;
extern size_t		buffersize;
extern int		alloc_buffer(size_t, int, unsigned int);
extern int		read_buffer(int, off64_t, long long, long long *,
					int, int);
extern void		dump_buffer(off64_t, ssize_t);

extern void		attr_init(void);
extern void		bmap_init(void);
extern void		file_init(void);
extern void		freeze_init(void);
extern void		fsync_init(void);
extern void		getrusage_init(void);
extern void		help_init(void);
extern void		imap_init(void);
extern void		inject_init(void);
extern void		mmap_init(void);
extern void		open_init(void);
extern void		parent_init(void);
extern void		pread_init(void);
extern void		prealloc_init(void);
extern void		pwrite_init(void);
extern void		quit_init(void);
extern void		shutdown_init(void);
extern void		truncate_init(void);

#ifdef HAVE_FADVISE
extern void		fadvise_init(void);
#else
#define fadvise_init()	do { } while (0)
#endif

#ifdef HAVE_RESBLKS
extern void		resblks_init(void);
#else
#define resblks_init()	do { } while (0)
#endif

#ifdef HAVE_SENDFILE
extern void		sendfile_init(void);
#else
#define sendfile_init()	do { } while (0)
#endif

#ifdef HAVE_MADVISE
extern void		madvise_init(void);
#else
#define madvise_init()	do { } while (0)
#endif

#ifdef HAVE_MINCORE
extern void		mincore_init(void);
#else
#define mincore_init()	do { } while (0)
#endif

#ifdef HAVE_FIEMAP
extern void		fiemap_init(void);
#else
#define fiemap_init()	do { } while (0)
#endif
