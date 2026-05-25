/* ISC license. */

#ifndef SKALIBS_EXEC_H
#define SKALIBS_EXEC_H

#include <stddef.h>

#include <skalibs/posixplz.h>
#include <skalibs/env.h>
#include <skalibs/gccattributes.h>


extern void xexecvep (char const *, char const *const *, char const *const *, char const *) gccattr_noreturn ;
extern void xexecvep_loose (char const *, char const *const *, char const *const *, char const *) gccattr_noreturn ;


 /*
    Direct execution with PATH (calls execvep).
    (was called pathexec_run in earlier skalibs versions)
    a for provided file name (default: argv[0])
    e for provided envp (default: environ)
    foo0 exits 0 if argv[0] is NULL
    xfoo dies if the exec fails
 */

extern void exec_ae (char const *, char const *const *, char const *const *) ;
#define exec_a(file, argv) exec_ae(file, (argv), (char const *const *)environ)
#define exec_e(argv, envp) exec_ae((argv)[0], (argv), envp)
#define exec(argv) exec_a((argv)[0], (argv))

extern void xexec_ae (char const *, char const *const *, char const *const *) gccattr_noreturn ;
#define xexec_a(file, argv) xexec_ae(file, (argv), (char const *const *)environ)
#define xexec_e(argv, envp) xexec_ae((argv)[0], (argv), envp)
#define xexec(argv) xexec_a((argv)[0], (argv))

extern void exec0_ae (char const *, char const *const *, char const *const *) ;
#define exec0_a(file, argv) exec0_ae(file, (argv), (char const *const *)environ)
#define exec0_e(argv, envp) exec0_ae((argv)[0], (argv), envp)
#define exec0(argv) exec0_a((argv)[0], (argv))

extern void xexec0_ae (char const *, char const *const *, char const *const *) gccattr_noreturn ;
#define xexec0_a(file, argv) xexec0_ae(file, (argv), (char const *const *)environ)
#define xexec0_e(argv, envp) xexec0_ae((argv)[0], (argv), envp)
#define xexec0(argv) xexec0_a((argv)[0], (argv))


 /*
    Execution with environment modifications : env_merge and exec.
    (was called pathexec in earlier skalibs versions)
    a for provided file name (default: argv[0])
    e for provided envp (default: environ)
    f for provided envp *and* length of the envp
    m for provided modif string plus its length (the length is always needed because the modifs are null-terminated)
    n for provided modif string, length *and* number of modifs
 */

extern int env_mexec (char const *, char const *) ;

extern void mexec_afn (char const *, char const *const *, char const *const *, size_t, char const *, size_t, size_t) ;
extern void mexec_afm (char const *, char const *const *, char const *const *, size_t, char const *, size_t) ;
extern void mexec_af (char const *, char const *const *, char const *const *, size_t) ;

#define mexec_aen(file, argv, envp, modif, modiflen, modifn) mexec_afn(file, argv, envp, env_len(envp), modif, modiflen, modifn)
#define mexec_aem(file, argv, envp, modif, modiflen) mexec_afm(file, argv, envp, env_len(envp), modif, modiflen)
#define mexec_ae(file, argv, envp) mexec_af(file, argv, (envp), env_len(envp))

#define mexec_an(file, argv, modif, modiflen, modifn) mexec_aen(file, argv, (char const *const *)environ, modif, modiflen, modifn)
#define mexec_am(file, argv, modif, modiflen) mexec_aem(file, argv, (char const *const *)environ, modif, modiflen)
#define mexec_a(file, argv) mexec_ae(file, (argv), (char const *const *)environ)

#define mexec_fn(argv, envp, envlen, modif, modiflen, modifn) mexec_afn((argv)[0], (argv), envp, envlen, modif, modiflen, modifn)
#define mexec_fm(argv, envp, envlen, modif, modiflen) mexec_afm((argv)[0], (argv), envp, envlen, modif, modiflen)
#define mexec_f(argv, envp, envlen) mexec_af((argv)[0], (argv), envp, envlen)

