/***********************license start***************
 * Copyright (c) 2012  Cavium Inc. (support@cavium.com). All rights
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

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/export.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-bootmem.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-ipd.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-app-config.h>
#else
#include "cvmx.h"
#include "cvmx-bootmem.h"
#include "cvmx-fpa.h"
#include "cvmx-ipd.h"
#include "cvmx-helper.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-helper-util.h"
#include "cvmx-app-config.h"
#endif

#define MAX_IFACE CVMX_HELPER_MAX_IFACE
#define MAX_PORT_PER_IFACE CVMX_HELPER_CFG_MAX_PORT_PER_IFACE
#define MAX_PKO_PORTS CVMX_HELPER_CFG_MAX_PKO_PORT
#define addr_of_data(base, index) (1ull << 63 | \
					(base + (index) * sizeof(int64_t)))

/* only wqe pool is exported for now */
#define MAX_NUM_OF_POOLS_EXPORTED 1

typedef int (*cvmx_import_config_t)(void);
typedef int (*cvmx_export_config_t)(void);

extern cvmx_import_config_t cvmx_import_app_config;
extern cvmx_export_config_t cvmx_export_app_config;

/* named block used to export the config to other applications */
CVMX_SHARED char cvmx_appcfg_transport_block_name[CVMX_BOOTMEM_NAME_LEN];

/* fpa pool type */
enum
{
	FPA_PACKET_POOL = 0,
	FPA_WQE_POOL,
	FPA_OUTPUT_POOL
};

/* fpa pool app config */
struct cvmx_fpa_appconfig
{
	int64_t pool_type;
	cvmx_fpa_pool_config_t pool_config;
};

/* get the size of pko config that will be copied into named block */
static inline int __cvmx_pko_config_get_size(void)
{
	int sz = 0;

	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		sz = (sizeof(struct cvmx_cfg_port_param) * MAX_IFACE * MAX_PORT_PER_IFACE);

	sz += (sizeof(struct cvmx_cfg_pko_port_param) * MAX_PKO_PORTS);

	return sz;
}

/* get the size of fpa config that will be copied into named block */
static inline int __cvmx_fpa_config_get_size(void)
{
	int sz = 0;

	sz = sizeof(struct cvmx_fpa_appconfig) * MAX_NUM_OF_POOLS_EXPORTED;
	
	return sz;
}

/**
 * @INTERNAL
 * Copy size bytes from local buffer to bootmem dest address.
 *
 * @param src_ptr is pointer to local buffer and is source for mem copy.
 * @param bootmem_dest_addr is destination bootmem addr for mem copy.
 * @param size is number of bytes to copy from src to dest.
 *
 * @return return zero on success.
 */
int __cvmx_copy_to_bootmem(void * src_ptr, int64_t bootmem_dest_addr, int size)
{
	int64_t base_addr;
	int sz, i;
	int64_t *ptr_64;
	int8_t *ptr_8;

	/* copy in 64bit words */
	sz = (size/8);

	ptr_64 = (int64_t *) src_ptr;
	base_addr = addr_of_data(bootmem_dest_addr, 0);

	for (i = 0; i < sz; i++) {
		cvmx_write64_int64(base_addr, ptr_64[i]);
		base_addr += 8;
	}

	/* write rem bytes */
	ptr_8 = (int8_t *)(ptr_64 + sz);
	sz = (size%8);
	if (sz) {
		for (i = 0; i < sz; i++) {
			cvmx_write64_int8(base_addr, ptr_8[i]);
			base_addr += 1;
		}
	}

	return 0;
}

/**
 * @INTERNAL
 * Copy size bytes from bootmem addr to local buffer.
 *
 * @param bootmem_src_addr is source bootmem addr for mem copy.
 * @param dst_ptr is pointer to local buffer and is dest for mem copy.
 * @param size is number of bytes to copy from src to dest.
 *
 * @return return zero on success, non-zero on failure.
 */
int __cvmx_copy_from_bootmem(int64_t bootmem_src_addr, void * dst_ptr, int size)
{
	int64_t base_addr;
	int sz, i;
	int64_t *ptr_64;
	int8_t *ptr_8;

	/* read 64bit words */
	sz = (size/8);

	ptr_64 = (int64_t *) dst_ptr;
	base_addr = addr_of_data(bootmem_src_addr, 0);

	for (i = 0; i < sz; i++) {
		ptr_64[i] = cvmx_read64_int64(base_addr);
		base_addr += 8;
	}

	/* read rem bytes */
	ptr_8 = (int8_t *)(ptr_64 + sz);
	sz = (size%8);
	if (sz) {
		for (i = 0; i < sz; i++) {
			ptr_8[i] = cvmx_read64_int8(base_addr);
			base_addr += 1;
		}
	}

	return 0;
}

