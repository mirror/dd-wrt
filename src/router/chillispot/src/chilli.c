/* 
 * chilli - ChilliSpot.org. A Wireless LAN Access Point Controller.
 *
 * Copyright (c) 2006, Jens Jakobsen 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 *   Neither the names of copyright holders nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Copyright (C) 2003, 2004, 2005 Mondru AB.
 *
 * The contents of this file may be used under the terms of the GNU
 * General Public License Version 2, provided that the above copyright
 * notice and this permission notice is included in all copies or
 * substantial portions of the software.
 * 
 */


#include <syslog.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sysinfo.h>

#if defined(__linux__)
#include <asm/types.h>
#include <linux/netlink.h> 
#include <linux/if_ether.h>

#elif defined (__FreeBSD__)  || defined (__APPLE__)
#include <netinet/in.h>
#endif

#if defined (__OpenBSD__)
#include <netinet/in.h>
#include <net/if_tun.h>
#ifndef EIDRM
#define EIDRM   EINVAL
#endif
#ifndef ENOMSG
#define ENOMSG  EAGAIN
#endif
#endif

#if defined (__NetBSD__)
#include <netinet/in.h>
#include <net/if_tun.h>
#endif

#include <time.h>
#include <sys/time.h>

#include <stdio.h>
#include <ctype.h>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include <resolv.h> /* _res */

#include "../config.h"
#include "tun.h"
#include "ippool.h"
#include "radius.h"
#include "radius_wispr.h"
#include "radius_chillispot.h"
#include "redir.h"
#include "syserr.h"
#include "dhcp.h"
#include "cmdline.h"
#include "chilli.h"

struct options_t options;

struct tun_t *tun;                /* TUN instance            */
struct ippool_t *ippool;          /* Pool of IP addresses */
struct radius_t *radius;          /* Radius client instance */
struct dhcp_t *dhcp = NULL;       /* DHCP instance */
struct redir_t *redir = NULL;     /* Redir instance */

struct app_conn_t connection[APP_NUM_CONN];
struct app_conn_t *firstfreeconn; /* First free in linked list */
struct app_conn_t *lastfreeconn;  /* Last free in linked list */
struct app_conn_t *firstusedconn; /* First used in linked list */
struct app_conn_t *lastusedconn;  /* Last used in linked list */

struct timeval checktime;
struct timeval rereadtime;

static int keep_going = 1;
static int do_timeouts = 1;
static int do_sighup = 0;

/* Forward declarations */
int static acct_req(struct app_conn_t *conn, int status_type);
int static config_radius();

/* Fireman catches falling childs and eliminates zombies */
void static fireman(int signum) { 
  while (wait3(NULL, WNOHANG, NULL) > 0);
}

/* Termination handler for clean shutdown */
void static termination_handler(int signum) {
  if (options.debug) printf("SIGTERM received!\n");
  keep_going = 0;
}

/* Alarm handler for general house keeping */
void static alarm_handler(int signum) {
  /*if (options.debug) printf("SIGALRM received!\n");*/
  do_timeouts = 1;
}

/* Sighup handler for rereading configuration file */
void static sighup_handler(int signum) {
  if (options.debug) printf("SIGHUP received!\n");
  do_sighup = 1;
}


void static set_sessionid(struct app_conn_t *appconn) {
  struct timeval timenow;
  gettimeofday(&timenow, NULL);
  (void) snprintf(appconn->sessionid, REDIR_SESSIONID_LEN, "%.8x%.8x",
	   (int) timenow.tv_sec, appconn->unit);
}

/* Used to write process ID to file. Assume someone else will delete */
void static log_pid(char *pidfile) {
  FILE *file;
  mode_t oldmask;

  oldmask = umask(022);
  file = fopen(pidfile, "w");
  umask(oldmask);
  if(!file)
    return;
  fprintf(file, "%d\n", getpid());
  (void) fclose(file);
}

#ifndef NO_LEAKY_BUCKET
/* Perform leaky bucket on up- and downlink traffic */
int static leaky_bucket(struct app_conn_t *conn, int octetsup, int octetsdown) {
  
  struct timeval timenow;
  uint64_t timediff; /* In microseconds */
  int result = 0;

 
  gettimeofday(&timenow, NULL);

  timediff = (timenow.tv_sec - conn->last_time.tv_sec) * ((uint64_t) 1000000);
  timediff += (timenow.tv_usec - conn->last_time.tv_usec);

  /*  if (options.debug) printf("Leaky bucket timediff: %lld, bucketup: %d, bucketdown: %d %d %d\n", 
			    timediff, conn->bucketup, conn->bucketdown, 
			    octetsup, octetsdown);*/

  if (conn->bandwidthmaxup) {

    /* Subtract what the leak since last time we visited */
    if (conn->bucketup > ((timediff * conn->bandwidthmaxup)/8000000)) {
      conn->bucketup -= (timediff * conn->bandwidthmaxup) / 8000000;
    }
    else {
      conn->bucketup = 0;
    }
    
    if ((conn->bucketup + octetsup) > conn->bucketupsize) {
      /*if (options.debug) printf("Leaky bucket deleting uplink packet\n");*/
      result = -1;
    }
    else {
      conn->bucketup += octetsup;
    }
  }

  if (conn->bandwidthmaxdown) {
    if (conn->bucketdown > ((timediff * conn->bandwidthmaxdown)/8000000)) {
      conn->bucketdown -= (timediff * conn->bandwidthmaxdown) / 8000000;
    }
    else {
      conn->bucketdown = 0;
    }
    
    if ((conn->bucketdown + octetsdown) > conn->bucketdownsize) {
      /*if (options.debug) printf("Leaky bucket deleting downlink packet\n");*/
      result = -1;
    }
    else {
      conn->bucketdown += octetsdown;
    }
  }

  gettimeofday(&conn->last_time, NULL);
    
  return result;
}
#endif /* ifndef NO_LEAKY_BUCKET */

/* Run external script */

int set_env(char *name, char *value, int len, struct in_addr *addr,
	    uint8_t *mac, long int *integer) {
  char s[1024];
  if (addr!=NULL) {
    strncpy(s, inet_ntoa(*addr), sizeof(s)); s[sizeof(s)-1] = 0;
    value = s;
  }
  else if (mac != NULL) {
    (void) snprintf(s, sizeof(s)-1, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
		    mac[0], mac[1],
		    mac[2], mac[3],
		    mac[4], mac[5]);
    value = s;
  }
  else if (integer != NULL) {
    (void) snprintf(s, sizeof(s)-1, "%d", *integer);
    value = s;
  }
  else if (len != 0) {
    if (len >= sizeof(s)) {
      return 0;
    }
    memcpy(s, value, len);
    s[len] = 0;
    value = s;
  }
  if (name != NULL && value!= NULL) {
    if (setenv(name, value, 1) != 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "setenv(%s, %s, 1) did not return 0!", name, value);
      exit(0);
    }
  }
}

int runscript(struct app_conn_t *appconn, char* script) {  
  long int l;
  int status;

  if ((status = fork()) < 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "fork() returned -1!");
    return 0;
  }

  if (status > 0) { /* Parent */
    return 0; 
  }

  if (clearenv() != 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "clearenv() did not return 0!");
    exit(0);
  }

  set_env("DEV", tun->devname, 0, NULL, NULL, NULL);
  set_env("NET", NULL, 0, &appconn->net, NULL, NULL);
  set_env("MASK", NULL, 0, &appconn->mask, NULL, NULL);
  set_env("ADDR", NULL, 0, &appconn->ourip,NULL, NULL);
  set_env("USER_NAME", appconn->proxyuser, 0, NULL, NULL, NULL);
  set_env("NAS_IP_ADDRESS", NULL, 0, &options.radiusnasip, NULL, NULL);
  set_env("SERVICE_TYPE", "1", 0, NULL, NULL, NULL);
  set_env("FRAMED_IP_ADDRESS", NULL, 0, &appconn->hisip, NULL, NULL);
  set_env("FILTER_ID", appconn->filteridbuf, 0, NULL, NULL, NULL);
  set_env("STATE", (char*) appconn->statebuf, appconn->statelen, NULL, NULL, NULL);
  set_env("CLASS", (char*) appconn->classbuf, appconn->classlen, NULL, NULL, NULL);
  set_env("SESSION_TIMEOUT", NULL, 0, NULL, NULL, &appconn->sessiontimeout);
  set_env("IDLE_TIMEOUT", NULL, 0, NULL, NULL, &appconn->idletimeout);
  set_env("CALLING_STATION_ID", NULL, 0, NULL, appconn->hismac, NULL);
  set_env("CALLED_STATION_ID", options.radiuscalled, 0, NULL, NULL, NULL);
  set_env("NAS_ID", options.radiusnasid, 0, NULL, NULL, NULL);
  set_env("NAS_PORT_TYPE", "19", 0, NULL, NULL, NULL);
  set_env("ACCT_SESSION_ID", appconn->sessionid, 0, NULL, NULL, NULL);
  l = appconn->interim_interval;
  set_env("ACCT_INTERIM_INTERVAL", NULL, 0, NULL, NULL, &l);
  set_env("WISPR_LOCATION_ID", options.radiuslocationid, 0, NULL, NULL, NULL);
  set_env("WISPR_LOCATION_NAME", options.radiuslocationname, 0, NULL, NULL, NULL);
  l = appconn->bandwidthmaxup;
  set_env("WISPR_BANDWIDTH_MAX_UP", NULL, 0, NULL, NULL, &l);
  l = appconn->bandwidthmaxdown;
  set_env("WISPR_BANDWIDTH_MAX_DOWN", NULL, 0, NULL, NULL, &l);
  /*set_env("WISPR-SESSION_TERMINATE_TIME", appconn->sessionterminatetime, 0,
    NULL, NULL, NULL);*/
  l = appconn->maxinputoctets;
  set_env("CHILLISPOT_MAX_INPUT_OCTETS", NULL, 0, NULL, NULL, &l);
  l = appconn->maxoutputoctets;
  set_env("CHILLISPOT_MAX_OUTPUT_OCTETS", NULL, 0, NULL, NULL, &l);
  l = appconn->maxtotaloctets;
  set_env("CHILLISPOT_MAX_TOTAL_OCTETS", NULL, 0, NULL, NULL, &l);

  if (execl(script, script, (char *) 0) != 0) {
      sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	      "execl() did not return 0!");
      exit(0);
  }

  exit(0);
}



/* Extract domain name and port from URL */
int static get_namepart(char *src, char *host, int hostsize, int *port) {
  char *slashslash = NULL;
  char *slash = NULL;
  char *colon = NULL;
  int hostlen;
  
  *port = 0;

  if (!memcmp(src, "http://", 7)) {
    *port = DHCP_HTTP;
  }
  else   if (!memcmp(src, "https://", 8)) {
    *port = DHCP_HTTPS;
  }
  else {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "URL must start with http:// or https:// %s!", src);
    return -1;
  }
  
  /* The host name must be initiated by "//" and terminated by /, :  or \0 */
  if (!(slashslash = strstr(src, "//"))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "// not found in url: %s!", src);
    return -1;
  }
  slashslash+=2;
  
  slash = strstr(slashslash, "/");
  colon = strstr(slashslash, ":");
  
  if ((slash != NULL) && (colon != NULL) &&
      (slash < colon)) {
    hostlen = slash - slashslash;
  }
  else if ((slash != NULL) && (colon == NULL)) {
    hostlen = slash - slashslash;
  }
  else if (colon != NULL) {
    hostlen = colon - slashslash;
    if (1 != sscanf(colon+1, "%d", port)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Not able to parse URL port: %s!", src);
      return -1;
    }
  }
  else {
    hostlen = strlen(src);
  }

  if (hostlen > (hostsize-1)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "URL hostname larger than %d: %s!", hostsize-1, src);
    return -1;
  }

  strncpy(host, slashslash, hostsize);
  host[hostlen] = 0;

  return 0;
}

int static set_uamallowed(char *uamallowed, int len) {
  char *p1 = NULL;
  char *p2 = NULL;
  char *p3 = malloc(len+1);
  struct hostent *host;
  char hostname[USERURLSIZE];

  memcpy(p3, uamallowed, len);

  p3[len] = 0;
  p1 = p3;
  if ((p2 = strchr(p1, ','))) {
    *p2 = '\0';
  }
  while (p1) {
    if (strchr(p1, '/')) {
      if (options.uamoknetlen>=UAMOKNET_MAX) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"Too many network segments in uamallowed %s!",
		p3);
	free(p3);
	return -1;
      }
      if(ippool_aton(&options.uamokaddr[options.uamoknetlen], 
		     &options.uamokmask[options.uamoknetlen], 
		     p1, 0)) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"Invalid uamallowed network address or mask %s!", 
		p3);
	free(p3);
	return -1;
      }
      options.uamoknetlen++;
    }
    else {
      if (!(host = gethostbyname(p1))) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
		"Invalid uamallowed domain or address: %s!", 
		p3);
	free(p3);
	return -1;
      }
      else {
	int j = 0;
	while (host->h_addr_list[j] != NULL) {
	  if (options.debug & DEBUG_CONF) {
	    printf("Uamallowed IP address #%d:%d: %s\n", 
		   j, options.uamokiplen,
		   inet_ntoa(*(struct in_addr*) host->h_addr_list[j]));
	  }
	  if (options.uamokiplen>=UAMOKIP_MAX) {
	    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		    "Too many domains or IPs in uamallowed %s!",
		    p3);
	    free(p3);
	    return -1;
	  }
	  else {
	    options.uamokip[options.uamokiplen++] = 
	      *((struct in_addr*) host->h_addr_list[j++]);
	  }
	}
      }
    }
    if (p2) {
      p1 = p2+1;
      if ((p2 = strchr(p1, ','))) {
	*p2 = '\0';
      }
    }
    else {
      p1 = NULL;
    }
  }
  free(p3);
  return 0;
}

int static set_macallowed(char *macallowed, int len) {
  char *p1 = NULL;
  char *p2 = NULL;
  char *p3 = malloc(len+1);
  p3[len] = 0;
  int i;
  strcpy(p3, macallowed);
  p1 = p3;
  if ((p2 = strchr(p1, ','))) {
    *p2 = '\0';
  }
  while (p1) {
    if (options.macoklen>=MACOK_MAX) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Too many addresses in macallowed %s!",
	      p3);
      free(p3);
      return -1;
    }
    /* Replace anything but hex and comma with space */
    for (i=0; i<strlen(p1); i++) 
      if (!isxdigit(p1[i])) p1[i] = 0x20;
    
    if (sscanf (p1, "%2x %2x %2x %2x %2x %2x",
		&options.macok[options.macoklen][0], 
		&options.macok[options.macoklen][1], 
		&options.macok[options.macoklen][2], 
		&options.macok[options.macoklen][3], 
		&options.macok[options.macoklen][4], 
		&options.macok[options.macoklen][5]) != 6) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to convert macallowed option to MAC Address");
      free(p3);
      return -1;
    }
    if (options.debug & DEBUG_CONF) {
      printf("Macallowed address #%d: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n", 
	     options.macoklen,
	     options.macok[options.macoklen][0],
	     options.macok[options.macoklen][1],
	     options.macok[options.macoklen][2],
	     options.macok[options.macoklen][3],
	     options.macok[options.macoklen][4],
	     options.macok[options.macoklen][5]);
    }
    options.macoklen++;
    
    if (p2) {
      p1 = p2+1;
      if ((p2 = strchr(p1, ','))) {
	*p2 = '\0';
      }
    }
    else {
      p1 = NULL;
    }
  }
  free(p3);
}

