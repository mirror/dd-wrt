/*
 * dproc.c - Linux process access functions for /proc-based lsof
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

#include <inttypes.h>

/*
 * Local definitions
 */

#define FDINFO_FLAGS 0x1 /* fdinfo flags available */
#define FDINFO_POS 0x2   /* fdinfo position available */

#if defined(HASEPTOPTS)
#    define FDINFO_EVENTFD_ID 0x4 /* fdinfo eventfd-id available */
#    if defined(HASPTYEPT)
#        define FDINFO_TTY_INDEX 0x8 /* fdinfo tty-index available */
#    endif                           /* defined(HASPTYEPT) */
#endif                               /* defined(HASEPTOPTS) */

#define FDINFO_PID 0x10 /* pidfd pid available */
#define FDINFO_TFD 0x20 /* fd monitored by eventpoll fd */

#define FDINFO_BASE (FDINFO_FLAGS | FDINFO_POS)
#if defined(HASEPTOPTS)
#    if defined(HASPTYEPT)
#        define FDINFO_ALL                                                     \
            (FDINFO_BASE | FDINFO_TTY_INDEX | FDINFO_EVENTFD_ID | FDINFO_PID | \
             FDINFO_TFD)
#    else /* !defined(HASPTYEPT) */
#        define FDINFO_ALL                                                     \
            (FDINFO_BASE | FDINFO_EVENTFD_ID | FDINFO_PID | FDINFO_TFD)
#    endif /* defined(HASPTYEPT) */
#    define FDINFO_OPTIONAL (FDINFO_ALL & ~FDINFO_BASE)
#else /* !defined(HASEPTOPTS) */
#    define FDINFO_ALL (FDINFO_BASE | FDINFO_PID | FDINFO_TFD)
#endif /* defined(HASEPTOPTS) */

#define LSTAT_TEST_FILE "/"
#define LSTAT_TEST_SEEK 1

#if !defined(ULLONG_MAX)
#    define ULLONG_MAX 18446744073709551615ULL
#endif /* !defined(ULLONG_MAX) */

#define NS_PATH_LENGTH 100  /* namespace path string length */
#define MAP_PATH_LENGTH 100 /* map_files path length */
#define ADDR_LENGTH 100     /* addr range of map_files length */

/*
 * Local structures
 */

struct l_fdinfo {
    int flags; /* flags: line value */
    off_t pos; /* pos: line value */

#if defined(HASEPTOPTS)
    int eventfd_id;
#    if defined(HASPTYEPT)
    int tty_index; /* pty line index */
#    endif         /* defined(HASPTYEPT) */
#endif             /* defined(HASEPTOPTS) */

    int pid; /* for pidfd */

#define EPOLL_MAX_TFDS 32
    int tfds[EPOLL_MAX_TFDS];
    size_t tfd_count;
};

/*
 * Local variables
 */

static short Cckreg; /* conditional status of regular file
                      * checking:
                      *     0 = unconditionally check
                      *     1 = conditionally check */
static short Ckscko; /* socket file only checking status:
                      *     0 = none
                      *     1 = check only socket files */

/*
 * Local function prototypes
 */

static MALLOC_S alloc_cbf(struct lsof_context *ctx, MALLOC_S len, char **cbf,
                          MALLOC_S cbfa);
static int get_fdinfo(struct lsof_context *ctx, char *p, int msk,
                      struct l_fdinfo *fi);
static int getlinksrc(char *ln, char *src, int srcl, char **rest);
static int isefsys(struct lsof_context *ctx, char *path,
                   enum lsof_file_type type, int l, efsys_list_t **rep,
                   struct lfile **lfr);
static int nm2id(char *nm, int *id, int *idl);
static int read_id_stat(struct lsof_context *ctx, char *p, int id, char **cmd,
                        int *ppid, int *pgid);
static void process_proc_map(struct lsof_context *ctx, char *p, struct stat *s,
                             int ss);
static int process_id(struct lsof_context *ctx, char *idp, int idpl, char *cmd,
                      UID_ARG uid, int pid, int ppid, int pgid, int tid,
                      char *tcmd);
static int statEx(struct lsof_context *ctx, char *p, struct stat *s, int *ss);

static void snp_eventpoll(char *p, int len, int *tfds, int tfd_count);

#if defined(HASSELINUX)
static int cmp_cntx_eq(char *pcntx, char *ucntx);

#    include <fnmatch.h>

/*
 * cmp_cntx_eq -- compare program and user security contexts
 */

static int cmp_cntx_eq(char *pcntx, /* program context */
                       char *ucntx) /* user supplied context */
{
    return !fnmatch(ucntx, pcntx, 0);
}

/*
 * enter_cntx_arg() - enter name ecurity context argument
 */

int enter_cntx_arg(struct lsof_context *ctx, /* context */
                   char *cntx)               /* context */
{
    cntxlist_t *cntxp;
    /*
     * Search the argument list for a duplicate.
     */
    for (cntxp = CntxArg; cntxp; cntxp = cntxp->next) {
        if (!strcmp(cntxp->cntx, cntx)) {
            if (!Fwarn) {
                (void)fprintf(stderr, "%s: duplicate context: %s\n", Pn, cntx);
            }
            return (1);
        }
    }
    /*
     * Create and link a new context argument list entry.
     */
    if (!(cntxp = (cntxlist_t *)malloc((MALLOC_S)sizeof(cntxlist_t)))) {
        (void)fprintf(stderr, "%s: no space for context: %s\n", Pn, cntx);
        Error(ctx);
    }
    cntxp->f = 0;
    cntxp->cntx = cntx;
    cntxp->next = CntxArg;
    CntxArg = cntxp;
    return (0);
}
#endif /* defined(HASSELINUX) */

/*
 * alloc_cbf() -- allocate a command buffer
 */

static MALLOC_S alloc_cbf(struct lsof_context *ctx, /* context */
                          MALLOC_S len,             /* required length */
                          char **cbf,               /* current buffer */
                          MALLOC_S cbfa) /* current buffer allocation */
{
    if (*cbf)
        *cbf = (char *)realloc((MALLOC_P *)*cbf, len);
    else
        *cbf = (char *)malloc(len);
    if (!*cbf) {
        (void)fprintf(stderr, "%s: can't allocate command %d bytes\n", Pn,
                      (int)len);
        Error(ctx);
    }
    return (len);
}

/*
 * gather_proc_info() -- gather process information
 */

