/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef __QCA808X_MACSEC_REG_H
#define __QCA808X_MACSEC_REG_H

#define BIT_15S                 (1U << 15)
#define BIT_14S                 (1 << 14)
#define BIT_13S                 (1 << 13)
#define BIT_12S                 (1 << 12)
#define BIT_11S                 (1 << 11)
#define BIT_10S                 (1 << 10)
#define BIT_9S                  (1 << 9)
#define BIT_8S                  (1 << 8)
#define BIT_7S                  (1 << 7)
#define BIT_6S                  (1 << 6)
#define BIT_5S                  (1 << 5)
#define BIT_4S                  (1 << 4)
#define BIT_3S                  (1 << 3)
#define BIT_2S                  (1 << 2)
#define BIT_1S                  (1 << 1)
#define BIT_0S                   1
#define BIT_NULL                 0x00

#define BIT_15                   15
#define BIT_14                   14
#define BIT_13                   13
#define BIT_12                   12
#define BIT_11                   11
#define BIT_10                   10
#define BIT_9                    9
#define BIT_8                    8
#define BIT_7                    7
#define BIT_6                    6
#define BIT_5                    5
#define BIT_4                    4
#define BIT_3                    3
#define BIT_2                    2
#define BIT_1                    1
#define BIT_0                    0


/**********************MMD3********************************/
/* MACsec register base address definition in PHY */
#define MACSEC_SYS_BASE             0xC000
#define MACSEC_EPR_BASE             0xC080
#define MACSEC_SA_CONTROL_BASE      0xC0A0
#define MACSEC_TX_AN_BASE           0xC0A8
#define MACSEC_RX_AN_BASE           0xC0AC
#define MACSEC_TX_NPN_BASE          0xC100
#define MACSEC_RX_NPN_BASE          0xC300
#define MACSEC_SC_BIND_BASE         0xC500
#define MACSEC_TX_MIB_BASE          0xC700
#define MACSEC_RX_MIB_BASE          0xCA00
#define MACSEC_TX_SAK_BASE          0xD000
#define MACSEC_RX_SAK_BASE          0xD200
#define MACSEC_DEBUG_BASE           0xF000
#define MACSEC_TX_EXTENDED_SAK_BASE 0xD400
#define MACSEC_RX_EXTENDED_SAK_BASE 0xD600
#define MACSEC_TX_SSCI_BASE         0xCD00
#define MACSEC_RX_SSCI_BASE         0xCD02
#define MACSEC_RX_KI_BASE           0xDA00
#define MACSEC_TX_KI_BASE           0xD800
#define MACSEC_TX_XPN_BASE          0xC180
#define MACSEC_RX_XPN_BASE          0xC380

/**********************MMD7********************************/
/* MACsec register base address definition in PHY */
#define MACSEC_SHADOW_REGISTER           0x807f

#define MACSEC_SHADOW_DUPLEX_EN                    BIT_8
#define MACSEC_SHADOW_DUPLEX_EN_OFFSET             8
#define MACSEC_SHADOW_DUPLEX_EN_LEN                1
#define MACSEC_SHADOW_LEGACY_DUPLEX_EN             BIT_3
#define MACSEC_SHADOW_LEGACY_DUPLEX_EN_OFFSET      3
#define MACSEC_SHADOW_LEGACY_DUPLEX_EN_LEN         1

/* system registers*/

#define MACSEC_SYS_EPR_CONFIG       (MACSEC_SYS_BASE + 0x01)
#define MACSEC_SYS_UIEPR_CONFIG     (MACSEC_SYS_BASE + 0x02)
#define MACSEC_SYS_MAC_ADDR_BASE    (MACSEC_SYS_BASE + 0x04)
#define MACSEC_SYS_FRAME_LEN        (MACSEC_SYS_BASE + 0x07)
#define MACSEC_SYS_REPLAY_WIN_BASE  (MACSEC_SYS_BASE + 0x10)
#define MACSEC_SYS_TX_ETHTYPE       (MACSEC_SYS_BASE + 0x12)
#define MACSEC_SYS_RX_ETHTYPE       (MACSEC_SYS_BASE + 0x13)
#define MACSEC_SYS_PAUSE_BASE       (MACSEC_SYS_BASE + 0x18)


