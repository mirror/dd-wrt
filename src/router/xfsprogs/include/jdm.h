/*
 * Copyright (c) 2000-2002, 2005 Silicon Graphics, Inc.
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
#ifndef __JDM_H__
#define __JDM_H__

typedef int	intgen_t;
typedef void	jdm_fshandle_t;		/* filesystem handle */
typedef void	jdm_filehandle_t;	/* filehandle */

struct xfs_bstat;
struct attrlist_cursor;
struct parent;

extern jdm_fshandle_t *
jdm_getfshandle( char *mntpnt);

extern void
jdm_new_filehandle( jdm_filehandle_t **handlep,	/* new filehandle */
		    size_t *hlen,		/* new filehandle size */
		    jdm_fshandle_t *fshandlep,	/* filesystem filehandle */
		    struct xfs_bstat *sp);	/* bulkstat info */

extern void
jdm_delete_filehandle( jdm_filehandle_t *handlep,/* filehandle to delete */
		       size_t hlen);		/* filehandle size */

extern intgen_t
jdm_open( jdm_fshandle_t *fshandlep,
	  struct xfs_bstat *sp,
	  intgen_t oflags);

extern intgen_t
jdm_readlink( jdm_fshandle_t *fshandlep,
	      struct xfs_bstat *sp,
	      char *bufp,
	      size_t bufsz);

extern intgen_t
jdm_attr_multi(	jdm_fshandle_t *fshp,
		xfs_bstat_t *statp,
		char *bufp, int rtrvcnt, int flags);

extern intgen_t
jdm_attr_list(	jdm_fshandle_t *fshp,
		xfs_bstat_t *statp,
		char *bufp, size_t bufsz, int flags,
		struct attrlist_cursor *cursor);

extern int
jdm_parents( jdm_fshandle_t *fshp,
		xfs_bstat_t *statp,
		struct parent *bufp, size_t bufsz,
		unsigned int *count);

extern int
jdm_parentpaths( jdm_fshandle_t *fshp,
		xfs_bstat_t *statp,
		struct parent *bufp, size_t bufsz,
		unsigned int *count);

/* macro for determining the size of a structure member */
#define sizeofmember( t, m )	sizeof( ( ( t * )0 )->m )

/* macro for calculating the offset of a structure member */
#define offsetofmember( t, m )	( ( size_t )( char * )&( ( ( t * )0 )->m ) )

#endif	/* __JDM_H__ */
