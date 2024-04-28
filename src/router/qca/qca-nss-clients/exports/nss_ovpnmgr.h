/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

/**
 * nss_ovpnmgr.h
 *	NSS OpenVPN (OVPN) Manager interface definitions.
 */

#ifndef _NSS_OVPNMGR_H_
#define _NSS_OVPNMGR_H_

#define NSS_OVPNMGR_CIPHER_KEYLEN_MAX 32
#define NSS_OVPNMGR_AUTH_KEYLEN_MAX 32
#define NSS_OVPNMGR_NONCE_SIZE_MAX 4
/*
 * TODO: Need to implement communication between NSS FW and OVPN manager
 * to get offload capabilities. Maximum number of tunnels supported is one of
 * the capabilities
 */
#define NSS_OVPNMGR_TUNNEL_MAX 32
#define NSS_OVPNMGR_PEER_ID_MAX (1 << 24)
/*
 * Maximum headroom size
 *	EDMA_PRE_HDR(32) + Ethernet (14) + VLAN (8) + PPPoE (8) + IPv6 (40) + UDP (8) + OVPN_HDR(4) +
 *	HMAC_HASH(32) + PACKET_ID (4)
 */
#define NSS_OVPNMGR_TUN_HEADROOM 192		/* Headroom needed for encapsulation. */
#define NSS_OVPNMGR_TUN_TAILROOM 256		/* Tailroom needed for decapsulation. */

/*
 * OpenVPN protocol definitions
 */
#define NSS_OVPNMGR_TUN_DATA_V1       6		/* Data version 1 packet. */
#define NSS_OVPNMGR_TUN_DATA_V2       9		/* Data version 2 packet. */
#define NSS_OVPNMGR_TUN_KEY_ID_MASK   0x07	/* Key ID mask, lower 3 bits. */
#define NSS_OVPNMGR_TUN_OPCODE_SHIFT  3		/* Opcode shift, upper 5 bits. */
#define NSS_OVPNMGR_TUN_PEER_ID_SHIFT 24	/* Peer ID shift, lower 24 bits. */
#define NSS_OVPNMGR_TUN_PEER_ID_MASK  0xFFFFFF	/* Peer ID mask. */


/*
 * OVPN flags which define encapsulation header parameters.
 */
#define NSS_OVPNMGR_HDR_FLAG_IPV6 0x0001		/* Outer IP header is IPv6. */
#define NSS_OVPNMGR_HDR_FLAG_SHARED_KEY 0x0002		/* Tunnel is established with shared key. */
#define NSS_OVPNMGR_HDR_FLAG_NO_IV 0x0004		/* Do not transmit IV in data packet. */
#define NSS_OVPNMGR_HDR_FLAG_NO_AUTH 0x0008		/* No HMAC Calculation. */
#define NSS_OVPNMGR_HDR_FLAG_NO_CIPHER 0x0010		/* No Crypto operation. */
#define NSS_OVPNMGR_HDR_FLAG_FRAG 0x0020		/* OpenVPN Fragmentation is enabled. */
#define NSS_OVPNMGR_HDR_FLAG_NO_REPLAY 0x0040		/* Disable replay attack check. */
#define NSS_OVPNMGR_HDR_FLAG_PID_LONG_FMT 0x0080	/* Use long packet id: <sequence number><current time> */
#define NSS_OVPNMGR_HDR_FLAG_DATA_V2 0x0100		/* Data packet type is V2, peer_id is valid */
#define NSS_OVPNMGR_HDR_FLAG_COPY_TOS 0x0200		/* Copy TOS from inner to outer. */
#define NSS_OVPNMGR_HDR_FLAG_L4_PROTO_TCP 0x0400	/* OpenVPN tunnel is TCP. */
#define NSS_OVPNMGR_HDR_FLAG_PEER_DATA_V2 0x0800	/* Peer is configured to transmit V2 data packets. */

/*
 * Packet offload flags
 */
#define NSS_OVPNMGR_METADATA_FLAG_PKT_DECAP 0x1		/* Decapsulate. */
#define NSS_OVPNMGR_METADATA_FLAG_PKT_TYPE_IPV6 0x2	/* IPv6 packet. */
#define NSS_OVPNMGR_METADATA_FLAG_PKT_TYPE_CTRL 0x4	/* Control packets in data channel. */

/*
 * Application modes.
 */
enum nss_ovpnmgr_app_mode {
	NSS_OVPNMGR_APP_MODE_CLIENT = 1,	/* Application mode is client. */
	NSS_OVPNMGR_APP_MODE_SERVER = 2		/* Application mode is server. */
};

/*
 * NSS OVPN manager supported crypto algorithms
 */
enum nss_ovpnmgr_algo {
	NSS_OVPNMGR_ALGO_AES_CBC_SHA1_HMAC,		/* AEAD  - AES_CBC_SHA1_HMAC */
	NSS_OVPNMGR_ALGO_AES_CBC_SHA256_HMAC,		/* AEAD  - AES_CBC_SHA256_HMAC */
	NSS_OVPNMGR_ALGO_3DES_CBC_SHA1_HMAC,		/* AEAD  - 3DES_CBC_SHA1_HMAC */
	NSS_OVPNMGR_ALGO_3DES_CBC_SHA256_HMAC,		/* AEAD  - 3DES_CBC_SHA256_HMAC */
	NSS_OVPNMGR_ALGO_NULL_CIPHER_SHA1_HMAC,		/* AHASH - SHA1_HMAC */
	NSS_OVPNMGR_ALGO_NULL_CIPHER_SHA256_HMAC,	/* AHASH - SHA256_HMAC */
	NSS_OVPNMGR_ALGO_AES_CBC_NULL_AUTH,		/* ABLK  - AES_CBC */
	NSS_OVPNMGR_ALGO_3DES_CBC_NULL_AUTH,		/* ABLK  - 3DES_CBC */
	NSS_OVPNMGR_ALGO_NULL_CIPHER_NULL_AUTH,		/* Crypto is not enabled. */
	NSS_OVPNMGR_ALGO_MAX
};

