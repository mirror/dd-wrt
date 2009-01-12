
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>

#include <string.h>
#include <linux/version.h>

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE
#define IS_KERNEL26 1
#else
#define IS_KERNEL26 0
#endif

#if IS_KERNEL26
#include <linux/mtd/mtd-abi.h>
#else
#include <linux/mtd/mtd.h>
#endif

#include <trxhdr.h>
#include <crc.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <shutils.h>

#include <cy_conf.h>
#include <utils.h>

#include <endian.h>
#include <byteswap.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define STORE32_LE(X)		bswap_32(X)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define STORE32_LE(X)		(X)
#else
#error unkown endianness!
#endif

/* Netgear WGR614 v8/L defines */
static unsigned long calculate_checksum (int action, char *s, int size);
#define WGR614_FLASH_SIZE              4 * 1024 * 1024
#define WGR614_FLASH_BASE              0xBC000000
#define WGR614_KERNEL_FLASH_ADDR       0xBC020000
#define WGR614_KERNEL_LEN_ADDR         (WGR614_FLASH_BASE + WGR614_FLASH_SIZE - 0x50000 - 8)
#define WGR614_KERNEL_CHKSUM_ADDR      (WGR614_KERNEL_LEN_ADDR + 4)
/* end */

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
    struct mtd_info_user mtd_info;
    struct erase_info_user erase_info;

    /* 
     * char *et0; char *et1;
     * 
     * et0=nvram_safe_get("et0macaddr"); et1=nvram_safe_get("et1macaddr");
     * et0=strdup(et0); et1=strdup(et1); 
     */
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
	fprintf( stderr, "erase[%d]\r", erase_info.start );
	( void )ioctl( mtd_fd, MEMUNLOCK, &erase_info );
	if( ioctl( mtd_fd, MEMERASE, &erase_info ) != 0 )
	{
	    perror( mtd );
	    close( mtd_fd );
	    return errno;
	}
    }

    close( mtd_fd );
    fprintf( stderr, "erase[%d]\n", erase_info.start );
    /* 
     * nvram_set("et0macaddr",et0); nvram_set("et1macaddr",et1);
     * nvram_commit(); free(et0); free(et1); 
     */

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

struct img_info
{
    uint32_t lenght;
    uint32_t CRC;
};

#define SQUASHFS_MAGIC			0x74717368

