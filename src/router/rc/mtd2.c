
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <sys/mount.h>

#include <linux/mtd/mtd.h>

#include <trxhdr.h>
#include <rts/crc.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <shutils.h>

#include <cy_conf.h>
#include <utils.h>

/*
 * Open an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @param       flags   open() flags
 * @return      return value of open()
 */
int mtd_open( const char *mtd, int flags )
{
    FILE *fp;
    char dev[PATH_MAX];
    int i;

    if( ( fp = fopen( "/proc/mtd", "r" ) ) )
    {
	while( fgets( dev, sizeof( dev ), fp ) )
	{
	    if( sscanf( dev, "mtd%d:", &i ) && strstr( dev, mtd ) )
	    {
		snprintf( dev, sizeof( dev ), "/dev/mtd/%d", i );
		fclose( fp );
		return open( dev, flags );
	    }
	}
	fclose( fp );
    }

    return open( mtd, flags );
}

/*
 * Erase an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
int mtd_erase( const char *mtd )
{
    int mtd_fd;
    mtd_info_t mtd_info;
    erase_info_t erase_info;

    // char *et0;
    // char *et1;
    // et0 = nvram_safe_get ("et0macaddr");
    // et1 = nvram_safe_get ("et1macaddr");
    // et0 = strdup (et0);
    // et1 = strdup (et1);

    /*
     * Open MTD device 
     */
    if( ( mtd_fd = mtd_open( mtd, O_RDWR ) ) < 0 )
    {
	perror( mtd );
	return errno;
    }

    /*
     * Get sector size 
     */
    if( ioctl( mtd_fd, MEMGETINFO, &mtd_info ) != 0 )
    {
	perror( mtd );
	close( mtd_fd );
	return errno;
    }

    erase_info.length = mtd_info.erasesize;

    for( erase_info.start = 0;
	 erase_info.start < mtd_info.size;
	 erase_info.start += mtd_info.erasesize )
    {
	( void )ioctl( mtd_fd, MEMUNLOCK, &erase_info );
	if( ioctl( mtd_fd, MEMERASE, &erase_info ) != 0 )
	{
	    perror( mtd );
	    close( mtd_fd );
	    return errno;
	}
    }

    close( mtd_fd );
    // nvram_set ("et0macaddr", et0);
    // nvram_set ("et1macaddr", et1);
    // nvram_commit ();
    // free (et0);
    // free (et1);
    return 0;
}

extern int http_get( const char *server, char *buf, size_t count,
		     off_t offset );

/*
 * Write a file to an MTD device
 * @param       path    file to write or a URL
 * @param       mtd     path to or partition name of MTD device 
 * @return      0 on success and errno on failure
 */
