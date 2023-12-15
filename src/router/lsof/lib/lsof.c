/*
 * lsof.c -- implement lsof_* functions() for liblsof
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

#ifdef AUTOTOOLS
#    include "config.h"
#endif
#include "common.h"
#include "lsof.h"
#include <unistd.h>

#ifndef API_EXPORT
#    define API_EXPORT
#endif

API_EXPORT
int lsof_get_api_version() { return LSOF_API_VERSION; }

#ifdef AUTOTOOLS
API_EXPORT
char *lsof_get_library_version() { return PACKAGE_VERSION; }
#endif

API_EXPORT
struct lsof_context *lsof_new() {
    struct lsof_context *ctx =
        (struct lsof_context *)malloc(sizeof(struct lsof_context));
    if (ctx) {
        /* Initialization */
        memset(ctx, 0, sizeof(struct lsof_context));

        if (!(Namech = (char *)malloc(MAXPATHLEN + 1))) {
            free(ctx);
            return NULL;
        }
        Namechl = (size_t)(MAXPATHLEN + 1);

#if defined(WARNINGSTATE)
        /* suppress warnings */
        Fwarn = 1;
#else  /* !defined(WARNINGSTATE) */
        /* display warnings */
        Fwarn = 0;
#endif /* defined(WARNINGSTATE) */

#if defined(HASXOPT_VALUE)
        /* -X option status */
        Fxopt = HASXOPT_VALUE;
#endif /* defined(HASXOPT_VALUE) */

        /* -1 == none */
        FdlTy = -1;

        /* Readlink() and stat() timeout (seconds) */
        TmLimit = TMLIMIT;

        /* default */
        AllProc = 1;

        /* -1 == none */
        FdlTy = -1;

        /* device cache file descriptor */
        DCfd = -1;

        /* device cache path index: -1 = path not defined */
        DCpathX = -1;

        /* device cache state: 3 = update; read and rebuild if necessary */
        DCstate = 3;

        /* COMMAND column width limit */
        CmdLim = CMDL;
    }
    return ctx;
}

API_EXPORT
enum lsof_error lsof_avoid_blocking(struct lsof_context *ctx, int avoid) {
    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }
    Fblock = avoid;
    return LSOF_EXIT_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_avoid_forking(struct lsof_context *ctx, int avoid) {
    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }
    Fovhd = avoid;
    return LSOF_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_logic_and(struct lsof_context *ctx) {
    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }
    Fand = 1;
    return LSOF_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_select_process(struct lsof_context *ctx, char *command,
                                    int exclude) {
    char *cp; /* command copy */
    MALLOC_S len;
    struct str_lst *lpt, *str;
    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }

    /*
     * Check for command inclusion/exclusion conflicts.
     */
    for (str = Cmdl; str; str = str->next) {
        if (str->x != exclude) {
            if (!strcmp(str->str, command)) {
                if (ctx->err) {
                    (void)fprintf(ctx->err, "%s: -c^%s and -c%s conflict.\n",
                                  Pn, str->str, command);
                }
                return LSOF_ERROR_INVALID_ARGUMENT;
            }
        }
    }

    if (!(cp = mkstrcpy(command, &len))) {
        if (ctx->err) {
            (void)fprintf(ctx->err, "%s: no string copy space: ", Pn);
            safestrprt(command, ctx->err, 1);
        }
        return LSOF_ERROR_NO_MEMORY;
    }

#if defined(MAXSYSCMDL)
    if (len > MAXSYSCMDL) {
        /* Impossible to match */
        if (ctx->err) {
            (void)fprintf(ctx->err, "%s: \"-c ", Pn);
            (void)safestrprt(command, ctx->err, 2);
            (void)fprintf(ctx->err, "\" length (%zu) > what system", len);
            (void)fprintf(ctx->err, " provides (%d)\n", MAXSYSCMDL);
        }
        CLEAN(cp);
        return LSOF_ERROR_INVALID_ARGUMENT;
    }
