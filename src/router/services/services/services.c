/*
 * services.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>		/* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>		/* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 70

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

/*
 * AhMan March 18 2005 
 */
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

void start_force_to_dial( void );

static int alreadyInHost( char *host )
{
    FILE *in = fopen( "/tmp/hosts", "rb" );

    if( in == NULL )
	return 0;
    char buf[100];

    while( 1 )
    {
	fscanf( in, "%s", buf );
	if( !strcmp( buf, host ) )
	{
	    fclose( in );
	    return 1;
	}
	if( feof( in ) )
	{
	    fclose( in );
	    return 0;
	}
    }
}

void addHost( char *host, char *ip )
{
    char buf[100];
    char newhost[100];

    if( host == NULL )
	return;
    if( ip == NULL )
	return;
    strcpy( newhost, host );
    char *domain = nvram_safe_get( "lan_domain" );

    if( domain != NULL && strlen( domain ) > 0
	&& strcmp( host, "localhost" ) )
    {
	sprintf( newhost, "%s.%s", host, domain );
    }
    else
	sprintf( newhost, "%s", host );

    if( alreadyInHost( newhost ) )
	return;
    sysprintf( "echo \"%s\t%s\">>/tmp/hosts", ip, newhost );
}

void start_vpn_modules( void )
{
#if defined(HAVE_XSCALE) || defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_X86) ||defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5) || defined(HAVE_RT2880)

    if( ( nvram_match( "pptp_pass", "1" ) || nvram_match( "l2tp_pass", "1" )
	  || nvram_match( "ipsec_pass", "1" ) ) )
    {
	insmod( "nf_conntrack_proto_gre" );
	dd_syslog( LOG_INFO,
		   "vpn modules : nf_conntrack_proto_gre successfully loaded\n" );
	insmod( "nf_nat_proto_gre" );
	dd_syslog( LOG_INFO,
		   "vpn modules : nf_nat_proto_gre successfully loaded\n" );
    }
    if( nvram_match( "pptp_pass", "1" ) )
    {
	insmod( "nf_conntrack_pptp" );
	dd_syslog( LOG_INFO,
		   "vpn modules : nf_conntrack_pptp successfully loaded\n" );
	insmod( "nf_nat_pptp" );
	dd_syslog( LOG_INFO,
		   "vpn modules : nf_nat_pptp successfully loaded\n" );
    }

#else
    if( ( nvram_match( "pptp_pass", "1" ) || nvram_match( "l2tp_pass", "1" )
	  || nvram_match( "ipsec_pass", "1" ) ) )
    {
	insmod( "ip_conntrack_proto_gre" );
	dd_syslog( LOG_INFO,
		   "vpn modules : ip_conntrack_proto_gre successfully loaded\n" );
	insmod( "ip_nat_proto_gre" );
	dd_syslog( LOG_INFO,
		   "vpn modules : ip_nat_proto_gre successfully loaded\n" );
    }
    if( nvram_match( "pptp_pass", "1" ) )
    {
	insmod( "ip_conntrack_pptp" );
	dd_syslog( LOG_INFO,
		   "vpn modules : ip_conntrack_pptp successfully loaded\n" );
	insmod( "ip_nat_pptp" );
	dd_syslog( LOG_INFO,
		   "vpn modules : ip_nat_pptp successfully loaded\n" );
    }
#endif
}

void stop_vpn_modules( void )
{
#if defined(HAVE_XSCALE) || defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5) || defined(HAVE_RT2880)
    rmmod( "nf_nat_pptp" );
    rmmod( "nf_conntrack_pptp" );
    rmmod( "nf_nat_proto_gre" );
    rmmod( "nf_conntrack_proto_gre" );
    dd_syslog( LOG_INFO,
	       "vpn modules : vpn modules successfully unloaded\n" );
#else
    rmmod( "ip_nat_pptp" );
    rmmod( "ip_nat_proto_gre" );
    rmmod( "ip_conntrack_pptp" );
    rmmod( "ip_conntrack_proto_gre" );
    dd_syslog( LOG_INFO,
	       "vpn modules : vpn modules successfully unloaded\n" );

#endif
}

/*
 * AhMan March 18 2005 
 */
void start_tmp_ppp( int num );

int write_nvram( char *name, char *nv )
{
    if( nvram_invmatch( nv, "" ) )
    {
	writenvram( nv, name );
    }
    else
	return -1;
    return 0;
}

int usejffs = 0;

void stop_dns_clear_resolv( void )
{
    FILE *fp_w;

    if( pidof( "dnsmasq" ) > 0 )
	dd_syslog( LOG_INFO,
		   "dnsmasq : dnsmasq daemon successfully stopped\n" );
    // int ret = killps("dnsmasq",NULL);
    int ret = killall( "dnsmasq", SIGTERM );

    /*
     * Save DNS to resolv.conf 
     */
    if( !( fp_w = fopen( RESOLV_FILE, "w" ) ) )
    {
	perror( RESOLV_FILE );
	return;
    }
    fprintf( fp_w, " " );
    fclose( fp_w );

    cprintf( "done\n" );
    return;
}

#if 0
void start_ntpc( void )
{
    char *servers = nvram_safe_get( "ntp_server" );

    if( !nvram_invmatch( "ntpd_enable", "0" ) )
	return;

    if( strlen( servers ) )
    {
	char *nas_argv[] =
	    { "ntpclient", "-h", servers, "-i", "5", "-l", "-s", "-c", "2",
	    NULL
	};
	pid_t pid;

	_evalpid( nas_argv, NULL, 0, &pid );
	dd_syslog( LOG_INFO,
		   "ntpclient : ntp client successfully started\n" );
    }

    cprintf( "done\n" );
    return;
}
#endif
void stop_ntpc( void )
{
    if( pidof( "ntpclient" ) > 0 )
	dd_syslog( LOG_INFO,
		   "ntpclient : ntp client successfully stopped\n" );
    int ret = killall( "ntpclient", SIGTERM );

    cprintf( "done\n" );
    return;
}

// ///////////////////////////////////////////////////
void start_resetbutton( void )
{
    int ret = 0;

    ret = eval( "resetbutton" );
    dd_syslog( LOG_INFO,
	       "reset button : resetbutton daemon successfully started\n" );

    cprintf( "done\n" );
    return;
}

void stop_resetbutton( void )
{
    int ret = 0;

    if( pidof( "resetbutton" ) > 0 )
	dd_syslog( LOG_INFO,
		   "reset button : resetbutton daemon successfully stopped\n" );
    ret = killall( "resetbutton", SIGKILL );

    cprintf( "done\n" );
    return;
}

