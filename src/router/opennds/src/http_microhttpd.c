/************************************************************************\
 * This program is free software; you can redistribute it and/or	*
 * modify it under the terms of the GNU General Public License as	*
 * published by the Free:Software Foundation; either version 2 of	*
 * the License, or (at your option) any later version.			*
 *									*
 * This program is distributed in the hope that it will be useful,	*
 * but WITHOUT ANY WARRANTY; without even the implied warranty of	*
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.		*
 * See the GNU General Public License for more details.		*
\************************************************************************/

/** @internal
 * @file http_microhttpd.c
 * @brief a httpd implementation using libmicrohttpd
 * @author Copyright (C) 2015 Alexander Couzens <lynxis@fe80.eu>
 * @author Copyright (C) 2015-2023 The openNDS contributors <opennds@blue-wave.net>
 * @author Copyright (C) 2015-2025 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
 */


#include <microhttpd.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "client_list.h"
#include "conf.h"
#include "common.h"
#include "debug.h"
#include "auth.h"
#include "http_microhttpd.h"
#include "http_microhttpd_utils.h"
#include "fw_iptables.h"
#include "mimetypes.h"
#include "safe.h"
#include "util.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

// Max length of a query string QUERYMAXLEN in bytes defined in common.h

// Max dynamic html page size HTMLMAXSIZE in bytes defined in common.h


static t_client *add_client(const char mac[], const char ip[]);
static int authenticated(struct MHD_Connection *connection, const char *url, t_client *client);
static int preauthenticated(struct MHD_Connection *connection, const char *url, t_client *client);
static int authenticate_client(struct MHD_Connection *connection, const char *redirect_url, t_client *client);
static enum MHD_Result get_host_value_callback(void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
static enum MHD_Result get_user_agent_callback(void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
static enum MHD_Result get_accept_callback(void *cls, enum MHD_ValueKind kind, const char *key, const char *value);
static int serve_file(struct MHD_Connection *connection, t_client *client, const char *url);
static int show_preauthpage(struct MHD_Connection *connection, const char *query);
static int send_json(struct MHD_Connection *connection, const char *json);
static int encode_and_redirect_to_splashpage(struct MHD_Connection *connection, t_client *client, const char *originurl, const char *querystr);
static int redirect_to_splashpage(struct MHD_Connection *connection, t_client *client, const char *host, const char *url);
static int send_error(struct MHD_Connection *connection, int error);
static int send_redirect_temp(struct MHD_Connection *connection, t_client *client, const char *url);
static int is_foreign_hosts(struct MHD_Connection *connection, const char *host);
static int get_query(struct MHD_Connection *connection, char **collect_query, const char *separator);
static char *construct_querystring(struct MHD_Connection *connection, t_client *client, char *originurl, char *querystr);
static const char *lookup_mimetype(const char *filename);

struct MHD_Daemon * webserver = NULL;

void stop_mhd(void)
{
	debug(LOG_INFO, "Calling MHD_stop_daemon [%lu]", webserver);
	MHD_stop_daemon(webserver);
}

void start_mhd(void)
{
	// Initializes the web server
	s_config *config;
	config = config_get_config();

	if ((webserver = MHD_start_daemon(
		MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_THREAD_PER_CONNECTION | MHD_USE_TCP_FASTOPEN,
		config->gw_port,
		NULL,
		NULL,
		libmicrohttpd_cb,
		NULL,
		MHD_OPTION_CONNECTION_LIMIT, (unsigned int) 100,
		MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 10,
		MHD_OPTION_PER_IP_CONNECTION_LIMIT, (unsigned int) 10,
		MHD_OPTION_END)) == NULL) {

		debug(LOG_ERR, "Could not create web server: %s", strerror(errno));
		exit(1);

	}

	debug(LOG_INFO, "MHD Handle [%lu]", webserver);
}


/* Call the BinAuth script or program with output and input arguments.
 * Output arguments to BinAuth:
 * We will send client->mac, username_enc, password_enc, redirect_url_enc_buf, enc_user_agent, client->ip, client->token, custom_enc.
 * The BinAuth script will return &seconds, &upload_rate, &download_rate, &upload_quota, &download_quota
 *
 * Input arguments from BinAuth:
 * BinAuth will return values for Session length, rates and quotas.
 * It is the responsibility of the script to obtain/calculate/generate these values.
*/

static int do_binauth(
	struct MHD_Connection *connection,
	const char *binauth,
	t_client *client,
	int *seconds_ret,
	unsigned long long int *upload_rate_ret,
	unsigned long long int *download_rate_ret,
	unsigned long long int *upload_quota_ret,
	unsigned long long int *download_quota_ret,
	const char *redirect_url
	)
{

	char *redirect_url_enc_buf;
	const char *custom;
	char *custom_enc;
	char *msg;
	char *argv = NULL;
	const char *user_agent;
	char *enc_user_agent;
	int seconds;
	unsigned long long int upload_rate;
	unsigned long long int download_rate;
	unsigned long long int upload_quota;
	unsigned long long int download_quota;
	int rc =1;

	// Get the client user agent
	user_agent = safe_calloc(USER_AGENT);

	MHD_get_connection_values(connection, MHD_HEADER_KIND, get_user_agent_callback, &user_agent);

	if (user_agent == NULL) {
		return send_error(connection, 403);
	}

	debug(LOG_DEBUG, "BinAuth: User Agent is [ %s ]", user_agent);

	// Get custom data string as passed in the query string

	custom = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "custom");

	if (!custom || strlen(custom) == 0) {
		custom="bmE=";
	}

	custom_enc = safe_calloc(CUSTOM_ENC);
	uh_urlencode(custom_enc, CUSTOM_ENC, custom, strlen(custom));

	debug(LOG_DEBUG, "BinAuth: custom data [ %s ]", custom_enc);

	redirect_url_enc_buf = safe_calloc(REDIRECT_URL_ENC_BUF);
	uh_urlencode(redirect_url_enc_buf, REDIRECT_URL_ENC_BUF, redirect_url, strlen(redirect_url));

	debug(LOG_DEBUG, "BinAuth: Redirect URL is [ %s ]", redirect_url_enc_buf);

	enc_user_agent = safe_calloc(ENC_USER_AGENT);
	uh_urlencode(enc_user_agent, ENC_USER_AGENT, user_agent, strlen(user_agent));

	debug(LOG_DEBUG, "BinAuth: User Agent is [ %s ]", enc_user_agent);

	// Note: username, password and user_agent may contain spaces so argument should be quoted
	argv = safe_calloc(REDIRECT_URL_ENC_BUF + CUSTOM_ENC);
	safe_snprintf(argv, REDIRECT_URL_ENC_BUF + CUSTOM_ENC, "%s auth_client '%s' '%s' '%s' '%s' '%s' '%s'",
		binauth,
		client->mac,
		redirect_url_enc_buf,
		enc_user_agent,
		client->ip,
		client->token,
		custom_enc
	);

	debug(LOG_DEBUG, "BinAuth argv: %s", argv);

	// ndsctl will deadlock if run within the BinAuth script so lock it
	msg = safe_calloc(SMALL_BUF);

	if(ndsctl_lock() == 0) {
		// execute the script
		rc = execute_ret_url_encoded(msg, SMALL_BUF, argv);
		debug(LOG_DEBUG, "BinAuth returned arguments: %s", msg);

		// unlock ndsctl
		ndsctl_unlock();
	}

	free(custom_enc);
	free(redirect_url_enc_buf);
	free(enc_user_agent);
	free(argv);


	if (rc != 0) {
		debug(LOG_DEBUG, "BinAuth script failed to execute");
		free(msg);
		return 0;
	}

	rc = sscanf(msg, "%d %llu %llu %llu %llu", &seconds, &upload_rate, &download_rate, &upload_quota, &download_quota);
	debug(LOG_DEBUG, "BinAuth returned session length: %d", seconds);
	free(msg);

	// store assigned parameters
	switch (rc) {
		case 5:
			*download_quota_ret = MAX(download_quota, 0);
		case 4:
			*upload_quota_ret = MAX(upload_quota, 0);
		case 3:
			*download_rate_ret = MAX(download_rate, 0);
		case 2:
			*upload_rate_ret = MAX(upload_rate, 0);
		case 1:
			*seconds_ret = MAX(seconds, 0);
		case 0:
			break;
		default:
			return -1;
	}

	return 0;
}

struct collect_query {
	int i;
	char **elements;
};

static enum MHD_Result collect_query_string(void *cls, enum MHD_ValueKind kind, const char *key, const char * value)
{
	// what happens when '?=foo' supplied?
	struct collect_query *collect_query = cls;
	if (key && !value) {
		collect_query->elements[collect_query->i] = safe_strdup(key);
	} else if (key && value) {
		safe_asprintf(&(collect_query->elements[collect_query->i]), "%s=%s", key, value);
	}
	collect_query->i++;
	return MHD_YES;
}

// a dump iterator required for counting all elements
static enum MHD_Result counter_iterator(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	return MHD_YES;
}

static int is_foreign_hosts(struct MHD_Connection *connection, const char *host)
{
	char our_host[MAX_HOSTPORTLEN];
	s_config *config = config_get_config();
	snprintf(our_host, MAX_HOSTPORTLEN, "%s", config->gw_address);
	debug(LOG_DEBUG, "Our host: %s Requested host: %s", our_host, host);


	// we serve all request without a host entry as well we serve all request going to our gw_address
	if (host == NULL)
		return 0;

	if (!strcmp(host, our_host))
		return 0;

	if (!strcmp(host, config->gw_ip))
		return 0;

	if (config->gw_fqdn) {
		if (!strcmp(host, config->gw_fqdn))
			return 0;
	}

	// port 80 is special, because the hostname doesn't need a port
	if (config->gw_port == 80 && !strcmp(host, config->gw_ip))
		return 0;

	return 1;
}

// @brief Get client mac by ip address from neighbor cache
int
get_client_mac(char mac[18], const char req_ip[])
{
	char line[255] = {0};
	char ip[64];
	FILE *stream;
	int len;

	len = strlen(req_ip);

	if ((len + 2) > sizeof(ip)) {
		return -1;
	}

	// Extend search string by one space
	memcpy(ip, req_ip, len);
	ip[len] = ' ';
	ip[len+1] = '\0';

	stream = popen("ip neigh show", "r");
	if (!stream) {
		return -1;
	}

	while (fgets(line, sizeof(line) - 1, stream) != NULL) {
		if (0 == strncmp(line, ip, len + 1)) {
			if (1 == sscanf(line, "%*s %*s %*s %*s %17[A-Fa-f0-9:] ", mac)) {
				pclose(stream);
				return 0;
			}
		}
	}

	pclose(stream);

	return -1;
}

/**
 * @brief get_client_ip
 * @param connection
 * @return ip address - must be freed by caller
 */
static int
get_client_ip(char ip_addr[INET6_ADDRSTRLEN], struct MHD_Connection *connection)
{
	const union MHD_ConnectionInfo *connection_info;
	const struct sockaddr *client_addr;
	const struct sockaddr_in *addrin;
	const struct sockaddr_in6 *addrin6;

	if (!(connection_info = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS))) {
		return -1;
	}

	// cast required for legacy MHD API < 0.9.6
	client_addr = (const struct sockaddr *) connection_info->client_addr;
	addrin = (const struct sockaddr_in *) client_addr;
	addrin6 = (const struct sockaddr_in6 *) client_addr;

	switch (client_addr->sa_family) {
	case AF_INET:
		if (inet_ntop(AF_INET, &addrin->sin_addr, ip_addr, INET_ADDRSTRLEN)) {
		debug(LOG_DEBUG, "client ip address is [ %s ]", ip_addr);
			return 0;
		}
		break;

	case AF_INET6:
		if (inet_ntop(AF_INET6, &addrin6->sin6_addr, ip_addr, INET6_ADDRSTRLEN)) {
		debug(LOG_DEBUG, "client ip address is [ %s ]", ip_addr);
			return 0;
		}
		break;
	}

	return -1;
}