void gather_proc_info(struct lsof_context *ctx) {
    char *cmd, *tcmd;
    char cmdbuf[MAXPATHLEN];
    struct dirent *dp;
    unsigned char ht, pidts;
    int n, nl, pgid, pid, ppid, prv, rv, tid, tpgid, tppid, tx;
    static char *path = (char *)NULL;
    static int pathl = 0;
    static char *pidpath = (char *)NULL;
    static MALLOC_S pidpathl = 0;
    static MALLOC_S pidx = 0;
    static DIR *ps = (DIR *)NULL;
    struct stat sb;
    static char *taskpath = (char *)NULL;
    static int taskpathl = 0;
    static char *tidpath = (char *)NULL;
    static int tidpathl = 0;
    DIR *ts;
    UID_ARG uid;

    /*
     * Do one-time setup.
     */
    if (!pidpath) {
        pidx = strlen(PROCFS) + 1;
        pidpathl = pidx + 64 + 1; /* 64 is growth room */
        if (!(pidpath = (char *)malloc(pidpathl))) {
            (void)fprintf(stderr,
                          "%s: can't allocate %d bytes for \"%s/\"<pid>\n", Pn,
                          (int)pidpathl, PROCFS);
            Error(ctx);
        }
        (void)snpf(pidpath, pidpathl, "%s/", PROCFS);
    }
    /*
     * Get lock and net information.
     */
    (void)make_proc_path(ctx, pidpath, pidx, &path, &pathl, "locks");
    (void)get_locks(ctx, path);
    (void)make_proc_path(ctx, pidpath, pidx, &path, &pathl, "net/");
    (void)set_net_paths(ctx, path, strlen(path));
    /*
     * If only socket files have been selected, or socket files have been
     * selected ANDed with other selection options, enable the skipping of
     * regular files.
     *
     * If socket files and some process options have been selected, enable
     * conditional skipping of regular file; i.e., regular files will be skipped
     * unless they belong to a process selected by one of the specified options.
     */
    if (Selflags & SELNW) {

        /*
         * Some network files selection options have been specified.
         */
        if (Fand || !(Selflags & ~SELNW)) {

            /*
             * Selection ANDing or only network file options have been
             * specified, so set unconditional skipping of regular files
             * and socket file only checking.
             */
            Cckreg = 0;
            Ckscko = 1;
        } else {

            /*
             * If ORed file selection options have been specified, or no ORed
             * process selection options have been specified, enable
             * unconditional file checking and clear socket file only checking.
             *
             * If only ORed process selection options have been specified,
             * enable conditional file skipping and socket file only checking.
             */
            if ((Selflags & SELFILE) || !(Selflags & SelProc))
                Cckreg = Ckscko = 0;
            else
                Cckreg = Ckscko = 1;
        }
    } else {

        /*
         * No network file selection options were specified.  Enable
         * unconditional file checking and clear socket file only checking.
         */
        Cckreg = Ckscko = 0;
    }
    /*
     * Read /proc, looking for PID directories.  Open each one and
     * gather its process and file information.
     */
    if (!ps) {
        if (!(ps = opendir(PROCFS))) {
            (void)fprintf(stderr, "%s: can't open %s\n", Pn, PROCFS);
            Error(ctx);
        }
    } else
        (void)rewinddir(ps);
    while ((dp = readdir(ps))) {
        if (nm2id(dp->d_name, &pid, &n))
            continue;
        /*
         * Build path to PID's directory.
         */
        if ((pidx + n + 1 + 1) > pidpathl) {
            pidpathl = pidx + n + 1 + 1 + 64;
            if (!(pidpath = (char *)realloc((MALLOC_P *)pidpath, pidpathl))) {
                (void)fprintf(stderr,
                              "%s: can't allocate %d bytes for \"%s/%s/\"\n",
                              Pn, (int)pidpathl, PROCFS, dp->d_name);
                Error(ctx);
            }
        }
        (void)snpf(pidpath + pidx, pidpathl - pidx, "%s/", dp->d_name);
        n += (pidx + 1);
        /*
         * Process the PID's stat info.
         */
        if (stat(pidpath, &sb))
            continue;
        uid = (UID_ARG)sb.st_uid;
        ht = pidts = 0;
        /*
         * Get the PID's command name.
         */
        (void)make_proc_path(ctx, pidpath, n, &path, &pathl, "stat");
        if ((prv = read_id_stat(ctx, path, pid, &cmd, &ppid, &pgid)) < 0)
            cmd = NULL; /* NULL means failure to get command name */

#if defined(HASTASKS)
        /*
         * Task reporting has been selected, so save the process' command
         * string, so that task processing won't change it in the buffer of
         * read_id_stat().
         *
         * Check the tasks of the process first, so that the "-p<PID> -aK"
         * options work properly.
         */
        else if (!IgnTasks && (Selflags & SELTASK)) {
            /*
             * Copy cmd before next call to read_id_stat due to static
             * variables
             */
            if (cmd) {
                strncpy(cmdbuf, cmd, sizeof(cmdbuf) - 1);
                cmdbuf[sizeof(cmdbuf) - 1] = '\0';
                cmd = cmdbuf;
            }

            (void)make_proc_path(ctx, pidpath, n, &taskpath, &taskpathl,
                                 "task");
            tx = n + 4;
            if ((ts = opendir(taskpath))) {

                /*
                 * Process the PID's tasks.  Record the open files of those
                 * whose TIDs do not match the PID and which are themselves
                 * not zombies.
                 */
                while ((dp = readdir(ts))) {

                    /*
                     * Get the task ID.  Skip the task if its ID matches the
                     * process PID.
                     */
                    if (nm2id(dp->d_name, &tid, &nl))
                        continue;
                    if (tid == pid) {
                        pidts = 1;
                        continue;
                    }
                    /*
                     * Form the path for the TID.
                     */
                    if ((tx + 1 + nl + 1 + 4) > tidpathl) {
                        tidpathl = tx + 1 + n + 1 + 4 + 64;
                        if (tidpath)
                            tidpath =
                                (char *)realloc((MALLOC_P *)tidpath, tidpathl);
                        else
                            tidpath = (char *)malloc((MALLOC_S)tidpathl);
                        if (!tidpath) {
                            (void)fprintf(stderr,
                                          "%s: can't allocate %d task bytes",
                                          Pn, tidpathl);
                            (void)fprintf(stderr, " for \"%s/%s/stat\"\n",
                                          taskpath, dp->d_name);
                            Error(ctx);
                        }
                    }
                    (void)snpf(tidpath, tidpathl, "%s/%s/stat", taskpath,
                               dp->d_name);
                    /*
                     * Check the task state.
                     */
                    rv = read_id_stat(ctx, tidpath, tid, &tcmd, &tppid, &tpgid);
                    if ((rv < 0) || (rv == 1))
                        continue;
                    /*
                     * Attempt to record the task.
                     */
                    if (!process_id(ctx, tidpath, (tx + 1 + nl + 1), cmd, uid,
                                    pid, tppid, tpgid, tid, tcmd)) {
                        ht = 1;
                    }
                }
                (void)closedir(ts);
            }
        }
#endif /* defined(HASTASKS) */

        /*
         * If the main process is a task and task selection has been specified
         * along with option ANDing, enter the main process temporarily as a
         * task, so that the "-aK" option set lists the main process along
         * with its tasks.
         */
        if ((prv >= 0) && (prv != 1)) {
            tid = (Fand && ht && pidts && !IgnTasks && (Selflags & SELTASK))
                      ? pid
                      : 0;
            if ((!process_id(ctx, pidpath, n, cmd, uid, pid, ppid, pgid, tid,
                             (char *)NULL)) &&
                tid) {
                Lp->tid = 0;
            }
        }
    }
}

/*
 * get_fdinfo() - get values from /proc/<PID>fdinfo/FD
 */

