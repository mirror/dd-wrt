/* ISC license. */

#ifndef SKALIBS_CSPAWN_H
#define SKALIBS_CSPAWN_H

#include <sys/types.h>
#include <stdint.h>

#define CSPAWN_FLAGS_SELFPIPE_FINISH 0x0001U
#define CSPAWN_FLAGS_SIGBLOCKNONE 0x0002U
#define CSPAWN_FLAGS_SETSID 0x0004U
#define CSPAWN_FLAGS_NEWPIDNS 0x8000U

enum cspawn_fileaction_type_e
{
  CSPAWN_FA_CLOSE,
  CSPAWN_FA_COPY,
  CSPAWN_FA_MOVE,
  CSPAWN_FA_OPEN,
  CSPAWN_FA_CHDIR,
  CSPAWN_FA_FCHDIR
} ;

struct cspawn_fa_openinfo_s
{
  int fd ;
  char const *file ;
  int oflag ;
  mode_t mode ;
} ;

union cspawn_fileaction_u
{
  int fd ;
  int fd2[2] ;
  char const *path ;
  struct cspawn_fa_openinfo_s openinfo ;
} ;

typedef struct cspawn_fileaction_s cspawn_fileaction, *cspawn_fileaction_ref ;
struct cspawn_fileaction_s
{
  enum cspawn_fileaction_type_e type ;
  union cspawn_fileaction_u x ;
} ;


 /* Generic interface for posix_spawn() with a fork()+execve() fallback */

extern pid_t cspawn (char const *, char const *const *, char const *const *, uint16_t, cspawn_fileaction const *, size_t) ;


 /* Simple spawn functions with 0 or 1 communicating fds. */

extern pid_t child_spawn0 (char const *, char const *const *, char const *const *) ;
extern pid_t child_spawn1_pipe (char const *, char const *const *, char const *const *, int *, int) ;
extern pid_t child_spawn1_socket (char const *, char const *const *, char const *const *, int *) ;


 /*
    Spawn function with 2 communicating pipes. The int * points to 2 fds.
    Input: fds[0] and fds[1] are the fds to move the pipes to in the child.
    Output: fds[0] and fds[1] contain the pipes to the child.
 */

extern pid_t child_spawn2 (char const *, char const *const *, char const *const *, int *) ;


 /*
    Same, with an additional pipe from the child to the parent.
    The int * points to 3 fds.
    The additional fd# is available to the child in the defined env variable.
 */

#define SKALIBS_CHILD_SPAWN_FDS_ENVVAR "SKALIBS_CHILD_SPAWN_FDS"

extern pid_t child_spawn3 (char const *, char const *const *, char const *const *, int *) ;


 /*
    Generalization of the previous functions.
    * requests n (the last arg) communication fds between parent and child. Uses pipes.
    * if n=1, equivalent to child_spawn1_pipe; child writes, parent reads.
    * if n>=2, parent reads on even and writes on odd.
 */

extern pid_t child_spawn (char const *, char const *const *, char const *const *, int *, size_t) ;


 /* cspawn, but running as a grandchild. Uses one fork(). */

extern pid_t gcspawn (char const *, char const *const *, char const *const *, uint16_t, cspawn_fileaction const *, size_t) ;


 /* mexec (see skalibs/exec.h), but with cspawn instead */

extern int env_mspawn (char const *, char const *) ;

extern pid_t mspawn_afn (char const *, char const *const *, char const *const *, size_t, char const *, size_t, size_t, uint16_t, cspawn_fileaction const *, size_t) ;
extern pid_t mspawn_afm (char const *, char const *const *, char const *const *, size_t, char const *, size_t, uint16_t, cspawn_fileaction const *, size_t) ;
extern pid_t mspawn_af (char const *, char const *const *, char const *const *, size_t, uint16_t, cspawn_fileaction const *, size_t) ;

#define mspawn_aen(file, argv, envp, modif, modiflen, modifn, flags, fa, n) mspawn_afn(file, argv, envp, env_len(envp), modif, modiflen, modifn, flags, fa, n)
#define mspawn_aem(file, argv, envp, modif, modiflen, flags, fa, n) mspawn_afm(file, argv, envp, env_len(envp), modif, modiflen, flags, fa, n)
#define mspawn_ae(file, argv, envp, flags, fa, n) mspawn_af(file, argv, (envp), env_len(envp), flags, fa, n)

#define mspawn_an(file, argv, modif, modiflen, modifn, flags, fa, n) mspawn_aen(file, argv, (char const *const *)environ, modif, modiflen, modifn, flags, fa, n)
#define mspawn_am(file, argv, modif, modiflen, flags, fa, n) mspawn_aem(file, argv, (char const *const *)environ, modif, modiflen, flags, fa, n)
#define mspawn_a(file, argv, flags, fa, n) mspawn_ae(file, (argv), (char const *const *)environ, flags, fa, n)

