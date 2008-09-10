
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <broadcom.h>
#include <cyutils.h>

#define MIN_BUF_SIZE    4096

#define SERVICE_MODULE "/lib/services.so"
#define cprintf(fmt, args...)

/*
 * #define cprintf(fmt, args...) do { \ FILE *fp = fopen("/dev/console",
 * "w"); \ if (fp) { \ fprintf(fp, fmt, ## args); \ fclose(fp); \ } \ } while 
 * (0) 
 */

static int restore_ret;

static char *filter[] = { "lan_ifnames",
    "lan_ifname",
    "wan_ifnames",
    "wan_ifname",
    "et0macaddr",
    "et1macaddr",
    "il0macaddr",
    "boardnum",
    "boardtype",
    "boardrev",
    "melco_id",
    "product_name",
    "phyid_num",
    "cardbus",
    "CFEver",
    "clkfreq",
    "boardflags",
    "boardflags2",
    "sromrev",
    "sdram_config",
    "sdram_init",
    "sdram_refresh",
    "sdram_ncdl",
    "boot_wait",
    "wait_time",
    "et0phyaddr",
    "et0mdcport",
    "vlan0ports",
    "vlan1ports",
    "vlan2ports",
    "vlan0hwname",
    "vlan1hwname",
    "vlan2hwname",
    "wl_use_coregpio",
    "wl0gpio0",
    "wl0gpio1",
    "wl0gpio2",
    "wl0gpio3",
    "wl0gpio4",
    "reset_gpio",
    "af_hash",
    NULL
};
static int isCritical( char *name )
{
    int a = 0;

    while( filter[a] != NULL )
    {
	if( !strcmp( name, filter[a++] ) )
	{
	    return 1;
	}
    }
    return 0;
}

static void nvram_clear(  )
{
    char *buf = ( char * )malloc( NVRAM_SPACE );

    nvram_getall( buf, NVRAM_SPACE );
    nvram_open(  );
    char *p = buf;
    int i;

    while( strlen( p ) != 0 )
    {
	int len = strlen( p );

	for( i = 0; i < len; i++ )
	    if( p[i] == '=' )
		p[i] = 0;
	if( !isCritical( p ) )
	    nvram_immed_set( p, NULL );
	p += len + 1;
    }
    nvram_close(  );
}

