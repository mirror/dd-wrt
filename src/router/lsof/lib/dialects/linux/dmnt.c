/*
 * dmnt.c -- Linux mount support functions for /proc-based lsof
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
 * Local definitions
 */

#if defined(HASMNTSUP)
#    define HASHMNT                                                            \
        128 /* mount supplement hash bucket count                              \
             * !!!MUST BE A POWER OF 2!!! */
#endif      /* defined(HASMNTSUP) */

/*
 * Local function prototypes
 */

static char *convert_octal_escaped(struct lsof_context *ctx, char *orig_str);

#if defined(HASMNTSUP)
static int getmntdev(struct lsof_context *ctx, char *dir_name,
                     size_t dir_name_len, struct stat *s, int *ss);
static int hash_mnt(char *dir_name);
#endif /* defined(HASMNTSUP) */

/*
 * Local structure definitions.
 */

#if defined(HASMNTSUP)
typedef struct mntsup {
    char *dir_name;      /* mounted directory name */
    size_t dir_name_len; /* strlen(dir_name) */
    dev_t dev;           /* device number */
    int ln;              /* line on which defined */
    struct mntsup *next; /* next entry */
} mntsup_t;
#endif /* defined(HASMNTSUP) */

/*
 * Local static definitions
 */

static mntsup_t **MSHash = (mntsup_t **)NULL;      /* mount supplement
                                                    * hash buckets */

/*
 * convert_octal_escaped() -- convert octal-escaped characters in string
 */
static char *convert_octal_escaped(struct lsof_context *ctx, /* context */
                                   char *orig_str /* original string */) {
    int cur_ch, cvt_len, cvt_idx, orig_len, orig_idx, temp_idx;
    char *cvt_str;
    int temp_ch;
    /*
     * Allocate space for a copy of the string in which octal-escaped characters
     * can be replaced by the octal value -- e.g., \040 with ' '.  Leave room
     * for a '\0' terminator.
     */
    if (!(orig_len = (int)strlen(orig_str)))
        return ((char *)NULL);
    if (!(cvt_str = (char *)malloc(orig_len + 1))) {
        (void)fprintf(stderr,
                      "%s: can't allocate %d bytes for octal-escaping.\n", Pn,
                      orig_len + 1);
        Error(ctx);
    }
    /*
     * Copy the string, replacing octal-escaped characters as they are found.
     */
    for (cvt_idx = orig_idx = 0, cvt_len = orig_len; orig_idx < orig_len;
         orig_idx++) {
        if (((cur_ch = (int)orig_str[orig_idx]) == (int)'\\') &&
            ((orig_idx + 3) < orig_len)) {

            /*
             * The beginning of an octal-escaped character has been found.
             *
             * Convert the octal value to a character value.
             */
            for (temp_ch = 0, temp_idx = 1;
                 orig_str[orig_idx + temp_idx] && (temp_idx < 4); temp_idx++) {
                if (((int)orig_str[orig_idx + temp_idx] < (int)'0') ||
                    ((int)orig_str[orig_idx + temp_idx] > (int)'7')) {

                    /*
                     * The escape isn't followed by octets, so ignore the
                     * escape and just copy it.
                     */
                    break;
                }
                temp_ch <<= 3;
                temp_ch += (int)(orig_str[orig_idx + temp_idx] - '0');
            }
            if (temp_idx == 4) {

                /*
                 * If three octets (plus the escape) were assembled, use their
                 * character-forming result.
                 *
                 * Otherwise copy the escape and what follows it until another
                 * escape is found.
                 */
                orig_idx += 3;
                cur_ch = (temp_ch & 0xff);
            }
        }

        if (cvt_idx >= cvt_len) {
            /*
             * Expand the copy string, as required.  Leave room for a '\0'
             * terminator.
             */
            cvt_len += 64; /* (Make an arbitrary increase.) */
            if (!(cvt_str = (char *)realloc(cvt_str, cvt_len + 1))) {
                (void)fprintf(
                    stderr, "%s: can't realloc %d bytes for octal-escaping.\n",
                    Pn, cvt_len + 1);
                Error(ctx);
            }
        }

        /*
         * Copy the character.
         */
        cvt_str[cvt_idx++] = (char)cur_ch;
    }

    /*
     * Terminate the copy and return its pointer.
     */
    cvt_str[cvt_idx] = '\0';
    return (cvt_str);
}

