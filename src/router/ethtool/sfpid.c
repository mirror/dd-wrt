/****************************************************************************
 * Support for Solarflare Solarstorm network controllers and boards
 * Copyright 2010 Solarflare Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, incorporated herein by reference.
 */

#include <stdio.h>
#include <errno.h>
#include "internal.h"
#include "module-common.h"
#include "netlink/extapi.h"

#define SFF8079_PAGE_SIZE		0x80
#define SFF8079_I2C_ADDRESS_LOW		0x50
#define SFF8079_I2C_ADDRESS_HIGH	0x51

static void sff8079_show_identifier(const __u8 *id)
{
	module_show_identifier(id, 0);
}

static void sff8079_show_ext_identifier(const __u8 *id)
{
	char description[SFF_MAX_DESC_LEN];

	if (id[1] == 0x00)
		sprintf(description, "%s",
			"GBIC not specified / not MOD_DEF compliant");
	else if (id[1] == 0x04)
		sprintf(description, "%s",
			"GBIC/SFP defined by 2-wire interface ID");
	else if (id[1] <= 0x07)
		sprintf(description, "%s %u", "GBIC compliant with MOD_DEF",
			id[1]);
	else
		sprintf(description, "%s", "unknown");

	sff_print_any_hex_field("Extended identifier", "extended_identifier",
				id[1], description);
}

static void sff8079_show_connector(const __u8 *id)
{
	module_show_connector(id, 2);
}

