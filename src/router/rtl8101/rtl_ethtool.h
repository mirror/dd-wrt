/* SPDX-License-Identifier: GPL-2.0-only */
/*
################################################################################
#
# r8101 is the Linux device driver released for Realtek Fast Ethernet
# controllers with PCI-Express interface.
#
# Copyright(c) 2024 Realtek Semiconductor Corp. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <http://www.gnu.org/licenses/>.
#
# Author:
# Realtek NIC software team <nicfae@realtek.com>
# No. 2, Innovation Road II, Hsinchu Science Park, Hsinchu 300, Taiwan
#
################################################################################
*/

/************************************************************************************
 *  This product is covered by one or more of the following patents:
 *  US6,570,884, US6,115,776, and US6,327,625.
 ***********************************************************************************/

/*****************************************************************************/
/* 2.6.4 => 2.6.0 */
#if ( LINUX_VERSION_CODE < KERNEL_VERSION(2,4,25) || \
    ( LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(2,6,4) ) )
#define ETHTOOL_OPS_COMPAT
#endif /* 2.6.4 => 2.6.0 */

/*****************************************************************************/

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#undef ethtool_ops
#define ethtool_ops _kc_ethtool_ops

struct _kc_ethtool_ops {
        int  (*get_settings)(struct net_device *, struct ethtool_cmd *);
        int  (*set_settings)(struct net_device *, struct ethtool_cmd *);
        void (*get_drvinfo)(struct net_device *, struct ethtool_drvinfo *);
        int  (*get_regs_len)(struct net_device *);
        void (*get_regs)(struct net_device *, struct ethtool_regs *, void *);
        void (*get_wol)(struct net_device *, struct ethtool_wolinfo *);
        int  (*set_wol)(struct net_device *, struct ethtool_wolinfo *);
        u32  (*get_msglevel)(struct net_device *);
        void (*set_msglevel)(struct net_device *, u32);
        int  (*nway_reset)(struct net_device *);
        u32  (*get_link)(struct net_device *);
        int  (*get_eeprom_len)(struct net_device *);
        int  (*get_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
        int  (*set_eeprom)(struct net_device *, struct ethtool_eeprom *, u8 *);
        int  (*get_coalesce)(struct net_device *, struct ethtool_coalesce *);
        int  (*set_coalesce)(struct net_device *, struct ethtool_coalesce *);
        void (*get_ringparam)(struct net_device *, struct ethtool_ringparam *);
        int  (*set_ringparam)(struct net_device *, struct ethtool_ringparam *);
        void (*get_pauseparam)(struct net_device *,
                               struct ethtool_pauseparam*);
        int  (*set_pauseparam)(struct net_device *,
                               struct ethtool_pauseparam*);
        u32  (*get_rx_csum)(struct net_device *);
        int  (*set_rx_csum)(struct net_device *, u32);
        u32  (*get_tx_csum)(struct net_device *);
        int  (*set_tx_csum)(struct net_device *, u32);
        u32  (*get_sg)(struct net_device *);
        int  (*set_sg)(struct net_device *, u32);
        u32  (*get_tso)(struct net_device *);
        int  (*set_tso)(struct net_device *, u32);
        int  (*self_test_count)(struct net_device *);
        void (*self_test)(struct net_device *, struct ethtool_test *, u64 *);
        void (*get_strings)(struct net_device *, u32 stringset, u8 *);
        int  (*phys_id)(struct net_device *, u32);
        int  (*get_stats_count)(struct net_device *);
        void (*get_ethtool_stats)(struct net_device *, struct ethtool_stats *,
                                  u64 *);
} *ethtool_ops = NULL;

#undef SET_ETHTOOL_OPS
#define SET_ETHTOOL_OPS(netdev, ops) (ethtool_ops = (ops))

#endif //LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0)
#ifndef SET_ETHTOOL_OPS
#define SET_ETHTOOL_OPS(netdev,ops) \
         ( (netdev)->ethtool_ops = (ops) )
#endif //SET_ETHTOOL_OPS
#endif //LINUX_VERSION_CODE >= KERNEL_VERSION(3,16,0)

/*****************************************************************************/
/* Installations with ethtool version without eeprom, adapter id, or statistics
 * support */

#ifndef ETH_GSTRING_LEN
#define ETH_GSTRING_LEN 32
#endif

#ifndef ETHTOOL_GSTATS
#define ETHTOOL_GSTATS 0x1d
#undef ethtool_drvinfo
#define ethtool_drvinfo k_ethtool_drvinfo
struct k_ethtool_drvinfo {
        u32 cmd;
        char driver[32];
        char version[32];
        char fw_version[32];
        char bus_info[32];
        char reserved1[32];
        char reserved2[16];
        u32 n_stats;
        u32 testinfo_len;
        u32 eedump_len;
        u32 regdump_len;
};

struct ethtool_stats {
        u32 cmd;
        u32 n_stats;
        u64 data[0];
};
#endif /* ETHTOOL_GSTATS */

#ifndef ETHTOOL_PHYS_ID
#define ETHTOOL_PHYS_ID 0x1c
#endif /* ETHTOOL_PHYS_ID */

#ifndef ETHTOOL_GSTRINGS
#define ETHTOOL_GSTRINGS 0x1b
enum ethtool_stringset {
        ETH_SS_TEST             = 0,
        ETH_SS_STATS,
};
struct ethtool_gstrings {
        u32 cmd;            /* ETHTOOL_GSTRINGS */
        u32 string_set;     /* string set id e.c. ETH_SS_TEST, etc*/
        u32 len;            /* number of strings in the string set */
        u8 data[0];
};
#endif /* ETHTOOL_GSTRINGS */

#ifndef ETHTOOL_TEST
#define ETHTOOL_TEST 0x1a
enum ethtool_test_flags {
        ETH_TEST_FL_OFFLINE	= (1 << 0),
        ETH_TEST_FL_FAILED	= (1 << 1),
};
struct ethtool_test {
        u32 cmd;
        u32 flags;
        u32 reserved;
        u32 len;
        u64 data[0];
};
#endif /* ETHTOOL_TEST */

#ifndef ETHTOOL_GEEPROM
#define ETHTOOL_GEEPROM 0xb
#undef ETHTOOL_GREGS
struct ethtool_eeprom {
        u32 cmd;
        u32 magic;
        u32 offset;
        u32 len;
        u8 data[0];
};

struct ethtool_value {
        u32 cmd;
        u32 data;
};
#endif /* ETHTOOL_GEEPROM */

#ifndef ETHTOOL_GLINK
#define ETHTOOL_GLINK 0xa
#endif /* ETHTOOL_GLINK */

#ifndef ETHTOOL_GREGS
#define ETHTOOL_GREGS		0x00000004 /* Get NIC registers */
#define ethtool_regs _kc_ethtool_regs
/* for passing big chunks of data */
struct _kc_ethtool_regs {
        u32 cmd;
        u32 version; /* driver-specific, indicates different chips/revs */
        u32 len; /* bytes */
        u8 data[0];
};
#endif /* ETHTOOL_GREGS */

#ifndef ETHTOOL_GMSGLVL
#define ETHTOOL_GMSGLVL		0x00000007 /* Get driver message level */
#endif
#ifndef ETHTOOL_SMSGLVL
#define ETHTOOL_SMSGLVL		0x00000008 /* Set driver msg level, priv. */
#endif
#ifndef ETHTOOL_NWAY_RST
#define ETHTOOL_NWAY_RST	0x00000009 /* Restart autonegotiation, priv */
#endif
#ifndef ETHTOOL_GLINK
#define ETHTOOL_GLINK		0x0000000a /* Get link status */
#endif
#ifndef ETHTOOL_GEEPROM
#define ETHTOOL_GEEPROM		0x0000000b /* Get EEPROM data */
#endif
#ifndef ETHTOOL_SEEPROM
#define ETHTOOL_SEEPROM		0x0000000c /* Set EEPROM data */
#endif
#ifndef ETHTOOL_GCOALESCE
#define ETHTOOL_GCOALESCE	0x0000000e /* Get coalesce config */
/* for configuring coalescing parameters of chip */
#define ethtool_coalesce _kc_ethtool_coalesce
struct _kc_ethtool_coalesce {
        u32	cmd;	/* ETHTOOL_{G,S}COALESCE */

