/* 
 * Written 1999-03-19 by Jonathan Larmour, Cygnus Solutions
 * Modifed for touchscreen testing by Richard Panton 13-09-00
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
#ifdef CYGPKG_FS_ROM
#define USE_ROMDISK
#endif
#ifdef CYGPKG_FS_JFFS2
# include <pkgconf/io_flash.h>
#define USE_JFFS2
#undef  USE_ROMDISK
#endif
#include <pkgconf/microwindows.h>

#ifndef CYGFUN_KERNEL_API_C
# error Kernel API must be enabled to build this application
#endif

#ifndef CYGPKG_LIBC_STDIO
# error C library standard I/O must be enabled to build this application
#endif

#ifndef CYGPKG_IO_SERIAL_HALDIAG
# error I/O HALDIAG pseudo-device driver must be enabled to build this application
#endif

/* INCLUDES */

#include <stdio.h>                      /* printf */
#include <stdlib.h>                      /* printf */
#include <string.h>                     /* strlen */
#include <ctype.h>                      /* tolower */
#include <cyg/kernel/kapi.h>            /* All the kernel specific stuff */
#include <cyg/hal/hal_arch.h>           /* CYGNUM_HAL_STACK_SIZE_TYPICAL */
#include <sys/time.h>
#include <network.h>                    /* init_all_network_interfaces() */
#include <microwin/ecos_mw_app.h>

// Define table boundaries
CYG_HAL_TABLE_BEGIN( __MW_APP_TAB__, _mw_apps );
CYG_HAL_TABLE_END( __MW_APP_TAB_END__, _mw_apps );
extern struct _mw_app_entry __MW_APP_TAB__[], __MW_APP_TAB_END__;

static char startup_stack[STACKSIZE];
cyg_handle_t startup_thread;
cyg_thread   startup_thread_obj;


int 
gettimeofday(struct timeval *tv,
             struct timezone *tz)
{
    tv->tv_usec = 0;
    tv->tv_sec = time(NULL);
    return(0);
}

int
strcasecmp(const char *s1, const char *s2)
{
    char c1, c2;

    while ((c1 = tolower(*s1++)) == (c2 = tolower(*s2++)))
        if (c1 == 0)
            return (0);
    return ((unsigned char)c1 - (unsigned char)c2);
}

static void 
startup(CYG_ADDRESS data)
{
    cyg_ucount32 nanox_data_index;
    struct _mw_app_entry *nx;

    printf("SYSTEM INITIALIZATION in progress\n");
    printf("NETWORK:\n");
    init_all_network_interfaces();

#ifdef USE_ROMDISK
    {
        char ROM_fs[32];
        int res;

        printf("Mount ROM file system\n");
#if (defined CYGPKG_HAL_ARM_SA11X0_IPAQ) && (!defined CYGBLD_MICROWINDOWS_VNC_DRIVERS)
        // Work around hardware anomaly which causes major screen flicker
        {
            char *hold_rom_fs;
            if ((hold_rom_fs = malloc(0x80080)) != 0) {
                // Note: ROM fs requires 32 byte alignment
                hold_rom_fs = (char *)(((unsigned long)hold_rom_fs + 31) & ~31);
                memcpy(hold_rom_fs, 0x50F00000, 0x80000);
                sprintf(ROM_fs, "0x%08x", hold_rom_fs);
            } else {
                printf("Can't allocate memory to hold ROM fs!\n");
            }
        }
#else
        sprintf(ROM_fs, "0x%08x", 0x50F00000);
        sprintf(ROM_fs, "0x%08x", 0x61F00000);
#endif
        printf("ROM fs at %s\n", ROM_fs);
        if ((res = mount(ROM_fs, "/", "romfs")) < 0) {
            printf("... failed\n");
        }
        chdir("/");
    }
#endif

#ifdef USE_JFFS2
    {
        int res;
        printf("... Mounting JFFS2 on \"/\"\n");
        res = mount( CYGDAT_IO_FLASH_BLOCK_DEVICE_NAME_1, "/", "jffs2" );
        if (res < 0) {
            printf("Mount \"/\" failed - res: %d\n", res);
        }
        chdir("/");
    }   
#endif

    // Allocate a free thread data slot
    // Note: all MicroWindows/NanoX threads use this slot for NanoX-private
    // data.  That's why there is only one call here.
    nanox_data_index = cyg_thread_new_data_index();
    printf("data index = %d\n", nanox_data_index);

    printf("Creating system threads\n");
    for (nx = __MW_APP_TAB__; nx != &__MW_APP_TAB_END__;  nx++) {
        printf("Creating %s thread\n", nx->name);
        cyg_thread_create(nx->prio,
                          nx->entry,
                          (cyg_addrword_t) nanox_data_index,
                          nx->name,
                          (void *)nx->stack, STACKSIZE,
                          &nx->t,
                          &nx->t_obj);
    }
    printf("Starting threads\n");
    for (nx = __MW_APP_TAB__; nx != &__MW_APP_TAB_END__;  nx++) {
        printf("Starting %s\n", nx->name);
        cyg_thread_resume(nx->t);
        if (nx->init) {
            (nx->init)(nanox_data_index);
        }
    }

    printf("SYSTEM THREADS STARTED!\n");
}

void cyg_user_start(void)
{
    // Create the initial thread and start it up
    cyg_thread_create(ECOS_MW_STARTUP_PRIORITY,
                      startup,
                      (cyg_addrword_t) 0,
                      "System startup",
                      (void *)startup_stack, STACKSIZE,
                      &startup_thread,
                      &startup_thread_obj);
    cyg_thread_resume(startup_thread);
}
