/*
 Copyright 2004-2010 Broadcom Corp. All Rights Reserved.

 <:label-BRCM:2011:DUAL/GPL:standard    
 
 Unless you and Broadcom execute a separate written software license
 agreement governing use of this software, this software is licensed
 to you under the terms of the GNU General Public License version 2
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give
    you permission to link this software with independent modules, and
    to copy and distribute the resulting executable under terms of your
    choice, provided that you also meet, for each linked independent
    module, the terms and conditions of the license of that module.
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications
    of the software.
 
 Not withstanding the above, under no circumstances may you combine
 this software in any way with any other Broadcom software provided
 under a license other than the GPL, without Broadcom's express prior
 written consent.
 
 :>
*/
#ifndef _ETHSW_RUNNER_H_
#define _ETHSW_RUNNER_H_

#include "bcmtypes.h"
#include "bcmmii.h"


#define bcmeapi_ioctl_ethsw_dscp_to_priority_mapping(e) 0
#define bcmeapi_ioctl_ethsw_pcp_to_priority_mapping(e) 0
#define bcmeapi_ioctl_ethsw_set_multiport_address(e) 0


/****************************************************************************
    Prototypes
****************************************************************************/

#if defined(STAR_FIGHTER2)
void sf2_mdio_master_enable(void);
void sf2_mdio_master_disable(void);
#endif

#define ethsw_set_wanoe_portmap(wan_port_map) {}
#define bcmeapi_ethsw_init_config() {}
void ethsw_port_mirror_get(int *enable, int *mirror_port, unsigned int *ing_pmap,
                           unsigned int *eg_pmap, unsigned int *blk_no_mrr,
                           int *tx_port, int *rx_port);
void ethsw_port_mirror_set(int enable, int mirror_port, unsigned int ing_pmap,
                           unsigned int eg_pmap, unsigned int blk_no_mrr,
                           int tx_port, int rx_port);

#endif /* _ETHSW_RUNNER_H_ */
