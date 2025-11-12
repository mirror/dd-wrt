/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
 \********************************************************************/

/** @file conf.c
  @brief Config file parsing
  @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
  @author Copyright (C) 2007 Paul Kube <nodogsplash@kokoro.ucsd.edu>
  @author Copyright (C) 2015-2025 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#include <pthread.h>

#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include "common.h"
#include "safe.h"
#include "debug.h"
#include "conf.h"
#include "auth.h"
#include "util.h"
#include "http_microhttpd_utils.h"
#include "fw_iptables.h"
#include "commandline.h"

/** @internal
 * Holds the current configuration of the gateway */
static s_config config = {{0}};

/**
 * Mutex for the configuration file, used by the auth_servers related
 * functions. */
pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;


/** Accessor for the current gateway configuration
@return:  A pointer to the current config.  The pointer isn't opaque, but should be treated as READ-ONLY
 */
s_config *
config_get_config(void)
{
	return &config;
}

char *set_list_str(char *list, const char *default_list, char *debug_level)
{
	char msg[MID_BUF];
	char debuglevel[STATUS_BUF];

	memset(msg, 0, MID_BUF);
	memset(debuglevel, 0, STATUS_BUF);

	get_list_from_config(msg, MID_BUF, list);

	if (strcmp(msg, "") == 0) {

		if (strcmp(debug_level, "3") == 0 || (strcmp(debug_level, "2") == 0)) {
			openlog ("opennds", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
			syslog (LOG_INFO, "list %s is [ %s ]", list, default_list);
			closelog ();
		}

		return strdup(default_list);
	} else {

		if (strcmp(debug_level, "3") == 0 || (strcmp(debug_level, "2") == 0)) {
			openlog ("opennds", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
			syslog (LOG_INFO, "list %s is [ %s ]", list, msg);
			closelog ();
		}

		return safe_strdup(msg);
	}
}

char *set_option_str(char *option, const char *default_option, char *debug_level)
{
	char msg[SMALL_BUF];
	char debuglevel[STATUS_BUF];

	memset(msg, 0, SMALL_BUF);
	memset(debuglevel, 0, STATUS_BUF);

	get_option_from_config(msg, SMALL_BUF, option);

	if (strcmp(msg, "") == 0) {

		if (strcmp(debug_level, "3") == 0 || (strcmp(debug_level, "2") == 0)) {
			openlog ("opennds", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
			syslog (LOG_INFO, "option %s is [ %s ]", option, default_option);
			closelog ();
		}

		return strdup(default_option);
	} else {

		if (strcmp(debug_level, "3") == 0 || (strcmp(debug_level, "2") == 0)) {
			openlog ("opennds", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
			syslog (LOG_INFO, "option %s is [ %s ]", option, msg);
			closelog ();
		}

		return safe_strdup(msg);
	}
}


// Sets the default config parameters and initialises the configuration system
void
config_init(int argc, char **argv)
{
	const char *gatewayname;
	char *gatewayname_raw;
	char *libcmd;
	char *setupcmd;
	char debug_level[STATUS_BUF];
	char *msg;
	FILE *fd;
	char *lockfile;

	// Check if nodogsplash is installed. If it is, issue a warning and exit
	libcmd = safe_calloc(STATUS_BUF);
	msg = safe_calloc(STATUS_BUF);

	safe_snprintf(libcmd, STATUS_BUF, "/usr/lib/opennds/libopennds.sh \"is_nodog\"");


	if (execute_ret_url_encoded(msg, STATUS_BUF - 1, libcmd) == 0) {
		debug(LOG_DEBUG, "NoDogSplash is installed, to continue please uninstall it and restart openNDS, exiting.....");
		exit (1);
	}

	free(libcmd);
	free(msg);

	// get configured debuglevel
	memset(debug_level, 0, STATUS_BUF);
	get_option_from_config(debug_level, STATUS_BUF, "debuglevel");

	// Are we enabled?
	config.debuglevel = 1;
	sscanf(set_option_str("enabled", DEFAULT_ENABLED, debug_level), "%u", &config.enabled);

	if(config.enabled != 1) {
		debug(LOG_NOTICE, "openNDS is disabled (see \"option enabled\" in config).\n");
		exit(0);
	}

	//parse_commandline(argc, argv);
	strncpy(config.configfile, DEFAULT_CONFIGFILE, sizeof(config.configfile)-1);

	openlog ("opennds", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog (LOG_NOTICE, "openNDS Version %s is in startup\n", VERSION);
	closelog ();

	printf ("openNDS Version %s is in startup - Please wait....\n", VERSION);

	// get configured debuglevel
	memset(debug_level, 0, STATUS_BUF);
	get_option_from_config(debug_level, STATUS_BUF, "debuglevel");

	/*
	********** String config parameters **********
	*/

	// Special handling for gatewayname as library call returns a url-encoded response
	gatewayname_raw = safe_calloc(SMALL_BUF);

	gatewayname = safe_strdup(set_option_str("gatewayname", DEFAULT_GATEWAYNAME, debug_level));
	uh_urldecode(gatewayname_raw, SMALL_BUF, gatewayname, SMALL_BUF);
	config.gw_name = safe_strdup(gatewayname_raw);

	openlog ("opennds", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
	syslog (LOG_NOTICE, "The name of this gateway is %s", gatewayname_raw);
	closelog ();

	free(gatewayname_raw);
	//

	config.gw_fqdn = safe_strdup(set_option_str("gatewayfqdn", DEFAULT_GATEWAYFQDN, debug_level));
	config.status_path = safe_strdup(set_option_str("statuspath", DEFAULT_STATUSPATH, debug_level));
	config.gw_interface = safe_strdup(set_option_str("gatewayinterface", DEFAULT_GATEWAYINTERFACE, debug_level));
	config.gw_iprange = safe_strdup(set_option_str("gateway_iprange", DEFAULT_GATEWAY_IPRANGE, debug_level));
	config.fas_key = safe_strdup(set_option_str("faskey", DEFAULT_FASKEY, debug_level));
	config.log_mountpoint = safe_strdup(set_option_str("log_mountpoint", DEFAULT_LOG_MOUNTPOINT, debug_level));
	config.webroot = safe_strdup(set_option_str("webroot", DEFAULT_WEBROOT, debug_level));
	config.authdir = safe_strdup(set_option_str("authdir", DEFAULT_AUTHDIR, debug_level));
	config.denydir = safe_strdup(set_option_str("denydir", DEFAULT_DENYDIR, debug_level));
	config.preauthdir = safe_strdup(set_option_str("preauthdir", DEFAULT_PREAUTHDIR, debug_level));
	config.ndsctl_sock = safe_strdup(set_option_str("ndsctl_sock", DEFAULT_NDSCTL_SOCK, debug_level));
	config.authentication_mark = safe_strdup(set_option_str("authentication_mark", DEFAULT_AUTHENTICATION_MARK, debug_level));
	config.binauth = safe_strdup(set_option_str("binauth", DEFAULT_BINAUTH, debug_level));
	config.fas_path = safe_strdup(set_option_str("faspath", DEFAULT_FASPATH, debug_level));
	config.themespec_path = safe_strdup(set_option_str("themespec_path", DEFAULT_THEMESPEC_PATH, debug_level));
	config.fas_remoteip = safe_strdup(set_option_str("fasremoteip", DEFAULT_FAS_REMOTEIP, debug_level));
	config.fas_remotefqdn = safe_strdup(set_option_str("fasremotefqdn", DEFAULT_FAS_REMOTEFQDN, debug_level));
	config.fas_ssl = safe_strdup(set_option_str("fas_ssl", DEFAULT_FAS_SSL, debug_level));

	/*
	********** Integer config parameters **********
	*/

	// Special handling of debuglevel - check valid level and inform externals
	sscanf(set_option_str("debuglevel", DEFAULT_DEBUGLEVEL, debug_level), "%u", &config.debuglevel);

	if (config.debuglevel > DEBUGLEVEL_MAX) {
		config.debuglevel = DEBUGLEVEL_MAX;
	}

	libcmd = safe_calloc(STATUS_BUF);
	msg = safe_calloc(STATUS_BUF);

	safe_snprintf(libcmd, STATUS_BUF, "/usr/lib/opennds/libopennds.sh \"debuglevel\" \"%u\"", config.debuglevel);

	if (execute_ret_url_encoded(msg, STATUS_BUF - 1, libcmd) == 0) {
		debug(LOG_DEBUG, "debuglevel [%u] signaled to externals - [%s] acknowledged", config.debuglevel, msg);
	} else {
		debug(LOG_ERR, "debuglevel [%u] signaled to externals - unable to set", config.debuglevel);
	}

	free(libcmd);
	free(msg);
	//

	sscanf(set_option_str("sessiontimeout", DEFAULT_SESSION_TIMEOUT, debug_level), "%u", &config.session_timeout);
	sscanf(set_option_str("preauthidletimeout", DEFAULT_PREAUTH_IDLE_TIMEOUT, debug_level), "%u", &config.preauth_idle_timeout);
	sscanf(set_option_str("authidletimeout", DEFAULT_AUTH_IDLE_TIMEOUT, debug_level), "%u", &config.auth_idle_timeout);
	sscanf(set_option_str("maxclients", DEFAULT_MAXCLIENTS, debug_level), "%u", &config.maxclients);
	sscanf(set_option_str("enable_serial_number_suffix", DEFAULT_ENABLE_SERIAL_NUMBER_SUFFIX, debug_level), "%u", &config.enable_serial_number_suffix);
	sscanf(set_option_str("dhcp_default_url_enable", DEFAULT_DHCP_DEFAULT_URL_ENABLE, debug_level), "%u", &config.dhcp_default_url_enable);
	sscanf(set_option_str("gatewayport", DEFAULT_GATEWAYPORT, debug_level), "%u", &config.gw_port);
	sscanf(set_option_str("fasport", DEFAULT_FASPORT, debug_level), "%u", &config.fas_port);
	sscanf(set_option_str("login_option_enabled", DEFAULT_LOGIN_OPTION_ENABLED, debug_level), "%u", &config.login_option_enabled);
	sscanf(set_option_str("use_outdated_mhd", DEFAULT_USE_OUTDATED_MHD, debug_level), "%u", &config.use_outdated_mhd);
	sscanf(set_option_str("max_page_size", DEFAULT_MAX_PAGE_SIZE, debug_level), "%llu", &config.max_page_size);
	sscanf(set_option_str("max_log_entries", DEFAULT_MAX_LOG_ENTRIES, debug_level), "%llu", &config.max_log_entries);
	sscanf(set_option_str("allow_preemptive_authentication", DEFAULT_ALLOW_PREEMPTIVE_AUTHENTICATION, debug_level), "%u", &config.allow_preemptive_authentication);
	sscanf(set_option_str("fas_secure_enabled", DEFAULT_FAS_SECURE_ENABLED, debug_level), "%u", &config.fas_secure_enabled);
	sscanf(set_option_str("remotes_refresh_interval", DEFAULT_REMOTES_REFRESH_INTERVAL, debug_level), "%u", &config.remotes_refresh_interval);
	sscanf(set_option_str("checkinterval", DEFAULT_CHECKINTERVAL, debug_level), "%u", &config.checkinterval);
	sscanf(set_option_str("ratecheckwindow", DEFAULT_RATE_CHECK_WINDOW, debug_level), "%u", &config.rate_check_window);
	sscanf(set_option_str("uploadrate", DEFAULT_UPLOAD_RATE, debug_level), "%llu", &config.upload_rate);
	sscanf(set_option_str("downloadrate", DEFAULT_DOWNLOAD_RATE, debug_level), "%llu", &config.download_rate);
	sscanf(set_option_str("download_bucket_ratio", DEFAULT_DOWNLOAD_BUCKET_RATIO, debug_level), "%llu", &config.download_bucket_ratio);
	sscanf(set_option_str("max_download_bucket_size", DEFAULT_MAX_DOWNLOAD_BUCKET_SIZE, debug_level), "%llu", &config.max_download_bucket_size);
	sscanf(set_option_str("download_unrestricted_bursting", DEFAULT_DOWNLOAD_UNRESTRICTED_BURSTING, debug_level), "%u", &config.download_unrestricted_bursting);
	sscanf(set_option_str("upload_bucket_ratio", DEFAULT_UPLOAD_BUCKET_RATIO, debug_level), "%llu", &config.upload_bucket_ratio);
	sscanf(set_option_str("max_upload_bucket_size", DEFAULT_MAX_UPLOAD_BUCKET_SIZE, debug_level), "%llu", &config.max_upload_bucket_size);
	sscanf(set_option_str("upload_unrestricted_bursting", DEFAULT_UPLOAD_UNRESTRICTED_BURSTING, debug_level), "%u", &config.upload_unrestricted_bursting);
	sscanf(set_option_str("uploadquota", DEFAULT_UPLOAD_QUOTA, debug_level), "%llu", &config.upload_quota);
	sscanf(set_option_str("downloadquota", DEFAULT_DOWNLOAD_QUOTA, debug_level), "%llu", &config.download_quota);
	sscanf(set_option_str("fup_upload_throttle_rate", DEFAULT_FUP_UPLOAD_THROTTLE_RATE, debug_level), "%llu", &config.fup_upload_throttle_rate);
	sscanf(set_option_str("fup_download_throttle_rate", DEFAULT_FUP_DOWNLOAD_THROTTLE_RATE, debug_level), "%llu", &config.fup_download_throttle_rate);
	sscanf(set_option_str("fw_mark_authenticated", DEFAULT_FW_MARK_AUTHENTICATED, debug_level), "%x", &config.fw_mark_authenticated);
	sscanf(set_option_str("fw_mark_auth_blocked", DEFAULT_FW_MARK_AUTH_BLOCKED, debug_level), "%x", &config.fw_mark_auth_blocked);
	sscanf(set_option_str("fw_mark_trusted", DEFAULT_FW_MARK_TRUSTED, debug_level), "%x", &config.fw_mark_trusted);

	// config.ip6 = DEFAULT_IP6;

	// Parameters kept in config but have no default or config value
	config.gw_address = NULL;
	config.gw_ip = NULL;
	config.http_encoded_gw_name = NULL;
	config.url_encoded_gw_name = NULL;
	config.fas_url = NULL;
	config.fas_hid = NULL;
	config.custom_params = NULL;
	config.custom_vars = NULL;
	config.custom_images = NULL;
	config.custom_files = NULL;
	config.tmpfsmountpoint = NULL;
	config.preauth = NULL;
	config.lockfd = 0;
	config.online_status = 0;

	// Lists
	parse_trusted_mac_list(set_list_str("trustedmac", DEFAULT_TRUSTEDMACLIST, debug_level));
	parse_fas_custom_parameters_list(set_list_str("fas_custom_parameters_list", DEFAULT_FAS_CUSTOM_PARAMETERS_LIST, debug_level));
	parse_fas_custom_images_list(set_list_str("fas_custom_images_list", DEFAULT_FAS_CUSTOM_IMAGES_LIST, debug_level));
	parse_fas_custom_files_list(set_list_str("fas_custom_files_list", DEFAULT_FAS_CUSTOM_FILES_LIST, debug_level));

	// Before we do anything else, reset the firewall (cleans it, in case we are restarting or after an opennds crash)
	iptables_fw_destroy();

	// Call the pre setup library function to create the base nttables ruleset
	setupcmd = safe_calloc(STATUS_BUF);
	safe_snprintf(setupcmd, STATUS_BUF, "/usr/lib/opennds/libopennds.sh \"pre_setup\"");
	msg = safe_calloc(STATUS_BUF);

	if (execute_ret_url_encoded(msg, STATUS_BUF - 1, setupcmd) == 0) {
		debug(LOG_INFO, "Pre-Setup request sent");
	}

	free(setupcmd);
	free(msg);

	// If we don't have the Gateway IP address, get it. Exit on failure.
	if (!config.gw_ip) {
		debug(LOG_DEBUG, "Finding IP address of %s", config.gw_interface);
		config.gw_ip = get_iface_ip(config.gw_interface, config.ip6);

		if (is_addr(config.gw_ip) != 1) {
			debug(LOG_ERR, "Could not get IP address information of %s, exiting...", config.gw_interface);
			exit(1);
		} else {
			debug(LOG_NOTICE, "Interface %s is up", config.gw_interface);
		}
	}

	// format gw_address accordingly depending on if gw_ip is v4 or v6
	const char *ipfmt = config.ip6 ? "[%s]:%d" : "%s:%d";
	safe_asprintf(&config.gw_address, ipfmt, config.gw_ip, config.gw_port);

	config.gw_mac = get_iface_mac(config.gw_interface);

	if (strcmp(config.gw_mac, "00:00:00:00:00:00") == 0 || config.gw_mac == NULL) {
		debug(LOG_ERR, "Could not get MAC address information of %s, exiting...", config.gw_interface);
		exit(1);
	}

	debug(LOG_NOTICE, "Interface %s is at %s (%s)", config.gw_interface, config.gw_ip, config.gw_mac);

	// Make sure fas_remoteip is set. Note: This does not enable FAS.
	if (strcmp(config.fas_remoteip, "disabled") == 0) {
		config.fas_remoteip = safe_strdup(config.gw_ip);
	}

	debug(LOG_DEBUG, "FAS remote ip address is [ %s ]", config.fas_remoteip);

	// Generate a unique faskey if not set in config
	if (strcmp(config.fas_key, DEFAULT_FASKEY) == 0) {
		setupcmd = safe_calloc(STATUS_BUF);
		safe_snprintf(setupcmd, STATUS_BUF, "/usr/lib/opennds/libopennds.sh \"generate_key\"");
		msg = safe_calloc(STATUS_BUF);

		if (execute_ret_url_encoded(msg, STATUS_BUF - 1, setupcmd) == 0) {
			config.fas_key = safe_strdup(msg);
		}

		free(setupcmd);
		free(msg);

		setupcmd = safe_calloc(SMALL_BUF);
		safe_snprintf(setupcmd, SMALL_BUF, "/usr/lib/opennds/libopennds.sh \"set_key\" \"%s\"", config.fas_key);
		msg = safe_calloc(STATUS_BUF);

		if (execute_ret_url_encoded(msg, STATUS_BUF - 1, setupcmd) == 0) {
			debug(LOG_NOTICE, "faskey generated");
		}

		free(setupcmd);
		free(msg);
	}

	// Clean up any old database files and set some needed parameters
	libcmd = safe_calloc(STATUS_BUF);
	msg = safe_calloc(STATUS_BUF);

	safe_snprintf(libcmd, STATUS_BUF, "/usr/lib/opennds/libopennds.sh clean");

	execute_ret_url_encoded(msg, STATUS_BUF, libcmd);
	free(libcmd);

	if (strlen(msg) == 0) {
		config.tmpfsmountpoint = safe_strdup("/tmp");
	} else {
		config.tmpfsmountpoint = safe_strdup(msg);
	}

	free(msg);
	lockfile = safe_calloc(STATUS_BUF);
	safe_snprintf(lockfile, STATUS_BUF, "%s/ndsctl.lock", config.tmpfsmountpoint);

	//Remove ndsctl lock file if it exists
	if ((fd = fopen(lockfile, "r")) != NULL) {
		fclose(fd);
		remove(lockfile);
	}

	free (lockfile);
}

// Parse a string to see if it is valid decimal dotted quad IP V4 format
int check_ip_format(const char *possibleip)
{
	unsigned char buf[sizeof(struct in6_addr)];
	return inet_pton(AF_INET, possibleip, buf) > 0;
}

// Parse a string to see if it is valid MAC address format
int check_mac_format(const char possiblemac[])
{
	return ether_aton(possiblemac) != NULL;
}

int add_to_fas_custom_parameters_list(const char possibleparam[])
{
	char param[512];
	t_FASPARAM *p = NULL;

	sscanf(possibleparam, "%s", param);

	// Add Parameter to head of list
	p = safe_calloc(sizeof(t_FASPARAM));
	p->fasparam = safe_strdup(param);
	p->next = config.fas_custom_parameters_list;

	config.fas_custom_parameters_list = p;
	debug(LOG_INFO, "Added Custom Parameter [%s]", possibleparam);
	return 0;
}

int add_to_fas_custom_variables_list(const char possiblevar[])
{
	char var[512];
	t_FASVAR *p = NULL;

	sscanf(possiblevar, "%s", var);

	// Add Variable to head of list
	p = safe_calloc(sizeof(t_FASVAR));
	p->fasvar = safe_strdup(var);
	p->next = config.fas_custom_variables_list;

	config.fas_custom_variables_list = p;
	debug(LOG_INFO, "Added Custom Variable [%s]", possiblevar);
	return 0;
}

int add_to_fas_custom_images_list(const char possibleimage[])
{
	char image[512];
	t_FASIMG *p = NULL;

	sscanf(possibleimage, "%s", image);

	// Add Image to head of list
	p = safe_calloc(sizeof(t_FASIMG));
	p->fasimg = safe_strdup(image);
	p->next = config.fas_custom_images_list;

	config.fas_custom_images_list = p;
	debug(LOG_INFO, "Added Custom Image [%s]", possibleimage);
	return 0;
}

int add_to_fas_custom_files_list(const char possiblefile[])
{
	char file[512];
	t_FASFILE *p = NULL;

	sscanf(possiblefile, "%s", file);

	// Add File to head of list
	p = safe_calloc(sizeof(t_FASFILE));
	p->fasfile = safe_strdup(file);
	p->next = config.fas_custom_files_list;

	config.fas_custom_files_list = p;
	debug(LOG_INFO, "Added Custom File [%s]", possiblefile);
	return 0;
}


int add_to_trusted_mac_list(const char possiblemac[])
{
	char mac[18];
	t_MAC *p = NULL;

	// check for valid format
	if (!check_mac_format(possiblemac)) {
		debug(LOG_WARNING, "[%s] is not a valid MAC address", possiblemac);
		debug(LOG_WARNING, "[%s]  - please remove from trustedmac list in config file", possiblemac);
		return 1;
	}

	sscanf(possiblemac, "%17[A-Fa-f0-9:]", mac);

	// See if MAC is already on the list; don't add duplicates
	for (p = config.trustedmaclist; p != NULL; p = p->next) {
		if (!strcasecmp(p->mac, mac)) {
			debug(LOG_INFO, "MAC address [%s] already on trusted list", mac);
			return 1;
		}
	}

	// Add MAC to head of list
	p = safe_calloc(sizeof(t_MAC));
	p->mac = safe_strdup(mac);
	p->next = config.trustedmaclist;
	config.trustedmaclist = p;
	debug(LOG_INFO, "Added MAC address [%s] to trusted list", mac);
	return 0;
}


/* Remove given MAC address from the config's trusted mac list.
 * Return 0 on success, nonzero on failure
 */
int remove_from_trusted_mac_list(const char possiblemac[])
{
	char mac[18];
	t_MAC **p = NULL;
	t_MAC *del = NULL;

	// check for valid format
	if (!check_mac_format(possiblemac)) {
		debug(LOG_ERR, "[%s] not a valid MAC address", possiblemac);
		return -1;
	}

	sscanf(possiblemac, "%17[A-Fa-f0-9:]", mac);

	// If empty list, nothing to do
	if (config.trustedmaclist == NULL) {
		debug(LOG_INFO, "MAC address [%s] not on empty trusted list", mac);
		return -1;
	}

	// Find MAC on the list, remove it
	for (p = &(config.trustedmaclist); *p != NULL; p = &((*p)->next)) {
		if (!strcasecmp((*p)->mac, mac)) {
			// found it
			del = *p;
			*p = del->next;
			debug(LOG_INFO, "Removed MAC address [%s] from trusted list", mac);
			free(del);
			return 0;
		}
	}

	// MAC was not on list
	debug(LOG_INFO, "MAC address [%s] not on  trusted list", mac);
	return -1;
}


/* Given a pointer to a comma or whitespace delimited sequence of
 * MAC addresses, add each MAC address to config.trustedmaclist.
 */
void parse_trusted_mac_list(const char ptr[])
{
	char *ptrcopy = NULL, *ptrcopyptr;
	char *possiblemac = NULL;

	// strsep modifies original, so let's make a copy
	ptrcopyptr = ptrcopy = safe_strdup(ptr);

	while ((possiblemac = strsep(&ptrcopy, ", \t"))) {
		if (strlen(possiblemac) > 0) {
			if (add_to_trusted_mac_list(possiblemac) < 0) {
				exit(1);
			}
		}
	}

	free(ptrcopy);
	free(ptrcopyptr);
}

int is_trusted_mac(const char *mac)
{
	s_config *config;
	t_MAC *trust_mac;

	config = config_get_config();

	// Is a client even recognized here?
	for (trust_mac = config->trustedmaclist; trust_mac != NULL; trust_mac = trust_mac->next) {
		if (!strcmp(trust_mac->mac, mac)) {
			return 1;
		}
	}

	return 0;
}

/* Given a pointer to a comma or whitespace delimited sequence of
 * Custom FAS Parameters, add each parameter to config.fas_custom_parameters_list
 */
void parse_fas_custom_parameters_list(const char ptr[])
{
	char *ptrcopy = NULL;
	char *possibleparam = NULL;
	char msg[512] = {0};
	char *cmd = NULL;
	char possibleparam_urlencoded[512] = {0};

	// strsep modifies original, so let's make a copy
	ptrcopy = safe_strdup(ptr);

	while ((possibleparam = strsep(&ptrcopy, ", \t"))) {
		if (strlen(possibleparam) > 0) {

			// URL encode Parameter before parsing
			memset(possibleparam_urlencoded, 0, sizeof(possibleparam_urlencoded));
			uh_urlencode(possibleparam_urlencoded, sizeof(possibleparam_urlencoded), possibleparam, strlen(possibleparam));
			debug(LOG_DEBUG, "[%s] is the urlencoded Custom FAS Parameter", possibleparam_urlencoded);

			safe_asprintf(&cmd,
				"echo \"%s\" | awk -F'%s' 'NF>1 && $1!=\"\" && $2!=\"\" {printf \"%s\",$0}'",
				possibleparam_urlencoded,
				CUSTOM_SEPARATOR,
				FORMAT_SPECIFIER
			);
			debug(LOG_DEBUG, "Parse command [%s]", cmd);
			memset(msg, 0, sizeof(msg));
			execute_ret_url_encoded(msg, sizeof(msg) - 1, cmd);
			free(cmd);
			if (strcmp(msg, possibleparam_urlencoded) == 0) {
				debug(LOG_INFO, "Adding parameter [%s] [%s]", possibleparam, msg);
				add_to_fas_custom_parameters_list(possibleparam);
			} else {
				debug(LOG_WARNING, "Invalid Custom Parameter [%s] [%s] - skipping", possibleparam, msg);
			}
		}
	}
}

/* Given a pointer to a comma or whitespace delimited sequence of
 * Custom FAS Variables, add each parameter to config.fas_custom_variables_list
 */
void parse_fas_custom_variables_list(const char ptr[])
{
	char *ptrcopy = NULL;
	char *possiblevar = NULL;
	char msg[512] = {0};
	char *cmd = NULL;
	char possiblevar_urlencoded[512] = {0};

	debug(LOG_INFO, "Parsing list [%s] for Custom FAS Variables", ptr);

	// strsep modifies original, so let's make a copy
	ptrcopy = safe_strdup(ptr);

	while ((possiblevar = strsep(&ptrcopy, ", \t"))) {
		if (strlen(possiblevar) > 0) {

			// URL encode Variable before parsing
			memset(possiblevar_urlencoded, 0, sizeof(possiblevar_urlencoded));
			uh_urlencode(possiblevar_urlencoded, sizeof(possiblevar_urlencoded), possiblevar, strlen(possiblevar));
			debug(LOG_DEBUG, "[%s] is the urlencoded Custom FAS Variable", possiblevar_urlencoded);

			safe_asprintf(&cmd,
				"echo \"%s\" | awk -F'%s' 'NF>1 && $1!=\"\" && $2!=\"\" {printf \"%s\",$0}'",
				possiblevar_urlencoded,
				CUSTOM_SEPARATOR,
				FORMAT_SPECIFIER
			);
			debug(LOG_DEBUG, "Parse command [%s]", cmd);
			memset(msg, 0, sizeof(msg));
			execute_ret_url_encoded(msg, sizeof(msg) - 1, cmd);
			free(cmd);
			if (strcmp(msg, possiblevar_urlencoded) == 0) {
				debug(LOG_INFO, "Adding variable [%s] [%s]", possiblevar, msg);
				add_to_fas_custom_variables_list(possiblevar);
			} else {
				debug(LOG_WARNING, "Invalid Custom Variable [%s] [%s] - skipping", possiblevar, msg);
			}
		}
	}
}

/* Given a pointer to a comma or whitespace delimited sequence of
 * Custom FAS Images, add each image to config.fas_custom_images_list
 */
void parse_fas_custom_images_list(const char ptr[])
{
	char *ptrcopy = NULL;
	char *possibleimage = NULL;
	char msg[512] = {0};
	char *cmd = NULL;
	char possibleimage_urlencoded[512] = {0};

	debug(LOG_INFO, "Parsing list [%s] for Custom FAS Images", ptr);

	// strsep modifies original, so let's make a copy
	ptrcopy = safe_strdup(ptr);

	while ((possibleimage = strsep(&ptrcopy, ", \t"))) {
		if (strlen(possibleimage) > 0) {

			// URL encode Image before parsing
			memset(possibleimage_urlencoded, 0, sizeof(possibleimage_urlencoded));
			uh_urlencode(possibleimage_urlencoded, sizeof(possibleimage_urlencoded), possibleimage, strlen(possibleimage));
			debug(LOG_DEBUG, "[%s] is the urlencoded Custom FAS Image", possibleimage_urlencoded);

			safe_asprintf(&cmd,
				"echo \"%s\" | awk -F'%s' 'NF>1 && $1!=\"\" && $2!=\"\" {printf \"%s\",$0}'",
				possibleimage_urlencoded,
				CUSTOM_SEPARATOR,
				FORMAT_SPECIFIER
			);
			debug(LOG_DEBUG, "Parse command [%s]", cmd);
			memset(msg, 0, sizeof(msg));
			execute_ret_url_encoded(msg, sizeof(msg) - 1, cmd);
			free(cmd);
			if (strcmp(msg, possibleimage_urlencoded) == 0) {
				debug(LOG_INFO, "Adding image [%s] [%s]", possibleimage, msg);
				add_to_fas_custom_images_list(possibleimage);
			} else {
				debug(LOG_WARNING, "Invalid Custom Image [%s] [%s] - skipping", possibleimage, msg);
			}
		}
	}
}

/* Given a pointer to a comma or whitespace delimited sequence of
 * Custom FAS Files, add each image to config.fas_custom_files_list
 */
void parse_fas_custom_files_list(const char ptr[])
{
	char *ptrcopy = NULL;
	char *possiblefile = NULL;
	char msg[512] = {0};
	char *cmd = NULL;
	char possiblefile_urlencoded[512] = {0};

	debug(LOG_INFO, "Parsing list [%s] for Custom FAS Files", ptr);

	// strsep modifies original, so let's make a copy
	ptrcopy = safe_strdup(ptr);

	while ((possiblefile = strsep(&ptrcopy, ", \t"))) {
		if (strlen(possiblefile) > 0) {

			// URL encode Image before parsing
			memset(possiblefile_urlencoded, 0, sizeof(possiblefile_urlencoded));
			uh_urlencode(possiblefile_urlencoded, sizeof(possiblefile_urlencoded), possiblefile, strlen(possiblefile));
			debug(LOG_DEBUG, "[%s] is the urlencoded Custom FAS Image", possiblefile_urlencoded);

			safe_asprintf(&cmd,
				"echo \"%s\" | awk -F'%s' 'NF>1 && $1!=\"\" && $2!=\"\" {printf \"%s\",$0}'",
				possiblefile_urlencoded,
				CUSTOM_SEPARATOR,
				FORMAT_SPECIFIER
			);
			debug(LOG_DEBUG, "Parse command [%s]", cmd);
			memset(msg, 0, sizeof(msg));
			execute_ret_url_encoded(msg, sizeof(msg) - 1, cmd);
			free(cmd);
			if (strcmp(msg, possiblefile_urlencoded) == 0) {
				debug(LOG_INFO, "Adding file [%s] [%s]", possiblefile, msg);
				add_to_fas_custom_files_list(possiblefile);
			} else {
				debug(LOG_WARNING, "Invalid Custom File [%s] [%s] - skipping", possiblefile, msg);
			}
		}
	}
}

/** Set the debug log level.  See syslog.h
 *  Return 0 on success.
 */
int set_debuglevel(const char opt[])
{
	char *libcmd;
	char *msg;

	if (opt == NULL || strlen(opt) == 0) {
		return 1;
	}

	// parse number
	int level = strtol(opt, NULL, 10);


	if (level >= (int) DEBUGLEVEL_MIN && level <= (int) DEBUGLEVEL_MAX) {
		msg = safe_calloc(STATUS_BUF);

		sscanf(opt, "%u", &config.debuglevel);

		libcmd = safe_calloc(STATUS_BUF);
		safe_snprintf(libcmd, STATUS_BUF, "/usr/lib/opennds/libopennds.sh \"debuglevel\" \"%s\"", opt);

		if (execute_ret_url_encoded(msg, STATUS_BUF - 1, libcmd) == 0) {
			debug(LOG_DEBUG, "debuglevel [%d] signaled to externals - [%s] acknowledged", level, msg);
		} else {
			debug(LOG_ERR, "debuglevel [%d] signaled to externals - unable to set", level);
		}

		free(libcmd);
		free(msg);
		return 0;
	} else {
		return 1;
	}
}

