/*
 * module-common.c: Implements common utilities across CMIS, SFF-8436/8636
 * and SFF-8472/8079.
 */

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "module-common.h"

const struct module_aw_mod module_aw_mod_flags[] = {
	{ MODULE_TYPE_CMIS, "Module temperature high alarm",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_HALARM_STATUS },
	{ MODULE_TYPE_CMIS, "Module temperature low alarm",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_LALARM_STATUS },
	{ MODULE_TYPE_CMIS, "Module temperature high warning",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_HWARN_STATUS },
	{ MODULE_TYPE_CMIS, "Module temperature low warning",
	  CMIS_TEMP_AW_OFFSET, CMIS_TEMP_LWARN_STATUS },

	{ MODULE_TYPE_CMIS, "Module voltage high alarm",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_HALARM_STATUS },
	{ MODULE_TYPE_CMIS, "Module voltage low alarm",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_LALARM_STATUS },
	{ MODULE_TYPE_CMIS, "Module voltage high warning",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_HWARN_STATUS },
	{ MODULE_TYPE_CMIS, "Module voltage low warning",
	  CMIS_VCC_AW_OFFSET, CMIS_VCC_LWARN_STATUS },

	{ MODULE_TYPE_SFF8636, "Module temperature high alarm",
	  SFF8636_TEMP_AW_OFFSET, (SFF8636_TEMP_HALARM_STATUS) },
	{ MODULE_TYPE_SFF8636, "Module temperature low alarm",
	  SFF8636_TEMP_AW_OFFSET, (SFF8636_TEMP_LALARM_STATUS) },
	{ MODULE_TYPE_SFF8636, "Module temperature high warning",
	  SFF8636_TEMP_AW_OFFSET, (SFF8636_TEMP_HWARN_STATUS) },
	{ MODULE_TYPE_SFF8636, "Module temperature low warning",
	  SFF8636_TEMP_AW_OFFSET, (SFF8636_TEMP_LWARN_STATUS) },

	{ MODULE_TYPE_SFF8636, "Module voltage high alarm",
	  SFF8636_VCC_AW_OFFSET, (SFF8636_VCC_HALARM_STATUS) },
	{ MODULE_TYPE_SFF8636, "Module voltage low alarm",
	  SFF8636_VCC_AW_OFFSET, (SFF8636_VCC_LALARM_STATUS) },
	{ MODULE_TYPE_SFF8636, "Module voltage high warning",
	  SFF8636_VCC_AW_OFFSET, (SFF8636_VCC_HWARN_STATUS) },
	{ MODULE_TYPE_SFF8636, "Module voltage low warning",
	  SFF8636_VCC_AW_OFFSET, (SFF8636_VCC_LWARN_STATUS) },

	{ 0, NULL, 0, 0 },
};