/* system configure register */
#define MACSEC_SYS_CONFIG           (MACSEC_SYS_BASE + 0x00)

#define SYS_IN_PHY_EN                    BIT_15
#define SYS_IN_PHY_EN_OFFSET             15
#define SYS_IN_PHY_EN_LEN                1

#define SYS_INCLUDED_SCI_EN              BIT_13
#define SYS_INCLUDED_SCI_EN_OFFSET       13
#define SYS_INCLUDED_SCI_EN_LEN          1

#define SYS_CEPR_EN                      BIT_12
#define SYS_CEPR_EN_OFFSET               12
#define SYS_CEPR_EN_LEN                  1

#define SYS_UEPR_EN                      BIT_11
#define SYS_UEPR_EN_OFFSET               11
#define SYS_UEPR_EN_LEN                  1

#define SYS_UIER_EN                      BIT_10
#define SYS_UIER_EN_OFFSET               10
#define SYS_UIER_EN_LEN                  1

#define SYS_BIND_EN                      BIT_9
#define SYS_BIND_EN_OFFSET               9
#define SYS_BIND_EN_LEN                  1

#define SYS_REPLAY_PROTECT_EN            BIT_8
#define SYS_REPLAY_PROTECT_EN_OFFSET     8
#define SYS_REPLAY_PROTECT_EN_LEN        1

#define SYS_SRAM_DEBUG_EN                BIT_7
#define SYS_SRAM_DEBUG_EN_OFFSET         7
#define SYS_SRAM_DEBUG_EN_LEN            1

#define SYS_TX_UDEF_ETH_TYPE_EN          BIT_6
#define SYS_TX_UDEF_ETH_TYPE_EN_OFFSET   6
#define SYS_TX_UDEF_ETH_TYPE_EN_LEN      1

#define SYS_RX_UDEF_ETH_TYPE_EN          BIT_5
#define SYS_RX_UDEF_ETH_TYPE_EN_OFFSET   5
#define SYS_RX_UDEF_ETH_TYPE_EN_LEN      1

#define SYS_SA_SELECT_EN                 BIT_3
#define SYS_SA_SELECT_EN_OFFSET          3
#define SYS_SA_SELECT_EN_LEN             1

#define SYS_USED_MAC_ADDR_REG            BIT_2
#define SYS_USED_MAC_ADDR_REG_OFFSET     2
#define SYS_USED_MAC_ADDR_REG_LEN        1

#define SYS_USE_ES_EN                    BIT_1
#define SYS_USE_ES_EN_OFFSET             1
#define SYS_USE_ES_EN_LEN                1

#define SYS_USE_SCB_EN                   BIT_0
#define SYS_USE_SCB_EN_OFFSET            0
#define SYS_USE_SCB_EN_LEN               1


/* frame control register */
#define MACSEC_SYS_FRAME_CTRL       (MACSEC_SYS_BASE + 0x0B)

#define SYS_FRAME_BYPASS_LOOP          BIT_15
#define SYS_FRAME_BYPASS_LOOP_OFFSET   15
#define SYS_FRAME_BYPASS_LOOP_LEN      1

#define SYS_RX_LOOSE_VALIDATION_MODE          BIT_14
#define SYS_RX_LOOSE_VALIDATION_MODE_OFFSET   14
#define SYS_RX_LOOSE_VALIDATION_MODE_LEN      1

#define SYS_AES256_ENABLE          BIT_7
#define SYS_AES256_ENABLE_OFFSET   7
#define SYS_AES256_ENABLE_LEN      1

#define SYS_XPN_ENABLE          BIT_6
#define SYS_XPN_ENABLE_OFFSET   6
#define SYS_XPN_ENABLE_LEN      1

