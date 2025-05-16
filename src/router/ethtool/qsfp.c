/*
 * qsfp.c: Implements SFF-8636 based QSFP+/QSFP28 Diagnostics Memory map.
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
 *  Vidya Ravipati <vidya@cumulusnetworks.com>
 *   This implementation is loosely based on current SFP parser
 *   and SFF-8636 spec Rev 2.7 (ftp://ftp.seagate.com/pub/sff/SFF-8636.PDF)
 *   by SFF Committee.
 */

/*
 *	Description:
 *	a) The register/memory layout is up to 5 128 byte pages defined by
 *		a "pages valid" register and switched via a "page select"
 *		register. Memory of 256 bytes can be memory mapped at a time
 *		according to SFF 8636.
 *	b) SFF 8636 based 640 bytes memory layout is presented for parser
 *
 *           SFF 8636 based QSFP Memory Map
 *
 *           2-Wire Serial Address: 1010000x
 *
 *           Lower Page 00h (128 bytes)
 *           ======================
 *           |                     |
 *           |Page Select Byte(127)|
 *           ======================
 *                    |
 *                    V
 *	     ----------------------------------------
 *	    |             |            |             |
 *	    V             V            V             V
 *	 ----------   ----------   ---------    ------------
 *	| Upper    | | Upper    | | Upper    | | Upper      |
 *	| Page 00h | | Page 01h | | Page 02h | | Page 03h   |
 *	|          | |(Optional)| |(Optional)| | (Optional) |
 *	|          | |          | |          | |            |
 *	|          | |          | |          | |            |
 *	|    ID    | |   AST    | |  User    | |  For       |
 *	|  Fields  | |  Table   | | EEPROM   | |  Cable     |
 *	|          | |          | | Data     | | Assemblies |
 *	|          | |          | |          | |            |
 *	|          | |          | |          | |            |
 *	-----------  -----------   ----------  --------------
 *
 *
 **/
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include "internal.h"
#include "module-common.h"
#include "qsfp.h"
#include "cmis.h"
#include "netlink/extapi.h"

struct sff8636_memory_map {
	const __u8 *lower_memory;
	const __u8 *upper_memory[4];
#define page_00h upper_memory[0x0]
#define page_03h upper_memory[0x3]
};

#define SFF8636_PAGE_SIZE	0x80
#define SFF8636_I2C_ADDRESS	0x50
#define SFF8636_MAX_CHANNEL_NUM	4

#define MAX_DESC_SIZE	42

static void sff8636_show_identifier(const struct sff8636_memory_map *map)
{
	module_show_identifier(map->lower_memory, SFF8636_ID_OFFSET);
}

static void sff8636_show_ext_identifier(const struct sff8636_memory_map *map)
{
	static const char *pfx =
		"\tExtended identifier description           :";
	char description[64];

	if (is_json_context()) {
		open_json_object("extended_identifier");
		print_uint(PRINT_JSON, "value", "%u",
			   map->page_00h[SFF8636_EXT_ID_OFFSET]);
	} else {
		printf("\t%-41s : 0x%02x\n", "Extended identifier",
		       map->page_00h[SFF8636_EXT_ID_OFFSET]);
	}

	open_json_array("description", "");
	switch (map->page_00h[SFF8636_EXT_ID_OFFSET] &
		SFF8636_EXT_ID_PWR_CLASS_MASK) {
	case SFF8636_EXT_ID_PWR_CLASS_1:
		strncpy(description, "1.5W max. Power consumption", 64);
		break;
	case SFF8636_EXT_ID_PWR_CLASS_2:
		strncpy(description, "1.5W max. Power consumption", 64);
		break;
	case SFF8636_EXT_ID_PWR_CLASS_3:
		strncpy(description, "2.5W max. Power consumption", 64);
		break;
	case SFF8636_EXT_ID_PWR_CLASS_4:
		strncpy(description, "3.5W max. Power consumption", 64);
		break;
	}

	if (is_json_context())
		print_string(PRINT_JSON, NULL, "%s", description);
	else
		printf("%s %s\n", pfx, description);

	if (map->page_00h[SFF8636_EXT_ID_OFFSET] & SFF8636_EXT_ID_CDR_TX_MASK)
		strncpy(description, "CDR present in TX,", 64);
	else
		strncpy(description, "No CDR in TX,", 64);

	if (map->page_00h[SFF8636_EXT_ID_OFFSET] & SFF8636_EXT_ID_CDR_RX_MASK)
		strcat(description, " CDR present in RX");
	else
		strcat(description, " No CDR in RX");

	if (is_json_context())
		print_string(PRINT_JSON, NULL, "%s", description);
	else
		printf("%s %s\n", pfx, description);

	switch (map->page_00h[SFF8636_EXT_ID_OFFSET] &
		SFF8636_EXT_ID_EPWR_CLASS_MASK) {
	case SFF8636_EXT_ID_PWR_CLASS_LEGACY:
		strncpy(description, "", 64);
		break;
	case SFF8636_EXT_ID_PWR_CLASS_5:
		strncpy(description, "4.0W max. Power consumption,", 64);
		break;
	case SFF8636_EXT_ID_PWR_CLASS_6:
		strncpy(description, "4.5W max. Power consumption,", 64);
		break;
	case SFF8636_EXT_ID_PWR_CLASS_7:
		strncpy(description, "5.0W max. Power consumption,", 64);
		break;
	}

	if (map->lower_memory[SFF8636_PWR_MODE_OFFSET] &
	    SFF8636_HIGH_PWR_ENABLE)
		strcat(description, " High Power Class (> 3.5 W) enabled");
	else
		strcat(description,
		       " High Power Class (> 3.5 W) not enabled");

	if (is_json_context())
		print_string(PRINT_JSON, NULL, "%s", description);
	else
		printf("%s %s\n", pfx, description);

	close_json_array("");
	close_json_object();

	bool value = map->lower_memory[SFF8636_PWR_MODE_OFFSET] &
			SFF8636_LOW_PWR_SET;
	module_print_any_bool("Power set", "power_set", value, ONOFF(value));
	value = map->lower_memory[SFF8636_PWR_MODE_OFFSET] &
			SFF8636_PWR_OVERRIDE;
	module_print_any_bool("Power override", "power_override", value,
			      ONOFF(value));
}

