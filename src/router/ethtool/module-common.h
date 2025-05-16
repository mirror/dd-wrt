/*
 * module-common.h: Declares common utilities across CMIS, SFF-8436/8636
 * and SFF-8472/8079.
 */

#ifndef MODULE_COMMON_H__
#define MODULE_COMMON_H__

#include <stdio.h>
#include "internal.h"
#include "sff-common.h"

enum module_type {
	MODULE_TYPE_SFF8636,
	MODULE_TYPE_CMIS,
};

#define  MODULE_ID_OFFSET				0x00
#define  MODULE_ID_UNKNOWN				0x00
#define  MODULE_ID_GBIC				0x01
#define  MODULE_ID_SOLDERED_MODULE		0x02
#define  MODULE_ID_SFP					0x03
#define  MODULE_ID_300_PIN_XBI			0x04
#define  MODULE_ID_XENPAK				0x05
#define  MODULE_ID_XFP					0x06
#define  MODULE_ID_XFF					0x07
#define  MODULE_ID_XFP_E				0x08
#define  MODULE_ID_XPAK				0x09
#define  MODULE_ID_X2					0x0A
#define  MODULE_ID_DWDM_SFP			0x0B
#define  MODULE_ID_QSFP				0x0C
#define  MODULE_ID_QSFP_PLUS			0x0D
#define  MODULE_ID_CXP					0x0E
#define  MODULE_ID_HD4X				0x0F
#define  MODULE_ID_HD8X				0x10
#define  MODULE_ID_QSFP28				0x11
#define  MODULE_ID_CXP2				0x12
#define  MODULE_ID_CDFP				0x13
#define  MODULE_ID_HD4X_FANOUT			0x14
#define  MODULE_ID_HD8X_FANOUT			0x15
#define  MODULE_ID_CDFP_S3				0x16
#define  MODULE_ID_MICRO_QSFP			0x17
#define  MODULE_ID_QSFP_DD				0x18
#define  MODULE_ID_OSFP				0x19
#define  MODULE_ID_DSFP				0x1B
#define  MODULE_ID_QSFP_PLUS_CMIS			0x1E
#define  MODULE_ID_SFP_DD_CMIS				0x1F
#define  MODULE_ID_SFP_PLUS_CMIS			0x20
#define  MODULE_ID_LAST				MODULE_ID_SFP_PLUS_CMIS
#define  MODULE_ID_UNALLOCATED_LAST	0x7F
#define  MODULE_ID_VENDOR_START		0x80
#define  MODULE_ID_VENDOR_LAST			0xFF

#define  MODULE_CTOR_UNKNOWN			0x00
#define  MODULE_CTOR_SC				0x01
#define  MODULE_CTOR_FC_STYLE_1		0x02
#define  MODULE_CTOR_FC_STYLE_2		0x03
#define  MODULE_CTOR_BNC_TNC			0x04
#define  MODULE_CTOR_FC_COAX			0x05
#define  MODULE_CTOR_FIBER_JACK		0x06
#define  MODULE_CTOR_LC				0x07
#define  MODULE_CTOR_MT_RJ				0x08
#define  MODULE_CTOR_MU				0x09
#define  MODULE_CTOR_SG				0x0A
#define  MODULE_CTOR_OPT_PT			0x0B
#define  MODULE_CTOR_MPO				0x0C
#define  MODULE_CTOR_MPO_2				0x0D
/* 0E-1Fh --- Reserved */
#define  MODULE_CTOR_HSDC_II			0x20
#define  MODULE_CTOR_COPPER_PT			0x21
#define  MODULE_CTOR_RJ45				0x22
#define  MODULE_CTOR_NO_SEPARABLE		0x23
#define  MODULE_CTOR_MXC_2x16			0x24
#define  MODULE_CTOR_CS_OPTICAL		0x25
#define  MODULE_CTOR_CS_OPTICAL_MINI		0x26
#define  MODULE_CTOR_MPO_2X12			0x27
#define  MODULE_CTOR_MPO_1X16			0x28
#define  MODULE_CTOR_LAST			MODULE_CTOR_MPO_1X16

