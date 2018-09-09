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
 * Provides APIs for applications to register for hotplug. It also provides
 * APIs for requesting shutdown of a running target application.
 *
 * <hr>$Revision: $<hr>
 */

#include "cvmx-app-hotplug.h"
#include "cvmx-spinlock.h"
#include "cvmx-mbox.h"
#include "cvmx-debug.h"

//#define DEBUG 1

static cvmx_app_hotplug_global_t *hotplug_global_ptr = 0;
static bool cvmx_app_hotplug_disabled = false;

#ifdef CVMX_BUILD_FOR_LINUX_USER
#include <unistd.h>
#endif

#ifndef CVMX_BUILD_FOR_LINUX_USER

static CVMX_SHARED cvmx_spinlock_t cvmx_app_hotplug_sync_lock = { CVMX_SPINLOCK_UNLOCKED_VAL };
static CVMX_SHARED cvmx_spinlock_t cvmx_app_hotplug_lock = { CVMX_SPINLOCK_UNLOCKED_VAL };

static CVMX_SHARED cvmx_app_hotplug_info_t *cvmx_app_hotplug_info_ptr = NULL;

static void __cvmx_app_hotplug_sync(void);
static void __cvmx_app_hotplug_reset(void);

static void __cvmx_app_hotplug_shutdown(
	struct cvmx_mbox *self, uint64_t *registers);
static void __cvmx_app_hotplug_addcores(
	struct cvmx_mbox *self, uint64_t *registers);
static void __cvmx_app_hotplug_delcores(
	struct cvmx_mbox *self, uint64_t *registers);

static CVMX_SHARED struct cvmx_mbox __cvmx_shutdown_handler = {
	.handler = __cvmx_app_hotplug_shutdown
};

static CVMX_SHARED struct cvmx_mbox __cvmx_addcores_handler = {
	.handler = __cvmx_app_hotplug_addcores
};

static CVMX_SHARED struct cvmx_mbox __cvmx_delcores_handler = {
	.handler = __cvmx_app_hotplug_delcores
};

/* Declaring this array here is a compile time check to ensure that the
   size of  cvmx_app_hotplug_info_t is 1024. If the size is not 1024
   the size of the array will be -1 and this results in a compilation
   error */
char __hotplug_info_check[(sizeof(cvmx_app_hotplug_info_t) == 1024) ? 1 : -1];
/**
 * This routine registers an application for hotplug. It installs a handler for
 * any incoming shutdown request. It also registers a callback routine from the
 * application. This callback is invoked when the application receives a
 * shutdown notification.
 *
 * This routine only needs to be called once per application.
 *
 * @param fn      Callback routine from the application.
 * @param arg     Argument to the application callback routine.
 * @return        Return 0 on success, -1 on failure
 *
 */
int cvmx_app_hotplug_register(void (*fn) (void *), void *arg)
{
	/* Find the list of applications launched by bootoct utility. */

	if (!(cvmx_app_hotplug_info_ptr =
	      cvmx_app_hotplug_get_info(&cvmx_sysinfo_get()->core_mask))) {
		/* Application not launched by bootoct? */
		printf("ERROR: cmvx_app_hotplug_register() failed\n");
		return -1;
	}

	/* Register the callback */
	cvmx_app_hotplug_info_ptr->data = CAST64(arg);
	cvmx_app_hotplug_info_ptr->shutdown_callback = CAST64(fn);

#ifdef DEBUG
	printf("%s(): coremask: ", __FUNCTION__);
	cvmx_coremask_print(&cvmx_app_hotplug_info_ptr->coremask);
#endif
	return 0;
}

/**
 * This routine deprecates the the cvmx_app_hotplug_register method. This
 * registers application for hotplug and the application will have CPU
 * hotplug callbacks. Various callbacks are specified in cb.
 * cvmx_app_hotplug_callbacks_t documents the callbacks
 *
 * This routine only needs to be called once per application.
 *
 * @param cb      Callback routine from the application.
 * @param arg     Argument to the application callback routins
 * @param app_shutdown   When set to 1 the application will invoke core_shutdown
                         on each core. When set to 0 core shutdown will be
                         called invoked automatically after invoking the
                         application callback.
 * @return        Return index of app on success, -1 on failure
 *
 */