void start_iptqueue( void )
{
    int ret = 0;

    if( !nvram_invmatch( "iptqueue_enable", "0" ) )
	return;

    ret = eval( "iptqueue" );
    dd_syslog( LOG_INFO, "iptqueue successfully started\n" );

    cprintf( "done\n" );
    return;
}

void stop_iptqueue( void )
{
    int ret = 0;

    if( pidof( "iptqueue" ) > 0 )
	dd_syslog( LOG_INFO,
		   "iptqueue : iptqueue daemon successfully stopped\n" );
    ret = killall( "iptqueue", SIGKILL );

    cprintf( "done\n" );
    return;
}

void start_cron( void )
{
    int ret = 0;
    struct stat buf;

    if( nvram_match( "cron_enable", "0" ) )
	return;

    /*
     * Create cron's database directory 
     */
    if( stat( "/var/spool", &buf ) != 0 )
    {
	mkdir( "/var/spool", 0700 );
	mkdir( "/var/spool/cron", 0700 );
    }
    mkdir( "/tmp/cron.d", 0700 );

    buf_to_file( "/tmp/cron.d/check_ps",
		 "*/2 * * * * root /sbin/check_ps\n" );
    if( nvram_match( "reconnect_enable", "1" ) )	// pppoe reconnect
    {
	FILE *fp;

	fp = fopen( "/tmp/cron.d/pppoe_reconnect", "w" );
	fprintf( fp, "%s %s * * * root /usr/bin/killall pppd\n",
		 nvram_safe_get( "reconnect_minutes" ),
		 nvram_safe_get( "reconnect_hours" ) );
	fclose( fp );
    }
    /*
     * reboot scheduler 
     */
    unlink( "/tmp/cron.d/check_schedules" );
    if( nvram_match( "schedule_enable", "1" )
	&& nvram_match( "schedule_hour_time", "2" ) )
    {
	FILE *fp;

	fp = fopen( "/tmp/cron.d/check_schedules", "w" );
	fprintf( fp, "%s %s * * %s root /sbin/reboot\n",
		 nvram_safe_get( "schedule_minutes" ),
		 nvram_safe_get( "schedule_hours" ),
		 nvram_safe_get( "schedule_weekdays" ) );
	fclose( fp );
    }

    /*
     * Additional options 
     */
    int i = 0;

    unlink( "/tmp/cron.d/cron_jobs" );

    if( nvram_invmatch( "cron_jobs", "" ) )
    {
	FILE *fp;

	fp = fopen( "/tmp/cron.d/cron_jobs", "w" );
	char *cron_job = nvram_safe_get( "cron_jobs" );

	do
	{
	    if( cron_job[i] != 0x0D )	// strip dos CRs
		fprintf( fp, "%c", cron_job[i] );
	}
	while( cron_job[++i] );

	fprintf( fp, "\n" );	// extra new line at the end

	fclose( fp );
    }

    /*
     * Custom cron files 
     */
    eval( "cp", "-af", "/tmp/mycron.d/*", "/tmp/cron.d/" );
    eval( "cp", "-af", "/jffs/mycron.d/*", "/tmp/cron.d/" );
    eval( "cp", "-af", "/mmc/mycron.d/*", "/tmp/cron.d/" );

    cprintf( "starting cron\n" );
    ret = eval( "cron" );
    dd_syslog( LOG_INFO, "cron : cron daemon successfully started\n" );

    cprintf( "done\n" );
    return;
}

void stop_cron( void )
{
    int ret = 0;

    if( pidof( "cron" ) > 0 )
	dd_syslog( LOG_INFO, "cron : cron daemon successfully stopped\n" );
    // ret = killps("cron","-9");
    ret = killall( "cron", SIGKILL );
    eval( "rm", "-rf", "/tmp/cron.d" );
    cprintf( "done\n" );
    return;
}

#ifdef HAVE_SYSLOG
void start_syslog( void )
{
    int ret1 = 0, ret2 = 0;

    if( !nvram_invmatch( "syslogd_enable", "0" ) )
	return;

    if( strlen( nvram_safe_get( "syslogd_rem_ip" ) ) > 0 )
	ret1 = eval( "syslogd", "-R", nvram_safe_get( "syslogd_rem_ip" ) );
    else
	ret1 = eval( "syslogd", "-L" );

    dd_syslog( LOG_INFO, "syslogd : syslog daemon successfully started\n" );
    ret2 = eval( "klogd" );
    dd_syslog( LOG_INFO, "klogd : klog daemon successfully started\n" );

    return;
}

void stop_syslog( void )
{
    int ret;

    if( pidof( "klogd" ) > 0 )
	dd_syslog( LOG_INFO, "klogd : klog daemon successfully stopped\n" );
    ret = killall( "klogd", SIGKILL );
    if( pidof( "syslogd" ) > 0 )
	dd_syslog( LOG_INFO,
		   "syslogd : syslog daemon successfully stopped\n" );
    ret += killall( "syslogd", SIGKILL );

    cprintf( "done\n" );
    return;
}
#endif

void stop_redial( void )
{
    int ret;

    if( pidof( "redial" ) > 0 )
	dd_syslog( LOG_INFO,
		   "ppp_redial : redial daemon successfully stopped\n" );
    // ret = killps("redial","-9");
    ret = killall( "redial", SIGKILL );

    cprintf( "done\n" );
    return;
}

void start_redial( void )
{
    int ret;
    pid_t pid;
    char *redial_argv[] = { "/tmp/ppp/redial",
	nvram_safe_get( "ppp_redialperiod" ),
	NULL
    };
    if( pidof( "redial" ) > 0 )
    {
	return;		// not required, already running
    }

    symlink( "/sbin/rc", "/tmp/ppp/redial" );

    ret = _evalpid( redial_argv, NULL, 0, &pid );
    dd_syslog( LOG_INFO,
	       "ppp_redial : redial process successfully started\n" );

    cprintf( "done\n" );
    return;
}