int static process_options(int argc, char **argv, int firsttime) {
  struct gengetopt_args_info args_info;
  struct hostent *host;
  char hostname[USERURLSIZE];
  int numargs;

  if (cmdline_parser (argc, argv, &args_info) != 0) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to parse command line options");
    return -1;
  }

  if (cmdline_parser_configfile (args_info.conf_arg, &args_info, 0, 0, 0)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to parse configuration file: %s!", 
	    args_info.conf_arg);
    return -1;
  }

  /* Get the system default DNS entries */
  if (res_init()) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to update system DNS settings (res_init()!");
    return -1;
  }

  /* Handle each option */

  /* debug                                                        */
  if (args_info.debug_flag) {
    options.debug = args_info.debugfacility_arg;
  }
  else {
    options.debug = 0;
  }

  /* interval */
  options.interval = args_info.interval_arg;


  /* Currently we do not need statedir for chilli                   */

  /* dhcpif */
  if (!args_info.dhcpif_arg) {
    options.nodhcp = 1;
  }
  else {
    options.nodhcp = 0;
    options.dhcpif = args_info.dhcpif_arg;
  }

  /* dhcpmac */
  if (!args_info.dhcpmac_arg) {
    memset(options.dhcpmac, 0, DHCP_ETH_ALEN);
    options.dhcpusemac  = 0;
  }
  else {
    unsigned int temp[DHCP_ETH_ALEN];
    int	i;
    char macstr[RADIUS_ATTR_VLEN];
    int macstrlen;

    if ((macstrlen = strlen(args_info.dhcpmac_arg)) >= (RADIUS_ATTR_VLEN-1)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "MAC address too long");
      return -1;
    }
    memcpy(macstr, args_info.dhcpmac_arg, macstrlen);
    macstr[macstrlen] = 0;

    /* Replace anything but hex with space */
    for (i=0; i<macstrlen; i++) 
      if (!isxdigit(macstr[i])) macstr[i] = 0x20;

    if (sscanf (macstr, "%2x %2x %2x %2x %2x %2x", 
		&temp[0], &temp[1], &temp[2], 
		&temp[3], &temp[4], &temp[5]) != 6) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "MAC conversion failed!");
      return -1;
    }
    
    for(i = 0; i < DHCP_ETH_ALEN; i++) 
      options.dhcpmac[i] = temp[i];
    options.dhcpusemac  = 1;
  }

  /* lease                                                           */
  options.lease = args_info.lease_arg;

  /* eapolenable                                                     */
  options.eapolenable = args_info.eapolenable_flag;

  /* net                                                          */
  /* Store net as in_addr net and mask                            */
  if (args_info.net_arg) {
    if(ippool_aton(&options.net, &options.mask, args_info.net_arg, 0)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Invalid network address: %s!", args_info.net_arg);
      return -1;
    }
    /* Set DHCP server IP address to network address plus 1 */
    options.dhcplisten.s_addr = htonl(ntohl(options.net.s_addr)+1);
  }
  else {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Network address must be specified: %s!", args_info.net_arg);
    return -1;
  }

  /* uamserver                                                    */
  if (options.debug & DEBUG_CONF) {
    printf ("Uamserver: %s\n", args_info.uamserver_arg);
  }
  memset(options.uamserver, 0, sizeof(options.uamserver));
  options.uamserverlen = 0;
  if (get_namepart(args_info.uamserver_arg, hostname, USERURLSIZE, 
		   &options.uamserverport)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to parse uamserver: %s!", args_info.uamserver_arg);
    return -1;
  }
  
  if (!(host = gethostbyname(hostname))) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	    "Could not resolve IP address of uamserver: %s!", 
	    args_info.uamserver_arg);
    return -1;
  }
  else {
    int j = 0;
    while (host->h_addr_list[j] != NULL) {
      if (options.debug & DEBUG_CONF) {
	printf("Uamserver IP address #%d: %s\n", j,
	       inet_ntoa(*(struct in_addr*) host->h_addr_list[j]));
      }
      if (options.uamserverlen>=UAMSERVER_MAX) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"Too many IPs in uamserver %s!",
		args_info.uamserver_arg);
	return -1;
      }
      else {
	options.uamserver[options.uamserverlen++] = 
	  *((struct in_addr*) host->h_addr_list[j++]);
      }
    }
  }
  options.uamurl = args_info.uamserver_arg;
  

  /* uamhomepage                                                  */
  if (!args_info.uamhomepage_arg) {
    options.uamhomepage = args_info.uamhomepage_arg;
  }
  else {
    if (get_namepart(args_info.uamhomepage_arg, hostname, USERURLSIZE, 
		     &options.uamhomepageport)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to parse uamhomepage: %s!", args_info.uamhomepage_arg);
      return -1;
    }

    if (!(host = gethostbyname(hostname))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	      "Invalid uamhomepage: %s!", 
	      args_info.uamhomepage_arg);
      return -1;
    }
    else {
      int j = 0;
      while (host->h_addr_list[j] != NULL) {
	if (options.uamserverlen>=UAMSERVER_MAX) {
	  sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		  "Too many IPs in uamhomepage %s!",
		  args_info.uamhomepage_arg);
	  return -1;
	}
	else {
	  options.uamserver[options.uamserverlen++] = 
	    *((struct in_addr*) host->h_addr_list[j++]);
	}
      }
    }
    options.uamhomepage = args_info.uamhomepage_arg;
  }

  /* uamsecret                                                    */
  options.uamsecret = args_info.uamsecret_arg;


  /* uamlisten                                                    */
  /* Defaults to net plus 1                                       */
  if (!args_info.uamlisten_arg) {
    options.uamlisten.s_addr = htonl(ntohl(options.net.s_addr)+1);
  }
  else if (!inet_aton(args_info.uamlisten_arg, &options.uamlisten)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Invalid UAM IP address: %s!", args_info.uamlisten_arg);
    return -1;
  }


  /* uamport                                                      */
  options.uamport = args_info.uamport_arg;


  /* uamallowed                                                   */
  memset(options.uamokip, 0, sizeof(options.uamokip));
  options.uamokiplen = 0;
  memset(options.uamokaddr, 0, sizeof(options.uamokaddr));
  memset(options.uamokmask, 0, sizeof(options.uamokmask));
  options.uamoknetlen = 0;
  for (numargs = 0; numargs < args_info.uamallowed_given; ++numargs) {
    if (options.debug & DEBUG_CONF) {
      printf ("Uamallowed #%d: %s\n", 
	      numargs, args_info.uamallowed_arg[numargs]);
    }
    if (set_uamallowed(args_info.uamallowed_arg[numargs],
		       strlen(args_info.uamallowed_arg[numargs])))
      return -1;
  }

  /* uamanydns                                                    */
  options.uamanydns = args_info.uamanydns_flag;


  /* dynip                                                        */
  options.allowdyn = 1;
  if (!args_info.dynip_arg) {
    options.dynip = args_info.net_arg;
  }
  else {
    struct in_addr addr;
    struct in_addr mask;
    options.dynip = args_info.dynip_arg;
    if (ippool_aton(&addr, &mask, options.dynip, 0)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to parse dynamic IP address pool!");
      return -1;
    }
  }

  /* statip                                                        */
  if (args_info.statip_arg) {
    struct in_addr addr;
    struct in_addr mask;
    options.statip = args_info.statip_arg;
    if (ippool_aton(&addr, &mask, options.statip, 0)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to parse static IP address pool!");
      return -1;
    }
    options.allowstat = 1;
  }
  else {
    options.allowstat = 0;
  }


  /* dns1                                                         */
  /* Store dns1 as in_addr                                        */
  /* If DNS not given use system default                          */
  if (args_info.dns1_arg) {
    if (!inet_aton(args_info.dns1_arg, &options.dns1)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Invalid primary DNS address: %s!", 
	      args_info.dns1_arg);
      return -1;
    }
  }
  else if (_res.nscount >= 1) {
    options.dns1 = _res.nsaddr_list[0].sin_addr;
  }
  else {
    options.dns1.s_addr = 0;
  }

  /* dns2                                                         */
  /* Store dns2 as in_addr                                        */
  /* If DNS not given use system default else use DNS1            */
  if (args_info.dns2_arg) {
    if (!inet_aton(args_info.dns2_arg, &options.dns2)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Invalid secondary DNS address: %s!", 
	      args_info.dns1_arg);
      return -1;
    }
  }
  else if (_res.nscount >= 2) {
    options.dns2 = _res.nsaddr_list[1].sin_addr;
  }
  else {
    options.dns2.s_addr = options.dns1.s_addr;
  }

  /* Domain                                                       */
  options.domain = args_info.domain_arg;


  /* ipup */
  options.ipup = args_info.ipup_arg;

  /* ipdown */
  options.ipdown = args_info.ipdown_arg;
  
  /* conup */
  options.conup = args_info.conup_arg;

  /* condown */
  options.condown = args_info.condown_arg;


  /* radiuslisten                                                 */
  /* If no listen option is specified listen to any local port    */
  /* Do hostname lookup to translate hostname to IP address       */
  if (args_info.radiuslisten_arg) {
    if (!(host = gethostbyname(args_info.radiuslisten_arg))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	      "Invalid listening address: %s!", 
	      args_info.radiuslisten_arg);
      return -1;
    }
    else {
      memcpy(&options.radiuslisten.s_addr, host->h_addr, host->h_length);
    }
  }
  else {
    options.radiuslisten.s_addr = htonl(INADDR_ANY);
  }


  /* radiusserver1 */
  /* If no option is specified terminate                          */
  /* Do hostname lookup to translate hostname to IP address       */
  if (args_info.radiusserver1_arg) {
    if (!(host = gethostbyname(args_info.radiusserver1_arg))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Invalid radiusserver1 address: %s!", 
	      args_info.radiusserver1_arg);
      return -1;
    }
    else {
      memcpy(&options.radiusserver1.s_addr, host->h_addr, host->h_length);
    }
  }
  else {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No radiusserver1 address given!");
    return -1;
  }

  /* radiusserver2 */
  /* If no option is specified terminate                          */
  /* Do hostname lookup to translate hostname to IP address       */
  if (args_info.radiusserver2_arg) {
    if (!(host = gethostbyname(args_info.radiusserver2_arg))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Invalid radiusserver2 address: %s!", 
	      args_info.radiusserver2_arg);
      return -1;
    }
    else {
      memcpy(&options.radiusserver2.s_addr, host->h_addr, host->h_length);
    }
  }
  else {
    options.radiusserver2.s_addr = 0;
  }

  /* radiusauthport */
  options.radiusauthport = args_info.radiusauthport_arg;

  /* radiusacctport */
  options.radiusacctport = args_info.radiusacctport_arg;

  /* radiussecret */
  if (!args_info.radiussecret_arg) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	    "radiussecret must be specified!");
    return -1;
  }
  options.radiussecret = args_info.radiussecret_arg;


  /* radiusnasid */
  options.radiusnasid = args_info.radiusnasid_arg;

  /* radiusnasip                                                  */
  /* If not specified default to radiuslisten                     */
  /* Do hostname lookup to translate hostname to IP address       */
  if (args_info.radiusnasip_arg) {
    if (!(host = gethostbyname(args_info.radiusnasip_arg))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	      "Invalid nasip address: %s!", 
	      args_info.radiusnasip_arg);
      return -1;
    }
    else {
      memcpy(&options.radiusnasip.s_addr, host->h_addr, host->h_length);
    }
  }
  else {
    options.radiusnasip.s_addr = options.radiuslisten.s_addr;
  }

  /* radiuscalled (Called-Station-ID)                             */
  /* If not specified default to dhcpmac                          */
  /* If no dhcpmac default to real mac address                    */
  if (args_info.radiuscalled_arg) {
    options.radiuscalled = args_info.radiuscalled_arg;
  }
  else if (options.dhcpusemac == 1) {
    options.radiuscalled = malloc(MACSTRLEN+1);
    (void) snprintf(options.radiuscalled, MACSTRLEN+1,
		    "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
		    options.dhcpmac[0], options.dhcpmac[1],
		    options.dhcpmac[2], options.dhcpmac[3],
		    options.dhcpmac[4], options.dhcpmac[5]);
  }
  else if (options.dhcpif) {
    unsigned char macaddr[DHCP_ETH_ALEN];
    (void) dhcp_getmac(options.dhcpif, (char*) macaddr);
    options.radiuscalled = malloc(MACSTRLEN+1);
    (void) snprintf(options.radiuscalled, MACSTRLEN+1,
		    "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
		    macaddr[0], macaddr[1],
		    macaddr[2], macaddr[3],
		    macaddr[4], macaddr[5]);
  }
  else {
    options.radiuscalled = NULL;
  }

  /* radiuslocationid */
  options.radiuslocationid = args_info.radiuslocationid_arg;

  /* radiuslocationname */
  options.radiuslocationname = args_info.radiuslocationname_arg;

  /* radiusnasporttype */
  options.radiusnasporttype = args_info.radiusnasporttype_arg;

  /* coaport */
  options.coaport = args_info.coaport_arg;

  /* coanoipcheck                                                */
  options.coanoipcheck = args_info.coanoipcheck_flag;


  /* proxylisten                                                  */
  /* If no listen option is specified listen to any local port    */
  /* Do hostname lookup to translate hostname to IP address       */
  if (args_info.proxylisten_arg) {
    if (!(host = gethostbyname(args_info.proxylisten_arg))) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, 
	      "Invalid listening address: %s!", 
	      args_info.proxylisten_arg);
      return -1;
    }
    else {
      memcpy(&options.proxylisten.s_addr, host->h_addr, host->h_length);
    }
  }
  else {
    options.proxylisten.s_addr = htonl(INADDR_ANY);
  }

  /* proxyport                                                   */
  options.proxyport = args_info.proxyport_arg;


  /* proxyclient */
  /* Store proxyclient as in_addr net and mask                       */
  if (args_info.proxyclient_arg) {
    if(ippool_aton(&options.proxyaddr, &options.proxymask, 
		   args_info.proxyclient_arg, 0)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Invalid proxy client address: %s!", args_info.proxyclient_arg);
      return -1;
    }
  }
  else {
    options.proxyaddr.s_addr = ~0; /* Let nobody through */
    options.proxymask.s_addr = 0; 
  }

  /* proxysecret */
  /* If omitted default to radiussecret */
  if (!args_info.proxysecret_arg) {
  options.proxysecret = args_info.radiussecret_arg;
  }
  else {
    options.proxysecret = args_info.proxysecret_arg;
  }

  options.macauth = args_info.macauth_flag;
  options.macsuffix = args_info.macsuffix_arg;
  options.macpasswd = args_info.macpasswd_arg;


  /* Radius remote configuration management */
  options.confusername = args_info.confusername_arg;
  options.confpassword = args_info.confpassword_arg;

  /* macallowed                                                   */
  memset(options.macok, 0, sizeof(options.macok));
  options.macoklen = 0;
  for (numargs = 0; numargs < args_info.macallowed_given; ++numargs) {
    if (options.debug & DEBUG_CONF) {
      printf ("Macallowed #%d: %s\n", numargs, 
	      args_info.macallowed_arg[numargs]);
    }

    if (set_macallowed(args_info.macallowed_arg[numargs],
		       strlen(args_info.macallowed_arg[numargs]))) 
      return -1;
  }


  /* foreground                                                   */
  /* If flag not given run as a daemon                            */
  if ((!args_info.fg_flag) && (firsttime))
    {
      closelog(); 
      /* Close the standard file descriptors. */
      /* Is this really needed ? */
      (void) freopen("/dev/null", "w", stdout);
      (void) freopen("/dev/null", "w", stderr);
      (void) freopen("/dev/null", "r", stdin);
      if (daemon(1, 1)) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno,
		"daemon() failed!");
      }
      
      /* Open log again. This time with new pid */
      openlog(PACKAGE, LOG_PID, LOG_DAEMON);
    }

	/* pidfile */
  options.pidfile = args_info.pidfile_arg;

  return 0;
}

void static reprocess_options(int argc, char **argv) {
  struct options_t options2;
  sys_err(LOG_INFO, __FILE__, __LINE__, 0,
	  "Rereading configuration file and doing DNS lookup");

  memcpy(&options2, &options, sizeof(options)); /* Save original */
  if (process_options(argc, argv, 0)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Error reading configuration file!");
    memcpy(&options, &options2, sizeof(options));
    return;
  }

  /* Options which we do not allow to be affected */
  /* fg, conf and statedir are not stored in options */
  options.net = options2.net; /* net */
  options.mask = options2.mask; /* net */
  options.dhcplisten = options2.dhcplisten; /* net */
  options.dynip = options2.dynip; /* dynip */
  options.allowdyn = options2.allowdyn; /* dynip */
  options.statip = options2.statip; /* statip */
  options.allowstat = options2.allowstat; /* statip */
  options.uamlisten = options2.uamlisten; /* uamlisten */
  options.uamport = options2.uamport; /* uamport */
  options.radiuslisten = options2.radiuslisten; /* radiuslisten */
  options.coaport = options.coaport; /* coaport */
  options.coanoipcheck = options.coanoipcheck; /* coanoipcheck */
  options.proxylisten = options2.proxylisten; /* proxylisten */
  options.proxyport = options2.proxyport; /* proxyport */
  options.proxyaddr = options2.proxyaddr; /* proxyclient */
  options.proxymask = options2.proxymask; /* proxyclient */
  options.proxysecret = options2.proxysecret; /*proxysecret */
  options.nodhcp = options2.nodhcp; /* dhcpif */
  options.dhcpif = options2.dhcpif; /* dhcpif */
  memcpy(options.dhcpmac, options2.dhcpmac, DHCP_ETH_ALEN); /* dhcpmac*/
  options.dhcpusemac = options2.dhcpusemac; /* dhcpmac */
  options.lease = options2.lease; /* lease */
  options.eapolenable = options2.eapolenable; /* eapolenable */
  options.pidfile = options2.pidfile; /* pidfile */

  /* Reinit DHCP parameters */
  (void) dhcp_set(dhcp, (options.debug & DEBUG_DHCP),
		  options.uamserver, options.uamserverlen, options.uamanydns,
		  options.uamokip, options.uamokiplen,
		  options.uamokaddr, options.uamokmask, options.uamoknetlen);
  
  /* Reinit RADIUS parameters */
  (void) radius_set(radius, (options.debug & DEBUG_RADIUS),
		    &options.radiusserver1, &options.radiusserver2,
		    options.radiusauthport, options.radiusacctport,
		    options.radiussecret);
  
  /* Reinit Redir parameters */
  (void) redir_set(redir, (options.debug & DEBUG_REDIR),
		   options.uamurl, options.uamhomepage, options.uamsecret,
		   &options.radiuslisten,
		   &options.radiusserver1, &options.radiusserver2,
		   options.radiusauthport, options.radiusacctport,
		   options.radiussecret, options.radiusnasid,
		   &options.radiusnasip, options.radiuscalled,
		   options.radiuslocationid, options.radiuslocationname,
		   options.radiusnasporttype);

  (void) config_radius();
}

