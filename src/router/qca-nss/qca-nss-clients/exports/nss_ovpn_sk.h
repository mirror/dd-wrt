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
 * nss_ovpn_sk.h
 *	NSS OpenVPN (OVPN) Manager interface definitions.
 */

#ifndef _NSS_OVPN_SK_H_
#define _NSS_OVPN_SK_H_

#define NSS_OVPN_SK_CIPHER_KEYLEN_MAX   32
#define NSS_OVPN_SK_AUTH_KEYLEN_MAX     32

/*
 * OVPN flags which define encapsulation header parameters.
 */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_IPv6 0x1		/* Outer IP header is IPv6. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_SHARED_KEY 0x2	/* Tunnel is established with shared key. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_NO_IV 0x4		/* Do not transmit IV in data packet. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_NO_AUTH 0x8		/* No HMAC Calculation. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_NO_CIPHER 0x10	/* No Crypto operation. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_FRAG 0x20		/* OpenVPN Fragmentation is enabled. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_DISABLE_REPLAY 0x40	/* Disable replay attack check. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_PID_LONG_FMT 0x80	/* Use long packet id: <sequence number><current time> */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_DATA_V2 0x100		/* Data packet type is V2, peer_id is valid */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_INHERIT_TOS 0x200	/* Copy TOS from inner to outer. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_L4_PROTO_TCP 0x400	/* OpenVPN tunnel is TCP. */
#define NSS_OVPN_SK_OVPN_HDR_FLAG_PEER_DATA_V2 0x800	/* Peer is configured to transmit V2 data packets. */

/*
 * Packet offload flags
 */
#define NSS_OVPN_SK_PKT_INFO_FLAG_DIR_DECAP 0x1		/* Encapsulate. */
#define NSS_OVPN_SK_PKT_INFO_FLAG_PKT_TYPE_IPV6 0x2	/* IPv6 packet. */
#define NSS_OVPN_SK_PKT_INFO_FLAG_PKT_TYPE_CTRL 0x4	/* Control packets in data channel. */

/*
 * OVPN socket ioctl options
 */
enum nss_ovpn_sk_sioc {
	NSS_OVPN_SK_SIOC_APP_REG = SIOCPROTOPRIVATE,	/* Command to register OVPN application. */
	NSS_OVPN_SK_SIOC_APP_DEREG,			/* Command to deregister OVPN application. */
	NSS_OVPN_SK_SIOC_TUN_ADD,			/* Command to add tunnel. */
	NSS_OVPN_SK_SIOC_TUN_DEL,			/* Command to delete tunnel. */
	NSS_OVPN_SK_SIOC_ROUTE_ADD,			/* Command to add route. */
	NSS_OVPN_SK_SIOC_ROUTE_DEL,			/* Command to delete route. */
	NSS_OVPN_SK_SIOC_ROUTE_STATE_GET,		/* Command to get route state. */
	NSS_OVPN_SK_SIOC_CRYPTO_KEY_ADD,		/* Command to add crypto key. */
	NSS_OVPN_SK_SIOC_CRYPTO_KEY_DEL,		/* Command to delete crypto key. */
	NSS_OVPN_SK_SIOC_STATS_GET			/* Command to get tunnel statistics. */
};

/*
 * Application modes.
 */
enum nss_ovpn_sk_app_mode {
	NSS_OVPN_SK_APP_MODE_CLIENT = 1,	/* Application mode is client. */
	NSS_OVPN_SK_APP_MODE_SERVER = 2		/* Application mode is server. */
};

/*
 * NSS OVPN manager supported crypto algorithms
 */
enum nss_ovpn_sk_algo {
	NSS_OVPN_SK_ALGO_AES_CBC_SHA1_HMAC,		/* AEAD  - AES_CBC_SHA1_HMAC */
	NSS_OVPN_SK_ALGO_AES_CBC_SHA256_HMAC,		/* AEAD  - AES_CBC_SHA256_HMAC */
	NSS_OVPN_SK_ALGO_3DES_CBC_SHA1_HMAC,		/* AEAD  - 3DES_CBC_SHA1_HMAC */
	NSS_OVPN_SK_ALGO_3DES_CBC_SHA256_HMAC,		/* AEAD  - 3DES_CBC_SHA256_HMAC */
	NSS_OVPN_SK_ALGO_NULL_CIPHER_SHA1_HMAC,		/* AHASH - SHA1_HMAC */
	NSS_OVPN_SK_ALGO_NULL_CIPHER_SHA256_HMAC,	/* AHASH - SHA256_HMAC */
	NSS_OVPN_SK_ALGO_AES_CBC_NULL_AUTH,		/* ABLK  - AES_CBC */
	NSS_OVPN_SK_ALGO_3DES_CBC_NULL_AUTH,		/* ABLK  - 3DES_CBC */
	NSS_OVPN_SK_ALGO_NULL_CIPHER_NULL_AUTH,		/* Crypto is not enabled. */
	NSS_OVPN_SK_ALGO_MAX
};

