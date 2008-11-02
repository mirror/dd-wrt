#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <services.h>

int setuserpasswd_main( int argc, char **argv )
{

    if( !argv[1] || !argv[2] )
    {
	fprintf( stderr, "usage: setuserpasswd [username] [password]\n" );
	return EINVAL;
    }
    nvram_set( "http_username", zencrypt( argv[1] ) );
    nvram_set( "http_passwd", zencrypt( argv[2] ) );
    nvram_commit(  );
    start_mkfiles(  );
    return 0;
}