#endif

    if ((lpt = (struct str_lst *)malloc(sizeof(struct str_lst))) == NULL) {
        if (ctx->err) {
            safestrprt(command, ctx->err, 1);
            (void)fprintf(ctx->err, "%s: no list space: ", Pn);
            safestrprt(command, ctx->err, 1);
        }
        CLEAN(cp);
        return LSOF_ERROR_NO_MEMORY;
    }

    /* Insert into list */
    lpt->f = 0;
    lpt->str = cp;
    lpt->len = (int)len;
    lpt->x = exclude;
    if (exclude) {
        Cmdnx++;
    } else {
        Cmdni++;
        /* Update selection flag for inclusions */
        Selflags |= SELCMD;
    }
    lpt->next = Cmdl;
    Cmdl = lpt;

    return LSOF_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_set_output_stream(struct lsof_context *ctx, FILE *fp,
                                       char *program_name, int warn) {
    if (!ctx) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }
    ctx->err = fp;
    ctx->program_name = mkstrcpy(program_name, NULL);
    ctx->warn = warn;
    return LSOF_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_freeze(struct lsof_context *ctx) {
    if (ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }

    if (Selflags == 0) {
        Selflags = SelAll;
    } else {
        if ((Selflags & (SELNA | SELNET)) != 0 &&
            (Selflags & ~(SELNA | SELNET)) == 0)
            Selinet = 1;
        AllProc = 0;
    }

    initialize(ctx);
    hashSfile(ctx);
    ctx->frozen = 1;
    return LSOF_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_gather(struct lsof_context *ctx,
                            struct lsof_result **result) {
    enum lsof_error ret = LSOF_SUCCESS;
    int pi = 0;  /* proc index */
    int upi = 0; /* user proc index */
    struct lsof_process *p;
    struct lproc *lp;
    int fi = 0; /* file index */
    size_t num_files;
    struct lsof_file *f;
    struct lfile *lf;
    struct lfile *lf_next;
    size_t sel_procs = 0;
    char *cp;
    char buf[64];
    int s;
    struct str_lst *str;
    struct sfile *sfp;
    struct nwad *np, *npn;
#if defined(HASPROCFS)
    struct procfsid *pfi;
#endif /* defined(HASPROCFS) */
#if defined(HASZONES)
    znhash_t *zp;
#endif /* defined(HASZONES) */
#if defined(HASSELINUX)
    cntxlist_t *cntxp;
#endif /* defined(HASSELINUX) */
    int pass;
    int i;
    struct lsof_selection *selections;
    size_t num_selections = 0;

    if (!result) {
        ret = LSOF_ERROR_INVALID_ARGUMENT;
        return ret;
    } else if (!ctx->frozen) {
        ret = lsof_freeze(ctx);
        if (ret != LSOF_SUCCESS)
            return ret;
    }

    gather_proc_info(ctx);

    /* Cleanup orphaned cur_file, if any*/
    if (ctx->cur_file) {
        CLEAN(ctx->cur_file->dev_ch);
        CLEAN(ctx->cur_file->nm);
        CLEAN(ctx->cur_file->nma);
        CLEAN(ctx->cur_file);
    }

    /* Count selected procs */
    for (pi = 0; pi < ctx->procs_size; pi++) {
        lp = &ctx->procs[pi];
        if (lp->pss) {
            sel_procs++;
        }
    }

    /* Fill result */
    struct lsof_result *res =
        (struct lsof_result *)malloc(sizeof(struct lsof_result));
    struct lsof_process *user_procs =
        (struct lsof_process *)malloc(sizeof(struct lsof_process) * sel_procs);
    memset(user_procs, 0, sizeof(struct lsof_process) * sel_procs);

    for (pi = 0, upi = 0; pi < ctx->procs_size; pi++) {
        /* Copy fields from lproc */
        lp = &ctx->procs[pi];
        if (lp->pss) {
            /* selected process */
            p = &user_procs[upi++];

            p->command = lp->cmd;
            lp->cmd = NULL;
            p->pid = lp->pid;

#if defined(HASTASKS)
            p->tid = lp->tid;
            p->task_cmd = lp->tcmd;
            lp->tcmd = NULL;
#endif
#if defined(HASZONES)
            p->solaris_zone = lp->zn;
            lp->zn = NULL;
#endif
#if defined(HASSELINUX)
            p->selinux_context = lp->cntx;
            lp->cntx = NULL;
#endif

            p->pgid = lp->pgid;
            p->ppid = lp->ppid;
            p->uid = lp->uid;

            /* Compute number of files in the linked list */
            num_files = 0;
            for (lf = lp->file; lf; lf = lf->next) {
                if (!is_file_sel(ctx, lp, lf))
                    continue;
                num_files++;
            }

            p->files = (struct lsof_file *)malloc(sizeof(struct lsof_file) *
                                                  num_files);
            memset(p->files, 0, sizeof(struct lsof_file) * num_files);
            p->num_files = num_files;
            for (fi = 0, lf = lp->file; lf; lf = lf_next) {
                if (is_file_sel(ctx, lp, lf)) {
                    /* Copy fields from lfile */
                    f = &p->files[fi++];
                    f->flags = 0;

                    /* FD column */
                    f->fd_type = lf->fd_type;
                    f->fd_num = lf->fd_num;
                    f->access = lf->access;
                    f->lock = lf->lock;

                    /* TYPE column */
                    f->file_type = lf->type;
                    f->unknown_file_type_number = lf->unknown_file_type_number;

                    /* DEVICE column */
                    f->dev = lf->dev;
                    if (lf->dev_def) {
                        f->flags |= LSOF_FILE_FLAG_DEV_VALID;
                    }
                    f->rdev = lf->rdev;
                    if (lf->rdev_def) {
                        f->flags |= LSOF_FILE_FLAG_RDEV_VALID;
                    }

                    /* SIZE, SIZE/OFF, OFFSET column */
                    f->size = lf->sz;
                    if (lf->sz_def) {
                        f->flags |= LSOF_FILE_FLAG_SIZE_VALID;
                    }
                    f->offset = lf->off;
                    if (lf->off_def) {
                        f->flags |= LSOF_FILE_FLAG_OFFSET_VALID;
                    }

                    /* NLINK column */
                    f->num_links = lf->nlink;
                    if (lf->nlink_def) {
                        f->flags |= LSOF_FILE_FLAG_NUM_LINKS_VALID;
                    }

                    /* NODE column */
                    f->inode = lf->inode;
                    if (lf->inp_ty == 1 || lf->inp_ty == 3) {
                        f->flags |= LSOF_FILE_FLAG_INODE_VALID;
                    }

                    /* NAME column */
                    f->name = lf->nm;
                    lf->nm = NULL;
                }
                lf_next = lf->next;
            }
        }

        for (lf = lp->file; lf; lf = lf_next) {
            /* free lf */
            lf_next = lf->next;
            CLEAN(lf->nma);
            CLEAN(lf->dev_ch);
#if defined(CLRLFILEADD)
            CLRLFILEADD(lf)
#endif /* defined(CLRLFILEADD) */
            CLEAN(lf);
        }
        lp->file = NULL;

        /* skip and free */
        CLEAN(lp->cmd);
#if defined(HASTASKS)
        CLEAN(lp->tcmd);
#endif
#if defined(HASSELINUX)
        CLEAN(lp->cntx);
#endif /* defined(HASSELINUX) */
        continue;
    }

    /* Cleanup */
    CLEAN(ctx->procs);
    ctx->cur_proc = NULL;

    res->processes = user_procs;
    res->num_processes = sel_procs;

    ctx->procs_size = ctx->procs_cap = 0;
    ctx->cur_file = ctx->prev_file = NULL;

    /* Collect selection result */
    for (pass = 0; pass < 2; pass++) {
        num_selections = 0;

        /* command */
        for (str = Cmdl; str; str = str->next) {
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_COMMAND;
                selections[num_selections].found = str->f;
                selections[num_selections].string = str->str;
            }
            num_selections++;
        }

        /* command regex */
        for (i = 0; i < NCmdRxU; i++) {
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_COMMAND_REGEX;
                selections[num_selections].found = CmdRx[i].mc > 0;
                selections[num_selections].string = CmdRx[i].exp;
            }
            num_selections++;
        }

        /* select file or file system */
        for (sfp = Sfile; sfp; sfp = sfp->next) {
            if (pass) {
                selections[num_selections].type =
                    sfp->type ? LSOF_SELECTION_PATH
                              : LSOF_SELECTION_FILE_SYSTEM;
                selections[num_selections].found = sfp->f;
                selections[num_selections].string = sfp->aname;
            }
            num_selections++;
        }

#if defined(HASPROCFS)
        /* procfs */
        if (Procsrch) {
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_FILE_SYSTEM;
                selections[num_selections].found = Procfind;
                selections[num_selections].string =
                    Mtprocfs ? Mtprocfs->dir : HASPROCFS;
            }
            num_selections++;
        }

        for (pfi = Procfsid; pfi; pfi = pfi->next) {
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_PATH;
                selections[num_selections].found = pfi->f;
                selections[num_selections].string = pfi->nm;
            }
            num_selections++;
        }
