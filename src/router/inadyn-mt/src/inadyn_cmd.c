/*
Copyright (C) 2003-2004 Narcis Ilisei
Modifications by Steve Horbachuk
Copyright (C) 2006 Steve Horbachuk
Modifications by Bryan Hoover (bhoover@wecs.com)
Copyright (C) 2007 Bryan Hoover (bhoover@wecs.com)

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
		- many options added
		- january 2005 - new format for the config file =Thanks to Jerome Benoit. 
		- january 30 2005 - new parser for config file -
		- october 2007 - debug level command line parameter added
		- dec 2007 - file options handler now provides for command line options precedence
		- June/July 2009 - added audible alert related options
		- dec 2009 - accomodate/change relative to static to heap memory DYN_DNS_CLIENT members
		- dec 2009 - fixed handlers' allocation overflow/range checking
*/
#define MODULE_TAG "CMD_OPTS: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <wchar.h>

#include "dyndns.h"
#include "debug_if.h"
#include "base64.h"
#include "get_cmd.h"
#include "unicode_util.h"
#include "safe_mem.h"
#include "path.h"

/* command line options */
#define DYNDNS_INPUT_FILE_OPT_STRING "--input_file"

static RC_TYPE help_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE wildcard_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_username_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_password_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_alias_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_debug_level_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_dns_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_dns_server_url_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_ip_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_online_check_url_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_dyndns_system_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_update_period_sec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_forced_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

#ifdef USE_THREADS

static RC_TYPE get_forced_update_adjust_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

#endif

static RC_TYPE get_logfile_name(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_silent_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_verbose_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_proxy_server_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_options_from_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_iterations_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_syslog_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE set_change_persona_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE print_version_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_exec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_cache_dir(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_retries_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_retry_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_retry_pending_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_retry_pending_off_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_lang_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

#ifdef USE_SNDFILE

static RC_TYPE get_wave_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_audible_on_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_alert_retries_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_alert_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_wave_loops_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_wave_gain_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_wave_buff_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

#ifdef USE_THREADS

static RC_TYPE get_status_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);
static RC_TYPE get_status_offline_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context);

#endif

#endif

static CMD_DESCRIPTION_TYPE cmd_options_table[] =
    {
        {"--help",		0,	{help_handler, NULL,0},	"help" },
        {"-h",			0,	{help_handler, NULL,0},	"help" },

        {"--username",	1,	{get_username_handler, NULL,0},	"<name or hash> - your  membername/hash"},
        {"-u",			1,	{get_username_handler, NULL,0},	"<name or hash> - your membername/hash"},

        {"--password",	1,	{get_password_handler, NULL,0},	"<password> - your password. Optional."},
        {"-p",			1,	{get_password_handler, NULL,0},	"<password> - your password"},

        {"--alias",		3,	{get_alias_handler, NULL,2},	"<alias host name>[,hash] [dual | " DUAL_LIST " | ip4 | ip6 | auto] - alias host name, optional comma delimted hash, and optional ip version, default auto. This option can appear multiple times." },
        {"-a",			3,	{get_alias_handler, NULL,2},	"<alias host name>[,hash] [dual | " DUAL_LIST " | ip4 | ip6 | auto] - alias host name, optional comma delimited hash, and optional ip version, default auto. This option can appear multiple times." },

        {"--debug",     1,  {get_debug_level_handler, NULL,0}, "<#> - debug level 0..7; higher number, more log debug messages."},
        {"-d",          1,  {get_debug_level_handler, NULL,0}, "<#> - debug level 0..7; higher number, more log debug messages."},

        /*
        for help display only -- service_main takes care of these handlers

        could have a servie_main_display_help() function instead
        */

#ifdef _WIN32  
        {"-i",3,{NULL, NULL,3},"[quoted service description] [-n <service name>] - install service"},
        {"-s",2,{NULL, NULL,2},"[-n <service name>] - start service"},
        {"-e",2,{NULL, NULL,2},"[-n <service name>] - exit service"},
        {"-r",2,{NULL, NULL,2},"[-n <service name>] - restart service"},
        {"-x",2,{NULL, NULL,2},"[-n <service name>] - uninstall service"},
#endif


        {DYNDNS_INPUT_FILE_OPT_STRING, 1, {get_options_from_file_handler, NULL,0}, "<path/file> - the file containing [further] inadyn options.  "
         "The default config file, '" DYNDNS_MT_DEFAULT_CONFIG_FILE "' is used if inadyn is called without any cmd line options.  "\
         "Input file options are inserted at point of this option's appearance." },

        {"--ip_server_name",	2,	{get_ip_server_name_handler, NULL,0},
         "<srv_name[:port] local_url> - local IP is detected by parsing the response after returned by this server and URL.  "\
         "The first IP in found http response is considered 'my IP'."},

        {"--online_check_url",	2,	{get_online_check_url_handler, NULL,0},
         "<srv_name[:port] local_url> - URL to reach to confirm online status.  "\
         "Default value:  " DYNDNS_MY_ONLINE_CHECK " /"},

        {"--dyndns_server_name", 1,	{get_dns_server_name_handler, NULL,0},
         "<NAME>[:port] - The server that receives the update DNS request.  Allows the use of unknown DNS services that "\
		 "accept HTTP updates.  If no proxy is wanted, then it is enough to set the dyndns system. The default servers "\
		 "will be taken."},

        {"--dyndns_server_url", 1, {get_dns_server_url_handler, NULL,0},
         "<name> - full URL relative to DynDNS server root.  Ex: /some_script.php?hostname="},

        {"--dyndns_system",	1,	{get_dyndns_system_handler, NULL,0},
         "[NAME] - optional DYNDNS service type. SHOULD be one of the following: \n"
         "\t\t-For dyndns.org:  dyndns@dyndns.org OR statdns@dyndns.org OR customdns@dyndns.org.\n"
         "\t\t-For freedns.afraid.org:  default@freedns.afraid.org\n"
         "\t\t-For zoneedit.com:  default@zoneedit.com\n"
         "\t\t-For no-ip.com:  default@no-ip.com\n"
         "\t\t-For easydns.com:  default@easydns.com\n"
         "\t\t-For 3322.org:  dyndns@3322.org\n"
		 "\t\t-For sitelutions.com:  default@sitelutions.com\n"
		 "\t\t-For dnsomatic.com:  default@dnsomatic.com\n"
		 "\t\t-For tunnelbroker.net:  ipv6tb@he.net\n"
		 "\t\t-For tzo.com:  default@tzo.com\n"
		 "\t\t-For dynsip.org:  default@dynsip.org\n"
		 "\t\t-For dhis.org:  default@dhis.org\n"
		 "\t\t-For majimoto.net:  default@majimoto.net\n"
		 "\t\t-For zerigo.com:  default@zerigo.com\n"
         "\t\t-For generic:  custom@http_svr_basic_auth\n"
         "\t\tDEFAULT value: dyndns@dyndns.org\n"},

        {"--proxy_server", 1, {get_proxy_server_handler, NULL,0},
         "[NAME[:port]]  - the http proxy server name and port. Default is none."},
        {"--update_period",	1,	{get_update_period_handler, NULL,0},"<#> - how often the IP is checked. The period is in [ms]. 30000..864000000.  Default is about 10 min. Max is 10 days"},
        {"--update_period_sec",	1, {get_update_period_sec_handler, NULL,0},"<#> - how often the IP is checked. The period is in [sec]. 30..864000.  Default is about 10 min. Max is 10 days"},
        {"--forced_update_period", 1, {get_forced_update_period_handler, NULL,0},"<#> - how often, in seconds, the IP is updated even if it is not changed. 30 sec..30 days, default, 30 days."},

#ifdef USE_THREADS

		{"--forced_update_adjust", 1,   {get_forced_update_adjust_handler, NULL,0},"<#> - fine timer control.  Slow, or speed timer between -4..5.  Default, 0."},
#endif

        {"--log_file",	1,	{get_logfile_name, NULL,0},		"<path/file> - log file path and name"},
        {"--background", 0,	{set_silent_handler, NULL,0},		"run in background. output to log file or to syslog"},

		{"--verbose",	1,	{set_verbose_handler, NULL,0},	"<#> - set dbg level. 0 to 5"},

		{"--iterations",	1,	{set_iterations_handler, NULL,0},	"<#> - set the number of DNS updates. Default is 0, which means infinity."},
        {"--syslog",	0,	{set_syslog_handler, NULL,0},	"force logging to syslog . (e.g. /var/log/messages). Works on **NIX systems only."},
        {"--change_persona", 1, {set_change_persona_handler, NULL,0}, "<uid[:gid]> - after init switch to a new user/group. Parameters: <uid[:gid]> to change to. Works on **NIX systems only."},
        {"--version", 0, {print_version_handler, NULL,0}, "print the version number\n"},
        {"--exec", 1, {get_exec_handler, NULL,0}, "<command> - external command to exec after an IP update. Include the full path."},
        {"--cache_dir", 1, {get_cache_dir, NULL,0}, "<path> - cache directory name. (e.g. /tmp/ddns). Defaults to /tmp on **NIX systems."},
        {"--wildcard", 0, {wildcard_handler, NULL,0}, "enable domain wildcarding for dyndns.org, 3322.org, or easydns.com."},
        {"--retries", 1, {get_retries_handler, NULL,0}, "<#> - network comm retry attempts.  0 to 100, default 0"},
        {"--retry_interval", 1, {get_retry_interval_handler, NULL,0}, "<#> - network comm miliseconds retry interval.  0 to 30,000, default 1,000"},
		{"--retry_pending", 0, {get_retry_pending_off_handler, NULL,0}, "<#> - retry ip update even after network comm retries exhausted,  default on"},
		{"--retry_pending_interval", 1, {get_retry_pending_interval_handler, NULL,0}, "<#> - network comm seconds update retry interval, after retries exhausted.  5..3600, default 300"},
        {"--lang_file", 1, {get_lang_file_handler, NULL,1}, "[path/file] - language file path, and file name, defaults to either ../inadyn-mt/lang/en.lng, or etc/inadyn-mt/en.lng.  Empty parameter option gives hard coded english defaults."},

#ifdef USE_SNDFILE
		{"--audible", 0, {get_audible_on_handler, NULL,0}, "audible network status alerts toggle.  default off"},
		{"--wave_file", 2, {get_wave_file_handler, NULL,1}, "<path/file [#] - audible network status alerts wave file path, and file name. defaults\n" \
		"\t\tto either ../inadyn-mt/extra/wav/alarm.wav, or etc/inadyn-mt/extra/wav/alarm.wav \n"\
		"\t\t[wave loops: 0..100; -1 for infinite] same as wave_loops below -- optional number of\n"\
		"\t\ttimes per wave play call to repeat wave file play, default 0"},
		{"--wave_loops", 1, {get_wave_loops_handler, NULL,0}, "<#> - same as wave_file parameter optional parameter --\n"\
		"\t\t0..100; -1 for infinite number of times per wave play call to repeat wave file play, default 0"},
		{"--alert_retries", 1, {get_alert_retries_handler, NULL,0}, "<#> - network retries before audible network status alerts. [0..100], default 0"},
		{"--alert_interval", 1, {get_alert_interval_handler, NULL,0}, "<#> - time in miliseconds between consecutive audible network status alerts. [0..3600000], default 0"},
		{"--wave_gain", 1, {get_wave_gain_handler, NULL,0}, "<#> - gain (amplitude adjust) at which to play audible alert (beware clipping), integer or float. [-10..10], default 10 (0db, no attenuation)."},
		{"--wave_buff", 1, {get_wave_buff_handler, NULL,0}, "<#> - wave file output buffer size control -- integer or float multiple of wave file bytes per sec. [.25..10], default .25"},

/*possible other options:
		-time before go into rest period
		-time to rest
		-time of day block out period	
		-for dial-up, don't test connect 
		'til after dail-up
		-for dial-up, don't alert if
		user terminated connection
		-status check at random intervals over
		a specified range
		-alert sounding decay functions options

*/

#ifdef USE_THREADS

		{"--status_interval", 1, {get_status_interval_handler, NULL,0}, "<#> - seconds [30..864000] interval at which to check online status.  defaults to 600"},
		{"--status_offline_interval", 1, {get_status_offline_interval_handler, NULL,0}, "<#> - seconds [0..864000] interval at which to check online status, after offline detected.  defaults to 15"},

#endif
#endif
        {NULL,		0,	{0, NULL,0},	"" }
    };


