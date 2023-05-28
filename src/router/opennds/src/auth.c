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

/** @file auth.c
    @brief Authentication handling thread
    @author Copyright (C) 2004 Alexandre Carmel-Veilleux <acv@miniguru.ca>
    @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>

#include "safe.h"
#include "conf.h"
#include "debug.h"
#include "auth.h"
#include "fw_iptables.h"
#include "client_list.h"
#include "util.h"
#include "http_microhttpd_utils.h"
#include "http_microhttpd.h"

#define ENABLE 1
#define DISABLE 0

extern pthread_mutex_t client_list_mutex;
extern pthread_mutex_t config_mutex;

// Count number of authentications
unsigned int authenticated_since_start = 0;


static void binauth_action(t_client *client, const char *reason, const char *customdata)
{
	s_config *config = config_get_config();
	time_t now = time(NULL);
	int seconds = 60 * config->session_timeout;
	unsigned long int sessionstart;
	unsigned long int sessionend;
	char *deauth = "deauth";
	char *client_auth = "client_auth";
	char *ndsctl_auth = "ndsctl_auth";
	char customdata_enc[384] = {0};
	int ret = 1;

	if (config->binauth) {
		debug(LOG_DEBUG, "client->custom=%s", client->custom);

		if (!client->custom || strlen(client->custom) == 0) {
			customdata="none";
		} else {
			customdata=client->custom;
		}

		uh_urlencode(customdata_enc, sizeof(customdata_enc), customdata, strlen(customdata));
		debug(LOG_DEBUG, "binauth_action: customdata_enc [%s]", customdata_enc);

		// get client's current session start and end
		sessionstart = client->session_start;
		sessionend = client->session_end;
		debug(LOG_DEBUG, "binauth_action client: seconds=%lu, sessionstart=%lu, sessionend=%lu", seconds, sessionstart, sessionend);

		// Check for client_auth reason
		if (strstr(reason, client_auth) != NULL) {
			sessionstart = now;
		}

		// Check for a deauth reason
		if (strstr(reason, deauth) != NULL) {
			sessionend = now;
		}

		// Check for ndsctl_auth reason
		if (strstr(reason, ndsctl_auth) != NULL) {
			sessionstart = now;
		}

		// ndsctl will deadlock if run within the BinAuth script so lock it.
		// But if a call to ndsctl auth or deauth brought us here then it is locked already.
		if (strstr(reason, deauth) == NULL && strstr(reason, ndsctl_auth) == NULL) {
			ret=ndsctl_lock();
		}

		debug(LOG_DEBUG, "BinAuth %s - client session end time: [ %lu ]", reason, sessionend);

		execute("%s %s %s %llu %llu %lu %lu %s %s",
			config->binauth,
			reason ? reason : "unknown",
			client->mac,
			client->counters.incoming,
			client->counters.outgoing,
			sessionstart,
			sessionend,
			client->token,
			customdata_enc
		);

		if (strstr(reason, deauth) == NULL && strstr(reason, ndsctl_auth) == NULL) {
			// unlock ndsctl
			if (ret == 0) {
				ndsctl_unlock();
			}
		}
	}
}

static int auth_change_state(t_client *client, const unsigned int new_state, const char *reason, const char *customdata)
{
	const unsigned int state = client->fw_connection_state;
	const time_t now = time(NULL);
	int action;
	s_config *config = config_get_config();

	if (state == new_state) {
		return -1;
	} else if (state == FW_MARK_PREAUTHENTICATED) {
		if (new_state == FW_MARK_AUTHENTICATED) {
			iptables_fw_authenticate(client);

			if (client->upload_rate == 0) {
				client->upload_rate = config->upload_rate;
			}

			if (client->download_rate == 0) {
				client->download_rate = config->download_rate;
			}

			if (client->upload_quota == 0) {
				client->upload_quota = config->upload_quota;
			}

			if (client->download_quota == 0) {
				client->download_quota = config->download_quota;
			}

			debug(LOG_DEBUG, "auth_change_state > authenticated - download_rate [%llu] upload_rate [%llu] ",
				client->download_rate,
				client->upload_rate
			);

			client->window_start = now;
			client->window_counter = config->rate_check_window;
			client->initial_loop = 1;
			client->counters.in_window_start = client->counters.incoming;
			client->counters.out_window_start = client->counters.outgoing;


			if (customdata && strlen(customdata) > 0) {
				client->custom = safe_strdup(customdata);
			} else {
				client->custom = "bmE=";
			}

			debug(LOG_DEBUG, "auth_change_state: client->custom=%s ", client->custom);

			action = ENABLE;

			// Update all the counters
			if (-1 == iptables_fw_counters_update()) {
				debug(LOG_ERR, "Could not get counters from firewall!");
				return -1;
			}


			if (config->download_unrestricted_bursting == 0 && config->download_bucket_ratio > 0) {
				iptables_download_ratelimit_enable(client, action);
				//bit 0 is not set so toggle it to signify rate limiting is on
				client->rate_exceeded = client->rate_exceeded^1;
			}

			if (config->upload_unrestricted_bursting == 0 && config->upload_bucket_ratio > 0) {
				iptables_upload_ratelimit_enable(client, action);
				//bit 1 is not set so toggle it to signify rate limiting is on
				client->rate_exceeded = client->rate_exceeded^2;
			}

			binauth_action(client, reason, customdata);
		} else if (new_state == FW_MARK_BLOCKED) {
			return -1;
		} else if (new_state == FW_MARK_TRUSTED) {
			return -1;
		} else {
			return -1;
		}
	} else if (state == FW_MARK_AUTHENTICATED) {
		if (new_state == FW_MARK_PREAUTHENTICATED) {
			iptables_fw_deauthenticate(client);
			binauth_action(client, reason, customdata);
			client_reset(client);
			client_list_delete(client);
		} else if (new_state == FW_MARK_BLOCKED) {
			return -1;
		} else if (new_state == FW_MARK_TRUSTED) {
			return -1;
		} else {
			return -1;
		}
	} else if (state == FW_MARK_BLOCKED) {
		if (new_state == FW_MARK_PREAUTHENTICATED) {
			return -1;
		} else if (new_state == FW_MARK_AUTHENTICATED) {
			return -1;
		} else if (new_state == FW_MARK_TRUSTED) {
			return -1;
		} else {
			return -1;
		}
	} else if (state == FW_MARK_TRUSTED) {
		if (new_state == FW_MARK_PREAUTHENTICATED) {
			return -1;
		} else if (new_state == FW_MARK_AUTHENTICATED) {
			return -1;
		} else if (new_state == FW_MARK_BLOCKED) {
			return -1;
		} else {
			return -1;
		}
	} else {
		return -1;
	}

	client->fw_connection_state = new_state;

	return 0;
}

/** See if they are still active,
 *  refresh their traffic counters,
 *  remove and deny them if timed out
 */
