/*
Copyright (C) 2003-2004 Narcis Ilisei
Modifications by Steve Horbachuk
Copyright (C) 2006 Steve Horbachuk
Modifications by Bryan Hoover (bhoover@wecs.com)
Copyright (C) 2007-2011 Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*
	Dyn Dns update main implementation file 
	Author: narcis Ilisei
	Date: May 2003

	History:
        - first implemetnation
        - 18 May 2003 : cmd line option reading added - 
        - Nov 2003 - new version
        - April 2004 - freedns.afraid.org system added.
        - October 2004 - Unix syslog capability added.
        - October 2007 - win32 RAS events trap thread, RAS, 
          and network online status checking added.  Debug 
          level command line parameter added.  Refactored,
          augmented main loop, including moving one time
          initialization outside of loop.  Two files
          changed -- dyndns.c, inadyn_cmd.c.  Two files
          added -- event_trap.c, event_trap.h.
        - November 2007 - multithread safe debug log.  One file
		  changed -- os.c.
        - December 2007 - Windows service routines, parser default 
          command, and registry, config file, command line command
          options hierachy.  Parser error handling callback.  
          Added files service.c, service.h, service_main.c,
          service_main.h, debug_service.h.  Changed  inadyn_cmd.c, 
        - get_cmd.c, os_windows.c, main.c.
        - June/July 2009 
	      libao and Windows waveOut interfacing for .wav file 
		  audible alerts on net down.
         -Sept. 2009
	      non-blocking socket create, connect
		 -Nov. 2010
		  pending updates mode
		  multiple aliases handled correctly
		  multiple Windows service instances
		  added trolobit.com's inadyn, ipv6tb, dnsomatic, tzo, milkfish support
		  fixed zoneedit rsp_config false positive
		-Jan 2011
		 added IPv6, etc.
*/

/*
UNICODE (Win 95/98/ME)

For Windows 32 non-NT (95/98/ME) unicode, Define UNICOWS.  Then compile and link with 
unicows.lib.  Install unicows.dll.

When linking, order is important.  The following (all of it) on the link line, 
without the cr/lf's, will do:

/nod:kernel32.lib /nod:advapi32.lib /nod:user32.lib /nod:gdi32.lib /nod:shell32.lib 
/nod:comdlg32.lib /nod:version.lib /nod:mpr.lib /nod:rasapi32.lib /nod:winmm.lib 
/nod:winspool.lib /nod:vfw32.lib /nod:secur32.lib /nod:oleacc.lib /nod:oledlg.lib 
/nod:sensapi.lib 

unicows.lib 

kernel32.lib advapi32.lib user32.lib gdi32.lib shell32.lib comdlg32.lib version.lib 
mpr.lib rasapi32.lib winmm.lib winspool.lib vfw32.lib secur32.lib oleacc.lib 
oledlg.lib sensapi.lib Ws2_32.lib libcmt.lib

There's no unicows.lib debug info, so this should be release target compile only.
*/
#define MODULE_TAG      "INADYN: "  

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32

#include <process.h>

#endif

#include "errorcode.h"

#include "path.h"

#include "dyndns.h"
#include "debug_if.h"
#include "base64.h"
#include "get_cmd.h"
#include "unicode_util.h"
#include "os.h"
#include "safe_mem.h"
#include "lang.h"
#include "md5.h"
#include "numbers.h"
#include "ip.h"

extern int nvram_match(char *name, char *match);
extern char *nvram_safe_get(const char *name);

typedef struct CB_ALERT_DATA {


	DYN_DNS_CLIENT	*p_dyndns;
#ifndef _WIN32
	void			*p_data;
#else
	RAS_THREAD_DATA *p_data;
#endif

} CB_ALERT_DATA;


/*
	Define this, and link with winmm.lib
	for audible alerts (see inadyn-mt/extra/wav/alarm.wav)
	on network connection loss.
*/
#ifdef USE_SNDFILE

#include "waveout.h"

#endif

static volatile	BOOL		global_is_online=true;
static volatile BOOL		is_online_thread_exit=false;
static volatile BOOL		is_alert_thread_exit=false;
static volatile BOOL		is_update_pending=false;

#ifdef USE_THREADS

/*use recently developed threads wrapper -- previous conditional compiles predominate in the present module
*/
static mutex_t		mutex_global_is_online=MUTEX_T_INIT;
static my_timer_t	update_timer;

#endif

#ifndef _WIN32

#ifdef USE_THREADS

#include <pthread.h>

static pthread_t thread_online_test=0;
static pthread_t thread_alert=0;

#endif

#else

#include "debug_service.h"
#include "unicode_util.h"

static BOOL						returnSignaled=false;
static unsigned long			thread_online_test=0;
static unsigned	long			thread_alert=0;

char get_mutex_dyn(DYN_DNS_CLIENT *p_dyndns,HANDLE *hMutex,int *is_waited);
DWORD get_mutex_wait_dyn(DYN_DNS_CLIENT *p_dyndns,HANDLE *hMutex);
int release_mutex_dyn(DYN_DNS_CLIENT *p_dyndns,HANDLE *hMutex);
static void atomic_inc(DYN_DNS_CLIENT *p_self,int *src,int inc,HANDLE hMutex);

#endif

int is_exit_requested(DYN_DNS_CLIENT *p_self);
int is_exit_requested_void(void *p_self);
int do_is_dyndns_online(DYN_DNS_CLIENT *p_self);
static int increment_iterations(DYN_DNS_CLIENT *p_dyndns);
static RC_TYPE do_handle_bad_config(DYN_DNS_CLIENT *p_self,int i);

#ifdef USE_SNDFILE
#ifdef USE_THREADS

static int is_dyndns_online(DYN_DNS_CLIENT *p_self,void *p_data);

#ifndef _WIN32
static void *is_dyndns_online_thread(void *p_data);
static void *alert_if_offline_thread(void *p_data);
#else
static void is_dyndns_online_thread(void *p_data);
static void alert_if_offline_thread(void *p_data);
#endif

#else

void alert_if_offline(DYN_DNS_CLIENT *p_dyndns,void *p_data);

#endif
#endif

#define MD5_DIGEST_BYTES (16)

/* DNS systems specific configurations*/

DYNDNS_ORG_SPECIFIC_DATA dyndns_org_dynamic = {"dyndns"};
DYNDNS_ORG_SPECIFIC_DATA dyndns_org_custom = {"custom"};
DYNDNS_ORG_SPECIFIC_DATA dyndns_org_static = {"statdns"};

static int get_req_for_dyndns_server(DYN_DNS_CLIENT *this, int nr, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_freedns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_generic_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_noip_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_easydns_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_sitelutions_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_tzo_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_he_ipv6_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_dhis_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_majimoto_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info);
static int get_req_for_zerigo_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info);

static BOOL is_dyndns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_freedns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_generic_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_zoneedit_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_easydns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_sitelutions_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_tzo_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_he_ipv6_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_dhis_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_majimoto_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);
static BOOL is_zerigo_server_rsp_ok( DYN_DNS_CLIENT *p_self, char*p_rsp, char* p_ok_string);

static BOOL is_dyndns_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_freedns_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_zoneedit_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_easydns_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_sitelutions_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_generic_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_tzo_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_he_ipv6_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_dhis_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_majimoto_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);
static BOOL is_zerigo_server_rsp_config(DYN_DNS_CLIENT *p_self, char*p_rsp);

DYNDNS_SYSTEM_INFO dns_system_table[] =
    {
		{DYNDNS_DEFAULT,
			{"default@dyndns.org", &dyndns_org_dynamic,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC)get_req_for_dyndns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},

		{DYNDNS_DYNAMIC,
			{"dyndns@dyndns.org", &dyndns_org_dynamic,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},

		{DYNDNS_CUSTOM,
			{"custom@dyndns.org", &dyndns_org_custom,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},

		{DYNDNS_STATIC,
			{"statdns@dyndns.org", &dyndns_org_static,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			DYNDNS_MY_DNS_SERVER, DYNDNS_MY_DNS_SERVER_URL, NULL}},

		{FREEDNS_AFRAID_ORG_DEFAULT,
			{"default@freedns.afraid.org", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_freedns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_freedns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_freedns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"freedns.afraid.org", "/dynamic/update.php?", NULL}},

		{ZONE_EDIT_DEFAULT,
			{"default@zoneedit.com", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_zoneedit_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_zoneedit_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_generic_http_dns_server,
			"dynamic.zoneedit.com", "/checkip.html",
			"dynamic.zoneedit.com", "/auth/dynamic.html?host=", ""}},

		{NOIP_DEFAULT,
			{"default@no-ip.com", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_noip_http_dns_server,
			"ip1.dynupdate.no-ip.com", "/",
			"dynupdate.no-ip.com", "/nic/update?hostname=", ""}},

		{EASYDNS_DEFAULT,
			{"default@easydns.com", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_easydns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_easydns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_easydns_http_dns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"members.easydns.com", "/dyn/dyndns.php?hostname=", ""}},

		{DYNDNS_3322_DYNAMIC,
			{"dyndns@3322.org", &dyndns_org_dynamic,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
			DYNDNS_3322_MY_IP_SERVER, DYNDNS_3322_MY_IP_SERVER_URL,
			DYNDNS_3322_MY_DNS_SERVER, DYNDNS_3322_MY_DNS_SERVER_URL, NULL}},

		{SITELUTIONS_DOMAIN,
			{"default@sitelutions.com", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_sitelutions_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_sitelutions_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_sitelutions_http_dns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"www.sitelutions.com", "/dnsup?", NULL}},

		{TZO_DEFAULT,
			{"default@tzo.com", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_tzo_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_tzo_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_tzo_http_dns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"cgi.tzo.com", "/webclient/signedon.html?TZOName=", NULL}},

		{DNSOMATIC_DEFAULT,
			{"default@dnsomatic.com", &dyndns_org_dynamic,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_dyndns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"updates.dnsomatic.com", "/nic/update?", NULL}},

		{HE_IPV6TB,
			{"ipv6tb@he.net", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_he_ipv6_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_he_ipv6_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_he_ipv6_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"ipv4.tunnelbroker.net", "/ipv4_end.php?", NULL}},

		/* Support for dynsip.org by milkfish, from DD-WRT */
		{DYNSIP_DEFAULT,
			{"default@dynsip.org", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dyndns_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC)get_req_for_generic_http_dns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"dynsip.org", "/nic/update?hostname=", ""}},
			
		{DHIS_DEFAULT,
			{"default@dhis.org", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_dhis_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dhis_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_dhis_http_dns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"is.dhis.org", "/?", ""}},

		{MAJIMOTO_DEFAULT,
			{"default@majimoto.net", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_majimoto_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_dyndns_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_majimoto_http_dns_server,
			DYNDNS_MY_IP_SERVER, DYNDNS_MY_IP_SERVER_URL,
			"dyndns.majimoto.net", "/nic/update?", ""}},

		{ZERIGO_DEFAULT,
			{"default@zerigo.com", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_zerigo_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_zerigo_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_zerigo_http_dns_server,
			"checkip.zerigo.com", "/",
			"update.zerigo.com", "/dynamic?host=", "OK"}},

		{CUSTOM_HTTP_BASIC_AUTH,
			{"custom@http_svr_basic_auth", NULL,
			(DNS_SYSTEM_SRV_RESPONSE_OK_FUNC)is_generic_server_rsp_ok,
			(DNS_SYSTEM_SRV_RESPONSE_CONFIG_FUNC)is_generic_server_rsp_config,
			(DNS_SYSTEM_REQUEST_FUNC) get_req_for_generic_http_dns_server,
			GENERIC_DNS_IP_SERVER_NAME, DYNDNS_MY_IP_SERVER_URL,
			"", "", "OK"}},

        {LAST_DNS_SYSTEM, {NULL, NULL, NULL, NULL, NULL, NULL}}
    };

static DYNDNS_SYSTEM* get_dns_system_by_id(DYNDNS_SYSTEM_ID id)
{
	{
		DYNDNS_SYSTEM_INFO *it;

		for (it = dns_system_table; it->id != LAST_DNS_SYSTEM; ++it)
		{
			if (it->id == id)
			{
				return &it->system;
			}
		}
	}
	return NULL;
}

DYNDNS_SYSTEM_INFO* get_dyndns_system_table(void)
{
	return dns_system_table;
}

/*************PRIVATE FUNCTIONS ******************/
static void init_cmd_timer(int *counter,int *counter_init,int *cmd_check_period_ms,DYN_DNS_CMD *old_cmd,
						   int is_update_pending,DYN_DNS_CLIENT *p_self)
{

	if (is_update_pending && !(p_self->retry_pending_off))

		*counter=p_self->retry_pending_interval / p_self->cmd_check_period;
	else
		*counter=p_self->sleep_sec / p_self->cmd_check_period;

	*counter_init=*counter;

	*cmd_check_period_ms = p_self->cmd_check_period * 1000;
	*old_cmd = p_self->cmd;
}

#ifdef USE_THREADS

static void dec_forced_update_count(DYN_DNS_CLIENT *p_self)
{

	unsigned	counter_store=p_self->forced_update_counter;
	double		timer_time;


	if (!(p_self->forced_update_counter))

		return;

	timer_time=get_timer(&update_timer);

	timer_time=round_up(timer_time/1000,p_self->forced_update_adjust);

	p_self->forced_update_counter-=((unsigned) (round_up((double) (timer_time/p_self->cmd_check_period),
		p_self->forced_update_adjust)));

	if (counter_store<p_self->forced_update_counter) /*overflow?*/

		p_self->forced_update_counter=0;
}

#endif

static RC_TYPE dyn_dns_wait_for_cmd(DYN_DNS_CLIENT *p_self)
{
	int			counter;	
	int			counter_init;
	int			cmd_check_period_ms;
	DYN_DNS_CMD	old_cmd;


	init_cmd_timer(&counter,&counter_init,&cmd_check_period_ms,&old_cmd,is_update_pending,p_self);

	if (old_cmd != NO_CMD)
	{
		return RC_OK;
	}

#ifdef USE_THREADS
	/*keeping track of time out of this routine (for forced update period) -- 
	  a bit experimental, but appears unneccessary.  Easy enough to 
	  remove though -- these two functions here, the restart below and in
	  do_update_alias_table, the create in init, and the destroy in main.  
	  Compiler option?  :-)
	*/
	stop_timer(&update_timer);

	dec_forced_update_count(p_self);
#endif

	
	while(1)
	{

#ifdef _WIN32

		if (returnSignaled)

			break;
#endif

		if (p_self->cmd != old_cmd)

			break;		

		if (p_self->forced_update_counter) /*unsigned*/

			p_self->forced_update_counter--;

		if (!(p_self->forced_update_counter)) {

			if (!(is_update_pending && !(p_self->retry_pending_off))) {

				/*
					If not retrying pendings, forced update retries fallback to update_period.

					So, this setup performs as inadyn if retry pendings off.  But, it will attempt 
					a forced update before update_period expiration if havn't tried since last update 
					and not doing pendings.  A bit overdone perhaps...the idea is to decouple ip 
					change checks (and associated interval) from required account maintenance ip 
					updates.  A failed update prompted by either update interval trigger will cause 
					mode shift to pending interval (15 minute default) if pending updates not explicitly 
					turned off.
				*/

				if (!(is_update_pending) || !(p_self->is_forced_update_attempted)) {

					if (!(p_self->is_bad_config)) {

						DBG_PRINTF((LOG_INFO,"I:DYNDNS: Command loop breaking for forced update...\n"));

						p_self->is_forced_update_attempted=true;

						break;
					}
				}
			}
		}

		os_sleep_ms(cmd_check_period_ms);

		if (!(--counter)) /*ip check*/

			break;
	}


#ifdef USE_THREADS
	/*keep track of time out of loop (for forced update period)*/

	if (p_self->forced_update_counter)

		restart_timer(&update_timer);
#endif

#ifdef _WIN32

	if (returnSignaled)

		p_self->cmd=CMD_STOP;
#endif

	return RC_OK;
}

static int get_req_for_dyndns_server(DYN_DNS_CLIENT *p_self, int cnt,DYNDNS_SYSTEM *p_sys_info)
{

	int bytes_stored=0;


	DYNDNS_ORG_SPECIFIC_DATA *p_dyndns_specific =
	    (DYNDNS_ORG_SPECIFIC_DATA*) p_sys_info->p_specific_data;

	memset(p_self->p_req_buffer,0,DYNDNS_HTTP_REQUEST_BUFFER_SIZE);


	bytes_stored=sprintf(p_self->p_req_buffer, DYNDNS_GET_MY_IP_HTTP_REQUEST_FORMAT,
	                     p_self->info.dyndns_server_url,
	                     p_dyndns_specific->p_system,
	                     p_self->alias_info.names[cnt].name,
	                     p_self->info.my_ip_address.name[ip_store],
	                     p_self->wildcard ? "ON" : "OFF",
	                     p_self->alias_info.names[cnt].name,
	                     p_self->info.dyndns_server_name.name[ip_store],
	                     p_self->info.credentials.p_enc_usr_passwd_buffer);


	return bytes_stored;
}

static int get_req_for_freedns_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, FREEDNS_UPDATE_MY_IP_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
	               p_self->alias_info.hashes[cnt].str,
				   p_self->info.my_ip_address.name[ip_store],
	               p_self->info.dyndns_server_name.name[ip_store]);
}


