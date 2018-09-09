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
#include <asm/octeon/cvmx-fpa.h>
#include <asm/octeon/cvmx-pko3.h>
#include <asm/octeon/cvmx-helper-pko3.h>
#else
#include "cvmx.h"
#include "cvmx-pko3.h"
#include "cvmx-fpa.h"
#include "cvmx-helper-pko3.h"
#endif

uint64_t cvmx_pko_free_fifo_mask[CVMX_MAX_NODES] = 
			{[0 ... CVMX_MAX_NODES - 1] = 0x0fffffff};

/*
 * PKO global intialization for 78XX.
 *
 * @param node is the node on which PKO block is initialized.
 * @return none.
 */
void cvmx_pko_initialize_global_78xx(int node) 
{
	cvmx_pko_dpfi_fpa_aura_t pko_aura;
	cvmx_pko_dpfi_ena_t dpfi_enable;
	cvmx_pko_ptf_iobp_cfg_t ptf_iobp_cfg;

#define CVMX_PKO_POOL_NUM 2
#define CVMX_PKO_AURA_NUM 2
#define CVMX_PKO_POOL_BUFFERS 1024
#define CVMX_PKO_AURA_BUFFERS 256
#define CVMX_PKO_POOL_BUFFER_SIZE 4096 /* 78XX PKO requires 4KB */

#ifndef CVMX_BUILD_FOR_LINUX_KERNEL
	/* TODO: Reserve PKO Aura and Pool number */

	/* fpa pool intialization for pko command buffers */
	cvmx_fpa_pool_stack_init(node, CVMX_PKO_POOL_NUM, "PKO Pool", 0, CVMX_PKO_POOL_BUFFERS,
				 FPA_NATURAL_ALIGNMENT, CVMX_PKO_POOL_BUFFER_SIZE);
	cvmx_fpa_assign_aura(node, CVMX_PKO_AURA_NUM, CVMX_PKO_POOL_NUM); 
	cvmx_fpa_aura_init(node, CVMX_PKO_AURA_NUM,"PKO Aura", 0, NULL,
			   CVMX_PKO_AURA_BUFFERS, 0);
#endif

	/* set the aura number in pko */
	pko_aura.u64 = 0;
	pko_aura.s.node = node;
	pko_aura.s.laura = CVMX_PKO_AURA_NUM;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_FPA_AURA, pko_aura.u64); 

	dpfi_enable.u64 = 0;
	dpfi_enable.s.enable = 1;
	cvmx_write_csr_node(node, CVMX_PKO_DPFI_ENA, dpfi_enable.u64);

	/* set max outstanding requests in IOBP for any FIFO */
	ptf_iobp_cfg.u64 = 0;
	ptf_iobp_cfg.s.max_read_size = 72;
	cvmx_write_csr_node(node, CVMX_PKO_PTF_IOBP_CFG, ptf_iobp_cfg.u64); 

	/* initialize pko queue global data named block */
	__cvmx_pko_queue_initialize();

	/* initialize default pko queue configuration per interface/port */
	__cvmx_pko_init_port_config_data_78xx(node);
}

/*
 * Transmit packets through pko on specified node and queue.
 *
 * @param node is to specify which node's pko block to send the commands.
 * @param dq is the queue to write the commands to. 
 * @param bufptr specifies packet in linked or gather mode.
 * @param packet_len is the total packet len of the packet in bufptr.
 * @param aura_free is the aura to free packet buffers after trasnmit.
 * @return returns 0 if successful and -1 on failure.
 */
int cvmx_pko_transmit_packet(int node, int dq, cvmx_pko_buf_ptr_t bufptr, 
			     int packet_len, int aura_free) 
{

	cvmx_pko_send_hdr_t pko_send_hdr;
	cvmx_pko_query_rtn_s_t pko_status;
	uint64_t scraddr;
	int scr_len, num_descr_words;

	pko_send_hdr.u64 = 0;
	pko_send_hdr.s.aura = aura_free;
	/* TODO: n2 is not currently supported in simulator */
	pko_send_hdr.s.n2 = 0;
	pko_send_hdr.s.total = packet_len;

	/* write packet descriptor, packet ptr in command words */
	cvmx_pko_write_descr_word(0, pko_send_hdr.u64);
	cvmx_pko_write_descr_word(1, bufptr.u64);
	num_descr_words = 2;

	/* use this line for return status */
	cvmx_pko_write_descr_word(2, -1);
	scraddr = CVMX_LMTDMA_WORD(2);
	scr_len = 1;
	
	cvmx_pko_send_command(node, dq, num_descr_words, scraddr, scr_len);

	/* check status */
	pko_status.u64 = 0;
	pko_status.u64 = cvmx_scratch_read64(CVMX_LMTDMA_WORD(2));
	if (pko_status.s.dqstatus) {
		cvmx_dprintf("%d: Error: pko transmit failed with error: %d\n",
				__LINE__, pko_status.s.dqstatus);
		return -1;
	}

	return 0;
}