static void sff8636_show_connector(const struct sff8636_memory_map *map)
{
	module_show_connector(map->page_00h, SFF8636_CTOR_OFFSET);
}

static void sff8636_show_transceiver(const struct sff8636_memory_map *map)
{
	static const char *pfx = "Transceiver type";
	char value[140] = "";

	if (is_json_context()) {
		open_json_array("transceiver_codes", "");
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_ETHERNET_COMP_OFFSET]);
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_SONET_COMP_OFFSET]);
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_SAS_COMP_OFFSET]);
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_GIGE_COMP_OFFSET]);
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_FC_LEN_OFFSET]);
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_FC_TECH_OFFSET]);
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET]);
		print_uint(PRINT_JSON, NULL, "%u",
			   map->page_00h[SFF8636_FC_SPEED_OFFSET]);
		close_json_array("");
	} else {
		printf("\t%-41s : 0x%02x 0x%02x 0x%02x " \
		       "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
		       "Transceiver codes",
		       map->page_00h[SFF8636_ETHERNET_COMP_OFFSET],
		       map->page_00h[SFF8636_SONET_COMP_OFFSET],
		       map->page_00h[SFF8636_SAS_COMP_OFFSET],
		       map->page_00h[SFF8636_GIGE_COMP_OFFSET],
		       map->page_00h[SFF8636_FC_LEN_OFFSET],
		       map->page_00h[SFF8636_FC_TECH_OFFSET],
		       map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET],
		       map->page_00h[SFF8636_FC_SPEED_OFFSET]);
	}

	/* 10G/40G Ethernet Compliance Codes */
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_10G_LRM)
		sprintf(value, "%s", "10G Ethernet: 10G Base-LRM");
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_10G_LR)
		sprintf(value, "%s", "10G Ethernet: 10G Base-LR");
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_10G_SR)
		sprintf(value, "%s", "10G Ethernet: 10G Base-SR");
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_40G_CR4)
		sprintf(value, "%s", "40G Ethernet: 40G Base-CR4");
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_40G_SR4)
		sprintf(value, "%s", "40G Ethernet: 40G Base-SR4");
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_40G_LR4)
		sprintf(value, "%s", "40G Ethernet: 40G Base-LR4");
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_40G_ACTIVE)
		sprintf(value, "%s", "40G Ethernet: 40G Active Cable (XLPPI)");
	/* Extended Specification Compliance Codes from SFF-8024 */
	if (map->page_00h[SFF8636_ETHERNET_COMP_OFFSET] &
	    SFF8636_ETHERNET_RSRVD) {
		switch (map->page_00h[SFF8636_OPTION_1_OFFSET]) {
		case SFF8636_ETHERNET_UNSPECIFIED:
			sprintf(value, "%s", "(reserved or unknown)");
			break;
		case SFF8636_ETHERNET_100G_AOC:
			sprintf(value, "%s",
				"100G Ethernet: 100G AOC or 25GAUI C2M AOC with worst BER of 5x10^(-5)");
			break;
		case SFF8636_ETHERNET_100G_SR4:
			sprintf(value, "%s",
				"100G Ethernet: 100G Base-SR4 or 25GBase-SR");
			break;
		case SFF8636_ETHERNET_100G_LR4:
			sprintf(value, "%s", "100G Ethernet: 100G Base-LR4");
			break;
		case SFF8636_ETHERNET_100G_ER4:
			sprintf(value, "%s", "100G Ethernet: 100G Base-ER4");
			break;
		case SFF8636_ETHERNET_100G_SR10:
			sprintf(value, "%s", "100G Ethernet: 100G Base-SR10");
			break;
		case SFF8636_ETHERNET_100G_CWDM4_FEC:
			sprintf(value, "%s",
				"100G Ethernet: 100G CWDM4 MSA with FEC");
			break;
		case SFF8636_ETHERNET_100G_PSM4:
			sprintf(value, "%s",
				"100G Ethernet: 100G PSM4 Parallel SMF");
			break;
		case SFF8636_ETHERNET_100G_ACC:
			sprintf(value, "%s",
				"100G Ethernet: 100G ACC or 25GAUI C2M ACC with worst BER of 5x10^(-5)");
			break;
		case SFF8636_ETHERNET_100G_CWDM4_NO_FEC:
			sprintf(value, "%s",
				"100G Ethernet: 100G CWDM4 MSA without FEC");
			break;
		case SFF8636_ETHERNET_100G_RSVD1:
			sprintf(value, "%s", "(reserved or unknown)");
			break;
		case SFF8636_ETHERNET_100G_CR4:
			sprintf(value, "%s",
				"100G Ethernet: 100G Base-CR4 or 25G Base-CR CA-L");
			break;
		case SFF8636_ETHERNET_25G_CR_CA_S:
			sprintf(value, "%s", "25G Ethernet: 25G Base-CR CA-S");
			break;
		case SFF8636_ETHERNET_25G_CR_CA_N:
			sprintf(value, "%s", "25G Ethernet: 25G Base-CR CA-N");
			break;
		case SFF8636_ETHERNET_40G_ER4:
			sprintf(value, "%s", "40G Ethernet: 40G Base-ER4");
			break;
		case SFF8636_ETHERNET_4X10_SR:
			sprintf(value, "%s", "4x10G Ethernet: 10G Base-SR");
			break;
		case SFF8636_ETHERNET_40G_PSM4:
			sprintf(value, "%s",
				"40G Ethernet: 40G PSM4 Parallel SMF");
			break;
		case SFF8636_ETHERNET_G959_P1I1_2D1:
			sprintf(value, "%s",
				"Ethernet: G959.1 profile P1I1-2D1 (10709 MBd, 2km, 1310nm SM)");
			break;
		case SFF8636_ETHERNET_G959_P1S1_2D2:
			sprintf(value, "%s",
				"Ethernet: G959.1 profile P1S1-2D2 (10709 MBd, 40km, 1550nm SM)");
			break;
		case SFF8636_ETHERNET_G959_P1L1_2D2:
			sprintf(value, "%s",
				"Ethernet: G959.1 profile P1L1-2D2 (10709 MBd, 80km, 1550nm SM)");
			break;
		case SFF8636_ETHERNET_10GT_SFI:
			sprintf(value, "%s",
				"10G Ethernet: 10G Base-T with SFI electrical interface");
			break;
		case SFF8636_ETHERNET_100G_CLR4:
			sprintf(value, "%s", "100G Ethernet: 100G CLR4");
			break;
		case SFF8636_ETHERNET_100G_AOC2:
			sprintf(value, "%s",
				"100G Ethernet: 100G AOC or 25GAUI C2M AOC with worst BER of 10^(-12)");
			break;
		case SFF8636_ETHERNET_100G_ACC2:
			sprintf(value, "%s",
				"100G Ethernet: 100G ACC or 25GAUI C2M ACC with worst BER of 10^(-12)");
			break;
		case SFF8636_ETHERNET_100GE_DWDM2:
			sprintf(value, "%s",
				"100GE-DWDM2 (DWDM transceiver using 2 wavelengths on a 1550 nm DWDM grid with a reach up to 80 km)");
			break;
		case SFF8636_ETHERNET_100G_1550NM_WDM:
			sprintf(value, "%s", "100G 1550nm WDM (4 wavelengths)");
			break;
		case SFF8636_ETHERNET_10G_BASET_SR:
			sprintf(value, "%s",
				"10GBASE-T Short Reach (30 meters)");
			break;
		case SFF8636_ETHERNET_5G_BASET:
			sprintf(value, "%s", "5GBASE-T");
			break;
		case SFF8636_ETHERNET_2HALFG_BASET:
			sprintf(value, "%s", "2.5GBASE-T");
			break;
		case SFF8636_ETHERNET_40G_SWDM4:
			sprintf(value, "%s", "40G SWDM4");
			break;
		case SFF8636_ETHERNET_100G_SWDM4:
			sprintf(value, "%s", "100G SWDM4");
			break;
		case SFF8636_ETHERNET_100G_PAM4_BIDI:
			sprintf(value, "%s", "100G PAM4 BiDi");
			break;
		case SFF8636_ETHERNET_4WDM10_MSA:
			sprintf(value, "%s",
				"4WDM-10 MSA (10km version of 100G CWDM4 with same RS(528,514) FEC in host system)");
			break;
		case SFF8636_ETHERNET_4WDM20_MSA:
			sprintf(value, "%s", "4WDM-20 MSA (20km version of 100GBASE-LR4 with RS(528,514) FEC in host system)");
			break;
		case SFF8636_ETHERNET_4WDM40_MSA:
			sprintf(value, "%s",
				"4WDM-40 MSA (40km reach with APD receiver and RS(528,514) FEC in host system)");
			break;
		case SFF8636_ETHERNET_100G_DR:
			sprintf(value, "%s",
				"100GBASE-DR (clause 140), CAUI-4 (no FEC)");
			break;
		case SFF8636_ETHERNET_100G_FR_NOFEC:
			sprintf(value, "%s",
				"100G-FR or 100GBASE-FR1 (clause 140), CAUI-4 (no FEC)");
			break;
		case SFF8636_ETHERNET_100G_LR_NOFEC:
			sprintf(value, "%s",
				"100G-LR or 100GBASE-LR1 (clause 140), CAUI-4 (no FEC)");
			break;
		case SFF8636_ETHERNET_200G_ACC1:
			sprintf(value, "%s",
				"Active Copper Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 10-6 or below");
			break;
		case SFF8636_ETHERNET_200G_AOC1:
			sprintf(value, "%s",
				"Active Optical Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 10-6 or below");
			break;
		case SFF8636_ETHERNET_200G_ACC2:
			sprintf(value, "%s",
				"Active Copper Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 2.6x10-4 for ACC, 10-5 for AUI, or below");
			break;
		case SFF8636_ETHERNET_200G_A0C2:
			sprintf(value, "%s",
				"Active Optical Cable with 50GAUI, 100GAUI-2 or 200GAUI-4 C2M. Providing a worst BER of 2.6x10-4 for ACC, 10-5 for AUI, or below");
			break;
		case SFF8636_ETHERNET_200G_CR4:
			sprintf(value, "%s",
				"50GBASE-CR, 100GBASE-CR2, or 200GBASE-CR4");
			break;
		case SFF8636_ETHERNET_200G_SR4:
			sprintf(value, "%s",
				"50GBASE-SR, 100GBASE-SR2, or 200GBASE-SR4");
			break;
		case SFF8636_ETHERNET_200G_DR4:
			sprintf(value, "%s", "50GBASE-FR or 200GBASE-DR4");
			break;
		case SFF8636_ETHERNET_200G_FR4:
			sprintf(value, "%s", "200GBASE-FR4");
			break;
		case SFF8636_ETHERNET_200G_PSM4:
			sprintf(value, "%s", "200G 1550 nm PSM4");
			break;
		case SFF8636_ETHERNET_50G_LR:
			sprintf(value, "%s", "50GBASE-LR");
			break;
		case SFF8636_ETHERNET_200G_LR4:
			sprintf(value, "%s", "200GBASE-LR4");
			break;
		case SFF8636_ETHERNET_64G_EA:
			sprintf(value, "%s", "64GFC EA");
			break;
		case SFF8636_ETHERNET_64G_SW:
			sprintf(value, "%s", "64GFC SW");
			break;
		case SFF8636_ETHERNET_64G_LW:
			sprintf(value, "%s", "64GFC LW");
			break;
		case SFF8636_ETHERNET_128FC_EA:
			sprintf(value, "%s", "128GFC EA");
			break;
		case SFF8636_ETHERNET_128FC_SW:
			sprintf(value, "%s", "128GFC SW");
			break;
		case SFF8636_ETHERNET_128FC_LW:
			sprintf(value, "%s", "128GFC LW");
			break;
		default:
			sprintf(value, "%s", "(reserved or unknown)");
			break;
		}
	}

	/* SONET Compliance Codes */
	if (map->page_00h[SFF8636_SONET_COMP_OFFSET] &
	    (SFF8636_SONET_40G_OTN))
		sprintf(value, "%s", "40G OTN (OTU3B/OTU3C)");
	if (map->page_00h[SFF8636_SONET_COMP_OFFSET] & (SFF8636_SONET_OC48_LR))
		sprintf(value, "%s", "SONET: OC-48, long reach");
	if (map->page_00h[SFF8636_SONET_COMP_OFFSET] & (SFF8636_SONET_OC48_IR))
		sprintf(value, "%s", "SONET: OC-48, intermediate reach");
	if (map->page_00h[SFF8636_SONET_COMP_OFFSET] & (SFF8636_SONET_OC48_SR))
		sprintf(value, "%s", "SONET: OC-48, short reach");

	/* SAS/SATA Compliance Codes */
	if (map->page_00h[SFF8636_SAS_COMP_OFFSET] & (SFF8636_SAS_6G))
		sprintf(value, "%s", "SAS 6.0G");
	if (map->page_00h[SFF8636_SAS_COMP_OFFSET] & (SFF8636_SAS_3G))
		sprintf(value, "%s", "SAS 3.0G");

	/* Ethernet Compliance Codes */
	if (map->page_00h[SFF8636_GIGE_COMP_OFFSET] & SFF8636_GIGE_1000_BASE_T)
		sprintf(value, "%s", "Ethernet: 1000BASE-T");
	if (map->page_00h[SFF8636_GIGE_COMP_OFFSET] & SFF8636_GIGE_1000_BASE_CX)
		sprintf(value, "%s", "Ethernet: 1000BASE-CX");
	if (map->page_00h[SFF8636_GIGE_COMP_OFFSET] & SFF8636_GIGE_1000_BASE_LX)
		sprintf(value, "%s", "Ethernet: 1000BASE-LX");
	if (map->page_00h[SFF8636_GIGE_COMP_OFFSET] & SFF8636_GIGE_1000_BASE_SX)
		sprintf(value, "%s", "Ethernet: 1000BASE-SX");

	/* Fibre Channel link length */
	if (map->page_00h[SFF8636_FC_LEN_OFFSET] & SFF8636_FC_LEN_VERY_LONG)
		sprintf(value, "%s", "FC: very long distance (V)");
	if (map->page_00h[SFF8636_FC_LEN_OFFSET] & SFF8636_FC_LEN_SHORT)
		sprintf(value, "%s", "FC: short distance (S)");
	if (map->page_00h[SFF8636_FC_LEN_OFFSET] & SFF8636_FC_LEN_INT)
		sprintf(value, "%s", "FC: intermediate distance (I)");
	if (map->page_00h[SFF8636_FC_LEN_OFFSET] & SFF8636_FC_LEN_LONG)
		sprintf(value, "%s", "FC: long distance (L)");
	if (map->page_00h[SFF8636_FC_LEN_OFFSET] & SFF8636_FC_LEN_MED)
		sprintf(value, "%s", "FC: medium distance (M)");

	/* Fibre Channel transmitter technology */
	if (map->page_00h[SFF8636_FC_LEN_OFFSET] & SFF8636_FC_TECH_LONG_LC)
		sprintf(value, "%s", "FC: Longwave laser (LC)");
	if (map->page_00h[SFF8636_FC_LEN_OFFSET] & SFF8636_FC_TECH_ELEC_INTER)
		sprintf(value, "%s", "FC: Electrical inter-enclosure (EL)");
	if (map->page_00h[SFF8636_FC_TECH_OFFSET] & SFF8636_FC_TECH_ELEC_INTRA)
		sprintf(value, "%s", "FC: Electrical intra-enclosure (EL)");
	if (map->page_00h[SFF8636_FC_TECH_OFFSET] &
	    SFF8636_FC_TECH_SHORT_WO_OFC)
		sprintf(value, "%s", "FC: Shortwave laser w/o OFC (SN)");
	if (map->page_00h[SFF8636_FC_TECH_OFFSET] & SFF8636_FC_TECH_SHORT_W_OFC)
		sprintf(value, "%s", "FC: Shortwave laser with OFC (SL)");
	if (map->page_00h[SFF8636_FC_TECH_OFFSET] & SFF8636_FC_TECH_LONG_LL)
		sprintf(value, "%s", "FC: Longwave laser (LL)");

	/* Fibre Channel transmission media */
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_TW)
		sprintf(value, "%s", "FC: Twin Axial Pair (TW)");
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_TP)
		sprintf(value, "%s", "FC: Twisted Pair (TP)");
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_MI)
		sprintf(value, "%s", "FC: Miniature Coax (MI)");
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_TV)
		sprintf(value, "%s", "FC: Video Coax (TV)");
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_M6)
		sprintf(value, "%s", "FC: Multimode, 62.5m (M6)");
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_M5)
		sprintf(value, "%s", "FC: Multimode, 50m (M5)");
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_OM3)
		sprintf(value, "%s", "FC: Multimode, 50um (OM3)");
	if (map->page_00h[SFF8636_FC_TRANS_MEDIA_OFFSET] &
	    SFF8636_FC_TRANS_MEDIA_SM)
		sprintf(value, "%s", "FC: Single Mode (SM)");

	/* Fibre Channel speed */
	if (map->page_00h[SFF8636_FC_SPEED_OFFSET] & SFF8636_FC_SPEED_1200_MBPS)
		sprintf(value, "%s", "FC: 1200 MBytes/sec");
	if (map->page_00h[SFF8636_FC_SPEED_OFFSET] & SFF8636_FC_SPEED_800_MBPS)
		sprintf(value, "%s", "FC: 800 MBytes/sec");
	if (map->page_00h[SFF8636_FC_SPEED_OFFSET] & SFF8636_FC_SPEED_1600_MBPS)
		sprintf(value, "%s", "FC: 1600 MBytes/sec");
	if (map->page_00h[SFF8636_FC_SPEED_OFFSET] & SFF8636_FC_SPEED_400_MBPS)
		sprintf(value, "%s", "FC: 400 MBytes/sec");
	if (map->page_00h[SFF8636_FC_SPEED_OFFSET] & SFF8636_FC_SPEED_200_MBPS)
		sprintf(value, "%s", "FC: 200 MBytes/sec");
	if (map->page_00h[SFF8636_FC_SPEED_OFFSET] & SFF8636_FC_SPEED_100_MBPS)
		sprintf(value, "%s", "FC: 100 MBytes/sec");

	module_print_any_string(pfx, value);
}

