/*
 *  AES-256 file encryption program
 *
 *  Copyright (C) 2006  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#ifndef WIN32
#include <sys/types.h>
#include <unistd.h>
#else
#include <windows.h>
#include <io.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "xyssl/aes.h"
#include "xyssl/sha2.h"

#define MODE_ENCRYPT    0
#define MODE_DECRYPT    1

void scanf_argv( char *prompt, char **arg )
{
    printf( "%s", prompt );
    fflush( stdout );
    *arg = (char *) malloc( 1024 );
    scanf( "%1023s", *arg );
}

int main( int argc, char *argv[] )
{
    char user_key[512];
    unsigned char IV[16];
    unsigned char tmp[16];
    unsigned char digest[32];
    unsigned char k_ipad[64];
    unsigned char k_opad[64];
    unsigned char buffer[1024];

    int ret, i, n, mode, lastn;
    FILE *fkey, *fin, *fout;

#ifndef WIN32
      off_t filesize, offset;
#else
    __int64 filesize, offset;
#endif

    aes_context aes_ctx;
    sha2_context sha_ctx;

    ret = 1;

    if( argc != 5 )
    {
        printf( "\n  aescrypt2 <mode> <infile> <outfile> <key_file or"
                " key_string>\n\n  mode: 0 = encrypt, 1 = decrypt\n\n" );

#ifndef WIN32
        goto exit;
#else
        scanf_argv( "  mode    -> ", &argv[1] );
        scanf_argv( "  infile  -> ", &argv[2] );
        scanf_argv( "  outfile -> ", &argv[3] );
        scanf_argv( "  key     -> ", &argv[4] );
        printf( "\n" );
#endif
    }

    mode = atoi( argv[1] );

    if( mode != MODE_ENCRYPT && mode != MODE_DECRYPT )
    {
        fprintf( stderr, "invalide operation mode\n" );
        goto exit;
    }

    if( strcmp( argv[2], argv[3] ) == 0 )
    {
        fprintf( stderr, "input and output filenames must differ\n" );
        goto exit;
    }

    if( ( fin = fopen( argv[2], "rb" ) ) == NULL )
    {
        fprintf( stderr, "fopen(%s,rb) failed\n", argv[2] );
        goto exit;
    }

    if( ( fout = fopen( argv[3], "wb" ) ) == NULL )
    {
        fprintf( stderr, "fopen(%s,wb) failed\n", argv[3] );
        goto exit;
    }

    /*
     * Read the secret key and clean the command line.
     */
    memset( user_key, 0, sizeof( user_key ) );

    if( ( fkey = fopen( argv[4], "rb" ) ) != NULL )
    {
        fread( user_key, 1, sizeof( user_key ) - 1, fkey );
        fclose( fkey );
    }
    else
        strncpy( user_key, argv[4], sizeof( user_key ) - 1 );

    memset( argv[4], 0, strlen( argv[4] ) );

    /*
     * Read the input file size.
     */
#ifndef WIN32
    if( ( filesize = lseek( fileno( fin ), 0, SEEK_END ) ) < 0 )
    {
        perror( "lseek" );
        goto exit;
    }
#else
    {
        /*
         * Properly handle very large files on Win32.
         */
        LARGE_INTEGER li_size;

        li_size.QuadPart = 0;
        li_size.LowPart  = SetFilePointer(
            (HANDLE) _get_osfhandle( _fileno( fin ) ),
            li_size.LowPart, &li_size.HighPart, FILE_END );

        if( li_size.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR )
        {
            fprintf( stderr, "SetFilePointer(0,FILE_END) failed\n" );
            goto exit;
        }

        filesize = li_size.QuadPart;
    }