        /* How many usecs to delay an RX interrupt after
         * a packet arrives.  If 0, only rx_max_coalesced_frames
         * is used.
         */
        u32	rx_coalesce_usecs;

        /* How many packets to delay an RX interrupt after
         * a packet arrives.  If 0, only rx_coalesce_usecs is
         * used.  It is illegal to set both usecs and max frames
         * to zero as this would cause RX interrupts to never be
         * generated.
         */
        u32	rx_max_coalesced_frames;

        /* Same as above two parameters, except that these values
         * apply while an IRQ is being serviced by the host.  Not
         * all cards support this feature and the values are ignored
         * in that case.
         */
        u32	rx_coalesce_usecs_irq;
        u32	rx_max_coalesced_frames_irq;

        /* How many usecs to delay a TX interrupt after
         * a packet is sent.  If 0, only tx_max_coalesced_frames
         * is used.
         */
        u32	tx_coalesce_usecs;

        /* How many packets to delay a TX interrupt after
         * a packet is sent.  If 0, only tx_coalesce_usecs is
         * used.  It is illegal to set both usecs and max frames
         * to zero as this would cause TX interrupts to never be
         * generated.
         */
        u32	tx_max_coalesced_frames;

        /* Same as above two parameters, except that these values
         * apply while an IRQ is being serviced by the host.  Not
         * all cards support this feature and the values are ignored
         * in that case.
         */
        u32	tx_coalesce_usecs_irq;
        u32	tx_max_coalesced_frames_irq;