static int get_fdinfo(struct lsof_context *ctx, /* context */
                      char *p,                  /* path to fdinfo file */
                      int msk,             /* mask for information type: e.g.,
                                            * the FDINFO_* definition */
                      struct l_fdinfo *fi) /* pointer to local fdinfo values
                                            * return structure */
{
    char buf[MAXPATHLEN + 1], *ep, **fp;
    FILE *fs;
    int rv = 0;
    unsigned long ul;
    unsigned long long ull;
    /*
     * Signal no values returned (0) if no fdinfo pointer was provided or if the
     * fdinfo path can't be opened.
     */
    if (!fi)
        return (0);

#if defined(HASEPTOPTS)
    fi->eventfd_id = -1;
#    if defined(HASPTYEPT)
    fi->tty_index = -1;
#    endif /* defined(HASPTYEPT) */
#endif     /* defined(HASEPTOPTS) */
    fi->pid = -1;
    fi->tfd_count = 0;

    if (!p || !*p || !(fs = fopen(p, "r")))
        return (0);
    /*
     * Read the fdinfo file.
     */
    while (fgets(buf, sizeof(buf), fs)) {
        int opt_flg = 0;
        if (get_fields(ctx, buf, (char *)NULL, &fp, (int *)NULL, 0) < 2)
            continue;
        if (!fp[0] || !*fp[0] || !fp[1] || !*fp[1])
            continue;
        if ((msk & FDINFO_FLAGS) && !strcmp(fp[0], "flags:")) {

            /*
             * Process a "flags:" line.
             */
            ep = (char *)NULL;
            if ((ul = strtoul(fp[1], &ep, 0)) == ULONG_MAX || !ep || *ep)
                continue;
            fi->flags = (unsigned int)ul;
            if ((rv |= FDINFO_FLAGS) == msk)
                break;
        } else if ((msk & FDINFO_POS) && !strcmp(fp[0], "pos:")) {

            /*
             * Process a "pos:" line.
             */
            ep = (char *)NULL;
            if ((ull = strtoull(fp[1], &ep, 0)) == ULLONG_MAX || !ep || *ep)
                continue;
            fi->pos = (off_t)ull;
            if ((rv |= FDINFO_POS) == msk)
                break;

        } else if (((msk & FDINFO_PID) && !strcmp(fp[0], "Pid:") &&
                    ((opt_flg = FDINFO_PID))) ||
                   ((msk & FDINFO_TFD) && !strcmp(fp[0], "tfd:") &&
                    ((opt_flg = FDINFO_TFD)))
#if defined(HASEPTOPTS)
                   || ((msk & FDINFO_EVENTFD_ID) &&
                       !strcmp(fp[0], "eventfd-id:") &&
                       ((opt_flg = FDINFO_EVENTFD_ID)))
#    if defined(HASPTYEPT)
                   ||
                   ((msk & FDINFO_TTY_INDEX) && !strcmp(fp[0], "tty-index:") &&
                    ((opt_flg = FDINFO_TTY_INDEX)))
#    endif /* defined(HASPTYEPT) */
#endif     /* defined(HASEPTOPTS) */
        ) {
            int val;
            /*
             * Process a "tty-index:", "eventfd-id:", "Pid:", or "tfid:" line.
             */
            ep = (char *)NULL;
            if ((ul = strtoul(fp[1], &ep, 0)) == ULONG_MAX || !ep || *ep)
                continue;

            val = (int)ul;
            if (val < 0) {
                /*
                 * Oops! If integer overflow occurred, reset the field.
                 */
                val = -1;
            }

            rv |= opt_flg;
            switch (opt_flg) {
#if defined(HASEPTOPTS)
            case FDINFO_EVENTFD_ID:
                fi->eventfd_id = val;
                break;
#    if defined(HASPTYEPT)
            case FDINFO_TTY_INDEX:
                fi->tty_index = val;
                break;
#    endif /* defined(HASPTYEPT) */
#endif     /* defined(HASEPTOPTS) */
            case FDINFO_PID:
                fi->pid = val;
                break;
            case FDINFO_TFD:
                if (fi->tfd_count < EPOLL_MAX_TFDS) {
                    fi->tfds[fi->tfd_count] = val;
                    fi->tfd_count++;
                }
                break;
            }

            if ((
                    /* There can be more than one tfd: lines.
                   So even if we found one, we can not exit the loop.
                   However, we can assume tfd lines are continuous. */
                    opt_flg != FDINFO_TFD &&
                    (rv == msk || (rv & FDINFO_TFD))) ||
                (
                    /* Too many tfds. */
                    opt_flg == FDINFO_TFD && rv == msk &&
                    fi->tfd_count == EPOLL_MAX_TFDS))
                break;
        }
    }
    fclose(fs);
    /*
     * Signal via the return value what information was obtained. (0 == none)
     */
    return (rv);
}

/*
 * getlinksrc() - get the source path name for the /proc/<PID>/fd/<FD> link
 */

static int getlinksrc(char *ln,    /* link path */
                      char *src,   /* link source path return address */
                      int srcl,    /* length of src[] */
                      char **rest) /* pointer to what follows the ':' in
                                    * the link source path (NULL if no
                                    * return requested) */
{
    char *cp;
    int ll;

    if (rest)
        *rest = (char *)NULL;
    if ((ll = readlink(ln, src, srcl - 1)) < 1 || ll >= srcl)
        return (-1);
    src[ll] = '\0';
    if (*src == '/')
        return (ll);
    if ((cp = strchr(src, ':'))) {
        *cp = '\0';
        ll = strlen(src);
        if (rest)
            *rest = cp + 1;
    }
    return (ll);
}

/*
 * initialize() - perform all initialization
 */

void initialize(struct lsof_context *ctx) {
    int fd;
    struct l_fdinfo fi;
    char path[MAXPATHLEN];
    struct stat sb;
    /*
     * Test for -i and -X option conflict.
     */
    if (Fxopt && (Fnet || Nwad)) {
        (void)fprintf(stderr, "%s: -i is useless when -X is specified.\n", Pn);
        usage(ctx, 1, 0, 0);
    }
    /*
     * Open LSTAT_TEST_FILE and seek to byte LSTAT_TEST_SEEK, then lstat the
     * /proc/<PID>/fd/<FD> for LSTAT_TEST_FILE to see what position is reported.
     * If the result is LSTAT_TEST_SEEK, enable offset reporting.
     *
     * If the result isn't LSTAT_TEST_SEEK, next check the fdinfo file for the
     * open LSTAT_TEST_FILE file descriptor.  If it exists and contains a "pos:"
     * value, and if the value is LSTAT_TEST_SEEK, enable offset reporting.
     */
    if ((fd = open(LSTAT_TEST_FILE, O_RDONLY)) >= 0) {
        if (lseek(fd, (off_t)LSTAT_TEST_SEEK, SEEK_SET) ==
            (off_t)LSTAT_TEST_SEEK) {
            (void)snpf(path, sizeof(path), "%s/%d/fd/%d", PROCFS, Mypid, fd);
            if (!lstat(path, &sb)) {
                if (sb.st_size == (off_t)LSTAT_TEST_SEEK)
                    OffType = OFFSET_LSTAT;
            }
        }
        if (OffType == OFFSET_UNKNOWN) {
            (void)snpf(path, sizeof(path), "%s/%d/fdinfo/%d", PROCFS, Mypid,
                       fd);
            if (get_fdinfo(ctx, path, FDINFO_POS, &fi) & FDINFO_POS) {
                if (fi.pos == (off_t)LSTAT_TEST_SEEK)
                    OffType = OFFSET_FDINFO;
            }
        }
        (void)close(fd);
    }
    if (OffType == OFFSET_UNKNOWN) {
        if (Foffset && !Fwarn)
            (void)fprintf(
                stderr, "%s: WARNING: can't report offset; disregarding -o.\n",
                Pn);
        Foffset = 0;
        Fsize = 1;
    }
    /*
     * Make sure the local mount info table is loaded if doing anything other
     * than just Internet lookups.  (HasNFS is defined during the loading of the
     * local mount table.)
     */
    if (Selinet == 0)
        (void)readmnt(ctx);
}

/*
 * make_proc_path() - make a path in a /proc directory
 *
 * entry:
 *	pp = pointer to /proc prefix
 *	lp = length of prefix
 *	np = pointer to malloc'd buffer to receive new file's path
 *	nl = size of new file path buffer
 *	sf = new path's suffix
 *
 * return: length of new path
 *	np = updated with new path
 *	nl = updated with new buffer size
 */
int make_proc_path(struct lsof_context *ctx, /* context */
                   char *pp,  /* path prefix -- e.g., /proc/<pid>/ */
                   int pl,    /* strlen(pp) */
                   char **np, /* malloc'd receiving buffer */
                   int *nl,   /* malloc'd size */
                   char *sf)  /* suffix of new path */
{
    char *cp;
    MALLOC_S rl, sl;

    sl = strlen(sf);
    if ((rl = pl + sl + 1) > *nl) {
        if ((cp = *np))
            cp = (char *)realloc((MALLOC_P *)cp, rl);
        else
            cp = (char *)malloc(rl);
        if (!cp) {
            (void)fprintf(stderr, "%s: can't allocate %d bytes for %s%s\n", Pn,
                          (int)rl, pp, sf);
            Error(ctx);
        }
        *nl = rl;
        *np = cp;
    }
    (void)snpf(*np, *nl, "%s", pp);
    (void)snpf(*np + pl, *nl - pl, "%s", sf);
    return (rl - 1);
}

/*
 * isefsys() -- is path on a file system exempted with -e
 *
 * Note: alloc_lfile() must have been called in advance.
 */

static int isefsys(struct lsof_context *ctx, /* context */
                   char *path,               /* path to file */
                   enum lsof_file_type type, /* unknown file type */
                   int l,                    /* link request: 0 = report
                                              *               1 = link */
                   efsys_list_t **rep,       /* returned Efsysl pointer, if not
                                              * NULL */
                   struct lfile **lfr) /* allocated struct lfile pointer */
{
    efsys_list_t *ep;
    int ds, len;
    struct mounts *mp;
    char nmabuf[MAXPATHLEN + 1];