/* 
 * A few functions to manage connections 
 */

int static initconn()
{
  int n;
  firstusedconn = NULL; /* Redundant */
  lastusedconn  = NULL; /* Redundant */

  gettimeofday(&checktime, NULL);
  gettimeofday(&rereadtime, NULL);

  
  for (n=0; n<APP_NUM_CONN; n++) {
    connection[n].inuse = 0; /* Redundant */
    if (n == 0) {
      connection[n].prev = NULL; /* Redundant */
      firstfreeconn = &connection[n];

    }
    else {
      connection[n].prev = &connection[n-1];
      connection[n-1].next = &connection[n];
    }
    if (n == (APP_NUM_CONN-1)) {
      connection[n].next = NULL; /* Redundant */
      lastfreeconn  = &connection[n];
    }
  }

  return 0;
}

int static newconn(struct app_conn_t **conn)
{

  if (!firstfreeconn) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Out of free connection");
    return -1;
  }

  *conn = firstfreeconn;

  /* Remove from link of free */
  if (firstfreeconn->next) {
    firstfreeconn->next->prev = NULL;
    firstfreeconn = firstfreeconn->next;
  }
  else { /* Took the last one */
    firstfreeconn = NULL; 
    lastfreeconn = NULL;
  }

  /* Initialise structures */
  memset(*conn, 0, sizeof(**conn));

  /* Insert into link of used */
  if (firstusedconn) {
    firstusedconn->prev = *conn;
    (*conn)->next = firstusedconn;
  }
  else { /* First insert */
    lastusedconn = *conn;
  }

  firstusedconn = *conn;

  (*conn)->inuse = 1;
  (*conn)->unit = (*conn) - connection;

  return 0; /* Success */
}

int static freeconn(struct app_conn_t *conn)
{
  /* Remove from link of used */
  if ((conn->next) && (conn->prev)) {
    conn->next->prev = conn->prev;
    conn->prev->next = conn->next;
  }
  else if (conn->next) { /* && prev == 0 */
    conn->next->prev = NULL;
    firstusedconn = conn->next;
  }
  else if (conn->prev) { /* && next == 0 */
    conn->prev->next = NULL;
    lastusedconn = conn->prev;
  }
  else { /* if ((next == 0) && (prev == 0)) */
    firstusedconn = NULL;
    lastusedconn = NULL;
  }
  
  /* Initialise structures */
  memset(conn, 0, sizeof(*conn));
  
  /* Insert into link of free */
  if (firstfreeconn) {
    firstfreeconn->prev = conn;
  }
  else { /* First insert */
    lastfreeconn = conn;
  }

  conn->next = firstfreeconn;
  firstfreeconn = conn;

  return 0;
}

int static getconn(struct app_conn_t **conn, uint32_t nasip, uint32_t nasport) 
{
  struct app_conn_t *appconn;
  
  /* Count the number of used connections */
  appconn = firstusedconn;
  while (appconn) {
    if (!appconn->inuse) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Connection with inuse == 0!");
    }
    if ((appconn->nasip == nasip) && (appconn->nasport == nasport)) {
      *conn = appconn;
      return 0;
    }
    appconn = appconn->next;
  }
  return -1; /* Not found */
}

int static getconn_username(struct app_conn_t **conn, char *username, 
		     int usernamelen)
{
  struct app_conn_t *appconn;
  username[usernamelen] = 0; printf("username: %s\n", username);
  
  appconn = firstusedconn;
  while (appconn) {
    if (!appconn->inuse) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Connection with inuse == 0!");
    }
    appconn->user[appconn->userlen] = 0; printf("user: %s\n", appconn->user);

    if ((appconn->authenticated) && (appconn->userlen == usernamelen) &&
	!memcmp(appconn->user, username, usernamelen)) {
      *conn = appconn;
      printf("Found\n");
      return 0;
    }
    appconn = appconn->next;
  }
  return -1; /* Not found */
}

int static dnprot_terminate(struct app_conn_t *appconn) {
  appconn->authenticated = 0;
  switch (appconn->dnprot) {
  case DNPROT_WPA:
  case DNPROT_EAPOL:
    if (!appconn->dnlink) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }
    ((struct dhcp_conn_t*) appconn->dnlink)->authstate = DHCP_AUTH_NONE;
    return 0;
  case DNPROT_MAC:
  case DNPROT_UAM:
    if (!appconn->dnlink) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }
    ((struct dhcp_conn_t*) appconn->dnlink)->authstate = DHCP_AUTH_DNAT;
    return 0;
  case DNPROT_DHCP_NONE:
    return 0;
  default: 
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Unknown downlink protocol"); 
    return 0;
  }
}



/* Check for:
 * - Session-Timeout
 * - Idle-Timeout
 * - Interim-Interim accounting
 * - Reread configuration file and DNS entries
 */

int static checkconn()
{
  int n;
  struct app_conn_t *conn;
  struct dhcp_conn_t* dhcpconn;
  struct timeval timenow;
  uint32_t sessiontime;
  uint32_t idletime;
  uint32_t interimtime;
  uint32_t checkdiff;
  uint32_t rereaddiff;

  gettimeofday(&timenow, NULL);

  checkdiff = timenow.tv_sec - checktime.tv_sec;
  checkdiff += (timenow.tv_usec - checktime.tv_usec) / 1000000;

  if (checkdiff < CHECK_INTERVAL)
    return 0;

  checktime = timenow;
  
  for (n=0; n<APP_NUM_CONN; n++) {
    conn = &connection[n];
    if ((conn->inuse != 0) && (conn->authenticated == 1)) {
      if (!(dhcpconn = (struct dhcp_conn_t*) conn->dnlink)) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
	return -1;
      }
      sessiontime = timenow.tv_sec - conn->start_time.tv_sec;
      sessiontime += (timenow.tv_usec - conn->start_time.tv_usec) / 1000000;
      idletime = timenow.tv_sec - conn->last_time.tv_sec;
      idletime += (timenow.tv_usec - conn->last_time.tv_usec) / 1000000;
      interimtime = timenow.tv_sec - conn->interim_time.tv_sec;
      interimtime += (timenow.tv_usec - conn->interim_time.tv_usec) / 1000000;

      if ((conn->sessiontimeout) &&
	  (sessiontime > conn->sessiontimeout)) {
	dnprot_terminate(conn);
	conn->terminate_cause = RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT;
	(void) acct_req(conn, RADIUS_STATUS_TYPE_STOP);
	set_sessionid(conn);
      }
      else if ((conn->sessionterminatetime) && 
	       (timenow.tv_sec > conn->sessionterminatetime)) {
	dnprot_terminate(conn);
	conn->terminate_cause = RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT;
	(void) acct_req(conn, RADIUS_STATUS_TYPE_STOP);
	set_sessionid(conn);
      }
      else if ((conn->idletimeout) && 
	       (idletime > conn->idletimeout)) {
	dnprot_terminate(conn);
	conn->terminate_cause = RADIUS_TERMINATE_CAUSE_IDLE_TIMEOUT;
	(void) acct_req(conn, RADIUS_STATUS_TYPE_STOP);
	set_sessionid(conn);
      }
      else if ((conn->maxinputoctets) &&
	       (conn->input_octets > conn->maxinputoctets)) {
	dnprot_terminate(conn);
	conn->terminate_cause = RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT;
	(void) acct_req(conn, RADIUS_STATUS_TYPE_STOP);
	set_sessionid(conn);
      }
      else if ((conn->maxoutputoctets) &&
	       (conn->output_octets > conn->maxoutputoctets)) {
	dnprot_terminate(conn);
	conn->terminate_cause = RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT;
	(void) acct_req(conn, RADIUS_STATUS_TYPE_STOP);
	set_sessionid(conn);
      }
      else if ((conn->maxtotaloctets) &&
	       ((conn->input_octets + conn->output_octets) > 
		conn->maxtotaloctets)) {
	dnprot_terminate(conn);
	conn->terminate_cause = RADIUS_TERMINATE_CAUSE_SESSION_TIMEOUT;
	(void) acct_req(conn, RADIUS_STATUS_TYPE_STOP);
	set_sessionid(conn);
      }
      else if ((conn->interim_interval) &&
	       (interimtime > conn->interim_interval)) {
	(void) acct_req(conn, RADIUS_STATUS_TYPE_INTERIM_UPDATE);
      }
    }
  }
  
  /* Reread configuration file and recheck DNS */
  if (options.interval) {
    rereaddiff = timenow.tv_sec - rereadtime.tv_sec;
    rereaddiff += (timenow.tv_usec - rereadtime.tv_usec) / 1000000;
    if (rereaddiff >= options.interval) {
      rereadtime = timenow;
      do_sighup = 1;
    }
  }
  
  return 0;
}

/* Kill all connections and send Radius Acct Stop */
int static killconn()
{
  int n;
  struct app_conn_t *conn;
  struct dhcp_conn_t* dhcpconn;

  for (n=0; n<APP_NUM_CONN; n++) {
    conn = &connection[n];
    if ((conn->inuse != 0) && (conn->authenticated == 1)) {
      if (!(dhcpconn = (struct dhcp_conn_t*) conn->dnlink)) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
	return -1;
      }
      dnprot_terminate(conn);
      conn->terminate_cause = RADIUS_TERMINATE_CAUSE_NAS_REBOOT;
      (void) acct_req(conn, RADIUS_STATUS_TYPE_STOP);
      set_sessionid(conn);
    }
  }
  return 0;
}

/* Compare a MAC address to the addresses given in the macallowed option */
int static maccmp(unsigned char *mac) {
  int i;
  for (i=0; i<options.macoklen; i++) {
    if (!memcmp(mac, options.macok[i], DHCP_ETH_ALEN)) {
      return 0;
    }
  }
  return -1;
}

int static macauth_radius(struct app_conn_t *appconn) {
  struct radius_packet_t radius_pack;
  struct dhcp_conn_t* dhcpconn = (struct dhcp_conn_t*) appconn->dnlink;
  char mac[MACSTRLEN+1];

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }
  
  /* Include his MAC address */
  (void) snprintf(mac, MACSTRLEN+1, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	   dhcpconn->hismac[0], dhcpconn->hismac[1],
	   dhcpconn->hismac[2], dhcpconn->hismac[3],
	   dhcpconn->hismac[4], dhcpconn->hismac[5]);

  strncpy(appconn->proxyuser, mac, USERNAMESIZE);
  appconn->proxyuser[USERNAMESIZE-1] = 0;
  if (options.macsuffix) {
    strncat(appconn->proxyuser, options.macsuffix, USERNAMESIZE);
    appconn->proxyuser[USERNAMESIZE-1] = 0;
  }
  appconn->proxyuserlen = strlen(appconn->proxyuser);

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
			(uint8_t*) appconn->proxyuser, appconn->proxyuserlen);

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
		 (uint8_t*) options.macpasswd, strlen(options.macpasswd));

  appconn->authtype = PAP_PASSWORD;

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0,
		 (uint8_t*) mac, MACSTRLEN);
  
  /* Include our MAC address */
  if (options.radiuscalled)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLED_STATION_ID, 0, 0, 0,
			(uint8_t*) options.radiuscalled, strlen(options.radiuscalled));
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT, 0, 0,
		 appconn->unit, NULL, 0);
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
		 ntohl(options.radiusnasip.s_addr), NULL, 0);
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		 RADIUS_SERVICE_TYPE_LOGIN, NULL, 0); /* WISPr_V1.0 */
  
  /* Include NAS-Identifier if given in configuration options */
  if (options.radiusnasid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
		   (uint8_t*) options.radiusnasid, strlen(options.radiusnasid));

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
		 (uint8_t*) appconn->sessionid, REDIR_SESSIONID_LEN-1);

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
			options.radiusnasporttype, NULL, 0);

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  if (options.radiuslocationid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_ID, 0,
		   (uint8_t*) options.radiuslocationid, 
		   strlen(options.radiuslocationid));

  if (options.radiuslocationname)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_NAME, 0,
		   (uint8_t*) options.radiuslocationname, 
		   strlen(options.radiuslocationname));
  
  return radius_req(radius, &radius_pack, appconn);
}


int static config_radius() {
  struct radius_packet_t radius_pack;

  if (!options.confusername || !options.confpassword) return 0;

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
			(uint8_t*) options.confusername, strlen(options.confusername));

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
		 (uint8_t*) options.confpassword, strlen(options.confpassword));

  if (options.radiuscalled)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLED_STATION_ID, 0, 0, 0,
			  (uint8_t*) options.radiuscalled, strlen(options.radiuscalled));
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
		 ntohl(options.radiusnasip.s_addr), NULL, 0);
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_SERVICE_TYPE, 0, 0,
		 RADIUS_SERVICE_TYPE_CHILLISPOT_AUTHORIZE_ONLY, NULL, 0);
  
  /* Include NAS-Identifier if given in configuration options */
  if (options.radiusnasid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
		   (uint8_t*) options.radiusnasid, strlen(options.radiusnasid));

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		 0, 0, 0, NULL, RADIUS_MD5LEN);

  if (options.radiuslocationid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_ID, 0,
		   (uint8_t*) options.radiuslocationid, 
		   strlen(options.radiuslocationid));

  if (options.radiuslocationname)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_NAME, 0,
		   (uint8_t*) options.radiuslocationname, 
		   strlen(options.radiuslocationname));
  
  return radius_req(radius, &radius_pack, NULL);
}


/*********************************************************
 *
 * radius proxy functions
 * Used to send a response to a received radius request
 *
 *********************************************************/

/* Reply with an access reject */
int static radius_access_reject(struct app_conn_t *conn) {
  struct radius_packet_t radius_pack;
  conn->radiuswait = 0;
  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REJECT)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }

  radius_pack.id = conn->radiusid;
  (void) radius_resp(radius, &radius_pack, &conn->radiuspeer, conn->authenticator);
  return 0;
}

/* Reply with an access challenge */
int static radius_access_challenge(struct app_conn_t *conn) {
  struct radius_packet_t radius_pack;
  int offset = 0;
  int eaplen = 0;
  conn->radiuswait = 0;
  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_CHALLENGE)){
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }
  radius_pack.id = conn->radiusid;

  /* Include EAP */
  do {
    if ((conn->challen - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = conn->challen - offset;
    if (radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		       conn->chal + offset, eaplen)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "radius_default_pack() failed");
      return -1;
    }
    offset += eaplen;
  } while (offset < conn->challen);
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		 0, 0, 0, NULL, RADIUS_MD5LEN);
  
  if (conn->statelen) {
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_STATE, 0, 0, 0,
		   conn->statebuf,
		   conn->statelen);
  }
  
  (void) radius_resp(radius, &radius_pack, &conn->radiuspeer, conn->authenticator);

  return 0;
}

/* Send off an access accept */

