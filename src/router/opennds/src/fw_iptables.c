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

/** @internal
  @file fw_iptables.c
  @brief Firewall nftables functions
  @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
  @author Copyright (C) 2007 Paul Kube <nodogsplash@kokoro.ucsd.edu>
  @author Copyright (C) 2015-2025 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"

#include "safe.h"
#include "conf.h"
#include "auth.h"
#include "client_list.h"
#include "fw_iptables.h"
#include "debug.h"
#include "util.h"

static int _iptables_init_marks(void);

// Used to mark packets, and characterize client state.  Unmarked packets are considered 'preauthenticated'
unsigned int FW_MARK_PREAUTHENTICATED;	// @brief 0: Actually not used as a packet mark
unsigned int FW_MARK_AUTHENTICATED;	// @brief The client is authenticated
unsigned int FW_MARK_AUTH_BLOCKED;	// @brief The client is authenticated
unsigned int FW_MARK_TRUSTED;		// @brief The client is trusted
unsigned int FW_MARK_MASK;		// @brief nftables mask: bitwise or of the others

extern pthread_mutex_t client_list_mutex;
extern pthread_mutex_t config_mutex;

// Make nonzero to supress the error output of the firewall during destruction.
static int fw_quiet = 0;

// Used to configure use of --or-mark vs. --set-mark
static const char* markop = "--set-mark";

// Used to configure use of mark mask, or not
static const char* markmask = "";

// Return a string representing a connection state
const char *
fw_connection_state_as_string(int mark)
{
	if (mark == FW_MARK_PREAUTHENTICATED)
		return "Preauthenticated";
	if (mark == FW_MARK_AUTHENTICATED)
		return "Authenticated";
	if (mark == FW_MARK_AUTH_BLOCKED)
		return "AuthBlocked";
	if (mark == FW_MARK_TRUSTED)
		return "Trusted";
	return "ERROR: unrecognized mark";
}

// @internal
int
_iptables_init_marks()
{
	// Check FW_MARK values are distinct.
	if (FW_MARK_TRUSTED == FW_MARK_AUTHENTICATED) {
		debug(LOG_ERR, "FW_MARK_TRUSTED, FW_MARK_AUTHENTICATED not distinct values.");
		return -1;
	}

	// Check FW_MARK values nonzero.
	if (FW_MARK_TRUSTED == 0 || FW_MARK_AUTHENTICATED == 0) {
		debug(LOG_ERR, "FW_MARK_TRUSTED, FW_MARK_AUTHENTICATED not all nonzero.");
		return -1;
	}

	FW_MARK_PREAUTHENTICATED = 0;  // always 0
	// FW_MARK_MASK is bitwise OR of other marks
	FW_MARK_MASK = FW_MARK_TRUSTED | FW_MARK_AUTHENTICATED;

	debug(LOG_DEBUG,"nftables mark %s: 0x%x",
		fw_connection_state_as_string(FW_MARK_PREAUTHENTICATED),
		FW_MARK_PREAUTHENTICATED);
	debug(LOG_DEBUG,"nftables mark %s: 0x%x",
		fw_connection_state_as_string(FW_MARK_AUTHENTICATED),
		FW_MARK_AUTHENTICATED);
	debug(LOG_DEBUG,"Inftables mark %s: 0x%x",
		fw_connection_state_as_string(FW_MARK_TRUSTED),
		FW_MARK_TRUSTED);
	return 0;
}

// @internal
int
_iptables_check_mark_masking()
{
	// nft migration - default to mark or-ing
	fw_quiet = 1; // do it quietly
	markop = "--or-mark";
	char *tmp = NULL;
	safe_asprintf(&tmp,"/0x%x",FW_MARK_MASK);
	markmask = tmp;
	fw_quiet = 0; // restore verbosity

	return 0;
}

// @internal
int
nftables_do_command(const char *format, ...)
{
	va_list vlist;
	char *fmt_cmd = NULL;
	int rc;
	int i;

	va_start(vlist, format);
	safe_vasprintf(&fmt_cmd, format, vlist);
	va_end(vlist);

	for (i = 0; i < 5; i++) {

		rc = execute("nft %s", fmt_cmd);
		debug(LOG_DEBUG,"nftables command [ %s ], iteration [ %d ]return code [ %d ]", fmt_cmd, i, rc);

		if (rc != 0) {
			/* nftables error code != 0 indicates a resource problem that might
			 * be temporary. So we retry to insert the rule a few times. (Mitar) */
			sleep(1);
		} else {
			break;
		}
	}

	free(fmt_cmd);

	return rc;
}

