/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-msm.h"

static const struct pinctrl_pin_desc ipq4019_pins[] = {
	PINCTRL_PIN(0, "GPIO_0"),
	PINCTRL_PIN(1, "GPIO_1"),
	PINCTRL_PIN(2, "GPIO_2"),
	PINCTRL_PIN(3, "GPIO_3"),
	PINCTRL_PIN(4, "GPIO_4"),
	PINCTRL_PIN(5, "GPIO_5"),
	PINCTRL_PIN(6, "GPIO_6"),
	PINCTRL_PIN(7, "GPIO_7"),
	PINCTRL_PIN(8, "GPIO_8"),
	PINCTRL_PIN(9, "GPIO_9"),
	PINCTRL_PIN(10, "GPIO_10"),
	PINCTRL_PIN(11, "GPIO_11"),
	PINCTRL_PIN(12, "GPIO_12"),
	PINCTRL_PIN(13, "GPIO_13"),
	PINCTRL_PIN(14, "GPIO_14"),
	PINCTRL_PIN(15, "GPIO_15"),
	PINCTRL_PIN(16, "GPIO_16"),
	PINCTRL_PIN(17, "GPIO_17"),
	PINCTRL_PIN(18, "GPIO_18"),
	PINCTRL_PIN(19, "GPIO_19"),
	PINCTRL_PIN(20, "GPIO_20"),
	PINCTRL_PIN(21, "GPIO_21"),
	PINCTRL_PIN(22, "GPIO_22"),
	PINCTRL_PIN(23, "GPIO_23"),
	PINCTRL_PIN(24, "GPIO_24"),
	PINCTRL_PIN(25, "GPIO_25"),
	PINCTRL_PIN(26, "GPIO_26"),
	PINCTRL_PIN(27, "GPIO_27"),
	PINCTRL_PIN(28, "GPIO_28"),
	PINCTRL_PIN(29, "GPIO_29"),
	PINCTRL_PIN(30, "GPIO_30"),
	PINCTRL_PIN(31, "GPIO_31"),
	PINCTRL_PIN(32, "GPIO_32"),
	PINCTRL_PIN(33, "GPIO_33"),
	PINCTRL_PIN(34, "GPIO_34"),
	PINCTRL_PIN(35, "GPIO_35"),
	PINCTRL_PIN(36, "GPIO_36"),
	PINCTRL_PIN(37, "GPIO_37"),
	PINCTRL_PIN(38, "GPIO_38"),
	PINCTRL_PIN(39, "GPIO_39"),
	PINCTRL_PIN(40, "GPIO_40"),
	PINCTRL_PIN(41, "GPIO_41"),
	PINCTRL_PIN(42, "GPIO_42"),
	PINCTRL_PIN(43, "GPIO_43"),
	PINCTRL_PIN(44, "GPIO_44"),
	PINCTRL_PIN(45, "GPIO_45"),
	PINCTRL_PIN(46, "GPIO_46"),
	PINCTRL_PIN(47, "GPIO_47"),
	PINCTRL_PIN(48, "GPIO_48"),
	PINCTRL_PIN(49, "GPIO_49"),
	PINCTRL_PIN(50, "GPIO_50"),
	PINCTRL_PIN(51, "GPIO_51"),
	PINCTRL_PIN(52, "GPIO_52"),
	PINCTRL_PIN(53, "GPIO_53"),
	PINCTRL_PIN(54, "GPIO_54"),
	PINCTRL_PIN(55, "GPIO_55"),
	PINCTRL_PIN(56, "GPIO_56"),
	PINCTRL_PIN(57, "GPIO_57"),
	PINCTRL_PIN(58, "GPIO_58"),
	PINCTRL_PIN(59, "GPIO_59"),
	PINCTRL_PIN(60, "GPIO_60"),
	PINCTRL_PIN(61, "GPIO_61"),
	PINCTRL_PIN(62, "GPIO_62"),
	PINCTRL_PIN(63, "GPIO_63"),
	PINCTRL_PIN(64, "GPIO_64"),
	PINCTRL_PIN(65, "GPIO_65"),
	PINCTRL_PIN(66, "GPIO_66"),
	PINCTRL_PIN(67, "GPIO_67"),
	PINCTRL_PIN(68, "GPIO_68"),
	PINCTRL_PIN(69, "GPIO_69"),
	PINCTRL_PIN(70, "GPIO_70"),
	PINCTRL_PIN(71, "GPIO_71"),
	PINCTRL_PIN(72, "GPIO_72"),
	PINCTRL_PIN(73, "GPIO_73"),
	PINCTRL_PIN(74, "GPIO_74"),
	PINCTRL_PIN(75, "GPIO_75"),
	PINCTRL_PIN(76, "GPIO_76"),
	PINCTRL_PIN(77, "GPIO_77"),
	PINCTRL_PIN(78, "GPIO_78"),
	PINCTRL_PIN(79, "GPIO_79"),
	PINCTRL_PIN(80, "GPIO_80"),
	PINCTRL_PIN(81, "GPIO_81"),
	PINCTRL_PIN(82, "GPIO_82"),
	PINCTRL_PIN(83, "GPIO_83"),
	PINCTRL_PIN(84, "GPIO_84"),
	PINCTRL_PIN(85, "GPIO_85"),
	PINCTRL_PIN(86, "GPIO_86"),
	PINCTRL_PIN(87, "GPIO_87"),
	PINCTRL_PIN(88, "GPIO_88"),
	PINCTRL_PIN(89, "GPIO_89"),
	PINCTRL_PIN(90, "GPIO_90"),
	PINCTRL_PIN(91, "GPIO_91"),
	PINCTRL_PIN(92, "GPIO_92"),
	PINCTRL_PIN(93, "GPIO_93"),
	PINCTRL_PIN(94, "GPIO_94"),
	PINCTRL_PIN(95, "GPIO_95"),
	PINCTRL_PIN(96, "GPIO_96"),
	PINCTRL_PIN(97, "GPIO_97"),
	PINCTRL_PIN(98, "GPIO_98"),
	PINCTRL_PIN(99, "GPIO_99"),
};

#define DECLARE_QCA_GPIO_PINS(pin) \
	static const unsigned int gpio##pin##_pins[] = { pin }