static int get_req_for_generic_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_DNS_BASIC_AUTH_MY_IP_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
	               p_self->alias_info.names[cnt].name,
	               p_self->info.credentials.p_enc_usr_passwd_buffer,
	               p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_noip_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_NOIP_AUTH_MY_IP_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
	               p_self->alias_info.names[cnt].name,
	               p_self->info.my_ip_address.name[ip_store],
	               p_self->info.credentials.p_enc_usr_passwd_buffer,
	               p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_easydns_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, GENERIC_EASYDNS_AUTH_MY_IP_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
	               p_self->alias_info.names[cnt].name,
	               p_self->info.my_ip_address.name[ip_store],
	               p_self->wildcard ? "ON" : "OFF",
	               p_self->info.credentials.p_enc_usr_passwd_buffer,
	               p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_tzo_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info)
{
	(void)p_sys_info;

	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	return sprintf(p_self->p_req_buffer, GENERIC_TZO_AUTH_MY_IP_REQUEST_FORMAT,
		       p_self->info.dyndns_server_url,
		       p_self->alias_info.names[cnt].name,
		       p_self->info.credentials.my_username,
		       p_self->info.credentials.my_password,
		       p_self->info.my_ip_address.name[ip_store],
		       p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_sitelutions_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info)
{

	(void)p_sys_info;
	return sprintf(p_self->p_req_buffer, SITELUTIONS_GET_MY_IP_HTTP_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
	               p_self->info.credentials.my_username,
	               p_self->info.credentials.my_password,
	               p_self->alias_info.names[cnt].name,
				   p_self->info.my_ip_address.name[ip_store],
	               p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_he_ipv6_server(DYN_DNS_CLIENT *p_self, int cnt, DYNDNS_SYSTEM *p_sys_info)
{
	unsigned char digestbuf[MD5_DIGEST_BYTES];
	char digeststr[MD5_DIGEST_BYTES*2+1];
	int i;

	(void)p_sys_info;


	memset(&digestbuf,0,MD5_DIGEST_BYTES);
	memset(&digeststr,0,MD5_DIGEST_BYTES*2+1);


	if (p_self == NULL)
	{
		/* 0 == "No characters written" */
		return 0;
	}

	md5_buffer(p_self->info.credentials.my_password,
		   strlen(p_self->info.credentials.my_password), digestbuf);

	for (i = 0; i < MD5_DIGEST_BYTES; i++)
		sprintf(&digeststr[i*2], "%02x", digestbuf[i]);

	return sprintf(p_self->p_req_buffer, HE_IPV6TB_UPDATE_MY_IP_REQUEST_FORMAT,
				p_self->info.dyndns_server_url,
				p_self->info.my_ip_address.name[ip_store],
				p_self->info.credentials.my_username,
				digeststr,
				p_self->alias_info.names[cnt].name,
				p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_dhis_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info)
{

	(void)p_sys_info;

	return sprintf(p_self->p_req_buffer, DHIS_MY_IP_UPDATE_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
				   p_self->alias_info.names[cnt].name,
				   p_self->info.credentials.my_password,
				   p_self->info.my_ip_address.name[ip_store],
				   p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_majimoto_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info)
{

	(void)p_sys_info;

	return sprintf(p_self->p_req_buffer, MAJIMOTO_MY_IP_UPDATE_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
				   p_self->alias_info.names[cnt].name,
				   p_self->info.my_ip_address.name[ip_store],
				   p_self->info.credentials.p_enc_usr_passwd_buffer,				   
				   p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_zerigo_http_dns_server(DYN_DNS_CLIENT *p_self, int cnt,  DYNDNS_SYSTEM *p_sys_info)
{

	(void)p_sys_info;

	return sprintf(p_self->p_req_buffer, ZERIGO_MY_IP_UPDATE_REQUEST_FORMAT,
	               p_self->info.dyndns_server_url,
				   p_self->alias_info.names[cnt].name,
				   p_self->info.my_ip_address.name[ip_store],
				   p_self->info.credentials.p_enc_usr_passwd_buffer,				   
				   p_self->info.dyndns_server_name.name[ip_store]);
}

static int get_req_for_ip_server(DYN_DNS_CLIENT *p_self, void *p_specific_data)
{
	(void)p_specific_data;

	return sprintf(p_self->p_req_buffer, DYNDNS_GET_MY_IP_HTTP_REQUEST,
	               p_self->info.ip_server_name.name[ip_store], p_self->info.ip_server_url);
}

RC_TYPE dyn_dns_set_online_check_dest(HTTP_CLIENT *dest,DYN_DNS_CLIENT *p_self)
{

	if (p_self->info.proxy_server_name.name[ip_store])
	{
		http_client_set_remote_name(dest,p_self->info.proxy_server_name.name[ip_store]);
		http_client_set_port(dest,p_self->info.proxy_server_name.port);
	}
	else
	{
		http_client_set_remote_name(dest,p_self->info_online_status.ip_server_name.name[ip_store]);
		http_client_set_port(dest,p_self->info_online_status.ip_server_name.port);
	}


	return RC_OK;
}

RC_TYPE dyn_dns_set_ip_server_dest(HTTP_CLIENT *dest,DYN_DNS_CLIENT *p_self)
{

	if (p_self->info.proxy_server_name.name[ip_store])
	{
		http_client_set_remote_name(dest,p_self->info.proxy_server_name.name[ip_store]);
		http_client_set_port(dest,p_self->info.proxy_server_name.port);
	}
	else
	{
		http_client_set_remote_name(dest,p_self->info.ip_server_name.name[ip_store]);
		http_client_set_port(dest,p_self->info.ip_server_name.port);
	}


	return RC_OK;
}

RC_TYPE dyn_dns_set_dyndns_server_dest(HTTP_CLIENT *dest,DYN_DNS_CLIENT *p_self)
{

	if (p_self->info.proxy_server_name.name[ip_store])
	{
		http_client_set_remote_name(dest,p_self->info.proxy_server_name.name[ip_store]);
		http_client_set_port(dest,p_self->info.proxy_server_name.port);
	}
	else
	{
		http_client_set_remote_name(dest,p_self->info.dyndns_server_name.name[ip_store]);
		http_client_set_port(dest,p_self->info.dyndns_server_name.port);
	}


	return RC_OK;
}

RC_TYPE dyn_dns_set_http_clients(DYN_DNS_CLIENT *p_self)
{

	dyn_dns_set_ip_server_dest(&p_self->http_to_ip_server,p_self);
	dyn_dns_set_dyndns_server_dest(&p_self->http_to_dyndns,p_self);


	return RC_OK;
}

static RC_TYPE test_connect(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE rc=RC_OK;
	HTTP_CLIENT *http_to_ip_server;


	http_to_ip_server=safe_malloc(sizeof(HTTP_CLIENT));
	
	rc=http_client_construct(http_to_ip_server);


	dyn_dns_set_online_check_dest(http_to_ip_server,p_self);

#ifndef USE_THREADS
	rc=http_client_init(http_to_ip_server);
#else
	rc=http_client_test_connect(http_to_ip_server,is_exit_requested_void,p_self);	
#endif

	http_client_shutdown(http_to_ip_server);
	http_client_destruct(http_to_ip_server);

	free(http_to_ip_server);


	return rc;
}

/*
    Note:
        it updates the flag: info->'my_ip_has_changed' if the old address was different 
*/
static RC_TYPE do_check_my_ip_address(DYN_DNS_CLIENT *p_self)
{

	RC_TYPE		rc=RC_OK;
	char		*p_current_str=p_self->http_tr.p_rsp;
	char		*p_ip_str;
	DYNDNS_IPV	ip_enum;


	if (p_self->http_tr.rsp_len <= 0 || p_self->http_tr.p_rsp == NULL) {

		return RC_INVALID_POINTER;
	}

	if (!((rc=parse_ip_address(&p_ip_str,p_current_str))==RC_OK)) {

		return rc;
	} 
	else {

		if ((strstr(p_ip_str,".")!=NULL))

			ip_enum=ip_4;
		else
			ip_enum=ip_6;

		p_self->info.my_ip_has_changed[ip_enum]=(strcmp(p_ip_str,p_self->info.my_ip_address.name[ip_enum])!=0);

		strcpy(p_self->info.my_ip_address.name[ip_enum],p_ip_str);

		/*put currently operated upon here -- for auto ip type too*/
		strcpy(p_self->info.my_ip_address.name[ip_store],p_ip_str);
		p_self->info.my_ip_has_changed[ip_store]=p_self->info.my_ip_has_changed[ip_enum];

		free(p_ip_str);

		return RC_OK;
	}
}

/*
	Send req to IP server and get the response

	12/2010 - bhoover@wecs.com
	extract ip address from response
	iterate over any fallback addresses
	break on first success
*/
static RC_TYPE check_my_ip_address(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE rc;
	HTTP_CLIENT *p_http;
	

	p_http = &p_self->http_to_ip_server;


	rc = http_client_init_all(&p_self->http_to_ip_server);

	if (rc != RC_OK)
	{
		http_client_shutdown(&p_self->http_to_ip_server);

		return rc;
	}

	/*return good if at least one transaction*/

	rc=RC_ERROR;

	do
	{
		/*prepare request for IP server*/
		{
			int					i=0;
			HTTP_TRANSACTION	*p_tr = &p_self->http_tr;
			struct addrinfo		*addr;
			BOOL				is_got_ip4=false;
			BOOL				is_got_ip6=false;
			BOOL				is_got_afinet=false;


			p_tr->req_len = get_req_for_ip_server((DYN_DNS_CLIENT*) p_self,
			                                      p_self->info.p_dns_system->p_specific_data);
			if (p_self->dbg.level > LOG_CRIT) {

				DBG_PRINTF((LOG_DEBUG,"The request for IP server:\n%s\n",p_self->p_req_buffer));
			}

			p_tr->p_req = (char*) p_self->p_req_buffer;
			p_tr->p_rsp = (char*) p_self->p_work_buffer;
			p_tr->max_rsp_len = p_self->work_buffer_size - 1;
			p_tr->rsp_len = 0;


			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Entering Loop.  Got %d sockets...\n",
				p_self->http_to_ip_server.super.super.server_socket_count));

			for (i=0;i<p_self->http_to_ip_server.super.super.server_socket_count;i++,
				p_self->http_to_ip_server.super.super.sock_index++) {
				
				addr=p_self->http_to_ip_server.super.super.addr_ar[i];

				/*on *nix, IPv4 precedence could mean no IPv6 routes which could cause a crash on socket send*/
				if (((is_got_afinet=(is_got_afinet || addr->ai_family==AF_INET)) && addr->ai_family==AF_INET6))

#ifndef _WIN32

					continue;
#else
					;
#endif

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "In It.  SOCKET family:  %s...\n",
					addr_family_get_name(addr->ai_family)));

				if (!(addr->ai_family==AF_INET || addr->ai_family==AF_INET6))

					continue;
					
				/*doing any ip4?*/
				if ((addr->ai_family==AF_INET) && !(p_self->info.is_update_ip4 || p_self->info.is_update_auto))

					continue;
					
				/*doing any ip6?*/
				if ((addr->ai_family==AF_INET6) && !(p_self->info.is_update_ip6 || p_self->info.is_update_auto))

					continue;

				if (!(RC_OK==(rc=http_client_transaction(&p_self->http_to_ip_server,&p_self->http_tr)))) {

					DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Failed getting %s ip from %s%s in check_my_ip_address...\n",
						addr_family_get_name(addr->ai_family),p_self->info.ip_server_name.name[ip_store],p_self->info.ip_server_url));
				}
				else {

					DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Transaction %d DONE...\n",i));

					DBG_PRINTF((LOG_INFO,"I:DYNDNS: IP server response: %s\n", p_self->p_work_buffer));


					p_self->http_tr.p_rsp[p_tr->rsp_len]=0;

					DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "GONNA PARSE...\n"));

					if (!(RC_OK==(rc=do_check_my_ip_address(p_self)))) {

						DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "ip parse failed in check_my_ip_address...\n"));
					}
					else {

						DBG_PRINTF((LOG_NOTICE,"N:DYNDNS: My IP address: %s\n", p_self->info.my_ip_address.name[ip_store]));

						p_self->info.is_got_ip4=is_got_ip4=(is_got_ip4 || (addr->ai_family==AF_INET));
						p_self->info.is_got_ip6=is_got_ip6=(is_got_ip6 || (addr->ai_family==AF_INET6));
						
						/*detect if just doing auto (based on ip server) and dump out when done*/
						if ((is_got_ip4 && is_got_ip6) || !(p_self->info.is_update_ip4 && p_self->info.is_update_ip6))

							/*support ip4 only updates, on dual ip server*/
							if ((p_self->info.is_update_ip4 && is_got_ip4) || !(p_self->info.is_update_ip4))							

								break;
					}
				}
			}			
		}
	}
	while(0);

	/*close*/
	http_client_shutdown(&p_self->http_to_ip_server);


	return rc;
}

