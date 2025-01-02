/***********************license start***************
 * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
 * Helper functions for PKI
 */

#ifndef __CVMX_HELPER_PKI_H__
#define __CVMX_HELPER_PKI_H__

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pki.h>
#else
#include "cvmx-pki.h"
#endif

#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* Modify this if more than 8 ilk channels need to be supported */
#define CVMX_MAX_PORT_PER_INTERFACE	64
#define CVMX_MAX_QOS_PRIORITY		64
#define CVMX_PKI_FIND_AVAILABLE_RSRC    (-1)

struct cvmx_pki_qos_schd {
	cvmx_fpa3_gaura_t _aura;	/* FPA3 AURA handle */
	cvmx_fpa3_pool_t _pool;	/* FPA3 POOL handle */
	bool pool_per_qos;	/* This qos priority will use its own pool, if FALSE use port pool */
	int pool_num;		/* pool number to use, if -1 allocated by sdk otherwise software should alloc it */
	char *pool_name;
	uint64_t pool_buff_size;/* size of buffer in pool , if this priority is using its own pool*/
				/* it's good to have same buffer size if qos are using separate pools */
	uint64_t pool_max_buff;	/* number of max buffers allowed in the pool, if this priority is using its own pool*/
	bool aura_per_qos;	/* This qos priority will use its own aura, if FALSE use port aura */
	int aura_num;		/* aura number to use, if -1 allocated by sdk otherwise software should alloc it */
	char *aura_name;
	uint64_t aura_buff_cnt;	/* number of buffers in aura, if this priority is using its own aura*/
	bool sso_grp_per_qos;	/* This qos priority will use its own group, if FALSE use port group */
	int sso_grp;		/* group number to use, if -1 allocated by sdk otherwise software should alloc it */
	uint16_t port_add;      /* for BGX super MAC ports which wants to have PFC enabled */
	int qpg_base;           /* offset in qpg table to use, if -1 allocated by sdk otherwise software should alloc it*/
};

struct cvmx_pki_prt_schd {
	cvmx_fpa3_pool_t _pool;	/* FPA3 POOL handle */
	cvmx_fpa3_gaura_t _aura;/* FPA3 AURA handle */
	bool cfg_port;          /* Set to 1 if this port on the interface is not used */
	int style;              /* If port is using its own style and not interface style */
	bool pool_per_prt; 	/* Port will use its own pool, if FALSE use interface pool */
	int pool_num;		/* pool number to use, if -1 allocated by sdk otherwise software should alloc it*/
	char *pool_name;
	uint64_t pool_buff_size;/*size of buffer in pool , if this port is using its own pool*/
			/* it's good to have same buffer size if ports are using same style but different pools*/
	uint64_t pool_max_buff;	/* number of max buffers allowed in the pool, if this port is using its own pool*/
	bool aura_per_prt;	/* port will use its own aura, if FALSE use interface aura */
	int aura_num;		/* aura number to use, if -1 allocated by sdk otherwise software should alloc it */
	char *aura_name;
	uint64_t aura_buff_cnt;	/* number of buffers in aura, if this pool is using its own aura*/
	bool sso_grp_per_prt; 	/* port will use its own sso group, if FALSE use interface group*/
	int sso_grp;		/* sso group number to use, if -1 allocated by sdk otherwise software should alloc it */
	enum cvmx_pki_qpg_qos qpg_qos;
	int qpg_base;           /* offset in qpg table to use, if -1 allocated by sdk otherwise software should alloc it*/
	struct cvmx_pki_qos_schd qos_s[CVMX_MAX_QOS_PRIORITY];
};