void nv_file_in( char *url, webs_t wp, int len, char *boundary )
{
    char buf[1024];

    restore_ret = EINVAL;
    char sign[7];

    sign[6] = 0;
    char *nvram_ver = NULL;
#ifdef HAVE_REGISTER
    if (!isregistered_real())
	{
	return;
	}
#endif

    /*
     * Look for our part 
     */
    while( len > 0 )
    {
	if( !wfgets( buf, MIN( len + 1, sizeof( buf ) ), wp ) )
	    return;
	len -= strlen( buf );
	if( !strncasecmp( buf, "Content-Disposition:", 20 ) )
	{
	    if( strstr( buf, "name=\"file\"" ) )
	    {
		break;
	    }
	}
    }
    /*
     * Skip boundary and headers 
     */
    while( len > 0 )
    {
	if( !wfgets( buf, sizeof( buf ), wp ) )
	    return;
	len -= strlen( buf );
	if( !strcmp( buf, "\n" ) || !strcmp( buf, "\r\n" ) )
	    break;
    }
#if defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_GATEWORX) || defined(HAVE_MAGICBOX) || defined(HAVE_X86) || defined(HAVE_LS2) || defined(HAVE_MERAKI) || defined(HAVE_CA8) || defined(HAVE_TW6600)  || defined(HAVE_LS5)
    eval( "rm", "-f", "/tmp/nvram/*" );	// delete nvram database
    eval( "rm", "-f", "/tmp/nvram/.lock" );	// delete nvram database
#endif
    // fprintf (stderr, "file write");
    unsigned short count;

    wfread( sign, 6, 1, wp );
    len -= 6;
    if( !strcmp( sign, "DD-WRT" ) )
    {
	nvram_clear(  );
	nvram_open(  );
	unsigned char b;

	wfread( ( char * )&b, 1, 1, wp );
	count = b;
	wfread( ( char * )&b, 1, 1, wp );
	count += ( ( unsigned int )b << 8 );
	len -= 2;
	int i;

	for( i = 0; i < count && len > 0; i++ )
	{
	    unsigned short l = 0;
	    unsigned char c = 0;

	    wfread( ( char * )&c, 1, 1, wp );
	    char *name = ( char * )malloc( c + 1 );

	    wfread( name, c, 1, wp );
	    name[c] = 0;
	    len -= ( c + 1 );

	    wfread( ( char * )&b, 1, 1, wp );
	    l = b;
	    wfread( ( char * )&b, 1, 1, wp );
	    l += ( ( unsigned int )b << 8 );

	    char *value = ( char * )malloc( l + 1 );

	    wfread( value, l, 1, wp );
	    len -= ( l + 2 );
	    value[l] = 0;
	    // cprintf("setting %s to %s\n",name,value);
	    if( !strcmp( name, "nvram_ver" ) )
		nvram_ver = value;

	    if( !isCritical( name ) )
	    {
		nvram_immed_set( name, value );
	    }
	    free( value );
	    free( name );
	}
	restore_ret = 0;
	nvram_close(  );
    }
    else
    {
	restore_ret = 99;
    }
    /*
     * Slurp anything remaining in the request 
     */
    while( len-- )
    {
#ifdef HAVE_HTTPS
	if( do_ssl )
	{
	    wfgets( buf, 1, wp );
	}
	else
#endif
	    ( void )fgetc( wp );
    }
    if( nvram_ver == NULL )
    {
	nvram_set( "http_passwd",
		   zencrypt( nvram_safe_get( "http_passwd" ) ) );
	nvram_set( "http_username",
		   zencrypt( nvram_safe_get( "http_username" ) ) );
	if( nvram_get( "newhttp_passwd" ) != NULL )
	{
	    nvram_set( "newhttp_passwd",
		       zencrypt( nvram_safe_get( "newhttp_passwd" ) ) );
	    nvram_set( "newhttp_username",
		       zencrypt( nvram_safe_get( "newhttp_username" ) ) );
	}
    }
    nvram_commit(  );

    chdir( "/www" );
}

void sr_config_cgi( char *path, webs_t wp, char *query )
{
    if( restore_ret != 0 )
	do_ej( "Fail.asp", wp, NULL );
    else
	do_ej( "Success_rest.asp", wp, NULL );

    websDone( wp, 200 );

    /*
     * Reboot if successful 
     */
    if( restore_ret == 0 )
    {
	nvram_commit(  );
	sleep( 5 );
	sys_reboot(  );
    }
}

void nv_file_out( char *path, webs_t wp, char *query )
{

    int backupcount = 0;
    char sign[7] = { "DD-WRT" };

#ifdef HAVE_REGISTER
    if (!isregistered_real())
	{
	return;
	}
#endif
    char *buf = ( char * )malloc( NVRAM_SPACE );

    nvram_getall( buf, NVRAM_SPACE );
    char *p = buf;
    int i;

    for( i = 0; i < NVRAM_SPACE; i++ )
    {
	if( i > 0 && buf[i] == 0 && buf[i - 1] == 0 )
	{
	    break;
	}
	if( buf[i] == 0 )
	    backupcount++;
    }
    wfwrite( sign, 6, 1, wp );
    wfputc( backupcount & 255, wp );	// high byte
    wfputc( backupcount >> 8, wp );	// low byte
    while( strlen( p ) != 0 )
    {
	int len = strlen( p );

	for( i = 0; i < len; i++ )
	    if( p[i] == '=' )
		p[i] = 0;
	char *name = p;

	wfputc( strlen( name ), wp );

	for( i = 0; i < strlen( name ); i++ )
	    wfputc( name[i], wp );
	char *val = nvram_safe_get( name );

	wfputc( strlen( val ) & 255, wp );
	wfputc( strlen( val ) >> 8, wp );
	for( i = 0; i < strlen( val ); i++ )
	    wfputc( val[i], wp );

	p += len + 1;
    }
    free( buf );

    return;
}
