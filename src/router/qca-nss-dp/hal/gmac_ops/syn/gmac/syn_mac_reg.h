/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __SYN_MAC_REG_H__
#define __SYN_MAC_REG_H__

/*
 * MAC register offset
 */
#define SYN_MAC_CONFIGURATION				0x0000
#define SYN_MAC_FRAME_FILTER				0x0004
#define SYN_MAC_FLOW_CONTROL				0x0018
#define SYN_VLAN_TAG					0x001C
#define SYN_VERSION					0x0020
#define SYN_DEBUG					0x0024
#define SYN_REMOTE_WAKE_UP_FRAME_FILTER			0x0028
#define SYN_PMT_CONTROL_STATUS				0x002C
#define SYN_LPI_CONTROL_STATUS				0x0030
#define SYN_LPI_TIMERS_CONTROL				0x0034
#define SYN_INTERRUPT_STATUS				0x0038
#define SYN_INTERRUPT_MASK				0x003C

/*
 * MAC address register offset
 */
#define SYN_MAC_ADDR0_HIGH				0x0040
#define SYN_MAC_ADDR0_LOW				0x0044
#define SYN_MAC_ADDR1_HIGH				0x0048
#define SYN_MAC_ADDR1_LOW				0x004C
#define SYN_MAC_ADDR2_HIGH				0x0050
#define SYN_MAC_ADDR2_LOW				0x0054
#define SYN_MAC_ADDR3_HIGH				0x0058
#define SYN_MAC_ADDR3_LOW				0x005C
#define SYN_MAC_ADDR4_HIGH				0x0060
#define SYN_MAC_ADDR4_LOW				0x0064

/*
 * Watchdog timeout register
 */
#define SYN_WDOG_TIMEOUT				0x00DC

/*
 * Mac Management Counters (MMC) register offset
 */
#define SYN_MMC_CONTROL					0x0100
#define SYN_MMC_RX_INTERRUPT				0x0104
#define SYN_MMC_TX_INTERRUPT				0x0108
#define SYN_MMC_RX_INTERRUPT_MASK			0x010C
#define SYN_MMC_TX_INTERRUPT_MASK			0x0110
#define SYN_MMC_IPC_RX_INTR_MASK			0x0200

/*
 * Optional HW feature register
 */
#define SYN_HW_FEATURE					0x1058

/*
 * Register Bit Definitions
 */

/*
 * SYN_MAC_CONFIGURATION = 0x0000,	MAC config Register Layout
 */