#define SYS_TX_LOOSE_VALIDATION_MODE          BIT_5
#define SYS_TX_LOOSE_VALIDATION_MODE_OFFSET   5
#define SYS_TX_LOOSE_VALIDATION_MODE_LEN      1

#define SYS_FRAME_PROTECT_EN           BIT_2
#define SYS_FRAME_PROTECT_EN_OFFSET    2
#define SYS_FRAME_PROTECT_EN_LEN       1

#define SYS_FRAME_VALIDATE             BIT_0
#define SYS_FRAME_VALIDATE_OFFSET      0
#define SYS_FRAME_VALIDATE_LEN         2
       #define SYS_FRAME_VALIDATE_STRICT      BIT_NULL
       #define SYS_FRAME_VALIDATE_CHECK       BIT_0S
       #define SYS_FRAME_VALIDATE_DIS         BIT_1S


/* packet control register */
#define MACSEC_SYS_PACKET_CTRL      (MACSEC_SYS_BASE + 0x1F)

#define SYS_SECY_LPBK               BIT_14
#define SYS_SECY_LPBK_OFFSET        14
#define SYS_SECY_LPBK_LEN           2
       #define SYS_MACSEC_EN               BIT_NULL
       #define SYS_PHY_LB                  BIT_0S
       #define SYS_SWITCH_LB               BIT_1S
       #define SYS_BYPASS                  0x3

#define SYS_FILTER_LPBK_EN          BIT_13
#define SYS_FILTER_LPBK_EN_OFFSET   13
#define SYS_FILTER_LPBK_EN_LEN      1

#define SYS_LLDP_SW                 BIT_4
#define SYS_LLDP_SW_OFFSET          4
#define SYS_LLDP_SW_LEN             2
       #define MSB_CRYPTO_LSB_DISCARD      0x0
       #define MSB_CRYPTO_LSB_UPLOAD       0x1
       #define MSB_PLAIN_LSB_DISCARD       0x2
       #define MSB_PLAIN_LSB_UPLOAD        0x3

#define SYS_CDP_SW                  BIT_2
#define SYS_CDP_SW_OFFSET           2
#define SYS_CDP_SW_LEN              2
       #define MSB_CRYPTO_LSB_DISCARD      0x0
       #define MSB_CRYPTO_LSB_UPLOAD       0x1
       #define MSB_PLAIN_LSB_DISCARD       0x2
       #define MSB_PLAIN_LSB_UPLOAD        0x3

#define SYS_STP_SW                  BIT_0
#define SYS_STP_SW_OFFSET           0
#define SYS_STP_SW_LEN              2
       #define MSB_CRYPTO_LSB_DISCARD      0x0
       #define MSB_CRYPTO_LSB_UPLOAD       0x1
       #define MSB_PLAIN_LSB_DISCARD       0x2
       #define MSB_PLAIN_LSB_UPLOAD        0x3


/* controlled port status */
#define MACSEC_SYS_PORT_CTRL        (MACSEC_SYS_BASE + 0x20)

#define SYS_PORT_EN                 BIT_15
#define SYS_PORT_EN_OFFSET          15
#define SYS_PORT_EN_LEN             1


#define SYS_MAC_EN_MASK             0x4000
#define SYS_MAC_OP_MASK             0x2000
#define SYS_PTP_MASK                0x1000


/* pause control register */
#define MACSEC_SYS_PAUSE_CTRL_BASE  (MACSEC_SYS_BASE + 0x28)

#define SYS_E_PAUSE_EN               BIT_3
#define SYS_E_PAUSE_EN_OFFSET        3
#define SYS_E_PAUSE_EN_LEN           1

#define SYS_E_PAUSE_FWD              BIT_2
#define SYS_E_PAUSE_FWD_OFFSET       2
#define SYS_E_PAUSE_FWD_LEN          1

