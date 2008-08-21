
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <rc.h>
#include <bcmnvram.h>

int start_shorewall( void )
{

    if( !nvram_invmatch( "shorewall_enable", "0" ) )
	return 0;

    stop_firewall(  );
    stop_shorewall(  );

    mkdir( "/var/shorewall", 0700 );

    nvram2file( "sh_interfaces", "/var/shorewall/interfaces" );
    nvram2file( "sh_masq", "/var/shorewall/masq" );
    nvram2file( "sh_policy", "/var/shorewall/policy" );
    nvram2file( "sh_routestopped", "/var/shorewall/route_stopped" );
    nvram2file( "sh_rules", "/var/shorewall/rules" );
    nvram2file( "sh_zones", "/var/shorewall/zones" );
    symlink( "/usr/sbin/shorewall", "/var/shorewall/shorewall" );
    symlink( "/usr/sbin/shorewall.conf", "/var/shorewall/shorewall.conf" );
    symlink( "/usr/sbin/firewall", "/var/shorewall/firewall" );
    symlink( "/usr/sbin/functions", "/var/shorewall/functions" );
    symlink( "/usr/sbin/common.def", "/var/shorewall/common.def" );
    symlink( "/usr/sbin/version", "/var/shorewall/version" );

    system( "/var/shorewall/shorewall start" );

    return 0;
}

int stop_shorewall( void )
{
    system( "/var/shorewall/shorewall clear" );
    system( "rm -Rf /var/shorewall" );

    return 0;
}
