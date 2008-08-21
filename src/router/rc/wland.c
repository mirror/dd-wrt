#undef HAVE_DDLAN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <rc.h>

#include <cy_conf.h>
#include <bcmutils.h>
#include <utils.h>
#include <nvparse.h>

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 70

#ifdef HAVE_MADWIFI
#define WLAND_INTERVAL 60
#else
#define WLAND_INTERVAL 15
#endif

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

static char *get_wshaper_dev( void )
{
    if( nvram_match( "wshaper_dev", "WAN" ) )
	return get_wan_face(  );
    else
	return "br0";
}

static int do_ap_watchdog( void )
{

    /* 
     * AP Watchdog - experimental check and fix for hung AP 
     */
    int val = 0;
    struct stat s;
    static time_t last;
    int interval =
	atoi( nvram_safe_get( "apwatchdog_interval" ) ) >
	WLAND_INTERVAL ? atoi( nvram_safe_get( "apwatchdog_interval" ) ) :
	WLAND_INTERVAL;

    system2( "/usr/sbin/wl assoclist 2>&1 > /tmp/.assoclist" );
    stat( "/tmp/.assoclist", &s );
    unlink( "/tmp/.assoclist" );

    if( s.st_size <= 0 &&
	time( NULL ) - last > interval &&
	nvram_match( "apwatchdog_enable", "1" ) &&
	nvram_invmatch( "wl_net_mode", "disabled" ) )
    {
	time( &last );
	cprintf( "resetting ap radio\n" );
	eval( "/usr/sbin/wlconf", get_wdev(  ), "down" );

	val = atoi( nvram_safe_get( "wl0_channel" ) ) + 1;
	if( val <= 2 || val >= 14 )
	    val = 2;

	wl_ioctl( get_wdev(  ), WLC_SET_CHANNEL, &val, sizeof( val ) );
	wl_ioctl( get_wdev(  ), WLC_UP, NULL, 0 );

	eval( "/usr/sbin/wlconf", get_wdev(  ), "down" );
	eval( "startservice", "wlconf" );
	// wlconf_up (get_wdev ());

    }

    return 0;

}

#ifdef HAVE_AQOS
int compareNet( char *ip, char *net, char *dest )
{
    if( ip == NULL || net == NULL || dest == NULL )
	return 0;
    // fprintf(stderr,"compare ip%s/net%s with %s\n",ip,net,dest);
    char ips2[32];
    char dest2[32];

    strcpy( ips2, ip );
    strcpy( dest2, dest );
    ip = ips2;
    dest = dest2;
    unsigned int ip1 = atoi( strsep( &ip, "." ) );
    unsigned int ip2 = atoi( strsep( &ip, "." ) );
    unsigned int ip3 = atoi( strsep( &ip, "." ) );
    unsigned int ip4 = atoi( ip );

    unsigned int dip1 = atoi( strsep( &dest, "." ) );
    unsigned int dip2 = atoi( strsep( &dest, "." ) );
    unsigned int dip3 = atoi( strsep( &dest, "." ) );
    unsigned int dip4 = atoi( dest );

    unsigned int fullip = ( ip1 << 24 ) | ( ip2 << 16 ) | ( ip3 << 8 ) | ip4;
    unsigned int dfullip =
	( dip1 << 24 ) | ( dip2 << 16 ) | ( dip3 << 8 ) | dip4;
    int bit = atoi( net );
    unsigned long long n = ( unsigned long long )1 << ( unsigned long long )bit;	// convert 

    // 
    // 
    // 
    // net 
    // to 
    // full 
    // mask
    int shift = 32 - bit;

    n--;
    n <<= shift;
    /* 
     * fprintf(stderr, "compare %08X with %08X\n",(unsigned
     * int)(dfullip&(unsigned int)n),(unsigned int)fullip&(unsigned int)n);
     * fprintf(stderr, "fullip %08X\n",fullip); fprintf(stderr, "dfullip
     * %08X\n",dfullip); fprintf(stderr, "n %08X\n",(unsigned int)n);
     * fprintf(stderr, "nl %08lX\n",n); 
     */
    if( ( unsigned int )( dfullip & ( unsigned int )n ) ==
	( unsigned int )( fullip & ( unsigned int )n ) )
	return 1;
    return 0;
}

