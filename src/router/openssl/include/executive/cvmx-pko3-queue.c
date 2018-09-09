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


/*
 * File version info: $Id$
 *
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/module.h>
#include <asm/octeon/cvmx.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-pko3-resources.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#include <asm/octeon/cvmx-bootmem.h>
#else
#include "cvmx.h"
#include "cvmx-pko3.h"
#include "cvmx-pko3-resources.h"
#include "cvmx-helper-pko3.h"
#include "cvmx-bootmem.h"
#endif

#if 0
#define DEBUG printf("PKO_EXAMPLE line:%d\n", __LINE__);
#else
#define DEBUG
#endif

#define CVMX_PKO_GET_QUEUE_TOPOLOGY(addr, field, count) \
	__cvmx_pko_get_queue_field(addr, offsetof(cvmx_pko_queue_topology_t, field) + \
					(count * sizeof(uint64_t)))
#define CVMX_PKO_SET_QUEUE_TOPOLOGY(addr, field, count, value) \
	__cvmx_pko_set_queue_field(addr, offsetof(cvmx_pko_queue_topology_t, field) + \
					(count * sizeof(uint64_t)), value)

#define CVMX_PKO_GET_PORT_QUEUE_INFO(addr, field) \
	__cvmx_pko_get_queue_field(addr, offsetof(cvmx_pko_port_queue_t, queue_info) + \
					offsetof(cvmx_pko_queue_info_t, field))
#define CVMX_PKO_SET_PORT_QUEUE_INFO(addr, field, value) \
	__cvmx_pko_set_queue_field(addr, offsetof(cvmx_pko_port_queue_t, queue_info) + \
					offsetof(cvmx_pko_queue_info_t, field), value)

#define CVMX_PKO_GET_QUEUE_INFO(addr, field) \
	__cvmx_pko_get_queue_field(addr, offsetof(cvmx_pko_queue_t, queue_info) + \
					offsetof(cvmx_pko_queue_info_t, field))

#define CVMX_PKO_SET_QUEUE_INFO(addr, field, value) \
	__cvmx_pko_set_queue_field(addr, offsetof(cvmx_pko_queue_t, queue_info) + \
					offsetof(cvmx_pko_queue_info_t, field), value)

#define CVMX_PKO_GET_PORT_QUEUE(addr, field) \
	__cvmx_pko_get_queue_field(addr, offsetof(cvmx_pko_port_queue_t, field))
#define CVMX_PKO_SET_PORT_QUEUE(addr, field, value) \
	__cvmx_pko_set_queue_field(addr, offsetof(cvmx_pko_port_queue_t, field), value)

#define CVMX_PKO_GET_QUEUE(addr, field) \
	__cvmx_pko_get_queue_field(addr, offsetof(cvmx_pko_queue_t, field))
#define CVMX_PKO_SET_QUEUE(addr, field, value) \
	__cvmx_pko_set_queue_field(addr, offsetof(cvmx_pko_queue_t, field), value)

#define CVMX_PKO_QUEUE_DATA_ADDR_NAME "cvmx-pko-queue-data"

/* points to cvmx_pko_queue_topology for all the nodes */
static CVMX_SHARED uint64_t __cvmx_pko_queue_data_addr = 0;

/* port queue list */
CVMX_SHARED int64_t cvmx_port_queue_list[CVMX_MAX_NODES][CVMX_PKO_MAX_MACS] = 
	{[0 ... CVMX_MAX_NODES - 1] = {[0 ... CVMX_PKO_MAX_MACS - 1] =  -1}};

/* Descriptor queues list */
static CVMX_SHARED struct cvmx_pko_dq_list pko_dq_list[CVMX_MAX_NODES][CVMX_PKO_NUM_PORT_INDEX];

static uint64_t __cvmx_pko_get_queue_data_addr(int node)
{
	if (__cvmx_pko_queue_data_addr == 0)
		__cvmx_pko_queue_initialize();

	return (__cvmx_pko_queue_data_addr + (node * sizeof(struct cvmx_pko_queue_topology)));
}

static uint64_t __cvmx_pko_get_queue_field(uint64_t base_addr, int offset)
{
	uint64_t addr = ((1ull << 63) | (base_addr + offset));
	return (cvmx_read64_uint64(addr));
}

static void __cvmx_pko_set_queue_field(uint64_t base_addr, int offset, uint64_t value)
{
	uint64_t addr = ((1ull << 63) | (base_addr + offset)); 
	cvmx_write64_uint64(addr, value);
}

static uint64_t __cvmx_pko_alloc_queue_node(void) 
{
	int size = sizeof(struct cvmx_pko_queue);
	void * tmp = NULL;

	/* TODO: allocate mem from a pool */
	tmp = cvmx_bootmem_alloc(size, CVMX_CACHE_LINE_SIZE);
	if (tmp) {
		memset(tmp, 0, size);
		return(cvmx_ptr_to_phys(tmp));
	}
	return 0;
}

static void __cvmx_pko_queue_add_next_node(uint64_t base_node, uint64_t new_node) 
{
	uint64_t next_node_addr = ((1ull << 63) | base_node);	
	cvmx_write64_uint64(next_node_addr, new_node);
}

static uint64_t __cvmx_pko_queue_get_next_node(uint64_t base_node) 
{
	uint64_t next_node_addr = ((1ull << 63) | base_node);	
	return(cvmx_read64_uint64(next_node_addr));
}

static void __cvmx_pko_queue_add_child_node(uint64_t parent_node, uint64_t new_child_node) 
{
	uint64_t child_node_addr = ((1ull << 63) | (parent_node + offsetof(struct cvmx_pko_queue_node, child_node)));
	cvmx_write64_uint64(child_node_addr, new_child_node);
}

static uint64_t __cvmx_pko_queue_get_child_node(uint64_t parent_node) 
{
	uint64_t child_node_addr = ((1ull << 63) | (parent_node + offsetof(struct cvmx_pko_queue_node, child_node)));
	return (cvmx_read64_uint64(child_node_addr));
}

/* lock for pko queue */
static void __cvmx_pko_queue_lock(int node)
{
	uint64_t lock_addr;
	unsigned int tmp;

	lock_addr = (1ull << 63) | (__cvmx_pko_get_queue_data_addr(node));

	__asm__ __volatile__(".set noreorder            \n"
			     "1: ll   %[tmp], 0(%[addr])\n"
			     "   bnez %[tmp], 1b        \n"
			     "   li   %[tmp], 1         \n"
			     "   sc   %[tmp], 0(%[addr])\n"
			     "   beqz %[tmp], 1b        \n"
			     "   nop                    \n"
			     ".set reorder              \n"
			     : [tmp] "=&r"(tmp)
			     : [addr] "r"(lock_addr)
			     : "memory");
}

/* Release the pko queue lock. */
static void __cvmx_pko_queue_unlock(int node)
{
	uint64_t lock_addr;

	lock_addr = (1ull << 63) | (__cvmx_pko_get_queue_data_addr(node));

	CVMX_SYNCW;
	__asm__ __volatile__("sw $0, 0(%[addr])\n"
			     : : [addr] "r"(lock_addr)
			     : "memory");
	CVMX_SYNCW;
}

/*
 * Initialize PKO queue data structures.
 *
 * @param none
 * @return returns 0 on success or -1 on failure.
 */
int __cvmx_pko_queue_initialize(void) 
{
	cvmx_bootmem_named_block_desc_t *block_desc;
	int size = sizeof(cvmx_pko_queue_topology_t) * CVMX_MAX_NODES;
	int res = 0, count;
	int64_t phys;
	uint64_t base;

	cvmx_bootmem_lock();

	block_desc = (cvmx_bootmem_named_block_desc_t *)
		__cvmx_bootmem_find_named_block_flags(CVMX_PKO_QUEUE_DATA_ADDR_NAME,
                                                      CVMX_BOOTMEM_FLAG_NO_LOCKING);

	if (!block_desc) {

		phys = cvmx_bootmem_phy_named_block_alloc(size, 0, 0, CVMX_CACHE_LINE_SIZE,
							  CVMX_PKO_QUEUE_DATA_ADDR_NAME,
							  CVMX_BOOTMEM_FLAG_NO_LOCKING);

		if (phys < 0) {
			cvmx_dprintf("Error: Failed to allocate pko queue data named block\n");
			res = -1;
			goto end;
		}
		
		__cvmx_pko_queue_data_addr = (uint64_t) phys;

		base = (1ull << 63) | __cvmx_pko_queue_data_addr;
		for (count = 0; count < (size/8); count++) {
			cvmx_write64_uint64(base, 0);
			base += 8;
		}
	} else {
		__cvmx_pko_queue_data_addr = block_desc->base_addr;
	}
 end:
	cvmx_bootmem_unlock();
	return res;

}

