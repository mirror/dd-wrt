/*
 * Copyright (c) 2000-2006 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __XFS_IRIX_H__
#define __XFS_IRIX_H__

#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <values.h>
#include <strings.h>
#include <inttypes.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/uuid.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/syssgi.h>
#include <sys/sysmacros.h>
#include <sys/fs/xfs_fsops.h>
#include <sys/fs/xfs_itable.h>

#define __int8_t	char
#define __int16_t	short
#define __uint8_t	unsigned char
#define __uint16_t	unsigned short
#define loff_t		off64_t
typedef off64_t		xfs_off_t;
typedef __int64_t	xfs_ino_t;
typedef __int32_t	xfs_dev_t;
typedef __int64_t	xfs_daddr_t;
typedef char*		xfs_caddr_t;

#define xfs_flock64	flock64
#define xfs_flock64_t	struct flock64

typedef struct xfs_error_injection {
	__int32_t	fd;
	__int32_t	errtag;
} xfs_error_injection_t;

/* --- xfs_fsop_*req - request data structures --- */

typedef struct xfs_fsop_bulkreq {
	ino64_t	*lastip;
	__int32_t	icount;
	xfs_bstat_t	*ubuffer;
	__int32_t	*ocount;
} xfs_fsop_bulkreq_t;

typedef struct xfs_fsop_handlereq {
	__u32		fd;		/* fd for FD_TO_HANDLE		*/
	void		*path;		/* user pathname		*/
	__u32		oflags;		/* open flags			*/
	void		*ihandle;	/* user supplied handle		*/
	__u32		ihandlen;	/* user supplied length		*/
	void		*ohandle;	/* user buffer for handle	*/
	__u32		*ohandlen;	/* user buffer length		*/
} xfs_fsop_handlereq_t;

typedef struct xfs_fsop_setdm_handlereq {
	struct xfs_fsop_handlereq	hreq;	/* handle information	*/
	struct fsdmidata		*data;	/* DMAPI data	*/
} xfs_fsop_setdm_handlereq_t;

typedef struct xfs_attrlist_cursor {
	__u32		opaque[4];
} xfs_attrlist_cursor_t;

typedef struct xfs_fsop_attrlist_handlereq {
	struct xfs_fsop_handlereq	hreq; /* handle interface structure */
	struct xfs_attrlist_cursor	pos; /* opaque cookie, list offset */
	__u32				flags;	/* which namespace to use */
	__u32				buflen;	/* length of buffer supplied */
	void				*buffer;	/* returned names */
} xfs_fsop_attrlist_handlereq_t;

typedef struct xfs_fsop_getparents_handlereq {
	struct xfs_fsop_handlereq	hreq; /* handle interface structure */
	struct xfs_attrlist_cursor	pos; /* opaque cookie, list offset */
	__u32				buflen;	/* length of buffer supplied */
	void				*buffer; /* returned data */
	__u32				*ocount; /* return number of links */
	__u32				*omore; /* return whether more to come */
} xfs_fsop_getparents_handlereq_t;

typedef struct xfs_attr_multiop {
	__u32		am_opcode;
	__s32		am_error;
	void		*am_attrname;
	void		*am_attrvalue;
	__u32		am_length;
	__u32		am_flags;
} xfs_attr_multiop_t;

typedef struct xfs_fsop_attrmulti_handlereq {
	struct xfs_fsop_handlereq	hreq; /* handle interface structure */
	__u32				opcount;/* count of following multiop */
	struct xfs_attr_multiop		*ops; /* attr_multi data */
} xfs_fsop_attrmulti_handlereq_t;

/* start doing packed stuctures here */
#define HAVE_FORMAT32	1
#pragma pack 1
typedef struct xfs_inode_log_format_32 {
	__u16			ilf_type;	/* inode log item type */
	__u16			ilf_size;	/* size of this item */
	__u32			ilf_fields;	/* flags for fields logged */
	__u16			ilf_asize;	/* size of attr d/ext/root */
	__u16			ilf_dsize;	/* size of data/ext/root */
	__u64			ilf_ino;	/* inode number */
	union {
		__u32		ilfu_rdev;	/* rdev value for dev inode*/
		uuid_t		ilfu_uuid;	/* mount point value */
	} ilf_u;
	__s64			ilf_blkno;	/* blkno of inode buffer */
	__s32			ilf_len;	/* len of inode buffer */
	__s32			ilf_boffset;	/* off of inode in buffer */
} xfs_inode_log_format_32_t;