int containsIP( char *ip )
{
    FILE *in;
    char buf_ip[32];
    char *i, *net;
    char cip[32];

    strcpy( cip, ip );
    in = fopen( "/tmp/aqos_ips", "rb" );
    if( in == NULL )
	return 0;

    while( feof( in ) == 0 && fscanf( in, "%s", buf_ip ) == 1 )
    {
	i = ( char * )&buf_ip[0];
	net = strsep( &i, "/" );
	if( compareNet( net, i, cip ) )
	{
	    fclose( in );
	    return 1;
	}
	memset( buf_ip, 0, 32 );
    }
    fclose( in );
    return 0;
}
static int qosidx = 1000;

int containsMAC( char *ip )
{
    FILE *in;
    char buf_ip[32];

    in = fopen( "/tmp/aqos_macs", "rb" );
    if( in == NULL )
	return 0;
    while( feof( in ) == 0 && fscanf( in, "%s", buf_ip ) == 1 )
    {
	if( !strcmp( buf_ip, ip ) )
	{
	    fclose( in );
	    return 1;
	}
    }
    fclose( in );
    return 0;
}

static void do_aqos_check( void )
{
    if( !nvram_invmatch( "wshaper_enable", "0" ) )
	return;
    if( nvram_match( "qos_done", "0" ) )
	return;

    FILE *arp = fopen( "/proc/net/arp", "rb" );
    char ip_buf[32];
    char hw_buf[16];
    char cmd[1024];
    char fl_buf[16];
    char mac_buf[32];
    char mask_buf[16];
    char dev_buf[16];
    char *wdev = get_wshaper_dev(  );
    int cmac;
    int defaultlevel;
    char *defaulup;
    char *defauldown;
    int cip;

    if( arp == NULL )
    {
	cprintf( "/proc/net/arp missing, check kernel config\n" );
	return;
    }
    defaulup = nvram_safe_get( "default_uplevel" );
    defauldown = nvram_safe_get( "default_downlevel" );
    if( defaulup == NULL || strlen( defaulup ) == 0 )
    {
	fclose( arp );
	return;
    }
    if( defauldown == NULL || strlen( defauldown ) == 0 )
    {
	fclose( arp );
	return;
    }
    while( fgetc( arp ) != '\n' );

    while( !feof( arp ) && fscanf
	   ( arp, "%s %s %s %s %s %s", ip_buf, hw_buf, fl_buf, mac_buf,
	     mask_buf, dev_buf ) == 6 )
    {
	char *wan = get_wan_face(  );

	if( wan && strlen( wan ) > 0 && !strcmp( dev_buf, wan ) )
	    continue;

	cmac = containsMAC( mac_buf );
	cip = containsIP( ip_buf );

	if( cip || cmac )
	{
	    continue;
	}

	if( !cip && strlen( ip_buf ) > 0 )
	{
	    char ipnet[32];

	    sprintf( ipnet, "%s/32", ip_buf );
	    sysprintf( "echo \"%s\" >>/tmp/aqos_ips", ipnet );
	    if( strlen( mac_buf ) )
		sysprintf( "echo \"%s\" >>/tmp/aqos_macs", mac_buf );
	    // create default rule for ip
	    add_userip( ipnet, qosidx, defaulup, defauldown );
	    qosidx += 2;
	    memset( ip_buf, 0, 32 );
	    memset( mac_buf, 0, 32 );
	    continue;
	}
	if( !cmac && strlen( mac_buf ) > 0 )
	{

	    sysprintf( "echo \"%s\" >>/tmp/aqos_macs", mac_buf );
	    if( strlen( ip_buf ) )
		sysprintf( "echo \"%s\" >>/tmp/aqos_macs", ip_buf );
	    // create default rule for mac
	    add_usermac( mac_buf, qosidx, defaulup, defauldown );
	    qosidx += 2;
	}
	memset( ip_buf, 0, 32 );
	memset( mac_buf, 0, 32 );
    }
    fclose( arp );

}
#endif
#ifndef HAVE_MADWIFI