    len = (int)strlen(path);
    for (ep = Efsysl; ep; ep = ep->next) {

        /*
         * Look for a matching exempt file system path at the beginning of
         * the file path.
         */
        if (ep->pathl > len)
            continue;
        if (strncmp(ep->path, path, ep->pathl))
            continue;
        /*
         * If only reporting, return information as requested.
         */
        if (!l) {
            if (rep)
                *rep = ep;
            return (0);
        }
        /*
         * Process an exempt file.
         */
        ds = 0;
        if ((mp = ep->mp)) {
            if (mp->ds & SB_DEV) {
                Lf->dev = mp->dev;
                ds = Lf->dev_def = 1;
            }
            if (mp->ds & SB_RDEV) {
                Lf->rdev = mp->rdev;
                ds = Lf->rdev_def = 1;
            }
        }
        if (!ds)
            (void)enter_dev_ch(ctx, "UNKNOWN");
        Lf->ntype = N_UNKN;
        Lf->type = type != LSOF_FILE_NONE ? type : LSOF_FILE_UNKNOWN;
        (void)enter_nm(ctx, path);
        (void)snpf(nmabuf, sizeof(nmabuf), "(%ce %s)", ep->rdlnk ? '+' : '-',
                   ep->path);
        nmabuf[sizeof(nmabuf) - 1] = '\0';
        (void)add_nma(ctx, nmabuf, strlen(nmabuf));
        if (Lf->sf) {
            if (lfr)
                *lfr = Lf;
            link_lfile(ctx);
        } else if (lfr)
            *lfr = (struct lfile *)NULL;
        return (0);
    }
    return (1);
}

/*
 * nm2id() - convert a name to an integer ID
 */

static int nm2id(char *nm, /* pointer to name */
                 int *id,  /* pointer to ID receiver */
                 int *idl) /* pointer to ID length receiver */
{
    int tid, tidl;
    int invalid;

    for (*id = *idl = tid = tidl = 0; *nm; nm++) {

#if defined(__STDC__) /* { */
        invalid = !isdigit((unsigned char)*nm);
#else  /* !defined(__STDC__)	   } { */
        invalid = !isascii(*nm) || !isdigit((unsigned char)*cp);
#endif /* defined(__STDC__)	   } */
        if (invalid) {
            return (1);
        }
        tid = tid * 10 + (int)(*nm - '0');
        tidl++;
    }
    *id = tid;
    *idl = tidl;
    return (0);
}

/*
 * open_proc_stream() -- open a /proc stream
 */

FILE *open_proc_stream(struct lsof_context *ctx, /* context */
                       char *p,                  /* pointer to path to open */
                       char *m,    /* pointer to mode -- e.g., "r" */
                       char **buf, /* pointer tp setvbuf() address
                                    * (NULL if none) */
                       size_t *sz, /* setvbuf() size (0 if none or if
                                    * getpagesize() desired */
                       int act)    /* fopen() failure action:
                                    *     0 : return (FILE *)NULL
                                    *   <>0 : fprintf() an error message
                                    *         and Error()
                                    */
{
    FILE *fs;                      /* opened stream */
    static size_t psz = (size_t)0; /* page size */
    size_t tsz;                    /* temporary size */
    /*
     * Open the stream.
     */
    if (!(fs = fopen(p, m))) {
        if (!act)
            return ((FILE *)NULL);
        (void)fprintf(stderr, "%s: can't fopen(%s, \"%s\"): %s\n", Pn, p, m,
                      strerror(errno));
        Error(ctx);
    }
    /*
     * Return the stream if no buffer change is required.
     */
    if (!buf)
        return (fs);
    /*
     * Determine the buffer size required.
     */
    if (!(tsz = *sz)) {
        if (!psz)
            psz = getpagesize();
        tsz = psz;
    }
    /*
     * Allocate a buffer for the stream, as required.
     */
    if (!*buf) {
        if (!(*buf = (char *)malloc((MALLOC_S)tsz))) {
            (void)fprintf(stderr,
                          "%s: can't allocate %d bytes for %s stream buffer\n",
                          Pn, (int)tsz, p);
            Error(ctx);
        }
        *sz = tsz;
    }
    /*
     * Assign the buffer to the stream.
     */
    if (setvbuf(fs, *buf, _IOFBF, tsz)) {
        (void)fprintf(stderr, "%s: setvbuf(%s)=%d failure: %s\n", Pn, p,
                      (int)tsz, strerror(errno));
        Error(ctx);
    }
    return (fs);
}

/*
 * process_id - process ID: PID or LWP
 *
 * return:  0 == ID processed
 *          1 == ID not processed
 */