/**
 * @brief libmicrohttpd_cb called when the client does a request to this server
 * @param cls unused
 * @param connection - client connection
 * @param url - which url was called
 * @param method - POST / GET / ...
 * @param version http 1.0 or 1.1
 * @param upload_data - unused
 * @param upload_data_size - unused
 * @param ptr - unused
 * @return
 */
enum MHD_Result libmicrohttpd_cb(
	void *cls,
	struct MHD_Connection *connection,
	const char *url,
	const char *method,
	const char *version,
	const char *upload_data,
	size_t *upload_data_size,
	void **ptr) {

	t_client *client;
	char ip[INET6_ADDRSTRLEN+1];
	char mac[18];
	const char *dds = "../";
	const char *mhdstatus = "/mhdstatus";
	int rc = 0;
	char *msg;
	char *testcmd;
	s_config *config;

	config = config_get_config();

	debug(LOG_DEBUG, "client access: %s %s", method, url);

	// only allow get
	if (0 != strcmp(method, "GET")) {
		debug(LOG_DEBUG, "Unsupported http method %s, Network Authentication required (Error 511)", method);
		return send_error(connection, 511);
	}

	// block path traversal
	if (strstr(url, dds) != NULL) {
		debug(LOG_WARNING, "Probable Path Traversal Attack Detected - %s", url);
		return send_error(connection, 403);
	}

	// check for mhdstatus request
	if (strstr(url, mhdstatus) != NULL) {
		debug(LOG_DEBUG, "MHD Status Request - %s", url);
		return send_error(connection, 200);
	}


	/* switch between preauth, authenticated
	 * - always - set caching headers
	 * a) possible implementation - redirect first and serve them using a tempo redirect
	 * b) serve direct
	 * should all requests redirected? even those to .css, .js, ... or respond with 404/503/...
	 */

	rc = get_client_ip(ip, connection);
	if (rc != 0) {
		return send_error(connection, 503);
	}


	// check if client ip is on our subnet
	testcmd = safe_calloc(SMALL_BUF);
	safe_snprintf(testcmd, SMALL_BUF, "/usr/lib/opennds/libopennds.sh get_interface_by_ip \"%s\"", ip);
	msg = safe_calloc(SMALL_BUF);
	rc = execute_ret_url_encoded(msg, SMALL_BUF, testcmd);
	free(testcmd);

	if (rc == 0) {
		debug(LOG_DEBUG, "Interface used to route ip [%s] is [%s]", ip, msg);
		debug(LOG_DEBUG, "Gateway Interface is [%s]", config->gw_interface);

		if (strcmp(config->gw_interface, msg) == 0) {
			debug(LOG_DEBUG, "Client ip address [%s] is on our subnet using interface [%s]", ip, msg);
		} else {
			debug(LOG_NOTICE, "Client ip address [%s] is  NOT on our subnet and is using interface [%s]", ip, msg);
			return send_error(connection, 403);
		}
	} else {
		debug(LOG_DEBUG, "ip subnet test failed: Continuing...");
	}

	free (msg);

	rc = get_client_mac(mac, ip);
	if (rc != 0) {
		return send_error(connection, 503);
	}

	client = client_list_find(mac, ip);
	if (!client) {
		client = add_client(mac, ip);
		if (!client) {
			return send_error(connection, 403);
		}
	}

	if (client && (client->fw_connection_state == FW_MARK_AUTHENTICATED ||
			client->fw_connection_state == FW_MARK_TRUSTED)) {
		// client is already authenticated, maybe they clicked/tapped "back" on the CPD browser or maybe they want the info page.
		return authenticated(connection, url, client);
	}

	return preauthenticated(connection, url, client);
}

/**
 * @brief check if url contains authdir
 * @param url
 * @param authdir
 * @return
 *
 * url must look ("/%s/", authdir) to match this
 */
static int check_authdir_match(const char *url, const char *authdir)
{
	debug(LOG_DEBUG, "url is [ %s ], checking for [ %s ]", url, authdir);

	if (strlen(url) != (2 + strlen(authdir)))
		return 0;

	if (strncmp(url + 1, authdir, strlen(authdir)))
		return 0;

	// match
	return 1;
}

/**
 * @brief try_to_authenticate
 * @param connection
 * @param client
 * @param host
 * @param url
 * @return
 */
