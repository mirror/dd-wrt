#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <broadcom.h>

#ifdef FILTER_DEBUG
extern FILE *debout;

#define D(a) fprintf(debout,"%s\n",a); fflush(debout);
#else
#define D(a)
#endif

/*
 * Format: filter_rule{1...10}=$STAT:1$NAME:test1$ (1=>disable 2=>enable)
 * 
 * Format: filter_tod{1...10} = hr:min hr:min wday filter_tod_buf{1...10} =
 * sun mon tue wed thu fri sat //only for web page read Example: Everyday and 
 * 24-hour filter_todXX = 0:0 23:59 0-0 filter_tod_bufXX = 7 (for web)
 * 
 * From 9:55 to 22:00 every sun, wed and thu filter_todXX = 9:55 22:00 0,3-4
 * filter_tod_bufXX = 1 0 1 1 0 0 0 (for web)
 * 
 * Format: filter_ip_grp{1...10} = ip1 ip2 ip3 ip4 ip5 ip6 ip_r1-ipr2
 * ip_r3-ip_r4 filter_ip_mac{1...10} = 00:11:22:33:44:55 00:12:34:56:78:90
 * 
 * Format: filter_port=udp:111-222 both:22-33 disable:22-333 tcp:11-22222
 * 
 * Converting Between AM/PM and 24 Hour Clock: Converting from AM/PM to 24
 * hour clock: 12:59 AM -> 0059 (between 12:00 AM and 12:59 AM, subtract 12
 * hours) 10:00 AM -> 1000 (between 1:00 AM and 12:59 PM, a straight
 * conversion) 10:59 PM -> 2259 (between 1:00 PM and 11:59 PM, add 12 hours)
 * Converting from 24 hour clock to AM/PM 0059 -> 12:59 AM (between 0000 and
 * 0059, add 12 hours) 0100 -> 1:00 AM (between 0100 and 1159, straight
 * converion to AM) 1259 -> 12:59 PM (between 1200 and 1259, straight
 * converion to PM) 1559 -> 3:59 PM (between 1300 and 2359, subtract 12
 * hours)
 * 
 */

int filter_id = 1;
int day_all = 0, week0 = 0, week1 = 0, week2 = 0, week3 = 0, week4 =
    0, week5 = 0, week6 = 0;
int start_week = 0, end_week = 0;
int time_all = 0, start_hour = 0, start_min = 0, start_time = 0, end_hour =
    0, end_min = 0, end_time = 0;
int tod_data_null = 0;

/*
 * Example: 100-200 250-260 (ie. 192.168.1.100-192.168.1.200
 * 192.168.1.250-192.168.1.260) 
 */

char *filter_ip_get( char *type, int which )
{
    static char word[256];
    char *start, *end, *wordlist, *next;
    int temp;
    char filter_ip[] = "filter_ip_grpXXX";

    D( "filter_ip_get" );
    snprintf( filter_ip, sizeof( filter_ip ), "filter_ip_grp%s",
	      nvram_safe_get( "filter_id" ) );

    wordlist = nvram_safe_get( filter_ip );
    if( !wordlist )
	return "0";

    temp = which;

    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    if( temp == 6 )
	    {
		end = word;
		start = strsep( &end, "-" );
		if( !strcmp( type, "ip_range0_0" ) )
		    return start;
		else
		    return end;
	    }
	    else if( temp == 7 )
	    {
		end = word;
		start = strsep( &end, "-" );
		if( !strcmp( type, "ip_range1_0" ) )
		    return start;
		else
		    return end;
	    }
	    D( "return word" );
	    return word;
	}
    }
    D( "return zero" );
    return "0";
}

/*
 * Example: tcp:100-200 udp:210-220 both:250-260 
 */

