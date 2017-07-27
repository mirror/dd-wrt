/*
 * dnode2.c - SCO UnixWare node functions for lsof
 *
 * This module must be separate to keep separate the multiple kernel inode
 * structure definitions.
 */

/*
 * Copyright 1998 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */

#ifndef lint
static char copyright[] =
"@(#) Copyright 1996 Purdue Research Foundation.\nAll rights reserved.\n";
static char *rcsid = "$Id: dnode3.c,v 1.5 2005/08/13 16:21:41 abe Exp $";
#endif


#include "lsof.h"

#include <sys/fs/bfs.h>

#if	UNIXWAREV>=7000
# if	UNIXWAREV>=70103
#undef	IEXEC
#undef	IREAD
#undef	IWRITE
# endif	/* UNIXWAREV>=70103 */
#undef	ISUID
#undef	ISGID
#undef	ISVTX
#include <sys/fs/cdfs_fs.h>
#endif	/* UNIXWAREV>=7000 */


/*
 * Local definitions
 */

#define	DOS_NAME_PFX	"(DOS name: "
#define	DOS_NAME_SFX	")"

#if	UNIXWAREV>=7000 && !defined(TYPELOGSECSHIFT)
#define	TYPELOGSECSHIFT	uint_t		/* just in case Configure missed it */
#endif	/* UNIXWAREV>=7000 && !defined(TYPELOGSECSHIFT) */


/*
 * Local static variables
 */

static unsigned char Dos2Unix[] = {	/* derived from tounix[] in
					 * /etc/conf/pack.d/ccnv/space.c */
    '_', '_', '_', '_', '_', '_', '_', '_',		/* 0x01-0x08 */
    '_', '_', '_', '_', '_', '_', '_', '_',		/* 0x09-0x10 */
    '_', '_', '_', '_', '_', '_', '_', '_',		/* 0x11-0x18 */
    '_', '_', '_', '_', '_', '_', '_', ' ',		/* 0x19-0x20 */
    '!', '"', '#', '$', '%', '&', '\'', '(',		/* 0x21-0x28 */
    ')', '*', '+', ',', '-', '.', '/', '0',		/* 0x29-0x30 */
    '1', '2', '3', '4', '5', '6', '7', '8',		/* 0x31-0x38 */
    '9', ':', ';', '<', '=', '>', '?', '@',		/* 0x39-0x40 */
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',		/* 0x41-0x48 */
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',		/* 0x49-0x50 */
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x',		/* 0x51-0x58 */
    'y', 'z', '[', '\\', ']', '^', '_', '`',		/* 0x59-0x60 */
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',		/* 0x61-0x68 */
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',		/* 0x69-0x70 */
    'q', 'r', 's', 't', 'u', 'v', 'w', 'x',		/* 0x71-0x78 */
    'y', 'z', '{', '|', '}', '~', '_', 0347,		/* 0x79-0x80 */
    0374, 0351, 0342, 0344, 0340, 0345, 0347, 0352,	/* 0x81-0x88 */
    0353, 0350, 0357, 0356, 0354, 0344, 0345, 0351,	/* 0x89-0x90 */
    0346, 0346, 0364, 0366, 0362, 0373, 0371, 0377,	/* 0X91-0X98 */
    0366, 0374, 0370, 0243, 0370, 0367, '_',  0341,	/* 0x99-0xa0 */
    0355, 0363, 0372, 0361, 0361, 0252, 0272, 0277,	/* 0xa1-0xa8 */
    0256, 0254, 0275, 0274, 0241, 0253, 0273, '_',	/* 0xa9-0xb0 */
    '_',  '_',  '_',  '_',  0341, 0342, 0340, 0251,	/* 0xb1-0xb8 */
    '_',  '_',  '_',  '_',  0242, 0245, '_',  '_',	/* 0xb9-0xc0 */
    '_',  '_',  '_',  '_',  '_',  0343, 0343, '_',	/* 0xc1-0xc8 */
    '_',  '_',  '_',  '_',  '_',  '_',  0244, 0360,	/* 0xc9-0xd0 */
    0320, 0352, 0353, 0350, '_',  0355, 0356, 0357,	/* 0xd1-0xd8 */
    '_',  '_',  '_',  '_',  0246, 0354, '_',  0363,	/* 0xd9-0xe0 */
    0337, 0364, 0362, 0365, 0365, 0265, 0376, 0376,	/* 0xe1-0xe8 */
    0372, 0373, 0371, 0375, 0375, 0257, 0264, 0255,	/* 0xe9-0xf0 */
    0261, '_',  0276, 0266, 0247, 0327, 0270, 0260,	/* 0xf1-0xf8 */
    0250, 0267, 0271, 0263, 0262, '_',  '_'		/* 0xf9-0xff */
};



/*
 * readbfslino() - read bfs inode's local inode information
 */

int
readbfslino(v, i)
	struct vnode *v;		/* containing vnode */
	struct l_ino *i;		/* local inode information */
{
	struct inode b;
	struct vfs kv;