#ifdef HAVE_RADVD
void start_radvd( void )
{
    int ret = 0;
    int c = 0;
    char *buf, *buf2;
    int i;
    FILE *fp;

    if( !nvram_match( "radvd_enable", "1" ) )
	return;
    if( !nvram_match( "ipv6_enable", "1" ) )
	return;
    buf = nvram_safe_get( "radvd_conf" );
    if( buf != NULL )
    {
	buf2 = ( char * )malloc( strlen( buf ) + 1 );
	memcpy( buf2, buf, strlen( buf ) );
	buf2[strlen( buf )] = 0;

	i = 0;
	while( buf2[i++] != 0 )
	{
	    cprintf( "." );
	    if( buf2[i - 1] == '\r' )
		continue;
	    buf2[c++] = buf2[i - 1];
	}
	buf2[c++] = 0;
	fp = fopen( "/tmp/radvd.conf", "wb" );
	fwrite( buf2, 1, c - 1, fp );
	fclose( fp );
	free( buf2 );
    }
    // nvram2file("radvd_conf", "/tmp/radvd.conf");

    system2( "sync" );

    ret = eval( "radvd" );
    dd_syslog( LOG_INFO, "radvd : RADV daemon successfully started\n" );

    cprintf( "done\n" );
    return;
}

void stop_radvd( void )
{
    int ret = 0;

    if( pidof( "radvd" ) > 0 )
	dd_syslog( LOG_INFO, "radvd : RADV daemon successfully stopped\n" );
    // ret = killps("radvd",NULL);
    ret = killall( "radvd", SIGKILL );

    unlink( "/var/run/radvd.pid" );

    cprintf( "done\n" );
    return;
}
#endif
#ifdef HAVE_IPV6
void start_ipv6( void )
{
    int ret = 0;

    if( !nvram_invmatch( "ipv6_enable", "0" ) )
	return;

    ret = insmod( "ipv6" );
    dd_syslog( LOG_INFO, "ipv6 successfully started\n" );

    cprintf( "done\n" );
    return;
}
#endif

#ifdef HAVE_PPPOE
void stop_pppoe( void )
{
    int ret;

    unlink( "/tmp/ppp/link" );
    if( pidof( "pppd" ) > 0 )
	dd_syslog( LOG_INFO, "pppoe process successfully stopped\n" );
    ret = killall( "pppd", SIGTERM );
    if( nvram_match( "wan_vdsl", "1" ) )
    {
	eval( "ifconfig", nvram_safe_get( "wan_iface" ), "down" );
	eval( "vconfig", "rem", nvram_safe_get( "wan_iface" ) );
    }
    // ret += killall ("ip-up", SIGKILL);
    // ret += killall ("ip-down", SIGKILL);

    cprintf( "done\n" );
    return;
}

void stop_single_pppoe( int pppoe_num )
{
    int ret;
    char pppoe_pid[15], pppoe_ifname[15];
    char ppp_unlink[2][20] = { "/tmp/ppp/link", "/tmp/ppp/link_1" };
    char ppp_wan_dns[2][20] = { "wan_get_dns", "wan_get_dns_1" };

    sprintf( pppoe_pid, "pppoe_pid%d", pppoe_num );
    sprintf( pppoe_ifname, "pppoe_ifname%d", pppoe_num );
    dprintf( "start! stop pppoe %d, pid %s \n", pppoe_num,
	     nvram_safe_get( pppoe_pid ) );

    ret = eval( "kill", nvram_safe_get( pppoe_pid ) );
    unlink( ppp_unlink[pppoe_num] );
    nvram_unset( pppoe_ifname );

    nvram_set( ppp_wan_dns[pppoe_num], "" );
    stop_dns_clear_resolv(  );

    dprintf( "done\n" );
    return;
}
#endif
void stop_dhcpc( void )
{
    int ret = 0;

    if( pidof( "udhcpc" ) > 0 )
	dd_syslog( LOG_INFO,
		   "udhcpc : udhcp client process successfully stopped\n" );
    ret = killall( "udhcpc", SIGTERM );

    cprintf( "done\n" );
    return;
}

#ifdef HAVE_PPTP

static void create_pptp_config( char *servername, char *username )
{

    FILE *fp;

    mkdir( "/tmp/ppp", 0777 );
    symlink( "/sbin/rc", "/tmp/ppp/ip-up" );
    symlink( "/sbin/rc", "/tmp/ppp/ip-down" );
    symlink( "/dev/null", "/tmp/ppp/connect-errors" );

    /*
     * Generate options file 
     */
    if( !( fp = fopen( "/tmp/ppp/options", "w" ) ) )
    {
	perror( "/tmp/ppp/options" );
	return;
    }
    fprintf( fp, "defaultroute\n" );	// Add a default route to the 
    // system routing tables,
    // using the peer as the
    // gateway
    fprintf( fp, "usepeerdns\n" );	// Ask the peer for up to 2 DNS
    // server addresses
    fprintf( fp, "pty 'pptp %s --nolaunchpppd", servername );

    // PPTP client also supports synchronous mode.
    // This should improve the speeds.
    if( nvram_match( "pptp_synchronous", "1" ) )
	fprintf( fp, " --sync'\nsync\n" );
    else
	fprintf( fp, "'\n" );

    fprintf( fp, "user '%s'\n", username );
    // fprintf(fp, "persist\n"); // Do not exit after a connection is
    // terminated.

    if( nvram_match( "mtu_enable", "1" ) )
	fprintf( fp, "mtu %s\n", nvram_safe_get( "wan_mtu" ) );

    if( nvram_match( "ppp_demand", "1" ) )
    {				// demand mode
	fprintf( fp, "idle %d\n",
		 nvram_match( "ppp_demand",
			      "1" ) ?
		 atoi( nvram_safe_get( "ppp_idletime" ) ) * 60 : 0 );
	fprintf( fp, "demand\n" );	// Dial on demand
	fprintf( fp, "persist\n" );	// Do not exit after a connection is
	// terminated.
	fprintf( fp, "%s:%s\n", PPP_PSEUDO_IP, PPP_PSEUDO_GW );	// <local 
	// IP>:<remote 
	// IP>
	fprintf( fp, "ipcp-accept-remote\n" );
	fprintf( fp, "ipcp-accept-local\n" );
	fprintf( fp, "connect true\n" );
	fprintf( fp, "noipdefault\n" );	// Disables the default
	// behaviour when no local IP 
	// address is specified
	fprintf( fp, "ktune\n" );	// Set /proc/sys/net/ipv4/ip_dynaddr
	// to 1 in demand mode if the local
	// address changes
    }
    else
    {				// keepalive mode
	start_redial(  );
    }
    if( nvram_match( "pptp_encrypt", "0" ) )
    {
	fprintf( fp, "nomppe\n" );	// Disable mppe negotiation
	fprintf( fp, "noccp\n" );	// Disable CCP (Compression Control
	// Protocol)
    }
    else
    {
	fprintf( fp, "mppe required,stateless\n" );
    }
    fprintf( fp, "default-asyncmap\n" );	// Disable asyncmap
    // negotiation
    fprintf( fp, "nopcomp\n" );	// Disable protocol field compression
    fprintf( fp, "noaccomp\n" );	// Disable Address/Control
    // compression
    fprintf( fp, "novj\n" );	// Disable Van Jacobson style TCP/IP
    // header compression
    fprintf( fp, "nobsdcomp\n" );	// Disables BSD-Compress compression
    fprintf( fp, "nodeflate\n" );	// Disables Deflate compression
    fprintf( fp, "lcp-echo-interval 0\n" );	// Don't send an LCP
    // echo-request frame to the
    // peer
    fprintf( fp, "noipdefault\n" );
    fprintf( fp, "lock\n" );
    fprintf( fp, "noauth\n" );

    if( nvram_invmatch( "pptp_extraoptions", "" ) )
	fprintf( fp, "%s\n", nvram_safe_get( "pptp_extraoptions" ) );

    fclose( fp );

}