static void
fw_refresh_client_list(void)
{
	t_client *cp1, *cp2;
	s_config *config = config_get_config();
	const int preauth_idle_timeout_secs = 60 * config->preauth_idle_timeout;
	const int auth_idle_timeout_secs = 60 * config->auth_idle_timeout;
	const int remotes_refresh_interval_secs = 60 * config->remotes_refresh_interval;
	const time_t now = time(NULL);
	unsigned long long int durationsecs;
	unsigned long long int download_bytes, upload_bytes;
	unsigned long long int uprate;
	unsigned long long int downrate;
	int action;

	// Check if router is online
	int watchdog = 1;
	int routercheck;
	routercheck = check_routing(watchdog);

	if (routercheck > 0) {
		/* If the refresh interval has expired, refresh the downloaded remote files.
			This can be used to update data files or images used by openNDS from storage on a remote server.
			Access to the openNDS router is not required to update these files as openNDS downloads them.
			The primary uses are:
				to provide up to date info to clients.
				to provide adverising content that automatically updates.
		*/

		if (remotes_refresh_interval_secs > 0 ) {
			// Refresh downloaded files with new ones
			if ((config->remotes_last_refresh + remotes_refresh_interval_secs) <= now) {
				download_remotes(1);
				config->remotes_last_refresh = now;
			} else {
				// Check if all required files are present, if any are missing, download them
				download_remotes(0);
			}
		}
	}

	debug(LOG_DEBUG, "Rate Check Window is set to %u period(s) of checkinterval", config->rate_check_window);

	// Update all the counters
	if (-1 == iptables_fw_counters_update()) {
		debug(LOG_ERR, "Could not get counters from firewall!");
		return;
	}

	LOCK_CLIENT_LIST();

	for (cp1 = cp2 = client_get_first_client(); NULL != cp1; cp1 = cp2) {
		cp2 = cp1->next;

		if (!(cp1 = client_list_find_by_id(cp1->id))) {
			debug(LOG_ERR, "Client was freed while being re-validated!");
			continue;
		}

		time_t last_updated = cp1->counters.last_updated;

		unsigned int conn_state = cp1->fw_connection_state;

		debug(LOG_DEBUG, "conn_state [%x]", conn_state);

		if (conn_state == FW_MARK_PREAUTHENTICATED) {

			// Preauthenticated client reached Idle Timeout without authenticating so delete from the client list
			if (preauth_idle_timeout_secs > 0
				&& conn_state == FW_MARK_PREAUTHENTICATED
				&& (last_updated + preauth_idle_timeout_secs) <= now)
				{

				debug(LOG_NOTICE, "Timeout preauthenticated idle user: %s %s, inactive: %lus",
					cp1->ip,
					cp1->mac, now - last_updated
				);

				client_list_delete(cp1);
			}
			continue;
		}

		debug(LOG_INFO, "Client @ %s %s, quotas: ", cp1->ip, cp1->mac);

		debug(LOG_INFO, "	Download DATA quota (kBytes): %llu, used: %llu ", cp1->download_quota, cp1->counters.incoming / 1024);

		debug(LOG_INFO, "	Upload DATA quota (kBytes): %llu, used: %llu \n", cp1->upload_quota, cp1->counters.outgoing / 1024);

		if (cp1->session_end > 0 && cp1->session_end <= now) {
			// Session Timeout so deauthenticate the client

			debug(LOG_NOTICE, "Session end time reached, deauthenticating: %s %s, connected: %lu, in: %llukB, out: %llukB",
				cp1->ip, cp1->mac, now - cp1->session_end,
				cp1->counters.incoming / 1024,
				cp1->counters.outgoing / 1024
			);

			auth_change_state(cp1, FW_MARK_PREAUTHENTICATED, "timeout_deauth", NULL);


		} else if (cp1->download_quota > 0 && cp1->download_quota <= (cp1->counters.incoming / 1024)) {
			// Download quota reached so deauthenticate the client

			debug(LOG_NOTICE, "Download quota reached, deauthenticating: %s %s, connected: %lus, in: %llukB, out: %llukB",
				cp1->ip, cp1->mac,
				now - cp1->session_end,
				cp1->counters.incoming / 1024,
				cp1->counters.outgoing / 1024
			);

			auth_change_state(cp1, FW_MARK_PREAUTHENTICATED, "downquota_deauth", NULL);

		} else if (cp1->upload_quota > 0 && cp1->upload_quota <= (cp1->counters.outgoing / 1024)) {
			// Upload quota reached so deauthenticate the client

			debug(LOG_NOTICE, "Upload quota reached, deauthenticating: %s %s, connected: %lus, in: %llukB, out: %llukB",
				cp1->ip,
				cp1->mac,
				now - cp1->session_end,
				cp1->counters.incoming / 1024,
				cp1->counters.outgoing / 1024
			);

			auth_change_state(cp1, FW_MARK_PREAUTHENTICATED, "upquota_deauth", NULL);

		} else if (auth_idle_timeout_secs > 0
				&& conn_state == FW_MARK_AUTHENTICATED
				&& (last_updated + auth_idle_timeout_secs) <= now) {
			// Authenticated client reached Idle Timeout so deauthenticate the client

			debug(LOG_NOTICE, "Timeout authenticated idle user: %s %s, inactive: %ds, in: %llukB, out: %llukB",
				cp1->ip, cp1->mac, now - last_updated,
				cp1->counters.incoming / 1024,
				cp1->counters.outgoing / 1024
			);

			auth_change_state(cp1, FW_MARK_PREAUTHENTICATED, "idle_deauth", NULL);

		}

		// Now we need to process rate quotas, so first refresh the connection state in case it has changed
		conn_state = cp1->fw_connection_state;


		if (conn_state != FW_MARK_PREAUTHENTICATED) {

			debug(LOG_DEBUG, "Window start [%lu] - window counter [%u]",
				cp1->window_start,
				cp1->window_counter
			);

			debug(LOG_DEBUG, "in_window_start [%llu] - out_window_start [%llu]",
				cp1->counters.in_window_start,
				cp1->counters.out_window_start
			);

			durationsecs = ((now - cp1->window_start)+1);
			// Note: 1 second added to prevent divide by zero if this code runs within 1 second of the client being authenticated

			// Calculate actual rates
			download_bytes = (cp1->counters.incoming - cp1->counters.in_window_start);
			upload_bytes = (cp1->counters.outgoing - cp1->counters.out_window_start);
			downrate = (download_bytes / 125 / durationsecs); // kbits/sec
			uprate = (upload_bytes / 125 / durationsecs); // kbits/sec
			cp1->downrate = downrate;
			cp1->uprate = uprate;

			debug(LOG_DEBUG, "durationsecs [%llu] download_bytes [%llu] upload_bytes [%llu] ",
				durationsecs,
				download_bytes,
				upload_bytes
			);

			debug(LOG_INFO, "	Download RATE quota (kbits/s): %llu, Current average download rate (kbits/s): %llu",
				cp1->download_rate, downrate
			);

			debug(LOG_INFO, "	Upload RATE quota (kbits/s): %llu, Current average upload rate (kbits/s): %llu",
				cp1->upload_rate, uprate
			);

			/*	OR with set bit sets the bit in a variable
				AND with set bit checks if bit is set in a variable
				XOR with set bit toggles bit in a variable
				In variable rate_exceeded we are interested in bits zero and one
				Bit zero pertains to download
				Bit one pertains to upload
				So variable can have integer values of 0, 1, 2 or 3
			*/

			// ** ENABLE rate limiting **
			// Has the client exceeded its rate quotas?
			// If so we should enable limiting

			//Handle download rate limiting

			if ((cp1->rate_exceeded&1) == 0) {
				// note checked for bit 0 of rate_exceeded set to 0, it was so we are here 
				if (cp1->download_rate > 0 && cp1->download_rate <= downrate) {
					debug(LOG_INFO, "Download RATE quota reached for: %s %s, in: %llukbits/s, out: %llukbits/s",
						cp1->ip, cp1->mac,
						downrate,
						uprate
					);
					action = ENABLE;

					if (config->download_bucket_ratio > 0) {
						iptables_download_ratelimit_enable(cp1, action);
						//bit 0 is not set so toggle it to signify rate limiting is on
						cp1->rate_exceeded = cp1->rate_exceeded^1;
					} else {
						debug(LOG_INFO, "Download RATE limiting is disabled");
					}
				}
			}

			//Handle upload rate limiting

			if ((cp1->rate_exceeded&2) == 0) {
				// note checked for bit 1 of rate_exceeded set to 0, it was so we are here 
				if (cp1->upload_rate > 0 && cp1->upload_rate <= uprate) {
					debug(LOG_INFO, "Upload RATE quota reached for: %s %s, in: %llukbits/s, out: %llukbits/s",
						cp1->ip, cp1->mac,
						downrate,
						uprate
					);

					action = ENABLE;

					if (config->upload_bucket_ratio > 0) {
						iptables_upload_ratelimit_enable(cp1, action);
						//bit 1 is not set so toggle it to signify rate limiting is on
						cp1->rate_exceeded = cp1->rate_exceeded^2;
					} else {
						debug(LOG_INFO, "Upload RATE limiting is disabled");
					}
				}
			}

			// ** DISABLE or refresh rate limiting **
			// Has ratecheck window expired?
			// Has client rate dropped below threshold?
			// If so we should disable limiting
			if (cp1->window_counter > 1) {
				--cp1->window_counter;
				// skip to next client
				continue;

			} else if (cp1->window_counter < 2) {
				// we have decremented from ratecheckwindow value to expiry
				// so check clients upload and download
				// If rates are below threshold we can disable
				//reset ratecheckwindow
				cp1->window_counter = config->rate_check_window;
				cp1->window_start = now;
				cp1->counters.in_window_start = cp1->counters.incoming;
				cp1->counters.out_window_start = cp1->counters.outgoing;


				//Handle download rate limiting

				if (cp1->download_rate > 0 && (cp1->rate_exceeded&1) == 1) {
					// note checked for bit 0 of rate_exceeded set to 1, it was so we are here 
					if (cp1->download_rate > 0 && cp1->download_rate > downrate) {
						// dropped below threshold

						if (config->download_unrestricted_bursting > 0) {
							//Unrestricted bursting is enabled
							debug(LOG_INFO,
								"Download RATE below quota threshold - bursting allowed: %s %s, in: %llukbits/s, out: %llukbits/s",
								cp1->ip, cp1->mac,
								downrate,
								uprate
							);

							action = DISABLE;
							iptables_download_ratelimit_enable(cp1, action);
							debug(LOG_INFO, "Download Rate Limiting for [%s] [%s] is off", cp1->ip, cp1->mac);
							//bit 0 is set so toggle it to signify rate limiting is off
							cp1->rate_exceeded = cp1->rate_exceeded^1;
						} else {
							// refresh rate limiting
							//Unrestricted bursting is disabled
							debug(LOG_DEBUG, "Refreshing Download Rate Limiting for [%s] [%s]", cp1->ip, cp1->mac);
							action = DISABLE;
							debug(LOG_DEBUG, "Refresh - Disabling Download Rate Limiting for [%s] [%s]", cp1->ip, cp1->mac);
							iptables_download_ratelimit_enable(cp1, action);
							action = ENABLE;
							debug(LOG_DEBUG, "Refresh - Enabling Download Rate Limiting for [%s] [%s]", cp1->ip, cp1->mac);
							iptables_download_ratelimit_enable(cp1, action);
						}
					} else {
						// still above threshold
						// refresh rate limiting
						debug(LOG_DEBUG, "Above threshold - Refreshing Download Rate Limiting for [%s] [%s]", cp1->ip, cp1->mac);
						action = DISABLE;
						iptables_download_ratelimit_enable(cp1, action);
						action = ENABLE;
						iptables_download_ratelimit_enable(cp1, action);
					}
				}

				//Handle upload rate limiting
				debug(LOG_DEBUG, "cp1->upload_rate: %llu uprate: %llu", cp1->upload_rate, uprate);

				if (cp1->upload_rate > 0 && (cp1->rate_exceeded&2) == 2) {
					// note checked for bit 1 of rate_exceeded set to 1, it was so we are here 
					if (cp1->upload_rate > 0 && cp1->upload_rate > uprate) {

						if (config->upload_unrestricted_bursting > 0) {
							debug(LOG_INFO,
								"Upload RATE below quota threshold - bursting allowed: %s %s, in: %llukbits/s, out: %llukbits/s",
								cp1->ip, cp1->mac,
								downrate,
								uprate
							);

							action = DISABLE;
							iptables_upload_ratelimit_enable(cp1, action);
							debug(LOG_INFO, "Upload Rate Limiting for [%s] [%s] is off", cp1->ip, cp1->mac);
							//bit 1 is set so toggle it to signify rate limiting is off
							cp1->rate_exceeded = cp1->rate_exceeded^2;
						} else {
							// refresh rate limiting
							action = DISABLE;
							iptables_upload_ratelimit_enable(cp1, action);
							action = ENABLE;
							iptables_upload_ratelimit_enable(cp1, action);
						}
					} else {
						// refresh rate limiting
						debug(LOG_DEBUG, "Above threshold - Refreshing Upload Rate Limiting for [%s] [%s]", cp1->ip, cp1->mac);
						action = DISABLE;
						iptables_upload_ratelimit_enable(cp1, action);
						action = ENABLE;
						iptables_upload_ratelimit_enable(cp1, action);
					}
				}
			}
		}
	}

	UNLOCK_CLIENT_LIST();
}

