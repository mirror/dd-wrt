/*
 *  Small RSA demonstration program
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

#include "xyssl/bignum.h"

int main( void )
{
    mpi E, P, Q, M, H, D, X, Y, Z;

    mpi_init( &E, &P, &Q, &M, &H,
              &D, &X, &Y, &Z, NULL );

    printf( "\n  Public key:\n\n" );

    mpi_read( &P, "2777", 10 );
    mpi_read( &Q, "3203", 10 );
    mpi_read( &E, "257", 10 );

    mpi_mul_mpi( &M, &P, &Q );
    mpi_show( "  . M = P*Q =", &M, 10 );
    mpi_show( "  . E =", &E, 10 );

    printf( "\n  Private key:\n\n" );

    mpi_show( "  . P =", &P, 10 );
    mpi_show( "  . Q =", &Q, 10 );
    mpi_sub_int( &P, &P, 1 );
    mpi_sub_int( &Q, &Q, 1 );
    mpi_mul_mpi( &H, &P, &Q );
    mpi_inv_mod( &D, &E, &H );
    mpi_show( "  . D = E^-1 mod (P-1)*(Q-1) =", &D, 10 );

    printf( "\n  RSA operation:\n\n" );

    mpi_read( &X, "55555", 10 );
    mpi_exp_mod( &Y, &X, &E, &M, NULL );
    mpi_exp_mod( &Z, &Y, &D, &M, NULL );
    mpi_show( "  . X =", &X, 10 );
    mpi_show( "  . Y = X^E mod M =", &Y, 10 );
    mpi_show( "  . Z = Y^D mod M =", &X, 10 );

    printf( "\n" );

    mpi_init( &Z, &Y, &X, &D, &H,
              &M, &Q, &P, &E, NULL );

#ifdef WIN32
    printf( "  Press Enter to exit this program.\n" );
    fflush( stdout ); getchar();
#endif

    return( 0 );
}
