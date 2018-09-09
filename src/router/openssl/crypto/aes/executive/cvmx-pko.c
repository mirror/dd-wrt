/***********************license start*************** * Copyright (c) 2003-2010  Cavium Inc. (support@cavium.com). All rights
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
 * Support library for the hardware Packet Output unit.
 *
 * <hr>$Revision: 96655 $<hr>
 */
#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/export.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pko.h>
#include <asm/octeon/cvmx-helper.h>
#include <asm/octeon/cvmx-helper-cfg.h>
#include <asm/octeon/cvmx-helper-util.h>
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-clock.h>
#else
#include "cvmx.h"
#include "cvmx-sysinfo.h"
#include "cvmx-pko.h"
#include "cvmx-helper.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-helper-util.h"
#include "cvmx-fpa.h"
#ifndef __U_BOOT__
#include "cvmx-error.h"
#endif
#endif

/* #define PKO_DEBUG */

#define CVMX_PKO_NQ_PER_PORT_MAX	32

/**
 * Internal state of packet output
 */

/*
 * PKO port iterator
 */

#define pko_for_each_port(__p)					\
    for (__p = 0; __p < CVMX_HELPER_CFG_MAX_PKO_PORT; __p++)	\
	if (__cvmx_helper_cfg_pko_queue_base(__p) != CVMX_HELPER_CFG_INVALID_VALUE)

CVMX_SHARED cvmx_fpa_pool_config_t pko_fpa_config = {2,1024,0};
void cvmx_pko_set_cmd_que_pool_config(int64_t pool, uint64_t buffer_size,
				    uint64_t buffer_count)
{
	pko_fpa_config.pool_num = pool;
	pko_fpa_config.buffer_size = buffer_size;
	pko_fpa_config.buffer_count = buffer_count;
}
EXPORT_SYMBOL(cvmx_pko_set_cmd_que_pool_config);

void cvmx_pko_set_cmd_queue_pool_buffer_count(uint64_t buffer_count)
{
	pko_fpa_config.buffer_count = buffer_count;
}

void cvmx_pko_get_cmd_que_pool_config(cvmx_fpa_pool_config_t *pko_pool)
{
	*pko_pool = pko_fpa_config;
}

/*
 * @INTERNAL
 *
 * Get INT for a port
 *
 * @param interface
 * @param index
 * @return the INT value on success and -1 on error
 */
static int __cvmx_pko_int(int interface, int index)
{
	cvmx_helper_cfg_assert(interface < CVMX_HELPER_MAX_IFACE);
	cvmx_helper_cfg_assert(index >= 0);

	switch (interface) {
	case 0:
		cvmx_helper_cfg_assert(index < 4);
		return index;
	case 1:
		cvmx_helper_cfg_assert(index == 0);
		return 4;
	case 2:
		cvmx_helper_cfg_assert(index < 4);
		return index + 8;
	case 3:
		cvmx_helper_cfg_assert(index < 4);
		return index + 0xC;
	case 4:
		cvmx_helper_cfg_assert(index < 4);
		return index + 0x10;
	case 5:
		cvmx_helper_cfg_assert(index < 256);
		return 0x1C;
	case 6:
		cvmx_helper_cfg_assert(index < 256);
		return 0x1D;
	case 7:
		cvmx_helper_cfg_assert(index < 32);
		return 0x1E;
	case 8:
		cvmx_helper_cfg_assert(index < 8);
		return 0x1F;
	}

	return -1;
}

int cvmx_pko_get_base_pko_port(int interface, int index)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return __cvmx_helper_cfg_pko_port_base(interface, index);
	else
		return cvmx_helper_get_ipd_port(interface, index);
}
EXPORT_SYMBOL(cvmx_pko_get_base_pko_port);

int cvmx_pko_get_num_pko_ports(int interface, int index)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND))
		return __cvmx_helper_cfg_pko_port_num(interface, index);
	else
		return 1;
}
EXPORT_SYMBOL(cvmx_pko_get_num_pko_ports);

int cvmx_pko_get_base_queue(int port)
{
	if (octeon_has_feature(OCTEON_FEATURE_PKND)) {
		return __cvmx_helper_cfg_pko_queue_base(cvmx_helper_cfg_ipd2pko_port_base(port));
	} else {
		if (port < 48)
			return cvmx_pko_queue_table[port].ccppp_queue_base;
		else
			return CVMX_PKO_ILLEGAL_QUEUE;
	}
}
EXPORT_SYMBOL(cvmx_pko_get_base_queue);

