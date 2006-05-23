/*
 * ADMtek switch setup functions
 *
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.                
 *                                     
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;   
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior      
 * written permission of Broadcom Corporation.                            
 *
 * $Id$
 */

#ifndef _adm_h_
#define _adm_h_

/* forward declarations */
typedef struct adm_info_s adm_info_t;

/* interface prototypes */
extern adm_info_t *adm_attach(sb_t *sbh, char *vars);
extern void adm_detach(adm_info_t *adm);
extern int adm_enable_device(adm_info_t *adm);
extern int adm_config_vlan(adm_info_t *adm);

#endif /* _adm_h_ */

