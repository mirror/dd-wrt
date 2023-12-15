/*
 * proto.h - common function prototypes for lsof
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

/*
 * $Id: proto.h,v 1.39 2018/02/14 14:20:14 abe Exp $
 */

#if !defined(PROTO_H)
#    define PROTO_H 1

/*
 * The following define keeps gcc>=2.7 from complaining about the failure
 * of the Exit() function to return.
 *
 * Paul Eggert supplied it.
 */

#    if defined(__GNUC__) &&                                                   \
        !(__GNUC__ < 2 || (__GNUC__ == 2 && __GNUC_MINOR__ < 7))
#        define exiting __attribute__((__noreturn__))
#    else /* !gcc || gcc<2.7 */
#        define exiting
#    endif /* gcc && gcc>=2.7 */

extern void add_nma(struct lsof_context *ctx, char *cp, int len);
extern void alloc_lfile(struct lsof_context *ctx, enum lsof_fd_type fd_type,
                        int num);
extern void alloc_lproc(struct lsof_context *ctx, int pid, int pgid, int ppid,
                        UID_ARG uid, char *cmd, int pss, int sf);
extern void build_IPstates(struct lsof_context *ctx);
extern void childx(struct lsof_context *ctx);
extern void closefrom_shim(struct lsof_context *ctx, int low);
extern int ck_fd_status(struct lsof_context *ctx, enum lsof_fd_type fd_type,
                        int num);
extern int ck_file_arg(struct lsof_context *ctx, int i, int ac, char *av[],
                       int fv, int rs, struct stat *sbp,
                       int accept_deleted_file);
extern void ckkv(struct lsof_context *ctx, char *d, char *er, char *ev,
                 char *ea);
extern void clr_devtab(struct lsof_context *ctx);
extern int compdev(COMP_P *a1, COMP_P *a2);
extern int comppid(COMP_P *a1, COMP_P *a2);

#    if defined(WILLDROPGID)
extern void dropgid(struct lsof_context *ctx);
#    endif /* defined(WILLDROPGID) */

extern char *endnm(struct lsof_context *ctx, size_t *sz);
extern int enter_cmd_rx(struct lsof_context *ctx, char *x);
extern void enter_dev_ch(struct lsof_context *ctx, char *m);
extern int enter_dir(struct lsof_context *ctx, char *d, int descend);

#    if defined(HASEOPT)
extern int enter_efsys(struct lsof_context *ctx, char *e, int rdlnk);
#    endif /* defined(HASEOPT) */

extern int enter_fd(struct lsof_context *ctx, char *f);
extern int enter_network_address(struct lsof_context *ctx, char *na);
extern int enter_id(struct lsof_context *ctx, enum IDType ty, char *p);
extern void enter_IPstate(struct lsof_context *ctx, char *ty, char *nm, int nr);
extern void enter_nm(struct lsof_context *ctx, char *m);

#    if defined(HASTCPUDPSTATE)
extern int enter_state_spec(struct lsof_context *ctx, char *ss);
#    endif /* defined(HASTCPUDPSTATE) */

extern int enter_cmd(struct lsof_context *ctx, char *opt, char *s);
extern int enter_uid(struct lsof_context *ctx, char *us);
extern void ent_inaddr(struct lsof_context *ctx, unsigned char *la, int lp,
                       unsigned char *fa, int fp, int af);
extern int examine_lproc(struct lsof_context *ctx);
extern void Exit(struct lsof_context *ctx, enum ExitStatus xv) exiting;
extern void Error(struct lsof_context *ctx) exiting;
extern void find_ch_ino(struct lsof_context *ctx);

#    if defined(HASEPTOPTS)
extern void clear_pinfo(struct lsof_context *ctx);
extern pxinfo_t *find_pepti(struct lsof_context *ctx, int pid, struct lfile *lf,
                            pxinfo_t *pp);