/*
 * Enables PKO on specified node for 78XX.
 *
 * @param node is the node on which PKO will be enabled.
 * @return none.
 */
void cvmx_pko_enable_78xx(int node) 
{
	cvmx_pko_enable_t pko_enable;
	int num_interfaces, interface, mode, num_ports, port;

	/* enable PKO */
	pko_enable.u64 = 0;
	pko_enable.s.enable = 1;
	cvmx_write_csr_node(node, CVMX_PKO_ENABLE, pko_enable.u64);

	/* open configured dqs for all the macs */
	num_interfaces = cvmx_helper_get_number_of_interfaces();
	for (interface = 0; interface < num_interfaces; interface++) {
		mode = cvmx_helper_interface_get_mode(interface);
		num_ports = cvmx_helper_interface_enumerate(interface);

		if ((mode == CVMX_HELPER_INTERFACE_MODE_NPI) || 
			(mode == CVMX_HELPER_INTERFACE_MODE_LOOP))
			num_ports = 1;

		for (port = 0; port < num_ports; port++) {
			int dq_list[CVMX_PKO_MAX_DQ_IN_LIST];
			int num_dq, i;
			num_dq = cvmx_pko_get_descriptor_queues(node, cvmx_helper_get_ipd_port(interface, port),
								dq_list, CVMX_PKO_MAX_DQ_IN_LIST);
			for (i = 0; i < num_dq; i++)
				cvmx_pko_dq_open(node, dq_list[i]);
		}
	}
}

/*
 * @INTERNAL  
 * Setup fifo size and mac credits for a given interface/mac.
 *
 * @param node is the node on which this applies.
 * @param interface is the interface number.
 * @param pko_mac_num is the mac number for the interface.
 * @return fifo number if success or -1 on failure.
 */
