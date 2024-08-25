/*
 * country.c
 *
 * Copyright (C) 2007 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <malloc.h>
#include <string.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>

#define YES 1
#define NO 0

enum EnumRd {
	/*
	 * The following regulatory domain definitions are
	 * found in the EEPROM. Each regulatory domain
	 * can operate in either a 5GHz or 2.4GHz wireless mode or
	 * both 5GHz and 2.4GHz wireless modes.
	 * In general, the value holds no special
	 * meaning and is used to decode into either specific
	 * 2.4GHz or 5GHz wireless mode for that particular
	 * regulatory domain.
	 */
	NO_ENUMRD = 0x00,
	NULL1_WORLD = 0x03, /* For 11b-only countries (no 11a allowed) */
	NULL1_ETSIB = 0x07, /* Israel */
	NULL1_ETSIC = 0x08,
	FCC1_FCCA = 0x10, /* USA */
	FCC1_WORLD = 0x11, /* Hong Kong */
	FCC4_FCCA = 0x12, /* USA - Public Safety */

	FCC2_FCCA = 0x20, /* Canada */
	FCC2_WORLD = 0x21, /* Australia & HK */
	FCC2_ETSIC = 0x22,
	FRANCE_RES = 0x31, /* Legacy France for OEM */
	FCC3_FCCA = 0x3A, /* USA & Canada w/5470 band, 11h, DFS enabled */
	FCC3_WORLD = 0x3B, /* USA & Canada w/5470 band, 11h, DFS enabled */

	ETSI1_WORLD = 0x37,
	ETSI3_ETSIA = 0x32, /* France (optional) */
	ETSI2_WORLD = 0x35, /* Hungary & others */
	ETSI3_WORLD = 0x36, /* France & others */
	ETSI4_WORLD = 0x30,
	ETSI4_ETSIC = 0x38,
	ETSI5_WORLD = 0x39,
	ETSI6_WORLD = 0x34, /* Bulgaria */
	ETSI7_WORLD = 0x3c, /* Bulgaria */
	ETSI8_WORLD = 0x3d, /* Bulgaria */
	//  ETSI_RESERVED = 0x33,               /* Reserved (Do not used) */
	BFWA = 0x33, /* Europe & others */
	EGAL = 0x92, /* Europe & others */

	MKK1_MKKA = 0x40, /* Japan (JP1) */
	MKK1_MKKB = 0x41, /* Japan (JP0) */
	APL4_WORLD = 0x42, /* Singapore */
	MKK2_MKKA = 0x43, /* Japan with 4.9G channels */
	APL_RESERVED = 0x44, /* Reserved (Do not used)  */
	APL2_WORLD = 0x45, /* Korea */
	APL2_APLC = 0x46,
	APL3_WORLD = 0x47,
	MKK1_FCCA = 0x48, /* Japan (JP1-1) */
	APL2_APLD = 0x49, /* Korea with 2.3G channels */
	MKK1_MKKA1 = 0x4A, /* Japan (JE1) */
	MKK1_MKKA2 = 0x4B, /* Japan (JE2) */
	MKK1_MKKC = 0x4C, /* Japan (MKK1_MKKA,except Ch14) */

	APL3_FCCA = 0x50,
	APL1_WORLD = 0x52, /* Latin America */
	APL1_FCCA = 0x53,
	APL1_APLA = 0x54,
	APL1_ETSIC = 0x55,
	APL2_ETSIC = 0x56, /* Venezuela */
	APL5_WORLD = 0x58, /* Chile */
	APL6_WORLD = 0x5B, /* Singapore */
	APL7_FCCA = 0x5C, /* Taiwan 5.47 Band */
	APL8_WORLD = 0x5D, /* Malaysia 5GHz */
	APL9_WORLD = 0x5E, /* Korea 5GHz */
	APL11_WORLD = 0x5A, /* India */

	APL6_FCCA = 0xaB, /* Singapore */
	APL8_FCCA = 0xaD, /* Malaysia 5GHz */
	NULL1_FCCA = 0xa3, /* For 11b-only countries (no 11a allowed) */
	APL11_FCCA = 0xaA, /* India */
	APL9_FCCA = 0xae, /* Korea 5GHz */

#ifdef HAVE_RAIEXTRA
	RAI_WORLD = 0xa0,
	RAIIT_WORLD = 0xa1,
	IT_WORLD = 0xa2,
	RAI = 0x01a0,
	RAIIT = 0x01a1,
#endif
#ifdef HAVE_TESTEM
	TESTEM_CLIENT_WORLD = 0xa0,
	TESTEM_AP_WORLD = 0xa1,

#endif
#ifdef HAVE_TMK
	TMK_AP_WORLD = 0xa1,