int cvmx_app_hotplug_register_cb(cvmx_app_hotplug_callbacks_t * cb, void *arg,
				 int app_shutdown)
{
	cvmx_app_hotplug_info_t *app_info;

	/* Find the list of applications launched by bootoct utility. */
	app_info = cvmx_app_hotplug_get_info(&cvmx_sysinfo_get()->core_mask);
	cvmx_app_hotplug_info_ptr = app_info;
	if (!app_info) {
		/* Application not launched by bootoct? */
		printf("ERROR: cmvx_app_hotplug_register() failed\n");
		return -1;
	}
	/* Register the callback */
	app_info->data = CAST64(arg);
	app_info->shutdown_callback = CAST64(cb->shutdown_callback);
	app_info->cores_added_callback = CAST64(cb->cores_added_callback);
	app_info->cores_removed_callback = CAST64(cb->cores_removed_callback);
	app_info->unplug_callback = CAST64(cb->unplug_core_callback);
	app_info->app_shutdown = app_shutdown;
#ifdef DEBUG
	printf("%s(): coremask: ", __FUNCTION__);
	cvmx_coremask_print(&app_info->coremask);
#endif
	return 0;

}

void cvmx_app_hotplug_remove_self_from_core_mask(void)
{
	cvmx_spinlock_lock(&cvmx_app_hotplug_lock);
	cvmx_coremask_clear_self(&cvmx_app_hotplug_info_ptr->coremask);
	cvmx_coremask_clear_self(&cvmx_app_hotplug_info_ptr->hotplug_activated_coremask);
	cvmx_spinlock_unlock(&cvmx_app_hotplug_lock);
}

/**
*  Returns 1 if the running core is being unplugged, else it returns 0.
*/
int is_core_being_unplugged(void)
{
	return cvmx_coremask_is_self_set(&cvmx_app_hotplug_info_ptr->unplug_cores);
}

/**
 * Activate the current application core for receiving hotplug shutdown requests.
 *
 * This routine makes sure that each core belonging to the application is enabled
 * to receive the shutdown notification and also provides a barrier sync to make
 * sure that all cores are ready.
 */
int cvmx_app_hotplug_activate(void)
{
	uint64_t cnt = 0;
	uint64_t cnt_interval = (1<<24);
	uint64_t hplug_bits = 0;

	while (!cvmx_app_hotplug_info_ptr) {
		CVMX_SYNC;
		cnt++;
#ifdef DEBUG
		if ((cnt % cnt_interval) == 0) {
			printf("%s: waiting for cnt=%lld\n",
				__FUNCTION__,
			       (unsigned long long)cnt);
		}
#endif
		if (cnt > (cnt_interval << 4)) {
			printf("%s: timeout waiting for hotplug info\n",
				__FUNCTION__);
			return -1;
		}
	}

	if (!cvmx_app_hotplug_info_ptr) {
		printf("ERROR: This application is not registered for hotplug\n");
		return -1;
	}

	if (cvmx_coremask_is_self_set(&cvmx_app_hotplug_info_ptr->hplugged_cores)) {
#ifdef DEBUG
		printf("%s: hotplugging\n",__func__);
#endif
		cvmx_sysinfo_add_self_to_core_mask();
	} else {
#ifdef DEBUG
		printf("%s: syncing\n", __func__);
#endif
		__cvmx_app_hotplug_sync();
	}

	/* Enable the interrupt before we mark the core as activated */
	hplug_bits |=  1ull << CVMX_MBOX_BIT_HOTPLUG_SHUTDOWN;

	if (cvmx_app_hotplug_info_ptr->cores_removed_callback)
		hplug_bits |=  1ull << CVMX_MBOX_BIT_HOTPLUG_REMOVECORES;
	if (cvmx_app_hotplug_info_ptr->cores_added_callback)
		hplug_bits |=  1ull << CVMX_MBOX_BIT_HOTPLUG_ADDCORES;

	cvmx_mbox_initialize(hplug_bits);

	/* Register mailbox handlers after initialization, on init core only */
	if (cvmx_is_init_core()) {
		cvmx_mbox_register(CVMX_MBOX_BIT_HOTPLUG_SHUTDOWN,
			&__cvmx_shutdown_handler);
		cvmx_mbox_register(CVMX_MBOX_BIT_HOTPLUG_REMOVECORES,
			&__cvmx_delcores_handler);
		cvmx_mbox_register(CVMX_MBOX_BIT_HOTPLUG_ADDCORES,
			&__cvmx_addcores_handler);
	}

	cvmx_spinlock_lock(&cvmx_app_hotplug_lock);
	cvmx_coremask_set_self(
		&cvmx_app_hotplug_info_ptr->hotplug_activated_coremask);
	cvmx_spinlock_unlock(&cvmx_app_hotplug_lock);

#ifdef DEBUG
	printf("%s: coremask: ", __func__);
	cvmx_coremask_print(&cvmx_app_hotplug_info_ptr->coremask);
#endif


	return 0;
}