typedef struct xfs_extent_32 {
	__u64	ext_start;
	__u32	ext_len;
} xfs_extent_32_t;

typedef struct xfs_efi_log_format_32 {
	__u16			efi_type;	/* efi log item type */
	__u16			efi_size;	/* size of this item */
	__u32			efi_nextents;	/* # extents to free */
	__u64			efi_id;		/* efi identifier */
	xfs_extent_32_t		efi_extents[1];	/* array of extents to free */
} xfs_efi_log_format_32_t;

typedef struct xfs_efd_log_format_32 {
	__u16			efd_type;	/* efd log item type */
	__u16			efd_size;	/* size of this item */
	__u32			efd_nextents;	/* # of extents freed */
	__u64			efd_efi_id;	/* id of corresponding efi */
	xfs_extent_32_t		efd_extents[1];	/* array of extents freed */
} xfs_efd_log_format_32_t;

#pragma pack 0
/* end of packed stuctures */

#include <sys/endian.h>
#define __BYTE_ORDER	BYTE_ORDER
#define __BIG_ENDIAN	BIG_ENDIAN
#define __LITTLE_ENDIAN	LITTLE_ENDIAN

/* Map some gcc macros for the MipsPRO compiler */
#ifndef __GNUC__
#define __builtin_constant_p(x)	(0)
#define __FUNCTION__	"XFS"
#define __sgi__		__sgi
#define __inline__	__inline
#define inline		__inline
#endif

#define constpp		char * const *

/*ARGSUSED*/
static __inline__ int xfsctl(const char *path, int fd, int cmd, void *arg)
{
	if (cmd >= 0 && cmd < XFS_FSOPS_COUNT) {
		/*
		 * We have a problem in that xfsctl takes 1 arg but
		 * some sgi xfs ops take an input arg and/or an output arg
		 * So have to special case the ops to decide if xfsctl arg
		 * is an input or an output argument.
		 */
		if (cmd == XFS_FS_GOINGDOWN)
			return syssgi(SGI_XFS_FSOPERATIONS, fd, cmd, arg, 0);
		return syssgi(SGI_XFS_FSOPERATIONS, fd, cmd, 0, arg);
	}
	switch (cmd) {
		case SGI_FS_INUMBERS:
		case SGI_FS_BULKSTAT:
			return syssgi(cmd, fd,
					((xfs_fsop_bulkreq_t*)arg)->lastip,
					((xfs_fsop_bulkreq_t*)arg)->icount,
					((xfs_fsop_bulkreq_t*)arg)->ubuffer,
					((xfs_fsop_bulkreq_t*)arg)->ocount);
		case SGI_FS_BULKSTAT_SINGLE:
			return syssgi(SGI_FS_BULKSTAT_SINGLE, fd,
					((xfs_fsop_bulkreq_t*)arg)->lastip,
					((xfs_fsop_bulkreq_t*)arg)->ubuffer);
		case SGI_XFS_INJECT_ERROR:
			return syssgi(SGI_XFS_INJECT_ERROR,
					((xfs_error_injection_t*)arg)->errtag,
					fd);
		case SGI_XFS_CLEARALL_ERROR:
			return syssgi(SGI_XFS_CLEARALL_ERROR, fd);
		case SGI_PATH_TO_HANDLE:
		case SGI_PATH_TO_FSHANDLE:
			return syssgi(cmd,
					((xfs_fsop_handlereq_t*)arg)->path,
					((xfs_fsop_handlereq_t*)arg)->ohandle,
					((xfs_fsop_handlereq_t*)arg)->ohandlen);
		case SGI_FD_TO_HANDLE:
			return syssgi(cmd,
					((xfs_fsop_handlereq_t*)arg)->fd,
					((xfs_fsop_handlereq_t*)arg)->ohandle,
					((xfs_fsop_handlereq_t*)arg)->ohandlen);
		case SGI_OPEN_BY_HANDLE:
			return syssgi(cmd,
					((xfs_fsop_handlereq_t*)arg)->ihandle,
					((xfs_fsop_handlereq_t*)arg)->ihandlen,
					((xfs_fsop_handlereq_t*)arg)->oflags);
		case SGI_READLINK_BY_HANDLE:
			return syssgi(cmd,
					((xfs_fsop_handlereq_t*)arg)->ihandle,
					((xfs_fsop_handlereq_t*)arg)->ihandlen,
					((xfs_fsop_handlereq_t*)arg)->ohandle,
					((xfs_fsop_handlereq_t*)arg)->ohandlen);
		case SGI_ATTR_LIST_BY_HANDLE:
			return syssgi(cmd,
					((xfs_fsop_attrlist_handlereq_t*)arg)->hreq.ihandle,
					((xfs_fsop_attrlist_handlereq_t*)arg)->hreq.ihandlen,
					((xfs_fsop_attrlist_handlereq_t*)arg)->buffer,
					((xfs_fsop_attrlist_handlereq_t*)arg)->buflen,
					((xfs_fsop_attrlist_handlereq_t*)arg)->flags,
					&(((xfs_fsop_attrlist_handlereq_t*)arg)->pos));
		case SGI_XFS_GETPARENTS:
		case SGI_XFS_GETPARENTPATHS:
			return syssgi(cmd,
					((xfs_fsop_getparents_handlereq_t*)arg)->hreq.ihandle,
					((xfs_fsop_getparents_handlereq_t*)arg)->hreq.ihandlen,
					((xfs_fsop_getparents_handlereq_t*)arg)->buffer,
					((xfs_fsop_getparents_handlereq_t*)arg)->buflen,
					&(((xfs_fsop_getparents_handlereq_t*)arg)->pos),
					((xfs_fsop_getparents_handlereq_t*)arg)->ocount,
					((xfs_fsop_getparents_handlereq_t*)arg)->omore);
		case SGI_ATTR_MULTI_BY_HANDLE:
			return syssgi(cmd,
					((xfs_fsop_attrmulti_handlereq_t*)arg)->hreq.ihandle,
					((xfs_fsop_attrmulti_handlereq_t*)arg)->hreq.ihandlen,
					((xfs_fsop_attrmulti_handlereq_t*)arg)->ops,
					((xfs_fsop_attrmulti_handlereq_t*)arg)->opcount,
					((xfs_fsop_attrmulti_handlereq_t*)arg)->hreq.oflags);
		case SGI_FSSETDM_BY_HANDLE:
			return syssgi(cmd,
					((xfs_fsop_setdm_handlereq_t*)arg)->hreq.ihandle,
					((xfs_fsop_setdm_handlereq_t*)arg)->hreq.ihandlen,
					((xfs_fsop_setdm_handlereq_t*)arg)->data);
	}
	return fcntl(fd, cmd, arg);
}