enum syn_mac_config_reg {
	SYN_MAC_TWOKPE = 0x08000000,			/* Support for 2K packets */
	SYN_MAC_TWOKPE_ENABLE = 0x08000000,
	SYN_MAC_TWOKPE_DISABLE = 0x00000000,
	SYN_MAC_CST = 0x02000000,			/* (CST) CRC Stripping for Type Frames */
	SYN_MAC_CST_ENABLE = 0x02000000,
	SYN_MAC_CST_DISABLE = 0x02000000,
	SYN_MAC_TC = 0x01000000,			/* (TC) Transmit configuration */
	SYN_MAC_WATCHDOG = 0x00800000,
	SYN_MAC_WATCHDOG_ENABLE = 0x00000000,		/* Enable watchdog timer */
	SYN_MAC_WATCHDOG_DISABLE = 0x00800000,		/* (WD)Disable watchdog timer on Rx */
	SYN_MAC_JABBER = 0x00400000,
	SYN_MAC_JABBER_ENABLE = 0x00000000,		/* Enable jabber timer */
	SYN_MAC_JABBER_DISABLE = 0x00400000,		/* (JD)Disable jabber timer on Tx */
	SYN_MAC_FRAME_BURST = 0x00200000,
	SYN_MAC_FRAME_BURST_ENABLE = 0x00200000,	/* (BE)Enable frame bursting
							   during Tx */
	SYN_MAC_FRAME_BURST_DISABLE = 0x00000000,	/* Disable frame bursting */
	SYN_MAC_JUMBO_FRAME = 0x00100000,
	SYN_MAC_JUMBO_FRAME_ENABLE = 0x00100000,	/* (JE)Enable jumbo frame for Rx */
	SYN_MAC_JUMBO_FRAME_DISABLE = 0x00000000,	/* Disable jumbo frame */
	SYN_MAC_INTER_FRAME_GAP7 = 0x000E0000,		/* (IFG) Config7 - 40bit times */
	SYN_MAC_INTER_FRAME_GAP6 = 0x000C0000,		/* (IFG) Config6 - 48bit times */
	SYN_MAC_INTER_FRAME_GAP5 = 0x000A0000,		/* (IFG) Config5 - 56bit times */
	SYN_MAC_INTER_FRAME_GAP4 = 0x00080000,		/* (IFG) Config4 - 64bit times */
	SYN_MAC_INTER_FRAME_GAP3 = 0x00060000,		/* (IFG) Config3 - 72bit times */
	SYN_MAC_INTER_FRAME_GAP2 = 0x00040000,		/* (IFG) Config2 - 80bit times */
	SYN_MAC_INTER_FRAME_GAP1 = 0x00020000,		/* (IFG) Config1 - 88bit times */
	SYN_MAC_INTER_FRAME_GAP0 = 0x00000000,		/* (IFG) Config0 - 96bit times */
	SYN_MAC_DISABLE_CRS = 0x00010000,		/* (DCRS) Disable Carrier Sense During Transmission */
	SYN_MAC_MII_GMII = 0x00008000,
	SYN_MAC_SELECT_MII = 0x00008000,		/* (PS)Port Select-MII mode */
	SYN_MAC_SELECT_GMII = 0x00000000,		/* GMII mode */
	SYN_MAC_FE_SPEED100 = 0x00004000,		/* (FES)Fast Ethernet speed 100Mbps */
	SYN_MAC_FE_SPEED = 0x00004000,			/* (FES)Fast Ethernet speed 100Mbps */
	SYN_MAC_FE_SPEED10 = 0x00000000,		/* (FES)Fast Ethernet speed 10Mbps */
	SYN_MAC_RX_OWN = 0x00002000,
	SYN_MAC_DISABLE_RX_OWN = 0x00002000,		/* (DO)Disable receive own packets */
	SYN_MAC_ENABLE_RX_OWN = 0x00000000,		/* Enable receive own packets */
	SYN_MAC_LOOPBACK = 0x00001000,
	SYN_MAC_LOOPBACK_ON = 0x00001000,		/* (LM)Loopback mode for GMII/MII */
	SYN_MAC_LOOPBACK_OFF = 0x00000000,		/* Normal mode */
	SYN_MAC_DUPLEX = 0x00000800,
	SYN_MAC_FULL_DUPLEX = 0x00000800,		/* (DM)Full duplex mode */
	SYN_MAC_HALF_DUPLEX = 0x00000000,		/* Half duplex mode */
	SYN_MAC_RX_IPC_OFFLOAD = 0x00000400,		/* IPC checksum offload */
	SYN_MAC_RX_IPC_OFFLOAD_ENABLE = 0x00000400,
	SYN_MAC_RX_IPC_OFFLOAD_DISABLE = 0x00000000,
	SYN_MAC_RETRY = 0x00000200,
	SYN_MAC_RETRY_DISABLE = 0x00000200,		/* (DR)Disable Retry */
	SYN_MAC_RETRY_ENABLE = 0x00000000,		/* Enable retransmission as per BL */
	SYN_MAC_LINK_UP = 0x00000100,			/* (LUD)Link UP */
	SYN_MAC_LINK_DOWN = 0x00000100,			/* Link Down */
	SYN_MAC_PAD_CRC_STRIP = 0x00000080,
	SYN_MAC_PAD_CRC_STRIP_ENABLE = 0x00000080,	/* (ACS) Automatic Pad/Crc strip enable */
	SYN_MAC_PAD_CRC_STRIP_DISABLE = 0x00000000,	/* Automatic Pad/Crc stripping disable */
	SYN_MAC_BACKOFF_LIMIT = 0x00000060,
	SYN_MAC_BACKOFF_LIMIT3 = 0x00000060,		/* (BL)Back-off limit in HD mode */
	SYN_MAC_BACKOFF_LIMIT2 = 0x00000040,
	SYN_MAC_BACKOFF_LIMIT1 = 0x00000020,
	SYN_MAC_BACKOFF_LIMIT0 = 0x00000000,
	SYN_MAC_DEFERRAL_CHECK = 0x00000010,
	SYN_MAC_DEFERRAL_CHECK_ENABLE = 0x00000010,	/* (DC)Deferral check enable in HD mode */
	SYN_MAC_DEFERRAL_CHECK_DISABLE = 0x00000000,	/* Deferral check disable */
	SYN_MAC_TX = 0x00000008,
	SYN_MAC_TX_ENABLE = 0x00000008,			/* (TE)Transmitter enable */
	SYN_MAC_TX_DISABLE = 0x00000000,		/* Transmitter disable */
	SYN_MAC_RX = 0x00000004,
	SYN_MAC_RX_ENABLE = 0x00000004,			/* (RE)Receiver enable */
	SYN_MAC_RX_DISABLE = 0x00000000,		/* Receiver disable */
	SYN_MAC_PRELEN_RESERVED = 0x00000003,		/* Preamble Length for Transmit Frames */
	SYN_MAC_PRELEN_3B = 0x00000002,
	SYN_MAC_PRELEN_5B = 0x00000001,
	SYN_MAC_PRELEN_7B = 0x00000000,
};