static void sff8079_show_transceiver(const __u8 *id)
{
	static const char *pfx = "Transceiver type";
	char value[140] = "";

	if (is_json_context()) {
		open_json_array("transceiver_codes", "");
		print_uint(PRINT_JSON, NULL, "%u", id[3]);
		print_uint(PRINT_JSON, NULL, "%u", id[4]);
		print_uint(PRINT_JSON, NULL, "%u", id[5]);
		print_uint(PRINT_JSON, NULL, "%u", id[6]);
		print_uint(PRINT_JSON, NULL, "%u", id[7]);
		print_uint(PRINT_JSON, NULL, "%u", id[8]);
		print_uint(PRINT_JSON, NULL, "%u", id[9]);
		print_uint(PRINT_JSON, NULL, "%u", id[10]);
		print_uint(PRINT_JSON, NULL, "%u", id[36]);
		close_json_array("");
	} else {
		printf("\t%-41s : 0x%02x 0x%02x 0x%02x " \
		       "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
		       "Transceiver codes", id[3], id[4], id[5], id[6],
		       id[7], id[8], id[9], id[10], id[36]);
	}
	/* 10G Ethernet Compliance Codes */
	if (id[3] & (1 << 7))
		sprintf(value, "%s",
			"10G Ethernet: 10G Base-LRM [SFF-8472 rev10.4 onwards]");
	if (id[3] & (1 << 6))
		sprintf(value, "%s", "10G Ethernet: 10G Base-LRM");
	if (id[3] & (1 << 5))
		sprintf(value, "%s", "10G Ethernet: 10G Base-LR");
	if (id[3] & (1 << 4))
		sprintf(value, "%s", "10G Ethernet: 10G Base-SR");
	/* Infiniband Compliance Codes */
	if (id[3] & (1 << 3))
		sprintf(value, "%s", "Infiniband: 1X SX");
	if (id[3] & (1 << 2))
		sprintf(value, "%s", "Infiniband: 1X LX");
	if (id[3] & (1 << 1))
		sprintf(value, "%s", "Infiniband: 1X Copper Active");
	if (id[3] & (1 << 0))
		sprintf(value, "%s", "Infiniband: 1X Copper Passive");
	/* ESCON Compliance Codes */
	if (id[4] & (1 << 7))
		sprintf(value, "%s", "ESCON: ESCON MMF, 1310nm LED");
	if (id[4] & (1 << 6))
		sprintf(value, "%s", "ESCON: ESCON SMF, 1310nm Laser");
	/* SONET Compliance Codes */
	if (id[4] & (1 << 5))
		sprintf(value, "%s", "SONET: OC-192, short reach");
	if (id[4] & (1 << 4))
		sprintf(value, "%s", "SONET: SONET reach specifier bit 1");
	if (id[4] & (1 << 3))
		sprintf(value, "%s", "SONET: SONET reach specifier bit 2");
	if (id[4] & (1 << 2))
		sprintf(value, "%s", "SONET: OC-48, long reach");
	if (id[4] & (1 << 1))
		sprintf(value, "%s", "SONET: OC-48, intermediate reach");
	if (id[4] & (1 << 0))
		sprintf(value, "%s", "SONET: OC-48, short reach");
	if (id[5] & (1 << 6))
		sprintf(value, "%s", "SONET: OC-12, single mode, long reach");
	if (id[5] & (1 << 5))
		sprintf(value, "%s", "SONET: OC-12, single mode, inter. reach");
	if (id[5] & (1 << 4))
		sprintf(value, "%s", "SONET: OC-12, short reach");
	if (id[5] & (1 << 2))
		sprintf(value, "%s", "SONET: OC-3, single mode, long reach");
	if (id[5] & (1 << 1))
		sprintf(value, "%s", "SONET: OC-3, single mode, inter. reach");
	if (id[5] & (1 << 0))
		sprintf(value, "%s", "SONET: OC-3, short reach");
	/* Ethernet Compliance Codes */
	if (id[6] & (1 << 7))
		sprintf(value, "%s", "Ethernet: BASE-PX");
	if (id[6] & (1 << 6))
		sprintf(value, "%s", "Ethernet: BASE-BX10");
	if (id[6] & (1 << 5))
		sprintf(value, "%s", "Ethernet: 100BASE-FX");
	if (id[6] & (1 << 4))
		sprintf(value, "%s", "Ethernet: 100BASE-LX/LX10");
	if (id[6] & (1 << 3))
		sprintf(value, "%s", "Ethernet: 1000BASE-T");
	if (id[6] & (1 << 2))
		sprintf(value, "%s", "Ethernet: 1000BASE-CX");
	if (id[6] & (1 << 1))
		sprintf(value, "%s", "Ethernet: 1000BASE-LX");
	if (id[6] & (1 << 0))
		sprintf(value, "%s", "Ethernet: 1000BASE-SX");
	/* Fibre Channel link length */
	if (id[7] & (1 << 7))
		sprintf(value, "%s", "FC: very long distance (V)");
	if (id[7] & (1 << 6))
		sprintf(value, "%s", "FC: short distance (S)");
	if (id[7] & (1 << 5))
		sprintf(value, "%s", "FC: intermediate distance (I)");
	if (id[7] & (1 << 4))
		sprintf(value, "%s", "FC: long distance (L)");
	if (id[7] & (1 << 3))
		sprintf(value, "%s", "FC: medium distance (M)");
	/* Fibre Channel transmitter technology */
	if (id[7] & (1 << 2))
		sprintf(value, "%s", "FC: Shortwave laser, linear Rx (SA)");
	if (id[7] & (1 << 1))
		sprintf(value, "%s", "FC: Longwave laser (LC)");
	if (id[7] & (1 << 0))
		sprintf(value, "%s", "FC: Electrical inter-enclosure (EL)");
	if (id[8] & (1 << 7))
		sprintf(value, "%s", "FC: Electrical intra-enclosure (EL)");
	if (id[8] & (1 << 6))
		sprintf(value, "%s", "FC: Shortwave laser w/o OFC (SN)");
	if (id[8] & (1 << 5))
		sprintf(value, "%s", "FC: Shortwave laser with OFC (SL)");
	if (id[8] & (1 << 4))
		sprintf(value, "%s", "FC: Longwave laser (LL)");
	if (id[8] & (1 << 3))
		sprintf(value, "%s", "Active Cable");
	if (id[8] & (1 << 2))
		sprintf(value, "%s", "Passive Cable");
	if (id[8] & (1 << 1))
		sprintf(value, "%s", "FC: Copper FC-BaseT");
	/* Fibre Channel transmission media */
	if (id[9] & (1 << 7))
		sprintf(value, "%s", "FC: Twin Axial Pair (TW)");
	if (id[9] & (1 << 6))
		sprintf(value, "%s", "FC: Twisted Pair (TP)");
	if (id[9] & (1 << 5))
		sprintf(value, "%s", "FC: Miniature Coax (MI)");
	if (id[9] & (1 << 4))
		sprintf(value, "%s", "FC: Video Coax (TV)");
	if (id[9] & (1 << 3))
		sprintf(value, "%s", "FC: Multimode, 62.5um (M6)");
	if (id[9] & (1 << 2))
		sprintf(value, "%s", "FC: Multimode, 50um (M5)");
	if (id[9] & (1 << 0))
		sprintf(value, "%s", "FC: Single Mode (SM)");
	/* Fibre Channel speed */
	if (id[10] & (1 << 7))
		sprintf(value, "%s", "FC: 1200 MBytes/sec");
	if (id[10] & (1 << 6))
		sprintf(value, "%s", "FC: 800 MBytes/sec");
	if (id[10] & (1 << 4))
		sprintf(value, "%s", "FC: 400 MBytes/sec");
	if (id[10] & (1 << 2))
		sprintf(value, "%s", "FC: 200 MBytes/sec");
	if (id[10] & (1 << 0))
		sprintf(value, "%s", "FC: 100 MBytes/sec");
	/* Extended Specification Compliance Codes from SFF-8024 */
	if (id[36] == 0x1)
		sprintf(value, "%s",
			"Extended: 100G AOC or 25GAUI C2M AOC with worst BER of 5x10^(-5)");
	if (id[36] == 0x2)
		sprintf(value, "%s", "Extended: 100G Base-SR4 or 25GBase-SR");
	if (id[36] == 0x3)
		sprintf(value, "%s", "Extended: 100G Base-LR4 or 25GBase-LR");
	if (id[36] == 0x4)
		sprintf(value, "%s", "Extended: 100G Base-ER4 or 25GBase-ER");
	if (id[36] == 0x8)
		sprintf(value, "%s",
			"Extended: 100G ACC or 25GAUI C2M ACC with worst BER of 5x10^(-5)");
	if (id[36] == 0xb)
		sprintf(value, "%s",
			"Extended: 100G Base-CR4 or 25G Base-CR CA-L");
	if (id[36] == 0xc)
		sprintf(value, "%s", "Extended: 25G Base-CR CA-S");
	if (id[36] == 0xd)
		sprintf(value, "%s", "Extended: 25G Base-CR CA-N");
	if (id[36] == 0x16)
		sprintf(value, "%s",
			"Extended: 10Gbase-T with SFI electrical interface");
	if (id[36] == 0x18)
		sprintf(value, "%s",
			"Extended: 100G AOC or 25GAUI C2M AOC with worst BER of 10^(-12)");
	if (id[36] == 0x19)
		sprintf(value, "%s",
			"Extended: 100G ACC or 25GAUI C2M ACC with worst BER of 10^(-12)");
	if (id[36] == 0x1a)
		sprintf(value, "%s",
			"Extended: 100GE-DWDM2 (DWDM transceiver using 2 wavelengths on a 1550 nm DWDM grid with a reach up to 80 km)");
	if (id[36] == 0x1b)
		sprintf(value, "%s",
			"Extended: 100G 1550nm WDM (4 wavelengths)");
	if (id[36] == 0x1c)
		sprintf(value, "%s", "Extended: 10Gbase-T Short Reach");
	if (id[36] == 0x1d)
		sprintf(value, "%s", "Extended: 5GBASE-T");
	if (id[36] == 0x1e)
		sprintf(value, "%s", "Extended: 2.5GBASE-T");
	if (id[36] == 0x1f)
		sprintf(value, "%s", "Extended: 40G SWDM4");
	if (id[36] == 0x20)
		sprintf(value, "%s", "Extended: 100G SWDM4");
	if (id[36] == 0x21)
		sprintf(value, "%s", "Extended: 100G PAM4 BiDi");
	if (id[36] == 0x22)
		sprintf(value, "%s",
			"Extended: 4WDM-10 MSA (10km version of 100G CWDM4 with same RS(528,514) FEC in host system)");
	if (id[36] == 0x23)
		sprintf(value, "%s",
			"Extended: 4WDM-20 MSA (20km version of 100GBASE-LR4 with RS(528,514) FEC in host system)");
	if (id[36] == 0x24)
		sprintf(value, "%s",
			"Extended: 4WDM-40 MSA (40km reach with APD receiver and RS(528,514) FEC in host system)");
	if (id[36] == 0x25)
		sprintf(value, "%s",
			"Extended: 100GBASE-DR (clause 140), CAUI-4 (no FEC)");
	if (id[36] == 0x26)
		sprintf(value, "%s",
			"Extended: 100G-FR or 100GBASE-FR1 (clause 140), CAUI-4 (no FEC)");
	if (id[36] == 0x27)
		sprintf(value, "%s",
			"Extended: 100G-LR or 100GBASE-LR1 (clause 140), CAUI-4 (no FEC)");
	if (id[36] == 0x30)
		sprintf(value, "%s",
			"Extended: Active Copper Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 10-6 or below");
	if (id[36] == 0x31)
		sprintf(value, "%s",
			"Extended: Active Optical Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 10-6 or below");
	if (id[36] == 0x32)
		sprintf(value, "%s",
			"Extended: Active Copper Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 2.6x10-4 for ACC, 10-5 for AUI, or below");
	if (id[36] == 0x33)
		sprintf(value, "%s",
			"Extended: Active Optical Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 2.6x10-4 for ACC, 10-5 for AUI, or below");
	if (id[36] == 0x40)
		sprintf(value, "%s",
			"Extended: 50GBASE-CR, 100GBASE-CR2, or 200GBASE-CR4");
	if (id[36] == 0x41)
		sprintf(value, "%s",
			"Extended: 50GBASE-SR, 100GBASE-SR2, or 200GBASE-SR4");
	if (id[36] == 0x42)
		sprintf(value, "%s", "Extended: 50GBASE-FR or 200GBASE-DR4");
	if (id[36] == 0x43)
		sprintf(value, "%s", "Extended: 200GBASE-FR4");
	if (id[36] == 0x44)
		sprintf(value, "%s", "Extended: 200G 1550 nm PSM4");
	if (id[36] == 0x45)
		sprintf(value, "%s", "Extended: 50GBASE-LR");
	if (id[36] == 0x46)
		sprintf(value, "%s", "Extended: 200GBASE-LR4");
	if (id[36] == 0x50)
		sprintf(value, "%s", "Extended: 64GFC EA");
	if (id[36] == 0x51)
		sprintf(value, "%s", "Extended: 64GFC SW");
	if (id[36] == 0x52)
		sprintf(value, "%s", "Extended: 64GFC LW");
	if (id[36] == 0x53)
		sprintf(value, "%s", "Extended: 128GFC EA");
	if (id[36] == 0x54)
		sprintf(value, "%s", "Extended: 128GFC SW");
	if (id[36] == 0x55)
		sprintf(value, "%s", "Extended: 128GFC LW");

	if (value[0] != '\0')
		module_print_any_string(pfx, value);
}

