/* ISC license. */

#ifndef SKALIBS_DJBUNIX_H
#define SKALIBS_DJBUNIX_H

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include <skalibs/gccattributes.h>
#include <skalibs/stralloc.h>
#include <skalibs/devino.h>
#include <skalibs/fcntl.h>

extern int coe (int) ;
extern int uncoe (int) ;
extern int ndelay_on (int) ;
extern int ndelay_off (int) ;
extern int pipe_internal (int *, unsigned int) ;
#define pipenb(p) pipe_internal(p, O_NONBLOCK)
#define pipecoe(p) pipe_internal(p, O_CLOEXEC)
#define pipenbcoe(p) pipe_internal(p, O_NONBLOCK|O_CLOEXEC)
extern int fd_copy (int, int) ;
extern int fd_copy2 (int, int, int, int) ;
extern int fd_move (int, int) ;
extern int fd_move2 (int, int, int, int) ;
extern void fd_close (int) ;
extern int fd_chmod (int, unsigned int) ;
extern int fd_chown (int, uid_t, gid_t) ;
extern int fd_sync (int) ;
extern off_t fd_cat (int, int) ;
extern off_t fd_catn (int, int, off_t) ;
extern int fd_ensure_open (int, int) ;
extern int fd_sanitize (void) ;
extern void fd_shutdown (int, int) ;

extern int fd_lock (int, int, int) ;
extern void fd_unlock (int) ;
extern int fd_islocked (int) ;

extern int open2 (char const *, unsigned int) ;
extern int open3 (char const *, unsigned int, unsigned int) ;
extern int open_read (char const *) ;
extern int openc_read (char const *) ;
extern int openb_read (char const *) ;
extern int openbc_read (char const *) ;
extern int open_readb (char const *) ;
extern int openc_readb (char const *) ;
extern int open_excl (char const *) ;
extern int openc_excl (char const *) ;
extern int open_append (char const *) ;
extern int openc_append (char const *) ;
extern int open_create (char const *) ;
extern int openc_create (char const *) ;
extern int open_trunc (char const *) ;
extern int openc_trunc (char const *) ;
extern int open_write (char const *) ;
extern int openc_write (char const *) ;

extern size_t path_canonicalize (char *, char const *, int) ;

extern pid_t wait_nointr (int *) ;
extern pid_t waitpid_nointr (pid_t, int *, int) ;
#define wait_pid(pid, wstat) waitpid_nointr(pid, (wstat), 0)
#define wait_nohang(wstat) waitpid_nointr(-1, (wstat), WNOHANG)
extern pid_t wait_pid_nohang (pid_t, int *) ;
extern int wait_pids_nohang (pid_t const *, unsigned int, int *) ;
#define wait_status(w) (WIFSIGNALED(w) ? 256 + WTERMSIG(w) : WEXITSTATUS(w))
#define wait_estatus(w) (WIFSIGNALED(w) ? 128 + WTERMSIG(w) : WEXITSTATUS(w) >= 128 ? 128 : WEXITSTATUS(w))

extern unsigned int wait_reap (void) ;
extern int waitn (pid_t *, unsigned int) ;
extern int waitn_posix (pid_t *, unsigned int, int *) ;
extern int waitn_reap (pid_t *, unsigned int) ;
extern int waitn_reap_posix (pid_t *, unsigned int, int *) ;

extern int fd_chdir (int) ;

extern int sarealpath (stralloc *, char const *) ;
extern int sabasename (stralloc *, char const *, size_t) ;
extern int sadirname (stralloc *, char const *, size_t) ;
extern int sagetcwd (stralloc *) ;
extern int sareadlink (stralloc *, char const *) ;
extern int sagethostname (stralloc *) ;
extern int sagetexecname (stralloc *) ;

#define slurp(sa, fd) slurpn((fd), (sa), 0)
#define openslurpclose(sa, fn) openslurpnclose((fn), (sa), 0)

