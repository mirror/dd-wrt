/* SPDX-License-Identifier: GPL-2.0-only */
/* OpenVPN data channel accelerator
 *
 *  Copyright (C) 2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_COMPAT_NET_GSO_H
#define _NET_OVPN_COMPAT_NET_GSO_H

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 10)
#include_next <net/gso.h>
#else
#include <linux/netdevice.h>
#endif

#endif /* _NET_OVPN_COMPAT_NET_GSO_H */
