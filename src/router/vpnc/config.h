/* IPSec VPN client compatible with Cisco equipment.
   Copyright (C) 2004-2005 Maurice Massar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: config.h 117 2007-02-08 02:55:59Z Maurice Massar $
*/

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <unistd.h>
#include <inttypes.h>

#include "vpnc-debug.h"

enum config_enum {
	CONFIG_NONE,
	CONFIG_SCRIPT,
	CONFIG_DEBUG,
	CONFIG_DOMAIN,
	CONFIG_ENABLE_1DES,
	CONFIG_ENABLE_NO_ENCRYPTION,
	CONFIG_ND,
	CONFIG_NON_INTERACTIVE,
	CONFIG_PID_FILE,
	CONFIG_LOCAL_PORT,
	CONFIG_VERSION,
	CONFIG_IF_NAME,
	CONFIG_IF_MODE,
	CONFIG_IKE_DH,
	CONFIG_IPSEC_PFS,
	CONFIG_IPSEC_GATEWAY,
	CONFIG_IPSEC_ID,
	CONFIG_IPSEC_SECRET,
	CONFIG_IPSEC_SECRET_OBF,
	CONFIG_XAUTH_USERNAME,
	CONFIG_XAUTH_PASSWORD,
	CONFIG_XAUTH_PASSWORD_OBF,
	CONFIG_XAUTH_INTERACTIVE,
	CONFIG_VENDOR,
	CONFIG_NATT_MODE,
	CONFIG_UDP_ENCAP_PORT,
	LAST_CONFIG
};

enum hex_dump_enum {
	DUMP_UINT8 = -1,
	DUMP_UINT16 = -2,
	DUMP_UINT32 = -4
};

enum vendor_enum {
	VENDOR_CISCO,
	VENDOR_NETSCREEN
};

enum natt_mode_enum {
	NATT_NONE,
	NATT_NORMAL,
	NATT_FORCE,
	NATT_CISCO_UDP
};

enum if_mode_enum {
	IF_MODE_TUN,
	IF_MODE_TAP
};

extern const char *config[LAST_CONFIG];

extern enum vendor_enum opt_vendor;
extern int opt_debug;
extern int opt_nd;
extern int opt_1des, opt_no_encryption;
extern enum natt_mode_enum opt_natt_mode;
extern enum if_mode_enum opt_if_mode;
extern uint16_t opt_udpencapport;

#define DEBUG(lvl, a) do {if (opt_debug >= (lvl)) {a;}} while (0)

extern void hex_dump(const char *str, const void *data, ssize_t len, const struct debug_strings *decode);
extern void do_config(int argc, char **argv);
extern int hex2bin(const char *str, char **bin, int *len);
extern int deobfuscate(char *ct, int len, const char **resp, char *reslenp);

#endif