static int try_to_authenticate(struct MHD_Connection *connection, t_client *client, const char *host, const char *url)
{
	s_config *config;
	const char *tok;
	char *rhid;
	char *rhidraw;

	config = config_get_config();

	// Check for authdir
	if (check_authdir_match(url, config->authdir)) {
		tok = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "tok");

		if (tok == NULL) {
			return send_error(connection, 403);
		}

		debug(LOG_DEBUG, "client->token=%s tok=%s ", client->token, tok );

		//Check if token (tok) or hash_id (hid) mode
		if (strlen(tok) > 8) {
			// hid mode
			rhidraw = safe_calloc(SMALL_BUF);
			safe_snprintf(rhidraw, SMALL_BUF, "%s%s", client->hid, config->fas_key);
			rhid = safe_calloc(STATUS_BUF);
			hash_str(rhid, STATUS_BUF, rhidraw);
			free (rhidraw);
			if (tok && !strcmp(rhid, tok)) {
				// rhid is valid
				free(rhid);
				return 1;
			}
			free(rhid);
		} else {
			// tok mode
			if (tok && !strcmp(client->token, tok)) {
				// Token is valid
				return 1;
			}
		}
	}

	debug(LOG_WARNING, "Token is invalid" );

/*	//TODO: do we need denydir?
	if (check_authdir_match(url, config->denydir)) {
		// matched to deauth
		return 0;
	}
*/

	return 0;
}

/**
 * @brief authenticate the client and redirect them
 * @param connection
 * @param ip_addr - needs to be freed
 * @param mac - needs to be freed
 * @param redirect_url - redirect the client to this url
 * @return
 */
static int authenticate_client(struct MHD_Connection *connection,
							const char *redirect_url,
							t_client *client)
{
	s_config *config = config_get_config();
	time_t now = time(NULL);
	int seconds = 60 * config->session_timeout;
	unsigned long long int uploadrate = 0;
	unsigned long long int downloadrate = 0;
	unsigned long long int uploadquota = 0;
	unsigned long long int downloadquota = 0;
	int rc;
	int ret;
	char *redirect_url_enc;
	char *querystr;
	const char *custom;

	client->window_counter = 0;
	client->rate_exceeded = 0;
	client->initial_loop = 0;
	client->upload_limiting = 0;
	client->download_limiting = 0;
	client->upload_rate = 0;
	client->download_rate = 0;
	client->uprate = 0;
	client->downrate = 0;
	client->upload_quota = 0;
	client->download_quota = 0;
	client->download_bucket_size = 0;
	client->upload_bucket_size = 0;
	client->inc_packet_limit = 0;
	client->out_packet_limit = 0;

	client->session_start = now;

	if (seconds > 0) {
		client->session_end = now + seconds;
	} else {
		client->session_end = 0;
	}

	debug(LOG_DEBUG, "redirect_url is [ %s ]", redirect_url);

	// get custom string
	custom = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "custom");

	if (!custom || strlen(custom) == 0) {
		custom="bmE=";
	}

	if (config->binauth) {
		rc = do_binauth(
			connection,
			config->binauth,
			client,
			&seconds,
			&uploadrate,
			&downloadrate,
			&uploadquota,
			&downloadquota,
			redirect_url
		);

		if (rc != 0) {
			/*BinAuth denies access so redirect client back to login/splash page where they can try again.
				If FAS is enabled, this will cause nesting of the contents of redirect_url,
				FAS should account for this if used with BinAuth.
			*/

			redirect_url_enc = safe_calloc(REDIRECT_URL_ENC_BUF);
			uh_urlencode(redirect_url_enc, REDIRECT_URL_ENC_BUF, redirect_url, strlen(redirect_url));

			debug(LOG_DEBUG, "redirect_url after binauth deny: %s", redirect_url);
			debug(LOG_DEBUG, "redirect_url_enc after binauth deny: %s", redirect_url_enc);

			querystr = safe_calloc(QUERYMAXLEN);
			querystr=construct_querystring(connection, client, redirect_url_enc, querystr);
			ret = encode_and_redirect_to_splashpage(connection, client, redirect_url_enc, querystr);
			free(querystr);
			free(redirect_url_enc);
			return ret;
		}
		rc = auth_client_auth(client->id, "client_auth", custom);
	} else {
		rc = auth_client_auth(client->id, NULL, custom);
	}

	// override remaining client values that might have been set by binauth

	if (seconds == 0) {
		seconds = (60 * config->session_timeout);
	}

	debug(LOG_DEBUG, "timeout seconds: %d", seconds);

	if (seconds != (60 * config->session_timeout)) {
		client->session_end = (client->session_start + seconds);
	}

	if (downloadrate > 0) {
		client->download_rate = downloadrate;
	}
	if (uploadrate > 0) {
		client->upload_rate = uploadrate;
	}

	if (downloadquota > 0) {
		client->download_quota = downloadquota;
	}
	if (uploadquota > 0) {
		client->upload_quota = uploadquota;
	}


	// error checking
	if (rc != 0) {
		return send_error(connection, 503);
	}

	if (redirect_url) {
		return send_redirect_temp(connection, client, redirect_url);
	} else {
		return send_error(connection, 200);
	}

	debug(LOG_DEBUG, "authenticate: Session Start - %lu Session End - %lu", client->session_start, client->session_end);
}

/**
 * @brief authenticated - called for all request from authenticated clients.
 * @param connection
 * @param ip_addr
 * @param mac
 * @param url
 * @param client
 * @return
 *
 * It's unsual to received request from clients which are already authenticated.
 * Happens when the user:
 * - clicked in multiple windows on "accept" -> redirect to origin - no checking
 * - when the user reloaded a splashpage -> redirect to origin
 * - when a user calls deny url -> deauth it
 */