void print_help_page(void)
{
	printf("\n\n\n" \
	       "			INADYN-MT Help\n\n" \
	       "	INADYN-MT is a dynamic DNS client. That is, it maintains the IP address\n" \
	       "of a host name. It periodically checks whether the IP address of the current machine\n" \
	       "(the external visible IP address of the machine that runs INADYN) has changed.\n" \
	       "If yes it performs an update in the dynamic dns server.\n\n");
	printf("Typical usage: \n"	\
	       "\t-for dyndns.org system: \n" \
	       "\t\tinadyn-mt -u username -p password -a my.registrated.name \n" \
	       "\t-for freedns.afraid.org:\n" \
	       "\t\t inadyn-mt --dyndns_system default@freedns.afraid.org -a my.registrated.name,hash -a anothername,hash2\n" \
	       "\t\t 'hash' is extracted from the grab url batch file that is downloaded from freedns.afraid.org\n\n" \
	       "Parameters:\n");

	{
		CMD_DESCRIPTION_TYPE *it = cmd_options_table;

		while( it->p_option != NULL)
		{
			printf(
			    "\t'%s': %s\n\r",
			    it->p_option, it->p_description);
			++it;
		}
	}
	printf("\n\n\n");
}

static RC_TYPE help_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->abort = TRUE;
	print_help_page();
	return RC_OK;
}

static RC_TYPE wildcard_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->wildcard = TRUE;
	return RC_OK;
}

static RC_TYPE set_verbose_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->dbg.level) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}
	return RC_OK;
}

static RC_TYPE set_iterations_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->total_iterations) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->total_iterations = (p_self->sleep_sec < 0) ?  DYNDNS_DEFAULT_ITERATIONS : p_self->total_iterations;
	return RC_OK;
}

static RC_TYPE set_silent_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	p_self->run_in_background = TRUE;
	return RC_OK;
}

static RC_TYPE get_logfile_name(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}


	if (p_self->dbg.p_logfilename) {

		free(p_self->dbg.p_logfilename);

		p_self->dbg.p_logfilename=NULL;
	}

	
#ifndef _WIN32

	p_self->dbg.p_logfilename=safe_malloc(strlen(p_cmd->argv[current_nr])+1);

	strcpy(p_self->dbg.p_logfilename, p_cmd->argv[current_nr]);
#else

	if (create_file2(p_cmd->argv[current_nr]))

		nt_console_name2(&p_self->dbg.p_logfilename,p_cmd->argv[current_nr]);	
#endif

	
	return RC_OK;
}

static RC_TYPE get_username_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT	*p_self = (DYN_DNS_CLIENT *) p_context;
	

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->info.credentials.my_username) {

		free(p_self->info.credentials.my_username);
	}

	/*user*/
	p_self->info.credentials.my_username=safe_malloc(strlen(p_cmd->argv[current_nr])+1);

	strcpy(p_self->info.credentials.my_username, p_cmd->argv[current_nr]);

	return RC_OK;
}

