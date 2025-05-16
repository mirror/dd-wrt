/*
 * sff-common.h: Implements SFF-8024 Rev 4.0 i.e. Specifcation
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
 *   and SFF-8024 specs (ftp://ftp.seagate.com/pub/sff/SFF-8024.PDF)
 *   by SFF Committee.
 */

#ifndef SFF_COMMON_H__
#define SFF_COMMON_H__

#include <stdio.h>
#include "internal.h"

#define SFF_MAX_DESC_LEN   120
#define SFF_MAX_FIELD_LEN  64

/* Revision compliance */
#define  SFF8636_REV_UNSPECIFIED		0x00
#define  SFF8636_REV_8436_48			0x01
#define  SFF8636_REV_8436_8636			0x02
#define  SFF8636_REV_8636_13			0x03
#define  SFF8636_REV_8636_14			0x04
#define  SFF8636_REV_8636_15			0x05
#define  SFF8636_REV_8636_20			0x06
#define  SFF8636_REV_8636_27			0x07

/* ENCODING Values */
#define  SFF8024_ENCODING_UNSPEC		0x00
#define  SFF8024_ENCODING_8B10B			0x01
#define  SFF8024_ENCODING_4B5B			0x02
#define  SFF8024_ENCODING_NRZ			0x03
/*
 * Value: 04h
 * SFF-8472      - Manchester
 * SFF-8436/8636 - SONET Scrambled
 */
#define  SFF8024_ENCODING_4h			0x04
/*
 * Value: 05h
 * SFF-8472      - SONET Scrambled
 * SFF-8436/8636 - 64B/66B
 */
#define  SFF8024_ENCODING_5h			0x05
/*
 * Value: 06h
 * SFF-8472      - 64B/66B
 * SFF-8436/8636 - Manchester
 */
#define  SFF8024_ENCODING_6h			0x06
#define  SFF8024_ENCODING_256B			0x07
#define  SFF8024_ENCODING_PAM4			0x08

/* Most common case: 16-bit unsigned integer in a certain unit */
#define OFFSET_TO_U16_PTR(ptr, offset) (ptr[offset] << 8 | ptr[(offset) + 1])
#define OFFSET_TO_U16(offset) OFFSET_TO_U16_PTR(id, offset)

# define PRINT_xX_PWR(string, var)                             \
		printf("\t%-41s : %.4f mW / %.2f dBm\n", (string),         \
		      (double)((var) / 10000.),                           \
		       convert_mw_to_dbm((double)((var) / 10000.)))

#define PRINT_xX_PWR_JSON(string, var)				\
		print_float(PRINT_JSON, string, "%.2f",		\
			    (double)((var) / 10000.))

#define PRINT_xX_PWR_ALL(string, json_string, var)		\
		is_json_context() ?				\
		PRINT_xX_PWR_JSON(json_string, var) :		\
		PRINT_xX_PWR(string, var)

#define PRINT_BIAS(string, bias_cur)                             \
		printf("\t%-41s : %.3f mA\n", (string),                       \
		      (double)(bias_cur / 500.))

#define PRINT_BIAS_JSON(string, bias_cur)			\
		print_float(PRINT_JSON, string, "%.3f",		\
			    (double)(bias_cur / 500.))

#define PRINT_BIAS_ALL(string, json_string, bias_cur)		\
		is_json_context() ?				\
		PRINT_BIAS_JSON(json_string, bias_cur) :	\
		PRINT_BIAS(string, bias_cur)

#define PRINT_TEMP(string, temp)                                   \
		printf("\t%-41s : %.2f degrees C / %.2f degrees F\n", \
		      (string), (double)(temp / 256.),                \
		      (double)(temp / 256. * 1.8 + 32.))

#define PRINT_TEMP_JSON(string, temp)				\
		print_float(PRINT_JSON, string, "%.2f", (double)(temp / 256.))

#define PRINT_TEMP_ALL(string, json_string, temp)		\
		is_json_context() ?				\
		PRINT_TEMP_JSON(json_string, temp) : PRINT_TEMP(string, temp)

#define PRINT_VCC(string, sfp_voltage)          \
		printf("\t%-41s : %.4f V\n", (string),       \
		      (double)(sfp_voltage / 10000.))

#define PRINT_VCC_JSON(string, sfp_voltage)			\
		print_float(PRINT_JSON, string, "%.4f",		\
			    (double)(sfp_voltage / 10000.))

#define PRINT_VCC_ALL(string, json_string, sfp_voltage)		\
		is_json_context() ? 				\
		PRINT_VCC_JSON(json_string, sfp_voltage) :	\
		PRINT_VCC(string, sfp_voltage)

# define PRINT_xX_THRESH_PWR(string, var, index)                       \
		PRINT_xX_PWR(string, (var)[(index)])

/* Channel Monitoring Fields */
struct sff_channel_diags {
	__u16 bias_cur;      /* Measured bias current in 2uA units */
	__u16 rx_power;      /* Measured RX Power */
	__u16 tx_power;      /* Measured TX Power */
};

/* Module Monitoring Fields */
struct sff_diags {

#define MAX_CHANNEL_NUM 32
#define LWARN 0
#define HWARN 1
#define LALRM 2
#define HALRM 3
#define MCURR 4

	/* Supports DOM */
	__u8 supports_dom;
	/* Supports alarm/warning thold */
	__u8 supports_alarms;
	/* RX Power: 0 = OMA, 1 = Average power */
	__u8  rx_power_type;
	/* TX Power: 0 = Not supported, 1 = Average power */
	__u8  tx_power_type;

	__u8 calibrated_ext;    /* Is externally calibrated */
	/* [5] tables are low/high warn, low/high alarm, current */
	/* SFP voltage in 0.1mV units */
	__u16 sfp_voltage[5];
	/* SFP Temp in 16-bit signed 1/256 Celcius */
	__s16 sfp_temp[5];
	/* Measured bias current in 2uA units */
	__u16 bias_cur[5];
	/* Measured TX Power */
	__u16 tx_power[5];
	/* Measured RX Power */
	__u16 rx_power[5];
	struct sff_channel_diags scd[MAX_CHANNEL_NUM];
};

double convert_mw_to_dbm(double mw);
void sff_print_any_hex_field(const char *field_name,
			     const char *json_field_name, u8 value,
			     const char *desc);
void sff_show_thresholds(struct sff_diags sd);
void sff_show_thresholds_json(struct sff_diags sd);

void sff8024_show_encoding(const __u8 *id, int encoding_offset, int sff_type);

#endif /* SFF_COMMON_H__ */