extern int slurpn (int, stralloc *, size_t) ;
extern int openslurpnclose (char const *, stralloc *, size_t) ;
extern ssize_t readnclose (int fd, char *, size_t) ;  /* closes fd */
extern ssize_t openreadnclose (char const *, char *, size_t) ;
extern ssize_t openreadnclose_nb (char const *, char *, size_t) ;
extern int openreadfileclose (char const *, stralloc *, size_t) ;

#define writenclose_unsafe(fd, s, n) writenclose_unsafe5(fd, s, (n), 0, 0)
#define writenclose_unsafe_sync(fd, s, n) writenclose_unsafe5(fd, s, (n), 0, 1)
extern int writenclose_unsafe5 (int, char const *, size_t, devino *, unsigned int) ;

#define openwritenclose_unsafe(f, s, n) openwritenclose_unsafe5(f, s, (n), 0, 0)
#define openwritenclose_unsafe_sync(f, s, n) openwritenclose_unsafe5(f, s, (n), 0, 1)
extern int openwritenclose_unsafe5 (char const *, char const *, size_t, devino *, unsigned int) ;

#define openwritenclose_suffix(f, s, n, t) openwritenclose_suffix6(f, s, n, 0, 0, t)
#define openwritenclose_suffix_sync(f, s, n, t) openwritenclose_suffix6(f, s, n, 0, 1, t)
extern int openwritenclose_suffix6 (char const *, char const *, size_t, devino *, unsigned int, char const *) ;

#define openwritenclose(f, s, n) openwritenclose5(f, s, (n), 0, 0)
#define openwritenclose_sync(f, s, n) openwritenclose5(f, s, (n), 0, 1)
extern int openwritenclose5 (char const *, char const *, size_t, devino *, unsigned int) ;

#define writenvclose_unsafe(fd, v, n) writevnclose_unsafe5(fd, v, (n), 0, 0)
#define writevnclose_unsafe_sync(fd, v, n) writevnclose_unsafe5(fd, v, (n), 0, 1)
extern int writevnclose_unsafe5 (int, struct iovec const *, unsigned int, devino *, unsigned int) ;

#define openwritevnclose_unsafe(f, v, n) openwritevnclose_unsafe5(f, v, (n), 0, 0)
#define openwritevnclose_unsafe_sync(f, v, n) openwritevnclose_unsafe5(f, v, (n), 0, 1)
extern int openwritevnclose_unsafe5 (char const *, struct iovec const *, unsigned int, devino *, unsigned int) ;

#define openwritevnclose_suffix(f, v, n, t) openwritevnclose_suffix6(f, v, n, 0, 0, t)
#define openwritevnclose_suffix_sync(f, v, n, t) openwritevnclose_suffix6(f, v, n, 0, 1, t)
extern int openwritevnclose_suffix6 (char const *, struct iovec const *, unsigned int, devino *, unsigned int, char const *) ;

#define openwritevnclose(f, v, n) openwritevnclose5(f, v, (n), 0, 0)
#define openwritevnclose_sync(f, v, n) openwritevnclose5(f, v, (n), 0, 1)
extern int openwritevnclose5 (char const *, struct iovec const *, unsigned int, devino *, unsigned int) ;

extern int rm_rf (char const *) ;
extern int rm_rf_tmp (char const *, stralloc *) ;
extern int rm_rf_in_tmp (stralloc *, size_t) ; /* caution ! */
extern int rmstar (char const *) ;
extern int rmstar_tmp (char const *, stralloc *) ;

extern int filecopy_unsafe (char const *, char const *, unsigned int) ;
extern int filecopy_suffix (char const *, char const *, unsigned int, char const *) ;
extern int sals (char const *, stralloc *, size_t *) ;
extern int hiercopy (char const *, char const *) ;
extern int hiercopy_tmp (char const *, char const *, stralloc *) ;
extern int hiercopy_loose (char const *, char const *) ;
extern int hiercopy_loose_tmp (char const *, char const *, stralloc *) ;
extern int hiercopy_internal (char const *, char const *, stralloc *, unsigned int) ;

#endif
