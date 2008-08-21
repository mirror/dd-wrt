#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>

struct mon
{
    char *name;			// Process name
    int count;			// Process conut, 0 means don't check
    int type;			// LAN or WAN
    int ( *stop ) ( void );	// stop function
    int ( *start ) ( void );	// start function
    int ( *condition ) ( void );	// restart condition
};

enum
{ M_LAN, M_WAN };

struct mon mons[] = {
    {"tftpd", 1, M_LAN, stop_tftpd, start_tftpd, NULL},
    {"httpd", 2, M_LAN, stop_httpd, start_httpd, NULL},
    {"splashd", 1, M_LAN, stop_splashd, start_splashd, NULL},
    {"udhcpd", 1, M_LAN, stop_dhcpd, start_dhcpd, check_dhcpd},
    // { "dnsmasq", 1, M_LAN, stop_dns, start_dns,NULL},
};

int search_process( char *name, int count )
{
    int *pidList = NULL;
    int c = 0;

    pidList = find_all_pid_by_ps( name );
    if( pidList && *pidList > 0 )
    {
	for( ; pidList && *pidList != 0; pidList++ )
	{
	    cprintf( "Find %s which pid is %d\n", name, *pidList );
	    c++;
	}
    }

    if( !c )
    {
	cprintf( "Cann't find %s\n", name );
	return 0;
    }
    else
    {
	cprintf( "Find %s which count is %d\n", name, c );
	if( count && c != count )
	{
	    cprintf( "%s count is not match\n", name );
	    return 0;
	}
	else
	    return 1;
    }
}

int do_mon( void )
{
    struct mon *v;

    for( v = mons; v < &mons[sizeof( mons ) / sizeof( mons[0] )]; v++ )
    {
	if( v->condition )
	{
	    if( v->condition(  ) )
	    {
		if( v->stop )
		    v->stop(  );
		sleep( 1 );
		if( v->start );
		v->start(  );
	    }
	}
	else
	{
	    if( !search_process( v->name, v->count ) )
	    {
		if( v->type == M_WAN )
		    if( !check_wan_link( 0 ) )
			continue;

		cprintf( "Maybe %s had died, we need to re-exec it\n",
			 v->name );
		if( v->stop )
		    v->stop(  );
		sleep( 1 );
		if( v->start )
		    v->start(  );
	    }
	}
    }

    return 1;
}

int monitor_ps_main( int argc, char **argv )
{
    pid_t pid;

    if( check_action(  ) != ACT_IDLE )
    {				// Don't execute during upgrading
	cprintf( "nothing to do...\n" );
	sleep( 120 );
	return 1;
    }

    pid = fork(  );
    switch ( pid )
    {
	case -1:
	    perror( "fork failed" );
	    exit( 1 );
	    break;
	case 0:
	    do_mon(  );
	    exit( 0 );
	    break;
	default:
	    _exit( 0 );
	    break;
    }
}