#define mexec_en(argv, envp, modif, modiflen, modifn) mexec_aen((argv)[0], (argv), envp, modif, modiflen, modifn)
#define mexec_em(argv, envp, modif, modiflen) mexec_aem((argv)[0], (argv), envp, modif, modiflen)
#define mexec_e(argv, envp) mexec_ae((argv)[0], (argv), envp)

#define mexec_n(argv, modif, modiflen, modifn) mexec_an((argv)[0], (argv), modif, modiflen, modifn)
#define mexec_m(argv, modif, modiflen) mexec_am((argv)[0], (argv), modif, modiflen)
#define mexec(argv) mexec_a((argv)[0], (argv))

extern void mexec0_afn (char const *, char const *const *, char const *const *, size_t, char const *, size_t, size_t) ;
extern void mexec0_afm (char const *, char const *const *, char const *const *, size_t, char const *, size_t) ;
extern void mexec0_af (char const *, char const *const *, char const *const *, size_t) ;

#define mexec0_aen(file, argv, envp, modif, modiflen, modifn) mexec0_afn(file, argv, envp, env_len(envp), modif, modiflen, modifn)
#define mexec0_aem(file, argv, envp, modif, modiflen) mexec0_afm(file, argv, envp, env_len(envp), modif, modiflen)
#define mexec0_ae(file, argv, envp) mexec0_af(file, argv, (envp), env_len(envp))

#define mexec0_an(file, argv, modif, modiflen, modifn) mexec0_aen(file, argv, (char const *const *)environ, modif, modiflen, modifn)
#define mexec0_am(file, argv, modif, modiflen) mexec0_aem(file, argv, (char const *const *)environ, modif, modiflen)
#define mexec0_a(file, argv) mexec0_ae(file, (argv), (char const *const *)environ)

#define mexec0_fn(argv, envp, envlen, modif, modiflen, modifn) mexec0_afn((argv)[0], (argv), envp, envlen, modif, modiflen, modifn)
#define mexec0_fm(argv, envp, envlen, modif, modiflen) mexec0_afm((argv)[0], (argv), envp, envlen, modif, modiflen)
#define mexec0_f(argv, envp, envlen) mexec0_af((argv)[0], (argv), envp, envlen)

#define mexec0_en(argv, envp, modif, modiflen, modifn) mexec0_aen((argv)[0], (argv), envp, modif, modiflen, modifn)
#define mexec0_em(argv, envp, modif, modiflen) mexec0_aem((argv)[0], (argv), envp, modif, modiflen)
#define mexec0_e(argv, envp) mexec0_ae((argv)[0], (argv), envp)

#define mexec0_n(argv, modif, modiflen, modifn) mexec0_an((argv)[0], (argv), modif, modiflen, modifn)
#define mexec0_m(argv, modif, modiflen) mexec0_am((argv)[0], (argv), modif, modiflen)
#define mexec0(argv) mexec0_a((argv)[0], (argv))

extern void xmexec_afn (char const *, char const *const *, char const *const *, size_t, char const *, size_t, size_t) gccattr_noreturn ;
extern void xmexec_afm (char const *, char const *const *, char const *const *, size_t, char const *, size_t) gccattr_noreturn ;
extern void xmexec_af (char const *, char const *const *, char const *const *, size_t) gccattr_noreturn ;

#define xmexec_aen(file, argv, envp, modif, modiflen, modifn) xmexec_afn(file, argv, envp, env_len(envp), modif, modiflen, modifn)
#define xmexec_aem(file, argv, envp, modif, modiflen) xmexec_afm(file, argv, envp, env_len(envp), modif, modiflen)
#define xmexec_ae(file, argv, envp) xmexec_af(file, argv, (envp), env_len(envp))