int
iptables_trust_mac(const char mac[])
{
	return nftables_do_command("add rule ip nds_mangle %s ether saddr %s counter meta mark set mark or 0x%x", CHAIN_TRUSTED, mac, FW_MARK_TRUSTED);
}

int
iptables_untrust_mac(const char mac[])
{
	return execute("/usr/lib/opennds/libopennds.sh delete_client_rule nds_mangle \"%s\" all \"%s\"", CHAIN_TRUSTED, mac);
}

int create_client_ruleset(char rulesetname[], char ruleset[])
{
	return execute("/usr/lib/opennds/libopennds.sh create_client_ruleset \"%s\" \"%s\"", rulesetname, ruleset);
}

// Initialize the firewall rules.
int
iptables_fw_init(void)
{
	s_config *config;
	char *gw_interface = NULL;
	char *gw_ip = NULL;
	char *gw_address = NULL;
	char *gw_iprange = NULL;
	int gw_port = 0;
	char *fas_remoteip = NULL;
	char *fas_remotefqdn = NULL;
	char *gw_fqdn = NULL;
	int fas_port = 0;
	t_MAC *pt;
	int rc = 0;
	char *dnscmd;
	char *fqdnip;
	char *fqdncmd;

	debug(LOG_NOTICE, "Initializing firewall rules");

	LOCK_CONFIG();
	config = config_get_config();

	gw_interface = safe_strdup(config->gw_interface); // must free
	
	// ip4 vs ip6 differences
	gw_ip = safe_calloc(STATUS_BUF);

	if (config->ip6) {
		// ip6 addresses must be in square brackets like [ffcc:e08::1]
		safe_snprintf(gw_ip, STATUS_BUF, "[%s]", config->gw_ip); // must free
	} else {
		gw_ip = safe_strdup(config->gw_ip);    // must free
	}
	
	if (config->fas_port) {
		fas_remoteip = safe_strdup(config->fas_remoteip);    // must free
		fas_port = config->fas_port;
	}

	fas_remotefqdn = safe_strdup(config->fas_remotefqdn);    // must free
	gw_fqdn = safe_strdup(config->gw_fqdn);    // must free

	debug(LOG_INFO, "fas_remotefqdn [ %s ]", fas_remotefqdn);

	gw_address = safe_strdup(config->gw_address);    // must free

	gw_iprange = safe_strdup(config->gw_iprange);    // must free

	gw_port = config->gw_port;
	pt = config->trustedmaclist;
	FW_MARK_TRUSTED = config->fw_mark_trusted;
	FW_MARK_AUTHENTICATED = config->fw_mark_authenticated;
	FW_MARK_AUTH_BLOCKED = config->fw_mark_auth_blocked;
	UNLOCK_CONFIG();

	// Set up packet marking methods
	rc |= _iptables_init_marks();
	rc |= _iptables_check_mark_masking();

	/*
	 *
	 **************************************
	 * Set up mangle table chains and rules
	 *
	 */


	// Create new chains in the mangle table
	rc |= nftables_do_command("add chain ip nds_mangle " CHAIN_TRUSTED); // for marking trusted packets
	rc |= nftables_do_command("add chain ip nds_mangle " CHAIN_INCOMING); // for counting incoming packets
	rc |= nftables_do_command("add chain ip nds_mangle " CHAIN_DOWNLOAD_RATE); // for controlling download rate per client
	rc |= nftables_do_command("add chain ip nds_mangle " CHAIN_OUTGOING); // for marking authenticated packets, and for counting outgoing packets

	// Assign jumps to these new chains
	rc |= nftables_do_command("insert rule ip nds_mangle %s iifname \"%s\" counter jump %s", CHAIN_PREROUTING, gw_interface, CHAIN_OUTGOING);
	rc |= nftables_do_command("insert rule ip nds_mangle %s iifname \"%s\" counter jump %s", CHAIN_PREROUTING, gw_interface, CHAIN_TRUSTED);
	rc |= nftables_do_command("insert rule ip nds_mangle %s oifname \"%s\" counter jump %s", CHAIN_POSTROUTING, gw_interface, CHAIN_INCOMING);
	rc |= nftables_do_command("insert rule ip nds_mangle %s oifname \"%s\" counter jump %s", CHAIN_INCOMING, gw_interface, CHAIN_FT_INC);
	rc |= nftables_do_command("insert rule ip nds_mangle %s oifname \"%s\" counter jump %s", CHAIN_INCOMING, gw_interface, CHAIN_DOWNLOAD_RATE);

	// Rules to mark as trusted MAC address packets in mangle PREROUTING
	for (; pt != NULL; pt = pt->next) {
		rc |= iptables_trust_mac(pt->mac);
	}

	/*
	 *
	 * End of mangle table chains and rules
	 **************************************
	 */

	/*
	 *
	 **************************************
	 * Set up nat table chains and rules (ip4 only)
	 *
	 */
	 
	if (!config->ip6) {
		// Create new chains in nat table
		rc |= nftables_do_command("add chain ip nds_nat " CHAIN_OUTGOING);

		// nat PREROUTING chain

		// packets coming in on gw_interface jump to CHAIN_OUTGOING
		rc |= nftables_do_command("insert rule ip nds_nat %s iifname \"%s\" counter jump %s", CHAIN_PREROUTING, gw_interface, CHAIN_OUTGOING);

		// CHAIN_OUTGOING, packets marked TRUSTED  ACCEPT
		rc |= nftables_do_command("add rule ip nds_nat %s mark and 0x%x == 0x%x counter return", CHAIN_OUTGOING, FW_MARK_MASK, FW_MARK_TRUSTED);

		// CHAIN_OUTGOING, packets marked AUTHENTICATED  ACCEPT
		rc |= nftables_do_command("add rule ip nds_nat %s mark and 0x%x == 0x%x counter return", CHAIN_OUTGOING, FW_MARK_MASK, FW_MARK_AUTHENTICATED);

		// Allow access to remote FAS - CHAIN_OUTGOING and CHAIN_TO_INTERNET packets for remote FAS, ACCEPT
		if (config->fas_port != 0) {
			if (strcmp(config->fas_remotefqdn, "disabled") != 0) {

				fqdncmd = safe_calloc(SMALL_BUF);
				safe_snprintf(fqdncmd, SMALL_BUF, "/usr/lib/opennds/libopennds.sh resolve_fqdn \"%s\"", fas_remotefqdn);
				fqdnip = safe_calloc(SMALL_BUF);
				rc = execute_ret_url_encoded(fqdnip, SMALL_BUF, fqdncmd);
				rc |= nftables_do_command("add rule ip nds_nat %s ip daddr %s tcp dport %d counter accept", CHAIN_OUTGOING, fqdnip, fas_port);
				free(fqdncmd);
				// do not free(fqdnip) just yet, we will need it again shortly
			} else {

				if (strcmp(config->fas_remoteip, "disabled") != 0) {
					rc |= nftables_do_command("add rule ip nds_nat %s ip daddr %s tcp dport %d counter accept", CHAIN_OUTGOING, fas_remoteip, fas_port);
				} else {
					rc |= nftables_do_command("add rule ip nds_nat %s ip daddr %s tcp dport %d counter accept", CHAIN_OUTGOING, gw_ip, fas_port);
				}
			}
		}

		// CHAIN_OUTGOING, packets for tcp port 80, redirect to gw_port on primary address for the iface
		rc |= nftables_do_command("add rule ip nds_nat %s tcp dport 80 counter dnat to %s", CHAIN_OUTGOING, gw_address);

		// CHAIN_OUTGOING, other packets ACCEPT
		rc |= nftables_do_command("add rule ip nds_nat %s counter accept", CHAIN_OUTGOING);

		if (strcmp(config->gw_fqdn, "disable") != 0) {
			rc |= nftables_do_command("insert rule ip nds_nat ndsOUT ip daddr %s tcp dport 80 counter redirect to :%d", config->gw_ip, config->gw_port);
		}
	}
	/*
	 * End of nat table chains and rules (ip4 only)
	 **************************************
	 */

	/*
	 *
	 **************************************
	 * Set up filter table chains and rules
	 *
	 */

	// Create new chains in the filter table
	rc |= nftables_do_command("add chain ip nds_filter " CHAIN_TO_INTERNET);
	rc |= nftables_do_command("add chain ip nds_filter " CHAIN_TO_ROUTER);
	rc |= nftables_do_command("add chain ip nds_filter " CHAIN_AUTHENTICATED);
	rc |= nftables_do_command("add chain ip nds_filter " CHAIN_UPLOAD_RATE);
	rc |= nftables_do_command("add chain ip nds_filter " CHAIN_FT_OUT); // flowoffload for outgoing packets

	// filter CHAIN_INPUT chain

	// packets coming in on gw_interface jump to CHAIN_TO_ROUTER
	rc |= nftables_do_command("insert rule ip nds_filter %s iifname \"%s\" counter jump %s", CHAIN_INPUT, gw_interface, CHAIN_TO_ROUTER);

	// CHAIN_TO_ROUTER, invalid packets DROP
	rc |= nftables_do_command("add rule ip nds_filter %s ct state invalid counter drop", CHAIN_TO_ROUTER);

	// CHAIN_TO_ROUTER, packets to HTTP listening on gw_port on router ACCEPT
	rc |= nftables_do_command("add rule ip nds_filter %s tcp dport %d counter accept", CHAIN_TO_ROUTER, gw_port);

	// CHAIN_TO_ROUTER, packets to HTTP listening on fas_port on router ACCEPT
	if (fas_port != gw_port && strcmp(fas_remoteip, gw_ip) == 0 && strcmp(fas_remotefqdn, gw_fqdn) == 0) {
		rc |= nftables_do_command("add rule ip nds_filter %s tcp dport %d counter accept", CHAIN_TO_ROUTER, fas_port);
	}

	/*
	 * filter CHAIN_FORWARD chain
	 */

	// packets coming in on gw_interface jump to CHAIN_TO_INTERNET
	rc |= nftables_do_command("insert rule ip nds_filter %s iifname \"%s\" counter jump %s", CHAIN_FORWARD, gw_interface, CHAIN_TO_INTERNET);

	// CHAIN_TO_INTERNET, invalid packets DROP
	rc |= nftables_do_command("add rule ip nds_filter %s ct state invalid counter drop", CHAIN_TO_INTERNET);

	// Allow access to remote FAS - CHAIN_TO_INTERNET packets for remote FAS, ACCEPT

	if (config->fas_port != 0) {
		if (strcmp(config->fas_remotefqdn, "disabled") != 0) {
			rc |= nftables_do_command("add rule ip nds_filter %s ip daddr %s tcp dport %d counter accept", CHAIN_TO_INTERNET, fqdnip, fas_port);
			// Now we can free(fqdnip) as we are now finished with it
			free(fqdnip);
		} else {

			if (strcmp(config->fas_remoteip, "disabled") != 0) {
				rc |= nftables_do_command("add rule ip nds_filter %s ip daddr %s tcp dport %d counter accept", CHAIN_TO_INTERNET, fas_remoteip, fas_port);
			} else {
				rc |= nftables_do_command("add rule ip nds_filter %s ip daddr %s tcp dport %d counter accept", CHAIN_TO_INTERNET, gw_ip, fas_port);
			}
		}
	}

	// CHAIN_TO_INTERNET, packets marked TRUSTED:
	rc |= nftables_do_command("add rule ip nds_filter %s mark and 0x%x == 0x%x counter accept", CHAIN_TO_INTERNET, FW_MARK_MASK, FW_MARK_TRUSTED);

	// CHAIN_TO_INTERNET, packets marked AUTHENTICATED:

	/* if authenticated-users ruleset is empty:
	 *    use empty ruleset policy
	 * else:
	 *    jump to CHAIN_AUTHENTICATED, and load and use authenticated-users ruleset
	 */

	rc |= nftables_do_command("add rule ip nds_filter %s mark and 0x%x == 0x%x counter goto %s", CHAIN_TO_INTERNET, FW_MARK_MASK, FW_MARK_AUTHENTICATED, CHAIN_AUTHENTICATED);

	// CHAIN_AUTHENTICATED, jump to CHAIN_UPLOAD_RATE to handle upload rate limiting
	rc |= nftables_do_command("add rule ip nds_filter %s counter jump %s", CHAIN_AUTHENTICATED, CHAIN_UPLOAD_RATE);

	// CHAIN_AUTHENTICATED, jump to CHAIN_FT_OUT to handle upload flowtable
	rc |= nftables_do_command("add rule ip nds_filter %s counter jump %s", CHAIN_AUTHENTICATED, CHAIN_FT_OUT);

	// CHAIN_AUTHENTICATED, any packets not matching that ruleset ACCEPT
	rc |= nftables_do_command("add rule ip nds_filter %s counter accept", CHAIN_AUTHENTICATED);

	// CHAIN_TO_INTERNET, all other packets REJECT
	rc |= nftables_do_command("add rule ip nds_filter %s counter reject", CHAIN_TO_INTERNET);

	/*
	 * End of filter table chains and rules
	 **************************************
	 */

	// Reload dnsmasq
	dnscmd = safe_calloc(STATUS_BUF);
	safe_snprintf(dnscmd, STATUS_BUF, "/usr/lib/opennds/dnsconfig.sh \"reload_only\"");
	debug(LOG_DEBUG, "reload command [ %s ]", dnscmd);
	if (system(dnscmd) == 0) {
		debug(LOG_INFO, "Dnsmasq reloaded");
	} else {
		debug(LOG_ERR, "Dnsmasq reload failed!");
	}
	free(dnscmd);

	free(gw_interface);
	free(gw_iprange);
	free(gw_ip);
	free(gw_address);
	free(fas_remoteip);
	free(fas_remotefqdn);
	free(gw_fqdn);

	return rc;
}