/**
 * This routine is only required if cvmx_app_hotplug_shutdown_request() was called
 * with wait=0. This routine waits for the application shutdown to complete.
 *
 * @param coremask     Coremask the application is running on.
 * @return             0 on success, -1 on error
 *
 */
int cvmx_app_hotplug_shutdown_complete(const struct cvmx_coremask *pcm)
{
	cvmx_app_hotplug_info_t *hotplug_info_ptr;

	if (!(hotplug_info_ptr = cvmx_app_hotplug_get_info(pcm))) {
		printf("\nERROR: Failed to get hotplug info for coremask:\n");
		cvmx_coremask_print(pcm);
		return -1;
	}

	while (!hotplug_info_ptr->shutdown_done)
		CVMX_SYNC;

	return 0;
}

/**
 * Disable recognition of any incoming shutdown request.
 */

void cvmx_app_hotplug_shutdown_disable(void)
{
	cvmx_spinlock_lock(&cvmx_app_hotplug_lock);
	if (!cvmx_coremask_is_self_set(
		&cvmx_app_hotplug_info_ptr->hotplug_activated_coremask)) {
		cvmx_spinlock_unlock(&cvmx_app_hotplug_lock);
		return;
	}
	cvmx_coremask_clear_self(
		&cvmx_app_hotplug_info_ptr->hotplug_activated_coremask);
	cvmx_app_hotplug_disabled = true ;
	cvmx_spinlock_unlock(&cvmx_app_hotplug_lock);
}

/**
 * Re-enable recognition of incoming shutdown requests.
 */

void cvmx_app_hotplug_shutdown_enable(void)
{
	cvmx_spinlock_lock(&cvmx_app_hotplug_lock);
	if (!cvmx_app_hotplug_disabled) {
		cvmx_spinlock_unlock(&cvmx_app_hotplug_lock);
		return;
	}

	cvmx_coremask_set_self(
		&cvmx_app_hotplug_info_ptr->hotplug_activated_coremask);
	cvmx_app_hotplug_disabled = false ;
	cvmx_spinlock_unlock(&cvmx_app_hotplug_lock);
}

/**
*  Request shutdown of the currently running core. Should be
*  called by the application when it has been registered with
*  app_shutdown option set to 1.
*/
void cvmx_app_hotplug_core_shutdown(void)
{
	if (!cvmx_coremask_is_empty(
	    &cvmx_app_hotplug_info_ptr->shutdown_cores)) {
		cvmx_app_hotplug_remove_self_from_core_mask();
		__cvmx_app_hotplug_sync();
		if (cvmx_is_init_core()) {
#ifdef DEBUG
			printf("%s: setting shutdown done! \n", __FUNCTION__);
#endif
			cvmx_app_hotplug_info_ptr->shutdown_done = 1;
		}
		/* Tell the debugger that this application is finishing.  */
		cvmx_debug_finish();
		cvmx_interrupt_disable_save();
		__cvmx_app_hotplug_sync();
		/* Reset the core */
		__cvmx_app_hotplug_reset();
	} else {
		cvmx_sysinfo_remove_self_from_core_mask();
		cvmx_app_hotplug_remove_self_from_core_mask();
		cvmx_interrupt_disable_save();
		__cvmx_app_hotplug_reset();
	}
}

/*
 * ISR for the incoming shutdown request interrupt.
 */
static void __cvmx_app_hotplug_shutdown(
	struct cvmx_mbox *self, uint64_t *registers)
{
	cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();
	cvmx_app_hotplug_info_t *ai = cvmx_app_hotplug_info_ptr;
	int core = cvmx_get_core_num();

	if (!cvmx_app_hotplug_info_ptr) {
		printf("ERROR: Application is not registered for hotplug!\n");
		return;
	}

