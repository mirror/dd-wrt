/*
 * LTbasic2.c -- Lsof Test basic tests 2
 *
 * The basic tests measure the finding by liblsof of its own open CWD, open
 * executable (when possible).
 *
 * V. Abell
 * Purdue University
 */

/*
 * Copyright 2002 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by V. Abell.
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

#include "lsof.h"
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char **argv) {
    struct lsof_result *result;
    struct lsof_context *ctx;
    struct lsof_process *p;
    struct lsof_file *f;
    int pi, fi;
    char buffer[128];
    int exec_found = 0; /* executable found in result */
    int cwd_found = 0;  /* cwd found in result */
    struct stat exec_stat;
    struct stat cwd_stat;
    if (stat(argv[0], &exec_stat)) {
        fprintf(stderr, "Cannot stat %s, skipping executable check\n", argv[0]);
        exec_found = 1;
    }
    if (stat(".", &cwd_stat)) {
        fprintf(stderr, "Cannot stat '.', skipping cwd check\n");
        cwd_found = 1;
    }

    ctx = lsof_new();
    lsof_select_process(ctx, "LTbasic2", 0);
    lsof_freeze(ctx);
    lsof_gather(ctx, &result);

    for (pi = 0; pi < result->num_processes; pi++) {
        p = &result->processes[pi];
        for (fi = 0; fi < p->num_files; fi++) {
            f = &p->files[fi];
            if (f->fd_type == LSOF_FD_PROGRAM_TEXT) {
                /* check if device and inode matches */
                if ((f->flags &
                     (LSOF_FILE_FLAG_DEV_VALID | LSOF_FILE_FLAG_INODE_VALID)) &&
                    f->dev == exec_stat.st_dev &&
                    f->inode == exec_stat.st_ino) {
                    exec_found = 1;
                }
            } else if (f->fd_type == LSOF_FD_CWD) {
                /* check if device and inode matches */
                if ((f->flags &
                     (LSOF_FILE_FLAG_DEV_VALID | LSOF_FILE_FLAG_INODE_VALID)) &&
                    f->dev == cwd_stat.st_dev && f->inode == cwd_stat.st_ino) {
                    cwd_found = 1;
                }
            }
        }
    }

    lsof_free_result(result);
    lsof_destroy(ctx);

    if (!exec_found) {
        fprintf(stderr, "ERROR!!!  open LTbasic2 executable wasn't found.\n");
    }
    if (!cwd_found) {
        fprintf(stderr, "ERROR!!!  current working directory wasn't found.\n");
    }
    return !(exec_found && cwd_found);
}