static void sff8636_show_encoding(const struct sff8636_memory_map *map)
{
	sff8024_show_encoding(map->page_00h, SFF8636_ENCODING_OFFSET,
			      ETH_MODULE_SFF_8636);
}

static void sff8636_show_rate_identifier(const struct sff8636_memory_map *map)
{
	/* TODO: Need to fix rate select logic */
	sff_print_any_hex_field("Rate identifier", "rate_identifier",
				map->page_00h[SFF8636_EXT_RS_OFFSET], NULL);


}

static void
sff8636_show_wavelength_or_copper_compliance(const struct sff8636_memory_map *map)
{
	u16 value = map->page_00h[SFF8636_DEVICE_TECH_OFFSET] &
			SFF8636_TRANS_TECH_MASK;

	module_show_mit_compliance(value);

	if (value >= SFF8636_TRANS_COPPER_PAS_UNEQUAL) {
		module_print_any_uint("Attenuation at 2.5GHz",
				      map->page_00h[SFF8636_WAVELEN_HIGH_BYTE_OFFSET],
				      "db");
		module_print_any_uint("Attenuation at 5.0GHz",
				      map->page_00h[SFF8636_WAVELEN_LOW_BYTE_OFFSET],
				      "db");
		module_print_any_uint("Attenuation at 7.0GHz",
				      map->page_00h[SFF8636_WAVELEN_HIGH_BYTE_OFFSET],
				      "db");
		module_print_any_uint("Attenuation at 12.9GHz",
				      map->page_00h[SFF8636_WAVELEN_LOW_BYTE_OFFSET],
				      "db");
	} else {
		module_print_any_float("Laser wavelength",
				       (((map->page_00h[SFF8636_WAVELEN_HIGH_BYTE_OFFSET] << 8) |
					map->page_00h[SFF8636_WAVELEN_LOW_BYTE_OFFSET]) * 0.05),
				       "nm");
		module_print_any_float("Laser wavelength tolerance",
				       (((map->page_00h[SFF8636_WAVE_TOL_HIGH_BYTE_OFFSET] << 8) |
					map->page_00h[SFF8636_WAVE_TOL_LOW_BYTE_OFFSET]) * 0.05),
				       "nm");
	}
}

