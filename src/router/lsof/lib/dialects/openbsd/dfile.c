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
 * process_kqueue() -- process kqueue file
 */
void process_kqueue_file(struct lsof_context *ctx, struct kinfo_file *file) {
    char buf[64];
    int flag;

    /* Alloc Lf and set fd */
    alloc_lfile(ctx, LSOF_FD_NUMERIC, file->fd_fd);

    /* Fill type name*/
    Lf->type = LSOF_FILE_KQUEUE;

    /* Fill dev with f_data if available */
    if (file->f_data) {
        (void)snpf(buf, sizeof(buf), "0x%" PRIx64, file->f_data);
        enter_dev_ch(ctx, buf);
    }

    /*
     * Construct access code.
     */
    if ((flag = (file->f_flag & (FREAD | FWRITE))) == FREAD)
        Lf->access = LSOF_FILE_ACCESS_READ;
    else if (flag == FWRITE)
        Lf->access = LSOF_FILE_ACCESS_WRITE;
    else if (flag == (FREAD | FWRITE))
        Lf->access = LSOF_FILE_ACCESS_READ_WRITE;

    /* Finish */
    if (Lf->sf)
        link_lfile(ctx);
}

/*
 * process_pipe() - process a file structure whose type is DTYPE_PIPE
 */
void process_pipe(struct lsof_context *ctx, struct kinfo_file *file) {}