static RC_TYPE get_password_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->info.credentials.my_password) {

		free(p_self->info.credentials.my_password);
	}

	/*password*/
	p_self->info.credentials.my_password=safe_malloc(strlen(p_cmd->argv[current_nr])+1);
	strcpy(p_self->info.credentials.my_password, (p_cmd->argv[current_nr]));
	return RC_OK;
}

/**
    Parses alias,hash ip_update_type.
    Example: blabla.domain.com,hashahashshahah
    Action:
	-search by ',' and replace the ',' with 0
	-read hash and alias, and ip update type
	01/23/11 -- added ip update type facility (bhoover@wecs.com)
				and side effect of setting related DYN_DNS_CLIENT, 
				DYN_DNS_INFO address type properties.
*/
static RC_TYPE do_get_alias_handler(CMD_DATA *p_cmd, int current_nr, void *p_context,char *ip_ver_str)
{
	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	char			*p_hash=NULL;
	int				alias_len=0;


	if (p_self == NULL)	{

		return RC_INVALID_POINTER;
	}

	if (p_self->alias_info.count-p_self->alias_info.count_dual >= DYNDNS_MAX_ALIAS_NUMBER) {

		return RC_DYNDNS_TOO_MANY_ALIASES;
	}

	/*hash*/
	p_hash = strstr(p_cmd->argv[current_nr],",");

	if (p_hash)	{

		if (p_self->alias_info.hashes[p_self->alias_info.count].str)

			free(p_self->alias_info.hashes[p_self->alias_info.count].str);

		/*allocate and copy hash len minus comma*/
		p_self->alias_info.hashes[p_self->alias_info.count].str=safe_malloc(strlen(p_hash));
		strcpy(p_self->alias_info.hashes[p_self->alias_info.count].str,(p_hash+1));

		alias_len=strlen(p_hash);
	}
	
	if (p_self->alias_info.names[p_self->alias_info.count].name)

		free(p_self->alias_info.names[p_self->alias_info.count].name);

	alias_len=strlen(p_cmd->argv[current_nr])-alias_len;

	/*alias*/
	p_self->alias_info.names[p_self->alias_info.count].name=safe_malloc(alias_len+1);
	strncpy(p_self->alias_info.names[p_self->alias_info.count].name,p_cmd->argv[current_nr],alias_len);

	/*default to letting ip server determine update type*/
	strcpy(p_self->alias_info.names[p_self->alias_info.count].ip_v,"auto");

	p_self->alias_info.names[p_self->alias_info.count].ip_v_enum=ip_store;

	/*IPv/type*/
	if (!(current_nr+1<p_cmd->argc)) {

		p_self->info.is_update_auto=true;
	}
	else {

		if (!(strncmp(p_cmd->argv[current_nr+1],"-",1))) {

			p_self->info.is_update_auto=true;
		}
		else {

			if ((strcmp(ip_ver_str,"ip4") && strcmp(ip_ver_str,"ip6") && strcmp(ip_ver_str,"auto")
				 && strcmp(ip_ver_str,DUAL_LIST))) {

				DBG_PRINTF((LOG_ERR,"E:" MODULE_TAG "Alias parameter option, ip version, %s, invalid...\n",ip_ver_str));

				return RC_DYNDNS_INVALID_OPTION;
			}
			else {

				strcpy(p_self->alias_info.names[p_self->alias_info.count].ip_v,ip_ver_str);

				if (strcmp(ip_ver_str,"auto")) {

					if (strcmp(ip_ver_str,DUAL_LIST)) {

						if (strcmp(ip_ver_str,"ip4"))

							p_self->alias_info.names[p_self->alias_info.count].ip_v_enum=ip_6;
						else
							p_self->alias_info.names[p_self->alias_info.count].ip_v_enum=ip_4;
					}
				}

				p_self->info.is_update_ip4=(p_self->info.is_update_ip4 || strstr(ip_ver_str,"ip4")
					 || strstr(ip_ver_str,DUAL_LIST));

				p_self->info.is_update_ip6=(p_self->info.is_update_ip6 || strstr(ip_ver_str,"ip6")
					 || strstr(ip_ver_str,DUAL_LIST));

				p_self->info.is_update_auto=(p_self->info.is_update_auto || strstr(ip_ver_str,"auto"));
			}
		}
	}

	p_self->alias_info.count++;


	return RC_OK;
}

static RC_TYPE get_alias_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	char			ip_ver_str[8]={'\0'};
	RC_TYPE			rc=RC_OK;


	if (!(current_nr+1<p_cmd->argc)) {

		p_self->info.is_update_auto=true;
	}
	else {

		if (!(strncmp(p_cmd->argv[current_nr+1],"-",1))) {

			p_self->info.is_update_auto=true;
		}
		else {

#ifndef _WIN32
			if (snprintf(ip_ver_str, 7, "%s", p_cmd->argv[current_nr+1]) < 0) {
#else
			if (_snprintf(ip_ver_str, 7, "%s", p_cmd->argv[current_nr+1]) < 0)	{
#endif
				
				return RC_DYNDNS_INVALID_OPTION;
			}
		}
	}

	if (strcmp(ip_ver_str,"dual")) {

		return do_get_alias_handler(p_cmd,current_nr,p_context,ip_ver_str);
	}
	else {

		if (!(RC_OK==(rc=do_get_alias_handler(p_cmd,current_nr,p_context,"ip4"))))

		  return rc;

		if ((RC_OK==(rc=do_get_alias_handler(p_cmd,current_nr,p_context,"ip6"))))

			p_self->alias_info.count_dual++;
	}

	return rc;
}

static RC_TYPE get_name_and_port(char *p_src, char **p_dest_name, int *p_dest_port)
{
    const char *p_port = NULL;


	if (*p_dest_name)

		free(*p_dest_name);


    p_port = strstr(p_src,":");
    if (p_port)
    {
        int port_nr, len;
        int port_ok = sscanf(p_port + 1, "%d",&port_nr);
        if (port_ok != 1)
        {
            return RC_DYNDNS_INVALID_OPTION;
        }
        *p_dest_port = port_nr;
        len = p_port - p_src;

	  *p_dest_name=safe_malloc(len);
	  memcpy(*p_dest_name, p_src, len);

        *(*p_dest_name+len) = '\0';
    }
    else
    {
	    
	    *p_dest_name=safe_malloc(strlen(p_src)+1);

	    strcpy(*p_dest_name, p_src);
    }
    return RC_OK;
}

/** Returns the svr name and port if the format is :
 * name[:port] url.
 */
static RC_TYPE set_url_param(CMD_DATA *p_cmd, int current_nr,DYNDNS_INFO_TYPE *info)
{
	RC_TYPE rc;
    int port = -1;

	if (info == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*user*/
    info->ip_server_name.port = HTTP_DEFAULT_PORT;

    rc = get_name_and_port(p_cmd->argv[current_nr], &info->ip_server_name.name[ip_store], &port);

    if (rc == RC_OK && port != -1)
    {
        info->ip_server_name.port = port;
    }        

    if (info->ip_server_url)

		free(info->ip_server_url);

    info->ip_server_url=safe_malloc(strlen(p_cmd->argv[current_nr + 1]) + 1);

    strcpy(info->ip_server_url, p_cmd->argv[current_nr + 1]);

    return rc;
}

static RC_TYPE get_ip_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

    return set_url_param(p_cmd,current_nr,&(((DYN_DNS_CLIENT *) (p_context))->info));
}