#define SYS_I_PAUSE_EN               BIT_1
#define SYS_I_PAUSE_EN_OFFSET        1
#define SYS_I_PAUSE_EN_LEN           1

#define SYS_I_PAUSE_FWD              BIT_0
#define SYS_I_PAUSE_FWD_OFFSET       0
#define SYS_I_PAUSE_FWD_LEN          1

/* macsec sofware control register */
#define MACSEC_SOFTWARE_EN_CTRL      0xA200

#define SYS_SECY_SOFTWARE_EN               BIT_14
#define SYS_SECY_SOFTWARE_EN_OFFSET        14
#define SYS_SECY_SOFTWARE_EN_LEN           2
       #define SOFTWARE_MII_DBG              BIT_NULL
       #define SOFTWARE_BYPASS               0x1
       #define SOFTWARE_EN                   0x3

/* macsec forward az pattern register */
#define MACSEC_FORWARD_AZ_PATTERN_EN_CTRL      0xC062
#define LPI_RCOVER_EN               BIT_15
#define LPI_RCOVER_EN_OFFSET        15
#define LPI_RCOVER_EN_LEN           1
#define E_FORWARD_PATTERN_EN               BIT_13
#define E_FORWARD_PATTERN_EN_OFFSET        13
#define E_FORWARD_PATTERN_EN_LEN           1
#define I_FORWARD_PATTERN_EN               BIT_12
#define I_FORWARD_PATTERN_EN_OFFSET        12
#define I_FORWARD_PATTERN_EN_LEN           1

/*macsec local generate az pattern*/
#define MACSEC_LOCAL_AZ_PATTERN_EN_CTRL      0xC064
#define E_MAC_LPI_EN               BIT_15
#define E_MAC_LPI_EN_OFFSET        15
#define E_MAC_LPI_EN_LEN           1
#define I_MAC_LPI_EN               BIT_14
#define I_MAC_LPI_EN_OFFSET        14
#define I_MAC_LPI_EN_LEN           1

/* egress epr configure register */
#define MACSEC_E_EPR_CONFIG           (MACSEC_SYS_BASE + 0x01)

#define E_EPR_PRIORITY                   BIT_12
#define E_EPR_PRIORITY_OFFSET             12
#define E_EPR_PRIORITY_LEN                3

#define E_EPR_REVERSE                BIT_6
#define E_EPR_REVERSE_OFFSET          6
#define E_EPR_REVERSE_LEN             4

#define E_EPR_PATTERN0              BIT_4
#define E_EPR_PATTERN0_OFFSET       4
#define E_EPR_PATTERN0_LEN          2

#define E_EPR_PATTERN1              BIT_2
#define E_EPR_PATTERN1_OFFSET       2
#define E_EPR_PATTERN1_LEN          2

#define E_EPR_PATTERN2              BIT_0
#define E_EPR_PATTERN2_OFFSET       0
#define E_EPR_PATTERN2_LEN          2

/* ingress epr configure register */
#define MACSEC_I_EPR_CONFIG           (MACSEC_SYS_BASE + 0x02)

#define I_EPR_PRIORITY                   BIT_12
#define I_EPR_PRIORITY_OFFSET             12
#define I_EPR_PRIORITY_LEN                3

#define I_EPR_REVERSE                BIT_6
#define I_EPR_REVERSE_OFFSET          6
#define I_EPR_REVERSE_LEN             4

#define I_EPR_PATTERN0              BIT_4
#define I_EPR_PATTERN0_OFFSET       4
#define I_EPR_PATTERN0_LEN          2

#define I_EPR_PATTERN1              BIT_2
#define I_EPR_PATTERN1_OFFSET       2
#define I_EPR_PATTERN1_LEN          2

#define I_EPR_PATTERN2              BIT_0
#define I_EPR_PATTERN2_OFFSET       0
#define I_EPR_PATTERN2_LEN          2

#define SECY_EPR_PATTERN0       0x0
#define SECY_EPR_PATTERN1       0x1
#define SECY_EPR_PATTERN2       0x2