int static radius_access_accept(struct app_conn_t *conn) {
  struct radius_packet_t radius_pack;
  int offset = 0;
  int eaplen = 0;
  uint8_t mppekey[RADIUS_ATTR_VLEN];
  int mppelen;

  conn->radiuswait = 0;
  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_ACCEPT)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }
  radius_pack.id = conn->radiusid;


  /* Include EAP (if present) */
  offset = 0;
  while (offset < conn->challen) {
    if ((conn->challen - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = conn->challen - offset;
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		   conn->chal + offset, eaplen);
    offset += eaplen;
  }

  /* Message Authenticator */
  if (conn->challen)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		   0, 0, 0, NULL, RADIUS_MD5LEN);

  if (conn->sendkey) {
    radius_keyencode(radius, mppekey, RADIUS_ATTR_VLEN,
		     &mppelen, conn->sendkey,
		     conn->sendlen, conn->authenticator,
		     radius->proxysecret, radius->proxysecretlen);

    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_MS, RADIUS_ATTR_MS_MPPE_SEND_KEY, 0,
		   mppekey, mppelen);
  }

  if (conn->recvkey) {
    radius_keyencode(radius, mppekey, RADIUS_ATTR_VLEN,
		     &mppelen, conn->recvkey,
		     conn->recvlen, conn->authenticator,
		     radius->proxysecret, radius->proxysecretlen);
    
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_MS, RADIUS_ATTR_MS_MPPE_RECV_KEY, 0,
		   mppekey, mppelen);
  }
  
  (void) radius_resp(radius, &radius_pack, &conn->radiuspeer, conn->authenticator);
  return 0;
}


/*********************************************************
 *
 * radius accounting functions
 * Used to send accounting request to radius server
 *
 *********************************************************/

int static acct_req(struct app_conn_t *conn, int status_type)
{
  struct radius_packet_t radius_pack;
  char mac[MACSTRLEN+1];
  char portid[16+1];
  struct timeval timenow;
  uint32_t timediff;

  if (RADIUS_STATUS_TYPE_START == status_type) {
    gettimeofday(&conn->start_time, NULL);
    conn->interim_time = conn->start_time;
    conn->last_time = conn->start_time;
    conn->input_packets = 0;
    conn->output_packets = 0;
    conn->input_octets = 0;
    conn->output_octets = 0;
  }

  if (RADIUS_STATUS_TYPE_INTERIM_UPDATE == status_type) {
    gettimeofday(&conn->interim_time, NULL);
  }

  if (radius_default_pack(radius, &radius_pack, 
			  RADIUS_CODE_ACCOUNTING_REQUEST)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0,
		 status_type, NULL, 0);

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 (uint8_t*) conn->user, conn->userlen);

  if (conn->classlen) {
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CLASS, 0, 0, 0,
		   conn->classbuf,
		   conn->classlen);
  }

  (void) snprintf(mac, MACSTRLEN+1, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	   conn->hismac[0], conn->hismac[1],
	   conn->hismac[2], conn->hismac[3],
	   conn->hismac[4], conn->hismac[5]);

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0,
		 (uint8_t*) mac, MACSTRLEN);

  (void) snprintf(mac, MACSTRLEN+1, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	   conn->ourmac[0], conn->ourmac[1],
	   conn->ourmac[2], conn->ourmac[3],
	   conn->ourmac[4], conn->ourmac[5]);

  if (options.radiuscalled)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLED_STATION_ID, 0, 0, 0,
			  (uint8_t*) options.radiuscalled, strlen(options.radiuscalled));

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
			options.radiusnasporttype, NULL, 0);

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT, 0, 0,
		 conn->unit, NULL, 0);

  (void) snprintf(portid, 16+1, "%.8d", conn->unit);
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT_ID, 0, 0, 0,
		 (uint8_t*) portid, strlen(portid));
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
		 ntohl(options.radiusnasip.s_addr), NULL, 0);

  /* Include NAS-Identifier if given in configuration options */
  if (options.radiusnasid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
		   (uint8_t*) options.radiusnasid, 
		   strlen(options.radiusnasid));

  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0,
		 ntohl(conn->hisip.s_addr), NULL, 0);

  /*
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_FRAMED_MTU, 0, 0,
  conn->mtu, NULL, 0);*/


  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_SESSION_ID, 0, 0, 0,
		 (uint8_t*) conn->sessionid, REDIR_SESSIONID_LEN-1);

  if ((status_type == RADIUS_STATUS_TYPE_STOP) ||
      (status_type == RADIUS_STATUS_TYPE_INTERIM_UPDATE)) {

    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_INPUT_OCTETS, 0, 0,
		   (uint32_t) conn->input_octets, NULL, 0);
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_OUTPUT_OCTETS, 0, 0,
		   (uint32_t) conn->output_octets, NULL, 0);
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_INPUT_GIGAWORDS, 
		   0, 0, (uint32_t) (conn->input_octets >> 32), NULL, 0);
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_OUTPUT_GIGAWORDS, 
		   0, 0, (uint32_t) (conn->output_octets >> 32), NULL, 0);
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_INPUT_PACKETS, 0, 0,
		   conn->input_packets, NULL, 0);
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_OUTPUT_PACKETS, 0, 0,
		   conn->output_packets, NULL, 0);

    gettimeofday(&timenow, NULL);
    timediff = timenow.tv_sec - conn->start_time.tv_sec;
    timediff += (timenow.tv_usec - conn->start_time.tv_usec) / 1000000;
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_SESSION_TIME, 0, 0,
		   timediff, NULL, 0);  
  }

  if (options.radiuslocationid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_ID, 0,
		   (uint8_t*) options.radiuslocationid,
		   strlen(options.radiuslocationid));

  if (options.radiuslocationname)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_VENDOR_SPECIFIC,
		   RADIUS_VENDOR_WISPR, RADIUS_ATTR_WISPR_LOCATION_NAME, 0,
		   (uint8_t*) options.radiuslocationname, 
		   strlen(options.radiuslocationname));


  if (status_type == RADIUS_STATUS_TYPE_STOP) {
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_ACCT_TERMINATE_CAUSE, 
		   0, 0, conn->terminate_cause, NULL, 0);
		
		/* TODO: This probably belongs somewhere else */
    if (options.condown) {
      if (options.debug)
	printf("Calling connection down script: %s\n",options.condown);
      (void) runscript(conn, options.condown);
    }   
  }


  (void) radius_req(radius, &radius_pack, conn);
  
  return 0;
}



/***********************************************************
 *
 * Functions handling downlink protocol authentication.
 * Called in response to radius access request response.
 *
 ***********************************************************/

int static dnprot_reject(struct app_conn_t *appconn) {
  struct dhcp_conn_t* dhcpconn = NULL;
  struct ippoolm_t *ipm;

  switch (appconn->dnprot) {
  case DNPROT_EAPOL:
    if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }
    (void) dhcp_sendEAPreject(dhcpconn, NULL, 0);
    return 0;
  case DNPROT_UAM:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Rejecting UAM");
    return 0;
  case DNPROT_WPA:
    return radius_access_reject(appconn);
  case DNPROT_MAC:
    if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }
    
    /* Allocate dynamic IP address */
    if (ippool_newip(ippool, &ipm, &appconn->reqip, 0)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed allocate dynamic IP address");
      return 0;
    }
    appconn->hisip.s_addr = ipm->addr.s_addr;
    
    /* TODO: Listening address is network address plus 1 */
    appconn->ourip.s_addr = htonl((ntohl(options.net.s_addr)+1));
    
    appconn->uplink =  ipm;
    ipm->peer = appconn;
    
    (void) dhcp_set_addrs(dhcpconn, &ipm->addr, &options.mask, &appconn->ourip,
			  &options.dns1, &options.dns2, options.domain);
    
    dhcpconn->authstate = DHCP_AUTH_DNAT;
    appconn->dnprot = DNPROT_UAM;
    
    return 0;    
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Unknown downlink protocol");
    return 0;
  }
}

int static dnprot_challenge(struct app_conn_t *appconn) {
  struct dhcp_conn_t* dhcpconn = NULL;

  switch (appconn->dnprot) {
  case DNPROT_EAPOL:
    if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }
    (void) dhcp_sendEAP(dhcpconn, appconn->chal, appconn->challen);
    break;
  case DNPROT_UAM:
  case DNPROT_WPA:
    radius_access_challenge(appconn);
    break;
  case DNPROT_MAC:
    break;
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Unknown downlink protocol");
  }

  return 0;
}

int static dnprot_accept(struct app_conn_t *appconn) {
  struct dhcp_conn_t* dhcpconn = NULL;

  if (!appconn->hisip.s_addr) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "IP address not allocated");
    return 0;
  }

  switch (appconn->dnprot) {
  case DNPROT_EAPOL:
    if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }

    (void) dhcp_set_addrs(dhcpconn, &appconn->hisip, &appconn->mask,
			  &appconn->ourip, &appconn->dns1, &appconn->dns2,
			  options.domain);
    
    /* This is the one and only place eapol authentication is accepted */
    dhcpconn->authstate = DHCP_AUTH_PASS;

    /* Initialise parameters for accounting */
    appconn->userlen = appconn->proxyuserlen; 
    memcpy(appconn->user, appconn->proxyuser, appconn->userlen);
    appconn->nasip = appconn->proxynasip; 
    appconn->nasport = appconn->proxynasport; 
    memcpy(appconn->hismac, appconn->proxyhismac, DHCP_ETH_ALEN);
    memcpy(appconn->ourmac, appconn->proxyourmac, DHCP_ETH_ALEN);

    /* Tell client it was successful */
    (void) dhcp_sendEAP(dhcpconn, appconn->chal, appconn->challen);

    sys_err(LOG_WARNING, __FILE__, __LINE__, 0, 
	    "Do not know how to set encryption keys on this platform!");
    break;

  case DNPROT_UAM:
    if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }

    (void) dhcp_set_addrs(dhcpconn, &appconn->hisip, &appconn->mask, 
			  &appconn->ourip, &appconn->dns1, &appconn->dns2,
			  options.domain);
    
    /* This is the one and only place UAM authentication is accepted */
    dhcpconn->authstate = DHCP_AUTH_PASS;

    /* Initialise parameters for accounting */
    appconn->userlen = appconn->proxyuserlen; 
    memcpy(appconn->user, appconn->proxyuser, appconn->userlen);
    appconn->nasip = appconn->proxynasip; 
    appconn->nasport = appconn->proxynasport; 
    memcpy(appconn->hismac, appconn->proxyhismac, DHCP_ETH_ALEN);
    memcpy(appconn->ourmac, appconn->proxyourmac, DHCP_ETH_ALEN);
    break;

  case DNPROT_WPA:
    if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }

    (void) dhcp_set_addrs(dhcpconn, &appconn->hisip, &appconn->mask, 
			  &appconn->ourip, &appconn->dns1, &appconn->dns2,
			  options.domain);
    
    /* This is the one and only place WPA authentication is accepted */
    dhcpconn->authstate = DHCP_AUTH_PASS;

    /* Initialise parameters for accounting */
    appconn->userlen = appconn->proxyuserlen; 
    memcpy(appconn->user, appconn->proxyuser, appconn->userlen);
    appconn->nasip = appconn->proxynasip; 
    appconn->nasport = appconn->proxynasport; 
    memcpy(appconn->hismac, appconn->proxyhismac, DHCP_ETH_ALEN);
    memcpy(appconn->ourmac, appconn->proxyourmac, DHCP_ETH_ALEN);

    /* Tell access point it was successful */
    radius_access_accept(appconn);

    break;

  case DNPROT_MAC:
    if (!(dhcpconn = (struct dhcp_conn_t*) appconn->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0, "No downlink protocol");
      return 0;
    }
    
    (void) dhcp_set_addrs(dhcpconn, &appconn->hisip, &appconn->mask, 
			  &appconn->ourip, &appconn->dns1, &appconn->dns2,
			  options.domain);
    
    /* This is the one and only place MAC authentication is accepted */
    dhcpconn->authstate = DHCP_AUTH_PASS;
    
    /* Initialise parameters for accounting */
    appconn->userlen = appconn->proxyuserlen; 
    memcpy(appconn->user, appconn->proxyuser, appconn->userlen);
    appconn->nasip = appconn->proxynasip; 
    appconn->nasport = appconn->proxynasport; 
    memcpy(appconn->hismac, appconn->proxyhismac, DHCP_ETH_ALEN);
    memcpy(appconn->ourmac, appconn->proxyourmac, DHCP_ETH_ALEN);

    break;
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Unknown downlink protocol");
    return 0;
  }
  
  /* Run connection up script */
  if ((options.conup) && (!appconn->authenticated)) {
    if (options.debug) 
      printf("Calling connection up script: %s\n", options.conup);

    (void) runscript(appconn, options.conup);
  }

  /* This is the one and only place state is switched to authenticated */
  if (!appconn->authenticated) {
    appconn->authenticated = 1;
    (void) acct_req(appconn, RADIUS_STATUS_TYPE_START);
  }

  return 0;
}


/*********************************************************
 *
 * Tun callbacks
 *
 *********************************************************/


/* Callback for receiving messages from tun */
int cb_tun_ind(struct tun_t *tun, void *pack, unsigned len) {
  struct ippoolm_t *ipm;
  struct in_addr dst;
  struct tun_packet_t *iph = (struct tun_packet_t*) pack;
  struct app_conn_t *appconn;

  /*if (options.debug) 
    printf("cb_tun_ind. Packet received: Forwarding to link layer\n");*/
  
  dst.s_addr = iph->dst;

  if (ippool_getip(ippool, &ipm, &dst)) {
    if (options.debug) printf("Received packet with no destination!!!\n");
    return 0;
  }

  if (!((ipm->peer) || ((struct app_conn_t*) ipm->peer)->dnlink)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No peer protocol defined");
    return 0;
  }

  appconn = (struct app_conn_t*) ipm->peer;

  if (appconn->authenticated == 1) {
#ifndef NO_LEAKY_BUCKET
#ifndef COUNT_DOWNLINK_DROP
    if (leaky_bucket(appconn, 0, len)) return 0;
#endif /* ifndef COUNT_DOWNLINK_DROP */
#endif /* ifndef NO_LEAKY_BUCKET */
    appconn->output_packets++;
    appconn->output_octets += len;
#ifndef NO_LEAKY_BUCKET
#ifdef COUNT_DOWNLINK_DROP
    if (leaky_bucket(appconn, 0, len)) return 0;
#endif /* ifdef COUNT_DOWNLINK_DROP */
#endif /* ifndef NO_LEAKY_BUCKET */
  }

  switch (appconn->dnprot) {
  case DNPROT_UAM:
  case DNPROT_WPA:
  case DNPROT_MAC:
    (void) dhcp_data_req((struct dhcp_conn_t *) appconn->dnlink, pack, len);
    break;
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0, "Unknown downlink protocol: %d",
	    appconn->dnprot);
    break;
  }

  return 0;
 
}


/*********************************************************
 *
 * Redir callbacks
 *
 *********************************************************/

int cb_redir_getstate(struct redir_t *redir, struct in_addr *addr,
		      struct redir_conn_t *conn) {
  struct ippoolm_t *ipm;
  struct app_conn_t *appconn;
  struct dhcp_conn_t *dhcpconn;

  if (ippool_getip(ippool, &ipm, addr)) {
    return -1;
  }
  
  if (!((ipm->peer) || ((struct app_conn_t*) ipm->peer)->dnlink)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No peer protocol defined");
    return -1;
  }
  
  appconn = (struct app_conn_t*) ipm->peer;
  dhcpconn = (struct dhcp_conn_t*) appconn->dnlink;
  
  conn->authenticated = appconn->authenticated;
  memcpy(conn->uamchal, appconn->uamchal, REDIR_MD5LEN);
  conn->uamtime = appconn->uamtime;
  conn->nasip = options.radiuslisten;
  conn->nasport = appconn->unit;
  memcpy(conn->hismac, dhcpconn->hismac, DHCP_ETH_ALEN);
  memcpy(conn->ourmac, dhcpconn->ourmac, DHCP_ETH_ALEN);
  conn->ourip = appconn->ourip;
  conn->hisip = appconn->hisip;
  memcpy(conn->sessionid, appconn->sessionid, REDIR_SESSIONID_LEN);
  
  /*
  strncpy(conn->userurl, appconn->userurl, REDIR_MAXCHAR);
  conn->userurl[REDIR_MAXCHAR-1] = 0;
  */
  
  /* Stuff needed for status */
  conn->input_octets    = appconn->input_octets;
  conn->output_octets   = appconn->output_octets;
  conn->sessiontimeout  = appconn->sessiontimeout;
  conn->maxinputoctets  = appconn->maxinputoctets;
  conn->maxoutputoctets = appconn->maxoutputoctets;
  conn->maxtotaloctets  = appconn->maxtotaloctets;
  conn->start_time      = appconn->start_time;
  
  if (appconn->authenticated == 1)
    return 1;
  else 
    return 0;
}


/*********************************************************
 *
 * Functions supporting radius callbacks
 *
 *********************************************************/