static RC_TYPE get_online_check_url_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

    return set_url_param(p_cmd,current_nr,&(((DYN_DNS_CLIENT *) (p_context))->info_online_status));
}

static RC_TYPE get_dns_server_name_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	RC_TYPE rc;
	int port = -1;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
   
    p_self->info.dyndns_server_name.port = HTTP_DEFAULT_PORT;
    rc = get_name_and_port(p_cmd->argv[current_nr], &p_self->info.dyndns_server_name.name[ip_store], &port);
    if (rc == RC_OK && port != -1)
    {
        p_self->info.dyndns_server_name.port = port;
    }                                   
	return rc;
}

RC_TYPE get_dns_server_url_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->info.dyndns_server_url)

		free(p_self->info.dyndns_server_url);

	p_self->info.dyndns_server_url=safe_malloc(strlen(p_cmd->argv[current_nr])+1);

	/*url*/
	strcpy(p_self->info.dyndns_server_url, p_cmd->argv[current_nr]);

	return RC_OK;
}

/* returns the proxy server name and port
*/
static RC_TYPE get_proxy_server_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
    RC_TYPE rc;
    int port = -1;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	/*user*/
    p_self->info.proxy_server_name.port = HTTP_DEFAULT_PORT;
    rc = get_name_and_port(p_cmd->argv[current_nr], &p_self->info.proxy_server_name.name[ip_store], &port);
    if (rc == RC_OK && port != -1)
    {
        p_self->info.proxy_server_name.port = port;
    }                                   
	return rc;    
}

/* Read the dyndnds name update period.
   and impose the max and min limits
*/
static RC_TYPE get_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;


	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->sleep_sec) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->sleep_sec /= 1000;
	p_self->sleep_sec = (p_self->sleep_sec < DYNDNS_MIN_SLEEP) ?  DYNDNS_MIN_SLEEP: p_self->sleep_sec;
	(p_self->sleep_sec > DYNDNS_MAX_SLEEP) ?  p_self->sleep_sec = DYNDNS_MAX_SLEEP: 1;

	return RC_OK;
}

static RC_TYPE get_update_period_sec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &p_self->sleep_sec) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->sleep_sec = (p_self->sleep_sec < DYNDNS_MIN_SLEEP) ?  DYNDNS_MIN_SLEEP: p_self->sleep_sec;
	(p_self->sleep_sec > DYNDNS_MAX_SLEEP) ?  p_self->sleep_sec = DYNDNS_MAX_SLEEP: 1;

	return RC_OK;
}

static RC_TYPE get_forced_update_period_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%u", &p_self->forced_update_period_sec) != 1 ||
	        sscanf(p_cmd->argv[current_nr], "%u", &p_self->forced_update_period_sec_orig) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->forced_update_period_sec = (p_self->forced_update_period_sec < DYNDNS_MIN_FORCED_UPDATE_PERIOD_S) ?  DYNDNS_MIN_FORCED_UPDATE_PERIOD_S: p_self->forced_update_period_sec;
	(p_self->forced_update_period_sec > DYNDNS_MAX_FORCED_UPDATE_PERIOD_S) ?  p_self->forced_update_period_sec = DYNDNS_MAX_FORCED_UPDATE_PERIOD_S: 1;

	p_self->forced_update_period_sec_orig=p_self->forced_update_period_sec;


	return RC_OK;
}

#ifdef USE_THREADS

/*round up cutoff for forced update period expiration calc.  map [-4 .. 5] to [0 .. .9]*/
static RC_TYPE get_forced_update_adjust_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	int				adjust;
	DYN_DNS_CLIENT	*p_self = (DYN_DNS_CLIENT *) p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (sscanf(p_cmd->argv[current_nr], "%d", &adjust) != 1)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	if ((adjust < DYNDNS_MIN_FORCED_UPDATE_ADJUST) || (adjust > DYNDNS_MAX_FORCED_UPDATE_ADJUST)) {
		
		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "forced_update_adjust, %d, not [%d..%d]...\n",adjust,DYNDNS_MIN_FORCED_UPDATE_ADJUST,DYNDNS_MAX_FORCED_UPDATE_ADJUST));

		return RC_DYNDNS_INVALID_OPTION;
	} 
	else {

		p_self->forced_update_adjust=(float) adjust*-1;

		p_self->forced_update_adjust=p_self->forced_update_adjust/10+.5f;
	}

	
	return RC_OK;
}

#endif

static RC_TYPE set_syslog_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}
	p_self->debug_to_syslog = TRUE;
	return RC_OK;
}

/**
 * Reads the params for change persona. Format:
 * <uid[:gid]>
 */
static RC_TYPE set_change_persona_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	{
		int gid = -1;
		int uid = -1;

		char *p_gid = strstr(p_cmd->argv[current_nr],":");
		if (p_gid)
		{
			if ((strlen(p_gid + 1) > 0) &&  /* if something is present after :*/
			        sscanf(p_gid + 1, "%d",&gid) != 1)
			{
				return RC_DYNDNS_INVALID_OPTION;
			}
		}
		if (sscanf(p_cmd->argv[current_nr], "%d",&uid) != 1)
		{
			return RC_DYNDNS_INVALID_OPTION;
		}

		p_self->change_persona = TRUE;
		p_self->sys_usr_info.gid = gid;
		p_self->sys_usr_info.uid = uid;
	}
	return RC_OK;
}

RC_TYPE print_version_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	DBG_PRINTF((LOG_SYSTEM,"Version: %s\n", DYNDNS_VERSION_STRING));
	p_self->abort = TRUE;
	return RC_OK;
}

static RC_TYPE get_exec_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (p_self->external_command)

		free(p_self->external_command);

	p_self->external_command=safe_malloc(strlen(p_cmd->argv[current_nr])+1);

	strcpy(p_self->external_command, p_cmd->argv[current_nr]);
	return RC_OK;
}

static RC_TYPE set_file_param(char **dest, char *dir,char *file_name)
{
	RC_TYPE	rc=RC_FILE_IO_OPEN_ERROR;
	char		*src_ptr=NULL;	
	int		is_file_exists;
	int		slen;
	

	slen=strlen(dir)+strlen(DIR_DELIM_STR)+strlen(file_name)+1;

	src_ptr=safe_malloc(slen);

	sprintf(src_ptr,"%s" DIR_DELIM_STR "%s",dir,file_name);

	if (!(is_file_exists=is_file(src_ptr)))

		is_file_exists=create_file2(src_ptr);

	if (is_file_exists) {

		rc=RC_OK;

#ifdef _WIN32

		nt_console_name2(dest,src_ptr);
#else
		*dest=safe_malloc(slen);

		sprintf(*dest, "%s" DIR_DELIM_STR "%s",dir,file_name);
#endif
		
	}

	free(src_ptr);


	return rc;
}

static RC_TYPE get_cache_dir(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;

	char *ip_cache=NULL;
	char *time_cache=NULL;


	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	if (RC_OK==set_file_param(&ip_cache,p_cmd->argv[current_nr],DYNDNS_DEFAULT_IP_FILE)) {

		if (p_self->ip_cache) {

			free(p_self->ip_cache);			
		}

		p_self->ip_cache=ip_cache;
	}

	if (RC_OK==set_file_param(&time_cache,p_cmd->argv[current_nr],DYNDNS_DEFAULT_TIME_FILE)) {

		if (p_self->time_cache) {

			free(p_self->time_cache);
		}

		p_self->time_cache=time_cache;
	}


	return RC_OK;
}