static int authenticated(struct MHD_Connection *connection,
						const char *url,
						t_client *client)
{
	s_config *config = config_get_config();
	const char *host = config->gw_address;
	char *redirect_to_us = NULL;
	char *fasurl;
	char *query;
	char *msg;
	char *clientif;
	const char *accept;
	char *originurl_raw = NULL;
	char *captive_json = NULL;
	char *buff;
	int rc;
	int ret;
	struct MHD_Response *response;

	ret = MHD_get_connection_values(connection, MHD_HEADER_KIND, get_host_value_callback, &host);

	if (ret < 1) {
		debug(LOG_ERR, "authenticated: Error getting host");
		return MHD_NO;
	} else {
		debug(LOG_DEBUG, "An authenticated client is requesting: host [%s] url [%s]", host, url);
	}

	if (host == NULL) {
		debug(LOG_ERR, "authenticated: Error getting host");
		host = config->gw_address;
	}

	// Is it an RFC8908 type request? - check Accept: header

	accept = safe_calloc(SMALL_BUF);
	ret = MHD_get_connection_values(connection, MHD_HEADER_KIND, get_accept_callback, &accept);

	if (ret < 1) {
		debug(LOG_ERR, "authenticated: Error getting Accept header");
		return MHD_NO;
	}

	if (accept && strcmp(accept, "application/captive+json") == 0) {
		debug(LOG_NOTICE, "authenticated: Accept header [%s]", accept);
		debug(LOG_NOTICE, "authenticated: RFC 8908 captive+json request received");

		originurl_raw = safe_calloc(SMALL_BUF);
		captive_json = safe_calloc(SMALL_BUF);

		if (strcmp(config->gw_fqdn, "disable") == 0 || strcmp(config->gw_fqdn, "disabled") == 0) {
			safe_snprintf(originurl_raw, SMALL_BUF, "http://%s", config->gw_ip);
		} else {
			safe_snprintf(originurl_raw, SMALL_BUF, "http://%s", config->gw_fqdn);
		}

		safe_snprintf(captive_json, SMALL_BUF,
			"{ \"captive\": false, \"user-portal-url\": \"%s\", \"venue-info-url\": \"%s\", \"can-extend-session\": false }",
			originurl_raw,
			originurl_raw
		);

		debug(LOG_DEBUG, "captive_json [%s]", captive_json);
		ret = send_json(connection, captive_json);

		free(originurl_raw);
		free(captive_json);
		return ret;
	}

	/* check if this is a late request, meaning the user tries to get the internet, but ended up here,
	 * because the nftables rule came too late
	 */
	if (is_foreign_hosts(connection, host)) {
		// might happen if the firewall rule isn't yet installed
			return send_error(connection, 511);
	}

	if (check_authdir_match(url, config->denydir)) {
		debug(LOG_NOTICE, "Deauthentication request - client  [%s]", client->mac);
		auth_client_deauth(client->id, "client_deauth");
		debug(LOG_DEBUG, "Post deauth redirection [%s]", config->gw_address);

		redirect_to_us = safe_calloc(QUERYMAXLEN);

		if (!redirect_to_us) {
			ret = send_error(connection, 503);
			free(redirect_to_us);
			return ret;
		}

		safe_snprintf(redirect_to_us, QUERYMAXLEN, "http://%s/", config->gw_address);

		ret = send_redirect_temp(connection, client, redirect_to_us);
		free (redirect_to_us);
		return ret;
	}

	if (check_authdir_match(url, config->authdir)) {
		clientif = safe_calloc(STATUS_BUF);
		get_client_interface(clientif, STATUS_BUF, client->mac);

		if (config->fas_port && !config->preauth) {
			query = safe_calloc(QUERYMAXLEN);

			if (!query) {
				ret = send_error(connection, 503);
				free(query);
				return ret;
			}

			fasurl = safe_calloc(QUERYMAXLEN);

			if (!fasurl) {
				ret = send_error(connection, 503);
				free(clientif);
				free(query);
				free(fasurl);
				return ret;
			}

			get_query(connection, &query, HTMLQUERYSEPARATOR);

			safe_snprintf(fasurl, QUERYMAXLEN, "%s%s%sstatus=authenticated",
				config->fas_url,
				query,
				HTMLQUERYSEPARATOR
			);
			debug(LOG_DEBUG, "fasurl [%s]", fasurl);
			debug(LOG_DEBUG, "query [%s]", query);
			ret = send_redirect_temp(connection, client, fasurl);
			free(clientif);
			free(query);
			free(fasurl);
			return ret;
		} else if (config->fas_port && config->preauth) {
			fasurl = safe_calloc(QUERYMAXLEN);
			safe_snprintf(fasurl, QUERYMAXLEN, "?clientip=%s%sgatewayname=%s%sgatewayaddress=%s%sclientif=%s%sstatus=authenticated",
				client->ip,
				QUERYSEPARATOR,
				config->url_encoded_gw_name,
				QUERYSEPARATOR,
				config->gw_address,
				QUERYSEPARATOR,
				clientif,
				QUERYSEPARATOR
			);
			debug(LOG_DEBUG, "fasurl %s", fasurl);
			ret = show_preauthpage(connection, fasurl);
			free(fasurl);
			free(clientif);
			return ret;	
		}
		free(clientif);
	}

	if (check_authdir_match(url, config->preauthdir)) {

		if (config->fas_port) {
			query = safe_calloc(QUERYMAXLEN);
			fasurl = safe_calloc(QUERYMAXLEN);

			if (!query) {
				ret = send_error(connection, 503);
				free(query);
				free(fasurl);
				return ret;
			}

			get_query(connection, &query, QUERYSEPARATOR);

			safe_snprintf(fasurl, QUERYMAXLEN, "%s%sstatus=authenticated",
				query,
				QUERYSEPARATOR
			);

			debug(LOG_DEBUG, "preauthdir: fasurl %s", fasurl);
			ret = show_preauthpage(connection, fasurl);
			free(query);
			free(fasurl);
			return ret;
		}
	}

	// User just entered gatewayaddress:gatewayport so give them the info page
	if (strcmp(url, "/") == 0 || strcmp(url, "/login") == 0) {
		query = safe_calloc(QUERYMAXLEN);

		if (!query) {
			ret = send_error(connection, 503);
			free(query);
			return ret;
		}

		get_query(connection, &query, QUERYSEPARATOR);
		debug(LOG_DEBUG, "status_query=[%s]", query);

		buff = safe_calloc(MID_BUF);

		if (!buff) {
			ret = send_error(connection, 503);
			free(buff);
			return ret;
		}

		b64_encode(buff, MID_BUF, query, strlen(query));

		debug(LOG_DEBUG, "b64_status_query=[%s]", buff);

		msg = safe_calloc(HTMLMAXSIZE);

		if (!msg) {
			ret = send_error(connection, 503);
			free(msg);
			return ret;
		}

		rc = execute_ret(msg, HTMLMAXSIZE - 1, "%s status '%s' '%s'", config->status_path, client->ip, buff);

		if (rc != 0) {
			debug(LOG_WARNING, "Script: %s - failed to execute", config->status_path);
			ret = send_error(connection, 503);
			free(msg);
			return ret;
		}

		// serve the script output (in msg)
		response = MHD_create_response_from_buffer(strlen(msg), (char *)msg, MHD_RESPMEM_MUST_FREE);

		if (!response) {
			return send_error(connection, 503);
		}

		MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
		MHD_destroy_response(response);
		return ret;
	}

	// Client wants a specific file eg /images/splash.jpg etc.:
	return serve_file(connection, client, url);
}

/**
 * @brief show_preauthpage - run preauth script and serve output.
 */
static int show_preauthpage(struct MHD_Connection *connection, const char *query)
{
	s_config *config = config_get_config();

	char *msg;
	const char *user_agent;
	char *enc_user_agent;
	char *preauthpath;
	char *cmd;
	char *enc_query;

	int rc;
	int ret;
	struct MHD_Response *response;

	if ( strlen(query) < 1 ) {
		// query string is blank, too long or corrupt
		return send_error(connection, 511);
	} else {	
		preauthpath = safe_calloc(SMALL_BUF);
		safe_snprintf(preauthpath, SMALL_BUF, "/%s/", config->preauthdir);

		if (strcmp(preauthpath, config->fas_path) == 0) {
			free (preauthpath);

			MHD_get_connection_values(connection, MHD_HEADER_KIND, get_user_agent_callback, &user_agent);
			debug(LOG_DEBUG, "PreAuth: MHD User Agent ptr is [ %llu ]", &user_agent);

			if (user_agent == NULL) {
				return send_error(connection, 403);
			}

			enc_user_agent = safe_calloc(ENC_USER_AGENT);
			uh_urlencode(enc_user_agent, ENC_USER_AGENT, user_agent, strlen(user_agent));
			debug(LOG_DEBUG, "PreAuth: Encoded User Agent is [ %s ]", enc_user_agent);

			enc_query = safe_calloc(ENC_QUERYSTR);
			uh_urlencode(enc_query, ENC_QUERYSTR, query, strlen(query));
			debug(LOG_DEBUG, "PreAuth: Encoded query: %s", enc_query);

			msg = safe_calloc(HTMLMAXSIZE);

			if (!msg) {
				ret = send_error(connection, 503);
				free(msg);
				free(enc_user_agent);
				free(enc_query);
				return ret;
			}

			cmd = safe_calloc(QUERYMAXLEN);
			safe_snprintf(cmd, QUERYMAXLEN, "%s '%s' '%s' '%d' '%s'", config->preauth, enc_query, enc_user_agent, config->login_option_enabled, config->themespec_path);
			rc = execute_ret_url_encoded(msg, HTMLMAXSIZE - 1, cmd);
			free(cmd);

			if (rc != 0) {
				debug(LOG_WARNING, "Preauth script - failed to execute: %s, Query[%s]", config->preauth, query);
				free(msg);
				free(enc_user_agent);
				free(enc_query);

				return send_error(connection, 511);
			}

			// serve the script output (in msg)
			response = MHD_create_response_from_buffer(strlen(msg), (char *)msg, MHD_RESPMEM_MUST_FREE);

			if (!response) {
				free(msg);
				free(enc_user_agent);
				free(enc_query);

				return send_error(connection, 503);
			}

			MHD_add_response_header(response, "Content-Type", "text/html; charset=utf-8");
			ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
			MHD_destroy_response(response);

			// MHD will free(msg) when it has finished with it ( ie MHD_RESPMEM_MUST_FREE). Do not free here or MHD will page fault.
			free(enc_user_agent);
			free(enc_query);
			return ret;
		} else {
			free (preauthpath);
			return send_error(connection, 404);
		}
	}
}

/**
 * @brief send_json - send the rfc8908 json response.
 */
static int send_json(struct MHD_Connection *connection, const char *json)
{
	char *msg;
	int ret;
	struct MHD_Response *response;

	msg = safe_calloc(SMALL_BUF * 2);

	if (!msg) {
		ret = send_error(connection, 503);
		free(msg);
		return ret;
	}

	safe_snprintf(msg, SMALL_BUF * 2, "%s", json);

	debug(LOG_DEBUG, "json string [%s],  buffer [%s]", json, msg);

	response = MHD_create_response_from_buffer(strlen(msg), (char *)msg, MHD_RESPMEM_MUST_FREE);

	if (!response) {
		free(msg);
		ret = send_error(connection, 503);
		return ret;
	}

	MHD_add_response_header(response, "Cache-Control", "private");
	MHD_add_response_header(response, "Content-Type", "application/captive+json");
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);
	free(msg);
	return ret;

}

