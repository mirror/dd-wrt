/* Copyright (c) 2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __DT_BINDINGS_QCOM_TCSR_H
#define __DT_BINDINGS_QCOM_TCSR_H

#define TCSR_USB_SELECT_USB3_P0		0x1
#define TCSR_USB_SELECT_USB3_P1		0x2
#define TCSR_USB_SELECT_USB3_DUAL	0x3

/* IPQ40xx HS PHY Mode Select */
#define TCSR_USB_HSPHY_HOST_MODE	0x00E700E7
#define TCSR_USB_HSPHY_DEVICE_MODE	0x00C700E7

/* IPQ40xx ess interface mode select */
#define TCSR_ESS_PSGMII              0
#define TCSR_ESS_PSGMII_RGMII5       1
#define TCSR_ESS_PSGMII_RMII0        2
#define TCSR_ESS_PSGMII_RMII1        4
#define TCSR_ESS_PSGMII_RMII0_RMII1  6
#define TCSR_ESS_PSGMII_RGMII4       9

/*
 * IPQ40xx WiFi Global Config
 * Bit 30:AXID_EN
 * Enable AXI master bus Axid translating to confirm all txn submitted by order
 * Bit 24: Use locally generated socslv_wxi_bvalid
 * 1:  use locally generate socslv_wxi_bvalid for performance.
 * 0:  use SNOC socslv_wxi_bvalid.
 */
#define TCSR_WIFI_GLB_CFG		0x41000000

/* IPQ40xx MEM_TYPE_SEL_M0_M2 Select Bit 26:24 - 2 NORMAL */
#define TCSR_WIFI_NOC_MEMTYPE_M0_M2	0x02222222

/* TCSR A/B REG */
#define IPQ806X_TCSR_REG_A_ADM_CRCI_MUX_SEL     0
#define IPQ806X_TCSR_REG_B_ADM_CRCI_MUX_SEL     1

#endif
