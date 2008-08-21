#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>

#include <cymac.h>

#define GET_MAC	0x11
#define SET_MAC	0x12

#define GET_EOU	0x13
#define SET_EOU	0x14

#define GET_SN	0x15
#define SET_SN	0x16

#define GET_FLASH_TYPE	0x17

#define FULL	-1
#define ILLEGAL	-2
#define	ERR	-3

#define DEV_MISC "/dev/ctmisc"

typedef union
{
    struct
    {
	int index;
	unsigned char maclist[RESERVE_MAC][PER_MAC_LEN];
    } mac;
    struct
    {
	int index;
	unsigned char eoulist[RESERVE_EOU_KEY][PER_EOU_KEY_LEN];
    } eou;
    struct
    {
	int index;
	unsigned char snlist[RESERVE_SN][PER_SN_LEN];
    } sn;
} MYDATA;

struct table
{
    char *name;
    int cmd;
    char *desc;
    int count;
    int len;
    int ( *get ) ( struct table * v, int write_to_nv );
    int ( *set ) ( char *string, int len, struct table * v );
};

MYDATA mydatas;

int get_data( struct table *v, int write_to_nv )
{
    int fp;
    int ret;
    int i = 0;
    char *ptr = ( char * )&mydatas;
    char buf[1024];
    int index;

    cprintf( "%s(): cmd=0x%02x count=%d len=%d\n", __FUNCTION__, v->cmd,
	     v->count, v->len );

    if( ( fp = open( DEV_MISC, O_RDWR, 0 ) ) )
    {
	if( ( ret = ioctl( fp, v->cmd, &mydatas ) ) < 0 )
	{
	    perror( "ioctl" );
	    return ret;
	}
	close( fp );
    }

    bzero( buf, sizeof( buf ) );

    ret = index = ( int )*ptr;

    cprintf( "%s(): Get %s count is [%d]\n", __FUNCTION__, v->desc, index );

    ptr = ptr + sizeof( int );	// skip index
    for( i = 0; i < index; i++ )
    {
	memcpy( buf, ptr, v->len );
	cprintf( "%s(): %s %d: [%s]\n", __FUNCTION__, v->desc, i, buf );
	ptr = ptr + v->len;
    }

    /*
     * Save to nvram 
     */
    if( write_to_nv == 1 || write_to_nv == 3 )
	nvram_set( v->name, buf );

    if( write_to_nv == 2 || write_to_nv == 3 )
    {
	char tmp1[5];
	char tmp2[20];

	snprintf( tmp1, sizeof( tmp1 ), "%d", ret );
	snprintf( tmp2, sizeof( tmp2 ), "%s_index", v->name );
	nvram_set( tmp2, tmp1 );
    }

    cprintf( "%s(): done\n", __FUNCTION__ );

    return ret;
}

int set_data( char *string, int len, struct table *v )
{
    int fp;
    int ret;

    cprintf( "%s(): string=[%s] len=[%d]\n", __FUNCTION__, string, len );

    if( ( fp = open( DEV_MISC, O_RDWR, 0 ) ) )
    {
	if( ( ret = ioctl( fp, v->cmd, string ) ) < 0 )
	{
	    perror( "ioctl" );
	    return ret;
	}
	close( fp );
    }

    ret = ( int )*string;

    cprintf( "%s(): ret=%d\n", __FUNCTION__, ret );

    switch ( ret )
    {
	case FULL:
	    cprintf( "%s(): The %s space is full\n", __FUNCTION__, v->desc );
	    break;
	case ILLEGAL:
	    cprintf( "%s(): Illegal %s value\n", __FUNCTION__, v->desc );
	    break;
	case ERR:
	    cprintf( "%s(): Cann't write %s to flash\n", __FUNCTION__,
		     v->desc );
	default:
	    if( ret >= 0 && ret < v->count )
		cprintf( "%s(): Save %s to location %d\n", __FUNCTION__,
			 v->desc, ret );
	    else
		cprintf( "%s(): Something error, return code is %d\n",
			 __FUNCTION__, ret );

	    break;
    }

    cprintf( "%s(): done\n", __FUNCTION__ );

    return ret;
}