void start_pptp( int status )
{
    int ret;
    FILE *fp;
    char *pptp_argv[] = { "pppd",
	NULL
    };
    char username[80], passwd[80];

    stop_dhcpc(  );
#ifdef HAVE_PPPOE
    stop_pppoe(  );
#endif
    stop_vpn_modules(  );

    if( nvram_match( "aol_block_traffic", "0" ) )
    {
	snprintf( username, sizeof( username ), "%s",
		  nvram_safe_get( "ppp_username" ) );
	snprintf( passwd, sizeof( passwd ), "%s",
		  nvram_safe_get( "ppp_passwd" ) );
    }
    else
    {
	if( !strcmp( nvram_safe_get( "aol_username" ), "" ) )
	{
	    snprintf( username, sizeof( username ), "%s",
		      nvram_safe_get( "ppp_username" ) );
	    snprintf( passwd, sizeof( passwd ), "%s",
		      nvram_safe_get( "ppp_passwd" ) );
	}
	else
	{
	    snprintf( username, sizeof( username ), "%s",
		      nvram_safe_get( "aol_username" ) );
	    snprintf( passwd, sizeof( passwd ), "%s",
		      nvram_safe_get( "aol_passwd" ) );
	}
    }

    if( status != REDIAL )
    {
	create_pptp_config( nvram_safe_get( "pptp_server_name" ), username );
	/*
	 * Generate pap-secrets file 
	 */
	if( !( fp = fopen( "/tmp/ppp/pap-secrets", "w" ) ) )
	{
	    perror( "/tmp/ppp/pap-secrets" );
	    return;
	}
	fprintf( fp, "\"%s\" * \"%s\" *\n", username, passwd );
	fclose( fp );
	chmod( "/tmp/ppp/pap-secrets", 0600 );

	/*
	 * Generate chap-secrets file 
	 */
	if( !( fp = fopen( "/tmp/ppp/chap-secrets", "w" ) ) )
	{
	    perror( "/tmp/ppp/chap-secrets" );
	    return;
	}
	fprintf( fp, "\"%s\" * \"%s\" *\n", username, passwd );
	fclose( fp );
	chmod( "/tmp/ppp/chap-secrets", 0600 );

	/*
	 * Enable Forwarding 
	 */
	if( ( fp = fopen( "/proc/sys/net/ipv4/ip_forward", "r+" ) ) )
	{
	    fputc( '1', fp );
	    fclose( fp );
	}
	else
	    perror( "/proc/sys/net/ipv4/ip_forward" );
    }
    char *wan_ifname = nvram_safe_get( "wan_ifname" );

    if( isClient(  ) )
    {
	wan_ifname = getSTA(  );
    }

    nvram_set( "pptp_ifname", wan_ifname );
    /*
     * Bring up WAN interface 
     */
    if( nvram_match( "pptp_use_dhcp", "1" ) )
    {
	// pid_t pid;
	// char *wan_ipaddr;
	// char *wan_netmask;
	// char *wan_gateway;

	// char *pptp_server_ip = nvram_safe_get ("pptp_server_ip");
	// char *wan_hostname = nvram_safe_get ("wan_hostname");

	nvram_set( "wan_get_dns", "" );
	nvram_unset( "dhcpc_done" );
	//dirty hack
	start_dhcpc( wan_ifname );
	int timeout;

	for( timeout = 60; !nvram_match( "dhcpc_done", "1" ) && timeout > 0;
	     --timeout )
	{			/* wait for info from dhcp server */
	    sleep( 1 );
	}
	stop_dhcpc(  );		/* we don't need dhcp client anymore */
	create_pptp_config( nvram_safe_get( "pptp_server_ip" ), username );

	/*
	 * //this stuff has already been configured in dhcpc->bound
	 * wan_ipaddr = nvram_safe_get ("wan_ipaddr"); wan_netmask =
	 * nvram_safe_get ("wan_netmask"); wan_gateway = nvram_safe_get
	 * ("wan_gateway"); pptp_server_ip = nvram_safe_get
	 * ("pptp_server_ip");
	 * 
	 * while (route_del (wan_ifname, 0, NULL, NULL, NULL) == 0);
	 * 
	 * 
	 * for (timeout = 10; ifconfig (wan_ifname, IFUP, wan_ipaddr,
	 * wan_netmask) && timeout > 0; --timeout) { sleep (1); } for
	 * (timeout = 10; route_add (wan_ifname, 0, pptp_server_ip,
	 * wan_gateway, "255.255.255.255") && timeout > 0; --timeout) { sleep 
	 * (1); }
	 */
    }
    else
    {
	ifconfig( wan_ifname, IFUP,
		  nvram_safe_get( "wan_ipaddr" ),
		  nvram_safe_get( "wan_netmask" ) );
    }
    ret = _evalpid( pptp_argv, NULL, 0, NULL );

    /*
     * if(nvram_match("pptp_usedhcp", "1")){ char *wan_hostname =
     * nvram_get("wan_hostname"); char *dhcp_argv[] = { "udhcpc", "-i",
     * nvram_safe_get("wan_ifname"), "-p", "/var/run/udhcpc.pid", "-s",
     * "/tmp/udhcpc", wan_hostname && *wan_hostname ? "-H" : NULL,
     * wan_hostname && *wan_hostname ? wan_hostname : NULL, NULL };
     * 
     * ifconfig(nvram_safe_get("wan_ifname"), IFUP, NULL, NULL);
     * 
     * symlink("/sbin/rc", "/tmp/udhcpc"); nvram_set("wan_get_dns","");
     * //killps("udhcpc",NULL);
     * 
     * eval("killall","udhcpc");
     * 
     * _eval(dhcp_argv, NULL, 0, &pid);
     * 
     * // Give enough time for DHCP to get IP address. sleep(2);
     * 
     * } else ifconfig(nvram_safe_get("wan_ifname"), IFUP,
     * nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
     * 
     * // Start pptp client on wan interface ret = _eval(pptp_argv, NULL, 0,
     * NULL); 
     */
    if( nvram_match( "ppp_demand", "1" ) )
    {
	/*
	 * Trigger Connect On Demand if user press Connect button in Status
	 * page 
	 */
	if( nvram_match( "action_service", "start_pptp" )
	    || nvram_match( "action_service", "start_l2tp" ) )
	{
	    start_force_to_dial(  );
	    // force_to_dial(nvram_safe_get("action_service"));
	    nvram_unset( "action_service" );
	}
	/*
	 * Trigger Connect On Demand if user ping pptp server 
	 */
	else
	{
	    eval( "listen", nvram_safe_get( "lan_ifname" ) );
	}
    }
    stop_wland(  );
    start_wshaper(  );
    start_wland(  );
    cprintf( "done\n" );
    return;
}