/* Handle an accounting request */
int accounting_request(struct radius_packet_t *pack,
		       struct sockaddr_in *peer) {
  int n;
  struct radius_attr_t *hismacattr = NULL;
  struct radius_attr_t *typeattr = NULL;
  struct radius_attr_t *nasipattr = NULL;
  struct radius_attr_t *nasportattr = NULL;
  struct radius_packet_t radius_pack;
  struct app_conn_t *appconn = NULL;
  struct dhcp_conn_t *dhcpconn = NULL;
  uint8_t hismac[DHCP_ETH_ALEN];
  char macstr[RADIUS_ATTR_VLEN];
  int macstrlen;
  unsigned int temp[DHCP_ETH_ALEN];
  int	i;
  uint32_t nasip = 0;
  uint32_t nasport = 0;


  if (radius_default_pack(radius, &radius_pack, 
			  RADIUS_CODE_ACCOUNTING_RESPONSE)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }
  radius_pack.id = pack->id;
  
  /* Status type */
  if (radius_getattr(pack, &typeattr, RADIUS_ATTR_ACCT_STATUS_TYPE, 0, 0, 0)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Status type is missing from radius request");
    (void) radius_resp(radius, &radius_pack, peer, pack->authenticator);
    return 0;
  }

  if (typeattr->v.i != htonl(RADIUS_STATUS_TYPE_STOP)) {
    (void) radius_resp(radius, &radius_pack, peer, pack->authenticator);
    return 0;
  }


  /* NAS IP */
  if (!radius_getattr(pack, &nasipattr, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 0)) {
    if ((nasipattr->l-2) != sizeof(appconn->nasip)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of NAS IP address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    nasip = nasipattr->v.i;
  }
  
  /* NAS PORT */
  if (!radius_getattr(pack, &nasportattr, RADIUS_ATTR_NAS_PORT, 0, 0, 0)) {
    if ((nasportattr->l-2) != sizeof(appconn->nasport)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of NAS port");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    nasport = nasportattr->v.i;
  }
  
  /* Calling Station ID (MAC Address) */
  if (!radius_getattr(pack, &hismacattr, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0)) {
    if (options.debug) {
      printf("Calling Station ID is: ");
      for (n=0; n<hismacattr->l-2; n++) printf("%c", hismacattr->v.t[n]);
      printf("\n");
    }
    if ((macstrlen = hismacattr->l-2) >= (RADIUS_ATTR_VLEN-1)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of called station ID");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    memcpy(macstr,hismacattr->v.t, macstrlen);
    macstr[macstrlen] = 0;
    
    /* Replace anything but hex with space */
    for (i=0; i<macstrlen; i++) 
      if (!isxdigit(macstr[i])) macstr[i] = 0x20;
    
    if (sscanf (macstr, "%2x %2x %2x %2x %2x %2x",
		&temp[0], &temp[1], &temp[2], 
		&temp[3], &temp[4], &temp[5]) != 6) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to convert Calling Station ID to MAC Address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    
    for(i = 0; i < DHCP_ETH_ALEN; i++) 
      hismac[i] = temp[i];
  }

  if (hismacattr) { /* Look for mac address.*/
    if (dhcp_hashget(dhcp, &dhcpconn, hismac)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Unknown connection");
      (void) radius_resp(radius, &radius_pack, peer, pack->authenticator);
      return 0;
    }
    if (!(dhcpconn->peer) || (!((struct app_conn_t*) dhcpconn->peer)->uplink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "No peer protocol defined");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    appconn = (struct app_conn_t*) dhcpconn->peer;
  }
  else if (nasipattr && nasportattr) { /* Look for NAS IP / Port */
    if (getconn(&appconn, nasip, nasport)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Unknown connection");
      (void) radius_resp(radius, &radius_pack, peer, pack->authenticator);
      return 0;
    }
  }
  else {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Calling Station ID or NAS IP/Port is missing from radius request");
    (void) radius_resp(radius, &radius_pack, peer, pack->authenticator);
    return 0;
  }
  
  /* Silently ignore radius request if allready processing one */
  if (appconn->radiuswait)
    return 0;
  
  /* TODO: Check validity of pointers */
  
  switch (appconn->dnprot) {
  case DNPROT_UAM:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,"Auth stop received for UAM");
    break;
  case DNPROT_WPA:
    dhcpconn = (struct dhcp_conn_t*) appconn->dnlink;
    if (!dhcpconn) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,"No downlink protocol");
      return 0;
    }
    /* Connection is simply deleted */
    dhcp_freeconn(dhcpconn);
    break;
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,"Unknown downlink protocol");
    (void) radius_resp(radius, &radius_pack, peer, pack->authenticator);
    return 0;
  }

  (void) radius_resp(radius, &radius_pack, peer, pack->authenticator);

  return 0;
}


int access_request(struct radius_packet_t *pack,
		   struct sockaddr_in *peer) {
  int n;
  struct radius_packet_t radius_pack;

  struct ippoolm_t *ipm = NULL;

  struct radius_attr_t *hisipattr = NULL;
  struct radius_attr_t *nasipattr = NULL;
  struct radius_attr_t *nasportattr = NULL;
  struct radius_attr_t *hismacattr = NULL;
  struct radius_attr_t *uidattr = NULL;
  struct radius_attr_t *pwdattr = NULL;
  struct radius_attr_t *eapattr = NULL;

  struct in_addr hisip;
  char pwd[RADIUS_ATTR_VLEN];
  int pwdlen;
  uint8_t hismac[DHCP_ETH_ALEN];
  char macstr[RADIUS_ATTR_VLEN];
  int macstrlen;
  unsigned int temp[DHCP_ETH_ALEN];
  int	i;
  char mac[MACSTRLEN+1];

  struct app_conn_t *appconn = NULL;
  struct dhcp_conn_t *dhcpconn = NULL;

  uint8_t resp[EAP_LEN];         /* EAP response */
  int resplen;                   /* Length of EAP response */

  int offset = 0;
  int instance = 0;
  int eaplen = 0;

  if (options.debug) printf("Radius access request received!\n");

  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REJECT)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }
  radius_pack.id = pack->id;

  /* User is identified by either IP address OR MAC address */
  
  /* Framed IP address (Conditional) */
  if (!radius_getattr(pack, &hisipattr, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)) {
    if (options.debug) {
      printf("Framed IP address is: ");
      for (n=0; n<hisipattr->l-2; n++) printf("%.2x", hisipattr->v.t[n]); 
      printf("\n");
    }
    if ((hisipattr->l-2) != sizeof(hisip.s_addr)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of framed IP address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    hisip.s_addr = hisipattr->v.i;
  }

  /* Calling Station ID: MAC Address (Conditional) */
  if (!radius_getattr(pack, &hismacattr, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0)) {
    if (options.debug) {
      printf("Calling Station ID is: ");
      for (n=0; n<hismacattr->l-2; n++) printf("%c", hismacattr->v.t[n]);
      printf("\n");
    }
    if ((macstrlen = hismacattr->l-2) >= (RADIUS_ATTR_VLEN-1)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of called station ID");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    memcpy(macstr,hismacattr->v.t, macstrlen);
    macstr[macstrlen] = 0;

    /* Replace anything but hex with space */
    for (i=0; i<macstrlen; i++) 
      if (!isxdigit(macstr[i])) macstr[i] = 0x20;

    if (sscanf (macstr, "%2x %2x %2x %2x %2x %2x",
		&temp[0], &temp[1], &temp[2], 
		&temp[3], &temp[4], &temp[5]) != 6) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to convert Calling Station ID to MAC Address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    
    for(i = 0; i < DHCP_ETH_ALEN; i++) 
      hismac[i] = temp[i];
  }

  /* Framed IP address or MAC Address must be given in request */
  if ((!hisipattr) && (!hismacattr)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Framed IP address or Calling Station ID is missing from radius request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  /* Username (Mandatory) */
  if (radius_getattr(pack, &uidattr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "User-Name is missing from radius request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  } 
  else {
    if (options.debug) {
      printf("Username is: ");
      for (n=0; n<uidattr->l-2; n++) printf("%c", uidattr->v.t[n]); 
      printf("\n");
    }
  }

  
  if (hisipattr) { /* Find user based on IP address */
    if (ippool_getip(ippool, &ipm, &hisip)) {
      if (options.debug) printf("Radius request: Address not found!!!\n");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    
    if (!(ipm->peer) || (!((struct app_conn_t*) ipm->peer)->dnlink)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "No peer protocol defined");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    appconn = (struct app_conn_t*) ipm->peer;
    dhcpconn = (struct dhcp_conn_t*) appconn->dnlink;
  }
  else if (hismacattr) { /* Look for mac address. If not found allocate new */
    if (dhcp_hashget(dhcp, &dhcpconn, hismac)) {
      if (dhcp_newconn(dhcp, &dhcpconn, hismac)) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"Out of connections");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }
    }
    if (!(dhcpconn->peer)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "No peer protocol defined");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    appconn = (struct app_conn_t*) dhcpconn->peer;
    if (appconn->dnprot == DNPROT_DHCP_NONE)
      appconn->dnprot = DNPROT_WPA;
  }
  else {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Framed IP address or Calling Station ID is missing from radius request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  /* Silently ignore radius request if allready processing one */
  if (appconn->radiuswait)
    return 0;
  
  /* Radius auth only for DHCP */
  if ((appconn->dnprot != DNPROT_UAM) &&
      (appconn->dnprot != DNPROT_WPA))  { 
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  /* Password */
  if (!radius_getattr(pack, &pwdattr, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0)) {
    if (options.debug) {
      printf("Password is: ");
      for (n=0; n<pwdattr->l-2; n++) printf("%.2x", pwdattr->v.t[n]); 
      printf("\n");
    }
    if (radius_pwdecode(radius, (uint8_t*) pwd, RADIUS_ATTR_VLEN, &pwdlen, 
			pwdattr->v.t, pwdattr->l-2, pack->authenticator,
			radius->proxysecret,
			radius->proxysecretlen)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "radius_pwdecode() failed");
      return -1;
    }
    if (options.debug) printf("Password is: %s\n", pwd);
  }

  /* Get EAP message */
  resplen = 0;
  do {
    eapattr=NULL;
    if (!radius_getattr(pack, &eapattr, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 
			instance++)) {
      if ((resplen + eapattr->l-2) > EAP_LEN) {
	sys_err(LOG_INFO, __FILE__, __LINE__, 0,
		"EAP message too long");
	return radius_resp(radius, &radius_pack, peer, pack->authenticator);
      }
      memcpy(resp+resplen, 
	     eapattr->v.t, eapattr->l-2);
      resplen += eapattr->l-2;
    }
  } while (eapattr);
  

  /* Passwd or EAP must be given in request */
  if ((!pwdattr) && (!resplen)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Password or EAP meaasge is missing from radius request");
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  /* Dublicate logins should be allowed as it might be the terminal
     moving from one access point to another. It is however
     unacceptable to login with another username on top of an allready
     existing connection */

  /* TODO: New username should be allowed, but should result in
     a accounting stop message for the old connection.
     this does however pose a denial of service attack possibility */
  
  /* If allready logged in send back accept message with username */
  /* TODO ? Should this be a reject: Dont login twice ? */


  /* Reject if trying to login with another username */
  if ((appconn->authenticated == 1) && 
      ((appconn->userlen != uidattr->l-2) ||
       (memcmp(appconn->user, uidattr->v.t, uidattr->l-2)))) {
    return radius_resp(radius, &radius_pack, peer, pack->authenticator);
  }

  /* NAS IP */
  if (!radius_getattr(pack, &nasipattr, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0, 0)) {
    if ((nasipattr->l-2) != sizeof(appconn->nasip)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of NAS IP address");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    appconn->proxynasip = nasipattr->v.i;
  }

  /* NAS PORT */
  if (!radius_getattr(pack, &nasportattr, RADIUS_ATTR_NAS_PORT, 0, 0, 0)) {
    if ((nasportattr->l-2) != sizeof(appconn->nasport)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of NAS port");
      return radius_resp(radius, &radius_pack, peer, pack->authenticator);
    }
    appconn->proxynasport = nasportattr->v.i;
  }

  /* Store parameters for later use */
  if (uidattr->l-2<=USERNAMESIZE) {
    memcpy(appconn->proxyuser, uidattr->v.t, uidattr->l-2);
    appconn->proxyuserlen = uidattr->l-2;
  }
  appconn->radiuswait = 1;
  appconn->radiusid = pack->id;
  if (pwdattr)
    appconn->authtype = PAP_PASSWORD;
  else
    appconn->authtype = EAP_MESSAGE;
  memcpy(&appconn->radiuspeer, peer, sizeof(*peer));
  memcpy(appconn->authenticator, pack->authenticator, RADIUS_AUTHLEN);
  memcpy(appconn->proxyhismac, dhcpconn->hismac, DHCP_ETH_ALEN);
  memcpy(appconn->proxyourmac, dhcpconn->ourmac, DHCP_ETH_ALEN);

  /* Build up radius request */
  radius_pack.code = RADIUS_CODE_ACCESS_REQUEST;
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
		 uidattr->v.t, uidattr->l - 2);

  if (appconn->statelen) {
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_STATE, 0, 0, 0,
		   appconn->statebuf,
		   appconn->statelen);
  }

  if (pwdattr)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_PASSWORD, 0, 0, 0,
		   (uint8_t*) pwd, pwdlen);

  /* Include EAP (if present) */
  offset = 0;
  while (offset < resplen) {
    if ((resplen - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = resplen - offset;
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		   resp + offset, eaplen);
    offset += eaplen;
  } 

  if (resplen)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		   0, 0, 0, NULL, RADIUS_MD5LEN);


  /* Include his MAC address */
  (void) snprintf(mac, MACSTRLEN+1, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
	   appconn->proxyhismac[0], appconn->proxyhismac[1],
	   appconn->proxyhismac[2], appconn->proxyhismac[3],
	   appconn->proxyhismac[4], appconn->proxyhismac[5]);
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLING_STATION_ID, 0, 0, 0,
		 (uint8_t*) mac, MACSTRLEN);
  
  if (options.radiuscalled)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_CALLED_STATION_ID, 0, 0, 0,
			  (uint8_t*) options.radiuscalled, strlen(options.radiuscalled));
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
		 options.radiusnasporttype, NULL, 0);
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT, 0, 0,
		 appconn->unit, NULL, 0);
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
		 ntohl(options.radiusnasip.s_addr), NULL, 0);
  
  /* Include NAS-Identifier if given in configuration options */
  if (options.radiusnasid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
			  (uint8_t*) options.radiusnasid, strlen(options.radiusnasid));
  
  return radius_req(radius, &radius_pack, appconn);
}


/*********************************************************
 *
 * radius proxy callback functions (request from radius server)
 *
 *********************************************************/

/* Radius callback when radius request has been received */
int cb_radius_ind(struct radius_t *rp, struct radius_packet_t *pack,
		  struct sockaddr_in *peer) {

  if (rp != radius) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Radius callback from unknown instance");
    return 0;
  }
  
  if (options.nodhcp) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Radius request received when not using dhcp");
    return 0;
  }

  switch (pack->code) {
  case RADIUS_CODE_ACCOUNTING_REQUEST: /* TODO: Exclude ??? */
    return accounting_request(pack, peer);
  case RADIUS_CODE_ACCESS_REQUEST:
    return access_request(pack, peer);
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Unsupported radius request received: %d", pack->code);
    return 0;
  }
}


/***********************************************************
 *
 * Functions handling uplink protocol authentication.
 * Called in response to radius access request response.
 *
 ***********************************************************/

int upprot_getip(struct app_conn_t *appconn, 
		 struct in_addr *hisip, int statip) {
  struct ippoolm_t *ipm;

  /* If IP address is allready allocated: Fill it in */
  /* This should only happen for UAM */
  /* TODO */
  if (appconn->uplink) {
    ipm = (struct ippoolm_t*) appconn->uplink;
  }
  else {
    /* Allocate static or dynamic IP address */
    
    if ((hisip) && (statip)) {
      if (ippool_newip(ippool, &ipm, hisip, 1)) {
	if (ippool_newip(ippool, &ipm, NULL, 0)) {
	  sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		  "Failed to allocate both static and dynamic IP address");
	  return dnprot_reject(appconn);
	}
      }
    }
    else {
      if (ippool_newip(ippool, &ipm, hisip, 0)) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"Failed to allocate dynamic IP address");
	return dnprot_reject(appconn);
      }
    }
    appconn->hisip.s_addr = ipm->addr.s_addr;

    /* TODO: Listening address is network address plus 1 */
    appconn->ourip.s_addr = htonl((ntohl(options.net.s_addr)+1));
    
    appconn->uplink = ipm;
    ipm->peer   = appconn; 
  }

  return dnprot_accept(appconn);

}


