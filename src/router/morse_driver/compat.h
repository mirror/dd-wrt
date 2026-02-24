/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * This header file is used to hide elements that must be used but would cause
 * the checkpatch utility to complain.
 */

#include "dot11ah/s1g_ieee80211.h"
#pragma once

/* Checkpatch does not like Camel Case */
#define morse_elf_ehdr Elf32_Ehdr
#define morse_elf_shdr Elf32_Shdr
#define morse_elf_phdr Elf32_Phdr

#define morse_compat_desc_num_endpoints(_desc) ((_desc).bNumEndpoints)

/* Checkpatch does not like used of the keyword 'fallthrough' */

/**
 * Define fallthrough explicitly for kernels that don't have compiler_attributes.h.
 */
#if KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE
#ifndef fallthrough
#if __has_attribute(__fallthrough__)
#define fallthrough	__attribute__((__fallthrough__))
#else
#define fallthrough	do {} while (0)	/* fallthrough */
#endif
#endif
#endif

#if !defined(struct_size)
/* Older kernels dont support this. Just add a simple def for it */
#define struct_size(p, member, count)					\
	(sizeof(*(p)) + (sizeof(*(p)->member) * count))
#endif

#if !defined(EM_RISCV)
#define EM_RISCV 243
#endif

#if KERNEL_VERSION(6, 7, 0) <= MAC80211_VERSION_CODE
    #define MORSE_IEEE80211_TX_STATUS(_hw, _skb) ieee80211_tx_status_skb(_hw, _skb)
#else
    #define MORSE_IEEE80211_TX_STATUS(_hw, _skb) ieee80211_tx_status(_hw, _skb)
#endif

#if KERNEL_VERSION(6, 9, 0) <= MAC80211_VERSION_CODE
    #define MORSE_IEEE80211_CSA_FINISH(_vif) ieee80211_csa_finish(_vif, 0)
#else
    #define MORSE_IEEE80211_CSA_FINISH(_vif) ieee80211_csa_finish(_vif)
#endif

#if KERNEL_VERSION(6, 9, 0) <= MAC80211_VERSION_CODE
    #define MORSE_BSS_CONF_CHAN_DEF(_vif)   (&_vif->bss_conf.chanreq.oper)
#else
    #define MORSE_BSS_CONF_CHAN_DEF(_vif)   (&_vif->bss_conf.chandef)
#endif

#if KERNEL_VERSION(6, 9, 0) <= MAC80211_VERSION_CODE
    #define MORSE_IEEE80211_CSA_COMPLETE(_vif)  ieee80211_beacon_cntdwn_is_complete(_vif, 0)
#elif KERNEL_VERSION(5, 10, 0) < MAC80211_VERSION_CODE
    #define MORSE_IEEE80211_CSA_COMPLETE(_vif)  ieee80211_beacon_cntdwn_is_complete(_vif)
#else
    #define MORSE_IEEE80211_CSA_COMPLETE(_vif)  ieee80211_csa_is_complete(_vif)
#endif

#if KERNEL_VERSION(4, 17, 0) > MAC80211_VERSION_CODE
/**
 * ieee80211_get_tid - get the QoS TID for a frame
 *
 * @hdr: Frame header
 *
 * @returns: TID
 */
static inline u8 ieee80211_get_tid(struct ieee80211_hdr *hdr)
{
	u8 *qc = ieee80211_get_qos_ctl(hdr);

	return qc[0] & IEEE80211_QOS_CTL_TID_MASK;
}
#endif
#define MORSE_IEEE80211_GET_TID(_hdr) ieee80211_get_tid(_hdr)
