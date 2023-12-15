/*
 * dfile.c - Linux file processing functions for /proc-based lsof
 */

/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
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

#include "common.h"

/*
 * Local structures
 */

/*
 * Local static variables
 */


/*
 * Local definitions
 */

#define SFDIHASH                                                               \
    4094 /* Sfile hash by (device,inode) number                                \
          * pair bucket count (power of 2!) */
#define SFFSHASH                                                               \
    1024 /* Sfile hash by file system device                                   \
          * number bucket count (power of 2!) */
/* hash for Sfile by major device,
 * minor device, and inode, modulo mod
 * (mod must be a power of 2) */
#define SFHASHDEVINO(maj, min, ino, mod)                                       \
    ((int)(((int)((((int)(maj + 1)) * ((int)((min + 1)))) + ino) * 31415) &    \
           (mod - 1)))
#define SFRDHASH                                                               \
    1024 /* Sfile hash by raw device number                                    \
          * bucket count (power of 2!) */
/* hash for Sfile by major device,
 * minor device, major raw device,
 * minor raw device, and inode, modulo
 * mod (mod must be a power of 2) */
#define SFHASHRDEVI(maj, min, rmaj, rmin, ino, mod)                            \
    ((int)(((int)((((int)(maj + 1)) * ((int)((min + 1)))) +                    \
                  ((int)(rmaj + 1) * (int)(rmin + 1)) + ino) *                 \
            31415) &                                                           \
           (mod - 1)))
#define SFNMHASH                                                               \
    4096 /* Sfile hash by name bucket count                                    \
          * (must be a power of 2!) */

/*
 * hashSfile() - hash Sfile entries for use in is_file_named() searches
 */
void hashSfile(struct lsof_context *ctx) {
    static int hs = 0;
    int i;
    struct sfile *s;
    struct hsfile *sh, *sn;
    /*
     * Do nothing if there are no file search arguments cached or if the
     * hashes have already been constructed.
     */
    if (!Sfile || hs)
        return;
    /*
     * Allocate hash buckets by (device,inode), file system device, and file
     * name.
     */
    if (!(HbyFdi = (struct hsfile *)calloc((MALLOC_S)SFDIHASH,
                                           sizeof(struct hsfile)))) {
        (void)fprintf(
            stderr, "%s: can't allocate space for %d (dev,ino) hash buckets\n",
            Pn, SFDIHASH);
        Error(ctx);
    }
    if (!(HbyFrd = (struct hsfile *)calloc((MALLOC_S)SFRDHASH,
                                           sizeof(struct hsfile)))) {
        (void)fprintf(stderr,
                      "%s: can't allocate space for %d rdev hash buckets\n", Pn,
                      SFRDHASH);
        Error(ctx);
    }
    if (!(HbyFsd = (struct hsfile *)calloc((MALLOC_S)SFFSHASH,
                                           sizeof(struct hsfile)))) {
        (void)fprintf(stderr,
                      "%s: can't allocate space for %d file sys hash buckets\n",
                      Pn, SFFSHASH);
        Error(ctx);
    }
    if (!(HbyNm = (struct hsfile *)calloc((MALLOC_S)SFNMHASH,
                                          sizeof(struct hsfile)))) {
        (void)fprintf(stderr,
                      "%s: can't allocate space for %d name hash buckets\n", Pn,
                      SFNMHASH);
        Error(ctx);
    }
    hs++;
    /*
     * Scan the Sfile chain, building file, file system, raw device, and file
     * name hash bucket chains.
     */
    for (s = Sfile; s; s = s->next) {
        for (i = 0; i < 3; i++) {
            switch (i) {
            case 0: /* hash by name */
                if (!s->aname)
                    continue;
                sh = &HbyNm[hashbyname(s->aname, SFNMHASH)];
                HbyNmCt++;
                break;
            case 1: /* hash by device and inode, or file
                     * system device */
                if (s->type) {
                    sh = &HbyFdi[SFHASHDEVINO(GET_MAJ_DEV(s->dev),
                                              GET_MIN_DEV(s->dev), s->i,
                                              SFDIHASH)];
                    HbyFdiCt++;
                } else {
                    sh = &HbyFsd[SFHASHDEVINO(
                        GET_MAJ_DEV(s->dev), GET_MIN_DEV(s->dev), 0, SFFSHASH)];
                    HbyFsdCt++;
                }
                break;
            case 2: /* hash by file's raw device */
                if ((s->mode == S_IFCHR) || (s->mode == S_IFBLK)) {
                    sh = &HbyFrd[SFHASHRDEVI(
                        GET_MAJ_DEV(s->dev), GET_MIN_DEV(s->dev),
                        GET_MAJ_DEV(s->rdev), GET_MIN_DEV(s->rdev), s->i,
                        SFRDHASH)];
                    HbyFrdCt++;
                } else
                    continue;
            }
            /*
             * Add hash to the bucket's chain, allocating new entries for
             * all after the first.
             */
            if (!sh->s) {
                sh->s = s;
                sh->next = (struct hsfile *)NULL;
                continue;
            } else {
                if (!(sn = (struct hsfile *)malloc(
                          (MALLOC_S)sizeof(struct hsfile)))) {
                    (void)fprintf(stderr,
                                  "%s: can't allocate hsfile bucket for: %s\n",
                                  Pn, s->aname);
                    Error(ctx);
                }
                sn->s = s;
                sn->next = sh->next;
                sh->next = sn;
            }
        }
    }
}

/*
 * is_file_named() - is this file named?
 */
