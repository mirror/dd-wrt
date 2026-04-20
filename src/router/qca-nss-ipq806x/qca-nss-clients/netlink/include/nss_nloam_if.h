/*
 **************************************************************************
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
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
 * @file nss_nloam_if.h
 *	NSS Netlink OAM headers
 */

#ifndef __NSS_NLOAM_IF_H
#define __NSS_NLOAM_IF_H

/*
 * OAM Family name
 *	OAM - Operations, Administration and Maintenance Service
 *	This netlink family is meant for servicing the request from the OAM proxy.
 */
#define NSS_NLOAM_FAMILY "nss_nloam"
#define NSS_NLOAM_MCAST_GRP "nss_nloam_mc"

#define NSS_NLOAM_ABS_TIME_WIDTH 4	/**< words*/
#define NSS_NLOAM_BIN_DATA_SZ 256
#define NSS_NLOAM_RESP_ERR_SZ 512

/*
 * @brief NSS oam command types
 *
 * @note These are the command types from oam proxy to nss oam server via oam adapter
 */
enum nss_nloam_cmd {
	NSS_NLOAM_CMD_NONE = 0,			/**< none */
	NSS_NLOAM_CMD_SET_REQ = 1,		/**< set request to nss */
	NSS_NLOAM_CMD_GET_REQ = 2,		/**< get request to nss */
	NSS_NLOAM_CMD_CLR_REQ = 3,		/**< clr request to nss*/
	NSS_NLOAM_CMD_MAX,			/**< Max command */
};

/*
 * @brief NSS oam command types
 *
 * @note These are the sub command types from oam proxy to nss oam server for get command
 */
enum nss_nloam_get_type {
	NSS_NLOAM_GET_TYPE_NONE = 0x0,
	NSS_NLOAM_GET_TYPE_FW_VERSION = 0x20050001,	/**< sub type get fw version */
	NSS_NLOAM_GET_TYPE_UNKNOWN = 0xFFFFFFFF,	/**< unknown type (-1) */
};

/*
 * @brief NSS oam response types
 *
 * @note These are the response types from nss oam server to oam proxy server via oam adapter
 */
enum nss_nloam_resp_type {
	NSS_NLOAM_RESP_TYPE_SET = 1,	/**< resp for set request command */
	NSS_NLOAM_RESP_TYPE_GET = 2,	/**< resp for get reqest command */
	NSS_NLOAM_RESP_TYPE_MAX,	/**< max response type */
};

/*
 * @brief message format for oam get command type
 *
 * @note get message format oam proxy to server as per protocol document
 */
struct nss_nloam_get_req {
	uint16_t trans_id;
	uint16_t batch_sz;

	uint32_t rec_id;

	uint8_t clear_on_read;
	uint8_t res;
};

/*
 * @brief message format for oam get resp
 *
 * @note get message format server to oam proxy as per protocol document
 */
struct nss_nloam_get_resp {
	uint16_t trans_id;
	uint8_t mgmt_res;
	uint8_t time_source;

	uint32_t abs_time[NSS_NLOAM_ABS_TIME_WIDTH];
	uint32_t rec_id;
	uint32_t error_no;

	uint16_t length;
	uint8_t res[2];

	uint8_t bin[NSS_NLOAM_BIN_DATA_SZ];
};

/*
 * @brief message format for oam set command type
 *
 * @note set message format from oam proxy to server as per protocol document
 */
struct nss_nloam_set_req {
	uint16_t trans_id;
	uint8_t mgmt_op_type;
	uint8_t reg_dwnld_flag;

	uint32_t rec_id;

	uint16_t length;
	uint8_t apply_action;
	uint8_t res;

	uint8_t bin[NSS_NLOAM_BIN_DATA_SZ];
};

/*
 * @brief message format for oam set resp
 *
 * @note set message format server to oam proxy as per protocol document
 */
struct nss_nloam_set_resp {
	uint16_t trans_id;
	uint16_t apply_action;

	uint32_t error_no;

	uint16_t length;
	uint8_t mgmt_res;
	uint8_t res;

	uint8_t error_msg[NSS_NLOAM_RESP_ERR_SZ];	/**< null terminated string */
};

/*
 * @brief oam session info
 */
struct nss_nloam_rule {
	struct nss_nlcmn cm;
	union {
		struct nss_nloam_set_req set_req;	/**< set request from oam adapter */
		struct nss_nloam_set_resp set_resp;	/**< set response to oam adapter */
		struct nss_nloam_get_req get_req;	/**< get request from oam adapter */
		struct nss_nloam_get_resp get_resp;	/**< get response to oam adapter */
	} msg;
};

#endif /* __NSS_OAM_IF_H */