/*********************************************************
 *
 * radius callback functions (response from radius server)
 *
 *********************************************************/

/* Radius handler for configuration management */
int radius_conf(struct radius_t *radius, 
		struct radius_packet_t *pack,
		struct radius_packet_t *pack_req) {

  struct radius_attr_t *hisipattr = NULL;
  struct radius_attr_t *lmntattr = NULL;
  struct radius_attr_t *sendattr = NULL;
  struct radius_attr_t *recvattr = NULL;
  struct radius_attr_t *succattr = NULL;
  struct radius_attr_t *policyattr = NULL;
  struct radius_attr_t *typesattr = NULL;

  struct radius_attr_t *eapattr = NULL;
  struct radius_attr_t *stateattr = NULL;
  struct radius_attr_t *classattr = NULL;
  struct radius_attr_t *interimattr = NULL;

  struct radius_attr_t *attr = NULL;

  char attrs[RADIUS_ATTR_VLEN+1];
  struct tm stt;
  int tzhour, tzmin;
  char *tz;

  int instance = 0;
  int n;
  int result;
  struct in_addr *hisip = NULL;
  int statip = 0;

  if (options.debug)
    printf("Received configuration management message from radius server\n");
  

  if (!pack) { /* Timeout */
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Radius request timed out");
    return 0;
  }

  /* ACCESS-REJECT */
  if (pack->code == RADIUS_CODE_ACCESS_REJECT) {
    if (options.debug)
      printf("Received access reject from radius server\n");
    return 0;
  }

  /* ACCESS-CHALLENGE */
  if (pack->code == RADIUS_CODE_ACCESS_CHALLENGE) {
    if (options.debug)
      printf("Received access reject from radius server\n");
    return 0;
  }
  
  /* ACCESS-ACCEPT */
  if (pack->code != RADIUS_CODE_ACCESS_ACCEPT) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Unknown code of radius access request confirmation");
    return 0;
  }

  /* Get Service Type */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_SERVICE_TYPE, 0, 0, 0)) {
    if(ntohl(attr->v.i) != RADIUS_SERVICE_TYPE_CHILLISPOT_AUTHORIZE_ONLY) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Chillispot-Authorize-Only Service-Type not in Access-Accept");
      return 0;
    }
  }

  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_UAM_ALLOWED, 0)) {
    memset(options.uamokip, 0, sizeof(options.uamokip));
    options.uamokiplen = 0;
    memset(options.uamokaddr, 0, sizeof(options.uamokaddr));
    memset(options.uamokmask, 0, sizeof(options.uamokmask));
    options.uamoknetlen = 0;
    (void) set_uamallowed((char*)attr->v.t, attr->l-2);
  }
  
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_MAC_ALLOWED, 0)) {
    memset(options.macok, 0, sizeof(options.macok));
    options.macoklen = 0;
    (void) set_macallowed((char*)attr->v.t, attr->l-2);
  }
  
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_INTERVAL, 0)) {
    options.interval = ntohl(attr->v.i);
    if (options.interval < 0) options.interval = 0;
  }


  /* Reinit DHCP parameters */
  (void) dhcp_set(dhcp, (options.debug & DEBUG_DHCP),
		  options.uamserver, options.uamserverlen, options.uamanydns,
		  options.uamokip, options.uamokiplen,
		  options.uamokaddr, options.uamokmask, options.uamoknetlen);
}

/* Radius callback when access accept/reject/challenge has been received */
int cb_radius_auth_conf(struct radius_t *radius, 
			struct radius_packet_t *pack,
			struct radius_packet_t *pack_req, void *cbp) {
  struct radius_attr_t *hisipattr = NULL;
  struct radius_attr_t *lmntattr = NULL;
  struct radius_attr_t *sendattr = NULL;
  struct radius_attr_t *recvattr = NULL;
  struct radius_attr_t *succattr = NULL;
  struct radius_attr_t *policyattr = NULL;
  struct radius_attr_t *typesattr = NULL;

  struct radius_attr_t *eapattr = NULL;
  struct radius_attr_t *stateattr = NULL;
  struct radius_attr_t *classattr = NULL;
  struct radius_attr_t *interimattr = NULL;

  struct radius_attr_t *attr = NULL;

  char attrs[RADIUS_ATTR_VLEN+1];
  struct tm stt;
  int tzhour, tzmin;
  char *tz;

  int instance = 0;
  int n;
  int result;
  struct in_addr *hisip = NULL;
  int statip = 0;

  struct app_conn_t *appconn = (struct app_conn_t*) cbp;

  if (options.debug)
    printf("Received access request confirmation from radius server\n");
  
  if (!appconn) {
    return radius_conf(radius, pack, pack_req);
    /*sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No peer protocol defined");
	    return 0;*/
  }

  /* Initialise */
  appconn->statelen = 0;
  appconn->challen  = 0;
  appconn->sendlen  = 0;
  appconn->recvlen  = 0;
  appconn->lmntlen  = 0;
  

  if (!pack) { /* Timeout */
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Radius request timed out");
    return dnprot_reject(appconn);
  }

  /* ACCESS-REJECT */
  if (pack->code == RADIUS_CODE_ACCESS_REJECT) {
    if (options.debug)
      printf("Received access reject from radius server\n");
    return dnprot_reject(appconn);
  }

  /* ACCESS-CHALLENGE */
  if (pack->code == RADIUS_CODE_ACCESS_CHALLENGE) {
    if (options.debug)
      printf("Received access challenge from radius server\n");

    /* Get EAP message */
    appconn->challen = 0;
    do {
      eapattr=NULL;
      if (!radius_getattr(pack, &eapattr, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 
			  instance++)) {
	if ((appconn->challen + eapattr->l-2) > EAP_LEN) {
	  sys_err(LOG_INFO, __FILE__, __LINE__, 0,
		  "EAP message too long");
	  return dnprot_reject(appconn);
	}
	memcpy(appconn->chal+appconn->challen, 
	       eapattr->v.t, eapattr->l-2);
	appconn->challen += eapattr->l-2;
      }
    } while (eapattr);
    
    if (!appconn->challen) {
      sys_err(LOG_INFO, __FILE__, __LINE__, 0,
	      "No EAP message found");
      return dnprot_reject(appconn);
    }
    
    /* Get State */
    if (!radius_getattr(pack, &stateattr, RADIUS_ATTR_STATE, 0, 0, 0)) {
      appconn->statelen = stateattr->l-2;
      memcpy(appconn->statebuf, stateattr->v.t, stateattr->l-2);
    }
    return dnprot_challenge(appconn);
  }
  
  /* ACCESS-ACCEPT */
  if (pack->code != RADIUS_CODE_ACCESS_ACCEPT) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Unknown code of radius access request confirmation");
    return dnprot_reject(appconn);
  }

  /* Get Service Type */
  if (!radius_getattr(pack, &stateattr, RADIUS_ATTR_SERVICE_TYPE, 0, 0, 0)) {
    if(ntohl(attr->v.i) == RADIUS_SERVICE_TYPE_CHILLISPOT_AUTHORIZE_ONLY) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Chillispot-Authorize-Only Service-Type in Access-Accept");
      return dnprot_reject(appconn);
    }
  }

  /* Get State */
  if (!radius_getattr(pack, &stateattr, RADIUS_ATTR_STATE, 0, 0, 0)) {
    appconn->statelen = stateattr->l-2;
    memcpy(appconn->statebuf, stateattr->v.t, stateattr->l-2);
  }

  /* Class */
  if (!radius_getattr(pack, &classattr, RADIUS_ATTR_CLASS, 0, 0, 0)) {
    appconn->classlen = classattr->l-2;
    memcpy(appconn->classbuf, classattr->v.t, classattr->l-2);
  }
  else {
    appconn->classlen = 0;
  }

  /* Session timeout */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_SESSION_TIMEOUT,
		      0, 0, 0)) {
    appconn->sessiontimeout = ntohl(attr->v.i);
  }
  else {
    appconn->sessiontimeout = 0;
  }

  /* Idle timeout */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_IDLE_TIMEOUT,
		      0, 0, 0)) {
    appconn->idletimeout = ntohl(attr->v.i);
  }
  else {
    appconn->idletimeout = 0;
  }

  /* Framed IP address (Optional) */
  if (!radius_getattr(pack, &hisipattr, RADIUS_ATTR_FRAMED_IP_ADDRESS, 0, 0, 0)) {
    if (options.debug) {
      printf("Framed IP address is: ");
      for (n=0; n<hisipattr->l-2; n++) printf("%.2x", hisipattr->v.t[n]); 
      printf("\n");
    }
    if ((hisipattr->l-2) != sizeof(struct in_addr)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of framed IP address");
      return dnprot_reject(appconn);
    }
    hisip = (struct in_addr*) &(hisipattr->v.i);
    statip = 1;
  }
  else {
    hisip = (struct in_addr*) &appconn->reqip.s_addr;
  }

	/* Filter ID */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_FILTER_ID,
		      0, 0, 0)) {
    appconn->filteridlen = attr->l-2;
    memcpy(appconn->filteridbuf, attr->v.t, attr->l-2);
    appconn->filteridbuf[attr->l-2] = 0;
    /*conn->filterid = conn->filteridbuf;*/
  }
  else {
    appconn->filteridlen = 0;
    appconn->filteridbuf[0] = 0;
    /*conn->filterid = NULL;*/
  }

  /* Interim interval */
  if (!radius_getattr(pack, &interimattr, RADIUS_ATTR_ACCT_INTERIM_INTERVAL, 
		      0, 0, 0)) {
    appconn->interim_interval = ntohl(interimattr->v.i);
    if (appconn->interim_interval < 60) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Received too small radius Acct-Interim-Interval value: %d. Disabling interim accounting",
	      appconn->interim_interval);
      appconn->interim_interval = 0;
    } 
    else if (appconn->interim_interval < 600) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Received small radius Acct-Interim-Interval value: %d",
	      appconn->interim_interval);
    }
  }
  else {
    appconn->interim_interval = 0;
  }

  /* Bandwidth up */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_BANDWIDTH_MAX_UP, 0)) {
    appconn->bandwidthmaxup = ntohl(attr->v.i);
#ifdef BUCKET_SIZE
    appconn->bucketupsize = BUCKET_SIZE;
#else
    appconn->bucketupsize = appconn->bandwidthmaxup / 8000 * BUCKET_TIME;
    if (appconn->bucketupsize < BUCKET_SIZE_MIN) 
      appconn->bucketupsize = BUCKET_SIZE_MIN;
#endif
  }
  else {
    appconn->bandwidthmaxup = 0;
  }
  
  /* Bandwidth down */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_BANDWIDTH_MAX_DOWN, 0)) {
    appconn->bandwidthmaxdown = ntohl(attr->v.i);
#ifdef BUCKET_SIZE
    appconn->bucketdownsize = BUCKET_SIZE;
#else
    appconn->bucketdownsize = appconn->bandwidthmaxdown / 8000 * BUCKET_TIME;
    if (appconn->bucketdownsize < BUCKET_SIZE_MIN) 
      appconn->bucketdownsize = BUCKET_SIZE_MIN;
#endif
  }
  else {
    appconn->bandwidthmaxdown = 0;
  }

#ifdef RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_UP
  /* Bandwidth up */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_UP, 0)) {
    appconn->bandwidthmaxup = ntohl(attr->v.i) * 1000;
    appconn->bucketupsize = BUCKET_TIME * appconn->bandwidthmaxup / 8000;
    if (appconn->bucketupsize < BUCKET_SIZE_MIN) 
      appconn->bucketupsize = BUCKET_SIZE_MIN;
  }
#endif

#ifdef RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_DOWN
  /* Bandwidth down */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_BANDWIDTH_MAX_DOWN, 0)) {
    appconn->bandwidthmaxdown = ntohl(attr->v.i) * 1000;
    appconn->bucketdownsize = BUCKET_TIME * appconn->bandwidthmaxdown / 8000;
    if (appconn->bucketdownsize < BUCKET_SIZE_MIN) 
      appconn->bucketdownsize = BUCKET_SIZE_MIN;
  }
#endif

  /* Max input octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_MAX_INPUT_OCTETS, 0)) {
    appconn->maxinputoctets = ntohl(attr->v.i);
  }
  else {
    appconn->maxinputoctets = 0;
  }

  /* Max output octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_MAX_OUTPUT_OCTETS, 0)) {
    appconn->maxoutputoctets = ntohl(attr->v.i);
  }
  else {
    appconn->maxoutputoctets = 0;
  }

  /* Max total octets */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_CHILLISPOT,
		      RADIUS_ATTR_CHILLISPOT_MAX_TOTAL_OCTETS, 0)) {
    appconn->maxtotaloctets = ntohl(attr->v.i);
  }
  else {
    appconn->maxtotaloctets = 0;
  }

  /* Session-Terminate-Time */
  if (!radius_getattr(pack, &attr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_WISPR,
		      RADIUS_ATTR_WISPR_SESSION_TERMINATE_TIME, 0)) {
    struct timeval timenow;
    gettimeofday(&timenow, NULL);
    memcpy(attrs, attr->v.t, attr->l-2);
    attrs[attr->l-2] = 0;
    memset(&stt, 0, sizeof(stt));
    result = sscanf(attrs, "%d-%d-%dT%d:%d:%d %d:%d",
		    &stt.tm_year, &stt.tm_mon, &stt.tm_mday,
		    &stt.tm_hour, &stt.tm_min, &stt.tm_sec,
		    &tzhour, &tzmin);
    if (result == 8) { /* Timezone */
      /* tzhour and tzmin is hours and minutes east of GMT */
      /* timezone is defined as seconds west of GMT. Excludes DST */
      stt.tm_year -= 1900;
      stt.tm_mon  -= 1;
      stt.tm_hour -= tzhour; /* Adjust for timezone */
      stt.tm_min  -= tzmin;  /* Adjust for timezone */
      /*      stt.tm_hour += daylight;*/
      /*stt.tm_min  -= (timezone / 60);*/
      tz = getenv("TZ");
      setenv("TZ", "", 1); /* Set environment to UTC */
      tzset();
      appconn->sessionterminatetime = mktime(&stt);
      if (tz) 
			setenv("TZ", tz, 1); 
      else
			unsetenv("TZ");
      tzset();
    }
    else if (result >= 6) { /* Local time */
      tzset();
      stt.tm_year -= 1900;
      stt.tm_mon  -= 1;
      stt.tm_isdst = -1; /*daylight;*/
      appconn->sessionterminatetime = mktime(&stt);
    }
    else {
      appconn->sessionterminatetime = 0;
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "Illegal WISPr-Session-Terminate-Time received: %s", attrs);
    }
    if ((appconn->sessionterminatetime) && 
	(timenow.tv_sec > appconn->sessionterminatetime)) {
      sys_err(LOG_WARNING, __FILE__, __LINE__, 0,
	      "WISPr-Session-Terminate-Time in the past received: %s", attrs);
      return dnprot_reject(appconn);
    }
  }
  else {
    appconn->sessionterminatetime = 0;
  }


  /* EAP Message */
  appconn->challen = 0;
  do {
    eapattr=NULL;
    if (!radius_getattr(pack, &eapattr, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 
			instance++)) {
      if ((appconn->challen + eapattr->l-2) > EAP_LEN) {
	sys_err(LOG_INFO, __FILE__, __LINE__, 0,
		"EAP message too long");
	return dnprot_reject(appconn);
      }
      memcpy(appconn->chal+appconn->challen,
	     eapattr->v.t, eapattr->l-2);
      appconn->challen += eapattr->l-2;
    }
  } while (eapattr);

  /* Get sendkey */
  if (!radius_getattr(pack, &sendattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_MPPE_SEND_KEY, 0)) {
    if (radius_keydecode(radius, appconn->sendkey, RADIUS_ATTR_VLEN, 
			 &appconn->sendlen, (uint8_t*) &sendattr->v.t,
			 sendattr->l-2, pack_req->authenticator,
			 radius->secret, radius->secretlen)) {
      sys_err(LOG_INFO, __FILE__, __LINE__, 0,
	      "radius_keydecode() failed!");
      return dnprot_reject(appconn);
    }
  }
    
  /* Get recvkey */
  if (!radius_getattr(pack, &recvattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_MPPE_RECV_KEY, 0)) {
    if (radius_keydecode(radius, appconn->recvkey, RADIUS_ATTR_VLEN,
			 &appconn->recvlen, (uint8_t*) &recvattr->v.t,
			 recvattr->l-2, pack_req->authenticator,
			 radius->secret, radius->secretlen) ) {
      sys_err(LOG_INFO, __FILE__, __LINE__, 0,
	      "radius_keydecode() failed!");
      return dnprot_reject(appconn);
    }
  }

  /* Get LMNT keys */
  if (!radius_getattr(pack, &lmntattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_CHAP_MPPE_KEYS, 0)) {

    /* TODO: Check length of vendor attributes */
    if (radius_pwdecode(radius, appconn->lmntkeys, RADIUS_MPPEKEYSSIZE,
			&appconn->lmntlen, (uint8_t*) &lmntattr->v.t,
			lmntattr->l-2, pack_req->authenticator,
			radius->secret, radius->secretlen)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "radius_pwdecode() failed");
      return dnprot_reject(appconn);
    }
  }
  
  /* Get encryption policy */
  if (!radius_getattr(pack, &policyattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS, 
		      RADIUS_ATTR_MS_MPPE_ENCRYPTION_POLICY, 0)) {
    appconn->policy = ntohl(policyattr->v.i);
  }
  
  /* Get encryption types */
  if (!radius_getattr(pack, &typesattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS, 
		      RADIUS_ATTR_MS_MPPE_ENCRYPTION_TYPES, 0)) {
    appconn->types = ntohl(typesattr->v.i);
  }
  

  /* Get MS_Chap_v2 SUCCESS */
  if (!radius_getattr(pack, &succattr, RADIUS_ATTR_VENDOR_SPECIFIC,
		      RADIUS_VENDOR_MS,
		      RADIUS_ATTR_MS_CHAP2_SUCCESS, 0)) {
    if ((succattr->l-5) != MS2SUCCSIZE) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Wrong length of MS-CHAP2 success: %d", succattr->l-5);
      return dnprot_reject(appconn);
    }
    memcpy(appconn->ms2succ, ((void*)&succattr->v.t)+3, MS2SUCCSIZE);
  }
  
  switch(appconn->authtype) {
  case PAP_PASSWORD:
    appconn->policy = 0; /* TODO */
    break;
  case EAP_MESSAGE:
    if (!appconn->challen) {
      sys_err(LOG_INFO, __FILE__, __LINE__, 0, "No EAP message found");
      return dnprot_reject(appconn);
    }
    break;
  case CHAP_DIGEST_MD5:
    appconn->policy = 0; /* TODO */
    break;
  case CHAP_MICROSOFT:
    if (!lmntattr) {
      sys_err(LOG_INFO, __FILE__, __LINE__, 0,
	      "No MPPE keys found");
      return dnprot_reject(appconn);
    }
    if (!succattr) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "No MS-CHAP2 success found");
      return dnprot_reject(appconn);
    }
    break;
  case CHAP_MICROSOFT_V2:
    if (!sendattr) {
      sys_err(LOG_INFO, __FILE__, __LINE__, 0, "No MPPE sendkey found");
      return dnprot_reject(appconn);
    }
    
    if (!recvattr) {
      sys_err(LOG_INFO, __FILE__, __LINE__, 0, "No MPPE recvkey found");
      return dnprot_reject(appconn);
    }

    break;
  default:
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Unknown authtype");
    return dnprot_reject(appconn);
  }

  return upprot_getip(appconn, hisip, statip);
}