static int __cvmx_pko_setup_fifo(int node, int interface, int pko_mac_num) 
{
	cvmx_helper_interface_mode_t mode;
	int fifo_req_size;
	int fifo = 0, index;
	cvmx_pko_ptgfx_cfg_t pko_ptgfx_cfg;
	cvmx_pko_mci0_max_credx_t pko_mci0_max_cred;
	cvmx_pko_mci1_max_credx_t pko_mci1_max_cred;
	int mac_credit;
	int credit;

	/* 2.5kb - 0, 5kb - 1, 10kb - 4 */
	fifo_req_size = 0;
	mode = cvmx_helper_interface_get_mode(interface);
	switch (mode) {
		case CVMX_HELPER_INTERFACE_MODE_SGMII:
			fifo_req_size = 1;
			break;
		case CVMX_HELPER_INTERFACE_MODE_RXAUI:
			fifo_req_size = 2;
			break;
		case CVMX_HELPER_INTERFACE_MODE_XAUI:
/*		case CVMX_HELPER_INTERFACE_MODE_LOOP: */
		case CVMX_HELPER_INTERFACE_MODE_ILK:
		case CVMX_HELPER_INTERFACE_MODE_NPI:
			fifo_req_size = 4;	
			break;
		default:
			return -1;
	}

	while (fifo < 32)
	{
		uint64_t mask = build_mask(fifo_req_size) << fifo;
        	/* Stop search if all bits are free */
        	if ((mask & cvmx_pko_free_fifo_mask[node]) == mask)
        	{
            		/* Found a spot, mark it used and stop searching */
            		cvmx_pko_free_fifo_mask[node] &= ~mask;
            		break;
        	}
        	/* Increment by size to keep alignment */
        	fifo += fifo_req_size;
	}
    
	if (fifo >= 32)
	{
		cvmx_dprintf("__cvmx_pko_setup_fifo: too many fifos\n");
		return -1;
	}

	/* configure PKO fifo sizes in h/w */
	index = fifo >> 2;
	pko_ptgfx_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_PTGFX_CFG(index)); 

	switch (pko_ptgfx_cfg.s.size)
	{
	    case 0: /* 2.5kb, 2.5kb, 2.5kb, 2.5kb */
		switch (fifo_req_size)
		{
		    case 1:
                    	pko_ptgfx_cfg.s.size = 0; /* 2.5kb for all */
                        break;
                    case 2:
                    	pko_ptgfx_cfg.s.size = 1; /* 5kb, 2.5kb, 2.5kb */

			/* Must set reset bit when size changes */
			pko_ptgfx_cfg.s.reset = 1;
                    	break;
                    default: /* 4 */
                    	pko_ptgfx_cfg.s.size = 3; /* 10kb */

			/* Must set reset bit when size changes */
			pko_ptgfx_cfg.s.reset = 1;
                    	break;
            	}
            	break;
            case 1: /* 5kb, 2.5kb, 2.5kb */
            	/* Can only possible combine the two 2.5kb */
            	switch (fifo_req_size)
            	{
                    case 1:
                    	/* No change */
                    	pko_ptgfx_cfg.s.size = 1; /* 5kb, 2.5kb, 2.5kb */
                    	break;
                    case 2:
                    	/* 2nd buffer is now 5kb */
                    	pko_ptgfx_cfg.s.size = 2; /* 5kb, 5kb */

			/* Must set reset bit when size changes */
			pko_ptgfx_cfg.s.reset = 1;
                    	break;
                    default: /* 4 */
                    	/* This shouldn't be possible */
                    	cvmx_dprintf("Error: Illegal fifo size\n");
                    	break;
            	}
            	break;
	    default: /* other cases */
		cvmx_dprintf("Error: Illegal fifo size\n");
            	break;
    	}
	
	cvmx_write_csr_node(node, CVMX_PKO_PTGFX_CFG(index), 
					pko_ptgfx_cfg.u64);

	/* Setup PKO MCI0 credits */
	credit = fifo_req_size * 2500;
	switch (pko_mac_num)
	{
		case 0: /* loopback */
			mac_credit = 4096; /* From HRM */
            		break;
        	case 1: /* DPI */
            		mac_credit = 40<<10; /* FIXME: Guess at MAC size */
            		break;
        	case 2: /* ILK0 */
        	case 3: /* ILK1 */
            		mac_credit = 40<<10; /* FIXME: Guess at MAC size */
            		break;
        	default: /* BGX */
            		mac_credit = fifo_req_size * (10<<10); /* 10KB, 20KB, or 40KB */
            		break;
    	}

	credit += mac_credit;
	pko_mci0_max_cred.u64 = 0;
	pko_mci0_max_cred.s.max_cred_lim = credit / 16;
	cvmx_write_csr_node(node, CVMX_PKO_MCI0_MAX_CREDX(pko_mac_num),
				pko_mci0_max_cred.u64);

	pko_mci1_max_cred.u64 = 0;
	pko_mci1_max_cred.s.max_cred_lim = (mac_credit / 16);
	cvmx_write_csr_node(node, CVMX_PKO_MCI1_MAX_CREDX(pko_mac_num), 
				pko_mci1_max_cred.u64);

	return fifo;
}

/*
 * PKO setup per Interface/port.
 *
 * @param node is to specify which node's pko block for this setup.
 * @param interface is the interface number.
 * @return returns 0 if successful and -1 on failure.
 */
int cvmx_pko_setup_interface_78xx(int node, int interface) 
{
	int port, num_ports;
	int pko_mac_num, fifo;
	cvmx_helper_interface_mode_t mode;
	cvmx_pko_macx_cfg_t pko_mac_cfg;
	
	mode = cvmx_helper_interface_get_mode(interface);
	num_ports = cvmx_helper_interface_enumerate(interface);

	if ((mode == CVMX_HELPER_INTERFACE_MODE_ILK) || 
		(mode == CVMX_HELPER_INTERFACE_MODE_NPI) ||
		(mode == CVMX_HELPER_INTERFACE_MODE_LOOP))
		num_ports = 1;

	for (port = 0; port < num_ports; port++) 
	{
		/* setup Tx fifo and mac credits for this port */
		pko_mac_num = __cvmx_pko_get_mac_num(interface, port); 
		if (pko_mac_num < 0)
			return -1;

		/* get fifo number for pko_mac_num */
		pko_mac_cfg.u64 = cvmx_read_csr_node(node, CVMX_PKO_MACX_CFG(pko_mac_num));
		/* allocate fifo for this mac */
		if (pko_mac_cfg.s.fifo_num == 0x1f) {
			fifo = __cvmx_pko_setup_fifo(node, interface, pko_mac_num);
			if (fifo < 0)
				return -1;
			pko_mac_cfg.s.fifo_num = fifo;
			cvmx_write_csr_node(node, CVMX_PKO_MACX_CFG(pko_mac_num), pko_mac_cfg.u64);
		}

		/* setup per port queue topology */
	        __cvmx_pko_setup_queue_topology(node, interface, port);
	}

	return 0;
}
