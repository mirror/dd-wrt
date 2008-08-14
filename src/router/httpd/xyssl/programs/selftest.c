/*
 *  Self-test demonstration program
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

#include <stdio.h>

#include "xyssl/md2.h"
#include "xyssl/md4.h"
#include "xyssl/md5.h"
#include "xyssl/sha1.h"
#include "xyssl/sha2.h"
#include "xyssl/arc4.h"
#include "xyssl/des.h"
#include "xyssl/aes.h"
#include "xyssl/bignum.h"
#include "xyssl/base64.h"
#include "xyssl/rsa.h"
#include "xyssl/x509.h"

int main( void )
{
    int ret;

    printf( "\n" );

    if( ( ret =    md2_self_test() ) == 0 &&
        ( ret =    md4_self_test() ) == 0 &&
        ( ret =    md5_self_test() ) == 0 &&
        ( ret =   sha1_self_test() ) == 0 &&
        ( ret =   sha2_self_test() ) == 0 &&
        ( ret =   arc4_self_test() ) == 0 &&
        ( ret =    des_self_test() ) == 0 &&
        ( ret =    aes_self_test() ) == 0 &&
        ( ret =    mpi_self_test() ) == 0 &&
        ( ret = base64_self_test() ) == 0 &&
        ( ret =    rsa_self_test() ) == 0 &&
        ( ret =   x509_self_test() ) == 0 )
        printf( "  [ All tests passed ]\n" );

    printf( "\n" );

#ifdef WIN32
    printf( "  Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( ret );
}