static void sff8079_show_encoding(const __u8 *id)
{
	sff8024_show_encoding(id, 11, ETH_MODULE_SFF_8472);
}

static void sff8079_show_rate_identifier(const __u8 *id)
{
	char description[SFF_MAX_DESC_LEN];

	switch (id[13]) {
	case 0x00:
		sprintf(description, "%s", "unspecified");
		break;
	case 0x01:
		sprintf(description, "%s", "4/2/1G Rate_Select & AS0/AS1");
		break;
	case 0x02:
		sprintf(description, "%s", "8/4/2G Rx Rate_Select only");
		break;
	case 0x03:
		sprintf(description, "%s",
			"8/4/2G Independent Rx & Tx Rate_Select");
		break;
	case 0x04:
		sprintf(description, "%s", "8/4/2G Tx Rate_Select only");
		break;
	default:
		sprintf(description, "%s", "reserved or unknown");
		break;
	}

	sff_print_any_hex_field("Rate identifier", "rate_identifier", id[13],
				description);
}

static void sff8079_show_wavelength_or_copper_compliance(const __u8 *id)
{
	char description[SFF_MAX_DESC_LEN];

	if (id[8] & (1 << 2)) {
		switch (id[60]) {
		case 0x00:
			strncpy(description, "unspecified",
				SFF_MAX_DESC_LEN);
			break;
		case 0x01:
			strncpy(description, "SFF-8431 appendix E",
				SFF_MAX_DESC_LEN);
			break;
		default:
			strncpy(description, "unknown", SFF_MAX_DESC_LEN);
			break;
		}
		strcat(description, " [SFF-8472 rev10.4 only]");
		sff_print_any_hex_field("Passive Cu cmplnce.",
					"passive_cu_cmplnce.",
					id[60], description);
	} else if (id[8] & (1 << 3)) {
		printf("\t%-41s : 0x%02x", "Active Cu cmplnce.", id[60]);
		switch (id[60]) {
		case 0x00:
			strncpy(description, "unspecified",
				SFF_MAX_DESC_LEN);
			break;
		case 0x01:
			strncpy(description, "SFF-8431 appendix E",
				SFF_MAX_DESC_LEN);
			break;
		case 0x04:
			strncpy(description, "SFF-8431 limiting",
				SFF_MAX_DESC_LEN);
			break;
		default:
			strncpy(description, "unknown", SFF_MAX_DESC_LEN);
			break;
		}
		strcat(description, " [SFF-8472 rev10.4 only]");
		sff_print_any_hex_field("Active Cu cmplnce.",
					"active_cu_cmplnce.",
					id[60], description);
	} else {
		module_print_any_uint("Laser wavelength",
				      (id[60] << 8) | id[61], "nm");
	}
}