void start_wds_check( void )
{
    int s, sock;

    if( ( sock = socket( AF_INET, SOCK_RAW, IPPROTO_RAW ) ) < 0 )
	return;

    /* 
     * logic - if separate ip defined bring it up 
     */
    /* 
     * else if flagged for br1 and br1 is enabled add to br1 
     */
    /* 
     * else add it to the br0 bridge 
     */
    int cnt = get_wl_instances(  );
    int c;

    for( c = 0; c < cnt; c++ )
    {
	for( s = 1; s <= MAX_WDS_DEVS; s++ )
	{
	    char *dev;
	    struct ifreq ifr;

	    dev = nvram_nget( "wl%d_wds%d_if", c, s );

	    if( nvram_nmatch( "0", "wl%d_wds%d_enable", c, s ) )
		continue;

	    memset( &ifr, 0, sizeof( struct ifreq ) );

	    snprintf( ifr.ifr_name, IFNAMSIZ, dev );
	    ioctl( sock, SIOCGIFFLAGS, &ifr );

	    if( ( ifr.ifr_flags & ( IFF_RUNNING | IFF_UP ) ) ==
		( IFF_RUNNING | IFF_UP ) )
		continue;

	    /* 
	     * P2P WDS type 
	     */
	    if( nvram_nmatch( "1", "wl%d_wds%d_enable", c, s ) )	// wds_s 
		// 
		// 
		// 
	    {
		char wdsbc[32] = { 0 };
		char *wdsip = nvram_nget( "wl%d_wds%d_ipaddr", c, s );
		char *wdsnm = nvram_nget( "wl%d_wds%d_netmask", c, s );

		snprintf( wdsbc, 31, "%s", wdsip );
		get_broadcast( wdsbc, wdsnm );
		eval( "ifconfig", dev, wdsip, "broadcast",
		      wdsbc, "netmask", wdsnm, "up" );
	    }
	    /* 
	     * Subnet WDS type 
	     */
	    else if( nvram_nmatch( "2", "wl%d_wds%d_enable", c, s )
		     && nvram_nmatch( "1", "wl%d_br1_enable", c ) )
	    {
		eval( "ifconfig", dev, "up" );
		eval( "brctl", "addif", "br1", dev );
	    }
	    /* 
	     * LAN WDS type 
	     */
	    else if( nvram_nmatch( "3", "wl%d_wds%d_enable", c, s ) )	// wds_s 
		// 
		// 
		// 
		// 
		// 
		// 
		// 
		// disabled
	    {
		eval( "ifconfig", dev, "up" );
		eval( "brctl", "addif", "br0", dev );

		// notify_nas ("lan", "br0", "up");
	    }

	}
    }
    close( sock );
    if( nvram_match( "lan_stp", "0" ) )
    {

	eval( "brctl", "stp", "br0", "0" );

    }
    else
    {
	eval( "brctl", "stp", "br0", "1" );

    }

    return;
}

static void do_ap_check( void )
{

    // if (nvram_match ("apwatchdog_enable", "1"))
    // do_ap_watchdog ();
    start_wds_check(  );
    // do_wds_check ();

    return;
}

int checkbssid( void )
{
    struct ether_addr bssid;
    wl_bss_info_t *bi;
    char buf[WLC_IOCTL_MAXLEN];
    char *ifname = getSTA(  );

    if( ifname == NULL )
	ifname = getWET(  );
    if( ifname == NULL )
	return 0;
    if( ( WL_IOCTL( ifname, WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN ) ) == 0 )
    {
	*( uint32 * ) buf = WLC_IOCTL_MAXLEN;
	if( ( WL_IOCTL( ifname, WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN ) ) <
	    0 )
	    return 0;
	bi = ( wl_bss_info_t * ) ( buf + 4 );
	int i;

	for( i = 0; i < 6; i++ )
	    if( bi->BSSID.octet[i] != 0 )
		return 1;
    }
    return 0;
}

/* 
 * for Client/Wet mode 
 */
/* 
 * be nice to rewrite this to use sta_info_t if we had proper Broadcom API
 * specs 
 */