static void sff8636_show_revision_compliance(const __u8 *id, int rev_offset)
{
	const char *pfx = "Revision Compliance";
	char value[64] = "";

	switch (id[rev_offset]) {
	case SFF8636_REV_UNSPECIFIED:
		sprintf(value, "%s", "Revision not specified");
		break;
	case SFF8636_REV_8436_48:
		sprintf(value, "%s", "SFF-8436 Rev 4.8 or earlier");
		break;
	case SFF8636_REV_8436_8636:
		sprintf(value, "%s", "SFF-8436 Rev 4.8 or earlier");
		break;
	case SFF8636_REV_8636_13:
		sprintf(value, "%s", "SFF-8636 Rev 1.3 or earlier");
		break;
	case SFF8636_REV_8636_14:
		sprintf(value, "%s", "SFF-8636 Rev 1.4");
		break;
	case SFF8636_REV_8636_15:
		sprintf(value, "%s", "SFF-8636 Rev 1.5");
		break;
	case SFF8636_REV_8636_20:
		sprintf(value, "%s", "SFF-8636 Rev 2.0");
		break;
	case SFF8636_REV_8636_27:
		sprintf(value, "%s", "SFF-8636 Rev 2.5/2.6/2.7");
		break;
	default:
		sprintf(value, "%s", "Unallocated");
		break;
	}
	module_print_any_string(pfx, value);
}