/**
    Searches the DYNDNS system by the argument.
    Input is like: system@server.name
    system=statdns|custom|dyndns|default
    server name = dyndns.org | freedns.afraid.org
    The result is a pointer in the table of DNS systems.
*/
static RC_TYPE get_dyndns_system_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	DYNDNS_SYSTEM *p_dns_system = NULL;
	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;
	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	{
		DYNDNS_SYSTEM_INFO *it = get_dyndns_system_table();
		for (; it != NULL && it->id != LAST_DNS_SYSTEM; ++it)
		{
			if (strcmp(it->system.p_key, p_cmd->argv[current_nr]) == 0)
			{
				p_dns_system = &it->system;
			}
		}
	}

	if (p_dns_system == NULL)
	{
		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->info.p_dns_system = p_dns_system;

	return RC_OK;
}

static RC_TYPE get_debug_level_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

  #define ASCII_ZERO     48

	int                    dwLevel=0;


	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;

	if (p_self == NULL)
	{
		return RC_INVALID_POINTER;
	}

	dwLevel=*(p_cmd->argv[current_nr])-ASCII_ZERO;

	if (dwLevel<LOG_EMERG || dwLevel>LOG_DEBUG)

		return RC_DYNDNS_INVALID_OPTION;

	p_self->dbg.level=dwLevel;

	return RC_OK;
}

static RC_TYPE get_retries_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	int	retries=0;


	DYN_DNS_CLIENT *p_self=(DYN_DNS_CLIENT *) p_context;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%d",&retries) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (retries<0 || retries>100)

		return RC_DYNDNS_INVALID_OPTION;


	p_self->net_retries=retries;


	return RC_OK;
}

static RC_TYPE get_retry_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	int	retry_interval=0;


	DYN_DNS_CLIENT *p_self=(DYN_DNS_CLIENT *) p_context;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%d",&retry_interval) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (retry_interval<DYNDNS_MIN_RETRY_INTERVAL || retry_interval>DYNDNS_MAX_RETRY_INTERVAL)

		return RC_DYNDNS_INVALID_OPTION;


	p_self->retry_interval=retry_interval;


	return RC_OK;
}

static RC_TYPE get_retry_pending_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{
	int	retry_interval=0;


	DYN_DNS_CLIENT *p_self=(DYN_DNS_CLIENT *) p_context;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%d",&retry_interval) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (retry_interval<DYNDNS_MIN_RETRY_PENDING_INTERVAL || retry_interval>DYNDNS_MAX_RETRY_PENDING_INTERVAL)

		return RC_DYNDNS_INVALID_OPTION;


	p_self->retry_pending_interval=retry_interval;


	return RC_OK;
}

static RC_TYPE get_retry_pending_off_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT *p_self=(DYN_DNS_CLIENT *) p_context;


	if (p_self==NULL)

		return RC_INVALID_POINTER;
	

	p_self->retry_pending_off=(!(p_self->retry_pending_off));


	return RC_OK;
}

static RC_TYPE get_lang_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT *p_self=(DYN_DNS_CLIENT *) p_context;


	/*
	use harded coded defaults if have 
	--lang_file parameter with no option --
	otherwise, use option indicated language
	strings file, and if not there, program 
	will automatically use locale indicated 
	language strings file, defaulting to
	en.lng if locale indicated file not
	present.
	*/

	if (p_self==NULL)

		return RC_INVALID_POINTER;

	p_self->lang_hard_coded=1;


	if (current_nr>=p_cmd->argc) /*no parameter -- means use hard coded*/

		return RC_OK;

	if (!(strncmp(p_cmd->argv[current_nr],"-",1))) /*no parameter -- means use hard coded*/

		return RC_OK;


	p_self->lang_hard_coded=0;


	if ((is_file(p_cmd->argv[current_nr]))) { /*ignore if invalid*/

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Using default override language strings file, %s...\n",p_cmd->argv[current_nr]));
	}
	else {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Could not open default override language strings file, %s...\n",p_cmd->argv[current_nr]));

		return RC_OK;
	}

	if (p_self->lang_file)

		free(p_self->lang_file);

#ifdef _WIN32

	nt_console_name2(&(p_self->lang_file),p_cmd->argv[current_nr]);

#else

	p_self->lang_file=safe_malloc(strlen(p_cmd->argv[current_nr])+1);

	strcpy(p_self->lang_file,p_cmd->argv[current_nr]);

#endif


	return RC_OK;
}

#ifdef USE_SNDFILE

static RC_TYPE get_audible_on_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT *p_self=(DYN_DNS_CLIENT *) p_context;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	p_self->audible_off=(!(p_self->audible_off));


	return RC_OK;
}

static RC_TYPE get_wave_loops_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	int			loops;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%d",&loops) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (((loops < DYNDNS_MIN_WAVE_LOOPS || loops > DYNDNS_MAX_WAVE_LOOPS) && loops!=DYNDNS_INFINITE_WAVE)) { /*ignore if invalid*/

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "wave_loops, %s, not [%d or %d..%d]...\n",p_cmd->argv[current_nr],DYNDNS_INFINITE_WAVE,DYNDNS_MIN_WAVE_LOOPS,DYNDNS_MAX_WAVE_LOOPS));

		return RC_DYNDNS_INVALID_OPTION;
	}


	p_self->wave_loops=loops;


	return RC_OK;
}

static RC_TYPE get_wave_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT *p_self=(DYN_DNS_CLIENT *) p_context;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if ((is_file(p_cmd->argv[current_nr]))) { /*ignore if invalid*/

		DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Using default override wave file, %s...\n",p_cmd->argv[current_nr]));
	}
	else {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "Could not open default override wave file, %s...\n",p_cmd->argv[current_nr]));

		return RC_OK;
	}

	/*in case already allocated for default*/
	free(p_self->wave_file);

#ifdef _WIN32

	nt_console_name2(&(p_self->wave_file),p_cmd->argv[current_nr]);

#else

	p_self->wave_file=safe_malloc(strlen(p_cmd->argv[current_nr])+1);

	strcpy(p_self->wave_file,p_cmd->argv[current_nr]);

#endif

	if (current_nr+1<p_cmd->argc)

		if (strncmp(p_cmd->argv[current_nr+1],"-",1))

			return get_wave_loops_handler(p_cmd,current_nr+1,p_context);

	return RC_OK;
}

static RC_TYPE get_alert_retries_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	int				retries=0;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%d",&retries) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (retries < DYNDNS_MIN_ALERT_RETRIES || retries > DYNDNS_MAX_ALERT_RETRIES) { /*ignore if invalid*/

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "alert_retries, %s, not [%d..%d]...\n",p_cmd->argv[current_nr],DYNDNS_MIN_ALERT_RETRIES,DYNDNS_MAX_ALERT_RETRIES));

		return RC_DYNDNS_INVALID_OPTION;
	}


	p_self->alert_retries=retries;


	return RC_OK;
}

static RC_TYPE get_alert_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	int			interval=0;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%i",&interval) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (interval < DYNDNS_MIN_ALERT_INTERVAL || interval > DYNDNS_MAX_ALERT_INTERVAL) {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "alert_retries, %s, not [%d..%d]...\n",p_cmd->argv[current_nr],DYNDNS_MIN_ALERT_INTERVAL,DYNDNS_MAX_ALERT_INTERVAL));

		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->alert_interval=interval;


	return RC_OK;
}

