/* SPDX-License-Identifier: GPL-2.0-only */
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2019-2023 OpenVPN, Inc.
 *
 *  Author:	James Yonan <james@openvpn.net>
 *		Antonio Quartulli <antonio@openvpn.net>
 */

#ifndef _NET_OVPN_DCO_OVPNRCU_H_
#define _NET_OVPN_DCO_OVPNRCU_H_

static inline void ovpn_rcu_lockdep_assert_held(void)
{
#ifdef CONFIG_PROVE_RCU
	RCU_LOCKDEP_WARN(!rcu_read_lock_held(),
			 "ovpn-dco RCU read lock not held");
#endif
}

#endif /* _NET_OVPN_DCO_OVPNRCU_H_ */
