/*
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __EIP_IPSEC_H
#define __EIP_IPSEC_H

/*
 * eip_ipsec_tuple
 *	IPsec SA tuple
 */
struct eip_ipsec_tuple {
	uint32_t src_ip[4];     /* IPv6/IPv4 source IP. */
	uint32_t dest_ip[4];	/* IPv6/IPv4 destination IP */
	uint32_t esp_spi;	/* SPI value */
	uint16_t sport;		/* Source port for ESP-In-UDP */
	uint16_t dport;		/* Destination port for ESP-In-UDP */
	uint8_t protocol;	/* ESP or UDP (NAT-T) */
	uint8_t ip_version;	/* IP version 4/6 */
};

/*
 * eip_ipsec_base
 *	IPsec Base meta information for SA allocation.
 */
struct eip_ipsec_base {
	struct keys {
		uint8_t *data;			/* Pointer to the Key array */
		uint16_t len;			/* Length of the key */
	} cipher, auth;

	uint32_t nonce;				/* nonce value for certain cipher algorithm */
	uint32_t flags;				/* SA flags */
#define EIP_IPSEC_FLAG_ENC      BIT(1)	/* set = Encapsulation, clear = Decapsulation */
#define EIP_IPSEC_FLAG_IPV6     BIT(2)	/* set = IPv6 tuple, clear = IPv4 tuple SA */
#define EIP_IPSEC_FLAG_UDP      BIT(3)	/* set = ESP-in-UDP, clear = ESP encapsulation */
#define EIP_IPSEC_FLAG_ESN      BIT(4)	/* set = Extended sequence number, clear = 32-bit sequence number */
#define EIP_IPSEC_FLAG_TUNNEL   BIT(5)	/* set = SKIP SA, clear = RFC */
#define EIP_IPSEC_FLAG_SKIP     BIT(6)	/* set = SKIP SA, clear = RFC */
#define EIP_IPSEC_FLAG_CP_TOS   BIT(7)	/* set = copy IP TOS value from inner, clear = use default TOS */
#define EIP_IPSEC_FLAG_CP_DF   	BIT(8)	/* set = copy IP DF value from inner, clear = use default DF */
	const int8_t *algo_name;		/* AEAD algorithm name for cipher operation */
	uint8_t icv_len;		/* IPsec ICV length */
};

/*
 * eip_ipsec_data
 *	IPsec meta information for SA allocation.
 */
struct eip_ipsec_data {
	struct eip_ipsec_base base;		/* Base meta information */
	uint32_t replay_win;			/* SA replay window */
	uint32_t mtu;				/* WAN mtu */
	uint8_t df;				/* Default DF value for outer IP */
	uint8_t dscp;				/* Default dscp value for outer IP */
	uint8_t hop_limit;			/* Default TTL limit for outer IP */
	struct xfrm_state *xs;                  /* Only used for xfrm state offload */
};

/*
 * A tunnel compose of two types of SA(s); one for encapsulation & one for decapsulation.
 * Each tunnel is represented using dev under which SA has to be inserted.
 * DEV --> ENC_SA(active, passive) & DEC_SA(active, active)
 */

/*
 * eip_ipsec_dev_add
 *	Create new IPsec device to handle specific SA pair.
 *
 * @return
 * net_device
 */
struct net_device *eip_ipsec_dev_add(void);

/*
 * eip_ipsec_dev_del
 *	Destroy the IPsec device.
 *
 * @param[in] net_device
 */
void eip_ipsec_dev_del(struct net_device *ndev);

/*
 * eip_ipsec_sa_add
 *	Insert new SA under the IPsec device.
 *
 * @datatypes
 * eip_ipsec_data
 * eip_ipsec_tuple
 *
 * @param[in] ndev	IPsec device under which SA will be added.
 * @param[in] data	IPsec Security Association parameters.
 * @param[in] t		SA outer tuple (identifier).
 *
 * @return
 * int
 */
int eip_ipsec_sa_add(struct net_device *ndev, struct eip_ipsec_data *data, struct eip_ipsec_tuple *t);

/*
 * eip_ipsec_sa_del
 *	Delete the SA from device.
 *
 * @datatypes
 * eip_ipsec_tuple
 *
 * @param[in] t		SA outer tuple (identifier).
 */
void eip_ipsec_sa_del(struct net_device *ndev, struct eip_ipsec_tuple *t);

#endif /* __EIP_IPSEC_H */
