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

/** @file conf.h
    @brief Config file parsing
    @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
    @author Copyright (C) 2007 Paul Kube <nodogsplash@kokoro.ucsd.edu>
    @author Copyright (C) 2015-2025 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
*/

#define COPYRIGHT "openNDS, Copyright (C) 2015-2025 Modifications and additions by BlueWave Projects and Services"

#ifndef _CONF_H_
#define _CONF_H_

#define VERSION "10.3.1"

/*
 * Defines how many times should we try detecting the interface with the default route (in seconds).
 * If set to 0, it will keep retrying forever.
 */
#define NUM_EXT_INTERFACE_DETECT_RETRY 0

// How long we should wait per try to detect the interface with the default route if it isn't up yet (interval in seconds)
#define EXT_INTERFACE_DETECT_RETRY_INTERVAL 1

// Defaults configuration values
#ifndef SYSCONFDIR
#define DEFAULT_CONFIGFILE "/etc/opennds/opennds.conf"
#else
#define DEFAULT_CONFIGFILE SYSCONFDIR"/opennds/opennds.conf"
#endif
#define DEFAULT_DAEMON "0"
#define DEFAULT_ENABLED "1"
#define DEFAULT_DEBUGLEVEL "1"
#define DEFAULT_MAXCLIENTS "250"
#define DEFAULT_ONLINE_STATUS "0"
#define DEFAULT_GATEWAYINTERFACE "br-lan"
#define DEFAULT_GATEWAY_IPRANGE "0.0.0.0/0"
#define DEFAULT_GATEWAYNAME "openNDS"
#define DEFAULT_ENABLE_SERIAL_NUMBER_SUFFIX "1"
#define DEFAULT_GATEWAYPORT "2050"
#define DEFAULT_GATEWAYFQDN "status.client"
#define DEFAULT_DHCP_DEFAULT_URL_ENABLE "1"
#define DEFAULT_STATUSPATH "/usr/lib/opennds/client_params.sh"
#define DEFAULT_LOG_MOUNTPOINT "/tmp"
#define DEFAULT_MAX_PAGE_SIZE "10240"
#define DEFAULT_FASPORT "0"
#define DEFAULT_LOGIN_OPTION_ENABLED "0"
#define DEFAULT_MAX_LOG_ENTRIES "100"
#define DEFAULT_USE_OUTDATED_MHD "0"
#define DEFAULT_ALLOW_PREEMPTIVE_AUTHENTICATION "1"
#define DEFAULT_FAS_SECURE_ENABLED "1"
#define DEFAULT_FASPATH "/"
#define DEFAULT_FASKEY ""
#define DEFAULT_BINAUTH "/usr/lib/opennds/binauth_log.sh"
#define DEFAULT_CHECKINTERVAL "15"
#define DEFAULT_SESSION_TIMEOUT "1440"
#define DEFAULT_PREAUTH_IDLE_TIMEOUT "30"
#define DEFAULT_AUTH_IDLE_TIMEOUT "120"
#define DEFAULT_REMOTES_REFRESH_INTERVAL "0"
#define DEFAULT_WEBROOT "/etc/opennds/htdocs"
#define DEFAULT_TMPFSMOUNTPOINT "/tmp"
#define DEFAULT_AUTHDIR "opennds_auth"
#define DEFAULT_DENYDIR "opennds_deny"
#define DEFAULT_PREAUTHDIR "opennds_preauth"
#define DEFAULT_SET_MSS "1" //allow setting the TCP Maximum Segment Size
#define DEFAULT_MSS_VALUE "0" // value to set the MSS. 0 means use max possible ie clamp-mss-to-pmtu
#define DEFAULT_RATE_CHECK_WINDOW "2" // The data rate check moving average window size multiply this by CHECKINTERVAL to give window size (or burst interval) in seconds
#define DEFAULT_UPLOAD_RATE "0" // 0 means no limit
#define DEFAULT_DOWNLOAD_RATE "0" // 0 means no limit
#define DEFAULT_UPLOAD_BUCKET_RATIO "1" // Allows control of upload rate limit threshold overrun per client
#define DEFAULT_DOWNLOAD_BUCKET_RATIO "1" // Allows control of download rate limit threshold overrun per client
#define DEFAULT_MAX_UPLOAD_BUCKET_SIZE "250" // Allows control over upload rate limiting packet loss at the expense of increased latency
#define DEFAULT_MAX_DOWNLOAD_BUCKET_SIZE "250" // Allows control over download rate limiting packet loss at the expense of increased latency
#define DEFAULT_UPLOAD_QUOTA "0" // 0 means no limit
#define DEFAULT_DOWNLOAD_QUOTA "0" // 0 means no limit
#define DEFAULT_FUP_UPLOAD_THROTTLE_RATE "0" // 0 means BLOCK the client until deauthed
#define DEFAULT_FUP_DOWNLOAD_THROTTLE_RATE "0" // 0 means BLOCK the client until deauthed
#define DEFAULT_UPLOAD_UNRESTRICTED_BURSTING "0" // 0 means disabled, 1 means enabled
#define DEFAULT_DOWNLOAD_UNRESTRICTED_BURSTING "0" // 0 means disabled, 1 means enabled
#define DEFAULT_NDSCTL_SOCK "ndsctl.sock"
#define DEFAULT_FW_MARK_AUTHENTICATED "0x30000"
#define DEFAULT_FW_MARK_AUTH_BLOCKED "0x30001"
#define DEFAULT_AUTHENTICATION_MARK "0x00030000"
#define DEFAULT_FW_MARK_TRUSTED "0x20000"
#define DEFAULT_THEMESPEC_PATH ""
#define DEFAULT_FAS_REMOTEFQDN "disabled"
#define DEFAULT_FAS_REMOTEIP "disabled"
#define DEFAULT_FAS_SSL "wget"