int mtd_write( const char *path, const char *mtd )
{
    int mtd_fd = -1;
    struct mtd_info_user mtd_info;
    struct erase_info_user erase_info;

    struct sysinfo info;
    struct trx_header trx;
    unsigned long crc;
    int squashfound = 0;
    unsigned int crc_data = 0;
    unsigned int data_len = 0;
    unsigned int cal_chksum = 0;
    FILE *fp;
    char *buf = NULL;
    long count, len, off;
    long sum = 0;		// for debug
    int ret = -1;
    int i;

    /* 
     * Examine TRX header 
     */
    if( ( fp = fopen( path, "r" ) ) )
	count = safe_fread( &trx, 1, sizeof( struct trx_header ), fp );
    else
	return -1;
    // count = http_get (path, (char *) &trx, sizeof (struct trx_header), 0);
    if( count < sizeof( struct trx_header ) )
    {
	fprintf( stderr, "%s: File is too small (%ld bytes)\n", path, count );
	goto fail;
    }
    sysinfo( &info );
#ifdef HAVE_MAGICBOX
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
#elif HAVE_FONERA
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
    // info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_MERAKI
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
    // info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_LS2
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
    // info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_LS5
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
#elif HAVE_WHRAG108
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
#elif HAVE_PB42
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
    // info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_LSX
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
    // info.freeram = 64; //fix, must be flashed in erase blocks
#elif HAVE_TW6600
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
#elif HAVE_CA8
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
#elif HAVE_XSCALE
    trx.magic = STORE32_LE( trx.magic );
    trx.len = STORE32_LE( trx.len );
    trx.crc32 = STORE32_LE( trx.crc32 );
#endif

    if( trx.magic != TRX_MAGIC || trx.len < sizeof( struct trx_header ) )
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
    if( STORE32_LE( trx.flag_version ) & TRX_NO_HEADER )
	trx.len -= sizeof( struct trx_header );

    // #ifndef HAVE_WRK54G
    if( mtd_info.size < trx.len )
    {
	fprintf( stderr, "Image too big for partition: %s\n", mtd );
	close( mtd_fd );
	return 0;
    }
    // #endif

    /* 
     * See if we have enough memory to store the whole file 
     */
    fprintf( stderr, "freeram=[%ld] bufferram=[%ld]\n", info.freeram,
	     info.bufferram );
    int mul = 1;		// temporarily use 1 instead of 4 until we

    // found a a solution
    if( ( info.freeram + info.bufferram ) >= ( trx.len + 1 * 1024 * 1024 ) )
    {
	fprintf( stderr, "The free memory is enough, writing image once.\n" );
	/* 
	 * Begin to write image after all image be downloaded by web upgrade.
	 * In order to avoid upgrade fail if user unplug the ethernet cable
	 * during upgrading 
	 */
	// if(check_action() == ACT_WEBS_UPGRADE || check_action() ==
	// ACT_WEB_UPGRADE)
#if defined(HAVE_EOC2610) || defined(HAVE_EAP3660) || defined(HAVE_ECB3500)
	erase_info.length = mtd_info.erasesize; 
#else
	erase_info.length = ROUNDUP( trx.len, mtd_info.erasesize );
#endif
	// else
	// erase_info.length = mtd_info.erasesize; 
    }
    else
    {
	erase_info.length = mtd_info.erasesize * mul;
	fprintf( stderr,
		 "The free memory is not enough, writing image per %d bytes.\n",
		 erase_info.length );
    }

    /* 
     * Allocate temporary buffer 
     */
    if( !( buf = malloc( erase_info.length ) ) )
    {
	mul = 1;
	erase_info.length = mtd_info.erasesize * mul;
	fprintf( stderr,
		 "The free memory is not enough, writing image per %d bytes.\n",
		 erase_info.length );
	if( !( buf = malloc( erase_info.length ) ) )
	{
	    perror( "malloc" );
	    goto fail;
	}
    }

    /* 
     * Calculate CRC over header 
     */
    crc = crc32( ( uint8 * ) & trx.flag_version,
		 sizeof( struct trx_header ) - OFFSETOF( struct trx_header,
							 flag_version ),
		 CRC32_INIT_VALUE );
    crc_data = 0;
    calculate_checksum (0, NULL, 0); // init
    /* 
     * Write file or URL to MTD device 
     */
    for( erase_info.start = 0; erase_info.start < trx.len;
	 erase_info.start += count )
    {
	len = MIN( erase_info.length, trx.len - erase_info.start );
	if( ( STORE32_LE( trx.flag_version ) & TRX_NO_HEADER )
	    || erase_info.start )
	    count = off = 0;
	else
	{
	    count = off = sizeof( struct trx_header );
	    memcpy( buf, &trx, sizeof( struct trx_header ) );
	}
	// if (fp)
	count += safe_fread( &buf[off], 1, len - off, fp );
	// else
	// count +=
	// http_get (path, &buf[off], len - off, erase_info.start + off);

	/* 
	 * for debug 
	 */
	sum = sum + count;
	fprintf( stderr, "write=[%ld]         \n", sum );

	if( ( ( count < len )
	      && ( len - off ) > ( mtd_info.erasesize * mul ) )
	    || ( count == 0 ) )
	{
	    fprintf( stderr, "%s: Truncated file (actual %ld expect %ld)\n",
		     path, count - off, len - off );
	    goto fail;
	}
	/* 
	 * Update CRC 
	 */
	crc = crc32( &buf[off], count - off, crc );
	calculate_checksum (1, buf, count);

	if( !squashfound )
	{
	    for( i = 0; i < ( count - off ); i++ )
	    {
		unsigned int *sq = ( unsigned int * )&buf[off + i];

		if( *sq == SQUASHFS_MAGIC )
		{
		    squashfound = 1;
		    break;
		}
		crc_data += ( unsigned char )buf[off + i];
	    }
	    data_len += i;
	}
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
	erase_info.length = ROUNDUP( count, mtd_info.erasesize );
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
	/* 
	 * Netgear WGR614v8_L: Write len and checksum at the end of mtd1 
	 */    
	if ( getRouterBrand(  ) == ROUTER_NETGEAR_WGR614L )
	{
	cal_chksum = calculate_checksum ( 2, NULL, 0 );
	
	int offset = WGR614_KERNEL_LEN_ADDR - WGR614_KERNEL_FLASH_ADDR;
	char imageInfo[8];
	
	trx.len = STORE32_LE( trx.len );
	cal_chksum = STORE32_LE( cal_chksum );
	memcpy( &imageInfo[0], (char *)&trx.len, 4 );
	memcpy( &imageInfo[4], (char *)&cal_chksum, 4 );
	
	int sector_start = ( offset / mtd_info.erasesize ) * mtd_info.erasesize;
	
	if( lseek( mtd_fd, sector_start, SEEK_SET) <= 0 )
	{
		//fprintf( stderr, "Error seeking the file descriptor\n" );
		goto fail;
	}

	free( buf );
	
	if( !( buf = malloc( mtd_info.erasesize ) ) ) 
	{
		//fprintf( stderr, "Error allocating image block\n");
		goto fail;
	}

	memset( buf, 0, mtd_info.erasesize );

	if ( read( mtd_fd, buf, mtd_info.erasesize ) != mtd_info.erasesize ) 
	{
		//fprintf( stderr, "Error reading last block from MTD device\n" );
		goto fail;
	}

	if( lseek( mtd_fd, sector_start, SEEK_SET) <= 0)
	{
		//fprintf( stderr, "Error seeking the file descriptor\n" );
		goto fail;
	}

	erase_info.start = sector_start;
	erase_info.length = mtd_info.erasesize;
	ioctl( mtd_fd, MEMUNLOCK, &erase_info );
		
	if ( ioctl( mtd_fd, MEMERASE, &erase_info ) != 0 ) 
	{
		//fprintf( stderr, "Error erasing MTD block\n" );
		goto fail;
	}
	
	char *tmp;	
	tmp = buf + ( offset % mtd_info.erasesize );
	memcpy( tmp, imageInfo, sizeof( imageInfo ) );	

	if ( write( mtd_fd, buf, mtd_info.erasesize ) != mtd_info.erasesize ) 
	{
		//fprintf( stderr, "Error writing chksum to MTD device\n" );
		goto fail;
	}
	
	//fprintf( stderr, "TRX LEN = %x , CHECKSUM = %x\n", trx.len, cal_chksum );	
	fprintf( stderr, "Write len/chksum @ 0x003AFFF8...done.\n" );
		
	} // end


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
#ifdef HAVE_CA8
#ifndef HAVE_ALPHA
#ifndef HAVE_USR5453
    buf = malloc( 65536 );
    FILE *in = fopen( "/dev/mtdblock/2", "rb" );

    fread( buf, 65536, 1, in );
    fclose( in );
    struct img_info *image_info;

    image_info = buf + 0x56;
    image_info->lenght = data_len;
    image_info->CRC = crc_data;
    in = fopen( "/tmp/bdata", "wb" );
    fwrite( buf, 65536, 1, in );
    fclose( in );
    fprintf( stderr, "fixup CRC %X and LEN %X\n", crc_data, data_len );
    eval( "mtd", "-f", "write", "/tmp/bdata", "bdata" );
#endif
#endif
#endif
    // eval("fischecksum");

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
    struct mtd_info_user mtd_info;
    struct erase_info_user lock_info;

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

// Netgear image checksum
static unsigned long calculate_checksum (int action, char *s, int size)
{
    static unsigned long c0, c1;
    unsigned long checksum, b;
    int i;

    switch (action)
    {
        case 0:
            c0 = c1 = 0;
            break;

        case 1:
            for (i = 0; i < size; i++)
            {
                c0 += s[i] & 0xff;
                c1 += c0;
            }
            break;

        case 2:
            b = (c0 & 65535) + ((c0 >> 16) & 65535);
            c0 = ((b >> 16) + b) & 65535;

            b = (c1 & 65535) + ((c1 >> 16) & 65535);
            c1 = ((b >> 16) + b) & 65535;

            checksum = ((c1 << 16) | c0);

            return checksum;
    }
    return 0;
}