#define  MODULE_CTOR_NO_SEP_QSFP_DD		0x6F
#define  MODULE_CTOR_UNALLOCATED_LAST		0x7F
#define  MODULE_CTOR_VENDOR_START		0x80
#define  MODULE_CTOR_VENDOR_LAST		0xFF

/* Transmitter Technology */
#define MODULE_850_VCSEL			0x00

/* SFF8636 */
#define	 SFF8636_TRANS_TECH_MASK		0xF0
#define	 SFF8636_TRANS_COPPER_LNR_EQUAL		(15 << 4)
#define	 SFF8636_TRANS_COPPER_NEAR_EQUAL	(14 << 4)
#define	 SFF8636_TRANS_COPPER_FAR_EQUAL		(13 << 4)
#define	 SFF8636_TRANS_COPPER_LNR_FAR_EQUAL	(12 << 4)
#define	 SFF8636_TRANS_COPPER_PAS_EQUAL		(11 << 4)
#define	 SFF8636_TRANS_COPPER_PAS_UNEQUAL	(10 << 4)
#define	 SFF8636_TRANS_1490_DFB			(9 << 4)
#define	 SFF8636_TRANS_OTHERS			(8 << 4)
#define	 SFF8636_TRANS_1550_EML			(7 << 4)
#define	 SFF8636_TRANS_1310_EML			(6 << 4)
#define  SFF8636_TRANS_1550_DFB			(5 << 4)
#define	 SFF8636_TRANS_1310_DFB			(4 << 4)
#define	 SFF8636_TRANS_1310_FP			(3 << 4)
#define	 SFF8636_TRANS_1550_VCSEL		(2 << 4)
#define	 SFF8636_TRANS_1310_VCSEL		(1 << 4)

/* CMIS */
#define CMIS_1310_VCSEL				0x01
#define CMIS_1550_VCSEL				0x02
#define CMIS_1310_FP				0x03
#define CMIS_1310_DFB				0x04
#define CMIS_1550_DFB				0x05
#define CMIS_1310_EML				0x06
#define CMIS_1550_EML				0x07
#define CMIS_OTHERS				0x08
#define CMIS_1490_DFB				0x09
#define CMIS_COPPER_UNEQUAL			0x0A
#define CMIS_COPPER_PASS_EQUAL			0x0B
#define CMIS_COPPER_NF_EQUAL			0x0C
#define CMIS_COPPER_F_EQUAL			0x0D
#define CMIS_COPPER_N_EQUAL			0x0E
#define CMIS_COPPER_LINEAR_EQUAL		0x0F

/* Module Flags (Page 0) */
#define CMIS_VCC_AW_OFFSET			0x09
#define CMIS_VCC_LWARN_STATUS			0x80
#define CMIS_VCC_HWARN_STATUS			0x40
#define CMIS_VCC_LALARM_STATUS			0x20
#define CMIS_VCC_HALARM_STATUS			0x10
#define CMIS_TEMP_AW_OFFSET			0x09
#define CMIS_TEMP_LWARN_STATUS			0x08
#define CMIS_TEMP_HWARN_STATUS			0x04
#define CMIS_TEMP_LALARM_STATUS			0x02
#define CMIS_TEMP_HALARM_STATUS			0x01

/* Supported Monitors Advertisement (Page 1) */
#define CMIS_DIAG_CHAN_ADVER_OFFSET		0xA0

/* Module Monitor Interrupt Flags - 6-8 */
#define	SFF8636_TEMP_AW_OFFSET	0x06
#define	 SFF8636_TEMP_HALARM_STATUS		(1 << 7)
#define	 SFF8636_TEMP_LALARM_STATUS		(1 << 6)
#define	 SFF8636_TEMP_HWARN_STATUS		(1 << 5)
#define	 SFF8636_TEMP_LWARN_STATUS		(1 << 4)

#define	SFF8636_VCC_AW_OFFSET	0x07
#define	 SFF8636_VCC_HALARM_STATUS		(1 << 7)
#define	 SFF8636_VCC_LALARM_STATUS		(1 << 6)
#define	 SFF8636_VCC_HWARN_STATUS		(1 << 5)
#define	 SFF8636_VCC_LWARN_STATUS		(1 << 4)