/**
 * @brief preauthenticated - called for all request of a client in this state.
 * @param connection
 * @param ip_addr
 * @param mac
 * @return
 */
static int preauthenticated(struct MHD_Connection *connection, const char *url, t_client *client)
{
	s_config *config = config_get_config();
	const char *host = config->gw_address;
	const char *accept = NULL;
	const char *redirect_url;
	char *query;
	char *querystr;
	char *originurl;
	char *originurl_raw = NULL;
	char *captive_json = NULL;

	int ret;

	ret = MHD_get_connection_values(connection, MHD_HEADER_KIND, get_host_value_callback, &host);

	if (ret < 1) {
		debug(LOG_ERR, "preauthenticated: Error getting host");
		return MHD_NO;
	}

	debug(LOG_DEBUG, "preauthenticated: host [%s] url [%s]", host, url);

	if (host == NULL) {
		debug(LOG_ERR, "preauthenticated: Error getting host");
		host = config->gw_address;
	}

	// check if this is an RFC8910 login request
	if (strcmp(url, "/login") == 0) {
		debug(LOG_INFO, "preauthenticated: RFC8910 login request received from client at [%s] [%s]", client->ip, client->mac);
		client->client_type = "cpi_url";
		return redirect_to_splashpage(connection, client, host, "/login");
	}

	// Is it an RFC8908 type request? - check Accept: header
	ret = MHD_get_connection_values(connection, MHD_HEADER_KIND, get_accept_callback, &accept);

	if (ret < 1) {
		debug(LOG_ERR, "preauthenticated: Error getting Accept header");
		return MHD_NO;
	}

	if (accept && strcmp(accept, "application/captive+json") == 0) {
		debug(LOG_DEBUG, "preauthenticated: Accept header [%s]", accept);
		debug(LOG_NOTICE, "preauthenticated: RFC 8908 captive+json request received from client at [%s] [%s]", client->ip, client->mac);

		client->client_type = "cpi_api";

		originurl_raw = safe_calloc(REDIRECT_URL);

		if (strcmp(config->gw_fqdn, "disable") == 0 || strcmp(config->gw_fqdn, "disabled") == 0) {
			safe_snprintf(originurl_raw, REDIRECT_URL, "http://%s", config->gw_ip);
		} else {
			safe_snprintf(originurl_raw, REDIRECT_URL, "http://%s", config->gw_fqdn);
		}

		originurl = safe_calloc(REDIRECT_URL_ENC_BUF);
		uh_urlencode(originurl, REDIRECT_URL_ENC_BUF, originurl_raw, strlen(originurl_raw));
		debug(LOG_DEBUG, "originurl: %s", originurl);

		querystr = safe_calloc(QUERYMAXLEN);

		if (!querystr) {
			ret = send_error(connection, 503);
			free(originurl);
			free(originurl_raw);
			free(querystr);
			return ret;
		}

		querystr=construct_querystring(connection, client, originurl, querystr);
		debug(LOG_DEBUG, "Constructed query string [%s]", querystr);
		debug(LOG_DEBUG, "FAS url [%s]", config->fas_url);

		captive_json = safe_calloc(QUERYMAXLEN);

		if (!captive_json) {
			ret = send_error(connection, 503);
			free(originurl);
			free(originurl_raw);
			free(querystr);
			free(captive_json);
			return ret;
		}

		safe_snprintf(captive_json, QUERYMAXLEN, "{ \"captive\": true, \"user-portal-url\": \"%s%s\" }", config->fas_url, querystr);

		debug(LOG_DEBUG, "captive_json [%s]", captive_json);
		ret = send_json(connection, captive_json);

		free(originurl);
		free(originurl_raw);
		free(querystr);
		free(captive_json);
		return ret;
	}

	// Did user just access gatewayaddress:gatewayport directly or by redirect
	if (strcmp(url, "/") == 0) {
		if (strcmp(host, config->gw_address) == 0 || strcmp(host, config->gw_ip) == 0 || strcmp(host, config->gw_fqdn) == 0) {
			return send_error(connection, 511);
		}
	}

	// Check for preauthdir
	if (check_authdir_match(url, config->preauthdir)) {

		debug(LOG_DEBUG, "preauthdir url detected: %s", url);
		query = safe_calloc(QUERYMAXLEN);

		if (!query) {
			ret = send_error(connection, 503);
			free(query);
			return ret;
		}

		get_query(connection, &query, QUERYSEPARATOR);
		debug(LOG_DEBUG, "preauthenticated: show_preauthpage [%s]", query);
		ret = show_preauthpage(connection, query);
		free(query);
		return ret;
	}

	// Check for denydir
	if (check_authdir_match(url, config->denydir)) {
		debug(LOG_DEBUG, "denydir url detected: %s", url);
		return send_error(connection, 511);
	}

	debug(LOG_DEBUG, "preauthenticated: Requested Host is [ %s ], url is [%s]", host, url);

	// check if this is a redirect query with a foreign host as target
	if (is_foreign_hosts(connection, host)) {
		debug(LOG_DEBUG, "preauthenticated: foreign host [%s] detected", host);
		return redirect_to_splashpage(connection, client, host, url);
	}

	// request is directed to us, check if client wants to be authenticated
	if (check_authdir_match(url, config->authdir)) {
		debug(LOG_DEBUG, "authdir url detected: %s", url);

		redirect_url = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "redir");

		if (redirect_url == NULL) {
			return send_error(connection, 403);
		}

		if (!try_to_authenticate(connection, client, host, url)) {
			// user used an invalid token, redirect to splashpage but hold query "redir" intact
			originurl = safe_calloc(REDIRECT_URL_ENC_BUF);
			uh_urlencode(originurl, REDIRECT_URL_ENC_BUF, redirect_url, strlen(redirect_url));

			querystr = safe_calloc(QUERYMAXLEN);

			if (!querystr) {
				ret = send_error(connection, 503);
				free(originurl);
				free(querystr);
				return ret;
			}

			querystr = construct_querystring(connection, client, originurl, querystr);

			ret = encode_and_redirect_to_splashpage(connection, client, originurl, querystr);
			free(originurl);
			free(querystr);
			return ret;
		}

		ret = authenticate_client(connection, redirect_url, client);
		return ret;
	}

	// no special handling left - try to serve static content to the user
	return serve_file(connection, client, url);
}

/**
 * @brief encode originurl and redirect the client to the splash page
 * @param connection
 * @param client
 * @param originurl
 * @return
 */
static int encode_and_redirect_to_splashpage(struct MHD_Connection *connection, t_client *client, const char *originurl, const char *querystr)
{
	char *splashpageurl = NULL;
	s_config *config;
	int ret;

	config = config_get_config();
	splashpageurl = safe_calloc(QUERYMAXLEN);

	if (config->fas_port) {
		// Generate secure query string or authaction url
		// Note: config->fas_path contains a leading / as it is the path from the FAS web root.
		if (config->fas_secure_enabled == 0) {
			safe_snprintf(splashpageurl, QUERYMAXLEN, "%s?authaction=http://%s/%s/%s",
				config->fas_url, config->gw_address, config->authdir, querystr);
		} else if (config->fas_secure_enabled >= 1) {
				safe_snprintf(splashpageurl, QUERYMAXLEN, "%s%s",
					config->fas_url, querystr);
		} else {
			safe_snprintf(splashpageurl, QUERYMAXLEN, "%s?%s",
				config->fas_url, querystr);
		}
	}

	debug(LOG_DEBUG, "splashpageurl: %s", splashpageurl);

	ret = send_redirect_temp(connection, client, splashpageurl);
	free(splashpageurl);
	return ret;
}

/**
 * @brief redirect_to_splashpage
 * @param connection
 * @param client
 * @param host
 * @param url
 * @return
 */