	if (cvmx_coremask_cmp(&ai->hotplug_activated_coremask,
			       &sys_info_ptr->core_mask)) {
		printf("WARNING: Hotplug requested on core %d "
			"when not all app cores have activated hotplug\n"
		       "Application coremask:\n", core);
		cvmx_coremask_print(&sys_info_ptr->core_mask);
		puts("Hotplug coremask:");
		cvmx_coremask_print(&ai->hotplug_activated_coremask);
		// return;
	}

#ifdef DEBUG
	printf("Shutting down application .\n");
#endif

	/* Call the application's own callback function */
	if (ai->shutdown_callback) {
		void (*shutdown_callback) (void *ptr);
		void * fptr = CASTPTR(void *, ai->shutdown_callback);
		shutdown_callback = fptr;
		shutdown_callback(CASTPTR(void *, ai->data));
		} else {
			printf("ERROR: Shutdown callback has not been registered\n");
		}
	if (!ai->app_shutdown) {
#ifdef DEBUG
		printf("%s: core = %d Invoking _core_shutdown from handler\n",
				       __func__, core);
#endif
		cvmx_app_hotplug_core_shutdown();
	}
}

static void __cvmx_app_hotplug_delcores(
	struct cvmx_mbox *self, uint64_t *registers)
{
	cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();
	cvmx_app_hotplug_info_t *ai = cvmx_app_hotplug_info_ptr;
	int core = cvmx_get_core_num();

	if (!cvmx_app_hotplug_info_ptr) {
		printf("ERROR: Application is not registered for hotplug!\n");
		return;
	}

	if (cvmx_coremask_cmp(&ai->hotplug_activated_coremask,
			       &sys_info_ptr->core_mask)) {
		printf("WARNING: Hotplug requested on core %d "
			"when not all app cores have activated hotplug\n"
		       "Application coremask:\n", core);
		cvmx_coremask_print(&sys_info_ptr->core_mask);
		puts("Hotplug coremask:");
		cvmx_coremask_print(&ai->hotplug_activated_coremask);
		// return;
	}
#ifdef	DEBUG
	printf("%s : core=%d Unplug event \n", __func__, core);
#endif

	/* Notify cores that are not being unplugged of the event */
	if (!is_core_being_unplugged()) {
		void (*cores_removed_callback)( cvmx_coremask_t *, void *ptr);
		void * fptr = CASTPTR(void *, ai->cores_removed_callback);

		if (fptr == NULL) {
			printf("ERROR: "
			"Cores-removed callback has not been registered\n");
			return;
		}

#ifdef	DEBUG
		printf("%s : core=%d Calling cores removed callback\n",
					       __func__, core);
#endif

		cores_removed_callback = fptr;
		cores_removed_callback(
			&ai->unplug_cores, CASTPTR(void *, ai->data));
		return;
	}

	/* Notify the cores being removed, and reset them if necesary */
	if (ai->unplug_callback) {
		void (*unplug_core_callback) (void *ptr);
		void * fptr = CASTPTR(void *, ai->unplug_callback);

#ifdef	DEBUG
		printf("%s : core=%d Calling unplug callback\n",
				__func__, core);
#endif
		unplug_core_callback = fptr;
		unplug_core_callback(CASTPTR(void *, ai->data));
		} else {
			printf("ERROR: "
				"Unplug callback has not been registered\n");
		}

	/* Reset the core if it can not do it from callback */
	if (!ai->app_shutdown) {
#ifdef	DEBUG
		printf("%s: core = %d Invoking _core_shutdown from handler\n",
				__func__, core);
#endif
		cvmx_app_hotplug_core_shutdown();
	}

}
static void __cvmx_app_hotplug_addcores(
	struct cvmx_mbox *self, uint64_t *registers)
{
	cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();
	cvmx_app_hotplug_info_t *ai = cvmx_app_hotplug_info_ptr;
	int core = cvmx_get_core_num();

	if (!cvmx_app_hotplug_info_ptr) {
		printf("ERROR: Application is not registered for hotplug!\n");
		return;
	}