#define xmexec_an(file, argv, modif, modiflen, modifn) xmexec_aen(file, argv, (char const *const *)environ, modif, modiflen, modifn)
#define xmexec_am(file, argv, modif, modiflen) xmexec_aem(file, argv, (char const *const *)environ, modif, modiflen)
#define xmexec_a(file, argv) xmexec_ae(file, (argv), (char const *const *)environ)

#define xmexec_fn(argv, envp, envlen, modif, modiflen, modifn) xmexec_afn((argv)[0], (argv), envp, envlen, modif, modiflen, modifn)
#define xmexec_fm(argv, envp, envlen, modif, modiflen) xmexec_afm((argv)[0], (argv), envp, envlen, modif, modiflen)
#define xmexec_f(argv, envp, envlen) xmexec_af((argv)[0], (argv), envp, envlen)

#define xmexec_en(argv, envp, modif, modiflen, modifn) xmexec_aen((argv)[0], (argv), envp, modif, modiflen, modifn)
#define xmexec_em(argv, envp, modif, modiflen) xmexec_aem((argv)[0], (argv), envp, modif, modiflen)
#define xmexec_e(argv, envp) xmexec_ae((argv)[0], (argv), envp)

#define xmexec_n(argv, modif, modiflen, modifn) xmexec_an((argv)[0], (argv), modif, modiflen, modifn)
#define xmexec_m(argv, modif, modiflen) xmexec_am((argv)[0], (argv), modif, modiflen)
#define xmexec(argv) xmexec_a((argv)[0], (argv))

extern void xmexec0_afn (char const *, char const *const *, char const *const *, size_t, char const *, size_t, size_t) gccattr_noreturn ;
extern void xmexec0_afm (char const *, char const *const *, char const *const *, size_t, char const *, size_t) gccattr_noreturn ;
extern void xmexec0_af (char const *, char const *const *, char const *const *, size_t) gccattr_noreturn ;

#define xmexec0_aen(file, argv, envp, modif, modiflen, modifn) xmexec0_afn(file, argv, envp, env_len(envp), modif, modiflen, modifn)
#define xmexec0_aem(file, argv, envp, modif, modiflen) xmexec0_afm(file, argv, envp, env_len(envp), modif, modiflen)
#define xmexec0_ae(file, argv, envp) xmexec0_af(file, argv, (envp), env_len(envp))

#define xmexec0_an(file, argv, modif, modiflen, modifn) xmexec0_aen(file, argv, (char const *const *)environ, modif, modiflen, modifn)
#define xmexec0_am(file, argv, modif, modiflen) xmexec0_aem(file, argv, (char const *const *)environ, modif, modiflen)
#define xmexec0_a(file, argv) xmexec0_ae(file, (argv), (char const *const *)environ)

#define xmexec0_fn(argv, envp, envlen, modif, modiflen, modifn) xmexec0_afn((argv)[0], (argv), envp, envlen, modif, modiflen, modifn)
#define xmexec0_fm(argv, envp, envlen, modif, modiflen) xmexec0_afm((argv)[0], (argv), envp, envlen, modif, modiflen)
#define xmexec0_f(argv, envp, envlen) xmexec0_af((argv)[0], (argv), envp, envlen)

#define xmexec0_en(argv, envp, modif, modiflen, modifn) xmexec0_aen((argv)[0], (argv), envp, modif, modiflen, modifn)
#define xmexec0_em(argv, envp, modif, modiflen) xmexec0_aem((argv)[0], (argv), envp, modif, modiflen)
#define xmexec0_e(argv, envp) xmexec0_ae((argv)[0], (argv), envp)

#define xmexec0_n(argv, modif, modiflen, modifn) xmexec0_an((argv)[0], (argv), modif, modiflen, modifn)
#define xmexec0_m(argv, modif, modiflen) xmexec0_am((argv)[0], (argv), modif, modiflen)
#define xmexec0(argv) xmexec0_a((argv)[0], (argv))

#endif