static int redirect_to_splashpage(struct MHD_Connection *connection, t_client *client, const char *host, const char *url)
{
	char *originurl_raw;
	char *originurl;
	char *query;
	int ret = 0;
	const char *separator = "&";
	char *querystr;

	query = safe_calloc(QUERYMAXLEN);

	if (!query) {
		ret = send_error(connection, 503);
		free(query);
		return ret;
	}

	querystr = safe_calloc(QUERYMAXLEN);

	if (!querystr) {
		ret = send_error(connection, 503);
		free(querystr);
		return ret;
	}

	originurl_raw = safe_calloc(MID_BUF);

	if (!originurl_raw) {
		ret = send_error(connection, 503);
		free(originurl_raw);
		return ret;
	}

	originurl = safe_calloc(CUSTOM_ENC);

	if (!originurl) {
		ret = send_error(connection, 503);
		free(originurl);
		return ret;
	}

	get_query(connection, &query, separator);

	if (!query) {
		debug(LOG_DEBUG, "Unable to get query string - error 503");
		free(query);
		// probably no mem
		return send_error(connection, 503);
	}

	debug(LOG_DEBUG, "Query string is [ %s ]", query);

	safe_asprintf(&originurl_raw, "http://%s%s%s", host, url, query);
	uh_urlencode(originurl, CUSTOM_ENC, originurl_raw, strlen(originurl_raw));

	if (strcmp(url, "/login") == 0) {
		client->cpi_query = safe_strdup(originurl);
		debug(LOG_DEBUG, "RFC8910 request: %s", client->cpi_query);
	}

	debug(LOG_DEBUG, "originurl_raw: %s", originurl_raw);
	debug(LOG_DEBUG, "originurl: %s", originurl);

	querystr=construct_querystring(connection, client, originurl, querystr);
	ret = encode_and_redirect_to_splashpage(connection, client, originurl, querystr);
	free(originurl_raw);
	free(originurl);
	free(query);
	free(querystr);
	return ret;
}

/**
 * @brief construct_querystring
 * @return the querystring
 */
static char *construct_querystring(struct MHD_Connection *connection, t_client *client, char *originurl, char *querystr ) {

	char cid[87] = {0};
	char *clienttype;
	char *clientif;
	char *query_str;
	char *query_str_b64;
	char *msg;
	char *cidinfo;
	char *cidfile;
	char *gw_url_raw;
	char *gw_url;
	char *phpcmd;
	int cidgood = 0;

	s_config *config = config_get_config();

	gw_url_raw = safe_calloc(REDIRECT_URL);

	if (strcmp(config->gw_fqdn, "disable") == 0 || strcmp(config->gw_fqdn, "disabled") == 0) {
		safe_snprintf(gw_url_raw, REDIRECT_URL, "http://%s", config->gw_ip);
	} else {
		safe_snprintf(gw_url_raw, REDIRECT_URL, "http://%s", config->gw_fqdn);
	}

	gw_url = safe_calloc(REDIRECT_URL_ENC_BUF);
	uh_urlencode(gw_url, REDIRECT_URL_ENC_BUF, gw_url_raw, strlen(gw_url_raw));
	debug(LOG_DEBUG, "gw_url: %s", gw_url);
	free (gw_url_raw);

	clienttype = safe_calloc(STATUS_BUF);

	if (!client->client_type || strlen(client->client_type) == 0) {
		clienttype = safe_strdup("cpd_can");
	} else {
		clienttype = safe_strdup(client->client_type);
	}

	if (config->fas_secure_enabled == 0) {
		snprintf(querystr, QUERYMAXLEN, "?clientip=%s&gatewayname=%s&tok=%s&redir=%s",
			client->ip,
			config->url_encoded_gw_name,
			client->token,
			originurl
		);

	} else if (config->fas_secure_enabled == 1 || config->fas_secure_enabled == 4) {

			if (config->fas_hid) {
				debug(LOG_DEBUG, "hid=%s", client->hid);

				clientif = safe_calloc(STATUS_BUF);

				get_client_interface(clientif, STATUS_BUF, client->mac);
				debug(LOG_DEBUG, "clientif: [%s] url_encoded_gw_name: [%s]", clientif, config->url_encoded_gw_name);

				query_str = safe_calloc(QUERYMAXLEN);

				snprintf(query_str, QUERYMAXLEN,
					"hid=%s%sclientip=%s%sclientmac=%s%sclient_type=%s%scpi_query=%s%sgatewayname=%s%sgatewayurl=%s%sversion=%s%sgatewayaddress=%s%sgatewaymac=%s%soriginurl=%s%sclientif=%s%sthemespec=%s%s%s%s%s%s",
					client->hid, QUERYSEPARATOR,
					client->ip, QUERYSEPARATOR,
					client->mac, QUERYSEPARATOR,
					clienttype, QUERYSEPARATOR,
					client->cpi_query, QUERYSEPARATOR,
					config->url_encoded_gw_name, QUERYSEPARATOR,
					gw_url, QUERYSEPARATOR,
					VERSION, QUERYSEPARATOR,
					config->gw_address, QUERYSEPARATOR,
					config->gw_mac, QUERYSEPARATOR,
					originurl, QUERYSEPARATOR,
					clientif, QUERYSEPARATOR,
					config->themespec_path, QUERYSEPARATOR,
					config->custom_params,
					config->custom_vars,
					config->custom_images,
					config->custom_files
				);

				query_str_b64 = safe_calloc(ENC_QUERYSTR);

				b64_encode(query_str_b64, ENC_QUERYSTR, query_str, strlen(query_str));

				snprintf(querystr, ENC_QUERYSTR,
					"?fas=%s",
					query_str_b64
				);

				if (config->login_option_enabled >=1) {
					if (client->cid) {
						cidgood = 1;
						cidfile = safe_calloc(SMALL_BUF);
						safe_snprintf(cidfile, SMALL_BUF, "%s/ndscids/%s", config->tmpfsmountpoint, client->cid);

						// Check if cidfile exists
						if(access(cidfile, F_OK) != 0) {
							// does not exist
							cidgood=0;
						}
						free(cidfile);
					}

					if (cidgood == 0) {
						strncpy(cid, query_str_b64+5, 86);
						client->cid = safe_strdup(cid);

						// Write the new cidfile:
						msg = safe_calloc(STATUS_BUF);
						cidinfo = safe_calloc(SMALL_BUF);
						debug(LOG_DEBUG, "writing cid file [%s]", cid);

						safe_snprintf(cidinfo, SMALL_BUF, "hid=\"%s\"\0", client->hid);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "clientip=\"%s\"\0", client->ip);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "clientmac=\"%s\"\0", client->mac);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "cpi_query=\"%s\"\0", client->cpi_query);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);


						safe_snprintf(cidinfo, SMALL_BUF, "client_type=\"%s\"\0", clienttype);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "gatewayname=\"%s\"\0", config->http_encoded_gw_name);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "gatewayurl=\"%s\"\0", gw_url);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "version=\"%s\"\0", VERSION);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "gatewayaddress=\"%s\"\0", config->gw_address);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "gatewaymac=\"%s\"\0", config->gw_mac);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "originurl=\"%s\"\0", originurl);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						safe_snprintf(cidinfo, SMALL_BUF, "clientif=\"%s\"\0", clientif);
						write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);

						if (config->themespec_path) {
							safe_snprintf(cidinfo, SMALL_BUF, "themespec=\"%s\"\0", config->themespec_path);
							write_client_info(msg, STATUS_BUF, "write", cid, cidinfo);
						}

						if (config->custom_params) {
							safe_snprintf(cidinfo, SMALL_BUF, "%s\0", config->custom_params);
							write_client_info(msg, STATUS_BUF, "parse", cid, cidinfo);
						}

						if (config->custom_vars) {
							safe_snprintf(cidinfo, SMALL_BUF, "%s\0", config->custom_vars);
							write_client_info(msg, STATUS_BUF, "parse", cid, cidinfo);
						}

						if (config->custom_images) {
							safe_snprintf(cidinfo, SMALL_BUF, "%s\0", config->custom_images);
							write_client_info(msg, STATUS_BUF, "parse", cid, cidinfo);
						}

						if (config->custom_files) {
							safe_snprintf(cidinfo, SMALL_BUF, "%s\0", config->custom_files);
							write_client_info(msg, STATUS_BUF, "parse", cid, cidinfo);
						}

						free(msg);
						free(cidinfo);
					}
				}

				free(query_str);
				free(query_str_b64);
				free(clientif);

			} else {
				snprintf(querystr, QUERYMAXLEN,
					"?clientip=%s&gatewayname=%s&redir=%s",
					client->ip,
					config->url_encoded_gw_name,
					originurl
				);
			}

	} else if (config->fas_secure_enabled == 2 || config->fas_secure_enabled == 3) {

		debug(LOG_DEBUG, "hid=%s", client->hid);

		clientif = safe_calloc(STATUS_BUF);
		get_client_interface(clientif, STATUS_BUF, client->mac);
		debug(LOG_DEBUG, "clientif: [%s]", clientif);
		snprintf(querystr, QUERYMAXLEN,
			"hid=%s%sclientip=%s%sclientmac=%s%sclient_type=%s%scpi_query=%s%sgatewayname=%s%sgatewayurl=%s%sversion=%s%sgatewayaddress=%s%sgatewaymac=%s%sauthdir=%s%soriginurl=%s%sclientif=%s%sthemespec=%s%s%s%s%s%s",
			client->hid, QUERYSEPARATOR,
			client->ip, QUERYSEPARATOR,
			client->mac, QUERYSEPARATOR,
			clienttype, QUERYSEPARATOR,
			client->cpi_query, QUERYSEPARATOR,
			config->url_encoded_gw_name, QUERYSEPARATOR,
			gw_url, QUERYSEPARATOR,
			VERSION, QUERYSEPARATOR,
			config->gw_address, QUERYSEPARATOR,
			config->gw_mac, QUERYSEPARATOR,
			config->authdir, QUERYSEPARATOR,
			originurl, QUERYSEPARATOR,
			clientif, QUERYSEPARATOR,
			config->themespec_path, QUERYSEPARATOR,
			config->custom_params,
			config->custom_vars,
			config->custom_images,
			config->custom_files
		);

		phpcmd = safe_calloc(QUERYMAXLEN);
		safe_snprintf(phpcmd, QUERYMAXLEN,
			"echo '<?php \n"
			"$key=\"%s\";\n"
			"$string=\"%s\";\n"
			"$cipher=\"aes-256-cbc\";\n"

			"if (in_array($cipher, openssl_get_cipher_methods())) {\n"
				"$secret_iv = base64_encode(openssl_random_pseudo_bytes(\"8\"));\n"
				"$iv = substr(openssl_digest($secret_iv, \"sha256\"), 0, 16 );\n"
				"$string = base64_encode( openssl_encrypt( $string, $cipher, $key, 0, $iv ) );\n"
				"echo \"?fas=\".$string.\"&iv=\".$iv;\n"
			"}\n"
			" ?>' "
			" | %s\n",
			config->fas_key,
			querystr,
			config->fas_ssl
		);

		debug(LOG_DEBUG, "phpcmd: %s", phpcmd);

		msg = safe_calloc(QUERYMAXLEN);

		if (!msg) {
			send_error(connection, 503);
			free(msg);
		} else {

			if (! execute_ret_url_encoded(msg, QUERYMAXLEN - 1, phpcmd) == 0) {
				debug(LOG_ERR, "Error encrypting query string. %s", msg);
			}

			snprintf(querystr, QUERYMAXLEN, "%s", msg);

			free(msg);
			free(phpcmd);
		}
		free(clientif);

	} else {
		snprintf(querystr, QUERYMAXLEN, "?clientip=%s&gatewayname=%s", client->ip, config->url_encoded_gw_name);
	}

	debug(LOG_DEBUG, "Constructed Query String [%s]", querystr);
	free(clienttype);
	free(gw_url);
	return querystr;
}