/* N.B.: default policies here must be ACCEPT, REJECT, or RETURN
 * In the .conf file, they must be allow, block, or passthrough
 * Mapping between these enforced by parse_empty_ruleset_policy()
 */
#define DEFAULT_EMPTY_TRUSTED_USERS_POLICY "accept"
#define DEFAULT_EMPTY_TRUSTED_USERS_TO_ROUTER_POLICY "accept"
#define DEFAULT_EMPTY_USERS_TO_ROUTER_POLICY "reject"
#define DEFAULT_EMPTY_AUTHENTICATED_USERS_POLICY "return"
#define DEFAULT_EMPTY_PREAUTHENTICATED_USERS_POLICY "reject"
#define DEFAULT_IP6 0

// Default lists
#define DEFAULT_TRUSTEDMACLIST ""
#define DEFAULT_FAS_CUSTOM_PARAMETERS_LIST ""
#define DEFAULT_FAS_CUSTOM_VARIABLES_LIST ""
#define DEFAULT_FAS_CUSTOM_IMAGES_LIST ""
#define DEFAULT_FAS_CUSTOM_FILES_LIST ""
#define DEFAULT_USERS_TO_ROUTER "allow%20udp%20port%2053 allow%20udp%20port%2067 allow%20tcp%20port%2022 allow%20tcp%20port%20443"
#define DEFAULT_AUTHENTICATED_USERS "allow%20all"
#define DEFAULT_PREAUTHENTICATED_USERS ""

// Firewall targets
typedef enum {
	TARGET_DROP,
	TARGET_REJECT,
	TARGET_ACCEPT,
	TARGET_RETURN,
	TARGET_LOG,
	TARGET_ULOG
} t_firewall_target;

// Firewall rules
typedef struct _firewall_rule_t {
	t_firewall_target target;	//@brief t_firewall_target
	char *protocol;		//@brief tcp, udp, etc ...
	char *port;			//@brief Port to block/allow
	char *mask;			//@brief Mask for the rule *destination*
	char *ipset;			//@brief IPset rule
	struct _firewall_rule_t *next;
} t_firewall_rule;

// MAC Addresses
typedef struct _MAC_t {
	char *mac;
	struct _MAC_t *next;
} t_MAC;

// Walled Garden Ports
typedef struct _WGP_t {
	unsigned int wgport;
	struct _WGP_t *next;
} t_WGP;

// Walled Garden FQDNs
typedef struct _WGFQDN_t {
	char *wgfqdn;
	struct _WGFQDN_t *next;
} t_WGFQDN;

