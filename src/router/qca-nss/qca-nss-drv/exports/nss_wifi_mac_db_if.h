/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

 /**
  * @file nss_wifi_mac_db_if.h
  *	NSS-to-HLOS interface definitions.
  */
#ifndef __NSS_WIFI_MAC_DB_H
#define __NSS_WIFI_MAC_DB_H

#define NSS_WIFI_MAC_DB_ENTRY_IF_LOCAL 0x1

/**
 * nss_wifi_mac_db_msg_types
 *	Wi-Fi MAC database messages.
 */
enum nss_wifi_mac_db_msg_types {
	NSS_WIFI_MAC_DB_INIT_MSG,		/**< Wi-Fi MAC database initialization message. */
	NSS_WIFI_MAC_DB_ADD_ENTRY_MSG,		/**< Wi-Fi MAC database add entry message. */
	NSS_WIFI_MAC_DB_DEL_ENTRY_MSG,		/**< Wi-Fi MAC database delete entry message. */
	NSS_WIFI_MAC_DB_UPDATE_ENTRY_MSG,	/**< Wi-Fi MAC database update entry message. */
	NSS_WIFI_MAC_DB_DEINIT_MSG,		/**< Wi-Fi MAC database deinitialization message. */
	NSS_WIFI_MAC_DB_MAX_MSG
};

/**
 * nss_wifi_mac_db_iftype
 * 	Wi-Fi MAC database interface type.
 */
enum nss_wifi_mac_db_iftype {
	NSS_WIFI_MAC_DB_ENTRY_IFTYPE_NONE,
	NSS_WIFI_MAC_DB_ENTRY_IFTYPE_VAP,	/**< Wi-Fi MAC database VAP entry interface. */
	NSS_WIFI_MAC_DB_ENTRY_IFTYPE_NON_VAP,	/**< Wi-Fi MAC database non-VAP entry interface. */
	NSS_WIFI_MAC_DB_ENTRY_IFTYPE_MAX	/**< Wi-Fi MAC database maximum interface. */
};

/**
 * nss_wifi_mac_db_if_opmode
 * 	Wi-Fi MAC database interface operation mode.
 */
enum nss_wifi_mac_db_if_opmode {
	NSS_WIFI_MAC_DB_ENTRY_IF_OPMODE_NONE,	/**< No entry database interface operation mode. */
	NSS_WIFI_MAC_DB_ENTRY_IF_OPMODE_ETH,	/**< Ethernet entry database interface operation mode. */
	NSS_WIFI_MAC_DB_ENTRY_IF_OPMODE_WIFI_AP,	/**< Wi-Fi AP entry database interface operation mode. */
	NSS_WIFI_MAC_DB_ENTRY_IF_OPMODE_WIFI_STA,	/**< Wi-Fi station entry database interface operation mode. */
	NSS_WIFI_MAC_DB_ENTRY_IF_OPMODE_MAX	/**< Maximum entry database interface operation mode. */
};

/**
 * nss_wifi_mac_db_entry_info_msg
 *	Wi-Fi MAC database entry information.
 */
struct nss_wifi_mac_db_entry_info_msg {
	uint8_t mac_addr[6];		/**< MAC address. */
	uint16_t flag;			/**< Flag information about NSS interface. */
	int32_t nss_if;		    	/**< NSS interface number. */
	uint32_t iftype;		/**< NSS interface type. */
	uint32_t opmode;		/**< NSS interface operation mode. */
	uint32_t wiphy_ifnum;		/**< NSS interface for wireless physical device. */
};

/**
 * nss_wifi_mac_db_msg
 *	Structure that describes Wi-Fi MAC database messages.
 */
struct nss_wifi_mac_db_msg {
	struct nss_cmn_msg cm;                  /**< Common message header. */

	/**
	 * Payload of Wi-Fi MAC database message.
	 */
	union {
		struct nss_wifi_mac_db_entry_info_msg nmfdbeimsg;
				/**< Wi-Fi MAC database information specific message. */
	} msg;			/**< Message payload. */
};

/**
 * nss_wifi_mac_db_msg_callback_t
 *	Callback to receive Wi-Fi MAC database messages.
 *
 * @datatypes
 * nss_wifi_mac_db_msg
 *
 * @param[in] app_data Application context of the message.
 * @param[in] msg      Message data.
 *
 * @return
 * void
 */
typedef void (*nss_wifi_mac_db_msg_callback_t)(void *app_data, struct nss_wifi_mac_db_msg *msg);

/**
 * nss_wifi_mac_db_callback_t
 *	Callback to receive Wi-Fi MAC database messages.
 *
 * @datatypes
 * net_device \n
 * sk_buff \n
 * napi_struct
 *
 *
 * @param[in] netdev  Pointer to the associated network device.
 * @param[in] skb     Pointer to the data socket buffer.
 * @param[in] napi    Pointer to the NAPI structure.
 *
 * @return
 * void
 */
typedef void (*nss_wifi_mac_db_callback_t)(struct net_device *netdev, struct sk_buff *skb, struct napi_struct *napi);


/**
 * nss_wifi_mac_db_tx_msg
 *	Send Wi-Fi MAC database messages.
 *
 * @datatypes
 * nss_ctx_instance \n
 * nss_wifi_mac_db_msg
 *
 * @param[in] nss_ctx NSS context.
 * @param[in] msg     NSS Wi-Fi MAC database message.
 *
 * @return
 * nss_tx_status_t Tx status
 */
extern nss_tx_status_t nss_wifi_mac_db_tx_msg(struct nss_ctx_instance *nss_ctx, struct nss_wifi_mac_db_msg *msg);

/**
 * nss_register_wifi_mac_db_if
 *	Register to send/receive Wi-Fi MAC database messages to NSS.
 *
 * @datatypes
 * nss_wifi_mac_db_callback_t \n
 * nss_wifi_mac_db_msg_callback_t \n
 * net_device
 *
 * @param[in] if_num             NSS interface number.
 * @param[in] mfdb_callback      Callback for the Wi-Fi MAC database device data.
 * @param[in] mfdb_ext_callback  Callback for the extended data.
 * @param[in] event_callback     Callback for the message.
 * @param[in] netdev             Pointer to the associated network device.
 * @param[in] features           Data socket buffer types supported by this
 *                               interface.
 *
 * @return
 * nss_ctx_instance* NSS context
 */
struct nss_ctx_instance *nss_register_wifi_mac_db_if(uint32_t if_num, nss_wifi_mac_db_callback_t wifi_mac_db_callback,
			nss_wifi_mac_db_callback_t wifi_mac_db_ext_callback, nss_wifi_mac_db_msg_callback_t event_callback, struct net_device *netdev, uint32_t features);

/**
 * nss_unregister_wifi_mac_db_if
 *	Deregister Wi-Fi MAC database SoC interface with NSS.
 *
 * @param[in] if_num NSS interface number.
 *
 * @return
 * void
 */
void nss_unregister_wifi_mac_db_if(uint32_t if_num);
struct nss_ctx_instance *nss_wifi_mac_db_get_context(void);
#endif /* __NSS_WIFI_MAC_DB_H */
