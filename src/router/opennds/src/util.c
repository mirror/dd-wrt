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

/**
  @file util.c
  @brief Misc utility functions
  @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
  @author Copyright (C) 2006 Benoit Grégoire <bock@step.polymtl.ca>
  @author Copyright (C) 2008 Paul Kube <nodogsplash@kokoro.ucsd.edu>
  @author Copyright (C) 2015-2023 Modifications and additions by BlueWave Projects and Services <opennds@blue-wave.net>
  @author Copyright (C) 2021 ndsctl_lock() and ndsctl_unlock() based on code by Linus Lüssing <ll@simonwunderlich.de>

 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <ifaddrs.h>
#include <unistd.h>

#if defined(__NetBSD__)
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <util.h>
#endif

#ifdef __linux__
#include <netinet/in.h>
#include <net/if.h>
#endif

#include <string.h>
#include <pthread.h>
#include <netdb.h>
#include <microhttpd.h>

#include "common.h"
#include "client_list.h"
#include "safe.h"
#include "util.h"
#include "conf.h"
#include "debug.h"
#include "fw_iptables.h"
#include "http_microhttpd_utils.h"

// Defined in main.c
extern time_t started_time;

// Defined in clientlist.c
extern pthread_mutex_t client_list_mutex;
extern pthread_mutex_t config_mutex;

// Defined in auth.c
extern unsigned int authenticated_since_start;

// Defined in main.c
extern int created_httpd_threads;
extern int current_httpd_threads;

int count_substrings(char* string, char* substring) {
	int idx;
	int len1;
	int len2;
	int numsubs = 0;

	len1 = strlen(string);
	len2 = strlen(substring);

	for(idx = 0; idx < len1 - len2 + 1; idx++) {
		if(strstr(string + idx, substring) == string + idx) {
			numsubs++;
			idx = idx + len2 -1;
		}
	}

	return numsubs;
}

int startdaemon(char *cmd, int daemonpid)
{
	/* Start a program in the background.
		cmd contains the full path and startup arguments of the program
		daemonpid returns with the pid of the running program
		daemonpid returns 0 if the program terminates quickly
		Function returns 0 if successfully running in background, 1 if failed
	*/

	char *buff;
	char *msg;
	char *daemoncmd;
	int ret;

	buff = safe_calloc(MID_BUF);
	msg = safe_calloc(STATUS_BUF);

	b64_encode(buff, MID_BUF, cmd, strlen(cmd));

	safe_asprintf(&daemoncmd, "/usr/lib/opennds/libopennds.sh startdaemon '%s'",
		buff
	);

	debug(LOG_DEBUG, "startdaemon command: %s", daemoncmd);

	ret = execute_ret_url_encoded(msg, STATUS_BUF - 1, daemoncmd);

	if (ret == 0) {

		if (strcmp(msg, "0") != 0) {
			debug(LOG_DEBUG, "Daemon pid: %s", msg);
		}
	} else {
		debug(LOG_INFO, "Failed start daemon from [%s] - retrying", cmd);
		sleep(1);

		ret = execute_ret_url_encoded(msg, STATUS_BUF - 1, daemoncmd);

		if (ret == 0) {

			if (strcmp(msg, "0") != 0) {
				debug(LOG_DEBUG, "Daemon pid: %s", msg);
			}
		} else {
			debug(LOG_INFO, "Failed start daemon from [%s] - giving up", cmd);
		}
	}
	free(daemoncmd);
	free(buff);
	free(msg);
	return ret;
}

int stopdaemon(int daemonpid)
{
	/* Stop a program that is running in the background, daemonpid contains the pid of the daemon
		Returns 0 if successful or 1 if failed to stop.
		Note: It might fail to stop because it was actually already stopped.
	*/
	char *msg;
	char *daemoncmd;
	int ret;

	msg = safe_calloc(STATUS_BUF);
	safe_asprintf(&daemoncmd, "/usr/lib/opennds/libopennds.sh stopdaemon '%d'",
		daemonpid
	);

	debug(LOG_DEBUG, "stopdaemon command: %s", daemoncmd);

	ret = execute_ret_url_encoded(msg, STATUS_BUF - 1, daemoncmd);

	if (ret == 0) {
		debug(LOG_DEBUG, "stopdaemon, pid: [%d], %s", daemonpid, msg);
	} else {
		debug(LOG_INFO, "Failed stopdaemon pid [%d] - retrying", daemonpid);
		sleep(1);

		ret = execute_ret_url_encoded(msg, STATUS_BUF - 1, daemoncmd);

		if (ret == 0) {
			debug(LOG_DEBUG, "stopdaemon, pid: [%d], %s", daemonpid, msg);
		} else {
			debug(LOG_INFO, "Failed stop daemon, pid [%d] - giving up", daemonpid);
		}
	}
	free(daemoncmd);
	free(msg);
	return ret;
}

