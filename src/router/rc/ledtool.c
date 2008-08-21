#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

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
    int times = atoi( argv[1] );

    while( times > 0 )
    {
	int count = 3;

	while( count > 0 )
	{
	    led_control( LED_DIAG, LED_ON );
	    usleep( 500000 );
	    led_control( LED_DIAG, LED_OFF );
	    usleep( 500000 );
	    count--;
	}
	times--;
	if( times > 0 )
	    sleep( 3 );
    }

    return 0;
}				// end main