extern void process_pinfo(struct lsof_context *ctx, int f);
extern void clear_psxmqinfo(struct lsof_context *ctx);
extern pxinfo_t *find_psxmqinfo(struct lsof_context *ctx, int pid,
                                struct lfile *lf, pxinfo_t *pp);
extern void process_psxmqinfo(struct lsof_context *ctx, int f);
#        if defined(HASUXSOCKEPT)
extern void clear_uxsinfo(struct lsof_context *ctx);
extern struct uxsin *find_uxsepti(struct lfile *lf);
extern void process_uxsinfo(struct lsof_context *ctx, int f);
#        endif /* defined(HASUXSOCKEPT) */
#        if defined(HASPTYEPT)
extern void clear_ptyinfo(struct lsof_context *ctx);
extern void enter_ptmxi(struct lsof_context *ctx, int mn);
extern pxinfo_t *find_ptyepti(struct lsof_context *ctx, int pid,
                              struct lfile *lf, int m, pxinfo_t *pp);
extern int is_pty_slave(int sm);
extern int is_pty_ptmx(dev_t dev);
extern void process_ptyinfo(struct lsof_context *ctx, int f);
#        endif /* defined(HASPTYEPT) */
extern void clear_evtfdinfo(struct lsof_context *ctx);
extern void enter_evtfdinfo(struct lsof_context *ctx, int id);
extern pxinfo_t *find_evtfdinfo(struct lsof_context *ctx, int pid,
                                struct lfile *lf, pxinfo_t *pp);
extern void process_evtfdinfo(struct lsof_context *ctx, int f);
extern void clear_netsinfo(struct lsof_context *ctx);
extern void process_netsinfo(struct lsof_context *ctx, int f);
#        if defined(HASIPv6)
extern void clear_nets6info(struct lsof_context *ctx);
extern void process_nets6info(struct lsof_context *ctx, int f);
#        endif /* defined(HASIPv6) */
#    endif     /* defined(HASEPTOPTS) */

extern void free_lproc(struct lproc *lp);
extern void gather_proc_info(struct lsof_context *ctx);
extern char *gethostnm(struct lsof_context *ctx, unsigned char *ia, int af);

#    if !defined(GET_MAX_FD)
/*
 * This is not strictly a prototype, but GET_MAX_FD is the name of the
 * function that, in lieu of getdtablesize(), returns the maximum file
 * descriptor plus one (or file descriptor count).  GET_MAX_FD may be
 * defined in the dialect's machine.h.  If it is not, the following
 * selects getdtablesize().
 */

#        define GET_MAX_FD getdtablesize
#    endif /* !defined(GET_MAX_FD) */

extern int hashbyname(char *nm, int mod);
extern void hashSfile(struct lsof_context *ctx);
extern void initialize(struct lsof_context *ctx);
extern int is_cmd_excl(struct lsof_context *ctx, char *cmd, short *pss,
                       short *sf);
extern int is_file_sel(struct lsof_context *ctx, struct lproc *lp,
                       struct lfile *lf);
extern int is_nw_addr(struct lsof_context *ctx, unsigned char *ia, int p,
                      int af);

#    if defined(HASTASKS)
extern int is_proc_excl(struct lsof_context *ctx, int pid, int pgid,
                        UID_ARG uid, short *pss, short *sf, int tid);
#    else  /* !defined(HASTASKS) */
extern int is_proc_excl(struct lsof_context *ctx, int pid, int pgid,
                        UID_ARG uid, short *pss, short *sf);
#    endif /* defined(HASTASKS) */

extern int is_readable(struct lsof_context *ctx, char *path, int msg);
extern int kread(struct lsof_context *ctx, KA_T addr, char *buf, READLEN_T len);
extern void link_lfile(struct lsof_context *ctx);
extern struct l_dev *lkupdev(struct lsof_context *ctx, dev_t *dev, dev_t *rdev,
                             int i, int r);