/*
 * nss_ovpn_sk_ovpn_params
 * 	OVPN header parameters.
 */
struct nss_ovpn_sk_ovpn_params {
	uint32_t flags;	/* OpenVPN configuration flags. */
	int32_t peer_id;			/* Peer id, maximum size is 24 bits. */
	uint32_t tunnel_id;			/* Tunnel ID, generated to identify tunnel info. */
};

/*
 * nss_ovpn_sk_tunnel_hdr
 * 	Tunnel header parameters - IPv4/v6 and udp header parameters.
 */
struct nss_ovpn_sk_tunnel_hdr {
	uint32_t src_ip[4];	/* IPv4/v6 source ip address. */
	uint32_t dst_ip[4];	/* IPv4/v6 destination ip address. */
	uint16_t src_port;	/* UDP source port. */
	uint16_t dst_port;	/* UDP destination port. */
	uint8_t hop_limit;	/* TTL or next hop limit. */
	uint8_t res[3];		/* Reserved for Alignment. */
};

/*
 * nss_ovpn_sk_crypto_key
 *	Crypto keys.
 */
struct nss_ovpn_sk_crypto_key {
	uint8_t cipher_key[NSS_OVPN_SK_CIPHER_KEYLEN_MAX];	/* Cipher key */
	uint8_t hmac_key[NSS_OVPN_SK_AUTH_KEYLEN_MAX];		/* HMAC key */
};

/*
 * nss_ovpn_sk_crypto_params
 *	Crypto configuration parameters.
 */
struct nss_ovpn_sk_crypto_params {
	enum nss_ovpn_sk_algo algo;	/* Crypto algorithm. */
	uint16_t cipher_key_size;	/* Cipher key size. */
	uint16_t hmac_key_size;		/* HMAC Key size. */
};

/*
 * nss_ovpn_sk_crypto_session
 *	Crypto session parameters parameters.
 */
struct nss_ovpn_sk_crypto_session {
	uint32_t tunnel_id;				/* OVPN tunnel id. */
	uint8_t key_id;					/* Crypto key id */
	struct nss_ovpn_sk_crypto_params config;	/* Crytpto configuration parameters. */
	struct nss_ovpn_sk_crypto_key encrypt;		/* Encryption/HMAC keys */
	struct nss_ovpn_sk_crypto_key decrypt;		/* Decryption/HMAC keys */
};

/*
 * nss_ovpn_sk_route_info
 *	Route configuration information.
 */
struct nss_ovpn_sk_route_info {
	uint32_t tunnel_id;	/* Tunnel id. */
	uint8_t ip_version;	/* IPv4/v6 version. */
	uint32_t ip_network[4];	/* IPv4/v6 network. */
	uint32_t netmask_bits;	/* Network mask bits. */
};

/*
 * nss_ovpn_sk_pkt_info
 *	Packet offload details.
 */
struct nss_ovpn_sk_pkt_info {
	uint32_t tunnel_id;			/* Tunnel id. */
	uint32_t flags;	/* Packet info flags. */
};

/*
 * nss_ovpn_sk_tunnel
 *	Tunnel configuration parameters.
 */
struct nss_ovpn_sk_tunnel {
	struct nss_ovpn_sk_ovpn_params ovpn;		/* OVPN tunnel parameters which define encap header parameters. */
	struct nss_ovpn_sk_tunnel_hdr tun_hdr;		/* Tunnel header parameters. */
	struct nss_ovpn_sk_crypto_session crypto;	/* Crypto parameters. */
};

/*
 * nss_ovpn_sk_app_inst
 * 	Register application with OVPN Manager.
 */
struct nss_ovpn_sk_app_inst {
	char tun_dev[IFNAMSIZ];			/* Tun/tap device name. */
	pid_t pid;				/* PID of the application. */
	int tun_fd;				/* Tun/tap device fd. */
	int udp_fd;				/* UDP socket fd. */
	enum nss_ovpn_sk_app_mode app_mode;	/* Application mode. */
};

/*
 * nss_ovpn_sk_tun_stats
 *	Statistics.
 */
struct nss_ovpn_sk_tun_stats {
	uint32_t tunnel_id;			/* Tunnel id. */
	unsigned int tun_read_bytes;		/* Total number of bytes read from tun/tap device. */
	unsigned int tun_write_bytes;		/* Total number of bytes written to tun/tap device. */
	unsigned int link_read_bytes;		/* Total number of bytes read from UDP socket. */
	unsigned int link_read_bytes_auth;	/* Total number of bytes authenticated from UDP socket. */
	unsigned int link_write_bytes;		/* Total number of bytes written to UDP socket. */
};

#endif /* _NSS_OVPN_SK_H_ */