static void sff8079_show_options(const __u8 *id)
{
	static const char *pfx = "Option";
	char value[64] = "";

	if (is_json_context()) {
		open_json_array("option_values", "");
		print_uint(PRINT_JSON, NULL, "%u", id[64]);
		print_uint(PRINT_JSON, NULL, "%u", id[65]);
		close_json_array("");
	} else {
		printf("\t%-41s : 0x%02x 0x%02x\n", "Option values", id[64],
		       id[65]);
	}
	if (id[65] & (1 << 1))
		sprintf(value, "%s", "RX_LOS implemented");
	if (id[65] & (1 << 2))
		sprintf(value, "%s", "RX_LOS implemented, inverted");
	if (id[65] & (1 << 3))
		sprintf(value, "%s", "TX_FAULT implemented");
	if (id[65] & (1 << 4))
		sprintf(value, "%s", "TX_DISABLE implemented");
	if (id[65] & (1 << 5))
		sprintf(value, "%s", "RATE_SELECT implemented");
	if (id[65] & (1 << 6))
		sprintf(value, "%s", "Tunable transmitter technology");
	if (id[65] & (1 << 7))
		sprintf(value, "%s", "Receiver decision threshold implemented");
	if (id[64] & (1 << 0))
		sprintf(value, "%s", "Linear receiver output implemented");
	if (id[64] & (1 << 1))
		sprintf(value, "%s", "Power level 2 requirement");
	if (id[64] & (1 << 2))
		sprintf(value, "%s", "Cooled transceiver implemented");
	if (id[64] & (1 << 3))
		sprintf(value, "%s", "Retimer or CDR implemented");
	if (id[64] & (1 << 4))
		sprintf(value, "%s", "Paging implemented");
	if (id[64] & (1 << 5))
		sprintf(value, "%s", "Power level 3 requirement");

	if (value[0] != '\0')
		module_print_any_string(pfx, value);
}