/*
    Updates for every maintained name the property: 'update_required'.
    The property will be checked in another function and updates performed.
        
      Action:
        Check if my IP address has changed. -> ALL names have to be updated.
        Note: In the update function the property will set to false if update was successful.
*/
static RC_TYPE do_check_alias_update_table(DYN_DNS_CLIENT *p_self)
{
	int			i;
	DYNDNS_IPV	ip_v;
	BOOL		is_ip_changed;
	BOOL		is_dyndns_dual;


	for (i = 0; i < p_self->alias_info.count; ++i)
	{

		ip_v=p_self->alias_info.names[i].ip_v_enum;		

		/*ip type DUAL_LIST is dual of form, 1.1.1.1,::1 -- both ip types in one update connection*/
		if (!(is_dyndns_dual=(NULL!=strstr(p_self->alias_info.names[i].ip_v,DUAL_LIST))))

			is_ip_changed=p_self->info.my_ip_has_changed[ip_v];
		else
			is_ip_changed=(p_self->info.my_ip_has_changed[ip_4] || p_self->info.my_ip_has_changed[ip_6]);

		p_self->alias_info.update_required[i] = 

			/*ip address changed?*/
			(is_ip_changed

			/*pending failed update?*/
			|| (is_update_pending && !(p_self->alias_info.update_succeeded[i]))

			/*administrative update?*/
			|| (!(p_self->forced_update_counter)));

		if (p_self->alias_info.update_required[i]) {	
	
			if (is_dyndns_dual)

				DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "IP address for alias '%s:%s' needs update to '%s%s%s'...\n",
				        p_self->alias_info.names[i].name,p_self->alias_info.names[i].ip_v, 
						p_self->info.my_ip_address.name[ip_4],",",p_self->info.my_ip_address.name[ip_6]));


			else

				DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "IP address for alias '%s:%s' needs update to '%s'...\n",
				        p_self->alias_info.names[i].name,p_self->alias_info.names[i].ip_v, 
						p_self->info.my_ip_address.name[ip_v]));

			p_self->alias_info.update_required[i]=!(p_self->alias_info.fatal_error[i]);
		}
	}
	
	return RC_OK;
}

/* DynDNS org.specific response validator.
    'good' or 'nochange' are the good answers,

  3 conditions:
  pass/good/success/nochange, etc.
  bad config
  dyn dns server error (server side trouble)
  So, for supported servers, we'll have 2 functions to check results:
  --The present (legacy) function indicating pass/fail
  --Added function indicating whether it was config error

  'cause on config error, client should stop attempting updates
  'till config fixed.

  dyndns.org has a fourth condition -- includes "badagent", "good 127.0.0.1" -- 
  both indicating non-conforming update client:

  badagent 			The user agent was not sent or HTTP method is not permitted (we recommend use of GET request method).

  good 127.0.0.1	This answer indicates good update only when 127.0.0.1 address is requested by update. 
					In all other cases it warns user that request was ignored because of agent that does not 
					follow our specifications. 
*/
static BOOL is_dyndns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char *p_ok_string)
{
	/*fail on (badauth, nohost, notfqdn, !yours, etc)*/

	(void) p_ok_string;
	return ( (strstr(p_rsp, DYNDNS_OK_RESPONSE) != NULL) ||
	         (strstr(p_rsp, DYNDNS_OK_NOCHANGE) != NULL) );
}

static BOOL is_dyndns_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{
	return (strstr(p_rsp, "!donator")  != NULL || strstr(p_rsp, "badauth") != NULL 
			|| strstr(p_rsp, "notfqdn") != NULL || strstr(p_rsp, "nohost") != NULL 
			|| strstr(p_rsp, "numhost")  != NULL || strstr(p_rsp, "badagent") != NULL 
			|| strstr(p_rsp, "abuse") != NULL || strstr(p_rsp, "good 127.0.0.1") != NULL
			|| strstr(p_rsp, "!yours") != NULL || strstr(p_rsp, "badsys") != NULL
			|| strstr(p_rsp, "911") != NULL);
}

/* Freedns afraid.org.specific response validator.
    ok blabla and n.n.n.n
    fail blabla and n.n.n.n
    are the good answers. We search our own IP address in response and that's enough.
*/
static BOOL is_freedns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char *p_ok_string)
{

	return (((strstr(p_rsp, "ERROR") == NULL) && strstr(p_rsp, p_self->info.my_ip_address.name[ip_store]) != NULL)
			|| strstr(p_rsp, "has not changed") != NULL);	
}

static BOOL is_freedns_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	return (strstr(p_rsp,"Invalid update URL") != NULL);
}

/** generic http dns server ok parser
	parses a given string. If found is ok,
	Example : 'SUCCESS CODE='
*/
static BOOL is_generic_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char *p_ok_string)
{
	if (p_ok_string == NULL)
	{
		return FALSE;
	}
	return (strstr(p_rsp, p_ok_string) != NULL);
}

static BOOL is_generic_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	/*caveat emptor*/

	return false;
}

/**
	the OK codes are:
		CODE=200
		CODE=707, for duplicated updates
*/
BOOL is_zoneedit_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char *p_ok_string)
{
	(void) p_ok_string;

	return ((strstr(p_rsp, "CODE=\"200\"") != NULL) || (strstr(p_rsp, "CODE=\"707\"") != NULL)
			 || (strstr(p_rsp, "CODE=\"201\"") != NULL));
}

static BOOL is_zoneedit_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	return (strstr(p_rsp, "CODE=\"703\"") != NULL || strstr(p_rsp, "CODE=\"707\"") != NULL 
			|| strstr(p_rsp, "CODE=\"704\"") != NULL || strstr(p_rsp, "CODE=\"701\"") != NULL 
			|| strstr(p_rsp, "CODE=\"705\"") != NULL || strstr(p_rsp, "CODE=\"708\"") != NULL);
}

/**
	NOERROR is the OK code here
*/
BOOL is_easydns_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char *p_ok_string)
{
	return (strstr(p_rsp, "NOERROR") != NULL);
}

static BOOL is_easydns_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	return (strstr(p_rsp, "NOACCESS")  != NULL || strstr(p_rsp, "NOSERVICE")  != NULL || strstr(p_rsp, "ILLEGAL INPUT") 
			 != NULL || strstr(p_rsp, "TOOSOON") != NULL);
}

static BOOL is_sitelutions_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char* p_ok_string)
{

	return (strstr(p_rsp,"success") != NULL);
}

static BOOL is_sitelutions_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	return (strstr(p_rsp, "noauth")  != NULL || strstr(p_rsp, "invalid ip")  != NULL || strstr(p_rsp, "invalid ttl") 
			 != NULL || strstr(p_rsp, "no record")  != NULL || strstr(p_rsp, "not owner") != NULL);
}

/* HE ipv6 tunnelbroker specific response validator.
   own IP address and 'already in use' are the good answers.
*/
static BOOL is_he_ipv6_server_rsp_ok(DYN_DNS_CLIENT *p_self, char *p_rsp, char *p_ok_string)
{
	(void)p_ok_string;

	return ((strstr(p_rsp, p_self->info.my_ip_address.name[ip_store]) != NULL) ||
		(strstr(p_rsp, "already in use") != NULL));
}

/*not yet implemented
*/
static BOOL is_he_ipv6_server_rsp_config( DYN_DNS_CLIENT *p_self, char*p_rsp)
{

	return false;
}

