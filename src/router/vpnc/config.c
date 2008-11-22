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

   $Id: config.c 315 2008-06-16 15:25:04Z Joerg Mayer $
*/

#define _GNU_SOURCE

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>

#include <gcrypt.h>

#include "sysdep.h"
#include "config.h"
#include "vpnc.h"
#include "supp.h"
#include "decrypt-utils.h"

const char *config[LAST_CONFIG];

int opt_debug = 0;
int opt_nd;
int opt_1des, opt_no_encryption, opt_auth_mode;
enum natt_mode_enum opt_natt_mode;
enum vendor_enum opt_vendor;
enum if_mode_enum opt_if_mode;
uint16_t opt_udpencapport;

void hex_dump(const char *str, const void *data, ssize_t len, const struct debug_strings *decode)
{
	size_t i;
	const uint8_t *p = data;
	const char *decodedval;

	if (opt_debug < 3)
		return;

	printf("   ");
	switch (len) {
	case DUMP_UINT8:
		decodedval = val_to_string(*(uint8_t *)p, decode);
		printf("%s: %02x%s\n", str, *(uint8_t *)p, decodedval);
		return;
	case DUMP_UINT16:
		decodedval = val_to_string(*(uint16_t *)p, decode);
		printf("%s: %04x%s\n", str, *(uint16_t *)p, decodedval);
		return;
	case DUMP_UINT32:
		decodedval = val_to_string(*(uint32_t *)p, decode);
		printf("%s: %08x%s\n", str, *(uint32_t *)p, decodedval);
		return;
	}

	printf("%s:%s", str, (len <= 16) ? " " : "\n   ");
	for (i = 0; i < (size_t)len; i++) {
		if (i && !(i % 32))
			printf("\n   ");
		else if (i && !(i % 4))
			printf(" ");
		printf("%02x", p[i]);
	}
	printf("\n");
}

static void config_deobfuscate(int obfuscated, int clear)
{
	int ret, len = 0;
	char *bin = NULL;
	
	if (config[obfuscated] == NULL)
		return;
	
	if (config[clear] != NULL) {
		config[obfuscated] = NULL;
		error(0, 0, "warning: ignoring obfuscated password because cleartext password set");
		return;
	}
	
	ret = hex2bin(config[obfuscated], &bin, &len);
	if (ret != 0) {
		error(1, 0, "error: deobfuscating of password failed (input not a hex string)");
	}
	
	ret = deobfuscate(bin, len, config+clear, NULL);
	free(bin);
	if (ret != 0) {
		error(1, 0, "error: deobfuscating of password failed");
	}
	
	config[obfuscated] = NULL;
	return;
}

static const char *config_def_ike_dh(void)
{
	return "dh2";
}

static const char *config_def_pfs(void)
{
	return "server";
}

static const char *config_def_local_addr(void)
{
	return "0.0.0.0";
}

static const char *config_def_local_port(void)
{
	return "500";
}

static const char *config_def_if_mode(void)
{
	return "tun";
}

static const char *config_def_natt_mode(void)
{
	return "natt";
}

static const char *config_def_udp_port(void)
{
	return "10000";
}

static const char *config_def_dpd_idle(void)
{
	return "300";
}

static const char *config_ca_dir(void)
{
	return "/etc/ssl/certs";
}

static const char *config_def_auth_mode(void)
{
	return "psk";
}

static const char *config_def_app_version(void)
{
	struct utsname uts;
	char *version;

	uname(&uts);
	asprintf(&version, "Cisco Systems VPN Client %s:%s", VERSION, uts.sysname);
	return version;
}

static const char *config_def_script(void)
{
	return "/etc/vpnc/vpnc-script";
}

static const char *config_def_pid_file(void)
{
	return "/var/run/vpnc/pid";
}

static const char *config_def_vendor(void)
{
	return "cisco";
}

static const char *config_def_target_network(void)
{
	return "0.0.0.0/0.0.0.0";
}