#endif /* defined(HASPROCFS) */

        /* network address */
        for (np = Nwad; np;) {
            int found = np->f;
            if (!(cp = np->arg)) {
                np = np->next;
                continue;
            }
            for (npn = np->next; npn; npn = npn->next) {
                if (!npn->arg)
                    continue;
                if (!strcmp(cp, npn->arg)) {
                    /* Found duplicate specification */
                    found |= npn->f;
                } else {
                    break;
                }
            }

            if (pass) {
                selections[num_selections].type =
                    LSOF_SELECTION_NETWORK_ADDRESS;
                selections[num_selections].found = found;
                selections[num_selections].string = np->arg;
            }
            num_selections++;
            np = npn;
        }

        /* ip protocol */
        if (Fnet) {
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_INTERNET;
                selections[num_selections].found = Fnet == 2;
            }
            num_selections++;
        }

#if defined(HASTCPUDPSTATE)
        /* tcp/tpi protocol state */
        if (TcpStIn) {
            for (i = 0; i < TcpNstates; i++) {
                if (TcpStI[i]) {
                    if (pass) {
                        selections[num_selections].type =
                            LSOF_SELECTION_PROTOCOL_STATE;
                        selections[num_selections].found = TcpStI[i] == 2;
                        selections[num_selections].string = TcpSt[i];
                    }
                    num_selections++;
                }
            }
        }
        if (UdpStIn) {
            for (i = 0; i < UdpNstates; i++) {
                if (UdpStI[i]) {
                    if (pass) {
                        selections[num_selections].type =
                            LSOF_SELECTION_PROTOCOL_STATE;
                        selections[num_selections].found = UdpStI[i] == 2;
                        selections[num_selections].string = UdpSt[i];
                    }
                    num_selections++;
                }
            }
        }