extern int main(int argc, char *argv[]);
extern int lstatsafely(struct lsof_context *ctx, char *path, struct stat *buf);
extern char *mkstrcpy(char *src, MALLOC_S *rlp);
extern char *mkstrcat(char *s1, int l1, char *s2, int l2, char *s3, int l3,
                      MALLOC_S *clp);
extern int printdevname(struct lsof_context *ctx, dev_t *dev, dev_t *rdev,
                        int f, int nty);
extern void print_file(struct lsof_context *ctx);
extern void print_init(struct lsof_context *ctx);
extern void printname(struct lsof_context *ctx, int nl);
extern char *print_kptr(KA_T kp, char *buf, size_t bufl);
extern int print_proc(struct lsof_context *ctx);
extern void fd_to_string(enum lsof_fd_type fd_type, int fd_num, char *buf);
extern void printrawaddr(struct lsof_context *ctx, struct sockaddr *sa);
extern void print_tcptpi(struct lsof_context *ctx, int nl);
extern char *printuid(struct lsof_context *ctx, UID_ARG uid, int *ty);
extern void printunkaf(struct lsof_context *ctx, int fam, int ty);
extern char access_to_char(enum lsof_file_access_mode access);
extern char lock_to_char(enum lsof_lock_mode access);
extern void file_type_to_string(enum lsof_file_type type,
                                uint32_t unknown_file_type_number, char *buf,
                                size_t buf_len);
extern char *printsockty(int ty);
extern void process_file(struct lsof_context *ctx, KA_T fp);
extern void process_node(struct lsof_context *ctx, KA_T f);
extern char *Readlink(struct lsof_context *ctx, char *arg);
extern void readdev(struct lsof_context *ctx, int skip);
extern struct mounts *readmnt(struct lsof_context *ctx);
extern void rereaddev(struct lsof_context *ctx);
extern char *safepup(unsigned int c, int *cl);
extern int safestrlen(char *sp, int flags);
extern void safestrprtn(char *sp, int len, FILE *fs, int flags);
extern void safestrprt(char *sp, FILE *fs, int flags);
extern int statsafely(struct lsof_context *ctx, char *path, struct stat *buf);
extern void stkdir(struct lsof_context *ctx, char *p);
extern void usage(struct lsof_context *ctx, int err, int fh, int version);
extern int util_strftime(char *fmtr, int fmtl, char *fmt);
extern int vfy_dev(struct lsof_context *ctx, struct l_dev *dp);
extern char *x2dev(char *s, dev_t *d);

#    if defined(HASBLKDEV)
extern void find_bl_ino(struct lsof_context *ctx);
extern struct l_dev *lkupbdev(struct lsof_context *ctx, dev_t *dev, dev_t *rdev,
                              int i, int r);
extern int printbdevname(dev_t *dev, dev_t *rdev, int f);
#    endif /* defined(HASBLKDEV) */

#    if defined(HASCDRNODE)
extern int readcdrnode(struct lsof_context *ctx, KA_T ca, struct cdrnode *c);
#    endif /* defined(HASCDRNODE) */

#    if defined(HASDCACHE)
extern void alloc_dcache(struct lsof_context *ctx);
extern void crc(char *b, int l, unsigned *s);
extern void crdbld(void);
extern int ctrl_dcache(struct lsof_context *ctx, char *p);
extern int dcpath(struct lsof_context *ctx, int rw, int npw);
extern int open_dcache(struct lsof_context *ctx, int m, int r, struct stat *sb);
extern int read_dcache(struct lsof_context *ctx);
extern int wr2DCfd(struct lsof_context *ctx, char *b, unsigned *c);
extern void write_dcache(struct lsof_context *ctx);
#    endif /* defined(HASDCACHE) */

#    if defined(HASFIFONODE)
extern int readfifonode(struct lsof_context *ctx, KA_T fa, struct fifonode *f);
#    endif /* defined(HASFIFONODE) */