/*
 * Configure Port Queue for Static priority child queues.
 *
 * @param node is to specify the node on which port queue is present.
 * @param interface is the interface for which port queue will be configured.
 * @param port is the port number for which port queue will be configured.
 * @param child_base is the first queue number for child static queues(specify -1 for api to allocate) 
 * @param lowest_static_priority is the lowest static priority input(higher in value) connected to the scheduler. 
 * @return returns start of child static queues(child_base) if successful or -1 on failure.
 */
int cvmx_pko_port_queue_setup_static_childs(int node, int interface, int port, 
					    int child_base,
		                            int lowest_static_prio)
{
	int port_queue, child_queue;
	int num_queues = lowest_static_prio + 1;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l2_head, l2_queue, l2_tail, num_l2_childs;
	int size;
	void * tmp;
	int prio = 0, ret_val = -1;

	/* TODO : assert if not proper values */

	/* get port queue for this interface/port */
	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	/* allocate or reserve level 2 queues */
	child_base = cvmx_pko_alloc_queues(node, CVMX_PKO_L2_QUEUES, port_queue,
					   child_base, num_queues);
	if (child_base < 0) {
		cvmx_dprintf ("Failed to allocate or reserve level 2 queues\n");
		return -1;
	}
	child_queue = child_base;

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		/* TODO: alloc memory from pool */
		size = sizeof(struct cvmx_pko_port_queue);

		tmp = cvmx_bootmem_alloc(size, CVMX_CACHE_LINE_SIZE);
		if (!tmp) {
			cvmx_dprintf("ERROR: unable to get memory size=%d\n", size);
			goto end;
		}
		port_queue_addr = cvmx_ptr_to_phys(tmp);
		CVMX_PKO_SET_QUEUE_TOPOLOGY(queue_topology_base_addr, 
					    port_queue_addr, port_queue, 
					    port_queue_addr);
		/* setup initial queue info */
		CVMX_PKO_SET_PORT_QUEUE_INFO(port_queue_addr, queue_num, port_queue);
	}

	/* setup other queue info */
	CVMX_PKO_SET_PORT_QUEUE_INFO(port_queue_addr, child_static_base, child_queue);

	/* fill l2 queue info */
	num_l2_childs = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, num_l2_childs);
	num_l2_childs += num_queues;

	CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, num_l2_childs, num_l2_childs);

	l2_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l2_head);
	l2_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l2_tail);

	if (!l2_head) {
		l2_head = l2_tail = __cvmx_pko_alloc_queue_node();
		if (!l2_head) {
			cvmx_dprintf("ERROR: Failed to get mem of size %d\n", size);
			goto end;
		}
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, parent_queue, port_queue);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_prio, prio);
		child_queue++;
		num_queues--;
		prio++;
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l2_head, l2_head);
	}

	/* add these childs at the end of l2 list */
        while (num_queues != 0) {
		l2_queue = __cvmx_pko_alloc_queue_node();
		if (!l2_queue) {
			cvmx_dprintf("ERROR: Failed to get mem of size %d\n", size);
			goto end;
		}

		/* add this to the list */
		__cvmx_pko_queue_add_next_node(l2_tail, l2_queue);
		l2_tail = l2_queue;
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, parent_queue, port_queue);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_prio, prio);
		child_queue++;
		num_queues--;
		prio++;
	}

	CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l2_tail, l2_tail);

	ret_val = child_base;

end:
	__cvmx_pko_queue_unlock(node);
	return ret_val;
}


/*
 * Configure Port Queue for round robin child queues.
 *
 * @param node is to specify the node on which port queue is present.
 * @param interface is the interface for which port queue will be configured.
 * @param port is the port number for which port queue will be configured.
 * @param rr_queues is the array specifying child rr queue numbers(specify -1 for api to allocate) 
 * @param rr_quantum is the array specifying child rr queue quantum values.
 * @param num_rr_queues is the total number of rr child queues in rr_queues. 
 * @param rr_prio is the round robin priority assigned to rr_queues.
 * @return returns 0 if successful or -1 on failure.
 */
int cvmx_pko_port_queue_setup_rr_childs(int node, int interface, int port, 
					int rr_queues[], int rr_quantum[],
					int num_rr_queues, int rr_prio)
{
	int port_queue;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l2_head, l2_tail, l2_queue;
	int num_l2_childs;
	int ret_val = -1, queue_num = -1;
	int idx = 0;
	int num_queues = num_rr_queues;
	

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		/* TODO: alloc memory from pool */
		int size;
		void * tmp;

		size = sizeof(struct cvmx_pko_port_queue);

		tmp = cvmx_bootmem_alloc(size, CVMX_CACHE_LINE_SIZE);
		port_queue_addr = cvmx_ptr_to_phys(tmp);
		if (!port_queue_addr) {
			cvmx_dprintf("ERROR: unable to get memory size=%d\n", size);
			goto end;
		}
		CVMX_PKO_SET_QUEUE_TOPOLOGY(queue_topology_base_addr, 
					    port_queue_addr, port_queue, 
					    port_queue_addr);

		/* setup initial queue info */
		CVMX_PKO_SET_PORT_QUEUE_INFO(port_queue_addr, queue_num, port_queue);
	}

	l2_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l2_head);
	l2_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l2_tail);
	num_l2_childs = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, num_l2_childs);

	/* check if there is a prev. configured static queue at rr_prio and convert it into rr */
	if(num_l2_childs >= rr_prio+1) {
		int rr_queue_num = 0;
		int num_childs = num_l2_childs;
		int queue_base = 0;

		queue_base = CVMX_PKO_GET_PORT_QUEUE_INFO(port_queue_addr, child_static_base);
		rr_queue_num = queue_base + rr_prio;

		l2_queue = l2_head;
		queue_num = -1;
		while (num_childs != 0) {
			queue_num = CVMX_PKO_GET_QUEUE_INFO(l2_queue, queue_num);
			if (queue_num == rr_queue_num)
				break;
			else
				queue_num = -1;
			l2_queue = __cvmx_pko_queue_get_next_node(l2_queue);
			num_childs--;
		}

		if (queue_num != -1) {
			if (rr_queue_num == queue_base) {
				/* change child static base in port queue */
				CVMX_PKO_SET_PORT_QUEUE_INFO(port_queue_addr, child_static_base, queue_base+1);
			}
			/* rr_queue_num will be filled in rr_queues */
			if (rr_queues[idx] == -1) {
				rr_queues[idx] = rr_queue_num;
				CVMX_PKO_SET_QUEUE_INFO(l2_queue, sched_algo, CVMX_PKO_RR);
				CVMX_PKO_SET_QUEUE_INFO(l2_queue, rr_quantum, rr_quantum[idx]);
				idx++;
				num_queues--;
			}
		}
	}

	/* reserve or allocate queues from global resources */
 	ret_val = cvmx_pko_alloc_queues_many(node, CVMX_PKO_L2_QUEUES, port_queue,
			                     &rr_queues[idx], num_queues);

	if (ret_val < 0) {
		cvmx_dprintf ("Error: Failed to allocate pko level2 queues\n");
		goto end;
	}

	/* set child_rr_prio in port queue */
	CVMX_PKO_SET_PORT_QUEUE_INFO(port_queue_addr, child_rr_prio, rr_prio);

	if (!l2_head) {
		l2_head = l2_tail = __cvmx_pko_alloc_queue_node();
		if (!l2_head) {
			cvmx_dprintf("ERROR: Failed to get mem\n");
			goto end;
		}
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, parent_queue, port_queue);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, rr_quantum, rr_quantum[idx]);
		idx++;

		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l2_head, l2_head);
	}

	/* add these childs at the end of l2 list */
        while (idx < num_rr_queues) {
		l2_queue = __cvmx_pko_alloc_queue_node();
		if (!l2_queue) {
			cvmx_dprintf("ERROR: Failed to get mem\n");
			goto end;
		}

		/* add this queue to the list */
		__cvmx_pko_queue_add_next_node(l2_tail, l2_queue);
		l2_tail = l2_queue;
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, parent_queue, port_queue);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l2_tail, rr_quantum, rr_quantum[idx]);
		idx++;
	}

	CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l2_tail, l2_tail);

	/* update number of childs in parent queue */
	num_l2_childs += num_rr_queues;
	CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, num_l2_childs, num_l2_childs);

	ret_val = 0;