#endif /* defined(HASTCPUDPSTATE) */

        /* nfs */
        if (Fnfs) {
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_NFS;
                selections[num_selections].found = Fnfs == 2;
            }
            num_selections++;
        }

        /* pid */
        for (i = 0; i < Npid; i++) {
            if (Spid[i].x)
                continue;
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_PID;
                selections[num_selections].found = Spid[i].f;
                selections[num_selections].integer = Spid[i].i;
            }
            num_selections++;
        }

        /* pgid */
        for (i = 0; i < Npgid; i++) {
            if (Spgid[i].x)
                continue;
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_PGID;
                selections[num_selections].found = Spgid[i].f;
                selections[num_selections].integer = Spgid[i].i;
            }
            num_selections++;
        }

        /* uid */
        for (i = 0; i < Nuid; i++) {
            if (Suid[i].excl)
                continue;
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_UID;
                selections[num_selections].found = Suid[i].f;
                selections[num_selections].string = Suid[i].lnm;
                selections[num_selections].integer = Suid[i].uid;
            }
            num_selections++;
        }

#if defined(HASTASKS)
        /* tasks */
        if (Ftask) {
            if (pass) {
                selections[num_selections].type = LSOF_SELECTION_TASK;
                selections[num_selections].found = Ftask == 2;
            }
            num_selections++;
        }
