/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * nss_wifi_meshmgr.h
 *	Client WiFi-Mesh manager for NSS
 */
#ifndef ___NSS_WIFI_MESHMGR_H_
#define ___NSS_WIFI_MESHMGR_H_

#define NSS_WIFI_MESH_TX_TIMEOUT 3000	/**< Unit is in milliseconds. */
#define NSS_WIFI_MESH_HANDLE_INVALID -1 /**< Invalid mesh handle ID. */

/**
 * Mesh status enums
 */
typedef enum {
	/*
	 * nss_tx_status_t enums
	 */
	NSS_WIFI_MESHMGR_SUCCESS = NSS_TX_SUCCESS,				/**< Wi-Fi mesh transmit success. */
	NSS_WIFI_MESHMGR_FAILURE = NSS_TX_FAILURE,				/**< Wi-Fi mesh transmit failure. */
	NSS_WIFI_MESHMGR_FAILURE_QUEUE = NSS_TX_FAILURE_QUEUE,			/**< Wi-Fi mesh transmit enqueue failed. */
	NSS_WIFI_MESHMGR_FAILURE_NOT_READY = NSS_TX_FAILURE_NOT_READY,		/**< Wi-Fi mesh tranmsit core not ready. */
	NSS_WIFI_MESHMGR_FAILURE_TOO_LARGE = NSS_TX_FAILURE_TOO_LARGE,		/**< Wi-Fi mesh transmit message too large. */
	NSS_WIFI_MESHMGR_FAILURE_TOO_SHORT = NSS_TX_FAILURE_TOO_SHORT,		/**< WI-Fi mesh transmit message too short. */
	NSS_WIFI_MESHMGR_FAILURE_NOT_SUPPORTED = NSS_TX_FAILURE_NOT_SUPPORTED,	/**< WI-Fi mesh transmit message not supported. */
	NSS_WIFI_MESHMGR_FAILURE_BAD_PARAM = NSS_TX_FAILURE_BAD_PARAM,		/**< Wi-Fi mesh transmit invalid parameter. */
	NSS_WIFI_MESHMGR_FAILURE_SYNC_TIMEOUT = NSS_TX_FAILURE_SYNC_TIMEOUT,	/**< Wi-Fi mesh transmit transmit timed out. */

	/*
	 * Mesh specific ones.
	 */
	NSS_WIFI_MESHMGR_FAILURE_NULL_MESH_CTX = 100,			/**< Wi-Fi NULL mesh context. */
	NSS_WIFI_MESHMGR_FAILURE_UNKNOWN_MSG,				/**< Wi-Fi mesh unknown message error. */
	NSS_WIFI_MESHMGR_FAILURE_TTL_CONFIG,				/**< Wi-Fi mesh invalid ttl error. */
	NSS_WIFI_MESHMGR_FAILURE_REFRESH_TIME_CONFIG,			/**< Wi-Fi mesh invalid refresh time. */
	NSS_WIFI_MESHMGR_FAILURE_MPP_LEARNING_MODE_CONFIG,			/**< Wi-Fi mesh invalid mpp learning mode. */
	NSS_WIFI_MESHMGR_FAILURE_PATH_ADD_MAX_RADIO_CNT,			/**< Wi-Fi mesh path add error due to max radio count */
	NSS_WIFI_MESHMGR_FAILURE_PATH_ADD_INVALID_INTERFACE_NUM,		/**< Wi-Fi mesh path invalid interface number */
	NSS_WIFI_MESHMGR_FAILURE_PATH_ADD_INTERFACE_NUM_NOT_FOUND,		/**< Wi-Fi mesh path interface number not found. */
	NSS_WIFI_MESHMGR_FAILURE_PATH_TABLE_FULL,				/**< Wi-Fi mesh path table full error */
	NSS_WIFI_MESHMGR_FAILURE_PATH_ALLOC_FAIL,				/**< Wi-Fi mesh path alloc error */
	NSS_WIFI_MESHMGR_FAILURE_PATH_INSERT_FAIL,				/**< Wi-Fi mesh path insert fail */
	NSS_WIFI_MESHMGR_FAILURE_PATH_NOT_FOUND,				/**< Wi-Fi mesh path not found error */
	NSS_WIFI_MESHMGR_FAILURE_PATH_UNHASHED,				/**< Wi-Fi mesh proxy path unhashed error */
	NSS_WIFI_MESHMGR_FAILURE_PATH_DELETE_FAIL,				/**< Wi-Fi mesh proxy path delete error */
	NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_NOT_FOUND,			/**< Wi-Fi mesh proxy path not found error */
	NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_UNHASHED,			/**< Wi-Fi mesh proxy path unhashed error */
	NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_DELETE_FAIL,			/**< Wi-Fi mesh proxy path delete error */
	NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_EXISTS,				/**< Wi-Fi mesh proxy path exists error */
	NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_ALLOC_FAIL,			/**< Wi-Fi mesh proxy path alloc error */
	NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_INSERT_FAIL,			/**< Wi-Fi mesh proxy path insert error */
	NSS_WIFI_MESHMGR_FAILURE_PROXY_PATH_TABLE_FULL,			/**< Wi-Fi mesh proxy path table full error */
	NSS_WIFI_MESHMGR_FAILURE_PB_ALLOC_FAIL,				/**< Wi-Fi mesh pb allocation failures */
	NSS_WIFI_MESHMGR_FAILURE_ENQUEUE_TO_HOST_FAIL,			/**< Wi-Fi mesh enqueue to host failure */
	NSS_WIFI_MESHMGR_FAILURE_ENABLE_INTERFACE_FAIL,			/**< Wi-Fi mesh enabling interface failure */
	NSS_WIFI_MESHMGR_FAILURE_DISABLE_INTERFACE_FAIL,			/**< Wi-Fi mesh disabling interface failure */
	NSS_WIFI_MESHMGR_FAILURE_INVALID_EXCEPTION_NUM,				/**< Wi-Fi mesh invalid exception number */
	NSS_WIFI_MESHMGR_FAILURE_ONESHOT_ALREADY_ATTACHED,			/**< Wi-Fi mesh oneshot already attached error */
	NSS_WIFI_MESHMGR_FAILURE_DUMMY_PATH_ADD,				/**< Wi-Fi mesh dummy path add failure */
	NSS_WIFI_MESHMGR_FAILURE_DUMMY_PROXY_PATH_ADD,				/**< Wi-Fi mesh dummy proxy path add failure */
} nss_wifi_meshmgr_status_t;