/* EPR block */

#define SECY_EPR_ANY_PACKET     0x0
#define SECY_EPR_IP_PACKET      0x2
#define SECY_EPR_TCP_PACKET     0x3

#define MACSEC_E_EPR_FIELD2(channel) \
	((MACSEC_EPR_BASE + channel * 0x4) + 0x00)
#define MACSEC_E_EPR_FIELD1(channel) \
	((MACSEC_EPR_BASE + channel * 0x4) + 0x01)
#define MACSEC_E_EPR_FIELD0(channel) \
	((MACSEC_EPR_BASE + channel * 0x4) + 0x02)

#define MACSEC_E_EPR_MSK_TYPE_OP(channel) \
	((MACSEC_EPR_BASE + channel * 0x4) + 0x03)

#define E_EPR_MASK              BIT_0
#define E_EPR_MASK_OFFSET        0
#define E_EPR_MASK_LEN           6

#define E_EPR_TYPE                 BIT_6
#define E_EPR_TYPE_OFFSET           6
#define E_EPR_TYPE_LEN              2

#define E_EPR_OFFSET              BIT_8
#define E_EPR_OFFSET_OFFSET           8
#define E_EPR_OFFSET_LEN              6

#define E_EPR_OPERATOR              BIT_15
#define E_EPR_OPERATOR_OFFSET           15
#define E_EPR_OPERATOR_LEN              1

#define MACSEC_I_EPR_FIELD2(channel) \
	((MACSEC_EPR_BASE + 0x10 + channel * 0x4) + 0x00)
#define MACSEC_I_EPR_FIELD1(channel) \
	((MACSEC_EPR_BASE + 0x10 + channel * 0x4) + 0x01)
#define MACSEC_I_EPR_FIELD0(channel) \
	((MACSEC_EPR_BASE + 0x10 + channel * 0x4) + 0x02)

#define MACSEC_I_EPR_MSK_TYPE_OP(channel) \
	((MACSEC_EPR_BASE + 0x10 + channel * 0x4) + 0x03)

#define I_EPR_MASK              BIT_0
#define I_EPR_MASK_OFFSET        0
#define I_EPR_MASK_LEN           6

#define I_EPR_TYPE                 BIT_6
#define I_EPR_TYPE_OFFSET           6
#define I_EPR_TYPE_LEN              2

#define I_EPR_OFFSET              BIT_8
#define I_EPR_OFFSET_OFFSET           8
#define I_EPR_OFFSET_LEN              6

#define I_EPR_OPERATOR              BIT_15
#define I_EPR_OPERATOR_OFFSET           15
#define I_EPR_OPERATOR_LEN              1

/* SA TX/RX control registers */
#define MACSEC_TX_SA_CONTROL        (MACSEC_SA_CONTROL_BASE + 0x00)
#define MACSEC_RX_SA_CONTROL        (MACSEC_SA_CONTROL_BASE + 0x02)

/* SA control register */
#define SA0_SAK0_ENABLE             BIT_0S
#define SA0_SAK1_ENABLE             BIT_1S
#define SA1_SAK0_ENABLE             BIT_2S
#define SA1_SAK1_ENABLE             BIT_3S
#define SA2_SAK0_ENABLE             BIT_4S
#define SA2_SAK1_ENABLE             BIT_5S
#define SA3_SAK0_ENABLE             BIT_6S
#define SA3_SAK1_ENABLE             BIT_7S
#define SA4_SAK0_ENABLE             BIT_8S
#define SA4_SAK1_ENABLE             BIT_9S
#define SA5_SAK0_ENABLE             BIT_10S
#define SA5_SAK1_ENABLE             BIT_11S
#define SA6_SAK0_ENABLE             BIT_12S
#define SA6_SAK1_ENABLE             BIT_13S
#define SA7_SAK0_ENABLE             BIT_14S
#define SA7_SAK1_ENABLE             BIT_15S