#endif /* defined(HASTASKS) */

#if defined(HASZONES)
        /* solaris zones */
        if (ZoneArg) {
            for (i = 0; i < HASHZONE; i++) {
                for (zp = ZoneArg[i]; zp; zp = zp->next) {
                    if (pass) {
                        selections[num_selections].type =
                            LSOF_SELECTION_SOLARIS_ZONE;
                        selections[num_selections].found = zp->f;
                        selections[num_selections].string = zp->zn;
                    }
                    num_selections++;
                }
            }
        }
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
        /* SELinux context */
        if (CntxArg) {
            for (cntxp = CntxArg; cntxp; cntxp = cntxp->next) {
                if (pass) {
                    selections[num_selections].type =
                        LSOF_SELECTION_SELINUX_CONTEXT;
                    selections[num_selections].found = cntxp->f;
                    selections[num_selections].string = cntxp->cntx;
                }
                num_selections++;
            }
        }
#endif /* defined(HASSELINUX) */

        /* allocation selections array */
        if (pass == 0) {
            selections = (struct lsof_selection *)malloc(
                sizeof(struct lsof_selection) * num_selections);
            memset(selections, 0,
                   sizeof(struct lsof_selection) * num_selections);
            res->selections = selections;
            res->num_selections = num_selections;
        }
    }

    /* Params */
    *result = res;

    return ret;
}

API_EXPORT
void lsof_destroy(struct lsof_context *ctx) {
    int i;
    struct str_lst *str_lst, *str_lst_next;
    struct int_lst *int_lst, *int_lst_next;
    struct mounts *mnt, *mnt_next;
    if (!ctx) {
        return;
    }
    /* Free parameters */
    for (str_lst = Cmdl; str_lst; str_lst = str_lst_next) {
        str_lst_next = str_lst->next;
        CLEAN(str_lst->str);
        CLEAN(str_lst);
    }
    CLEAN(Spid);
    CLEAN(Spgid);
    for (i = 0; i < Nuid; i++) {
        CLEAN(Suid[i].lnm);
    }
    CLEAN(Suid);
    CLEAN(Nmlst);

    /* Free temporary */
    CLEAN(Namech);
#if defined(HASNLIST)
    CLEAN(Nl);
    Nll = 0;
#endif /* defined(HASNLIST) */

    /* Free local mount info */
    if (Lmist) {
        for (mnt = Lmi; mnt; mnt = mnt_next) {
            mnt_next = mnt->next;
            CLEAN(mnt->dir);
            CLEAN(mnt->fsname);
            CLEAN(mnt->fsnmres);
#if defined(HASFSTYPE)
            CLEAN(mnt->fstype);
#endif
            CLEAN(mnt);
        }
        Lmi = NULL;
        Lmist = 0;
    }

    /* state table */
#if !defined(USE_LIB_PRINT_TCPTPI)
    for (i = 0; i < TcpNstates; i++) {
        CLEAN(TcpSt[i]);
    }
    CLEAN(TcpSt);
#endif /* !defined(USE_LIB_PRINT_TCPTPI) */
    for (i = 0; i < UdpNstates; i++) {
        CLEAN(UdpSt[i]);
    }
    CLEAN(UdpSt);
    CLEAN(Pn);

    CLEAN(ctx);
}