/* Radius callback when coa or disconnect request has been received */
int cb_radius_coa_ind(struct radius_t *radius, struct radius_packet_t *pack,
		      struct sockaddr_in *peer) {
  struct app_conn_t *appconn;
  struct dhcp_conn_t* dhcpconn;
  struct radius_attr_t *userattr = NULL;
  struct radius_packet_t radius_pack;
  int found = 0;

  if (options.debug)
    printf("Received coa or disconnect request\n");
  
  if (pack->code != RADIUS_CODE_DISCONNECT_REQUEST) {
    sys_err(LOG_INFO, __FILE__, __LINE__, 0, 
	    "Radius packet not supported: %d,\n", pack->code);
  }

  /* Get username */
  if (radius_getattr(pack, &userattr, RADIUS_ATTR_USER_NAME, 0, 0, 0)) {
    sys_err(LOG_INFO, __FILE__, __LINE__, 0, 
	    "Username must be included in disconnect request");
  }

  while (!getconn_username(&appconn, (char*) userattr->v.t, userattr->l-2)) {
    found = 1;
    if (appconn->authenticated == 1) {
      dnprot_terminate(appconn);
      appconn->terminate_cause = RADIUS_TERMINATE_CAUSE_ADMIN_RESET;
      (void) acct_req(appconn, RADIUS_STATUS_TYPE_STOP);
      set_sessionid(appconn);
    }
  }

  if (found) {
    if (radius_default_pack(radius, &radius_pack, 
			    RADIUS_CODE_DISCONNECT_ACK)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "radius_default_pack() failed");
      return -1;
    }
  }
  else {
    if (radius_default_pack(radius, &radius_pack, 
			    RADIUS_CODE_DISCONNECT_NAK)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "radius_default_pack() failed");
      return -1;
    }
  }
  radius_pack.id = pack->id;
  (void) radius_coaresp(radius, &radius_pack, peer, pack->authenticator);

  return 0;
}


/***********************************************************
 *
 * dhcp callback functions
 *
 ***********************************************************/

/* DHCP callback for allocating new IP address */
/* In the case of WPA it is allready allocated,
 * for UAM address is allocated before authentication */
int cb_dhcp_request(struct dhcp_conn_t *conn, struct in_addr *addr) {
  struct ippoolm_t *ipm;
  struct app_conn_t *appconn = conn->peer;

  if (options.debug) printf("DHCP requested IP address\n");

  if (!appconn) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Peer protocol not defined");
    return -1;
  }

  appconn->reqip.s_addr = addr->s_addr; /* Save for MAC auth later */


  /* If IP address is allready allocated: Fill it in */
  if (appconn->uplink) {
    ipm = (struct ippoolm_t*) appconn->uplink;
  }
  else if (appconn->dnprot == DNPROT_MAC) {
    return -1;
  }
  else if ((options.macauth) && (appconn->dnprot == DNPROT_DHCP_NONE) ){
    appconn->dnprot = DNPROT_MAC;
    (void) macauth_radius(appconn);
    return -1;
  }
  else if ((options.macoklen) && (appconn->dnprot == DNPROT_DHCP_NONE) &&
	   !maccmp(conn->hismac)) {
    appconn->dnprot = DNPROT_MAC;
    (void) macauth_radius(appconn);    
    return -1;
  }
  else {
    if (appconn->dnprot != DNPROT_DHCP_NONE) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Requested IP address when allready allocated");
    }
    
    /* Allocate dynamic IP address */
    if (ippool_newip(ippool, &ipm, &appconn->reqip, 0)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed allocate dynamic IP address");
      return -1;
    }
    appconn->hisip.s_addr = ipm->addr.s_addr;
    
    sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	    "Client MAC=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X assigned IP %s" , 
	    conn->hismac[0], conn->hismac[1], 
	    conn->hismac[2], conn->hismac[3],
	    conn->hismac[4], conn->hismac[5], 
	    inet_ntoa(appconn->hisip));

    /* TODO: Listening address is network address plus 1 */
    appconn->ourip.s_addr = htonl((ntohl(options.net.s_addr)+1));
    
    appconn->uplink =  ipm;
    ipm->peer   = appconn; 
  }
  
  (void) dhcp_set_addrs(conn, &ipm->addr, &options.mask, &appconn->ourip,
			&options.dns1, &options.dns2, options.domain);

  conn->authstate = DHCP_AUTH_DNAT;

  /* If IP was requested before authentication it was UAM */
  if (appconn->dnprot == DNPROT_DHCP_NONE)
    appconn->dnprot = DNPROT_UAM;
  
  return 0;
}

/* DHCP callback for establishing new connection */
int cb_dhcp_connect(struct dhcp_conn_t *conn) {
  struct app_conn_t *appconn;

  sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	  "New DHCP request from MAC=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X" , 
	  conn->hismac[0], conn->hismac[1], 
	  conn->hismac[2], conn->hismac[3],
	  conn->hismac[4], conn->hismac[5]);
  if (conn->hismac[0]==0 && conn->hismac[1]==0 && conn->hismac[2]==0 && conn->hismac[3]==0 && conn->hismac[4]==0 && conn->hismac[5]==0)
     {
     sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	  "New DHCP request invalid MAC, ignore=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X" , 
	  conn->hismac[0], conn->hismac[1], 
	  conn->hismac[2], conn->hismac[3],
	  conn->hismac[4], conn->hismac[5]);
        return 0;
     }
  
  if (options.debug) printf("New DHCP connection established\n");

  /* Allocate new application connection */
  if (newconn(&appconn)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to allocate connection");
    return 0;
  }

  appconn->dnlink =  conn;
  appconn->dnprot =  DNPROT_DHCP_NONE;
  conn->peer  = appconn;

  appconn->net.s_addr = options.net.s_addr;
  appconn->mask.s_addr = options.mask.s_addr;
  appconn->dns1.s_addr = options.dns1.s_addr;
  appconn->dns2.s_addr = options.dns2.s_addr;

  memcpy(appconn->hismac, conn->hismac, DHCP_ETH_ALEN);
  memcpy(appconn->ourmac, conn->ourmac, DHCP_ETH_ALEN);
  memcpy(appconn->proxyhismac, conn->hismac, DHCP_ETH_ALEN);
  memcpy(appconn->proxyourmac, conn->ourmac, DHCP_ETH_ALEN);
  
  set_sessionid(appconn);

  conn->authstate = DHCP_AUTH_NONE; /* TODO: Not yet authenticated */

  return 0;
}


/* Callback when a dhcp connection is deleted */
int cb_dhcp_disconnect(struct dhcp_conn_t *conn) {
  struct app_conn_t *appconn;

  sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	  "DHCP addr released by MAC=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X IP=%s", 
	  conn->hismac[0], conn->hismac[1], 
	  conn->hismac[2], conn->hismac[3],
	  conn->hismac[4], conn->hismac[5], 
	  inet_ntoa(conn->hisip));
  if (conn->hismac[0]==0 && conn->hismac[1]==0 && conn->hismac[2]==0 && conn->hismac[3]==0 && conn->hismac[4]==0 && conn->hismac[5]==0)
     {
     sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	  "DHCP addr release invalid MAC, ignore=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X" , 
	  conn->hismac[0], conn->hismac[1], 
	  conn->hismac[2], conn->hismac[3],
	  conn->hismac[4], conn->hismac[5]);
        return 0;
     }
  if (!strcmp(inet_ntoa(conn->hisip),"0.0.0.0"))
	 {
        sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	  "DHCP addr release invalid IP, ignore=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X" , 
	  conn->hismac[0], conn->hismac[1], 
	  conn->hismac[2], conn->hismac[3],
	  conn->hismac[4], conn->hismac[5]);
	 return 0;
	 }
  
  if (options.debug) printf("DHCP connection removed\n");

  if (!conn->peer) 
    return 0; /* No appconn allocated. Stop here */
  else
    appconn = (struct app_conn_t*) conn->peer;

  if ((appconn->dnprot != DNPROT_DHCP_NONE) &&
      (appconn->dnprot != DNPROT_UAM) &&
      (appconn->dnprot != DNPROT_MAC) &&
      (appconn->dnprot != DNPROT_WPA) &&
      (appconn->dnprot != DNPROT_EAPOL))  {
    return 0; /* DNPROT_WPA and DNPROT_EAPOL are unaffected by dhcp release? */
  }

  /* User is logged out here. Can also happen by radius disconnect */
  if (appconn->authenticated == 1) { /* Only send accounting if logged in */
    appconn->authenticated = 0;
    appconn->terminate_cause = RADIUS_TERMINATE_CAUSE_LOST_CARRIER;
    (void) acct_req(appconn, RADIUS_STATUS_TYPE_STOP);
    set_sessionid(appconn);
  }

  conn->authstate = DHCP_AUTH_NONE; /* TODO: Redundant */

  if (appconn->uplink)
  if (ippool_freeip(ippool, (struct ippoolm_t *) appconn->uplink)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		 "ippool_freeip() failed!");
  }
  
  (void) freeconn(appconn);

  return 0;
}


/* Callback for receiving messages from dhcp */
int cb_dhcp_data_ind(struct dhcp_conn_t *conn, void *pack, unsigned len) {
  struct tun_packet_t *iph = (struct tun_packet_t*) pack;
  struct app_conn_t *appconn = conn->peer;

  /*if (options.debug)
    printf("cb_dhcp_data_ind. Packet received. DHCP authstate: %d\n", 
	   conn->authstate);*/

  if (iph->src != conn->hisip.s_addr) {
    if (options.debug) printf("Received packet with spoofed source!!!\n");
    return 0;
  }

  if (!appconn) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No peer protocol defined");
    return -1;
  }

  if (appconn->authenticated == 1) {
#ifndef NO_LEAKY_BUCKET
#ifndef COUNT_UPLINK_DROP
    if (leaky_bucket(appconn, len, 0)) return 0;
#endif /* ifndef COUNT_UPLINK_DROP */
#endif /* ifndef NO_LEAKY_BUCKET */
    appconn->input_packets++;
    appconn->input_octets +=len;
#ifndef NO_LEAKY_BUCKET
#ifdef COUNT_UPLINK_DROP
    if (leaky_bucket(appconn, len, 0)) return 0;
#endif /* ifdef COUNT_UPLINK_DROP */
#endif /* ifndef NO_LEAKY_BUCKET */
  }

  return tun_encaps(tun, pack, len);
}

/* Callback for receiving messages from eapol */
int cb_dhcp_eap_ind(struct dhcp_conn_t *conn, void *pack, unsigned len) {
  struct dhcp_eap_t *eap = (struct dhcp_eap_t*) pack;
  struct app_conn_t *appconn = conn->peer;
  struct radius_packet_t radius_pack;
  int offset;

  if (options.debug) printf("EAP Packet received \n");

  /* If this is the first EAPOL authentication request */
  if ((appconn->dnprot == DNPROT_DHCP_NONE) || 
      (appconn->dnprot == DNPROT_EAPOL)) {
    if ((eap->code == 2) && /* Response */
	(eap->type == 1) && /* Identity */
	(len > 5) &&        /* Must be at least 5 octets */
	((len - 5) <= USERNAMESIZE )) {
      appconn->proxyuserlen = len -5;
      memcpy(appconn->proxyuser, eap->payload, appconn->proxyuserlen); 
      appconn->dnprot = DNPROT_EAPOL;
      appconn->authtype = EAP_MESSAGE;
    }
    else if (appconn->dnprot == DNPROT_DHCP_NONE) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Initial EAP response was not a valid identity response!");
      return 0;
    }
  }

  /* Return if not EAPOL */
  if (appconn->dnprot != DNPROT_EAPOL) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Received EAP message when not authenticating using EAP!");
    return 0;
  }
  
  if (radius_default_pack(radius, &radius_pack, RADIUS_CODE_ACCESS_REQUEST)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "radius_default_pack() failed");
    return -1;
  }


  /* Build up radius request */
  radius_pack.code = RADIUS_CODE_ACCESS_REQUEST;
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_USER_NAME, 0, 0, 0,
			(uint8_t*) appconn->proxyuser, appconn->proxyuserlen);

  if (appconn->statelen) {
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_STATE, 0, 0, 0,
		   appconn->statebuf,
		   appconn->statelen);
  }
  
  /* Include EAP (if present) */
  offset = 0;
  while (offset < len) {
    int eaplen;
    if ((len - offset) > RADIUS_ATTR_VLEN)
      eaplen = RADIUS_ATTR_VLEN;
    else
      eaplen = len - offset;
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_EAP_MESSAGE, 0, 0, 0,
		   pack + offset, eaplen);
    offset += eaplen;
  } 
  
  if (len)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_MESSAGE_AUTHENTICATOR, 
		   0, 0, 0, NULL, RADIUS_MD5LEN);
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT_TYPE, 0, 0,
		 options.radiusnasporttype, NULL, 0);
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_PORT, 0, 0,
		 appconn->unit, NULL, 0);
  
  
  (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IP_ADDRESS, 0, 0,
		 ntohl(options.radiusnasip.s_addr), NULL, 0);
  
  /* Include NAS-Identifier if given in configuration options */
  if (options.radiusnasid)
    (void) radius_addattr(radius, &radius_pack, RADIUS_ATTR_NAS_IDENTIFIER, 0, 0, 0,
		   (uint8_t*) options.radiusnasid,
		   strlen(options.radiusnasid));
  
  return radius_req(radius, &radius_pack, appconn);
}


/***********************************************************
 *
 * uam message handling functions
 *
 ***********************************************************/

