/*
   <:copyright-BRCM:2013-2016:DUAL/GPL:standard
   
      Copyright (c) 2013-2016 Broadcom 
      All Rights Reserved
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2, as published by
   the Free Software Foundation (the "GPL").
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   
   A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
   writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
   
:>
*/

#ifndef _RDD_IC_H
#define _RDD_IC_H

#define NUM_OF_GENERIC_RULE_CFG 4

void rdd_ic_debug_mode_enable(bdmf_boolean enable);
#ifdef CM3390
int rdd_ic_context_cfg(rdpa_traffic_dir direction, uint32_t context_id, const rdd_ic_context_t *context);
int rdd_ic_context_get(rdpa_traffic_dir direction, uint32_t context_id, rdd_ic_context_t *context);
#endif

#endif /* _RDD_IC_H */
