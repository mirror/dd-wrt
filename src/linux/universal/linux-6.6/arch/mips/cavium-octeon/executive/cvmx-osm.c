/***********************license start***************
 * Copyright (c) 2014  Cavium Inc. (support@cavium.com). All rights
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
 * Support library for the OSM
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-spinlock.h>
#include <asm/octeon/cvmx-osm-defs.h>
#include <asm/octeon/cvmx-osm.h>
#else
#include <stdio.h>
#include <string.h>
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "cvmx-osm-defs.h"
#include "cvmx-osm.h"
#endif /* CVMX_BUILD_FOR_LINUX_KERNEL */


static CVMX_SHARED cvmx_spinlock_t cvmx_osm_lock=CVMX_SPINLOCK_UNLOCKED_INITIALIZER;
static CVMX_SHARED int cvmx_osm_init_done;
static const char * cvmx_osm_bank_names[] =
		{"DISABLED", "HFA", "HNA", "ASE-TWC", "ASE-BWC", "ASE-RWC"};

/**
 * Initilaize OSM
 *
 * @return 0 on success non zero on failure
 */
int cvmx_osm_init(void)
{
	cvmx_osm_memx_bist_status_t bist_status;
	cvmx_osm_bankx_ctrl_t bank_ctrl;
	int itr;

	cvmx_spinlock_lock(&cvmx_osm_lock);
	if (cvmx_osm_init_done) {
		cvmx_spinlock_unlock(&cvmx_osm_lock);
		return 0;
	}
	
	for (itr = 0; itr < 8; itr++) {
		bist_status.u64 = cvmx_read_csr(CVMX_OSM_MEMX_BIST_STATUS(itr));
		if (bist_status.s.bist_status) {
			cvmx_spinlock_unlock(&cvmx_osm_lock);
			cvmx_dprintf("OSM MEM %d bist error\n", itr);
			return -1;
		}
	}

	for (itr = 0; itr < CVMX_OSM_MAX_BANKS; itr++) {
		bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(itr));
		bank_ctrl.s.bank_assign = CVMX_OSM_DISABLED;
		cvmx_write_csr(CVMX_OSM_BANKX_CTRL(itr), bank_ctrl.u64);
		bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(itr));
	}

	cvmx_osm_init_done = 1;
	cvmx_spinlock_unlock(&cvmx_osm_lock);

	return 0;
}   
EXPORT_SYMBOL(cvmx_osm_init);


/**
 * Reserve consecutive banks in OSM
 *
 * @param start_bank_num - starting bank number 
 * @param num_banks - number of banks
 * @param bank_assign - bank requester
 * @return 0 on success non zero on failure
 *
 */
int cvmx_osm_reserve_banks(int start_bank_num,
				int num_banks,
				cvmx_osm_bank_assign_e_t bank_assign)
{
	int itr;
	cvmx_osm_bankx_ctrl_t bank_ctrl;

	if (start_bank_num < 0 )
		return -1;
	if ((start_bank_num + num_banks) > CVMX_OSM_MAX_BANKS)
		return -1;

	cvmx_spinlock_lock(&cvmx_osm_lock);
	for (itr = start_bank_num; itr < start_bank_num + num_banks; itr++) {
		bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(itr));
		if (bank_ctrl.s.bank_assign != CVMX_OSM_DISABLED) {
			cvmx_spinlock_unlock(&cvmx_osm_lock);
			return -1;
		}
	}
	for (itr = start_bank_num; itr < start_bank_num + num_banks; itr++) {
		bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(itr));
		bank_ctrl.s.bank_assign = bank_assign;
		cvmx_write_csr(CVMX_OSM_BANKX_CTRL(itr), bank_ctrl.u64);
		bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(itr));
	}
	cvmx_spinlock_unlock(&cvmx_osm_lock);
	return 0;
}
EXPORT_SYMBOL(cvmx_osm_reserve_banks);

/**
 * Release consecutive banks in OSM
 *
 * @param start_bank_num - starting bank number
 * @param num_banks - number of banks
 * @param bank_owner - bank owner
 * @return 0 on success non zero on failure
 *
 */
int cvmx_osm_release_banks(int start_bank_num,
				int num_banks,
				cvmx_osm_bank_assign_e_t bank_owner)
{
	int itr;
	cvmx_osm_bankx_ctrl_t bank_ctrl;

	if (start_bank_num < 0 )
		return -1;
	if ((start_bank_num + num_banks) > CVMX_OSM_MAX_BANKS)
		return -1;

	cvmx_spinlock_lock(&cvmx_osm_lock);
	for (itr = start_bank_num; itr < start_bank_num + num_banks; itr++) {
		bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(itr));
		if (bank_owner == bank_ctrl.s.bank_assign) {
			bank_ctrl.s.bank_assign = CVMX_OSM_DISABLED;
		}
		cvmx_write_csr(CVMX_OSM_BANKX_CTRL(itr), bank_ctrl.u64);
	}
	cvmx_spinlock_unlock(&cvmx_osm_lock);
	return 0;
}
EXPORT_SYMBOL(cvmx_osm_release_banks);

/**
 * Check if an OSM bank is free
 *
 * @param bank_num - bank number 
 * @return 1 if the bank is free else 0
 *
 */
int cvmx_osm_is_bank_free(int bank_num)
{
	cvmx_osm_bankx_ctrl_t bank_ctrl;

	if (bank_num < 0 )
		return 0;
	if (bank_num > (CVMX_OSM_MAX_BANKS-1))
		return 0;

	cvmx_spinlock_lock(&cvmx_osm_lock);
	bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(bank_num));
	cvmx_spinlock_unlock(&cvmx_osm_lock);
	return (bank_ctrl.s.bank_assign == CVMX_OSM_DISABLED);
}
EXPORT_SYMBOL(cvmx_osm_is_bank_free);

/**
 * Dump the allocation of OSM banks
 *
 */
void cvmx_osm_dump_banks(void)
{
	int itr;
	cvmx_osm_bankx_ctrl_t bank_ctrl;

	for (itr = 0; itr < CVMX_OSM_MAX_BANKS; itr++) {
		bank_ctrl.u64 = cvmx_read_csr(CVMX_OSM_BANKX_CTRL(itr));
		cvmx_printf("Bank-%d Assigned to %s\n",
			    itr, cvmx_osm_bank_names[bank_ctrl.s.bank_assign]);
	}
}
EXPORT_SYMBOL(cvmx_osm_dump_banks);
