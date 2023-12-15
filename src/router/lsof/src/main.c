/*
 * main.c - common main function for lsof
 *
 * V. Abell, Purdue University
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

#include "common.h"
#include "cli.h"

/*
 * Local definitions
 */

static int GObk[] = {1, 1};      /* option backspace values */
static char GOp;                 /* option prefix -- '+' or '-' */
static char *GOv = (char *)NULL; /* option `:' value pointer */
static int GOx1 = 1;             /* first opt[][] index */
static int GOx2 = 0;             /* second opt[][] index */

static int GetOpt(struct lsof_context *ctx, int ct, char *opt[], char *rules,
                  int *err);
static char *sv_fmt_str(struct lsof_context *ctx, char *f);

/*
 * main() - main function for lsof
 */

int main(int argc, char *argv[]) {
    enum ExitStatus rv;
    int gopt_rv;
    int ad, c, i, n, se1, se2, ss;
    char *cp;
    int err = 0;
    enum ExitStatus ev = LSOF_EXIT_SUCCESS;
    int fh = 0;
    char *fmtr = (char *)NULL;
    long l;
    MALLOC_S len;
    struct lfile *lf;
    struct nwad *np, *npn;
    char options[128];
    int rc = 0;
    struct stat sb;
    struct sfile *sfp;
    struct lproc **slp = (struct lproc **)NULL;
    int sp = 0;
    struct str_lst *str, *strt;
    int version = 0;
    int xover = 0;
    int pr_count = 0;
    /** liblsof context */
    struct lsof_context *ctx = NULL;

#if defined(HAS_STRFTIME)
    char *fmt = (char *)NULL;
    size_t fmtl = (size_t)0;
#endif /* defined(HAS_STRFTIME) */

#if defined(HASZONES)
    znhash_t *zp;
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
    /*
     * This stanza must be immediately before the "Save progam name." code,
     * since it contains code itself.
     */
    cntxlist_t *cntxp;

    CntxStatus = is_selinux_enabled() ? 1 : 0;
#endif /* defined(HASSELINUX) */

    /* Initialize lsof context */
    ctx = lsof_new();

    /*
     * Save program name.
     */
    if ((Pn = strrchr(argv[0], '/')))
        Pn++;
    else
        Pn = argv[0];
    lsof_set_output_stream(ctx, stderr, Pn, 0);

    /*
     * Close enough file descriptors above 2 that library functions will have
     * open descriptors.
     *
     * Make sure stderr, stdout, and stdin are open descriptors.  Open /dev/null
     * for ones that aren't.  Be terse.
     *
     * Make sure umask allows lsof to define its own file permissions.
     */

    if ((MaxFd = (int)GET_MAX_FD()) < 53)
        MaxFd = 53;

    closefrom_shim(ctx, 3);

    while (((i = open("/dev/null", O_RDWR, 0)) >= 0) && (i < 2))
        ;
    if (i < 0)
        Error(ctx);
    if (i > 2)
        (void)close(i);
    (void)umask(0);

#if defined(HASSETLOCALE)
    /*
     * Set locale to environment's definition.
     */
    (void)setlocale(LC_CTYPE, "");
#endif /* defined(HASSETLOCALE) */

    /*
     * Common initialization.
     */
    Mypid = getpid();
    if ((Mygid = (gid_t)getgid()) != getegid())
        Setgid = 1;
    Euid = geteuid();
    if ((Myuid = (uid_t)getuid()) && !Euid)
        Setuidroot = 1;
    /*
     * Create option mask.
     */
    (void)snpf(options, sizeof(options),
               "?a%sbc:%sD:d:%s%sf:F:g:hHi:%s%slL:%s%snNo:Op:QPr:%ss:S:tT:u:"
               "UvVwx:%s%s%s",

#if defined(HAS_AFS) && defined(HASAOPT)
               "A:",
#else  /* !defined(HAS_AFS) || !defined(HASAOPT) */
               "",
#endif /* defined(HAS_AFS) && defined(HASAOPT) */

#if defined(HASNCACHE)
               "C",
#else  /* !defined(HASNCACHE) */
               "",
#endif /* defined(HASNCACHE) */

#if defined(HASEOPT)
               "e:",
#else  /* !defined(HASEOPT) */
               "",
#endif /* defined(HASEOPT) */

#if defined(HASEPTOPTS)
               "E",
#else  /* !defined(HASEPTOPTS) */
               "",
#endif /* defined(HASEPTOPTS) */

#if defined(HASKOPT)
               "k:",
#else  /* !defined(HASKOPT) */
               "",
#endif /* defined(HASKOPT) */

#if defined(HASTASKS)
               "K:",
#else  /* !defined(HASTASKS) */
               "",
#endif /* defined(HASTASKS) */

#if defined(HASMOPT) || defined(HASMNTSUP)
               "m:",
#else  /* !defined(HASMOPT) && !defined(HASMNTSUP) */
               "",
#endif /* defined(HASMOPT) || defined(HASMNTSUP) */

#if defined(HASNORPC_H)
               "",
#else  /* !defined(HASNORPC_H) */
               "M",
#endif /* defined(HASNORPC_H) */

#if defined(HASPPID)
               "R",
#else  /* !defined(HASPPID) */
               "",
#endif /* defined(HASPPID) */

#if defined(HASXOPT)
#    if defined(HASXOPT_ROOT)
               (Myuid == 0) ? "X" : "",
#    else  /* !defined(HASXOPT_ROOT) */
               "X",
#    endif /* defined(HASXOPT_ROOT) */
#else      /* !defined(HASXOPT) */
               "",
#endif     /* defined(HASXOPT) */

#if defined(HASZONES)
               "z:",
#else  /* !defined(HASZONES) */
               "",
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
               "Z:"
#else  /* !defined(HASSELINUX) */
               ""
#endif /* defined(HASSELINUX) */

    );
    /*
     * Loop through options.
     */
    while ((c = GetOpt(ctx, argc, argv, options, &gopt_rv)) != EOF) {
        if (gopt_rv) {
            err = 1;
            continue;
        }
        switch (c) {
        case 'a':
            Fand = 1;
            break;

#if defined(HAS_AFS) && defined(HASAOPT)
        case 'A':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                (void)fprintf(stderr, "%s: -A not followed by path\n", Pn);
                err = 1;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
            } else
                AFSApath = GOv;
            break;
#endif /* defined(HAS_AFS) && defined(HASAOPT) */

        case 'b':
            lsof_avoid_blocking(ctx, 1);
            break;
        case 'c':
            if (GOp == '+') {
                if (!GOv || (*GOv == '-') || (*GOv == '+') ||
                    !isdigit((int)*GOv)) {
                    (void)fprintf(stderr,
                                  "%s: +c not followed by width number\n", Pn);
                    err = 1;
                    if (GOv) {
                        GOx1 = GObk[0];
                        GOx2 = GObk[1];
                    }
                } else {
                    CmdLim = TaskCmdLim = atoi(GOv);

#if defined(MAXSYSCMDL)
                    if (CmdLim > MAXSYSCMDL) {
                        (void)fprintf(stderr,
                                      "%s: +c %d > what system provides (%d)\n",
                                      Pn, CmdLim, MAXSYSCMDL);
                        err = 1;
                    }
#endif /* defined(MAXSYSCMDL) */
                }
                break;
            }
            if (GOv && (*GOv == '/')) {
                if (lsof_select_process_regex(ctx, GOv))
                    err = 1;
            } else {
                if (enter_cmd(ctx, "-c", GOv))
                    err = 1;
            }
            break;

#if defined(HASNCACHE)
        case 'C':
            Fncache = (GOp == '-') ? 0 : 1;
            break;

#endif /* defined(HASNCACHE) */
        case 'd':
            if (GOp == '+') {
                if (enter_dir(ctx, GOv, 0))
                    err = 1;
                else {
                    Selflags |= SELNM;
                    xover = 1;
                }
            } else {
                if (enter_fd(ctx, GOv))
                    err = 1;
            }
            break;
        case 'D':
            if (GOp == '+') {
                if (enter_dir(ctx, GOv, 1))
                    err = 1;
                else {
                    Selflags |= SELNM;
                    xover = 1;
                }
            } else {

#if defined(HASDCACHE)
                if (ctrl_dcache(ctx, GOv))
                    err = 1;
#else  /* !defined(HASDCACHE) */
                (void)fprintf(stderr, "%s: unsupported option: -D\n", Pn);
                err = 1;
#endif /* defined(HASDCACHE) */
            }
            break;

#if defined(HASEOPT)
        case 'e':
            if (enter_efsys(ctx, GOv, ((GOp == '+') ? 1 : 0)))
                err = 1;
            break;
#endif /* defined(HASEOPT) */

#if defined(HASEPTOPTS)
        case 'E':
            FeptE = (GOp == '+') ? 2 : 1;
            break;
#endif /* defined(HASEPTOPTS) */

        case 'f':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Ffilesys = (GOp == '+') ? 2 : 1;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            }

#if defined(HASFSTRUCT)
            for (; *GOv; GOv++) {
                switch (*GOv) {

#    if !defined(HASNOFSCOUNT)
                case 'c':
                case 'C':
                    if (GOp == '+') {
                        Fsv |= FSV_CT;
                        FsvByf = 1;
                    } else
                        Fsv &= (unsigned char)~FSV_CT;
                    break;
#    endif /* !defined(HASNOFSCOUNT) */

#    if !defined(HASNOFSADDR)
                case 'f':
                case 'F':
                    if (GOp == '+') {
                        Fsv |= FSV_FA;
                        FsvByf = 1;
                    } else
                        Fsv &= (unsigned char)~FSV_FA;
                    break;
#    endif /* !defined(HASNOFSADDR) */

#    if !defined(HASNOFSFLAGS)
                case 'g':
                case 'G':
                    if (GOp == '+') {
                        Fsv |= FSV_FG;
                        FsvByf = 1;
                    } else
                        Fsv &= (unsigned char)~FSV_FG;
                    FsvFlagX = (*GOv == 'G') ? 1 : 0;
                    break;
#    endif /* !defined(HASNOFSFLAGS) */

#    if !defined(HASNOFSNADDR)
                case 'n':
                case 'N':
                    if (GOp == '+') {
                        Fsv |= FSV_NI;
                        FsvByf = 1;
                    } else
                        Fsv &= (unsigned char)~FSV_NI;
                    break;
#    endif /* !defined(HASNOFSNADDR */

                default:
                    (void)fprintf(stderr,
                                  "%s: unknown file struct option: %c\n", Pn,
                                  *GOv);
                    err++;
                }
            }
#else  /* !defined(HASFSTRUCT) */
            (void)fprintf(stderr, "%s: unknown string for %cf: %s\n", Pn, GOp,
                          GOv);
            err++;
#endif /* defined(HASFSTRUCT) */

            break;
        case 'F':
            if (!GOv || *GOv == '-' || *GOv == '+' || strcmp(GOv, "0") == 0) {
                if (GOv) {
                    if (*GOv == '-' || *GOv == '+') {
                        GOx1 = GObk[0];
                        GOx2 = GObk[1];
                    } else if (*GOv == '0')
                        Terminator = '\0';
                }
                for (i = 0; FieldSel[i].nm; i++) {

#if !defined(HASPPID)
                    if (FieldSel[i].id == LSOF_FID_PPID)
                        continue;
#endif /* !defined(HASPPID) */

#if !defined(HASTASKS)
                    if (FieldSel[i].id == LSOF_FID_TCMD)
                        continue;
#endif /* !defined(HASTASKS) */

#if !defined(HASFSTRUCT)
                    if (FieldSel[i].id == LSOF_FID_CT ||
                        FieldSel[i].id == LSOF_FID_FA ||
                        FieldSel[i].id == LSOF_FID_FG ||
                        FieldSel[i].id == LSOF_FID_NI)
                        continue;
#endif /* !defined(HASFSTRUCT) */

#if defined(HASSELINUX)
                    if ((FieldSel[i].id == LSOF_FID_CNTX) && !CntxStatus)
                        continue;
#else  /* !defined(HASSELINUX) */
                    if (FieldSel[i].id == LSOF_FID_CNTX)
                        continue;
#endif /* !defined(HASSELINUX) */

                    if (FieldSel[i].id == LSOF_FID_RDEV)
                        continue; /* for compatibility */

#if !defined(HASTASKS)
                    if (FieldSel[i].id == LSOF_FID_TID)
                        continue;
#endif /* !defined(HASTASKS) */

#if !defined(HASZONES)
                    if (FieldSel[i].id == LSOF_FID_ZONE)
                        continue;
#endif /* !defined(HASZONES) */

                    FieldSel[i].st = 1;
                    if (FieldSel[i].opt && FieldSel[i].ov)
                        *(FieldSel[i].opt) |= FieldSel[i].ov;
                }

#if defined(HASFSTRUCT)
                Ffield = FsvFlagX = 1;
#else  /* !defined(HASFSTRUCT) */
                Ffield = 1;
#endif /* defined(HASFSTRUCT) */

                break;
            }
            if (strcmp(GOv, "?") == 0) {
                fh = 1;
                break;
            }
            for (; *GOv; GOv++) {
                for (i = 0; FieldSel[i].nm; i++) {

#if !defined(HASPPID)
                    if (FieldSel[i].id == LSOF_FID_PPID)
                        continue;
#endif /* !defined(HASPPID) */

#if !defined(HASTASKS)
                    if (FieldSel[i].id == LSOF_FID_TCMD)
                        continue;
#endif /* !defined(HASTASKS) */

#if !defined(HASFSTRUCT)
                    if (FieldSel[i].id == LSOF_FID_CT ||
                        FieldSel[i].id == LSOF_FID_FA ||
                        FieldSel[i].id == LSOF_FID_FG ||
                        FieldSel[i].id == LSOF_FID_NI)
                        continue;
#endif /* !defined(HASFSTRUCT) */

#if !defined(HASTASKS)
                    if (FieldSel[i].id == LSOF_FID_TID)
                        continue;
#endif /* !defined(HASTASKS) */

                    if (FieldSel[i].id == *GOv) {
                        FieldSel[i].st = 1;
                        if (FieldSel[i].opt && FieldSel[i].ov)
                            *(FieldSel[i].opt) |= FieldSel[i].ov;

#if defined(HASFSTRUCT)
                        if (i == LSOF_FIX_FG)
                            FsvFlagX = 1;
#endif /* defined(HASFSTRUCT) */

                        if (i == LSOF_FIX_TERM)
                            Terminator = '\0';

                        if (i == LSOF_FIX_OFFSET)
                            Foffset = 1;

                        break;
                    }
                }
                if (!FieldSel[i].nm) {
                    (void)fprintf(stderr, "%s: unknown field: %c\n", Pn, *GOv);
                    err++;
                }
            }
            Ffield = 1;
            break;
        case 'g':
            if (GOv) {
                if (*GOv == '-' || *GOv == '+') {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                } else if (enter_id(ctx, PGID, GOv))
                    err = 1;
            }
            Fpgid = 1;
            break;
        case 'H':
            Fhuman = 1;
            break;
        case 'h':
        case '?':
            Fhelp = 1;
            break;
        case 'i':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Fnet = 1;
                FnetTy = 0;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            }
            if (enter_network_address(ctx, GOv))
                err = 1;
            break;