static const struct config_names_s {
	enum config_enum nm;
	const int needsArgument;
	const int long_only;
	const char *option;
	const char *name;
	const char *type;
	const char *desc;
	const char *(*get_def) (void);
} config_names[] = {
	/* Note: broken config file parser does NOT support option
	 * names where one is a prefix of another option. Needs just a bit work to
	 * fix the parser to care about ' ' or '\t' after the wanted
	 * option... */
	{
		CONFIG_IPSEC_GATEWAY, 1, 0,
		"--gateway",
		"IPSec gateway ",
		"<ip/hostname>",
		"IP/name of your IPSec gateway",
		NULL
	}, {
		CONFIG_IPSEC_ID, 1, 0,
		"--id",
		"IPSec ID ",
		"<ASCII string>",
		"your group name",
		NULL
	}, {
		CONFIG_IPSEC_SECRET, 1, 0,
		NULL,
		"IPSec secret ",
		"<ASCII string>",
		"your group password (cleartext)",
		NULL
	}, {
		CONFIG_IPSEC_SECRET_OBF, 1, 1,
		NULL,
		"IPSec obfuscated secret ",
		"<hex string>",
		"your group password (obfuscated)",
		NULL
	}, {
		CONFIG_XAUTH_USERNAME, 1, 0,
		"--username",
		"Xauth username ",
		"<ASCII string>",
		"your username",
		NULL
	}, {
		CONFIG_XAUTH_PASSWORD, 1, 0,
		NULL,
		"Xauth password ",
		"<ASCII string>",
		"your password (cleartext)",
		NULL
	}, {
		CONFIG_XAUTH_PASSWORD_OBF, 1, 1,
		NULL,
		"Xauth obfuscated password ",
		"<hex string>",
		"your password (obfuscated)",
		NULL
	}, {
		CONFIG_DOMAIN, 1, 1,
		"--domain",
		"Domain ",
		"<ASCII string>",
		"(NT-) Domain name for authentication",
		NULL
	}, {
		CONFIG_XAUTH_INTERACTIVE, 0, 1,
		"--xauth-inter",
		"Xauth interactive",
		NULL,
		"enable interactive extended authentication (for challenge response auth)",
		NULL
	}, {
		CONFIG_VENDOR, 1, 1,
		"--vendor",
		"Vendor ",
		"<cisco/netscreen>",
		"vendor of your IPSec gateway",
		config_def_vendor
	}, {
		CONFIG_NATT_MODE, 1, 1,
		"--natt-mode",
		"NAT Traversal Mode ",
		"<natt/none/force-natt/cisco-udp>",
		"Which NAT-Traversal Method to use:\n"
		" * natt -- NAT-T as defined in RFC3947\n"
		" * none -- disable use of any NAT-T method\n"
		" * force-natt -- always use NAT-T encapsulation even\n"
		"                 without presence of a NAT device\n"
		"                 (useful if the OS captures all ESP traffic)\n"
		" * cisco-udp -- Cisco proprietary UDP encapsulation, commonly over Port 10000\n"
		"Note: cisco-tcp encapsulation is not yet supported\n",
		config_def_natt_mode
	}, {
		CONFIG_SCRIPT, 1, 1,
		"--script",
		"Script ",
		"<command>",
		"command is executed using system() to configure the interface,\n"
		"routing and so on. Device name, IP, etc. are passed using enviroment\n"
		"variables, see README. This script is executed right after ISAKMP is\n"
		"done, but before tunneling is enabled. It is called when vpnc\n"
		"terminates, too\n",
		config_def_script
	}, {
		CONFIG_IKE_DH, 1, 1,
		"--dh",
		"IKE DH Group ",
		"<dh1/dh2/dh5>",
		"name of the IKE DH Group",
		config_def_ike_dh
	}, {
		CONFIG_IPSEC_PFS, 1, 1,
		"--pfs",
		"Perfect Forward Secrecy ",
		"<nopfs/dh1/dh2/dh5/server>",
		"Diffie-Hellman group to use for PFS",
		config_def_pfs
	}, {
		CONFIG_ENABLE_1DES, 0, 1,
		"--enable-1des",
		"Enable Single DES",
		NULL,
		"enables weak single DES encryption",
		NULL
	}, {
		CONFIG_ENABLE_NO_ENCRYPTION, 0, 1,
		"--enable-no-encryption",
		"Enable no encryption",
		NULL,
		"enables using no encryption for data traffic (key exchanged must be encrypted)",
		NULL
	}, {
		CONFIG_VERSION, 1, 1,
		"--application-version",
		"Application version ",
		"<ASCII string>",
		"Application Version to report. Note: Default string is generated at runtime.",
		config_def_app_version
	}, {
		CONFIG_IF_NAME, 1, 1,
		"--ifname",
		"Interface name ",
		"<ASCII string>",
		"visible name of the TUN/TAP interface",
		NULL
	}, {
		CONFIG_IF_MODE, 1, 1,
		"--ifmode",
		"Interface mode ",
		"<tun/tap>",
		"mode of TUN/TAP interface:\n"
		" * tun: virtual point to point interface (default)\n"
		" * tap: virtual ethernet interface\n",
		config_def_if_mode
	}, {
		CONFIG_DEBUG, 1, 1,
		"--debug",
		"Debug ",
		"<0/1/2/3/99>",
		"Show verbose debug messages\n"
		" *  0: Do not print debug information.\n"
		" *  1: Print minimal debug information.\n"
		" *  2: Show statemachine and packet/payload type information.\n"
		" *  3: Dump everything exluding authentication data.\n"
		" * 99: Dump everything INCLUDING AUTHENTICATION data (e.g. PASSWORDS).\n",
		NULL
	}, {
		CONFIG_ND, 0, 1,
		"--no-detach",
		"No Detach",
		NULL,
		"Don't detach from the console after login",
		NULL
	}, {
		CONFIG_PID_FILE, 1, 1,
		"--pid-file",
		"Pidfile ",
		"<filename>",
		"store the pid of background process in <filename>",
		config_def_pid_file
	}, {
		CONFIG_LOCAL_ADDR, 1, 1,
		"--local-addr",
		"Local Addr ",
		"<ip/hostname>",
		"local IP to use for ISAKMP / ESP / ... (0.0.0.0 == automatically assign)",
		config_def_local_addr
	}, {
		CONFIG_LOCAL_PORT, 1, 1,
		"--local-port",
		"Local Port ",
		"<0-65535>",
		"local ISAKMP port number to use (0 == use random port)",
		config_def_local_port
	}, {
		CONFIG_UDP_ENCAP_PORT, 1, 1,
		"--udp-port",
		"Cisco UDP Encapsulation Port ",
		"<0-65535>",
		"Local UDP port number to use (0 == use random port).\n"
		"This is only relevant if cisco-udp nat-traversal is used.\n"
		"This is the _local_ port, the remote udp port is discovered automatically.\n"
		"It is especially not the cisco-tcp port.\n",
		config_def_udp_port
	}, {
		CONFIG_DPD_IDLE, 1, 1,
		"--dpd-idle",
		"DPD idle timeout (our side) ",
		"<0,10-86400>",
		"Send DPD packet after not receiving anything for <idle> seconds.\n"
		"Use 0 to disable DPD completely (both ways).\n",
		config_def_dpd_idle
	}, {
		CONFIG_NON_INTERACTIVE, 0, 1,
		"--non-inter",
		"Noninteractive",
		NULL,
		"Don't ask anything, exit on missing options",
		NULL
	}, {
 		CONFIG_AUTH_MODE, 1, 1,
		"--auth-mode",
		"IKE Authmode ",
		"<psk/cert/hybrid>",
		"Authentication mode:\n"
		" * psk:    pre-shared key (default)\n"
		" * cert:   server + client certificate (not implemented yet)\n"
		" * hybrid: server certificate + xauth (if built with openssl support)\n",
		config_def_auth_mode
	}, {
		CONFIG_CA_FILE, 1, 1,
		"--ca-file",
		"CA-File ",
		"<filename>",
		"filename and path to the CA-PEM-File",
		NULL
	}, {
		CONFIG_CA_DIR, 1, 1,
		"--ca-dir",
		"CA-Dir ",
		"<directory>",
		"path of the trusted CA-Directory",
		config_ca_dir
	}, {
		CONFIG_IPSEC_TARGET_NETWORK, 1, 1,
		"--target-network",
		"IPSEC target network ",
		"<target network/netmask>",
		"Target network in dotted decimal or CIDR notation\n",
		config_def_target_network
	}, {
		0, 0, 0, NULL, NULL, NULL, NULL, NULL
	}
};