void stop_pptp( void )
{
    int ret;

    route_del( nvram_safe_get( "wan_ifname" ), 0,
	       nvram_safe_get( "pptp_server_ip" ), NULL, NULL );

    unlink( "/tmp/ppp/link" );
    // ret = killps("pppd","-9");
    // ret += killps("pptp","-9");
    // ret += killps("listen","-9");
    ret = killall( "pppd", SIGTERM );
    ret += killall( "pptp", SIGKILL );
    ret += killall( "listen", SIGKILL );

    cprintf( "done\n" );
    return;
}

#endif

// =========================================tallest============================================
/*
 * AhMan March 18 2005 Start the Original Linksys PPPoE 
 */
/*
 * This function build the pppoe instuction & execute it.
 */
#ifdef HAVE_PPPOE
void start_pppoe( int pppoe_num )
{
    char idletime[20], retry_num[20], param[4];
    char username[80], passwd[80];

    char ppp_username[2][20] = { "ppp_username", "ppp_username_1" };
    char ppp_passwd[2][20] = { "ppp_passwd", "ppp_passwd_1" };
    char ppp_demand[2][20] = { "ppp_demand", "ppp_demand_1" };
    char ppp_service[2][20] = { "ppp_service", "ppp_service_1" };
    char ppp_ac[2][10] = { "ppp_ac", "ppp_ac_1" };
    // char wanip[2][15] = { "wan_ipaddr", "wan_ipaddr_1" };
    // char wanmask[2][15] = { "wan_netmask", "wan_netmask_1" };
    // char wangw[2][15] = { "wan_gateway", "wan_gateway_1" };
    char pppoeifname[15];
    char *wan_ifname = nvram_safe_get( "wan_ifname" );

    if( isClient(  ) )
    {
	wan_ifname = getSTA(  );
    }

    pid_t pid;

    sprintf( pppoeifname, "pppoe_ifname%d", pppoe_num );
    nvram_set( pppoeifname, "" );

    cprintf( "start session %d\n", pppoe_num );
    sprintf( idletime, "%d", atoi( nvram_safe_get( "ppp_idletime" ) ) * 60 );
    snprintf( retry_num, sizeof( retry_num ), "%d",
	      ( atoi( nvram_safe_get( "ppp_redialperiod" ) ) / 5 ) - 1 );

    if( nvram_match( "aol_block_traffic", "1" ) && pppoe_num == PPPOE0 )
    {
	if( !strcmp( nvram_safe_get( "aol_username" ), "" ) )
	{
	    snprintf( username, sizeof( username ), "%s",
		      nvram_safe_get( "ppp_username" ) );
	    snprintf( passwd, sizeof( passwd ), "%s",
		      nvram_safe_get( "ppp_passwd" ) );
	}
	else
	{
	    snprintf( username, sizeof( username ), "%s",
		      nvram_safe_get( "aol_username" ) );
	    snprintf( passwd, sizeof( passwd ), "%s",
		      nvram_safe_get( "aol_passwd" ) );
	}

    }
    else
    {
	snprintf( username, sizeof( username ), "%s",
		  nvram_safe_get( ppp_username[pppoe_num] ) );
	snprintf( passwd, sizeof( passwd ), "%s",
		  nvram_safe_get( ppp_passwd[pppoe_num] ) );
    }
    sprintf( param, "%d", pppoe_num );
    /*
     * add here 
     */
    char *pppoe_argv[] = { "pppoecd",
	wan_ifname,
	"-u", username,
	"-p", passwd,
	"-r", nvram_safe_get( "wan_mtu" ),	// del by honor, add by
	// tallest.
	"-t", nvram_safe_get( "wan_mtu" ),
	"-i", nvram_match( ppp_demand[pppoe_num], "1" ) ? idletime : "0",
	"-I", "30",		// Send an LCP echo-request frame to the
	// server every 30 seconds
	"-T", "3",		// pppd will presume the server to be dead if 
	// 5 LCP echo-requests are sent without
	// receiving a valid LCP echo-reply
	"-P", param,		// PPPOE session number.
	"-N", retry_num,	// To avoid kill pppd when pppd has been
	// connecting.
#if LOG_PPPOE == 2
	"-d",
#endif
	"-C", "disconnected_pppoe",	// by tallest 0407
	NULL,			/* set default route */
	NULL, NULL,		/* pppoe_service */
	NULL, NULL,		/* pppoe_ac */
	NULL,			/* pppoe_keepalive */
	NULL
    }, **arg;
    /*
     * Add optional arguments 
     */
    for( arg = pppoe_argv; *arg; arg++ );

    /*
     * Removed by AhMan 
     */

    if( pppoe_num == PPPOE0 )
    {				// PPPOE0 must set default route.
	*arg++ = "-R";
    }

    if( nvram_invmatch( ppp_service[pppoe_num], "" ) )
    {
	*arg++ = "-s";
	*arg++ = nvram_safe_get( ppp_service[pppoe_num] );
    }
    if( nvram_invmatch( ppp_ac[pppoe_num], "" ) )
    {
	*arg++ = "-a";
	*arg++ = nvram_safe_get( ppp_ac[pppoe_num] );
    }
    if( nvram_match( "ppp_static", "1" ) )
    {
	*arg++ = "-L";
	*arg++ = nvram_safe_get( "ppp_static_ip" );
    }
    // if (nvram_match("pppoe_demand", "1") || nvram_match("pppoe_keepalive", 
    // "1"))
    *arg++ = "-k";

    mkdir( "/tmp/ppp", 0777 );
    symlink( "/sbin/rc", "/tmp/ppp/ip-up" );
    symlink( "/sbin/rc", "/tmp/ppp/ip-down" );
    symlink( "/sbin/rc", "/tmp/ppp/set-pppoepid" );	// tallest 1219
    unlink( "/tmp/ppp/log" );

    // Clean rpppoe client files - Added by ice-man (Wed Jun 1)
    unlink( "/tmp/ppp/options.pppoe" );
    unlink( "/tmp/ppp/connect-errors" );

    _evalpid( pppoe_argv, NULL, 0, &pid );

    if( nvram_match( ppp_demand[pppoe_num], "1" ) )
    {
	// int timeout = 5;
	start_tmp_ppp( pppoe_num );

	// This should be handled in start_wan_done
	// while (ifconfig (nvram_safe_get (pppoeifname), IFUP, NULL, NULL)
	// && timeout--)
	// sleep (1);
	// route_add (nvram_safe_get ("wan_iface"), 0, "0.0.0.0",
	// "10.112.112.112",
	// "0.0.0.0");

    }
    cprintf( "done. session %d\n", pppoe_num );
    return;
}
#endif
/*
 * AhMan March 18 2005 
 */