end:
	__cvmx_pko_queue_unlock(node);
	return ret_val;
}

/*
 * Configure level 2 queue for Static priority child queues.
 *
 * @param node is to specify the node on which l2 queue is present.
 * @param interface is the interface for which level2 queue will be configured.
 * @param port is the port number for which level2 queue will be configured.
 * @param queue is the l2 queue number to configure.
 * @param child_base is the first queue number for child static queues(specify -1 for api to allocate) 
 * @param lowest_static_priority is the lowest static priority input(higher in value) connected to the scheduler. 
 * @return returns start of child static queues if successful or -1 on failure.
 */
int cvmx_pko_l2_queue_setup_static_childs(int node, int interface, int port, 
					  int queue, 
					  int child_base, 
					  int lowest_static_prio)
{
	int child_queue, port_queue;
	int num_queues = lowest_static_prio + 1;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l2_head, l2_queue, num_l2_childs;
	uint64_t l3_head, l3_tail, l3_queue, l3_child, l3_prev;
	uint64_t num_l3_childs;
	int prio = 0, update_tail = 0;
	int queue_num = -1, ret_val = -1;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	/* allocate or reserve level 3 queues */
	child_base = cvmx_pko_alloc_queues(node, CVMX_PKO_L3_QUEUES, port_queue,
					   child_base, num_queues);
	if (child_base < 0) {
		cvmx_dprintf ("Failed to allocate or reserve level 3 queues\n");
		return -1;
	}
	child_queue = child_base;

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l2_head, num_l2_childs */
	l2_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l2_head);
	if (!l2_head) {
		cvmx_dprintf("ERROR: l2 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}
	num_l2_childs = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, num_l2_childs);

	/* find l2 parent in the l2 list */
	l2_queue = l2_head;
	while (num_l2_childs != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l2_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l2_queue = __cvmx_pko_queue_get_next_node(l2_queue);
		num_l2_childs--;
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l2 queue %d is not configured\n", queue);
		goto end;
	}

	/* get l3 queue list details */
	l3_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l3_tail);
	l3_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l3_head);

	l3_child = __cvmx_pko_queue_get_child_node(l2_queue);
	num_l3_childs = CVMX_PKO_GET_QUEUE(l2_queue, num_childs);

	/* add l3 child base node for l2 queue if not present */
	if (!l3_child) {
		l3_child = __cvmx_pko_alloc_queue_node();
		if (!l3_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set l3 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l3_child, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l3_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l3_child, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l3_child, sched_prio, prio);

		/* add l3 child base node to l2 parent queue */
		__cvmx_pko_queue_add_child_node(l2_queue, l3_child);
		CVMX_PKO_SET_QUEUE_INFO(l2_queue, child_static_base, child_queue);

		child_queue++;
		num_queues--;
		prio++;
		num_l3_childs = 1;
		
		if (!l3_head) {
			l3_head = l3_tail = l3_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l3_head, l3_head);
		}
		else {
			/* add new queue to the existing l3 list */
			__cvmx_pko_queue_add_next_node(l3_tail, l3_child);
		}
		update_tail = 1;
	} else {
		/* childs(RR) already exist for this l2_queue */
		l3_prev = __cvmx_pko_queue_get_next_node(l3_child);
	}

	/* add remaining childs to the l3 list */
	while (num_queues != 0) {
		l3_queue = __cvmx_pko_alloc_queue_node();
		if (!l3_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the l3 list */
		__cvmx_pko_queue_add_next_node(l3_child, l3_queue);

		/* set l3 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, sched_prio, prio);
		l3_child = l3_queue;
		child_queue++;
		num_queues--;
		prio++;
		num_l3_childs++;
	}

	if (update_tail) {
		/* new childs are added at end of list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l3_tail, l3_child);
	}
	else {
		/* add prev nodes back into the the l3 list */
		__cvmx_pko_queue_add_next_node(l3_child, l3_prev);
	}

	/* update number of l3 childs in l2 parent queue */
	CVMX_PKO_SET_QUEUE(l2_queue, num_childs, num_l3_childs);

	ret_val = child_base;

end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Configure level 2 queue for round robin child queues.
 *
 * @param node is to specify the node on which l2 queue is present.
 * @param interface is the interface for which level2 queue will be configured.
 * @param port is the port number for which level2 queue will be configured.
 * @param queue is the l2 queue number to configure.
 * @param rr_queues is the array specifying child rr queue numbers(specify -1 for api to allocate) 
 * @param rr_quantum is the array specifying child rr queue quantum values.
 * @param num_rr_queues is the total number of rr child queues in rr_queues. 
 * @param rr_prio is the round robin priority assigned to rr_queues.
 * @return returns 0 if successful or -1 on failure.
 */
int cvmx_pko_l2_queue_setup_rr_childs(int node, int interface, int port, 
				      int queue, int rr_queues[],
				      int rr_quantum[], int num_rr_queues,
				      int rr_prio)
{
	int port_queue;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l2_head, l2_queue, num_l2_childs; 
	uint64_t l3_head, l3_tail, l3_queue, l3_child, l3_prev;
	int num_l3_childs;
	int queue_num = -1, ret_val = -1;
	int update_tail = 0;
	int idx = 0;
	int num_queues = num_rr_queues;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l2_head, num_l2_childs */
	l2_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l2_head);
	if (!l2_head) {
		cvmx_dprintf("ERROR: l2 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}
	num_l2_childs = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, num_l2_childs);

	/* find l2 parent in the l2 list */
	l2_queue = l2_head;
	while (num_l2_childs != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l2_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l2_queue = __cvmx_pko_queue_get_next_node(l2_queue);
		num_l2_childs--;
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l2 queue %d is not configured\n", queue);
		goto end;
	}

	/* get child base for l2 parent queue, if prev configured */
	l3_child = __cvmx_pko_queue_get_child_node(l2_queue);
	num_l3_childs = CVMX_PKO_GET_QUEUE(l2_queue, num_childs);

	/* check if there is a prev. configured static queue at rr_prio and convert it into rr */
	if(num_l3_childs >= rr_prio+1) {
		int rr_queue_num = 0;
		int num_childs = num_l3_childs;
		int queue_base = 0;

		queue_base = CVMX_PKO_GET_QUEUE_INFO(l2_queue, child_static_base);
		rr_queue_num = queue_base + rr_prio;

		l3_queue = l3_child;
		queue_num = -1;
		while (num_childs != 0) {
			queue_num = CVMX_PKO_GET_QUEUE_INFO(l3_queue, queue_num);
			if (queue_num == rr_queue_num)
				break;
			else
				queue_num = -1;
			l3_queue = __cvmx_pko_queue_get_next_node(l3_queue);
			num_childs--;
		}

		if (queue_num != -1) {
			if (rr_queue_num == queue_base) {
				/* change l2 child static base */
				CVMX_PKO_SET_QUEUE_INFO(l2_queue, child_static_base, queue_base+1);
			}
			/* rr_queue_num will be filled in rr_queues */
			if (rr_queues[idx] == -1) {
				rr_queues[idx] = rr_queue_num;
				CVMX_PKO_SET_QUEUE_INFO(l3_queue, sched_algo, CVMX_PKO_RR);
				CVMX_PKO_SET_QUEUE_INFO(l3_queue, rr_quantum, rr_quantum[idx]);
				idx++;
				num_queues--;
			}
		}
	}

	/* reserve or allocate queues from global resources */
 	ret_val = cvmx_pko_alloc_queues_many(node, CVMX_PKO_L3_QUEUES, port_queue,
			                     &rr_queues[idx], num_queues);

	if (ret_val < 0) {
		cvmx_dprintf ("Error: Failed to allocate pko level3 queues\n");
		goto end;
	}

	/* get l3 queue list details */
	l3_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l3_tail);
	l3_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l3_head);

	/* add l3 child base node for l2 queue if not present */
	if (!l3_child) {
		l3_child = __cvmx_pko_alloc_queue_node();
		if (!l3_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set l3 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l3_child, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l3_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l3_child, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l3_child, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l3_child, rr_quantum, rr_quantum[idx]);

		/* add l3 child base node to l2 parent queue */
		__cvmx_pko_queue_add_child_node(l2_queue, l3_child);

		idx++;
		num_l3_childs = 1;

		if (!l3_head) {
			l3_head = l3_tail = l3_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l3_head, l3_head);
		}
		else {
			/* add new queue to the existing l3 list */
			__cvmx_pko_queue_add_next_node(l3_tail, l3_child);
		}
		update_tail = 1;
	} else {
		/* childs already exist for this l2_queue */
		l3_prev = __cvmx_pko_queue_get_next_node(l3_child);
	}

	/* add remaining childs to the l3 list */
        while (idx < num_rr_queues) {
		l3_queue = __cvmx_pko_alloc_queue_node();
		if (!l3_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the l3 list */
		__cvmx_pko_queue_add_next_node(l3_child, l3_queue);

		/* set l3 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, rr_quantum, rr_quantum[idx]);
		l3_child = l3_queue;
		num_l3_childs++;
		idx++;
	}

	if (update_tail) {
		/* new childs are added at end of l3 list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l3_tail, l3_child);
	}
	else {
		/* add prev nodes back into the the l3 list */
		__cvmx_pko_queue_add_next_node(l3_child, l3_prev);
	}

	/* update number of l3 childs in l2 parent queue */
	CVMX_PKO_SET_QUEUE(l2_queue, num_childs, num_l3_childs);

	ret_val = 0;
end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Configure level 3 queue for Static priority child queues.
 *
 * @param node is to specify the node on which l3 queue is present.
 * @param interface is the interface for which level3 queue will be configured.
 * @param port is the port number for which level3 queue will be configured.
 * @param queue is the l3 queue number to configure.
 * @param child_base is the start of child static queues(specify -1 for api to allocate) 
 * @param lowest_static_priority is the lowest static priority input(higher in value) connected to the  scheduler. 
 * @return returns start of child static queues if successful or -1 on failure.
 */
int cvmx_pko_l3_queue_setup_static_childs(int node, int interface, int port, 
					  int queue, int child_base,
		                          int lowest_static_prio)
{
	int child_queue, port_queue;
	int num_queues = lowest_static_prio + 1;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l3_head, l3_queue;
	uint64_t l4_head, l4_tail, l4_queue, l4_child, l4_prev;
	uint64_t num_l4_childs;
	int prio = 0, update_tail = 0;
	int queue_num = -1, ret_val = -1;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	/* allocate or reserve level 4 queues */
	child_base = cvmx_pko_alloc_queues(node, CVMX_PKO_L4_QUEUES, port_queue,
					   child_base, num_queues);
	if (child_base < 0) {
		cvmx_dprintf ("Failed to allocate or reserve level 4 queues\n");
		return -1;
	}
	child_queue = child_base;

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l3_head, l3_tail */
	l3_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l3_head);
	if (!l3_head) {
		cvmx_dprintf("ERROR: l3 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}

	/* find l3 parent in the l3 list */
	l3_queue = l3_head;
	while (l3_queue != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l3_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l3_queue = __cvmx_pko_queue_get_next_node(l3_queue);
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l3 queue %d is not configured\n", queue);
		goto end;
	}

	/* get l4 queue list details */
	l4_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l4_tail);
	l4_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l4_head);

	l4_child = __cvmx_pko_queue_get_child_node(l3_queue);
	num_l4_childs = CVMX_PKO_GET_QUEUE(l3_queue, num_childs);

	/* add l4 child base node for l3 queue if not present */
	if (!l4_child) {
		l4_child = __cvmx_pko_alloc_queue_node();
		if (!l4_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set l4 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l4_child, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l4_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l4_child, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l4_child, sched_prio, prio);

		/* add l4 child base node to l3 parent queue */
		__cvmx_pko_queue_add_child_node(l3_queue, l4_child);
		CVMX_PKO_SET_QUEUE_INFO(l3_queue, child_static_base, child_queue);

		child_queue++;
		num_queues--;
		prio++;
		num_l4_childs = 1;
		
		if (!l4_head) {
			l4_head = l4_tail = l4_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l4_head, l4_head);
		}
		else {
			/* add new queue to the existing l4 list */
			__cvmx_pko_queue_add_next_node(l4_tail, l4_child);
		}
		update_tail = 1;
	} else {
		/* childs(RR) already exist for this l3_queue */
		l4_prev = __cvmx_pko_queue_get_next_node(l4_child);
	}

	/* add remaining childs to the l4 list */
	while (num_queues != 0) {
		l4_queue = __cvmx_pko_alloc_queue_node();
		if (!l4_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the l4 list */
		__cvmx_pko_queue_add_next_node(l4_child, l4_queue);

		/* set l3 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, sched_prio, prio);
		l4_child = l4_queue;
		child_queue++;
		num_queues--;
		prio++;
		num_l4_childs++;
	}

	if (update_tail) {
		/* new childs are added at end of list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l4_tail, l4_child);
	}
	else {
		/* add prev nodes back into the the l4 list */
		__cvmx_pko_queue_add_next_node(l4_child, l4_prev);
	}

	/* update number of l4 childs in l3 parent queue */
	CVMX_PKO_SET_QUEUE(l3_queue, num_childs, num_l4_childs);

	ret_val = child_base;

end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Configure level 3 queue for round robin child queues.
 *
 * @param node is to specify the node on which l3 queue is present.
 * @param interface is the interface for which level3 queue will be configured.
 * @param port is the port number for which level3 queue will be configured.
 * @param queue is the l3 queue number to configure.
 * @param rr_queues is the array specifying child rr queue numbers(specify -1 for api to allocate) 
 * @param rr_quantum is the array specifying child rr queue quantum values.
 * @param num_rr_queues is the total number of rr child queues in rr_queues. 
 * @param rr_prio is the round robin priority assigned to rr_queues.
 * @return returns 0 if successful or -1 on failure.
 */
int cvmx_pko_l3_queue_setup_rr_childs(int node, int interface, int port, 
				      int queue, int rr_queues[],
            			      int rr_quantum[], int num_rr_queues,
				      int rr_prio)
{
	int port_queue;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l3_head, l3_queue;  
	uint64_t l4_head, l4_tail, l4_queue, l4_child, l4_prev;
	int num_l4_childs;
	int queue_num = -1, ret_val = -1;
	int update_tail = 0;
	int idx = 0;
	int num_queues = num_rr_queues;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l3_head, num_l3_childs */
	l3_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l3_head);
	if (!l3_head) {
		cvmx_dprintf("ERROR: l3 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}

	/* find l3 parent in the l3 list */
	l3_queue = l3_head;
	while (l3_queue != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l3_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l3_queue = __cvmx_pko_queue_get_next_node(l3_queue);
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l3 queue %d is not configured\n", queue);
		goto end;
	}

	/* get child base for l3 parent queue, if prev configured */
	l4_child = __cvmx_pko_queue_get_child_node(l3_queue);
	num_l4_childs = CVMX_PKO_GET_QUEUE(l3_queue, num_childs);

	/* check if there is a prev. configured static queue at rr_prio and convert it into rr */
	if(num_l4_childs >= rr_prio+1) {
		int rr_queue_num = 0;
		int num_childs = num_l4_childs;
		int queue_base = 0;

		queue_base = CVMX_PKO_GET_QUEUE_INFO(l3_queue, child_static_base);
		rr_queue_num = queue_base + rr_prio;

		l4_queue = l4_child;
		queue_num = -1;
		while (num_childs != 0) {
			queue_num = CVMX_PKO_GET_QUEUE_INFO(l4_queue, queue_num);
			if (queue_num == rr_queue_num)
				break;
			else
				queue_num = -1;
			l4_queue = __cvmx_pko_queue_get_next_node(l4_queue);
			num_childs--;
		}

		if (queue_num != -1) {
			if (rr_queue_num == queue_base) {
				/* change l3 child static base */
				CVMX_PKO_SET_QUEUE_INFO(l3_queue, child_static_base, queue_base+1);
			}
			/* rr_queue_num will be filled in rr_queues */
			if (rr_queues[idx] == -1) {
				rr_queues[idx] = rr_queue_num;
				CVMX_PKO_SET_QUEUE_INFO(l4_queue, sched_algo, CVMX_PKO_RR);
				CVMX_PKO_SET_QUEUE_INFO(l4_queue, rr_quantum, rr_quantum[idx]);
				idx++;
				num_queues--;
			}
		}
	}

	/* reserve or allocate queues from global resources */
 	ret_val = cvmx_pko_alloc_queues_many(node, CVMX_PKO_L4_QUEUES, port_queue,
			                     &rr_queues[idx], num_queues);

	if (ret_val < 0) {
		cvmx_dprintf ("Error: Failed to allocate pko level4 queues\n");
		goto end;
	}

	/* get l4 queue list details */
	l4_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l4_tail);
	l4_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l4_head);

	/* add l4 child base node for l3 queue if not present */
	if (!l4_child) {
		l4_child = __cvmx_pko_alloc_queue_node();
		if (!l4_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set l4 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l4_child, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l4_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l4_child, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l4_child, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l4_child, rr_quantum, rr_quantum[idx]);

		/* add l4 child base node to l3 parent queue */
		__cvmx_pko_queue_add_child_node(l3_queue, l4_child);

		idx++;
		num_l4_childs = 1;

		if (!l4_head) {
			l4_head = l4_tail = l4_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l4_head, l4_head);
		}
		else {
			/* add new queue to the existing l4 list */
			__cvmx_pko_queue_add_next_node(l4_tail, l4_child);
		}
		update_tail = 1;
	} else {
		/* childs already exist for this l3_queue */
		l4_prev = __cvmx_pko_queue_get_next_node(l4_child);
	}

	/* add remaining childs to the l4 list */
        while (idx < num_rr_queues) {
		l4_queue = __cvmx_pko_alloc_queue_node();
		if (!l4_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the l4 list */
		__cvmx_pko_queue_add_next_node(l4_child, l4_queue);

		/* set l4 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, rr_quantum, rr_quantum[idx]);
		l4_child = l4_queue;
		num_l4_childs++;
		idx++;
	}

	if (update_tail) {
		/* new childs are added at end of l4 list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l4_tail, l4_child);
	}
	else {
		/* add prev nodes back into the the l4 list */
		__cvmx_pko_queue_add_next_node(l4_child, l4_prev);
	}

	/* update number of l4 childs in l3 parent queue */
	CVMX_PKO_SET_QUEUE(l3_queue, num_childs, num_l4_childs);

	ret_val = 0;
end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Configure level 4 queue for Static priority child queues.
 *
 * @param node is to specify the node on which l4 queue is present.
 * @param interface is the interface for which level4 queue will be configured.
 * @param port is the port number for which level4 queue will be configured.
 * @param queue is the l4 queue number to configure.
 * @param child_base is the start of child static queues(specify -1 for api to allocate) 
 * @param lowest_static_priority is the lowest static priority input(higher in value) connected to the  scheduler. 
 * @return returns start of child static queues if successful or  -1 on failure.
 */
int cvmx_pko_l4_queue_setup_static_childs(int node, int interface, int port, 
					  int queue, int child_base,
		                          int lowest_static_prio)
{
	int child_queue, port_queue;
	int num_queues = lowest_static_prio + 1;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l4_head, l4_queue;
	uint64_t l5_head, l5_tail, l5_queue, l5_child, l5_prev;
	uint64_t num_l5_childs;
	int prio = 0, update_tail = 0;
	int queue_num = -1, ret_val = -1;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	/* allocate or reserve level 5 queues */
	child_base = cvmx_pko_alloc_queues(node, CVMX_PKO_L5_QUEUES, port_queue,
					   child_base, num_queues);
	if (child_base < 0) {
		cvmx_dprintf ("Failed to allocate or reserve level 5 queues\n");
		return -1;
	}
	child_queue = child_base;

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l4_head */
	l4_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l4_head);
	if (!l4_head) {
		cvmx_dprintf("ERROR: l4 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}

	/* find l4 parent in the l4 list */
	l4_queue = l4_head;
	while (l4_queue != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l4_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l4_queue = __cvmx_pko_queue_get_next_node(l4_queue);
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l4 queue %d is not configured\n", queue);
		goto end;
	}

	/* get l5 queue list details */
	l5_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l5_tail);
	l5_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l5_head);

	l5_child = __cvmx_pko_queue_get_child_node(l4_queue);
	num_l5_childs = CVMX_PKO_GET_QUEUE(l4_queue, num_childs);

	/* add l5 child base node for l4 queue if not present */
	if (!l5_child) {
		l5_child = __cvmx_pko_alloc_queue_node();
		if (!l5_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set l5 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l5_child, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l5_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l5_child, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l5_child, sched_prio, prio);

		/* add l5 child base node to l4 parent queue */
		__cvmx_pko_queue_add_child_node(l4_queue, l5_child);
		CVMX_PKO_SET_QUEUE_INFO(l4_queue, child_static_base, child_queue);

		child_queue++;
		num_queues--;
		prio++;
		num_l5_childs = 1;
		
		if (!l5_head) {
			l5_head = l5_tail = l5_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l5_head, l5_head);
		}
		else {
			/* add new queue to the existing l5 list */
			__cvmx_pko_queue_add_next_node(l5_tail, l5_child);
		}
		update_tail = 1;
	} else {
		/* childs(RR) already exist for this l4_queue */
		l5_prev = __cvmx_pko_queue_get_next_node(l5_child);
	}

	/* add remaining childs to the l5 list */
	while (num_queues != 0) {
		l5_queue = __cvmx_pko_alloc_queue_node();
		if (!l5_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the l5 list */
		__cvmx_pko_queue_add_next_node(l5_child, l5_queue);

		/* set l5 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, sched_prio, prio);
		l5_child = l5_queue;
		child_queue++;
		num_queues--;
		prio++;
		num_l5_childs++;
	}

	if (update_tail) {
		/* new childs are added at end of list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l5_tail, l5_child);
	}
	else {
		/* add prev nodes back into the the l5 list */
		__cvmx_pko_queue_add_next_node(l5_child, l5_prev);
	}

	/* update number of l5 childs in l4 parent queue */
	CVMX_PKO_SET_QUEUE(l4_queue, num_childs, num_l5_childs);

	ret_val = child_base;

end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Configure level 4 queue for round robin child queues.
 *
 * @param node is to specify the node on which l4 queue is present.
 * @param interface is the interface for which level4 queue will be configured.
 * @param port is the port number for which level4 queue will be configured.
 * @param queue is the l4 queue number to configure.
 * @param rr_queues is the array specifying child rr queue numbers(specify -1 for api to allocate) 
 * @param rr_quantum is the array specifying child rr queue quantum values.
 * @param num_rr_queues is the total number of rr child queues in rr_queues. 
 * @param rr_prio is the round robin priority assigned to rr_queues.
 * @return returns 0 if successful or -1 on failure.
 */
int cvmx_pko_l4_queue_setup_rr_childs(int node, int interface, int port, 
				      int queue, int rr_queues[],
				      int rr_quantum[], int num_rr_queues,
				      int rr_prio)
{
	int port_queue;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l4_head, l4_queue; 
	uint64_t l5_head, l5_tail, l5_queue, l5_child, l5_prev;
	int num_l5_childs;
	int queue_num = -1, ret_val = -1;
	int update_tail = 0;
	int idx = 0;
	int num_queues = num_rr_queues;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l4_head */
	l4_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l4_head);
	if (!l4_head) {
		cvmx_dprintf("ERROR: l4 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}

	/* find l4 parent in the l4 list */
	l4_queue = l4_head;
	while (l4_queue != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l4_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l4_queue = __cvmx_pko_queue_get_next_node(l4_queue);
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l4 queue %d is not configured\n", queue);
		goto end;
	}

	/* get child base for l4 parent queue, if prev configured */
	l5_child = __cvmx_pko_queue_get_child_node(l4_queue);
	num_l5_childs = CVMX_PKO_GET_QUEUE(l4_queue, num_childs);

	/* check if there is a prev. configured static queue at rr_prio and convert it into rr */
	if(num_l5_childs >= rr_prio+1) {
		int rr_queue_num = 0;
		int num_childs = num_l5_childs;
		int queue_base = 0;

		queue_base = CVMX_PKO_GET_QUEUE_INFO(l4_queue, child_static_base);
		rr_queue_num = queue_base + rr_prio;

		l5_queue = l5_child;
		queue_num = -1;
		while (num_childs != 0) {
			queue_num = CVMX_PKO_GET_QUEUE_INFO(l5_queue, queue_num);
			if (queue_num == rr_queue_num)
				break;
			else
				queue_num = -1;
			l5_queue = __cvmx_pko_queue_get_next_node(l5_queue);
			num_childs--;
		}

		if (queue_num != -1) {
			if (rr_queue_num == queue_base) {
				/* change l4 child static base */
				CVMX_PKO_SET_QUEUE_INFO(l4_queue, child_static_base, queue_base+1);
			}
			/* rr_queue_num will be filled in rr_queues */
			if (rr_queues[idx] == -1) {
				rr_queues[idx] = rr_queue_num;
				CVMX_PKO_SET_QUEUE_INFO(l5_queue, sched_algo, CVMX_PKO_RR);
				CVMX_PKO_SET_QUEUE_INFO(l5_queue, rr_quantum, rr_quantum[idx]);
				idx++;
				num_queues--;
			}
		}
	}

	/* reserve or allocate queues from global resources */
 	ret_val = cvmx_pko_alloc_queues_many(node, CVMX_PKO_L5_QUEUES, port_queue,
			                     &rr_queues[idx], num_queues);

	if (ret_val < 0) {
		cvmx_dprintf ("Error: Failed to allocate pko level5 queues\n");
		goto end;
	}


	/* get l5 queue list details */
	l5_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l5_tail);
	l5_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l5_head);

	/* add l5 child base node for l4 queue if not present */
	if (!l5_child) {
		l5_child = __cvmx_pko_alloc_queue_node();
		if (!l5_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set l4 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l5_child, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l5_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l5_child, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l5_child, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l5_child, rr_quantum, rr_quantum[idx]);

		/* add l5 child base node to l4 parent queue */
		__cvmx_pko_queue_add_child_node(l4_queue, l5_child);

		idx++;
		num_l5_childs = 1;

		if (!l5_head) {
			l5_head = l5_tail = l5_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l5_head, l5_head);
		}
		else {
			/* add new queue to the existing l5 list */
			__cvmx_pko_queue_add_next_node(l5_tail, l5_child);
		}
		update_tail = 1;
	} else {
		/* childs already exist for this l4_queue */
		l5_prev = __cvmx_pko_queue_get_next_node(l5_child);
	}

	/* add remaining childs to the l5 list */
        while (idx < num_rr_queues) {
		l5_queue = __cvmx_pko_alloc_queue_node();
		if (!l5_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the l5 list */
		__cvmx_pko_queue_add_next_node(l5_child, l5_queue);

		/* set l5 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, rr_quantum, rr_quantum[idx]);
		l5_child = l5_queue;
		num_l5_childs++;
		idx++;
	}

	if (update_tail) {
		/* new childs are added at end of l5 list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, l5_tail, l5_child);
	}
	else {
		/* add prev nodes back into the the l5 list */
		__cvmx_pko_queue_add_next_node(l5_child, l5_prev);
	}

	/* update number of l5 childs in l4 parent queue */
	CVMX_PKO_SET_QUEUE(l4_queue, num_childs, num_l5_childs);

	ret_val = 0;
end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Configure level 5 queue for Static priority child queues.
 *
 * @param node is to specify the node on which l5 queue is present.
 * @param interface is the interface for which level5 queue will be configured.
 * @param port is the port number for which level5 queue will be configured.
 * @param queue is the l5 queue number to configure.
 * @param child_base is the start of child static queues(specify -1 for api to allocate) 
 * @param lowest_static_priority is the lowest static priority input(higher in value) connected to the  scheduler. 
 * @return returns start of child static queues if successful or -1 on failure.
 */
int cvmx_pko_l5_queue_setup_static_childs(int node, int interface, int port, 
					  int queue, int child_base,
	                                  int lowest_static_prio)
{
	int child_queue, port_queue;
	int num_queues = lowest_static_prio + 1;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l5_head, l5_queue;
	uint64_t dq_head, dq_tail, dq_queue, dq_child, dq_prev;
	uint64_t num_dq_childs;
	int prio = 0, update_tail = 0, i = 0;
	int port_index;
	int queue_num = -1, ret_val = -1;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	/* allocate or reserve descr queues */
	child_base = cvmx_pko_alloc_queues(node, CVMX_PKO_DESCR_QUEUES, port_queue,
					   child_base, num_queues);
	if (child_base < 0) {
		cvmx_dprintf ("Failed to allocate or reserve pko descr queues\n");
		return -1;
	}
	child_queue = child_base;

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l5_head */
	l5_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l5_head);
	if (!l5_head) {
		cvmx_dprintf("ERROR: l5 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}

	/* find l5 parent in the l5 list */
	l5_queue = l5_head;
	while (l5_queue != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l5_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l5_queue = __cvmx_pko_queue_get_next_node(l5_queue);
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l5 queue %d is not configured\n", queue);
		goto end;
	}

	/* get dq queue list details */
	dq_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, dq_tail);
	dq_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, dq_head);

	dq_child = __cvmx_pko_queue_get_child_node(l5_queue);
	num_dq_childs = CVMX_PKO_GET_QUEUE(l5_queue, num_childs);

	/* add dq child base node for l5 queue if not present */
	if (!dq_child) {
		dq_child = __cvmx_pko_alloc_queue_node();
		if (!dq_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set dq child queue info */
		CVMX_PKO_SET_QUEUE_INFO(dq_child, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(dq_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(dq_child, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(dq_child, sched_prio, prio);

		/* add dq child base node to l5 parent queue */
		__cvmx_pko_queue_add_child_node(l5_queue, dq_child);
		CVMX_PKO_SET_QUEUE_INFO(l5_queue, child_static_base, child_queue);

		child_queue++;
		num_queues--;
		prio++;
		num_dq_childs = 1;
		
		if (!dq_head) {
			dq_head = dq_tail = dq_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, dq_head, dq_head);
		}
		else {
			/* add new queue to the existing dq list */
			__cvmx_pko_queue_add_next_node(dq_tail, dq_child);
		}
		update_tail = 1;
	} else {
		/* childs(RR) already exist for this l5_queue */
		dq_prev = __cvmx_pko_queue_get_next_node(dq_child);
	}

	/* add remaining childs to the dq list */
	while (num_queues != 0) {
		dq_queue = __cvmx_pko_alloc_queue_node();
		if (!dq_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the dq list */
		__cvmx_pko_queue_add_next_node(dq_child, dq_queue);

		/* set dq child queue info */
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, queue_num, child_queue);
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, sched_algo, CVMX_PKO_STATIC);
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, sched_prio, prio);
		dq_child = dq_queue;
		child_queue++;
		num_queues--;
		prio++;
		num_dq_childs++;
	}

	if (update_tail) {
		/* new childs are added at end of list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, dq_tail, dq_child);
	}
	else {
		/* add prev nodes back into the the dq list */
		__cvmx_pko_queue_add_next_node(dq_child, dq_prev);
	}

	/* update number of dq childs in l5 parent queue */
	CVMX_PKO_SET_QUEUE(l5_queue, num_childs, num_dq_childs);

	DEBUG;
	/* fill dq list per interface/port */
	child_queue = child_base;
	num_queues = lowest_static_prio + 1;
	port_index = __cvmx_pko_get_port_index(node, interface, port); 
	if (port_index >= 0) {

		i = pko_dq_list[node][port_index].num_queues;

		if (i < CVMX_PKO_MAX_DQ_IN_LIST) {
			int avail_queues;
			avail_queues = CVMX_PKO_MAX_DQ_IN_LIST - i;
			if (avail_queues < num_queues) 
				num_queues = avail_queues;
			while (num_queues != 0) {
				pko_dq_list[node][port_index].queue_list[i] = child_queue;
				child_queue++;
				i++;
				num_queues--;
			}
			pko_dq_list[node][port_index].num_queues = i;
		}
	}
	DEBUG;

	ret_val = child_base;

end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Configure l5 queue for round robin child queues.
 *
 * @param node is to specify the node on which l5 queue is present.
 * @param interface is the interface for which level5 queue will be configured.
 * @param port is the port number for which level5 queue will be configured.
 * @param queue is the l5 queue number to configure.
 * @param rr_queues is the array specifying child rr queue numbers(specify -1 for api to allocate) 
 * @param rr_quantum is the array specifying child rr queue quantum values.
 * @param num_rr_queues is the total number of rr child queues in rr_queues. 
 * @param rr_prio is the round robin priority assigned to rr_queues.
 * @return returns 0 if successful or -1 on failure.
 */
int cvmx_pko_l5_queue_setup_rr_childs(int node, int interface, int port, 
				      int queue, int rr_queues[],
				      int rr_quantum[], int num_rr_queues,
				      int rr_prio)
{
	int port_queue;
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	uint64_t l5_head, l5_queue;
	uint64_t dq_head, dq_tail, dq_queue, dq_child, dq_prev;
	int num_dq_childs;
	int queue_num = -1, ret_val = -1;
	int update_tail = 0;
	int idx = 0, port_index, i;
	int num_queues = num_rr_queues;

	/* TODO : assert if not proper values */

	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* get l5_head */
	l5_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l5_head);
	if (!l5_head) {
		cvmx_dprintf("ERROR: l5 queues are not initialized for port queue : %d\n", port_queue);
		goto end;
	}

	/* find l5 parent in the l5 list */
	l5_queue = l5_head;
	while (l5_queue != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(l5_queue, queue_num);
		if (queue_num == queue)
			break;
		else
			queue_num = -1;
		l5_queue = __cvmx_pko_queue_get_next_node(l5_queue);
	}

	if (queue_num == -1) {
		cvmx_dprintf("ERROR: l5 queue %d is not configured\n", queue);
		goto end;
	}

	/* get child base for l5 parent queue, if prev configured */
	dq_child = __cvmx_pko_queue_get_child_node(l5_queue);
	num_dq_childs = CVMX_PKO_GET_QUEUE(l5_queue, num_childs);

	/* check if there is a prev. configured static queue at rr_prio and convert it into rr */
	if(num_dq_childs >= rr_prio+1) {
		int rr_queue_num = 0;
		int num_childs = num_dq_childs;
		int queue_base = 0;

		queue_base = CVMX_PKO_GET_QUEUE_INFO(l5_queue, child_static_base);
		rr_queue_num = queue_base + rr_prio;

		dq_queue = dq_child;
		queue_num = -1;
		while (num_childs != 0) {
			queue_num = CVMX_PKO_GET_QUEUE_INFO(dq_queue, queue_num);
			if (queue_num == rr_queue_num)
				break;
			else
				queue_num = -1;
			dq_queue = __cvmx_pko_queue_get_next_node(dq_queue);
			num_childs--;
		}

		if (queue_num != -1) {
			if (rr_queue_num == queue_base) {
				/* change l5 child static base */
				CVMX_PKO_SET_QUEUE_INFO(l5_queue, child_static_base, queue_base+1);
			}
			/* rr_queue_num will be filled in rr_queues */
			if (rr_queues[idx] == -1) {
				rr_queues[idx] = rr_queue_num;
				CVMX_PKO_SET_QUEUE_INFO(dq_queue, sched_algo, CVMX_PKO_RR);
				CVMX_PKO_SET_QUEUE_INFO(dq_queue, rr_quantum, rr_quantum[idx]);
				idx++;
				num_queues--;
			}
		}
	}

	/* reserve or allocate queues from global resources */
 	ret_val = cvmx_pko_alloc_queues_many(node, CVMX_PKO_DESCR_QUEUES, port_queue,
			                     &rr_queues[idx], num_queues);
	if (ret_val < 0) {
		cvmx_dprintf ("Error: Failed to allocate pko descr queues\n");
		goto end;
	}

	/* get dq queue list details */
	dq_tail = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, dq_tail);
	dq_head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, dq_head);

	/* add dq child base node for l5 queue if not present */
	if (!dq_child) {
		dq_child = __cvmx_pko_alloc_queue_node();
		if (!dq_child) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* set l5 child queue info */
		CVMX_PKO_SET_QUEUE_INFO(dq_child, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(dq_child, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(dq_child, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(dq_child, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(dq_child, rr_quantum, rr_quantum[idx]);

		/* add dq child base node to l5 parent queue */
		__cvmx_pko_queue_add_child_node(l5_queue, dq_child);

		idx++;
		num_dq_childs = 1;

		if (!dq_head) {
			dq_head = dq_tail = dq_child;
			CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, dq_head, dq_head);
		}
		else {
			/* add new queue to the existing dq list */
			__cvmx_pko_queue_add_next_node(dq_tail, dq_child);
		}
		update_tail = 1;
	} else {
		/* childs already exist for this l5_queue */
		dq_prev = __cvmx_pko_queue_get_next_node(dq_child);
	}

	/* add remaining childs to the dq list */
        while (idx < num_rr_queues) {
		dq_queue = __cvmx_pko_alloc_queue_node();
		if (!dq_queue) {
			cvmx_dprintf("ERROR: Failed to get memory\n");
			goto end;
		}

		/* add this queue to the dq list */
		__cvmx_pko_queue_add_next_node(dq_child, dq_queue);

		/* set dq child queue info */
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, queue_num, rr_queues[idx]);
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, parent_queue, queue);
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, sched_algo, CVMX_PKO_RR);
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, sched_prio, rr_prio);
		CVMX_PKO_SET_QUEUE_INFO(dq_queue, rr_quantum, rr_quantum[idx]);
		dq_child = dq_queue;
		num_dq_childs++;
		idx++;
	}

	if (update_tail) {
		/* new childs are added at end of dq list, so update tail */
		CVMX_PKO_SET_PORT_QUEUE(port_queue_addr, dq_tail, dq_child);
	}
	else {
		/* add prev nodes back into the the dq list */
		__cvmx_pko_queue_add_next_node(dq_child, dq_prev);
	}

	/* update number of dq childs in l5 parent queue */
	CVMX_PKO_SET_QUEUE(l5_queue, num_childs, num_dq_childs);

	DEBUG;
	/* fill dq list per interface/port */
	num_queues = num_rr_queues;
	idx = 0;

	port_index = __cvmx_pko_get_port_index(node, interface, port); 
	if (port_index >= 0) {
		i = pko_dq_list[node][port_index].num_queues;

		if (i < CVMX_PKO_MAX_DQ_IN_LIST) {
			int avail_queues;
			avail_queues = CVMX_PKO_MAX_DQ_IN_LIST - i;
			if (avail_queues < num_queues) 
				num_queues = avail_queues;
			while (num_queues != 0) {
				pko_dq_list[node][port_index].queue_list[i] = rr_queues[idx];
				i++;
				num_queues--;
				idx++;
			}
			pko_dq_list[node][port_index].num_queues = i;
		}
	}
	DEBUG;

	ret_val = 0;