/** Remove the firewall rules
 * This is used when we do a clean shutdown of opennds,
 * and when it starts, to make sure there are no rules left over from a crash
 */
int
iptables_fw_destroy(void)
{
	char *delchainscmd;
	char *msg;
	fw_quiet = 1;

	debug(LOG_DEBUG, "Destroying our nftables entries");

	// Call library function to delete chains
	safe_asprintf(&delchainscmd, "/usr/lib/opennds/libopennds.sh \"delete_chains\"");
	msg = safe_calloc(STATUS_BUF);

	if (execute_ret_url_encoded(msg, STATUS_BUF - 1, delchainscmd) == 0) {
		debug(LOG_INFO, "Chain delete request sent");
	}
	free(delchainscmd);
	free(msg);

	fw_quiet = 0;

	return 0;
}

/* Enable/Disable Download Rate Limiting for client
	"enable" can be 0, 1
	0 = disable
	1 = enable
*/
int
iptables_download_ratelimit_enable(t_client *client, int enable)
{
	int rc = 0;
	unsigned long long int packet_limit;
	unsigned long long int packets;
	unsigned long long int average_packet_size;
	unsigned long long int bucket;
	char *libcommand = NULL;
	s_config *config;
	config = config_get_config();

	if (client->download_rate == 0) {
		return 0;
	}

	if (client->counters.incoming == 0
		|| client->counters.inpackets == 0
		|| (client->counters.incoming - client->counters.incoming_previous) == 0
		|| (client->counters.inpackets - client->counters.inpackets_previous) == 0
	) {
		average_packet_size = 1500;
		packet_limit = client->download_rate * 1024 * 60 / average_packet_size / 8; // packets per minute
		bucket = 5;
	} else {
		average_packet_size = (client->counters.incoming - client->counters.incoming_previous) /
			(client->counters.inpackets - client->counters.inpackets_previous) + 1;
		packet_limit = client->download_rate * 1024 * 60 / average_packet_size / 8; // packets per minute
		packets = client->downrate * 1024 * 60 / average_packet_size / 8; // packets per minute
		bucket = (packets - packet_limit) / packet_limit;
	}

	bucket = bucket * config->download_bucket_ratio;

	if ( bucket < 5) {
		bucket = 5;
	}

	// max allowed packet limit =300,000 per minute
	if ( packet_limit > 300000) {
		packet_limit = 300000;
	}

	if ( bucket > config->max_download_bucket_size) {
		bucket = config->max_download_bucket_size;
	}

	// Disable
	if (enable == 0) {
		debug(LOG_DEBUG, "client->inc_packet_limit %llu client->download_bucket_size %llu", client->inc_packet_limit, client->download_bucket_size);

		client->inc_packet_limit = 0;
		client->download_bucket_size = 0;

		libcommand = safe_calloc(SMALL_BUF);

		safe_snprintf(libcommand, SMALL_BUF, "/usr/lib/opennds/libopennds.sh replace_client_rule nds_mangle %s return %s \"ip daddr %s counter packets %llu bytes %llu return\"",
			CHAIN_DOWNLOAD_RATE,
			client->ip,
			client->ip,
			client->counters.inpackets,
			client->counters.incoming
		);

		rc = execute(libcommand);
		free(libcommand);

	}

	// Enable
	if (enable == 1) {

		debug(LOG_INFO, "Average Download Packet Size for [%s] is [%llu] bytes", client->ip, average_packet_size);
		debug(LOG_INFO, "Download Rate Limiting of [%s %s] to [%llu] packets/min, bucket size [%llu]", client->ip, client->mac, packet_limit, bucket);
		// Update limiting rule set for this client

		libcommand = safe_calloc(SMALL_BUF);

		safe_snprintf(libcommand, SMALL_BUF, "/usr/lib/opennds/libopennds.sh replace_client_rule nds_mangle %s return %s \"ip daddr %s limit rate %llu/minute burst %llu packets counter packets %llu bytes %llu return\"",
			CHAIN_DOWNLOAD_RATE,
			client->ip,
			client->ip,
			packet_limit,
			bucket,
			client->counters.inpackets,
			client->counters.incoming
		);

		rc = execute(libcommand);
		free(libcommand);

		client->inc_packet_limit = packet_limit;
		client->download_bucket_size = bucket;
	}

	return rc;
}