/*
 * Get the IP, Subnetmask, Geteway from WAN interface
 * and set to NV ram.
 */
void start_tmp_ppp( int num )
{

    int timeout = 5;
    char pppoeifname[15];
    char wanip[2][15] = { "wan_ipaddr", "wan_ipaddr_1" };
    char wanmask[2][15] = { "wan_netmask", "wan_netmask_1" };
    char wangw[2][15] = { "wan_gateway", "wan_gateway_1" };
    // char wanif[2][15]={"wan_ifname","wan_ifname_1"};
    // char *wan_ifname = nvram_safe_get("wan_ifname");
    struct ifreq ifr;
    int s;

    cprintf( "start session %d\n", num );

    sprintf( pppoeifname, "pppoe_ifname%d", num );

    if( ( s = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) < 0 )
	return;

    /*
     * Wait for ppp0 to be created 
     */
    while( ifconfig( nvram_safe_get( pppoeifname ), IFUP, NULL, NULL )
	   && timeout-- )
	sleep( 1 );

    strncpy( ifr.ifr_name, nvram_safe_get( pppoeifname ), IFNAMSIZ );

    /*
     * Set temporary IP address 
     */
    timeout = 3;
    while( ioctl( s, SIOCGIFADDR, &ifr ) && timeout-- )
    {
	perror( nvram_safe_get( pppoeifname ) );
	printf( "Wait %s inteface to init (1) ...\n",
		nvram_safe_get( pppoeifname ) );
	sleep( 1 );
    };
    nvram_set( wanip[num], inet_ntoa( sin_addr( &( ifr.ifr_addr ) ) ) );
    nvram_set( wanmask[num], "255.255.255.255" );

    /*
     * Set temporary P-t-P address 
     */
    timeout = 3;
    while( ioctl( s, SIOCGIFDSTADDR, &ifr ) && timeout-- )
    {
	perror( nvram_safe_get( pppoeifname ) );
	printf( "Wait %s inteface to init (2) ...\n",
		nvram_safe_get( pppoeifname ) );
	sleep( 1 );
    }
    nvram_set( wangw[num], inet_ntoa( sin_addr( &( ifr.ifr_dstaddr ) ) ) );

    start_wan_done( nvram_safe_get( pppoeifname ) );

    // if user press Connect" button from web, we must force to dial
    if( nvram_match( "action_service", "start_pppoe" )
	|| nvram_match( "action_service", "start_pppoe_1" ) )
    {
	sleep( 3 );
	// force_to_dial(nvram_safe_get("action_service"));
	start_force_to_dial(  );
	nvram_unset( "action_service" );
    }

    close( s );
    cprintf( "done session %d\n", num );
    return;
}

// =====================================================================================================