/*
 * nss_ovpnmgr_route_tuple
 *	Route tuple for configuration of host based route (cache).
 */
struct nss_ovpnmgr_route_tuple {
	uint32_t ip_addr[4];	/* IPv4/v6 address. */
	uint8_t ip_version;	/* IPv4/v6 version. */
};

/*
 * nss_ovpnmgr_crypto_key
 */
struct nss_ovpnmgr_crypto_key {
	uint8_t cipher_key[NSS_OVPNMGR_CIPHER_KEYLEN_MAX];	/* Cipher key */
	uint8_t hmac_key[NSS_OVPNMGR_AUTH_KEYLEN_MAX];		/* HMAC key */
	uint8_t nonce[NSS_OVPNMGR_NONCE_SIZE_MAX];		/* Cryptographic nonse. */
	uint16_t cipher_keylen;					/* Cipher key size. */
	uint16_t hmac_keylen;					/* HMAC Key size. */
};

/*
 * nss_ovpnmgr_crypto_config
 */
struct nss_ovpnmgr_crypto_config {
	struct nss_ovpnmgr_crypto_key encrypt;	/* Encryption/HMAC keys */
	struct nss_ovpnmgr_crypto_key decrypt;	/* Decryption/HMAC keys */
	enum nss_ovpnmgr_algo algo;		/* Crypto algorithm. */
};

/*
 * nss_ovpnmgr_tun_config
 *	OVPN header configuration parameters.
 */
struct nss_ovpnmgr_tun_config {
	uint32_t flags;		/* OpenVPN configuration flags. */
	uint32_t peer_id;	/* Peer id, maximum size is 24 bits. */
};

/*
 * nss_ovpnmgr_metadata
 *	Metadata for sending/receiving packet to/from applcation.
 */
struct nss_ovpnmgr_metadata {
	uint32_t flags;		/* Packet offload flags */
	uint32_t tunnel_id;	/* OVPN tunnel ID. */
};

/*
 * nss_ovpnmgr_tun_stats
 *	OVPN tunnel statistics maintained by application.
 */
struct nss_ovpnmgr_tun_stats {
	uint32_t tun_read_bytes;	/* Bytes (pkts * sizeof(each pkt)) from tun/tap device. */
	uint32_t tun_write_bytes;	/* Bytes (pkts * sizeof(each pkt)) to tun/tap device. */
	uint32_t link_read_bytes;	/* Bytes (pkts * sizeof(each pkt)) from UDP socket. */
	uint32_t link_read_bytes_auth;	/* Bytes (pkts * sizeof(each pkt)) from UDP socket and authenticated. */
	uint32_t link_write_bytes;	/* Bytes (pkts * sizeof(each pkt)) to UDP socket. */
};

/*
 * nss_ovpnmgr_tun_tuple
 *	Tunnel header parameters - IPv4/v6 and udp header parameters.
 */
struct nss_ovpnmgr_tun_tuple {
	uint32_t src_ip[4];	/* IPv4/v6 source ip address. */
	uint32_t dst_ip[4];	/* IPv4/v6 destination ip address. */
	uint16_t src_port;	/* UDP source port. */
	uint16_t dst_port;	/* UDP destination port. */
};

int nss_ovpnmgr_app_add(struct net_device *app_dev, enum nss_ovpnmgr_app_mode mode, void *app_data);
int nss_ovpnmgr_app_del(struct net_device *app_dev);
struct net_device *nss_ovpnmgr_app_find_tun(struct net_device *app_dev, struct nss_ovpnmgr_route_tuple *rt, uint32_t *ifnum);

uint32_t nss_ovpnmgr_tun_add(struct net_device *app_dev, struct nss_ovpnmgr_tun_tuple *tuple,
		struct nss_ovpnmgr_tun_config *tun_cfg, struct nss_ovpnmgr_crypto_config *crypto_cfg);
int nss_ovpnmgr_tun_del(uint32_t tunnel_id);
int nss_ovpnmgr_tun_tx(uint32_t tunnel_id, struct nss_ovpnmgr_metadata *mdata, struct sk_buff *skb);
void nss_ovpnmgr_tun_route_update(uint32_t tunnel_id, uint32_t *from_addr, uint32_t *to_addr, int version);
int nss_ovpnmgr_tun_stats_get(uint32_t tunnel_id, struct nss_ovpnmgr_tun_stats *stats);

int nss_ovpnmgr_crypto_key_add(uint32_t tunnel_id, uint8_t key_id, struct nss_ovpnmgr_crypto_config *crypto_cfg);
int nss_ovpnmgr_crypto_key_del(uint32_t tunnel_id);

int nss_ovpnmgr_route_add(uint32_t tunnel_id, struct nss_ovpnmgr_route_tuple *route);
int nss_ovpnmgr_route_del(uint32_t tunnel_id, struct nss_ovpnmgr_route_tuple *route);
bool nss_ovpnmgr_route_is_active(uint32_t tunnel_id, struct nss_ovpnmgr_route_tuple *route);
#endif /*_NSS_OVPNMGR_H_ */