/* TZO specific response validator.
   If we have an HTTP 302 the update wasn't good and we're being redirected 
*/
static BOOL is_tzo_server_rsp_ok(DYN_DNS_CLIENT *p_self, char *p_rsp, char *p_ok_string)
{
	
	return strstr(p_rsp,"200")  != NULL || strstr(p_rsp,"304") != NULL;
}

static BOOL is_tzo_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	return (strstr(p_rsp, "480") != NULL || strstr(p_rsp, "405")  != NULL || strstr(p_rsp, "401")  != NULL 
			|| strstr(p_rsp, "403") != NULL || strstr(p_rsp, "414") != NULL || strstr(p_rsp, "405") 
			 != NULL || strstr(p_rsp, "407") != NULL || strstr(p_rsp, "415") != NULL);
}

static BOOL is_dhis_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char* p_ok_string)
{

	return (strstr(p_rsp,p_self->info.my_ip_address.name[ip_store]) != NULL);
}

/*not yet implemented -- at moment, this seems only failure returned
*/
static BOOL is_dhis_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	return !(strstr(p_rsp,"Authorization Required")==NULL);
}

/*the quiet type*/
static BOOL is_majimoto_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char* p_ok_string)
{

	return (!(strlen(p_rsp)));
}

/*not yet implemented -- there *are* relevant return codes on error --
  presently, use dyndns server config fuction
*/
static BOOL is_majimoto_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{

	return false;
}

static BOOL is_zerigo_server_rsp_ok( DYN_DNS_CLIENT *p_self, char *p_rsp, char* p_ok_string)
{
	return (strstr(p_rsp,"Status: 200")!=NULL);
}

static BOOL is_zerigo_server_rsp_config( DYN_DNS_CLIENT *p_self, char *p_rsp)
{
	return (strstr(p_rsp,"Status: 400")!=NULL || strstr(p_rsp,"Status: 403")!=NULL || strstr(p_rsp,"Status: 404")!=NULL);
}

static RC_TYPE update_update_state(DYN_DNS_CLIENT *p_self,int updates_needed,int success_updates,
										  int config_fails,RC_TYPE *rc)
{
	FILE *fp;


	if (config_fails==p_self->alias_info.count)

		*rc=RC_DYNDNS_RSP_CONFIG;

	else {

		if (success_updates) {

			/*reset forced update period*/
			p_self->forced_update_counter=p_self->forced_update_period_sec_orig/p_self->cmd_check_period;
			p_self->forced_update_period_sec=p_self->forced_update_period_sec_orig;			

			if ((fp=utf_fopen(p_self->time_cache, "w"))) {

				fprintf(fp,"%ld",time(NULL));

				fclose(fp);
			}

			if ((fp=utf_fopen(p_self->ip_cache, "w"))) {

				int			i;
				DYNDNS_IPV	ip_enum;
				BOOL		is_dyndns_dual;
				
				for (i=0;i<p_self->alias_info.count;i++) {

					is_dyndns_dual=(NULL!=strstr(p_self->alias_info.names[i].ip_v,DUAL_LIST));

					ip_enum=p_self->alias_info.names[i].ip_v_enum;

					if (p_self->alias_info.update_succeeded[i]) { 						

						if (!(is_dyndns_dual))

							fprintf(fp,"%s %s%s%s\n",p_self->info.my_ip_address.name[ip_enum],
								p_self->alias_info.names[i].name,":",p_self->alias_info.names[i].ip_v);

						else {

							fprintf(fp,"%s %s%s%s\n",p_self->info.my_ip_address.name[ip_4],
								p_self->alias_info.names[i].name,":","ip4");

							fprintf(fp,"%s %s%s%s\n",p_self->info.my_ip_address.name[ip_6],
								p_self->alias_info.names[i].name,":","ip6");
						}
					}
				}

				fclose(fp);
			}
		}
	
		/*any pending?*/
		if (!(updates_needed && !(success_updates==updates_needed))) {

			/*no pendings*/
			if (success_updates) {

				p_self->is_forced_update_attempted=false;

				*rc=RC_OK;
			}
		}
		else {

			if (*rc==RC_OK) /*not break after http client init*/
			
				*rc=RC_DYNDNS_RSP_NOTOK;

			DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "One or more (%d) alias updates failed...\n",
						updates_needed-success_updates));
		}
	}

	return *rc;
}

static RC_TYPE do_update_alias_table(DYN_DNS_CLIENT *p_self,char *is_forced_update_reset)
{
	int		i;
	int		success_updates=0;	
	int		update_ok=false;
	int		config_fails=0;
	int		updates_needed=p_self->alias_info.count;
	BOOL	is_ipv4;

	RC_TYPE rc = RC_OK;


	do
	{
		for (i = 0; i < p_self->alias_info.count; ++i) {

			if (!(p_self->alias_info.update_required[i])) {

				if (p_self->alias_info.fatal_error[i]) {

					do_handle_bad_config(p_self,i);

					config_fails++;
				}

				updates_needed--;
			}
			else {

				/*dual stack with ip's updated in comma delimited list fashion?*/
				if (NULL==(strstr(p_self->alias_info.names[i].ip_v,DUAL_LIST))) {

					if (!(ip_store==p_self->alias_info.names[i].ip_v_enum))

						strcpy(p_self->info.my_ip_address.name[ip_store],
								p_self->info.my_ip_address.name[p_self->alias_info.names[i].ip_v_enum]);
				}
				else {

					strcat(strcat(strcpy(p_self->info.my_ip_address.name[ip_store],p_self->info.my_ip_address.name[ip_4]),
							","),p_self->info.my_ip_address.name[ip_6]);

					if (!(p_self->info.is_got_ip4 && p_self->info.is_got_ip6)) {

						p_self->alias_info.update_succeeded[i]=false;

						DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Skipping partially empty address (%s) update of alias "\
							"%s\n",p_self->info.my_ip_address.name[ip_store],p_self->alias_info.names[i].name));

						continue;
					}
				}

				if (!(strlen(p_self->info.my_ip_address.name[ip_store]))) {

					p_self->alias_info.update_succeeded[i]=false;

					DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Skipping empty address update of alias %s\n",p_self->alias_info.names[i].name));

					continue;
				}

				is_ipv4=(p_self->alias_info.names[i].ip_v_enum==ip_4);

				/*bind to dynamic dns server according to address type to be updated*/
				http_client_set_is_ipv4(&p_self->http_to_dyndns,is_ipv4);

				if (!((rc=http_client_init(&p_self->http_to_dyndns))==RC_OK)) {

					DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Init error:  %s updating alias %s\n",errorcode_get_name(rc),p_self->alias_info.names[i].name));

					p_self->alias_info.update_succeeded[i]=false;
				}
				else {

					/*build dyndns transaction*/
					{
						HTTP_TRANSACTION http_tr;
						
						http_tr.req_len = p_self->info.p_dns_system->p_dns_update_req_func(
							                  (struct _DYN_DNS_CLIENT*) p_self,i,
								              (struct DYNDNS_SYSTEM*) p_self->info.p_dns_system);
						http_tr.p_req = (char*) p_self->p_req_buffer;
						http_tr.p_rsp = (char*) p_self->p_work_buffer;
						http_tr.max_rsp_len = p_self->work_buffer_size - 1;/*save place for a \0 at the end*/
						http_tr.rsp_len = 0;
						p_self->p_work_buffer[http_tr.rsp_len+1] = 0;				

						/*send it*/
						rc = http_client_transaction(&p_self->http_to_dyndns, &http_tr);
						http_tr.p_rsp[http_tr.rsp_len]=0;

						if (p_self->dbg.level > 2)
						{
							p_self->p_req_buffer[http_tr.req_len] = 0;
							DBG_PRINTF((LOG_DEBUG,"DYNDNS my Request:\n%s\n", p_self->p_req_buffer));
						}

						/*error in send?*/
						if (!(rc == RC_OK))

							p_self->alias_info.update_succeeded[i]=false;

						else {

							update_ok =
								p_self->info.p_dns_system->p_rsp_ok_func((struct _DYN_DNS_CLIENT*)p_self,
									    http_tr.p_rsp,
										p_self->info.p_dns_system->p_success_string);

							if (update_ok)
							{
								success_updates++;

#ifdef USE_THREADS
								if (!(*is_forced_update_reset)) {

									restart_timer(&update_timer);

									*is_forced_update_reset=true;
								}
#endif
								p_self->alias_info.update_required[i]=false;
								p_self->alias_info.update_succeeded[i]=true;

								DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Alias '%s' to IP '%s' updated successfully.\n",
											p_self->alias_info.names[i].name,
											p_self->info.my_ip_address.name[ip_store]));

								if (p_self->external_command)

									os_shell_execute(p_self->external_command);
							}
							else {

								p_self->alias_info.update_succeeded[i]=false;

								if (!(p_self->info.p_dns_system->p_rsp_config_func((struct _DYN_DNS_CLIENT*)p_self,http_tr.p_rsp))) {

									DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error updating alias %s\n",p_self->alias_info.names[i].name));
								} 
								else {

									DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Error validating DYNDNS svr answer. Check usr,pass,hostname!\n"));

									p_self->alias_info.fatal_error[i]=TRUE;

									DBG_PRINTF((LOG_CRIT,"C:" MODULE_TAG "\n"\
												ERROR_FLAG \
												"Fatal dyndns server update error for "\
												"alias, %s.\nThis client should be stopped and corrected for "\
												"configuration errors, and restarted...\n" \
												ERROR_FLAG,p_self->alias_info.names[i].name));

									config_fails++;
									updates_needed--;
								}
							}

							if (p_self->dbg.level > LOG_CRIT)
							{
								http_tr.p_rsp[http_tr.rsp_len] = 0;
								DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "DYNDNS Server response:\n%s\n", http_tr.p_rsp));
							}
						}
					}
				}

				{
					RC_TYPE rc2 = http_client_shutdown(&p_self->http_to_dyndns);

					if (!(rc2==RC_OK))

						DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "http client possibly improper shutdown, possible memory leak, system instability...\n"));

				}				

				if (i<p_self->alias_info.count-1)

					os_sleep_ms(1000);
			}

			/*reset for loop*/
			rc=RC_OK;
		}
	}
	while(0);

	return update_update_state(p_self,updates_needed,success_updates,config_fails,&rc);
}

RC_TYPE get_default_config_data(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE	rc = RC_OK;
	int		prefix_len;
	int		ip_len;
	int		time_len;

	do
	{
		p_self->info.p_dns_system = get_dns_system_by_id(DYNDNS_MY_DNS_SYSTEM);
		if (p_self->info.p_dns_system == NULL)
		{
			rc = RC_DYNDNS_INVALID_DNS_SYSTEM_DEFAULT;
			break;
		}

		/*forced update period*/
		p_self->forced_update_period_sec = DYNDNS_MY_FORCED_UPDATE_PERIOD_S;
		p_self->forced_update_period_sec_orig = DYNDNS_MY_FORCED_UPDATE_PERIOD_S;

		p_self->forced_update_adjust = DYNDNS_DEFAULT_UPDATE_ADJUST;

		/*network comm retries*/
		p_self->net_retries = DYNDNS_DEFAULT_NET_RETRIES;
		p_self->retry_interval = DYNDNS_DEFAULT_RETRY_INTERVAL;
		p_self->retry_pending_interval = DYNDNS_DEFAULT_RETRY_PENDING_INTERVAL;

		prefix_len=strlen(DYNDNS_DEFAULT_CACHE_PREFIX);
		ip_len=strlen(DYNDNS_DEFAULT_IP_FILE)+1;
		time_len=strlen(DYNDNS_DEFAULT_TIME_FILE)+1;
#ifdef UNIX_OS
		p_self->ip_cache=safe_malloc(prefix_len+ip_len);
		sprintf(p_self->ip_cache, "%s%s", DYNDNS_DEFAULT_CACHE_PREFIX, DYNDNS_DEFAULT_IP_FILE);

		p_self->time_cache=safe_malloc(prefix_len+time_len);
		sprintf(p_self->time_cache, "%s%s", DYNDNS_DEFAULT_CACHE_PREFIX, DYNDNS_DEFAULT_TIME_FILE);
#endif
#ifdef _WIN32
		p_self->ip_cache=safe_malloc(ip_len);
		p_self->time_cache=safe_malloc(time_len);

		sprintf(p_self->ip_cache, "%s", DYNDNS_DEFAULT_IP_FILE);
		sprintf(p_self->time_cache, "%s", DYNDNS_DEFAULT_TIME_FILE);
#endif
		/*update period*/
		p_self->sleep_sec = DYNDNS_DEFAULT_SLEEP;

		p_self->audible_off = DYNDNS_DEFAULT_INAUDIBLE;
		p_self->alert_retries = DYNDNS_DEFAULT_ALERT_RETRIES;
		p_self->alert_interval = DYNDNS_DEFAULT_ALERT_INTERVAL;
		p_self->status_interval = DYNDNS_DEFAULT_STATUS_INTERVAL*1000;
		p_self->status_offline_interval = DYNDNS_DEFAULT_STATUS_OFFLINE_INTERVAL*1000;
		p_self->wave_loops = DYNDNS_DEFAULT_WAVE_LOOPS;

#ifdef USE_SNDFILE

		set_wave_gain(p_self,DYNDNS_DEFAULT_USER_WAVE_GAIN);

		searchedProgFile(NULL,&(p_self->wave_file),"inadyn-mt","extra" DIR_DELIM_STR "wav",4,DYNDNS_DEFAULT_WAVE_FILE);


		if (!(p_self->wave_file)) {

			DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Could not open default wave file, extra" DIR_DELIM_STR "wav" \
						DIR_DELIM_STR "alarm.wav.  Using default, ." DIR_DELIM_STR DYNDNS_DEFAULT_WAVE_FILE "...\n"));

			p_self->wave_file=safe_malloc(strlen(DYNDNS_DEFAULT_WAVE_FILE) + 1);
			strcpy(p_self->wave_file,DYNDNS_DEFAULT_WAVE_FILE);
		}	

		/*
			default:  tell wave routines 
			use quarter bytes/sec buffer
		*/
		p_self->wave_buff_factor=DYNDNS_DEFAULT_WAVE_BUFF_FACTOR;
#endif

		p_self->dbg.level=DYNDNS_DEFAULT_DEBUG_LEVEL;
	}
	while(0);

	return rc;
}

