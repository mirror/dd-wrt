/***********************license start***************
 * Copyright (c) 2003-2013  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * Header file for the hotplug APIs
 *
 * <hr>$Revision:  $<hr>
 */

#ifndef __CVMX_APP_HOTPLUG_H__
#define __CVMX_APP_HOTPLUG_H__

#ifdef    __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-spinlock.h>
#else
#include "cvmx.h"
#include "cvmx-coremask.h"
#include "cvmx-interrupt.h"
#include "cvmx-bootmem.h"
#include "cvmx-spinlock.h"
#endif

#define CVMX_APP_HOTPLUG_MAX_APPS 48
#define CVMX_APP_HOTPLUG_MAX_APPNAME_LEN 256

/**
* hotplug_start          is the entry  point for hot plugged cores.
* cores_added_callback   is callback which in invoked when new cores are added
*                        to the application. This is invoked on all the old core
*                        that existed before the current set of cores were
*                        added.
* cores_removed_callback is callback which in invoked when cores are removed
*                        an application. This is invoked on  all the cores that
*                        exist after the set of cores being requesed are
*                        removed.
* shutdown_done_callback before the application is shutdown this callback is
*                        invoked on all the cores that are part of the app.
* unplug_callback        before the cores are unplugged this callback is invoked
*                        only on the cores that are being unlpuuged.
*/
typedef struct cvmx_app_hotplug_callbacks {
	void (*hotplug_start) (void *ptr);
	void (*cores_added_callback) (cvmx_coremask_t *, void *ptr);
	void (*cores_removed_callback) (cvmx_coremask_t *, void *ptr);
	void (*shutdown_callback) (void *ptr);
	void (*unplug_core_callback) (void *ptr);
} cvmx_app_hotplug_callbacks_t;

/*
 * The size of this struct should be a fixed size of 1024 bytes.
 * and is endianness-agnostic, it may seem wasteful but all flags
 * occupy a 64-bit field.
 *
 * <coremask> represents all core currently running the application
 * <hotplug_activated_coremask> represents which cores have registered
 * to accept hotplug mailbox interrupts
 * <hplugged_cores> represents which cores are being added to an application.
 * <unplug_cores> represents the core being removed from an application.
 * <shutdown_cores> is set to all cores in <coremask> when the application
 * is being shut down.
 *
 * All callback pointers are in the application virtual address space.
 *
 * <coremask> indicates all rores running the application.
 * <hotplug_activated_coremask> indicates which cores have called
 * cvmx_app_hotplug_activate() and are thus ready to receive events.
 * <hplugged_cores> lists cores being added to an app dynamically.
 * <shutdown_cores> lists all application cores when it is being shut down.
 * <unplug_cores> lists the cores being removed from an application.
 *
 * <app_shutdown> indicates if the application will call cvmx_core_shutdown()
 * or the call needs to be made on its behalf by the hotplug library.
 * <cvmx_app_boot_record_ptr> is a physical address of the primary
 * boot record for an application.
 */
typedef struct cvmx_app_hotplug_info {
	struct cvmx_coremask coremask;
	struct cvmx_coremask hotplug_activated_coremask;
	struct cvmx_coremask hplugged_cores;
	struct cvmx_coremask shutdown_cores;
	struct cvmx_coremask unplug_cores;
	uint64_t shutdown_callback;
	uint64_t unplug_callback;
	uint64_t cores_added_callback;
	uint64_t cores_removed_callback;
	uint64_t data;
	uint64_t app_shutdown;
	uint64_t shutdown_done;
	uint64_t cvmx_app_boot_record_ptr;
	char app_name[CVMX_APP_HOTPLUG_MAX_APPNAME_LEN];
	uint64_t unused[7];
	uint64_t valid_magic;
} cvmx_app_hotplug_info_t;

struct cvmx_app_hotplug_global {
	cvmx_app_hotplug_info_t hotplug_info_array[CVMX_APP_HOTPLUG_MAX_APPS];
	struct cvmx_coremask avail_coremask;
	cvmx_spinlock_t hotplug_global_lock;
	uint64_t app_under_boot;
	uint64_t app_under_shutdown;
	uint64_t reserved[8];
	uint64_t magic_version;
};
typedef struct cvmx_app_hotplug_global cvmx_app_hotplug_global_t;

#define	CVMX_HOTPLUG_MAGIC_VERSION	0xb10ce1abe1000001ULL
#define	CVMX_HOTPLUG_MAGIC_VALID	0xf1a90001f1a90001ULL

int is_core_being_hot_plugged(void);
int is_app_under_boot_or_shutdown(void);
void set_app_under_boot(int val);
void set_app_under_shutdown(int val);
int cvmx_app_hotplug_shutdown_request(const struct cvmx_coremask *, int);
int cvmx_app_hotplug_unplug_cores(int index, const struct cvmx_coremask *pcm,
				  int wait);
cvmx_app_hotplug_info_t *cvmx_app_hotplug_get_info(const struct cvmx_coremask *);
int cvmx_app_hotplug_get_index(const struct cvmx_coremask *pcm);
cvmx_app_hotplug_info_t *cvmx_app_hotplug_get_info_at_index(int index);
int is_app_hotpluggable(int index);
int cvmx_app_hotplug_call_add_cores_callback(int index);
#ifndef CVMX_BUILD_FOR_LINUX_USER
int cvmx_app_hotplug_register(void (*)(void *), void *);
int cvmx_app_hotplug_register_cb(cvmx_app_hotplug_callbacks_t *, void *, int);
int cvmx_app_hotplug_activate(void);
void cvmx_app_hotplug_core_shutdown(void);
void cvmx_app_hotplug_shutdown_disable(void);
void cvmx_app_hotplug_shutdown_enable(void);
#endif

#define CVMX_APP_HOTPLUG_INFO_REGION_SIZE  sizeof(cvmx_app_hotplug_global_t)
#define CVMX_APP_HOTPLUG_INFO_REGION_NAME  "cvmx-app-hotplug-block"

#ifdef  __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_APP_HOTPLUG_H__ */