static char *get_config_filename(const char *name, int add_dot_conf)
{
	char *realname;
	
	asprintf(&realname, "%s%s%s", index(name, '/') ? "" : "/etc/vpnc/", name, add_dot_conf ? ".conf" : "");
	return realname;
}

static void read_config_file(const char *name, const char **configs, int missingok)
{
	FILE *f;
	char *line = NULL;
	size_t line_length = 0;
	int linenum = 0;
	char *realname;

	if (!strcmp(name, "-")) {
		f = stdin;
		realname = strdup("stdin");
	} else {
		realname = get_config_filename(name, 0);
		f = fopen(realname, "r");
		if (f == NULL && errno == ENOENT) {
			free(realname);
			realname = get_config_filename(name, 1);
			f = fopen(realname, "r");
		}
		if (missingok && f == NULL && errno == ENOENT) {
			free(realname);
			return;
		}
		if (f == NULL)
			error(1, errno, "couldn't open `%s'", realname);
	}
	for (;;) {
		ssize_t llen;
		int i;

		llen = getline(&line, &line_length, f);
		if (llen == -1 && feof(f))
			break;
		if (llen == -1)
			error(1, errno, "reading `%s'", realname);
		if (llen > 0 && line[llen - 1] == '\n')
			line[--llen] = 0;
		if (llen > 0 && line[llen - 1] == '\r')
			line[--llen] = 0;
		linenum++;
		for (i = 0; config_names[i].name != NULL; i++) {
			if (strncasecmp(config_names[i].name, line,
					strlen(config_names[i].name)) == 0) {
				/* boolean implementation, using harmless pointer targets as true */
				if (!config_names[i].needsArgument) {
					configs[config_names[i].nm] = config_names[i].name;
					break;
				}
				if (configs[config_names[i].nm] == NULL)
					configs[config_names[i].nm] =
						strdup(line + strlen(config_names[i].name));
				if (configs[config_names[i].nm] == NULL)
					error(1, errno, "can't allocate memory");
				break;
			}
		}
		if (config_names[i].name == NULL && line[0] != '#' && line[0] != 0)
			error(0, 0, "warning: unknown configuration directive in %s at line %d",
				realname, linenum);
	}
	free(line);
	free(realname);
	if (strcmp(name, "-"))
		fclose(f);
}