/**
 *	Add client making a request to client list.
 *	Return pointer to the client list entry for this client.
 *
 *	N.B.: This does not authenticate the client; it only makes
 *	their information available on the client list.
 */
static t_client *
add_client(const char *mac, const char *ip)
{
	t_client *client;

	LOCK_CLIENT_LIST();
	client = client_list_add_client(mac, ip);
	UNLOCK_CLIENT_LIST();

	return client;
}

int send_redirect_temp(struct MHD_Connection *connection, t_client *client, const char *url)
{
	// Warning - *client will be undefined if not authenticated
	struct MHD_Response *response;
	int ret;
	char *redirect;

	redirect = safe_calloc(SMALL_BUF);

	safe_snprintf(redirect, SMALL_BUF, "<html><head></head><body><a href='%s'>Click here to continue to<br>%s</a></body></html", url, url);

	debug(LOG_DEBUG, "send_redirect_temp: MHD_create_response_from_buffer. url [%s]", url);
	debug(LOG_DEBUG, "send_redirect_temp: Redirect to [%s]", redirect);

	response = MHD_create_response_from_buffer(SMALL_BUF, redirect, MHD_RESPMEM_MUST_FREE);

	if (!response) {
		debug(LOG_DEBUG, "send_redirect_temp: Failed to create response....");
		return send_error(connection, 503);
	} else {
		debug(LOG_DEBUG, "send_redirect_temp: Response created");
	}

	ret = MHD_add_response_header(response, "Location", url);

	if (ret == MHD_NO) {
		debug(LOG_ERR, "send_redirect_temp: Error adding Location header to redirection page");
	} else {
		debug(LOG_DEBUG, "send_redirect_temp: Location header added to redirection page");
	}

	ret = MHD_add_response_header(response, "Connection", "close");

	if (ret == MHD_NO) {
		debug(LOG_ERR, "send_redirect_temp: Error adding Connection header to redirection page");
	} else {
		debug(LOG_DEBUG, "send_redirect_temp: Connection header added to redirection page");
	}

	debug(LOG_DEBUG, "send_redirect_temp: Queueing response");

	ret = MHD_queue_response(connection, MHD_HTTP_TEMPORARY_REDIRECT, response);

	if (ret == MHD_NO) {
		debug(LOG_ERR, "send_redirect_temp: Error queueing response");
	} else {
		debug(LOG_DEBUG, "send_redirect_temp: Response is Queued");
	}

	MHD_destroy_response(response);

	return ret;
}

// save the query or empty string into **query.
static int get_query(struct MHD_Connection *connection, char **query, const char *separator)
{
	int element_counter;
	char **elements;
	char *query_str;
	struct collect_query collect_query;
	int i;
	int j = 4;
	int length = 0;

	debug(LOG_DEBUG, " Getting query, separator is [%s].", separator);

	element_counter = MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, counter_iterator, NULL);
	if (element_counter < 0) {
		*query = safe_strdup("");
		return MHD_NO;
	}
	elements = calloc(element_counter, sizeof(char *));
	if (elements == NULL) {
		return MHD_NO;
	}
	collect_query.i = 0;
	collect_query.elements = elements;

	// Collect the arguments of the query string from MHD
	MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, collect_query_string, &collect_query);

	for (i = 0; i < element_counter; i++) {
		if (!elements[i])
			continue;
		length += strlen(elements[i]);

		if (i > 0) // q=foo&o=bar the '&' need also some space
			length++;

		debug(LOG_DEBUG, " cumulative element length is [%d]", length);
	}

	// don't miss the zero terminator
	if (*query == NULL) {
		for (i = 0; i < element_counter; i++) {
			free(elements[i]);
		}
		free(elements);
		return 0;
	}

	query_str = safe_calloc(QUERYMAXLEN);

	for (i = 0, j = 0; i < element_counter; i++) {
		if (!elements[i]) {
			continue;
		}

		debug(LOG_DEBUG, " element [%d] is [%s]", i, elements[i]);

		if (length + j < QUERYMAXLEN) {

			strncpy(*query + j, elements[i], length - j);

			if (i == 0) {
				// query_str is empty when i = 0 so safe to copy a single char into it
				strcpy(query_str, "?");
			} else {
				if (QUERYMAXLEN - strlen(query_str) > length - j + 1) {
					strncat(query_str, separator, QUERYMAXLEN - strlen(query_str));
				}
			}

			strncat(query_str, *query, QUERYMAXLEN - strlen(query_str));
		} else {
			debug(LOG_WARNING, " Query string is too long, invalid or corrupt so is ignored.");
		}

		free(elements[i]);
	}

	debug(LOG_DEBUG, " query is [%s]", query_str);
	strncpy(*query, query_str, QUERYMAXLEN);
	free(query_str);
	free(elements);
	return 0;
}