int get_flash_type( struct table *v, int write_to_nv )
{
    int fp;
    int ret;
    char buf[1024];

    cprintf( "%s(): cmd=0x%02x count=%d len=%d\n", __FUNCTION__, v->cmd,
	     v->count, v->len );

    memset( &buf, 0, sizeof( buf ) );

    if( ( fp = open( DEV_MISC, O_RDWR, 0 ) ) )
    {
	if( ( ret = ioctl( fp, v->cmd, &buf ) ) < 0 )
	{
	    perror( "ioctl" );
	    return ret;
	}
	close( fp );
    }

    cprintf( "Get %s is [%s]\n", v->desc, buf );

    if( write_to_nv == 1 )
	nvram_set( "flash_type", buf );
}

struct table tables[] = {
    {"get_mac", GET_MAC, "MAC", RESERVE_MAC, PER_MAC_LEN, get_data, NULL},
    {"set_mac", SET_MAC, "MAC", RESERVE_MAC, PER_MAC_LEN, NULL, set_data},
    {"get_eou", GET_EOU, "EOU", RESERVE_EOU_KEY, PER_EOU_KEY_LEN, get_data,
     NULL},
    {"set_eou", SET_EOU, "EOU", RESERVE_EOU_KEY, PER_EOU_KEY_LEN, NULL,
     set_data},
    {"get_sn", GET_SN, "SN", RESERVE_SN, PER_SN_LEN, get_data, NULL},
    {"set_sn", SET_SN, "SN", RESERVE_SN, PER_SN_LEN, NULL, set_data},
    {"get_flash_type", GET_FLASH_TYPE, "FLASH TYPE", 0, 0, get_flash_type,
     NULL},
};

static void usage( char *cmd )
{
    cprintf( "Usage: %s [OPTIONS]\n"
	     "\t-t\n"
	     "\t\tWhich action do you want to do ? [get_mac|set_mac|get_eou|set_eou|get_sn|set_sn|get_flash_type]\n"
	     "\t-s\n"
	     "\t\tWrite the string to assgined address\n"
	     "\t-h, --help\n"
	     "\t\tDisplay the help\n"
	     "\n\tExample:\n"
	     "\n\t%s -t get_mac\n"
	     "\t%s -t set_mac -s 00:11:22:33:44:55\n", cmd, cmd, cmd );
}

int misc_main( int argc, char **argv )
{
    char *type = NULL;
    char *string = NULL;
    int ret = 0;
    struct table *v = NULL;
    int c;
    int i = 0;
    int write_to_nv = 0;
    int match = 0;

    while( ( c = getopt( argc, argv, "t:hs:w:" ) ) != -1 )
	switch ( c )
	{
	    case 'h':
		usage( argv[0] );
		exit( 0 );
		break;
	    case 't':
		type = optarg;

		cprintf( "type = [%s]\n", type );
		break;
	    case 's':
		string = optarg;

		cprintf( "string = [%s]\n", string );
	    case 'w':
		write_to_nv = atoi( optarg );
		break;

	    default:
		break;
	}

    if( !type )
    {
	usage( argv[0] );
	exit( 0 );
    }

    memset( &mydatas, 0, sizeof( mydatas ) );

    for( v = tables; v < &tables[STRUCT_LEN( tables )]; v++ )
    {
	if( !strcmp( v->name, type ) )
	{
	    if( v->get )
		ret = v->get( v, write_to_nv );
	    if( v->set )
	    {
		if( !string )
		{
		    usage( argv[0] );
		    exit( 0 );
		}

		ret = v->set( string, strlen( string ), v );
	    }
	    match = 1;
	}
    }

    if( !match )
	cprintf( "No support type [%s]\n", type );

    return ret;
}