// Enable/Disable Upload Rate Limiting for client
int
iptables_upload_ratelimit_enable(t_client *client, int enable)
{
	int rc = 0;
	unsigned long long int packet_limit;
	unsigned long long int packets;
	unsigned long long int average_packet_size;
	unsigned long long int bucket;
	char *libcommand = NULL;
	s_config *config;
	config = config_get_config();

	if (client->upload_rate == 0) {
		return 0;
	}

	if (client->counters.outgoing == 0 || client->counters.outpackets == 0 || (client->counters.outgoing - client->counters.outgoing_previous) == 0) {
		average_packet_size = 1500;
		packet_limit = client->upload_rate * 1024 * 60 / average_packet_size / 8; // packets per minute
		bucket = 5;
	} else {
		average_packet_size = (client->counters.outgoing - client->counters.outgoing_previous) /
			(client->counters.outpackets - client->counters.outpackets_previous) + 1;
		packet_limit = client->upload_rate * 1024 * 60 / average_packet_size / 8; // packets per minute
		packets = client->uprate * 1024 * 60 / average_packet_size / 8; // packets per minute
		bucket = (packets - packet_limit) / packet_limit;
	}

	bucket = bucket * config->upload_bucket_ratio;

	if ( bucket < 5) {
		bucket = 5;
	}

	// max allowed packet limit =300,000 per minute
	if ( packet_limit > 300000) {
		packet_limit = 300000;
	}

	if ( bucket > config->max_upload_bucket_size) {
		bucket = config->max_upload_bucket_size;
	}

	// Disable
	if (enable == 0) {

		client->out_packet_limit = 0;
		client->upload_bucket_size = 0;

		libcommand = safe_calloc(SMALL_BUF);

		safe_snprintf(libcommand, SMALL_BUF, "/usr/lib/opennds/libopennds.sh replace_client_rule nds_filter %s return %s \"ip saddr %s counter packets %llu bytes %llu return\"",
			CHAIN_UPLOAD_RATE,
			client->ip,
			client->ip,
			client->counters.outpackets,
			client->counters.outgoing
		);

		rc = execute(libcommand);
		free(libcommand);
	}

	// Enable
	if (enable == 1) {
		debug(LOG_INFO, "Average Upload Packet Size for [%s] is [%llu] bytes", client->ip, average_packet_size);
		debug(LOG_INFO, "Upload Rate Limiting of [%s %s] to [%llu] packets/min, bucket size [%llu]", client->ip, client->mac, packet_limit, bucket);

		// Update limiting rule set for this client
		libcommand = safe_calloc(SMALL_BUF);

		safe_snprintf(libcommand, SMALL_BUF, "/usr/lib/opennds/libopennds.sh replace_client_rule nds_filter %s return %s \"ip saddr %s limit rate %llu/minute burst %llu packets counter packets %llu bytes %llu return\"",
			CHAIN_UPLOAD_RATE,
			client->ip,
			client->ip,
			packet_limit,
			bucket,
			client->counters.outpackets,
			client->counters.outgoing
		);

		rc = execute(libcommand);
		free(libcommand);

		client->out_packet_limit = packet_limit;
		client->upload_bucket_size = bucket;
	}

	return rc;
}