#endif
	IT = 0x01a2,

	/*
	 * World mode SKUs
	 */
	//      WOR0_WORLD      = 0x60,         /* World0 (WO0 SKU) */
	WOR1_WORLD = 0x61, /* World1 (WO1 SKU) */
	WOR2_WORLD = 0x62, /* World2 (WO2 SKU) */
	WOR3_WORLD = 0x63, /* World3 (WO3 SKU) */
	WOR4_WORLD = 0x64, /* World4 (WO4 SKU) */
	WOR5_ETSIC = 0x65, /* World5 (WO5 SKU) */

	WOR01_WORLD = 0x66, /* World0-1 (WW0-1 SKU) */
	WOR02_WORLD = 0x67, /* World0-2 (WW0-2 SKU) */
	EU1_WORLD = 0x68, /* Same as World0-2 (WW0-2 SKU), except active scan ch1-13. No ch14 */

	WOR9_WORLD = 0x69, /* World9 (WO9 SKU) */
	WORA_WORLD = 0x6A, /* WorldA (WOA SKU) */

	MKK3_MKKB = 0x80, /* Japan UNI-1 even + MKKB */
	MKK3_MKKA2 = 0x81, /* Japan UNI-1 even + MKKA2 */
	MKK3_MKKC = 0x82, /* Japan UNI-1 even + MKKC */

	MKK4_MKKB = 0x83, /* Japan UNI-1 even + UNI-2 + MKKB */
	MKK4_MKKA2 = 0x84, /* Japan UNI-1 even + UNI-2 + MKKA2 */
	MKK4_MKKC = 0x85, /* Japan UNI-1 even + UNI-2 + MKKC */

	MKK5_MKKB = 0x86, /* Japan UNI-1 even + UNI-2 + mid-band + MKKB */
	MKK5_MKKA2 = 0x87, /* Japan UNI-1 even + UNI-2 + mid-band + MKKA2 */
	MKK5_MKKC = 0x88, /* Japan UNI-1 even + UNI-2 + mid-band + MKKC */

	MKK6_MKKB = 0x89, /* Japan UNI-1 even + UNI-1 odd MKKB */
	MKK6_MKKA2 = 0x8A, /* Japan UNI-1 even + UNI-1 odd + MKKA2 */
	MKK6_MKKC = 0x8B, /* Japan UNI-1 even + UNI-1 odd + MKKC */

	MKK7_MKKB = 0x8C, /* Japan UNI-1 even + UNI-1 odd + UNI-2 + MKKB */
	MKK7_MKKA2 = 0x8D, /* Japan UNI-1 even + UNI-1 odd + UNI-2 + MKKA2 */
	MKK7_MKKC = 0x8E, /* Japan UNI-1 even + UNI-1 odd + UNI-2 + MKKC */

	MKK8_MKKB = 0x8F, /* Japan UNI-1 even + UNI-1 odd + UNI-2 + mid-band + MKKB */
	MKK8_MKKA2 = 0x90, /* Japan UNI-1 even + UNI-1 odd + UNI-2 + mid-band + MKKA2 */
	MKK8_MKKC = 0x91, /* Japan UNI-1 even + UNI-1 odd + UNI-2 + mid-band + MKKC */

	/* Following definitions are used only by s/w to map old
	 * Japan SKUs.
	 */
	MKK3_MKKA = 0xF0, /* Japan UNI-1 even + MKKA */
	MKK3_MKKA1 = 0xF1, /* Japan UNI-1 even + MKKA1 */
	MKK3_FCCA = 0xF2, /* Japan UNI-1 even + FCCA */
	MKK4_MKKA = 0xF3, /* Japan UNI-1 even + UNI-2 + MKKA */
	MKK4_MKKA1 = 0xF4, /* Japan UNI-1 even + UNI-2 + MKKA1 */
	MKK4_FCCA = 0xF5, /* Japan UNI-1 even + UNI-2 + FCCA */
	MKK9_MKKA = 0xF6, /* Japan UNI-1 even + 4.9GHz */
	MKK10_MKKA = 0xF7, /* Japan UNI-1 even + UNI-2 + 4.9GHz */

	/*
	 * Regulator domains ending in a number (e.g. APL1,
	 * MK1, ETSI4, etc) apply to 5GHz channel and power
	 * information.  Regulator domains ending in a letter
	 * (e.g. APLA, FCCA, etc) apply to 2.4GHz channel and
	 * power information.
	 */
	APL1 = 0x0150, /* LAT & Asia */
	APL2 = 0x0250, /* LAT & Asia */
	APL3 = 0x0350, /* Taiwan */
	APL4 = 0x0450, /* Jordan */
	APL5 = 0x0550, /* Chile */
	APL6 = 0x0650, /* Singapore */
	APL7 = 0x0750, /* Singapore */
	APL8 = 0x0850, /* Malaysia */
	APL9 = 0x0950, /* Korea (South) ROC 3 */

	ETSI1 = 0x0130, /* Europe & others */
	ETSI2 = 0x0230, /* Europe & others */
	ETSI3 = 0x0330, /* Europe & others */
	ETSI4 = 0x0430, /* Europe & others */
	ETSI5 = 0x0530, /* Europe & others */
	ETSI6 = 0x0630, /* Europe & others */
	ETSI7 = 0x0730, /* Europe & others */
	ETSIA = 0x0A30, /* France */
	ETSIB = 0x0B30, /* Israel */
	ETSIC = 0x0C30, /* Latin America */

	FCC1 = 0x0110, /* US & others */
	FCC2 = 0x0120, /* Canada, Australia & New Zealand */
	FCC3 = 0x0160, /* US w/new middle band & DFS */
	FCC4 = 0x0165, /* US Public Safety */
	FCCA = 0x0A10,

	APLD = 0x0D50, /* South Korea */

	MKK1 = 0x0140, /* Japan (UNI-1 odd) */
	MKK2 = 0x0240, /* Japan (4.9 GHz + UNI-1 odd) */
	MKK3 = 0x0340, /* Japan (UNI-1 even) */
	MKK4 = 0x0440, /* Japan (UNI-1 even + UNI-2) */
	MKK5 = 0x0540, /* Japan (UNI-1 even + UNI-2 + mid-band) */
	MKK6 = 0x0640, /* Japan (UNI-1 odd + UNI-1 even) */
	MKK7 = 0x0740, /* Japan (UNI-1 odd + UNI-1 even + UNI-2 */
	MKK8 = 0x0840, /* Japan (UNI-1 odd + UNI-1 even + UNI-2 + mid-band) */
	MKK9 = 0x0940, /* Japan (UNI-1 even + 4.9 GHZ) */
	MKK10 = 0x0B40, /* Japan (UNI-1 even + UNI-2 + 4.9 GHZ) */
	MKKA = 0x0A40, /* Japan */
	MKKC = 0x0A50,

	NULL1 = 0x0198,
	WORLD = 0x0199,
	DEBUG_REG_DMN = 0x01ff,
};

#define WORLD_SKU_MASK 0x00F0
#define WORLD_SKU_PREFIX 0x0060

enum { /* conformance test limits */
       FCC = 0x10,
       MKK = 0x40,
       ETSI = 0x30,
};

