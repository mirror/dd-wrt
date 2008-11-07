#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>
#include <utils.h>

int autokill_wiviz_main( int argc, char **argv )
{
    pid_t pid;

    pid = fork(  );
    switch ( pid )
    {
	case -1:
	    perror( "fork failed" );
	    exit( 1 );
	    break;
	case 0:
	    sleep( 10 );
	    killall( "wiviz", SIGTERM );
	    unlink( "/tmp/wiviz2-cfg" );
	    unlink( "/tmp/wiviz2-dump" );
	    eval("wiviz","terminate");
	    exit( 0 );
	    break;
	default:
	    _exit( 0 );
	    break;
    }
}