int static uam_msg(struct redir_msg_t *msg) {

  struct ippoolm_t *ipm;
  struct app_conn_t *appconn = NULL;
  struct dhcp_conn_t* dhcpconn;

  if (ippool_getip(ippool, &ipm, &msg->addr)) {
    if (options.debug) printf("UAM login with unknown IP address: %s\n",
			      inet_ntoa(msg->addr));
    return 0;
  }

  if (!((ipm->peer) || ((struct app_conn_t*) ipm->peer)->dnlink)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "No peer protocol defined");
    return 0;
  }

  appconn = (struct app_conn_t*) ipm->peer;
  dhcpconn = (struct dhcp_conn_t*) appconn->dnlink;

  if (msg->type == REDIR_LOGIN) {
    
    if (appconn->uamabort) {
      sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	      "UAM login from username=%s IP=%s was aborted!", 
	      msg->username, inet_ntoa(appconn->hisip));
      appconn->uamabort = 0;
      return 0;
    }

    sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	    "Successful UAM login from username=%s IP=%s", 
	    msg->username, inet_ntoa(appconn->hisip));
    
    if (options.debug)
      printf("Received login from UAM\n");
    
    /* Initialise */
    appconn->statelen = 0;
    appconn->challen  = 0;
    appconn->sendlen  = 0;
    appconn->recvlen  = 0;
    appconn->lmntlen  = 0;
    
    /* Store user name for accounting records */
    strncpy(appconn->user, msg->username, USERNAMESIZE);
    appconn->userlen = strlen(msg->username);

    strncpy(appconn->proxyuser, msg->username, USERNAMESIZE);
    appconn->proxyuserlen = strlen(msg->username);

    memcpy(appconn->hismac, dhcpconn->hismac, DHCP_ETH_ALEN);
    memcpy(appconn->ourmac, dhcpconn->ourmac, DHCP_ETH_ALEN);
    memcpy(appconn->proxyhismac, dhcpconn->hismac, DHCP_ETH_ALEN);
    memcpy(appconn->proxyourmac, dhcpconn->ourmac, DHCP_ETH_ALEN);
    
    appconn->policy = 0; /* TODO */

    appconn->statelen = msg->statelen;
    memcpy(appconn->statebuf, msg->statebuf, msg->statelen);
    appconn->classlen = msg->classlen;
    memcpy(appconn->classbuf, msg->classbuf, msg->classlen);
    appconn->sessiontimeout = msg->sessiontimeout;
    appconn->idletimeout = msg->idletimeout;
    appconn->interim_interval = msg->interim_interval;
    appconn->bandwidthmaxup = msg->bandwidthmaxup;
    appconn->bandwidthmaxdown = msg->bandwidthmaxdown;
    appconn->maxinputoctets = msg->maxinputoctets;
    appconn->maxoutputoctets = msg->maxoutputoctets;
    appconn->maxtotaloctets = msg->maxtotaloctets;
    appconn->sessionterminatetime = msg->sessionterminatetime;
    strncpy(appconn->filteridbuf, msg->filteridbuf, RADIUS_ATTR_VLEN+1);
    appconn->filteridlen = msg->filteridlen;

#ifdef BUCKET_SIZE
    appconn->bucketupsize = BUCKET_SIZE;
#else
    appconn->bucketupsize = appconn->bandwidthmaxup / 8000 * BUCKET_TIME;
    if (appconn->bucketupsize < BUCKET_SIZE_MIN) 
      appconn->bucketupsize = BUCKET_SIZE_MIN;
#endif

#ifdef BUCKET_SIZE
    appconn->bucketdownsize = BUCKET_SIZE;
#else
    appconn->bucketdownsize = appconn->bandwidthmaxdown / 8000 * BUCKET_TIME;
    if (appconn->bucketdownsize < BUCKET_SIZE_MIN) 
      appconn->bucketdownsize = BUCKET_SIZE_MIN;
#endif

    return upprot_getip(appconn, NULL, 0);
  }
  else if (msg->type == REDIR_LOGOUT) {

    sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	    "Received UAM logoff from username=%s IP=%s",
	    appconn->user, inet_ntoa(appconn->hisip));

    if (options.debug)
      printf("Received logoff from UAM\n");

    memcpy(appconn->uamchal, msg->uamchal, REDIR_MD5LEN);
    appconn->uamtime = time(NULL);
    appconn->uamabort = 0;
    dhcpconn->authstate = DHCP_AUTH_DNAT;

    if (appconn->authenticated == 1) {
      appconn->authenticated = 0;
      appconn->terminate_cause = RADIUS_TERMINATE_CAUSE_USER_REQUEST;
      (void) acct_req(appconn, RADIUS_STATUS_TYPE_STOP);
      set_sessionid(appconn);
    }

    return 0;
  }
  else if (msg->type == REDIR_ABORT) {
    
    sys_err(LOG_NOTICE, __FILE__, __LINE__, 0,
	    "Received UAM abort from IP=%s", inet_ntoa(appconn->hisip));

    appconn->uamabort = 1; /* Next login will be aborted */
    appconn->uamtime = 0;  /* Force generation of new challenge */
    dhcpconn->authstate = DHCP_AUTH_DNAT;

    if (appconn->authenticated == 1) {
      appconn->authenticated = 0;
      appconn->terminate_cause = RADIUS_TERMINATE_CAUSE_USER_REQUEST;
      (void) acct_req(appconn, RADIUS_STATUS_TYPE_STOP);
      set_sessionid(appconn);
    }
    return 0;
  }
  else if (msg->type == REDIR_CHALLENGE) {
    memcpy(appconn->uamchal, msg->uamchal, REDIR_MD5LEN);
    appconn->uamtime = time(NULL);
    appconn->uamabort = 0;
    if (msg->userurl[0]) {
      strncpy(appconn->userurl, msg->userurl, USERURLSIZE);
      appconn->userurl[USERURLSIZE-1] = 0;
    }
  }
  else {
    return 0;
  }
  return 0;
}

long lastboot;

int main(int argc, char **argv)
{
  
  int maxfd = 0;	                /* For select() */
  fd_set fds;			/* For select() */
  struct timeval idleTime;	/* How long to select() */
  int status;
  int msgresult;

  struct redir_msg_t msg;
  struct sigaction act;
  struct itimerval itval;

   /* Set startup time */
  struct sysinfo info;
  sysinfo(&info);
  lastboot=info.uptime;
 
/* open a connection to the syslog daemon */
  /*openlog(PACKAGE, LOG_PID, LOG_DAEMON);*/
  openlog(PACKAGE, (LOG_PID | LOG_PERROR), LOG_DAEMON);

  /* Process options given in configuration file and command line */
  if (process_options(argc, argv, 1))
    exit(1);
  
  if (options.debug) 
    printf("ChilliSpot version %s started.\n", VERSION);

  syslog(LOG_INFO, "ChilliSpot %s. Copyright 2002-2005 Mondru AB. Licensed under GPL. See http://www.chillispot.org for credits.", VERSION);
  

  /* Initialise connections */
  (void) initconn();
  
  /* Allocate ippool for dynamic IP address allocation */
  if (ippool_new(&ippool, options.dynip, options.statip, 
		 options.allowdyn, options.allowstat,
		 IPPOOL_NONETWORK | IPPOOL_NOBROADCAST | IPPOOL_NOGATEWAY)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to allocate IP pool!");
    exit(1);
  }

  /* Create a tunnel interface */
  if (tun_new((struct tun_t**) &tun)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to create tun");
    exit(1);
  }

  /*tun_setaddr(tun, &options.dhcplisten,  &options.net, &options.mask);*/
  (void) tun_setaddr(tun, &options.dhcplisten,  &options.dhcplisten, 
		     &options.mask);
  (void) tun_set_cb_ind(tun, cb_tun_ind);
  if (tun->fd > maxfd) maxfd = tun->fd;

  if (options.ipup) (void) tun_runscript(tun, options.ipup);
  

  /* Create an instance of dhcp */
  if (!options.nodhcp) {
    if (dhcp_new(&dhcp, APP_NUM_CONN, options.dhcpif,
		 options.dhcpusemac, options.dhcpmac, options.dhcpusemac, 
		 &options.dhcplisten, options.lease, 1, 
		 &options.uamlisten, options.uamport, options.eapolenable)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to create dhcp");
      exit(1);
    }
    if (dhcp->fd > maxfd)
      maxfd = dhcp->fd;
    if (dhcp->arp_fd > maxfd)
      maxfd = dhcp->arp_fd;
    if (dhcp->eapol_fd > maxfd)
      maxfd = dhcp->eapol_fd;
    
    (void) dhcp_set_cb_request(dhcp, cb_dhcp_request);
    (void) dhcp_set_cb_connect(dhcp, cb_dhcp_connect);
    (void) dhcp_set_cb_disconnect(dhcp, cb_dhcp_disconnect);
    (void) dhcp_set_cb_data_ind(dhcp, cb_dhcp_data_ind);
    (void) dhcp_set_cb_eap_ind(dhcp, cb_dhcp_eap_ind);
    if (dhcp_set(dhcp, (options.debug & DEBUG_DHCP),
		 options.uamserver, options.uamserverlen, options.uamanydns,
		 options.uamokip, options.uamokiplen,
		 options.uamokaddr, options.uamokmask, options.uamoknetlen)) {
      sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	      "Failed to set DHCP parameters");
      exit(1);
    }

  }

  /* Create an instance of radius */
  if (radius_new(&radius,
		 &options.radiuslisten, options.coaport, options.coanoipcheck,
		 &options.proxylisten, options.proxyport,
		 &options.proxyaddr, &options.proxymask,
		 options.proxysecret)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to create radius");
    return -1;
  }
  if (radius->fd > maxfd)
    maxfd = radius->fd;

  if ((radius->proxyfd != -1) && (radius->proxyfd > maxfd))
    maxfd = radius->proxyfd;

  radius_set(radius, (options.debug & DEBUG_RADIUS),
	     &options.radiusserver1, &options.radiusserver2,
	     options.radiusauthport, options.radiusacctport,
	     options.radiussecret);

  (void) radius_set_cb_auth_conf(radius, cb_radius_auth_conf);
  (void) radius_set_cb_ind(radius, cb_radius_ind);
  (void) radius_set_cb_coa_ind(radius, cb_radius_coa_ind);

  /* Get remote config from radius server */
  (void) config_radius();


  /* Create an instance of redir */
  if (redir_new(&redir,
		&options.uamlisten, options.uamport)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, 0,
	    "Failed to create redir");
    return -1;
  }
  if (redir->fd > maxfd)
    maxfd = redir->fd;

  redir_set(redir, (options.debug & DEBUG_REDIR),
	    options.uamurl, options.uamhomepage, options.uamsecret,
	    &options.radiuslisten,
	    &options.radiusserver1, &options.radiusserver2,
	    options.radiusauthport, options.radiusacctport,
	    options.radiussecret, options.radiusnasid,
	    &options.radiusnasip, options.radiuscalled,
	    options.radiuslocationid, options.radiuslocationname,
	    options.radiusnasporttype);


  (void) redir_set_cb_getstate(redir, cb_redir_getstate);

  /* Set up signal handlers */
  memset(&act, 0, sizeof(act));
  act.sa_handler = fireman;
  sigaction(SIGCHLD, &act, NULL);
  act.sa_handler = termination_handler;
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGINT, &act, NULL);
  act.sa_handler = alarm_handler;
  sigaction(SIGALRM, &act, NULL);
  act.sa_handler = sighup_handler;
  sigaction(SIGHUP, &act, NULL);

  memset(&itval, 0, sizeof(itval));
  itval.it_interval.tv_sec = 0;
  itval.it_interval.tv_usec = 500000; /* TODO 0.5 second */
  itval.it_value.tv_sec = 0; 
  itval.it_value.tv_usec = 500000; /* TODO 0.5 second */
  if (setitimer(ITIMER_REAL, &itval, NULL)) {
    sys_err(LOG_ERR, __FILE__, __LINE__, errno,
	    "setitimer() failed!");
  }
  
   /* Store the process ID in pidfile */
  if (options.pidfile) {
    log_pid(options.pidfile);
  }

  /* Store the process ID in pidfile */
  if (options.pidfile) {
    log_pid(options.pidfile);
  }

  if (options.debug) 
    printf("Waiting for client request...\n");


  /******************************************************************/
  /* Main select loop                                               */
  /******************************************************************/

  while (keep_going) {

    if (do_timeouts) {
      /*if (options.debug) printf("Do timeouts!\n");*/
      (void) radius_timeout(radius);
      if (dhcp) (void) dhcp_timeout(dhcp);
      (void) checkconn();
      do_timeouts = 0;
    }

    if (do_sighup) {
      reprocess_options(argc, argv);
      do_sighup = 0;
    }

    FD_ZERO(&fds);
    if (tun->fd != -1) FD_SET(tun->fd, &fds);
#if defined(__linux__)
    if (dhcp) FD_SET(dhcp->fd, &fds);
    if ((dhcp) && (dhcp->arp_fd)) FD_SET(dhcp->arp_fd, &fds);
    if ((dhcp) && (dhcp->eapol_fd)) FD_SET(dhcp->eapol_fd, &fds);
#elif defined (__FreeBSD__)  || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__) 
    if (dhcp) FD_SET(dhcp->fd, &fds);
#endif
    if (radius->fd != -1) FD_SET(radius->fd, &fds);
    if (radius->proxyfd != -1) FD_SET(radius->proxyfd, &fds);
    if (redir->fd != -1) FD_SET(redir->fd, &fds);

    idleTime.tv_sec = IDLETIME;
    idleTime.tv_usec = 0;
    /*radius_timeleft(radius, &idleTime);
      if (dhcp) dhcp_timeleft(dhcp, &idleTime);*/
    switch (status = select(maxfd + 1, &fds, NULL, NULL, /*&idleTime*/ NULL)) {
    case -1:
      if (EINTR != errno) {
	sys_err(LOG_ERR, __FILE__, __LINE__, errno,
		"select() returned -1!");
      }
      break;
    case 0:
      if (options.debug) printf("ChilliSpot is alive and ready to process packets!\n");
      break; 
    default:
      break;
    }

    if ((msgresult = msgrcv(redir->msgid, (struct msgbuf*) &msg, sizeof(msg), 
			   0, IPC_NOWAIT)) < 0) {
      if ((errno != EAGAIN) && (errno != ENOMSG))
	sys_err(LOG_ERR, __FILE__, __LINE__, errno, "msgrcv() failed!");
    }
    if (msgresult > 0) (void) uam_msg(&msg);
    
    if (status > 0) {

      if (tun->fd != -1 && FD_ISSET(tun->fd, &fds) && 
	  tun_decaps(tun) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"tun_decaps failed!");
      }
     
#if defined(__linux__)
      if ((dhcp) && FD_ISSET(dhcp->fd, &fds) && 
	  dhcp_decaps(dhcp) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"dhcp_decaps() failed!");
      }
      
      if ((dhcp) && FD_ISSET(dhcp->arp_fd, &fds) && 
	  dhcp_arp_ind(dhcp) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"dhcp_arpind() failed!");
      }

      if ((dhcp) && (dhcp->eapol_fd) && FD_ISSET(dhcp->eapol_fd, &fds) && 
	  dhcp_eapol_ind(dhcp) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"dhcp_eapol_ind() failed!");
      }

#elif defined (__FreeBSD__)  || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__APPLE__)
      if ((dhcp) && FD_ISSET(dhcp->fd, &fds) && 
	  dhcp_receive(dhcp) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"dhcp_decaps() failed!");
      }
#endif

      if (radius->fd != -1 && FD_ISSET(radius->fd, &fds) && 
	  radius_decaps(radius) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"radius_ind() failed!");
      }

      if (radius->proxyfd != -1 && FD_ISSET(radius->proxyfd, &fds) && 
	  radius_proxy_ind(radius) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"radius_proxy_ind() failed!");
      }

      if (redir->fd != -1 && FD_ISSET(redir->fd, &fds) && 
	  redir_accept(redir) < 0) {
	sys_err(LOG_ERR, __FILE__, __LINE__, 0,
		"redir_accept() failed!");
      }

    }
  }

  if (options.debug) printf("Terminating ChilliSpot!\n");

  (void) killconn();

  if (redir) (void) redir_free(redir);

  if (radius) (void) radius_free(radius);

  if (dhcp) (void) dhcp_free(dhcp);

  if (tun && options.ipdown) (void) tun_runscript(tun, options.ipdown);
  if (tun) (void) tun_free(tun);

  if (ippool) (void) ippool_free(ippool);

  return 0;
  
}