	if (cvmx_coremask_cmp(&ai->hotplug_activated_coremask,
			       &sys_info_ptr->core_mask)) {
		printf("WARNING: Hotplug requested on core %d "
			"when not all app cores have activated hotplug\n"
		       "Application coremask:\n", core);
		cvmx_coremask_print(&sys_info_ptr->core_mask);
		puts("Hotplug coremask:");
		cvmx_coremask_print(&ai->hotplug_activated_coremask);
		// return;
	}

#ifdef	DEBUG
	printf("%s : core=%d Add cores event \n", __func__, core);
#endif

	if (ai->cores_added_callback) {
		void (*cores_added_callback) (cvmx_coremask_t *, void *ptr);
		void * fptr = CASTPTR(void *, ai->cores_added_callback);
		cores_added_callback = fptr;
#ifdef	DEBUG
		printf("%s: core=%d Calling cores added callback\n",
				       __func__, core);
#endif
		cores_added_callback(
			&ai->hplugged_cores, CASTPTR(void *, ai->data));
	}
}



void __cvmx_app_hotplug_reset(void)
{
#define IDLE_CORE_BLOCK_NAME    "idle-core-loop"
#define HPLUG_MAKE_XKPHYS(x)       ((1ULL << 63) | (x))
	uint64_t reset_addr;
	const cvmx_bootmem_named_block_desc_t *block_desc;

	block_desc = cvmx_bootmem_find_named_block(IDLE_CORE_BLOCK_NAME);
	if (!block_desc) {
		cvmx_dprintf("Named block(%s) is not created\n", IDLE_CORE_BLOCK_NAME);
		/* loop here, should not happen */
		__asm__ volatile (".set noreorder      \n"
				  "\tsync               \n"
				  "\tnop               \n" "1:\twait            \n" "\tb 1b              \n" "\tnop               \n" ".set reorder        \n"::);
	}

	reset_addr = HPLUG_MAKE_XKPHYS(block_desc->base_addr);
	asm volatile ("       .set push                \n"
		      "       .set mips64              \n"
		      "       .set noreorder           \n"
		      "       move  $2, %[addr]        \n" "       jr    $2                 \n" "       nop                      \n" "       .set pop "::[addr] "r"(reset_addr)
		      :"$2");

	/*Should never reach here */
	while (1) ;

}

/* 
 * We need a separate sync operation from cvmx_coremask_barrier_sync() to
 * avoid a deadlock on state.lock, since the application itself maybe doing a
 * cvmx_coremask_barrier_sync(). 
 */
static void __cvmx_app_hotplug_sync(void)
{
	static CVMX_SHARED struct cvmx_coremask sync_coremask = CVMX_COREMASK_EMPTY;
	cvmx_sysinfo_t *sys_info_ptr = cvmx_sysinfo_get();

	cvmx_spinlock_lock(&cvmx_app_hotplug_sync_lock);

	cvmx_coremask_set_self(&sync_coremask);
	CVMX_SYNC;

	cvmx_spinlock_unlock(&cvmx_app_hotplug_sync_lock);

	/*
	 * Wait for all cores in sys_info_ptr->core_mask to assert their
	 * flag in the barrier mask.
	 */
	
	while (cvmx_coremask_cmp(&sync_coremask, &sys_info_ptr->core_mask)) {
#ifdef	DEBUG
		int cnt=0;
		if((++cnt & 0xfffff) == 0 ) {
			printf("%s: waiting cnt=%d sync_coremask: ",
				__FUNCTION__, cnt );
			cvmx_coremask_print(&sync_coremask);
		}
#endif	/* DEBUG */
		CVMX_SYNC;
		if( cvmx_coremask_is_empty( &sync_coremask ))
			return;
	}

	/*
	 * The first core to pass the barrier will clear the mask
	 * and therefore let all other cores loose.
	 */
	cvmx_spinlock_lock(&cvmx_app_hotplug_sync_lock);
	cvmx_coremask_clear_all(&sync_coremask);
	cvmx_spinlock_unlock(&cvmx_app_hotplug_sync_lock);

}

#endif /* CVMX_BUILD_FOR_LINUX_USER */

/**
*  Returns 1 if the running core is being hotplugged, else it returns 0.
*/
int is_core_being_hot_plugged(void)
{

#ifndef CVMX_BUILD_FOR_LINUX_USER
	if (!cvmx_app_hotplug_info_ptr)
		return 0;
	if (cvmx_coremask_is_self_set(&cvmx_app_hotplug_info_ptr->hplugged_cores))
		return 1;
	return 0;
#else
	return 0;
#endif
}

