/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Device Tree constants for the Qualcomm QCA807X PHYs
 */

#ifndef _DT_BINDINGS_QCOM_QCA807X_H
#define _DT_BINDINGS_QCOM_QCA807X_H

#define PSGMII_QSGMII_TX_DRIVER_140MV	0
#define PSGMII_QSGMII_TX_DRIVER_160MV	1
#define PSGMII_QSGMII_TX_DRIVER_180MV	2
#define PSGMII_QSGMII_TX_DRIVER_200MV	3
#define PSGMII_QSGMII_TX_DRIVER_220MV	4
#define PSGMII_QSGMII_TX_DRIVER_240MV	5
#define PSGMII_QSGMII_TX_DRIVER_260MV	6
#define PSGMII_QSGMII_TX_DRIVER_280MV	7
#define PSGMII_QSGMII_TX_DRIVER_300MV	8
#define PSGMII_QSGMII_TX_DRIVER_320MV	9
#define PSGMII_QSGMII_TX_DRIVER_400MV	10
#define PSGMII_QSGMII_TX_DRIVER_500MV	11
/* Default value */
#define PSGMII_QSGMII_TX_DRIVER_600MV	12

/* Full amplitude, full bias current */
#define QCA807X_CONTROL_DAC_FULL_VOLT_BIAS		0
/* Amplitude follow DSP (amplitude is adjusted based on cable length), half bias current */
#define QCA807X_CONTROL_DAC_DSP_VOLT_HALF_BIAS		1
/* Full amplitude, bias current follow DSP (bias current is adjusted based on cable length) */
#define QCA807X_CONTROL_DAC_FULL_VOLT_DSP_BIAS		2
/* Both amplitude and bias current follow DSP */
#define QCA807X_CONTROL_DAC_DSP_VOLT_BIAS		3
/* Full amplitude, half bias current */
#define QCA807X_CONTROL_DAC_FULL_VOLT_HALF_BIAS		4
/* Amplitude follow DSP setting; 1/4 bias current when cable<10m,
 * otherwise half bias current
 */
#define QCA807X_CONTROL_DAC_DSP_VOLT_QUARTER_BIAS	5
/* Full amplitude; same bias current setting with “010” and “011”,
 * but half more bias is reduced when cable <10m
 */
#define QCA807X_CONTROL_DAC_FULL_VOLT_HALF_BIAS_SHORT	6
/* Amplitude follow DSP; same bias current setting with “110”, default value */
#define QCA807X_CONTROL_DAC_DSP_VOLT_HALF_BIAS_SHORT	7

#endif