static void do_client_check( void )
{
    FILE *fp = NULL;
    char buf[1024];

    // char mac[512];
    int len;

    system2( "/usr/sbin/wl assoc 2>&1 > /tmp/.xassocx" );
    if( ( fp = fopen( "/tmp/.xassocx", "r" ) ) == NULL )
	return;

    len = fread( buf, 1, 1023, fp );

    buf[len] = 0;

    if( ( len > 0 && strstr( buf, "Not associated." ) )
	|| checkbssid(  ) == 0 )
    {
#ifdef HAVE_DDLAN

	nvram_unset( "cur_rssi" );
	nvram_unset( "cur_noise" );
	nvram_unset( "cur_bssid" );
	nvram_unset( "cur_snr" );
	nvram_set( "cur_state",
		   "<span style=\"background-color: rgb(255, 0, 0);\">Nicht Verbunden</span>" );

#endif
	eval( "wl", "disassoc" );
#ifndef HAVE_MSSID
	eval( "wl", "join", nvram_safe_get( "wl_ssid" ) );
#else
	if( nvram_match( "roaming_enable", "1" ) )
	{
	    eval( "wl", "join", nvram_safe_get( "roaming_ssid" ) );
	}
	else
	{
	    eval( "wl", "join", nvram_safe_get( "wl0_ssid" ) );
	}
#endif
	// join(nvram_get("wl_ssid"));
	fclose( fp );
    }
    else
    {
#ifdef HAVE_DDLAN
	nvram_set( "cur_state",
		   "<span style=\"background-color: rgb(135, 255, 51);\">Verbunden</span>" );
	eval( "/sbin/check.sh" );
#endif
    }
    fclose( fp );
    return;
}
#endif

#ifdef HAVE_MADWIFI

static int notstarted[32];
static char assoclist[24 * 1024];
static int lastchans[256];
static void do_madwifi_check( void )
{
    // fprintf(stderr,"do wlan check\n");
    int c = getdevicecount(  );
    char dev[32];
    int i, s;

    for( i = 0; i < c; i++ )
    {
	sprintf( dev, "ath%d", i );
	for( s = 1; s <= 10; s++ )
	{
	    char wdsvarname[32] = { 0 };
	    char wdsdevname[32] = { 0 };
	    char wdsmacname[32] = { 0 };
	    char *wdsdev;
	    char *hwaddr;

	    sprintf( wdsvarname, "%s_wds%d_enable", dev, s );
	    sprintf( wdsdevname, "%s_wds%d_if", dev, s );
	    sprintf( wdsmacname, "%s_wds%d_hwaddr", dev, s );
	    wdsdev = nvram_safe_get( wdsdevname );
	    if( strlen( wdsdev ) == 0 )
		continue;
	    if( nvram_match( wdsvarname, "0" ) )
		continue;
	    hwaddr = nvram_get( wdsmacname );
	    if( hwaddr != NULL )
	    {
		int count = getassoclist( wdsdev, &assoclist[0] );

		if( count < 1 )
		{
		    eval( "/sbin/ifconfig", wdsdev, "down" );
		    sleep( 1 );
		    eval( "/sbin/ifconfig", wdsdev, "up" );
		    eval( "startservice", "set_routes" );
		}
	    }
	}
	char mode[32];

	sprintf( mode, "%s_mode", dev );
	if( nvram_match( mode, "sta" ) || nvram_match( mode, "wdssta" )
	    || nvram_match( mode, "wet" ) )
	{
	    int chan = wifi_getchannel( dev );

	    // fprintf(stderr,"current channel %d\n",chan);
	    if( lastchans[i] == -1 && chan < 1000 )
		lastchans[i] = chan;
	    else
	    {
		// fprintf(stderr,"current channel %d =
		// %d\n",chan,lastchans[i]);
		if( chan == lastchans[i] )
		{
		    int count = getassoclist( dev, &assoclist[0] );

		    if( count == 0 || count == -1 )
		    {
			// fprintf(stderr,"get assoclist returns %d, restart
			// ifnames\n",count);
			char *next;
			char var[80];
			char *vifs;
			char mode[32];
			char *m;
			char wifivifs[32];

			sprintf( wifivifs, "%s_vifs", dev );
			vifs = nvram_safe_get( wifivifs );
			if( vifs != NULL && strlen( vifs ) > 0 )
			{
			    foreach( var, vifs, next )
			    {
				eval( "/sbin/ifconfig", var, "down" );
			    }
			}

			notstarted[i] = 0;
			eval( "/sbin/ifconfig", dev, "down" );
			sleep( 1 );
			eval( "/sbin/ifconfig", dev, "up" );
			eval( "startservice", "set_routes" );
			lastchans[i] = -1;
		    }
		    else if( !notstarted[i] )
		    {
			notstarted[i] = 1;
			char *next;
			char var[80];
			char *vifs;
			char mode[32];
			char *m;
			char wifivifs[32];

			sprintf( wifivifs, "%s_vifs", dev );
			vifs = nvram_safe_get( wifivifs );
			if( vifs != NULL && strlen( vifs ) > 0 )
			{
			    foreach( var, vifs, next )
			    {
				eval( "/sbin/ifconfig", var, "up" );
				eval( "startservice", "set_routes" );
			    }
			}

		    }

		}
		lastchans[i] = chan;

	    }
	}

    }
    // fprintf(stderr,"do wlancheck end\n");
}
#endif