static void print_desc(const char *pre, const char *text)
{
	const char *p, *q;

	for (p = text, q = strchr(p, '\n'); q; p = q+1, q = strchr(p, '\n'))
		printf("%s%.*s\n", pre, (int)(q-p), p);

	if (*p != '\0')
		printf("%s%s\n", pre, p);
}

static void print_usage(char *argv0, int print_level)
{
	int c;

	printf("Usage: %s [--version] [--print-config] [--help] [--long-help] [options] [config files]\n\n",
		argv0);
	printf("Options:\n");
	for (c = 0; config_names[c].name != NULL; c++) {
		if (config_names[c].long_only > print_level)
			continue;

		printf("  %s %s\n", (config_names[c].option == NULL ?
				"(configfile only option)" : config_names[c].option),
			((config_names[c].type == NULL || config_names[c].option == NULL) ?
				"" : config_names[c].type));

		print_desc("      ", config_names[c].desc);

		if (config_names[c].get_def != NULL)
			printf("    Default: %s\n", config_names[c].get_def());

		printf("  conf-variable: %s%s\n", config_names[c].name,
			(config_names[c].type == NULL ? "" : config_names[c].type));

		printf("\n");
	}
	
	if (!print_level)
		printf("Use --long-help to see all options\n\n");
	
	printf("Report bugs to vpnc@unix-ag.uni-kl.de\n");
}

static void print_version(void)
{
	unsigned int i;

	printf("vpnc version " VERSION "\n");
	printf("Copyright (C) 2002-2006 Geoffrey Keating, Maurice Massar, others\n");
	printf("vpnc comes with NO WARRANTY, to the extent permitted by law.\n"
		"You may redistribute copies of vpnc under the terms of the GNU General\n"
		"Public License.  For more information about these matters, see the files\n"
		"named COPYING.\n");
#ifdef OPENSSL_GPL_VIOLATION
	printf("Built with openssl (certificate) support. Be aware of the\n"
		"license implications.\n");
#else /* OPENSSL_GPL_VIOLATION */
	printf("Built without openssl (certificate) support.\n");
#endif /* OPENSSL_GPL_VIOLATION */
	printf("\n");

	printf("Supported DH-Groups:");
	for (i = 0; supp_dh_group[i].name != NULL; i++)
		printf(" %s", supp_dh_group[i].name);
	printf("\n");

	printf("Supported Hash-Methods:");
	for (i = 0; supp_hash[i].name != NULL; i++)
		printf(" %s", supp_hash[i].name);
	printf("\n");

	printf("Supported Encryptions:");
	for (i = 0; supp_crypt[i].name != NULL; i++)
		printf(" %s", supp_crypt[i].name);
	printf("\n");

	printf("Supported Auth-Methods:");
	for (i = 0; supp_auth[i].name != NULL; i++)
		printf(" %s", supp_auth[i].name);
	printf("\n");
}