/*
 * 2-byte internal temperature conversions:
 * First byte is a signed 8-bit integer, which is the temp decimal part
 * Second byte are 1/256th of degree, which are added to the dec part.
 */
#define SFF8636_OFFSET_TO_TEMP(offset) ((__s16)OFFSET_TO_U16(offset))

static void sff8636_dom_parse(const struct sff8636_memory_map *map,
			      struct sff_diags *sd)
{
	const __u8 *id = map->lower_memory;
	int i = 0;

	/* Monitoring Thresholds for Alarms and Warnings */
	sd->sfp_voltage[MCURR] = OFFSET_TO_U16_PTR(id, SFF8636_VCC_CURR);
	sd->sfp_temp[MCURR] = SFF8636_OFFSET_TO_TEMP(SFF8636_TEMP_CURR);

	if (!map->page_03h)
		goto out;

	sd->sfp_voltage[HALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						   SFF8636_VCC_HALRM);
	sd->sfp_voltage[LALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						   SFF8636_VCC_LALRM);
	sd->sfp_voltage[HWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						   SFF8636_VCC_HWARN);
	sd->sfp_voltage[LWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						   SFF8636_VCC_LWARN);

	sd->sfp_temp[HALRM] = (__s16)OFFSET_TO_U16_PTR(map->page_03h,
						       SFF8636_TEMP_HALRM);
	sd->sfp_temp[LALRM] = (__s16)OFFSET_TO_U16_PTR(map->page_03h,
						       SFF8636_TEMP_LALRM);
	sd->sfp_temp[HWARN] = (__s16)OFFSET_TO_U16_PTR(map->page_03h,
						       SFF8636_TEMP_HWARN);
	sd->sfp_temp[LWARN] = (__s16)OFFSET_TO_U16_PTR(map->page_03h,
						       SFF8636_TEMP_LWARN);

