/*

  Broadcom BCM43xx SPROM data modification tool

  Copyright (c) 2006 Michael Buesch <mbuesch@freenet.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
  Boston, MA 02110-1301, USA.

*/

#ifndef BCM43xx_SPROMTOOL_H_
#define BCM43xx_SPROMTOOL_H_

#include "utils.h"

#define VERSION			bcm43xx_stringify(VERSION_)

#define SPROM_SIZE		128 /* bytes */

/* byte offsets */
#define SPROM_SUBP		(0x02 * 2)
#define SPROM_SUBV		(0x03 * 2)
#define SPROM_PPID		(0x04 * 2)
#define SPROM_BFLHI		(0x1C * 2)
#define SPROM_IL0MACADDR	(0x24 * 2)
#define SPROM_ET0MACADDR	(0x27 * 2)
#define SPROM_ET1MACADDR	(0x2a * 2)
#define SPROM_ETHPHY		(0x2d * 2)
#define SPROM_BOARDREV		(0x2e * 2)
#define SPROM_PA0B0		(0x2f * 2)
#define SPROM_PA0B1		(0x30 * 2)
#define SPROM_PA0B2		(0x31 * 2)
#define SPROM_WL0GPIO0		(0x32 * 2)
#define SPROM_WL0GPIO2		(0x33 * 2)
#define SPROM_MAXPWR		(0x34 * 2)
#define SPROM_PA1B0		(0x35 * 2)
#define SPROM_PA1B1		(0x36 * 2)
#define SPROM_PA1B2		(0x37 * 2)
#define SPROM_IDL_TSSI_TGT	(0x38 * 2)
#define SPROM_BOARDFLAGS	(0x39 * 2)
#define SPROM_ANTENNA_GAIN	(0x3a * 2)
#define SPROM_VERSION		(0x3f * 2)

enum valuetype {
	VALUE_RAW,

	VALUE_SUBP,
	VALUE_SUBV,
	VALUE_PPID,
	VALUE_BFLHI,
	VALUE_BFL,
	VALUE_BGMAC,
	VALUE_ETMAC,
	VALUE_AMAC,
	VALUE_ET0PHY,
	VALUE_ET1PHY,
	VALUE_ET0MDC,
	VALUE_ET1MDC,
	VALUE_BREV,
	VALUE_LOC,
	VALUE_ANTA0,
	VALUE_ANTA1,
	VALUE_ANTBG0,
	VALUE_ANTBG1,
	VALUE_ANTGA,
	VALUE_ANTGBG,
	VALUE_PA0B0,
	VALUE_PA0B1,
	VALUE_PA0B2,
	VALUE_PA1B0,
	VALUE_PA1B1,
	VALUE_PA1B2,
	VALUE_WL0GPIO0,
	VALUE_WL0GPIO1,
	VALUE_WL0GPIO2,
	VALUE_WL0GPIO3,
	VALUE_MAXPA,
	VALUE_MAXPBG,
	VALUE_ITSSIA,
	VALUE_ITSSIBG,
	VALUE_SVER,
};

struct cmdline_vparm {
	enum valuetype type;
	int set;
	int bits;
	union {
		uint16_t value;
		uint8_t mac[6];
		struct {
			uint8_t value;
			uint8_t offset;
		} raw;
	} u;
};

struct cmdline_args {
	const char *infile;
	const char *outfile;
	int verbose;
	int force;
	int hex_mode;

#define MAX_VPARM	512
	struct cmdline_vparm vparm[MAX_VPARM];
	int nr_vparm;
};
extern struct cmdline_args cmdargs;

#endif /* BCM43xx_SPROMTOOL_H_ */