/** Launched in its own thread.
 *  This just wakes up every config.checkinterval seconds, and calls fw_refresh_client_list()
@todo This thread loops infinitely, need a watchdog to verify that it is still running?
*/
void *
thread_client_timeout_check(void *arg)
{
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
	struct timespec timeout;
	char msg[8] = {0};
	const char mhd_fail[] = "2";
	char *testcmd;
	s_config *config = config_get_config();

	// Build command to check MHD
	safe_asprintf(&testcmd,
		"/usr/lib/opennds/libopennds.sh mhdcheck \"%s\"",
		config->gw_address
	);


	while (1) {
		// check gateway mac
		config->gw_mac = get_iface_mac(config->gw_interface);
		debug(LOG_DEBUG, "Watchdog: Gateway Interface [%s], mac [%s]", config->gw_interface, config->gw_mac);

		// check MHD
		if (execute_ret_url_encoded(msg, sizeof(msg) - 1, testcmd) == 0) {
			debug(LOG_DEBUG, "MHD Test Result: %s", msg);
		} else {
			debug(LOG_DEBUG, "MHD Test failed: Continuing...");
		}

		if (strcmp(msg, mhd_fail) == 0) {
			debug(LOG_INFO, "MHD Watchdog - Restart requested");
			debug(LOG_DEBUG, "MHD Watchdog - Attempting to stop failing MHD instance");
			stop_mhd();
			debug(LOG_DEBUG, "MHD Watchdog - Restarting MHD");
			start_mhd();
			debug(LOG_INFO, "MHD Restarted");
		}

		memset(msg, 0, sizeof(msg));

		debug(LOG_DEBUG, "Starting Refresh Client List");

		fw_refresh_client_list();

		debug(LOG_DEBUG, "Client List Refresh is Done");

		// Sleep for config.checkinterval seconds...
		timeout.tv_sec = time(NULL) + config_get_config()->checkinterval;
		timeout.tv_nsec = 0;

		// Mutex must be locked for pthread_cond_timedwait...
		pthread_mutex_lock(&cond_mutex);

		// Thread safe "sleep"
		pthread_cond_timedwait(&cond, &cond_mutex, &timeout);

		// No longer needs to be locked
		pthread_mutex_unlock(&cond_mutex);
	}

	free(testcmd);
	return NULL;
}

