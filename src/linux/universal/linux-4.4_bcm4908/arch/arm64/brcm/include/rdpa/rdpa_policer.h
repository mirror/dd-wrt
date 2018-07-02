/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef _RDPA_POLICER_H_
#define _RDPA_POLICER_H_

/**
 * \defgroup policer Traffic Policer
 * API in this group controls Runner Traffic Policing capabilities. Traffic policing allows you to control the maximum
 * rate of traffic.\n
 * Policer can be assigned to any type of  \ref ingress_class_d "classification flow". 
 * \ingroup tm
 * @{
 */

#define RDPA_TM_MAX_US_POLICER 16 /**< Max number of US policers */
#define RDPA_TM_MAX_DS_POLICER 16 /**< Max number of DS policers */

/** Traffic policer type */
typedef enum {
    rdpa_tm_policer_token_bucket,   /**< Simple tocken bucket */

    rdpa_tm_policer_type__num_of,   /* Number of possible types */
} rdpa_tm_policer_type;

/** Traffic policer action type */
typedef enum {
    rdpa_tm_policer_action_none,        /**< Do nothing */
    rdpa_tm_policer_action_drop,        /**< Discard */

    rdpa_tm_policer_action__num_of,     /* Number of possible actions */
} rdpa_tm_policer_action;

/** Policer configuration.
 * Underlying type for tm_policer_cfg aggregate type
 */
typedef struct {
    rdpa_tm_policer_type  type;             /**< Policer type */
    uint32_t commited_rate;                 /**< Committed Information Rate (CIR) - bps */
    uint32_t committed_burst_size;          /**< Committed Burst Size (CBS) - bytes */
    rdpa_tm_policer_action red_action;      /**< Action for non-conforming packets */
} rdpa_tm_policer_cfg_t;

/** Policer statistics.
 * Underlying structure for tm_policer_stat aggregate type
 */
typedef struct {
    rdpa_stat_t red;            /**< Red statistics */
} rdpa_tm_policer_stat_t;

#define RDPA_POLICER_MIN_SR       64000   /**< Min sustain rate */
#define RDPA_POLICER_MAX_SR       1000000000 /**< Max sustain rate */
#define RDPA_POLICER_SR_QUANTA    100   /**< Rate quanta */

/* @} end of policer Doxygen group */

#endif /* _RDPA_POLICER_H_ */
