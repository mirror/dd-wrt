/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: mapmgr.h,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#ifndef _mapmgr_h_
#define _mapmgr_h_

typedef struct _netconf_nat_t mapping_t;

void mapmgr_update();
bool mapmgr_get_port_map(int n, mapping_t *m);
bool mapmgr_add_port_map(mapping_t *m);
bool mapmgr_delete_port_map(int n);
bool mapmgr_get_range_map(int n, mapping_t *m);
bool mapmgr_add_range_map(mapping_t *m);
bool mapmgr_delete_range_map(int n);
int mapmgr_port_map_count();
int mapmgr_range_map_count();

#define MAX_PORT_MAPPINGS FD_SETSIZE


#endif /* _mapmgr_h_ */