const struct module_aw_chan module_aw_chan_flags[] = {
	{ MODULE_TYPE_CMIS, "Laser bias current high alarm",
	  CMIS_TX_BIAS_AW_HALARM_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_BIAS_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser bias current low alarm",
	  CMIS_TX_BIAS_AW_LALARM_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_BIAS_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser bias current high warning",
	  CMIS_TX_BIAS_AW_HWARN_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_BIAS_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser bias current low warning",
	  CMIS_TX_BIAS_AW_LWARN_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_BIAS_MON_MASK },

	{ MODULE_TYPE_CMIS, "Laser tx power high alarm",
	  CMIS_TX_PWR_AW_HALARM_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_PWR_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser tx power low alarm",
	  CMIS_TX_PWR_AW_LALARM_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_PWR_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser tx power high warning",
	  CMIS_TX_PWR_AW_HWARN_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_PWR_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser tx power low warning",
	  CMIS_TX_PWR_AW_LWARN_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_TX_PWR_MON_MASK },

	{ MODULE_TYPE_CMIS, "Laser rx power high alarm",
	  CMIS_RX_PWR_AW_HALARM_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_RX_PWR_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser rx power low alarm",
	  CMIS_RX_PWR_AW_LALARM_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_RX_PWR_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser rx power high warning",
	  CMIS_RX_PWR_AW_HWARN_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_RX_PWR_MON_MASK },
	{ MODULE_TYPE_CMIS, "Laser rx power low warning",
	  CMIS_RX_PWR_AW_LWARN_OFFSET, CMIS_DIAG_CHAN_ADVER_OFFSET,
	  CMIS_RX_PWR_MON_MASK },

	{ MODULE_TYPE_SFF8636, "Laser bias current high alarm",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0, (SFF8636_TX_BIAS_1_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser bias current high alarm",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0, (SFF8636_TX_BIAS_2_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser bias current high alarm",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_3_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser bias current high alarm",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_4_HALARM) },

	{ MODULE_TYPE_SFF8636, "Laser bias current low alarm",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0,(SFF8636_TX_BIAS_1_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser bias current low alarm",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0, (SFF8636_TX_BIAS_2_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser bias current low alarm",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_3_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser bias current low alarm",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_4_LALARM) },

	{ MODULE_TYPE_SFF8636, "Laser bias current high warning",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0, (SFF8636_TX_BIAS_1_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser bias current high warning",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0, (SFF8636_TX_BIAS_2_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser bias current high warning",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_3_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser bias current high warning",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_4_HWARN) },

	{ MODULE_TYPE_SFF8636, "Laser bias current low warning",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0, (SFF8636_TX_BIAS_1_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser bias current low warning",
	  SFF8636_TX_BIAS_12_AW_OFFSET, 0, (SFF8636_TX_BIAS_2_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser bias current low warning",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_3_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser bias current low warning",
	  SFF8636_TX_BIAS_34_AW_OFFSET, 0, (SFF8636_TX_BIAS_4_LWARN) },

	{ MODULE_TYPE_SFF8636, "Laser tx power high alarm",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_1_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser tx power high alarm",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_2_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser tx power high alarm",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_3_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser tx power high alarm",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_4_HALARM) },

	{ MODULE_TYPE_SFF8636, "Laser tx power low alarm",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_1_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser tx power low alarm",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_2_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser tx power low alarm",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_3_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser tx power low alarm",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_4_LALARM) },

	{ MODULE_TYPE_SFF8636, "Laser tx power high warning",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_1_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser tx power high warning",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_2_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser tx power high warning",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_3_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser tx power high warning",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_4_HWARN) },

	{ MODULE_TYPE_SFF8636, "Laser tx power low warning",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_1_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser tx power low warning",
	  SFF8636_TX_PWR_12_AW_OFFSET, 0, (SFF8636_TX_PWR_2_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser tx power low warning",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_3_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser tx power low warning",
	  SFF8636_TX_PWR_34_AW_OFFSET, 0, (SFF8636_TX_PWR_4_LWARN) },

	{ MODULE_TYPE_SFF8636, "Laser rx power high alarm",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_1_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser rx power high alarm",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_2_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser rx power high alarm",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_3_HALARM) },
	{ MODULE_TYPE_SFF8636, "Laser rx power high alarm",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_4_HALARM) },

	{ MODULE_TYPE_SFF8636, "Laser rx power low alarm",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_1_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser rx power low alarm",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_2_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser rx power low alarm",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_3_LALARM) },
	{ MODULE_TYPE_SFF8636, "Laser rx power low alarm",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_4_LALARM) },

	{ MODULE_TYPE_SFF8636, "Laser rx power high warning",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_1_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser rx power high warning",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_2_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser rx power high warning",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_3_HWARN) },
	{ MODULE_TYPE_SFF8636, "Laser rx power high warning",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_4_HWARN) },

	{ MODULE_TYPE_SFF8636, "Laser rx power low warning",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_1_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser rx power low warning",
	  SFF8636_RX_PWR_12_AW_OFFSET, 0, (SFF8636_RX_PWR_2_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser rx power low warning",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_3_LWARN) },
	{ MODULE_TYPE_SFF8636, "Laser rx power low warning",
	  SFF8636_RX_PWR_34_AW_OFFSET, 0, (SFF8636_RX_PWR_4_LWARN) },

	{ 0, NULL, 0, 0, 0 },
};

void convert_json_field_name(const char *str,  char *json_str)
{
	for (size_t i = 0; i < strlen(str); i++)
		json_str[i] = (str[i] == ' ') ?
				'_' : tolower((unsigned char)str[i]);
}

void module_print_any_uint(const char *fn, int value, const char *unit)
{
	char json_fn[100] = "";

	if (is_json_context()) {
		convert_json_field_name(fn, json_fn);
		print_uint(PRINT_JSON, json_fn, "%u", value);
	} else {
		printf("\t%-41s : %u%s\n", fn, value, unit ? unit : "");
	}
}

void module_print_any_string(const char *fn, const char *value)
{
	char json_fn[100] = "";

	if (is_json_context()) {
		convert_json_field_name(fn, json_fn);
		print_string(PRINT_JSON, json_fn, "%s", value);
	} else {
		printf("\t%-41s : %s\n", fn, value);
	}
}

void module_print_any_float(const char *fn, float value, const char *unit)
{
	char json_fn[100] = "";

	if (is_json_context()) {
		convert_json_field_name(fn, json_fn);
		print_float(PRINT_JSON, json_fn, "%.04f", value);
	} else {
		printf("\t%-41s : %.04f%s\n", fn, value, unit ? unit : "");
	}
}

void module_print_any_bool(const char *fn, char *given_json_fn, bool value,
			   const char *str_value)
{
	char json_fn[100] = "";

	if (!given_json_fn)
		convert_json_field_name(fn, json_fn);
	else
		strcpy(json_fn, given_json_fn);

	if (is_json_context())
		print_bool(PRINT_JSON, json_fn, NULL, value);
	else
		printf("\t%-41s : %s\n", fn, str_value);
}

void module_show_value_with_unit(const __u8 *id, unsigned int reg,
				 const char *name, unsigned int mult,
				 const char *unit)
{
	unsigned int val = id[reg];

	module_print_any_uint(name, val * mult, unit);
}

void module_show_ascii(const __u8 *id, unsigned int first_reg,
		       unsigned int last_reg, const char *name)
{
	char json_fn[100] = "";
	char val_str[32] = "";
	unsigned int reg, val;

	if (is_json_context())
		convert_json_field_name(name, json_fn);
	else
		printf("\t%-41s : ", name);

	while (first_reg <= last_reg && id[last_reg] == ' ')
		last_reg--;
	for (reg = first_reg; reg <= last_reg; reg++) {
		char val_char;

		val = id[reg];
		val_char = (char)val;

		if (is_json_context())
			val_str[reg - first_reg] = (((val >= 32) && (val <= 126)) ?
						    val_char : '_');
		else
			putchar(((val >= 32) && (val <= 126)) ? val : '_');
	}

	if (is_json_context())
		print_string(PRINT_JSON, json_fn, "%s", val_str);
	else
		printf("\n");
}

void module_show_lane_status(const char *name, unsigned int lane_cnt,
			     const char *yes, const char *no,
			     unsigned int value)
{
	char json_fn[100] = "";

	convert_json_field_name(name, json_fn);

	if (!value) {
		if (is_json_context())
			print_bool(PRINT_JSON, json_fn, NULL, false);
		else
			printf("\t%-41s : None\n", name);
		return;
	}

	if (is_json_context()) {
		open_json_array(json_fn, "");

		while (lane_cnt--) {
			print_string(PRINT_JSON, NULL, "%s",
				     value & 1 ? yes : no);
			value >>= 1;
		}
		close_json_array("");
	} else {
		printf("\t%-41s : [", name);
		while (lane_cnt--) {
			printf(" %s%c", value & 1 ? yes : no, lane_cnt ? ',': ' ');
			value >>= 1;
		}
		printf("]\n");
	}
}

void module_show_oui(const __u8 *id, int id_offset)
{
	char oui_value[16];

	if (is_json_context()) {
		open_json_array("vendor_oui", "");
		print_int(PRINT_JSON, NULL, NULL, id[id_offset]);
		print_int(PRINT_JSON, NULL, NULL, id[(id_offset) + 1]);
		print_int(PRINT_JSON, NULL, NULL, id[(id_offset) + 2]);
		close_json_array("");
	} else {
		snprintf(oui_value, 16, "%02x:%02x:%02x", id[id_offset],
			 id[(id_offset) + 1], id[(id_offset) + 2]);
		printf("\t%-41s : %s\n", "Vendor OUI", oui_value);
	}
}

void module_show_identifier(const __u8 *id, int id_offset)
{
	char id_description[SFF_MAX_DESC_LEN];

	switch (id[id_offset]) {
	case MODULE_ID_UNKNOWN:
		strncpy(id_description,
			"no module present, unknown, or unspecified",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_GBIC:
		strncpy(id_description, "GBIC", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_SOLDERED_MODULE:
		strncpy(id_description,
			"module soldered to motherboard", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_SFP:
		strncpy(id_description, "SFP", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_300_PIN_XBI:
		strncpy(id_description, "300 pin XBI", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_XENPAK:
		strncpy(id_description, "XENPAK", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_XFP:
		strncpy(id_description, "XFP", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_XFF:
		strncpy(id_description, "XFF", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_XFP_E:
		strncpy(id_description, "XFP-E", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_XPAK:
		strncpy(id_description, "XPAK", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_X2:
		strncpy(id_description, "X2", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_DWDM_SFP:
		strncpy(id_description, "DWDM-SFP", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_QSFP:
		strncpy(id_description, "QSFP", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_QSFP_PLUS:
		strncpy(id_description, "QSFP+", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_CXP:
		strncpy(id_description, "CXP", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_HD4X:
		strncpy(id_description, "Shielded Mini Multilane HD 4X",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_HD8X:
		strncpy(id_description, "Shielded Mini Multilane HD 8X",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_QSFP28:
		strncpy(id_description, "QSFP28", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_CXP2:
		strncpy(id_description, "CXP2/CXP28", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_CDFP:
		strncpy(id_description, "CDFP Style 1/Style 2",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_HD4X_FANOUT:
		strncpy(id_description,
			"Shielded Mini Multilane HD 4X Fanout Cable",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_HD8X_FANOUT:
		strncpy(id_description,
			"Shielded Mini Multilane HD 8X Fanout Cable",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_CDFP_S3:
		strncpy(id_description, "CDFP Style 3", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_MICRO_QSFP:
		strncpy(id_description, "microQSFP", SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_QSFP_DD:
		strncpy(id_description,
			"QSFP-DD Double Density 8X Pluggable Transceiver (INF-8628)",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_OSFP:
		strncpy(id_description, "OSFP 8X Pluggable Transceiver",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_DSFP:
		strncpy(id_description,
			"DSFP Dual Small Form Factor Pluggable Transceiver",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_QSFP_PLUS_CMIS:
		strncpy(id_description,
			"QSFP+ or later with Common Management Interface Specification (CMIS)",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_SFP_DD_CMIS:
		strncpy(id_description,
			"SFP-DD Double Density 2X Pluggable Transceiver with Common Management Interface Specification (CMIS)",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_ID_SFP_PLUS_CMIS:
		strncpy(id_description,
			"SFP+ and later with Common Management Interface Specification (CMIS)",
			SFF_MAX_DESC_LEN);
		break;
	default:
		strncpy(id_description, "reserved or unknown",
			SFF_MAX_DESC_LEN);
		break;
	}

	sff_print_any_hex_field("Identifier", "identifier", id[id_offset],
				id_description);
}

void module_show_connector(const __u8 *id, int ctor_offset)
{
	char ctor_description[SFF_MAX_DESC_LEN];

	switch (id[ctor_offset]) {
	case  MODULE_CTOR_UNKNOWN:
		strncpy(ctor_description, "unknown or unspecified",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_SC:
		strncpy(ctor_description, "SC", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_FC_STYLE_1:
		strncpy(ctor_description, "Fibre Channel Style 1 copper",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_FC_STYLE_2:
		strncpy(ctor_description, "Fibre Channel Style 2 copper",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_BNC_TNC:
		strncpy(ctor_description, "BNC/TNC", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_FC_COAX:
		strncpy(ctor_description, "Fibre Channel coaxial headers",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_FIBER_JACK:
		strncpy(ctor_description, "FibreJack", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_LC:
		strncpy(ctor_description, "LC", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_MT_RJ:
		strncpy(ctor_description, "MT-RJ", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_MU:
		strncpy(ctor_description, "MU", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_SG:
		strncpy(ctor_description, "SG", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_OPT_PT:
		strncpy(ctor_description, "Optical pigtail",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_MPO:
		strncpy(ctor_description, "MPO Parallel Optic",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_MPO_2:
		strncpy(ctor_description, "MPO Parallel Optic - 2x16",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_HSDC_II:
		strncpy(ctor_description, "HSSDC II", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_COPPER_PT:
		strncpy(ctor_description, "Copper pigtail",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_RJ45:
		strncpy(ctor_description, "RJ45", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_NO_SEPARABLE:
		strncpy(ctor_description, "No separable connector",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_MXC_2x16:
		strncpy(ctor_description, "MXC 2x16", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_CS_OPTICAL:
		strncpy(ctor_description, "CS optical connector",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_CS_OPTICAL_MINI:
		strncpy(ctor_description, "Mini CS optical connector",
			SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_MPO_2X12:
		strncpy(ctor_description, "MPO 2x12", SFF_MAX_DESC_LEN);
		break;
	case MODULE_CTOR_MPO_1X16:
		strncpy(ctor_description, "MPO 1x16", SFF_MAX_DESC_LEN);
		break;
	default:
		strncpy(ctor_description, "reserved or unknown",
			SFF_MAX_DESC_LEN);
		break;
	}

	sff_print_any_hex_field("Connector", "connector", id[ctor_offset],
				ctor_description);
}

void module_show_mit_compliance(u16 value)
{
	static const char *cc = "Copper cable,";
	char description[SFF_MAX_DESC_LEN];

	switch (value) {
	case MODULE_850_VCSEL:
		strncpy(description, "850 nm VCSEL", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1310_VCSEL:
	case SFF8636_TRANS_1310_VCSEL:
		strncpy(description, "1310 nm VCSEL", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1550_VCSEL:
	case SFF8636_TRANS_1550_VCSEL:
		strncpy(description, "1550 nm VCSEL", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1310_FP:
	case SFF8636_TRANS_1310_FP:
		strncpy(description, "1310 nm FP", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1310_DFB:
	case SFF8636_TRANS_1310_DFB:
		strncpy(description, "1310 nm DFB", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1550_DFB:
	case SFF8636_TRANS_1550_DFB:
		strncpy(description, "1550 nm DFB", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1310_EML:
	case SFF8636_TRANS_1310_EML:
		strncpy(description, "1310 nm EML", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1550_EML:
	case SFF8636_TRANS_1550_EML:
		strncpy(description, "1550 nm EML", SFF_MAX_DESC_LEN);
		break;
	case CMIS_OTHERS:
	case SFF8636_TRANS_OTHERS:
		strncpy(description, "Others/Undefined", SFF_MAX_DESC_LEN);
		break;
	case CMIS_1490_DFB:
	case SFF8636_TRANS_1490_DFB:
		strncpy(description, "1490 nm DFB", SFF_MAX_DESC_LEN);
		break;
	case CMIS_COPPER_UNEQUAL:
	case SFF8636_TRANS_COPPER_PAS_UNEQUAL:
		snprintf(description, SFF_MAX_DESC_LEN, "%s unequalized", cc);
		break;
	case CMIS_COPPER_PASS_EQUAL:
	case SFF8636_TRANS_COPPER_PAS_EQUAL:
		snprintf(description, SFF_MAX_DESC_LEN, "%s passive equalized",
			 cc);
		break;
	case CMIS_COPPER_NF_EQUAL:
	case SFF8636_TRANS_COPPER_LNR_FAR_EQUAL:
		snprintf(description, SFF_MAX_DESC_LEN,
			 "%s near and far end limiting active equalizers", cc);
		break;
	case CMIS_COPPER_F_EQUAL:
	case SFF8636_TRANS_COPPER_FAR_EQUAL:
		snprintf(description, SFF_MAX_DESC_LEN,
			 "%s far end limiting active equalizers", cc);
		break;
	case CMIS_COPPER_N_EQUAL:
	case SFF8636_TRANS_COPPER_NEAR_EQUAL:
		snprintf(description, SFF_MAX_DESC_LEN,
			 "%s near end limiting active equalizers", cc);
		break;
	case CMIS_COPPER_LINEAR_EQUAL:
	case SFF8636_TRANS_COPPER_LNR_EQUAL:
		snprintf(description, SFF_MAX_DESC_LEN, "%s linear active equalizers",
			 cc);
		break;
	}

	sff_print_any_hex_field("Transmitter technology",
				"transmitter_technology", value, description);
}

void module_show_dom_mod_lvl_monitors(const struct sff_diags *sd)
{
	PRINT_TEMP_ALL("Module temperature", "module_temperature",
		       sd->sfp_temp[MCURR]);
	PRINT_VCC_ALL("Module voltage", "module_voltage",
		      sd->sfp_voltage[MCURR]);
}