API_EXPORT
void lsof_free_result(struct lsof_result *result) {
    int pi, fi;
    struct lsof_process *p;
    struct lsof_file *f;
    for (pi = 0; pi < result->num_processes; pi++) {
        p = &result->processes[pi];
        /* Free files */
        for (fi = 0; fi < p->num_files; fi++) {
            f = &p->files[fi];
            CLEAN(f->name);
        }
        CLEAN(p->files);

        /* Free process fields */
        CLEAN(p->command);
        CLEAN(p->task_cmd);
        CLEAN(p->solaris_zone);
        CLEAN(p->selinux_context);
    }
    CLEAN(result->processes);
    CLEAN(result->selections);
    CLEAN(result);
}

API_EXPORT
enum lsof_error lsof_select_process_regex(struct lsof_context *ctx, char *x) {
    int bmod = 0;
    int bxmod = 0;
    int i, re;
    int imod = 0;
    int xmod = 0;
    int co = REG_NOSUB | REG_EXTENDED;
    char reb[256], *xb, *xe, *xm;
    MALLOC_S xl;
    char *xp = (char *)NULL;
    enum lsof_error ret = LSOF_SUCCESS;

    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }

    /*
     * Make sure the supplied string starts a regular expression.
     */
    if (!*x || (*x != '/')) {
        if (ctx->err) {
            (void)fprintf(ctx->err, "%s: regexp doesn't begin with '/': ", Pn);
            if (x)
                safestrprt(x, ctx->err, 1);
        }
        ret = LSOF_ERROR_INVALID_ARGUMENT;
        goto cleanup;
    }

    /*
     * Skip to the end ('/') of the regular expression.
     */
    xb = x + 1;
    for (xe = xb; *xe; xe++) {
        if (*xe == '/')
            break;
    }
    if (*xe != '/') {
        if (ctx->err) {
            (void)fprintf(ctx->err, "%s: regexp doesn't end with '/': ", Pn);
            safestrprt(x, ctx->err, 1);
        }
        ret = LSOF_ERROR_INVALID_ARGUMENT;
        goto cleanup;
    }

    /*
     * Decode any regular expression modifiers.
     */
    for (i = 0, xm = xe + 1; *xm; xm++) {
        switch (*xm) {
        case 'b': /* This is a basic expression. */
            if (++bmod > 1) {
                if (bmod == 2 && ctx->err) {
                    (void)fprintf(ctx->err,
                                  "%s: b regexp modifier already used: ", Pn);
                    safestrprt(x, ctx->err, 1);
                }
                i = 1;
            } else if (xmod) {
                if (++bxmod == 1 && ctx->err) {
                    (void)fprintf(
                        ctx->err,
                        "%s: b and x regexp modifiers conflict: ", Pn);
                    safestrprt(x, ctx->err, 1);
                }
                i = 1;
            } else
                co &= ~REG_EXTENDED;
            break;
        case 'i': /* Ignore case. */
            if (++imod > 1) {
                if (imod == 2 && ctx->err) {
                    (void)fprintf(ctx->err,
                                  "%s: i regexp modifier already used: ", Pn);
                    safestrprt(x, ctx->err, 1);
                }
                i = 1;
            } else
                co |= REG_ICASE;
            break;
        case 'x': /* This is an extended expression. */
            if (++xmod > 1) {
                if (xmod == 2 && ctx->err) {
                    (void)fprintf(ctx->err,
                                  "%s: x regexp modifier already used: ", Pn);
                    safestrprt(x, ctx->err, 1);
                }
                i = 1;
            } else if (bmod) {
                if (++bxmod == 1 && ctx->err) {
                    (void)fprintf(
                        ctx->err,
                        "%s: b and x regexp modifiers conflict: ", Pn);
                    safestrprt(x, ctx->err, 1);
                }
                i = 1;
            } else
                co |= REG_EXTENDED;
            break;
        default:
            if (ctx->err)
                (void)fprintf(ctx->err, "%s: invalid regexp modifier: %c\n", Pn,
                              (int)*xm);
            i = 1;
        }
    }
    if (i) {
        ret = LSOF_ERROR_INVALID_ARGUMENT;
        goto cleanup;
    }

    /*
     * Allocate space to hold expression and copy it there.
     */
    xl = (MALLOC_S)(xe - xb);
    if (!(xp = (char *)malloc(xl + 1))) {
        if (ctx->err) {
            (void)fprintf(ctx->err, "%s: no regexp space for: ", Pn);
            safestrprt(x, ctx->err, 1);
        }
        Error(ctx);
        ret = LSOF_ERROR_NO_MEMORY;
        goto cleanup;
    }
    (void)strncpy(xp, xb, xl);
    xp[(int)xl] = '\0';
    /*
     * Assign a new CmdRx[] slot for this expression.
     */
    if (NCmdRxA <= NCmdRxU) {
        /*
         * More CmdRx[] space must be assigned.
         */
        NCmdRxA += 32;
        xl = (MALLOC_S)(ctx->cmd_regex_cap * sizeof(lsof_rx_t));
        if (CmdRx)
            CmdRx = (lsof_rx_t *)realloc((MALLOC_P *)CmdRx, xl);
        else
            CmdRx = (lsof_rx_t *)malloc(xl);
        if (!CmdRx) {
            if (ctx->err) {
                (void)fprintf(ctx->err, "%s: no space for regexp: ", Pn);
                safestrprt(x, ctx->err, 1);
            }
            Error(ctx);
            ret = LSOF_ERROR_NO_MEMORY;
            goto cleanup;
        }
    }
    i = NCmdRxU;
    CmdRx[i].exp = xp;
    /*
     * Compile the expression.
     */
    if ((re = regcomp(&CmdRx[i].cx, xp, co))) {
        if (ctx->err) {
            (void)fprintf(ctx->err, "%s: regexp error: ", Pn);
            safestrprt(x, ctx->err, 0);
            (void)regerror(re, &CmdRx[i].cx, &reb[0], sizeof(reb));
            (void)fprintf(ctx->err, ": %s\n", reb);
        }
        ret = LSOF_ERROR_INVALID_ARGUMENT;
        goto cleanup;
    }
    /*
     * Complete the CmdRx[] table entry.
     */
    CmdRx[i].mc = 0;
    CmdRx[i].exp = xp;
    NCmdRxU++;

    /** Update selection flags for inclusion */
    if (CmdRx)
        Selflags |= SELCMD;
    ret = LSOF_SUCCESS;
