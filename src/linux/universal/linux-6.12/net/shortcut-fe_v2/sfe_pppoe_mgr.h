/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * sfe_pppoe_mgr.h
 *	SFE PPPoE mgr definitions
 */

#ifndef _SFE_PPPOE_MGR_H_
#define _SFE_PPPOE_MGR_H_

/*
 * struct sfe_pppoe_mgr_session_info
 *	Structure for PPPoE client driver session info
 */
struct sfe_pppoe_mgr_session_info {
	uint32_t session_id;		/* PPPoE Session ID */
	uint8_t server_mac[ETH_ALEN];	/* PPPoE server's MAC address */
};

/*
 * struct sfe_pppoe_mgr_session_entry
 *	Structure for PPPoE session entry into HASH table
 */
struct sfe_pppoe_mgr_session_entry {
	struct sfe_pppoe_mgr_session_info info;
					/* Session information */
	struct net_device *dev;		/* Net device */
	struct hlist_node hash_list;	/* Hash list for sessions */
};

bool sfe_pppoe_mgr_find_session(uint16_t session_id, uint8_t *server_mac);
int sfe_pppoe_mgr_init(void);
void sfe_pppoe_mgr_exit(void);

#endif