#if defined(HASMNTSUP)
/*
 * getmntdev() - get mount device from mount supplement
 */
static int
getmntdev(struct lsof_context *ctx, /* context */
      char *dir_name,               /* mounted directory name */
      size_t dir_name_len,          /* strlen(dir_name) */
      struct stat *s,               /* stat(2) buffer receptor */
      int *ss                       /* stat(2) status result -- i.e., SB_*
                                     * values */)
{
    static int err = 0;
    int h;
    mntsup_t *mp, *mpn;
    static char *vbuf = (char *)NULL;
    static size_t vsz = (size_t)0;

    if (err)
        return (0);
    if (!MSHash) {

        /*
         * No mount supplement hash buckets have been allocated, so read the
         * mount supplement file and create hash buckets for its entries.
         */
        char buf[(MAXPATHLEN * 2) + 1], *dp, path[(MAXPATHLEN * 2) + 1];
        dev_t dev;
        FILE *fs;
        int ln = 0;
        size_t sz;

        if ((MntSup != 2) || !MntSupP)
            return (0);
        if (!is_readable(ctx, MntSupP, 1)) {

            /*
             * The mount supplement file isn't readable.
             */
            err = 1;
            return (0);
        }
        if (!(fs = open_proc_stream(ctx, MntSupP, "r", &vbuf, &vsz, 0))) {

            /*
             * The mount supplement file can't be opened for reading.
             */
            if (!Fwarn)
                (void)fprintf(stderr, "%s: can't open(%s): %s\n", Pn, MntSupP,
                              strerror(errno));
            err = 1;
            return (0);
        }
        buf[sizeof(buf) - 1] = '\0';
        /*
         * Read the mount supplement file.
         */
        while (fgets(buf, sizeof(buf) - 1, fs)) {
            ln++;
            if ((dp = strchr(buf, '\n')))
                *dp = '\0';
            if (buf[0] != '/') {

                /*
                 * The mount supplement line doesn't begin with the absolute
                 * path character '/'.
                 */
                if (!Fwarn)
                    (void)fprintf(stderr, "%s: %s line %d: no path: \"%s\"\n",
                                  Pn, MntSupP, ln, buf);
                err = 1;
                continue;
            }
            if (!(dp = strchr(buf, ' ')) || strncmp(dp + 1, "0x", 2)) {

                /*
                 * The path on the mount supplement line isn't followed by
                 * " 0x".
                 */
                if (!Fwarn)
                    (void)fprintf(stderr, "%s: %s line %d: no device: \"%s\"\n",
                                  Pn, MntSupP, ln, buf);
                err = 1;
                continue;
            }
            sz = (size_t)(dp - buf);
            (void)strncpy(path, buf, sz);
            path[sz] = '\0';
            /*
             * Assemble the hexadecimal device number of the mount supplement
             * line.
             */
            for (dev = 0, dp += 3; *dp; dp++) {
                if (!isxdigit((int)*dp))
                    break;
                if (isdigit((int)*dp))
                    dev = (dev << 4) + (int)*dp - (int)'0';
                else
                    dev = (dev << 4) + (int)tolower(*dp) - (int)'a' + 10;
            }
            if (*dp) {

                /*
                 * The device number couldn't be assembled.
                 */
                if (!Fwarn)
                    (void)fprintf(stderr,
                                  "%s: %s line %d: illegal device: \"%s\"\n",
                                  Pn, MntSupP, ln, buf);
                err = 1;
                continue;
            }
            /*
             * Search the mount supplement hash buckets.  (Allocate them as
             * required.)
             */
            if (!MSHash) {
                if (!(MSHash =
                          (mntsup_t **)calloc(HASHMNT, sizeof(mntsup_t *)))) {
                    (void)fprintf(
                        stderr,
                        "%s: no space for mount supplement hash buckets\n", Pn);
                    Error(ctx);
                }
            }
            h = hash_mnt(path);
            for (mp = MSHash[h]; mp; mp = mp->next) {
                if ((mp->dir_name_len == dir_name_len) &&
                    !strcmp(mp->dir_name, path))
                    break;
            }
            if (mp) {

                /*
                 * A path match was located.  If the device number is the
                 * same, skip this mount supplement line.  Otherwise, issue
                 * a warning.
                 */
                if (mp->dev != dev) {
                    (void)fprintf(
                        stderr, "%s: %s line %d path duplicate of %d: \"%s\"\n",
                        Pn, MntSupP, ln, mp->ln, buf);
                    err = 1;
                }
                continue;
            }
            /*
             * Allocate and fill a new mount supplement hash entry.
             */
            if (!(mpn = (mntsup_t *)malloc(sizeof(mntsup_t)))) {
                (void)fprintf(
                    stderr,
                    "%s: no space for mount supplement entry: %d \"%s\"\n", Pn,
                    ln, buf);
                Error(ctx);
            }
            if (!(mpn->dir_name = (char *)malloc(sz + 1))) {
                (void)fprintf(
                    stderr,
                    "%s: no space for mount supplement path: %d \"%s\"\n", Pn,
                    ln, buf);
                Error(ctx);
            }
            (void)strcpy(mpn->dir_name, path);
            mpn->dir_name_len = sz;
            mpn->dev = dev;
            mpn->ln = ln;
            mpn->next = MSHash[h];
            MSHash[h] = mpn;
        }
        if (ferror(fs)) {
            if (!Fwarn)
                (void)fprintf(stderr, "%s: error reading %s\n", Pn, MntSupP);
            err = 1;
        }
        (void)fclose(fs);
        if (err) {
            if (MSHash) {
                for (h = 0; h < HASHMNT; h++) {
                    for (mp = MSHash[h]; mp; mp = mpn) {
                        mpn = mp->next;
                        if (mp->dir_name)
                            (void)free((MALLOC_P *)mp->dir_name);
                        (void)free((MALLOC_P *)mp);
                    }
                }
                (void)free((MALLOC_P *)MSHash);
                MSHash = (mntsup_t **)NULL;
            }
            return (0);
        }
    }

    /*
     * If no errors have been detected reading the mount supplement file, search
     * its hash buckets for the supplied directory path.
     */
    if (err)
        return (0);
    h = hash_mnt(dir_name);
    for (mp = MSHash[h]; mp; mp = mp->next) {
        if ((dir_name_len == mp->dir_name_len) &&
            !strcmp(dir_name, mp->dir_name)) {
            zeromem((char *)s, sizeof(struct stat));
            s->st_dev = mp->dev;
            *ss |= SB_DEV;
            return (1);
        }
    }
    return (0);
}

