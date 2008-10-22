/*
 * pppoeserver.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_PPPOESERVER
#include <stdio.h>
#include <signal.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <malloc.h>
#include <sys/stat.h>
#include <syslog.h>

void add_pppoe_natrule( void )
{

    if( nvram_match( "wan_proto", "disabled" ) )
    {
	char mask[128];

	sprintf( mask, "%s/%s", nvram_safe_get( "pppoeserver_remotenet" ),
		 nvram_safe_get( "pppoeserver_remotemask" ) );
	eval( "iptables", "-A", "INPUT", "-i", nvram_get( "lan_ipaddr" ),
	      "-s", mask, "-j", "DROP" );
	eval( "iptables", "-t", "nat", "-A", "POSTROUTING", "-s", mask, "-j",
	      "SNAT", "--to-source", nvram_get( "lan_ipaddr" ) );
    }
}

void del_pppoe_natrule( void )
{
    if( nvram_match( "wan_proto", "disabled" ) )
    {
	char mask[128];

	sprintf( mask, "%s/%s", nvram_safe_get( "pppoeserver_remotenet" ),
		 nvram_safe_get( "pppoeserver_remotemask" ) );
	eval( "iptables", "-D", "INPUT", "-i", nvram_get( "lan_ipaddr" ),
	      "-s", mask, "-j", "DROP" );
	eval( "iptables", "-t", "nat", "-D", "POSTROUTING", "-s", mask, "-j",
	      "SNAT", "--to-source", nvram_get( "lan_ipaddr" ) );
    }
}

static void makeipup( void )
{
    int mss;

    if( nvram_match( "mtu_enable", "1" ) )
	mss = atoi( nvram_safe_get( "wan_mtu" ) ) - 40 - 108;
    else
	mss = 1500 - 40 - 108;

    FILE *fp = fopen( "/tmp/pppoeserver/ip-up", "w" );

    fprintf( fp, "#!/bin/sh\n" "startservice set_routes\n"	// reinitialize 
	     // routing, 
	     // just 
	     // in 
	     // case 
	     // that 
	     // a
	     // target 
	     // route 
	     // exists
	     "iptables -I FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n"
	     "iptables -I INPUT -i $1 -j ACCEPT\n"
	     "iptables -I FORWARD -i $1 -j ACCEPT\n");
    fclose( fp );
    fp = fopen( "/tmp/pppoeserver/ip-down", "w" );
    fprintf( fp, "#!/bin/sh\n"
	     "iptables -D FORWARD -i $1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n"
	     "iptables -D INPUT -i $1 -j ACCEPT\n"
	     "iptables -D FORWARD -i $1 -j ACCEPT\n");
    fclose( fp );
    chmod( "/tmp/pppoeserver/ip-up", 0744 );
    chmod( "/tmp/pppoeserver/ip-down", 0744 );

}

void start_pppoeserver( void )
{
    if( nvram_default_match( "pppoeserver_enabled", "1", "0" ) )
    {
	add_pppoe_natrule(  );
	if( nvram_default_match( "pppoeradius_enabled", "0", "0" ) )
	{
	    FILE *fp;

	    mkdir( "/tmp/pppoeserver", 0777 );
	    fp = fopen( "/tmp/pppoeserver/pppoe-server-options", "wb" );
	    // fprintf (fp, "crtscts\n");
	    if( nvram_default_match( "pppoeserver_bsdcomp", "0", "0" ) )
		fprintf( fp, "nobsdcomp\n" );
	    else
		fprintf( fp, "bsdcomp 12\n" );
	    if( nvram_default_match( "pppoeserver_deflate", "0", "0" ) )
		fprintf( fp, "nodeflate\n" );
	    else
		fprintf( fp, "deflate 12\n" );
	    if( nvram_default_match( "pppoeserver_lzs", "0", "0" ) )
		fprintf( fp, "nolzs\n" );
	    else
		fprintf( fp, "lzs\n" );
	    if( nvram_default_match( "pppoeserver_mppc", "0", "0" ) )
		fprintf( fp, "nomppc\n" );
	    else
		fprintf( fp, "mppc\n" );
	    fprintf( fp, "nopcomp\n" );
	    fprintf( fp, "idle %s\n", nvram_safe_get( "pppoeserver_idle" ) );	// todo 
	    // ...
	    if( nvram_default_match( "pppoeserver_encryption", "1", "0" ) )	// make 
		// it 
		// configureable
	    {
		fprintf( fp, "mppe required,no56,no40,stateless\n"
			 "refuse-eap\n"
			 "refuse-pap\n"
			 "refuse-chap\n"
			 "refuse-mschap\n" "require-mschap-v2\n" );
	    }
	    else
		fprintf( fp, "nomppe\n" );
	    fprintf( fp, "auth\n"
		     "default-mru\n"
		     "default-asyncmap\n"
		     "lcp-echo-interval %s\n"
		     "lcp-echo-failure %s\n",
		     nvram_safe_get( "pppoeserver_lcpechoint" ),
		     nvram_safe_get( "pppoeserver_lcpechofail" ) );
	    struct dns_lists *dns_list = get_dns_list(  );

	    if( !dns_list || dns_list->num_servers == 0 )
	    {
		if( nvram_invmatch( "lan_ipaddr", "" ) )
		    fprintf( fp, "ms-dns %s\n",
			     nvram_safe_get( "lan_ipaddr" ) );
	    }
	    else if( nvram_match( "local_dns", "1" ) )
	    {
		if( dns_list
		    && ( nvram_invmatch( "lan_ipaddr", "" )
			 || strlen( dns_list->dns_server[0] ) > 0
			 || strlen( dns_list->dns_server[1] ) > 0
			 || strlen( dns_list->dns_server[2] ) > 0 ) )
		{

		    if( nvram_invmatch( "lan_ipaddr", "" ) )
			fprintf( fp, "ms-dns %s\n",
				 nvram_safe_get( "lan_ipaddr" ) );
		    if( strlen( dns_list->dns_server[0] ) > 0 )
			fprintf( fp, "ms-dns %s\n", dns_list->dns_server[0] );
		    if( strlen( dns_list->dns_server[1] ) > 0 )
			fprintf( fp, "ms-dns %s\n", dns_list->dns_server[1] );
		    if( strlen( dns_list->dns_server[2] ) > 0 )
			fprintf( fp, "ms-dns %s\n", dns_list->dns_server[2] );
		}
	    }
	    else
	    {
		if( dns_list
		    && ( strlen( dns_list->dns_server[0] ) > 0
			 || strlen( dns_list->dns_server[1] ) > 0
			 || strlen( dns_list->dns_server[2] ) > 0 ) )
		{
		    if( strlen( dns_list->dns_server[0] ) > 0 )
			fprintf( fp, "ms-dns  %s\n",
				 dns_list->dns_server[0] );
		    if( strlen( dns_list->dns_server[1] ) > 0 )
			fprintf( fp, "ms-dns  %s\n",
				 dns_list->dns_server[1] );
		    if( strlen( dns_list->dns_server[2] ) > 0 )
			fprintf( fp, "ms-dns  %s\n",
				 dns_list->dns_server[2] );

		}
	    }

	    if( dns_list )
		free( dns_list );
	    fprintf( fp, "noipdefault\n"
		     "nodefaultroute\n"
		     "noproxyarp\n"
		     "noktune\n"
		     "netmask 255.255.255.255\n"
		     "chap-secrets /tmp/pppoeserver/chap-secrets\n"
		     "ip-up-script /tmp/pppoeserver/ip-up\n"
		     "ip-down-script /tmp/pppoeserver/ip-down\n" );
	    fclose( fp );

	    // parse chaps from nvram to file
	    static char word[256];
	    char *next, *wordlist;
	    char *user, *pass, *ip, *enable;

	    wordlist = nvram_safe_get( "pppoeserver_chaps" );

	    fp = fopen( "/tmp/pppoeserver/chap-secrets", "wb" );

	    foreach( word, wordlist, next )
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

		if( !strcmp( enable, "on" ) )
		    fprintf( fp, "%s * %s %s\n", user, pass, ip );

	    }
	    fclose( fp );
	    makeipup(  );
	    // end parsing
	    eval( "pppoe-server", "-k", "-I", "br0", "-L", nvram_safe_get( "lan_ipaddr" ), "-R", nvram_safe_get( "pppoeserver_remoteaddr" ) );	// todo, 
	    // make 
	    // interface 
	    // and 
	    // base 
	    // address 
	    // configurable, 
	    // see 
	    // networking 
	    // page 
	    // options
	}
	else
	{
	    FILE *fp;

	    mkdir( "/tmp/pppoeserver", 0777 );
	    fp = fopen( "/tmp/pppoeserver/pppoe-server-options", "wb" );
	    // fprintf (fp, "crtscts\n");
	    if( nvram_default_match( "pppoeserver_bsdcomp", "0", "0" ) )
		fprintf( fp, "nobsdcomp\n" );
	    else
		fprintf( fp, "bsdcomp 12\n" );
	    if( nvram_default_match( "pppoeserver_deflate", "0", "0" ) )
		fprintf( fp, "nodeflate\n" );
	    else
		fprintf( fp, "deflate 12\n" );
	    if( nvram_default_match( "pppoeserver_lzs", "0", "0" ) )
		fprintf( fp, "nolzs\n" );
	    else
		fprintf( fp, "lzs\n" );
	    if( nvram_default_match( "pppoeserver_mppc", "0", "0" ) )
		fprintf( fp, "nomppc\n" );
	    else
		fprintf( fp, "mppc\n" );
	    fprintf( fp, "nopcomp\n" );
	    fprintf( fp, "idle %s\n", nvram_safe_get( "pppoeserver_idle" ) );	// todo 
	    // ...
	    if( nvram_default_match( "pppoeserver_encryption", "1", "0" ) )	// make 
		// it 
		// configureable
	    {
		fprintf( fp, "mppe required,no56,no40,stateless\n"
			 "refuse-eap\n"
			 "refuse-pap\n"
			 "refuse-chap\n"
			 "refuse-mschap\n" "require-mschap-v2\n" );
	    }
	    else
		fprintf( fp, "nomppe\n" );

	    struct dns_lists *dns_list = get_dns_list(  );

	    if( !dns_list || dns_list->num_servers == 0 )
	    {
		if( nvram_invmatch( "lan_ipaddr", "" ) )
		    fprintf( fp, "ms-dns %s\n",
			     nvram_safe_get( "lan_ipaddr" ) );
	    }
	    else if( nvram_match( "local_dns", "1" ) )
	    {
		if( dns_list
		    && ( nvram_invmatch( "lan_ipaddr", "" )
			 || strlen( dns_list->dns_server[0] ) > 0
			 || strlen( dns_list->dns_server[1] ) > 0
			 || strlen( dns_list->dns_server[2] ) > 0 ) )
		{

		    if( nvram_invmatch( "lan_ipaddr", "" ) )
			fprintf( fp, "ms-dns %s\n",
				 nvram_safe_get( "lan_ipaddr" ) );
		    if( strlen( dns_list->dns_server[0] ) > 0 )
			fprintf( fp, "ms-dns %s\n", dns_list->dns_server[0] );
		    if( strlen( dns_list->dns_server[1] ) > 0 )
			fprintf( fp, "ms-dns %s\n", dns_list->dns_server[1] );
		    if( strlen( dns_list->dns_server[2] ) > 0 )
			fprintf( fp, "ms-dns %s\n", dns_list->dns_server[2] );
		}
	    }
	    else
	    {
		if( dns_list
		    && ( strlen( dns_list->dns_server[0] ) > 0
			 || strlen( dns_list->dns_server[1] ) > 0
			 || strlen( dns_list->dns_server[2] ) > 0 ) )
		{
		    if( strlen( dns_list->dns_server[0] ) > 0 )
			fprintf( fp, "ms-dns  %s\n",
				 dns_list->dns_server[0] );
		    if( strlen( dns_list->dns_server[1] ) > 0 )
			fprintf( fp, "ms-dns  %s\n",
				 dns_list->dns_server[1] );
		    if( strlen( dns_list->dns_server[2] ) > 0 )
			fprintf( fp, "ms-dns  %s\n",
				 dns_list->dns_server[2] );
		}
	    }

	    if( dns_list )
		free( dns_list );
	    fprintf( fp, "login\n" "require-mschap-v2\n" "default-mru\n" "default-asyncmap\n" "lcp-echo-interval %s\n"	// todo 
		     // optionally 
		     // configurable
		     "lcp-echo-failure %s\n"	// todo 
		     // optionally 
		     // configureable
		     "noipdefault\n"
		     "nodefaultroute\n"
		     "noproxyarp\n"
		     "noktune\n"
		     "netmask 255.255.255.255\n"
		     "plugin radius.so\n"
		     "plugin radattr.so\n"
		     "radius-config-file /tmp/pppoeserver/radius/radiusclient.conf\n"
		     "ip-up-script /tmp/pppoeserver/ip-up\n"
		     "ip-down-script /tmp/pppoeserver/ip-down\n",
		     nvram_safe_get( "pppoeserver_lcpechoint" ),
		     nvram_safe_get( "pppoeserver_lcpechofail" ) );
	    fclose( fp );
	    mkdir( "/tmp/pppoeserver/radius", 0777 );
	    fp = fopen( "/tmp/pppoeserver/radius/radiusclient.conf", "wb" );
	    fprintf( fp, "auth_order\tradius\n"
		     "login_tries\t4\n"
		     "login_timeout\t60\n"
		     "nologin\t/etc/nologin\n"
		     "issue\t/etc/issue\n"
		     "servers\t/tmp/pppoeserver/radius/servers\n"
		     "dictionary\t/etc/dictionary\n"
		     "login_radius\t/usr/local/sbin/login.radius\n"
		     "seqfile\t/var/run/radius.seq\n"
		     "mapfile\t/etc/port-id-map\n"
		     "default_realm\n"
		     "radius_timeout\t10\n"
		     "radius_retries\t3\n"
		     "login_local\t/bin/login\n"
		     "authserver %s:%s\n"
		     "acctserver %s:%s\n",
		     nvram_safe_get( "pppoeserver_authserverip" ),
		     nvram_safe_get( "pppoeserver_authserverport" ),
		     nvram_safe_get( "pppoeserver_authserverip" ),
		     nvram_safe_get( "pppoeserver_acctserverport" ) );
	    fclose( fp );
	    fp = fopen( "/tmp/pppoeserver/radius/servers", "wb" );
	    fprintf( fp, "%s %s\n", nvram_safe_get( "pppoeserver_authserverip" ), nvram_safe_get( "pppoeserver_sharedkey" ) );	// todo, 
	    // shared 
	    // secret 
	    // for 
	    // radius 
	    // server, 
	    // see 
	    // above 
	    // for 
	    // server 
	    // name, 
	    // must 
	    // be 
	    // identical
	    fclose( fp );
	    makeipup(  );
	    eval( "pppoe-server", "-k", "-I", "br0", "-L", nvram_safe_get( "lan_ipaddr" ), "-R", nvram_safe_get( "pppoeserver_remoteaddr" ) );	// todo, 
	    // make 
	    // interface 
	    // and 
	    // base 
	    // address 
	    // configurable, 
	    // remote 
	    // addr 
	    // as 
	    // well, 
	    // see 
	    // networking 
	    // page 
	    // options
	}
	dd_syslog( LOG_INFO,
		   "rp-pppoe : pppoe server successfully started\n" );
    }
}

void stop_pppoeserver( void )
{
    if( pidof( "pppoe-server" ) > 0 )
    {
	dd_syslog( LOG_INFO,
		   "rp-pppoe : pppoe server successfully stopped\n" );
	killall( "pppoe-server", SIGTERM );
	del_pppoe_natrule(  );
    }
}

#endif
