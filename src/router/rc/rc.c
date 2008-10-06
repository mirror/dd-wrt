
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <dirent.h>

#include <epivers.h>
#include <bcmnvram.h>
#include <mtd.h>
#include <shutils.h>
#include <rc.h>
#include <netconf.h>
#include <nvparse.h>
#include <bcmdevs.h>

#include <wlutils.h>
#include <utils.h>
#include <cyutils.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <typedefs.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <wlutils.h>
#include <cy_conf.h>

#include <revision.h>

/* 
 * Call when keepalive mode
 */
int redial_main( int argc, char **argv )
{
    int need_redial = 0;
    int status;
    pid_t pid;
    int count = 1;
    int num;

    while( 1 )
    {

	sleep( atoi( argv[1] ) );
	num = 0;
	count++;

	// fprintf(stderr, "check PPPoE %d\n", num);
	if( !check_wan_link( num ) )
	{
	    // fprintf(stderr, "PPPoE %d need to redial\n", num);
	    need_redial = 1;
	}
	else
	{
	    // fprintf(stderr, "PPPoE %d not need to redial\n", num);
	    continue;
	}

#if 0
	cprintf( "Check pppx if exist: " );
	if( ( fp = fopen( "/proc/net/dev", "r" ) ) == NULL )
	{
	    return -1;
	}

	while( fgets( line, sizeof( line ), fp ) != NULL )
	{
	    if( strstr( line, "ppp" ) )
	    {
		match = 1;
		break;
	    }
	}
	fclose( fp );
	cprintf( "%s", match == 1 ? "have exist\n" : "ready to dial\n" );
#endif

	if( need_redial )
	{
	    pid = fork(  );
	    switch ( pid )
	    {
		case -1:
		    perror( "fork failed" );
		    exit( 1 );
		case 0:
#ifdef HAVE_PPPOE
		    if( nvram_match( "wan_proto", "pppoe" ) )
		    {
			sleep( 1 );
			start_service( "wan_redial" );
		    }
#endif
#if defined(HAVE_PPTP) || defined(HAVE_L2TP) || defined(HAVE_HEARTBEAT)
		    else
#endif
#ifdef HAVE_PPTP
		    if( nvram_match( "wan_proto", "pptp" ) )
		    {
			stop_service( "pptp" );
			sleep( 1 );
			start_service( "wan_redial" );
		    }
#endif
#if defined(HAVE_L2TP) || defined(HAVE_HEARTBEAT)
		    else
#endif
#ifdef HAVE_L2TP
		    if( nvram_match( "wan_proto", "l2tp" ) )
		    {
			stop_service( "l2tp" );
			sleep( 1 );
			start_service( "l2tp_redial" );
		    }
#endif
		    // Moded by Boris Bakchiev
		    // We dont need this at all.
		    // But if this code is executed by any of pppX programs
		    // we might have to do this.

#ifdef HAVE_HEARTBEAT
		    else if( nvram_match( "wan_proto", "heartbeat" ) )
		    {
			if( is_running( "bpalogin" ) == 0 )
			{
			    stop_service( "heartbeat" );
			    sleep( 1 );
			    start_service( "heartbeat_redial" );
			}

		    }
#endif

		    exit( 0 );
		    break;
		default:
		    waitpid( pid, &status, 0 );
		    // dprintf("parent\n");
		    break;
	    }			// end switch
	}			// end if
    }				// end while
}				// end main

int get_wanface( int argc, char **argv )
{
    fprintf( stdout, "%s", get_wan_face(  ) );
    return 0;
}

struct MAIN
{
    char *callname;
    char *execname;
    int ( *exec ) ( int argc, char **argv );
};