	sd->bias_cur[HALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_BIAS_HALRM);
	sd->bias_cur[LALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_BIAS_LALRM);
	sd->bias_cur[HWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_BIAS_HWARN);
	sd->bias_cur[LWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_BIAS_LWARN);

	sd->tx_power[HALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_PWR_HALRM);
	sd->tx_power[LALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_PWR_LALRM);
	sd->tx_power[HWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_PWR_HWARN);
	sd->tx_power[LWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_TX_PWR_LWARN);

	sd->rx_power[HALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_RX_PWR_HALRM);
	sd->rx_power[LALRM] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_RX_PWR_LALRM);
	sd->rx_power[HWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_RX_PWR_HWARN);
	sd->rx_power[LWARN] = OFFSET_TO_U16_PTR(map->page_03h,
						SFF8636_RX_PWR_LWARN);

out:
	/* Channel Specific Data */
	for (i = 0; i < SFF8636_MAX_CHANNEL_NUM; i++) {
		u8 rx_power_offset, tx_bias_offset;
		u8 tx_power_offset;

		switch (i) {
		case 0:
			rx_power_offset = SFF8636_RX_PWR_1_OFFSET;
			tx_power_offset = SFF8636_TX_PWR_1_OFFSET;
			tx_bias_offset = SFF8636_TX_BIAS_1_OFFSET;
			break;
		case 1:
			rx_power_offset = SFF8636_RX_PWR_2_OFFSET;
			tx_power_offset = SFF8636_TX_PWR_2_OFFSET;
			tx_bias_offset = SFF8636_TX_BIAS_2_OFFSET;
			break;
		case 2:
			rx_power_offset = SFF8636_RX_PWR_3_OFFSET;
			tx_power_offset = SFF8636_TX_PWR_3_OFFSET;
			tx_bias_offset = SFF8636_TX_BIAS_3_OFFSET;
			break;
		case 3:
			rx_power_offset = SFF8636_RX_PWR_4_OFFSET;
			tx_power_offset = SFF8636_TX_PWR_4_OFFSET;
			tx_bias_offset = SFF8636_TX_BIAS_4_OFFSET;
			break;
		}
		sd->scd[i].bias_cur = OFFSET_TO_U16(tx_bias_offset);
		sd->scd[i].rx_power = OFFSET_TO_U16(rx_power_offset);
		sd->scd[i].tx_power = OFFSET_TO_U16(tx_power_offset);
	}
}

static void sff8636_show_dom_chan_lvl_tx_bias(const struct sff_diags *sd)
{
	char power_string[MAX_DESC_SIZE];
	int i;

	open_json_array("laser_tx_bias_current", "");
	for (i = 0; i < SFF8636_MAX_CHANNEL_NUM; i++) {
		if (is_json_context()) {
			print_float(PRINT_JSON, NULL, "%.3f",
				    (double)sd->scd[i].bias_cur / 500.);
		} else {
			snprintf(power_string, MAX_DESC_SIZE, "%s (Channel %d)",
				 "Laser tx bias current", i+1);
			PRINT_BIAS(power_string, sd->scd[i].bias_cur);
		}
	}
	close_json_array("");
}

static void sff8636_show_dom_chan_lvl_tx_power(const struct sff_diags *sd)
{
	char power_string[MAX_DESC_SIZE];
	int i;

	open_json_array("transmit_avg_optical_power", "");
	for (i = 0; i < SFF8636_MAX_CHANNEL_NUM; i++) {
		if (is_json_context()) {
			print_float(PRINT_JSON, NULL, "%.4f",
				    (double)sd->scd[i].tx_power / 10000.);
		} else {
			snprintf(power_string, MAX_DESC_SIZE, "%s (Channel %d)",
				 "Transmit avg optical power", i+1);
			PRINT_xX_PWR(power_string, sd->scd[i].tx_power);
		}
	}
	close_json_array("");
}

static void sff8636_show_dom_chan_lvl_rx_power(const struct sff_diags *sd)
{
	char *rx_power_type_string = NULL;
	char power_string[MAX_DESC_SIZE];
	int i;

	if (!sd->rx_power_type)
		rx_power_type_string = "Receiver signal OMA";
	else
		rx_power_type_string = "Rcvr signal avg optical power";

	open_json_object("rx_power");

	open_json_array("values", "");
	for (i = 0; i < SFF8636_MAX_CHANNEL_NUM; i++) {
		if (is_json_context()) {
			print_float(PRINT_JSON, NULL, "%.4f",
				    (double)sd->scd[i].rx_power / 10000.);
		} else {
			snprintf(power_string, MAX_DESC_SIZE, "%s (Channel %d)",
				 rx_power_type_string, i+1);
			PRINT_xX_PWR(power_string, sd->scd[i].rx_power);
		}
	}
	close_json_array("");

	if (is_json_context())
		module_print_any_string("type", rx_power_type_string);
	close_json_object();
}