/** Take action on a client.
 * Alter the firewall rules and client list accordingly.
*/
int
auth_client_deauth(const unsigned id, const char *reason)
{
	t_client *client;
	int rc = -1;

	LOCK_CLIENT_LIST();

	client = client_list_find_by_id(id);

	// Client should already have hit the server and be on the client list
	if (client == NULL) {
		debug(LOG_ERR, "Client %u to deauthenticate is not on client list", id);
		goto end;
	}

	rc = auth_change_state(client, FW_MARK_PREAUTHENTICATED, reason, NULL);

end:
	UNLOCK_CLIENT_LIST();
	return rc;
}


/**
 * @brief auth_client_auth_nolock authenticate a client without holding the CLIENT_LIST lock
 * @param id the client id
 * @param reason can be NULL
 * @return 0 on success
 */
int
auth_client_auth_nolock(const unsigned id, const char *reason, const char *customdata)
{
	t_client *client;
	int rc;

	client = client_list_find_by_id(id);

	// Client should already have hit the server and be on the client list
	if (client == NULL) {
		debug(LOG_ERR, "Client %u to authenticate is not on client list", id);
		return -1;
	}

	rc = auth_change_state(client, FW_MARK_AUTHENTICATED, reason, customdata);
	if (rc == 0) {
		authenticated_since_start++;
	}

	return rc;
}