/**
 * For a given PKO port number, return the base output queue
 * for the port.
 *
 * @param pko_port   PKO port number
 * @return           Base output queue
 */
int cvmx_pko_get_base_queue_pkoid(int pko_port)
{
	return __cvmx_helper_cfg_pko_queue_base(pko_port);
}

/**
 * For a given PKO port number, return the number of output queues
 * for the port.
 *
 * @param pko_port	PKO port number
 * @return		the number of output queues
 */
int cvmx_pko_get_num_queues_pkoid(int pko_port)
{
	return __cvmx_helper_cfg_pko_queue_num(pko_port);
}

int cvmx_pko_get_num_queues(int port)
{
	if (OCTEON_IS_MODEL(OCTEON_CN78XX)) {
		return 1;
	} else if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		return __cvmx_helper_cfg_pko_queue_num(cvmx_helper_cfg_ipd2pko_port_base(port));
	} else {
		if (port < 48)
			return cvmx_pko_queue_table[port].ccppp_num_queues;
	}
	return 0;
}
EXPORT_SYMBOL(cvmx_pko_get_num_queues);

#ifdef PKO_DEBUG
/**
 * Show queues for the internal ports
 */
void cvmx_pko_show_queue_map(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		int port;
		pko_for_each_port(port) {
			cvmx_dprintf("pko_port %d (interface%d index%d) has %d queues (queue base = %d)\n",
				     port,
				     __cvmx_helper_cfg_pko_port_interface(port),
				     __cvmx_helper_cfg_pko_port_index(port), __cvmx_helper_cfg_pko_queue_num(port), __cvmx_helper_cfg_pko_queue_base(port));
		}
	} else {
		int port;
		int pko_output_ports;
		pko_output_ports = 36;
		cvmx_dprintf("pko queue info \n");
		for (port = 0; port < pko_output_ports; port++) {
			cvmx_dprintf("%3d=%3d ", port, cvmx_pko_get_base_queue(port));
			if (((port+1) % 8) == 0)
				cvmx_dprintf("\n");
		}
		cvmx_dprintf("\n");
	}
}
#endif /* PKO_DEBUG */

/*
 * Configure queues for an internal port.
 * @INTERNAL
 * @param pko_port PKO internal port number
 * Note: o68 only
 */