#if defined(HASKOPT)
        case 'k':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                (void)fprintf(stderr, "%s: -k not followed by path\n", Pn);
                err = 1;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
            } else
                Nmlst = GOv;
            break;
#endif /* defined(HASKOPT) */

#if defined(HASTASKS)
        case 'K':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Ftask = 1;
                IgnTasks = 0;
                Selflags |= SELTASK;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
            } else {
                if (!strcasecmp(GOv, "i")) {
                    Ftask = 0;
                    IgnTasks = 1;
                    Selflags &= ~SELTASK;
                } else {
                    (void)fprintf(stderr,
                                  "%s: -K not followed by i (but by %s)\n", Pn,
                                  GOv);
                    err = 1;
                }
            }
            break;
#endif /* defined(HASTASKS) */

        case 'l':
            Futol = 0;
            break;
        case 'L':
            Fnlink = (GOp == '+') ? 1 : 0;
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Nlink = 0l;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            }
            for (cp = GOv, l = 0l, n = 0; *cp; cp++) {
                if (!isdigit((unsigned char)*cp))
                    break;
                l = (l * 10l) + ((long)*cp - (long)'0');
                n++;
            }
            if (n) {
                if (GOp != '+') {
                    (void)fprintf(stderr, "%s: no number may follow -L\n", Pn);
                    err = 1;
                } else {
                    Nlink = l;
                    Selflags |= SELNLINK;
                }
            } else
                Nlink = 0l;
            if (*cp) {
                GOx1 = GObk[0];
                GOx2 = GObk[1] + n;
            }
            break;