int mtd_write( const char *path, const char *mtd )
{
    int mtd_fd = -1;
    mtd_info_t mtd_info;
    erase_info_t erase_info;

    struct sysinfo info;
    struct trx_header trx;
    unsigned long crc;

    FILE *fp;
    char *buf = NULL;
    long count, len, off;
    long sum = 0;		// for debug
    int ret = -1;
    long support_max_len;

    if( strstr( nvram_safe_get( "flash_type" ), "640" ) )	// This is a
								// 8MB flash
	support_max_len = 0x7A0000;	// 8*1024*1024 - 256*1024 - 128*1024;
    else
	support_max_len = TRX_MAX_LEN;

    /*
     * Examine TRX header 
     */
    if( ( fp = fopen( path, "r" ) ) )
	count = safe_fread( &trx, 1, sizeof( struct trx_header ), fp );
    else
	count =
	    http_get( path, ( char * )&trx, sizeof( struct trx_header ), 0 );
    if( count < sizeof( struct trx_header ) )
    {
	fprintf( stderr, "%s: File is too small (%ld bytes)\n", path, count );
	goto fail;
    }
    if( trx.magic != TRX_MAGIC ||
	trx.len > support_max_len || trx.len < sizeof( struct trx_header ) )
    {
	fprintf( stderr, "%s: Bad trx header\n", path );
	goto fail;
    }

    /*
     * Open MTD device and get sector size 
     */
    if( ( mtd_fd = mtd_open( mtd, O_RDWR ) ) < 0 ||
	ioctl( mtd_fd, MEMGETINFO, &mtd_info ) != 0 ||
	mtd_info.erasesize < sizeof( struct trx_header ) )
    {
	perror( mtd );
	goto fail;
    }

    /*
     * See if we have enough memory to store the whole file 
     */
    sysinfo( &info );
    fprintf( stderr, "freeram=[%ld] bufferram=[%ld]\n", info.freeram,
	     info.bufferram );
    if( info.freeram >= ( trx.len + 1 * 1024 * 1024 ) )
    {
	fprintf( stderr, "The free memory is enough, writing image once.\n" );
	/*
	 * Begin to write image after all image be downloaded by web upgrade.
	 * In order to avoid upgrade fail if user unplug the ethernet cable
	 * during upgrading 
	 */
	// if(check_action() == ACT_WEBS_UPGRADE || check_action() ==
	// ACT_WEB_UPGRADE)
	erase_info.length = ROUNDUP( trx.len, mtd_info.erasesize );
	// else
	// erase_info.length = mtd_info.erasesize; 
    }
    else
    {
	erase_info.length = mtd_info.erasesize;
	fprintf( stderr,
		 "The free memory is not enough, writing image per %d bytes.\n",
		 erase_info.length );
    }

    /*
     * Allocate temporary buffer 
     */
    if( !( buf = malloc( erase_info.length ) ) )
    {
	perror( "malloc" );
	goto fail;
    }

    /*
     * Calculate CRC over header 
     */
    crc = crc32( ( uint8 * ) & trx.flag_version,
		 sizeof( struct trx_header ) - OFFSETOF( struct trx_header,
							 flag_version ),
		 CRC32_INIT_VALUE );

    if( trx.flag_version & TRX_NO_HEADER )
	trx.len -= sizeof( struct trx_header );

    /*
     * Write file or URL to MTD device 
     */
    for( erase_info.start = 0; erase_info.start < trx.len;
	 erase_info.start += count )
    {
	len = MIN( erase_info.length, trx.len - erase_info.start );
	if( ( trx.flag_version & TRX_NO_HEADER ) || erase_info.start )
	    count = off = 0;
	else
	{
	    count = off = sizeof( struct trx_header );
	    memcpy( buf, &trx, sizeof( struct trx_header ) );
	}
	if( fp )
	    count += safe_fread( &buf[off], 1, len - off, fp );
	else
	    count +=
		http_get( path, &buf[off], len - off,
			  erase_info.start + off );

	/*
	 * for debug 
	 */
	sum = sum + count;
	fprintf( stderr, "sum=[%ld]\n", sum );

	if( count < len )
	{
	    fprintf( stderr, "%s: Truncated file (actual %ld expect %ld)\n",
		     path, count - off, len - off );
	    goto fail;
	}
	/*
	 * Update CRC 
	 */
	crc = crc32( &buf[off], count - off, crc );
	/*
	 * Check CRC before writing if possible 
	 */
	if( count == trx.len )
	{
	    if( crc != trx.crc32 )
	    {
		fprintf( stderr, "%s: Bad CRC\n", path );
		goto fail;
	    }
	    else
	    {
		fprintf( stderr, "%s: CRC OK\n", mtd );
		fprintf( stderr,
			 "Writing image to flash, waiting a moment...\n" );
	    }
	}
	/*
	 * Do it 
	 */
	( void )ioctl( mtd_fd, MEMUNLOCK, &erase_info );
	if( ioctl( mtd_fd, MEMERASE, &erase_info ) != 0 ||
	    write( mtd_fd, buf, count ) != count )
	{
	    perror( mtd );
	    goto fail;
	}
    }

    ret = 0;

  fail:
    if( buf )
    {
	/*
	 * Dummy read to ensure chip(s) are out of lock/suspend state 
	 */
	( void )read( mtd_fd, buf, 2 );
	free( buf );
    }

    if( mtd_fd >= 0 )
	close( mtd_fd );
    if( fp )
	fclose( fp );
    return ret;
}