static cvmx_app_hotplug_global_t *cvmx_app_get_hotplug_global_ptr(void)
{
	const struct cvmx_bootmem_named_block_desc *block_desc;
	cvmx_app_hotplug_global_t *hgp;

	if (hotplug_global_ptr != 0)
		return hotplug_global_ptr;

	block_desc = cvmx_bootmem_find_named_block(CVMX_APP_HOTPLUG_INFO_REGION_NAME);
	if (!block_desc) {
		printf("ERROR: Hotplug info region is not setup\n");
		return NULL;
	} else
#ifdef CVMX_BUILD_FOR_LINUX_USER
	{
		size_t pg_sz = sysconf(_SC_PAGESIZE), size;
		off_t offset;
		char *vaddr;
		int fd;

		if ((fd = open("/dev/mem", O_RDWR)) == -1) {
			perror("open");
			return NULL;
		}

		/*
		 * We need to mmap() this memory, since this was allocated from the
		 * kernel bootup code and does not reside in the RESERVE32 region.
		 */
		size = CVMX_APP_HOTPLUG_INFO_REGION_SIZE + pg_sz - 1;
		offset = block_desc->base_addr & ~(pg_sz - 1);
		if ((vaddr = mmap(NULL, size, PROT_READ | PROT_WRITE,
				  MAP_SHARED, fd, offset))
		    == MAP_FAILED) {
			perror("mmap");
			return NULL;
		}

		hgp = (cvmx_app_hotplug_global_t *)
			(vaddr + (block_desc->base_addr & (pg_sz - 1)));
	}
#else
		hgp = CASTPTR(void, CVMX_ADD_SEG(CVMX_MIPS_SPACE_XKPHYS,
						 block_desc->base_addr));
#endif
	if (hgp->magic_version != CVMX_HOTPLUG_MAGIC_VERSION) {
		printf("ERROR: Hotplug info region is invalid\n");
		return NULL;
	}

	hotplug_global_ptr = hgp;
	return hgp;

}

/**
 * Return the hotplug info structure (cvmx_app_hotplug_info_t) pointer for the
 * application running on the given coremask.
 *
 * @param coremask     Coremask of application.
 * @return             Returns hotplug info struct on success, NULL on failure
 *
 */
cvmx_app_hotplug_info_t *
cvmx_app_hotplug_get_info(const struct cvmx_coremask *pcm)
{
	cvmx_app_hotplug_info_t *hip;
	cvmx_app_hotplug_global_t *hgp;
	int i;

	hgp = cvmx_app_get_hotplug_global_ptr();
	if (!hgp)
		return NULL;
	hip = hgp->hotplug_info_array;

 	i = cvmx_app_hotplug_get_index(pcm);

	if (i < 0)
		return NULL;
	else
		return &hip[i];

}

/**
 * Return the hotplug application index structure for the application running on the
 * given coremask.
 *
 * @param coremask     Coremask of application.
 * @return             Returns hotplug application index on success.
 *                     -1 on failure
 *
 */
int cvmx_app_hotplug_get_index(const struct cvmx_coremask *pcm)
{
	cvmx_app_hotplug_info_t *hip;
	cvmx_app_hotplug_global_t *hgp;
	int i;
	int dbg = 0;

#ifdef DEBUG
	dbg = 1;
#endif
	hgp = cvmx_app_get_hotplug_global_ptr();
	if (!hgp)
		return -1;
	hip = hgp->hotplug_info_array;

	if(cvmx_coremask_is_empty(pcm)) {
		printf("%s: empty coremask\n", __FUNCTION__);
		return -1;
	}

	/* Look for the current app's info */
	for (i = 0; i < CVMX_APP_HOTPLUG_MAX_APPS; i++) {
		if (cvmx_coremask_cmp(&hip[i].coremask, pcm))
			continue;
		if (dbg) {
			printf("%s: app_index: %d matched coremask:",
				__FUNCTION__,i);
			cvmx_coremask_print(&hip[i].coremask);
		}
		if (hip[i].valid_magic != CVMX_HOTPLUG_MAGIC_VALID) {
			printf("%s: hotplug info %d not valid\n",
				__FUNCTION__, i);
			break;
		}
		return i;
	}

	if(dbg) {
		printf("%s: no match on coremask", __FUNCTION__);
		cvmx_coremask_print(pcm);
	}

	return -1;
}

