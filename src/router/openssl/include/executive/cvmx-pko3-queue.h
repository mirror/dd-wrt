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
 * <hr>$Revision: 0 $<hr>
 */

#ifndef __CVMX_PKO3_QUEUE_H__
#define __CVMX_PKO3_QUEUE_H__

#ifdef	__cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/* these are s/w defines and can be varied according to need */
#define CVMX_PKO_MAX_DQ_IN_LIST 16
#define CVMX_PKO_NUM_PORT_INDEX 128

enum {
	CVMX_PKO_STATIC = 0,
	CVMX_PKO_RR
};

/* queue topology is present for each node */
struct cvmx_pko_queue_topology {
	uint64_t queue_lock;
	uint64_t port_queue_addr[32];  /* port_queue_addr points to cvmx_pko_port_queue */
};
typedef struct cvmx_pko_queue_topology cvmx_pko_queue_topology_t;

struct cvmx_pko_queue_node {
	uint64_t next_node;  /* next node at the same level */
	uint64_t child_node; /* node points to child base node */
};

struct cvmx_pko_queue_info {
	uint64_t queue_num;
	uint64_t parent_queue;
	uint64_t sched_algo;
	uint64_t sched_prio;
	uint64_t rr_quantum;
	uint64_t child_static_base;
	uint64_t child_rr_prio;
};
typedef struct cvmx_pko_queue_info cvmx_pko_queue_info_t;

struct cvmx_pko_port_queue {
	struct cvmx_pko_queue_info queue_info;
	uint64_t num_l2_childs;
	uint64_t l2_head;
	uint64_t l2_tail;
	uint64_t l3_head;
	uint64_t l3_tail;
	uint64_t l4_head;
	uint64_t l4_tail;
	uint64_t l5_head;
	uint64_t l5_tail;
	uint64_t dq_head;
	uint64_t dq_tail;
};
typedef struct cvmx_pko_port_queue cvmx_pko_port_queue_t;

struct cvmx_pko_queue {
	struct cvmx_pko_queue_node queue_list;
	struct cvmx_pko_queue_info queue_info;
	uint64_t num_childs;
};
typedef struct cvmx_pko_queue cvmx_pko_queue_t;

/* pko cached data structure */
/* DQ list */
struct cvmx_pko_dq_list {
	int queue_list[CVMX_PKO_MAX_DQ_IN_LIST];
	int num_queues;
};

/*
 * Get Descriptor queues for ipd_port.
 *
 * @param node is the node on which descriptor queues are required.
 * @param ipd_port is the ipd port number for which descriptor queues are required. 
 * @param dq_list is the empty array, which will be filled with list of dqs for this ipd_port. 
 * @param num_dq is the number of dqs pointed by dq_list. 
 * @return returns valid number of dqs filled on success or -1 on failure.
 */
int cvmx_pko_get_descriptor_queues(int node, int ipd_port, 
				   int dq_list[], int num_dq); 

/*
 * Get PKO port queue for interface/port number.
 *
 * @param node is the node on which port queue number is required.
 * @param interface is the interface number for which port queue is required. 
 * @param port is the port number for which port queue is required.
 * @return returns port queue number on success or -1 on failure.
 */
int cvmx_pko_get_port_queue(int node, int interface, int port);

/*
 * Setup PKO queue topology and scheduling for interface/port
 * in hardware.
 *
 * @param node is the node on which PKO queue topology is setup.
 * @param interface is the interface number for which queue topology is configured. 
 * @param port is the port number for which queue topology is configured.
 * @return returns 0 on success or -1 on failure.
 */
int __cvmx_pko_setup_queue_topology(int node, int interface, int port);

/*
 * Initialize PKO queue data structures.
 *
 * @param none
 * @return returns 0 on success or -1 on failure.
 */
int __cvmx_pko_queue_initialize(void);


/* topology and scheduling api */

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
int cvmx_pko_port_queue_setup_static_childs(int node, int interface, int port, int child_base,
		                            int lowest_static_prio); 

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
int cvmx_pko_port_queue_setup_rr_childs(int node, int interface, int port, int rr_queues[], 
				        int rr_quantum[], int num_rr_queues, 
				        int rr_prio);

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
int cvmx_pko_l2_queue_setup_static_childs(int node, int interface, int port, int queue, int child_base,
			                  int lowest_static_prio); 

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
int cvmx_pko_l2_queue_setup_rr_childs(int node, int interface, int port, int queue, int rr_queues[], 
				      int rr_quantum[], int num_rr_queues, 
				      int rr_prio);

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
int cvmx_pko_l3_queue_setup_static_childs(int node, int interface, int port, int queue, 
					  int child_base, 
					  int lowest_static_prio); 

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
int cvmx_pko_l3_queue_setup_rr_childs(int node, int interface, int port, int queue, 
				      int rr_queues[], int rr_quantum[], 
				      int num_rr_queues, int rr_prio);

/*
 * Configure level 4 queue for Static priority child queues.
 *
 * @param node is to specify the node on which l4 queue is present.
 * @param interface is the interface for which level4 queue will be configured.
 * @param port is the port number for which level4 queue will be configured.
 * @param queue is the l4 queue number to configure.
 * @param child_base is the start of child static queues(specify -1 for api to allocate) 
 * @param lowest_static_priority is the lowest static priority input(higher in value) connected to the  scheduler. 
 * @return returns  start of child static queues if successful or -1 on failure.
 */
int cvmx_pko_l4_queue_setup_static_childs(int node, int interface, int port,int queue, 
					  int child_base, 
					  int lowest_static_prio); 

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
int cvmx_pko_l4_queue_setup_rr_childs(int node, int interface, int port, int queue, 
				      int rr_queues[], int rr_quantum[], 
				      int num_rr_queues, int rr_prio);

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
int cvmx_pko_l5_queue_setup_static_childs(int node, int interface, int port, int queue, 
					  int child_base, 
					  int lowest_static_prio); 

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
int cvmx_pko_l5_queue_setup_rr_childs(int node, int interface, int port, int queue, 
				      int rr_queues[], int rr_quantum[], 
				      int num_rr_queues, int rr_prio);

#ifdef	__cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#endif /* __CVMX_PKO3_QUEUE_H__ */