        /* How many usecs to delay in-memory statistics
         * block updates.  Some drivers do not have an in-memory
         * statistic block, and in such cases this value is ignored.
         * This value must not be zero.
         */
        u32	stats_block_coalesce_usecs;

        /* Adaptive RX/TX coalescing is an algorithm implemented by
         * some drivers to improve latency under low packet rates and
         * improve throughput under high packet rates.  Some drivers
         * only implement one of RX or TX adaptive coalescing.  Anything
         * not implemented by the driver causes these values to be
         * silently ignored.
         */
        u32	use_adaptive_rx_coalesce;
        u32	use_adaptive_tx_coalesce;

        /* When the packet rate (measured in packets per second)
         * is below pkt_rate_low, the {rx,tx}_*_low parameters are
         * used.
         */
        u32	pkt_rate_low;
        u32	rx_coalesce_usecs_low;
        u32	rx_max_coalesced_frames_low;
        u32	tx_coalesce_usecs_low;
        u32	tx_max_coalesced_frames_low;

        /* When the packet rate is below pkt_rate_high but above
         * pkt_rate_low (both measured in packets per second) the
         * normal {rx,tx}_* coalescing parameters are used.
         */

        /* When the packet rate is (measured in packets per second)
         * is above pkt_rate_high, the {rx,tx}_*_high parameters are
         * used.
         */
        u32	pkt_rate_high;
        u32	rx_coalesce_usecs_high;
        u32	rx_max_coalesced_frames_high;
        u32	tx_coalesce_usecs_high;
        u32	tx_max_coalesced_frames_high;

        /* How often to do adaptive coalescing packet rate sampling,
         * measured in seconds.  Must not be zero.
         */
        u32	rate_sample_interval;
};
#endif /* ETHTOOL_GCOALESCE */

#ifndef ETHTOOL_SCOALESCE
#define ETHTOOL_SCOALESCE	0x0000000f /* Set coalesce config. */
#endif
#ifndef ETHTOOL_GRINGPARAM
#define ETHTOOL_GRINGPARAM	0x00000010 /* Get ring parameters */
/* for configuring RX/TX ring parameters */
#define ethtool_ringparam _kc_ethtool_ringparam
struct _kc_ethtool_ringparam {
        u32	cmd;	/* ETHTOOL_{G,S}RINGPARAM */