void do_config(int argc, char **argv)
{
	char *s;
	int i, c, known;
	int got_conffile = 0, print_config = 0;
	size_t s_len;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] && (argv[i][0] != '-' || argv[i][1] == '\0')) {
			read_config_file(argv[i], config, 0);
			got_conffile = 1;
			continue;
		}

		known = 0;

		for (c = 0; config_names[c].name != NULL && !known; c++) {
			if (config_names[c].option == NULL
				|| strncmp(argv[i], config_names[c].option,
					strlen(config_names[c].option)) != 0)
				continue;

			s = NULL;

			known = 1;
			if (argv[i][strlen(config_names[c].option)] == '=')
				s = argv[i] + strlen(config_names[c].option) + 1;
			else if (argv[i][strlen(config_names[c].option)] == 0) {
				if (config_names[c].needsArgument) {
					if (i + 1 < argc)
						s = argv[++i];
					else
						known = 0;
				} else
					s = argv[i]; /* no arg, fill in something */
			} else
				known = 0;
			if (known)
				config[config_names[c].nm] = s;
		}

		if (!known && strcmp(argv[i], "--version") == 0) {
			print_version();
			exit(0);
		}
		if (!known && strcmp(argv[i], "--print-config") == 0) {
			print_config = 1;
			known = 1;
		}
		if (!known && strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0], 0);
			exit(0);
		}
		if (!known && strcmp(argv[i], "--long-help") == 0) {
			print_usage(argv[0], 1);
			exit(0);
		}
		if (!known) {
			printf("%s: unknown option %s\n\n", argv[0], argv[i]);

			print_usage(argv[0], 1);
			exit(1);
		}
	}
	
	if (!got_conffile) {
		read_config_file("/etc/vpnc/default.conf", config, 1);
		read_config_file("/etc/vpnc.conf", config, 1);
	}
	
	if (!print_config) {
		for (i = 0; config_names[i].name != NULL; i++)
			if (!config[config_names[i].nm]
				&& config_names[i].get_def != NULL)
				config[config_names[i].nm] = config_names[i].get_def();
		
		opt_debug = (config[CONFIG_DEBUG]) ? atoi(config[CONFIG_DEBUG]) : 0;
		opt_nd = (config[CONFIG_ND]) ? 1 : 0;
		opt_1des = (config[CONFIG_ENABLE_1DES]) ? 1 : 0;

		if (!strcmp(config[CONFIG_AUTH_MODE], "psk")) {
			opt_auth_mode = AUTH_MODE_PSK;
		} else if (!strcmp(config[CONFIG_AUTH_MODE], "cert")) {
			opt_auth_mode = AUTH_MODE_CERT;
		} else if (!strcmp(config[CONFIG_AUTH_MODE], "hybrid")) {
			opt_auth_mode = AUTH_MODE_HYBRID;
		} else {
			printf("%s: unknown authentication mode %s\nknown modes: psk cert hybrid\n", argv[0], config[CONFIG_AUTH_MODE]);
			exit(1);
		}
#ifndef OPENSSL_GPL_VIOLATION
		if (opt_auth_mode == AUTH_MODE_HYBRID ||
			opt_auth_mode == AUTH_MODE_CERT) {
			printf("%s was built without openssl: Can't do hybrid or cert mode.\n", argv[0]);
			exit(1);
		}
#endif
		opt_no_encryption = (config[CONFIG_ENABLE_NO_ENCRYPTION]) ? 1 : 0;
		opt_udpencapport=atoi(config[CONFIG_UDP_ENCAP_PORT]);
		
		if (!strcmp(config[CONFIG_NATT_MODE], "natt")) {
			opt_natt_mode = NATT_NORMAL;
		} else if (!strcmp(config[CONFIG_NATT_MODE], "none")) {
			opt_natt_mode = NATT_NONE;
		} else if (!strcmp(config[CONFIG_NATT_MODE], "force-natt")) {
			opt_natt_mode = NATT_FORCE;
		} else if (!strcmp(config[CONFIG_NATT_MODE], "cisco-udp")) {
			opt_natt_mode = NATT_CISCO_UDP;
		} else {
			printf("%s: unknown nat traversal mode %s\nknown modes: natt none force-natt cisco-udp\n", argv[0], config[CONFIG_NATT_MODE]);
			exit(1);
		}
		
		if (!strcmp(config[CONFIG_IF_MODE], "tun")) {
			opt_if_mode = IF_MODE_TUN;
		} else if (!strcmp(config[CONFIG_IF_MODE], "tap")) {
			opt_if_mode = IF_MODE_TAP;
		} else {
			printf("%s: unknown interface mode %s\nknown modes: tun tap\n", argv[0], config[CONFIG_IF_MODE]);
			exit(1);
		}
		
		if (!strcmp(config[CONFIG_VENDOR], "cisco")) {
			opt_vendor = VENDOR_CISCO;
		} else if (!strcmp(config[CONFIG_VENDOR], "netscreen")) {
			opt_vendor = VENDOR_NETSCREEN;
		} else {
			printf("%s: unknown vendor %s\nknown vendors: cisco netscreen\n", argv[0], config[CONFIG_VENDOR]);
			exit(1);
		}
	}
	
	if (opt_debug >= 99) {
		printf("WARNING! active debug level is >= 99, output includes username and password (hex encoded)\n");
		fprintf(stderr,
			"WARNING! active debug level is >= 99, output includes username and password (hex encoded)\n");
	}
	
	config_deobfuscate(CONFIG_IPSEC_SECRET_OBF, CONFIG_IPSEC_SECRET);
	config_deobfuscate(CONFIG_XAUTH_PASSWORD_OBF, CONFIG_XAUTH_PASSWORD);
	
	for (i = 0; i < LAST_CONFIG; i++) {
		if (config[i] != NULL || config[CONFIG_NON_INTERACTIVE] != NULL)
			continue;
		if (config[CONFIG_XAUTH_INTERACTIVE] && i == CONFIG_XAUTH_PASSWORD)
			continue;
		
		s = NULL;
		s_len = 0;

		switch (i) {
		case CONFIG_IPSEC_GATEWAY:
			printf("Enter IPSec gateway address: ");
			break;
		case CONFIG_IPSEC_ID:
			printf("Enter IPSec ID for %s: ", config[CONFIG_IPSEC_GATEWAY]);
			break;
		case CONFIG_IPSEC_SECRET:
			printf("Enter IPSec secret for %s@%s: ",
				config[CONFIG_IPSEC_ID], config[CONFIG_IPSEC_GATEWAY]);
			break;
		case CONFIG_XAUTH_USERNAME:
			printf("Enter username for %s: ", config[CONFIG_IPSEC_GATEWAY]);
			break;
		case CONFIG_XAUTH_PASSWORD:
			printf("Enter password for %s@%s: ",
				config[CONFIG_XAUTH_USERNAME],
				config[CONFIG_IPSEC_GATEWAY]);
			break;
		}
		fflush(stdout);
		switch (i) {
		case CONFIG_IPSEC_SECRET:
		case CONFIG_XAUTH_PASSWORD:
			s = strdup(getpass(""));
			break;
		case CONFIG_IPSEC_GATEWAY:
		case CONFIG_IPSEC_ID:
		case CONFIG_XAUTH_USERNAME:
			getline(&s, &s_len, stdin);
		}
		if (s != NULL && strlen(s) > 0 && s[strlen(s) - 1] == '\n')
			s[strlen(s) - 1] = 0;
		config[i] = s;
	}

	if (print_config) {
		fprintf(stderr, "vpnc.conf:\n\n");
		for (i = 0; config_names[i].name != NULL; i++) {
			if (config[config_names[i].nm] == NULL)
				continue;
			printf("%s%s\n", config_names[i].name,
				config_names[i].needsArgument ?
					config[config_names[i].nm] : "");
		}
		exit(0);
	}

	if (!config[CONFIG_IPSEC_GATEWAY])
		error(1, 0, "missing IPSec gatway address");
	if (!config[CONFIG_IPSEC_ID])
		error(1, 0, "missing IPSec ID");
	if (!config[CONFIG_IPSEC_SECRET])
		error(1, 0, "missing IPSec secret");
	if (!config[CONFIG_XAUTH_USERNAME])
		error(1, 0, "missing Xauth username");
	if (!config[CONFIG_XAUTH_PASSWORD] && !config[CONFIG_XAUTH_INTERACTIVE])
		error(1, 0, "missing Xauth password");
	if (get_dh_group_ike() == NULL)
		error(1, 0, "IKE DH Group \"%s\" unsupported\n", config[CONFIG_IKE_DH]);
	if (get_dh_group_ipsec(-1) == NULL)
		error(1, 0, "Perfect Forward Secrecy \"%s\" unsupported\n",
			config[CONFIG_IPSEC_PFS]);
	if (get_dh_group_ike()->ike_sa_id == 0)
		error(1, 0, "IKE DH Group must not be nopfs\n");

	return;
}