#if defined(HASMOPT) || defined(HASMNTSUP)
        case 'm':
            if (GOp == '-') {

#    if defined(HASMOPT)
                if (!GOv || *GOv == '-' || *GOv == '+') {
                    (void)fprintf(stderr, "%s: -m not followed by path\n", Pn);
                    err = 1;
                    if (GOv) {
                        GOx1 = GObk[0];
                        GOx2 = GObk[1];
                    }
                } else
                    Memory = GOv;
#    else  /* !defined(HASMOPT) */
                (void)fprintf(stderr, "%s: -m not supported\n", Pn);
                err = 1;
#    endif /* defined(HASMOPT) */

            } else if (GOp == '+') {

#    if defined(HASMNTSUP)
                if (!GOv || *GOv == '-' || *GOv == '+') {
                    MntSup = 1;
                    if (GOv) {
                        GOx1 = GObk[0];
                        GOx2 = GObk[1];
                    }
                } else {
                    MntSup = 2;
                    MntSupP = GOv;
                }
#    else  /* !defined(HASMNTSUP) */
                (void)fprintf(stderr, "%s: +m not supported\n", Pn);
                err = 1;
#    endif /* defined(HASMNTSUP) */

            } else {
                (void)fprintf(stderr, "%s: %cm not supported\n", Pn, GOp);
                err = 1;
            }
            break;
#endif /* defined(HASMOPT) || defined(HASMNTSUP) */

#if !defined(HASNORPC_H)
        case 'M':
            FportMap = (GOp == '+') ? 1 : 0;
            break;
#endif /* !defined(HASNORPC_H) */

        case 'n':
            Fhost = (GOp == '-') ? 0 : 1;
            break;
        case 'N':
            Fnfs = 1;
            break;
        case 'o':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Foffset = 1;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            }
            for (cp = GOv, i = n = 0; *cp; cp++) {
                if (!isdigit((unsigned char)*cp))
                    break;
                i = (i * 10) + ((int)*cp - '0');
                n++;
            }
            if (n)
                OffDecDig = i;
            else
                Foffset = 1;
            if (*cp) {
                GOx1 = GObk[0];
                GOx2 = GObk[1] + n;
            }
            break;
        case 'O':
            lsof_avoid_forking(ctx, (GOp == '-') ? 1 : 0);
            break;
        case 'p':
            if (enter_id(ctx, PID, GOv))
                err = 1;
            break;
        case 'Q':
            FsearchErr = 0;
            break;
        case 'P':
            Fport = (GOp == '-') ? 0 : 1;
            break;
        case 'r':
            if (GOp == '+') {
                ev = LSOF_EXIT_ERROR;
                rc = 1;
            }
            if (!GOv || *GOv == '-' || *GOv == '+') {
                RptTm = RPTTM;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            }
            for (cp = GOv, i = n = 0; *cp; cp++) {
                if (!isdigit((unsigned char)*cp))
                    break;
                i = (i * 10) + ((int)*cp - '0');
                n++;
            }
            if (n)
                RptTm = i;
            else
                RptTm = RPTTM;
            if (!*cp)
                break;
            while (*cp && (*cp == ' '))
                cp++;

            if (*cp == 'c') {
                cp++;
                for (i = 0; *cp && isdigit((unsigned char)*cp); cp++)
                    i = (i * 10) + ((int)*cp - '0');
                RptMaxCount = i;
            }

            if (*cp != LSOF_FID_MARK) {
                GOx1 = GObk[0];
                GOx2 = GObk[1] + n;
                break;
            }

