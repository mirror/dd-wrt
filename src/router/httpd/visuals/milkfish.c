#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>

void ej_exec_milkfish_service( webs_t wp, int argc, char_t ** argv )
{

    FILE *fp;
    char line[254];
    char *request;

#ifdef FASTWEB
    ejArgs( argc, argv, "%s", &request );
#else
    if( ejArgs( argc, argv, "%s", &request ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
    }
#endif

    if( ( fp = popen( request, "r" ) ) )
    {
	while( fgets( line, sizeof( line ), fp ) != NULL )
	{
	    websWrite( wp, line );
	    websWrite( wp, "<br>" );
	}
	pclose( fp );
    }

    return;
}

void ej_exec_milkfish_phonebook( webs_t wp, int argc, char_t ** argv )
{

    FILE *fp;
    char line[254];
    char *request;

#ifdef FASTWEB
    ejArgs( argc, argv, "%s", &request );
#else
    if( ejArgs( argc, argv, "%s", &request ) < 1 )
    {
	websError( wp, 400, "Insufficient args\n" );
    }
#endif

    if( ( fp = popen( request, "r" ) ) )
    {
	while( fgets( line, sizeof( line ), fp ) != NULL )
	{
	    websWrite( wp, line );
	}
	pclose( fp );
    }

    return;
}

void show_subscriber_table( webs_t wp, char *type, int which )
{

    static char word[256];
    char *next, *wordlist;
    char *user, *pass;
    static char new_user[200], new_pass[200];
    int temp;

    wordlist = nvram_safe_get( "milkfish_ddsubscribers" );
    temp = which;

    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    pass = word;
	    user = strsep( &pass, ":" );
	    if( !user || !pass )
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
	    return;
	}
    }
}

void ej_exec_show_subscribers( webs_t wp, int argc, char_t ** argv )
{
    int i;
    char *count;
    int c = 0;

    count = nvram_safe_get( "milkfish_ddsubscribersnum" );
    if( count == NULL || strlen( count ) == 0 || ( c = atoi( count ) ) <= 0 )
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
	show_subscriber_table( wp, "user", i );
	websWrite( wp, "\" /></td>\n" );
	websWrite( wp, "<td>\n" );
	websWrite( wp,
		   "<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		   i );
	show_subscriber_table( wp, "pass", i );
	websWrite( wp, "\" /></td>\n" );
	websWrite( wp, "</tr>\n" );
    }
    return;
}

void show_aliases_table( webs_t wp, char *type, int which )
{

    static char word[256];
    char *next, *wordlist;
    char *user, *pass;
    static char new_user[200], new_pass[200];
    int temp;

    wordlist = nvram_safe_get( "milkfish_ddaliases" );
    temp = which;

    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    pass = word;
	    user = strsep( &pass, ":" );
	    if( !user || !pass )
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
	    return;
	}
    }
}

void ej_exec_show_aliases( webs_t wp, int argc, char_t ** argv )
{
    int i;
    char *count;
    int c = 0;

    count = nvram_safe_get( "milkfish_ddaliasesnum" );
    if( count == NULL || strlen( count ) == 0 || ( c = atoi( count ) ) <= 0 )
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
	show_aliases_table( wp, "user", i );
	websWrite( wp, "\" /></td>\n" );
	websWrite( wp, "<td>\n" );
	websWrite( wp,
		   "<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		   i );
	show_aliases_table( wp, "pass", i );
	websWrite( wp, "\" /></td>\n" );
	websWrite( wp, "</tr>\n" );
    }
    return;
}

void show_registrations_table( webs_t wp, char *type, int which )
{

    static char word[256];
    char *next, *wordlist;
    char *user, *contact, *agent;
    static char new_user[200], new_contact[200], new_agent[200];
    int temp;

    wordlist = nvram_safe_get( "milkfish_ddactive" );
    temp = which;

    foreach( word, wordlist, next )
    {
	if( which-- == 0 )
	{
	    contact = word;
	    user = strsep( &contact, ":" );
	    if( !user || !contact )
		continue;

	    agent = contact;
	    contact = strsep( &agent, ":" );
	    if( !contact || !agent )
		continue;

	    if( !strcmp( type, "user" ) )
	    {
		httpd_filter_name( user, new_user, sizeof( new_user ), GET );
		websWrite( wp, "%s", new_user );
	    }
	    else if( !strcmp( type, "contact" ) )
	    {
		httpd_filter_name( contact, new_contact,
				   sizeof( new_contact ), GET );
		websWrite( wp, "%s", new_contact );
	    }
	    else if( !strcmp( type, "agent" ) )
	    {
		httpd_filter_name( agent, new_agent, sizeof( new_agent ),
				   GET );
		websWrite( wp, "%s", new_agent );
	    }

	    return;
	}
    }
}

void ej_exec_show_registrations( webs_t wp, int argc, char_t ** argv )
{
    int i;
    char *count;
    int c = 0;

    count = nvram_safe_get( "milkfish_ddactivenum" );
    if( count == NULL || strlen( count ) == 0 || ( c = atoi( count ) ) <= 0 )
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
		   "<input maxlength=\"20\" size=\"20\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		   i );
	show_registrations_table( wp, "user", i );
	websWrite( wp, "\" readonly=\"readonly\" /></td>\n" );
	websWrite( wp, "<td>\n" );
	websWrite( wp,
		   "<input maxlength=\"50\" size=\"50\" name=\"contact%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		   i );
	show_registrations_table( wp, "contact", i );
	websWrite( wp, "\" readonly=\"readonly\" /></td>\n" );
	websWrite( wp, "<td>\n" );
	websWrite( wp,
		   "<input maxlength=\"50\" size=\"50\" name=\"agent%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		   i );
	show_registrations_table( wp, "agent", i );
	websWrite( wp, "\" readonly=\"readonly\" /></td>\n" );
	websWrite( wp, "</tr>\n" );
    }
    return;
}
