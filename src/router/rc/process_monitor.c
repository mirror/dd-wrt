
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <shutils.h>
#include <code_pattern.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <rc.h>
#include <cy_conf.h>
#include <utils.h>

extern void ntp_main( timer_t t, int arg );
extern void do_redial( timer_t t, int arg );
extern int do_ntp( void );
extern void check_udhcpd( timer_t t, int arg );
extern void init_event_queue( int n );
extern int timer_connect( timer_t timerid, void ( *routine ) ( timer_t, int ),
			  int arg );

#define NTP_M_TIMER 3600
#define NTP_N_TIMER 30

int main( int argc, char **argv )
{
    struct itimerspec t1, t4, t5;
    timer_t ntp1_id, ntp2_id, udhcpd_id;
    int time;
    long leasetime = 0;

    init_event_queue( 40 );

    openlog( "process_monitor", LOG_PID | LOG_NDELAY, LOG_DAEMON );

    if( nvram_invmatch( "dhcp_dnsmasq", "1" ) )
    {
	leasetime = atol( nvram_safe_get( "dhcp_lease" ) ) * 60;

	if( leasetime <= 0 )
	    leasetime = 86400;

	memset( &t1, 0, sizeof( t1 ) );
	t1.it_interval.tv_sec = ( int )leasetime;
	t1.it_value.tv_sec = ( int )leasetime;
	timer_create( CLOCK_REALTIME, NULL, ( timer_t * ) & udhcpd_id );
	timer_connect( udhcpd_id, check_udhcpd, FIRST );
	timer_settime( udhcpd_id, 0, &t1, NULL );
    }

    if( nvram_invmatch( "ntp_enable", "0" ) )
    {				// && check_wan_link(0) ) {

	/* 
	 * init ntp timer 
	 */
#if defined(HAVE_SNMP) || defined(HAVE_WIFIDOG)
	struct timeval now;

	gettimeofday( &now, NULL );
#endif
	if( do_ntp(  ) != 0 )
	{
	    dd_syslog( LOG_ERR,
		    "Last update failed, we need to re-update after %d seconds\n",
		    NTP_N_TIMER );
	    time = NTP_N_TIMER;

	    memset( &t4, 0, sizeof( t4 ) );
	    t4.it_interval.tv_sec = time;
	    t4.it_value.tv_sec = time;

	    timer_create( CLOCK_REALTIME, NULL, ( timer_t * ) & ntp1_id );
	    timer_connect( ntp1_id, ntp_main, FIRST );
	    timer_settime( ntp1_id, 0, &t4, NULL );
	}

	struct timeval then;

	gettimeofday( &then, NULL );

#ifdef HAVE_SNMP
	if( ( abs( now.tv_sec - then.tv_sec ) > 100000000 )
	    && nvram_match( "snmpd_enable", "1" ) )
	{
	    eval( "stopservice", "snmp" );
	    sleep( 1 );
	    dd_syslog( LOG_DEBUG, "Restarting snmpd\n" );
	    eval( "startservice", "snmp" );

	}
#endif
#ifdef HAVE_WIFIDOG		// dirty fix for wifidog
	if( ( abs( now.tv_sec - then.tv_sec ) > 100000000 )
	    && nvram_match( "wd_enable", "1" ) )
	{
	    eval( "stopservice", "wifidog" );
	    sleep( 1 );
	    dd_syslog( LOG_DEBUG, "Restarting Wifidog daemon\n" );
	    eval( "startservice", "wifidog" );
	}
#endif
	// give user a chance to use resetbutton for first 4 min even if
	// disabled and time is not synched
	if( then.tv_sec > 240 && nvram_match( "resetbutton_enable", "0" )
	    && pidof( "resetbutton" ) > 0 )
	{
	    eval( "stopservice", "resetbutton" );
	}

	dd_syslog( LOG_DEBUG, "We need to re-update after %d seconds\n",
		NTP_M_TIMER );

	time = NTP_M_TIMER;

	memset( &t5, 0, sizeof( t5 ) );
	t5.it_interval.tv_sec = time;
	t5.it_value.tv_sec = time;

	timer_create( CLOCK_REALTIME, NULL, ( timer_t * ) & ntp2_id );
	timer_connect( ntp2_id, ntp_main, SECOND );
	timer_settime( ntp2_id, 0, &t5, NULL );

    }
    printf( "process_monitor..done\n" );
    while( 1 )
    {
	sleep( 3600 );
    }

    closelog(  );

    return 1;
}