DECLARE_QCA_GPIO_PINS(0);
DECLARE_QCA_GPIO_PINS(1);
DECLARE_QCA_GPIO_PINS(2);
DECLARE_QCA_GPIO_PINS(3);
DECLARE_QCA_GPIO_PINS(4);
DECLARE_QCA_GPIO_PINS(5);
DECLARE_QCA_GPIO_PINS(6);
DECLARE_QCA_GPIO_PINS(7);
DECLARE_QCA_GPIO_PINS(8);
DECLARE_QCA_GPIO_PINS(9);
DECLARE_QCA_GPIO_PINS(10);
DECLARE_QCA_GPIO_PINS(11);
DECLARE_QCA_GPIO_PINS(12);
DECLARE_QCA_GPIO_PINS(13);
DECLARE_QCA_GPIO_PINS(14);
DECLARE_QCA_GPIO_PINS(15);
DECLARE_QCA_GPIO_PINS(16);
DECLARE_QCA_GPIO_PINS(17);
DECLARE_QCA_GPIO_PINS(18);
DECLARE_QCA_GPIO_PINS(19);
DECLARE_QCA_GPIO_PINS(20);
DECLARE_QCA_GPIO_PINS(21);
DECLARE_QCA_GPIO_PINS(22);
DECLARE_QCA_GPIO_PINS(23);
DECLARE_QCA_GPIO_PINS(24);
DECLARE_QCA_GPIO_PINS(25);
DECLARE_QCA_GPIO_PINS(26);
DECLARE_QCA_GPIO_PINS(27);
DECLARE_QCA_GPIO_PINS(28);
DECLARE_QCA_GPIO_PINS(29);
DECLARE_QCA_GPIO_PINS(30);
DECLARE_QCA_GPIO_PINS(31);
DECLARE_QCA_GPIO_PINS(32);
DECLARE_QCA_GPIO_PINS(33);
DECLARE_QCA_GPIO_PINS(34);
DECLARE_QCA_GPIO_PINS(35);
DECLARE_QCA_GPIO_PINS(36);
DECLARE_QCA_GPIO_PINS(37);
DECLARE_QCA_GPIO_PINS(38);
DECLARE_QCA_GPIO_PINS(39);
DECLARE_QCA_GPIO_PINS(40);
DECLARE_QCA_GPIO_PINS(41);
DECLARE_QCA_GPIO_PINS(42);
DECLARE_QCA_GPIO_PINS(43);
DECLARE_QCA_GPIO_PINS(44);
DECLARE_QCA_GPIO_PINS(45);
DECLARE_QCA_GPIO_PINS(46);
DECLARE_QCA_GPIO_PINS(47);
DECLARE_QCA_GPIO_PINS(48);
DECLARE_QCA_GPIO_PINS(49);
DECLARE_QCA_GPIO_PINS(50);
DECLARE_QCA_GPIO_PINS(51);
DECLARE_QCA_GPIO_PINS(52);
DECLARE_QCA_GPIO_PINS(53);
DECLARE_QCA_GPIO_PINS(54);
DECLARE_QCA_GPIO_PINS(55);
DECLARE_QCA_GPIO_PINS(56);
DECLARE_QCA_GPIO_PINS(57);
DECLARE_QCA_GPIO_PINS(58);
DECLARE_QCA_GPIO_PINS(59);
DECLARE_QCA_GPIO_PINS(60);
DECLARE_QCA_GPIO_PINS(61);
DECLARE_QCA_GPIO_PINS(62);
DECLARE_QCA_GPIO_PINS(63);
DECLARE_QCA_GPIO_PINS(64);
DECLARE_QCA_GPIO_PINS(65);
DECLARE_QCA_GPIO_PINS(66);
DECLARE_QCA_GPIO_PINS(67);
DECLARE_QCA_GPIO_PINS(68);
DECLARE_QCA_GPIO_PINS(69);
DECLARE_QCA_GPIO_PINS(70);
DECLARE_QCA_GPIO_PINS(71);
DECLARE_QCA_GPIO_PINS(72);
DECLARE_QCA_GPIO_PINS(73);
DECLARE_QCA_GPIO_PINS(74);
DECLARE_QCA_GPIO_PINS(75);
DECLARE_QCA_GPIO_PINS(76);
DECLARE_QCA_GPIO_PINS(77);
DECLARE_QCA_GPIO_PINS(78);
DECLARE_QCA_GPIO_PINS(79);
DECLARE_QCA_GPIO_PINS(80);
DECLARE_QCA_GPIO_PINS(81);
DECLARE_QCA_GPIO_PINS(82);
DECLARE_QCA_GPIO_PINS(83);
DECLARE_QCA_GPIO_PINS(84);
DECLARE_QCA_GPIO_PINS(85);
DECLARE_QCA_GPIO_PINS(86);
DECLARE_QCA_GPIO_PINS(87);
DECLARE_QCA_GPIO_PINS(88);
DECLARE_QCA_GPIO_PINS(89);
DECLARE_QCA_GPIO_PINS(90);
DECLARE_QCA_GPIO_PINS(91);
DECLARE_QCA_GPIO_PINS(92);
DECLARE_QCA_GPIO_PINS(93);
DECLARE_QCA_GPIO_PINS(94);
DECLARE_QCA_GPIO_PINS(95);
DECLARE_QCA_GPIO_PINS(96);
DECLARE_QCA_GPIO_PINS(97);
DECLARE_QCA_GPIO_PINS(98);
DECLARE_QCA_GPIO_PINS(99);