#define SA0_SAK0_DIS                ~BIT_0S
#define SA0_SAK1_DIS                ~BIT_1S
#define SA1_SAK0_DIS                ~BIT_2S
#define SA1_SAK1_DIS                ~BIT_3S
#define SA2_SAK0_DIS                ~BIT_4S
#define SA2_SAK1_DIS                ~BIT_5S
#define SA3_SAK0_DIS                ~BIT_6S
#define SA3_SAK1_DIS                ~BIT_7S
#define SA4_SAK0_DIS                ~BIT_8S
#define SA4_SAK1_DIS                ~BIT_9S
#define SA5_SAK0_DIS                ~BIT_10S
#define SA5_SAK1_DIS                ~BIT_11S
#define SA6_SAK0_DIS                ~BIT_12S
#define SA6_SAK1_DIS                ~BIT_13S
#define SA7_SAK0_DIS                ~BIT_14S
#define SA7_SAK1_DIS                ~BIT_15S


/* TX Next PN register */
#define MACSEC_TX_NPN(channel)          (MACSEC_TX_NPN_BASE + channel * 4)

/* RX Next PN register */
#define MACSEC_RX_NPN(channel)          (MACSEC_RX_NPN_BASE + channel * 4)

/* SA Bind block */
/* destination address bind for tx */
#define MACSEC_SC_BIND_TXDA_BASE(channel)  \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x00)
/* source address bind for tx */
#define MACSEC_SC_BIND_TXSA_BASE(channel)  \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x03)
/* ethernet type bind for tx */
#define MACSEC_SC_BIND_TXETHERTYPE(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x06)
/* vtag bind for tx */
#define MACSEC_SC_BIND_TXOUTVTAG(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x07)
#define MACSEC_SC_BIND_TXINVTAG(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x08)
/* sci bind for tx */
#define MACSEC_SC_BIND_TXSCI_BASE(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x09)
/* tci bind for tx */
#define MACSEC_SC_BIND_TXTCI(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x0d)

#define SC_BIND_TXOFFSET               BIT_8
#define SC_BIND_TXOFFSET_OFFSET        8
#define SC_BIND_TXOFFSET_LEN           6

#define SC_BIND_TXTCI                  BIT_0
#define SC_BIND_TXTCI_OFFSET           0
#define SC_BIND_TXTCI_LEN              8
/* bind context for  tx */
#define MACSEC_SC_BIND_TXCTX(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x0e)

#define SC_BIND_TX_IFBC                BIT_15
#define SC_BIND_TX_IFBC_OFFSET         15
#define SC_BIND_TX_IFBC_LEN            1

#define SC_BIND_TXCTX                  BIT_0
#define SC_BIND_TXCTX_OFFSET           0
#define SC_BIND_TXCTX_LEN              4


/* bind mask for tx */
#define MACSEC_SC_BIND_TXMASK(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x0f)

#define SC_BIND_TX_VALID                BIT_15
#define SC_BIND_TX_VALID_OFFSET         15
#define SC_BIND_TX_VALID_LEN            1

#define SC_BIND_TXMASK                  BIT_0
#define SC_BIND_TXMASK_OFFSET           0
#define SC_BIND_TXMASK_LEN              8


/* destination address bind for rx */
#define MACSEC_SC_BIND_RXDA_BASE(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x10)
/* source address bind for rx */
#define MACSEC_SC_BIND_RXSA_BASE(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x13)
/* ethernet type bind for rx */
#define MACSEC_SC_BIND_RXETHERTYPE(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x16)
/* vtag bind for rx */
#define MACSEC_SC_BIND_RXOUTVTAG(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x17)
#define MACSEC_SC_BIND_RXINVTAG(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x18)
/* sci bind for rx */
#define MACSEC_SC_BIND_RXSCI_BASE(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x19)
/* tci bind for rx */
#define MACSEC_SC_BIND_RXTCI(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x1D)

#define SC_BIND_RXOFFSET               BIT_8
#define SC_BIND_RXOFFSET_OFFSET        8
#define SC_BIND_RXOFFSET_LEN           6