static __inline__ int platform_test_xfs_fd(int fd)
{
	struct statvfs sbuf;
	if (fstatvfs(fd, &sbuf) < 0)
		return 0;
	return strncmp(sbuf.f_basetype, "xfs", 4) == 0;
}

static __inline__ int platform_test_xfs_path(const char *path)
{
	struct statvfs sbuf;
	if (statvfs(path, &sbuf) < 0)
		return 0;
	return strncmp(sbuf.f_basetype, "xfs", 4) == 0;
}

static __inline__ int platform_fstatfs(int fd, struct statfs *buf)
{
	return fstatfs(fd, buf, sizeof(struct statfs), 0);
}

static __inline__ void platform_getoptreset(void)
{
	getoptreset();
}

static __inline__ int platform_uuid_compare(uuid_t *uu1, uuid_t *uu2)
{
	uint_t status;
	return uuid_compare(uu1, uu2, &status);
}

static __inline__ void platform_uuid_unparse(uuid_t *uu, char *buffer)
{
	uint_t status;
	char *s;
	uuid_to_string(uu, &s, &status);
	if (status == uuid_s_ok)
		strcpy(buffer, s);
	else buffer[0] = '\0';
	free(s);
}

static __inline__ int platform_uuid_parse(char *buffer, uuid_t *uu)
{
	uint_t status;
	uuid_from_string(buffer, uu, &status);
	return (status == uuid_s_ok);
}

static __inline__ int platform_uuid_is_null(uuid_t *uu)
{
	uint status;
	return uuid_is_nil(uu, &status);
}

static __inline__ void platform_uuid_generate(uuid_t *uu)
{
	uint_t status;
	uuid_create(uu, &status);
}

static __inline__ void platform_uuid_clear(uuid_t *uu)
{
	uint_t status;
	uuid_create_nil(uu, &status);
}

static __inline__ void platform_uuid_copy(uuid_t *dst, uuid_t *src)
{
	memcpy(dst, src, sizeof(uuid_t));
}