/* Channel Monitor Interrupt Flags - 9-21 */
#define	SFF8636_RX_PWR_12_AW_OFFSET	0x09
#define	 SFF8636_RX_PWR_1_HALARM		(1 << 7)
#define	 SFF8636_RX_PWR_1_LALARM		(1 << 6)
#define	 SFF8636_RX_PWR_1_HWARN			(1 << 5)
#define	 SFF8636_RX_PWR_1_LWARN			(1 << 4)
#define	 SFF8636_RX_PWR_2_HALARM		(1 << 3)
#define	 SFF8636_RX_PWR_2_LALARM		(1 << 2)
#define	 SFF8636_RX_PWR_2_HWARN			(1 << 1)
#define	 SFF8636_RX_PWR_2_LWARN			(1 << 0)

#define	SFF8636_RX_PWR_34_AW_OFFSET	0x0A
#define	 SFF8636_RX_PWR_3_HALARM		(1 << 7)
#define	 SFF8636_RX_PWR_3_LALARM		(1 << 6)
#define	 SFF8636_RX_PWR_3_HWARN			(1 << 5)
#define	 SFF8636_RX_PWR_3_LWARN			(1 << 4)
#define	 SFF8636_RX_PWR_4_HALARM		(1 << 3)
#define	 SFF8636_RX_PWR_4_LALARM		(1 << 2)
#define	 SFF8636_RX_PWR_4_HWARN			(1 << 1)
#define	 SFF8636_RX_PWR_4_LWARN			(1 << 0)

#define	SFF8636_TX_BIAS_12_AW_OFFSET	0x0B
#define	 SFF8636_TX_BIAS_1_HALARM		(1 << 7)
#define	 SFF8636_TX_BIAS_1_LALARM		(1 << 6)
#define	 SFF8636_TX_BIAS_1_HWARN		(1 << 5)
#define	 SFF8636_TX_BIAS_1_LWARN		(1 << 4)
#define	 SFF8636_TX_BIAS_2_HALARM		(1 << 3)
#define	 SFF8636_TX_BIAS_2_LALARM		(1 << 2)
#define	 SFF8636_TX_BIAS_2_HWARN		(1 << 1)
#define	 SFF8636_TX_BIAS_2_LWARN		(1 << 0)

#define	SFF8636_TX_BIAS_34_AW_OFFSET	0xC
#define	 SFF8636_TX_BIAS_3_HALARM		(1 << 7)
#define	 SFF8636_TX_BIAS_3_LALARM		(1 << 6)
#define	 SFF8636_TX_BIAS_3_HWARN		(1 << 5)
#define	 SFF8636_TX_BIAS_3_LWARN		(1 << 4)
#define	 SFF8636_TX_BIAS_4_HALARM		(1 << 3)
#define	 SFF8636_TX_BIAS_4_LALARM		(1 << 2)
#define	 SFF8636_TX_BIAS_4_HWARN		(1 << 1)
#define	 SFF8636_TX_BIAS_4_LWARN		(1 << 0)

#define	SFF8636_TX_PWR_12_AW_OFFSET	0x0D
#define	 SFF8636_TX_PWR_1_HALARM		(1 << 7)
#define	 SFF8636_TX_PWR_1_LALARM		(1 << 6)
#define	 SFF8636_TX_PWR_1_HWARN			(1 << 5)
#define	 SFF8636_TX_PWR_1_LWARN			(1 << 4)
#define	 SFF8636_TX_PWR_2_HALARM		(1 << 3)
#define	 SFF8636_TX_PWR_2_LALARM		(1 << 2)
#define	 SFF8636_TX_PWR_2_HWARN			(1 << 1)
#define	 SFF8636_TX_PWR_2_LWARN			(1 << 0)

#define	SFF8636_TX_PWR_34_AW_OFFSET	0x0E
#define	 SFF8636_TX_PWR_3_HALARM		(1 << 7)
#define	 SFF8636_TX_PWR_3_LALARM		(1 << 6)
#define	 SFF8636_TX_PWR_3_HWARN			(1 << 5)
#define	 SFF8636_TX_PWR_3_LWARN			(1 << 4)
#define	 SFF8636_TX_PWR_4_HALARM		(1 << 3)
#define	 SFF8636_TX_PWR_4_LALARM		(1 << 2)
#define	 SFF8636_TX_PWR_4_HWARN			(1 << 1)
#define	 SFF8636_TX_PWR_4_LWARN			(1 << 0)