/*
	Allow wave attenuation to -120db

	User interface gain [-10..10] (present values of related constants) 
	is translated to [-20..0]

	This in turn is translated according to waveout.c's WAVE_DECIBEL_STEP
*/
RC_TYPE set_wave_gain(DYN_DNS_CLIENT *p_self,float gain)
{

	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (gain < DYNDNS_MIN_WAVE_GAIN || gain > DYNDNS_MAX_WAVE_GAIN) {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "wave_gain, %d, not [%d..%d]...\n",gain,DYNDNS_MIN_WAVE_GAIN,DYNDNS_MAX_WAVE_GAIN));

		return RC_DYNDNS_INVALID_OPTION;
	}

	if (!(gain))

		gain=DYNDNS_DEFAULT_WAVE_GAIN;

	else
		gain=DYNDNS_MIN_WAVE_GAIN+gain;		


	p_self->wave_gain=gain*DYNDNS_DECIBEL_STEP/WAVE_DECIBEL_STEP;

	
	return RC_OK;
}

static RC_TYPE get_wave_gain_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	float			gain=0;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%f",&gain) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	return set_wave_gain(p_self,gain);
}

static RC_TYPE get_wave_buff_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	float			buff_fact=0;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%f",&buff_fact) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (buff_fact>=DYNDNS_MIN_WAVE_BUFF_FACTOR && buff_fact<=DYNDNS_MAX_WAVE_BUFF_FACTOR)

		p_self->wave_buff_factor=buff_fact;
	else

		return RC_DYNDNS_INVALID_OPTION;


	return RC_OK;
}

#ifdef USE_THREADS

static RC_TYPE get_status_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	int				interval=0;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%i",&interval) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (interval < DYNDNS_MIN_STATUS_INTERVAL || interval > DYNDNS_MAX_STATUS_INTERVAL) {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "status_interval, %s, not [%i..%i]...\n",p_cmd->argv[current_nr],
					DYNDNS_MIN_STATUS_INTERVAL,DYNDNS_MAX_STATUS_INTERVAL));
		

		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->status_interval=interval*1000;


	return RC_OK;

}

static RC_TYPE get_status_offline_interval_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	DYN_DNS_CLIENT	*p_self=(DYN_DNS_CLIENT *) p_context;
	int				interval=0;


	if (p_self==NULL)

		return RC_INVALID_POINTER;


	if (sscanf(p_cmd->argv[current_nr], "%i",&interval) != 1)

		return RC_DYNDNS_INVALID_OPTION;


	if (interval < DYNDNS_MIN_STATUS_OFFLINE_INTERVAL || interval > DYNDNS_MAX_STATUS_OFFLINE_INTERVAL) {

		DBG_PRINTF((LOG_WARNING,"W:" MODULE_TAG "status_offline_interval, %s, not [%i..%i]...\n",p_cmd->argv[current_nr],
					DYNDNS_MIN_STATUS_OFFLINE_INTERVAL,DYNDNS_MAX_STATUS_OFFLINE_INTERVAL));

		return RC_DYNDNS_INVALID_OPTION;
	}

	p_self->status_offline_interval=interval*1000;


	return RC_OK;

}

#endif /*USE_THREADS*/
#endif /*USE_SNDFILE*/

static RC_TYPE push_in_buffer(char* p_src, int src_len, char *p_buffer, int* p_act_len, int max_len)
{
	if (*p_act_len + src_len > max_len)
	{
		return RC_FILE_IO_OUT_OF_BUFFER;
	}
	memcpy(p_buffer + *p_act_len,p_src, src_len);
	*p_act_len += src_len;
	return RC_OK;
}

typedef enum
{
    NEW_LINE,
    COMMENT,
    DATA,
    SPACE,
    ESCAPE,
    QUOTED_DATA,
    QUOTE_END
} PARSER_STATE;

typedef struct
{
	FILE *p_file;
	PARSER_STATE state;
} OPTION_FILE_PARSER;

typedef RC_TYPE (*UTF_PARSE_FUNC)(OPTION_FILE_PARSER *p_cfg, char *p_buffer, int maxlen);

static RC_TYPE parser_init(OPTION_FILE_PARSER *p_cfg, FILE *p_file)
{
	memset(p_cfg, 0, sizeof(*p_cfg));
	p_cfg->state = NEW_LINE;
	p_cfg->p_file = p_file;

	return RC_OK;
}

static RC_TYPE do_parser_read_option(OPTION_FILE_PARSER *p_cfg,
                                     char *p_buffer,
                                     int maxlen,
                                     char *ch,
                                     int *count,
                                     BOOL *parse_end)
{

	RC_TYPE rc = RC_OK;


	switch (p_cfg->state)
	{
	case NEW_LINE:

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{
			return rc;
		}

		if (!(strcmp(ch,"\\")))
		{
			p_cfg->state = ESCAPE;

			return rc;
		}

		if (!(strcmp(ch,"#"))) /*comment*/
		{
			p_cfg->state = COMMENT;

			return rc;
		}

		if ((strcmp(ch," ")) && (strcmp(ch,"	")))
		{
			if (strcmp(ch,"-"))   /*add '--' to first word in line*/
			{
				if ((rc = push_in_buffer("--", 2, p_buffer, count, maxlen)) != RC_OK)
				{
					return rc;
				}
			}
			if ((rc = push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen)) != RC_OK)
			{
				return rc;
			}

			p_cfg->state = DATA;

			return rc;
		}

		/*skip actual leading (before option) spaces*/
		return rc;

	case SPACE:

		if (!(strcmp(ch,"\"")))
		{
			p_cfg->state = QUOTED_DATA;

			return rc;
		}

		if (!(strcmp(ch,"\\")))
		{
			p_cfg->state = ESCAPE;

			return rc;
		}

		if (!(strcmp(ch,"#"))) /*comment*/
		{
			p_cfg->state = COMMENT;

			return rc;
		}

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{
			p_cfg->state = NEW_LINE;

			return rc;
		}

		if ((strcmp(ch," ")) && (strcmp(ch,"	")))
		{
			if ((rc = push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen)) != RC_OK)
			{
				return rc;
			}
			p_cfg->state = DATA;
		}

		return rc;

	case COMMENT:

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{
			p_cfg->state = NEW_LINE;
		}

		/*skip comments*/
		return rc;

	case DATA:

		if (!(strcmp(ch,"\\")))
		{
			p_cfg->state = ESCAPE;

			return rc;
		}

		if (!(strcmp(ch,"#")))
		{
			p_cfg->state = COMMENT;

			return rc;
		}

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")))
		{

			p_cfg->state = NEW_LINE;
			*parse_end = TRUE;

			return rc;
		}

		if (!(strcmp(ch," ")) || !(strcmp(ch,"	")))
		{
			p_cfg->state = SPACE;
			*parse_end = TRUE;

			return rc;
		}

		/*actual data*/

		return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);

	case QUOTED_DATA:

		if (!(strcmp(ch,"\"")))
		{

			p_cfg->state=QUOTE_END;

			return rc;
		}

		if (strcmp(ch,"\n") && strcmp(ch,"\r")) {

			/*actual data*/

			return push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen);
		}

		return RC_CMD_PARSER_INVALID_OPTION;

	case QUOTE_END:

		if (!(strcmp(ch,"\n")) || !(strcmp(ch,"\r")) || !(strcmp(ch," ")) || !(strcmp(ch,"	")))
		{

			p_cfg->state=NEW_LINE;

			*parse_end=1;

			return rc;
		}

		return RC_CMD_PARSER_INVALID_OPTION;

	case ESCAPE:

		if ((rc = push_in_buffer(ch, strlen(ch), p_buffer, count, maxlen)) != RC_OK)
		{
			return rc;
		}

		p_cfg->state = DATA;

		return rc;

	default:

		return RC_CMD_PARSER_INVALID_OPTION;
	}
}