/**
 * nss_wifi_mesh_handle_t
 *	Opaque mesh handler
 */
typedef int32_t nss_wifi_mesh_handle_t;

/**
 * nss_wifi_meshmgr_tx_buf
 *	Sends a Wi-Fi mesh data packet to the NSS interface.
 *
 * Note: This API should is used in testing purpose for sending
 * packets to NSS mesh I/F directly. It should be avoided by the
 * user.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	os_buf		Pointer to the OS data buffer.
 *
 * @return
 * Status of the Tx operation.
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_tx_buf(nss_wifi_mesh_handle_t mesh_handle, struct sk_buff *os_buf);

/**
 * nss_wifi_meshmgr_if_down()
 *	Make the NSS interface down synchronously.
 *
 * Note: This is blocking API and should not be called from
 * atomic context.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_if_down(nss_wifi_mesh_handle_t mesh_handle);

/**
 * nss_wifi_meshmgr_if_up()
 *	Make the NSS interface up synchronously.
 *
 * Note: This is blocking API and should not be called from
 * atomic context.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_if_up(nss_wifi_mesh_handle_t mesh_handle);

/**
 * nss_wifi_meshmgr_dump_mesh_path()
 *	Send dump mesh path table request.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_dump_mesh_path(nss_wifi_mesh_handle_t mesh_handle,
						       nss_wifi_mesh_msg_callback_t msg_cb,
						       void *app_data);

/**
 * nss_wifi_meshmgr_dump_mesh_path_sync()
 *	Send dump mesh path table request synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_dump_mesh_path_sync(nss_wifi_mesh_handle_t mesh_handle);

/**
 * nss_wifi_meshmgr_dump_mesh_proxy_path()
 *	Dump mesh proxy path table request.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_dump_mesh_proxy_path(nss_wifi_mesh_handle_t mesh_handle,
							     nss_wifi_mesh_msg_callback_t msg_cb,
							     void *app_data);

/**
 * nss_wifi_meshmgr_dump_mesh_proxy_path_sync()
 *	Send dump mesh proxy path table request synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_dump_mesh_proxy_path_sync(nss_wifi_mesh_handle_t mesh_handle);

/**
 * nss_wifi_meshmgr_assoc_link_vap
 *	Associate the link interface to the mesh I/F.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_assoc_link_vap \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmalv		WiFi interface link vap association message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_assoc_link_vap(nss_wifi_mesh_handle_t mesh_handle,
						       struct nss_wifi_mesh_assoc_link_vap *wmalv,
						       nss_wifi_mesh_msg_callback_t msg_cb,
						       void *app_data);

/**
 * nss_wifi_meshmgr_assoc_link_vap_sync
 *	Associate the link interface to the mesh I/F synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_assoc_link_vap \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmalv		WiFi interface link vap association message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t
nss_wifi_meshmgr_assoc_link_vap_sync(nss_wifi_mesh_handle_t mesh_handle, struct nss_wifi_mesh_assoc_link_vap *wmalv);

/**
 * nss_wifi_meshmgr_mesh_config_update
 *	Update mesh configuration message.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_config_msg \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmcum		WiFi interface configuration message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_config_update(nss_wifi_mesh_handle_t mesh_handle,
							   struct nss_wifi_mesh_config_msg *wmcum,
							   nss_wifi_mesh_msg_callback_t msg_cb,
							   void *app_data);
/**
 * nss_wifi_meshmgr_mesh_config_update_sync
 *	Send mesh configuration update message synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_config_msg \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmcum		WiFi interface configuration message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_config_update_sync(nss_wifi_mesh_handle_t mesh_handle,
								struct nss_wifi_mesh_config_msg *wmcum);

/**
 * nss_wifi_meshmgr_mesh_proxy_path_delete
 *	Send mesh proxy path delete.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_proxy_path_del_msg \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmppdm		WiFi mesh proxy path delete message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_proxy_path_delete(nss_wifi_mesh_handle_t mesh_handle,
							       struct nss_wifi_mesh_proxy_path_del_msg *wmppdm,
							       nss_wifi_mesh_msg_callback_t msg_cb,
							       void *app_data);

/**
 * nss_wifi_meshmgr_mesh_proxy_path_delete_sync
 *	Send mesh proxy path delete synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_proxy_path_del_msg \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmppdm		WiFi mesh proxy path delete message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_proxy_path_delete_sync(nss_wifi_mesh_handle_t mesh_handle,
								    struct nss_wifi_mesh_proxy_path_del_msg *wmppdm);

/**
 * nss_wifi_meshmgr_mesh_proxy_path_update
 *	Send mesh proxy path update.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_proxy_path_update_msg \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmppum		WiFi mesh proxy path update message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_proxy_path_update(nss_wifi_mesh_handle_t mesh_handle,
							       struct nss_wifi_mesh_proxy_path_update_msg *wmppum,
							       nss_wifi_mesh_msg_callback_t msg_cb,
							       void *app_data);

/**
 * nss_wifi_meshmgr_mesh_proxy_path_update_sync
 *	Send mesh proxy path update synchronously
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_proxy_path_update_msg \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmppum		WiFi mesh proxy path update message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_proxy_path_update_sync(nss_wifi_mesh_handle_t mesh_handle,
								    struct nss_wifi_mesh_proxy_path_update_msg *wmppum);

/**
 * nss_wifi_meshmgr_mesh_proxy_path_add
 *	Send mesh proxy path add message.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_proxy_path_add_msg\n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmppam		WiFi mesh proxy path add message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_proxy_path_add(nss_wifi_mesh_handle_t mesh_handle,
							    struct nss_wifi_mesh_proxy_path_add_msg *wmppam,
							    nss_wifi_mesh_msg_callback_t msg_cb,
							    void *app_data);

/**
 * nss_wifi_meshmgr_mesh_proxy_path_add_sync
 *	Send mesh proxy path add message synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_proxy_path_add_msg\n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmppam		WiFi mesh proxy path add message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_proxy_path_add_sync(nss_wifi_mesh_handle_t mesh_handle,
								 struct nss_wifi_mesh_proxy_path_add_msg *wmppam);

/**
 * nss_wifi_meshmgr_mesh_path_delete
 *	Send mesh path delete message.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_mpath_del_msg \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmpdm		WiFi mesh path delete message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_delete(nss_wifi_mesh_handle_t mesh_handle,
							 struct nss_wifi_mesh_mpath_del_msg *wmpdm,
							 nss_wifi_mesh_msg_callback_t msg_cb,
							 void *app_data);

/**
 * nss_wifi_meshmgr_mesh_path_delete_sync
 *	Send mesh path delete message synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_mpath_del_msg \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmpdm		WiFi mesh path delete message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_delete_sync(nss_wifi_mesh_handle_t mesh_handle,
							      struct nss_wifi_mesh_mpath_del_msg *wmpdm);

/**
 * nss_wifi_meshmgr_mesh_path_update
 *	Send mesh path update message.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_mpath_update_msg \n
 * nss_wifi_mesh_msg_callback_t \n
 * msg_cb
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmpum		WiFi mesh path update message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_update(nss_wifi_mesh_handle_t mesh_handle,
							 struct nss_wifi_mesh_mpath_update_msg *wmpum,
							 nss_wifi_mesh_msg_callback_t msg_cb,
							 void *app_data);

/**
 * nss_wifi_meshmgr_mesh_path_update_sync
 *	Send mesh path update message synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_mpath_update_msg \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmpum		WiFi mesh path update message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_update_sync(nss_wifi_mesh_handle_t mesh_handle,
							      struct nss_wifi_mesh_mpath_update_msg *wmpum);

/**
 * nss_wifi_meshmgr_mesh_path_add
 *	Send mesh path add message.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_mpath_add_msg \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle     Pointer to the mesh handle.
 * @param[in]	wmpam		WiFi mesh path add message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_add(nss_wifi_mesh_handle_t mesh_handle,
						      struct nss_wifi_mesh_mpath_add_msg *wmpam,
						      nss_wifi_mesh_msg_callback_t msg_cb,
						      void *app_data);

/**
 * nss_wifi_meshmgr_mesh_path_add_sync
 *	Send mesh path add message synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_mpath_add_msg \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmpam		WiFi mesh path add message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_add_sync(nss_wifi_mesh_handle_t mesh_handle,
							   struct nss_wifi_mesh_mpath_add_msg *wmpam);

/**
 * nss_wifi_meshmgr_mesh_path_exception
 *	Send mesh path exception message.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_exception_flag_msg \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle     Pointer to the mesh handle.
 * @param[in]	wmpefm		WiFi mesh path exception message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_exception(nss_wifi_mesh_handle_t mesh_handle,
						      struct nss_wifi_mesh_exception_flag_msg *wmpefm,
						      nss_wifi_mesh_msg_callback_t msg_cb,
						      void *app_data);

/**
 * nss_wifi_meshmgr_mesh_path_exception_sync
 *	Send mesh path update message synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_exception_flag_msg \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmpefm		WiFi mesh path exception flag message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_mesh_path_exception_sync(nss_wifi_mesh_handle_t mesh_handle,
							      struct nss_wifi_mesh_exception_flag_msg *wmpefm);

/**
 * nss_wifi_meshmgr_config_mesh_exception
 *	Configure mesh exceptions.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_rate_limit_config \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]	mesh_handle     Pointer to the mesh handle.
 * @param[in]	wmrlc		WiFi mesh exception config message.
 * @param[in]	msg_cb		Callback for NACK/ACK messages from NSS.
 * @param[in]	app_data	Application data for the message callback.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_config_mesh_exception(nss_wifi_mesh_handle_t mesh_handle,
						      struct nss_wifi_mesh_rate_limit_config *wmrlc,
						      nss_wifi_mesh_msg_callback_t msg_cb,
						      void *app_data);

/**
 * nss_wifi_meshmgr_config_mesh_exception_sync
 *	configure mesh exception synchronously.
 *
 * @datatypes
 * nss_wifi_mesh_handle_t \n
 * nss_wifi_mesh_rate_limit_config \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 * @param[in]	wmrlc		WiFi mesh config exception message.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_config_mesh_exception_sync(nss_wifi_mesh_handle_t mesh_handle,
							      struct nss_wifi_mesh_rate_limit_config *wmrlc);

/**
 * nss_wifi_meshmgr_mesh_if_destroy_sync
 *	Destroy NSS mesh interfaces synchronously.
 *
 * @dataypes
 * nss_wifi_mesh_handle_t \n
 *
 * @param[in]	mesh_handle	Pointer to the mesh handle.
 *
 * @return
 * Status
 */