// Insert or delete firewall mangle rules marking a client's packets.
int
iptables_fw_authenticate(t_client *client)
{
	int rc = 0;

	debug(LOG_NOTICE, "Authenticating %s %s", client->ip, client->mac);

	// This rule is for marking upload (outgoing) packets, and for upload byte accounting. Drop all bucket overflow packets
	rc |= nftables_do_command("insert rule ip nds_mangle %s ip saddr %s ether saddr %s counter meta mark set mark or 0x%x", CHAIN_OUTGOING, client->ip, client->mac, FW_MARK_AUTHENTICATED);
	rc |= nftables_do_command("add rule ip nds_filter %s ip saddr %s counter return", CHAIN_UPLOAD_RATE, client->ip);
	rc |= nftables_do_command("add rule ip nds_filter %s ip saddr %s counter drop", CHAIN_UPLOAD_RATE, client->ip);

	// This rule is just for download (incoming) byte accounting. Drop all bucket overflow packets
	rc |= nftables_do_command("insert rule ip nds_mangle %s ip daddr %s counter meta mark set mark or 0x%x", CHAIN_INCOMING, client->ip, FW_MARK_AUTHENTICATED);
	rc |= nftables_do_command("add rule ip nds_mangle %s ip daddr %s counter return", CHAIN_DOWNLOAD_RATE, client->ip);
	rc |= nftables_do_command("add rule ip nds_mangle %s ip daddr %s counter drop", CHAIN_DOWNLOAD_RATE, client->ip);

	client->counters.incoming = 0;
	client->counters.incoming_previous = 0;
	client->counters.outgoing = 0;
	client->counters.outgoing_previous = 0;
	client->counters.inpackets = 0;
	client->counters.inpackets_previous = 0;
	client->counters.outpackets = 0;
	client->counters.outpackets_previous = 0;

	return rc;
}