/*
 * hash_mnt() - hash mount point
 */
static int hash_mnt(char *dir_name /* mount point directory name */) {
    register int i, h;
    size_t l;

    if (!(l = strlen(dir_name)))
        return (0);
    if (l == 1)
        return ((int)*dir_name & (HASHMNT - 1));
    for (i = h = 0; i < (int)(l - 1); i++) {
        h ^= ((int)dir_name[i] * (int)dir_name[i + 1]) << ((i * 3) % 13);
    }
    return (h & (HASHMNT - 1));
}
#endif /* defined(HASMNTSUP) */

/*
 * readmnt() - read mount table
 */
struct mounts *readmnt(struct lsof_context *ctx) {
    char buf[MAXPATHLEN], *cp, **fp;
    char *dn = (char *)NULL;
    size_t dnl;
    int ds, ne;
    char *fp0 = (char *)NULL;
    char *fp1 = (char *)NULL;
    int fr, ignrdl, ignstat;
    char *ln;
    struct mounts *mp;
    FILE *ms;
    int nfs;
    int mqueue;
    struct stat sb;
    static char *vbuf = (char *)NULL;
    static size_t vsz = (size_t)0;

    if (Lmi || Lmist)
        return (Lmi);
    /*
     * Open access to /proc/mounts, assigning a page size buffer to its stream.
     */
    (void)snpf(buf, sizeof(buf), "%s/mounts", PROCFS);
    ms = open_proc_stream(ctx, buf, "r", &vbuf, &vsz, 1);
    /*
     * Read mount table entries.
     */
    while (fgets(buf, sizeof(buf), ms)) {
        if (get_fields(ctx, buf, (char *)NULL, &fp, (int *)NULL, 0) < 3 ||
            !fp[0] || !fp[1] || !fp[2])
            continue;
        /*
         * Convert octal-escaped characters in the device name and mounted-on
         * path name.
         */
        if (fp0) {
            (void)free((FREE_P *)fp0);
            fp0 = (char *)NULL;
        }
        if (fp1) {
            (void)free((FREE_P *)fp1);
            fp1 = (char *)NULL;
        }
        if (!(fp0 = convert_octal_escaped(ctx, fp[0])) ||
            !(fp1 = convert_octal_escaped(ctx, fp[1])))
            continue;
        /*
         * Locate any colon (':') in the device name.
         *
         * If the colon is followed by * "(pid*" -- it's probably an
         * automounter entry.
         *
         * Ignore autofs, pipefs, and sockfs entries.
         */
        cp = strchr(fp0, ':');
        if (cp && !strncasecmp(++cp, "(pid", 4))
            continue;
        if (!strcasecmp(fp[2], "autofs") || !strcasecmp(fp[2], "pipefs") ||
            !strcasecmp(fp[2], "sockfs"))
            continue;

        /*
         * Interpolate a possible symbolic mounted directory link.
         */
        if (dn)
            (void)free((FREE_P *)dn);
        dn = fp1;
        fp1 = (char *)NULL;

#if defined(HASEOPT)
        if (Efsysl) {

            /*
             * If there is an -e file system list, check it to decide if a
             * stat() and Readlink() on this one should be performed.
             */
            efsys_list_t *ep;

            for (ignrdl = ignstat = 0, ep = Efsysl; ep; ep = ep->next) {
                if (!strcmp(dn, ep->path)) {
                    ignrdl = ep->rdlnk;
                    ignstat = 1;
                    break;
                }
            }
        } else

#endif /* defined(HASEOPT */

            ignrdl = ignstat = 0;

        /*
         * Avoid Readlink() when requested.
         */
        if (!ignrdl) {
            if (!(ln = Readlink(ctx, dn))) {
                if (!Fwarn) {
                    (void)fprintf(
                        stderr,
                        "      Output information may be incomplete.\n");
                }
                continue;
            }
            if (ln != dn) {
                (void)free((FREE_P *)dn);
                dn = ln;
            }
        }
        if (*dn != '/')
            continue;
        dnl = strlen(dn);

        /*
         * Test Mqueue directory
         */
        mqueue = strcmp(fp[2], "mqueue");

        /*
         * Test for duplicate and NFS directories.
         */
        for (mp = Lmi; mp; mp = mp->next) {
            if ((dnl == mp->dirl) && !strcmp(dn, mp->dir))
                break;
        }
        if ((nfs = strcasecmp(fp[2], "nfs"))) {
            if ((nfs = strcasecmp(fp[2], "nfs3")))
                nfs = strcasecmp(fp[2], "nfs4");
        }
        if (!nfs && !HasNFS)
            HasNFS = 1;
        if (mp) {

            /*
             * If this duplicate directory is not root, ignore it.  If the
             * already remembered entry is NFS-mounted, ignore this one.  If
             * this one is NFS-mounted, ignore the already remembered entry.
             */
            if (strcmp(dn, "/"))
                continue;
            if (mp->ty == N_NFS)
                continue;
            if (nfs)
                continue;
        }

        /*
         * Stat() the directory.
         */
        if (ignstat)
            fr = 1;
        else {
            if ((fr = statsafely(ctx, dn, &sb))) {
                if (!Fwarn) {
                    (void)fprintf(stderr, "%s: WARNING: can't stat() ", Pn);
                    safestrprt(fp[2], stderr, 0);
                    (void)fprintf(stderr, " file system ");
                    safestrprt(dn, stderr, 1);
                    (void)fprintf(
                        stderr,
                        "      Output information may be incomplete.\n");
                }
            } else
                ds = SB_ALL;
        }

#if defined(HASMNTSUP)
        if (fr) {

            /*
             * If the stat() failed or wasn't called, check the mount
             * supplement table, if possible.
             */
            if ((MntSup == 2) && MntSupP) {
                ds = 0;
                if (getmntdev(ctx, dn, dnl, &sb, &ds) || !(ds & SB_DEV)) {
                    (void)fprintf(stderr,
                                  "%s: assuming dev=%#lx for %s from %s\n", Pn,
                                  (long)sb.st_dev, dn, MntSupP);
                }
            } else {
                if (!ignstat)
                    continue;
                ds = 0; /* No stat() was allowed. */
            }
        }
#else  /* !defined(HASMNTSUP) */
        if (fr) {
            if (!ignstat)
                continue;
            ds = 0; /* No stat() was allowed. */
        }
#endif /* defined(HASMNTSUP) */

        /*
         * Fill a local mount structure or reuse a previous entry when
         * indicated.
         */
        if (mp) {
            ne = 0;
            if (mp->dir) {
                (void)free((FREE_P *)mp->dir);
                mp->dir = (char *)NULL;
            }
            if (mp->fsname) {
                (void)free((FREE_P *)mp->fsname);
                mp->fsname = (char *)NULL;
            }
        } else {
            ne = 1;
            if (!(mp = (struct mounts *)malloc(sizeof(struct mounts)))) {
                (void)fprintf(stderr,
                              "%s: can't allocate mounts struct for: ", Pn);
                safestrprt(dn, stderr, 1);
                Error(ctx);
            }
        }
        mp->dir = dn;
        dn = (char *)NULL;
        mp->dirl = dnl;
        if (ne)
            mp->next = Lmi;
        mp->dev = ((mp->ds = ds) & SB_DEV) ? sb.st_dev : 0;
        mp->rdev = (ds & SB_RDEV) ? sb.st_rdev : 0;
        mp->inode = (INODETYPE)((ds & SB_INO) ? sb.st_ino : 0);
        mp->mode = (ds & SB_MODE) ? sb.st_mode : 0;
        if (!nfs) {
            mp->ty = N_NFS;
            if (HasNFS < 2)
                HasNFS = 2;
        } else if (!mqueue) {
            mp->ty = N_MQUEUE;
            MqueueDev = mp->dev;
        } else {
            mp->ty = N_REGLR;
        }

#if defined(HASMNTSUP)
        /*
         * If support for the mount supplement file is defined and if the
         * +m option was supplied, print mount supplement information.
         */
        if (MntSup == 1) {
            if (mp->dev)
                (void)printf("%s %#lx\n", mp->dir, (long)mp->dev);
            else
                (void)printf("%s 0x0\n", mp->dir);
        }
#endif /* defined(HASMNTSUP) */

        /*
         * Save mounted-on device or directory name.
         */
        dn = fp0;
        fp0 = (char *)NULL;
        mp->fsname = dn;

        /*
         * Interpolate a possible file system (mounted-on) device name or
         * directory name link.
         *
         * Avoid Readlink() when requested.
         */
        if (ignrdl || (*dn != '/')) {
            if (!(ln = mkstrcpy(dn, (MALLOC_S *)NULL))) {
                (void)fprintf(stderr, "%s: can't allocate space for: ", Pn);
                safestrprt(dn, stderr, 1);
                Error(ctx);
            }
            ignstat = 1;
        } else
            ln = Readlink(ctx, dn);
        dn = (char *)NULL;

        /*
         * Stat() the file system (mounted-on) name and add file system
         * information to the local mount table entry.
         */
        if (ignstat || !ln || statsafely(ctx, ln, &sb))
            sb.st_mode = 0;
        mp->fsnmres = ln;
        ln = NULL;
        mp->fs_mode = sb.st_mode;
        if (ne)
            Lmi = mp;
    }

    /*
     * Clean up and return the local mount info table address.
     */
    (void)fclose(ms);
    if (dn)
        (void)free((FREE_P *)dn);
    if (fp0)
        (void)free((FREE_P *)fp0);
    if (fp1)
        (void)free((FREE_P *)fp1);
    Lmist = 1;
    return (Lmi);
}