enum CountryCode {
	CTRY_DEBUG = 0x1ff, /* debug country code */
	CTRY_DEFAULT = 0, /* default country code */
	CTRY_ALBANIA = 8, /* Albania */
	CTRY_ALGERIA = 12, /* Algeria */
	CTRY_ARGENTINA = 32, /* Argentina */
	CTRY_ARMENIA = 51, /* Armenia */
	CTRY_AUSTRALIA = 36, /* Australia */
	CTRY_AUSTRIA = 40, /* Austria */
	CTRY_AZERBAIJAN = 31, /* Azerbaijan */
	CTRY_BAHRAIN = 48, /* Bahrain */
	CTRY_BANGLADESH = 50, /* Bangladesh */
	CTRY_BARBADOS = 52, /* Barbados */
	CTRY_BELARUS = 112, /* Belarus */
	CTRY_BELGIUM = 56, /* Belgium */
	CTRY_BELIZE = 84, /* Belize */
	CTRY_BOLIVIA = 68, /* Bolivia */
	CTRY_BOSNIA_HERZ = 70, /* Bosnia and Herzegowina */
	CTRY_BRAZIL = 76, /* Brazil */
	CTRY_BRUNEI_DARUSSALAM = 96, /* Brunei Darussalam */
	CTRY_BULGARIA = 100, /* Bulgaria */
	CTRY_CAMBODIA = 116, /* Cambodia */
	CTRY_CANADA = 124, /* Canada */
	CTRY_CHILE = 152, /* Chile */
	CTRY_CHINA = 156, /* People's Republic of China */
	CTRY_CHRISTMAS_ISLAND = 157, /* People's Republic of China */
	CTRY_CUBA = 158, /* People's Republic of China */
	CTRY_COLOMBIA = 170, /* Colombia */
	CTRY_COSTA_RICA = 188, /* Costa Rica */
	CTRY_CROATIA = 191, /* Croatia */
	CTRY_CYPRUS = 196,
	CTRY_CZECH = 203, /* Czech Republic */
	CTRY_DENMARK = 208, /* Denmark */
	CTRY_DOMINICAN_REPUBLIC = 214, /* Dominican Republic */
	CTRY_ECUADOR = 218, /* Ecuador */
	CTRY_EGYPT = 818, /* Egypt */
	CTRY_EL_SALVADOR = 222, /* El Salvador */
	CTRY_ESTONIA = 233, /* Estonia */
	CTRY_FAEROE_ISLANDS = 234, /* Faeroe Islands */
	CTRY_FINLAND = 246, /* Finland */
	CTRY_FRANCE = 250, /* France */
	CTRY_FRANCE2 = 255, /* France2 */
	CTRY_GEORGIA = 268, /* Georgia */
	CTRY_GERMANY = 276, /* Germany */
	CTRY_GERMANY_BFWA = 277, /* Germany */
	CTRY_EGALISTAN = 278,
	CTRY_GREECE = 300, /* Greece */
	CTRY_GREENLAND = 304, /* Greenland */
	CTRY_GRENADA = 308, /* Grenada */
	CTRY_GUAM = 316, /* Guam */
	CTRY_GUATEMALA = 320, /* Guatemala */
	CTRY_HAITI = 332, /* Haiti */
	CTRY_HONDURAS = 340, /* Honduras */
	CTRY_HONG_KONG = 344, /* Hong Kong S.A.R., P.R.C. */
	CTRY_HUNGARY = 348, /* Hungary */
	CTRY_ICELAND = 352, /* Iceland */
	CTRY_INDIA = 356, /* India */
	CTRY_INDIA_BFWA = 357, /* India BFWA */
	CTRY_INDONESIA = 360, /* Indonesia */
	CTRY_IRAN = 364, /* Iran */
	CTRY_IRAQ = 368, /* Iraq */
	CTRY_IRELAND = 372, /* Ireland */
	CTRY_ISRAEL = 376, /* Israel */
	CTRY_ITALY = 380, /* Italy */
#ifdef HAVE_RAIEXTRA
	CTRY_ITALYRAI = 381, /* Italy */
	CTRY_RAI = 382, /* Italy */
#endif
#ifdef HAVE_TESTEM
	CTRY_TESTEM_CLIENT = 381, /* Testem */
	CTRY_TESTEM_AP = 382, /* Testem */
#endif
#ifdef HAVE_TMK
	CTRY_EXTRATMK = 381, /* */
#endif
	CTRY_JAMAICA = 388, /* Jamaica */
	CTRY_JAPAN = 392, /* Japan */
	CTRY_JORDAN = 400, /* Jordan */
	CTRY_KAZAKHSTAN = 398, /* Kazakhstan */
	CTRY_KENYA = 404, /* Kenya */
	CTRY_KOREA_NORTH = 408, /* North Korea */
	CTRY_KOREA_ROC = 410, /* South Korea */
	CTRY_KOREA_ROC2 = 411, /* South Korea */
	CTRY_KOREA_ROC3 = 412, /* South Korea */
	CTRY_KUWAIT = 414, /* Kuwait */
	CTRY_LATVIA = 428, /* Latvia */
	CTRY_LEBANON = 422, /* Lebanon */
	CTRY_LIBYA = 434, /* Libya */
	CTRY_LIECHTENSTEIN = 438, /* Liechtenstein */
	CTRY_LITHUANIA = 440, /* Lithuania */
	CTRY_LUXEMBOURG = 442, /* Luxembourg */
	CTRY_MACAU = 446, /* Macau */
	CTRY_MACEDONIA = 807, /* the Former Yugoslav Republic of Macedonia */
	CTRY_MALAYSIA = 458, /* Malaysia */
	CTRY_MALDIVES = 462, /* Maldives */
	CTRY_MALTA = 470, /* Malta */
	CTRY_MEXICO = 484, /* Mexico */
	CTRY_MONACO = 492, /* Principality of Monaco */
	CTRY_MOROCCO = 504, /* Morocco */
	CTRY_NEPAL = 524, /* Nepal */
	CTRY_NETHERLANDS = 528, /* Netherlands */
	CTRY_NETHERLANDS_ANTILLES = 530, /* Netherlands-Antilles */
	CTRY_ARUBA = 533, /* Aruba */
	CTRY_NEW_ZEALAND = 554, /* New Zealand */
	CTRY_NICARAGUA = 558, /* Nicaragua */
	CTRY_NORWAY = 578, /* Norway */
	CTRY_NORWAY_BFWA = 579, /* Norway */
	CTRY_OMAN = 512, /* Oman */
	CTRY_PAKISTAN = 586, /* Islamic Republic of Pakistan */
	CTRY_PANAMA = 591, /* Panama */
	CTRY_PAPUA_NEW_GUINEA = 598, /* Papua New Guinea */
	CTRY_PARAGUAY = 600, /* Paraguay */
	CTRY_PERU = 604, /* Peru */
	CTRY_PHILIPPINES = 608, /* Republic of the Philippines */
	CTRY_POLAND = 616, /* Poland */
	CTRY_PORTUGAL = 620, /* Portugal */
	CTRY_PUERTO_RICO = 630, /* Puerto Rico */
	CTRY_QATAR = 634, /* Qatar */
	CTRY_ROMANIA = 642, /* Romania */
	CTRY_RUSSIA = 643, /* Russia */
	CTRY_SAUDI_ARABIA = 682, /* Saudi Arabia */
	CTRY_SERBIA = 688, /* Republic of Serbia */
	CTRY_MONTENEGRO = 499, /* Montenegro */
	CTRY_SINGAPORE = 702, /* Singapore */
	CTRY_SLOVAKIA = 703, /* Slovak Republic */
	CTRY_SLOVENIA = 705, /* Slovenia */
	CTRY_SOUTH_AFRICA = 710, /* South Africa */
	CTRY_SPAIN = 724, /* Spain */
	CTRY_SRI_LANKA = 144, /* Sri Lanka */
	CTRY_SWEDEN = 752, /* Sweden */
	CTRY_SWITZERLAND = 756, /* Switzerland */
	CTRY_SYRIA = 760, /* Syria */
	CTRY_TAIWAN = 158, /* Taiwan */
	CTRY_THAILAND = 764, /* Thailand */
	CTRY_TRINIDAD_Y_TOBAGO = 780, /* Trinidad y Tobago */
	CTRY_TUNISIA = 788, /* Tunisia */
	CTRY_TURKEY = 792, /* Turkey */
	CTRY_UAE = 784, /* U.A.E. */
	CTRY_UKRAINE = 804, /* Ukraine */
	CTRY_UNITED_KINGDOM = 826, /* United Kingdom */
	CTRY_UNITED_KINGDOM_BFWA = 827, /* United Kingdom */
	CTRY_UNITED_STATES = 840, /* United States */
	CTRY_UNITED_STATES2 = 841, /* United States for AP */
	CTRY_UNITED_STATES_FCC49 = 842, /* United States (Public Safety) */
	CTRY_URUGUAY = 858, /* Uruguay */
	CTRY_UZBEKISTAN = 860, /* Uzbekistan */
	CTRY_VENEZUELA = 862, /* Venezuela */
	CTRY_VIET_NAM = 704, /* Viet Nam */
	CTRY_YEMEN = 887, /* Yemen */
	CTRY_ZIMBABWE = 716, /* Zimbabwe */

	/*
	 ** Japan special codes.  Boy, do they have a lot
	 */