int
iptables_fw_deauthenticate(t_client *client)
{
	int rc = 0;

	// Remove the authentication rules.
	debug(LOG_NOTICE, "Deauthenticating %s %s", client->ip, client->mac);

	rc = execute("/usr/lib/opennds/libopennds.sh delete_client_rule nds_mangle \"%s\" all \"%s\"", CHAIN_OUTGOING, client->ip);
	rc = execute("/usr/lib/opennds/libopennds.sh delete_client_rule nds_filter \"%s\" all \"%s\"", CHAIN_UPLOAD_RATE, client->ip);
	rc = execute("/usr/lib/opennds/libopennds.sh delete_client_rule nds_mangle \"%s\" all \"%s\"", CHAIN_INCOMING, client->ip);
	rc = execute("/usr/lib/opennds/libopennds.sh delete_client_rule nds_mangle \"%s\" all \"%s\"", CHAIN_DOWNLOAD_RATE, client->ip);

	return rc;
}

// Return the total upload usage in bytes
unsigned long long int
iptables_fw_total_upload()
{
	FILE *output;
	char *script;
	char target[MAX_BUF];
	int rc;
	unsigned long long int packets;
	unsigned long long int counter;

	// Look for outgoing traffic
	safe_asprintf(&script, "nft list chain ip nds_mangle %s 2>/dev/null | grep -w %s ", CHAIN_PREROUTING, CHAIN_OUTGOING);
	output = popen(script, "r");
	free (script);
	
	if (!output) {
		debug(LOG_ERR, "popen(): %s", strerror(errno));
		return 0;
	}

	while (!feof(output)) {
		rc = fscanf(output, "%*s %*s %*s %*s %llu %*s %llu %*s %s ", &packets, &counter, target);
		
		if (3 == rc && !strcmp(target,CHAIN_OUTGOING)) {
			debug(LOG_DEBUG, "Total outgoing traffic [%llu] packets, [%llu] bytes", packets, counter);
			pclose(output);
			return counter;
		}
		// eat rest of line
		while (('\n' != fgetc(output)) && !feof(output)) {}
	}

	pclose(output);
	debug(LOG_INFO, "Can't find target %s in mangle table", CHAIN_OUTGOING);
	return 0;
}