static void sff8079_show_all_common(const __u8 *id)
{
	sff8079_show_identifier(id);
	if (((id[0] == 0x02) || (id[0] == 0x03)) && (id[1] == 0x04)) {
		unsigned int br_nom, br_min, br_max;

		if (id[12] == 0) {
			br_nom = br_min = br_max = 0;
		} else if (id[12] == 255) {
			br_nom = id[66] * 250;
			br_max = id[67];
			br_min = id[67];
		} else {
			br_nom = id[12] * 100;
			br_max = id[66];
			br_min = id[67];
		}
		sff8079_show_ext_identifier(id);
		sff8079_show_connector(id);
		sff8079_show_transceiver(id);
		sff8079_show_encoding(id);
		module_print_any_uint("BR Nominal", br_nom, "MBd");
		sff8079_show_rate_identifier(id);
		module_show_value_with_unit(id, 14, "Length (SMF)", 1, "km");
		module_show_value_with_unit(id, 16, "Length (OM2)", 10, "m");
		module_show_value_with_unit(id, 17, "Length (OM1)", 10, "m");
		module_show_value_with_unit(id, 18,
					    "Length (Copper or Active cable)",
					    1, "m");
		module_show_value_with_unit(id, 19, "Length (OM3)", 10, "m");
		sff8079_show_wavelength_or_copper_compliance(id);
		module_show_ascii(id, 20, 35, "Vendor name");
		module_show_oui(id, 37);
		module_show_ascii(id, 40, 55, "Vendor PN");
		module_show_ascii(id, 56, 59, "Vendor rev");
		sff8079_show_options(id);
		module_print_any_uint("BR margin max", br_max, "%");
		module_print_any_uint("BR margin min", br_min, "%");
		module_show_ascii(id, 68, 83, "Vendor SN");
		module_show_ascii(id, 84, 91, "Date code");
	}
}