cleanup:
    CLEAN(xp);
    return ret;
}

/* Internel helper for lsof_select_pid/pgid */
enum lsof_error lsof_select_pid_pgid(struct lsof_context *ctx, int32_t id,
                                     struct int_lst **sel, int *cap, int *size,
                                     int *incl_num, int *excl_num, int exclude,
                                     int is_pid) {
    int i, j;
    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }

    /*
     * Avoid entering duplicates and conflicts.
     */
    for (i = 0; i < *size; i++) {
        if (id == (*sel)[i].i) {
            if (exclude == (*sel)[i].x) {
                return LSOF_SUCCESS;
            }
            if (ctx->err) {
                (void)fprintf(ctx->err,
                              "%s: P%sID %d has been included and excluded.\n",
                              Pn, is_pid ? "" : "G", id);
            }
            return LSOF_ERROR_INVALID_ARGUMENT;
        }
    }

    /*
     * Allocate table table space.
     */
    if (*size >= *cap) {
        *cap += 10;
        if (!(*sel))
            *sel = (struct int_lst *)malloc(
                (MALLOC_S)(sizeof(struct int_lst) * (*cap)));
        else
            *sel = (struct int_lst *)realloc(
                (MALLOC_P *)(*sel),
                (MALLOC_S)(sizeof(struct int_lst) * (*cap)));
        if (!(*sel)) {
            if (ctx->err) {
                (void)fprintf(ctx->err, "%s: no space for %d process%s IDs", Pn,
                              *cap, is_pid ? "" : " group");
            }
            Error(ctx);
            return LSOF_ERROR_NO_MEMORY;
        }
    }

    /* Insert selection into sel_pid/sel_pgid*/
    (*sel)[*size].f = 0;
    (*sel)[*size].i = id;
    (*sel)[(*size)++].x = exclude;
    if (exclude)
        (*excl_num)++;
    else {
        (*incl_num)++;
        /* Update selection flags */
        Selflags |= is_pid ? SELPID : SELPGID;
    }
    return LSOF_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_select_pid(struct lsof_context *ctx, uint32_t pid,
                                int exclude) {
    enum lsof_error res = lsof_select_pid_pgid(ctx, pid, &Spid, &Mxpid, &Npid,
                                               &Npidi, &Npidx, exclude, 1);
    /* Record number of unselected PIDs for optimization */
    Npuns = Npid;
    return res;
}