end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Setup PKO queue topology and scheduling for interface/port
 * in hardware.
 *
 * @param node is the node on which PKO queue topology is setup.
 * @param interface is the interface number for which queue topology is configured. 
 * @param port is the port number for which queue topology is configured.
 * @return returns 0 on success or -1 on failure.
 */
int __cvmx_pko_setup_queue_topology(int node, int interface, int port) 
{
	uint64_t port_queue_addr;
	uint64_t queue_topology_base_addr;
	int port_queue;
	int queue_num, child_base, child_rr_prio, prio, parent_queue, rr_quantum;
	uint64_t head, queue_node;
	int ret_val = -1;
	int pko_mac_num;
       
	port_queue = cvmx_pko_get_port_queue(node, interface, port);
	if (port_queue < 0) {
		cvmx_dprintf ("Failed to get port_queue for interface %d, port %d\n",
			      interface, port);
		return -1;
	}

	queue_topology_base_addr = __cvmx_pko_get_queue_data_addr(node);

	if (!queue_topology_base_addr) {
		cvmx_dprintf("Error: queue_topology_base_addr is null\n");
		return -1;
	}

	__cvmx_pko_queue_lock(node);

	port_queue_addr = CVMX_PKO_GET_QUEUE_TOPOLOGY(queue_topology_base_addr,
						      port_queue_addr, port_queue);

	if (!port_queue_addr) {
		cvmx_dprintf("ERROR: Port queue: %d is not setup\n", port_queue);
		goto end;
	}

	/* setup PQ configuration */
	pko_mac_num = __cvmx_pko_get_mac_num(interface, port);
	if (pko_mac_num < 0) {
		cvmx_dprintf ("interface %d port %d does not have pko mac\n", interface, port);
		return -1;
	}
	child_base = CVMX_PKO_GET_PORT_QUEUE_INFO(port_queue_addr, child_static_base);
	child_rr_prio = CVMX_PKO_GET_PORT_QUEUE_INFO(port_queue_addr, child_rr_prio);
	cvmx_pko_configure_port_queue(node, port_queue, child_base, child_rr_prio, pko_mac_num);

	/* setup l2 queues */
	head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l2_head);

	queue_node = head;
	while (queue_node != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(queue_node, queue_num);
		parent_queue = CVMX_PKO_GET_QUEUE_INFO(queue_node, parent_queue);
		prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, sched_prio);
		rr_quantum = CVMX_PKO_GET_QUEUE_INFO(queue_node, rr_quantum);
		child_base = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_static_base);
		child_rr_prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_rr_prio);
		cvmx_pko_configure_l2_queue(node, queue_num, parent_queue, prio, 
			       	            rr_quantum, child_base, child_rr_prio);
		queue_node = __cvmx_pko_queue_get_next_node(queue_node); 
	}

	/* setup l3 queues */
	head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l3_head);

	queue_node = head;
	while (queue_node != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(queue_node, queue_num);
		parent_queue = CVMX_PKO_GET_QUEUE_INFO(queue_node, parent_queue);
		prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, sched_prio);
		rr_quantum = CVMX_PKO_GET_QUEUE_INFO(queue_node, rr_quantum);
		child_base = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_static_base);
		child_rr_prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_rr_prio);
		cvmx_pko_configure_l3_queue(node, queue_num, parent_queue, prio, rr_quantum, 
					    child_base, child_rr_prio);
		queue_node = __cvmx_pko_queue_get_next_node(queue_node); 
	}

	/* setup l4 queues */
	head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l4_head);

	queue_node = head;
	while (queue_node != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(queue_node, queue_num);
		parent_queue = CVMX_PKO_GET_QUEUE_INFO(queue_node, parent_queue);
		prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, sched_prio);
		rr_quantum = CVMX_PKO_GET_QUEUE_INFO(queue_node, rr_quantum);
		child_base = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_static_base);
		child_rr_prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_rr_prio);
		cvmx_pko_configure_l4_queue(node, queue_num, parent_queue, prio,
			       		    rr_quantum,	child_base, child_rr_prio);
		queue_node = __cvmx_pko_queue_get_next_node(queue_node); 
	}

	/* setup l5 queues */
	head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, l5_head);

	queue_node = head;
	while (queue_node != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(queue_node, queue_num);
		parent_queue = CVMX_PKO_GET_QUEUE_INFO(queue_node, parent_queue);
		prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, sched_prio);
		rr_quantum = CVMX_PKO_GET_QUEUE_INFO(queue_node, rr_quantum);
		child_base = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_static_base);
		child_rr_prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, child_rr_prio);
		cvmx_pko_configure_l5_queue(node, queue_num, parent_queue, prio,
			       		    rr_quantum,	child_base, child_rr_prio);
		queue_node = __cvmx_pko_queue_get_next_node(queue_node); 
	}

	/* setup descriptor queues */
	head = CVMX_PKO_GET_PORT_QUEUE(port_queue_addr, dq_head);

	queue_node = head;
	while (queue_node != 0) {
		queue_num = CVMX_PKO_GET_QUEUE_INFO(queue_node, queue_num);
		parent_queue = CVMX_PKO_GET_QUEUE_INFO(queue_node, parent_queue);
		prio = CVMX_PKO_GET_QUEUE_INFO(queue_node, sched_prio);
		rr_quantum = CVMX_PKO_GET_QUEUE_INFO(queue_node, rr_quantum);
		cvmx_pko_configure_dq(node, queue_num, parent_queue, prio, rr_quantum);
		queue_node = __cvmx_pko_queue_get_next_node(queue_node); 
	}
        
	ret_val = 0;