/*-----------------------------------------------------------------------
 * Upper Memory Page 0x11: Optional Page that contains lane dynamic status
 * bytes.
 */

/* Media Lane-Specific Flags (Page 0x11) */
#define CMIS_TX_FAIL_OFFSET			0x87
#define CMIS_TX_LOS_OFFSET			0x88
#define CMIS_TX_LOL_OFFSET			0x89
#define CMIS_TX_EQ_FAIL_OFFSET			0x8a
#define CMIS_TX_PWR_AW_HALARM_OFFSET		0x8B
#define CMIS_TX_PWR_AW_LALARM_OFFSET		0x8C
#define CMIS_TX_PWR_AW_HWARN_OFFSET		0x8D
#define CMIS_TX_PWR_AW_LWARN_OFFSET		0x8E
#define CMIS_TX_BIAS_AW_HALARM_OFFSET		0x8F
#define CMIS_TX_BIAS_AW_LALARM_OFFSET		0x90
#define CMIS_TX_BIAS_AW_HWARN_OFFSET		0x91
#define CMIS_TX_BIAS_AW_LWARN_OFFSET		0x92
#define CMIS_RX_LOS_OFFSET			0x93
#define CMIS_RX_LOL_OFFSET			0x94
#define CMIS_RX_PWR_AW_HALARM_OFFSET		0x95
#define CMIS_RX_PWR_AW_LALARM_OFFSET		0x96
#define CMIS_RX_PWR_AW_HWARN_OFFSET		0x97
#define CMIS_RX_PWR_AW_LWARN_OFFSET		0x98

/* Media Lane-Specific Monitors (Page 0x11) */
#define CMIS_TX_PWR_OFFSET			0x9A
#define CMIS_TX_BIAS_OFFSET			0xAA
#define CMIS_RX_PWR_OFFSET			0xBA

#define CMIS_TX_BIAS_MON_MASK			0x01
#define CMIS_TX_PWR_MON_MASK			0x02
#define CMIS_RX_PWR_MON_MASK			0x04

#define YESNO(x) (((x) != 0) ? "Yes" : "No")
#define ONOFF(x) (((x) != 0) ? "On" : "Off")

struct module_aw_mod {
	enum module_type type;
	const char *str;	/* Human-readable string, null at the end */
	int offset;
	__u8 value;		/* Alarm is on if (offset & value) != 0. */
};

struct module_aw_chan {
	enum module_type type;
	const char *fmt_str;
	int offset;
	int adver_offset;	/* In Page 01h. */
	__u8 adver_value;	/* Supported if (offset & value) != 0. */
};

extern const struct module_aw_mod module_aw_mod_flags[];
extern const struct module_aw_chan module_aw_chan_flags[];

void convert_json_field_name(const char *str, char *json_str);
void module_print_any_uint(const char *fn, int value, const char *unit);
void module_print_any_string(const char *fn, const char *value);
void module_print_any_float(const char *fn, float value, const char *unit);
void module_print_any_bool(const char *fn, char *given_json_fn, bool value,
			   const char *str_value);
void module_show_value_with_unit(const __u8 *id, unsigned int reg,
				 const char *name, unsigned int mult,
				 const char *unit);
void module_show_ascii(const __u8 *id, unsigned int first_reg,
		       unsigned int last_reg, const char *name);
void module_show_lane_status(const char *name, unsigned int lane_cnt,
			     const char *yes, const char *no,
			     unsigned int value);
void module_show_oui(const __u8 *id, int id_offset);
void module_show_identifier(const __u8 *id, int id_offset);
void module_show_connector(const __u8 *id, int ctor_offset);
void module_show_mit_compliance(u16 value);
void module_show_dom_mod_lvl_monitors(const struct sff_diags *sd);

#endif /* MODULE_COMMON_H__ */