int
auth_client_auth(const unsigned id, const char *reason, const char *customdata)
{
	int rc;

	LOCK_CLIENT_LIST();
	rc = auth_client_auth_nolock(id, reason, customdata);
	UNLOCK_CLIENT_LIST();

	return rc;
}

int
auth_client_trust(const char *mac)
{
	int rc = -1;

	LOCK_CONFIG();

	if (!add_to_trusted_mac_list(mac) && !iptables_trust_mac(mac)) {
		rc = 0;
	}

	UNLOCK_CONFIG();

	return rc;
}

int
auth_client_untrust(const char *mac)
{
	int rc = -1;

	LOCK_CONFIG();

	if (!remove_from_trusted_mac_list(mac) && !iptables_untrust_mac(mac)) {
		rc = 0;
	}

	UNLOCK_CONFIG();
	return rc;
}

int
auth_client_allow(const char *mac)
{
	int rc = -1;

	LOCK_CONFIG();

	if (!add_to_allowed_mac_list(mac) && !iptables_allow_mac(mac)) {
		rc = 0;
	}

	UNLOCK_CONFIG();

	return rc;
}

int
auth_client_unallow(const char *mac)
{
	int rc = -1;

	LOCK_CONFIG();

	if (!remove_from_allowed_mac_list(mac) && !iptables_unallow_mac(mac)) {
		rc = 0;
	}

	UNLOCK_CONFIG();

	return rc;
}

int
auth_client_block(const char *mac)
{
	int rc = -1;

	LOCK_CONFIG();

	if (!add_to_blocked_mac_list(mac) && !iptables_block_mac(mac)) {
		rc = 0;
	}

	UNLOCK_CONFIG();

	return rc;
}

int
auth_client_unblock(const char *mac)
{
	int rc = -1;

	LOCK_CONFIG();

	if (!remove_from_blocked_mac_list(mac) && !iptables_unblock_mac(mac)) {
		rc = 0;
	}

	UNLOCK_CONFIG();

	return rc;
}

void
auth_client_deauth_all()
{
	t_client *cp1, *cp2;

	LOCK_CLIENT_LIST();

	for (cp1 = cp2 = client_get_first_client(); NULL != cp1; cp1 = cp2) {
		cp2 = cp1->next;

		if (!(cp1 = client_list_find_by_id(cp1->id))) {
			debug(LOG_ERR, "Client was freed while being re-validated!");
			continue;
		}

		auth_change_state(cp1, FW_MARK_PREAUTHENTICATED, "shutdown_deauth", NULL);
	}

	UNLOCK_CLIENT_LIST();
}
