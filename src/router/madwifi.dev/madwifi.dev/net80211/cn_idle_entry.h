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
#ifndef _NET80211_CN_IDLE_ENTRY_H_
#define _NET80211_CN_IDLE_ENTRY_H_

#define CN_IDLE_REQ_MULTIPLIER	5
struct cn_idle_entry  {
	TAILQ_ENTRY(cn_idle_entry) entries;
	struct cn_policy_entry *client;
};
 struct cn_idle_state  {
	TAILQ_HEAD(, cn_idle_entry) idle_list;
	uint32_t idle_clients;
	uint32_t cn_idle_multiplier;
	uint32_t cn_req_multiplier;
};
 
#endif	/*  */