int nvram_restore( const char *path, char *mtd )
{

    int mtd_fd = -1;
    mtd_info_t mtd_info;
    erase_info_t erase_info;

    struct nvram_header header;
    unsigned long crc = 0;

    FILE *fp, *fp1;
    char buf[NVRAM_SPACE];
    long count = 0;
    int ret = -1;

    /*
     * Examine TRX header 
     */
    if( ( fp = fopen( path, "r" ) ) )
	count = safe_fread( &buf, 1, sizeof( buf ), fp );

    if( count < sizeof( struct nvram_header ) )
    {
	fprintf( stderr, "%s: File is too small (%ld bytes)\n", path, count );
	goto fail;
    }

    decode( buf, sizeof( buf ) );

    memcpy( &header, buf, sizeof( struct nvram_header ) );

    fprintf( stderr,
	     "count=[%ld] header.len=[%d] header.magic[%x] header.crc[%x]\n",
	     count, header.len, header.magic, header.crc_ver_init );
    if( count < header.len || header.magic != NVRAM_MAGIC )
    {
	fprintf( stderr, "Invalid nvram header\n" );
	return EINVAL;
    }

    crc = crc8( ( uint8 * ) ( &buf ) + 9, header.len - 9, CRC8_INIT_VALUE );

    if( crc != ( header.crc_ver_init & 0xff ) )
    {
	fprintf( stderr, "Bad CRC\n" );
	goto fail;
    }
    else
	fprintf( stderr, "CRC OK\n" );

    fprintf( stderr, "Erase old nvram data\n" );
    mtd_erase( mtd );

    /*
     * Open MTD device and get sector size 
     */
    if( ( mtd_fd = mtd_open( mtd, O_RDWR ) ) < 0 ||
	ioctl( mtd_fd, MEMGETINFO, &mtd_info ) != 0 )
    {
	printf( "fail1\n" );
	perror( mtd );
	goto fail;
    }

    ( void )ioctl( mtd_fd, MEMUNLOCK, &erase_info );

    fprintf( stderr, "Write new nvram data\n" );
    if( !( fp1 = fopen( "/dev/mtd/3", "w" ) ) )
    {
	perror( mtd );
	goto fail;
    }

    /*
     * Write NVRAM space 
     */
    fseek( fp1, -NVRAM_SPACE, SEEK_END );
    fwrite( buf, count, 1, fp1 );

    /*
     * Close NVRAM and regenerate environment file 
     */
    fclose( fp );
    fclose( fp1 );

    fprintf( stderr, "Done\n" );

    ret = 0;

  fail:

    return ret;

}

/*
 * Irving -  We need an unlock function in order to mount a r/w jffs2 partition
 * Unlock an MTD device
 * @param       mtd     path to or partition name of MTD device
 * @return      0 on success and errno on failure
 */
int mtd_unlock( const char *mtd )
{
    int mtd_fd;
    mtd_info_t mtd_info;
    erase_info_t lock_info;

    /*
     * Open MTD device 
     */
    if( ( mtd_fd = mtd_open( mtd, O_RDWR ) ) < 0 )
    {
	perror( mtd );
	return errno;
    }

    /*
     * Get sector size 
     */
    if( ioctl( mtd_fd, MEMGETINFO, &mtd_info ) != 0 )
    {
	perror( mtd );
	close( mtd_fd );
	return errno;
    }

    lock_info.start = 0;
    lock_info.length = mtd_info.size;

    if( ioctl( mtd_fd, MEMUNLOCK, &lock_info ) )
    {
	fprintf( stderr, "Could not unlock MTD device: %s\n", mtd );
	perror( mtd );
	close( mtd_fd );
	return errno;
    }

    close( mtd_fd );
    return 0;
}