	CTRY_JAPAN1 = 393, /* Japan (JP1) */
	CTRY_JAPAN2 = 394, /* Japan (JP0) */
	CTRY_JAPAN3 = 395, /* Japan (JP1-1) */
	CTRY_JAPAN4 = 396, /* Japan (JE1) */
	CTRY_JAPAN5 = 397, /* Japan (JE2) */
	CTRY_JAPAN6 = 4006, /* Japan (JP6) */
	CTRY_JAPAN7 = 4007, /* Japan (J7) */
	CTRY_JAPAN8 = 4008, /* Japan (J8) */
	CTRY_JAPAN9 = 4009, /* Japan (J9) */
	CTRY_JAPAN10 = 4010, /* Japan (J10) */
	CTRY_JAPAN11 = 4011, /* Japan (J11) */
	CTRY_JAPAN12 = 4012, /* Japan (J12) */
	CTRY_JAPAN13 = 4013, /* Japan (J13) */
	CTRY_JAPAN14 = 4014, /* Japan (J14) */
	CTRY_JAPAN15 = 4015, /* Japan (J15) */
	CTRY_JAPAN16 = 4016, /* Japan (J16) */
	CTRY_JAPAN17 = 4017, /* Japan (J17) */
	CTRY_JAPAN18 = 4018, /* Japan (J18) */
	CTRY_JAPAN19 = 4019, /* Japan (J19) */
	CTRY_JAPAN20 = 4020, /* Japan (J20) */
	CTRY_JAPAN21 = 4021, /* Japan (J21) */
	CTRY_JAPAN22 = 4022, /* Japan (J22) */
	CTRY_JAPAN23 = 4023, /* Japan (J23) */
	CTRY_JAPAN24 = 4024, /* Japan (J24) */
	CTRY_JAPAN25 = 4025, /* Japan (J25) */
	CTRY_JAPAN26 = 4026, /* Japan (J26) */
	CTRY_JAPAN27 = 4027, /* Japan (J27) */
	CTRY_JAPAN28 = 4028, /* Japan (J28) */
	CTRY_JAPAN29 = 4029, /* Japan (J29) */
	CTRY_JAPAN30 = 4030, /* Japan (J30) */
	CTRY_JAPAN31 = 4031, /* Japan (J31) */
	CTRY_JAPAN32 = 4032, /* Japan (J32) */
	CTRY_JAPAN33 = 4033, /* Japan (J33) */
	CTRY_JAPAN34 = 4034, /* Japan (J34) */
	CTRY_JAPAN35 = 4035, /* Japan (J35) */
	CTRY_JAPAN36 = 4036, /* Japan (J36) */
	CTRY_JAPAN37 = 4037, /* Japan (J37) */
	CTRY_JAPAN38 = 4038, /* Japan (J38) */
	CTRY_JAPAN39 = 4039, /* Japan (J39) */
	CTRY_JAPAN40 = 4040, /* Japan (J40) */
	CTRY_JAPAN41 = 4041, /* Japan (J41) */
	CTRY_JAPAN42 = 4042, /* Japan (J42) */
	CTRY_JAPAN43 = 4043, /* Japan (J43) */
	CTRY_JAPAN44 = 4044, /* Japan (J44) */
	CTRY_JAPAN45 = 4045, /* Japan (J45) */
	CTRY_JAPAN46 = 4046, /* Japan (J46) */
	CTRY_JAPAN47 = 4047, /* Japan (J47) */
	CTRY_JAPAN48 = 4048, /* Japan (J48) */
	CTRY_JAPAN49 = 4049, /* Japan (J49) */
	CTRY_JAPAN50 = 4050, /* Japan (J50) */
	CTRY_JAPAN51 = 4051, /* Japan (J51) */
	CTRY_JAPAN52 = 4052, /* Japan (J52) */
	CTRY_JAPAN53 = 4053, /* Japan (J53) */
	CTRY_JAPAN54 = 4054, /* Japan (J54) */
	CTRY_JAPAN55 = 4055, /* Japan (J55) */
	CTRY_JAPAN56 = 4056, /* Japan (J56) */
	CTRY_JAPAN57 = 4057, /* Japan (J57) */
	CTRY_JAPAN58 = 4058, /* Japan (J58) */
	CTRY_JAPAN59 = 4059, /* Japan (J59) */

	/*
	 ** "Special" codes for multiply defined countries, with the exception
	 ** of Japan and US.
	 */

	CTRY_AUSTRALIA2 = 5000, /* Australia for AP only */
	CTRY_CANADA2 = 5001, /* Canada for AP only */
	CTRY_BELGIUM2 = 5002 /* Belgium/Cisco implementation */
};

typedef struct {
	unsigned short countryCode;
	unsigned short regDmnEnum;
	char *isoName;
	const char *name;
} COUNTRY_CODE_TO_ENUM_RD;
#define DEF_REGDMN FCC1_FCCA

#define N(a) (sizeof(a) / sizeof(a[0]))