API_EXPORT
enum lsof_error lsof_select_pgid(struct lsof_context *ctx, uint32_t pgid,
                                 int exclude) {
    return lsof_select_pid_pgid(ctx, pgid, &Spgid, &Mxpgid, &Npgid, &Npgidi,
                                &Npgidx, exclude, 0);
}

/* Internal helper for lsof_select_uid/lsof_select_login*/
enum lsof_error lsof_select_uid_login(struct lsof_context *ctx, uint32_t uid,
                                      char *login, int exclude) {
    int i, j;
    MALLOC_S len;
    char *lp;

    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }

    /*
     * Avoid entering duplicates.
     */
    for (i = 0; i < Nuid; i++) {
        if (uid != Suid[i].uid)
            continue;
        /* Duplicate */
        if (Suid[i].excl == exclude) {
            return LSOF_SUCCESS;
        }

        /* Conflict */
        if (ctx->err) {
            (void)fprintf(ctx->err,
                          "%s: UID %d has been included and excluded.\n", Pn,
                          (int)uid);
        }
        return LSOF_ERROR_INVALID_ARGUMENT;
    }

    /*
     * Allocate space for User IDentifier.
     */
    if (Nuid >= Mxuid) {
        Mxuid += 10;
        len = (MALLOC_S)(Mxuid * sizeof(struct seluid));
        if (!Suid)
            Suid = (struct seluid *)malloc(len);
        else
            Suid = (struct seluid *)realloc((MALLOC_P *)Suid, len);
        if (!Suid) {
            if (ctx->err) {
                (void)fprintf(ctx->err, "%s: no space for UIDs", Pn);
            }
            Error(ctx);
            return LSOF_ERROR_NO_MEMORY;
        }
    }
    if (login) {
        if (!(lp = mkstrcpy(login, (MALLOC_S *)NULL))) {
            if (ctx->err) {
                (void)fprintf(ctx->err, "%s: no space for login: ", Pn);
                safestrprt(login, ctx->err, 1);
            }
            Error(ctx);
            return LSOF_ERROR_NO_MEMORY;
        }
        Suid[Nuid].lnm = lp;
    } else
        Suid[Nuid].lnm = (char *)NULL;
    Suid[Nuid].f = 0;
    Suid[Nuid].uid = uid;
    Suid[Nuid++].excl = exclude;
    if (exclude)
        Nuidexcl++;
    else {
        Nuidincl++;
        Selflags |= SELUID;
    }
    return LSOF_SUCCESS;
}

API_EXPORT
enum lsof_error lsof_select_uid(struct lsof_context *ctx, uint32_t uid,
                                int exclude) {
    return lsof_select_uid_login(ctx, uid, NULL, exclude);
}

API_EXPORT
enum lsof_error lsof_select_login(struct lsof_context *ctx, char *login,
                                  int exclude) {
    struct passwd *pw;
    if (!ctx || ctx->frozen) {
        return LSOF_ERROR_INVALID_ARGUMENT;
    }

    /* Convert login to uid, then call lsof_select_uid_login */
    if ((pw = getpwnam(login)) == NULL) {
        if (ctx->err) {
            (void)fprintf(ctx->err, "%s: can't get UID for ", Pn);
            safestrprt(login, ctx->err, 1);
        }
        return LSOF_ERROR_INVALID_ARGUMENT;
    }
    return lsof_select_uid_login(ctx, pw->pw_uid, login, exclude);
}