static RC_TYPE get_encoded_user_passwd(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE			rc=RC_OK;
	char			*str_uri_encoded;
	const char		*format="%s:%s";
	char			*p_tmp_buff=NULL;
	int				size;
	int				actual_len;


	if (!(p_self->info.credentials.my_password) && !(p_self->info.credentials.my_username))

		return RC_OK;

	size=strlen(format)+1;

	if (!(p_self->info.credentials.my_password))

		p_self->info.credentials.my_password=safe_malloc(1);

	size+=strlen(p_self->info.credentials.my_password);

	if (!(p_self->info.credentials.my_username))

		p_self->info.credentials.my_username=safe_malloc(1);

	size+=strlen(p_self->info.credentials.my_username);

	do
	{
		p_tmp_buff = (char *) safe_malloc(size);
		if (p_tmp_buff == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		actual_len = sprintf(p_tmp_buff, format,
		                     p_self->info.credentials.my_username,
		                     p_self->info.credentials.my_password);

		if (actual_len >= size)
		{
			rc = RC_OUT_BUFFER_OVERFLOW;
			break;
		}

		/*encode*/

		p_self->info.credentials.p_enc_usr_passwd_buffer =
		    b64encode(utf_8_uri_encoded(&str_uri_encoded,p_tmp_buff,"&#",";"));

		free(str_uri_encoded);

		p_self->info.credentials.encoded =
		    (p_self->info.credentials.p_enc_usr_passwd_buffer != NULL);

		p_self->info.credentials.size = strlen(p_self->info.credentials.p_enc_usr_passwd_buffer);
	}
	while(0);

	if (p_tmp_buff != NULL)
	{
		free(p_tmp_buff);
	}

	return rc;
}

/*************PUBLIC FUNCTIONS ******************/

/*
	printout
*/
void dyn_dns_print_hello(void*p)
{
	(void) p;

	DBG_PRINTF((LOG_SYSTEM, "S:" MODULE_TAG "Started 'inadyn-mt version %s' - dynamic DNS updater.\n", DYNDNS_VERSION_STRING));
}

/*
	 basic resource allocations for the dyn_dns object
*/
RC_TYPE dyn_dns_construct(DYN_DNS_CLIENT **pp_self)
{
	RC_TYPE rc;
	DYN_DNS_CLIENT *p_self;
	BOOL http_to_dyndns_constructed = FALSE;
	BOOL http_to_ip_constructed = FALSE;

	if (pp_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	/*alloc space for me*/
	*pp_self = (DYN_DNS_CLIENT *) safe_malloc(sizeof(DYN_DNS_CLIENT));
	if (*pp_self == NULL)
	{
		return RC_OUT_OF_MEMORY;
	}

	do
	{
		p_self = *pp_self;
		memset(p_self, 0, sizeof(DYN_DNS_CLIENT));

		/*alloc space for http_to_ip_server data*/
		p_self->work_buffer_size = DYNDNS_HTTP_RESPONSE_BUFFER_SIZE;
		p_self->p_work_buffer = (char*) safe_malloc(p_self->work_buffer_size);
		if (p_self->p_work_buffer == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		/*alloc space for request data*/
		p_self->req_buffer_size = DYNDNS_HTTP_REQUEST_BUFFER_SIZE;
		p_self->p_req_buffer = (char*) safe_malloc(p_self->req_buffer_size);
		if (p_self->p_req_buffer == NULL)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}


		rc = http_client_construct(&p_self->http_to_ip_server);
		if (rc != RC_OK)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}

		p_self->info.my_ip_address.name[ip_store]=safe_malloc(DYNDNS_MY_IP_ADDRESS_LENGTH);
		p_self->info.my_ip_address.name[ip_4]=safe_malloc(DYNDNS_MY_IP_ADDRESS_LENGTH);
		p_self->info.my_ip_address.name[ip_6]=safe_malloc(DYNDNS_MY_IP_ADDRESS_LENGTH);

		http_to_ip_constructed = TRUE;

		rc = http_client_construct(&p_self->http_to_dyndns);
		if (rc != RC_OK)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}
		http_to_dyndns_constructed = TRUE;

		(p_self)->cmd = NO_CMD;
		(p_self)->sleep_sec = DYNDNS_DEFAULT_SLEEP;
		(p_self)->total_iterations = DYNDNS_DEFAULT_ITERATIONS;
		(p_self)->initialized = FALSE;

		p_self->info.credentials.p_enc_usr_passwd_buffer = NULL;

		p_self->lang_file = NULL;

	}
	while(0);

	if (rc != RC_OK)
	{
		if (*pp_self)
		{
			free(*pp_self);
		}
		if (p_self->p_work_buffer != NULL)
		{
			free(p_self->p_work_buffer);
		}
		if (p_self->p_req_buffer != NULL)
		{
			free (p_self->p_work_buffer);
		}
		if (http_to_dyndns_constructed)
		{
			http_client_destruct(&p_self->http_to_dyndns);
		}
		if (http_to_ip_constructed)
		{
			http_client_destruct(&p_self->http_to_ip_server);
		}
		if (p_self->info.my_ip_address.name[ip_store])
		{
			free(p_self->info.my_ip_address.name[ip_store]);
		}
		if (p_self->info.my_ip_address.name[ip_4])
		{
			free(p_self->info.my_ip_address.name[ip_4]);
		}
		if (p_self->info.my_ip_address.name[ip_6])
		{
			free(p_self->info.my_ip_address.name[ip_6]);
		}
	}

	return RC_OK;
}


/*
	Resource free.
*/	
RC_TYPE dyn_dns_destruct(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE rc;
	if (p_self == NULL)
	{
		return RC_OK;
	}

	if (p_self->initialized == TRUE)
	{
		dyn_dns_shutdown(p_self);
	}

	rc = http_client_destruct(&p_self->http_to_ip_server);
	if (rc != RC_OK)
	{

	}

	rc = http_client_destruct(&p_self->http_to_dyndns);
	if (rc != RC_OK)
	{

	}

	if (p_self->p_work_buffer != NULL)
	{
		free(p_self->p_work_buffer);
		p_self->p_work_buffer = NULL;
	}

	if (p_self->p_req_buffer != NULL)
	{
		free(p_self->p_req_buffer);
		p_self->p_req_buffer = NULL;
	}

	if (p_self->info.credentials.p_enc_usr_passwd_buffer != NULL)
	{
		free(p_self->info.credentials.p_enc_usr_passwd_buffer);
		p_self->info.credentials.p_enc_usr_passwd_buffer = NULL;
	}

	if (p_self->lang_file != NULL)
	{

		free(p_self->lang_file);
		p_self->lang_file = NULL;
	}

	if (p_self->dbg.p_logfilename != NULL)
	{

		free(p_self->dbg.p_logfilename);
		p_self->dbg.p_logfilename = NULL;
	}

	if (p_self->ip_cache != NULL)
	{
		free(p_self->ip_cache);
		p_self->ip_cache=NULL;
	}

	if (p_self->time_cache != NULL)
	{
		free(p_self->time_cache);
		p_self->time_cache=NULL;
	}

	if (p_self->info.credentials.my_username != NULL)
	{

		free(p_self->info.credentials.my_username);
		p_self->info.credentials.my_username=NULL;
	}

	if (p_self->info.credentials.my_password != NULL)
	{

		free(p_self->info.credentials.my_password);
		p_self->info.credentials.my_password=NULL;
	}

	{
		int i=0;


		while (1) {

			if (!(p_self->alias_info.names[i].name != NULL))

				break;
			else
			{

				free(p_self->alias_info.names[i].name);
				p_self->alias_info.names[i].name=NULL;
			}
		}
	}

	if (p_self->info.ip_server_url != NULL)
	{

		free(p_self->info.ip_server_url);
		p_self->info.ip_server_url=NULL;
	}

	if (p_self->info.dyndns_server_url != NULL)
	{

		free(p_self->info.dyndns_server_url);
		p_self->info.dyndns_server_url=NULL;
	}

	if (p_self->external_command != NULL)
	{

		free(p_self->external_command);
		p_self->external_command=NULL;
	}

	if (p_self->info.proxy_server_name.name[ip_store] != NULL)
	{

		free(p_self->info.proxy_server_name.name[ip_store]);
		p_self->info.proxy_server_name.name[ip_store]=NULL;
	}

	if (p_self->info.ip_server_name.name[ip_store] != NULL)
	{

		free(p_self->info.ip_server_name.name[ip_store]);
		p_self->info.ip_server_name.name[ip_store]=NULL;
	}

	if (p_self->info.my_ip_address.name[ip_store] != NULL)
	{

		free(p_self->info.my_ip_address.name[ip_store]);
		p_self->info.my_ip_address.name[ip_store]=NULL;
	}

	if (p_self->info.my_ip_address.name[ip_4] != NULL)
	{

		free(p_self->info.my_ip_address.name[ip_4]);
		p_self->info.my_ip_address.name[ip_4]=NULL;
	}

	if (p_self->info.my_ip_address.name[ip_6] != NULL)
	{

		free(p_self->info.my_ip_address.name[ip_6]);
		p_self->info.my_ip_address.name[ip_6]=NULL;
	}

	free(p_self);
	p_self = NULL;

	return RC_OK;
}

/*
	Sets up the object.
	- sets the IPs of the DYN DNS server
    - if proxy server is set use it! 
	- ...
*/
RC_TYPE do_dyn_dns_init(DYN_DNS_CLIENT *p_self)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->initialized == TRUE)
	{
		return RC_OK;
	}

	p_self->abort_on_network_errors = FALSE;
	p_self->force_addr_update = FALSE;

	dyn_dns_set_http_clients(p_self);

	p_self->cmd = NO_CMD;
	if (p_self->cmd_check_period == 0)
	{
		p_self->cmd_check_period = DYNDNS_DEFAULT_CMD_CHECK_PERIOD;
	}

	p_self->initialized = TRUE;
	return RC_OK;
}

RC_TYPE dyn_dns_init(DYN_DNS_CLIENT *p_self)
{
	RC_TYPE	rc;

	if (!((rc=do_dyn_dns_init(p_self))==RC_OK))
		
		return rc;

	p_self->forced_update_counter=p_self->forced_update_period_sec/p_self->cmd_check_period;

	return RC_OK;
}

/*
	Disconnect and some other clean up.
*/
RC_TYPE dyn_dns_shutdown(DYN_DNS_CLIENT *p_self)
{
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->initialized == FALSE)
	{
		return RC_OK;
	}

	return RC_OK;
}