#define SC_BIND_RXTCI                  BIT_0
#define SC_BIND_RXTCI_OFFSET           0
#define SC_BIND_RXTCI_LEN              8


/* bind context for rx */
#define MACSEC_SC_BIND_RXCTX(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x1E)

#define SC_BIND_RX_IFBC                BIT_15
#define SC_BIND_RX_IFBC_OFFSET         15
#define SC_BIND_RX_IFBC_LEN            1

#define SC_BIND_RXCTX                  BIT_0
#define SC_BIND_RXCTX_OFFSET           0
#define SC_BIND_RXCTX_LEN              4

/* bind mask for rx */
#define MACSEC_SC_BIND_RXMASK(channel) \
((MACSEC_SC_BIND_BASE + channel * 0x20) + 0x1F)

#define SC_BIND_RX_VALID                BIT_15
#define SC_BIND_RX_VALID_OFFSET         15
#define SC_BIND_RX_VALID_LEN            1

#define SC_BIND_RXMASK                  BIT_0
#define SC_BIND_RXMASK_OFFSET           0
#define SC_BIND_RXMASK_LEN              8

/* TX SAK block */
#define MACSEC_TX_SAK_KEY0(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x00)
#define MACSEC_TX_SAK_KEY1(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x01)
#define MACSEC_TX_SAK_KEY2(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x02)
#define MACSEC_TX_SAK_KEY3(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x03)
#define MACSEC_TX_SAK_KEY4(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x04)
#define MACSEC_TX_SAK_KEY5(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x05)
#define MACSEC_TX_SAK_KEY6(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x06)
#define MACSEC_TX_SAK_KEY7(channel) \
((MACSEC_TX_SAK_BASE + channel * 0x10) + 0x07)


/* RX SAK block */
#define MACSEC_RX_SAK_KEY0(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x00)
#define MACSEC_RX_SAK_KEY1(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x01)
#define MACSEC_RX_SAK_KEY2(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x02)
#define MACSEC_RX_SAK_KEY3(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x03)
#define MACSEC_RX_SAK_KEY4(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x04)
#define MACSEC_RX_SAK_KEY5(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x05)
#define MACSEC_RX_SAK_KEY6(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x06)
#define MACSEC_RX_SAK_KEY7(channel) \
((MACSEC_RX_SAK_BASE + channel * 0x10) + 0x07)

/* TX Extended SAK block */
#define MACSEC_TX_EXTENDED_SAK_KEY0(channel) \
((MACSEC_TX_EXTENDED_SAK_BASE + channel * 0x10) + 0x00)

/* RX Extended SAK block */
#define MACSEC_RX_EXTENDED_SAK_KEY0(channel) \
((MACSEC_RX_EXTENDED_SAK_BASE + channel * 0x10) + 0x00)

/* TX SSCI block */
#define MACSEC_SC_TX_SSCI0(channel) \
((MACSEC_TX_SSCI_BASE + channel * 0x4) + 0x00)

/* RX SSCI block */
#define MACSEC_SC_RX_SSCI0(channel) \
((MACSEC_RX_SSCI_BASE + channel * 0x4) + 0x00)

/* TX Key Identifier block */
#define MACSEC_TX_KI_KEY0(channel) \
((MACSEC_TX_KI_BASE + channel * 0x10) + 0x00)

/* RX Key Identifier block */
#define MACSEC_RX_KI_KEY0(channel) \
((MACSEC_RX_KI_BASE + channel * 0x10) + 0x00)

/* TX Next PN register */
#define MACSEC_TX_XPN(channel) \
(MACSEC_TX_XPN_BASE + channel * 4)

/* RX Next PN register */
#define MACSEC_RX_XPN(channel) \
(MACSEC_RX_XPN_BASE + channel * 4)