struct cvmx_pki_intf_schd {
	cvmx_fpa3_pool_t _pool;	/* FPA3 POOL handle */
	cvmx_fpa3_gaura_t _aura;/* FPA3 AURA handle */
	bool style_per_intf;	/* 1: all ports on this interface will use same style; 0:ports on this intf will use their own style */
	int style;              /* style number to use, if -1 allocated by sdk otherwise software should alloc it*/
	bool pool_per_intf; 	/* Ports will use either this shared pool or their own pool*/
	int pool_num;		/* pool number to use, if -1 allocated by sdk otherwise software should alloc it*/
	char *pool_name;
	uint64_t pool_buff_size;
	uint64_t pool_max_buff;
	bool aura_per_intf; 	/* Ports will use either this shared aura or their own aura */
	int aura_num;		/* aura number to use, if -1 allocated by sdk otherwise software should alloc it*/
	char *aura_name;
	uint64_t aura_buff_cnt;
	bool sso_grp_per_intf;	/* Ports will use either this shared group or their own aura */
	int sso_grp;		/* sso group number to use, if -1 allocated by sdk otherwise software should alloc it */
	bool qos_share_aura;	/* All ports share the same aura for respective qos if qpg_qos used*/
	bool qos_share_grp; 	/* All ports share the same sso group for respective qos if qps qos used*/
	int qpg_base;           /* offset in qpg table to use, if -1 allocated by sdk otherwise software should alloc it*/
	struct cvmx_pki_prt_schd prt_s[CVMX_MAX_PORT_PER_INTERFACE];
};

struct cvmx_pki_global_schd {
	bool setup_pool;
	int pool_num;              /* pool number to use, if -1 allocated by sdk otherwise software should alloc it*/
	char *pool_name;
	uint64_t pool_buff_size;
	uint64_t pool_max_buff;
	bool setup_aura;
	int aura_num;              /* aura number to use, if -1 allocated by sdk otherwise software should alloc it*/
	char *aura_name;
	uint64_t aura_buff_cnt;
	bool setup_sso_grp;
	int sso_grp;            /* sso group number to use, if -1 allocated by sdk otherwise software should alloc it */
	cvmx_fpa3_pool_t _pool;
	cvmx_fpa3_gaura_t _aura;
};

struct cvmx_pki_legacy_qos_watcher {
	bool configured;
	enum cvmx_pki_term field;
	uint32_t data;
	uint32_t data_mask;
	uint8_t advance;
	uint8_t sso_grp;
};

extern CVMX_SHARED bool cvmx_pki_dflt_init[CVMX_MAX_NODES];

extern CVMX_SHARED struct cvmx_pki_pool_config pki_dflt_pool[CVMX_MAX_NODES];
extern CVMX_SHARED struct cvmx_pki_aura_config pki_dflt_aura[CVMX_MAX_NODES];
extern CVMX_SHARED struct cvmx_pki_style_config pki_dflt_style[CVMX_MAX_NODES];
extern CVMX_SHARED struct cvmx_pki_pkind_config pki_dflt_pkind[CVMX_MAX_NODES];
extern CVMX_SHARED uint64_t pkind_style_map[CVMX_MAX_NODES][CVMX_PKI_NUM_PKIND];
extern CVMX_SHARED struct cvmx_pki_sso_grp_config pki_dflt_sso_grp[CVMX_MAX_NODES];
extern CVMX_SHARED struct cvmx_pki_legacy_qos_watcher qos_watcher[8];

/**
 * This function Enabled the PKI hardware to
 * start accepting/processing packets.
 * @param node    node number
 */
void cvmx_helper_pki_enable(int node);

/**
 * This function frees up PKI resources consumed by that port.
 * This function should only be called if port resources
 * (fpa pools aura, style qpg entry pcam entry etc.) are not shared
 * @param xipd_port     ipd port number for which resources need to
 *                      be freed.
 */
int cvmx_helper_pki_port_shutdown(int xipd_port);

/**
 * This function shuts down complete PKI hardware
 * and software resources.
 * @param node          node number where PKI needs to shutdown.
 */
void cvmx_helper_pki_shutdown(int node);

/**
 * This function calculates how mant qpf entries will be needed for
 * a particular QOS.
 * @param qpg_qos       qos value for which entries need to be calculated.
 */