/**
 * @INTERNAL
 * Export fpa config using named block.
 *
 * @param fpa_config_addr is address of named block.
 */
static void __cvmx_export_fpa_config(uint64_t fpa_config_addr)
{
	struct cvmx_fpa_appconfig fpa_config;

	/* copy wqe pool information */
	fpa_config.pool_config.pool_num = cvmx_fpa_get_wqe_pool();
	fpa_config.pool_config.buffer_size = cvmx_fpa_get_wqe_pool_block_size();
	fpa_config.pool_config.buffer_count = 0;	/* not used */
	fpa_config.pool_type = FPA_WQE_POOL;

	__cvmx_copy_to_bootmem(&fpa_config, fpa_config_addr, 
				sizeof(struct cvmx_fpa_appconfig));
}

/**
 * @INTERNAL
 * Exports pko config(port and queue config) using named block.
 *
 * @param pko_config_addr is address of named block.
 *
 * @return return zero on success.
 */
static int __cvmx_export_pko_config(int64_t pko_config_addr)
{
	int port_sz = 0, pko_port_sz;

	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		port_sz = (sizeof(struct cvmx_cfg_port_param) * MAX_IFACE * MAX_PORT_PER_IFACE);

	pko_port_sz = (sizeof(struct cvmx_cfg_pko_port_param) * MAX_PKO_PORTS);

	/* copy cvmx_cfg_pko_port_param */
	__cvmx_copy_to_bootmem(cvmx_pko_queue_table, pko_config_addr, 
				pko_port_sz);

	/* no cvmx_cfg_port_parm, return */
	if (port_sz == 0)
		return 0;

	pko_config_addr += pko_port_sz;

	/* copy cvmx_cfg_port_parm */
	__cvmx_copy_to_bootmem(cvmx_cfg_port, pko_config_addr, port_sz);

	return 0;
}

/**
 * @INTERNAL
 * Exports app config(pko, fpa pool config) using named block.
 *
 * @return return zero on success, non-zero on failure.
 */
int __cvmx_export_app_config(void)
{
	int sz, pko_cfg_sz, fpa_cfg_sz;
	int64_t app_config_addr;

	pko_cfg_sz = __cvmx_pko_config_get_size();
	fpa_cfg_sz = __cvmx_fpa_config_get_size();

	sz = pko_cfg_sz + fpa_cfg_sz;

	/* allocate named block */
	app_config_addr = cvmx_bootmem_phy_named_block_alloc(sz, 0, 0,
					CVMX_CACHE_LINE_SIZE,
					cvmx_appcfg_transport_block_name, 0);
	if (app_config_addr < 0) {
		cvmx_dprintf("ERROR: Could not allocate mem from bootmem\n");
		return -1;
	}

	/* write pko config to named block */
	__cvmx_export_pko_config(app_config_addr);

	app_config_addr += pko_cfg_sz;

	/* write fpa pool config to named block */
	__cvmx_export_fpa_config(app_config_addr);

	return 0;
}

/**
 * Called by apps to export app config to other
 * cooperating applications using a named block
 * defined by param block_name.
 *
 * @param block_name Name of the named block to use for exporting config.
 *
 * @return 0 on success.
 */
int __cvmx_export_app_config_to_named_block(char * block_name)
{
	int bootmem_name_len;

	bootmem_name_len = strlen(block_name);
	if (bootmem_name_len >= CVMX_BOOTMEM_NAME_LEN)
		bootmem_name_len = (CVMX_BOOTMEM_NAME_LEN - 1);

	memset (cvmx_appcfg_transport_block_name, 0 , CVMX_BOOTMEM_NAME_LEN);
	strncpy (cvmx_appcfg_transport_block_name, block_name,
			bootmem_name_len);

	cvmx_export_app_config = __cvmx_export_app_config;

	return 0;
}
EXPORT_SYMBOL(__cvmx_export_app_config_to_named_block);

/**
 * Called by apps to clean app config named block.
 */
