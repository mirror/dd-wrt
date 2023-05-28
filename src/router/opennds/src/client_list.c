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

/** @file client_list.c
  @brief Client List Functions
  @author Copyright (C) 2004 Alexandre Carmel-Veillex <acv@acv.ca>
  @author Copyright (C) 2007 Paul Kube <nodogsplash@kokoro.ucsd.edu>
  @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <string.h>

#include "safe.h"
#include "debug.h"
#include "conf.h"
#include "client_list.h"
#include "http_microhttpd.h"
#include "fw_iptables.h"
#include "util.h"


// Client counter
static int client_count = 0;
static int client_id = 1;

// Global mutex to protect access to the client list
pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

/** @internal
 * Holds a pointer to the first element of the list
 */
static t_client *firstclient = NULL;

// Return current length of the client list
int
get_client_list_length()
{
	return client_count;
}

// Get the first element of the client list
t_client *
client_get_first_client(void)
{
	return firstclient;
}

// Initialize the list of connected clients
void
client_list_init(void)
{
	firstclient = NULL;
	client_count = 0;
}

/** @internal
 * Given IP, MAC, and client token, appends a new entry
 * to the end of the client list and returns a pointer to the new entry.
 * All the memory allocation for a list entry is done here.
 * Checks for number of current clients.
 * Does not check for duplicate entries; so check before calling.
 * @param ip IP address
 * @param mac MAC address
 * @param token Token
 * @return Pointer to the client we just created
 */
static t_client *
_client_list_append(const char mac[], const char ip[])
{
	t_client *client, *prevclient;
	s_config *config;

	config = config_get_config();
	if (client_count >= config->maxclients) {
		debug(LOG_NOTICE, "Already list %d clients, cannot add %s %s", client_count, ip, mac);
		return NULL;
	}

	prevclient = NULL;
	client = firstclient;

	while (client != NULL) {
		prevclient = client;
		client = client->next;
	}

	client = safe_calloc(sizeof(t_client));

	client->mac = safe_strdup(mac);
	client->ip = safe_strdup(ip);

	// Reset volatile fields and create new token
	client_reset(client);

	// Blocked or Trusted client do not trigger the splash page.
	if (is_blocked_mac(mac)) {
		client->fw_connection_state = FW_MARK_BLOCKED;
	} else if(is_allowed_mac(mac) || is_trusted_mac(mac)) {
		client->fw_connection_state = FW_MARK_TRUSTED;
	} else {
		client->fw_connection_state = FW_MARK_PREAUTHENTICATED;
	}

	client->id = client_id;
	client->out_packet_limit = 0;
	client->inc_packet_limit = 0;

	debug(LOG_NOTICE, "Adding %s %s token %s to client list",
		client->ip, client->mac, client->token ? client->token : "none");

	if (prevclient == NULL) {
		firstclient = client;
	} else {
		prevclient->next = client;
	}

	client_id++;
	client_count++;

	return client;
}

/** @internal
 *  Reset volatile fields
 */
void client_reset(t_client *client)
{
	char hash[128] = {0};
	char msg[16] = {0};
	char *cidinfo;

	debug(LOG_DEBUG, "Resetting client [%s]", client->mac);
	// Reset traffic counters
	client->counters.incoming = 0;
	client->counters.outgoing = 0;
	client->counters.last_updated = time(NULL);

	// Reset session time
	client->session_start = 0;
	client->session_end = 0;

	// Reset token and hid
	free(client->token);
	safe_asprintf(&client->token, "%04hx%04hx", rand16(), rand16());
	hash_str(hash, sizeof(hash), client->token);
	client->hid = safe_strdup(hash);

	// Reset custom and client_type
	client->custom = safe_strdup("\0");
	client->client_type = safe_strdup("\0");

	//Reset cid and remove cidfile using rmcid
	if (client->cid) {

		if (strlen(client->cid) > 0) {
			safe_asprintf(&cidinfo, "cid=\"%s\"\0", client->cid);
			write_client_info(msg, sizeof(msg), "rmcid", client->cid, cidinfo);
			free(cidinfo);
		}
		client->cid = safe_strdup("\0");
	}

}

/**
 *  Given an IP address, add a client corresponding to that IP to client list.
 *  Return a pointer to the new client list entry, or to an existing entry
 *  if one with the given IP already exists.
 *  Return NULL if no new client entry can be created.
 */
t_client *
client_list_add_client(const char mac[], const char ip[])
{
	t_client *client;
	int rc = -1;
	char *libcmd;
	char *msg;

	if (!check_mac_format(mac)) {
		// Inappropriate format in IP address
		debug(LOG_NOTICE, "Illegal MAC format [%s]", mac);
		return NULL;
	}

	if (!check_ip_format(ip)) {
		// Inappropriate format in IP address
		debug(LOG_NOTICE, "Illegal IP format [%s]", ip);
		return NULL;
	}

	// check if client ip was allocated by dhcp
	safe_asprintf(&libcmd, "/usr/lib/opennds/libopennds.sh dhcpcheck \"%s\"", ip);
	msg = safe_calloc(64);
	rc = execute_ret_url_encoded(msg, 64 - 1, libcmd);
	free(libcmd);

	if (rc > 0) {
		// IP address is not in the dhcp database
		debug(LOG_NOTICE, "IP not allocated by dhcp [%s]", ip);
		return NULL;
	}

	client = client_list_find(mac, ip);

	if (!client) {
		client = _client_list_append(mac, ip);
	} else {
		debug(LOG_INFO, "Client %s %s token %s already on client list", ip, mac, client->token);
	}

	return client;
}