#if !defined(HAVE_NORTHSTAR) && !defined(HAVE_80211AC)
static COUNTRY_CODE_TO_ENUM_RD allCountries[] = {
	//  {CTRY_DEFAULT, DEF_REGDMN, "NA", "NO_COUNTRY_SET"},
	{ CTRY_ALBANIA, NULL1_WORLD, "AL", "ALBANIA" },
	{ CTRY_ALGERIA, NULL1_WORLD, "DZ", "ALGERIA" },
	{ CTRY_ARGENTINA, APL3_WORLD, "AR", "ARGENTINA" },
	{ CTRY_ARMENIA, ETSI4_WORLD, "AM", "ARMENIA" },
	{ CTRY_ARUBA, ETSI1_WORLD, "AW", "ARUBA" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_AUSTRALIA, FCC2_FCCA, "AU", "AUSTRALIA"},
	//#else
	//      {CTRY_AUSTRALIA, FCC2_WORLD, "AU", "AUSTRALIA"},
	//#endif
	{ CTRY_AUSTRALIA, FCC2_WORLD, "AU", "AUSTRALIA" },
	{ CTRY_AUSTRIA, ETSI1_WORLD, "AT", "AUSTRIA" },
	{ CTRY_AZERBAIJAN, ETSI4_WORLD, "AZ", "AZERBAIJAN" },
	{ CTRY_BANGLADESH, NULL1_WORLD, "BD", "BANGLADESH" },
	{ CTRY_BARBADOS, FCC2_WORLD, "BB", "BARBADOS" },
	{ CTRY_BAHRAIN, APL6_WORLD, "BH", "BAHRAIN" },
	{ CTRY_BELARUS, NULL1_WORLD, "BY", "BELARUS" },
	{ CTRY_BELGIUM, ETSI1_WORLD, "BE", "BELGIUM" },
	{ CTRY_BELIZE, APL1_ETSIC, "BZ", "BELIZE" },
	{ CTRY_BOLIVIA, APL1_ETSIC, "BO", "BOLIVIA" },
	{ CTRY_BRAZIL, FCC3_WORLD, "BR", "BRAZIL" },
	{ CTRY_BRUNEI_DARUSSALAM, APL1_WORLD, "BN", "BRUNEI_DARUSSALAM" },
	{ CTRY_BULGARIA, ETSI6_WORLD, "BG", "BULGARIA" },
	{ CTRY_CAMBODIA, ETSI1_WORLD, "KH", "CAMBODIA" },
	{ CTRY_CANADA, FCC2_FCCA, "CA", "CANADA" },
	{ CTRY_CHILE, APL6_WORLD, "CL", "CHILE" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_CHINA, APL1_FCCA, "CN", "CHINA"},
	//#else
	{ CTRY_CHINA, APL1_WORLD, "CN", "CHINA" },
	//#endif
	{ CTRY_COLOMBIA, FCC1_FCCA, "CO", "COLOMBIA" },
	{ CTRY_COSTA_RICA, NULL1_WORLD, "CR", "COSTA_RICA" },
#ifdef HAVE_ATH9K
	{ CTRY_CUBA, APL1_WORLD, "CU", "CUBA" },
	{ CTRY_CHRISTMAS_ISLAND, APL1_WORLD, "CX", "CHRISTMAS_ISLAND" },
#endif
	{ CTRY_CROATIA, ETSI3_WORLD, "HR", "CROATIA" },
	{ CTRY_CYPRUS, ETSI1_WORLD, "CY", "CYPRUS" },
	{ CTRY_CZECH, ETSI3_WORLD, "CZ", "CZECH_REPUBLIC" },
	{ CTRY_DENMARK, ETSI1_WORLD, "DK", "DENMARK" },
	{ CTRY_DOMINICAN_REPUBLIC, FCC1_FCCA, "DO", "DOMINICAN_REPUBLIC" },
	//  {CTRY_EGALISTAN, EGAL, "EGAL", "EGALISTAN"},
	{ CTRY_ECUADOR, NULL1_WORLD, "EC", "ECUADOR" },
	{ CTRY_EGYPT, ETSI3_WORLD, "EG", "EGYPT" },
	{ CTRY_EL_SALVADOR, NULL1_WORLD, "SV", "EL_SALVADOR" },
	{ CTRY_ESTONIA, ETSI1_WORLD, "EE", "ESTONIA" },
	{ CTRY_FINLAND, ETSI1_WORLD, "FI", "FINLAND" },
	{ CTRY_FRANCE, ETSI1_WORLD, "FR", "FRANCE" },
	{ CTRY_FRANCE2, ETSI3_WORLD, "F2", "FRANCE_RES" },
	{ CTRY_GEORGIA, ETSI4_WORLD, "GE", "GEORGIA" },
	{ CTRY_GERMANY, ETSI1_WORLD, "DE", "GERMANY" },
	// Not assigned, use for GERMANY_BFWA
	{ CTRY_GERMANY_BFWA, BFWA, "ZD", "GERMANY_BFWA" },
	{ CTRY_GREECE, ETSI1_WORLD, "GR", "GREECE" },
	{ CTRY_GREENLAND, ETSI1_WORLD, "GL", "GREENLAND" },
	{ CTRY_GRENADA, FCC3_FCCA, "GD", "GRENADA" },
	{ CTRY_GUAM, FCC1_FCCA, "GU", "GUAM" },
	{ CTRY_GUATEMALA, FCC1_FCCA, "GT", "GUATEMALA" },
	{ CTRY_HAITI, ETSI1_WORLD, "HT", "HAITI" },
	{ CTRY_HONDURAS, NULL1_WORLD, "HN", "HONDURAS" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_HONG_KONG, FCC2_FCCA, "HK", "HONG_KONG"},
	//#else
	{ CTRY_HONG_KONG, FCC2_WORLD, "HK", "HONG_KONG" },
	//#endif
	{ CTRY_HUNGARY, ETSI1_WORLD, "HU", "HUNGARY" },
	{ CTRY_ICELAND, ETSI1_WORLD, "IS", "ICELAND" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_INDIA, APL11_FCCA, "IN", "INDIA"},
	//#else
	{ CTRY_INDIA, APL11_WORLD, "IN", "INDIA" },
	{ CTRY_INDIA_BFWA, APL11_WORLD, "97", "INDIA_BFWA" },
	//#endif
	{ CTRY_INDONESIA, APL1_WORLD, "ID", "INDONESIA" },
	{ CTRY_IRAN, APL1_WORLD, "IR", "IRAN" },
	{ CTRY_IRELAND, ETSI1_WORLD, "IE", "IRELAND" },
	{ CTRY_ISRAEL, NULL1_WORLD, "IL", "ISRAEL" },
#ifdef HAVE_RAIEXTRA
	{ CTRY_ITALY, IT_WORLD, "IT", "ITALY" },
	{ CTRY_ITALYRAI, RAIIT_WORLD, "IY", "ITALY+RAI" },
	{ CTRY_RAI, RAI_WORLD, "IZ", "RAI" },
#else
	{ CTRY_ITALY, ETSI1_WORLD, "IT", "ITALY" },
#endif
#ifdef HAVE_TESTEM
	{ CTRY_TESTEM_AP, TESTEM_AP_WORLD, "IY", "TESTEM_AP" },
	{ CTRY_TESTEM_CLIENT, TESTEM_CLIENT_WORLD, "IZ", "TESTEM_CLIENT" },
#endif
#ifdef HAVE_TMK
	{ CTRY_EXTRATMK, TMK_AP_WORLD, "IY", "BOS" },
#endif
	{ CTRY_JAPAN, MKK1_MKKA, "JP", "JAPAN" },
	{ CTRY_JORDAN, APL4_WORLD, "JO", "JORDAN" },
	{ CTRY_KAZAKHSTAN, NULL1_WORLD, "KZ", "KAZAKHSTAN" },
	{ CTRY_KENYA, APL1_WORLD, "KE", "KENYA" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_KOREA_ROC3, APL9_FCCA, "KR", "KOREA_REPUBLIC"},
	//#else
	{ CTRY_KOREA_ROC3, APL9_WORLD, "KR", "KOREA_REPUBLIC" },
	//#endif
	{ CTRY_KUWAIT, NULL1_WORLD, "KW", "KUWAIT" },
	{ CTRY_LATVIA, ETSI1_WORLD, "LV", "LATVIA" },
	{ CTRY_LEBANON, NULL1_WORLD, "LB", "LEBANON" },
	{ CTRY_LIECHTENSTEIN, ETSI1_WORLD, "LI", "LIECHTENSTEIN" },
	{ CTRY_LITHUANIA, ETSI1_WORLD, "LT", "LITHUANIA" },
	{ CTRY_LUXEMBOURG, ETSI1_WORLD, "LU", "LUXEMBOURG" },
	{ CTRY_MACAU, FCC2_WORLD, "MO", "MACAU" },
	{ CTRY_MACEDONIA, NULL1_WORLD, "MK", "MACEDONIA" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_MALAYSIA, APL8_FCCA, "MY", "MALAYSIA"},
	//#else
	{ CTRY_MALAYSIA, APL8_WORLD, "MY", "MALAYSIA" },
	//#endif
	{ CTRY_MALTA, ETSI1_WORLD, "MT", "MALTA" },
	{ CTRY_MEXICO, FCC1_FCCA, "MX", "MEXICO" },
	{ CTRY_MONACO, ETSI4_WORLD, "MC", "MONACO" },
	{ CTRY_MOROCCO, NULL1_WORLD, "MA", "MOROCCO" },
	{ CTRY_NEPAL, APL1_WORLD, "NP", "NEPAL" },
	{ CTRY_NETHERLANDS, ETSI1_WORLD, "NL", "NETHERLANDS" },
	{ CTRY_NEW_ZEALAND, FCC2_ETSIC, "NZ", "NEW_ZEALAND" },
	{ CTRY_KOREA_NORTH, APL2_WORLD, "KP", "NORTH_KOREA" },
	{ CTRY_NORWAY, ETSI1_WORLD, "NO", "NORWAY" },
	// Not assigned, use for NORWAY_BFWA
	{ CTRY_NORWAY_BFWA, ETSI8_WORLD, "ZN", "NORWAY_BFWA" },
	{ CTRY_OMAN, APL6_WORLD, "OM", "OMAN" },
	{ CTRY_PAKISTAN, NULL1_WORLD, "PK", "PAKISTAN" },
	{ CTRY_PAPUA_NEW_GUINEA, FCC1_WORLD, "PG", "PAPUA_NEW_GUINEA" },
	{ CTRY_PANAMA, FCC1_FCCA, "PA", "PANAMA" },
	{ CTRY_PERU, APL1_WORLD, "PE", "PERU" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_PHILIPPINES, FCC3_FCCA, "PH", "PHILIPPINES"},
	//#else
	{ CTRY_PHILIPPINES, FCC3_WORLD, "PH", "PHILIPPINES" },
	//#endif
	{ CTRY_POLAND, ETSI1_WORLD, "PL", "POLAND" },
	{ CTRY_PORTUGAL, ETSI1_WORLD, "PT", "PORTUGAL" },
	{ CTRY_PUERTO_RICO, FCC1_FCCA, "PR", "PUERTO_RICO" },
	{ CTRY_QATAR, NULL1_WORLD, "QA", "QATAR" },
	{ CTRY_ROMANIA, NULL1_WORLD, "RO", "ROMANIA" },
	{ CTRY_RUSSIA, NULL1_WORLD, "RU", "RUSSIA" },
	{ CTRY_SAUDI_ARABIA, NULL1_WORLD, "SA", "SAUDI_ARABIA" },
	{ CTRY_SERBIA, ETSI1_WORLD, "RS", "REPUBLIC_OF_SERBIA" },
	{ CTRY_MONTENEGRO, ETSI1_WORLD, "ME", "MONTENEGRO" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_SINGAPORE, APL6_FCCA, "SG", "SINGAPORE"},
	//#else
	{ CTRY_SINGAPORE, APL6_WORLD, "SG", "SINGAPORE" },
	//#endif
	{ CTRY_SLOVAKIA, ETSI1_WORLD, "SK", "SLOVAK_REPUBLIC" },
	{ CTRY_SLOVENIA, ETSI1_WORLD, "SI", "SLOVENIA" },
	{ CTRY_SOUTH_AFRICA, FCC3_WORLD, "ZA", "SOUTH_AFRICA" },
	{ CTRY_SPAIN, ETSI1_WORLD, "ES", "SPAIN" },
	{ CTRY_SWEDEN, ETSI1_WORLD, "SE", "SWEDEN" },
	{ CTRY_SWITZERLAND, ETSI1_WORLD, "CH", "SWITZERLAND" },
	{ CTRY_SYRIA, NULL1_WORLD, "SY", "SYRIA" },
	{ CTRY_TAIWAN, APL3_FCCA, "TW", "TAIWAN" },
	//#ifdef HAVE_BUFFALO
	//      {CTRY_THAILAND, NULL1_FCCA, "TH", "THAILAND"},
	//#else
	{ CTRY_THAILAND, NULL1_WORLD, "TH", "THAILAND" },
	//#endif
	{ CTRY_TRINIDAD_Y_TOBAGO, ETSI4_WORLD, "TT", "TRINIDAD&TOBAGO" },
	{ CTRY_TUNISIA, ETSI3_WORLD, "TN", "TUNISIA" },
	{ CTRY_TURKEY, ETSI3_WORLD, "TR", "TURKEY" },
	{ CTRY_UKRAINE, NULL1_WORLD, "UA", "UKRAINE" },
	{ CTRY_UAE, NULL1_WORLD, "AE", "UNITED_ARAB_EMIRATES" },
	{ CTRY_UNITED_KINGDOM, ETSI1_WORLD, "GB", "UNITED_KINGDOM" },
	// Not assigned, use for UK_BFWA
	{ CTRY_UNITED_KINGDOM_BFWA, ETSI7_WORLD, "ZG", "UNITED_KINGDOM_BFWA" },
	{ CTRY_UNITED_STATES, FCC1_FCCA, "US", "UNITED_STATES" },
	{ CTRY_UNITED_STATES_FCC49, FCC4_FCCA, "PS", "UNITED_STATES_(PUBLIC_SAFETY)" },
	{ CTRY_URUGUAY, APL2_WORLD, "UY", "URUGUAY" },
	{ CTRY_UZBEKISTAN, FCC3_FCCA, "UZ", "UZBEKISTAN" },
	{ CTRY_VENEZUELA, APL2_ETSIC, "VE", "VENEZUELA" },
	{ CTRY_VIET_NAM, NULL1_WORLD, "VN", "VIETNAM" },
	{ CTRY_YEMEN, NULL1_WORLD, "YE", "YEMEN" },
	{ CTRY_ZIMBABWE, NULL1_WORLD, "ZW", "ZIMBABWE" }
};
#endif

