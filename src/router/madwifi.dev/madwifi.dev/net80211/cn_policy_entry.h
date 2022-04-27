/* Copyright TMM (Private) Limited 2007
 * All Rights Reserved.
 *
 * THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
 * PROPERTY OF TMM (PRIVATE) LIMITED OR ITS LICENSORS AND IS SUBJECT
 * TO LICENSE TERMS.
 *
 * Email: info@tmm-ltd.com
 * Website: www.tmm-ltd.com
 *
 * $Id$
 */

#ifndef _NET80211_CN_POLICY_ENTRY_H_
#define _NET80211_CN_POLICY_ENTRY_H_
struct cn_policy_entry  {
	TAILQ_ENTRY(cn_policy_entry) entries;
	u_int32_t last_data;
	 u_int16_t timeout;
	 u_int8_t mac_address[IEEE80211_ADDR_LEN];
	 u_int8_t multiplier;
	 u_int8_t current_multiplier;
	 u_int8_t idle_multiplier;
	 u_int8_t idle_polls;
	 u_int8_t is_idle;
};
 struct cn_policy_state  {
	TAILQ_HEAD(, cn_policy_entry) policy_list;
	u_int32_t clients;
};
 
#endif	/*  */
