/*
 * dproc.c - OpenBSD process access functions for lsof
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

static void process_kinfo_file(struct lsof_context *ctx,
                               struct kinfo_file *file);

/*
 * Local static values
 */

/*
 * gather_proc_info() -- gather process information
 */
void gather_proc_info(struct lsof_context *ctx) {
    short pss, sf;
    uid_t uid;
    struct stat st;
    int mib[6];
    size_t size = 0;
    int res;

    struct kinfo_proc *procs = NULL;
    struct kinfo_proc *proc;
    int num_procs;
    int px; /* process loop index */

    struct kinfo_file *files = NULL;
    struct kinfo_file *file;
    int num_files;
    int fx; /* file loop index */

    char path[PATH_MAX];

    /*
     * Read the process table.
     */

    /* See OpenSBD kernel sys/kern/kern_sysctl.c sysctl_doproc */
    /* sysctl(CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0, sizeof(struct kinfo_proc),
     * count) */
    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;             /* op */
    mib[3] = 0;                         /* arg */
    mib[4] = sizeof(struct kinfo_proc); /* elem_size */

    /* Loop to probe size, learned from libkvm */
    while (1) {
        mib[5] = 0; /* elem_count */

        /* Probe number of entries */
        if (sysctl(mib, 6, NULL, &size, NULL, 0) < 0) {
            (void)fprintf(stderr, "%s: can't read process table: %d\n", Pn,
                          errno);
            Error(ctx);
        }

        /* Alloc more to handle new processes in the meantime */
        size = (size_t)(size / sizeof(struct kinfo_proc) * 1.1) *
               sizeof(struct kinfo_proc);

        if (!procs) {
            procs = (struct kinfo_proc *)malloc(size);
        } else {
            procs = (struct kinfo_proc *)realloc(procs, size);
        }
        if (!procs) {
            (void)fprintf(stderr, "%s: no kinfo_proc * space\n", Pn);
            Error(ctx);
        }

        mib[5] = size / sizeof(struct kinfo_proc); /* elem_count */
        res = sysctl(mib, 6, procs, &size, NULL, 0);
        if (res >= 0) {
            num_procs = size / sizeof(struct kinfo_proc);
            break;
        } else if (res < 0 && errno != ENOMEM) {
            (void)fprintf(stderr, "%s: can't read process table: %d\n", Pn,
                          errno);
            Error(ctx);
        }
    };

    /*
     * Examine proc structures and their associated information.
     */

    for (proc = procs, px = 0; px < num_procs; px++, proc++) {
        if (proc->p_stat == 0 || proc->p_stat == SZOMB)
            continue;
        /*
         * Read process information, process group structure (if
         * necessary), and User ID (if necessary).
         *
         * See if process is excluded.
         *
         * Read file structure pointers.
         */
        uid = proc->p_uid;
        if (is_proc_excl(ctx, (int)proc->p_pid, (int)proc->p__pgid,
                         (UID_ARG)uid, &pss, &sf)) {
            continue;
        }

        /*
         * Allocate a local process structure.
         */
        if (is_cmd_excl(ctx, proc->p_comm, &pss, &sf))
            continue;
        alloc_lproc(ctx, (int)proc->p_pid, (int)proc->p__pgid,
                    (int)proc->p_ppid, (UID_ARG)uid, proc->p_comm, (int)pss,
                    (int)sf);
        Plf = (struct lfile *)NULL; /* Empty list head */

        /*
         * Read open file structure pointers.
         * sysctl(CTL_KERN, KERN_FILE, KERN_FILE_BYPID, pid, sizeof(struct
         * kinfo_file), count)
         */
        mib[0] = CTL_KERN;
        mib[1] = KERN_FILE;
        mib[2] = KERN_FILE_BYPID;
        mib[3] = proc->p_pid;
        mib[4] = sizeof(struct kinfo_file);
        size = 0;

        /* Loop to probe size, learned from libkvm */
        while (1) {
            mib[5] = 0; /* elem_count */

            /* Probe number of entries */
            if (sysctl(mib, 6, NULL, &size, NULL, 0) < 0) {
                (void)fprintf(stderr, "%s: can't read file table: %d\n", Pn,
                              errno);
                Error(ctx);
            }

            /* Alloc more to handle new processes in the meantime */
            size = (size_t)(size / sizeof(struct kinfo_file) * 1.1) *
                   sizeof(struct kinfo_file);

            if (!files) {
                files = (struct kinfo_file *)malloc(size);
            } else {
                files = (struct kinfo_file *)realloc(files, size);
            }
            if (!files) {
                (void)fprintf(stderr, "%s: no kinfo_file * space\n", Pn);
                Error(ctx);
            }

            mib[5] = size / sizeof(struct kinfo_file); /* elem_count */
            res = sysctl(mib, 6, files, &size, NULL, 0);
            if (res >= 0) {
                num_files = size / sizeof(struct kinfo_file);
                break;
            } else if (res < 0 && errno != ENOMEM) {
                (void)fprintf(stderr, "%s: can't read file table: %d\n", Pn,
                              errno);
                Error(ctx);
            }
        };

        for (file = files, fx = 0; fx < num_files; fx++, file++) {
            process_kinfo_file(ctx, file);
        }

        /*
         * Examine results.
         */
        if (examine_lproc(ctx))
            return;
    }
}

/*
 * initialize() - perform all initialization
 */
void initialize(struct lsof_context *ctx) {}

/*
 * process_kinfo_file() - process kinfo_file
 */
void process_kinfo_file(struct lsof_context *ctx, struct kinfo_file *file) {
    switch (file->f_type) {
    case DTYPE_VNODE: /* file */
        process_vnode(ctx, file);
        break;
    case DTYPE_SOCKET:
        process_socket(ctx, file);
        break;
    case DTYPE_PIPE:
        process_pipe(ctx, file);
        break;
    case DTYPE_KQUEUE:
        process_kqueue_file(ctx, file);
        break;
    }
}