void sff8079_show_all_ioctl(const __u8 *id)
{
	sff8079_show_all_common(id);
}

static int sff8079_get_eeprom_page(struct cmd_context *ctx, u8 i2c_address,
				   __u8 *buf)
{
	struct ethtool_module_eeprom request = {
		.length = SFF8079_PAGE_SIZE,
		.i2c_address = i2c_address,
	};
	int ret;

	ret = nl_get_eeprom_page(ctx, &request);
	if (!ret)
		memcpy(buf, request.data, SFF8079_PAGE_SIZE);

	return ret;
}

int sff8079_show_all_nl(struct cmd_context *ctx)
{
	u8 *buf;
	int ret;

	/* The SFF-8472 parser expects a single buffer that contains the
	 * concatenation of the first 256 bytes from addresses A0h and A2h,
	 * respectively.
	 */
	buf = calloc(1, ETH_MODULE_SFF_8472_LEN);
	if (!buf)
		return -ENOMEM;

	/* Read A0h page */
	ret = sff8079_get_eeprom_page(ctx, SFF8079_I2C_ADDRESS_LOW, buf);
	if (ret)
		goto out;

	new_json_obj(ctx->json);
	open_json_object(NULL);
	sff8079_show_all_common(buf);
	close_json_object();
	delete_json_obj();

	/* Finish if A2h page is not present */
	if (!(buf[92] & (1 << 6)))
		goto out;

	/* Read A2h page */
	ret = sff8079_get_eeprom_page(ctx, SFF8079_I2C_ADDRESS_HIGH,
				      buf + ETH_MODULE_SFF_8079_LEN);
	if (ret) {
		fprintf(stderr, "Failed to read Page A2h.\n");
		goto out;
	}

	sff8472_show_all(buf);
out:
	free(buf);

	return ret;
}