#define FUNCTION(fname)			                \
	[qca_mux_##fname] = {		                \
		.name = #fname,				\
		.groups = fname##_groups,               \
		.ngroups = ARRAY_SIZE(fname##_groups),	\
	}

#define PINGROUP(id, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12, f13, f14) \
	{					        \
		.name = "gpio" #id,			\
		.pins = gpio##id##_pins,		\
		.npins = (unsigned)ARRAY_SIZE(gpio##id##_pins),	\
		.funcs = (int[]){			\
			qca_mux_gpio, /* gpio mode */	\
			qca_mux_##f1,			\
			qca_mux_##f2,			\
			qca_mux_##f3,			\
			qca_mux_##f4,			\
			qca_mux_##f5,			\
			qca_mux_##f6,			\
			qca_mux_##f7,			\
			qca_mux_##f8,			\
			qca_mux_##f9,			\
			qca_mux_##f10,			\
			qca_mux_##f11,			\
			qca_mux_##f12,			\
			qca_mux_##f13,			\
			qca_mux_##f14			\
		},				        \
		.nfuncs = 15,				\
		.ctl_reg = 0x0 + 0x1000 * id,		\
		.io_reg = 0x4 + 0x1000 * id,		\
		.intr_cfg_reg = 0x8 + 0x1000 * id,	\
		.intr_status_reg = 0xc + 0x1000 * id,	\
		.intr_target_reg = 0x8 + 0x1000 * id,	\
		.mux_bit = 2,			\
		.pull_bit = 0,			\
		.drv_bit = 6,			\
		.oe_bit = 9,			\
		.in_bit = 0,			\
		.out_bit = 1,			\
		.intr_enable_bit = 0,		\
		.intr_status_bit = 0,		\
		.intr_target_bit = 5,		\
		.intr_raw_status_bit = 4,	\
		.intr_polarity_bit = 1,		\
		.intr_detection_bit = 2,	\
		.intr_detection_width = 2,	\
	}


enum ipq4019_functions {
	qca_mux_rmii0_refclk,
	qca_mux_wifi0_rfsilient0,
	qca_mux_wifi1_rfsilient0,
	qca_mux_smart2,
	qca_mux_led4,
	qca_mux_wifi0_cal,
	qca_mux_wifi1_cal,
	qca_mux_wifi_wci0,
	qca_mux_rmii0_dv,
	qca_mux_wifi_wci1,
	qca_mux_rmii1_refclk,
	qca_mux_blsp_spi1,
	qca_mux_led5,
	qca_mux_rmii10,
	qca_mux_led6,
	qca_mux_rmii11,
	qca_mux_led7,
	qca_mux_rmii1_dv,
	qca_mux_led8,
	qca_mux_rmii1_tx,
	qca_mux_aud_pin,
	qca_mux_led9,
	qca_mux_rmii1_rx,
	qca_mux_led10,
	qca_mux_wifi0_rfsilient1,
	qca_mux_wifi1_rfsilient1,
	qca_mux_led11,
	qca_mux_boot7,
	qca_mux_qpic_pad,
	qca_mux_pcie_clk,
	qca_mux_tm_clk0,
	qca_mux_wifi00,
	qca_mux_wifi10,
	qca_mux_mdio1,
	qca_mux_prng_rosc,
	qca_mux_dbg_out,
	qca_mux_tm0,
	qca_mux_wifi01,
	qca_mux_wifi11,
	qca_mux_atest_char3,
	qca_mux_pmu0,
	qca_mux_boot8,
	qca_mux_tm1,
	qca_mux_atest_char2,
	qca_mux_pmu1,
	qca_mux_boot9,
	qca_mux_tm2,
	qca_mux_atest_char1,
	qca_mux_tm_ack,
	qca_mux_wifi03,
	qca_mux_wifi13,
	qca_mux_qpic_pad4,
	qca_mux_atest_char0,
	qca_mux_tm3,
	qca_mux_wifi02,
	qca_mux_wifi12,
	qca_mux_qpic_pad5,
	qca_mux_smart3,
	qca_mux_wcss0_dbg14,
	qca_mux_tm4,
	qca_mux_wifi04,
	qca_mux_wifi14,
	qca_mux_qpic_pad6,
	qca_mux_wcss0_dbg15,
	qca_mux_qdss_tracectl_a,
	qca_mux_boot18,
	qca_mux_tm5,
	qca_mux_qpic_pad7,
	qca_mux_atest_char,
	qca_mux_wcss0_dbg4,
	qca_mux_qdss_traceclk_a,
	qca_mux_boot19,
	qca_mux_tm6,
	qca_mux_wcss0_dbg5,
	qca_mux_qdss_cti_trig_out_a0,
	qca_mux_boot14,
	qca_mux_tm7,
	qca_mux_chip_rst,
	qca_mux_wcss0_dbg6,
	qca_mux_qdss_cti_trig_out_b0,
	qca_mux_boot11,
	qca_mux_tm8,
	qca_mux_wcss0_dbg7,
	qca_mux_wcss1_dbg7,
	qca_mux_boot20,
	qca_mux_tm9,
	qca_mux_qpic_pad1,
	qca_mux_wcss0_dbg8,
	qca_mux_wcss1_dbg8,
	qca_mux_qpic_pad2,
	qca_mux_wcss0_dbg9,
	qca_mux_wcss1_dbg9,
	qca_mux_qpic_pad3,
	qca_mux_wcss0_dbg10,
	qca_mux_wcss1_dbg10,
	qca_mux_qpic_pad0,
	qca_mux_wcss0_dbg11,
	qca_mux_wcss1_dbg11,
	qca_mux_qpic_pad8,
	qca_mux_wcss0_dbg12,
	qca_mux_wcss1_dbg12,
	qca_mux_wifi034,
	qca_mux_wifi134,
	qca_mux_jtag_tdi,
	qca_mux_gpio,
	qca_mux_i2s_rx_bclk,
	qca_mux_jtag_tck,
	qca_mux_i2s_rx_fsync,
	qca_mux_jtag_tms,
	qca_mux_i2s_rxd,
	qca_mux_smart0,
	qca_mux_jtag_tdo,
	qca_mux_jtag_rst,
	qca_mux_jtag_trst,
	qca_mux_mdio0,
	qca_mux_wcss0_dbg18,
	qca_mux_wcss1_dbg18,
	qca_mux_qdss_tracedata_a,
	qca_mux_mdc,
	qca_mux_wcss0_dbg19,
	qca_mux_wcss1_dbg19,
	qca_mux_blsp_uart1,
	qca_mux_wifi0_uart,
	qca_mux_wifi1_uart,
	qca_mux_smart1,
	qca_mux_wcss0_dbg20,
	qca_mux_wcss1_dbg20,
	qca_mux_wifi0_uart0,
	qca_mux_wifi1_uart0,
	qca_mux_wcss0_dbg21,
	qca_mux_wcss1_dbg21,
	qca_mux_blsp_i2c0,
	qca_mux_wcss0_dbg22,
	qca_mux_wcss1_dbg22,
	qca_mux_wcss0_dbg23,
	qca_mux_wcss1_dbg23,
	qca_mux_blsp_spi0,
	qca_mux_blsp_i2c1,
	qca_mux_wcss0_dbg24,
	qca_mux_wcss1_dbg24,
	qca_mux_wcss0_dbg25,
	qca_mux_wcss1_dbg25,
	qca_mux_wcss0_dbg26,
	qca_mux_wcss1_dbg26,
	qca_mux_wcss0_dbg,
	qca_mux_wcss1_dbg,
	qca_mux_blsp_uart0,
	qca_mux_led0,
	qca_mux_wcss0_dbg28,
	qca_mux_wcss1_dbg28,
	qca_mux_led1,
	qca_mux_wcss0_dbg29,
	qca_mux_wcss1_dbg29,
	qca_mux_wifi0_uart1,
	qca_mux_wifi1_uart1,
	qca_mux_wcss0_dbg30,
	qca_mux_wcss1_dbg30,
	qca_mux_wcss0_dbg31,
	qca_mux_wcss1_dbg31,
	qca_mux_i2s_rx_mclk,
	qca_mux_wcss0_dbg16,
	qca_mux_wcss1_dbg16,
	qca_mux_wcss0_dbg17,
	qca_mux_wcss1_dbg17,
	qca_mux_rgmii0,
	qca_mux_sdio0,
	qca_mux_rgmii1,
	qca_mux_sdio1,
	qca_mux_rgmii2,
	qca_mux_i2s_tx_mclk,
	qca_mux_sdio2,
	qca_mux_rgmii3,
	qca_mux_i2s_tx_bclk,
	qca_mux_sdio3,
	qca_mux_rgmii_rx,
	qca_mux_i2s_tx_fsync,
	qca_mux_sdio_clk,
	qca_mux_rgmii_txc,
	qca_mux_i2s_td1,
	qca_mux_sdio_cmd,
	qca_mux_i2s_td2,
	qca_mux_sdio4,
	qca_mux_i2s_td3,
	qca_mux_sdio5,
	qca_mux_audio_pwm0,
	qca_mux_sdio6,
	qca_mux_audio_pwm1,
	qca_mux_wcss0_dbg27,
	qca_mux_wcss1_dbg27,
	qca_mux_sdio7,
	qca_mux_rgmii_rxc,
	qca_mux_audio_pwm2,
	qca_mux_rgmii_tx,
	qca_mux_audio_pwm3,
	qca_mux_boot2,
	qca_mux_i2s_spdif_in,
	qca_mux_i2s_spdif_out,
	qca_mux_rmii00,
	qca_mux_led2,
	qca_mux_rmii01,
	qca_mux_wifi0_wci,
	qca_mux_wifi1_wci,
	qca_mux_boot4,
	qca_mux_rmii0_tx,
	qca_mux_boot5,
	qca_mux_rmii0_rx,
	qca_mux_pcie_clk1,
	qca_mux_led3,
	qca_mux_sdio_cd,
	qca_mux_NA,
};

static const char * const rmii0_refclk_groups[] = {
	"gpio40",
};
static const char * const wifi0_rfsilient0_groups[] = {
	"gpio40",
};
static const char * const wifi1_rfsilient0_groups[] = {
	"gpio40",
};
static const char * const smart2_groups[] = {
	"gpio40", "gpio41", "gpio48", "gpio49",
};
static const char * const led4_groups[] = {
	"gpio40",
};
static const char * const wifi0_cal_groups[] = {
	"gpio41", "gpio51",
};
static const char * const wifi1_cal_groups[] = {
	"gpio41", "gpio51",
};
static const char * const wifi_wci0_groups[] = {
	"gpio42",
};
static const char * const rmii0_dv_groups[] = {
	"gpio43",
};
static const char * const wifi_wci1_groups[] = {
	"gpio43",
};
static const char * const rmii1_refclk_groups[] = {
	"gpio44",
};
static const char * const blsp_spi1_groups[] = {
	"gpio44", "gpio45", "gpio46", "gpio47",
};
static const char * const led5_groups[] = {
	"gpio44",
};
static const char * const rmii10_groups[] = {
	"gpio45", "gpio50",
};
static const char * const led6_groups[] = {
	"gpio45",
};
static const char * const rmii11_groups[] = {
	"gpio46", "gpio51",
};
static const char * const led7_groups[] = {
	"gpio46",
};
static const char * const rmii1_dv_groups[] = {
	"gpio47",
};
static const char * const led8_groups[] = {
	"gpio47",
};
static const char * const rmii1_tx_groups[] = {
	"gpio48",
};
static const char * const aud_pin_groups[] = {
	"gpio48", "gpio49", "gpio50", "gpio51",
};
static const char * const led9_groups[] = {
	"gpio48",
};
static const char * const rmii1_rx_groups[] = {
	"gpio49",
};
static const char * const led10_groups[] = {
	"gpio49",
};
static const char * const wifi0_rfsilient1_groups[] = {
	"gpio50",
};
static const char * const wifi1_rfsilient1_groups[] = {
	"gpio50",
};
static const char * const led11_groups[] = {
	"gpio50",
};
static const char * const boot7_groups[] = {
	"gpio51",
};
static const char * const qpic_pad_groups[] = {
	"gpio52", "gpio53", "gpio54", "gpio55", "gpio56", "gpio61", "gpio62",
	"gpio63", "gpio69",
};
static const char * const pcie_clk_groups[] = {
	"gpio52",
};
static const char * const tm_clk0_groups[] = {
	"gpio52",
};
static const char * const wifi00_groups[] = {
	"gpio52",
};
static const char * const wifi10_groups[] = {
	"gpio52",
};
static const char * const mdio1_groups[] = {
	"gpio53",
};
static const char * const prng_rosc_groups[] = {
	"gpio53",
};
static const char * const dbg_out_groups[] = {
	"gpio53",
};
static const char * const tm0_groups[] = {
	"gpio53",
};
static const char * const wifi01_groups[] = {
	"gpio53",
};
static const char * const wifi11_groups[] = {
	"gpio53",
};
static const char * const atest_char3_groups[] = {
	"gpio54",
};
static const char * const pmu0_groups[] = {
	"gpio54",
};
static const char * const boot8_groups[] = {
	"gpio54",
};
static const char * const tm1_groups[] = {
	"gpio54",
};
static const char * const atest_char2_groups[] = {
	"gpio55",
};
static const char * const pmu1_groups[] = {
	"gpio55",
};
static const char * const boot9_groups[] = {
	"gpio55",
};
static const char * const tm2_groups[] = {
	"gpio55",
};
static const char * const atest_char1_groups[] = {
	"gpio56",
};
static const char * const tm_ack_groups[] = {
	"gpio56",
};
static const char * const wifi03_groups[] = {
	"gpio56",
};
static const char * const wifi13_groups[] = {
	"gpio56",
};
static const char * const qpic_pad4_groups[] = {
	"gpio57",
};
static const char * const atest_char0_groups[] = {
	"gpio57",
};
static const char * const tm3_groups[] = {
	"gpio57",
};
static const char * const wifi02_groups[] = {
	"gpio57",
};
static const char * const wifi12_groups[] = {
	"gpio57",
};
static const char * const qpic_pad5_groups[] = {
	"gpio58",
};
static const char * const smart3_groups[] = {
	"gpio58", "gpio59", "gpio60", "gpio61",
};
static const char * const wcss0_dbg14_groups[] = {
	"gpio58",
};
static const char * const tm4_groups[] = {
	"gpio58",
};
static const char * const wifi04_groups[] = {
	"gpio58",
};
static const char * const wifi14_groups[] = {
	"gpio58",
};
static const char * const qpic_pad6_groups[] = {
	"gpio59",
};
static const char * const wcss0_dbg15_groups[] = {
	"gpio59",
};
static const char * const qdss_tracectl_a_groups[] = {
	"gpio59",
};
static const char * const boot18_groups[] = {
	"gpio59",
};
static const char * const tm5_groups[] = {
	"gpio59",
};
static const char * const qpic_pad7_groups[] = {
	"gpio60",
};
static const char * const atest_char_groups[] = {
	"gpio60",
};
static const char * const wcss0_dbg4_groups[] = {
	"gpio60",
};
static const char * const qdss_traceclk_a_groups[] = {
	"gpio60",
};
static const char * const boot19_groups[] = {
	"gpio60",
};
static const char * const tm6_groups[] = {
	"gpio60",
};
static const char * const wcss0_dbg5_groups[] = {
	"gpio61",
};
static const char * const qdss_cti_trig_out_a0_groups[] = {
	"gpio61",
};
static const char * const boot14_groups[] = {
	"gpio61",
};
static const char * const tm7_groups[] = {
	"gpio61",
};
static const char * const chip_rst_groups[] = {
	"gpio62",
};
static const char * const wcss0_dbg6_groups[] = {
	"gpio62",
};
static const char * const qdss_cti_trig_out_b0_groups[] = {
	"gpio62",
};
static const char * const boot11_groups[] = {
	"gpio62",
};
static const char * const tm8_groups[] = {
	"gpio62",
};
static const char * const wcss0_dbg7_groups[] = {
	"gpio63",
};
static const char * const wcss1_dbg7_groups[] = {
	"gpio63",
};
static const char * const boot20_groups[] = {
	"gpio63",
};
static const char * const tm9_groups[] = {
	"gpio63",
};
static const char * const qpic_pad1_groups[] = {
	"gpio64",
};
static const char * const wcss0_dbg8_groups[] = {
	"gpio64",
};
static const char * const wcss1_dbg8_groups[] = {
	"gpio64",
};
static const char * const qpic_pad2_groups[] = {
	"gpio65",
};
static const char * const wcss0_dbg9_groups[] = {
	"gpio65",
};
static const char * const wcss1_dbg9_groups[] = {
	"gpio65",
};
static const char * const qpic_pad3_groups[] = {
	"gpio66",
};
static const char * const wcss0_dbg10_groups[] = {
	"gpio66",
};
static const char * const wcss1_dbg10_groups[] = {
	"gpio66",
};
static const char * const qpic_pad0_groups[] = {
	"gpio67",
};
static const char * const wcss0_dbg11_groups[] = {
	"gpio67",
};
static const char * const wcss1_dbg11_groups[] = {
	"gpio67",
};
static const char * const qpic_pad8_groups[] = {
	"gpio68",
};
static const char * const wcss0_dbg12_groups[] = {
	"gpio68",
};
static const char * const wcss1_dbg12_groups[] = {
	"gpio68",
};
static const char * const wifi034_groups[] = {
	"gpio98",
};
static const char * const wifi134_groups[] = {
	"gpio98",
};
static const char * const jtag_tdi_groups[] = {
	"gpio0",
};
static const char * const gpio_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio3", "gpio4", "gpio5", "gpio6", "gpio7",
	"gpio8", "gpio9", "gpio10", "gpio11", "gpio12", "gpio13", "gpio14",
	"gpio15", "gpio16", "gpio17", "gpio18", "gpio19", "gpio20", "gpio21",
	"gpio22", "gpio23", "gpio24", "gpio25", "gpio26", "gpio27", "gpio28",
	"gpio29", "gpio30", "gpio31", "gpio32", "gpio33", "gpio34", "gpio35",
	"gpio36", "gpio37", "gpio38", "gpio39", "gpio40", "gpio41", "gpio42",
	"gpio43", "gpio44", "gpio45", "gpio46", "gpio47", "gpio48", "gpio49",
	"gpio50", "gpio51", "gpio52", "gpio53", "gpio54", "gpio55", "gpio56",
	"gpio57", "gpio58", "gpio59", "gpio60", "gpio61", "gpio62", "gpio63",
	"gpio64", "gpio65", "gpio66", "gpio67", "gpio68", "gpio69", "gpio70",
	"gpio71", "gpio72", "gpio73", "gpio74", "gpio75", "gpio76", "gpio77",
	"gpio78", "gpio79", "gpio80", "gpio81", "gpio82", "gpio83", "gpio84",
	"gpio85", "gpio86", "gpio87", "gpio88", "gpio89", "gpio90", "gpio91",
	"gpio92", "gpio93", "gpio94", "gpio95", "gpio96", "gpio97", "gpio98",
	"gpio99",
};
static const char * const i2s_rx_bclk_groups[] = {
	"gpio0", "gpio21", "gpio60",
};
static const char * const jtag_tck_groups[] = {
	"gpio1",
};
static const char * const i2s_rx_fsync_groups[] = {
	"gpio1", "gpio22", "gpio61",
};
static const char * const jtag_tms_groups[] = {
	"gpio2",
};
static const char * const i2s_rxd_groups[] = {
	"gpio2", "gpio23", "gpio63",
};
static const char * const smart0_groups[] = {
	"gpio0", "gpio1", "gpio2", "gpio5", "gpio44", "gpio45", "gpio46",
	"gpio47",
};
static const char * const jtag_tdo_groups[] = {
	"gpio3",
};
static const char * const jtag_rst_groups[] = {
	"gpio4",
};
static const char * const jtag_trst_groups[] = {
	"gpio5",
};
static const char * const mdio0_groups[] = {
	"gpio6",
};
static const char * const wcss0_dbg18_groups[] = {
	"gpio6", "gpio22", "gpio39",
};
static const char * const wcss1_dbg18_groups[] = {
	"gpio6", "gpio22", "gpio39",
};
static const char * const qdss_tracedata_a_groups[] = {
	"gpio6", "gpio7", "gpio8", "gpio9", "gpio10", "gpio11", "gpio16",
	"gpio17", "gpio37", "gpio38", "gpio39", "gpio40", "gpio41", "gpio42",
	"gpio43",
};
static const char * const mdc_groups[] = {
	"gpio7", "gpio52",
};
static const char * const wcss0_dbg19_groups[] = {
	"gpio7", "gpio23", "gpio40",
};
static const char * const wcss1_dbg19_groups[] = {
	"gpio7", "gpio23", "gpio40",
};
static const char * const blsp_uart1_groups[] = {
	"gpio8", "gpio9", "gpio10", "gpio11",
};
static const char * const wifi0_uart_groups[] = {
	"gpio8", "gpio9", "gpio11", "gpio19", "gpio62",
};
static const char * const wifi1_uart_groups[] = {
	"gpio8", "gpio11", "gpio19", "gpio62", "gpio63",
};
static const char * const smart1_groups[] = {
	"gpio8", "gpio9", "gpio16", "gpio17", "gpio58", "gpio59", "gpio60",
	"gpio61",
};
static const char * const wcss0_dbg20_groups[] = {
	"gpio8", "gpio24", "gpio41",
};
static const char * const wcss1_dbg20_groups[] = {
	"gpio8", "gpio24", "gpio41",
};
static const char * const wifi0_uart0_groups[] = {
	"gpio9", "gpio10",
};
static const char * const wifi1_uart0_groups[] = {
	"gpio9", "gpio10",
};
static const char * const wcss0_dbg21_groups[] = {
	"gpio9", "gpio25", "gpio42",
};
static const char * const wcss1_dbg21_groups[] = {
	"gpio9", "gpio25", "gpio42",
};
static const char * const blsp_i2c0_groups[] = {
	"gpio10", "gpio11", "gpio20", "gpio21", "gpio58", "gpio59",
};
static const char * const wcss0_dbg22_groups[] = {
	"gpio10", "gpio26", "gpio43",
};
static const char * const wcss1_dbg22_groups[] = {
	"gpio10", "gpio26", "gpio43",
};
static const char * const wcss0_dbg23_groups[] = {
	"gpio11", "gpio27", "gpio44",
};
static const char * const wcss1_dbg23_groups[] = {
	"gpio11", "gpio27", "gpio44",
};
static const char * const blsp_spi0_groups[] = {
	"gpio12", "gpio13", "gpio14", "gpio15", "gpio45",
	"gpio54", "gpio55", "gpio56", "gpio57",
};
static const char * const blsp_i2c1_groups[] = {
	"gpio12", "gpio13", "gpio34", "gpio35",
};
static const char * const wcss0_dbg24_groups[] = {
	"gpio12", "gpio28", "gpio45",
};
static const char * const wcss1_dbg24_groups[] = {
	"gpio12", "gpio28", "gpio45",
};
static const char * const wcss0_dbg25_groups[] = {
	"gpio13", "gpio29", "gpio46",
};
static const char * const wcss1_dbg25_groups[] = {
	"gpio13", "gpio29", "gpio46",
};
static const char * const wcss0_dbg26_groups[] = {
	"gpio14", "gpio30", "gpio47",
};
static const char * const wcss1_dbg26_groups[] = {
	"gpio14", "gpio30", "gpio47",
};
static const char * const wcss0_dbg_groups[] = {
	"gpio15", "gpio69",
};
static const char * const wcss1_dbg_groups[] = {
	"gpio15",
};
static const char * const blsp_uart0_groups[] = {
	"gpio16", "gpio17", "gpio60", "gpio61",
};
static const char * const led0_groups[] = {
	"gpio16", "gpio36", "gpio60",
};
static const char * const wcss0_dbg28_groups[] = {
	"gpio16", "gpio32", "gpio49",
};
static const char * const wcss1_dbg28_groups[] = {
	"gpio16", "gpio32", "gpio49",
};
static const char * const led1_groups[] = {
	"gpio17", "gpio37", "gpio61",
};
static const char * const wcss0_dbg29_groups[] = {
	"gpio17", "gpio33", "gpio50",
};
static const char * const wcss1_dbg29_groups[] = {
	"gpio17", "gpio33", "gpio50",
};
static const char * const wifi0_uart1_groups[] = {
	"gpio18", "gpio63",
};
static const char * const wifi1_uart1_groups[] = {
	"gpio18", "gpio63",
};
static const char * const wcss0_dbg30_groups[] = {
	"gpio18", "gpio34", "gpio51",
};
static const char * const wcss1_dbg30_groups[] = {
	"gpio18", "gpio34", "gpio51",
};
static const char * const wcss0_dbg31_groups[] = {
	"gpio19", "gpio35", "gpio52",
};
static const char * const wcss1_dbg31_groups[] = {
	"gpio19", "gpio35",
};
static const char * const i2s_rx_mclk_groups[] = {
	"gpio20", "gpio58",
};
static const char * const wcss0_dbg16_groups[] = {
	"gpio20", "gpio37",
};
static const char * const wcss1_dbg16_groups[] = {
	"gpio20", "gpio37",
};
static const char * const wcss0_dbg17_groups[] = {
	"gpio21", "gpio38",
};
static const char * const wcss1_dbg17_groups[] = {
	"gpio21", "gpio38",
};
static const char * const rgmii0_groups[] = {
	"gpio22", "gpio28",
};
static const char * const sdio0_groups[] = {
	"gpio23",
};
static const char * const rgmii1_groups[] = {
	"gpio23", "gpio29",
};
static const char * const sdio1_groups[] = {
	"gpio24",
};
static const char * const rgmii2_groups[] = {
	"gpio24", "gpio30",
};
static const char * const i2s_tx_mclk_groups[] = {
	"gpio24", "gpio52",
};
static const char * const sdio2_groups[] = {
	"gpio25",
};
static const char * const rgmii3_groups[] = {
	"gpio25", "gpio31",
};
static const char * const i2s_tx_bclk_groups[] = {
	"gpio25", "gpio53", "gpio60",
};
static const char * const sdio3_groups[] = {
	"gpio26",
};
static const char * const rgmii_rx_groups[] = {
	"gpio26",
};
static const char * const i2s_tx_fsync_groups[] = {
	"gpio26", "gpio57", "gpio61",
};
static const char * const sdio_clk_groups[] = {
	"gpio27",
};
static const char * const rgmii_txc_groups[] = {
	"gpio27",
};
static const char * const i2s_td1_groups[] = {
	"gpio27", "gpio54", "gpio63",
};
static const char * const sdio_cmd_groups[] = {
	"gpio28",
};
static const char * const i2s_td2_groups[] = {
	"gpio28", "gpio55",
};
static const char * const sdio4_groups[] = {
	"gpio29",
};
static const char * const i2s_td3_groups[] = {
	"gpio29", "gpio56",
};
static const char * const sdio5_groups[] = {
	"gpio30",
};
static const char * const audio_pwm0_groups[] = {
	"gpio30", "gpio64",
};
static const char * const sdio6_groups[] = {
	"gpio31",
};
static const char * const audio_pwm1_groups[] = {
	"gpio31", "gpio65",
};
static const char * const wcss0_dbg27_groups[] = {
	"gpio31", "gpio48",
};
static const char * const wcss1_dbg27_groups[] = {
	"gpio31", "gpio48",
};
static const char * const sdio7_groups[] = {
	"gpio32",
};
static const char * const rgmii_rxc_groups[] = {
	"gpio32",
};
static const char * const audio_pwm2_groups[] = {
	"gpio32", "gpio66",
};
static const char * const rgmii_tx_groups[] = {
	"gpio33",
};
static const char * const audio_pwm3_groups[] = {
	"gpio33", "gpio67",
};
static const char * const boot2_groups[] = {
	"gpio33",
};
static const char * const i2s_spdif_in_groups[] = {
	"gpio34", "gpio59", "gpio63",
};
static const char * const i2s_spdif_out_groups[] = {
	"gpio35", "gpio62", "gpio63",
};
static const char * const rmii00_groups[] = {
	"gpio36", "gpio41",
};
static const char * const led2_groups[] = {
	"gpio36", "gpio38", "gpio58",
};
static const char * const rmii01_groups[] = {
	"gpio37", "gpio42",
};
static const char * const wifi0_wci_groups[] = {
	"gpio37",
};
static const char * const wifi1_wci_groups[] = {
	"gpio37",
};
static const char * const boot4_groups[] = {
	"gpio37",
};
static const char * const rmii0_tx_groups[] = {
	"gpio38",
};
static const char * const boot5_groups[] = {
	"gpio38",
};
static const char * const rmii0_rx_groups[] = {
	"gpio39",
};
static const char * const pcie_clk1_groups[] = {
	"gpio39",
};
static const char * const led3_groups[] = {
	"gpio39",
};
static const char * const sdio_cd_groups[] = {
	"gpio22",
};

static const struct msm_function ipq4019_functions[] = {
	FUNCTION(rmii0_refclk),
	FUNCTION(wifi0_rfsilient0),
	FUNCTION(wifi1_rfsilient0),
	FUNCTION(smart2),
	FUNCTION(led4),
	FUNCTION(wifi0_cal),
	FUNCTION(wifi1_cal),
	FUNCTION(wifi_wci0),
	FUNCTION(rmii0_dv),
	FUNCTION(wifi_wci1),
	FUNCTION(rmii1_refclk),
	FUNCTION(blsp_spi1),
	FUNCTION(led5),
	FUNCTION(rmii10),
	FUNCTION(led6),
	FUNCTION(rmii11),
	FUNCTION(led7),
	FUNCTION(rmii1_dv),
	FUNCTION(led8),
	FUNCTION(rmii1_tx),
	FUNCTION(aud_pin),
	FUNCTION(led9),
	FUNCTION(rmii1_rx),
	FUNCTION(led10),
	FUNCTION(wifi0_rfsilient1),
	FUNCTION(wifi1_rfsilient1),
	FUNCTION(led11),
	FUNCTION(boot7),
	FUNCTION(qpic_pad),
	FUNCTION(pcie_clk),
	FUNCTION(tm_clk0),
	FUNCTION(wifi00),
	FUNCTION(wifi10),
	FUNCTION(mdio1),
	FUNCTION(prng_rosc),
	FUNCTION(dbg_out),
	FUNCTION(tm0),
	FUNCTION(wifi01),
	FUNCTION(wifi11),
	FUNCTION(atest_char3),
	FUNCTION(pmu0),
	FUNCTION(boot8),
	FUNCTION(tm1),
	FUNCTION(atest_char2),
	FUNCTION(pmu1),
	FUNCTION(boot9),
	FUNCTION(tm2),
	FUNCTION(atest_char1),
	FUNCTION(tm_ack),
	FUNCTION(wifi03),
	FUNCTION(wifi13),
	FUNCTION(qpic_pad4),
	FUNCTION(atest_char0),
	FUNCTION(tm3),
	FUNCTION(wifi02),
	FUNCTION(wifi12),
	FUNCTION(qpic_pad5),
	FUNCTION(smart3),
	FUNCTION(wcss0_dbg14),
	FUNCTION(tm4),
	FUNCTION(wifi04),
	FUNCTION(wifi14),
	FUNCTION(qpic_pad6),
	FUNCTION(wcss0_dbg15),
	FUNCTION(qdss_tracectl_a),
	FUNCTION(boot18),
	FUNCTION(tm5),
	FUNCTION(qpic_pad7),
	FUNCTION(atest_char),
	FUNCTION(wcss0_dbg4),
	FUNCTION(qdss_traceclk_a),
	FUNCTION(boot19),
	FUNCTION(tm6),
	FUNCTION(wcss0_dbg5),
	FUNCTION(qdss_cti_trig_out_a0),
	FUNCTION(boot14),
	FUNCTION(tm7),
	FUNCTION(chip_rst),
	FUNCTION(wcss0_dbg6),
	FUNCTION(qdss_cti_trig_out_b0),
	FUNCTION(boot11),
	FUNCTION(tm8),
	FUNCTION(wcss0_dbg7),
	FUNCTION(wcss1_dbg7),
	FUNCTION(boot20),
	FUNCTION(tm9),
	FUNCTION(qpic_pad1),
	FUNCTION(wcss0_dbg8),
	FUNCTION(wcss1_dbg8),
	FUNCTION(qpic_pad2),
	FUNCTION(wcss0_dbg9),
	FUNCTION(wcss1_dbg9),
	FUNCTION(qpic_pad3),
	FUNCTION(wcss0_dbg10),
	FUNCTION(wcss1_dbg10),
	FUNCTION(qpic_pad0),
	FUNCTION(wcss0_dbg11),
	FUNCTION(wcss1_dbg11),
	FUNCTION(qpic_pad8),
	FUNCTION(wcss0_dbg12),
	FUNCTION(wcss1_dbg12),
	FUNCTION(wifi034),
	FUNCTION(wifi134),
	FUNCTION(jtag_tdi),
	FUNCTION(gpio),
	FUNCTION(i2s_rx_bclk),
	FUNCTION(jtag_tck),
	FUNCTION(i2s_rx_fsync),
	FUNCTION(jtag_tms),
	FUNCTION(i2s_rxd),
	FUNCTION(smart0),
	FUNCTION(jtag_tdo),
	FUNCTION(jtag_rst),
	FUNCTION(jtag_trst),
	FUNCTION(mdio0),
	FUNCTION(wcss0_dbg18),
	FUNCTION(wcss1_dbg18),
	FUNCTION(qdss_tracedata_a),
	FUNCTION(mdc),
	FUNCTION(wcss0_dbg19),
	FUNCTION(wcss1_dbg19),
	FUNCTION(blsp_uart1),
	FUNCTION(wifi0_uart),
	FUNCTION(wifi1_uart),
	FUNCTION(smart1),
	FUNCTION(wcss0_dbg20),
	FUNCTION(wcss1_dbg20),
	FUNCTION(wifi0_uart0),
	FUNCTION(wifi1_uart0),
	FUNCTION(wcss0_dbg21),
	FUNCTION(wcss1_dbg21),
	FUNCTION(blsp_i2c0),
	FUNCTION(wcss0_dbg22),
	FUNCTION(wcss1_dbg22),
	FUNCTION(wcss0_dbg23),
	FUNCTION(wcss1_dbg23),
	FUNCTION(blsp_spi0),
	FUNCTION(blsp_i2c1),
	FUNCTION(wcss0_dbg24),
	FUNCTION(wcss1_dbg24),
	FUNCTION(wcss0_dbg25),
	FUNCTION(wcss1_dbg25),
	FUNCTION(wcss0_dbg26),
	FUNCTION(wcss1_dbg26),
	FUNCTION(wcss0_dbg),
	FUNCTION(wcss1_dbg),
	FUNCTION(blsp_uart0),
	FUNCTION(led0),
	FUNCTION(wcss0_dbg28),
	FUNCTION(wcss1_dbg28),
	FUNCTION(led1),
	FUNCTION(wcss0_dbg29),
	FUNCTION(wcss1_dbg29),
	FUNCTION(wifi0_uart1),
	FUNCTION(wifi1_uart1),
	FUNCTION(wcss0_dbg30),
	FUNCTION(wcss1_dbg30),
	FUNCTION(wcss0_dbg31),
	FUNCTION(wcss1_dbg31),
	FUNCTION(i2s_rx_mclk),
	FUNCTION(wcss0_dbg16),
	FUNCTION(wcss1_dbg16),
	FUNCTION(wcss0_dbg17),
	FUNCTION(wcss1_dbg17),
	FUNCTION(rgmii0),
	FUNCTION(sdio0),
	FUNCTION(rgmii1),
	FUNCTION(sdio1),
	FUNCTION(rgmii2),
	FUNCTION(i2s_tx_mclk),
	FUNCTION(sdio2),
	FUNCTION(rgmii3),
	FUNCTION(i2s_tx_bclk),
	FUNCTION(sdio3),
	FUNCTION(rgmii_rx),
	FUNCTION(i2s_tx_fsync),
	FUNCTION(sdio_clk),
	FUNCTION(rgmii_txc),
	FUNCTION(i2s_td1),
	FUNCTION(sdio_cmd),
	FUNCTION(i2s_td2),
	FUNCTION(sdio4),
	FUNCTION(i2s_td3),
	FUNCTION(sdio5),
	FUNCTION(audio_pwm0),
	FUNCTION(sdio6),
	FUNCTION(audio_pwm1),
	FUNCTION(wcss0_dbg27),
	FUNCTION(wcss1_dbg27),
	FUNCTION(sdio7),
	FUNCTION(rgmii_rxc),
	FUNCTION(audio_pwm2),
	FUNCTION(rgmii_tx),
	FUNCTION(audio_pwm3),
	FUNCTION(boot2),
	FUNCTION(i2s_spdif_in),
	FUNCTION(i2s_spdif_out),
	FUNCTION(rmii00),
	FUNCTION(led2),
	FUNCTION(rmii01),
	FUNCTION(wifi0_wci),
	FUNCTION(wifi1_wci),
	FUNCTION(boot4),
	FUNCTION(rmii0_tx),
	FUNCTION(boot5),
	FUNCTION(rmii0_rx),
	FUNCTION(pcie_clk1),
	FUNCTION(led3),
	FUNCTION(sdio_cd),
};

static const struct msm_pingroup ipq4019_groups[] = {
	PINGROUP(0, jtag_tdi, smart0, i2s_rx_bclk, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA, NA),
	PINGROUP(1, jtag_tck, smart0, i2s_rx_fsync, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA, NA),
	PINGROUP(2, jtag_tms, smart0, i2s_rxd, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(3, jtag_tdo, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(4, jtag_rst, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA),
	PINGROUP(5, jtag_trst, smart0, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(6, mdio0, NA, wcss0_dbg18, wcss1_dbg18, NA, qdss_tracedata_a,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(7, mdc, NA, wcss0_dbg19, wcss1_dbg19, NA, qdss_tracedata_a,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(8, blsp_uart1, wifi0_uart, wifi1_uart, smart1, NA,
		 wcss0_dbg20, wcss1_dbg20, NA, qdss_tracedata_a, NA, NA, NA,
		 NA, NA),
	PINGROUP(9, blsp_uart1, wifi0_uart0, wifi1_uart0, smart1, wifi0_uart,
		 NA, wcss0_dbg21, wcss1_dbg21, NA, qdss_tracedata_a, NA, NA,
		 NA, NA),
	PINGROUP(10, blsp_uart1, wifi0_uart0, wifi1_uart0, blsp_i2c0, NA,
		 wcss0_dbg22, wcss1_dbg22, NA, qdss_tracedata_a, NA, NA, NA,
		 NA, NA),
	PINGROUP(11, blsp_uart1, wifi0_uart, wifi1_uart, blsp_i2c0, NA,
		 wcss0_dbg23, wcss1_dbg23, NA, qdss_tracedata_a, NA, NA, NA,
		 NA, NA),
	PINGROUP(12, blsp_spi0, blsp_i2c1, NA, wcss0_dbg24, wcss1_dbg24, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(13, blsp_spi0, blsp_i2c1, NA, wcss0_dbg25, wcss1_dbg25, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(14, blsp_spi0, NA, wcss0_dbg26, wcss1_dbg26, NA, NA, NA, NA,
		 NA, NA, NA, NA, NA, NA),
	PINGROUP(15, blsp_spi0, NA, wcss0_dbg, wcss1_dbg, NA, NA, NA, NA, NA,
		 NA, NA, NA, NA, NA),
	PINGROUP(16, blsp_uart0, led0, smart1, NA, wcss0_dbg28, wcss1_dbg28,
		 NA, qdss_tracedata_a, NA, NA, NA, NA, NA, NA),
	PINGROUP(17, blsp_uart0, led1, smart1, NA, wcss0_dbg29, wcss1_dbg29,
		 NA, qdss_tracedata_a, NA, NA, NA, NA, NA, NA),
	PINGROUP(18, wifi0_uart1, wifi1_uart1, NA, wcss0_dbg30, wcss1_dbg30,
		 NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(19, wifi0_uart, wifi1_uart, NA, wcss0_dbg31, wcss1_dbg31, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(20, blsp_i2c0, i2s_rx_mclk, NA, wcss0_dbg16, wcss1_dbg16, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(21, blsp_i2c0, i2s_rx_bclk, NA, wcss0_dbg17, wcss1_dbg17, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(22, rgmii0, i2s_rx_fsync, NA, wcss0_dbg18, wcss1_dbg18, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(23, sdio0, rgmii1, i2s_rxd, NA, wcss0_dbg19, wcss1_dbg19, NA,
		 NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(24, sdio1, rgmii2, i2s_tx_mclk, NA, wcss0_dbg20, wcss1_dbg20,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(25, sdio2, rgmii3, i2s_tx_bclk, NA, wcss0_dbg21, wcss1_dbg21,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(26, sdio3, rgmii_rx, i2s_tx_fsync, NA, wcss0_dbg22,
		 wcss1_dbg22, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(27, sdio_clk, rgmii_txc, i2s_td1, NA, wcss0_dbg23,
		 wcss1_dbg23, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(28, sdio_cmd, rgmii0, i2s_td2, NA, wcss0_dbg24, wcss1_dbg24,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(29, sdio4, rgmii1, i2s_td3, NA, wcss0_dbg25, wcss1_dbg25, NA,
		 NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(30, sdio5, rgmii2, audio_pwm0, NA, wcss0_dbg26, wcss1_dbg26,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(31, sdio6, rgmii3, audio_pwm1, NA, wcss0_dbg27, wcss1_dbg27,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(32, sdio7, rgmii_rxc, audio_pwm2, NA, wcss0_dbg28,
		 wcss1_dbg28, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(33, rgmii_tx, audio_pwm3, NA, wcss0_dbg29, wcss1_dbg29, NA,
		 boot2, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(34, blsp_i2c1, i2s_spdif_in, NA, wcss0_dbg30, wcss1_dbg30, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(35, blsp_i2c1, i2s_spdif_out, NA, wcss0_dbg31, wcss1_dbg31,
		 NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(36, rmii00, led2, led0, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(37, rmii01, wifi0_wci, wifi1_wci, led1, NA, NA, wcss0_dbg16,
		 wcss1_dbg16, NA, qdss_tracedata_a, boot4, NA, NA, NA),
	PINGROUP(38, rmii0_tx, led2, NA, NA, wcss0_dbg17, wcss1_dbg17, NA,
		 qdss_tracedata_a, boot5, NA, NA, NA, NA, NA),
	PINGROUP(39, rmii0_rx, pcie_clk1, led3, NA, NA, wcss0_dbg18,
		 wcss1_dbg18, NA, NA, qdss_tracedata_a, NA, NA, NA, NA),
	PINGROUP(40, rmii0_refclk, wifi0_rfsilient0, wifi1_rfsilient0, smart2,
		 led4, NA, NA, wcss0_dbg19, wcss1_dbg19, NA, NA,
		 qdss_tracedata_a, NA, NA),
	PINGROUP(41, rmii00, wifi0_cal, wifi1_cal, smart2, NA, NA, wcss0_dbg20,
		 wcss1_dbg20, NA, NA, qdss_tracedata_a, NA, NA, NA),
	PINGROUP(42, rmii01, wifi_wci0, NA, NA, wcss0_dbg21, wcss1_dbg21, NA,
		 NA, qdss_tracedata_a, NA, NA, NA, NA, NA),
	PINGROUP(43, rmii0_dv, wifi_wci1, NA, NA, wcss0_dbg22, wcss1_dbg22, NA,
		 NA, qdss_tracedata_a, NA, NA, NA, NA, NA),
	PINGROUP(44, rmii1_refclk, blsp_spi1, smart0, led5, NA, NA,
		 wcss0_dbg23, wcss1_dbg23, NA, NA, NA, NA, NA, NA),
	PINGROUP(45, rmii10, blsp_spi1, blsp_spi0, smart0, led6, NA, NA,
		 wcss0_dbg24, wcss1_dbg24, NA, NA, NA, NA, NA),
	PINGROUP(46, rmii11, blsp_spi1, smart0, led7, NA, NA, wcss0_dbg25,
		 wcss1_dbg25, NA, NA, NA, NA, NA, NA),
	PINGROUP(47, rmii1_dv, blsp_spi1, smart0, led8, NA, NA, wcss0_dbg26,
		 wcss1_dbg26, NA, NA, NA, NA, NA, NA),
	PINGROUP(48, rmii1_tx, aud_pin, smart2, led9, NA, NA, wcss0_dbg27,
		 wcss1_dbg27, NA, NA, NA, NA, NA, NA),
	PINGROUP(49, rmii1_rx, aud_pin, smart2, led10, NA, NA, wcss0_dbg28,
		 wcss1_dbg28, NA, NA, NA, NA, NA, NA),
	PINGROUP(50, rmii10, aud_pin, wifi0_rfsilient1, wifi1_rfsilient1,
		 led11, NA, NA, wcss0_dbg29, wcss1_dbg29, NA, NA, NA, NA, NA),
	PINGROUP(51, rmii11, aud_pin, wifi0_cal, wifi1_cal, NA, NA,
		 wcss0_dbg30, wcss1_dbg30, NA, boot7, NA, NA, NA, NA),
	PINGROUP(52, qpic_pad, mdc, pcie_clk, i2s_tx_mclk, NA, NA, wcss0_dbg31,
		 tm_clk0, wifi00, wifi10, NA, NA, NA, NA),
	PINGROUP(53, qpic_pad, mdio1, i2s_tx_bclk, prng_rosc, dbg_out, tm0,
		 wifi01, wifi11, NA, NA, NA, NA, NA, NA),
	PINGROUP(54, qpic_pad, blsp_spi0, i2s_td1, atest_char3, pmu0, NA, NA,
		 boot8, tm1, NA, NA, NA, NA, NA),
	PINGROUP(55, qpic_pad, blsp_spi0, i2s_td2, atest_char2, pmu1, NA, NA,
		 boot9, tm2, NA, NA, NA, NA, NA),
	PINGROUP(56, qpic_pad, blsp_spi0, i2s_td3, atest_char1, NA, tm_ack,
		 wifi03, wifi13, NA, NA, NA, NA, NA, NA),
	PINGROUP(57, qpic_pad4, blsp_spi0, i2s_tx_fsync, atest_char0, NA, tm3,
		 wifi02, wifi12, NA, NA, NA, NA, NA, NA),
	PINGROUP(58, qpic_pad5, led2, blsp_i2c0, smart3, smart1, i2s_rx_mclk,
		 NA, wcss0_dbg14, tm4, wifi04, wifi14, NA, NA, NA),
	PINGROUP(59, qpic_pad6, blsp_i2c0, smart3, smart1, i2s_spdif_in, NA,
		 NA, wcss0_dbg15, qdss_tracectl_a, boot18, tm5, NA, NA, NA),
	PINGROUP(60, qpic_pad7, blsp_uart0, smart1, smart3, led0, i2s_tx_bclk,
		 i2s_rx_bclk, atest_char, NA, wcss0_dbg4, qdss_traceclk_a,
		 boot19, tm6, NA),
	PINGROUP(61, qpic_pad, blsp_uart0, smart1, smart3, led1, i2s_tx_fsync,
		 i2s_rx_fsync, NA, NA, wcss0_dbg5, qdss_cti_trig_out_a0,
		 boot14, tm7, NA),
	PINGROUP(62, qpic_pad, chip_rst, wifi0_uart, wifi1_uart, i2s_spdif_out,
		 NA, NA, wcss0_dbg6, qdss_cti_trig_out_b0, boot11, tm8, NA, NA,
		 NA),
	PINGROUP(63, qpic_pad, wifi0_uart1, wifi1_uart1, wifi1_uart, i2s_td1,
		 i2s_rxd, i2s_spdif_out, i2s_spdif_in, NA, wcss0_dbg7,
		 wcss1_dbg7, boot20, tm9, NA),
	PINGROUP(64, qpic_pad1, audio_pwm0, NA, wcss0_dbg8, wcss1_dbg8, NA, NA,
		 NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(65, qpic_pad2, audio_pwm1, NA, wcss0_dbg9, wcss1_dbg9, NA, NA,
		 NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(66, qpic_pad3, audio_pwm2, NA, wcss0_dbg10, wcss1_dbg10, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(67, qpic_pad0, audio_pwm3, NA, wcss0_dbg11, wcss1_dbg11, NA,
		 NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(68, qpic_pad8, NA, wcss0_dbg12, wcss1_dbg12, NA, NA, NA, NA,
		 NA, NA, NA, NA, NA, NA),
	PINGROUP(69, qpic_pad, NA, wcss0_dbg, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA, NA),
	PINGROUP(70, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(71, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(72, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(73, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(74, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(75, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(76, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(77, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(78, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(79, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(80, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(81, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(82, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(83, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(84, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(85, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(86, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(87, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(88, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(89, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(90, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(91, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(92, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(93, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(94, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(95, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(96, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(97, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
	PINGROUP(98, wifi034, wifi134, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA,
		 NA, NA),
	PINGROUP(99, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA, NA),
};

static const struct msm_pinctrl_gpio_pull ipq4019_gpio_pull = {
	.no_pull = 0,
	.pull_down = 1,
	.keeper = 0,
	.pull_up = 2,
};

static const struct msm_pinctrl_soc_data ipq4019_pinctrl = {
	.pins = ipq4019_pins,
	.npins = ARRAY_SIZE(ipq4019_pins),
	.functions = ipq4019_functions,
	.nfunctions = ARRAY_SIZE(ipq4019_functions),
	.groups = ipq4019_groups,
	.ngroups = ARRAY_SIZE(ipq4019_groups),
	.ngpios = 100,
	.gpio_pull = &ipq4019_gpio_pull,
};

static int ipq4019_pinctrl_probe(struct platform_device *pdev)
{
	return msm_pinctrl_probe(pdev, &ipq4019_pinctrl);
}

static const struct of_device_id ipq4019_pinctrl_of_match[] = {
	{ .compatible = "qcom,ipq4019-pinctrl", },
	{ },
};

static struct platform_driver ipq4019_pinctrl_driver = {
	.driver = {
		.name = "ipq4019-pinctrl",
		.owner = THIS_MODULE,
		.of_match_table = ipq4019_pinctrl_of_match,
	},
	.probe = ipq4019_pinctrl_probe,
	.remove = msm_pinctrl_remove,
};

static int __init ipq4019_pinctrl_init(void)
{
	return platform_driver_register(&ipq4019_pinctrl_driver);
}
arch_initcall(ipq4019_pinctrl_init);

static void __exit ipq4019_pinctrl_exit(void)
{
	platform_driver_unregister(&ipq4019_pinctrl_driver);
}
module_exit(ipq4019_pinctrl_exit);

MODULE_DESCRIPTION("Qualcomm ipq4019 pinctrl driver");
MODULE_LICENSE("GPL v2");
MODULE_DEVICE_TABLE(of, ipq4019_pinctrl_of_match);