static struct MAIN maincalls[] = {
    // {"init", NULL, &main_loop},
    {"ip-up", "ipup", NULL},
    {"ip-down", "ipdown", NULL},
    {"ipdown", "disconnected_pppoe", NULL},
    {"udhcpc", "udhcpc", NULL},
    {"mtd", NULL, mtd_main},
#ifdef HAVE_PPTPD
    {"poptop", NULL, &pptpd_main},
#endif
    {"redial", NULL, &redial_main},
#ifndef HAVE_RB500
    // {"resetbutton", NULL, &resetbutton_main},
#endif
    // {"wland", NULL, &wland_main},
    {"hb_connect", "hb_connect", NULL},
    {"hb_disconnect", "hb_disconnect", NULL},
    {"gpio", "gpio", NULL},
    {"beep", "beep", NULL},
    // {"listen", NULL, &listen_main},
    // {"check_ps", NULL, &check_ps_main},
    {"ddns_success", "ddns_success", NULL},
    // {"process_monitor", NULL, &process_monitor_main},
    // {"radio_timer", NULL, &radio_timer_main},
    // {"ttraf", NULL, &ttraff_main},
#ifdef HAVE_WIVIZ
    {"run_wiviz", NULL, &run_wiviz_main},
    {"autokill_wiviz", NULL, &autokill_wiviz_main},
#endif
    {"site_survey", "site_survey", NULL},
#ifdef HAVE_WOL
    {"wol", NULL, &wol_main},
#endif
    // {"event", NULL, &event_main},
    {"switch", "switch", NULL},
#ifdef HAVE_MICRO
    {"brctl", "brctl", NULL},
#endif
    {"getbridgeprio", "getbridgeprio", NULL},
    {"setuserpasswd", "setuserpasswd", NULL},
    {"getbridge", "getbridge", NULL},
    {"stopservices", NULL, stop_services_main},
    {"startservices", NULL, start_services_main},
    {"start_single_service", NULL, start_single_service_main},
    {"startstop", NULL, startstop_main},
#if !defined(HAVE_MICRO) || defined(HAVE_ADM5120)
    {"watchdog", NULL, &watchdog_main},
#endif
    // {"nvram", NULL, &nvram_main},
#ifdef HAVE_ROAMING
    {"roaming_daemon", NULL, &roaming_daemon_main},
    {"supplicant", "supplicant", NULL},
#endif
    {"get_wanface", NULL, &get_wanface},
#ifndef HAVE_XSCALE
    // {"ledtool", NULL, &ledtool_main},
#endif
#ifdef HAVE_REGISTER
    {"regshell", NULL, &reg_main}
#endif
};