	if (kread((KA_T)v->v_data, (char *)&b, sizeof(b)))
	    return(1);
	if (!v->v_vfsp || kread((KA_T)v->v_vfsp, (char *)&kv, sizeof(kv)))
	    return(1);
	i->dev = kv.vfs_dev;
	i->dev_def = 1;
	i->rdev = (dev_t)0;
	i->rdev_def = 0;
	i->nlink = (long)b.i_diskino.d_fattr.va_nlink;
	i->nlink_def = 1;
	i->nm = (char *)NULL;
	i->number = (INODETYPE)b.i_diskino.d_ino;
	i->number_def = 1;
	i->size = (SZOFFTYPE)BFS_FILESIZE(&b.i_diskino);
	i->size_def = 1;
	return(0);
}


#if	UNIXWAREV>=7000
/*
 * reacdfslino() - read cdfs inode's local inode information
 *
 * Adapted from work by Eric Dumazet <edumazet@cosmosbay.com>.
 */

int
readcdfslino(v, i)
	struct vnode *v;		/* containing vnode */
	struct l_ino *i;		/* local inode information */
{
	cdfs_inode_t ci;
	TYPELOGSECSHIFT lss;
	KA_T ka;
	struct vfs kv;
/*
 * Read the CDFs node.  Fill in return values from its contents.
 */
	if (!v->v_data || kread((KA_T)v->v_data, (char *)&ci, sizeof(ci)))
	    return(1);
	if (!v->v_vfsp || kread((KA_T)v->v_vfsp, (char *)&kv, sizeof(kv)))
	    return(1);
	i->dev = kv.vfs_dev;
	i->dev_def = 1;
	i->rdev = ci.i_DevNum;
	i->rdev_def = 1;
	i->nlink = (long)ci.i_LinkCnt;
	i->nlink_def = 1;
	i->nm = (char *)NULL;
	i->size = (SZOFFTYPE)ci.i_Size;
	i->size_def = 1;
/*
 * Compute the node number.
 *
 * (See the CDFS_INUM() macro in <sys/fs/cdfs_inode.h>.)
 *
 * It's too wasteful to read the entire cdfs structure for one element,
 * cdfs_LogSecShift, so it's read specially.  Its type, TYPELOGSECSHIFT,
 * should be defined by lsof's Configure script, but for safety's sake
 * has a uint_t default, defined in this source file.
 */
	i->number = (INODETYPE)0;
	if (!(ka = (KA_T)kv.vfs_data))
	    return(0);
	ka = (KA_T)((char *)ka + offsetof(struct cdfs, cdfs_LogSecShift));
	if (!kread(ka, (char *)&lss, sizeof(lss))) {
	    i->number = (INODETYPE)((ci.i_Fid.fid_SectNum << lss)
		      + ci.i_Fid.fid_Offset);
	    i->number_def = 1;
	}
	return(0);
}


/*
 * readdosfslino() - read dosfs inode's local inode information
 *
 * Adapted from work by Eric Dumazet <edumazet@cosmosbay.com>.
 */


int
readdosfslino(v, i)
	struct vnode *v;		/* containing vnode */
	struct l_ino *i;		/* local inode information */
{
	struct dosfs_inode {
	    int pad1[19];
	    int device;
	    int pad2[21];
	    long offset;
	    long dirnum;
	    int pad3[3];
	    unsigned char name[16];
	    int pad4[3];
	    unsigned long size;
	} di;
	static char *nm = (char *)NULL;
	static MALLOC_S nml = (MALLOC_S)0;
	int sz;
/*
 * Read the DOSFS node.  Fill in return values from its contents.
 */
	if (!v->v_data || kread((KA_T)v->v_data, (char *)&di, sizeof(di)))
	    return(1);
	i->dev = (dev_t)di.device;
	i->dev_def = 1;
	i->rdev = (dev_t)0;
	i->rdev_def = 1;
	i->nlink_def = 0;
	i->size = (SZOFFTYPE)di.size;
	i->size_def = 1;
/*
 * Allocate space for and save the name, prepared as a name column addition.
 */
	i->nm = (char *)NULL;
	if (di.name[0]) {
	    if (!nm) {
		nml = (MALLOC_S)(strlen(DOS_NAME_PFX) + sizeof(di.name)
		    +  strlen(DOS_NAME_SFX) + 1);
		if (!(nm = (char *)malloc(nml))) {
		    (void) fprintf(stderr,
			"%s: can't allocate %d bytes for DOS name\n", nml, Pn);
		    Exit(1);
		}
	    }
	/*
	 * Calculate the DOS name length and convert DOS to UNIX characters.
	 */
	    for (sz = 0; sz < sizeof(di.name); sz++) {
		if (!di.name[sz])
		    break;
		di.name[sz] = Dos2Unix[(di.name[sz] & 0xff) - 1];
	    }
	    if (sz) {
		(void) snpf(nm, nml, "%s%-*.*s%s", DOS_NAME_PFX, sz, sz,
		    di.name, DOS_NAME_SFX);
		i->nm = nm;
	    }
	}
/*
 * Compute the node number.
 */
	i->number = (INODETYPE)((di.dirnum << 16) + di.offset);
	i->number_def = 1;
	return(0);
}
#endif	/* UNIXWAREV>=7000 */