static void
sff8636_show_dom_chan_lvl_flags(const struct sff8636_memory_map *map)
{
	bool value;
	int i;

	for (i = 0; module_aw_chan_flags[i].fmt_str; ++i) {
		bool is_start = (i % SFF8636_MAX_CHANNEL_NUM == 0);
		bool is_end = (i % SFF8636_MAX_CHANNEL_NUM ==
			       SFF8636_MAX_CHANNEL_NUM - 1);
		char json_str[80] = {};
		char str[80] = {};

		if (module_aw_chan_flags[i].type != MODULE_TYPE_SFF8636)
			continue;

		convert_json_field_name(module_aw_chan_flags[i].fmt_str,
					json_str);

		value = map->lower_memory[module_aw_chan_flags[i].offset] &
			module_aw_chan_flags[i].adver_value;
		if (is_json_context()) {
			if (is_start)
				open_json_array(json_str, "");

			print_bool(PRINT_JSON, NULL, NULL, value);

			if (is_end)
				close_json_array("");
		} else {
			snprintf(str, 80, "%s (Chan %d)",
				 module_aw_chan_flags[i].fmt_str,
				 (i % SFF8636_MAX_CHANNEL_NUM) + 1);
			printf("\t%-41s : %s\n", str, ONOFF(value));
		}

	}
}

static void
sff8636_show_dom_mod_lvl_flags(const struct sff8636_memory_map *map)
{
	bool value;
	int i;

	for (i = 0; module_aw_mod_flags[i].str; ++i) {
		if (module_aw_mod_flags[i].type != MODULE_TYPE_SFF8636)
			continue;

		value = map->lower_memory[module_aw_mod_flags[i].offset] &
			module_aw_mod_flags[i].value;

		module_print_any_bool(module_aw_mod_flags[i].str, NULL,
				      value, ONOFF(value));
	}
}

static void sff8636_show_dom(const struct sff8636_memory_map *map)
{
	struct sff_diags sd = {0};

	/*
	 * There is no clear identifier to signify the existence of
	 * optical diagnostics similar to SFF-8472. So checking existence
	 * of page 3, will provide the gurantee for existence of alarms
	 * and thresholds
	 * If pagging support exists, then supports_alarms is marked as 1
	 */
	if (map->page_03h)
		sd.supports_alarms = 1;

	sd.rx_power_type = map->page_00h[SFF8636_DIAG_TYPE_OFFSET] &
			   SFF8636_RX_PWR_TYPE_MASK;
	sd.tx_power_type = map->page_00h[SFF8636_DIAG_TYPE_OFFSET] &
			   SFF8636_RX_PWR_TYPE_MASK;

	sff8636_dom_parse(map, &sd);

	module_show_dom_mod_lvl_monitors(&sd);

	/*
	 * SFF-8636/8436 spec is not clear whether RX power/ TX bias
	 * current fields are supported or not. A valid temperature
	 * reading is used as existence for TX/RX power.
	 */
	if ((sd.sfp_temp[MCURR] == 0x0) ||
	    (sd.sfp_temp[MCURR] == (__s16)0xFFFF))
		return;

	module_print_any_bool("Alarm/warning flags implemented",
			      "alarm/warning_flags_implemented",
			      sd.supports_alarms, YESNO(sd.supports_alarms));

	sff8636_show_dom_chan_lvl_tx_bias(&sd);
	sff8636_show_dom_chan_lvl_tx_power(&sd);
	sff8636_show_dom_chan_lvl_rx_power(&sd);

	if (sd.supports_alarms) {
		sff8636_show_dom_chan_lvl_flags(map);
		sff8636_show_dom_mod_lvl_flags(map);

		if (is_json_context())
			sff_show_thresholds_json(sd);
		else
			sff_show_thresholds(sd);
	}
}

static void sff8636_show_signals(const struct sff8636_memory_map *map)
{
	unsigned int v;

	/* There appears to be no Rx LOS support bit, use Tx for both */
	if (map->page_00h[SFF8636_OPTION_4_OFFSET] & SFF8636_O4_TX_LOS) {
		v = map->lower_memory[SFF8636_LOS_AW_OFFSET] & 0xf;
		module_show_lane_status("Rx loss of signal", 4, "Yes", "No", v);
		v = map->lower_memory[SFF8636_LOS_AW_OFFSET] >> 4;
		module_show_lane_status("Tx loss of signal", 4, "Yes", "No", v);
	}

	v = map->lower_memory[SFF8636_LOL_AW_OFFSET] & 0xf;
	if (map->page_00h[SFF8636_OPTION_3_OFFSET] & SFF8636_O3_RX_LOL)
		module_show_lane_status("Rx loss of lock", 4, "Yes", "No", v);

	v = map->lower_memory[SFF8636_LOL_AW_OFFSET] >> 4;
	if (map->page_00h[SFF8636_OPTION_3_OFFSET] & SFF8636_O3_TX_LOL)
		module_show_lane_status("Tx loss of lock", 4, "Yes", "No", v);

	v = map->lower_memory[SFF8636_FAULT_AW_OFFSET] & 0xf;
	if (map->page_00h[SFF8636_OPTION_4_OFFSET] & SFF8636_O4_TX_FAULT)
		module_show_lane_status("Tx fault", 4, "Yes", "No", v);

	v = map->lower_memory[SFF8636_FAULT_AW_OFFSET] >> 4;
	if (map->page_00h[SFF8636_OPTION_2_OFFSET] & SFF8636_O2_TX_EQ_AUTO)
		module_show_lane_status("Tx adaptive eq fault", 4, "Yes", "No",
					v);
}