#define mspawn_fn(argv, envp, envlen, modif, modiflen, modifn, flags, fa, n) mspawn_afn((argv)[0], (argv), envp, envlen, modif, modiflen, modifn, flags, fa, n)
#define mspawn_fm(argv, envp, envlen, modif, modiflen, flags, fa, n) mspawn_afm((argv)[0], (argv), envp, envlen, modif, modiflen, flags, fa, n)
#define mspawn_f(argv, envp, envlen, flags, fa, n) mspawn_af((argv)[0], (argv), envp, envlen, flags, fa, n)

#define mspawn_en(argv, envp, modif, modiflen, modifn, flags, fa, n) mspawn_aen((argv)[0], (argv), envp, modif, modiflen, modifn, flags, fa, n)
#define mspawn_em(argv, envp, modif, modiflen, flags, fa, n) mspawn_aem((argv)[0], (argv), envp, modif, modiflen, flags, fa, n)
#define mspawn_e(argv, envp, flags, fa, n) mspawn_ae((argv)[0], (argv), envp, flags, fa, n)

#define mspawn_n(argv, modif, modiflen, modifn, flags, fa, n) mspawn_an((argv)[0], (argv), modif, modiflen, modifn, flags, fa, n)
#define mspawn_m(argv, modif, modiflen, flags, fa, n) mspawn_am((argv)[0], (argv), modif, modiflen, flags, fa, n)
#define mspawn(argv, flags, fa, n) mspawn_a((argv)[0], (argv), flags, fa, n)

extern pid_t xmspawn_afn (char const *, char const *const *, char const *const *, size_t, char const *, size_t, size_t, uint16_t, cspawn_fileaction const *, size_t) ;
extern pid_t xmspawn_afm (char const *, char const *const *, char const *const *, size_t, char const *, size_t, uint16_t, cspawn_fileaction const *, size_t) ;
extern pid_t xmspawn_af (char const *, char const *const *, char const *const *, size_t, uint16_t, cspawn_fileaction const *, size_t) ;

#define xmspawn_aen(file, argv, envp, modif, modiflen, modifn, flags, fa, n) xmspawn_afn(file, argv, envp, env_len(envp), modif, modiflen, modifn, flags, fa, n)
#define xmspawn_aem(file, argv, envp, modif, modiflen, flags, fa, n) xmspawn_afm(file, argv, envp, env_len(envp), modif, modiflen, flags, fa, n)
#define xmspawn_ae(file, argv, envp, flags, fa, n) xmspawn_af(file, argv, envp, env_len(envp), flags, fa, n)

#define xmspawn_an(file, argv, modif, modiflen, modifn, flags, fa, n) xmspawn_aen(file, argv, (char const *const *)environ, modif, modiflen, modifn, flags, fa, n)
#define xmspawn_am(file, argv, modif, modiflen, flags, fa, n) xmspawn_aem(file, argv, (char const *const *)environ, modif, modiflen, flags, fa, n)
#define xmspawn_a(file, argv, flags, fa, n) xmspawn_ae(file, argv, (char const *const *)environ, flags, fa, n)

#define xmspawn_fn(argv, envp, envlen, modif, modiflen, modifn, flags, fa, n) xmspawn_afn((argv)[0], (argv), envp, envlen, modif, modiflen, modifn, flags, fa, n)
#define xmspawn_fm(argv, envp, envlen, modif, modiflen, flags, fa, n) xmspawn_afm((argv)[0], (argv), envp, envlen, modif, modiflen, flags, fa, n)
#define xmspawn_f(argv, envp, envlen, flags, fa, n) xmspawn_af((argv)[0], (argv), envp, envlen, flags, fa, n)

#define xmspawn_en(argv, envp, modif, modiflen, modifn, flags, fa, n) xmspawn_aen((argv)[0], (argv), envp, modif, modiflen, modifn, flags, fa, n)
#define xmspawn_em(argv, envp, modif, modiflen, flags, fa, n) xmspawn_aem((argv)[0], (argv), envp, modif, modiflen, flags, fa, n)
#define xmspawn_e(argv, envp, flags, fa, n) xmspawn_ae((argv)[0], (argv), envp, flags, fa, n)

#define xmspawn_n(argv, modif, modiflen, modifn, flags, fa, n) xmspawn_an((argv)[0], (argv), modif, modiflen, modifn, flags, fa, n)
#define xmspawn_m(argv, modif, modiflen, flags, fa, n) xmspawn_am((argv)[0], (argv), modif, modiflen, flags, fa, n)
#define xmspawn(argv, flags, fa, n) xmspawn_a((argv)[0], (argv), flags, fa, n)

extern pid_t gmspawn_afn (char const *, char const *const *, char const *const *, size_t, char const *, size_t, size_t, uint16_t, cspawn_fileaction const *, size_t) ;

#endif