// Return the total download usage in bytes
unsigned long long int
iptables_fw_total_download()
{
	FILE *output;
	char *script;
	char target[MAX_BUF];
	int rc;
	unsigned long long int packets;
	unsigned long long int counter;

	// Look for incoming traffic
	safe_asprintf(&script, "nft list chain ip nds_mangle %s  2>/dev/null | grep -w %s ", CHAIN_POSTROUTING, CHAIN_INCOMING);
	output = popen(script, "r");
	free (script);

	if (!output) {
		debug(LOG_ERR, "popen(): %s", strerror(errno));
		return 0;
	}

	while (!feof(output)) {
		rc = fscanf(output, "%*s %*s %*s %*s %llu %*s %llu %*s %s ", &packets, &counter, target);
		
		if (3 == rc && !strcmp(target, CHAIN_INCOMING)) {
			debug(LOG_DEBUG, "Total incoming traffic [%llu] packets, [%llu] bytes", packets, counter);
			pclose(output);
			return counter;
		}
		// eat rest of line
		while (('\n' != fgetc(output)) && !feof(output)) {}
	}

	pclose(output);
	debug(LOG_INFO, "Can't find target %s in nds_mangle table", CHAIN_INCOMING);
	return 0;
}

// Update the counters of all the clients in the client list
int
iptables_fw_counters_update(void)
{
	FILE *output;
	char *script;
	char ip[INET6_ADDRSTRLEN];
	char target[MAX_BUF];
	int rc;
	int af;
	s_config *config;
	unsigned long long int counter;
	unsigned long long int packets;
	t_client *p1;
	struct sockaddr_storage tempaddr;

	config = config_get_config();
	af = config->ip6 ? AF_INET6 : AF_INET;

	// Look for outgoing (upload) traffic of authenticated clients.
	safe_asprintf(&script, "nft list chain ip nds_mangle %s 2>/dev/null", CHAIN_OUTGOING);
	output = popen(script, "r");
	free(script);

	if (!output) {
		debug(LOG_ERR, "popen(): %s", strerror(errno));
		return -1;
	}

	// skip the first two lines
	while (('\n' != fgetc(output)) && !feof(output)) {}
	while (('\n' != fgetc(output)) && !feof(output)) {}

	while (!feof(output)) {
		rc = fscanf(output, " %*s %*s %15[0-9.] %*s %*s %*s %*s %*s %llu %*s %llu  %*s %*s %*s %*s %*s %*s %s", ip, &packets, &counter, target);

		// eat rest of line
		while (('\n' != fgetc(output)) && !feof(output)) {}

		if (4 == rc && !strcmp(target, config->authentication_mark)) {
			// Sanity
			if (!inet_pton(af, ip, &tempaddr)) {
				debug(LOG_WARNING, "I was supposed to read an IP address but instead got [%s] - ignoring it", ip);
				continue;
			}

			if (strcmp(ip, "0.0.0.0") == 0) {
				continue;
			}


			debug(LOG_DEBUG, "Read outgoing traffic for %s: Bytes=%llu, Packets=%llu", ip, counter, packets);

			if ((p1 = client_list_find_by_ip(ip))) {
				if (p1->counters.outgoing < counter) {
					p1->counters.outgoing_previous = p1->counters.outgoing;
					p1->counters.outgoing = counter;
					p1->counters.outpackets_previous = p1->counters.outpackets;
					p1->counters.outpackets = packets;
					p1->counters.last_updated = time(NULL);

					debug(LOG_DEBUG, "%s - Updated counter.outgoing to %llu bytes, packets=%llu.  Updated last_updated to %d",
						ip,
						counter,
						packets,
						p1->counters.last_updated
					);
				}
			} else {
				debug(LOG_WARNING, "Could not find %s in client list", ip);
			}
		}
	}
	pclose(output);

	// Look for incoming (download) traffic
	safe_asprintf(&script, "nft list chain ip nds_mangle %s 2>/dev/null", CHAIN_INCOMING);
	output = popen(script, "r");
	free(script);

	if (!output) {
		debug(LOG_ERR, "popen(): %s", strerror(errno));
		return -1;
	}

	// skip the first two lines
	while (('\n' != fgetc(output)) && !feof(output)) {}
	while (('\n' != fgetc(output)) && !feof(output)) {}

	while (!feof(output)) {
		rc = fscanf(output, " %*s %*s %15[0-9.] %*s %*s %llu %*s %llu  %*s %*s %*s %*s %*s %*s %s", ip, &packets, &counter, target);

		// eat rest of line
		while (('\n' != fgetc(output)) && !feof(output)) {}

		// Sanity check
		if (4 == rc && !strcmp(target, config->authentication_mark)) {

			if (!inet_pton(af, ip, &tempaddr)) {
				debug(LOG_WARNING, "I was supposed to read an IP address but instead got [%s] - ignoring it", ip);
				continue;
			}

			if (strcmp(ip, "0.0.0.0") == 0) {
				continue;
			}

			debug(LOG_DEBUG, "Read incoming traffic for %s: Bytes=%llu, Packets=%llu", ip, counter, packets);

			if ((p1 = client_list_find_by_ip(ip))) {
				if (p1->counters.incoming < counter) {
					p1->counters.incoming_previous = p1->counters.incoming;
					p1->counters.incoming = counter;
					p1->counters.inpackets_previous = p1->counters.inpackets;
					p1->counters.inpackets = packets;
					debug(LOG_DEBUG, "%s - Updated counter.incoming to %llu bytes, packets=%llu.  Updated last_updated to %d",
						ip,
						counter,
						packets,
						p1->counters.last_updated
					);
				}
			} else {
				debug(LOG_WARNING, "Could not find %s in client list", ip);
			}
		}
	}
	pclose(output);

	return 0;
}
