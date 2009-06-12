/*
 * Manuel Novoa III           Feb 2001
 * Erik Andersen              Mar 2002
 *
 * __uClibc_main is the routine to be called by all the arch-specific
 * versions of crt0.S in uClibc.
 *
 * It is meant to handle any special initialization needed by the library
 * such as setting the global variable(s) __environ (environ) and
 * initializing the stdio package.  Using weak symbols, the latter is
 * avoided in the static library case.
 */

#define	_ERRNO_H
#include <features.h>
#include <unistd.h>
#include <stdlib.h>
#ifdef __UCLIBC_PROPOLICE__
extern void __guard_setup(void);
#endif


/*
 * Prototypes.
 */
extern int  main(int argc, char **argv, char **envp);
extern void weak_function _stdio_init(void);
extern int *weak_const_function __errno_location(void);
extern int *weak_const_function __h_errno_location(void);
#ifdef __UCLIBC_HAS_LOCALE__
extern void weak_function _locale_init(void);
#endif
#ifdef __UCLIBC_HAS_THREADS__
extern void weak_function __pthread_initialize_minimal(void);
#endif





/*
 * Declare the __environ global variable and create a weak alias environ.
 * Note: Apparently we must initialize __environ to ensure that the weak 
 * environ symbol is also included.
 */

char **__environ = 0;
const char *__progname = 0;
weak_alias(__environ, environ);



/* __uClibc_init completely initialize uClibc so it is ready to use.
 *
 * On ELF systems (with a dynamic loader) this function must be called
 * from the dynamic loader (see TIS and ELF Specification), so that
 * constructors of shared libraries (which depend on libc) can use all
 * the libc code without restriction.  For this we link the shared
 * version of the uClibc with -init __uClibc_init so DT_INIT for
 * uClibc is the address of __uClibc_init
 * 
 * In all other cases we call it from the main stub
 * __uClibc_start_main.
 */

void __uClibc_init(void) 
{
    static int been_there_done_that = 0;

    if (been_there_done_that) 
	return;
    been_there_done_that++;

#ifdef __UCLIBC_HAS_THREADS__
    /* Before we start initialzing uClibc we have to call
     * __pthread_initialize_minimal so we can use pthread_locks 
     * whenever they are needed.
     */
    if (likely(__pthread_initialize_minimal!=NULL))
	__pthread_initialize_minimal();
#endif

#if 0
    /* Some security at this point.  Prevent starting a SUID binary
     * where the standard file descriptors are not opened.  We have
     * to do this only for statically linked applications since
     * otherwise the dynamic loader did the work already.  */
    if (unlikely (__libc_enable_secure!=NULL))
	__libc_check_standard_fds ();
#endif

#ifdef __UCLIBC_HAS_LOCALE__
    /* Initialize the global locale structure. */
    if (likely(_locale_init!=NULL))
	_locale_init();
#endif

    /*
     * Initialize stdio here.  In the static library case, this will
     * be bypassed if not needed because of the weak alias above.
     */
    if (likely(_stdio_init != NULL))
	_stdio_init();

}

#ifdef __UCLIBC_CTOR_DTOR__
void (*__app_fini)(void) = NULL;
#endif

/* __uClibc_start_main is the new main stub for uClibc. This function is 
 * called from crt0 (version 0.9.16 or newer), after ALL shared libraries 
 * are initialized, just before we call the application's main function.
 */
void __attribute__ ((__noreturn__)) 
__uClibc_start_main(int argc, char **argv, char **envp, 
	void (*app_init)(void), void (*app_fini)(void))
{

    /* If we are dynamically linked the shared lib loader already
     * did this for us.  But if we are statically linked, we need
     * to do this for ourselves.  */
    if (__environ==NULL) {
	/* Statically linked. */ 
	__environ = envp;
    }

    /* We need to initialize uClibc.  If we are dynamically linked this
     * may have already been completed by the shared lib loader.  We call 
     * __uClibc_init() regardless, to be sure the right thing happens. */
    __uClibc_init();

    __progname = *argv;

#ifdef __UCLIBC_CTOR_DTOR__
    /* Arrange for the application's dtors to run before we exit.  */
	__app_fini = app_fini;

    /* Run all the application's ctors now.  */
    if (app_init!=NULL) {
	app_init();
    }
#endif

#ifdef __UCLIBC_PROPOLICE__
     __guard_setup ();
#endif

    /* Note: It is possible that any initialization done above could
     * have resulted in errno being set nonzero, so set it to 0 before
     * we call main.
     */
    if (likely(__errno_location!=NULL))
	*(__errno_location()) = 0;

    /* Set h_errno to 0 as well */
    if (likely(__h_errno_location!=NULL))
	*(__h_errno_location()) = 0;

    /*
     * Finally, invoke application's main and then exit.
     */
    exit(main(argc, argv, envp));
}


/* __uClibc_main is the old main stub of the uClibc. This
 * function is called from crt0 (uClibc 0.9.15 and older) after
 * ALL shared libraries are initialized, and just before we call
 * the application's main() function.  
 *
 * Attention: This stub does not call the .init/.fini sections of
 * the application. If you need this, please fix your uClibc port
 * so that  __uClibc_start_main is called by your crt0.S with
 * _init and _fini properly set.
*/
void __attribute__ ((__noreturn__)) 
__uClibc_main(int argc, char **argv, char ** envp)
{
    __uClibc_start_main(argc, argv, envp, NULL, NULL);
}