static int process_id(struct lsof_context *ctx, /* context */
                      char *idp,                /* pointer to ID's path */
                      int idpl,    /* pointer to ID's path length */
                      char *cmd,   /* pointer to ID's command */
                      UID_ARG uid, /* ID's UID */
                      int pid,     /* ID's PID */
                      int ppid,    /* parent PID */
                      int pgid,    /* parent GID */
                      int tid,     /* task ID, if non-zero */
                      char *tcmd)  /* task command, if non-NULL) */
{
    int av = 0;
    static char *dpath = (char *)NULL;
    static int dpathl = 0;
    short efs, enls, enss, lnk, oty, pn, pss, sf;
    int fd, i, ls = 0, n, ss, sv;
    struct l_fdinfo fi;
    DIR *fdp;
    struct dirent *fp;
    static char *ipath = (char *)NULL;
    static int ipathl = 0;
    int j = 0;
    struct lfile *lfr;
    struct stat lsb, sb;
    char nmabuf[MAXPATHLEN + 1], pbuf[MAXPATHLEN + 1];
    static char *path = (char *)NULL;
    static int pathl = 0;
    static char *pathi = (char *)NULL;
    static int pathil = 0;
    char *rest;
    int txts = 0;

#if defined(HASSELINUX)
    cntxlist_t *cntxp;
#endif /* defined(HASSELINUX) */

    /*
     * See if process is excluded.
     */
    if (is_proc_excl(ctx, pid, pgid, uid, &pss, &sf, tid) ||
        is_cmd_excl(ctx, cmd, &pss, &sf)) {

#if defined(HASEPTOPTS)
        if (!FeptE)
            return (1);
#else  /* !defined(HASEPTOPTS) */
        return (1);
#endif /* defined(HASEPTOPTS) */
    }
    if (Cckreg && !FeptE) {

        /*
         * If conditional checking of regular files is enabled, enable
         * socket file only checking, based on the process' selection
         * status.
         */
        Ckscko = (sf & SelProc) ? 0 : 1;
    }
    alloc_lproc(ctx, pid, pgid, ppid, uid, cmd, (int)pss, (int)sf);
    Plf = (struct lfile *)NULL;

#if defined(HASTASKS)
    /*
     * Enter task information.
     */
    Lp->tid = tid;
    if (tid && tcmd) {
        if (!(Lp->tcmd = mkstrcpy(tcmd, (MALLOC_S *)NULL))) {
            (void)fprintf(stderr,
                          "%s: PID %d, TID %d, no space for task name: ", Pn,
                          pid, tid);
            safestrprt(tcmd, stderr, 1);
            Error(ctx);
        }
    }
#endif /* defined(HASTASKS) */

    /*
     * Process the ID's current working directory info.
     */
    efs = 0;
    if (!Ckscko) {
        (void)make_proc_path(ctx, idp, idpl, &path, &pathl, "cwd");
        alloc_lfile(ctx, LSOF_FD_CWD, -1);
        if (getlinksrc(path, pbuf, sizeof(pbuf), (char **)NULL) < 1) {
            if (!Fwarn) {
                zeromem((char *)&sb, sizeof(sb));
                lnk = ss = 0;
                (void)snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
                           strerror(errno));
                nmabuf[sizeof(nmabuf) - 1] = '\0';
                (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                pn = 1;
            } else
                pn = 0;
        } else {
            lnk = pn = 1;
            if (Efsysl &&
                !isefsys(ctx, pbuf, LSOF_FILE_UNKNOWN_CWD, 1, NULL, &lfr)) {
                efs = 1;
                pn = 0;
            } else {
                ss = SB_ALL;
                if (HasNFS) {
                    if ((sv = statsafely(ctx, path, &sb)))
                        sv = statEx(ctx, pbuf, &sb, &ss);
                } else
                    sv = stat(path, &sb);
                if (sv) {
                    ss = 0;
                    if (!Fwarn) {
                        (void)snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
                                   strerror(errno));
                        nmabuf[sizeof(nmabuf) - 1] = '\0';
                        (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                    }
                }
            }
        }
        if (pn) {
            (void)process_proc_node(ctx, lnk ? pbuf : path, path, &sb, ss,
                                    (struct stat *)NULL, 0);
            if (Lf->sf)
                link_lfile(ctx);
        }
    }
    /*
     * Process the ID's root directory info.
     */
    lnk = ss = 0;
    if (!Ckscko) {
        (void)make_proc_path(ctx, idp, idpl, &path, &pathl, "root");
        alloc_lfile(ctx, LSOF_FD_ROOT_DIR, -1);
        if (getlinksrc(path, pbuf, sizeof(pbuf), (char **)NULL) < 1) {
            if (!Fwarn) {
                zeromem((char *)&sb, sizeof(sb));
                (void)snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
                           strerror(errno));
                nmabuf[sizeof(nmabuf) - 1] = '\0';
                (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                pn = 1;
            } else
                pn = 0;
        } else {
            lnk = pn = 1;
            if (Efsysl &&
                !isefsys(ctx, pbuf, LSOF_FILE_UNKNOWN_ROOT_DIR, 1, NULL, NULL))
                pn = 0;
            else {
                ss = SB_ALL;
                if (HasNFS) {
                    if ((sv = statsafely(ctx, path, &sb)))
                        sv = statEx(ctx, pbuf, &sb, &ss);
                } else
                    sv = stat(path, &sb);
                if (sv) {
                    ss = 0;
                    if (!Fwarn) {
                        (void)snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
                                   strerror(errno));
                        nmabuf[sizeof(nmabuf) - 1] = '\0';
                        (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                    }
                }
            }
        }
        if (pn) {
            (void)process_proc_node(ctx, lnk ? pbuf : path, path, &sb, ss,
                                    (struct stat *)NULL, 0);
            if (Lf->sf)
                link_lfile(ctx);
        }
    }
    /*
     * Process the ID's execution info.
     */
    lnk = ss = txts = 0;
    if (!Ckscko) {
        (void)make_proc_path(ctx, idp, idpl, &path, &pathl, "exe");
        alloc_lfile(ctx, LSOF_FD_PROGRAM_TEXT, -1);
        if (getlinksrc(path, pbuf, sizeof(pbuf), (char **)NULL) < 1) {
            zeromem((void *)&sb, sizeof(sb));
            if (!Fwarn) {
                if ((errno != ENOENT) || uid) {
                    (void)snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
                               strerror(errno));
                    nmabuf[sizeof(nmabuf) - 1] = '\0';
                    (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                }
                pn = 1;
            } else
                pn = 0;
        } else {
            lnk = pn = 1;
            if (Efsysl && !isefsys(ctx, pbuf, LSOF_FILE_UNKNOWN_PROGRAM_TEXT, 1,
                                   NULL, NULL))
                pn = 0;
            else {
                ss = SB_ALL;
                if (HasNFS) {
                    if ((sv = statsafely(ctx, path, &sb))) {
                        sv = statEx(ctx, pbuf, &sb, &ss);
                        if (!sv && (ss & SB_DEV) && (ss & SB_INO))
                            txts = 1;
                    }
                } else
                    sv = stat(path, &sb);
                if (sv) {
                    ss = 0;
                    if (!Fwarn) {
                        (void)snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
                                   strerror(errno));
                        nmabuf[sizeof(nmabuf) - 1] = '\0';
                        (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                    }
                } else
                    txts = 1;
            }
        }
        if (pn) {
            (void)process_proc_node(ctx, lnk ? pbuf : path, path, &sb, ss,
                                    (struct stat *)NULL, 0);
            if (Lf->sf)
                link_lfile(ctx);
        }
    }
    /*
     * Process the ID's memory map info.
     */
    if (!Ckscko) {
        (void)make_proc_path(ctx, idp, idpl, &path, &pathl, "maps");
        (void)process_proc_map(ctx, path, txts ? &sb : (struct stat *)NULL,
                               txts ? ss : 0);
    }

#if defined(HASSELINUX)
    /*
     * Process the PID's SELinux context.
     */
    /*
     * match the valid contexts.
     */
    errno = 0;
    if (getpidcon(pid, &Lp->cntx) == -1) {
        Lp->cntx = (char *)NULL;
        if (!Fwarn) {
            (void)snpf(nmabuf, sizeof(nmabuf), "(getpidcon: %s)",
                       strerror(errno));
            if (!(Lp->cntx = strdup(nmabuf))) {
                (void)fprintf(stderr, "%s: no context error space: PID %ld", Pn,
                              (long)Lp->pid);
                Error(ctx);
            }
        }
    } else if (CntxArg) {

        /*
         * See if context includes the process.
         */
        for (cntxp = CntxArg; cntxp; cntxp = cntxp->next) {
            if (cmp_cntx_eq(Lp->cntx, cntxp->cntx)) {
                cntxp->f = 1;
                Lp->pss |= PS_PRI;
                Lp->sf |= SELCNTX;
                break;
            }
        }
    }
#endif /* defined(HASSELINUX) */

    /*
     * Process the ID's file descriptor directory.
     */
    if ((i = make_proc_path(ctx, idp, idpl, &dpath, &dpathl, "fd/")) < 3)
        return (0);
    dpath[i - 1] = '\0';
    if ((OffType == OFFSET_FDINFO) &&
        ((j = make_proc_path(ctx, idp, idpl, &ipath, &ipathl, "fdinfo/")) >= 7))
        oty = 1;
    else
        oty = 0;
    if (!(fdp = opendir(dpath))) {
        if (!Fwarn) {
            (void)snpf(nmabuf, sizeof(nmabuf), "%s (opendir: %s)", dpath,
                       strerror(errno));
            alloc_lfile(ctx, LSOF_FD_NOFD, -1);
            nmabuf[sizeof(nmabuf) - 1] = '\0';
            (void)add_nma(ctx, nmabuf, strlen(nmabuf));
            link_lfile(ctx);
        }
        return (0);
    }
    dpath[i - 1] = '/';
    while ((fp = readdir(fdp))) {
        if (nm2id(fp->d_name, &fd, &n))
            continue;
        (void)make_proc_path(ctx, dpath, i, &path, &pathl, fp->d_name);
        (void)alloc_lfile(ctx, LSOF_FD_NUMERIC, fd);
        if (getlinksrc(path, pbuf, sizeof(pbuf), &rest) < 1) {
            zeromem((char *)&sb, sizeof(sb));
            lnk = ss = 0;
            if (!Fwarn) {
                ls = 0;
                (void)snpf(nmabuf, sizeof(nmabuf), "(readlink: %s)",
                           strerror(errno));
                nmabuf[sizeof(nmabuf) - 1] = '\0';
                (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                pn = 1;
            } else
                pn = 0;
        } else {
            lnk = 1;
            if (Efsysl &&
                !isefsys(ctx, pbuf, LSOF_FILE_UNKNOWN_FD, 1, NULL, &lfr)) {
                efs = 1;
                pn = 0;
            } else {
                if (HasNFS) {
                    if (lstatsafely(ctx, path, &lsb)) {
                        (void)statEx(ctx, pbuf, &lsb, &ls);
                        enls = errno;
                    } else {
                        enls = 0;
                        ls = SB_ALL;
                    }
                    if (statsafely(ctx, path, &sb)) {
                        (void)statEx(ctx, pbuf, &sb, &ss);
                        enss = errno;
                    } else {
                        enss = 0;
                        ss = SB_ALL;
                    }
                } else {
                    ls = lstat(path, &lsb) ? 0 : SB_ALL;
                    enls = errno;
                    ss = stat(path, &sb) ? 0 : SB_ALL;
                    enss = errno;
                }
                if (!ls && !Fwarn) {
                    (void)snpf(nmabuf, sizeof(nmabuf), "lstat: %s)",
                               strerror(enls));
                    nmabuf[sizeof(nmabuf) - 1] = '\0';
                    (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                }
                if (!ss && !Fwarn) {
                    (void)snpf(nmabuf, sizeof(nmabuf), "(stat: %s)",
                               strerror(enss));
                    nmabuf[sizeof(nmabuf) - 1] = '\0';
                    (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                }
                if (Ckscko) {
                    if ((ss & SB_MODE) && ((sb.st_mode & S_IFMT) == S_IFSOCK)) {
                        pn = 1;
                    } else
                        pn = 0;
                } else
                    pn = 1;
            }
        }
        if (pn || (efs && lfr && oty)) {
            /* Clear fi in case oty == 0 */
            fi.eventfd_id = -1;
            fi.pid = -1;
            fi.tfd_count = -1;

            if (oty) {
                int fdinfo_mask = FDINFO_BASE;
                (void)make_proc_path(ctx, ipath, j, &pathi, &pathil,
                                     fp->d_name);

                if (rest && rest[0] == '[' && rest[1] == 'e' &&
                    rest[2] == 'v' && rest[3] == 'e' && rest[4] == 'n' &&
                    rest[5] == 't') {
#if defined(HASEPTOPTS)
                    if (rest[6] == 'f')
                        fdinfo_mask |= FDINFO_EVENTFD_ID;
#endif /* defined(HASEPTOPTS) */
                    else if (rest[6] == 'p')
                        fdinfo_mask |= FDINFO_TFD;
                }
#if defined(HASEPTOPTS)
#    if defined(HASPTYEPT)
                fdinfo_mask |= FDINFO_TTY_INDEX;
#    endif /* defined(HASPTYEPT) */
#endif     /* defined(HASEPTOPTS) */
                if (rest && rest[0] == '[' && rest[1] == 'p')
                    fdinfo_mask |= FDINFO_PID;

                if ((av = get_fdinfo(ctx, pathi, fdinfo_mask, &fi)) &
                    FDINFO_POS) {
                    if (efs) {
                        lfr->off = (SZOFFTYPE)fi.pos;
                        lfr->off_def = 1;
                    } else {
                        ls |= SB_SIZE;
                        lsb.st_size = fi.pos;
                    }
                } else
                    ls &= ~SB_SIZE;

#if !defined(HASNOFSFLAGS)
                if (av & FDINFO_FLAGS) {
                    if (efs) {
                        lfr->ffg = (long)fi.flags;
                        lfr->fsv |= FSV_FG;
                    } else {
                        Lf->ffg = (long)fi.flags;
                        Lf->fsv |= FSV_FG;
                    }
                }
#endif /* !defined(HASNOFSFLAGS) */
            }
            if (pn) {
                process_proc_node(ctx, lnk ? pbuf : path, path, &sb, ss, &lsb,
                                  ls);
                if (Lf->ntype == N_ANON_INODE) {
                    if (rest && *rest) {
#if defined(HASEPTOPTS)
                        if (fi.eventfd_id != -1 &&
                            strcmp(rest, "[eventfd]") == 0) {
                            (void)snpf(rest, sizeof(pbuf) - (rest - pbuf),
                                       "[eventfd:%d]", fi.eventfd_id);
                        }
#endif /* defined(HASPTYEPT) */
                        if (fi.pid != -1 && strcmp(rest, "[pidfd]") == 0) {
                            (void)snpf(rest, sizeof(pbuf) - (rest - pbuf),
                                       "[pidfd:%d]", fi.pid);
                        }
                        if (fi.tfd_count > 0 &&
                            strcmp(rest, "[eventpoll]") == 0) {
                            snp_eventpoll(rest, sizeof(pbuf) - (rest - pbuf),
                                          fi.tfds, fi.tfd_count);
                        }

                        enter_nm(ctx, rest);
                    }
#if defined(HASEPTOPTS)
                    if (FeptE && fi.eventfd_id != -1) {
                        enter_evtfdinfo(ctx, fi.eventfd_id);
                        Lf->eventfd_id = fi.eventfd_id;
                        Lf->sf |= SELEVTFDINFO;
                    }
#endif /* defined(HASPTYEPT) */
                }
#if defined(HASEPTOPTS) && defined(HASPTYEPT)
                else if (FeptE && Lf->rdev_def && is_pty_ptmx(Lf->rdev) &&
                         (av & FDINFO_TTY_INDEX)) {
                    enter_ptmxi(ctx, fi.tty_index);
                    Lf->tty_index = fi.tty_index;
                    Lf->sf |= SELPTYINFO;
                }
#endif /* defined(HASEPTOPTS) && defined(HASPTYEPT) */

                if (Lf->sf)
                    link_lfile(ctx);
            }
        }
    }
    (void)closedir(fdp);
    return (0);
}

/* compare mount namespace of this lsof process and the target process */

static int compare_mntns(int pid) /* pid of the target process */
{
    char nspath[NS_PATH_LENGTH];
    struct stat sb_self, sb_target;
    int ret;

    if (stat("/proc/self/ns/mnt", &sb_self))
        return -1;

    ret = snprintf(nspath, sizeof(nspath), "/proc/%d/ns/mnt", pid);
    if (ret >= sizeof(nspath) || ret <= 0)
        return -1;

    if (stat(nspath, &sb_target))
        return -1;

    if (sb_self.st_ino != sb_target.st_ino)
        return -1;

    return 0;
}

/*
 * process_proc_map() - process the memory map of a process
 */

static void
process_proc_map(struct lsof_context *ctx, /* context */
                 char *p,                  /* path to process maps file */
                 struct stat *s, /* executing text file state buffer */
                 int ss)         /* *s status -- i.e., SB_* values */
{
    char buf[MAXPATHLEN + 1], *ep, fmtbuf[32], **fp, nmabuf[MAXPATHLEN + 1];
    dev_t dev;
    int ds, efs, en, i, mss, sv;
    int eb = 6;
    INODETYPE inode;
    MALLOC_S len;
    long maj, min;
    FILE *ms;
    int ns = 0;
    struct stat sb;
    struct saved_map {
        dev_t dev;
        INODETYPE inode;
    };
    static struct saved_map *sm = (struct saved_map *)NULL;
    efsys_list_t *rep;
    static int sma = 0;
    static char *vbuf = (char *)NULL;
    static size_t vsz = (size_t)0;
    int diff_mntns = 0;
    /*
     * Open the /proc/<pid>/maps file, assign a page size buffer to its stream,
     * and read it/
     */
    if (!(ms = open_proc_stream(ctx, p, "r", &vbuf, &vsz, 0)))
        return;

    /* target process in a different mount namespace from lsof process. */
    if (compare_mntns(Lp->pid))
        diff_mntns = 1;

    while (fgets(buf, sizeof(buf), ms)) {
        if (get_fields(ctx, buf, ":", &fp, &eb, 1) < 7)
            continue; /* not enough fields */
        if (!fp[6] || !*fp[6])
            continue; /* no path name */
        /*
         * See if the path ends in " (deleted)".  If it does, strip the
         * " (deleted)" characters and remember that they were there.
         */
        if (((ds = (int)strlen(fp[6])) > 10) &&
            !strcmp(fp[6] + ds - 10, " (deleted)")) {
            *(fp[6] + ds - 10) = '\0';
        } else
            ds = 0;
        /*
         * Assemble the major and minor device numbers.
         */
        ep = (char *)NULL;
        if (!fp[3] || !*fp[3] || (maj = strtol(fp[3], &ep, 16)) == LONG_MIN ||
            maj == LONG_MAX || !ep || *ep)
            continue;
        ep = (char *)NULL;
        if (!fp[4] || !*fp[4] || (min = strtol(fp[4], &ep, 16)) == LONG_MIN ||
            min == LONG_MAX || !ep || *ep)
            continue;
        /*
         * Assemble the device and inode numbers.  If they are both zero, skip
         * the entry.
         */
        dev = (dev_t)makedev((int)maj, (int)min);
        if (!fp[5] || !*fp[5])
            continue;
        ep = (char *)NULL;
        if ((inode = strtoull(fp[5], &ep, 0)) == ULLONG_MAX || !ep || *ep)
            continue;
        if (!dev && !inode)
            continue;
        /*
         * See if the device + inode pair match that of the executable.
         * If they do, skip this map entry.
         */
        if (s && (ss & SB_DEV) && (ss & SB_INO) && (dev == s->st_dev) &&
            (inode == (INODETYPE)s->st_ino))
            continue;
        /*
         * See if this device + inode pair has already been processed as
         * a map entry.
         */
        for (i = 0; i < ns; i++) {
            if (dev == sm[i].dev && inode == sm[i].inode)
                break;
        }
        if (i < ns)
            continue;
        /*
         * Record the processing of this map entry's device and inode pair.
         */
        if (ns >= sma) {
            sma += 10;
            len = (MALLOC_S)(sma * sizeof(struct saved_map));
            if (sm)
                sm = (struct saved_map *)realloc(sm, len);
            else
                sm = (struct saved_map *)malloc(len);
            if (!sm) {
                (void)fprintf(
                    stderr,
                    "%s: can't allocate %d bytes for saved maps, PID %d\n", Pn,
                    (int)len, Lp->pid);
                Error(ctx);
            }
        }
        sm[ns].dev = dev;
        sm[ns++].inode = inode;
        /*
         * Allocate space for the mapped file, then get stat(2) information
         * for it.  Skip the stat(2) operation if this is on an exempt file
         * system.
         */
        alloc_lfile(ctx, LSOF_FD_MEMORY, -1);
        if (Efsysl && !isefsys(ctx, fp[6], LSOF_FILE_NONE, 0, &rep, NULL))
            efs = sv = 1;
        else
            efs = 0;

        /* For processes in different mount namespace from lsof process,
         * stat corresponding files under /proc/[pid]/map_files would follow
         * symlinks regardless of namespaces.
         */
        if (diff_mntns) {
            char path[MAP_PATH_LENGTH];
            char addr[ADDR_LENGTH];
            uint64_t start, end;
            int ret;

            if (sscanf(fp[0], "%" SCNx64 "-%" SCNx64, &start, &end) != 2)
                goto stat_directly;

            ret = snprintf(addr, sizeof(addr), "%" PRIx64 "-%" PRIx64, start,
                           end);
            if (ret >= sizeof(addr) || ret <= 0)
                goto stat_directly;

            ret = snprintf(path, sizeof(path), "/proc/%d/map_files/%s", Lp->pid,
                           addr);
            if (ret >= sizeof(path) || ret <= 0)
                goto stat_directly;

            if (!efs) {
                if (HasNFS)
                    sv = statsafely(ctx, path, &sb);
                else
                    sv = stat(path, &sb);
            }
        } else {
        stat_directly:
            if (!efs) {
                if (HasNFS)
                    sv = statsafely(ctx, fp[6], &sb);
                else
                    sv = stat(fp[6], &sb);
            }
        }
        if (sv || efs) {
            en = errno;
            /*
             * Applying stat(2) to the file was not possible (file is on an
             * exempt file system) or stat(2) failed, so manufacture a partial
             * stat(2) reply from the process' maps file entry.
             *
             * If the file has been deleted, reset its type to "DEL";
             * otherwise generate a stat() error name addition.
             */
            zeromem((char *)&sb, sizeof(sb));
            sb.st_dev = dev;
            sb.st_ino = (ino_t)inode;
            sb.st_mode = S_IFREG;
            mss = SB_DEV | SB_INO | SB_MODE;
            if (ds)
                alloc_lfile(ctx, LSOF_FD_DELETED, -1);
            else if (!efs && !Fwarn) {
                (void)snpf(nmabuf, sizeof(nmabuf), "(stat: %s)", strerror(en));
                nmabuf[sizeof(nmabuf) - 1] = '\0';
                (void)add_nma(ctx, nmabuf, strlen(nmabuf));
            }
        } else if (diff_mntns) {
            mss = SB_ALL;
        } else if ((sb.st_dev != dev) || ((INODETYPE)sb.st_ino != inode)) {

            /*
             * The stat(2) device and inode numbers don't match those obtained
             * from the process' maps file.
             *
             * If the file has been deleted, reset its type to "DEL"; otherwise
             * generate inconsistency name additions.
             *
             * Manufacture a partial stat(2) reply from the maps file
             * information.
             */
            if (ds)
                alloc_lfile(ctx, LSOF_FD_DELETED, -1);
            else if (!Fwarn) {
                char *sep;

                if (sb.st_dev != dev) {
                    (void)snpf(nmabuf, sizeof(nmabuf), "(path dev=%d,%d%s",
                               GET_MAJ_DEV(sb.st_dev), GET_MIN_DEV(sb.st_dev),
                               ((INODETYPE)sb.st_ino == inode) ? ")" : ",");
                    nmabuf[sizeof(nmabuf) - 1] = '\0';
                    (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                    sep = "";
                } else
                    sep = "(path ";
                if ((INODETYPE)sb.st_ino != inode) {
                    (void)snpf(fmtbuf, sizeof(fmtbuf),
                               "%%sinode=%%" INODEPSPEC "u)");
                    (void)snpf(nmabuf, sizeof(nmabuf), fmtbuf, sep,
                               (INODETYPE)sb.st_ino);
                    nmabuf[sizeof(nmabuf) - 1] = '\0';
                    (void)add_nma(ctx, nmabuf, strlen(nmabuf));
                }
            }
            zeromem((char *)&sb, sizeof(sb));
            sb.st_dev = dev;
            sb.st_ino = (ino_t)inode;
            sb.st_mode = S_IFREG;
            mss = SB_DEV | SB_INO | SB_MODE;
        } else
            mss = SB_ALL;
        /*
         * Record the file's information.
         */
        if (!efs)
            process_proc_node(ctx, fp[6], fp[6], &sb, mss, (struct stat *)NULL,
                              0);
        else {

            /*
             * If this file is on an exempt file system, complete the lfile
             * structure, but change its type and add the exemption note to
             * the NAME column.
             */
            Lf->dev = sb.st_dev;
            Lf->inode = (ino_t)sb.st_ino;
            Lf->dev_def = Lf->inp_ty = 1;
            (void)enter_nm(ctx, fp[6]);
            Lf->type =
                ds ? LSOF_FILE_UNKNOWN_DELETED : LSOF_FILE_UNKNOWN_MEMORY;
            (void)snpf(nmabuf, sizeof(nmabuf), "(%ce %s)",
                       rep->rdlnk ? '+' : '-', rep->path);
            nmabuf[sizeof(nmabuf) - 1] = '\0';
            (void)add_nma(ctx, nmabuf, strlen(nmabuf));
        }
        if (Lf->sf)
            link_lfile(ctx);
    }
    (void)fclose(ms);
}

/*
 * read_id_stat() - read ID (PID or LWP ID) status
 *
 * return: -1 == ID is unavailable
 *          0 == ID OK
 *          1 == ID is a zombie
 *	    2 == ID is a thread
 */
static int read_id_stat(struct lsof_context *ctx, /* context */
                        char *p,                  /* path to status file */
                        int id,                   /* ID: PID or LWP */
                        char **cmd,               /* malloc'd command name */
                        int *ppid, /* returned parent PID for PID type */
                        int *pgid) /* returned process group ID for PID
                                    * type */
{
    char buf[MAXPATHLEN], *cp, *cp1, **fp;
    int ch, cx, es, pc;
    static char *cbf = (char *)NULL;
    static MALLOC_S cbfa = 0;
    FILE *fs;
    static char *vbuf = (char *)NULL;
    static size_t vsz = (size_t)0;
    /*
     * Open the stat file path, assign a page size buffer to its stream,
     * and read the file's first line.
     */
    if (!(fs = open_proc_stream(ctx, p, "r", &vbuf, &vsz, 0)))
        return (-1);
    if (!(cp = fgets(buf, sizeof(buf), fs))) {

    read_id_stat_exit:

        (void)fclose(fs);
        return (-1);
    }
    /*
     * Skip to the first field, and make sure it is a matching ID.
     */
    cp1 = cp;
    while (*cp && (*cp != ' ') && (*cp != '\t'))
        cp++;
    if (*cp)
        *cp = '\0';
    if (atoi(cp1) != id)
        goto read_id_stat_exit;
    /*
     * The second field should contain the command, enclosed in parentheses.
     * If it also has embedded '\n' characters, replace them with '?'
     * characters, accumulating command characters until a closing parentheses
     * appears.
     *
     */
    for (++cp; *cp && (*cp == ' '); cp++)
        ;
    if (!cp || (*cp != '('))
        goto read_id_stat_exit;
    cp++;
    pc = 1; /* start the parenthesis balance count at 1 */

    /* empty process name to avoid leaking previous process name,
     * see issue #246
     */
    if (cbf) {
        cbf[0] = '\0';
    }

    /*
     * Enter the command characters safely.  Supply them from the initial read
     * of the stat file line, a '\n' if the initial read didn't yield a ')'
     * command closure, or by reading the rest of the command a character at
     * a time from the stat file.  Count embedded '(' characters and balance
     * them with embedded ')' characters.  The opening '(' starts the balance
     * count at one.
     */
    for (cx = es = 0;;) {
        if (!es)
            ch = *cp++;
        else {
            if ((ch = fgetc(fs)) == EOF)
                goto read_id_stat_exit;
        }
        if (ch == '(') /* a '(' advances the balance count */
            pc++;
        if (ch == ')') {

            /*
             * Balance parentheses when a closure is encountered.  When
             * they are balanced, this is the end of the command.
             */
            pc--;
            if (!pc)
                break;
        }
        if ((cx + 2) > cbfa)
            cbfa = alloc_cbf(ctx, (cx + 2), &cbf, cbfa);
        cbf[cx] = ch;
        cx++;
        cbf[cx] = '\0';
        if (!es && !*cp)
            es = 1; /* Switch to fgetc() when a '\0' appears. */
    }
    *cmd = cbf;
    /*
     * Read the remainder of the stat line if it was necessary to read command
     * characters individually from the stat file.
     *
     * Separate the reminder into fields.
     */
    if (es)
        cp = fgets(buf, sizeof(buf), fs);
    (void)fclose(fs);
    if (!cp || !*cp)
        return (-1);
    if (get_fields(ctx, cp, (char *)NULL, &fp, (int *)NULL, 0) < 3)
        return (-1);
    /*
     * Convert and return parent process (fourth field) and process group (fifth
     * field) IDs.
     */
    if (fp[1] && *fp[1])
        *ppid = atoi(fp[1]);
    else
        return (-1);
    if (fp[2] && *fp[2])
        *pgid = atoi(fp[2]);
    else
        return (-1);
    /*
     * Check the state in the third field.  If it is 'Z', return that
     * indication.
     */
    if (fp[0] && !strcmp(fp[0], "Z"))
        return (1);
    else if (fp[0] && !strcmp(fp[0], "T"))
        return (2);
    return (0);
}

/*
 * statEx() - extended stat() to get device numbers when a "safe" stat has
 *	      failed and the system has an NFS mount
 *
 * Note: this function was suggested by Paul Szabo as a way to get device
 *       numbers for NFS files when an NFS mount point has the root_squash
 *       option set.  In that case, even if lsof is setuid(root), the identity
 *	 of its requests to stat() NFS files lose root permission and may fail.
 *
 *	 This function should be used only when links have been successfully
 *	 resolved in the /proc path by getlinksrc().
 */
static int statEx(struct lsof_context *ctx, /* context */
                  char *p,                  /* file path */
                  struct stat *s,           /* stat() result -- NULL if none
                                             * wanted */
                  int *ss)                  /* stat() status --  SB_* values */
{
    static size_t ca = 0;
    static char *cb = NULL;
    char *cp;
    int ensv = ENOENT;
    struct stat sb;
    int st = 0;
    size_t sz;
    /*
     * Make a copy of the path.
     */
    sz = strlen(p);
    if ((sz + 1) > ca) {
        if (cb)
            cb = (char *)realloc((MALLOC_P *)cb, sz + 1);
        else
            cb = (char *)malloc(sz + 1);
        if (!cb) {
            (void)fprintf(stderr, "%s: PID %ld: no statEx path space: %s\n", Pn,
                          (long)Lp->pid, p);
            Error(ctx);
        }
        ca = sz + 1;
    }
    (void)strcpy(cb, p);
    /*
     * Trim trailing leaves from the end of the path one at a time and do a safe
     * stat() on each trimmed result.  Stop when a safe stat() succeeds or
     * doesn't fail because of EACCES or EPERM.
     */
    for (cp = strrchr(cb, '/'); cp && (cp != cb);) {
        *cp = '\0';
        if (!statsafely(ctx, cb, &sb)) {
            st = 1;
            break;
        }
        ensv = errno;
        if ((ensv != EACCES) && (ensv != EPERM))
            break;
        cp = strrchr(cb, '/');
    }
    /*
     * If a stat() on a trimmed result succeeded, form partial results
     * containing only the device and raw device numbers.
     */
    zeromem((char *)s, sizeof(struct stat));
    if (st) {
        errno = 0;
        s->st_dev = sb.st_dev;
        s->st_rdev = sb.st_rdev;
        *ss = SB_DEV | SB_RDEV;
        return (0);
    }
    errno = ensv;
    *ss = 0;
    return (1);
}

static int cal_order(int n) {
    int i = 0;
    do {
        n /= 10;
        i++;
    } while (n);
    return i;
}

static int snp_eventpoll_fds(char *p, int len, int *tfds, int count) {
    int wl = 0;
    int i;

    if (len > 0)
        p[0] = '\0';

    for (i = 0; i < count; i++) {
        int is_last_item = (i == count - 1);
        int needs = cal_order(tfds[i]) + (is_last_item ? 0 : 3);

        if ((len - wl + (wl ? 2 : 0)) <= needs) {
            /* No space to print the tfd. */
            break;
        }

        if (wl) {
            /* Rewrite the last "..." to ",". */
            wl -= 3;
            p[wl++] = ',';
        }
        wl += snpf(p + wl, len - wl, (is_last_item ? "%d" : "%d..."), tfds[i]);
    }

    return wl;
}

static int fd_compare(const void *a, const void *b) {
    int ia = *(int *)a, ib = *(int *)b;

    return ia - ib;
}

static void snp_eventpoll(char *p, int len, int *tfds, int tfd_count) {
    /* Reserve the area for prefix: "[eventpoll:" */
    len -= 11;
    /* Reserve the area for postfix */
    len -= ((tfd_count == EPOLL_MAX_TFDS) ? 4 /* "...]" */
                                          : 1 /* "]" */
            ) +
           1 /* for the last \0 */
        ;

    if (len > 1) {
        qsort(tfds, tfd_count, sizeof(tfds[0]), fd_compare);

        p[10] = ':'; /* "[eventpoll]" => "[eventpoll:" */
        int wl = snp_eventpoll_fds(p + 11, len, tfds, tfd_count);

        /* If the buffer doesn't have enough space, snp_eventpoll_fds puts
         * "..." at the end of the buffer. In that case we don't
         * have to "..." here.
         */
        const char *postfix =
            ((wl > 3 && p[11 + wl - 1] == '.')
                 ? "]"
                 : ((tfd_count == EPOLL_MAX_TFDS)
                        /* File descriptors more than EPOLL_MAX_TFDS are
                         * associated to the eventpoll fd. */
                        ? "...]"
                        : "]"));
        strcpy(p + 11 + wl, postfix);
    }
}