void __cvmx_export_app_config_cleanup(void)
{
	const cvmx_bootmem_named_block_desc_t *block_desc;
	int dbg = 0, res = 0;

	/* find named block */
	block_desc = cvmx_bootmem_find_named_block(cvmx_appcfg_transport_block_name);
	if (!block_desc) {
		if (dbg) cvmx_dprintf("Could not find transport config named block\n");
		return;
	}

	res = cvmx_bootmem_free_named(cvmx_appcfg_transport_block_name);
	if (dbg) cvmx_dprintf("free transport config block res=%d\n", res);
}
EXPORT_SYMBOL(__cvmx_export_app_config_cleanup);

/**
 * Called by kernel modules to update appconfig
 * @return 0 if export successful, -1 on failure
 */
int __cvmx_export_config(void)
{
	if (cvmx_export_app_config)
		return (*cvmx_export_app_config)();
	return -1;
}
EXPORT_SYMBOL(__cvmx_export_config);

/**
 * @INTERNAL
 * Imports fpa config using named block.
 *
 * @param fpa_config_addr is address of named block.
 */
static void __cvmx_import_fpa_config(uint64_t fpa_config_addr)
{
	struct cvmx_fpa_appconfig fpa_config[MAX_NUM_OF_POOLS_EXPORTED];

	/* copy pool info */
	__cvmx_copy_from_bootmem(fpa_config_addr, fpa_config,
				__cvmx_fpa_config_get_size());

	/* get wqe pool info */
	if (fpa_config[0].pool_type == FPA_WQE_POOL) {
		int64_t pool_num = fpa_config[0].pool_config.pool_num;
		uint64_t buffer_size = fpa_config[0].pool_config.buffer_size;
		/* communicate pool information to ipd */
		cvmx_ipd_set_wqe_pool_config(pool_num, buffer_size, 0);
	}
}

/**
 * @INTERNAL
 * Imports pko config(port and queue config) using named block.
 *
 * @param pko_config_addr is address of named block.
 *
 * @return return zero on success, non-zero on failure.
 */
static int __cvmx_import_pko_config(int64_t pko_config_addr)
{
	int port_sz = 0, pko_port_sz;

	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		port_sz = (sizeof(struct cvmx_cfg_port_param) * MAX_IFACE * MAX_PORT_PER_IFACE);

	pko_port_sz = (sizeof(struct cvmx_cfg_pko_port_param) * MAX_PKO_PORTS);

	/* copy cvmx_cfg_pko_port_param */
	__cvmx_copy_from_bootmem(pko_config_addr, cvmx_pko_queue_table,
					pko_port_sz);

	if (port_sz == 0)
		return 0;

	pko_config_addr += pko_port_sz;

	/* copy cvmx_cfg_port_parm */
	__cvmx_copy_from_bootmem(pko_config_addr, cvmx_cfg_port, port_sz);

	return 0;
}

/**
 * @INTERNAL
 * Imports app config(pko, fpa pool config) from named block.
 *
 * @return return zero on success, non-zero on failure.
 */
int __cvmx_import_app_config(void)
{
	const cvmx_bootmem_named_block_desc_t *block_desc;
	int pko_cfg_sz;
	int64_t app_config_addr;

	/* find named block */
	block_desc = cvmx_bootmem_find_named_block(cvmx_appcfg_transport_block_name);
	if (!block_desc) {
		cvmx_dprintf("Could not find transport config named block\n");
		return -1;
	}

	app_config_addr = block_desc->base_addr;

	pko_cfg_sz = __cvmx_pko_config_get_size();

	/* read pko config from named block */
	__cvmx_import_pko_config(app_config_addr);

	app_config_addr += pko_cfg_sz;

	/* read fpa pool config from named block */
	__cvmx_import_fpa_config(app_config_addr);

	return 0;
}

/**
 * Called by apps to import app config from other
 * cooperating applications using a named block
 * defined by param block_name.
 *
 * @param block_name Name of the named block to use for exporting config.
 *
 * @return 0 on success.
 */
int __cvmx_import_app_config_from_named_block(char * block_name)
{
	int bootmem_name_len;

	bootmem_name_len = strlen(block_name);
	if (bootmem_name_len >= CVMX_BOOTMEM_NAME_LEN)
		bootmem_name_len = (CVMX_BOOTMEM_NAME_LEN - 1);

	memset (cvmx_appcfg_transport_block_name, 0 , CVMX_BOOTMEM_NAME_LEN);
	strncpy (cvmx_appcfg_transport_block_name, block_name,
			bootmem_name_len);

	cvmx_import_app_config = __cvmx_import_app_config;

	return 0;
}