#    if defined(HASFSTRUCT)
extern char *print_fflags(struct lsof_context *ctx, long ffg, long pof);
#    endif /* defined(HASFSTRUCT) */

#    if defined(HASGNODE)
extern int readgnode(struct lsof_context *ctx, KA_T ga, struct gnode *g);
#    endif /* defined(HASGNODE) */

#    if defined(HASKQUEUE)
extern void process_kqueue(struct lsof_context *ctx, KA_T ka);
#    endif /* defined(HASKQUEUE) */

#    if defined(HASHSNODE)
extern int readhsnode(struct lsof_context *ctx, KA_T ha, struct hsnode *h);
#    endif /* defined(HASHSNODE) */

#    if defined(HASINODE)
extern int readinode(struct lsof_context *ctx, KA_T ia, struct inode *i);
#    endif /* defined(HASINODE) */

#    if defined(HASNCACHE)
extern void ncache_load(struct lsof_context *ctx);
extern char *ncache_lookup(struct lsof_context *ctx, char *buf, int blen,
                           int *fp);
#    endif /* defined(HASNCACHE) */

#    if defined(HASNLIST)
extern void build_Nl(struct lsof_context *ctx, struct drive_Nl *d);
extern int get_Nl_value(struct lsof_context *ctx, char *nn, struct drive_Nl *d,
                        KA_T *v);
#    endif /* defined(HASNLIST) */

#    if defined(HASPIPENODE)
extern int readpipenode(struct lsof_context *ctx, KA_T pa, struct pipenode *p);
#    endif /* defined(HASPIPENODE) */

#    if defined(HASPRINTDEV)
extern char *HASPRINTDEV(struct lfile *lf, dev_t *dev);
#    endif /* defined(HASPRINTDEV) */

#    if defined(HASPRINTINO)
extern char *HASPRINTINO(struct lfile *lf);
#    endif /* defined(HASPRINTINO) */

#    if defined(HASPRINTNM)
extern void HASPRINTNM(struct lsof_context *ctx, struct lfile *lf);
#    endif /* defined(HASPRINTNM) */

#    if defined(HASPRIVNMCACHE)
extern int HASPRIVNMCACHE(struct lsof_context *ctx, struct lfile *lf);
#    endif /* defined(HASPRIVNMCACHE) */

#    if !defined(HASPRIVPRIPP)
extern void printiproto(struct lsof_context *ctx, int p);
#    endif /* !defined(HASPRIVPRIPP) */

#    if defined(HASRNODE)
extern int readrnode(struct lsof_context *ctx, KA_T ra, struct rnode *r);
#    endif /* defined(HASRNODE) */

#    if defined(HASSPECDEVD)
extern void HASSPECDEVD(struct lsof_context *ctx, char *p, struct stat *s);
#    endif /* defined(HASSPECDEVD) */

#    if defined(HASSNODE)
extern int readsnode(struct lsof_context *ctx, KA_T sa, struct snode *s);
#    endif /* defined(HASSNODE) */

#    if defined(HASSTREAMS)
extern int readstdata(struct lsof_context *ctx, KA_T addr, struct stdata *buf);
extern int readsthead(struct lsof_context *ctx, KA_T addr, struct queue *buf);
extern int readstidnm(struct lsof_context *ctx, KA_T addr, char *buf,
                      READLEN_T len);
extern int readstmin(struct lsof_context *ctx, KA_T addr,
                     struct module_info *buf);
extern int readstqinit(struct lsof_context *ctx, KA_T addr, struct qinit *buf);
#    endif /* defined(HASSTREAMS) */

#    if defined(HASTMPNODE)
extern int readtnode(struct lsof_context *ctx, KA_T ta, struct tmpnode *t);
#    endif /* defined(HASTMPNODE) */

#    if defined(HASVNODE)
extern int readvnode(struct lsof_context *ctx, KA_T va, struct vnode *v);
#    endif /* defined(HASVNODE) */

#endif /* !defined(PROTO_H) */