#if defined(HAS_STRFTIME)

            /*
             * Collect the strftime(3) format and test it.
             */
            cp++;
            if ((fmtl = strlen(cp) + 1) < 1) {
                (void)fprintf(stderr, "%s: <fmt> too short: \"%s\"\n", Pn, cp);
                err = 1;
            } else {
                fmt = cp;
                fmtl = (fmtl * 8) + 1;
                if (!(fmtr = (char *)malloc((MALLOC_S)fmtl))) {
                    (void)fprintf(
                        stderr, "%s: no space (%d) for <fmt> result: \"%s\"\n",
                        Pn, (int)fmtl, cp);
                    Error(ctx);
                }
                if (util_strftime(fmtr, fmtl - 1, fmt) < 1) {
                    (void)fprintf(stderr, "%s: illegal <fmt>: \"%s\"\n", Pn,
                                  fmt);
                    err = 1;
                }
            }

#else  /* !defined(HAS_STRFTIME) */
            (void)fprintf(stderr, "%s: m<fmt> not supported: \"%s\"\n", Pn, cp);
            err = 1;
#endif /* defined(HAS_STRFTIME) */

            break;

#if defined(HASPPID)
        case 'R':
            Fppid = 1;
            break;
#endif /* defined(HASPPID) */

        case 's':

#if defined(HASTCPUDPSTATE)
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Fsize = 1;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
            } else {
                if (enter_state_spec(ctx, GOv))
                    err = 1;
            }
#else  /* !defined(HASTCPUDPSTATE) */
            Fsize = 1;
#endif /* defined(HASTCPUDPSTATE) */

            break;
        case 'S':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                TmLimit = TMLIMIT;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            }
            for (cp = GOv, i = n = 0; *cp; cp++) {
                if (!isdigit((unsigned char)*cp))
                    break;
                i = (i * 10) + ((int)*cp - '0');
                n++;
            }
            if (n)
                TmLimit = i;
            else
                TmLimit = TMLIMIT;
            if (*cp) {
                GOx1 = GObk[0];
                GOx2 = GObk[1] + n;
            }
            if (TmLimit < TMLIMMIN) {
                (void)fprintf(stderr,
                              "%s: WARNING: -S time (%d) changed to %d\n", Pn,
                              TmLimit, TMLIMMIN);
                TmLimit = TMLIMMIN;
            }
            break;
        case 't':
            Fterse = Fwarn = 1;
            break;
        case 'T':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Ftcptpi = (GOp == '-') ? 0 : TCPTPI_STATE;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            }
            for (Ftcptpi = 0; *GOv; GOv++) {
                switch (*GOv) {

#if defined(HASSOOPT) || defined(HASSOSTATE) || defined(HASTCPOPT)
                case 'f':
                    Ftcptpi |= TCPTPI_FLAGS;
                    break;
#endif /* defined(HASSOOPT) || defined(HASSOSTATE) || defined(HASTCPOPT) */

#if defined(HASTCPTPIQ)
                case 'q':
                    Ftcptpi |= TCPTPI_QUEUES;
                    break;
#endif /* defined(HASTCPTPIQ) */

                case 's':
                    Ftcptpi |= TCPTPI_STATE;
                    break;

#if defined(HASTCPTPIW)
                case 'w':
                    Ftcptpi |= TCPTPI_WINDOWS;
                    break;
#endif /* defined(HASTCPTPIW) */

                default:
                    (void)fprintf(
                        stderr, "%s: unsupported TCP/TPI info selection: %c\n",
                        Pn, *GOv);
                    err = 1;
                }
            }
            break;
        case 'u':
            if (enter_uid(ctx, GOv))
                err = 1;
            break;
        case 'U':
            Funix = 1;
            break;
        case 'v':
            version = 1;
            break;
        case 'V':
            Fverbose = 1;
            break;
        case 'w':
            Fwarn = (GOp == '+') ? 0 : 1;
            break;
        case 'x':
            if (!GOv || *GOv == '-' || *GOv == '+') {
                Fxover = XO_ALL;
                if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
                break;
            } else {
                for (; *GOv; GOv++) {
                    switch (*GOv) {
                    case 'f':
                        Fxover |= XO_FILESYS;
                        break;
                    case 'l':
                        Fxover |= XO_SYMLINK;
                        break;
                    default:
                        (void)fprintf(stderr,
                                      "%s: unknown cross-over option: %c\n", Pn,
                                      *GOv);
                        err++;
                    }
                }
            }
            break;

#if defined(HASXOPT)
        case 'X':
            Fxopt = Fxopt ? 0 : 1;
            break;
#endif /* defined(HASXOPT) */

#if defined(HASZONES)
        case 'z':
            Fzone = 1;
            if (GOv && (*GOv != '-') && (*GOv != '+')) {

                /*
                 * Add to the zone name argument hash.
                 */
                if (enter_zone_arg(ctx, GOv))
                    err = 1;
            } else if (GOv) {
                GOx1 = GObk[0];
                GOx2 = GObk[1];
            }
            break;
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
        case 'Z':
            if (!CntxStatus) {
                (void)fprintf(stderr, "%s: -Z limited to SELinux\n", Pn);
                err = 1;
            } else {
                Fcntx = 1;
                if (GOv && (*GOv != '-') && (*GOv != '+')) {

                    /*
                     * Add to the context name argument hash.
                     */
                    if (enter_cntx_arg(ctx, GOv))
                        err = 1;
                } else if (GOv) {
                    GOx1 = GObk[0];
                    GOx2 = GObk[1];
                }
            }
            break;
#endif /* defined(HASSELINUX) */

        default:
            (void)fprintf(stderr, "%s: unknown option (%c)\n", Pn, c);
            err = 1;
        }
    }
    /*
     * If IgnTasks is set, remove SELTASK from SelAll and SelProc.
     */
    SelAll = IgnTasks ? (SELALL & ~SELTASK) : SELALL;
    SelProc = IgnTasks ? (SELPROC & ~SELTASK) : SELPROC;
    /*
     * Check for argument consistency.
     */
    if (Cmdnx && Cmdni) {

        /*
         * Check for command inclusion/exclusion conflicts.
         */
        for (str = Cmdl; str; str = str->next) {
            if (str->x) {
                for (strt = Cmdl; strt; strt = strt->next) {
                    if (!strt->x) {
                        if (!strcmp(str->str, strt->str)) {
                            (void)fprintf(stderr,
                                          "%s: -c^%s and -c%s conflict.\n", Pn,
                                          str->str, strt->str);
                            err++;
                        }
                    }
                }
            }
        }
    }