void write_ndsinfo(void)
{
	char *cmd;
	char *msg;
	char write_yes[] = "done";

	s_config *config = config_get_config();

	msg = safe_calloc(SMALL_BUF);
	safe_asprintf(&cmd,
		"/usr/lib/opennds/libopennds.sh write ndsinfo '%s' 'tmpfsmountpoint=\"%s\"'",
		config->tmpfsmountpoint,
		config->tmpfsmountpoint
	);
	execute_ret_url_encoded(msg, SMALL_BUF - 1, cmd);

	msg = safe_calloc(SMALL_BUF);
	safe_asprintf(&cmd,
		"/usr/lib/opennds/libopennds.sh write ndsinfo '%s' 'gatewaynamehtml=\"%s\"'",
		config->tmpfsmountpoint,
		config->http_encoded_gw_name
	);
	execute_ret_url_encoded(msg, SMALL_BUF - 1, cmd);

	msg = safe_calloc(SMALL_BUF);
	safe_asprintf(&cmd,
		"/usr/lib/opennds/libopennds.sh write ndsinfo '%s' 'gatewayaddress=\"%s\"'",
		config->tmpfsmountpoint,
		config->gw_ip
	);
	execute_ret_url_encoded(msg, SMALL_BUF - 1, cmd);

	msg = safe_calloc(SMALL_BUF);
	safe_asprintf(&cmd,
		"/usr/lib/opennds/libopennds.sh write ndsinfo '%s' 'gatewayfqdn=\"%s\"'",
		config->tmpfsmountpoint,
		config->gw_fqdn
	);
	execute_ret_url_encoded(msg, SMALL_BUF - 1, cmd);

	msg = safe_calloc(SMALL_BUF);
	safe_asprintf(&cmd,
		"/usr/lib/opennds/libopennds.sh write ndsinfo '%s' 'version=%s'",
		config->tmpfsmountpoint,
		VERSION
	);
	execute_ret_url_encoded(msg, SMALL_BUF - 1, cmd);

	if (strcmp(msg, write_yes) != 0) {
		debug(LOG_ERR, "Unable to write ndsinfo, exiting ...");
		exit(1);
	}

	free(msg);
	free(cmd);
}

int check_routing(int watchdog)
{
	// Check routing configuration
	char *rtest;
	char *rcmd;
	char rtr_fail[] = "-";
	char rtr_offline[] = "offline";
	char rtr_online[] = "online";
	int online_count;
	int offline_count;
	s_config *config = config_get_config();

	safe_asprintf(&rcmd,
		"/usr/lib/opennds/libopennds.sh gatewayroute \"%s\"",
		config->gw_interface
	);

	rtest = safe_calloc(SMALL_BUF);

	if (execute_ret_url_encoded(rtest, SMALL_BUF - 1, rcmd) == 0) {
		online_count = count_substrings(rtest, rtr_online);
		offline_count = count_substrings(rtest, rtr_offline);


		if (strcmp(rtest, rtr_fail) == 0) {
			debug(LOG_ERR, "Routing configuration is not valid for openNDS, exiting ...");
			exit(1);
		} else {

			if (watchdog == 0) {
				debug(LOG_NOTICE, "Number of Upstream gateway(s) [ %d ]", (online_count + offline_count));
			}

			if (offline_count > 0) {
				// An upstream gateway is offline
				if (online_count == config->online_status) {
					// no change since last time so issue warning
					debug(LOG_WARNING, "Upstream gateway(s) [ %s ]", rtest);

				} else if (online_count > config->online_status) {
					// an interface came online
					debug(LOG_NOTICE, "Upstream gateway(s) [ %s ]", rtest);
					config->online_status = online_count;

				} else if (online_count < config->online_status) {
					// an interface went offline
					debug(LOG_WARNING, "Upstream gateway(s) [ %s ]", rtest);
					config->online_status = online_count;
				}
			} else {

				if (online_count > config->online_status) {
					// an interface came online
					debug(LOG_NOTICE, "Upstream gateway(s) [ %s ]", rtest);
				}

				config->online_status = online_count;
			}
		}

		config->ext_gateway = safe_strdup(rtest);
		free (rcmd);
		free (rtest);
		debug(LOG_DEBUG, "Online Status [ %d ]", config->online_status);
		return config->online_status;
	} else {
		debug(LOG_ERR, "Unable to get routing configuration, exiting ...");
		exit(1);
	}
}


