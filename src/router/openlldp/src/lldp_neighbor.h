/*******************************************************************
 *
 * OpenLLDP Neighbor Header
 *
 * Licensed under a dual GPL/Proprietary license.  
 * See LICENSE file for more info.
 * 
 * File: lldp_neighbor.h
 * 
 * Authors: Jason Peterson (condurre@users.sourceforge.net)
 *
 *******************************************************************/

#ifndef LLDP_NEIGHBOR_H
#define LLDP_NEIGHBOR_H



char lldp_systemname[512];
char lldp_systemdesc[512];

int get_sys_desc(void);
int get_sys_fqdn(void);

#endif /* LLDP_NEIGHBOR_H */