#if defined(HASTCPUDPSTATE)
    if (TcpStXn && TcpStIn) {

        /*
         * Check for excluded and included TCP states.
         */
        for (i = 0; i < TcpNstates; i++) {
            if (TcpStX[i] && TcpStI[i]) {
                (void)fprintf(stderr,
                              "%s: can't include and exclude TCP state: %s\n",
                              Pn, TcpSt[i]);
                err = 1;
            }
        }
    }
    if (UdpStXn && UdpStIn) {

        /*
         * Check for excluded and included UDP states.
         */
        for (i = 0; i < UdpNstates; i++) {
            if (UdpStX[i] && UdpStI[i]) {
                (void)fprintf(stderr,
                              "%s: can't include and exclude UDP state: %s\n",
                              Pn, UdpSt[i]);
                err = 1;
            }
        }
    }
#endif /* defined(HASTCPUDPSTATE) */

    if (Fsize && Foffset) {
        (void)fprintf(stderr, "%s: -o and -s are mutually exclusive\n", Pn);
        err++;
    }
    if (Ffield) {
        if (Fterse) {
            (void)fprintf(stderr, "%s: -F and -t are mutually exclusive\n", Pn);
            err++;
        }
        FieldSel[LSOF_FIX_PID].st = 1;

#if defined(HAS_STRFTIME)
        if (fmtr) {

            /*
             * The field output marker format can't contain "%n" new line
             * requests.
             */
            for (cp = strchr(fmt, '%'); cp; cp = strchr(cp, '%')) {
                if (*++cp == 'n') {
                    (void)fprintf(
                        stderr, "%s: %%n illegal in -r m<fmt> when -F has", Pn);
                    (void)fprintf(stderr, " been specified: \"%s\"\n", fmt);
                    err++;
                    break;
                } else if (*cp == '%')
                    cp++;
            }
        }
#endif /* defined(HAS_STRFTIME) */
    }
    if (Fxover && !xover) {
        (void)fprintf(stderr, "%s: -x must accompany +d or +D\n", Pn);
        err++;
    }

#if defined(HASEOPT)
    if (Efsysl) {

        /*
         * If there are file systems specified by -e options, check them.
         */
        efsys_list_t *ep;        /* Efsysl pointer */
        struct mounts *mp, *mpw; /* local mount table pointers */

        if ((mp = readmnt(ctx))) {
            for (ep = Efsysl; ep; ep = ep->next) {
                for (mpw = mp; mpw; mpw = mpw->next) {
                    if (!strcmp(mpw->dir, ep->path)) {
                        ep->mp = mpw;
                        break;
                    }
                }
                if (!ep->mp) {
                    (void)fprintf(
                        stderr, "%s: \"-e %s\" is not a mounted file system.\n",
                        Pn, ep->path);
                    err++;
                }
            }
        }
    }
#endif /* defined(HASEOPT) */

    if (DChelp || err || Fhelp || fh || version)
        usage(ctx, err ? 1 : 0, fh, version);
    /*
     * Reduce the size of Suid[], if necessary.
     */
    if (Suid && Nuid && Nuid < Mxuid) {
        if (!(Suid = (struct seluid *)realloc(
                  (MALLOC_P *)Suid,
                  (MALLOC_S)(sizeof(struct seluid) * Nuid)))) {
            (void)fprintf(stderr, "%s: can't realloc UID table\n", Pn);
            Error(ctx);
        }
        Mxuid = Nuid;
    }
    /*
     * Compute the selection flags.
     */
    if ((Cmdl && Cmdni) || CmdRx)
        Selflags |= SELCMD;

#if defined(HASSELINUX)
    if (CntxArg)
        Selflags |= SELCNTX;
#endif /* defined(HASSELINUX) */

    if (Fdl)
        Selflags |= SELFD;
    if (Fnet)
        Selflags |= SELNET;
    if (Fnfs)
        Selflags |= SELNFS;
    if (Funix)
        Selflags |= SELUNX;
    if (Npgid && Npgidi)
        Selflags |= SELPGID;
    if (Npid && Npidi)
        Selflags |= SELPID;
    if (Nuid && Nuidincl)
        Selflags |= SELUID;
    if (Nwad)
        Selflags |= SELNA;

#if defined(HASZONES)
    if (ZoneArg)
        Selflags |= SELZONE;
#endif /* defined(HASZONES) */

    if (GOx1 < argc)
        Selflags |= SELNM;
    if (Selflags == 0) {
        if (Fand) {
            (void)fprintf(stderr, "%s: no select options to AND via -a\n", Pn);
            usage(ctx, 1, 0, 0);
        }
        Selflags = SelAll;
    } else {
        if (GOx1 >= argc && (Selflags & (SELNA | SELNET)) != 0 &&
            (Selflags & ~(SELNA | SELNET)) == 0)
            Selinet = 1;
        AllProc = 0;
    }
    /*
     * Get the device for DEVDEV_PATH.
     */
    if (stat(DEVDEV_PATH, &sb)) {
        se1 = errno;
        if ((ad = strcmp(DEVDEV_PATH, "/dev"))) {
            if ((ss = stat("/dev", &sb)))
                se2 = errno;
            else
                se2 = 0;
        } else {
            se2 = 0;
            ss = 1;
        }
        if (ss) {
            (void)fprintf(stderr, "%s: can't stat(%s): %s\n", Pn, DEVDEV_PATH,
                          strerror(se1));
            if (ad) {
                (void)fprintf(stderr, "%s: can't stat(/dev): %s\n", Pn,
                              strerror(se2));
            }
            Error(ctx);
        }
    }
    DevDev = sb.st_dev;
    /*
     * Process the file arguments.
     */
    if (GOx1 < argc) {
        if (ck_file_arg(ctx, GOx1, argc, argv, Ffilesys, 0, (struct stat *)NULL,
                        FsearchErr == 0))
            Error(ctx);
    }
    /*
     * Do dialect-specific initialization.
     */
    initialize(ctx);
#if defined(LINUX_LSOF_H)
    if (Fsv && (OffType != OFFSET_FDINFO)) {
        if (!Fwarn && FsvByf)
            (void)fprintf(
                stderr,
                "%s: WARNING: can't report file flags; disregarding +f.\n", Pn);
        Fsv = 0;
    }
#endif
    if (Sfile)
        (void)hashSfile(ctx);

#if defined(WILLDROPGID)
    /*
     * If this process isn't setuid(root), but it is setgid(not_real_gid),
     * relinquish the setgid power.  (If it hasn't already been done.)
     */
    (void)dropgid(ctx);
#endif /* defined(WILLDROPGID) */

#if defined(HASDCACHE)
    /*
     * If there is a device cache, prepare the device table.
     */
    if (DCstate)
        readdev(ctx, 0);