/*
 * SYN_MAC_FRAME_FILTER = 0x0004,	Mac frame filtering controls Register
 */
enum syn_mac_frame_filter_reg {
	SYN_MAC_FILTER = 0x80000000,
	SYN_MAC_FILTER_OFF = 0x80000000,		/* (RA)Receive all incoming packets */
	SYN_MAC_FILTER_ON = 0x00000000,			/* Receive filtered pkts only */
	SYN_MAC_HASH_PERFECT_FILTER = 0x00000400,	/* Hash or Perfect Filter enable */
	SYN_MAC_SRC_ADDR_FILTER = 0x00000200,
	SYN_MAC_SRC_ADDR_FILTER_ENABLE = 0x00000200,	/* (SAF)Source Address Filter enable */
	SYN_MAC_SRC_ADDR_FILTER_DISABLE = 0x00000000,
	SYN_MAC_SRC_INVA_ADDR_FILTER = 0x00000100,
	SYN_MAC_SRC_INV_ADDR_FILTER_EN = 0x00000100,	/* (SAIF)Inv Src Addr Filter enable */
	SYN_MAC_SRC_INV_ADDR_FILTER_DIS = 0x00000000,
	SYN_MAC_PASS_CONTROL = 0x000000C0,
	SYN_MAC_PASS_CONTROL3 = 0x000000C0,		/* (PCF)Forwards ctrl frames that pass AF */
	SYN_MAC_PASS_CONTROL2 = 0x00000080,		/* Forwards all control frames
							   even if they fail the AF */
	SYN_MAC_PASS_CONTROL1 = 0x00000040,		/* Forwards all control frames except
							   PAUSE control frames to application
							   even if they fail the AF */
	SYN_MAC_PASS_CONTROL0 = 0x00000000,		/* Don't pass control frames */
	SYN_MAC_BROADCAST = 0x00000020,
	SYN_MAC_BROADCAST_DISABLE = 0x00000020,		/* (DBF)Disable Rx of broadcast frames */
	SYN_MAC_BROADCAST_ENABLE = 0x00000000,		/* Enable broadcast frames */
	SYN_MAC_MULTICAST_FILTER = 0x00000010,
	SYN_MAC_MULTICAST_FILTER_OFF = 0x00000010,	/* (PM) Pass all multicast packets */
	SYN_MAC_MULTICAST_FILTER_ON = 0x00000000,	/* Pass filtered multicast packets */
	SYN_MAC_DEST_ADDR_FILTER = 0x00000008,
	SYN_MAC_DEST_ADDR_FILTER_INV = 0x00000008,	/* (DAIF)Inverse filtering for DA */
	SYN_MAC_DEST_ADDR_FILTER_NOR = 0x00000000,	/* Normal filtering for DA */
	SYN_MAC_MCAST_HASH_FILTER = 0x00000004,
	SYN_MAC_MCAST_HASH_FILTER_ON = 0x00000004,	/* (HMC)perfom multicast hash filtering */
	SYN_MAC_MCAST_HASH_FILTER_OFF = 0x00000000,	/* perfect filtering only */
	SYN_MAC_UCAST_HASH_FILTER = 0x00000002,
	SYN_MAC_UCAST_HASH_FILTER_ON = 0x00000002,	/* (HUC)Unicast Hash filtering only */
	SYN_MAC_UCAST_HASH_FILTER_OFF = 0x00000000,	/* perfect filtering only */
	SYN_MAC_PROMISCUOUS_MODE = 0x00000001,
	SYN_MAC_PROMISCUOUS_MODE_ON = 0x00000001,	/* Receive all frames */
	SYN_MAC_PROMISCUOUS_MODE_OFF = 0x00000000,	/* Receive filtered packets only */
};

