/*
 ***************************************************************************
 * Copyright (c) 2014-2015,2019, The Linux Foundation. All rights reserved.
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
 * NSS Netlink gre_redir headers
 */
#define NSS_NLGRE_REDIR_CMN_NEEDED_HEADROOM 192		/**< Maximum headroom available */
#define NSS_NLGRE_REDIR_CMN_RADIO_ID_MAX 2		/**< Radio Id max size */
#define NSS_NLGRE_REDIR_CMN_RADIO_ID_MIN 0		/**< Radio Id min size */
#define NSS_NLGRE_REDIR_CMN_VAP_ID_MAX 16		/**< Vap Id max size */
#define NSS_NLGRE_REDIR_CMN_VAP_ID_MIN 0		/**< Vap Id min size */
#define NSS_NLGRE_REDIR_CMN_MAX_TUNNELS 24		/**< Maximum number of tunnels allowed */
#define NSS_NLGRE_REDIR_CMN_TUN_TYPE_MAX_SZ 16		/**< Maximum size of tunnel type */
#define NSS_NLGRE_REDIR_CMN_MAX_SKB_PRINT_LEN 3		/**< Maximum length of skb to print */
#define NSS_NLGRE_REDIR_CMN_MIN_TUNNELS 0		/**< Mininum number of tunnels required */
#define NSS_NLGRE_REDIR_CMN_IP_TTL 128			/**< Time to live for IP */
#define NSS_NLGRE_REDIR_PKT_DUMP_SZ 64			/**< Size of packet to dump */
#define NSS_NLGRE_REDIR_PKT_DUMP_OFFSET 0		/**< Dump offset */

/*
 * netdevice private data
 */
struct nss_gre_redir_cmn_ndev_priv {
	uint32_t gre_seq;				/**< Sequence number */
};

/*
 * Context need to be maintained globally for GRE redirect tunnel.
 */
struct nss_nlgre_redir_cmn_tun_data {
	struct net_device *dev;				/**< Net device */
	int32_t host_inner_ifnum;			/**< Interface no. of pnode host inner */
	int32_t wifi_offl_inner_ifnum;			/**< Interface no. of pnode wifi offld inner */
	int32_t sjack_inner_ifnum;			/**< Interface no. of pnode sjack inner */
	int32_t outer_ifnum;				/**< Interface no. of pnode outer */
	bool enable;					/**< Device is enabled or not */
};

/*
 * nss_nlgre_redir_cmn_tun_type
 * 	Different tunnel types supported in gre_redir
 */
enum nss_nlgre_redir_cmn_tun_type {
	NSS_NLGRE_REDIR_CMN_TUN_TYPE_UNKNOWN,		/**< Unknown tunnel type */
	NSS_NLGRE_REDIR_CMN_TUN_TYPE_TUN,		/**< Raw mode 802.11 frames traffic*/
	NSS_NLGRE_REDIR_CMN_TUN_TYPE_DTUN,		/**< For 802.3 frames traffic */
	NSS_NLGRE_REDIR_CMN_TUN_TYPE_SPLIT,		/**< For split mode */
	NSS_NLGRE_REDIR_CMN_TUN_TYPE_MAX		/**< Max number of tun type supported */
};

/*
 * nss_nlgre_redir_cmn_mode_type
 * 	Modes available for setting next hop
 */
enum nss_nlgre_redir_cmn_mode_type {
	NSS_NLGRE_REDIR_CMN_MODE_TYPE_UNKNOWN,	/**< Unknown mode type */
	NSS_NLGRE_REDIR_CMN_MODE_TYPE_WIFI,	/**< ath0 ---> wifi_offld_inner */
	NSS_NLGRE_REDIR_CMN_MODE_TYPE_SPLIT,	/**< ath0 ---> ETH_RX_INTERFACE */
	NSS_NLGRE_REDIR_CMN_MODE_TYPE_MAX	/**< Max number of modes supported */
};

/*
 * nss_nlgre_redir_cmn_init()
 * 	Initializes the tun_data
 */
void nss_nlgre_redir_cmn_init(void);

/*
 * nss_nlgre_redir_cmn_get_tun_ifnum()
 * 	Returns the interface number corresponding to dev
 */
int32_t nss_nlgre_redir_cmn_get_tun_ifnum(enum nss_nlgre_redir_cmn_mode_type type, struct net_device *dev);
/*
 * nss_nlgre_redir_cmn_mode_str_to_enum()
 * 	Returns the enum converted value of the string
 */
enum nss_nlgre_redir_cmn_mode_type nss_nlgre_redir_cmn_mode_str_to_enum(char *mode);

/*
 * nss_nlgre_redir_cmn_get_tun_data_index()
 * 	Returns the interface number of dev
 */
int nss_nlgre_redir_cmn_get_tun_data_index(struct net_device *dev);

/*
 * nss_nlgre_redir_cmn_get_dev_ifnum()
 * 	Returns the interface number of dev
 */
int32_t nss_nlgre_redir_cmn_get_dev_ifnum(char *dev_name);

/*
 * nss_nlgre_redir_cmn_get_tun_type()
 * 	Returns the tunnel type
 */
enum nss_nlgre_redir_cmn_tun_type nss_nlgre_redir_cmn_get_tun_type(char *tun_type);

/*
 * nss_gre_redir_cmn_unregister_and_deallocate()
 * 	Unregisters and deallocates a node.
 */
bool nss_nlgre_redir_cmn_unregister_and_deallocate(struct net_device *dev, uint32_t type);

/*
 * nss_nlgre_redir_cmn_print_hex_dump()
 *	Prints the initials few bytes of packet
 */
void nss_nlgre_redir_cmn_print_hex_dump(struct sk_buff *skb);

/*
 * nss_nlgre_redir_cmn_create_tun()
 * 	Creates a gre_redir tunnel
 */
struct net_device *nss_nlgre_redir_cmn_create_tun(uint32_t sip[4], uint32_t dip[4], uint8_t iptype);

/*
 * nss_nlgre_redir_cmn_destroy_tun()
 * 	Destroys the gre_redir tunnel
 */
int nss_nlgre_redir_cmn_destroy_tun(struct net_device *dev);

/*
 * nss_nlgre_redir_cmn_unmap_interface()
 * 	Unmaps the nss interface
 */
int nss_nlgre_redir_cmn_unmap_interface(struct nss_nlgre_redir_unmap *unmap_params);

/*
 * nss_nlgre_redir_cmn_map_interface()
 * 	Unmaps the nss interface
 */
int nss_nlgre_redir_cmn_map_interface(uint32_t nexthop_nssif, uint16_t lag_en, struct nss_nlgre_redir_map *map_params);

/*
 * nss_nlgre_redir_cmn_set_next_hop()
 * 	Sets the next hop of the nss ath0 interface
 */
int nss_nlgre_redir_cmn_set_next_hop(uint32_t next_dev_ifnum, struct nss_nlgre_redir_set_next *set_next_params);

/*
 * nss_nlgre_redir_cmn_init()
 *	Initializes tun_data and lock variable.
 */
void nss_nlgre_redir_cmn_init(void);