        /* Read only attributes.  These indicate the maximum number
         * of pending RX/TX ring entries the driver will allow the
         * user to set.
         */
        u32	rx_max_pending;
        u32	rx_mini_max_pending;
        u32	rx_jumbo_max_pending;
        u32	tx_max_pending;

        /* Values changeable by the user.  The valid values are
         * in the range 1 to the "*_max_pending" counterpart above.
         */
        u32	rx_pending;
        u32	rx_mini_pending;
        u32	rx_jumbo_pending;
        u32	tx_pending;
};
#endif /* ETHTOOL_GRINGPARAM */

#ifndef ETHTOOL_SRINGPARAM
#define ETHTOOL_SRINGPARAM	0x00000011 /* Set ring parameters, priv. */
#endif
#ifndef ETHTOOL_GPAUSEPARAM
#define ETHTOOL_GPAUSEPARAM	0x00000012 /* Get pause parameters */
/* for configuring link flow control parameters */
#define ethtool_pauseparam _kc_ethtool_pauseparam
struct _kc_ethtool_pauseparam {
        u32	cmd;	/* ETHTOOL_{G,S}PAUSEPARAM */

        /* If the link is being auto-negotiated (via ethtool_cmd.autoneg
         * being true) the user may set 'autonet' here non-zero to have the
         * pause parameters be auto-negotiated too.  In such a case, the
         * {rx,tx}_pause values below determine what capabilities are
         * advertised.
         *
         * If 'autoneg' is zero or the link is not being auto-negotiated,
         * then {rx,tx}_pause force the driver to use/not-use pause
         * flow control.
         */
        u32	autoneg;
        u32	rx_pause;
        u32	tx_pause;
};
#endif /* ETHTOOL_GPAUSEPARAM */

#ifndef ETHTOOL_SPAUSEPARAM
#define ETHTOOL_SPAUSEPARAM	0x00000013 /* Set pause parameters. */
#endif
#ifndef ETHTOOL_GRXCSUM
#define ETHTOOL_GRXCSUM		0x00000014 /* Get RX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_SRXCSUM
#define ETHTOOL_SRXCSUM		0x00000015 /* Set RX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_GTXCSUM
#define ETHTOOL_GTXCSUM		0x00000016 /* Get TX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_STXCSUM
#define ETHTOOL_STXCSUM		0x00000017 /* Set TX hw csum enable (ethtool_value) */
#endif
#ifndef ETHTOOL_GSG
#define ETHTOOL_GSG		0x00000018 /* Get scatter-gather enable
* (ethtool_value) */
#endif
#ifndef ETHTOOL_SSG
#define ETHTOOL_SSG		0x00000019 /* Set scatter-gather enable
* (ethtool_value). */
#endif
#ifndef ETHTOOL_TEST
#define ETHTOOL_TEST		0x0000001a /* execute NIC self-test, priv. */
#endif
#ifndef ETHTOOL_GSTRINGS
#define ETHTOOL_GSTRINGS	0x0000001b /* get specified string set */
#endif
#ifndef ETHTOOL_PHYS_ID
#define ETHTOOL_PHYS_ID		0x0000001c /* identify the NIC */
#endif
#ifndef ETHTOOL_GSTATS
#define ETHTOOL_GSTATS		0x0000001d /* get NIC-specific statistics */
#endif
#ifndef ETHTOOL_GTSO
#define ETHTOOL_GTSO		0x0000001e /* Get TSO enable (ethtool_value) */
#endif
#ifndef ETHTOOL_STSO
#define ETHTOOL_STSO		0x0000001f /* Set TSO enable (ethtool_value) */
#endif

#ifndef ETHTOOL_BUSINFO_LEN
#define ETHTOOL_BUSINFO_LEN	32
#endif

/*****************************************************************************/