static RC_TYPE do_handle_bad_config(DYN_DNS_CLIENT *p_self,int i)
{

	/*might want to sound an error specific alert too*/

	if (i<p_self->alias_info.count)

		DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "\n"\
					ERROR_FLAG \
					"Skipping IP update attempt subsequent to "\
					"fatal dyndns server update error for "\
					"alias, %s.\nThis client should be stopped and corrected for "\
					"configuration errors, and restarted...\n" \
					ERROR_FLAG,p_self->alias_info.names[i].name));
	else

		DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "\n"\
					ERROR_FLAG \
					"Skipping IP update attempt subsequent to "\
					"fatal dyndns server update error.  "\
					"\nThis client should be stopped and corrected for "\
					"configuration errors, and restarted...\n" \
					ERROR_FLAG));

	return RC_ERROR;
}

static RC_TYPE dyn_dns_handle_bad_config(DYN_DNS_CLIENT *p_self)
{

	/*ip update subsequent dns server return bad config related error*/

	int	i;

	for (i=0;i<p_self->alias_info.count;i++) {

		if (p_self->alias_info.fatal_error[i])

			break;
	}

	return do_handle_bad_config(p_self,i);
}

/*
	- increment the forced update times counter
	- detect current IP
		- connect to an HTTP server 
		- parse the response for IP addr

	- for all the names that have to be maintained
		- get the current DYN DNS address from DYN DNS server
		- compare and update if neccessary
*/
RC_TYPE dyn_dns_update_ip(DYN_DNS_CLIENT *p_self)
{

	RC_TYPE		rc=RC_ERROR;
	int			is_exit=false;
	int			net_attempts=1; /*use same retries parameter for both of these*/
	int			ip_attempts=1;
	char		is_forced_update_reset=0;


	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->is_bad_config)

		return dyn_dns_handle_bad_config(p_self);

	do
	{

		if (nvram_match("ddns_wan_ip","1"))
		{
		char new_ip_str[32];
		int wan_link = check_wan_link(0);
		char *wan_ipaddr = NULL;
		if (nvram_match("wan_proto", "pptp")) {
			wan_ipaddr =
			    wan_link ? nvram_safe_get("pptp_get_ip") :
			    nvram_safe_get("wan_ipaddr");
		} else if (!strcmp(nvram_safe_get("wan_proto"), "pppoe")) {
			wan_ipaddr =
			    wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
		} else if (!strcmp(nvram_safe_get("wan_proto"), "3g")) {
			wan_ipaddr =
			    wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
		} else if (nvram_match("wan_proto", "l2tp")) {
			wan_ipaddr =
			    wan_link ? nvram_safe_get("l2tp_get_ip") :
			    nvram_safe_get("wan_ipaddr");
		} else if (nvram_match("wan_proto", "disabled")) {
			wan_ipaddr = "0.0.0.0";
		} else {
			wan_ipaddr = nvram_safe_get("wan_ipaddr");
		}
		if (!strcmp(wan_ipaddr,"0.0.0.0")) {
			DBG_PRINTF((LOG_WARNING,"W:DYNDNS: Error: device has no WAN Address...\n"));
			rc = RC_ERROR; 
			is_exit=true;
			break;
		    }
		strcpy(new_ip_str,wan_ipaddr);
		p_self->info.my_ip_has_changed[ip_store] = (strcmp(new_ip_str, p_self->info.my_ip_address.name[ip_store]) != 0);
		strcpy(p_self->info.my_ip_address.name[ip_store], new_ip_str);
		rc = RC_OK;
		}else{

		DBG_PRINTF((LOG_INFO,"I:DYNDNS: dyn_dns_update_ip entering connect loop...\n"));

		while (!(is_exit=(is_exit_requested(p_self)))) {

			if (!(do_is_dyndns_online(p_self))) {

				if ((is_exit=(p_self->net_retries<net_attempts++))) {

					is_update_pending=true;

					break;
				}

				DBG_PRINTF((LOG_WARNING,"W:DYNDNS: Online failed getting ip...iterating after retry interval...\n"));
			}
			else {

				/*ask IP server something so will respond and give me my IP */
				rc = check_my_ip_address(p_self);

				if (rc==RC_OK)

					break;

				DBG_PRINTF((LOG_WARNING,"W:DYNDNS: Failed checking current ip...\n"));

				if (p_self->net_retries<ip_attempts++) {

					is_update_pending=true;

					break;
				}

				DBG_PRINTF((LOG_WARNING,"W:DYNDNS:  Iterating after retry interval...\n"));
			}

			sleep_lightly_ms(p_self->retry_interval,&is_exit_requested_void,p_self);
		}
		}
		if (is_exit) {

			DBG_PRINTF((LOG_INFO,"I:DYNDNS: Exit requested or not online in dyn_dns_update_ip...\n"));
		}
		else {

			if (rc != RC_OK)
			{
				DBG_PRINTF((LOG_WARNING,"W:DYNDNS: Error '%s' (0x%x) when talking to IP server\n",
				            errorcode_get_name(rc), rc));
				break;
			}

			DBG_PRINTF((LOG_INFO,"I:DYNDNS: dyn_dns_update_ip checking alias table...\n"));

			/*step through aliases list, resolve them and check if they point to my IP*/
			rc = do_check_alias_update_table(p_self);
			if (rc != RC_OK)
			{
				break;
			}
			net_attempts=1;
			ip_attempts=1;

			DBG_PRINTF((LOG_INFO,"I:DYNDNS: dyn_dns_update_ip entering update loop...\n"));

			while(!(is_exit_requested(p_self))) {
				
				if (!(do_is_dyndns_online(p_self))) {

					if (p_self->net_retries<net_attempts++) {

						is_update_pending=true;

						break;
					}

					DBG_PRINTF((LOG_WARNING,"W:DYNDNS: Online failed updating alias table...iterating after retry interval...\n"));
				}
				else {

					/*update IPs marked as not identical with my IP*/
					rc = do_update_alias_table(p_self,&is_forced_update_reset);

					if (rc==RC_OK) {

						is_update_pending=false;

						break;
					}

					DBG_PRINTF((LOG_WARNING,"W:DYNDNS: Failed updating alias table...\n"));

					if (p_self->net_retries<ip_attempts++) {

						is_update_pending=(!(rc==RC_DYNDNS_RSP_CONFIG));						
						
						break;
					}

					if (rc==RC_DYNDNS_RSP_CONFIG) { /*might want to trigger alert here*/

						is_update_pending=false;

						break;
					}

					DBG_PRINTF((LOG_WARNING,"W:DYNDNS:  Iterating after retry interval...\n"));
				}

				sleep_lightly_ms(p_self->retry_interval,&is_exit_requested_void,p_self);
			}
		}
	}
	while(0);

	return rc;
}

int do_is_dyndns_online(DYN_DNS_CLIENT *p_self)
{

	return (test_connect(p_self)==RC_OK);
}

int is_exit_requested(DYN_DNS_CLIENT *p_self)
{

	/*
		Check whether Windows service requested exit or
		user signaled.
	*/

#ifndef _WIN32

	return (p_self->cmd==CMD_STOP);
#else
	return ((returnSignaled) || (p_self->cmd==CMD_STOP));	
#endif

}

int is_exit_requested_void(void *p_self)
{

	return is_exit_requested((DYN_DNS_CLIENT *) p_self);
}

int is_dyndns_online(DYN_DNS_CLIENT *p_self,void *p_data)
{

#ifdef USE_THREADS

	BOOL	is_online;


	get_mutex(&mutex_global_is_online);

	is_online=global_is_online;

	release_mutex(&mutex_global_is_online);

	return is_online;
#else

	return do_is_dyndns_online(p_self);
#endif

}

int alert_abort_cond(void *p_data)
{

	CB_ALERT_DATA	*p_online_data=(CB_ALERT_DATA *) p_data;


	return is_dyndns_online(p_online_data->p_dyndns,p_online_data->p_data) || is_exit_requested(p_online_data->p_dyndns);
}

#ifdef USE_SNDFILE

int sound_abort_cond(WAVE_PARAMS *p_data)
{

	CB_ALERT_DATA	*p_this_data=(CB_ALERT_DATA *) (p_data->cb_client_data);


	return (alert_abort_cond(p_data->cb_client_data) || is_exit_requested(p_this_data->p_dyndns));
}

void alert_if_offline(DYN_DNS_CLIENT *p_dyndns,void *p_data)
{

	CB_ALERT_DATA		alert_cond_param;
	char				*waveout=NULL;
	WAVE_PARAMS			*wave_params;

#ifndef USE_THREADS
	int				attempts=0;
#endif


	memset(&alert_cond_param,0,sizeof(CB_ALERT_DATA));


	if (p_dyndns->audible_off)

		return;


	alert_cond_param.p_data=p_data;
	alert_cond_param.p_dyndns=p_dyndns;


	while (1) {	

		if (is_alert_thread_exit) {

			DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "is_alert_thread_exit "\
						"true in alert_if_offline.  Exiting alert loop...\n"));

			break;
		}

		if (is_exit_requested(p_dyndns)) {

			DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "is_exit_requested returned true in alert_if_offline.  "\
						"Exiting alert loop...\n"));

			break;
		}

		if (is_dyndns_online(p_dyndns,p_data)) {

			DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "is_dyndns_online returned true in alert_if_offline.  "\
						"Exiting alert loop...\n"));

			break;
		}

#ifndef USE_THREADS

		if (p_dyndns->alert_retries<++attempts)
#endif

		{
			/*future implementation may not preclude parameters changing*/

			waveout=safe_malloc(strlen(p_dyndns->wave_file)+1);
			strcpy(waveout,p_dyndns->wave_file);

			wave_params=safe_malloc(sizeof(WAVE_PARAMS));

			wave_params->buff_factor=p_dyndns->wave_buff_factor;
			wave_params->loops=p_dyndns->wave_loops;
			wave_params->gain=p_dyndns->wave_gain;
			wave_params->wave_file=waveout;
			wave_params->cb_client_data=&alert_cond_param;
			wave_params->p_func=sound_abort_cond;				


			do {

				DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Entering sound_play...\n"));

				play_wave1(wave_params);


				DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Returned from sound_play...\n"));

				if (is_alert_thread_exit) {

					break;
				}

				if (is_exit_requested(p_dyndns)) {

					DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "is_exit_requested returned true in alert_if_offline.  "\
								"Exiting alert loop...\n"));

					break;
				}

				if (is_dyndns_online(p_dyndns,p_data)) {

					DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "is_dyndns_online returned true in alert_if_offline.  "\
								"Exiting alert loop...\n"));

					break;
				}

				sleep_lightly_ms(p_dyndns->alert_interval,&alert_abort_cond,&alert_cond_param);
				

			} while (1);

			free(wave_params);
			free(waveout);
		}		
	}
}

#endif

#ifdef USE_THREADS

static void exit_alert_thread()
{

	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Entered exit_alert_thread.  Setting return flag...\n"));

	is_alert_thread_exit=TRUE;

	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Joining alert thread:  %d...\n",thread_alert));

#ifndef _WIN32	

	if (thread_alert)

		pthread_join(thread_alert,NULL);		

#else

	if (thread_alert)

		WaitForSingleObject((HANDLE) thread_alert,INFINITE);
#endif

	thread_alert=0;
}

static void exit_online_thread()
{


	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Entered exit_online_thread.  Setting return flag...\n"));

	is_online_thread_exit=TRUE;
		
	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Joining online test thread:  %d...\n",thread_online_test));

#ifndef _WIN32	

	if (thread_online_test)

		pthread_join(thread_online_test,NULL);	
#else

	if (thread_online_test)

		WaitForSingleObject((HANDLE) thread_online_test,INFINITE);
#endif

	thread_online_test=0;
}

static void exit_online_test_threads()
{

	exit_online_thread();
	exit_alert_thread();
}

int thread_exit_cond(void *p_data)
{

	return is_online_thread_exit || is_exit_requested((DYN_DNS_CLIENT *) p_data);
}

#ifdef USE_SNDFILE

#ifndef _WIN32
static void *alert_if_offline_thread(void *p_data)
#else
static void alert_if_offline_thread(void *p_data)
#endif
{
	DYN_DNS_CLIENT		*p_dyndns;

	if (p_data) {

		p_dyndns=((CB_ALERT_DATA *) p_data)->p_dyndns;

		if (p_dyndns) {

			is_alert_thread_exit=false;

			alert_if_offline(p_dyndns,((CB_ALERT_DATA *) p_data)->p_data);

			DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Exiting alert_if_offline_thread...\n"));
		}
	}

#ifdef _WIN32
	_endthread();
#else
	pthread_exit(p_data);

	/*compiler complaints (pthread_exit does not return)*/
	return p_data;
#endif
}

#endif

