/* 
 * Written 1999-03-19 by Jonathan Larmour, Cygnus Solutions
 * This file is in the public domain and may be used for any purpose
 */

/* CONFIGURATION CHECKS */

#include <pkgconf/system.h>     /* which packages are enabled/disabled */
#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
#endif
#ifdef CYGPKG_LIBC
# include <pkgconf/libc.h>
#endif
#ifdef CYGPKG_IO_SERIAL
# include <pkgconf/io_serial.h>
#endif

#ifndef CYGFUN_KERNEL_API_C
# error Kernel API must be enabled to build this example
#endif

#ifndef CYGPKG_LIBC_STDIO
# error C library standard I/O must be enabled to build this example
#endif

#ifndef CYGPKG_IO_SERIAL_HALDIAG
# error I/O HALDIAG pseudo-device driver must be enabled to build this example
#endif

/* INCLUDES */

#include <stdio.h>                      /* printf */
#include <string.h>                     /* strlen */
#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */
#include <cyg/io/io.h>                  /* I/O functions */
#include <cyg/hal/hal_arch.h>           /* CYGNUM_HAL_STACK_SIZE_TYPICAL */



/* DEFINES */

#define NTHREADS 1
#define STACKSIZE ( CYGNUM_HAL_STACK_SIZE_TYPICAL + 4096 )

/* STATICS */

static cyg_handle_t thread[NTHREADS];
static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

/* FUNCTIONS */

static void simple_prog(CYG_ADDRESS data)
{
    cyg_io_handle_t handle;
    Cyg_ErrNo err;
    const char test_string[] = "serial example is working correctly!\n";
    cyg_uint32 len = strlen(test_string);

    printf("Starting serial example\n");

    err = cyg_io_lookup( "/dev/haldiag", &handle );

    if (ENOERR == err) {
        printf("Found /dev/haldiag. Writing string....\n");
        err = cyg_io_write( handle, test_string, &len );
    }
        
    if (ENOERR == err) {
        printf("I think I wrote the string. Did you see it?\n");
    }
    
    printf("Serial example finished\n");

}

void cyg_user_start(void)
{
    cyg_thread_create(4, simple_prog, (cyg_addrword_t) 0, "serial",
                      (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);
}