char *filter_dport_get( char *type, int which )
{
    static char word[256];
    char *wordlist, *next;
    char *start, *end, *proto;
    int temp;
    char name[] = "filter_dport_grpXXX";

    sprintf( name, "filter_dport_grp%s", nvram_safe_get( "filter_id" ) );
    wordlist = nvram_safe_get( name );
    temp = which;
    D( "filter dport get" );
    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    start = word;
	    proto = strsep( &start, ":" );
	    end = start;
	    start = strsep( &end, "-" );
	    if( !strcmp( type, "disable" ) )
	    {
		if( !strcmp( proto, "disable" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "both" ) )
	    {
		if( !strcmp( proto, "both" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "tcp" ) )
	    {
		if( !strcmp( proto, "tcp" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "udp" ) )
	    {
		if( !strcmp( proto, "udp" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "l7" ) )
	    {
		if( !strcmp( proto, "l7" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "start" ) )
		return start;
	    else if( !strcmp( type, "end" ) )
		return end;
	}
    }
    D( "check type and return" );
    if( !strcmp( type, "start" ) || !strcmp( type, "end" ) )
	return "0";
    else
	return "";
}

void ej_filter_dport_get( webs_t wp, int argc, char_t ** argv )
{
    int which;
    char *type;

    D( "ej filter dport get" );
    if( ejArgs( argc, argv, "%s %d", &type, &which ) < 2 )
    {
	websError( wp, 400, "Insufficient args\n" );
	D( "bad value" );
	return;
    }

    websWrite( wp, "%s", filter_dport_get( type, which ) );
    D( "good value" );

    return;

}

/*
 * Example: tcp:100-200 udp:210-220 both:250-260 
 */

char *filter_port_get( char *type, int which )
{
    static char word[256];
    char *wordlist, *next;
    char *start, *end, *proto;
    int temp;

    D( "filter port get" );
    wordlist = nvram_safe_get( "filter_port" );
    temp = which;

    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    start = word;
	    proto = strsep( &start, ":" );
	    end = start;
	    start = strsep( &end, "-" );
	    if( !strcmp( type, "disable" ) )
	    {
		if( !strcmp( proto, "disable" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "both" ) )
	    {
		if( !strcmp( proto, "both" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "tcp" ) )
	    {
		if( !strcmp( proto, "tcp" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "udp" ) )
	    {
		if( !strcmp( proto, "udp" ) )
		    return "selected";
		else
		    return " ";
	    }
	    else if( !strcmp( type, "start" ) )
		return start;
	    else if( !strcmp( type, "end" ) )
		return end;
	}
    }
    D( "return type" );
    if( !strcmp( type, "start" ) || !strcmp( type, "end" ) )
	return "0";
    else
	return "";
}

void ej_filter_port_get( webs_t wp, int argc, char_t ** argv )
{
    int which;
    char *type;

    D( "ej filter port get" );
    if( ejArgs( argc, argv, "%s %d", &type, &which ) < 2 )
    {
	websError( wp, 400, "Insufficient args\n" );
	D( "bad value" );
	return;
    }

    websWrite( wp, "%s", filter_port_get( type, which ) );

    D( "good value" );
    return;

}

/*
 * Example: 00:11:22:33:44:55 00:11:22:33:44:56 
 */

char *filter_mac_get( int which )
{
    static char word[256];
    char *wordlist, *next;
    char *mac;
    int temp;
    char filter_mac[] = "filter_mac_grpXXX";

    D( "filter mac get" );
    snprintf( filter_mac, sizeof( filter_mac ), "filter_mac_grp%s",
	      nvram_safe_get( "filter_id" ) );

    wordlist = nvram_safe_get( filter_mac );
    if( !wordlist )
	return "";

    temp = which;

    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    mac = word;
	    D( "return mac" );
	    return mac;
	}
    }
    D( "return zero mac" );
    return "00:00:00:00:00:00";
}

void ej_filter_ip_get( webs_t wp, int argc, char_t ** argv )
{
    int which;
    char *type;

    D( "ej-filter ip get" );
    if( ejArgs( argc, argv, "%s %d", &type, &which ) < 2 )
    {

	websError( wp, 400, "Insufficient args\n" );
	D( "BAD VALUE" );
	return;
    }

    websWrite( wp, "%s", filter_ip_get( type, which ) );

    D( "good value" );
    return;
}

void ej_filter_mac_get( webs_t wp, int argc, char_t ** argv )
{
    int which;

    D( "ej filter mac get" );
    if( ejArgs( argc, argv, "%d", &which ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
	D( "bad value" );
	return;
    }

    websWrite( wp, "%s", filter_mac_get( which ) );
    D( "good value" );
    return;
}

void ej_filter_policy_select( webs_t wp, int argc, char_t ** argv )
{
    int i;

    D( "ej policy select" );
    for( i = 1; i <= 10; i++ )
    {
	char filter[] = "filter_ruleXXX";
	char *data = "";
	char name[50] = "";

	snprintf( filter, sizeof( filter ), "filter_rule%d", i );
	data = nvram_safe_get( filter );

	if( data && strcmp( data, "" ) )
	{
	    find_match_pattern( name, sizeof( name ), data, "$NAME:", "" );	// get 
										// name 
										// value
	}
	websWrite( wp, "<option value=%d %s>%d ( %s ) </option>\n",
		   i,
		   ( atoi( nvram_safe_get( "filter_id" ) ) ==
		     i ? "selected=\"selected\" " : "" ), i, name );
    }
    D( "okay" );
    return;
}

void ej_filter_policy_get( webs_t wp, int argc, char_t ** argv )
{

    char *type, *part;

    D( "ej filter policy get" );
    if( ejArgs( argc, argv, "%s %s", &type, &part ) < 2 )
    {
	websError( wp, 400, "Insufficient args\n" );
	return;
    }

    if( !strcmp( type, "f_id" ) )
    {
	websWrite( wp, "%s", nvram_safe_get( "filter_id" ) );
    }
    else if( !strcmp( type, "f_name" ) )
    {
	char name[50] = "";
	char filter[] = "filter_ruleXXX";
	char *data = "";

	snprintf( filter, sizeof( filter ), "filter_rule%s",
		  nvram_safe_get( "filter_id" ) );
	data = nvram_safe_get( filter );
	if( strcmp( data, "" ) )
	{
	    find_match_pattern( name, sizeof( name ), data, "$NAME:", "" );	// get 
										// name 
										// value
	    websWrite( wp, "%s", name );
	}
    }
    else if( !strcmp( type, "f_status" ) )
    {
	char status[50] = "", deny[50] = "";
	char filter[] = "filter_ruleXXX";
	char *data = "";

	snprintf( filter, sizeof( filter ), "filter_rule%s",
		  nvram_safe_get( "filter_id" ) );
	data = nvram_safe_get( filter );
	if( strcmp( data, "" ) )
	{			// have data
	    find_match_pattern( status, sizeof( status ), data, "$STAT:", "1" );	// get 
											// status 
											// value
	    find_match_pattern( deny, sizeof( deny ), data, "$DENY:", "" );	// get 
										// deny 
										// value
	    if( !strcmp( deny, "" ) )
	    {			// old format
		if( !strcmp( status, "0" ) || !strcmp( status, "1" ) )
		    strcpy( deny, "1" );	// Deny
		else
		    strcpy( deny, "0" );	// Allow
	    }
#if 0
	    if( !strcmp( part, "disable" ) )
	    {
		if( !strcmp( status, "1" ) )
		    websWrite( wp, "checked=\"checked\" " );
	    }
	    else if( !strcmp( part, "enable" ) )
	    {
		if( !strcmp( status, "2" ) )
		    websWrite( wp, "checked=\"checked\" " );
	    }
#endif
	    if( !strcmp( part, "disable" ) )
	    {
		if( !strcmp( status, "0" ) )
		    websWrite( wp, "checked=\"checked\" " );
	    }
	    else if( !strcmp( part, "enable" ) )
	    {
		if( strcmp( status, "0" ) )
		    websWrite( wp, "checked=\"checked\" " );
	    }
	    else if( !strcmp( part, "deny" ) )
	    {
		if( !strcmp( deny, "1" ) )
		    websWrite( wp, "checked=\"checked\" " );
	    }
	    else if( !strcmp( part, "allow" ) )
	    {
		if( !strcmp( deny, "0" ) )
		    websWrite( wp, "checked=\"checked\" " );
	    }
	    else if( !strcmp( part, "onload_status" ) )
	    {
		if( !strcmp( deny, "1" ) )
		    websWrite( wp, "deny" );
		else
		    websWrite( wp, "allow" );

	    }
	    else if( !strcmp( part, "init" ) )
	    {
		if( !strcmp( status, "1" ) )
		    websWrite( wp, "disable" );
		else if( !strcmp( status, "2" ) )
		    websWrite( wp, "enable" );
		else
		    websWrite( wp, "disable" );
	    }
	}
	else
	{			// no data
	    if( !strcmp( part, "disable" ) )
		websWrite( wp, "checked=\"checked\" " );
	    else if( !strcmp( part, "allow" ) )	// default policy is allow,
						// 2003-10-21
		websWrite( wp, "checked=\"checked\" " );
	    else if( !strcmp( part, "onload_status" ) )	// default policy is
							// allow, 2003-10-21
		websWrite( wp, "allow" );
	    else if( !strcmp( part, "init" ) )
		websWrite( wp, "disable" );
	}
    }
    D( "okay" );
    return;
}

int filter_tod_init( int which )
{
    int ret;
    char *tod_data, *tod_buf_data;
    char filter_tod[] = "filter_todXXX";
    char filter_tod_buf[] = "filter_tod_bufXXX";
    char temp[3][20];

    tod_data_null = 0;
    day_all = week0 = week1 = week2 = week3 = week4 = week5 = week6 = 0;
    time_all = start_hour = start_min = start_time = end_hour = end_min =
	end_time = 0;
    start_week = end_week = 0;
    D( "filter tod init" );
    snprintf( filter_tod, sizeof( filter_tod ), "filter_tod%d", which );
    snprintf( filter_tod_buf, sizeof( filter_tod_buf ), "filter_tod_buf%d",
	      which );

    /*
     * Parse filter_tod{1...10} 
     */
    tod_data = nvram_safe_get( filter_tod );
    if( !tod_data )
	return -1;		// no data
    if( strcmp( tod_data, "" ) )
    {
	sscanf( tod_data, "%s %s %s", temp[0], temp[1], temp[2] );
	sscanf( temp[0], "%d:%d", &start_hour, &start_min );
	sscanf( temp[1], "%d:%d", &end_hour, &end_min );
	ret = sscanf( temp[2], "%d-%d", &start_week, &end_week );
	if( ret == 1 )
	    end_week = start_week;

	if( start_hour == 0 && start_min == 0 && end_hour == 23
	    && end_min == 59 )
	{			// 24 Hours
	    time_all = 1;
	    start_hour = end_hour = 0;
	    start_min = start_time = end_min = end_time = 0;
	}
	/*
	 * else { // check AM or PM time_all = 0; if (start_hour > 11) {
	 * start_hour = start_hour - 12; start_time = 1; } if (end_hour > 11)
	 * { end_hour = end_hour - 12; end_time = 1; } } 
	 */
    }
    else
    {				// default Everyday and 24 Hours
	tod_data_null = 1;
	day_all = 1;
	time_all = 1;
    }

    if( tod_data_null == 0 )
    {
	/*
	 * Parse filter_tod_buf{1...10} 
	 */
	tod_buf_data = nvram_safe_get( filter_tod_buf );
	if( !strcmp( tod_buf_data, "7" ) )
	{
	    day_all = 1;
	}
	else if( strcmp( tod_buf_data, "" ) )
	{
	    sscanf( tod_buf_data, "%d %d %d %d %d %d %d", &week0, &week1,
		    &week2, &week3, &week4, &week5, &week6 );
	    day_all = 0;
	}
    }
    D( "okay" );
    return 0;
}

void ej_filter_tod_get( webs_t wp, int argc, char_t ** argv )
{
    char *type;
    int i;

    D( "ej-filter-tod_get" );
    if( ejArgs( argc, argv, "%s", &type ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
	return;
    }

    filter_tod_init( atoi( nvram_safe_get( "filter_id" ) ) );

    if( !strcmp( type, "day_all_init" ) )
    {
	if( day_all == 0 )
	    websWrite( wp, "1" );
	else
	    websWrite( wp, "0" );
    }
    else if( !strcmp( type, "time_all_init" ) )
    {
	if( time_all == 0 )
	    websWrite( wp, "1" );
	else
	    websWrite( wp, "0" );
    }
    else if( !strcmp( type, "day_all" ) )
    {
	websWrite( wp, "%s", day_all == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "start_week" ) )
    {
	websWrite( wp, "%d", start_week );
    }
    else if( !strcmp( type, "end_week" ) )
    {
	websWrite( wp, "%d", end_week );
    }
    else if( !strcmp( type, "week0" ) )
    {				// Sun
	websWrite( wp, "%s", week0 == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "week1" ) )
    {				// Mon
	websWrite( wp, "%s", week1 == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "week2" ) )
    {				// Tue
	websWrite( wp, "%s", week2 == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "week3" ) )
    {				// Wed
	websWrite( wp, "%s", week3 == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "week4" ) )
    {				// Thu
	websWrite( wp, "%s", week4 == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "week5" ) )
    {				// Fri
	websWrite( wp, "%s", week5 == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "week6" ) )
    {				// Sat
	websWrite( wp, "%s", week6 == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "time_all_en" ) )
    {				// for linksys
	websWrite( wp, "%s", time_all == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "time_all_dis" ) )
    {				// for linksys
	websWrite( wp, "%s", time_all == 0 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "time_all" ) )
    {
	websWrite( wp, "%s", time_all == 1 ? "checked=\"checked\" " : "" );
    }
    else if( !strcmp( type, "start_hour_24" ) )
    {				// 00 -> 23
	for( i = 0; i < 24; i++ )
	{

	    websWrite( wp, "<option value=%d %s>%d</option>\n", i,
		       i == start_hour ? "selected=\"selected\" " : "", i );
	}
    }
    else if( !strcmp( type, "start_min_1" ) )
    {				// 0 1 2 3 4 .... -> 58 59
	for( i = 0; i < 60; i++ )
	{

	    websWrite( wp, "<option value=%02d %s>%02d</option>\n", i,
		       i == start_min ? "selected=\"selected\" " : "", i );
	}
    }
    else if( !strcmp( type, "end_hour_24" ) )
    {				// 00 ->23
	for( i = 0; i < 24; i++ )
	{

	    websWrite( wp, "<option value=%d %s>%d</option>\n", i,
		       i == end_hour ? "selected=\"selected\" " : "", i );
	}
    }
    else if( !strcmp( type, "end_min_1" ) )
    {				// 0 1 2 3 4 .... -> 58 59
	for( i = 0; i < 60; i++ )
	{

	    websWrite( wp, "<option value=%02d %s>%02d</option>\n", i,
		       i == end_min ? "selected=\"selected\" " : "", i );
	}
    }
    /*
     * else if (!strcmp (type, "start_hour_12")) { // 1 -> 12 for (i = 1; i
     * <= 12; i++) { int j; if (i == 12) j = 0; else j = i;
     * 
     * websWrite (wp, "<option value=%d %s>%d</option>\n", j, j == start_hour 
     * ? "selected=\"selected\" " : "", i); } } else if (!strcmp (type,
     * "start_min_5")) { // 0 5 10 15 20 25 30 35 40 45 50 55 for (i = 0; i < 
     * 12; i++) {
     * 
     * websWrite (wp, "<option value=%02d %s>%02d</option>\n", i * 5, i * 5
     * == start_min ? "selected=\"selected\" " : "", i * 5); } } else if
     * (!strcmp (type, "start_time_am")) { websWrite (wp, "%s", start_time == 
     * 1 ? "" : "selected=\"selected\" "); } else if (!strcmp (type,
     * "start_time_pm")) { websWrite (wp, "%s", start_time == 1 ?
     * "selected=\"selected\" " : ""); } else if (!strcmp (type,
     * "end_hour_12")) { // 1 -> 12 for (i = 1; i <= 12; i++) { int j; if (i
     * == 12) j = 0; else j = i;
     * 
     * websWrite (wp, "<option value=%d %s>%d</option>\n", j, j == end_hour ? 
     * "selected=\"selected\" " : "", i); } } else if (!strcmp (type,
     * "end_min_5")) { // 0 5 10 15 20 25 30 35 40 45 50 55 for (i = 0; i <
     * 12; i++) {
     * 
     * websWrite (wp, "<option value=%02d %s>%02d</option>\n", i * 5, i * 5
     * == end_min ? "selected=\"selected\" " : "", i * 5); } } else if
     * (!strcmp (type, "end_time_am")) { websWrite (wp, "%s", end_time == 1 ? 
     * "" : "selected=\"selected\" "); } else if (!strcmp (type,
     * "end_time_pm")) { websWrite (wp, "%s", end_time == 1 ?
     * "selected=\"selected\" " : ""); } 
     */
    D( "right" );
    return;
}

/*
 * Format: url0, url1, url2, url3, ....  keywd0, keywd1, keywd2, keywd3,
 * keywd4, keywd5, .... 
 */
void ej_filter_web_get( webs_t wp, int argc, char_t ** argv )
{
    char *type;
    int which;
    char *token = "<&nbsp;>";

    D( "filter-web-get" );
    if( ejArgs( argc, argv, "%s %d", &type, &which ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
	return;
    }

    if( !strcmp( type, "host" ) )
    {
	char *host_data, filter_host[] = "filter_web_hostXXX";;
	char host[80];

	snprintf( filter_host, sizeof( filter_host ), "filter_web_host%s",
		  nvram_safe_get( "filter_id" ) );
	host_data = nvram_safe_get( filter_host );
	if( !strcmp( host_data, "" ) )
	    return;		// no data
	find_each( host, sizeof( host ), host_data, token, which, "" );
	websWrite( wp, "%s", host );
    }
    else if( !strcmp( type, "url" ) )
    {
	char *url_data, filter_url[] = "filter_web_urlXXX";
	char url[80];

	snprintf( filter_url, sizeof( filter_url ), "filter_web_url%s",
		  nvram_safe_get( "filter_id" ) );
	url_data = nvram_safe_get( filter_url );
	if( !strcmp( url_data, "" ) )
	    return;		// no data
	find_each( url, sizeof( url ), url_data, token, which, "" );
	websWrite( wp, "%s", url );
    }
    D( "okay" );
    return;
}

void ej_filter_summary_show( webs_t wp, int argc, char_t ** argv )
{
    int i;

#if LANGUAGE == JAPANESE
    char w[7][10] = { "“ú", "ŒŽ", "‰Î", "?…", "–Ø", "‹à", "“y" };
    char am[] = "Œß‘O";
    char pm[] = "ŒßŒã";
    char _24h[] = "24 ŽžŠÔ";
#else
    char w[7][15] =
	{ "share.sun_s1", "share.mon_s1", "share.tue_s1", "share.wed_s1",
	"share.thu_s1", "share.fri_s1", "share.sat_s1"
    };
    // char am[] = "AM";
    // char pm[] = "PM";
    char _24h[] = "24 Hours.";
#endif
    D( "filter summary show" );
    for( i = 0; i < 10; i++ )
    {
	char name[50] = "---";
	char status[5] = "---";
	char filter[] = "filter_ruleXXX";
	char *data = "";
	char time_buf[50] = "---";

	snprintf( filter, sizeof( filter ), "filter_rule%d", i + 1 );
	data = nvram_safe_get( filter );
	if( data && strcmp( data, "" ) )
	{
	    find_match_pattern( name, sizeof( name ), data, "$NAME:", "&nbsp;" );	// get 
											// name 
											// value
	    find_match_pattern( status, sizeof( status ), data, "$STAT:", "---" );	// get 
											// name 
											// value
	}

	filter_tod_init( i + 1 );

	websWrite( wp, " \
		<tr align=\"center\" bgcolor=\"#CCCCCC\" >\n\
			<td width=\"50\" ><font face=\"Arial\" size=\"2\" >%d.</font></td>\n\
			<td width=\"200\" ><font face=\"Arial\" size=\"2\" >%s</font></td>\n\
			<td height=\"30\" width=\"150\" >\n\
			<table width=\"150\" height=\"30\" border=\"1\" cellspacing=\"1\" bordercolor=\"#000000\" bgcolor=\"#FFFFFF\" style=\"border-collapse:collapse\" >\n\
				<tr>\n", i + 1, name );
	websWrite( wp, " \
			<td align=\"center\" width=\"17\" bgcolor=\"%s\" style=\"border-style: solid\"><script type=\"text/javascript\">Capture(%s)</script></td>\n\
			<td align=\"center\" width=\"17\" bgcolor=\"%s\" style=\"border-style: solid\"><script type=\"text/javascript\">Capture(%s)</script></td>\n\
			<td align=\"center\" width=\"17\" bgcolor=\"%s\" style=\"border-style: solid\"><script type=\"text/javascript\">Capture(%s)</script></td>\n\
			<td align=\"center\" width=\"17\" bgcolor=\"%s\" style=\"border-style: solid\"><script type=\"text/javascript\">Capture(%s)</script></td>\n", tod_data_null == 0 && ( day_all == 1 || week0 == 1 ) ? "#C0C0C0" : "#FFFFFF", w[0], tod_data_null == 0 && ( day_all == 1 || week1 == 1 ) ? "#C0C0C0" : "#FFFFFF", w[1], tod_data_null == 0 && ( day_all == 1 || week2 == 1 ) ? "#C0C0C0" : "#FFFFFF", w[2], tod_data_null == 0 && ( day_all == 1 || week3 == 1 ) ? "#C0C0C0" : "#FFFFFF", w[3] );
	websWrite( wp, " \
    		<td align=\"center\" width=\"17\" bgcolor=\"%s\" style=\"border-style: solid\"><script type=\"text/javascript\">Capture(%s)</script></td>\n\
			<td align=\"center\" width=\"17\" bgcolor=\"%s\" style=\"border-style: solid\"><script type=\"text/javascript\">Capture(%s)</script></td>\n\
			<td align=\"center\" width=\"17\" bgcolor=\"%s\" style=\"border-style: solid\"><script type=\"text/javascript\">Capture(%s)</script></td>\n\
		</tr>\n\
		</table>\n\
		</td>\n", tod_data_null == 0 && ( day_all == 1 || week4 == 1 ) ? "#C0C0C0" : "#FFFFFF", w[4], tod_data_null == 0 && ( day_all == 1 || week5 == 1 ) ? "#C0C0C0" : "#FFFFFF", w[5], tod_data_null == 0 && ( day_all == 1 || week6 == 1 ) ? "#C0C0C0" : "#FFFFFF", w[6] );

	if( tod_data_null == 0 )
	{
	    if( time_all == 1 )
		strcpy( time_buf, _24h );
	    else
	    {
		snprintf( time_buf, sizeof( time_buf ),
			  "%02d:%02d - %02d:%02d",
			  start_hour, start_min, end_hour, end_min );
	    }
	}
	websWrite( wp, " \
        <td width=\"150\" ><font face=\"Arial\" size=\"2\" > %s </font></td>\n\
        <td width=\"70\" ><input type=\"checkbox\" name=\"sum%d\" value=\"1\" ></td>\n\
      </tr>\n", time_buf, i + 1 );
    }
    D( "okay" );
    return;

}

void ej_filter_init( webs_t wp, int argc, char_t ** argv )
{
    char *f_id = websGetVar( wp, "f_id", NULL );

    D( "ej_filter-init" );
    if( f_id )			// for first time enter this page and don't
				// press apply.
	nvram_set( "filter_id", f_id );
    else
	nvram_set( "filter_id", "1" );

    return;
}

void ej_filter_port_services_get( webs_t wp, int argc, char_t ** argv )
{
    char *type;
    int which;
    char word[1024], *next;
    char delim[] = "<&nbsp;>";
    int index = 0;

    D( "ej_filter_port_services get" );

    if( ejArgs( argc, argv, "%s %d", &type, &which ) < 2 )
    {
	websError( wp, 400, "Insufficient args\n" );
	return;
    }

    char services[8192];

    memset( services, 0, 8192 );

    // get_filter_services (services);

    if( !strcmp( type, "all_list" ) || !strcmp( type, "user_list" ) )
    {
	if( !strcmp( type, "all_list" ) )
	    get_filter_services( services );
	else			// user_list only
	{
	    strcat( services, nvram_safe_get( "filter_services" ) );	// this 
									// is 
									// user 
									// defined 
									// filters
	    strcat( services, nvram_safe_get( "filter_services_1" ) );	// this 
									// is 
									// user 
									// defined 
									// filters 
									// 
	}

	int count = 0;

	split( word, services, next, delim )
	{
	    int len = 0;
	    char *name, *prot, *port;
	    char protocol[100], ports[100];
	    int from = 0, to = 0;

	    // int proto;

	    if( ( name = strstr( word, "$NAME:" ) ) == NULL ||
		( prot = strstr( word, "$PROT:" ) ) == NULL ||
		( port = strstr( word, "$PORT:" ) ) == NULL )
		continue;

	    /*
	     * $NAME 
	     */
	    if( sscanf( name, "$NAME:%3d:", &len ) != 1 )
		continue;
	    strncpy( name, name + sizeof( "$NAME:nnn:" ) - 1, len );
	    name[len] = '\0';

	    /*
	     * $PROT 
	     */
	    if( sscanf( prot, "$PROT:%3d:", &len ) != 1 )
		continue;
	    strncpy( protocol, prot + sizeof( "$PROT:nnn:" ) - 1, len );
	    protocol[len] = '\0';

	    /*
	     * $PORT 
	     */
	    if( sscanf( port, "$PORT:%3d:", &len ) != 1 )
		continue;
	    strncpy( ports, port + sizeof( "$PORT:nnn:" ) - 1, len );
	    ports[len] = '\0';
	    if( sscanf( ports, "%d:%d", &from, &to ) != 2 )
		continue;

	    // cprintf("match:: name=%s, protocol=%s, ports=%s\n", 
	    // word, protocol, ports);

	    websWrite( wp,
		       "services[%d]=new service(%d, \"%s\", %d, %d, %d);\n",
		       count, count, name, from, to,
		       protocol_to_num( protocol ) );
	    count++;

	}

	websWrite( wp, "services_length = %d;\n", count );
    }
    else if( !strcmp( type, "service" ) )
    {
	char *port_data, filter_port[] = "filter_port_grpXXX";
	char name[80];

	snprintf( filter_port, sizeof( filter_port ), "filter_port_grp%s",
		  nvram_safe_get( "filter_id" ) );
	port_data = nvram_safe_get( filter_port );
	if( !strcmp( port_data, "" ) )
	    return;		// no data
	find_each( name, sizeof( name ), port_data, "<&nbsp;>", which, "" );
	websWrite( wp, "%s", name );

    }
    else if( !strcmp( type, "p2p" ) )
    {
	char *port_data, filter_port[] = "filter_p2p_grpXXX";

	snprintf( filter_port, sizeof( filter_port ), "filter_p2p_grp%s",
		  nvram_safe_get( "filter_id" ) );
	port_data = nvram_safe_get( filter_port );
	if( !strcmp( port_data, "" ) )
	    return;		// no data
	websWrite( wp, "%s", port_data );

    }
    D( "okay" );
    return;
}

void filtersummary_onload( webs_t wp, char *arg )
{
    D( "filter summary unload" );
    if( !strcmp( nvram_safe_get( "filter_summary" ), "1" ) )
    {
	websWrite( wp, arg );
    }
    D( "okay" );
}