static void start_online_alert_thread(CB_ALERT_DATA *p_online_data)
{

	/*
		launch alert_if_offline_thread to sound audible alert until
		back online
	*/

#ifdef USE_SNDFILE

#ifdef _WIN32

	thread_alert=_beginthread(alert_if_offline_thread,0,(void *) p_online_data);
#else

	pthread_create(&thread_alert,NULL,alert_if_offline_thread,(void *) p_online_data);
#endif

#endif

}

#ifndef _WIN32
static void *is_dyndns_online_thread(void *p_data)
#else
static void is_dyndns_online_thread(void *p_data)
#endif
{

	CB_ALERT_DATA		*online_data;

	DYN_DNS_CLIENT		*p_self;
	int					attempts;
	int					is_online=false;
	int					is_was_offline=false;


	if (p_data) {

		if (((CB_ALERT_DATA *) p_data)->p_dyndns) {

			p_self=((CB_ALERT_DATA *) p_data)->p_dyndns;

			online_data=safe_malloc(sizeof(CB_ALERT_DATA));

			online_data->p_data=p_data;
			online_data->p_dyndns=p_self;


			while(!(is_online_thread_exit) && !(is_exit_requested(p_self))) {

				attempts=0;

				do {

					DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "online thread calling do_is_dyndns_online...\n"));

					is_online=do_is_dyndns_online(p_self);


					DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "online thread returned from do_is_dyndns_online...\n"));

					if (is_online)

						break;

					if ((++attempts>p_self->alert_retries))

						break;

					DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "online thread pausing 5 seconds before retry...\n"));

					/*pause before retry -- need an option*/
					sleep_lightly_ms(5000,&thread_exit_cond,p_self);

				} while (!(is_online_thread_exit) && !(is_exit_requested(p_self)));


				get_mutex(&mutex_global_is_online);

				global_is_online=is_online;

				release_mutex(&mutex_global_is_online);

				if (global_is_online) {

					if (is_was_offline) {

						is_was_offline=false;

						p_self->cmd=CMD_UPDT;
					}

					exit_alert_thread();

					DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Status thread entering status interval...\n"));

					sleep_lightly_ms(p_self->status_interval,&thread_exit_cond,p_self);
				}
				else {

					DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "\n" \
								ERROR_FLAG \
								"Status thread detected offline...\n" \
								ERROR_FLAG));

					is_was_offline=true;

					if (!(thread_alert))

						start_online_alert_thread(online_data);

					/*sleep less, check status more, if offline*/
					sleep_lightly_ms(p_self->status_offline_interval,&thread_exit_cond,p_self);
				}
			}	

			exit_alert_thread();

			free(online_data);

			DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Exiting is_dyndns_online_thread...\n"));
		}
	}

#ifdef _WIN32
	_endthread();
#else
	pthread_exit(p_data);

	/*compiler complaints (pthread_exit does not return)*/
	return p_data;
#endif
}

static void start_online_test_thread(DYN_DNS_CLIENT *p_dyndns,void *p_data)
{

	/*
		launch is_dyndns_online_thread to periodically check online status and set
		a global flag, global_is_online, accordingly.
	*/

	CB_ALERT_DATA	*p_online_data=safe_malloc(sizeof(CB_ALERT_DATA));


	p_online_data->p_data=p_data;
	p_online_data->p_dyndns=p_dyndns;

#ifdef _WIN32

	thread_online_test=_beginthread(is_dyndns_online_thread,0,(void *) p_online_data);
#else

	pthread_create(&thread_online_test,NULL,is_dyndns_online_thread,(void *) p_online_data);
#endif

}

#endif /*USE_THREADS*/

/******************Windows RAS thread, and Windows Service related******************/

/*try to get mutex without waiting, and unless is_waited flagged as NULL,
  wait for mutex if have to, and set is_waited to indicate had to wait.
*/

#ifdef _WIN32

char get_mutex_dyn(DYN_DNS_CLIENT *p_dyndns,HANDLE *hMutex,int *is_waited)
{

	int	wait_ret=0;


	if (is_waited)

		*is_waited=0;


	if (*hMutex) {

		wait_ret=WaitForSingleObject(*hMutex,0);


		if (!(wait_ret==WAIT_OBJECT_0)) {

			if (!(wait_ret==WAIT_TIMEOUT)) {

				dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "WaitForSingleObject failed in get_mutex.",
				                       GetLastError(),LOG_CRIT,p_dyndns->dbg.level);
			}
			else {

				if (is_waited) {

					wait_ret=WaitForSingleObject(*hMutex,INFINITE);

					if (!(wait_ret==WAIT_OBJECT_0))

						dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "WaitForSingleObject failed in get_mutex.",
						                       GetLastError(),LOG_CRIT,p_dyndns->dbg.level);


					*is_waited=1;
				}
			}
		}
	}

	return (wait_ret==WAIT_OBJECT_0);
}

DWORD get_mutex_wait_dyn(DYN_DNS_CLIENT *p_dyndns,HANDLE *hMutex)
{

	int	is_waited;


	return get_mutex_dyn(p_dyndns,hMutex,&is_waited);
}

int release_mutex_dyn(DYN_DNS_CLIENT *p_dyndns,HANDLE *hMutex)
{

	int	release_ret=0;


	if (*hMutex)

		if (!(release_ret=ReleaseMutex(*hMutex)))

			dbg_printf_and_err_str(NULL,"C:" MODULE_TAG "Failed ReleaseMutex in get_mutex.  Low resources?  System is unstable...",GetLastError(),LOG_CRIT,p_dyndns->dbg.level);


	return release_ret;
}

/**********************Event Handlers**********************/

/*
	RAS thread event.
*/
static void dyn_dns_update_ip_handler(RAS_THREAD_DATA *p_ras_thread_data)
{

	DYN_DNS_CLIENT     *p_dyndns;


	p_dyndns=p_ras_thread_data->p_context;

	/* need reinit dyndns structure (with synchronization) on fail?
	*/

	if (p_ras_thread_data->dwEvent==RAS_CONNECT) {

		/*attempt_update(p_dyndns,p_ras_thread_data);*/

		p_dyndns->cmd=CMD_UPDT;
	}
	else {

		if (p_dyndns->dbg.level>=LOG_INFO)

			DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Not an event of interest...\n"));
	}
}

/*
	Windows SCM event.
*/
int service_event_handler(SERVICE_EVENT service_event)
{

	/*trigger dyn_dns_main return*/

	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Entered service_event_handler.  Setting return signal...\n"));


	returnSignaled=true;

	/*
		this is also set via call to threads
		exit routine, in main loop on return 
		from updating and/or waiting sleep 
		before update.  setting here as well
		could get online status alert threads 
		to return sooner.  the main possible 
		bottleneck on program quit is if in 
		middle of a network comm --	an update, 
		or do_dyndns_is_online online status 
		check -- socket routines are blocking 
		type.
	*/


	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Setting thread exit...\n"));

	is_online_thread_exit=true;


	return 1;
}

/********************End Event Handlers********************/

void start_ras_thread(DYN_DNS_CLIENT *p_dyndns,RAS_THREAD_DATA **p_ras_thread_data)
{

	if (p_dyndns->dbg.level>=LOG_INFO)

		DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Creating, launching ras thread...\n"));


	*p_ras_thread_data=construct_and_launch_trap_ras_events(dyn_dns_update_ip_handler,p_dyndns,p_dyndns->\
		                   http_to_ip_server.super.super.p_remote_host_name,\
		                   p_dyndns->dbg.level);

	if (*p_ras_thread_data) {

		if (p_dyndns->dbg.level>=LOG_INFO)

			DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Launched RAS events trapping...\n"));

	}
	else {

		if (p_dyndns->dbg.level>=LOG_ERR)

			DBG_PRINTF((LOG_ERR, "E:" MODULE_TAG "RAS events trapping constructor returned NULL.  Continuing without RAS events trapping...\n"));
	}	
}

static void atomic_inc(DYN_DNS_CLIENT *p_self,int *src,int inc,HANDLE hMutex)
{

	get_mutex_wait_dyn(p_self,&hMutex);


	*src+=inc;


	release_mutex_dyn(p_self,&hMutex);
}

#endif /*_WIN32*/

static void inc_update_times(DYN_DNS_CLIENT *p_self,int *src,int inc)
{

	*src+=inc;
}

RC_TYPE dyn_dns_reinit(DYN_DNS_CLIENT *p_dyndns)
{

	RC_TYPE		rc=RC_OK;


	dyn_dns_shutdown(p_dyndns);

	rc=do_dyn_dns_init(p_dyndns);


	return rc;
}

/*find the alias given by parameter alias in list of p_dyndns structure alias names and set
  correponding update state boolean to parameter is_updated

  some of the stuff related to ip type (auto,ip4,ip6,etc.) is not neccessary as all 
  input here is already in the form: alias:type as of advent of upgrading/rewriting 
  ip cache file if neccessary, on startup.  the alias type code remains because it
  is more general (it does not assume there's an alias ip type indicator in input 
  parameter, alias, and appends :auto if need be) -- may come in handy*/
static void set_update_state(DYN_DNS_CLIENT *p_dyndns,char *alias,bool is_updated)
{

	int		i;
	char	*alias_in=NULL;
	char	*alias_ip_v=alias;
	char	*p_alias_ip_v=NULL;
	char	*alias_cmp_src;
	char	*alias_type;
	BOOL	is_auto=false;
	BOOL	is_found=false;


	/*backward compat -- add ip version indicator*/
	

	alias_in=safe_malloc(strlen(alias)+1); /*save pre-processed version of alias*/
	strcpy(alias_in,alias);

	if ((alias_type=strstr(alias_in,":"))) { 

		is_auto=!(strcmp((alias_type+1),"auto"));

		*alias_type='\0';

		alias_type++;
	}
	else {

		alias_ip_v=safe_malloc(strlen(alias)+6);

		p_alias_ip_v=alias_ip_v;

		strcat(strcat(strcpy(alias_ip_v,alias),":"),"auto");

		is_auto=true;
	}

	for (i=0;i<p_dyndns->alias_info.count;i++) {

		/*add ip type [auto,coupled,ip6,ip4] to name[i].name to do compare*/

		alias_cmp_src=safe_malloc(strlen(p_dyndns->alias_info.names[i].name)+9);
		strcat(strcat(strcpy(alias_cmp_src,p_dyndns->alias_info.names[i].name),":"),p_dyndns->alias_info.names[i].ip_v);

		if (!(strcmp(alias_cmp_src,alias_ip_v)) || (strstr(p_dyndns->alias_info.names[i].ip_v,DUAL_LIST) 
			&& !(strcmp(p_dyndns->alias_info.names[i].name,alias_in)))) {

			p_dyndns->alias_info.update_succeeded[i]=is_updated;

			is_found=true;
		}
		else {

			/*for aliases of type auto, set update state for either ip6, ip4*/

			if (is_auto)

				if (!(strcmp(p_dyndns->alias_info.names[i].name,alias_in))) {

					p_dyndns->alias_info.update_succeeded[i]=is_updated;

					is_found=true;
				}
		}

		free(alias_cmp_src);

		if (is_found)

			break;
	}

	free(p_alias_ip_v);
	free(alias_in);
}

/*
	read in ip cache file lines containing ip address, alias pairs
	rewrite the file so lines not containing an ip type (ip4, ip6, auto) get ip type, auto
*/
RC_TYPE upgrade_ip_cache(DYN_DNS_CLIENT *p_dyndns)
{

	FILE		*fp;
	RC_TYPE		rc=RC_OK;
	

	fp=utf_fopen(p_dyndns->ip_cache,"r");

	if (fp)	{

		char	ch;
		char	*in_str[DYNDNS_MAX_ALIAS_NUMBER*2]; /*multiply for dual alias type*/
		int		i=0;
		int		line_count=0;
		int		max_len=256;
		BOOL	is_rewrite=false;

		in_str[line_count]=safe_malloc(max_len);


		while (1) {

			ch=fgetc(fp);

			if (feof(fp))

				break;

			if (!(ch=='\n')) {

				in_str[line_count][i++]=ch;

				if (i==max_len) {

					max_len+=256;

					in_str[line_count]=safe_realloc(in_str,max_len);
				}
			}
			else {

				in_str[line_count][i]='\0';

				if (!(is_rewrite))

					is_rewrite=!(strstr(in_str[line_count],":auto") || strstr(in_str[line_count],":ip4") 
						|| strstr(in_str[line_count],":ip6"));

				i=0;

				line_count++;

				in_str[line_count]=safe_malloc(max_len);
			}
		}

		fclose(fp);

		if (!(is_rewrite)) {

			return rc;
		}
		else {

			fp=utf_fopen(p_dyndns->ip_cache,"w");

			if (fp) {

				int ii;

				for (i=0;i<line_count;i++) {

					ii=0;

					while (1) {

						fputc(in_str[i][ii++],fp);

						if (!(in_str[i][ii])) {

							if ((strstr(in_str[i],":auto")) || (strstr(in_str[i],":ip4")) || (strstr(in_str[i],":ip6"))) {

								fputc('\n',fp);

								break;
							}
							else {

								fputs(":auto",fp);

								fputc('\n',fp);

								break;
							}
						}
					}
				}

				fclose(fp);
			}

			for (i=0;i<line_count;i++) {

				free(in_str[i]);
			}
		}
	}


	return rc;
}