#ifdef HAVE_L2TP
void start_l2tp( int status )
{
    int ret;
    FILE *fp;
    char *l2tp_argv[] = { "l2tpd",
	NULL
    };
    char username[80], passwd[80];

    // stop_dhcpc();
#ifdef HAVE_PPPOE
    stop_pppoe(  );
#endif
#ifdef HAVE_PPTP
    stop_pptp(  );
#endif

    if( nvram_match( "aol_block_traffic", "0" ) )
    {
	snprintf( username, sizeof( username ), "%s",
		  nvram_safe_get( "ppp_username" ) );
	snprintf( passwd, sizeof( passwd ), "%s",
		  nvram_safe_get( "ppp_passwd" ) );
    }
    else
    {
	if( !strcmp( nvram_safe_get( "aol_username" ), "" ) )
	{
	    snprintf( username, sizeof( username ), "%s",
		      nvram_safe_get( "ppp_username" ) );
	    snprintf( passwd, sizeof( passwd ), "%s",
		      nvram_safe_get( "ppp_passwd" ) );
	}
	else
	{
	    snprintf( username, sizeof( username ), "%s",
		      nvram_safe_get( "aol_username" ) );
	    snprintf( passwd, sizeof( passwd ), "%s",
		      nvram_safe_get( "aol_passwd" ) );
	}
    }

    if( status != REDIAL )
    {
	mkdir( "/tmp/ppp", 0777 );
	symlink( "/sbin/rc", "/tmp/ppp/ip-up" );
	symlink( "/sbin/rc", "/tmp/ppp/ip-down" );
	symlink( "/dev/null", "/tmp/ppp/connect-errors" );

	/*
	 * Generate L2TP configuration file 
	 */
	if( !( fp = fopen( "/tmp/l2tp.conf", "w" ) ) )
	{
	    perror( "/tmp/l2tp.conf" );
	    return;
	}
	fprintf( fp, "global\n" );	// Global section
	fprintf( fp, "load-handler \"sync-pppd.so\"\n" );	// Load
	// handlers
	fprintf( fp, "load-handler \"cmd.so\"\n" );
	fprintf( fp, "listen-port 1701\n" );	// Bind address
	fprintf( fp, "section sync-pppd\n" );	// Configure the sync-pppd
	// handler
	fprintf( fp, "section peer\n" );	// Peer section
	fprintf( fp, "peer %s\n", nvram_safe_get( "l2tp_server_ip" ) );
	fprintf( fp, "port 1701\n" );
	fprintf( fp, "lac-handler sync-pppd\n" );
	fprintf( fp, "section cmd\n" );	// Configure the cmd handler
	fclose( fp );

	/*
	 * Generate options file 
	 */
	if( !( fp = fopen( "/tmp/ppp/options", "w" ) ) )
	{
	    perror( "/tmp/ppp/options" );
	    return;
	}
	fprintf( fp, "defaultroute\n" );	// Add a default route to the 
	// system routing tables,
	// using the peer as the
	// gateway
	fprintf( fp, "usepeerdns\n" );	// Ask the peer for up to 2 DNS
	// server addresses
	// fprintf(fp, "pty 'pptp %s
	// --nolaunchpppd'\n",nvram_safe_get("pptp_server_ip")); 
	fprintf( fp, "user '%s'\n", username );
	// fprintf(fp, "persist\n"); // Do not exit after a connection is
	// terminated.

	if( nvram_match( "mtu_enable", "1" ) )
	{
	    fprintf( fp, "mtu %s\n", nvram_safe_get( "wan_mtu" ) );
	}

	if( nvram_match( "ppp_demand", "1" ) )
	{			// demand mode
	    fprintf( fp, "idle %d\n",
		     nvram_match( "ppp_demand",
				  "1" ) ?
		     atoi( nvram_safe_get( "ppp_idletime" ) ) * 60 : 0 );
	    // fprintf(fp, "demand\n"); // Dial on demand
	    // fprintf(fp, "persist\n"); // Do not exit after a connection is 
	    // terminated.
	    // fprintf(fp, "%s:%s\n",PPP_PSEUDO_IP,PPP_PSEUDO_GW); // <local
	    // IP>:<remote IP>
	    fprintf( fp, "ipcp-accept-remote\n" );
	    fprintf( fp, "ipcp-accept-local\n" );
	    fprintf( fp, "connect true\n" );
	    fprintf( fp, "noipdefault\n" );	// Disables the default
	    // behaviour when no local IP 
	    // address is specified
	    fprintf( fp, "ktune\n" );	// Set /proc/sys/net/ipv4/ip_dynaddr
	    // to 1 in demand mode if the local
	    // address changes
	}
	else
	{			// keepalive mode
	    start_redial(  );
	}

	fprintf( fp, "default-asyncmap\n" );	// Disable asyncmap
	// negotiation
	fprintf( fp, "nopcomp\n" );	// Disable protocol field compression
	fprintf( fp, "noaccomp\n" );	// Disable Address/Control
	// compression 
	fprintf( fp, "noccp\n" );	// Disable CCP (Compression Control
	// Protocol)
	fprintf( fp, "novj\n" );	// Disable Van Jacobson style TCP/IP
	// header compression
	fprintf( fp, "nobsdcomp\n" );	// Disables BSD-Compress compression
	fprintf( fp, "nodeflate\n" );	// Disables Deflate compression
	fprintf( fp, "lcp-echo-interval 0\n" );	// Don't send an LCP
	// echo-request frame to the
	// peer
	fprintf( fp, "lock\n" );
	fprintf( fp, "noauth\n" );

	fclose( fp );

	/*
	 * Generate pap-secrets file 
	 */
	if( !( fp = fopen( "/tmp/ppp/pap-secrets", "w" ) ) )
	{
	    perror( "/tmp/ppp/pap-secrets" );
	    return;
	}
	fprintf( fp, "\"%s\" * \"%s\" *\n", username, passwd );
	fclose( fp );
	chmod( "/tmp/ppp/pap-secrets", 0600 );

	/*
	 * Generate chap-secrets file 
	 */
	if( !( fp = fopen( "/tmp/ppp/chap-secrets", "w" ) ) )
	{
	    perror( "/tmp/ppp/chap-secrets" );
	    return;
	}
	fprintf( fp, "\"%s\" * \"%s\" *\n", username, passwd );
	fclose( fp );
	chmod( "/tmp/ppp/chap-secrets", 0600 );

	/*
	 * Enable Forwarding 
	 */
	if( ( fp = fopen( "/proc/sys/net/ipv4/ip_forward", "r+" ) ) )
	{
	    fputc( '1', fp );
	    fclose( fp );
	}
	else
	    perror( "/proc/sys/net/ipv4/ip_forward" );
    }

    /*
     * Bring up WAN interface 
     */
    // ifconfig(nvram_safe_get("wan_ifname"), IFUP,
    // nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));

    ret = _evalpid( l2tp_argv, NULL, 0, NULL );
    sleep( 1 );

    if( nvram_match( "ppp_demand", "1" ) )
    {
	/*
	 * Trigger Connect On Demand if user press Connect button in Status
	 * page 
	 */
	if( nvram_match( "action_service", "start_l2tp" ) )
	{
	    start_force_to_dial(  );
	    nvram_unset( "action_service" );
	}
	/*
	 * Trigger Connect On Demand if user ping pptp server 
	 */
	else
	    eval( "listen", nvram_safe_get( "lan_ifname" ) );
    }
    else
	sysprintf( "l2tp-control \"start-session %s\"",
		   nvram_safe_get( "l2tp_server_ip" ) );

    cprintf( "done\n" );
    return;
}

void start_l2tp_redial( void )
{
    start_l2tp( REDIAL );
}

void start_l2tp_boot( void )
{
    start_l2tp( BOOT );
}

void stop_l2tp( void )
{
    int ret = 0;

    unlink( "/tmp/ppp/link" );
    // ret = killps("pppd","-9");
    // ret += killps("l2tpd","-9");
    // ret += killps("listen","-9");

    ret = killall( "pppd", SIGTERM );
    ret += killall( "l2tpd", SIGKILL );
    ret += killall( "listen", SIGKILL );

    cprintf( "done\n" );
    return;
}
#endif

void stop_wland( void )
{
    if( pidof( "wland" ) > 0 )
	dd_syslog( LOG_INFO, "wland : WLAN daemon successfully stopped\n" );
    int ret = killall( "wland", SIGKILL );

    cprintf( "done\n" );
    return;
}

