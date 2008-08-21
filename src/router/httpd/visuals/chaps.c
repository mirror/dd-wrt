#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <broadcom.h>
#include <cy_conf.h>

/*
 * Example: name:pass:ip:on 
 */

static void show_chaps_table( webs_t wp, char *type, int which )
{

    static char word[256];
    char *next, *wordlist;
    char *user, *pass, *ip, *enable;
    static char new_user[200], new_pass[200];
    int temp;

    wordlist = nvram_safe_get( "pppoeserver_chaps" );
    temp = which;

    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    pass = word;
	    user = strsep( &pass, ":" );
	    if( !user || !pass )
		continue;

	    ip = pass;
	    pass = strsep( &ip, ":" );
	    if( !pass || !ip )
		continue;

	    enable = ip;
	    ip = strsep( &enable, ":" );
	    if( !ip || !enable )
		continue;

	    if( !strcmp( type, "user" ) )
	    {
		httpd_filter_name( user, new_user, sizeof( new_user ), GET );
		websWrite( wp, "%s", new_user );
	    }
	    else if( !strcmp( type, "pass" ) )
	    {
		httpd_filter_name( pass, new_pass, sizeof( new_pass ), GET );
		websWrite( wp, "%s", new_pass );
	    }
	    else if( !strcmp( type, "ip" ) )
		websWrite( wp, "%s", ip );
	    else if( !strcmp( type, "enable" ) )
	    {
		if( !strcmp( enable, "on" ) )
		    websWrite( wp, "checked=\"checked\"" );
		else
		    websWrite( wp, "" );
	    }
	    return;
	}
    }
    if( !strcmp( type, "ip" ) )
	websWrite( wp, "0.0.0.0" );
    else
	websWrite( wp, "" );

}

void ej_show_chaps( webs_t wp, int argc, char_t ** argv )
{
    int i;
    char *count;
    int c = 0;

    count = nvram_safe_get( "pppoeserver_chapsnum" );
    if( count == NULL || strlen( count ) == 0 )
    {
	websWrite( wp, "<tr>\n" );
	websWrite( wp,
		   "<td colspan=\"4\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n" );
	websWrite( wp, "</tr>\n" );
    }
    c = atoi( count );
    if( c <= 0 )
    {
	websWrite( wp, "<tr>\n" );
	websWrite( wp,
		   "<td colspan=\"4\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n" );
	websWrite( wp, "</tr>\n" );
    }
    for( i = 0; i < c; i++ )
    {
	websWrite( wp, "<tr><td>\n" );
	websWrite( wp,
		   "<input maxlength=\"30\" size=\"30\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		   i );
	show_chaps_table( wp, "user", i );
	websWrite( wp, "\" /></td>\n" );
	websWrite( wp, "<td>\n" );
	websWrite( wp,
		   "<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		   i );
	show_chaps_table( wp, "pass", i );
	websWrite( wp, "\" /></td>\n" );
	websWrite( wp, "<td>\n" );
	websWrite( wp,
		   "<input class=\"num\" maxlength=\"15\" size=\"26\" name=\"ip%d\" value=\"",
		   i );
	show_chaps_table( wp, "ip", i );
	websWrite( wp, "\" /></td>\n" );
	websWrite( wp, "<td>\n" );
	websWrite( wp,
		   "<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ",
		   i );
	show_chaps_table( wp, "enable", i );
	websWrite( wp, " /></td>\n" );
	websWrite( wp, "</tr>\n" );
    }
    return;
}
