
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <rc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define uint32 unsigned long
#define uint unsigned int
#define uint8 unsigned char
#define uint64 unsigned long long
#include <swmod.h>
#include <swapi.h>
#include <nvports.h>
#include <bcmdevs.h>

void setup_switch( void )
{
    int portid, max_port = BCM_NUM_PORTS;
    PORT_ATTRIBS port_attribs;

    return;

    if( ( bcm_api_init(  ) ) < 0 )
    {
	if( bcm_is_robo(  ) )
	{
	    fprintf( stderr, "No Robo device found\n" );
	}
	return;
    }
    printf( "Initializing Ethernet switch "
	    "controller (%d ports)\n", max_port );
    for( portid = 1; portid <= max_port; portid++ )
    {
	/* 
	 * get nvram attributes associated with port and set switch regs 
	 */
	port_attribs = nvGetSwitchPortAttribs( portid );
	bcm_set_port_attributes( &port_attribs, portid );
    }
    bcm_api_deinit(  );

}