int cvmx_helper_pki_get_num_qpg_entry(enum cvmx_pki_qpg_qos qpg_qos);

/**
 * This function setups the qos table by allocating qpg entry and writing
 * the provided parameters to that entry (offset).
 * @param node          node number.
 * @param qpg_cfg       pointer to struct containing qpg configuration
 */
int cvmx_helper_pki_set_qpg_entry(int node, struct cvmx_pki_qpg_config *qpg_cfg);

/**
 * This function sets up aura QOS for RED, backpressure and tail-drop.
 *
 * @param node       node number.
 * @param aura       aura to configure.
 * @param ena_red       enable RED based on [DROP] and [PASS] levels
 *			1: enable 0:disable
 * @param pass_thresh   pass threshold for RED.
 * @param drop_thresh   drop threshold for RED
 * @param ena_bp        enable backpressure based on [BP] level.
 *			1:enable 0:disable
 * @param bp_thresh     backpressure threshold.
 * @param ena_drop      enable tail drop.
 *			1:enable 0:disable
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_setup_aura_qos(int node, int aura, bool ena_red, bool ena_drop,
	uint64_t pass_thresh, uint64_t drop_thresh,
	bool ena_bp, uint64_t bp_thresh);

/**
 * This function maps specified bpid to all the auras from which it can receive bp and
 * then maps that bpid to all the channels, that bpid can asserrt bp on.
 *
 * @param node          node number.
 * @param aura          aura number which will back pressure specified bpid.
 * @param bpid          bpid to map.
 * @param chl_map       array of channels to map to that bpid.
 * @param chl_cnt	number of channel/ports to map to that bpid.
 * @return Zero on success. Negative on failure
 */
int cvmx_helper_pki_map_aura_chl_bpid(int node, uint16_t aura, uint16_t bpid,
		uint16_t chl_map[], uint16_t chl_cnt);

/**
 * This function sets up the global pool, aura and sso group
 * resources which application can use between any interfaces
 * and ports.
 * @param node          node number
 * @param gblsch        pointer to struct containing global
 *                      scheduling parameters.
 */
int cvmx_helper_pki_set_gbl_schd(int node, struct cvmx_pki_global_schd *gblsch);

/**
 * This function sets up scheduling parameters (pool, aura, sso group etc)
 * of an ipd port.
 * @param xipd_port     ipd port number
 * @param prtsch        pointer to struct containing port's
 *                      scheduling parameters.
 */
int cvmx_helper_pki_init_port(int xipd_port, struct cvmx_pki_prt_schd *prtsch);

/**
 * This function sets up scheduling parameters (pool, aura, sso group etc)
 * of an interface (all ports/channels on that interface).
 * @param xiface        interface number with node.
 * @param intfsch      pointer to struct containing interface
 *                      scheduling parameters.
 * @param gblsch       pointer to struct containing global scheduling parameters
 *                      (can be NULL if not used)
 */
int cvmx_helper_pki_init_interface(const int xiface,
	struct cvmx_pki_intf_schd *intfsch,
	struct cvmx_pki_global_schd *gblsch);
/**
 * This function gets all the PKI parameters related to that
 * particular port from hardware.
 * @param xipd_port     ipd port number to get parameter of
 * @param port_cfg      pointer to structure where to store read parameters
 */
void cvmx_pki_get_port_config(int xipd_port, struct cvmx_pki_port_config *port_cfg);

/**
 * This function sets all the PKI parameters related to that
 * particular port in hardware.
 * @param xipd_port     ipd port number to get parameter of
 * @param port_cfg      pointer to structure containing port parameters
 */
void cvmx_pki_set_port_config(int xipd_port, struct cvmx_pki_port_config *port_cfg);

/**
 * This function displays all the PKI parameters related to that
 * particular port.
 * @param xipd_port      ipd port number to display parameter of
 */