end:
	__cvmx_pko_queue_unlock(node);
	return (ret_val);
}

/*
 * Get PKO port queue for interface/port number.
 *
 * @param node is the node on which port queue number is required.
 * @param interface is the interface number for which port queue is required. 
 * @param port is the port number for which port queue is required.
 * @return returns port queue number on success or -1 on failure.
 */
int cvmx_pko_get_port_queue(int node, int interface, int port) 
{
	int pko_mac_num;
	int port_queue;

	pko_mac_num = __cvmx_pko_get_mac_num(interface, port);
	if (pko_mac_num < 0) {
		cvmx_dprintf ("interface %d port %d does not have pko mac\n", interface, port);
		return -1;
	}

	/* allocate port queue if it is not allocated */
	if (cvmx_port_queue_list[node][pko_mac_num] < 0) {
		port_queue = cvmx_pko_alloc_queues(node, CVMX_PKO_PORT_QUEUES, 
						   pko_mac_num, -1, 1); 
		if (port_queue < 0) {
			cvmx_dprintf ("Failed to allocate port queues\n");
			return -1;
		}
		cvmx_port_queue_list[node][pko_mac_num] = port_queue;
	}

	return (cvmx_port_queue_list[node][pko_mac_num]);
}

/*
 * Get Descriptor queues for ipd_port.
 *
 * @param node is the node on which descriptor queues are required.
 * @param ipd_port is the ipd port number for which descriptor queues are required. 
 * @param dq_list is the empty array, which will be filled with list of dqs for this ipd_port. 
 * @param num_dq is the number of dqs pointed by dq_list. 
 * @return returns valid number of dqs filled on success or -1 on failure.
 */
int cvmx_pko_get_descriptor_queues(int node, int ipd_port, int dq_list[], int num_dq) 
{
	int cnt, port_index;

	port_index = __cvmx_cfg_get_ipd2pko_index(node, ipd_port);
	if (port_index < 0) {
		cvmx_dprintf ("Failed to get port_index for ipd_port %d\n",
			      ipd_port);
		return -1;
	}

	if (num_dq > pko_dq_list[node][port_index].num_queues) {
		num_dq = pko_dq_list[node][port_index].num_queues;
	}

	for (cnt = 0; cnt < num_dq; cnt++) {
		dq_list[cnt] = pko_dq_list[node][port_index].queue_list[cnt];
	}

	return num_dq;
}