extern nss_wifi_meshmgr_status_t nss_wifi_meshmgr_if_destroy_sync(nss_wifi_mesh_handle_t mesh_handle);

/**
 * nss_wifi_meshmgr_mesh_if_create_sync
 *	Create NSS mesh interface synchronously.
 *
 * @datatypes
 * net_device \n
 * nss_wifi_mesh_config_msg \n
 * nss_wifi_mesh_data_callback_t \n
 * nss_wifi_mesh_ext_data_callback_t \n
 * nss_wifi_mesh_msg_callback_t \n
 *
 * @param[in]		dev		Pointer to the associated network device.
 * @param[in]		config_msg	Pointer to the mesh configuration message.
 * @param[in]		data_cb		Callback for the Wi-Fi mesh device data.
 * @param[in]		ext_data_cb	Callback for the extended data.
 * @param[in]		event_cb	Callback for the NSS to Host notify mesage.
 *
 * @return
 * Mesh handle
 */
extern nss_wifi_mesh_handle_t nss_wifi_meshmgr_if_create_sync(struct net_device *dev, struct nss_wifi_mesh_config_msg *config_msg,
							      nss_wifi_mesh_data_callback_t data_cb,
							      nss_wifi_mesh_ext_data_callback_t ext_data_cb,
							      nss_wifi_mesh_msg_callback_t event_cb);
#endif /* __NSS_WIFI_MESHMGR_H */
