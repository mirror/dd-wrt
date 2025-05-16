/*
 * sff-common.c: Implements SFF-8024 Rev 4.0 i.e. Specifcation
 * of pluggable I/O configuration
 *
 * Common utilities across SFF-8436/8636 and SFF-8472/8079
 * are defined in this file
 *
 * Copyright 2010 Solarflare Communications Inc.
 * Aurelien Guillaume <aurelien@iwi.me> (C) 2012
 * Copyright (C) 2014 Cumulus networks Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Freeoftware Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *  Vidya Sagar Ravipati <vidya@cumulusnetworks.com>
 *   This implementation is loosely based on current SFP parser
 *   and SFF-8024 Rev 4.0 spec (ftp://ftp.seagate.com/pub/sff/SFF-8024.PDF)
 *   by SFF Committee.
 */

#include <stdio.h>
#include <math.h>
#include "sff-common.h"

double convert_mw_to_dbm(double mw)
{
	return (10. * log10(mw / 1000.)) + 30.;
}

void sff_print_any_hex_field(const char *field_name,
			     const char *json_field_name, u8 value,
			     const char *desc)
{
	char desc_name[SFF_MAX_FIELD_LEN];

	if (is_json_context()) {
		print_uint(PRINT_JSON, json_field_name, "%u", value);
		if (desc) {
			snprintf(desc_name, SFF_MAX_FIELD_LEN,
				 "%s_description", json_field_name);
			print_string(PRINT_JSON, desc_name, "%s", desc);
		}
	} else {
		printf("\t%-41s : 0x%02x", field_name, value);
		if (desc)
			printf(" (%s)", desc);
		print_nl();
	}
}

void sff8024_show_encoding(const __u8 *id, int encoding_offset, int sff_type)
{
	char encoding_desc[64];

	switch (id[encoding_offset]) {
	case SFF8024_ENCODING_UNSPEC:
		strncpy(encoding_desc, "unspecified", 64);
		break;
	case SFF8024_ENCODING_8B10B:
		strncpy(encoding_desc, "8B/10B", 64);
		break;
	case SFF8024_ENCODING_4B5B:
		strncpy(encoding_desc, "4B/5B", 64);
		break;
	case SFF8024_ENCODING_NRZ:
		strncpy(encoding_desc, "NRZ", 64);
		break;
	case SFF8024_ENCODING_4h:
		if (sff_type == ETH_MODULE_SFF_8472)
			strncpy(encoding_desc, "Manchester", 64);
		else if (sff_type == ETH_MODULE_SFF_8636)
			strncpy(encoding_desc, "SONET Scrambled", 64);
		break;
	case SFF8024_ENCODING_5h:
		if (sff_type == ETH_MODULE_SFF_8472)
			strncpy(encoding_desc, "SONET Scrambled", 64);
		else if (sff_type == ETH_MODULE_SFF_8636)
			strncpy(encoding_desc, "64B/66B", 64);
		break;
	case SFF8024_ENCODING_6h:
		if (sff_type == ETH_MODULE_SFF_8472)
			strncpy(encoding_desc, "64B/66B", 64);
		else if (sff_type == ETH_MODULE_SFF_8636)
			strncpy(encoding_desc, "Manchester", 64);
		break;
	case SFF8024_ENCODING_256B:
		strncpy(encoding_desc,
			"256B/257B (transcoded FEC-enabled data)", 64);
		break;
	case SFF8024_ENCODING_PAM4:
		strncpy(encoding_desc, "PAM4", 64);
		break;
	default:
		strncpy(encoding_desc, "reserved or unknown", 64);
		break;
	}

	sff_print_any_hex_field("Encoding", "encoding", id[encoding_offset],
				encoding_desc);
}


