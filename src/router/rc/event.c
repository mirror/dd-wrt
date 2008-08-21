#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <shutils.h>

int main( int argc, char **argv )
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
	    sleep( atoi( argv[1] ) );
	    kill( atoi( argv[2] ), atoi( argv[3] ) );
	    exit( 0 );
	    break;
	default:
	    _exit( 0 );
	    break;
    }
}