/** Read one single option from utf-8 file into the given buffer.
	When the first separator is encountered it returns.
	Actions:
		- read chars while not eof
		- skip comments (parts beginning with '#' and ending with '\n')
		- switch to DATA STATE if non space char is encountered
		- assume first name in lines to be a long option name by adding '--' if necesssary
		- add data to buffer
		- do not forget a 0 at the end
 * States:
 * NEW_LINE - wait here until some option. Add '--' if not already there
 * SPACE - between options. Like NEW_LINE but no additions
 * DATA - real data. Stop on space.
 * COMMENT - everything beginning with # until EOLine
 * ESCAPE - everything that is otherwise (incl. spaces). Next char is raw copied.
*/
static RC_TYPE parser_utf_8_read_option(OPTION_FILE_PARSER *p_cfg, char *p_buffer, int maxlen)
{
	RC_TYPE rc = RC_OK;
	BOOL parse_end = FALSE;
	int count = 0;
	char c_buff[7];


	*p_buffer = 0;


	while(!parse_end)
	{

		memset(c_buff,0,7);

		if (!(utf_read_utf_8(c_buff,p_cfg->p_file)))
		{
			if (feof(p_cfg->p_file))
			{
				break;
			}

			rc = RC_FILE_IO_READ_ERROR;

			break;
		}

		rc=do_parser_read_option(p_cfg,p_buffer,maxlen,c_buff,&count,&parse_end);

		if (!(rc == RC_OK))

			return rc;
	}


	rc = push_in_buffer("\0",1,p_buffer,&count,maxlen);

	if (p_cfg->state==QUOTED_DATA) /*didn't get an end quote*/

		return RC_CMD_PARSER_INVALID_OPTION;


	return rc;
}

#ifdef _WIN32

/** Read one single option from utf-16 file into the given buffer.
	When the first separator is encountered it returns.
	Actions:
		- read chars while not eof
		- skip comments (parts beginning with '#' and ending with '\n')
		- switch to DATA STATE if non space char is encountered
		- assume first name in lines to be a long option name by adding '--' if necesssary
		- add data to buffer
		- do not forget a 0 at the end
 * States:
 * NEW_LINE - wait here until some option. Add '--' if not already there
 * SPACE - between options. Like NEW_LINE but no additions
 * DATA - real data. Stop on space.
 * COMMENT - everything beginning with # until EOLine
 * ESCAPE - everything that is otherwise (incl. spaces). Next char is raw copied.
*/
static RC_TYPE parser_utf_16_read_option(OPTION_FILE_PARSER *p_cfg, char *p_buffer, int maxlen)
{
	RC_TYPE rc = RC_OK;
	BOOL parse_end = FALSE;
	int count = 0;
	char utf_8[18];
	char c_buff[4]={0,0,0,0};

	*p_buffer = 0;


	while(!parse_end)

	{
		if (feof(p_cfg->p_file))
		{
			break;
		}

		c_buff[0]=fgetc(p_cfg->p_file);

		if ((c_buff[0]==EOF))
		{
			rc = RC_FILE_IO_READ_ERROR;

			break;
		}

		if (feof(p_cfg->p_file))
		{
			rc = RC_FILE_IO_READ_ERROR;

			break;
		}

		c_buff[1]=fgetc(p_cfg->p_file);

		if ((c_buff[1]==EOF))
		{
			rc = RC_FILE_IO_READ_ERROR;

			break;
		}

		/*convert to utf-8, and pass to parser*/

		rc=do_parser_read_option(p_cfg,p_buffer,maxlen,utf_16_to_8(utf_8,(wchar_t *) c_buff),&count,&parse_end);

		if (!(rc == RC_OK))

			return rc;
	}


	{
		char ch = 0;
		rc = push_in_buffer(&ch, 1, p_buffer, &count, maxlen);
	}

	if (p_cfg->state==QUOTED_DATA) /*didn't get an end quote*/

		return RC_CMD_PARSER_INVALID_OPTION;


	return rc;
}

#endif

static RC_TYPE do_get_options_from_file(CMD_DATA *p_cmd,int current_nr,void *p_context)
{

	RC_TYPE rc = RC_OK;
	FILE *p_file = NULL;
	char *p_tmp_buffer = NULL;
	const int buffer_size = DYNDNS_SERVER_NAME_LENGTH;
	OPTION_FILE_PARSER parser;

	UTF_PARSE_FUNC parse_func=parser_utf_8_read_option;

#ifdef _WIN32

	int		is_bom=0;	/*win32 utf byte order mark?*/
	int		is_bom_8=0;
#endif


	do
	{
		p_tmp_buffer = safe_malloc(buffer_size);
		if (!p_tmp_buffer)
		{
			rc = RC_OUT_OF_MEMORY;
			break;
		}


		p_file = utf_fopen(p_cmd->argv[current_nr], "r");


		if (!(p_file))

		{
			DBG_PRINTF((LOG_ERR,"W:" MODULE_TAG "Cannot open cfg file:%s\n", p_cmd->argv[current_nr]));
			rc = RC_FILE_IO_OPEN_ERROR;
			break;
		}

#ifdef _WIN32

		{

			wchar_t *utf_16;


			utf_8_to_16(utf_malloc_8_to_16(&utf_16,p_cmd->argv[current_nr]),p_cmd->argv[current_nr]);



			if (!(utf_is_win_utf_file(utf_16,&is_bom)))

				is_bom_8=is_bom;

			else

				parse_func=parser_utf_16_read_option;


			free(utf_16);
		}



		if (is_bom_8)

			fseek(p_file,3,0);

		else

			if (is_bom)

				fseek(p_file,2,0);

#endif

		if ((rc = parser_init(&parser, p_file)) != RC_OK)
		{
			break;
		}

		while(!feof(p_file))
		{
			rc = parse_func(&parser,p_tmp_buffer, buffer_size);

			if (rc != RC_OK)
			{
				break;
			}

			if (!strlen(p_tmp_buffer))
			{
				break;
			}

			rc = cmd_add_val(p_cmd, p_tmp_buffer);
			if (rc != RC_OK)
			{
				break;
			}
		}

	}
	while(0);


	if (p_file)
	{
		fclose(p_file);
	}
	if (p_tmp_buffer)
	{
		free(p_tmp_buffer);
	}

	return rc;
}

/**
	This handler reads the data in the passed file name.
	Then appends the words in the table (cutting all spaces) to the existing cmd line options.
	It adds to the CMD_DATA struct.
	Actions:
		- open file
		- read characters and cut spaces away
		- add values one by one to the existing p_cmd data
*/
static RC_TYPE get_options_from_file_handler(CMD_DATA *p_cmd, int current_nr, void *p_context)
{

	RC_TYPE		rc=RC_OK;
	int			curr_argc=0;
	int			i;
	int			destIndex;

	char		**arg_list;


	DYN_DNS_CLIENT *p_self = (DYN_DNS_CLIENT *) p_context;

	if (!p_self || !p_cmd)
	{
		return RC_INVALID_POINTER;
	}

	curr_argc=p_cmd->argc;

	rc=do_get_options_from_file(p_cmd,current_nr,p_context);

	if (!(rc==RC_OK))

		return rc;

	/*fix up so file arguments have been inserted after current_nr*/

	arg_list=safe_malloc(p_cmd->argc*sizeof(char *));

	current_nr++;

	for (i=0;i<current_nr;i++) {

		arg_list[i]=safe_malloc(strlen(p_cmd->argv[i])+1);

		strcpy(arg_list[i],p_cmd->argv[i]);

		free(p_cmd->argv[i]);
	}

	for (i=curr_argc,destIndex=current_nr;i<p_cmd->argc;i++,destIndex++) {

		arg_list[destIndex]=safe_malloc(strlen(p_cmd->argv[i])+1);

		strcpy(arg_list[destIndex],p_cmd->argv[i]);

		free(p_cmd->argv[i]);
	}

	for (i=current_nr;i<curr_argc;i++,destIndex++) {

		arg_list[destIndex]=safe_malloc(strlen(p_cmd->argv[i])+1);

		strcpy(arg_list[destIndex],p_cmd->argv[i]);

		free(p_cmd->argv[i]);
	}

	free(p_cmd->argv);

	p_cmd->argv=arg_list;


	return rc;
}

