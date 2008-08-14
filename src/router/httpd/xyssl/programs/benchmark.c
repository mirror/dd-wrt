/*
 *  Benchmark demonstration program
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "xyssl/md2.h"
#include "xyssl/md4.h"
#include "xyssl/md5.h"
#include "xyssl/sha1.h"
#include "xyssl/sha2.h"
#include "xyssl/arc4.h"
#include "xyssl/des.h"
#include "xyssl/aes.h"
#include "xyssl/rsa.h"
#include "xyssl/timing.h"

#define BUFSIZE 1024

int myrand( void *rng_state )
{
    rng_state = NULL;
    return( rand() );
}

int main( void )
{
    int keysize;
    unsigned long i, j, tsc;
    unsigned char buf[BUFSIZE];
    unsigned char tmp[32];
    arc4_context arc4;
    des3_context des3;
    des_context des;
    aes_context aes;
    rsa_context rsa;

    memset( buf, 0xAA, sizeof( buf ) );

    printf( "\n" );

    /*
     * MD2 timing
     */ 
    printf( "  MD2       :  " );
    fflush( stdout );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        md2_csum( buf, BUFSIZE, tmp );

    tsc = hardclock();
    for( j = 0; j < 32; j++ )
        md2_csum( buf, BUFSIZE, tmp );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * MD4 timing
     */ 
    printf( "  MD4       :  " );
    fflush( stdout );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        md4_csum( buf, BUFSIZE, tmp );

    tsc = hardclock();
    for( j = 0; j < 1024; j++ )
        md4_csum( buf, BUFSIZE, tmp );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * MD5 timing
     */ 
    printf( "  MD5       :  " );
    fflush( stdout );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        md5_csum( buf, BUFSIZE, tmp );

    tsc = hardclock();
    for( j = 0; j < 1024; j++ )
        md5_csum( buf, BUFSIZE, tmp );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * SHA-1 timing
     */ 
    printf( "  SHA-1     :  " );
    fflush( stdout );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        sha1_csum( buf, BUFSIZE, tmp );

    tsc = hardclock();
    for( j = 0; j < 1024; j++ )
        sha1_csum( buf, BUFSIZE, tmp );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * SHA-256 timing
     */ 
    printf( "  SHA-256   :  " );
    fflush( stdout );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        sha2_csum( buf, BUFSIZE, tmp );

    tsc = hardclock();
    for( j = 0; j < 1024; j++ )
        sha2_csum( buf, BUFSIZE, tmp );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * ARC4 timing
     */ 
    printf( "  ARC4      :  " );
    fflush( stdout );

    arc4_setup( &arc4, tmp, 32 );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        arc4_crypt( &arc4, buf, BUFSIZE );

    tsc = hardclock();
    for( j = 0; j < 1024; j++ )
        arc4_crypt( &arc4, buf, BUFSIZE );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * Triple-DES timing
     */ 
    printf( "  3DES      :  " );
    fflush( stdout );

    des3_set_3keys( &des3, tmp );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        des3_cbc_encrypt( &des3, tmp, buf, buf, BUFSIZE );

    tsc = hardclock();
    for( j = 0; j < 1024; j++ )
        des3_cbc_encrypt( &des3, tmp, buf, buf, BUFSIZE );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * DES timing
     */ 
    printf( "  DES       :  " );
    fflush( stdout );

    des_set_key( &des, tmp );

    set_alarm( 1 );
    for( i = 1; ! alarmed; i++ )
        des_cbc_encrypt( &des, tmp, buf, buf, BUFSIZE );

    tsc = hardclock();
    for( j = 0; j < 1024; j++ )
        des_cbc_encrypt( &des, tmp, buf, buf, BUFSIZE );

    printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                    ( hardclock() - tsc ) / ( j * BUFSIZE ) );

    /*
     * AES timings
     */ 
    for( keysize = 128; keysize <= 256; keysize += 64 )
    {
        printf( "  AES-%d   :  ", keysize );
        fflush( stdout );

        aes_set_key( &aes, tmp, keysize );

        set_alarm( 1 );

        for( i = 1; ! alarmed; i++ )
            aes_cbc_encrypt( &aes, tmp, buf, buf, BUFSIZE );

        tsc = hardclock();
        for( j = 0; j < 1024; j++ )
            aes_cbc_encrypt( &aes, tmp, buf, buf, BUFSIZE );

        printf( "%9ld Kb/s,  %9ld cycles/byte\n", i * BUFSIZE / 1024,
                        ( hardclock() - tsc ) / ( j * BUFSIZE ) );
    }

    /*
     * RSA-1024 timing
     */ 
    printf( "  RSA-1024  :  " );
    fflush( stdout );

    rsa_gen_key( &rsa, 1024, 65537, myrand, NULL );
    set_alarm( 4 );

    for( i = 1; ! alarmed; i++ )
    {
        buf[0] = 0;
        rsa_public( &rsa, buf, 128, buf, 128 );
    }

    printf( "%9ld  public/s\n", i / 4 );

    printf( "  RSA-1024  :  " );
    fflush( stdout );
    set_alarm( 4 );

    for( i = 1; ! alarmed; i++ )
    {
        buf[0] = 0;
        rsa_private( &rsa, buf, 128, buf, 128 );
    }

    printf( "%9ld private/s\n", i / 4 );

    rsa_free( &rsa );

    /*
     * RSA-2048 timing
     */ 
    printf( "  RSA-2048  :  " );
    fflush( stdout );

    rsa_gen_key( &rsa, 2048, 65537, myrand, NULL );
    set_alarm( 4 );

    for( i = 1; ! alarmed; i++ )
    {
        buf[0] = 0;
        rsa_public( &rsa, buf, 256, buf, 256 );
    }

    printf( "%9ld  public/s\n", i / 4 );

    printf( "  RSA-2048  :  " );
    fflush( stdout );

    set_alarm( 4 );

    for( i = 1; ! alarmed; i++ )
    {
        buf[0] = 0;
        rsa_private( &rsa, buf, 256, buf, 256 );
    }

    printf( "%9ld private/s\n\n", i / 4 );

    rsa_free( &rsa );

#ifdef WIN32
    printf( "  Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( 0 );
}
