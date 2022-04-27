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
#ifndef _NET80211_CN_POLLING_ENTRY_H_
#define _NET80211_CN_POLLING_ENTRY_H_
struct cn_polling_entry  {
	TAILQ_ENTRY(cn_polling_entry) entries;
	struct cn_policy_entry *client;
};
 struct cn_polling_state  {
	TAILQ_HEAD(, cn_polling_entry) polling_list;
	uint32_t poll_clients;
	uint8_t pkt_rcvd;
	uint8_t polling_started;
};
 
#endif	/*  */