static void sff8636_show_page_zero(const struct sff8636_memory_map *map)
{
	sff8636_show_ext_identifier(map);
	sff8636_show_connector(map);
	sff8636_show_transceiver(map);
	sff8636_show_encoding(map);
	module_show_value_with_unit(map->page_00h, SFF8636_BR_NOMINAL_OFFSET,
				    "BR Nominal", 100, "Mbps");
	sff8636_show_rate_identifier(map);
	module_show_value_with_unit(map->page_00h, SFF8636_SM_LEN_OFFSET,
				    "Length (SMF)", 1, "km");
	module_show_value_with_unit(map->page_00h, SFF8636_OM3_LEN_OFFSET,
				    "Length (OM3)", 2, "m");
	module_show_value_with_unit(map->page_00h, SFF8636_OM2_LEN_OFFSET,
				    "Length (OM2)", 1, "m");
	module_show_value_with_unit(map->page_00h, SFF8636_OM1_LEN_OFFSET,
				    "Length (OM1)", 1, "m");
	module_show_value_with_unit(map->page_00h, SFF8636_CBL_LEN_OFFSET,
				    "Length (Copper or Active cable)", 1, "m");
	sff8636_show_wavelength_or_copper_compliance(map);
	module_show_ascii(map->page_00h, SFF8636_VENDOR_NAME_START_OFFSET,
			  SFF8636_VENDOR_NAME_END_OFFSET, "Vendor name");
	module_show_oui(map->page_00h, SFF8636_VENDOR_OUI_OFFSET);
	module_show_ascii(map->page_00h, SFF8636_VENDOR_PN_START_OFFSET,
			  SFF8636_VENDOR_PN_END_OFFSET, "Vendor PN");
	module_show_ascii(map->page_00h, SFF8636_VENDOR_REV_START_OFFSET,
			  SFF8636_VENDOR_REV_END_OFFSET, "Vendor rev");
	module_show_ascii(map->page_00h, SFF8636_VENDOR_SN_START_OFFSET,
			  SFF8636_VENDOR_SN_END_OFFSET, "Vendor SN");
	module_show_ascii(map->page_00h, SFF8636_DATE_YEAR_OFFSET,
			  SFF8636_DATE_VENDOR_LOT_OFFSET + 1, "Date code");
	sff8636_show_revision_compliance(map->lower_memory,
					 SFF8636_REV_COMPLIANCE_OFFSET);
	sff8636_show_signals(map);
}

static void sff8636_show_all_common(const struct sff8636_memory_map *map)
{
	sff8636_show_identifier(map);
	switch (map->lower_memory[SFF8636_ID_OFFSET]) {
	case MODULE_ID_QSFP:
	case MODULE_ID_QSFP_PLUS:
	case MODULE_ID_QSFP28:
		sff8636_show_page_zero(map);
		sff8636_show_dom(map);
		break;
	}
}

static void sff8636_memory_map_init_buf(struct sff8636_memory_map *map,
					const __u8 *id, __u32 eeprom_len)
{
	/* Lower Memory and Page 00h are always present.
	 *
	 * Offset into Upper Memory is between page size and twice the page
	 * size. Therefore, set the base address of each page to base address
	 * plus page size multiplied by the page number.
	 */
	map->lower_memory = id;
	map->page_00h = id;

	/* Page 03h is only present when the module memory model is paged and
	 * not flat and when we got a big enough buffer from the kernel.
	 */
	if (map->lower_memory[SFF8636_STATUS_2_OFFSET] &
	    SFF8636_STATUS_PAGE_3_PRESENT ||
	    eeprom_len != ETH_MODULE_SFF_8636_MAX_LEN)
		return;

	map->page_03h = id + 3 * SFF8636_PAGE_SIZE;
}

void sff8636_show_all_ioctl(const __u8 *id, __u32 eeprom_len)
{
	struct sff8636_memory_map map = {};

	switch (id[SFF8636_ID_OFFSET]) {
	case MODULE_ID_QSFP_DD:
	case MODULE_ID_OSFP:
	case MODULE_ID_DSFP:
	case MODULE_ID_QSFP_PLUS_CMIS:
	case MODULE_ID_SFP_DD_CMIS:
	case MODULE_ID_SFP_PLUS_CMIS:
		cmis_show_all_ioctl(id);
		break;
	default:
		sff8636_memory_map_init_buf(&map, id, eeprom_len);
		sff8636_show_all_common(&map);
		break;
	}
}

static void sff8636_request_init(struct ethtool_module_eeprom *request, u8 page,
				 u32 offset)
{
	request->offset = offset;
	request->length = SFF8636_PAGE_SIZE;
	request->page = page;
	request->bank = 0;
	request->i2c_address = SFF8636_I2C_ADDRESS;
	request->data = NULL;
}

static int
sff8636_memory_map_init_pages(struct cmd_context *ctx,
			      struct sff8636_memory_map *map)
{
	struct ethtool_module_eeprom request;
	int ret;

	/* Lower Memory and Page 00h are always present.
	 *
	 * Offset into Upper Memory is between page size and twice the page
	 * size. Therefore, set the base address of each page to its base
	 * address minus page size.
	 */
	sff8636_request_init(&request, 0x0, 0);
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;
	map->lower_memory = request.data;

	sff8636_request_init(&request, 0x0, SFF8636_PAGE_SIZE);
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0)
		return ret;
	map->page_00h = request.data - SFF8636_PAGE_SIZE;

	/* Page 03h is only present when the module memory model is paged and
	 * not flat.
	 */
	if (map->lower_memory[SFF8636_STATUS_2_OFFSET] &
	    SFF8636_STATUS_PAGE_3_PRESENT)
		return 0;

	sff8636_request_init(&request, 0x3, SFF8636_PAGE_SIZE);
	ret = nl_get_eeprom_page(ctx, &request);
	if (ret < 0) {
		/* Page 03h is not available due to a bug in the driver.
		 * This is a non-fatal error and sff8636_dom_parse()
		 * handles this correctly.
		 */
		fprintf(stderr, "Failed to read Upper Page 03h, driver error?\n");
		return 0;
	}

	map->page_03h = request.data - SFF8636_PAGE_SIZE;

	return 0;
}

int sff8636_show_all_nl(struct cmd_context *ctx)
{
	struct sff8636_memory_map map = {};
	int ret;

	new_json_obj(ctx->json);
	open_json_object(NULL);

	ret = sff8636_memory_map_init_pages(ctx, &map);
	if (ret < 0)
		return ret;
	sff8636_show_all_common(&map);

	close_json_object();
	delete_json_obj();

	return 0;
}