void cvmx_pki_show_port_config(int xipd_port);

/**
 * Modifies maximum frame length to check.
 * It modifies the global frame length set used by this port, any other
 * port using the same set will get affected too.
 * @param xipd_port	ipd port for which to modify max len.
 * @param max_size	maximum frame length
 */
void cvmx_pki_set_max_frm_len(int xipd_port, uint32_t max_size);

/**
 * This function sets up all the ports of particular interface
 * for chosen fcs mode. (only use for backward compatibility).
 * New application can control it via init_interface calls.
 * @param node          node number.
 * @param interface     interface number.
 * @param nports        number of ports
 * @param has_fcs       1 -- enable fcs check and fcs strip.
 *                      0 -- disable fcs check.
 */
void cvmx_helper_pki_set_fcs_op(int node, int interface, int nports, int has_fcs);

/**
 * This function sets the wqe buffer mode of all ports. First packet data buffer can reside
 * either in same buffer as wqe OR it can go in separate buffer. If used the later mode,
 * make sure software allocate enough buffers to now have wqe separate from packet data.
 * @param node	                node number.
 * @param pkt_outside_wqe	0 = The packet link pointer will be at word [FIRST_SKIP]
 *				    immediately followed by packet data, in the same buffer
 *				    as the work queue entry.
 *				1 = The packet link pointer will be at word [FIRST_SKIP] in a new
 *				    buffer separate from the work queue entry. Words following the
 *				    WQE in the same cache line will be zeroed, other lines in the
 *				    buffer will not be modified and will retain stale data (from the
 *				    buffer’s previous use). This setting may decrease the peak PKI
 *				    performance by up to half on small packets.
 */
void cvmx_helper_pki_set_wqe_mode(int node, bool pkt_outside_wqe);

/**
 * This function sets the Packet mode of all ports and styles to little-endian.
 * It Changes write operations of packet data to L2C to
 * be in little-endian. Does not change the WQE header format, which is
 * properly endian neutral.
 * @param node	                node number.
 */
void cvmx_helper_pki_set_little_endian(int node);

void cvmx_helper_pki_set_dflt_pool(int node, int pool,
				   int buffer_size, int buffer_count);
void cvmx_helper_pki_set_dflt_aura(int node, int aura,
				   int pool, int buffer_count);
void cvmx_helper_pki_set_dflt_pool_buffer(int node, int buffer_count);

void cvmx_helper_pki_set_dflt_aura_buffer(int node, int buffer_count);

void cvmx_helper_pki_set_dflt_pkind_map(int node, int pkind, int style);

void cvmx_helper_pki_get_dflt_style(int node, struct cvmx_pki_style_config *style_cfg);

void cvmx_helper_pki_set_dflt_style(int node, struct cvmx_pki_style_config *style_cfg);

void cvmx_helper_pki_no_dflt_init(int node);

void cvmx_helper_pki_set_dflt_bp_en(int node, bool bp_en);

void cvmx_pki_dump_wqe(const cvmx_wqe_78xx_t *wqp);

int __cvmx_helper_pki_port_setup(int node, int xipd_port);

int __cvmx_helper_pki_global_setup(int node);
void cvmx_helper_pki_show_port_config(int xipd_port);

int __cvmx_helper_pki_install_dflt_vlan(int node);
void __cvmx_helper_pki_set_dflt_ltype_map(int node);
int cvmx_helper_pki_route_dmac(int node, int style, uint64_t mac_addr, uint64_t mac_addr_mask, int final_style);
int cvmx_pki_clone_style(int node, int style, uint64_t cluster_mask);
void cvmx_helper_pki_modify_prtgrp(int xipd_port, int grp_ok, int grp_bad);
int cvmx_helper_pki_route_prt_dmac(int xipd_port, uint64_t mac_addr, uint64_t mac_addr_mask, int grp);

void cvmx_helper_pki_errata(int node);

#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_HELPER_PKI_H__ */