static void __cvmx_pko_iport_config(int pko_port)
{
	int queue, base_queue, num_queues;
	int static_priority_base;
	int static_priority_end;
	union cvmx_pko_mem_iqueue_ptrs config;
	uint64_t *buf_ptr = NULL;
	uint64_t priorities[CVMX_PKO_NQ_PER_PORT_MAX] = {
		[0 ... CVMX_PKO_NQ_PER_PORT_MAX - 1] = 8
	};
	int outputbuffer_pool = (int)cvmx_fpa_get_pko_pool();
	uint64_t outputbuffer_pool_size = cvmx_fpa_get_pko_pool_block_size();

	static_priority_base = -1;
	static_priority_end = -1;
	base_queue = __cvmx_helper_cfg_pko_queue_base(pko_port);
	num_queues = __cvmx_helper_cfg_pko_queue_num(pko_port);

	/*
	 * Give the user a chance to override the per queue priorities.
	 */
	if (cvmx_override_pko_queue_priority)
		cvmx_override_pko_queue_priority(pko_port, &priorities[0]);

	/*
	 * static queue priority validation
	 */
	for (queue = 0; queue < num_queues; queue++) {
		if (static_priority_base == -1 && priorities[queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY)
			static_priority_base = queue;

		if (static_priority_base != -1 && static_priority_end == -1 && priorities[queue] != CVMX_PKO_QUEUE_STATIC_PRIORITY && queue)
			static_priority_end = queue - 1;
		else if (static_priority_base != -1 && static_priority_end == -1 && queue == num_queues - 1)
			static_priority_end = queue;	/* all queues are static priority */

		/*
		 * Check to make sure all static priority queues are contiguous.
		 * Also catches some cases of static priorites not starting from
		 * queue 0.
		 */
		if (static_priority_end != -1 && (int)queue > static_priority_end && priorities[queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY) {
			cvmx_dprintf("ERROR: __cvmx_pko_iport_config: Static priority "
				     "queues aren't contiguous or don't start at base queue. " "q: %d, eq: %d\n", (int)queue, static_priority_end);
		}
		if (static_priority_base > 0) {
			cvmx_dprintf("ERROR: __cvmx_pko_iport_config: Static priority " "queues don't start at base queue. sq: %d\n", static_priority_base);
		}
	}

	/*
	 * main loop to set the fields of CVMX_PKO_MEM_IQUEUE_PTRS for
	 * each queue
	 */
	for (queue = 0; queue < num_queues; queue++) {
		config.u64 = 0;
		config.s.index = queue;
		config.s.qid = base_queue + queue;
		config.s.ipid = pko_port;
		config.s.tail = (queue == (num_queues - 1));
		config.s.s_tail = (queue == static_priority_end);
		config.s.static_p = (static_priority_base >= 0);
		config.s.static_q = (queue <= static_priority_end);

		/*
		 * Convert the priority into an enable bit field.
		 * Try to space the bits out evenly so the packet
		 * don't get grouped up.
		 */
		switch ((int)priorities[queue]) {
		case 0:
			config.s.qos_mask = 0x00;
			break;
		case 1:
			config.s.qos_mask = 0x01;
			break;
		case 2:
			config.s.qos_mask = 0x11;
			break;
		case 3:
			config.s.qos_mask = 0x49;
			break;
		case 4:
			config.s.qos_mask = 0x55;
			break;
		case 5:
			config.s.qos_mask = 0x57;
			break;
		case 6:
			config.s.qos_mask = 0x77;
			break;
		case 7:
			config.s.qos_mask = 0x7f;
			break;
		case 8:
			config.s.qos_mask = 0xff;
			break;
		case CVMX_PKO_QUEUE_STATIC_PRIORITY:
			config.s.qos_mask = 0xff;
			break;
		default:
			cvmx_dprintf("ERROR: __cvmx_pko_iport_config: " "Invalid priority %llu\n",
				     (unsigned long long)priorities[queue]);
			config.s.qos_mask = 0xff;
			break;
		}

		/*
		 * The command queues
		 */
		{
			cvmx_cmd_queue_result_t cmd_res;

			cmd_res = cvmx_cmd_queue_initialize(CVMX_CMD_QUEUE_PKO(base_queue + queue),
							    CVMX_PKO_MAX_QUEUE_DEPTH,
							    outputbuffer_pool,
							    (outputbuffer_pool_size - CVMX_PKO_COMMAND_BUFFER_SIZE_ADJUST * 8));

			if (cmd_res != CVMX_CMD_QUEUE_SUCCESS) {
				switch (cmd_res) {
				case CVMX_CMD_QUEUE_NO_MEMORY:
					cvmx_dprintf("ERROR: __cvmx_pko_iport_config: Unable to allocate output buffer.");
					break;
				case CVMX_CMD_QUEUE_ALREADY_SETUP:
					cvmx_dprintf("ERROR: __cvmx_pko_iport_config: Port already setup");
					break;
				case CVMX_CMD_QUEUE_INVALID_PARAM:
				default:
					cvmx_dprintf("ERROR: __cvmx_pko_iport_config: Command queue initialization failed.");
					break;
				}
				cvmx_dprintf(" pko_port%d base_queue%d num_queues%d queue%d.\n",
					     pko_port, base_queue, num_queues, queue);
			}

			buf_ptr = (uint64_t *) cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_PKO(base_queue + queue));
			config.s.buf_ptr = cvmx_ptr_to_phys(buf_ptr) >> 7;
		}

		CVMX_SYNCWS;
		cvmx_write_csr(CVMX_PKO_MEM_IQUEUE_PTRS, config.u64);
	}
}

/*
 * Allocate queues for the PKO internal ports.
 * @INTERNAL
 *
 */
static void __cvmx_pko_queue_alloc_o68(void)
{
	int port;

	pko_for_each_port(port) {
		__cvmx_pko_iport_config(port);
	}

}

/*
 * Allocate memory for PKO engines.
 *
 * @param engine is the PKO engine ID.
 * @return # of 2KB-chunks allocated to this PKO engine.
 */
static int __cvmx_pko_memory_per_engine_o68(int engine)
{
	/* CN68XX has 40KB to devide between the engines in 2KB chunks */
	int max_engine;
	int size_per_engine;
	int size;

	max_engine = __cvmx_helper_cfg_pko_max_engine();
	size_per_engine = 40 / 2 / max_engine;

	if (engine >= max_engine)
		/* Unused engines get no space */
		size = 0;
	else if (engine == max_engine - 1)
		/*
		 * The last engine gets all the space lost by rounding. This means
		 * the ILK gets the most space
		 */
		size = 40 / 2 - engine * size_per_engine;
	else
		/* All other engines get the same space */
		size = size_per_engine;

	return size;
}

/*
 * Setup one-to-one mapping between PKO iport and eport.
 * @INTERNAL
 */
static void __cvmx_pko_port_map_o68(void)
{
	int i;
	int interface, index, port;
	cvmx_helper_interface_mode_t mode;
	union cvmx_pko_mem_iport_ptrs config;

	/*
	 * Initialize every iport with the invalid eid.
	 */
#define CVMX_O68_PKO_INVALID_EID	31
	config.u64 = 0;
	config.s.eid = CVMX_O68_PKO_INVALID_EID;
	for (i = 0; i < CVMX_HELPER_CFG_MAX_PKO_PORT; i++) {
		config.s.ipid = i;
		cvmx_write_csr(CVMX_PKO_MEM_IPORT_PTRS, config.u64);
	}

	/*
	 * Set up PKO_MEM_IPORT_PTRS
	 */
	pko_for_each_port(port) {
		interface = __cvmx_helper_cfg_pko_port_interface(port);
		index = __cvmx_helper_cfg_pko_port_index(port);
		mode = cvmx_helper_interface_get_mode(interface);

		if (mode == CVMX_HELPER_INTERFACE_MODE_DISABLED)
			continue;

		config.s.ipid = port;
		config.s.qos_mask = 0xff;
		config.s.crc = __cvmx_helper_get_has_fcs(interface);
		config.s.min_pkt = __cvmx_helper_get_pko_padding(interface);
		config.s.intr = __cvmx_pko_int(interface, index);
		config.s.eid = __cvmx_helper_cfg_pko_port_eid(port);
		config.s.pipe = (mode == CVMX_HELPER_INTERFACE_MODE_LOOP) ? index : port;
		cvmx_write_csr(CVMX_PKO_MEM_IPORT_PTRS, config.u64);
	}
}

int __cvmx_pko_get_pipe(int interface, int index)
{
	/* The loopback ports do not have pipes */
	if (cvmx_helper_interface_get_mode(interface) == CVMX_HELPER_INTERFACE_MODE_LOOP)
		return -1;
	/* We use pko_port as the pipe. See __cvmx_pko_port_map_o68(). */
	return cvmx_helper_get_pko_port(interface, index);
}

/*
 * chip-specific setup
 * @INTERNAL
 */
static void __cvmx_pko_chip_init(void)
{
	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		__cvmx_pko_port_map_o68();
		__cvmx_pko_queue_alloc_o68();
	} else {
		int i;
		uint64_t priority = 8;

		/*Initialize queues. */
		for (i = 0; i < CVMX_PKO_MAX_OUTPUT_QUEUES; i++)
			cvmx_pko_config_port(CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID, i, 1, &priority);
	}
}