int is_file_named(
    /* context */
    struct lsof_context *ctx,
    /* search type:	0 = only by device
     *		    and inode
     *		1 = by device and
     *		    inode, or by file
     *		    system device and
     *		    path for NFS file
     *		    systems
     *		2 = only by path
     */
    int search_type,
    /* path name (device and inode are
     * identified via *Lf) */
    char *path,
    /* NFS file system (NULL if not) */
    struct mounts *nfs_mount,
    /* character or block type file --
     * VCHR or VBLK vnode, or S_IFCHR
     * or S_IFBLK inode */
    int cd) {
    char *ep;
    int f = 0;
    struct mounts *smp;
    struct sfile *s = (struct sfile *)NULL;
    struct hsfile *sh;
    size_t sz;

    /*
     * Check for a path name match, as requested.
     */
    if ((search_type == 2) && path && HbyNmCt) {
        for (sh = &HbyNm[hashbyname(path, SFNMHASH)]; sh; sh = sh->next) {
            if ((s = sh->s) && strcmp(path, s->aname) == 0) {
                f = 2; /* Found match by path */
                break;
            }
        }
    }

    /*
     * Check for a regular file by device and inode number.
     */
    if (!f && (search_type < 2) && HbyFdiCt && Lf->dev_def &&
        (Lf->inp_ty == 1 || Lf->inp_ty == 3)) {
        for (sh = &HbyFdi[SFHASHDEVINO(GET_MAJ_DEV(Lf->dev),
                                       GET_MIN_DEV(Lf->dev), Lf->inode,
                                       SFDIHASH)];
             sh; sh = sh->next) {
            if ((s = sh->s) && (Lf->dev == s->dev) && (Lf->inode == s->i)) {
                f = 1; /* Found match by inode and dev */
                break;
            }
        }
    }

    /*
     * Check for a file system match.
     */
    if (!f && (search_type == 1) && HbyFsdCt && Lf->dev_def) {
        for (sh = &HbyFsd[SFHASHDEVINO(GET_MAJ_DEV(Lf->dev),
                                       GET_MIN_DEV(Lf->dev), 0, SFFSHASH)];
             sh; sh = sh->next) {
            if ((s = sh->s) && (s->dev == Lf->dev)) {
                if (Lf->ntype != N_NFS) {

                    /*
                     * A non-NFS file matches to a non-NFS file system by
                     * device.
                     */
                    if (!(smp = s->mp) || (smp->ty != N_NFS)) {
                        f = 1; /* Found match by fs */
                        break;
                    }
                } else {

                    /*
                     * An NFS file must also match to a file system by the
                     * the path name of the file system -- i.e., the first
                     * part of the file's path.  This terrible, non-UNIX
                     * hack is forced on lsof by an egregious error in
                     * Linux NFS that can assign the same device number
                     * to two different NFS mounts.
                     */
                    if (path && nfs_mount && nfs_mount->dirl &&
                        nfs_mount->dir && s->name &&
                        !strncmp(nfs_mount->dir, s->name, nfs_mount->dirl)) {
                        f = 1; /* Found match by fs */
                        break;
                    }
                }
            }
        }
    }

    /*
     * Check for a character or block device match.
     */
    if (!f && !search_type && HbyFrdCt && cd && Lf->dev_def &&
        (Lf->dev == DevDev) && Lf->rdev_def &&
        (Lf->inp_ty == 1 || Lf->inp_ty == 3)) {
        for (sh = &HbyFrd[SFHASHRDEVI(
                 GET_MAJ_DEV(Lf->dev), GET_MIN_DEV(Lf->dev),
                 GET_MAJ_DEV(Lf->rdev), GET_MIN_DEV(Lf->rdev), Lf->inode,
                 SFRDHASH)];
             sh; sh = sh->next) {
            if ((s = sh->s) && (s->dev == Lf->dev) && (s->rdev == Lf->rdev) &&
                (s->i == Lf->inode)) {
                f = 1; /* Found match by inode and dev */
                break;
            }
        }
    }
    /*
     * Convert the name if a match occurred.
     */
    switch (f) {
    case 0: /* Not found */
        return (0);
    case 1: /* Found match by inode and dev or fs */
        if (s->type) {

            /*
             * If the search argument isn't a file system, propagate it
             * to Namech[]; otherwise, let printname() compose the name.
             */
            (void)snpf(Namech, Namechl, "%s", s->name);
            if (s->devnm) {
                ep = endnm(ctx, &sz);
                (void)snpf(ep, sz, " (%s)", s->devnm);
            }
        }
        break;
    case 2: /* Found match by path */
        (void)strcpy(Namech, path);
        break;
    }
    if (s)
        s->f = 1;
    return (1);
}

/*
 * printdevname() - print character device name
 *
 * Note: this function should not be needed in /proc-based lsof, but
 *	 since it is called by printname() in print.c, an ersatz one
 *	 is provided here.
 */
int printdevname(struct lsof_context *ctx, /* context */
                 dev_t *dev,               /* device */
                 dev_t *rdev,              /* raw device */
                 int newline,              /* 1 = follow with '\n' */
                 int node_type)            /* node type: N_BLK or N_chr */
{
    char buf[128];

    (void)snpf(buf, sizeof(buf), "%s device: %d,%d",
               (node_type == N_BLK) ? "BLK" : "CHR", (int)GET_MAJ_DEV(*rdev),
               (int)GET_MIN_DEV(*rdev));
    safestrprt(buf, stdout, newline);
    return (1);
}