int main( int argc, char **argv )
{
    char *base = strrchr( argv[0], '/' );
    base = base ? base + 1 : argv[0];
    int i;
    for( i = 0; i < sizeof( maincalls ) / sizeof( struct MAIN ); i++ )
    {
	if( strstr( base, maincalls[i].callname ) )
	{
	    if( maincalls[i].execname )
		return start_main( maincalls[i].execname, argc, argv );
	    if( maincalls[i].exec )
		return maincalls[i].exec( argc, argv );
	}
    }

    if( strstr( base, "startservice" ) )
    {
	if( argc < 2 )
	{
	    puts( "try to be professional\n" );
	    return 0;
	}
	return start_service( argv[1] );
    }
    if( strstr( base, "stopservice" ) )
    {
	if( argc < 2 )
	{
	    puts( "try to be professional\n" );
	    return 0;
	}
	return stop_service( argv[1] );
    }

#ifndef HAVE_RB500
#ifndef HAVE_X86
    /* 
     * erase [device] 
     */
    if( strstr( base, "erase" ) )
    {
	int brand = getRouterBrand(  );

	if( brand == ROUTER_MOTOROLA || brand == ROUTER_MOTOROLA_V1 || brand == ROUTER_MOTOROLA_WE800G || brand == ROUTER_RT210W || brand == ROUTER_BUFFALO_WZRRSG54 )	// these 
	    // 
	    // 
	    // 
	    // 
	    // 
	    // 
	    // 
	    // routers 
	    // have 
	    // problem 
	    // erasing 
	    // nvram, 
	    // so 
	    // we 
	    // only 
	    // software 
	    // restore 
	    // defaults
	{
	    if( argv[1] && strcmp( argv[1], "nvram" ) )
	    {
		fprintf( stderr,
			 "Sorry, erasing nvram will get this router unuseable\n" );
		return 0;
	    }
	}
	else
	{
	    if( argv[1] )
		return mtd_erase( argv[1] );
	    else
	    {
		fprintf( stderr, "usage: erase [device]\n" );
		return EINVAL;
	    }
	}
	return 0;
    }

    /* 
     * write [path] [device] 
     */
    if( strstr( base, "write" ) )
    {
	if( argc >= 3 )
	    return mtd_write( argv[1], argv[2] );
	else
	{
	    fprintf( stderr, "usage: write [path] [device]\n" );
	    return EINVAL;
	}
    }
#else
    if( strstr( base, "erase" ) )
    {
	if( argv[1] && strcmp( argv[1], "nvram" ) )
	{
	    fprintf( stderr, "Erasing configuration data...\n" );
	    eval( "mount", "/usr/local", "-o", "remount,rw" );
	    eval( "rm", "-f", "/tmp/nvram/*" );	// delete nvram database
	    eval( "rm", "-f", "/tmp/nvram/.lock" );	// delete nvram
	    // database
	    eval( "rm", "-f", "/etc/nvram/*" );	// delete nvram database
	    eval( "mount", "/usr/local", "-o", "remount,ro" );
	}
	return 0;
    }
#endif
#endif
    /* 
     * hotplug [event] 
     */
    if( strstr( base, "hotplug" ) )
    {
	if( argc >= 2 )
	{
	    // fprintf (stderr, "hotplug %s\n", argv[1]);
	    if( !strcmp( argv[1], "net" ) )
		return start_service( "hotplug_net" );
#ifdef HAVE_USBHOTPLUG
	    if( !strcmp( argv[1], "usb" ) )
		return start_service( "hotplug_usb" );
#endif
#ifdef HAVE_XSCALE
	    if( !strcmp( argv[1], "firmware" ) )
		return eval( "/etc/upload", argv[1] );
#endif
	}
	else
	{
	    fprintf( stderr, "usage: hotplug [event]\n" );
	    return EINVAL;
	}
	return 0;
    }
    // ////////////////////////////////////////////////////
    // 
    if( strstr( base, "filtersync" ) )
	return start_service( "filtersync" );
    /* 
     * filter [add|del] number 
     */
    if( strstr( base, "filter" ) )
    {
	if( argv[1] && argv[2] )
	{
	    int num = 0;

	    if( ( num = atoi( argv[2] ) ) > 0 )
	    {
		if( strcmp( argv[1], "add" ) == 0 )
		    return start_servicei( "filter_add", num );
		else if( strcmp( argv[1], "del" ) == 0 )
		    return start_servicei( "filter_del", num );
	    }
	}
	else
	{
	    fprintf( stderr, "usage: filter [add|del] number\n" );
	    return EINVAL;
	}
	return 0;
    }

    if( strstr( base, "restart_dns" ) )
    {
	stop_service( "dnsmasq" );
	stop_service( "udhcpd" );
	start_service( "udhcpd" );
	start_service( "dnsmasq" );
	return 0;
    }
    if( strstr( base, "setpasswd" ) )
    {
	start_service( "mkfiles" );
	return 0;
    }
    /* 
     * rc [stop|start|restart ] 
     */
    else if( strstr( base, "rc" ) )
    {
	if( argv[1] )
	{
	    if( strncmp( argv[1], "start", 5 ) == 0 )
		return kill( 1, SIGUSR2 );
	    else if( strncmp( argv[1], "stop", 4 ) == 0 )
		return kill( 1, SIGINT );
	    else if( strncmp( argv[1], "restart", 7 ) == 0 )
		return kill( 1, SIGHUP );
	}
	else
	{
	    fprintf( stderr, "usage: rc [start|stop|restart]\n" );
	    return EINVAL;
	}
    }
    return 1;
}