static int send_error(struct MHD_Connection *connection, int error)
{
	struct MHD_Response *response = NULL;

	/* cannot automate since cannot translate automagically between error number and MHD's status codes
	 * -- and cannot rely on MHD_HTTP_ values to provide an upper bound for an array
	 */
	const char *page_200 = "<br>OK<br>";
	const char *page_202 = "<html><body><h1>Processing Request</h1></body></html>";
	const char *page_400 = "<html><head><title>Error 400</title></head><body><h1>Error 400 - Bad Request</h1></body></html>";
	const char *page_403 = "<html><head><title>Error 403</title></head><body><h1>Error 403 - Forbidden - Access Denied to this Client!</h1></body></html>";
	const char *page_404 = "<html><head><title>Error 404</title></head><body><h1>Error 404 - Not Found</h1></body></html>";
	const char *page_500 = "<html><head><title>Error 500</title></head><body><h1>Error 500 - Internal Server Error: Oh No!</h1></body></html>";
	const char *page_501 = "<html><head><title>Error 501</title></head><body><h1>Error 501 - Not Implemented</h1></body></html>";
	const char *page_503 = "<html><head><title>Error 503</title></head><body><h1>Error 503 - Service Unavailable. This may be a temporary condition."
		"</h1></body></html>";
	char *page_511;
	char *cmd;
	const char *mimetype = lookup_mimetype("foo.html");
	char ip[INET6_ADDRSTRLEN+1];

	int ret = MHD_NO;
	s_config *config = config_get_config();

	switch (error) {
	case 200:
		response = MHD_create_response_from_buffer(strlen(page_200), (char *)page_200, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
		break;

	case 202:
		response = MHD_create_response_from_buffer(strlen(page_202), (char *)page_202, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_ACCEPTED, response);
		break;

	case 400:
		response = MHD_create_response_from_buffer(strlen(page_400), (char *)page_400, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_BAD_REQUEST, response);
		break;

	case 403:
		response = MHD_create_response_from_buffer(strlen(page_403), (char *)page_403, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_FORBIDDEN, response);
		break;

	case 404:
		response = MHD_create_response_from_buffer(strlen(page_404), (char *)page_404, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
		break;

	case 500:
		response = MHD_create_response_from_buffer(strlen(page_500), (char *)page_500, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
		break;

	case 501:
		response = MHD_create_response_from_buffer(strlen(page_501), (char *)page_501, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_NOT_IMPLEMENTED, response);
		break;
	case 503:
		debug(LOG_INFO, "503: [%s] ", (char *)page_503);
		response = MHD_create_response_from_buffer(strlen(page_503), (char *)page_503, MHD_RESPMEM_MUST_COPY);
		MHD_add_response_header(response, "Content-Type", mimetype);
		ret = MHD_queue_response(connection, MHD_HTTP_SERVICE_UNAVAILABLE, response);
		break;
	case 511:
		get_client_ip(ip, connection);

		page_511 = safe_calloc(HTMLMAXSIZE);

		if (!page_511) {
			ret = send_error(connection, 503);
			free(page_511);
			break;
		}

		cmd = safe_calloc(SMALL_BUF);
		safe_snprintf(cmd, SMALL_BUF, "%s err511 '%s'", config->status_path, ip);

		if (execute_ret_url_encoded(page_511, HTMLMAXSIZE - 1, cmd) == 0) {
			debug(LOG_INFO, "Network Authentication Required - page_511 html generated for [%s]", ip);
		} else {
			debug(LOG_WARNING, "Script: %s - failed to execute", config->status_path);
			ret = send_error(connection, 503);
			free(cmd);
			free(page_511);
			return ret;
		}

		free(cmd);

		response = MHD_create_response_from_buffer(strlen(page_511), (char *)page_511, MHD_RESPMEM_MUST_FREE);

		if (response) {
			MHD_add_response_header(response, "Content-Type", mimetype);
			MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION, "close");
			ret = MHD_queue_response(connection, MHD_HTTP_NETWORK_AUTHENTICATION_REQUIRED, response);

			if (ret == MHD_NO) {
				debug(LOG_ERR, "send_error 511: Error queueing response");
			} else {
				debug(LOG_DEBUG, "send_error 511: Response is Queued");
			}

			break;
		}

		debug(LOG_ERR, "send_error 511: Error queueing response");
		break;
	}

	if (response)
		MHD_destroy_response(response);
	return ret;
}

/**
 * @brief get_host_value_callback safe Host into cls which is a char**
 * @param cls - a char ** pointer to our target buffer. This buffer will be alloc in this function.
 * @param kind - see doc of	MHD_KeyValueIterator's
 * @param key
 * @param value
 * @return MHD_YES or MHD_NO. MHD_NO means we found our item and this callback will not called again.
 */
static enum MHD_Result get_host_value_callback(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	const char **host = (const char **)cls;

	if (MHD_HEADER_KIND != kind) {
		*host = NULL;
		return MHD_NO;
	}

	if (!strcmp("Host", key)) {
		*host = value;
		return MHD_NO;
	}
	if (key && value) {

		if (!strcmp("Host", key)) {
			*host = value;
			return MHD_NO;
		}
	}

	return MHD_YES;
}

/**
 * @brief get_user_agent_callback save User-Agent into cls which is a char**
 * @param cls - a char ** pointer to our target buffer. This buffer will be alloc in this function.
 * @param kind - see doc of	MHD_KeyValueIterator's
 * @param key
 * @param value
 * @return MHD_YES or MHD_NO. MHD_NO means we found our item and this callback will not called again.
 */
static enum MHD_Result get_user_agent_callback(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	const char **user_agent = (const char **)cls;

	if (MHD_HEADER_KIND != kind) {
		*user_agent = NULL;
		return MHD_NO;
	}

	if (key && value) {

		if (!strcmp("User-Agent", key)) {
			*user_agent = value;
			return MHD_NO;
		}
	}

	return MHD_YES;
}

/**
 * @brief get_accept_callback save Accept: into cls which is a char**
 * @param cls - a char ** pointer to our target buffer. This buffer will be alloc in this function.
 * @param kind - see doc of	MHD_KeyValueIterator's
 * @param key
 * @param value
 * @return MHD_YES or MHD_NO. MHD_NO means we found our item and this callback will not called again.
 */
static enum MHD_Result get_accept_callback(void *cls, enum MHD_ValueKind kind, const char *key, const char *value)
{
	const char **accept = (const char **)cls;

	if (MHD_HEADER_KIND != kind) {
		*accept = NULL;
		return MHD_NO;
	}

	if (key && value) {

		if (!strcmp("Accept", key)) {
			*accept = value;
			return MHD_NO;
		}
	}

	return MHD_YES;
}

/**
 * @brief return an extension like `csv` if file = '/bar/foobar.csv'.
 * @param filename
 * @return a pointer within file is returned. NULL can be returned as well as
 */
const char *get_extension(const char *filename)
{
	int pos = strlen(filename);
	while (pos > 0) {
		pos--;
		switch (filename[pos]) {
		case '/':
			return NULL;
		case '.':
			return (filename+pos+1);
		}
	}

	return NULL;
}

#define DEFAULT_MIME_TYPE "application/octet-stream"

const char *lookup_mimetype(const char *filename)
{
	int i;
	const char *extension;

	if (!filename) {
		return NULL;
	}

	extension = get_extension(filename);
	if (!extension)
		return DEFAULT_MIME_TYPE;

	for (i = 0; i< ARRAY_SIZE(uh_mime_types); i++) {
		if (strcmp(extension, uh_mime_types[i].extn) == 0) {
			return uh_mime_types[i].mime;
		}
	}

	debug(LOG_ERR, "Could not find corresponding mimetype for %s extension", extension);

	return DEFAULT_MIME_TYPE;
}

/**
 * @brief serve_file try to serve a request via filesystem. Using webroot as root.
 * @param connection
 * @param client
 * @return
 */
static int serve_file(struct MHD_Connection *connection, t_client *client, const char *url)
{
	struct stat stat_buf;
	s_config *config = config_get_config();
	struct MHD_Response *response;
	char filename[PATH_MAX];
	int ret = MHD_NO;
	const char *mimetype = NULL;
	off_t size;

	snprintf(filename, PATH_MAX, "%s/%s", config->webroot, url);

	// check if file exists and is not a directory
	ret = stat(filename, &stat_buf);
	if (ret) {
		// stat failed
		debug(LOG_DEBUG, "File %s could not be found", filename);
		return send_error(connection, 404);
	}

	if (!S_ISREG(stat_buf.st_mode)) {
#ifdef S_ISLNK
		// ignore links
		if (!S_ISLNK(stat_buf.st_mode))
#endif // S_ISLNK
		return send_error(connection, 404);
	}

	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		return send_error(connection, 404);

	mimetype = lookup_mimetype(filename);

	// serving file and creating response
	size = lseek(fd, 0, SEEK_END);
	if (size < 0)
		return send_error(connection, 404);

	response = MHD_create_response_from_fd(size, fd);
	if (!response)
		return send_error(connection, 503);

	MHD_add_response_header(response, "Content-Type", mimetype);
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