int ndsctl_lock()
{
	char *lockfile;
	s_config *config = config_get_config();

	// Open or create the lock file
	safe_asprintf(&lockfile, "%s/ndsctl.lock", config->tmpfsmountpoint);
	config->lockfd = open(lockfile, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	free(lockfile);

	if (config->lockfd < 0) {
		debug(LOG_ERR, "CRITICAL ERROR - Unable to open [%s]", lockfile);
		return 5;
	}

	if (lockf(config->lockfd, F_TLOCK, 0) == 0) {
		return 0;
	} else {

		if (errno != EACCES && errno != EAGAIN) {
			// persistent error
			debug(LOG_ERR, "CRITICAL ERROR - Unable to create lock on [%s]", lockfile);
			close(config->lockfd);
			return 6;
		}

		debug(LOG_ERR, "ndsctl is locked by another process");
		close(config->lockfd);
		return 4;
	}
}

void ndsctl_unlock()
{
	s_config *config = config_get_config();

	if (lockf(config->lockfd, F_ULOCK, 0) < 0) {
		debug(LOG_ERR, "Unable to Unlock ndsctl");
	}

	close(config->lockfd);
}


int download_remotes(int refresh)
{
	char *cmd = NULL;
	int daemonpid = 0;
	s_config *config = config_get_config();

	if (config->themespec_path == NULL) {
		return 0;
	}

	if(refresh == 0) {
		debug(LOG_DEBUG, "Background Checking of remotes for: %s\n", config->themespec_path);
	} else {
		debug(LOG_DEBUG, "Background Refreshing of remotes for: %s\n", config->themespec_path);
	}

	safe_asprintf(&cmd,
		"/usr/lib/opennds/libopennds.sh download \"%s\" \"%s\" \"%s\" \"%d\" \"%s\"",
		config->themespec_path,
		config->custom_images,
		config->custom_files,
		refresh,
		config->webroot
	);

	if (config->online_status > 0) {
		debug(LOG_DEBUG, "Starting daemon: %s\n", cmd);

		if (startdaemon(cmd, daemonpid) == 0) {

			if (daemonpid != 0) {
				debug(LOG_DEBUG, "daemon(%s) pid is [%d]", cmd, daemonpid);
			}
		} else {
			debug(LOG_DEBUG, "Cannot download remotes - daemon failed to start");
		}
	} else {
		debug(LOG_DEBUG, "Cannot download remotes - upstream gateway(s) are offline");
	}

	free(cmd);
	return 0;
}

int write_client_info(char* msg, int msg_len, const char *mode, const char *cid, const char *info)
{
	char *cmd = NULL;
	s_config *config = config_get_config();

	debug(LOG_DEBUG, "Client Info: %s", info);
	safe_asprintf(&cmd, "/usr/lib/opennds/libopennds.sh '%s' '%s' '%s' '%s'", mode, cid, config->tmpfsmountpoint, info);
		debug(LOG_DEBUG, "WriteClientInfo command: %s", cmd);
	if (execute_ret_url_encoded(msg, msg_len - 1, cmd) == 0) {
		debug(LOG_DEBUG, "Client Info updated: %s", info);
	} else {
		debug(LOG_INFO, "Failed to write client info [%s] - retrying", info);
		sleep(1);

		if (execute_ret_url_encoded(msg, msg_len - 1, cmd) == 0) {
			debug(LOG_DEBUG, "Client Info updated: %s", info);
		} else {
			debug(LOG_INFO, "Failed to write client info [%s] - giving up", info);
		}
	}
	free (cmd);
	return 0;
}

int get_client_interface(char* clientif, int clientif_len, const char *climac)
{
	char *clifcmd = NULL;

	safe_asprintf(&clifcmd, "/usr/lib/opennds/get_client_interface.sh %s", climac);

	if (execute_ret_url_encoded(clientif, clientif_len - 1, clifcmd) == 0) {
		debug(LOG_DEBUG, "Client Mac Address: %s", climac);
		debug(LOG_DEBUG, "Client Connection(s) [localif] [remotemeshnodemac] [localmeshif]: %s", clientif);
	} else {
		debug(LOG_INFO, "Failed to get client connections for [%s] - retrying", climac);
		sleep(1);

		if (execute_ret_url_encoded(clientif, clientif_len - 1, clifcmd) == 0) {
			debug(LOG_DEBUG, "Client Connection(s) [localif] [remotemeshnodemac] [localmeshif]: %s", clientif);
		} else {
			debug(LOG_INFO, "Failed to get client connections for [%s] - giving up", climac);
		}
	}
	free (clifcmd);
	return 0;
}


int hash_str(char* hash, int hash_len, const char *src)
{
	char *hashcmd = NULL;

	s_config *config = config_get_config();

	safe_asprintf(&hashcmd, "printf '%s' | %s | awk -F' ' '{printf $1}'", src, config->fas_hid);

	if (execute_ret_url_encoded(hash, hash_len - 1, hashcmd) == 0) {
		debug(LOG_DEBUG, "Source string: %s", src);
		debug(LOG_DEBUG, "Hashed string: %s", hash);
	} else {
		debug(LOG_ERR, "Failed to hash string");
		free (hashcmd);
		return -1;
	}
	free (hashcmd);
	return 0;
}


static int _execute_ret(char* msg, int msg_len, const char *cmd)
{
	struct sigaction sa, oldsa;
	FILE *fp;
	int rc;
	size_t byte_count;

	debug(LOG_DEBUG, "Executing command: %s", cmd);

	// Temporarily get rid of SIGCHLD handler (see main.c), until child exits.
	sa.sa_handler = SIG_DFL;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
	if (sigaction(SIGCHLD, &sa, &oldsa) == -1) {
		debug(LOG_ERR, "sigaction() failed to set default SIGCHLD handler: %s", strerror(errno));
	}

	fp = popen(cmd, "r");
	if (fp == NULL) {
		debug(LOG_ERR, "popen(): [%s] Retrying..", strerror(errno));
		sleep(1);
		fp = popen(cmd, "r");

		if (fp == NULL) {
			debug(LOG_ERR, "popen(): [%s] Giving up..", strerror(errno));
			rc = -1;
			goto abort;
		}
	}

	if (msg && msg_len > 0) {
		debug(LOG_DEBUG, "Reading command output");
		byte_count = fread(msg, 1, msg_len - 1, fp);

		if (byte_count == msg_len - 1) {
			debug(LOG_ERR, "Buffer overflow, output may be truncated.");
		}

		debug(LOG_DEBUG, "command output: [%s]", msg);
	}

	rc = pclose(fp);

	if (WIFSIGNALED(rc) != 0) {
		debug(LOG_NOTICE, "Command process exited due to signal [%d]", WTERMSIG(rc));
		debug(LOG_NOTICE, "Requested command: [%s]", cmd);
		rc = WTERMSIG(rc);
	} else {
		rc = WEXITSTATUS(rc);
	}

abort:

	// Restore signal handler
	if (sigaction(SIGCHLD, &oldsa, NULL) == -1) {
		debug(LOG_ERR, "sigaction() failed to restore SIGCHLD handler! Error %s", strerror(errno));
	}

	return rc;
}

int execute(const char fmt[], ...)
{
	char cmd[QUERYMAXLEN];
	va_list vlist;
	int rc;

	va_start(vlist, fmt);
	rc = vsnprintf(cmd, sizeof(cmd), fmt, vlist);
	va_end(vlist);

	if (rc < 0 || rc >= sizeof(cmd)) {
		debug(LOG_ERR, "Format string too small or encoding error.");
		return -1;
	}

	return _execute_ret(NULL, 0, cmd);
}

int execute_ret(char* msg, int msg_len, const char fmt[], ...)
{
	char cmd[QUERYMAXLEN];
	va_list vlist;
	int rc;

	va_start(vlist, fmt);
	rc = vsnprintf(cmd, sizeof(cmd), fmt, vlist);
	va_end(vlist);

	if (rc < 0 || rc >= sizeof(cmd)) {
		debug(LOG_ERR, "Format string too small or encoding error.");
		return -1;
	}

	return _execute_ret(msg, msg_len, cmd);
}

/* Warning: Any client originated portion of the cmd string must be url encoded before calling this function.
 It may not be desired to url encode the entire cmd string,
 so it is our responsibility to encode the relevant parts (eg the clients original request url) before calling.
 */
int execute_ret_url_encoded(char* msg, int msg_len, const char *cmd)
{
	return _execute_ret(msg, msg_len, cmd);
}


char *
get_iface_ip(const char ifname[], int ip6)
{
	char addrbuf[INET6_ADDRSTRLEN] = {0};
	char cmd[256] = {0};
	char iptype[8] = {0};

	if (ip6) {
		snprintf(iptype, sizeof(iptype), "inet6");
	} else {
		snprintf(iptype, sizeof(iptype), "inet");
 	}

	snprintf(cmd, sizeof(cmd), "/usr/lib/opennds/libopennds.sh gatewayip \"%s\" \"%s\"",
		ifname,
		iptype
	);

	debug(LOG_NOTICE, "Attempting to Bind to interface: %s", ifname);

	if (execute_ret(addrbuf, sizeof(addrbuf), cmd) == 0) {
		return safe_strdup(addrbuf);
	} else {
		return "error";
	}
}

char *
get_iface_mac(const char ifname[])
{
	char addrbuf[20] = {0};
	char cmd[128] = {0};
	s_config *config;

	config = config_get_config();

	if (config->gw_mac == NULL) {
		config->gw_mac = safe_strdup("00:00:00:00:00:00");
	}

	snprintf(cmd, sizeof(cmd), "/usr/lib/opennds/libopennds.sh \"gatewaymac\" \"%s\" \"%s\"",
		ifname,
		config->gw_mac
	);


	execute_ret(addrbuf, sizeof(addrbuf), cmd);
	return safe_strdup(addrbuf);
}

char *
format_duration(time_t from, time_t to, char buf[64])
{
	int days, hours, minutes, seconds;
	long long int secs;
	const char *neg = "";

	if (from <= to) {
		secs = to - from;
	} else {
		secs = from - to;
		// Prepend minus sign
		neg = "-";
	}

	days = secs / (24 * 60 * 60);
	secs -= days * (24 * 60 * 60);
	hours = secs / (60 * 60);
	secs -= hours * (60 * 60);
	minutes = secs / 60;
	secs -= minutes * 60;
	seconds = secs;

	if (days > 0) {
		snprintf(buf, 64, "%s%dd %dh %dm %ds", neg, days, hours, minutes, seconds);
	} else if (hours > 0) {
		snprintf(buf, 64, "%s%dh %dm %ds", neg, hours, minutes, seconds);
	} else if (minutes > 0) {
		snprintf(buf, 64, "%s%dm %ds", neg, minutes, seconds);
	} else {
		snprintf(buf, 64, "%s%ds", neg, seconds);
	}

	return buf;
}

char *
format_time(time_t time, char buf[64])
{
	strftime(buf, 64, "%a %b %d %H:%M:%S %Y", localtime(&time));
	return buf;
}

char * get_uptime_string(char buf[64]) {
	time_t sysuptime;
	unsigned long int now, uptimesecs;

	sysuptime = get_system_uptime ();
	now = time(NULL);

	debug(LOG_DEBUG, "Uncorrected NDS Uptime: %li seconds ", (now - started_time));

	if ((now - started_time) > sysuptime) {
		uptimesecs = sysuptime;
	} else {
		uptimesecs = now - started_time;
	}

	return format_duration((now - uptimesecs), now, buf);
}

time_t get_system_uptime() {
	time_t sysuptime;
	char buf[64];
	FILE *pfp;

	pfp = fopen ("/proc/uptime", "r");

	if (pfp != NULL) {

		if(fgets (buf, sizeof(buf), pfp) != NULL) {
			sysuptime = atol(strtok(buf, "."));
			debug(LOG_DEBUG, "Operating System Uptime: %li seconds ", sysuptime);
			fclose (pfp);
			return sysuptime;
		}

		fclose (pfp);
	}

	debug(LOG_WARNING, "Unable to determine System Uptime.");
	return -1;
}


int is_addr(const char* addr) {
	struct sockaddr_in sa;
	struct sockaddr_in6 sa6;

	return (inet_pton(AF_INET, addr, &sa.sin_addr) == 1) ||
		(inet_pton(AF_INET6, addr, &sa6.sin6_addr) == 1);
}

void
ndsctl_status(FILE *fp)
{
	char timebuf[64];
	char durationbuf[64];
	s_config *config;
	t_client *client;
	int indx;
	unsigned int uploadburst = 0;
	unsigned int downloadburst = 0;
	unsigned long int now, uptimesecs, durationsecs = 0;
	unsigned long long int download_bytes, upload_bytes;
	t_MAC *trust_mac;
	time_t sysuptime;
	t_WGP *allowed_wgport;
	t_WGFQDN *allowed_wgfqdn;
	const char *mhdversion = MHD_get_version();

	config = config_get_config();

	if (config->upload_bucket_ratio > 0) {
		uploadburst = config->checkinterval * config->rate_check_window;
	}

	if (config->upload_bucket_ratio > 0) {
		downloadburst = config->checkinterval * config->rate_check_window;
	}

	fprintf(fp, "==================\nopenNDS Status\n====\n");
	sysuptime = get_system_uptime ();
	now = time(NULL);

	debug(LOG_DEBUG, "Uncorrected Uptime: %li seconds ", (now - started_time));

	if ((now - started_time) > sysuptime) {
		uptimesecs = sysuptime;
	} else {
		uptimesecs = (now - started_time) + 1;
	}

	fprintf(fp, "Version: " VERSION "\n");

	format_duration(0, uptimesecs, durationbuf);

	fprintf(fp, "Uptime: %s\n", durationbuf);
	fprintf(fp, "Gateway Name: [ %s ]\n", config->gw_name);
	fprintf(fp, "Debug Level: [ %d ]\n", config->debuglevel);
	fprintf(fp, "Gateway FQDN: [ %s ]\n", config->gw_fqdn);

	if (strstr(config->gw_iprange, "0.0.0.0/0")) {
		fprintf(fp, "Managed interface: %s\n", config->gw_interface);
	} else {
		fprintf(fp, "Managed interface: %s - IP address range: %s\n", config->gw_interface, config->gw_iprange);
	}

	// Check if router is online
	int watchdog = 0;
	int routercheck;
	routercheck = check_routing(watchdog);

	if (routercheck > 0) {
		fprintf(fp, "Upstream gateway(s) [ %s ]\n", config->ext_gateway);
	} else {
		fprintf(fp, "All Upstream gateway(s) are offline or not connected [ %s ]\n", config->ext_gateway);
	}

	fprintf(fp, "MHD Server [ version %s ] listening on: http://%s\n", mhdversion, config->gw_address);
	fprintf(fp, "Maximum Html Page size is [ %llu ] Bytes\n", HTMLMAXSIZE);

	if (config->allow_preemptive_authentication > 0) {
		fprintf(fp, "Preemptive Authentication is Enabled\n");
	} else {
		fprintf(fp, "Preemptive Authentication is Disabled\n");
	}

	if (config->binauth) {
		fprintf(fp, "Binauth Script: %s\n", config->binauth);
	} else {
		fprintf(fp, "Binauth: Disabled\n");
	}

	if (config->preauth) {
		fprintf(fp, "Preauth Script: %s\n", config->preauth);
	} else {
		fprintf(fp, "Preauth: Disabled\n");
	}

	if (config->fas_port) {
		fprintf(fp, "FAS: Secure Level %u, URL: %s\n",
			config->fas_secure_enabled,
			config->fas_url);
	} else {
		fprintf(fp, "FAS: Disabled\n");
	}

	fprintf(fp, "Client Check Interval: %ds\n", config->checkinterval);
	fprintf(fp, "Rate Check Window: %d check intervals (%ds)\n", config->rate_check_window, (config->rate_check_window * config->checkinterval));
	fprintf(fp, "Preauthenticated Client Idle Timeout: %dm\n", config->preauth_idle_timeout);
	fprintf(fp, "Authenticated Client Idle Timeout: %dm\n", config->auth_idle_timeout);

	if (config->download_rate > 0) {
		fprintf(fp, "Download rate limit threshold (default per client): %llu kbit/s\n", config->download_rate);
		fprintf(fp, "Download Burst Interval %u seconds\n", downloadburst);
	} else {
		fprintf(fp, "Download rate limit threshold (default per client): no limit\n");
	}
	if (config->upload_rate > 0) {
		fprintf(fp, "Upload rate limit threshold (default per client): %llu kbit/s\n", config->upload_rate);
		fprintf(fp, "Upload Burst Interval %u seconds\n", uploadburst);
	} else {
		fprintf(fp, "Upload rate limit threshold (default per client): no limit\n");
	}

	if (config->download_quota > 0) {
		fprintf(fp, "Download quota (default per client): %llu kB\n", config->download_quota);
	} else {
		fprintf(fp, "Download quota (default per client): no limit\n");
	}
	if (config->upload_quota > 0) {
		fprintf(fp, "Upload quota (default per client): %llu kB\n", config->upload_quota);
	} else {
		fprintf(fp, "Upload quota (default per client): no limit\n");
	}


	download_bytes = iptables_fw_total_download();
	fprintf(fp, "Total download: %llu kByte", download_bytes / 1024);
	fprintf(fp, "; average: %.2f kbit/s\n", ((double) download_bytes) / 125 / uptimesecs);

	upload_bytes = iptables_fw_total_upload();
	fprintf(fp, "Total upload: %llu kByte", upload_bytes / 1024);
	fprintf(fp, "; average: %.2f kbit/s\n", ((double) upload_bytes) / 125 / uptimesecs);

	fprintf(fp, "====\n");
	fprintf(fp, "Client authentications since start: %u\n", authenticated_since_start);

	// Update the client's counters so info is current
	iptables_fw_counters_update();

	LOCK_CLIENT_LIST();

	fprintf(fp, "Current clients: %d\n", get_client_list_length());

	client = client_get_first_client();
	if (client) {
		fprintf(fp, "\n");
	}

	indx = 0;
	while (client != NULL) {
		fprintf(fp, "Client %d\n", indx);

		if (!client->client_type || strlen(client->client_type) == 0) {
			fprintf(fp, "  Client Type: %s\n", "cpd_can");
		} else {
			fprintf(fp, "  Client Type: %s\n", client->client_type);
		}


		fprintf(fp, "  IP: %s MAC: %s\n", client->ip, client->mac);

		format_time(client->counters.last_updated, timebuf);
		format_duration(client->counters.last_updated, now, durationbuf);
		fprintf(fp, "  Last Activity: %s (%s ago)\n", timebuf, durationbuf);

		if (client->session_start) {
			format_time(client->session_start, timebuf);
			format_duration(client->session_start, now, durationbuf);
			fprintf(fp, "  Session Start: %s (%s ago)\n", timebuf, durationbuf);
		} else {
			fprintf(fp, "  Session Start: -\n");
		}

		if (client->session_end) {
			format_time(client->session_end, timebuf);
			format_duration(now, client->session_end, durationbuf);
			fprintf(fp, "  Session End:   %s (%s left)\n", timebuf, durationbuf);
		} else {
			fprintf(fp, "  Session End:   -\n");
		}

		fprintf(fp, "  Token: %s\n", client->token ? client->token : "none");
		fprintf(fp, "  State: %s\n", fw_connection_state_as_string(client->fw_connection_state));

		if (client->download_rate == 0) {
			fprintf(fp, "  Download Rate Limit Threshold: not set\n");
		} else {
			fprintf(fp, "  Download Rate Limit Threshold: %llu kb/s\n", client->download_rate);

			if (client->inc_packet_limit == 0) {
				fprintf(fp, "  Download Packet Rate Limit: Not active\n");
				fprintf(fp, "  Download Bucket Size: Not active\n");
			} else {
				fprintf(fp, "  Download Packet Rate Limit: %llu packets/min\n", client->inc_packet_limit);
				fprintf(fp, "  Download Bucket Size: %llu packets\n", client->download_bucket_size);
			}
		}


		if (client->upload_rate == 0) {
			fprintf(fp, "  Upload Rate Limit Threshold: not set\n");
		} else {
			fprintf(fp, "  Upload Rate Limit Threshold: %llu kb/s\n", client->upload_rate);

			if (client->out_packet_limit == 0) {
				fprintf(fp, "  Upload Packet Rate Limit: Not active\n");
				fprintf(fp, "  Upload Bucket Size: Not active\n");
			} else {
				fprintf(fp, "  Upload Packet Rate Limit: %llu packets/min\n", client->out_packet_limit);
				fprintf(fp, "  Upload Bucket Size: %llu packets\n", client->upload_bucket_size);
			}
		}

		if (client->download_quota == 0) {
			fprintf(fp, "  Download quota: not set\n");
		} else {
			fprintf(fp, "  Download quota: %llu kB\n", client->download_quota);
		}

		if (client->upload_quota == 0) {
			fprintf(fp, "  Upload quota: not set\n");
		} else {
			fprintf(fp, "  Upload quota: %llu kB\n", client->upload_quota);
		}

		download_bytes = client->counters.incoming;
		upload_bytes = client->counters.outgoing;
		durationsecs = now - client->session_start;

		// prevent divison by 0
		if (durationsecs < 1) {
			durationsecs = 1;
		}

		fprintf(fp, "  Download this session: %llu kB; Session average: %.2f kb/s\n",
			download_bytes / 1024,
			((double)download_bytes) / 125 / durationsecs)
		;

		fprintf(fp, "  Upload this session: %llu kB; Session average: %.2f kb/s\n\n",
			upload_bytes / 1024,
			((double)upload_bytes) / 125 / durationsecs)
		;

		indx++;
		client = client->next;
	}

	UNLOCK_CLIENT_LIST();

	fprintf(fp, "====\n");

	fprintf(fp, "Trusted MAC addresses:");

	if (config->trustedmaclist != NULL) {
		fprintf(fp, "\n");
		for (trust_mac = config->trustedmaclist; trust_mac != NULL; trust_mac = trust_mac->next) {
			fprintf(fp, "  %s\n", trust_mac->mac);
		}
	} else {
		fprintf(fp, " none\n");
	}

	fprintf(fp, "Walled Garden FQDNs:");

	if (config->walledgarden_fqdn_list != NULL) {
		fprintf(fp, "\n");
		for (allowed_wgfqdn = config->walledgarden_fqdn_list; allowed_wgfqdn != NULL; allowed_wgfqdn = allowed_wgfqdn->next) {
			fprintf(fp, "  %s\n", allowed_wgfqdn->wgfqdn);
		}
	} else {
		fprintf(fp, " none\n");
	}

	fprintf(fp, "Walled Garden Ports:");

	if (config->walledgarden_port_list != NULL) {
		fprintf(fp, "\n");
		for (allowed_wgport = config->walledgarden_port_list; allowed_wgport != NULL; allowed_wgport = allowed_wgport->next) {
			fprintf(fp, "  %u\n", allowed_wgport->wgport);
		}
	} else {
		fprintf(fp, " none\n");
	}

	fprintf(fp, "========\n");
}

static void
ndsctl_json_client(FILE *fp, const t_client *client, time_t now, char *indent)
{
	unsigned long int durationsecs;
	unsigned long long int download_bytes, upload_bytes;
	char clientif[64] = {0};
	s_config *config;

	config = config_get_config();
	get_client_interface(clientif, sizeof(clientif), client->mac);

	fprintf(fp, "  %s\"gatewayname\":\"%s\",\n", indent, config->url_encoded_gw_name);
	fprintf(fp, "  %s\"gatewayaddress\":\"%s\",\n", indent, config->gw_address);
	fprintf(fp, "  %s\"gatewayfqdn\":\"%s\",\n", indent, config->gw_fqdn);
	fprintf(fp, "  %s\"version\":\"%s\",\n", indent, VERSION);

	if (!client->client_type || strlen(client->client_type) == 0) {
		fprintf(fp, "  %s\"client_type\":\"%s\",\n", indent, "cpd_can");
	} else {
		fprintf(fp, "  %s\"client_type\":\"%s\",\n", indent, client->client_type);
	}

	fprintf(fp, "  %s\"mac\":\"%s\",\n", indent, client->mac);
	fprintf(fp, "  %s\"ip\":\"%s\",\n", indent, client->ip);
	fprintf(fp, "  %s\"clientif\":\"%s\",\n", indent, clientif);
	fprintf(fp, "  %s\"session_start\":\"%lld\",\n", indent, (long long) client->session_start);

	if (client->session_end == 0) {
		fprintf(fp, "  %s\"session_end\":\"null\",\n", indent);
	} else {
		fprintf(fp, "  %s\"session_end\":\"%lld\",\n", indent, (long long) client->session_end);
	}

	fprintf(fp, "  %s\"last_active\":\"%lld\",\n", indent, (long long) client->counters.last_updated);
	fprintf(fp, "  %s\"token\":\"%s\",\n", indent, client->token ? client->token : "none");
	fprintf(fp, "  %s\"state\":\"%s\",\n", indent, fw_connection_state_as_string(client->fw_connection_state));

	if (!client->custom || strlen(client->custom) == 0) {
		fprintf(fp, "  %s\"custom\":\"%s\",\n", indent, "none");
	} else {
		fprintf(fp, "  %s\"custom\":\"%s\",\n", indent, client->custom);
	}

	durationsecs = now - client->session_start;
	download_bytes = client->counters.incoming;
	upload_bytes = client->counters.outgoing;

	if (client->download_rate == 0) {
		fprintf(fp, "  %s\"download_rate_limit_threshold\":\"null\",\n", indent);
		fprintf(fp, "  %s\"download_packet_rate\":\"null\",\n", indent);
		fprintf(fp, "  %s\"download_bucket_size\":\"null\",\n", indent);
	} else {
		fprintf(fp, "  %s\"download_rate_limit_threshold\":\"%llu\",\n", indent, client->download_rate);

		if (client->inc_packet_limit == 0) {
			fprintf(fp, "  %s\"download_packet_rate\":\"null\",\n", indent);
			fprintf(fp, "  %s\"download_bucket_size\":\"null\",\n", indent);
		} else {
			fprintf(fp, "  %s\"download_packet_rate\":\"%llu\",\n", indent, client->inc_packet_limit);
			fprintf(fp, "  %s\"download_bucket_size\":\"%llu\",\n", indent, client->download_bucket_size);
		}
	}

	if (client->upload_rate == 0) {
		fprintf(fp, "  %s\"upload_rate_limit_threshold\":\"null\",\n", indent);
		fprintf(fp, "  %s\"upload_packet_rate\":\"null\",\n", indent);
		fprintf(fp, "  %s\"upload_bucket_size\":\"null\",\n", indent);
	} else {
		fprintf(fp, "  %s\"upload_rate_limit_threshold\":\"%llu\",\n", indent, client->upload_rate);

		if (client->out_packet_limit == 0) {
			fprintf(fp, "  %s\"upload_packet_rate\":\"null\",\n", indent);
			fprintf(fp, "  %s\"upload_bucket_size\":\"null\",\n", indent);
		} else {
			fprintf(fp, "  %s\"upload_packet_rate\":\"%llu\",\n", indent, client->out_packet_limit);
			fprintf(fp, "  %s\"upload_bucket_size\":\"%llu\",\n", indent, client->upload_bucket_size);
		}
	}

	if (client->download_quota == 0) {
		fprintf(fp, "  %s\"download_quota\":\"null\",\n", indent);
	} else {
		fprintf(fp, "  %s\"download_quota\":\"%llu\",\n", indent, client->download_quota);
	}

	if (client->upload_quota == 0) {
		fprintf(fp, "  %s\"upload_quota\":\"null\",\n", indent);
	} else {
		fprintf(fp, "  %s\"upload_quota\":\"%llu\",\n", indent, client->upload_quota);
	}

	// prevent divison by 0
	if (durationsecs < 1) {
		durationsecs = 1;
	}

	fprintf(fp, "  %s\"download_this_session\":\"%llu\",\n",
		indent,
		(download_bytes / 1024)
	);

	fprintf(fp, "  %s\"download_session_avg\":\"%.2f\",\n",
		indent,
		(double)download_bytes / 125 / durationsecs
	);

	fprintf(fp, "  %s\"upload_this_session\":\"%llu\",\n",
		indent,
		(upload_bytes / 1024)
	);

	fprintf(fp, "  %s\"upload_session_avg\":\"%.2f\"\n",
		indent,
		(double)upload_bytes / 125 / durationsecs
	);
}

static void
ndsctl_json_one(FILE *fp, const char *arg, char *indent)
{
	t_client *client;
	time_t now;
	now = time(NULL);

	// Update the client's counters so info is current
	iptables_fw_counters_update();

	LOCK_CLIENT_LIST();

	client = client_list_find_by_any(arg, arg, arg);

	if (client) {
		fprintf(fp, "{\n");
		ndsctl_json_client(fp, client, now, indent);
		fprintf(fp, "}\n");
	} else {
		fprintf(fp, "{}\n");
	}

	UNLOCK_CLIENT_LIST();
}

static void
ndsctl_json_all(FILE *fp, char *indent)
{
	t_client *client;
	time_t now;
	t_MAC *trust_mac;
	s_config *config;
	int count = 0;

	now = time(NULL);

	config = config_get_config();

	// Update the client's counters so info is current
	iptables_fw_counters_update();

	LOCK_CLIENT_LIST();

	fprintf(fp, "{\n  \"client_list_length\":\"%d\",\n", get_client_list_length());

	client = client_get_first_client();

	fprintf(fp, "  \"clients\":{\n");

	while (client != NULL) {
		fprintf(fp, "%s\"%s\":{\n", indent, client->mac);
		ndsctl_json_client(fp, client, now, indent);

		client = client->next;
		if (client) {
			fprintf(fp, "%s},\n", indent);
		} else {
			fprintf(fp, "%s}\n", indent);
		}
	}

	UNLOCK_CLIENT_LIST();

	// Trusted mac list
	if (config->trustedmaclist != NULL) {
		fprintf(fp, "  },\n");
		// count the number of trusted mac addresses
		for (trust_mac = config->trustedmaclist; trust_mac != NULL; trust_mac = trust_mac->next) {
			count++;
		}

		// output the count of trusted macs and list them in json array format
		fprintf(fp, "  \"trusted_list_length\":\"%d\",\n", count);
		fprintf(fp, "  \"trusted\":[\n");

		for (trust_mac = config->trustedmaclist; trust_mac != NULL; trust_mac = trust_mac->next) {

			if (count > 1) {
				fprintf(fp, "    \"%s\",\n", trust_mac->mac);
				count--;
			} else {
				fprintf(fp, "    \"%s\"\n", trust_mac->mac);
			}
		}

		fprintf(fp, "  ]\n");
	} else {
		fprintf(fp, "  }\n");
	}
	fprintf(fp, "}\n");
}

void
ndsctl_json(FILE *fp, const char *arg)
{
	char indent[5] = {0};
	//if (arg && strlen(arg)) {
	debug(LOG_DEBUG, "arg [%s %d]", arg, strlen(arg));
	if (strlen(arg) > 6) {
		//snprintf(indent, sizeof(indent), "%s", "");
		ndsctl_json_one(fp, arg, indent);
	} else {
		snprintf(indent, sizeof(indent), "%s", "    ");
		ndsctl_json_all(fp, indent);
	}
}

unsigned short
rand16(void)
{
	static int been_seeded = 0;

	if (!been_seeded) {
		unsigned int seed = 0;
		struct timeval now;

		// not a very good seed but what the heck, it needs to be quickly acquired
		gettimeofday(&now, NULL);
		seed = now.tv_sec ^ now.tv_usec ^ (getpid() << 16);

		srand(seed);
		been_seeded = 1;
	}

	/* Some rand() implementations have less randomness in low bits
	 than in high bits, so we only pay attention to the high ones.
	 But most implementations don't touch the high bit, so we
	 ignore that one.
	 */
	return( (unsigned short) (rand() >> 15) );
}