/**
 * Call before any other calls to initialize the packet
 * output system.  This does chip global config, and should only be
 * done by one core.
 */

void cvmx_pko_initialize_global(void)
{
	union cvmx_pko_reg_cmd_buf config;
	int i;
    	int outputbuffer_pool = (int)cvmx_fpa_get_pko_pool();
    	uint64_t outputbuffer_pool_size = cvmx_fpa_get_pko_pool_block_size();
	uint64_t outputbuffer_pool_count;

	outputbuffer_pool_count = cvmx_fpa_get_pko_pool_buffer_count();
	/** It allocate pools for pko command queues */
#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	cvmx_fpa_global_initialize();
	if(outputbuffer_pool_count != 0)
		__cvmx_helper_initialize_fpa_pool(outputbuffer_pool, outputbuffer_pool_size,
			outputbuffer_pool_count, "Pko Cmd Buffers");
#endif

#ifdef CVMX_BUILD_FOR_STANDALONE
	__cvmx_install_gmx_error_handler_for_xaui();
#endif
	__cvmx_helper_init_port_config_data();
	/*
	 * Set the size of the PKO command buffers to an odd number of
	 * 64bit words. This allows the normal two word send to stay
	 * aligned and never span a command word buffer.
	 */
	config.u64 = 0;
	config.s.pool = outputbuffer_pool;
	config.s.size = outputbuffer_pool_size / 8 - 1;
	cvmx_write_csr(CVMX_PKO_REG_CMD_BUF, config.u64);

	/* chip-specific setup. */
	__cvmx_pko_chip_init();

	/*
	 * If we aren't using all of the queues optimize PKO's
	 * internal memory.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN38XX) || OCTEON_IS_MODEL(OCTEON_CN58XX) ||
	    OCTEON_IS_MODEL(OCTEON_CN56XX) || OCTEON_IS_MODEL(OCTEON_CN52XX) ||
	    OCTEON_IS_OCTEON2() || OCTEON_IS_MODEL(OCTEON_CN70XX)) {
		int max_queues = __cvmx_helper_cfg_pko_max_queue();

		if (OCTEON_IS_MODEL(OCTEON_CN38XX)) {
			if (max_queues <= 32)
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_MODE, 2);
			else if (max_queues <= 64)
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_MODE, 1);
			else
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_MODE, 0);
		} else {
			if (OCTEON_IS_MODEL(OCTEON_CN68XX) && max_queues <= 32)
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_MODE, 3);
			else if (max_queues <= 64)
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_MODE, 2);
			else if (max_queues <= 128)
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_MODE, 1);
			else
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_MODE, 0);
			if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
				for (i = 0; i < 2; i++) {
					union cvmx_pko_reg_engine_storagex engine_storage;

#define PKO_ASSIGN_ENGINE_STORAGE(index)                        \
        engine_storage.s.engine##index =                        \
            __cvmx_pko_memory_per_engine_o68(16 * i + (index))

					engine_storage.u64 = 0;
					PKO_ASSIGN_ENGINE_STORAGE(0);
					PKO_ASSIGN_ENGINE_STORAGE(1);
					PKO_ASSIGN_ENGINE_STORAGE(2);
					PKO_ASSIGN_ENGINE_STORAGE(3);
					PKO_ASSIGN_ENGINE_STORAGE(4);
					PKO_ASSIGN_ENGINE_STORAGE(5);
					PKO_ASSIGN_ENGINE_STORAGE(6);
					PKO_ASSIGN_ENGINE_STORAGE(7);
					PKO_ASSIGN_ENGINE_STORAGE(8);
					PKO_ASSIGN_ENGINE_STORAGE(9);
					PKO_ASSIGN_ENGINE_STORAGE(10);
					PKO_ASSIGN_ENGINE_STORAGE(11);
					PKO_ASSIGN_ENGINE_STORAGE(12);
					PKO_ASSIGN_ENGINE_STORAGE(13);
					PKO_ASSIGN_ENGINE_STORAGE(14);
					PKO_ASSIGN_ENGINE_STORAGE(15);
					cvmx_write_csr(CVMX_PKO_REG_ENGINE_STORAGEX(i), engine_storage.u64);
				}
			}
		}
	}
}

/**
 * This function does per-core initialization required by the PKO routines.
 * This must be called on all cores that will do packet output, and must
 * be called after the FPA has been initialized and filled with pages.
 *
 * @return 0 on success
 *         !0 on failure
 */
int cvmx_pko_initialize_local(void)
{
	/* Nothing to do */
	return 0;
}

/**
 * Enables the packet output hardware. It must already be
 * configured.
 */
void cvmx_pko_enable(void)
{
	union cvmx_pko_reg_flags flags;

	flags.u64 = cvmx_read_csr(CVMX_PKO_REG_FLAGS);
	if (flags.s.ena_pko)
		cvmx_dprintf("Warning: Enabling PKO when PKO already enabled.\n");

	flags.s.ena_dwb = cvmx_helper_cfg_opt_get(CVMX_HELPER_CFG_OPT_USE_DWB);
	flags.s.ena_pko = 1;
	/*
	 * always enable big endian for 3-word command.  Does nothing
	 * for 2-word.
	 */
	flags.s.store_be = 1;
	cvmx_write_csr(CVMX_PKO_REG_FLAGS, flags.u64);
}

/**
 * Disables the packet output. Does not affect any configuration.
 */
void cvmx_pko_disable(void)
{
	union cvmx_pko_reg_flags pko_reg_flags;
	pko_reg_flags.u64 = cvmx_read_csr(CVMX_PKO_REG_FLAGS);
	pko_reg_flags.s.ena_pko = 0;
	cvmx_write_csr(CVMX_PKO_REG_FLAGS, pko_reg_flags.u64);
}

/**
 * @INTERNAL
 * Reset the packet output.
 */
static void __cvmx_pko_reset(void)
{
	union cvmx_pko_reg_flags pko_reg_flags;
	pko_reg_flags.u64 = cvmx_read_csr(CVMX_PKO_REG_FLAGS);
	pko_reg_flags.s.reset = 1;
	cvmx_write_csr(CVMX_PKO_REG_FLAGS, pko_reg_flags.u64);
}

/**
 * Shutdown and free resources required by packet output.
 */
void cvmx_pko_shutdown(void)
{
	int queue;

	cvmx_pko_disable();

	if (OCTEON_IS_MODEL(OCTEON_CN68XX)) {
		union cvmx_pko_mem_iqueue_ptrs config;
		config.u64 = 0;
		for (queue = 0; queue < CVMX_PKO_MAX_OUTPUT_QUEUES; queue++) {
			config.s.qid = queue;
			cvmx_write_csr(CVMX_PKO_MEM_IQUEUE_PTRS, config.u64);
			cvmx_cmd_queue_shutdown(CVMX_CMD_QUEUE_PKO(queue));
		}
	} else {
		union cvmx_pko_mem_queue_ptrs config;
		for (queue = 0; queue < CVMX_PKO_MAX_OUTPUT_QUEUES; queue++) {
			config.u64 = 0;
			config.s.tail = 1;
			config.s.index = 0;
			config.s.port = CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID;
			config.s.queue = queue & 0x7f;
			config.s.qos_mask = 0;
			config.s.buf_ptr = 0;
			if (!OCTEON_IS_MODEL(OCTEON_CN3XXX)) {
				union cvmx_pko_reg_queue_ptrs1 config1;
				config1.u64 = 0;
				config1.s.qid7 = queue >> 7;
				cvmx_write_csr(CVMX_PKO_REG_QUEUE_PTRS1, config1.u64);
			}
			cvmx_write_csr(CVMX_PKO_MEM_QUEUE_PTRS, config.u64);
			cvmx_cmd_queue_shutdown(CVMX_CMD_QUEUE_PKO(queue));
		}
	}

	__cvmx_pko_reset();
	cvmx_pko_queue_free_all();
}

/**
 * Configure a output port and the associated queues for use.
 *
 * @param port       Port to configure.
 * @param base_queue First queue number to associate with this port.
 * @param num_queues Number of queues to associate with this port
 * @param priority   Array of priority levels for each queue. Values are
 *                   allowed to be 0-8. A value of 8 get 8 times the traffic
 *                   of a value of 1.  A value of 0 indicates that no rounds
 *                   will be participated in. These priorities can be changed
 *                   on the fly while the pko is enabled. A priority of 9
 *                   indicates that static priority should be used.  If static
 *                   priority is used all queues with static priority must be
 *                   contiguous starting at the base_queue, and lower numbered
 *                   queues have higher priority than higher numbered queues.
 *                   There must be num_queues elements in the array.
 */
cvmx_pko_return_value_t cvmx_pko_config_port(uint64_t port, uint64_t base_queue,
				       uint64_t num_queues,
				       const uint64_t priority[])
{
	cvmx_pko_return_value_t result_code;
	uint64_t queue;
	union cvmx_pko_mem_queue_ptrs config;
	union cvmx_pko_reg_queue_ptrs1 config1;
	int static_priority_base = -1;
	int static_priority_end = -1;
	int outputbuffer_pool = (int)cvmx_fpa_get_pko_pool();
	uint64_t outputbuffer_pool_size = cvmx_fpa_get_pko_pool_block_size();

	if (OCTEON_IS_MODEL(OCTEON_CN68XX))
		return CVMX_PKO_SUCCESS;

	if ((port >= CVMX_PKO_NUM_OUTPUT_PORTS) &&
	    (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID)) {
		cvmx_dprintf("ERROR: cvmx_pko_config_port: Invalid port %llu\n",
			     (unsigned long long)port);
		return CVMX_PKO_INVALID_PORT;
	}

	if (base_queue + num_queues > CVMX_PKO_MAX_OUTPUT_QUEUES) {
		cvmx_dprintf("ERROR: cvmx_pko_config_port: Invalid queue range port = %lld base=%llu numques=%lld\n",
			     (unsigned long long) port, (unsigned long long) base_queue, (unsigned long long) num_queues);
		return CVMX_PKO_INVALID_QUEUE;
	}

	if (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID) {
		/*
		 * Validate the static queue priority setup and set
		 * static_priority_base and static_priority_end
		 * accordingly.
		 */
		for (queue = 0; queue < num_queues; queue++) {
			/* Find first queue of static priority */
			int p_queue = queue % 16;
			if (static_priority_base == -1 && priority[p_queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY)
				static_priority_base = queue;
			/* Find last queue of static priority */
			if (static_priority_base != -1 &&
			    static_priority_end == -1 &&
			    priority[p_queue] != CVMX_PKO_QUEUE_STATIC_PRIORITY &&
			    queue)
				static_priority_end = queue - 1;
			else if (static_priority_base != -1 &&
				 static_priority_end == -1 &&
				 queue == num_queues - 1)
				/* all queues're static priority */
				static_priority_end = queue;

			/*
			 * Check to make sure all static priority
			 * queues are contiguous.  Also catches some
			 * cases of static priorites not starting at
			 * queue 0.
			 */
			if (static_priority_end != -1 &&
			    (int)queue > static_priority_end &&
			    priority[p_queue] == CVMX_PKO_QUEUE_STATIC_PRIORITY) {
				cvmx_dprintf("ERROR: cvmx_pko_config_port: Static priority queues aren't contiguous or don't start at base queue. q: %d, eq: %d\n",
					     (int)queue, static_priority_end);
				return CVMX_PKO_INVALID_PRIORITY;
			}
		}
		if (static_priority_base > 0) {
			cvmx_dprintf("ERROR: cvmx_pko_config_port: Static priority queues don't start at base queue. sq: %d\n",
				     static_priority_base);
			return CVMX_PKO_INVALID_PRIORITY;
		}
	}

	/*
	 * At this point, static_priority_base and static_priority_end
	 * are either both -1, or are valid start/end queue numbers
	 */

	result_code = CVMX_PKO_SUCCESS;

	for (queue = 0; queue < num_queues; queue++) {
		uint64_t *buf_ptr = NULL;
		int p_queue = queue % 16;

		config1.u64 = 0;
		config1.s.idx3 = queue >> 3;
		config1.s.qid7 = (base_queue + queue) >> 7;

		config.u64 = 0;
		config.s.tail = queue == (num_queues - 1);
		config.s.index = queue;
		config.s.port = port;
		config.s.queue = base_queue + queue;

		config.s.static_p = static_priority_base >= 0;
		config.s.static_q = (int)queue <= static_priority_end;
		config.s.s_tail = (int)queue == static_priority_end;
		/*
		 * Convert the priority into an enable bit field. Try
		 * to space the bits out evenly so the packet don't
		 * get grouped up.
		 */
		switch ((int)priority[p_queue]) {
		case 0:
			config.s.qos_mask = 0x00;
			break;
		case 1:
			config.s.qos_mask = 0x01;
			break;
		case 2:
			config.s.qos_mask = 0x11;
			break;
		case 3:
			config.s.qos_mask = 0x49;
			break;
		case 4:
			config.s.qos_mask = 0x55;
			break;
		case 5:
			config.s.qos_mask = 0x57;
			break;
		case 6:
			config.s.qos_mask = 0x77;
			break;
		case 7:
			config.s.qos_mask = 0x7f;
			break;
		case 8:
			config.s.qos_mask = 0xff;
			break;
		case CVMX_PKO_QUEUE_STATIC_PRIORITY:
			config.s.qos_mask = 0xff;
			break;
		default:
			cvmx_dprintf("ERROR: cvmx_pko_config_port: Invalid priority %llu\n", (unsigned long long)priority[p_queue]);
			config.s.qos_mask = 0xff;
			result_code = CVMX_PKO_INVALID_PRIORITY;
			break;
		}

		if (port != CVMX_PKO_MEM_QUEUE_PTRS_ILLEGAL_PID) {
			cvmx_cmd_queue_result_t cmd_res;
			cmd_res = cvmx_cmd_queue_initialize(CVMX_CMD_QUEUE_PKO(base_queue + queue),
							    CVMX_PKO_MAX_QUEUE_DEPTH,
							    outputbuffer_pool,
							    outputbuffer_pool_size - CVMX_PKO_COMMAND_BUFFER_SIZE_ADJUST * 8);
			if (cmd_res != CVMX_CMD_QUEUE_SUCCESS) {
				switch (cmd_res) {
				case CVMX_CMD_QUEUE_NO_MEMORY:
					cvmx_dprintf("ERROR: cvmx_pko_config_port: " "Unable to allocate output buffer.\n");
					return (CVMX_PKO_NO_MEMORY);
				case CVMX_CMD_QUEUE_ALREADY_SETUP:
					cvmx_dprintf("ERROR: cvmx_pko_config_port: " "Port already setup. port=%d \n", (int) port);
					return (CVMX_PKO_PORT_ALREADY_SETUP);
				case CVMX_CMD_QUEUE_INVALID_PARAM:
				default:
					cvmx_dprintf("ERROR: cvmx_pko_config_port: " "Command queue initialization failed.\n");
					return (CVMX_PKO_CMD_QUEUE_INIT_ERROR);
				}
			}

			buf_ptr = (uint64_t *)cvmx_cmd_queue_buffer(CVMX_CMD_QUEUE_PKO(base_queue + queue));
			config.s.buf_ptr = cvmx_ptr_to_phys(buf_ptr);
		} else {
			config.s.buf_ptr = 0;
		}

		CVMX_SYNCWS;

		if (!OCTEON_IS_MODEL(OCTEON_CN3XXX)) {
			cvmx_write_csr(CVMX_PKO_REG_QUEUE_PTRS1, config1.u64);
		}
		cvmx_write_csr(CVMX_PKO_MEM_QUEUE_PTRS, config.u64);
	}

	return result_code;
}

/**
 * Rate limit a PKO port to a max packets/sec. This function is only
 * supported on CN51XX and higher, excluding CN58XX.
 *
 * @param port      Port to rate limit
 * @param packets_s Maximum packet/sec
 * @param burst     Maximum number of packets to burst in a row before rate
 *                  limiting cuts in.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_pko_rate_limit_packets(int port, int packets_s, int burst)
{
	union cvmx_pko_mem_port_rate0 pko_mem_port_rate0;
	union cvmx_pko_mem_port_rate1 pko_mem_port_rate1;

	pko_mem_port_rate0.u64 = 0;
	pko_mem_port_rate0.s.pid = port;
	pko_mem_port_rate0.s.rate_pkt = cvmx_clock_get_rate(CVMX_CLOCK_SCLK) / packets_s / 16;
	/* No cost per word since we are limited by packets/sec, not bits/sec */
	pko_mem_port_rate0.s.rate_word = 0;

	pko_mem_port_rate1.u64 = 0;
	pko_mem_port_rate1.s.pid = port;
	pko_mem_port_rate1.s.rate_lim = ((uint64_t) pko_mem_port_rate0.s.rate_pkt * burst) >> 8;

	cvmx_write_csr(CVMX_PKO_MEM_PORT_RATE0, pko_mem_port_rate0.u64);
	cvmx_write_csr(CVMX_PKO_MEM_PORT_RATE1, pko_mem_port_rate1.u64);
	return 0;
}

/**
 * Rate limit a PKO port to a max bits/sec. This function is only
 * supported on CN51XX and higher, excluding CN58XX.
 *
 * @param port   Port to rate limit
 * @param bits_s PKO rate limit in bits/sec
 * @param burst  Maximum number of bits to burst before rate
 *               limiting cuts in.
 *
 * @return Zero on success, negative on failure
 */
int cvmx_pko_rate_limit_bits(int port, uint64_t bits_s, int burst)
{
	union cvmx_pko_mem_port_rate0 pko_mem_port_rate0;
	union cvmx_pko_mem_port_rate1 pko_mem_port_rate1;
	uint64_t clock_rate = cvmx_clock_get_rate(CVMX_CLOCK_SCLK);
	uint64_t tokens_per_bit = clock_rate * 16 / bits_s;

	pko_mem_port_rate0.u64 = 0;
	pko_mem_port_rate0.s.pid = port;
	/*
	 * Each packet has a 12 bytes of interframe gap, an 8 byte
	 * preamble, and a 4 byte CRC. These are not included in the
	 * per word count. Multiply by 8 to covert to bits and divide
	 * by 256 for limit granularity.
	 */
	pko_mem_port_rate0.s.rate_pkt = (12 + 8 + 4) * 8 * tokens_per_bit / 256;
	/* Each 8 byte word has 64bits */
	pko_mem_port_rate0.s.rate_word = 64 * tokens_per_bit;

	pko_mem_port_rate1.u64 = 0;
	pko_mem_port_rate1.s.pid = port;
	pko_mem_port_rate1.s.rate_lim = tokens_per_bit * burst / 256;

	cvmx_write_csr(CVMX_PKO_MEM_PORT_RATE0, pko_mem_port_rate0.u64);
	cvmx_write_csr(CVMX_PKO_MEM_PORT_RATE1, pko_mem_port_rate1.u64);
	return 0;
}