#endif

    if( fseek( fin, 0, SEEK_SET ) < 0 )
    {
        fprintf( stderr, "fseek(0,SEEK_SET) failed\n" );
        goto exit;
    }

    if( mode == MODE_ENCRYPT )
    {
        /*
         * Generate the initialization vector as:
         * IV = sha2( time + filesize + filename )
         * truncated to the AES block size.
         */
        time_t cur_time = time( NULL );

        for( i = 0; i < 8; i++ )
            buffer[i    ] = (unsigned char)( cur_time >> (i * 8) );

        for( i = 0; i < 8; i++ )
            buffer[i + 8] = (unsigned char)( filesize >> (i * 8) );

        sha2_starts( &sha_ctx );
        sha2_update( &sha_ctx, buffer, 16 );
        sha2_update( &sha_ctx, (unsigned char *) argv[2],
                                         strlen( argv[2] ) );
        sha2_finish( &sha_ctx, digest );

        memcpy( IV, digest, 16 );

        /*
         * The last four bits in the IV are actually used
         * to store the file size modulo the AES block size.
         */
        lastn = (int) ( filesize & 0x0F );

        IV[15] &= 0xF0;
        IV[15] |= lastn;

        /*
         * Append the IV at the beginning of the output.
         */
        if( fwrite( IV, 1, 16, fout ) != 16 )
        {
            fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
            goto exit;
        }

        /*
         * Hash the IV and the secret key together 8192 times
         * using the result to setup the AES context and HMAC.
         */
        memset( digest, 0,  32 );
        memcpy( digest, IV, 16 );

        for( i = 0; i < 8192; i++ )
        {
            sha2_starts( &sha_ctx );
            sha2_update( &sha_ctx, digest, 32 );
            sha2_update( &sha_ctx, (unsigned char *) user_key,
                                             strlen( user_key ) );
            sha2_finish( &sha_ctx, digest );
        }

        memset( user_key, 0, sizeof( user_key ) );
        aes_set_key( &aes_ctx, digest, 256 );

        memset( k_ipad, 0x36, 64 );
        memset( k_opad, 0x5C, 64 );
        for( i = 0; i < 32; i++ )
        {
            k_ipad[i] ^= digest[i];
            k_opad[i] ^= digest[i];
        }

        /*
         * Encrypt and write the ciphertext.
         */
        sha2_starts( &sha_ctx );
        sha2_update( &sha_ctx, k_ipad, 64 );

        for( offset = 0; offset < filesize; offset += 16 )
        {
            n = ( filesize - offset > 16 ) ? 16 : (int)
                ( filesize - offset );

            if( fread( buffer, 1, n, fin ) != (size_t) n )
            {
                fprintf( stderr, "fread(%d bytes) failed\n", n );
                goto exit;
            }

            for( i = 0; i < 16; i++ )
                buffer[i] ^= IV[i];

            aes_encrypt( &aes_ctx, buffer, buffer );
            sha2_update( &sha_ctx, buffer, 16 );

            if( fwrite( buffer, 1, 16, fout ) != 16 )
            {
                fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
                goto exit;
            }

            memcpy( IV, buffer, 16 );
        }

        /*
         * Finally write the HMAC.
         */
        sha2_finish( &sha_ctx, digest );
        sha2_starts( &sha_ctx );
        sha2_update( &sha_ctx, k_opad, 64 );
        sha2_update( &sha_ctx, digest, 32 );
        sha2_finish( &sha_ctx, digest );

        if( fwrite( digest, 1, 32, fout ) != 32 )
        {
            fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
            goto exit;
        }

        ret = 0;
    }

    if( mode == MODE_DECRYPT )
    {
        /*
         *  The encrypted file must be structured as follows:
         *
         *        00 .. 15              Initialization Vector
         *        16 .. 31              AES Encrypted Block #1
         *           ..
         *      N*16 .. (N+1)*16 - 1    AES Encrypted Block #N
         *  (N+1)*16 .. (N+1)*16 + 32   HMAC-sha2(ciphertext)
         */
        if( filesize < 48 )
        {
            fprintf( stderr, "File too short to be encrypted.\n" );
            goto exit;
        }

        if( ( filesize & 0x0F ) != 0 )
        {
            fprintf( stderr, "File size not a multiple of 16.\n" );
            goto exit;
        }

        /*
         * Substract the IV + HMAC length.
         */
        filesize -= ( 16 + 32 );

        /*
         * Read the IV and original filesize modulo 16.
         */
        if( fread( buffer, 1, 16, fin ) != 16 )
        {
            fprintf( stderr, "fread(%d bytes) failed\n", 16 );
            goto exit;
        }

        memcpy( IV, buffer, 16 );

        lastn = IV[15] & 0x0F;

        /*
         * Hash the IV and the secret key together 8192 times
         * using the result to setup the AES context and HMAC.
         */
        memset( digest, 0,  32 );
        memcpy( digest, IV, 16 );

        for( i = 0; i < 8192; i++ )
        {
            sha2_starts( &sha_ctx );
            sha2_update( &sha_ctx, digest, 32 );
            sha2_update( &sha_ctx, (unsigned char *) user_key,
                                             strlen( user_key ) );
            sha2_finish( &sha_ctx, digest );
        }

        memset( user_key, 0, sizeof( user_key ) );

        aes_set_key( &aes_ctx, digest, 256 );

        memset( k_ipad, 0x36, 64 );
        memset( k_opad, 0x5C, 64 );

        for( i = 0; i < 32; i++ )
        {
            k_ipad[i] ^= digest[i];
            k_opad[i] ^= digest[i];
        }

        /*
         * Decrypt and write the plaintext.
         */
        sha2_starts( &sha_ctx );
        sha2_update( &sha_ctx, k_ipad, 64 );

        for( offset = 0; offset < filesize; offset += 16 )
        {
            if( fread( buffer, 1, 16, fin ) != 16 )
            {
                fprintf( stderr, "fread(%d bytes) failed\n", 16 );
                goto exit;
            }

            memcpy( tmp, buffer, 16 );
 
            sha2_update( &sha_ctx, buffer, 16 );
            aes_decrypt( &aes_ctx, buffer, buffer );
   
            for( i = 0; i < 16; i++ )
                buffer[i] ^= IV[i];

            memcpy( IV, tmp, 16 );

            n = ( lastn > 0 && offset == filesize - 16 )
                ? lastn : 16;

            if( fwrite( buffer, 1, n, fout ) != (size_t) n )
            {
                fprintf( stderr, "fwrite(%d bytes) failed\n", n );
                goto exit;
            }
        }

        /*
         * Verify the message authentication code.
         */
        sha2_finish( &sha_ctx, digest );

        sha2_starts( &sha_ctx );
        sha2_update( &sha_ctx, k_opad, 64 );
        sha2_update( &sha_ctx, digest, 32 );
        sha2_finish( &sha_ctx, digest );

        if( fread( buffer, 1, 32, fin ) != 32 )
        {
            fprintf( stderr, "fread(%d bytes) failed\n", 32 );
            goto exit;
        }

        if( memcmp( digest, buffer, 32 ) != 0 )
        {
            fprintf( stderr, "HMAC check failed: wrong key, "
                             "or file corrupted.\n" );
            goto exit;
        }

        ret = 0;
    }

exit:

#ifdef WIN32
    if( ret != 0 )
    {
        fflush( stderr );
        printf( "\nPress Enter to exit this program.\n" );
        fflush( stdout ); getchar();
    }
#endif

    return( ret );
}