void sff_show_thresholds_json(struct sff_diags sd)
{
	open_json_object("laser_bias_current");
	PRINT_BIAS_JSON("high_alarm_threshold", sd.bias_cur[HALRM]);
	PRINT_BIAS_JSON("low_alarm_threshold", sd.bias_cur[LALRM]);
	PRINT_BIAS_JSON("high_warning_threshold", sd.bias_cur[HWARN]);
	PRINT_BIAS_JSON("low_warning_threshold", sd.bias_cur[LWARN]);
	close_json_object();

	open_json_object("laser_output_power");
	PRINT_xX_PWR_JSON("high_alarm_threshold", sd.tx_power[HALRM]);
	PRINT_xX_PWR_JSON("low_alarm_threshold", sd.tx_power[LALRM]);
	PRINT_xX_PWR_JSON("high_warning_threshold", sd.tx_power[HWARN]);
	PRINT_xX_PWR_JSON("low_warning_threshold", sd.tx_power[LWARN]);
	close_json_object();

	open_json_object("module_temperature");
	PRINT_TEMP_JSON("high_alarm_threshold", sd.sfp_temp[HALRM]);
	PRINT_TEMP_JSON("low_alarm_threshold", sd.sfp_temp[LALRM]);
	PRINT_TEMP_JSON("high_warning_threshold", sd.sfp_temp[HWARN]);
	PRINT_TEMP_JSON("low_warning_threshold", sd.sfp_temp[LWARN]);
	close_json_object();

	open_json_object("module_voltage");
	PRINT_VCC_JSON("high_alarm_threshold", sd.sfp_voltage[HALRM]);
	PRINT_VCC_JSON("low_alarm_threshold", sd.sfp_voltage[LALRM]);
	PRINT_VCC_JSON("high_warning_threshold", sd.sfp_voltage[HWARN]);
	PRINT_VCC_JSON("low_warning_threshold", sd.sfp_voltage[LWARN]);
	close_json_object();

	open_json_object("laser_rx_power");
	PRINT_xX_PWR_JSON("high_alarm_threshold", sd.rx_power[HALRM]);
	PRINT_xX_PWR_JSON("low_alarm_threshold", sd.rx_power[LALRM]);
	PRINT_xX_PWR_JSON("high_warning_threshold", sd.rx_power[HWARN]);
	PRINT_xX_PWR_JSON("low_warning_threshold", sd.rx_power[LWARN]);
	close_json_object();
}

void sff_show_thresholds(struct sff_diags sd)
{
	PRINT_BIAS("Laser bias current high alarm threshold",
		   sd.bias_cur[HALRM]);
	PRINT_BIAS("Laser bias current low alarm threshold",
		   sd.bias_cur[LALRM]);
	PRINT_BIAS("Laser bias current high warning threshold",
		   sd.bias_cur[HWARN]);
	PRINT_BIAS("Laser bias current low warning threshold",
		   sd.bias_cur[LWARN]);

	PRINT_xX_PWR("Laser output power high alarm threshold",
		     sd.tx_power[HALRM]);
	PRINT_xX_PWR("Laser output power low alarm threshold",
		     sd.tx_power[LALRM]);
	PRINT_xX_PWR("Laser output power high warning threshold",
		     sd.tx_power[HWARN]);
	PRINT_xX_PWR("Laser output power low warning threshold",
		     sd.tx_power[LWARN]);

	PRINT_TEMP("Module temperature high alarm threshold",
		   sd.sfp_temp[HALRM]);
	PRINT_TEMP("Module temperature low alarm threshold",
		   sd.sfp_temp[LALRM]);
	PRINT_TEMP("Module temperature high warning threshold",
		   sd.sfp_temp[HWARN]);
	PRINT_TEMP("Module temperature low warning threshold",
		   sd.sfp_temp[LWARN]);

	PRINT_VCC("Module voltage high alarm threshold",
		  sd.sfp_voltage[HALRM]);
	PRINT_VCC("Module voltage low alarm threshold",
		  sd.sfp_voltage[LALRM]);
	PRINT_VCC("Module voltage high warning threshold",
		  sd.sfp_voltage[HWARN]);
	PRINT_VCC("Module voltage low warning threshold",
		  sd.sfp_voltage[LWARN]);

	PRINT_xX_PWR("Laser rx power high alarm threshold",
		     sd.rx_power[HALRM]);
	PRINT_xX_PWR("Laser rx power low alarm threshold",
		     sd.rx_power[LALRM]);
	PRINT_xX_PWR("Laser rx power high warning threshold",
		     sd.rx_power[HWARN]);
	PRINT_xX_PWR("Laser rx power low warning threshold",
		     sd.rx_power[LWARN]);
}