/*
 * SYN_MAC_FLOW_CONTROL = 0x0018,	Flow control Register Layout
 */
enum syn_mac_flow_control_reg {
	SYN_MAC_FC_PAUSE_TIME_MASK = 0xFFFF0000,	/* (PT) PAUSE TIME field
							   in the control frame */
	SYN_MAC_FC_PAUSE_TIME_SHIFT = 16,
	SYN_MAC_FC_PAUSE_LOW_THRESH = 0x00000030,
	SYN_MAC_FC_PAUSE_LOW_THRESH3 = 0x00000030,	/* (PLT)thresh for pause
							   tmr 256 slot time */
	SYN_MAC_FC_PAUSE_LOW_THRESH2 = 0x00000020,	/* 144 slot time */
	SYN_MAC_FC_PAUSE_LOW_THRESH1 = 0x00000010,	/* 28 slot time */
	SYN_MAC_FC_PAUSE_LOW_THRESH0 = 0x00000000,	/* 4 slot time */
	SYN_MAC_FC_UNICAST_PAUSE_FRAME = 0x00000008,
	SYN_MAC_FC_UNICAST_PAUSE_FRAME_ON = 0x00000008,	/* (UP)Detect pause frame
							   with unicast addr. */
	SYN_MAC_FC_UNICAST_PAUSE_FRAME_OFF = 0x00000000,/* Detect only pause frame
							   with multicast addr. */
	SYN_MAC_FC_RX_FLOW_CONTROL = 0x00000004,
	SYN_MAC_FC_RX_FLOW_CONTROL_ENABLE = 0x00000004,	/* (RFE)Enable Rx flow control */
	SYN_MAC_FC_RX_FLOW_CONTROL_DISABLE = 0x00000000,/* Disable Rx flow control */
	SYN_MAC_FC_TX_FLOW_CONTROL = 0x00000002,
	SYN_MAC_FC_TX_FLOW_CONTROL_ENABLE = 0x00000002,	/* (TFE)Enable Tx flow control */
	SYN_MAC_FC_TX_FLOW_CONTROL_DISABLE = 0x00000000,/* Disable flow control */
	SYN_MAC_FC_FLOW_CONTROL_BACK_PRESSURE = 0x00000001,
	SYN_MAC_FC_SEND_PAUSE_FRAME = 0x00000001,	/* (FCB/PBA)send pause frm/Apply
							   back pressure */
};

/*
 * SYN_MAC_ADDR_HIGH Register
 */
enum syn_mac_addr_high {
	SYN_MAC_ADDR_HIGH_AE = 0x80000000,
};

#endif /*__SYN_MAC_REG_H__*/