/*  Record the last ip as per that in ip cache file.  Report it via log output.

	Set update need for each space separated ip, alias pair line read.  
	
	Set global is_update_pending if number of aliases read less than p_dyndns->alias_info.count.
*/

RC_TYPE check_ip_cache(DYN_DNS_CLIENT *p_dyndns)
{

	FILE		*fp;
	RC_TYPE		rc=RC_OK;
	

	fp=utf_fopen(p_dyndns->ip_cache,"r");

	if (fp)	{

		char	ch;
		char	*in_str;
		int		i=0;
		int		alias_count=0;
		int		max_len=256;

		in_str=safe_malloc(max_len);


		while (1) {

			ch=fgetc(fp);

			if (feof(fp))

				break;

			if (!(ch==' ') && !(ch=='\n')) {

				in_str[i++]=ch;

				if (i==max_len) {

					max_len+=256;

					in_str=safe_realloc(in_str,max_len);
				}
			}
			else {

				if (ch==' ') { /*ip address*/

					/*redundant for multi-aliases.  and aliases needing update won't be here*/

					if (strstr(in_str,".")) {

						strcpy(p_dyndns->info.my_ip_address.name[ip_4],in_str);
					}
					else {

						strcpy(p_dyndns->info.my_ip_address.name[ip_6],in_str);
					}
				}
				else { /*alias*/

						/*
							set update succeeded for corresponding alias

							others will be false, and need is_update_pending true accordingly
						*/

						set_update_state(p_dyndns,in_str,true);

						alias_count++;
				}

				i=0;

				memset(in_str,0,max_len);
			}
		}

		is_update_pending=(alias_count<p_dyndns->alias_info.count);

		fclose(fp);

		if (strlen(p_dyndns->info.my_ip_address.name[ip_4]))

			DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "IPv4 address read from cache file (%s):  '%s'...\n",p_dyndns->ip_cache, p_dyndns->info.my_ip_address.name[ip_4]));

		if (strlen(p_dyndns->info.my_ip_address.name[ip_6]))

			DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "IPv6 address read from cache file (%s):  '%s'...\n",p_dyndns->ip_cache, p_dyndns->info.my_ip_address.name[ip_6]));

		free(in_str);
	}


	return rc;
}

static int increment_iterations(DYN_DNS_CLIENT *p_dyndns)
{
	int	iterations;


	p_dyndns->iterations++;

	iterations=p_dyndns->iterations;


	return iterations;
}

BOOL is_completed_iterations(DYN_DNS_CLIENT *p_dyndns,int *iteration)
{


	*iteration=p_dyndns->iterations;


	/* check if the user wants us to stop */

	return (*iteration >= p_dyndns->total_iterations && p_dyndns->total_iterations != 0);
}

RC_TYPE init_update_loop(DYN_DNS_CLIENT *p_dyndns,int argc, char* argv[],void **p_data,BOOL *init_flag)
{
	RC_TYPE			rc = RC_OK;
	RC_TYPE			rc_cmd_line = RC_OK;	

#ifndef _WIN32

#ifdef USE_THREADS

	void			**p_ras_thread_data=p_data;
#endif

#else
	RAS_THREAD_DATA	**p_ras_thread_data= (RAS_THREAD_DATA **) p_data;
	int             regParamsC=1;
	wchar_t         *regArgs[50];
	char            *utf_8_argv[50];
	int             i=0;

	RC_TYPE         rc_reg=RC_ERROR;
#endif

#ifdef USE_THREADS

	threads_wrapper_init();

	memset(&update_timer,0,sizeof(my_timer_t));
#endif


	if (p_dyndns == NULL)

		return RC_INVALID_POINTER;

	/*load default data */
	rc = get_default_config_data(p_dyndns);
	if (rc != RC_OK)
	{
		return rc;
	}


#ifdef _WIN32

	SetLastError(0);

	/*set up to any registry parameters first -- input file,
	  and command args override registry*/

	utf_8_argv[0]=safe_malloc(strlen(argv[0])+1);
	strcpy(utf_8_argv[0],argv[0]);

	/*if we're a service, service_main will have these*/
	regParamsC+=get_registry_parameters(regArgs);

	if (regParamsC>1) {

		utf_16_to_8_ar((utf_8_argv+1),regArgs,regParamsC-1);

		rc_reg=get_config_data(p_dyndns,regParamsC,utf_8_argv);

		for (i=0;i<regParamsC;i++) free(regArgs[i]);

		utf_free_ar(utf_8_argv,regParamsC);

		if (!(rc_reg == RC_OK))

			return rc_reg;
	}

#endif

	/* read cmd line options and set object properties*/
	rc_cmd_line = get_config_data(p_dyndns, argc, argv);


#ifndef _WIN32

	if (rc_cmd_line != RC_OK || p_dyndns->abort)

#else

	if (!(rc_cmd_line == RC_OK || rc_reg == RC_OK) || p_dyndns->abort)

#endif

		return rc_cmd_line;


	if (p_dyndns->lang_hard_coded) {

		/*use hard coded defaults -- don't use default locale file*/
		dealloc_lang_strings();

		DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Empty --lang_file parameter.  Deallocated language strings, using hard coded english defaults...\n"));
	}
	else {

		/*if opt around default language strings, use that*/
		if (p_dyndns->lang_file) {

			if (!(re_init_lang_strings(p_dyndns->lang_file)==RC_OK)) {

				DBG_PRINTF((LOG_WARNING, "W:" MODULE_TAG "Failed using default override language strings file, %s...\n",p_dyndns->lang_file));
			}
		}
	}

	/*if logfile provided, redirect output to log file*/
	if (p_dyndns->dbg.p_logfilename)
	{

		DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Attempting to open log file: %s...\n",p_dyndns->dbg.p_logfilename));

		rc = os_open_dbg_output(DBG_FILE_LOG, "", p_dyndns->dbg.p_logfilename,p_dyndns->dbg.level);

		if (rc != RC_OK)

			return rc;
	}

	if (p_dyndns->debug_to_syslog == TRUE || (p_dyndns->run_in_background == TRUE))
	{
		if (get_dbg_dest() == DBG_STD_LOG) /*avoid file and syslog output */
		{
			rc = os_open_dbg_output(DBG_SYS_LOG, "INADYN", NULL,p_dyndns->dbg.level);
			if (rc != RC_OK)

				return rc;
		}
	}

	if (p_dyndns->change_persona)
	{
		OS_USER_INFO os_usr_info;
		memset(&os_usr_info, 0, sizeof(os_usr_info));
		os_usr_info.gid = p_dyndns->sys_usr_info.gid;
		os_usr_info.uid = p_dyndns->sys_usr_info.uid;
		rc = os_change_persona(&os_usr_info);
		if (rc != RC_OK)
		{
			return rc;
		}
	}

	/*if silent required, close console window*/
	if (p_dyndns->run_in_background == TRUE)
	{
		rc = close_console_window();
		if (rc != RC_OK)
		{
			return rc;
		}
		if (get_dbg_dest() == DBG_SYS_LOG)
		{
			fclose(stdout);
		}
	}

	dyn_dns_print_hello(NULL);

	/*  now that log is open, report any command line errors eventhough registry params made up for them --
	    if argc is 1, parser returns error if can't open default config file -- ignore that condition
	*/
	if (rc_cmd_line != RC_OK && argc > 1) {

		DBG_PRINTF((LOG_WARNING, "W:" MODULE_TAG "%s error returned getting command line "\
		            "parameters.  One or more command line parameters ignored...\n",errorcode_get_name(rc_cmd_line)));

	}

	/*01/24/11 bhoover@wecs.com*/
	upgrade_ip_cache(p_dyndns);

	check_ip_cache(p_dyndns);

	rc = dyn_dns_init(p_dyndns);

	if (rc==RC_OK) {

		*init_flag=true;

		rc = os_install_signal_handler(p_dyndns);

		if (rc != RC_OK)

			DBG_PRINTF((LOG_WARNING,"DYNDNS: Error '%s' (0x%x) installing OS signal handler\n",errorcode_get_name(rc), rc));
	}

	if (rc==RC_OK) {

		rc = get_encoded_user_passwd(p_dyndns);

#ifdef _WIN32

		/*
			Windows RAS/connect/disconnect events threads
		*/
		start_ras_thread(p_dyndns,p_ras_thread_data);		
#endif

#ifdef USE_THREADS

		/*
			online status thread
		*/
		start_online_test_thread(p_dyndns,p_ras_thread_data);

		/*use update_timer when out of cmd wait loop servicing commands --
		  update/net comm can take some time, especially with retries 
		  parameter(s) set.  Makes forced_update_period more accurate.*/
		create_timer(&update_timer,25);
#endif

#ifdef USE_SNDFILE

		waveout_init();
#endif

	}

	return rc;
}

/* MAIN - Dyn DNS update entry point.*/

/*
	Actions:
		- read the configuration options
		- perform various init actions as specified in the options
		- create and init dyn_dns object.
		- launch the IP update action loop
*/		
int dyn_dns_main(DYN_DNS_CLIENT *p_dyndns, int argc, char* argv[])
{

	RC_TYPE				rc=RC_OK;
	BOOL				init_flag=FALSE;
	int					current_iteration=0;

#ifdef _WIN32

	RAS_THREAD_DATA	*p_ras_thread_data=NULL;
#else

	/*used in win32 only*/
	void *p_ras_thread_data=NULL;
#endif

	rc=init_update_loop(p_dyndns,argc,argv,&p_ras_thread_data,&init_flag);


	if (p_dyndns->abort)

		return rc;

	if (rc==RC_OK) /*init all okay?*/

		do
		{

			/*update IP address in a loop*/
			
			if (((rc=dyn_dns_update_ip(p_dyndns))==RC_OK))

				increment_iterations(p_dyndns);

			else {

				DBG_PRINTF((LOG_WARNING,"W:'%s' (0x%x) updating the IPs. (it %d)\n",
					        errorcode_get_name(rc), rc, current_iteration));

				p_dyndns->is_bad_config=(rc==RC_DYNDNS_RSP_CONFIG);

				rc=dyn_dns_reinit(p_dyndns);
			}

#ifdef USE_SNDFILE

#ifndef USE_THREADS

			alert_if_offline(p_dyndns,p_ras_thread_data);
#endif

#endif

			if (p_dyndns->cmd==CMD_STOP)
			{

				DBG_PRINTF((LOG_DEBUG,"D:" MODULE_TAG "STOP command received. Exiting.\n"));

				break;
			}

			if (is_completed_iterations(p_dyndns,&current_iteration)) {

				DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Iterations (%d) completed.  Exiting program...\n", current_iteration));

				break;
			} 
			else {

				if (!(rc==RC_OK)) {

					init_flag=false;

					break;
				}
			}

			/* sleep the time set in the ->sleep_sec data memeber*/
			dyn_dns_wait_for_cmd(p_dyndns);

			/*deprecated*/
			inc_update_times(p_dyndns,&(p_dyndns->times_since_last_update),1);

			/*reset the command*/
			if (!(p_dyndns->cmd==CMD_STOP))
			{
				p_dyndns->cmd=NO_CMD;
			}
			else {

				DBG_PRINTF((LOG_DEBUG,"D:" MODULE_TAG "STOP command received. Exiting.\n"));

				break;
			}

		}
		while(1);
	
#ifdef USE_THREADS

	destroy_timer(&update_timer);

	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Main loop calling exit_online_test_threads...\n"));

	exit_online_test_threads();

	DBG_PRINTF((LOG_INFO, "I:" MODULE_TAG "Returned from exit_online_test_threads...\n"));
#endif

	
#ifdef _WIN32

	destroy_trap_ras_events(&p_ras_thread_data);
#endif


	if (init_flag)
	{
		/* dyn_dns_shutdown object */
		rc = dyn_dns_shutdown(p_dyndns);
	}

#ifdef USE_SNDFILE

	waveout_shutdown();
#endif

	return rc;
}