/* sak value */
#define SAK_AN_MASK                 0x03
#define SAK_TCI_C                   BIT_2S
#define SAK_TCI_E                   BIT_3S
#define SAK_TCI_SCB                 BIT_4S
#define SAK_TCI_SC                  BIT_5S
#define SAK_TCI_ES                  BIT_6S
#define SAK_TCI_VER                 BIT_7S

#define SAK_OFFSET_MASK            0x03
#define SAK_OFFSET0                0x00
#define SAK_OFFSET30               0x01
#define SAK_OFFSET50               0x02
#define SAK_OFFSET128              0x03


/* Tx MIB register */
#define TX_SA_PROTECTED_PKTS_BASE       MACSEC_TX_MIB_BASE
#define TX_SA_ENCRYPTED_PKTS_BASE       (MACSEC_TX_MIB_BASE + 64)
#define TX_SC_PROTECTED_OCTETS_BASE     (MACSEC_TX_MIB_BASE + 128)
#define TX_SC_ENCRYPTED_OCTETS_BASE     (MACSEC_TX_MIB_BASE + 160)
#define TX_UNTAGGED_PKTS                (MACSEC_TX_MIB_BASE + 192)
#define TX_TOO_LONG_PKTS                (MACSEC_TX_MIB_BASE + 194)
#define TX_SA_PROTECTED_PKTS_HIGH_BASE      (MACSEC_TX_MIB_BASE + 0x100)
#define TX_SA_ENCRYPTED_PKTS_HIGH_BASE      (MACSEC_TX_MIB_BASE + 0x140)

/* Rx MIB register */
#define RX_SA_UNUSED_PKTS_BASE          (MACSEC_RX_MIB_BASE + 0)
#define RX_SA_NOUSING_PKTS_BASE         (MACSEC_RX_MIB_BASE + 64)
#define RX_SA_NOTVALID_PKTS_BASE        (MACSEC_RX_MIB_BASE + 128)
#define RX_SA_INVALID_PKTS_BASE         (MACSEC_RX_MIB_BASE + 192)
#define RX_SA_OK_PKTS_BASE              (MACSEC_RX_MIB_BASE + 256)
#define RX_SC_LATE_PKTS_BASE            (MACSEC_RX_MIB_BASE + 320)
#define RX_SC_DELAYED_PKTS_BASE         (MACSEC_RX_MIB_BASE + 352)
#define RX_SC_UNCHECKED_PKTS_BASE       (MACSEC_RX_MIB_BASE + 384)
#define RX_SC_VALIDATED_PKTS_BASE       (MACSEC_RX_MIB_BASE + 416)
#define RX_SC_DECRYPTED_PKTS_BASE       (MACSEC_RX_MIB_BASE + 448)
#define RX_UNTAGGED_PKTS                (MACSEC_RX_MIB_BASE + 480)
#define RX_NO_TAG_PKTS                  (MACSEC_RX_MIB_BASE + 482)
#define RX_BAD_TAG_PKTS                 (MACSEC_RX_MIB_BASE + 484)
#define RX_UNKNOWN_SCI_PKTS             (MACSEC_RX_MIB_BASE + 486)
#define RX_NO_SCI_PKTS                  (MACSEC_RX_MIB_BASE + 488)
#define RX_OVERRUN_PKTS                 (MACSEC_RX_MIB_BASE + 490)
#define RX_SA_UNUSED_PKTS_HIGH_BASE        (MACSEC_RX_MIB_BASE + 0x400)
#define RX_SA_NOUSING_PKTS_HIGH_BASE         (MACSEC_RX_MIB_BASE + 0x440)
#define RX_SA_NOTVALID_PKTS_HIGH_BASE        (MACSEC_RX_MIB_BASE + 0x480)
#define RX_SA_INVALID_PKTS_HIGH_BASE         (MACSEC_RX_MIB_BASE + 0x4c0)
#define RX_SA_OK_PKTS_HIGH_BASE              (MACSEC_RX_MIB_BASE + 0x500)


#endif  /* __QCA808X_MACSEC_REGS_H */