/**
 * Return the hotplug info structure (cvmx_app_hotplug_info_t) pointer for the
 * application with the specified index.
 *
 * @param index        index of application.
 * @return             Returns hotplug info struct on success, NULL on failure
 *
 */
cvmx_app_hotplug_info_t *cvmx_app_hotplug_get_info_at_index(int index)
{
	cvmx_app_hotplug_info_t *hip;
	cvmx_app_hotplug_global_t *hgp;

	CVMX_BUILD_ASSERT(sizeof(cvmx_app_hotplug_info_t)==1024u);

	hgp = cvmx_app_get_hotplug_global_ptr();
	if (!hgp)
		return NULL;
	hip = hgp->hotplug_info_array;

#ifdef DEBUG
	printf("%s: hotplug_info addr %p global_ptr %p\n",
		__FUNCTION__, hip, hgp);
#endif
	if (index < CVMX_APP_HOTPLUG_MAX_APPS) {
		if (hip[index].valid_magic == CVMX_HOTPLUG_MAGIC_VALID) {
			//print_hot_plug_info( &hip[index] );
			return &hip[index];
		}
	}
	return NULL;
}

/**
 * Determines if SE application at the index specified is hotpluggable.
 *
 * @param index        index of application.
 * @return             Returns -1  on error.
 *                     0 -> The application is not hotpluggable
 *                     1 -> The application is hotpluggable
*/
int is_app_hotpluggable(int index)
{
	cvmx_app_hotplug_info_t *ai;

	if (!(ai = cvmx_app_hotplug_get_info_at_index(index))) {
		printf("\nERROR: Failed to get hotplug info for app at index=%d\n",
		       index);
		return -1;
	}
	if (!cvmx_coremask_is_empty(&ai->hotplug_activated_coremask))
		return 1;
	return 0;
}

/**
 * This routine sends a shutdown request to a running target application.
 *
 * @param coremask     Coremask the application is running on.
 * @param wait         1 - Wait for shutdown completion
 *                     0 - Do not wait
 * @return             0 on success, -1 on error
 *
 */

int cvmx_app_hotplug_shutdown_request(const struct cvmx_coremask *pcm, int wait)
{
	int i;
	cvmx_app_hotplug_info_t *hotplug_info_ptr;

	if (!(hotplug_info_ptr = cvmx_app_hotplug_get_info(pcm))) {
		printf("\nERROR: Failed to get hotplug info for coremask\n");
		cvmx_coremask_print(pcm);
		return -1;
	}
	cvmx_coremask_copy(&hotplug_info_ptr->shutdown_cores, pcm);
	if (!hotplug_info_ptr->shutdown_callback) {
		printf("\nERROR: Target application has not registered for hotplug!\n");
		return -1;
	}

	if (cvmx_coremask_cmp(&hotplug_info_ptr->hotplug_activated_coremask,
			      pcm)) {
		printf("\nERROR: Not all application cores have activated hotplug\n");
		return -1;
	}

	/* Send IPIs to all application cores to request shutdown */
	cvmx_coremask_for_each_core(i, pcm) {
		cvmx_mbox_signal(CVMX_MBOX_BIT_HOTPLUG_SHUTDOWN, i);
	}

	if (wait) {
		while (!hotplug_info_ptr->shutdown_done) {
#ifdef CVMX_BUILD_FOR_LINUX_USER
			usleep(10000);
			if (hotplug_info_ptr->shutdown_done == 0)
				printf("WARNING: Waiting for application to shutdown\n");
			else
				break;
#endif
		CVMX_SYNC;
		}
	}

	return 0;
}

/**
 * This routine invokes the invoked the cores_added callbacks.
 */
int cvmx_app_hotplug_call_add_cores_callback(int index)
{
	cvmx_app_hotplug_info_t *ai;
	int i;
	if (!(ai = cvmx_app_hotplug_get_info_at_index(index))) {
		printf("\nERROR: Failed to get hotplug info for app at index=%d\n", index);
		return -1;
	}
	/* Send IPIs to all application cores to request add_cores callback */
	cvmx_coremask_for_each_core(i, &ai->coremask) {
		cvmx_mbox_signal(CVMX_MBOX_BIT_HOTPLUG_ADDCORES, i);

	}
	return 0;
}