RC_TYPE do_get_default_infile_config(char *in_file,DYN_DNS_CLIENT *p_self,CMD_OPTION_ERR_HANDLER_FUNC pf_err_handler)
{

	RC_TYPE		rc = RC_OK;
	int			custom_argc = 3;


	char **custom_argv = safe_malloc(sizeof(char **)*3);


	custom_argv[0] = safe_malloc(2);
	strcpy(custom_argv[0],"");

	custom_argv[1] = safe_malloc(strlen(DYNDNS_INPUT_FILE_OPT_STRING)+1);
	strcpy(custom_argv[1],DYNDNS_INPUT_FILE_OPT_STRING);

	custom_argv[2] = safe_malloc(strlen(in_file)+1);
	strcpy(custom_argv[2],in_file);


	DBG_PRINTF((LOG_INFO,"I:" MODULE_TAG "Attempting using default config file %s\n", in_file));


	rc = get_cmd_parse_data_with_error_handling(custom_argv, custom_argc, cmd_options_table,pf_err_handler);

	free(custom_argv[0]);
	free(custom_argv[1]);
	free(custom_argv[2]);

	free(custom_argv);


	return rc;
}

/*
	try inadyn-mt config file first
	and if it's not there, try mt 
	config file
*/
RC_TYPE get_default_infile_config(DYN_DNS_CLIENT *p_self,CMD_OPTION_ERR_HANDLER_FUNC pf_err_handler)
{

	RC_TYPE rc = RC_OK;


	rc = do_get_default_infile_config(DYNDNS_MT_DEFAULT_CONFIG_FILE,p_self,pf_err_handler);

	if (rc==RC_FILE_IO_OPEN_ERROR) {

		rc = do_get_default_infile_config(DYNDNS_MT_DEFAULT_CONFIG_FILE_OLD,p_self,pf_err_handler);

		if (rc==RC_FILE_IO_OPEN_ERROR) {

			rc = do_get_default_infile_config(DYNDNS_DEFAULT_CONFIG_FILE,p_self,pf_err_handler);
		}
	}


	return rc;
}

/*
	Set up all details:
		- ip server name
		- dns server name
		- username, passwd
		- ...
	Implementation:
		- load defaults
		- parse cmd line
        - assign settings that may change due to cmd line options
		- check data
	Note:
		- if no argument is specified tries to call the cmd line parser
	with the default cfg file path.
*/
RC_TYPE get_config_data_with_error_handling(DYN_DNS_CLIENT *p_self, int argc, char** argv,CMD_OPTION_ERR_HANDLER_FUNC pf_err_handler)
{
	RC_TYPE rc = RC_OK;
	FILE *fp;
	char cached_time[80];


	do
	{
		/*set up the context pointers */
		{
			CMD_DESCRIPTION_TYPE *it = cmd_options_table;
			while( it->p_option != NULL)
			{
				it->p_handler.p_context = (void*) p_self;
				++it;
			}
		}
		/* in case of no options, assume the default cfg file may be present */
		if (argc == 1)
		{

			rc = get_default_infile_config(p_self,pf_err_handler);
		}
		else
		{
			rc = get_cmd_parse_data_with_error_handling(argv, argc, cmd_options_table,pf_err_handler);
		}

		if (rc != RC_OK ||
		        p_self->abort)
		{
			break;
		}

		/*settings that may change due to cmd line options*/
		{
			int	len=0;

			
    		/*ip server*/
            if (!(p_self->info.ip_server_name.name[ip_store]))
            {
				p_self->info.ip_server_name.name[ip_store]=safe_malloc(strlen(p_self->info.p_dns_system->p_ip_server_name)+1);
				strcpy(p_self->info.ip_server_name.name[ip_store], p_self->info.p_dns_system->p_ip_server_name);

				p_self->info.ip_server_url=safe_malloc(strlen(p_self->info.p_dns_system->p_ip_server_url)+1);
				strcpy(p_self->info.ip_server_url, p_self->info.p_dns_system->p_ip_server_url);				
            }

            if (!(p_self->info_online_status.ip_server_name.name[ip_store]))
			{
				p_self->info_online_status.ip_server_name.name[ip_store]=safe_malloc(strlen(DYNDNS_MY_ONLINE_CHECK)+1);
				strcpy(p_self->info_online_status.ip_server_name.name[ip_store],DYNDNS_MY_ONLINE_CHECK);

				p_self->info_online_status.ip_server_url=safe_malloc(strlen(DYNDNS_MY_ONLINE_CHECK_URL)+1);
				strcpy(p_self->info_online_status.ip_server_url,DYNDNS_MY_ONLINE_CHECK_URL);

				p_self->info_online_status.ip_server_name.port=80;
            }

    		/*dyndns server*/
            if (!(p_self->info.dyndns_server_name.name[ip_store]))
            {
				if (!(0==(len=strlen(p_self->info.p_dns_system->p_dyndns_server_name)))) {

					p_self->info.dyndns_server_name.name[ip_store]=safe_malloc(len+1);
        			strcpy(p_self->info.dyndns_server_name.name[ip_store], p_self->info.p_dns_system->p_dyndns_server_name);
				}
			}

			if (!(p_self->info.dyndns_server_url)) 
			{
				if (!(0==(len=strlen(p_self->info.p_dns_system->p_dyndns_server_url)))) {

					p_self->info.dyndns_server_url=safe_malloc(len+1);
      				strcpy(p_self->info.dyndns_server_url, p_self->info.p_dns_system->p_dyndns_server_url);
				}
			}
		}

		/*check if the neccessary params have been provided*/
		if ((!(p_self->info.dyndns_server_name.name[ip_store])) || (!(p_self->info.ip_server_name.name[ip_store]))
			|| (!(p_self->info.dyndns_server_url)) || (p_self->alias_info.count == 0))

		{
			rc = RC_DYNDNS_INVALID_OR_MISSING_PARAMETERS;
			break;
		}
		/*forced update*/
		if ((fp=utf_fopen(p_self->time_cache, "r")))
		{
			if (fgets (cached_time, sizeof (cached_time), fp)) {

				time_t			current;
				unsigned long	cached;
				unsigned long	dif;
				unsigned long	fup_store=p_self->forced_update_period_sec;

				sscanf(cached_time,"%lu",&cached);
				current=time(NULL);


				if ((unsigned long) current<=cached) /*clock turned back?*/

					p_self->forced_update_period_sec=0;

				else {

					dif=current-cached;

					p_self->forced_update_period_sec-=dif;


					/*unsigned wrap around?*/

					if (fup_store<p_self->forced_update_period_sec)

						p_self->forced_update_period_sec=0;
				}
			}

			fclose(fp);
		}

		/*deprecated out, but may make a comeback, w/a compiler option*/
		p_self->times_since_last_update = 0;
		p_self->forced_update_times = p_self->forced_update_period_sec / p_self->sleep_sec;

		p_self->is_forced_update_attempted=(p_self->forced_update_period_sec==0);
	}
	while(0);

	return rc;
}

RC_TYPE get_config_data(DYN_DNS_CLIENT *p_self, int argc, char** argv)
{

	return get_config_data_with_error_handling(p_self,argc,argv,NULL);
}