/** Finds a client by its token, IP or MAC.
 * A found client is guaranteed to be unique.
 * @param ip IP we are looking for in the linked list
 * @param mac MAC we are looking for in the linked list
 * @return Pointer to the client, or NULL if not found
 */
t_client *
client_list_find_by_any(const char mac[], const char ip[], const char token[])
{
	t_client *client = NULL;

	if (!client && token) {
		client = client_list_find_by_token(token);
	}

	if (!client && ip) {
		client = client_list_find_by_ip(ip);
	}

	if (!client && mac) {
		client = client_list_find_by_mac(mac);
	}

	return client;
}

t_client *
client_list_find(const char mac[], const char ip[])
{
	t_client *ptr;

	ptr = firstclient;
	while (ptr) {
		if (!strcmp(ptr->mac, mac) && !strcmp(ptr->ip, ip)) {
			return ptr;
		}
		ptr = ptr->next;
	}

	return NULL;
}

/**
 * Finds a client by its IP address. Returns NULL if
 * the client could not be found.
 * @return Pointer to the client, or NULL if not found
 */
t_client *
client_list_find_by_id(const unsigned id)
{
	t_client *ptr;

	ptr = firstclient;
	while (ptr) {
		if (ptr->id == id) {
			return ptr;
		}
		ptr = ptr->next;
	}

	return NULL;
}

/**
 * Finds a client by its IP address. Returns NULL if
 * the client could not be found.
 * @return Pointer to the client, or NULL if not found
 */
t_client *
client_list_find_by_ip(const char ip[])
{
	t_client *ptr;

	ptr = firstclient;
	while (ptr) {
		if (!strcmp(ptr->ip, ip)) {
			return ptr;
		}
		ptr = ptr->next;
	}

	return NULL;
}

/**
 * Finds a client by its MAC address. Returns NULL if
 * the client could not be found.
 * @return Pointer to the client, or NULL if not found
 */
t_client *
client_list_find_by_mac(const char mac[])
{
	t_client *ptr;

	ptr = firstclient;
	while (ptr) {
		if (!strcmp(ptr->mac, mac)) {
			return ptr;
		}
		ptr = ptr->next;
	}

	return NULL;
}

/**
 * Finds a client by token. Returns NULL if
 * the client could not be found.
 * @return Pointer to the client, or NULL if not found
 */
t_client *
client_list_find_by_token(const char token[])
{
	t_client *ptr;
	s_config *config;
	config = config_get_config();
	char rhid[128] = {0};
	char *rhidraw = NULL;

	ptr = firstclient;
	while (ptr) {
		//Check if token (tok) or hash_id (hid) mode
		if (strlen(token) > 8) {
			// hid mode
			safe_asprintf(&rhidraw, "%s%s", ptr->hid, config->fas_key);
			hash_str(rhid, sizeof(rhid), rhidraw);
			free (rhidraw);

			if (token && !strcmp(rhid, token)) {
				// rhid is valid
				return ptr;
			}
		} else {
			// tok mode
			if (token && !strcmp(ptr->token, token)) {
				// Token is valid
				return ptr;
			}
		}

		ptr = ptr->next;
	}

	return NULL;
}

/** @internal
 * @brief Frees the memory used by a t_client structure
 * This function frees the memory used by the t_client structure in the
 * proper order.
 * @param client Points to the client to be freed
 */
static void
_client_list_free_node(t_client *client)
{

	char msg[16] = {0};
	char *cidinfo;

	if (client->cid) {

		// Remove any existing cidfile:
		if (strlen(client->cid) > 0) {
			safe_asprintf(&cidinfo, "cid=\"%s\"\0", client->cid);
			write_client_info(msg, sizeof(msg), "rmcid", client->cid, cidinfo);
			free(cidinfo);
		}
		free(client->cid);
	}

	if (client->mac)
		free(client->mac);

	if (client->ip)
		free(client->ip);

	if (client->token)
		free(client->token);

	if (client->hid)
		free(client->hid);

	if (client->client_type)
		free(client->client_type);

	if (client->custom)
		free(client->custom);

	free(client);
}

/**
 * @brief Deletes a client from the client list
 *
 * Removes the specified client from the client list and then calls
 * the function _client_list_free_node to free the memory taken by the client.
 * @param client Points to the client to be deleted
 */
void
client_list_delete(t_client *client)
{
	t_client *ptr;

	ptr = firstclient;

	if (ptr == NULL) {
		debug(LOG_ERR, "Node list empty!");
	} else if (ptr == client) {
		debug(LOG_NOTICE, "Deleting %s %s token %s from client list",
			  client->ip, client->mac, client->token ? client->token : "none");
		firstclient = ptr->next;
		_client_list_free_node(client);
		client_count--;
	} else {
		// Loop forward until we reach our point in the list.
		while (ptr->next != NULL && ptr->next != client) {
			ptr = ptr->next;
		}
		// If we reach the end before finding out element, complain.
		if (ptr->next == NULL) {
			debug(LOG_ERR, "Node to delete could not be found.");
		} else {
			// Free element.
			debug(LOG_NOTICE, "Deleting %s %s token %s from client list",
				  client->ip, client->mac, client->token ? client->token : "none");
			ptr->next = client->next;
			_client_list_free_node(client);
			client_count--;
		}
	}
}