#endif /* defined(HASDCACHE) */

    /*
     * Define the size and offset print formats.
     */
    (void)snpf(options, sizeof(options), "%%%su", INODEPSPEC);
    InodeFmt_d = sv_fmt_str(ctx, options);
    (void)snpf(options, sizeof(options), "%%#%sx", INODEPSPEC);
    InodeFmt_x = sv_fmt_str(ctx, options);
    (void)snpf(options, sizeof(options), "0t%%%su", SZOFFPSPEC);
    SzOffFmt_0t = sv_fmt_str(ctx, options);
    (void)snpf(options, sizeof(options), "%%%su", SZOFFPSPEC);
    SzOffFmt_d = sv_fmt_str(ctx, options);
    (void)snpf(options, sizeof(options), "%%*%su", SZOFFPSPEC);
    SzOffFmt_dv = sv_fmt_str(ctx, options);
    (void)snpf(options, sizeof(options), "%%#%sx", SZOFFPSPEC);
    SzOffFmt_x = sv_fmt_str(ctx, options);

#if defined(HASMNTSUP)
    /*
     * Report mount supplement information, as requested.
     */
    if (MntSup == 1) {
        (void)readmnt(ctx);
        Exit(ctx, LSOF_EXIT_SUCCESS);
    }
#endif /* defined(HASMNTSUP) */

    /*
     * Gather and report process information every RptTm seconds.
     */
    if (RptTm)
        CkPasswd = 1;
    do {

        /*
         * Gather information about processes.
         */
        gather_proc_info(ctx);
        /*
         * If the local process table has more than one entry, sort it by PID.
         */
        if (Nlproc > 1) {
            if (Nlproc > sp) {
                len = (MALLOC_S)(Nlproc * sizeof(struct lproc *));
                sp = Nlproc;
                if (!slp)
                    slp = (struct lproc **)malloc(len);
                else
                    slp = (struct lproc **)realloc((MALLOC_P *)slp, len);
                if (!slp) {
                    (void)fprintf(stderr, "%s: no space for %d sort pointers\n",
                                  Pn, Nlproc);
                    Error(ctx);
                }
            }
            for (i = 0; i < Nlproc; i++) {
                slp[i] = &Lproc[i];
            }
            (void)qsort((QSORT_P *)slp, (size_t)Nlproc,
                        (size_t)sizeof(struct lproc *), comppid);
        }
        if ((n = Nlproc)) {

#if defined(HASNCACHE)
            /*
             * If using the kernel name cache, force its reloading.
             */
            NcacheReload = 1;
#endif /* defined(HASNCACHE) */

#if defined(HASEPTOPTS)
            /*
             * If endpoint info has been requested, make sure it is coded for
             * printing.
             *
             * Lf contents must be preserved, since they may point to a
             * malloc()'d area, and since Lf is used throughout the printing
             * of the selected processes.
             */
            if (FeptE) {
                lf = Lf;
                /*
                 * Scan all selected processes.
                 */
                for (i = 0; i < Nlproc; i++) {
                    Lp = (Nlproc > 1) ? slp[i] : &Lproc[i];

                    /*
                     * For processes that have been selected for printing
                     * and have files that are the end point(s) of pipe(s),
                     * process the file endpoints.
                     */
                    if (Lp->pss && (Lp->ept & EPT_PIPE))
                        (void)process_pinfo(ctx, 0);
                    /*
                     * Process POSIX MQ endpoints.
                     */
                    if (Lp->ept & EPT_PSXMQ)
                        (void)process_psxmqinfo(ctx, 0);

#    if defined(HASUXSOCKEPT)
                    /*
                     * For processes that have been selected for printing
                     * and have files that are the end point(s) of UNIX
                     * socket(s), process the file endpoints.
                     */
                    if (Lp->pss && (Lp->ept & EPT_UXS))
                        (void)process_uxsinfo(ctx, 0);
#    endif /* defined(HASUXSOCKEPT) */

#    if defined(HASPTYEPT)
                    /*
                     * For processes that have been selected for printing
                     * and have files that are the end point(s) of pseudo-
                     * terminal files(s), process the file endpoints.
                     */
                    if (Lp->pss && (Lp->ept & EPT_PTY))
                        (void)process_ptyinfo(ctx, 0);
#    endif /* defined(HASPTYEPT) */

                    /*
                     * Process INET socket endpoints.
                     */
                    if (Lp->ept & EPT_NETS)
                        (void)process_netsinfo(ctx, 0);

#    if defined(HASIPv6)
                    /*
                     * Process INET6 socket endpoints.
                     */
                    if (Lp->ept & EPT_NETS6)
                        (void)process_nets6info(ctx, 0);
#    endif /* defined(HASIPv6) */
                    /*
                     * Process eventfd endpoints.
                     */
                    if (Lp->ept & EPT_EVTFD)
                        (void)process_evtfdinfo(ctx, 0);
                }
                /*
                 * In a second pass, look for unselected endpoint files,
                 * possibly selecting them for printing.
                 */
                for (i = 0; i < Nlproc; i++) {
                    Lp = (Nlproc > 1) ? slp[i] : &Lproc[i];

                    /*
                     * Process pipe endpoints.
                     */
                    if (Lp->ept & EPT_PIPE_END)
                        (void)process_pinfo(ctx, 1);
                    /*
                     * Process POSIX MQ endpoints.
                     */
                    if (Lp->ept & EPT_PSXMQ_END)
                        (void)process_psxmqinfo(ctx, 1);

#    if defined(HASUXSOCKEPT)
                    /*
                     * Process UNIX socket endpoints.
                     */
                    if (Lp->ept & EPT_UXS_END)
                        (void)process_uxsinfo(ctx, 1);
#    endif /* defined(HASUXSOCKEPT) */

#    if defined(HASPTYEPT)
                    /*
                     * Process pseudo-terminal endpoints.
                     */
                    if (Lp->ept & EPT_PTY_END)
                        (void)process_ptyinfo(ctx, 1);
#    endif /* defined(HASPTYEPT) */

                    /*
                     * Process INET socket endpoints.
                     */
                    if (Lp->ept & EPT_NETS_END)
                        (void)process_netsinfo(ctx, 1);

#    if defined(HASIPv6)
                    /*
                     * Process INET6 socket endpoints.
                     */
                    if (Lp->ept & EPT_NETS6_END)
                        (void)process_nets6info(ctx, 1);
#    endif /* defined(HASIPv6) */

                    /*
                     * Process envetfd endpoints.
                     */
                    if (Lp->ept & EPT_EVTFD_END)
                        (void)process_evtfdinfo(ctx, 1);
                }
                Lf = lf;
            }
#endif /* defined(HASEPTOPTS) */

            /*
             * Print the selected processes and count them.
             *
             * Lf contents must be preserved, since they may point to a
             * malloc()'d area, and since Lf is used throughout the print
             * process.
             */
            for (lf = Lf, print_init(ctx); PrPass < 2; PrPass++) {
                for (i = n = 0; i < Nlproc; i++) {
                    Lp = (Nlproc > 1) ? slp[i] : &Lproc[i];
                    if (Lp->pss) {
                        if (print_proc(ctx))
                            n++;
                    }
                    if (RptTm && PrPass)
                        (void)free_lproc(Lp);
                }
            }
            Lf = lf;
        }
        /*
         * If a repeat time is set, sleep for the specified time.
         *
         * If conditional repeat mode is in effect, see if it's time to exit.
         */
        if (RptTm) {

#if defined(HASEPTOPTS)
            (void)clear_pinfo(ctx);

            (void)clear_psxmqinfo(ctx);

#    if defined(HASUXSOCKEPT)
            (void)clear_uxsinfo(ctx);
#    endif /* defined(HASUXSOCKEPT) */

#    if defined(HASPTYEPT)
            (void)clear_ptyinfo(ctx);
#    endif /* defined(HASPTYEPT) */

            (void)clear_netsinfo(ctx);

#    if defined(HASIPv6)
            (void)clear_nets6info(ctx);
#    endif /* defined(HASIPv6) */

            (void)clear_evtfdinfo(ctx);
#endif /* defined(HASEPTOPTS) */

            if (rc) {
                if (!n)
                    break;
                else
                    ev = LSOF_EXIT_SUCCESS;
            }

#if defined(HAS_STRFTIME)
            if (fmt && fmtr) {

                /*
                 * Format the marker line.
                 */
                (void)util_strftime(fmtr, fmtl - 1, fmt);
                fmtr[fmtl - 1] = '\0';
            }
#endif /* defined(HAS_STRFTIME) */

            if (Ffield) {
                putchar(LSOF_FID_MARK);

#if defined(HAS_STRFTIME)
                if (fmtr)
                    (void)printf("%s", fmtr);
#endif /* defined(HAS_STRFTIME) */

                putchar(Terminator);
                if (Terminator != '\n')
                    putchar('\n');
            } else {

#if defined(HAS_STRFTIME)
                if (fmtr)
                    cp = fmtr;
                else
#endif /* defined(HAS_STRFTIME) */

                    cp = "=======";
                puts(cp);
            }
            (void)fflush(stdout);
            (void)childx(ctx);
            (void)sleep(RptTm);
            Hdr = Nlproc = 0;
            CkPasswd = 1;
        }
        if (RptMaxCount && (++pr_count == RptMaxCount))
            RptTm = 0;
    } while (RptTm);
    /*
     * See if all requested information was displayed.  Return zero if it
     * was; one, if not.  If -V was specified, report what was not displayed.
     */
    (void)childx(ctx);
    rv = LSOF_EXIT_SUCCESS;
    for (str = Cmdl; str; str = str->next) {

        /*
         * Check command specifications.
         */
        if (str->f)
            continue;
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose) {
            (void)printf("%s: command not located: ", Pn);
            safestrprt(str->str, stdout, 1);
        }
    }
    for (i = 0; i < NCmdRxU; i++) {

        /*
         * Check command regular expressions.
         */
        if (CmdRx[i].mc)
            continue;
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose) {
            (void)printf("%s: no command found for regex: ", Pn);
            safestrprt(CmdRx[i].exp, stdout, 1);
        }
    }
    for (sfp = Sfile; sfp; sfp = sfp->next) {

        /*
         * Check file specifications.
         */
        if (sfp->f)
            continue;
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose) {
            (void)printf("%s: no file%s use located: ", Pn,
                         sfp->type ? "" : " system");
            safestrprt(sfp->aname, stdout, 1);
        }
    }