#ifdef HAVE_MADWIFI
/* 
 * static HAL_MIB_STATS laststats[16]; void detectACK(void) { int count =
 * getdevicecount(); int i; int s = socket(AF_INET, SOCK_DGRAM, 0); for
 * (i=0;i<count;i++) { char wifi[16]; sprintf(wifi,"wifi%d",i); struct ifreq
 * ifr; strcpy(ifr.ifr_name, wifi); ifr.ifr_data = (caddr_t) &laststats[i];
 * if (ioctl(s, SIOCGATHMIBSTATS, &ifr) < 0) { fprintf(stderr,"Error while
 * gettting mib stats\n"); return; } }
 * 
 * close(s); } 
 */
#endif

#ifndef HAVE_ACK
#ifndef HAVE_MADWIFI
#ifndef HAVE_MSSID
static void setACK( void )
{
    char *v;
    char *name = get_wdev(  );

    if( ( v = nvram_get( "wl0_distance" ) ) )
    {

	rw_reg_t reg;
	uint32 shm;

	int val = atoi( v );

	if( val != 0 )
	{
	    val = 9 + ( val / 150 ) + ( ( val % 150 ) ? 1 : 0 );

	    shm = 0x10;
	    shm |= ( val << 16 );
	    WL_IOCTL( name, 197, &shm, sizeof( shm ) );

	    reg.byteoff = 0x684;
	    reg.val = val + 510;
	    reg.size = 2;
	    WL_IOCTL( name, 102, &reg, sizeof( reg ) );
	}
    }

}
#endif
#endif
#endif

/* 
 * static void setShortSlot(void) { char *shortslot = nvram_safe_get
 * ("wl0_shortslot");
 * 
 * else if (!strcmp (afterburner, "long")) eval ("wl", "shortslot_override",
 * "0"); else if (!strcmp (afterburner, "short")) eval ("wl",
 * "shortslot_override", "1");
 * 
 * }
 */

static void do_wlan_check( void )
{
#ifdef HAVE_AQOS
    do_aqos_check(  );
#endif
#ifndef HAVE_MADWIFI
    if( nvram_match( "wl0_mode", "sta" ) || nvram_match( "wl0_mode", "wet" )
	|| nvram_match( "wl0_mode", "apsta" )
	|| nvram_match( "wl0_mode", "apstawet" ) )
	do_client_check(  );
    else
	do_ap_check(  );
#else

    do_madwifi_check(  );
#endif
#ifndef HAVE_ACK
#ifndef HAVE_MADWIFI
#ifndef HAVE_MSSID
    setACK(  );
#endif
#endif
#endif

}

int main( int argc, char **argv )
{
    /* 
     * Run it in the background 
     */
    switch ( fork(  ) )
    {
	case -1:
	    // can't fork
	    exit( 0 );
	    break;
	case 0:
	    /* 
	     * child process 
	     */
	    // fork ok
	    ( void )setsid(  );
	    break;
	default:
	    /* 
	     * parent process should just die 
	     */
	    _exit( 0 );
    }
#ifdef HAVE_AQOS
    qosidx = 1000;
#endif
    /* 
     * Most of time it goes to sleep 
     */
#ifdef HAVE_MADWIFI
    memset( lastchans, -1, 256 * 4 );
    memset( notstarted, 0, 32 * 4 );
#endif
    while( 1 )
    {
	do_wlan_check(  );

	sleep( WLAND_INTERVAL );
    }

    return 0;
}				// end main

/* 
 * void main(int argc, char **argv) { wland_main(argc,argv); } 
 */
