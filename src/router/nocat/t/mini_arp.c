# include <glib.h>
# include <stdio.h>
# include "firewall.h"

int main ( int argc, char **argv ) {
    peer *p;
    gchar *hw;

    if (argc != 2) {
	fprintf( stderr, "Usage: %s <ip>", argv[0] );
	exit(-1);
    }

    p  = peer_new( argv[1] );
    hw = peer_arp( p );
    if ( hw )
	printf( "%s\n", hw );
    else
	printf( "Can't find IP %s in ARP table.\n", argv[1] );
}