#if defined(HASPROCFS)
    /*
     * Report on proc file system search results.
     */
    if (Procsrch && !Procfind) {
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose) {
            (void)printf("%s: no file system use located: ", Pn);
            safestrprt(Mtprocfs ? Mtprocfs->dir : HASPROCFS, stdout, 1);
        }
    }
    {
        struct procfsid *pfi;

        for (pfi = Procfsid; pfi; pfi = pfi->next) {
            if (!pfi->f) {
                rv = LSOF_SEARCH_FAILURE;
                if (Fverbose) {
                    (void)printf("%s: no file use located: ", Pn);
                    safestrprt(pfi->nm, stdout, 1);
                }
            }
        }
    }
#endif /* defined(HASPROCFS) */

    if ((np = Nwad)) {

        /*
         * Check Internet address specifications.
         *
         * If any Internet address derived from the same argument was found,
         * consider all derivations found.  If no derivation from the same
         * argument was found, report only the first failure.
         *
         */
        for (; np; np = np->next) {
            if (!(cp = np->arg))
                continue;
            for (npn = np->next; npn; npn = npn->next) {
                if (!npn->arg)
                    continue;
                if (!strcmp(cp, npn->arg)) {

                    /*
                     * If either of the duplicate specifications was found,
                     * mark them both found.  If neither was found, mark all
                     * but the first one found.
                     */
                    if (np->f)
                        npn->f = np->f;
                    else if (npn->f)
                        np->f = npn->f;
                    else
                        npn->f = 1;
                }
            }
        }
        for (np = Nwad; np; np = np->next) {
            if (!np->f && (cp = np->arg)) {
                rv = LSOF_SEARCH_FAILURE;
                if (Fverbose) {
                    (void)printf("%s: Internet address not located: ", Pn);
                    safestrprt(cp ? cp : "(unknown)", stdout, 1);
                }
            }
        }
    }
    if (Fnet && Fnet < 2) {

        /*
         * Report no Internet files located.
         */
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose)
            (void)printf("%s: no Internet files located\n", Pn);
    }

#if defined(HASTCPUDPSTATE)
    if (TcpStIn) {

        /*
         * Check for included TCP states not located.
         */
        for (i = 0; i < TcpNstates; i++) {
            if (TcpStI[i] == 1) {
                rv = LSOF_SEARCH_FAILURE;
                if (Fverbose)
                    (void)printf("%s: TCP state not located: %s\n", Pn,
                                 TcpSt[i]);
            }
        }
    }
    if (UdpStIn) {

        /*
         * Check for included UDP states not located.
         */
        for (i = 0; i < UdpNstates; i++) {
            if (UdpStI[i] == 1) {
                rv = LSOF_SEARCH_FAILURE;
                if (Fverbose)
                    (void)printf("%s: UDP state not located: %s\n", Pn,
                                 UdpSt[i]);
            }
        }
    }
