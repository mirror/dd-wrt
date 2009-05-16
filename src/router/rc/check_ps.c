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
#include <dlfcn.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>

struct mon
{
    char *name;			// Process name
    int count;			// Process count, 0 means don't check
    int type;			// LAN or WAN
    // int (*stop) (void); // stop function
    // int (*start) (void); // start function
    char *nvvalue;
    char *nvmatch;
};

enum
{ M_LAN, M_WAN };

struct mon mons[] = {
    // {"tftpd", 1, M_LAN, stop_tftpd, start_tftpd},
    {"upnp", 1, M_LAN, "upnp_enable", "1"},
    {"process_monitor", 1, M_LAN},
    {"httpd", 2, M_LAN},
    {"udhcpd", 1, M_LAN},
    {"dnsmasq", 1, M_LAN, "dnsmasq_enable", "1"},
    {"dhcpfwd", 1, M_LAN, "dhcpfwd_enable", "1"},
#ifdef HAVE_NOCAT
    {"splashd", 1, M_LAN, "NC_enable", "1"},
#endif
#ifdef HAVE_CHILLI
    {"chilli", 1, M_LAN, "chilli_enable", "1"},
#endif
#ifdef HAVE_WIFIDOG
    {"wifidog", 1, M_WAN, "wd_enable", "1"},
#endif
#ifdef HAVE_OLSRD
    {"olsrd", 1, M_LAN, "wk_mode", "olsrd"},
#endif
#ifdef HAVE_SPUTNIK_APD
    {"sputnik", 1, M_WAN, "apd_enable", "1"},
#endif
    {NULL, 0, 0}
};

int search_process( char *name, int count )
{
    int c = 0;

    c = count_processes( name );
    if( !c )
    {
	printf( "Can't find %s\n", name );
	return 0;
    }
    else
    {
	printf( "Find %s which count is %d\n", name, c );
	// if(count && c != count){
	// cprintf("%s count is not match\n", name);
	// return 0;
	// }
	// else
	return 1;
    }
}

void checknas( void )		// for broadcom v24 only
{
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)

    char buf[32];
    FILE *fnas = fopen( "/tmp/.nas", "r" );

    if( fnas == NULL )
	return;

    fgets( buf, sizeof( buf ), fnas );
    fclose( fnas );

    if( strlen( buf ) != count_processes( "nas" ) )	// restart all nas
	// processes
    {
	eval( "stopservice", "nas" );
	eval( "startservice_f", "nas" );
    }

    return;

#endif
}

/* 
 * software wlan led control 
 */
void softcontrol_wlan_led( void )	// done in watchdog.c for non-micro
					// builds.
{
#if defined(HAVE_MICRO) && !defined(HAVE_ADM5120)

    int brand;
    int radiostate = -1;
    int oldstate = -1;

#ifdef HAVE_MADWIFI
    radiostate = get_radiostate( "ath0" );
#else
    wl_ioctl( get_wdev(  ), WLC_GET_RADIO, &radiostate, sizeof( int ) );
#endif

    if( radiostate != oldstate )
    {
#ifdef HAVE_MADWIFI
	if( radiostate == 1 )
#else
	if( radiostate == 0 )
#endif
	    led_control( LED_WLAN, LED_ON );
	else
	{
	    led_control( LED_WLAN, LED_OFF );
#ifndef HAVE_MADWIFI
	    brand = getRouterBrand(  );
	    /* 
	     * Disable wireless will cause diag led blink, so we want to stop 
	     * it. 
	     */
	    if( brand == ROUTER_WRT54G )
		diag_led( DIAG, STOP_LED );
	    /* 
	     * Disable wireless will cause power led off, so we want to turn
	     * it on. 
	     */
	    if( brand == ROUTER_WRT54G_V8 )
		led_control( LED_POWER, LED_ON );
#endif
	}

	oldstate = radiostate;
    }
    return;

#endif
}

/* 
 * end software wlan led control 
 */

void checkupgrade( void )
{
#ifndef HAVE_X86
    FILE *in = fopen( "/tmp/firmware.bin", "rb" );

    if( in != NULL )
    {
	fclose( in );
	eval( "rm", "/tmp/cron.d/check_ps" );	// deleting cron file to
	// prevent double call of
	// this
	fprintf( stderr,
		 "found firmware upgrade, flashing now, but we will wait for another 30 seconds\n" );
	sleep( 30 );
#if defined(HAVE_WHRAG108) || defined(HAVE_TW6600) || defined(HAVE_LS5)
	eval( "write", "/tmp/firmware.bin", "rootfs" );
#else
	eval( "write", "/tmp/firmware.bin", "linux" );
#endif
	fprintf( stderr, "done. rebooting now\n" );
	eval( "killall", "-3", "init" );
    }
#endif
}
int do_mon( void )
{
    struct mon *v;

    checkupgrade(  );
    checknas(  );
#ifndef HAVE_RT2880
    softcontrol_wlan_led(  );
#endif
    for( v = mons; v < &mons[sizeof( mons ) / sizeof( struct mon )]; v++ )
    {
	if( v->name == NULL )
	    break;
	if( v->nvvalue && v->nvmatch )
	{
	    if( !nvram_match( v->nvvalue, v->nvmatch ) )
		continue;	// service not enabled. no need to check
	}
	printf( "checking %s\n", v->name );

	if( v->type == M_WAN )
	    if( !check_wan_link( 0 ) )
	    {
		printf( "process is wan, but wan is not up\n" );
		continue;
	    }
	if( !search_process( v->name, v->count ) )
	{

	    printf( "Maybe %s had died, we need to re-exec it\n", v->name );
	    eval( "stopservice", v->name );
	    killall( v->name, SIGKILL );
	    eval( "startservice_f", v->name );
	}
	printf( "checking for %s done\n", v->name );
    }

    return 1;
}

int main( int argc, char **argv )
{
    pid_t pid;

    if( check_action(  ) != ACT_IDLE )
    {				// Don't execute during upgrading
	printf( "check_ps: nothing to do...\n" );
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