static __inline__ int
platform_discard_blocks(int fd, uint64_t start, uint64_t len)
{
	return 0;
}

static __inline__ char * strsep(char **s, const char *ct)
{
	char *sbegin = *s, *end;

	if (!sbegin)
		return NULL;
	end = strpbrk(sbegin, ct);
	if (end)
		*end++ = '\0';
	*s = end;
	return sbegin;
}

#define HAVE_DIOATTR	1
#define HAVE_FSXATTR	1
#define HAVE_GETBMAP	1
#define HAVE_GETBMAPX	1
#define HAVE_FSDMIDATA	1
#define HAVE_FID	1
#define HAVE_IOCMACROS	1
#define HAVE_BBMACROS	1

#define __XFS_FS_H__	1

#define XFS_IOC_DIOINFO			F_DIOINFO
#define XFS_IOC_FSGETXATTR		F_FSGETXATTR
#define XFS_IOC_FSSETXATTR		F_FSSETXATTR
#define XFS_IOC_ALLOCSP64		F_ALLOCSP64
#define XFS_IOC_FREESP64		F_FREESP64
#define XFS_IOC_GETBMAP			F_GETBMAP
#define XFS_IOC_FSSETDM			F_FSSETDM
#define XFS_IOC_RESVSP			F_RESVSP
#define XFS_IOC_RESVSP64		F_RESVSP64
#define XFS_IOC_UNRESVSP		F_UNRESVSP
#define XFS_IOC_UNRESVSP64		F_UNRESVSP64
#define XFS_IOC_GETBMAPA		F_GETBMAPA
#define XFS_IOC_FSGETXATTRA		F_FSGETXATTRA
#define XFS_IOC_GETBMAPX		F_GETBMAPX

#define XFS_IOC_FSGEOMETRY_V1		XFS_FS_GEOMETRY
#define XFS_IOC_FSBULKSTAT		SGI_FS_BULKSTAT
#define XFS_IOC_FSBULKSTAT_SINGLE	SGI_FS_BULKSTAT_SINGLE
#define XFS_IOC_FSINUMBERS		SGI_FS_INUMBERS
#define XFS_IOC_PATH_TO_FSHANDLE	SGI_PATH_TO_FSHANDLE
#define XFS_IOC_PATH_TO_HANDLE		SGI_PATH_TO_HANDLE
#define XFS_IOC_FD_TO_HANDLE		SGI_FD_TO_HANDLE
#define XFS_IOC_OPEN_BY_HANDLE		SGI_OPEN_BY_HANDLE
#define XFS_IOC_READLINK_BY_HANDLE	SGI_READLINK_BY_HANDLE
#define XFS_IOC_SWAPEXT			/* TODO */
#define XFS_IOC_FSGROWFSDATA		XFS_GROWFS_DATA
#define XFS_IOC_FSGROWFSLOG		XFS_GROWFS_LOG
#define XFS_IOC_FSGROWFSRT		XFS_GROWFS_RT
#define XFS_IOC_FSCOUNTS		XFS_FS_COUNTS
#define XFS_IOC_SET_RESBLKS		XFS_SET_RESBLKS
#define XFS_IOC_GET_RESBLKS		XFS_GET_RESBLKS
#define XFS_IOC_ERROR_INJECTION		SGI_XFS_INJECT_ERROR
#define XFS_IOC_ERROR_CLEARALL		SGI_XFS_CLEARALL_ERROR
#define XFS_IOC_FREEZE			XFS_FS_FREEZE
#define XFS_IOC_THAW			XFS_FS_THAW
#define XFS_IOC_FSSETDM_BY_HANDLE	SGI_FSSETDM_BY_HANDLE
#define XFS_IOC_ATTRLIST_BY_HANDLE	SGI_ATTR_LIST_BY_HANDLE
#define XFS_IOC_ATTRMULTI_BY_HANDLE	SGI_ATTR_MULTI_BY_HANDLE
#define XFS_IOC_FSGEOMETRY		XFS_FS_GEOMETRY
#define XFS_IOC_GOINGDOWN		XFS_FS_GOINGDOWN
#define XFS_IOC_GETPARENTS		SGI_XFS_GETPARENTS
#define XFS_IOC_GETPARENTPATHS		SGI_XFS_GETPARENTPATHS

#define	_AIOCB64_T_DEFINED		1

#define	XFS_XFLAG_NODEFRAG		0x00002000

#endif	/* __XFS_IRIX_H__ */