/**
 * This routine sends a request to a running target application
 * to unplug a specified set cores
 * @param index        is the index of the target application
 * @param coremask     Coremask of the cores to be unplugged from the app.
 * @param wait         1 - Wait for shutdown completion
 *                     0 - Do not wait
 * @return             0 on success, -1 on error
 *
 */
int cvmx_app_hotplug_unplug_cores(int index, const struct cvmx_coremask *pcm,
				  int wait)
{
	cvmx_app_hotplug_info_t *ai;
	int i;
	struct cvmx_coremask tmp_cm;

	if (!(ai = cvmx_app_hotplug_get_info_at_index(index))) {
		printf("\nERROR: Failed to get hotplug info for app at index=%d\n",
		       index);
		return -1;
	}
	cvmx_coremask_copy(&ai->unplug_cores, pcm);
	cvmx_coremask_and(&tmp_cm, &ai->coremask, pcm);
	if (cvmx_coremask_cmp(&tmp_cm, pcm)) {
		printf("\nERROR: Not all cores requested are a part of the app "
		       "r:\n");
		cvmx_coremask_print(pcm);
		cvmx_coremask_print(&ai->coremask);
		return -1;
	}
	if (!cvmx_coremask_cmp(&ai->coremask, pcm)) {
		printf("\nERROR: Trying to remove all cores in app.  r:\n");
		cvmx_coremask_print(pcm);
		cvmx_coremask_print(&ai->coremask);
		return -1;
	}
	/* Send IPIs to all application cores to request unplug/remove_cores
	   callback */
	cvmx_coremask_for_each_core(i, &ai->coremask) {
		cvmx_mbox_signal(CVMX_MBOX_BIT_HOTPLUG_REMOVECORES, i);
	}
	/* Update coremask of removed cores */
	cvmx_coremask_maskoff(&ai->coremask, &ai->coremask, pcm);

	/* Wait for the unplugged cores to be unactivated */
	while (wait) {
		cvmx_coremask_t mask;
		CVMX_SYNC;
		cvmx_coremask_and( &mask, pcm, &ai->hotplug_activated_coremask);
		if (cvmx_coremask_is_empty(&mask))
			break;
	}

	return 0;
}

/**
 * Returns 1 if any app is currently being currently booted , hotplugged or
 * shutdown. Only one app can be under a boot, hotplug or shutdown condition.
 * Before booting an app this methods should be used to check whether boot or
 * shutdown activity is in progress and proceed with the boot or shutdown only
 * when there is no other activity.
 *
 */
int is_app_under_boot_or_shutdown(void)
{
	int ret = 0;
	cvmx_app_hotplug_global_t *hgp;

	hgp = cvmx_app_get_hotplug_global_ptr();
	cvmx_spinlock_lock(&hgp->hotplug_global_lock);
	if (hgp->app_under_boot || hgp->app_under_shutdown)
		ret = 1;
	cvmx_spinlock_unlock(&hgp->hotplug_global_lock);
	return ret;

}

/**
 * Sets or clear the app_under_boot value. This when set signifies that an app
 * is being currently booted or hotplugged with a new core.
 *
 *
 * @param val     sets the app_under_boot to the specified value. This should be
 *                set to 1 while app any is being booted and cleared after the
 *                application has booted up.
 *
 * LR: Need to add return value to make this into a working lock
 */
void set_app_under_boot(int val)
{
	cvmx_app_hotplug_global_t *hgp;

	hgp = cvmx_app_get_hotplug_global_ptr();
	cvmx_spinlock_lock(&hgp->hotplug_global_lock);
	hgp->app_under_boot = val;
	cvmx_spinlock_unlock(&hgp->hotplug_global_lock);
}

/**
 * Sets or clear the app_under_shutdown value. This when set signifies that an
 * app is being currently shutdown or some cores of an app are being shutdown.
 *
 * @param val     sets the app_under_shutdown to the specified value. This
 *                should be set to 1 while any app is being shutdown and cleared
 *                after the shutdown of the app is complete.
 *
 */
void set_app_under_shutdown(int val)
{
	cvmx_app_hotplug_global_t *hgp;

	hgp = cvmx_app_get_hotplug_global_ptr();
	cvmx_spinlock_lock(&hgp->hotplug_global_lock);
	hgp->app_under_shutdown = val;
	cvmx_spinlock_unlock(&hgp->hotplug_global_lock);
}