void start_wland( void )
{
    int ret;
    pid_t pid;
    char *wland_argv[] = { "wland",
	NULL
    };

    stop_wland(  );

    // if( nvram_match("apwatchdog_enable", "0") )
    // return 0;

    ret = _evalpid( wland_argv, NULL, 0, &pid );
    dd_syslog( LOG_INFO, "wland : WLAN daemon successfully started\n" );
    cprintf( "done\n" );
    return;
}

void start_process_monitor( void )
{
    if( nvram_match( "pmonitor_enable", "0" ) )
	return;

    pid_t pid;

    char *argv[] = { "process_monitor", NULL };
    int ret = _evalpid( argv, NULL, 0, &pid );

    dd_syslog( LOG_INFO, "process_monitor successfully started\n" );

    cprintf( "done" );

    return;
}

void stop_process_monitor( void )
{
    int ret;

    if( pidof( "process_monitor" ) > 0 )
	dd_syslog( LOG_INFO, "process_monitor successfully stopped\n" );
    ret = killall( "process_monitor", SIGKILL );

    cprintf( "done\n" );

    return;
}

void start_radio_timer( void )
{
    if( nvram_match( "radio0_timer_enable", "0" )
	&& nvram_match( "radio1_timer_enable", "0" ) )
	return;
#ifdef HAVE_MADWIFI
    if( nvram_match( "ath0_net_mode", "disabled" ) )
#elif HAVE_MSSID
    if( nvram_match( "wl0_net_mode", "disabled" )
	&& nvram_match( "wl1_net_mode", "disabled" ) )
#else
    if( nvram_match( "wl_net_mode", "disabled" ) )
#endif
	return;

    pid_t pid;

    char *argv[] = { "radio_timer", NULL };
    int ret = _evalpid( argv, NULL, 0, &pid );

    dd_syslog( LOG_INFO,
	       "radio_timer : radio timer daemon successfully started\n" );

    cprintf( "done" );

    return;
}

void stop_radio_timer( void )
{
    int ret;

    if( pidof( "radio_timer" ) > 0 )
	dd_syslog( LOG_INFO,
		   "radio_timer : radio timer daemon successfully stopped\n" );
    ret = killall( "radio_timer", SIGKILL );

    cprintf( "done\n" );

    return;
}

void start_ttraff( void )
{
    if( !nvram_match( "ttraff_enable", "1" ) )
	return;

    if( nvram_match( "wan_proto", "disabled" )
	|| nvram_match( "wl0_mode", "wet" )
	|| nvram_match( "wl0_mode", "apstawet" ) )
	return;

    pid_t pid;

    char *argv[] = { "ttraff", NULL };
    int ret = _evalpid( argv, NULL, 0, &pid );

    dd_syslog( LOG_INFO,
	       "ttraff : traffic counter daemon successfully started\n" );

    cprintf( "done" );

    return;
}

void stop_ttraff( void )
{
    int ret;

    if( pidof( "ttraff" ) > 0 )
	dd_syslog( LOG_INFO,
		   "ttraff : traffic counter daemon successfully stopped\n" );
    ret = killall( "ttraff", SIGKILL );

    cprintf( "done\n" );

    return;
}

extern void start_heartbeat_boot( void );

/*
 * Trigger Connect On Demand 
 */
void start_force_to_dial( void )
{
    // force_to_dial( char *whichone){
    int ret = 0;
    char dst[50];

    strcpy( &dst[0], nvram_safe_get( "wan_gateway" ) );

    char *ping_argv[] = { "ping",
	"-c", "1",
	dst,
	NULL
    };

    sleep( 1 );
#ifdef HAVE_L2TP
    if( nvram_match( "wan_proto", "l2tp" ) )
    {

	sysprintf( "l2tp-control \"start-session %s\"",
		   nvram_safe_get( "l2tp_server_ip" ) );
	return;
    }
#endif
#ifdef HAVE_HEARTBEAT
    if( nvram_match( "wan_proto", "heartbeat" ) )
    {
	start_heartbeat_boot(  );
	return ;
    }
#endif
    _evalpid( ping_argv, NULL, 3, NULL );

    return;
}

#ifdef HAVE_CPUTEMP

#ifdef HAVE_GATEWORX
#define TEMP_PATH "/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028"
// #define TEMP_PATH "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028"
#define TEMP_PREFIX "temp"
#define TEMP_MUL 100
#else
#ifdef HAVE_X86
#define TEMP_PATH "/sys/devices/platform/i2c-1/1-0048"
#else
#define TEMP_PATH "/sys/devices/platform/i2c-0/0-0048"
#endif
#define TEMP_PREFIX "temp1"
#define TEMP_MUL 1000
#endif

void start_hwmon( void )
{
    int temp_max = atoi( nvram_safe_get( "hwmon_temp_max" ) ) * TEMP_MUL;
    int temp_hyst = atoi( nvram_safe_get( "hwmon_temp_hyst" ) ) * TEMP_MUL;

    sysprintf( "/bin/echo %d > %s/%s_max", temp_max, TEMP_PATH, TEMP_PREFIX );
    sysprintf( "/bin/echo %d > %s/%s_max_hyst", temp_hyst, TEMP_PATH,
	       TEMP_PREFIX );
    dd_syslog( LOG_INFO, "hwmon successfully started\n" );
}

#endif

#ifdef HAVE_USBHOTPLUG
void start_hotplug_usb( void )
{
    // char *lan_ifname = nvram_safe_get("lan_ifname");
    char *interface = getenv( "INTERFACE" );
    char *action = getenv( "ACTION" );
    char *product = getenv( "PRODUCT" );
    char *devpath = getenv( "DEVPATH" );
    char *type = getenv( "TYPE" );
    char *devfs = getenv( "DEVFS" );
    char *device = getenv( "DEVICE" );

    fprintf( stderr, "interface %s\n", interface != NULL ? interface : "" );
    fprintf( stderr, "action %s\n", action != NULL ? action : "" );
    fprintf( stderr, "product %s\n", product != NULL ? product : "" );
    fprintf( stderr, "devpath %s\n", devpath != NULL ? devpath : "" );
    fprintf( stderr, "type %s\n", type != NULL ? type : "" );
    fprintf( stderr, "devfs %s\n", devfs != NULL ? devfs : "" );
    fprintf( stderr, "device %s\n", device != NULL ? device : "" );

    return;
}
#endif
