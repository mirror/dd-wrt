/*
 **************************************************************************
 * Copyright (c) 2015,2018-2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * @file nss_nlcmn_if.h
 *	NSS Netlink common headers
 */
#ifndef __NSS_NLCMN_IF_H
#define __NSS_NLCMN_IF_H

#define NSS_NLCMN_CB_MAX_SZ 64 /* bytes */

/**
 * @brief Common message header for each NSS netlink message
 */
struct nss_nlcmn {
	uint32_t version;			/**< message version */
	uint32_t pid;				/**< process ID for the message */
	nss_ptr_t sock_data;			/**< socket specific info, used by kernel */
	uint16_t cmd_len;			/**< command len */
	uint8_t cmd_type;			/**< command type */
	uint8_t res;				/**< reserve for future use */
	int32_t cb_owner;			/**< CB identifier */
	uint8_t cb_data[NSS_NLCMN_CB_MAX_SZ]; 	/**< user context buffer */
};

/**
 * @brief NSS subsystems in alphabetical order for nssinfo tool
 */
enum nss_nlcmn_subsys {
	NSS_NLCMN_SUBSYS_CAPWAP,
	NSS_NLCMN_SUBSYS_C2C_RX,
	NSS_NLCMN_SUBSYS_C2C_TX,
	NSS_NLCMN_SUBSYS_DYNAMIC_INTERFACE,
	NSS_NLCMN_SUBSYS_EDMA,
	NSS_NLCMN_SUBSYS_ETHRX,
	NSS_NLCMN_SUBSYS_IPV4,
	NSS_NLCMN_SUBSYS_IPV4_REASM,
	NSS_NLCMN_SUBSYS_IPV6,
	NSS_NLCMN_SUBSYS_IPV6_REASM,
	NSS_NLCMN_SUBSYS_L2TPV2,
	NSS_NLCMN_SUBSYS_LSO_RX,
	NSS_NLCMN_SUBSYS_MAP_T,
	NSS_NLCMN_SUBSYS_N2H,
	NSS_NLCMN_SUBSYS_PPPOE,
	NSS_NLCMN_SUBSYS_PPTP,
	NSS_NLCMN_SUBSYS_WIFILI,
	NSS_NLCMN_SUBSYS_MAX
};

/**
 * @brief messages senders must use this to initialize command
 *
 * @param cm[IN] common message
 * @param len[IN] command length
 * @param cmd[IN] command for the family
 */
static inline void nss_nlcmn_init_cmd(struct nss_nlcmn *cm, uint16_t len, uint8_t cmd)
{
	cm->cmd_type = cmd;
	cm->cmd_len = len;
}

/**
 * @brief check the version number of the incoming message
 *
 * @param cm[IN] common message header
 *
 * @return true on version match
 */
static inline bool nss_nlcmn_chk_ver(struct nss_nlcmn *cm, uint32_t ver)
{
	return cm->version == ver;
}

/**
 * @brief set the version number for common message header
 *
 * @param cm[IN] common message header
 * @param ver[IN] version number to apply
 */
static inline void nss_nlcmn_set_ver(struct nss_nlcmn *cm, uint32_t ver)
{
	cm->version = ver;
}

/**
 * @brief get the version number from common message header
 *
 * @param cm[IN] common message header
 *
 * @return version
 */
static inline uint32_t nss_nlcmn_get_ver(struct nss_nlcmn *cm)
{
	return cm->version;
}

/**
 * @brief get the NSS Family command type
 *
 * @param cm[IN] common message
 *
 * @return command type
 */
static inline uint8_t nss_nlcmn_get_cmd(struct nss_nlcmn *cm)
{
	return cm->cmd_type;
}

/**
 * @brief get the NSS Family command len
 *
 * @param cm[IN] common message
 *
 * @return command len
 */
static inline uint16_t nss_nlcmn_get_len(struct nss_nlcmn *cm)
{
	return cm->cmd_len;
}

/**
 * @brief get the callback data
 *
 * @param cm[IN] common message
 * @param cb_owner[IN] callback owner ID
 *
 * @return callback data or NULL if the owner doesn't match
 */
static inline void *nss_nlcmn_get_cb_data(struct nss_nlcmn *cm, int32_t cb_owner)
{
	/*
	 * if owner doesn't match then the caller is not the owner
	 */
	if (cm->cb_owner != cb_owner) {
		return NULL;
	}

	return cm->cb_data;
}

/**
 * @brief set the callback data ownership
 *
 * @param cm[IN] common message
 * @param cb_owner[IN] callback owner ID
 */
static inline void nss_nlcmn_set_cb_owner(struct nss_nlcmn *cm, int32_t cb_owner)
{
	cm->cb_owner = cb_owner;
}

/**
 * @brief clear the CB ownership (ID) after use
 *
 * @param cm[IN] common message
 */
static inline void nss_nlcmn_clr_cb_owner(struct nss_nlcmn *cm)
{
	nss_nlcmn_set_cb_owner(cm, -1);
}

#endif /* __NSS_NLCMN_IF_H */