#endif /* defined(HASTCPUDPSTATE) */

    if (Fnfs && Fnfs < 2) {

        /*
         * Report no NFS files located.
         */
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose)
            (void)printf("%s: no NFS files located\n", Pn);
    }
    for (i = 0; i < Npid; i++) {

        /*
         * Check inclusionary process ID specifications.
         */
        if (Spid[i].f || Spid[i].x)
            continue;
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose)
            (void)printf("%s: process ID not located: %d\n", Pn, Spid[i].i);
    }

#if defined(HASTASKS)
    if (Ftask && Ftask < 2) {

        /*
         * Report no tasks located.
         */
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose)
            (void)printf("%s: no tasks located\n", Pn);
    }
#endif /* defined(HASTASKS) */

#if defined(HASZONES)
    if (ZoneArg) {

        /*
         * Check zone argument results.
         */
        for (i = 0; i < HASHZONE; i++) {
            for (zp = ZoneArg[i]; zp; zp = zp->next) {
                if (!zp->f) {
                    rv = LSOF_SEARCH_FAILURE;
                    if (Fverbose) {
                        (void)printf("%s: zone not located: ", Pn);
                        safestrprt(zp->zn, stdout, 1);
                    }
                }
            }
        }
    }
#endif /* defined(HASZONES) */

#if defined(HASSELINUX)
    if (CntxArg) {

        /*
         * Check context argument results.
         */
        for (cntxp = CntxArg; cntxp; cntxp = cntxp->next) {
            if (!cntxp->f) {
                rv = LSOF_SEARCH_FAILURE;
                if (Fverbose) {
                    (void)printf("%s: context not located: ", Pn);
                    safestrprt(cntxp->cntx, stdout, 1);
                }
            }
        }
    }
#endif /* defined(HASSELINUX) */

    for (i = 0; i < Npgid; i++) {

        /*
         * Check inclusionary process group ID specifications.
         */
        if (Spgid[i].f || Spgid[i].x)
            continue;
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose)
            (void)printf("%s: process group ID not located: %d\n", Pn,
                         Spgid[i].i);
    }
    for (i = 0; i < Nuid; i++) {

        /*
         * Check inclusionary user ID specifications.
         */
        if (Suid[i].excl || Suid[i].f)
            continue;
        rv = LSOF_SEARCH_FAILURE;
        if (Fverbose) {
            if (Suid[i].lnm) {
                (void)printf("%s: login name (UID %lu) not located: ", Pn,
                             (unsigned long)Suid[i].uid);
                safestrprt(Suid[i].lnm, stdout, 1);
            } else
                (void)printf("%s: user ID not located: %lu\n", Pn,
                             (unsigned long)Suid[i].uid);
        }
    }
    if (!rv && rc)
        rv = ev;
    if (!rv && ErrStat)
        rv = LSOF_EXIT_ERROR;
    Exit(ctx, rv);
    return (rv); /* to make code analyzers happy */
}

/*
 * GetOpt() -- Local get option
 *
 * Liberally adapted from the public domain AT&T getopt() source,
 * distributed at the 1985 UNIFORM conference in Dallas
 *
 * The modifications allow `?' to be an option character and allow
 * the caller to decide that an option that may be followed by a
 * value doesn't have one -- e.g., has a default instead.
 */

static int GetOpt(struct lsof_context *ctx, /* context */
                  int ct,                   /* option count */
                  char *opt[],              /* options */
                  char *rules,              /* option rules */
                  int *err)                 /* error return */
{
    register int c;
    register char *cp = (char *)NULL;

    if (GOx2 == 0) {

        /*
         * Move to a new entry of the option array.
         *
         * EOF if:
         *
         *	Option list has been exhausted;
         *	Next option doesn't start with `-' or `+';
         *	Next option has nothing but `-' or `+';
         *	Next option is ``--'' or ``++''.
         */
        if (GOx1 >= ct || (opt[GOx1][0] != '-' && opt[GOx1][0] != '+') ||
            !opt[GOx1][1])
            return (EOF);
        if (strcmp(opt[GOx1], "--") == 0 || strcmp(opt[GOx1], "++") == 0) {
            GOx1++;
            return (EOF);
        }
        GOp = opt[GOx1][0];
        GOx2 = 1;
    }
    /*
     * Flag `:' option character as an error.
     *
     * Check for a rule on this option character.
     */
    *err = 0;
    if ((c = opt[GOx1][GOx2]) == ':') {
        (void)fprintf(stderr, "%s: colon is an illegal option character.\n",
                      Pn);
        *err = 1;
    } else if (!(cp = strchr(rules, c))) {
        (void)fprintf(stderr, "%s: illegal option character: %c\n", Pn, c);
        *err = 2;
    }
    if (*err) {

        /*
         * An error was detected.
         *
         * Advance to the next option character.
         *
         * Return the character causing the error.
         */
        if (opt[GOx1][++GOx2] == '\0') {
            GOx1++;
            GOx2 = 0;
        }
        return (c);
    }
    if (*(cp + 1) == ':') {

        /*
         * The option may have a following value.  The caller decides
         * if it does.
         *
         * Save the position of the possible value in case the caller
         * decides it does not belong to the option and wants it
         * reconsidered as an option character.  The caller does that
         * with:
         *		GOx1 = GObk[0]; GOx2 = GObk[1];
         *
         * Don't indicate that an option of ``--'' is a possible value.
         *
         * Finally, on the assumption that the caller will decide that
         * the possible value belongs to the option, position to the
         * option following the possible value, so that the next call
         * to GetOpt() will find it.
         */
        if (opt[GOx1][GOx2 + 1] != '\0') {
            GObk[0] = GOx1;
            GObk[1] = ++GOx2;
            GOv = &opt[GOx1++][GOx2];
        } else if (++GOx1 >= ct)
            GOv = (char *)NULL;
        else {
            GObk[0] = GOx1;
            GObk[1] = 0;
            GOv = opt[GOx1];
            if (strcmp(GOv, "--") == 0)
                GOv = (char *)NULL;
            else
                GOx1++;
        }
        GOx2 = 0;
    } else {

        /*
         * The option character stands alone with no following value.
         *
         * Advance to the next option character.
         */
        if (opt[GOx1][++GOx2] == '\0') {
            GOx2 = 0;
            GOx1++;
        }
        GOv = (char *)NULL;
    }
    /*
     * Return the option character.
     */
    return (c);
}

/*
 * sv_fmt_str() - save format string
 */

static char *sv_fmt_str(struct lsof_context *ctx, char *f) /* format string */
{
    char *cp;
    MALLOC_S l;

    l = (MALLOC_S)(strlen(f) + 1);
    if (!(cp = (char *)malloc(l))) {
        (void)fprintf(stderr, "%s: can't allocate %d bytes for format: %s\n",
                      Pn, (int)l, f);
        Error(ctx);
    }
    (void)snpf(cp, l, "%s", f);
    return (cp);
}