#if defined(HAVE_NORTHSTAR) || defined(HAVE_80211AC)
static COUNTRY_CODE_TO_ENUM_RD allCountries[] = {
	{ CTRY_ALBANIA, NULL1_WORLD, "AF", "AFGHANISTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AL", "ALBANIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "DZ", "ALGERIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AS", "AMERICAN_SAMOA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AD", "ANDORRA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AO", "ANGOLA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AI", "ANGUILLA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AG", "ANTIGUA_AND_BARBUDA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AR", "ARGENTINA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AM", "ARMENIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AW", "ARUBA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AU", "AUSTRALIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AT", "AUSTRIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AZ", "AZERBAIJAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BS", "BAHAMAS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BH", "BAHRAIN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BD", "BANGLADESH" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BB", "BARBADOS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BY", "BELARUS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BE", "BELGIUM" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BZ", "BELIZE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BJ", "BENIN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BM", "BERMUDA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BT", "BHUTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BO", "BOLIVIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BA", "BOSNIA_AND_HERZEGOVINA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BW", "BOTSWANA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BR", "BRAZIL" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IO", "BRITISH_INDIAN_OCEAN_TERRITORY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BN", "BRUNEI_DARUSSALAM" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BG", "BULGARIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BF", "BURKINA_FASO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "BI", "BURUNDI" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KH", "CAMBODIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CM", "CAMEROON" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CA", "CANADA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CV", "CAPE_VERDE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KY", "CAYMAN_ISLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CF", "CENTRAL_AFRICAN_REPUBLIC" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TD", "CHAD" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CL", "CHILE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CN", "CHINA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CX", "CHRISTMAS_ISLAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CO", "COLOMBIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KM", "COMOROS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CD", "CONGO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CG", "CONGO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CK", "COOK_ISLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CR", "COSTA_RICA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CI", "COTE_D'IVOIRE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "HR", "CROATIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CY", "CYPRUS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CZ", "CZECH_REPUBLIC" },
	{ CTRY_ALBANIA, NULL1_WORLD, "DK", "DENMARK" },
	{ CTRY_ALBANIA, NULL1_WORLD, "DJ", "DJIBOUTI" },
	{ CTRY_ALBANIA, NULL1_WORLD, "DM", "DOMINICA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "DO", "DOMINICAN_REPUBLIC" },
	{ CTRY_ALBANIA, NULL1_WORLD, "EC", "ECUADOR" },
	{ CTRY_ALBANIA, NULL1_WORLD, "EG", "EGYPT" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SV", "EL_SALVADOR" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GQ", "EQUATORIAL_GUINEA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ER", "ERITREA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "EE", "ESTONIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ET", "ETHIOPIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "EU", "EUROPE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "FK", "FALKLAND_ISLANDS_(MALVINAS)" },
	{ CTRY_ALBANIA, NULL1_WORLD, "FO", "FAROE_ISLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "FJ", "FIJI" },
	{ CTRY_ALBANIA, NULL1_WORLD, "FI", "FINLAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "FR", "FRANCE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GF", "FRENCH_GUIANA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PF", "FRENCH_POLYNESIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TF", "FRENCH_SOUTHERN_TERRITORIES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GA", "GABON" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GM", "GAMBIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GE", "GEORGIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "DE", "GERMANY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GH", "GHANA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GI", "GIBRALTAR" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GR", "GREECE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GD", "GRENADA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GP", "GUADELOUPE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GU", "GUAM" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GT", "GUATEMALA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GG", "GUERNSEY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GN", "GUINEA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GW", "GUINEA-BISSAU" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GY", "GUYANA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "HT", "HAITI" },
	{ CTRY_ALBANIA, NULL1_WORLD, "VA", "HOLY_SEE_(VATICAN_CITY_STATE)" },
	{ CTRY_ALBANIA, NULL1_WORLD, "HN", "HONDURAS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "HK", "HONG_KONG" },
	{ CTRY_ALBANIA, NULL1_WORLD, "HU", "HUNGARY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IS", "ICELAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IN", "INDIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ID", "INDONESIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IQ", "IRAQ" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IE", "IRELAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IL", "ISRAEL" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IT", "ITALY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "JM", "JAMAICA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "JP", "JAPAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "JE", "JERSEY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "JO", "JORDAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KZ", "KAZAKHSTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KE", "KENYA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KI", "KIRIBATI" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KR", "KOREA,_REPUBLIC_OF" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KW", "KUWAIT" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KG", "KYRGYZSTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LA", "LAO_PEOPLE'S_DEMOCRATIC_REPUBLIC" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LV", "LATVIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LB", "LEBANON" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LS", "LESOTHO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LR", "LIBERIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LY", "LIBYAN_ARAB_JAMAHIRIYA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LI", "LIECHTENSTEIN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LT", "LITHUANIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LU", "LUXEMBOURG" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MO", "MACAO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MK", "MACEDONIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MG", "MADAGASCAR" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MW", "MALAWI" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MY", "MALAYSIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MV", "MALDIVES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ML", "MALI" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MT", "MALTA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "IM", "MAN,_ISLE_OF" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MQ", "MARTINIQUE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MR", "MAURITANIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MU", "MAURITIUS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "YT", "MAYOTTE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MX", "MEXICO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "FM", "MICRONESIA,_FEDERATED_STATES_OF" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MD", "MOLDOVA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MC", "MONACO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MN", "MONGOLIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ME", "MONTENEGRO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MS", "MONTSERRAT" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MA", "MOROCCO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MZ", "MOZAMBIQUE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MM", "MYANMAR" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NA", "NAMIBIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NR", "NAURU" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NP", "NEPAL" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AN", "NETHERLANDS_ANTILLES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NL", "NETHERLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NC", "NEW_CALEDONIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NZ", "NEW_ZEALAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NI", "NICARAGUA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NE", "NIGER" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NG", "NIGERIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NU", "NIUE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NF", "NORFOLK_ISLAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "MP", "NORTHERN_MARIANA_ISLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "NO", "NORWAY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "OM", "OMAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PK", "PAKISTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PW", "PALAU" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PA", "PANAMA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PG", "PAPUA_NEW_GUINEA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PY", "PARAGUAY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PE", "PERU" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PH", "PHILIPPINES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PL", "POLAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PT", "PORTUGAL" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PR", "PUERTO_RICO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "QA", "QATAR" },
	{ CTRY_ALBANIA, NULL1_WORLD, "RE", "REUNION" },
	{ CTRY_ALBANIA, NULL1_WORLD, "RO", "ROMANIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "RU", "RUSSIAN_FEDERATION" },
	{ CTRY_ALBANIA, NULL1_WORLD, "RW", "RWANDA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "KN", "SAINT_KITTS_AND_NEVIS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LC", "SAINT_LUCIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "PM", "SAINT_PIERRE_AND_MIQUELON" },
	{ CTRY_ALBANIA, NULL1_WORLD, "VC", "SAINT_VINCENT_AND_THE_GRENADINES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "WS", "SAMOA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SM", "SAN_MARINO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ST", "SAO_TOME_AND_PRINCIPE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SA", "SAUDI_ARABIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SN", "SENEGAL" },
	{ CTRY_ALBANIA, NULL1_WORLD, "RS", "SERBIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SC", "SEYCHELLES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SL", "SIERRA_LEONE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SG", "SINGAPORE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SK", "SLOVAKIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SI", "SLOVENIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SB", "SOLOMON_ISLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SO", "SOMALIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ZA", "SOUTH_AFRICA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ES", "SPAIN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "LK", "SRI_LANKA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SR", "SURINAME" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SZ", "SWAZILAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "SE", "SWEDEN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "CH", "SWITZERLAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TW", "TAIWAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TJ", "TAJIKISTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TZ", "TANZANIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TH", "THAILAND" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TG", "TOGO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TO", "TONGA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TT", "TRINIDAD_AND_TOBAGO" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TN", "TUNISIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TR", "TURKEY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TM", "TURKMENISTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TC", "TURKS_AND_CAICOS_ISLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "TV", "TUVALU" },
	{ CTRY_ALBANIA, NULL1_WORLD, "UG", "UGANDA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "UA", "UKRAINE" },
	{ CTRY_ALBANIA, NULL1_WORLD, "AE", "UNITED_ARAB_EMIRATES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "GB", "UNITED_KINGDOM" },
	{ CTRY_ALBANIA, NULL1_WORLD, "UM", "UNITED_STATES_MINOR_OUTLYING_ISLANDS" },
	{ CTRY_ALBANIA, NULL1_WORLD, "US", "UNITED_STATES" },
	{ CTRY_ALBANIA, NULL1_WORLD, "UY", "URUGUAY" },
	{ CTRY_ALBANIA, NULL1_WORLD, "UZ", "UZBEKISTAN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "VU", "VANUATU" },
	{ CTRY_ALBANIA, NULL1_WORLD, "VE", "VENEZUELA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "VN", "VIETNAM" },
	{ CTRY_ALBANIA, NULL1_WORLD, "VG", "VIRGIN_ISLANDS,_BRITISH" },
	{ CTRY_ALBANIA, NULL1_WORLD, "VI", "VIRGIN_ISLANDS,_U.S." },
	{ CTRY_ALBANIA, NULL1_WORLD, "WF", "WALLIS_AND_FUTUNA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "EH", "WESTERN_SAHARA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "YE", "YEMEN" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ZM", "ZAMBIA" },
	{ CTRY_ALBANIA, NULL1_WORLD, "ZW", "ZIMBABWE" },
};
#endif

#ifdef HAVE_BUFFALO
static char regionCountryCodes[10][31][3] = {
	{ "EU", "BG", "HR", "CY", "CZ", "DK", "EE", "FI", "FR", "F2", "DE", "GR", "HU", "IE", "IT", "LV",
	  "LI", "LT", "LU", "MK", "NL", "NO", "PL", "PT", "RO", "SK", "SI", "ES", "SE", "CH", "GB" },
	{ "US", "US", "CA", "MX", "GT", "HN", "SV", "CR", "PA", "VE", "EC", "CO", "", "", "", "",
	  "",	"",   "",   "",	  "",	"",   "",   "",	  "",	"",   "",   "",	  "", "", "" },
	{ "JP", "JP", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	  "",	"",   "", "", "", "", "", "", "", "", "", "", "", "", "" },
	{ "TW", "TW", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	  "",	"",   "", "", "", "", "", "", "", "", "", "", "", "", "" },
	{ "KR", "KR", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	  "",	"",   "", "", "", "", "", "", "", "", "", "", "", "", "" },
	{ "CH", "CN", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	  "",	"",   "", "", "", "", "", "", "", "", "", "", "", "", "" },
	{ "AP", "AU", "SG", "HK", "PH", "IN", "TH", "MY", "BR", "PU", "AR", "PA", "VE", "EC", "GT", "CR",
	  "HN", "SV", "CO", "",	  "",	"",   "",   "",	  "",	"",   "",   "",	  "",	"",   "" },
	{ "RU", "RU", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	  "",	"",   "", "", "", "", "", "", "", "", "", "", "", "", "" },
	{ "DE", "DE", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	  "",	"",   "", "", "", "", "", "", "", "", "", "", "", "", "" },
	{ "_D", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	  "",	"", "", "", "", "", "", "", "", "", "", "", "", "", "" }
};
#endif
unsigned int getRegDomain(char *country)
{
	int i;
	for (i = 0; i < N(allCountries); i++) {
		if (!strcmp(allCountries[i].name, country))
			return allCountries[i].regDmnEnum;
	}
	return 0;
}

unsigned int getCountry(char *country)
{
	int i;
	for (i = 0; i < N(allCountries); i++) {
		if (!strcmp(allCountries[i].name, country))
			return allCountries[i].countryCode;
	}
	return 0;
}

const char *getIsoName(char *country)
{
	int i;
#ifdef HAVE_SUPERCHANNEL
	int issuperchannel(void);

	int sc = issuperchannel();
#endif
	for (i = 0; i < N(allCountries); i++) {
		if (!strcmp(allCountries[i].name, country)) {
#ifdef HAVE_SUPERCHANNEL
			if (!sc && !strcmp(allCountries[i].isoName, "PS"))
				return "US";
#else
			if (!strcmp(allCountries[i].isoName, "PS"))
				return "US";
#endif
			return allCountries[i].isoName;
		}
	}
	return NULL;
}

const char *getCountryByIso(char *country)
{
	int i;
	for (i = 0; i < N(allCountries); i++) {
		if (!strcmp(allCountries[i].isoName, country))
			return allCountries[i].name;
	}
	return NULL;
}

extern int getRouterBrand(void);

#ifdef HAVE_BUFFALO
#if defined(HAVE_NORTHSTAR) || defined(HAVE_MT7620)
#define getUEnv(name) nvram_safe_get(name)
#else
extern void *getUEnv(char *name);
#endif

static int isValidCountry(char *region, char *country)
{
	int i, j;
	for (i = 0; i < 9; i++) {
		if (!strcmp(region, regionCountryCodes[i][0])) {
			for (j = 1; j < 31; j++) {
				//fprintf(stderr, "[%s] %s\n", country, regionCountryCodes[i][j]);
				if (!strcmp(country, regionCountryCodes[i][j])) {
					return 1;
				}
			}
			return 0;
		}
	}
	// fallback
	if (strcmp(region, "US") && !strcmp(country, "US")) {
		return 1;
	}
	return 0;
}
#endif

static char *countries = NULL;

static int checkfilter(char *filter, char *check, int sc)
{
#ifdef HAVE_SUPERCHANNEL
	if (!sc && !strcmp(check, "PS"))
		return 0;
#else
	if (!strcmp(check, "PS"))
		return 0;
#endif

	char *next;
	char var[32];
	if (!filter)
		return 1;
	foreach(var, filter, next)
	{
		if (!strcmp(var, check))
			return 1;
	}
	return 0;
}

char *getCountryList(char *filter)
{
	int i;
#ifdef HAVE_SUPERCHANNEL
	int issuperchannel(void);
	int sc = issuperchannel();
#else
	int sc = 0;
#endif
#ifdef HAVE_BUFFALO
	char country[80];
	char *region = getUEnv("region");
	if (!region || !*region) {
		region = "_D";
	}
#endif
	if (countries == NULL) {
		int count = 0;
		for (i = 0; i < N(allCountries); i++) {
			if (!checkfilter(filter, allCountries[i].isoName, sc))
				continue;
#ifdef HAVE_BUFFALO
			sprintf(country, "%s", allCountries[i].isoName);
			if (isValidCountry(region, country)) {
#elif HAVE_WIKINGS
			if (nvram_safe_get("wkregdomain") == NULL ||
			    !strcmp(nvram_safe_get("wkregdomain"), allCountries[i].isoName) ||
			    !strcmp(nvram_safe_get("wkregdomain"), "")) {
#endif
				count += strlen(allCountries[i].name) + 1;
#ifdef HAVE_BUFFALO
			}
#elif HAVE_WIKINGS
			}
#endif
		}
		count++;
		countries = safe_malloc(count);
		bzero(countries, count);
		for (i = 0; i < N(allCountries); i++) {
			if (!checkfilter(filter, allCountries[i].isoName, sc))
				continue;
#ifdef HAVE_BUFFALO
			sprintf(country, "%s", allCountries[i].isoName);
			if (isValidCountry(region, country)) {
#elif HAVE_WIKINGS
			if (nvram_safe_get("wkregdomain") == NULL ||
			    !strcmp(nvram_safe_get("wkregdomain"), allCountries[i].isoName) ||
			    !strcmp(nvram_safe_get("wkregdomain"), "")) {
#endif
				strcat(countries, allCountries[i].name);
				strcat(countries, " ");

#ifdef HAVE_BUFFALO
			}
#elif HAVE_WIKINGS
			}
#endif
		}
	}
	return countries;
}
