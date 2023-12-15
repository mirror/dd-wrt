/*
 * dnode.c - OpenBSD node functions for lsof
 */

/*
 * Copyright 1994 Purdue Research Foundation, West Lafayette, Indiana
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
    "@(#) Copyright 1994 Purdue Research Foundation.\nAll rights reserved.\n";
#endif

#include "common.h"

/*
 * process_vnode() - process vnode
 */
void process_vnode(struct lsof_context *ctx, struct kinfo_file *file) {
    enum lsof_fd_type fd_type;
    int num = -1;
    uint32_t flag;
    int mib[3];
    size_t size;
    char path[PATH_MAX];

    /* Alloc Lf and set fd */
    switch (file->fd_fd) {
    case KERN_FILE_TEXT:
        fd_type = LSOF_FD_PROGRAM_TEXT;
        break;
    case KERN_FILE_CDIR:
        fd_type = LSOF_FD_CWD;
        break;
    case KERN_FILE_RDIR:
        fd_type = LSOF_FD_ROOT_DIR;
        break;
    default:
        fd_type = LSOF_FD_NUMERIC;
        num = file->fd_fd;
        break;
    }
    alloc_lfile(ctx, fd_type, num);

    if (file->fd_fd == KERN_FILE_CDIR) {
        /*
         * Save current working directory information if available
         * sysctl(CTL_KERN, KERN_PROC_CWD, pid)
         */
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC_CWD;
        mib[2] = file->p_pid;
        size = sizeof(path);
        if (sysctl(mib, 3, path, &size, NULL, 0) >= 0) {
            (void)snpf(Namech, Namechl, "%s", path);
            enter_nm(ctx, Namech);
        }
    }

    /*
     * Construct access code.
     */
    if (file->fd_fd >= 0) {
        if ((flag = (file->f_flag & (FREAD | FWRITE))) == FREAD)
            Lf->access = LSOF_FILE_ACCESS_READ;
        else if (flag == FWRITE)
            Lf->access = LSOF_FILE_ACCESS_WRITE;
        else if (flag == (FREAD | FWRITE))
            Lf->access = LSOF_FILE_ACCESS_READ_WRITE;
    }

    /* Fill file size/offset */
    if (file->v_type == VBLK || file->v_type == VCHR) {
        /* blk/char devices have no size, only offset */
        if (file->f_offset != (uint64_t)(-1)) {
            Lf->off = file->f_offset;
            Lf->off_def = 1;
        }
    } else {
        Lf->off = file->f_offset;
        Lf->off_def = 1;
        Lf->sz = file->va_size;
        Lf->sz_def = 1;
    }

    /* Fill inode */
    Lf->inode = file->va_fileid;
    Lf->inp_ty = 1;

    /* Fill dev && rdef */
    Lf->dev = file->va_fsid;
    Lf->dev_def = 1;
    if (file->v_type == VBLK || file->v_type == VCHR) {
        Lf->rdev = file->va_rdev;
        Lf->rdev_def = 1;
    }

    /* Fill type */
    switch (file->v_type) {
    case VREG:
        Lf->ntype = N_REGLR;
        Lf->type = LSOF_FILE_VNODE_VREG;
        break;
    case VDIR:
        Lf->type = LSOF_FILE_VNODE_VDIR;
        break;
    case VCHR:
        Lf->ntype = N_CHR;
        Lf->type = LSOF_FILE_VNODE_VCHR;
        break;
    case VFIFO:
        Lf->ntype = N_FIFO;
        Lf->type = LSOF_FILE_VNODE_VFIFO;
        break;
    }

    /* No way to read file path, request mount info  */
    Lf->lmi_srch = 1;

    /* Fill number of links */
    Lf->nlink = file->va_nlink;
    Lf->nlink_def = 1;

    /* Handle link count filter */
    if (Nlink && (Lf->nlink < Nlink))
        Lf->sf |= SELNLINK;

    /* Handle name match, must be done late, because if_file_named checks
     * Lf->dev etc. */
    if (is_file_named(ctx, NULL, 0)) {
        Lf->sf |= SELNM;
    }

    /* Finish */
    if (Lf->sf)
        link_lfile(ctx);
}