// Custom FAS Parameters
typedef struct _FASPARAM_t {
	char *fasparam;
	struct _FASPARAM_t *next;
} t_FASPARAM;

// Custom FAS Variables
typedef struct _FASVAR_t {
	char *fasvar;
	struct _FASVAR_t *next;
} t_FASVAR;

// Custom FAS Images
typedef struct _FASIMG_t {
	char *fasimg;
	struct _FASIMG_t *next;
} t_FASIMG;

// Custom FAS Files
typedef struct _FASFILE_t {
	char *fasfile;
	struct _FASFILE_t *next;
} t_FASFILE;

// Configuration structure
typedef struct {
	char configfile[255];					//@brief name of the config file
	char *ndsctl_sock;					//@brief ndsctl path to socket
	char *internal_sock;					//@brief internal path to socket
	int enabled;						//@brief if openNDS is enabled
	int daemon;						//@brief if daemon != 0, use daemon mode
	int debuglevel;						//@brief Debug information verbosity
	int maxclients;						//@brief Maximum number of clients allowed
	int online_status;					//@brief Online status of the router, 1=online, 0=offline
	char *gw_name;						//@brief Name of the gateway; e.g. its SSID or a unique identifier for use in a remote FAS
	int enable_serial_number_suffix;			//@brief Enable/disable serial number suffix to gateway name
	char *http_encoded_gw_name;				//@brief http encoded name of the gateway, used as a templated variable in splash.htm
	char *url_encoded_gw_name;				//@brief url encoded name of the gateway used as variable in Preauth
	char *gw_interface;					//@brief Interface we will manage
	char *ext_gateway;					//@brief The interfaces and IP addresses of upstream gateways
	char *gw_iprange;					//@brief IP range on gw_interface we will manage
	char *gw_ip;						//@brief Internal IP (v4 or v6) for our web server
	char *gw_address;					//@brief Internal IP with port for our web server
	char *gw_mac;						//@brief MAC address of the interface we manage
	char *gw_fqdn;						//@brief FQDN of the client status page
	char *status_path;					//@brief Path to the client status page script
	int dhcp_default_url_enable;				//@brief Enable DHCP default-url (code 114 - RFC8910)
	unsigned int gw_port;					//@brief Port the webserver will run on
	unsigned int fas_port;					//@brief Port the fas server will run on
	int login_option_enabled;				//@brief Use default PreAuth Login script
	unsigned long long int max_log_entries;			//@brief set the maximum number of log entries
	int use_outdated_mhd;					//@brief Use outdated libmicrohttpd
	unsigned long long int max_page_size;			//@brief Max page size to be served by libmicrohttpd
	int allow_preemptive_authentication;			//@brief Allow Preemptive Authentication using the ndsctl utility
	int fas_secure_enabled;					//@brief Enable Secure FAS
	char *fas_path;						//@brief Path to forward authentication page of FAS
	char *fas_key;						//@brief AES key for FAS
	char *fas_remoteip;					//@brief IP addess of a remote FAS
	char *fas_remotefqdn;					//@brief FQDN of a remote FAS
	char *fas_url;						//@brief URL of a remote FAS
	char *fas_ssl;						//@brief SSL provider for FAS
	char *fas_hid;						//@brief Hash provider for FAS
	char *themespec_path;					//@brief Path to the ThemeSpec file to use for login_option_enabled = 3
	char *tmpfsmountpoint;					//@brief Mountpoint of the tmpfs drive eg /tmp etc.
	char *log_mountpoint;					//@brief Mountpoint of the log drive eg a USB drive mounted at /logs
	char *webroot;						//@brief Directory containing splash pages, etc.
	char *authdir;						//@brief Notional relative dir for authentication URL
	char *denydir;						//@brief Notional relative dir for denial URL
	char *preauthdir;					//@brief Notional relative dir for preauth URL
	int session_timeout;					//@brief Minutes of the default session length
	int preauth_idle_timeout;				//@brief Minutes a preauthenticated client will be kept in the system
	int auth_idle_timeout;					//@brief Minutes an authenticated client will be kept in the system
	int remotes_refresh_interval;				//@brief Minutes before remote files will be refreshed
	unsigned long long int remotes_last_refresh;		//@brief Time of last refresh of remote files
	int checkinterval;					//@brief Period the the client timeout check thread will run, in seconds
	int set_mss;						//@brief boolean, whether to set mss
	int mss_value;						//@brief int, mss value; <= 0 clamp to pmtu
	int rate_check_window;					//@brief window size in multiples of checkinterval for rate check moving average
	unsigned long long int download_rate;			//@brief Download rate, kb/s
	unsigned long long int upload_rate;			//@brief Upload rate, kb/s
	unsigned long long int download_bucket_ratio;		//@brief Allows control of download rate limit threshold overrun per client
	unsigned long long int upload_bucket_ratio;		//@brief Allows control of upload rate limit threshold overrun per client
	unsigned long long int max_upload_bucket_size;		//@brief control upload rate limiting packet loss at the expense of increased latency
	unsigned long long int max_download_bucket_size;	//@brief control download rate limiting packet loss at the expense of increased latency
	unsigned long long int download_quota;			//@brief Download quota, kB
	unsigned long long int upload_quota;			//@brief Upload quota, kB
	unsigned long long int fup_download_throttle_rate;	//@brief Fair Useage Policy Download throttle rate, kb/s, activated when quota exceeded
	unsigned long long int fup_upload_throttle_rate;	//@brief Fair Usage Policy Upload throttle rate, kb/s, activated when quota exceeded
	int download_unrestricted_bursting;			//@brief Enable/disable unrestriced bursting
	int upload_unrestricted_bursting;			//@brief Enable/disable unrestriced bursting
	int syslog_facility;					//@brief facility to use when using syslog for logging
	int macmechanism; 					//@brief mechanism wrt MAC addrs
	t_MAC *trustedmaclist;					//@brief list of trusted macs
	t_FASPARAM *fas_custom_parameters_list;			//@brief list of Custom FAS parameters
	t_FASVAR *fas_custom_variables_list;			//@brief list of Custom FAS variables
	t_FASIMG *fas_custom_images_list;			//@brief list of Custom FAS images
	t_FASFILE *fas_custom_files_list;			//@brief list of Custom FAS files
	char *custom_params;					//@brief FAS custom parameter string
	char *custom_vars;					//@brief FAS custom variable string
	char *custom_images;					//@brief FAS custom image string
	char *custom_files;					//@brief FAS custom file string
	unsigned int fw_mark_authenticated;			//@brief nftables mark for authenticated packets
	unsigned int fw_mark_auth_blocked;			//@brief nftables mark for auth_blocked packets
	char *authentication_mark;				//@brief Padded authentication mark
	unsigned int fw_mark_trusted;				//@brief nftables mark for trusted packets
	int ip6;						//@brief enable IPv6
	char *binauth;						//@brief external postauthentication program
	char *preauth;						//@brief external preauthentication program
	int lockfd;						//@brief ndsctl lockfile file descriptor
} s_config;

// @brief Get the current gateway configuration
s_config *config_get_config(void);

// @brief Initialise the conf system
void config_init(int argc, char **argv);
void parse_trusted_mac_list(const char[]);
void parse_fas_custom_parameters_list(const char[]);
void parse_fas_custom_variables_list(const char[]);
void parse_fas_custom_images_list(const char[]);
void parse_fas_custom_files_list(const char[]);

int is_trusted_mac(const char *mac);

int remove_from_trusted_mac_list(const char possiblemac[]);
int add_to_trusted_mac_list(const char possiblemac[]);

int check_ip_format(const char[]);
int check_mac_format(const char[]);

// config API, used in commandline.c
int set_debuglevel(const char[]);

#define LOCK_CONFIG() do { \
	debug(LOG_DEBUG, "Locking config"); \
	pthread_mutex_lock(&config_mutex); \
	debug(LOG_DEBUG, "Config locked"); \
} while (0)

#define UNLOCK_CONFIG() do { \
	debug(LOG_DEBUG, "Unlocking config"); \
	pthread_mutex_unlock(&config_mutex); \
	debug(LOG_DEBUG, "Config unlocked"); \
} while (0)

#endif // _CONF_H_

char *set_list_str(char *list, const char *default_list, char *debug_level);
