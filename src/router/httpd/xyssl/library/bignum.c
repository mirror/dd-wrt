/*
 *  Multi-precision integer library
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
/*
 *  This MPI implementation is based on:
 *
 *  http://www.cacr.math.uwaterloo.ca/hac/about/chap14.pdf
 *  http://math.libtomcrypt.com/files/tommath.pdf
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "bignum.h"
#include "bn_asm.h"

#define ciL    sizeof(t_int)    /* chars in limb  */
#define biL    (ciL << 3)       /* bits  in limb  */
#define biH    (ciL << 2)       /* half limb size */

/*
 * Bits/chars to # of limbs conversion
 */
#define BITS_TO_LIMBS(i)  (((i) + biL - 1) / biL)
#define CHARS_TO_LIMBS(i) (((i) + ciL - 1) / ciL)

/*
 * Initialize one or more mpi
 */
void mpi_init( mpi *X, ... )
{
    va_list args;

    va_start( args, X );

    while( X != NULL )
    {
        memset( X, 0, sizeof( mpi ) );
        X = va_arg( args, mpi* );
    }

    va_end( args );
}

/*
 * Unallocate one or more mpi
 */
void mpi_free( mpi *X, ... )
{
    va_list args;

    va_start( args, X );

    while( X != NULL )
    {
        if( X->p != NULL )
        {
            memset( X->p, 0, X->n * ciL );
            free( X->p );
        }

        memset( X, 0, sizeof( mpi ) );

        X = va_arg( args, mpi* );
    }

    va_end( args );
}

/*
 * Enlarge X to the specified # of limbs
 */
int mpi_grow( mpi *X, int nblimbs )
{
    int n = X->n;

    if( n < nblimbs )
    {
        if( X->s == 0 )
            X->s = 1;

        X->n = nblimbs;
        X->p = (t_int *) realloc( X->p, X->n * ciL );

        if( X->p == NULL )
            return( 1 );

        memset( X->p + n, 0, ( X->n - n ) * ciL );
    }

    return( 0 );
}

/*
 * Copy the contents of Y into X
 */
int mpi_copy( mpi *X, mpi *Y )
{
    int ret, i;

    if( X == Y )
        return( 0 );

    for( i = Y->n - 1; i > 0; i-- )
        if( Y->p[i] != 0 )
            break;
    i++;

    X->s = Y->s;

    CHK( mpi_grow( X, i ) );

    memset( X->p, 0, X->n * ciL );
    memcpy( X->p, Y->p, i * ciL );

cleanup:

    return( ret );
}

/*
 * Swap the contents of X and Y
 */
void mpi_swap( mpi *X, mpi *Y )
{
    mpi T;

    memcpy( &T, X , sizeof( mpi ) );
    memcpy( X , Y , sizeof( mpi ) );
    memcpy( Y , &T, sizeof( mpi ) );
}

/*
 * Set value from integer
 */
int mpi_lset( mpi *X, int z )
{
    int ret;

    CHK( mpi_grow( X, 1 ) );
    memset( X->p, 0, X->n * ciL );
    X->p[0] = ( z < 0 ) ? -z : z;
    X->s    = ( z < 0 ) ? -1 : 1;

cleanup:

    return( ret );
}

/*
 * Convert an ASCII character to digit value
 */
static int mpi_get_digit( t_int *d, int radix, char c )
{
    *d = 16;

    if( c >= 0x30 && c <= 0x39 ) *d = c - 0x30;
    if( c >= 0x41 && c <= 0x46 ) *d = c - 0x37;
    if( c >= 0x61 && c <= 0x66 ) *d = c - 0x57;

    if( *d >= (t_int) radix )
        return( ERR_MPI_INVALID_CHARACTER );

    return( 0 );
}

/*
 * Set value from string
 */
int mpi_read( mpi *X, char *s, int radix )
{
    int ret, i, j, n;
    t_int d;
    mpi T;

    if( radix < 2 || radix > 16 )
        return( ERR_MPI_INVALID_PARAMETER );

    mpi_init( &T, NULL );

    if( radix == 16 )
    {
        n = BITS_TO_LIMBS( strlen(s) << 2 );

        CHK( mpi_grow( X, n ) );
        CHK( mpi_lset( X, 0 ) );

        for( i = strlen( s ) - 1, j = 0; i >= 0; i--, j++ )
        {
            if( i == 0 && s[i] == '-' )
            {
                X->s = -1;
                break;
            }

            CHK( mpi_get_digit( &d, radix, s[i] ) );
            X->p[j / (ciL * 2)] |= d << ( (j % (ciL * 2)) << 2 );
        }
    }
    else
    {
        CHK( mpi_lset( X, 0 ) );

        for( i = 0; i < (int) strlen( s ); i++ )
        {
            if( i == 0 && s[i] == '-' )
            {
                X->s = -1;
                continue;
            }

            CHK( mpi_get_digit( &d, radix, s[i] ) );
            CHK( mpi_mul_int( &T, X, radix ) );
            CHK( mpi_add_int( X, &T, d ) );
        }
    }

cleanup:

    mpi_free( &T, NULL );
    return( ret );
}

/*
 * Helper to display the digits high-order first
 */
static int mpi_recurse_showf( mpi *X, int radix, FILE *fout )
{
    int ret;
    t_int r;

    if( radix < 2 || radix > 16 )
        return( ERR_MPI_INVALID_PARAMETER );

    CHK( mpi_mod_int( &r, X, radix ) );
    CHK( mpi_div_int( X, NULL, X, radix ) );

    if( mpi_cmp_int( X, 0 ) != 0 )
        CHK( mpi_recurse_showf( X, radix, fout ) );

    if( fout != NULL )
        fprintf( fout, "%c", ( r < 10 ) ? ( (char) r + 0x30 )
                                        : ( (char) r + 0x37 ) );
    else
        printf( "%c", ( r < 10 ) ? ( (char) r + 0x30 )
                                 : ( (char) r + 0x37 ) );

cleanup:

    return( ret );
}

/*
 * Print the value of X into fout
 */
int mpi_showf( char *name, mpi *X, int radix, FILE *fout )
{
    int ret, i, j, k, l;
    mpi T;

    if( radix < 2 || radix > 16 )
        return( ERR_MPI_INVALID_PARAMETER );

    mpi_init( &T, NULL );

    if( fout != NULL )
        fprintf( fout, "%s%c", name, ( X->s == -1 ) ? '-' : ' ' );
    else
        printf( "%s%c", name, ( X->s == -1 ) ? '-' : ' ' );

    if( radix == 16 )
    {
        ret = 0;

        for( i = X->n - 1, l = 0; i >= 0; i-- )
        {
            for( j = ciL - 1; j >= 0; j-- )
            {
                k = ( X->p[i] >> (j << 3) ) & 0xFF;

                if( k == 0 && l == 0 && (i + j) != 0 )
                    continue;

                if( fout != NULL )
                    fprintf( fout, "%02X", k );
                else
                    printf( "%02X", k );

                l = 1;
            }
        }
    }
    else
    {
        CHK( mpi_copy( &T, X ) );
        CHK( mpi_recurse_showf( &T, radix, fout ) );
    }

    if( fout != NULL )
        fprintf( fout, "\n" );
    else
        printf( "\n" );

cleanup:

    mpi_free( &T, NULL );
    return( ret );
}

/*
 * Print the value of X on the console
 */
int mpi_show( char *name, mpi *X, int radix )
{
    return( mpi_showf( name, X, radix, NULL ) );
}

/*
 * Import an unsigned value from binary data
 */
int mpi_import( mpi *X, unsigned char *buf, int buflen )
{
    int ret, i, j, n;

    for( n = 0; n < buflen; n++ )
        if( buf[n] != 0 )
            break;

    CHK( mpi_grow( X, CHARS_TO_LIMBS(buflen - n) ) );
    CHK( mpi_lset( X, 0 ) );

    for( i = buflen - 1, j = 0; i >= n; i--, j++ )
        X->p[j / ciL] |= (t_int) buf[i] << ((j % ciL ) << 3);

cleanup:

    return( ret );
}

/*
 * Export an unsigned value into binary data
 */
int mpi_export( mpi *X, unsigned char *buf, int *buflen )
{
    int i, j, n;

    n = ( mpi_size( X ) + 7 ) >> 3;

    if( *buflen < n )
    {
        *buflen = n;
        return( ERR_MPI_BUFFER_TOO_SMALL );
    }

    memset( buf, 0, *buflen );

    for( i = *buflen - 1, j = 0; n > 0; i--, j++, n-- )
        buf[i] = (unsigned char) (X->p[j / ciL] >> ((j % ciL) << 3));

    return( 0 );
}

/*
 * Return the actual size in bits (without leading 0s)
 */
int mpi_size( mpi *X )
{
    int i, j;

    for( i = X->n - 1; i > 0; i-- )
        if( X->p[i] != 0 )
            break;

    for( j = biL - 1; j >= 0; j-- )
        if( ( ( X->p[i] >> j ) & 1 ) != 0 )
            break;

    return( ( i * biL ) + j + 1 );
}

/*
 * Return the number of least significant bits
 */
int mpi_lsb( mpi *X )
{
    int i, j, count = 0;

    for( i = 0; i < X->n; i++ )
        for( j = 0; j < (int) biL; j++, count++ )
            if( ( ( X->p[i] >> j ) & 1 ) != 0 )
                return( count );

    return( 0 );
}

/*
 * Left-shift: X <<= count
 */
int mpi_shift_l( mpi *X, int count )
{
    int ret, i, v0, t1;
    t_int r0 = 0, r1;

    v0 = count /  biL;
    t1 = count & (biL - 1);

    i = mpi_size( X ) + count;

    if( X->n * (int) biL < i )
        CHK( mpi_grow( X, BITS_TO_LIMBS( i ) ) );

    ret = 0;

    /*
     * shift by count / limb_size
     */
    if( v0 > 0 )
    {
        for( i = X->n - 1; i >= v0; i-- )
            X->p[i] = X->p[i - v0];

        for( ; i >= 0; i-- )
            X->p[i] = 0;
    }

    /*
     * shift by count % limb_size
     */
    if( t1 > 0 )
    {
        for( i = v0; i < X->n; i++ )
        {
            r1 = X->p[i] >> (biL - t1);
            X->p[i] <<= t1;
            X->p[i] |= r0;
            r0 = r1;
        }
    }

cleanup:

    return( ret );
}

/*
 * Right-shift: X >>= count
 */
int mpi_shift_r( mpi *X, int count )
{
    int i, v0, v1;
    t_int r0 = 0, r1;

    v0 = count /  biL;
    v1 = count & (biL - 1);

    /*
     * shift by count / limb_size
     */
    if( v0 > 0 )
    {
        for( i = 0; i < X->n - v0; i++ )
            X->p[i] = X->p[i + v0];

        for( ; i < X->n; i++ )
            X->p[i] = 0;
    }

    /*
     * shift by count % limb_size
     */
    if( v1 > 0 )
    {
        for( i = X->n - 1; i >= 0; i-- )
        {
            r1 = X->p[i] << (biL - v1);
            X->p[i] >>= v1;
            X->p[i] |= r0;
            r0 = r1;
        }
    }

    return( 0 );
}

/*
 * Compare unsigned values
 */
int mpi_cmp_abs( mpi *X, mpi *Y )
{
    int i, j;

    for( i = X->n - 1; i >= 0; i-- )
        if( X->p[i] != 0 )
            break;

    for( j = Y->n - 1; j >= 0; j-- )
        if( Y->p[j] != 0 )
            break;

    if( i < 0 && j < 0 )
        return( 0 );

    if( i > j ) return(  1 );
    if( j > i ) return( -1 );

    for( ; i >= 0; i-- )
    {
        if( X->p[i] > Y->p[i] ) return(  1 );
        if( X->p[i] < Y->p[i] ) return( -1 );
    }

    return( 0 );
}

/*
 * Compare signed values
 */
int mpi_cmp_mpi( mpi *X, mpi *Y )
{
    int i, j;

    for( i = X->n - 1; i >= 0; i-- )
        if( X->p[i] != 0 )
            break;

    for( j = Y->n - 1; j >= 0; j-- )
        if( Y->p[j] != 0 )
            break;

    if( i < 0 && j < 0 )
        return( 0 );

    if( i > j ) return(  X->s );
    if( j > i ) return( -X->s );

    if( X->s > 0 && Y->s < 0 ) return(  1 );
    if( Y->s > 0 && X->s < 0 ) return( -1 );

    for( ; i >= 0; i-- )
    {
        if( X->p[i] > Y->p[i] ) return(  X->s );
        if( X->p[i] < Y->p[i] ) return( -X->s );
    }

    return( 0 );
}

/*
 * Compare signed values
 */
int mpi_cmp_int( mpi *X, int z )
{
    mpi Y;
    t_int p[1];

    *p  = ( z < 0 ) ? -z : z;
    Y.s = ( z < 0 ) ? -1 : 1;
    Y.n = 1;
    Y.p = p;

    return( mpi_cmp_mpi( X, &Y ) );
}

/*
 * Unsigned addition: X = |A| + |B|  (HAC 14.7)
 */
int mpi_add_abs( mpi *X, mpi *A, mpi *B )
{
    int ret, i, j;
    t_int *o, *p, c;

    if( X == B )
    {
        mpi *T = A; A = X; B = T;
    }

    if( X != A )
        CHK( mpi_copy( X, A ) );

    for( j = B->n - 1; j >= 0; j-- )
        if( B->p[j] != 0 )
            break;

    CHK( mpi_grow( X, j + 1 ) );

    o = B->p; p = X->p; c = 0;

    for( i = 0; i <= j; i++, o++, p++ )
    {
        *p +=  c; c  = ( *p <  c );
        *p += *o; c += ( *p < *o );
    }

    while( c != 0 )
    {
        if( i >= X->n )
        {
            CHK( mpi_grow( X, i + 1 ) );
            p = X->p + i;
        }

        *p += c; c = ( *p < c ); i++;
    }

cleanup:

    return( ret );
}

/*
 * Helper for mpi substraction
 */
static void mpi_sub_hlp( int n, t_int *s, t_int *d )
{
    int i;
    t_int c, z;

    for( i = c = 0; i < n; i++, s++, d++ )
    {
        z = ( *d <  c );     *d -=  c;
        c = ( *d < *s ) + z; *d -= *s;
    }

    while( c != 0 )
    {
        z = ( *d < c ); *d -= c;
        c = z; i++; d++;
    }
}

/*
 * Unsigned substraction: X = |A| - |B|  (HAC 14.9)
 */
int mpi_sub_abs( mpi *X, mpi *A, mpi *B )
{
    mpi TB;
    int ret, n;

    if( mpi_cmp_abs( A, B ) < 0 )
        return( ERR_MPI_NEGATIVE_VALUE );

    mpi_init( &TB, NULL );

    if( X == B )
    {
        CHK( mpi_copy( &TB, B ) );
        B = &TB;
    }

    if( X != A )
        CHK( mpi_copy( X, A ) );

    ret = 0;

    for( n = B->n - 1; n >= 0; n-- )
        if( B->p[n] != 0 )
            break;

    mpi_sub_hlp( n + 1, B->p, X->p );

cleanup:

    mpi_free( &TB, NULL );
    return( ret );
}

/*
 * Signed addition: X = A + B
 */
int mpi_add_mpi( mpi *X, mpi *A, mpi *B )
{
    int ret, s = A->s;

    if( A->s * B->s < 0 )
    {
        if( mpi_cmp_abs( A, B ) >= 0 )
        {
            CHK( mpi_sub_abs( X, A, B ) );
            X->s =  s;
        }
        else
        {
            CHK( mpi_sub_abs( X, B, A ) );
            X->s = -s;
        }
    }
    else
    {
        CHK( mpi_add_abs( X, A, B ) );
        X->s = s;
    }

cleanup:

    return( ret );
}

/*
 * Signed substraction: X = A - B
 */
int mpi_sub_mpi( mpi *X, mpi *A, mpi *B )
{
    int ret, s = A->s;

    if( A->s * B->s > 0 )
    {
        if( mpi_cmp_abs( A, B ) >= 0 )
        {
            CHK( mpi_sub_abs( X, A, B ) );
            X->s =  s;
        }
        else
        {
            CHK( mpi_sub_abs( X, B, A ) );
            X->s = -s;
        }
    }
    else
    {
        CHK( mpi_add_abs( X, A, B ) );
        X->s = s;
    }

cleanup:

    return( ret );
}

/*
 * Signed addition: X = A + b
 */
int mpi_add_int( mpi *X, mpi *A, int b )
{
    mpi _B;
    t_int p[1];

    p[0] = ( b < 0 ) ? -b : b;
    _B.s = ( b < 0 ) ? -1 : 1;
    _B.n = 1;
    _B.p = p;

    return( mpi_add_mpi( X, A, &_B ) );
}

/*
 * Signed substraction: X = A - b
 */
int mpi_sub_int( mpi *X, mpi *A, int b )
{
    mpi _B;
    t_int p[1];

    p[0] = ( b < 0 ) ? -b : b;
    _B.s = ( b < 0 ) ? -1 : 1;
    _B.n = 1;
    _B.p = p;

    return( mpi_sub_mpi( X, A, &_B ) );
}

/*
 * Helper for mpi multiplication
 */ 
static void mpi_mul_hlp( int i, t_int *s, t_int *d, t_int b )
{
    t_int c = 0;

#if defined(MULADDC_HUIT)

    for( ; i >= 8; i -= 8 )
    {
        MULADDC_INIT
        MULADDC_HUIT
        MULADDC_STOP
    }

    for( ; i > 0; i-- )
    {
        MULADDC_INIT
        MULADDC_CORE
        MULADDC_STOP
    }

#else
    if( i == 32 )
    {
        MULADDC_INIT
        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE

        MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE

        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE   MULADDC_CORE
        MULADDC_CORE   MULADDC_CORE
        MULADDC_STOP
    }
    else
    {
        if( i == 16 )
        {
            MULADDC_INIT
            MULADDC_CORE   MULADDC_CORE
            MULADDC_CORE   MULADDC_CORE
            MULADDC_CORE   MULADDC_CORE
            MULADDC_CORE   MULADDC_CORE

            MULADDC_CORE   MULADDC_CORE
            MULADDC_CORE   MULADDC_CORE
            MULADDC_CORE   MULADDC_CORE
            MULADDC_CORE   MULADDC_CORE
            MULADDC_STOP
        }
        else
        {
            for( ; i >= 8; i -= 8 )
            {
                MULADDC_INIT
                MULADDC_CORE   MULADDC_CORE
                MULADDC_CORE   MULADDC_CORE

                MULADDC_CORE   MULADDC_CORE
                MULADDC_CORE   MULADDC_CORE
                MULADDC_STOP
            }

            for( ; i > 0; i-- )
            {
                MULADDC_INIT
                MULADDC_CORE
                MULADDC_STOP
            }
        }
    }
#endif

    do {
        *d += c; c = ( *d < c ); d++;
    }
    while( c != 0 );
}

/*
 * Baseline multiplication: X = A * B  (HAC 14.12)
 */
int mpi_mul_mpi( mpi *X, mpi *A, mpi *B )
{
    int ret, i, j;
    mpi TA, TB;

    mpi_init( &TA, &TB, NULL );

    if( X == A ) { CHK( mpi_copy( &TA, A ) ); A = &TA; }
    if( X == B ) { CHK( mpi_copy( &TB, B ) ); B = &TB; }

    for( i = A->n - 1; i >= 0; i-- )
        if( A->p[i] != 0 )
            break;

    for( j = B->n - 1; j >= 0; j-- )
        if( B->p[j] != 0 )
            break;

    CHK( mpi_grow( X, i + j + 2 ) );
    CHK( mpi_lset( X, 0 ) );

    for( i++; j >= 0; j-- )
        mpi_mul_hlp( i, A->p, X->p + j, B->p[j] );

    X->s = A->s * B->s;

cleanup:

    mpi_free( &TB, &TA, NULL );
    return( ret );
}

/*
 * Baseline multiplication: X = A * b
 */
int mpi_mul_int( mpi *X, mpi *A, t_int b )
{
    mpi _B;
    t_int p[1];

    _B.s = 1;
    _B.n = 1;
    _B.p = p;
    p[0] = b;

    return( mpi_mul_mpi( X, A, &_B ) );
}

/*
 * Division by mpi: A = Q * B + R  (HAC 14.20)
 */
int mpi_div_mpi( mpi *Q, mpi *R, mpi *A, mpi *B )
{
    int ret, i, n, t, k;
    mpi X, Y, Z, T1, T2;

    if( mpi_cmp_int( B, 0 ) == 0 )
        return( ERR_MPI_DIVISION_BY_ZERO );

    mpi_init( &X, &Y, &Z, &T1, &T2, NULL );

    if( mpi_cmp_abs( A, B ) < 0 )
    {
        if( Q != NULL ) CHK( mpi_lset( Q, 0 ) );
        if( R != NULL ) CHK( mpi_copy( R, A ) );
        return( 0 );
    }

    CHK( mpi_copy( &X, A ) );
    CHK( mpi_copy( &Y, B ) );
    X.s = Y.s = 1;

    CHK( mpi_grow( &Z, A->n + 2 ) );
    CHK( mpi_lset( &Z,  0 ) );
    CHK( mpi_grow( &T1, 2 ) );
    CHK( mpi_grow( &T2, 3 ) );

    k = mpi_size( &Y ) % biL;
    if( k < (int) biL - 1 )
    {
        k = biL - 1 - k;
        CHK( mpi_shift_l( &X, k ) );
        CHK( mpi_shift_l( &Y, k ) );
    }
    else k = 0;

    n = X.n - 1;
    t = Y.n - 1;
    mpi_shift_l( &Y, biL * (n - t) );
    while( mpi_cmp_mpi( &X, &Y ) >= 0 )
    {
        Z.p[n - t]++;
        mpi_sub_mpi( &X, &X, &Y );
    }
    mpi_shift_r( &Y, biL * (n - t) );

    for( i = n; i > t ; i-- )
    {
        if( X.p[i] >= Y.p[t] )
            Z.p[i - t - 1] = ~0;
        else
        {
#if defined(HAVE_LONGLONG)
            t_dbl r;

            r  = (t_dbl) X.p[i] << biL;
            r |= (t_dbl) X.p[i - 1];
            r /= Y.p[t];
            if( r > ((t_dbl) 1 << biL) - 1)
                r = ((t_dbl) 1 << biL) - 1;

            Z.p[i - t - 1] = (t_int) r;
#else
            /*
             * __udiv_qrnnd_c, from GMP/longlong.h
             */
            t_int q0, q1, r0, r1;
            t_int d0, d1, d, m;

            d  = Y.p[t];
            d0 = ( d << biH ) >> biH;
            d1 = ( d >> biH );

            q1 = X.p[i] / d1;
            r1 = X.p[i] - d1 * q1;
            r1 <<= biH;
            r1 |= ( X.p[i - 1] >> biH );

            m = q1 * d0;
            if( r1 < m )
            {
                q1--, r1 += d;
                while( r1 >= d && r1 < m )
                    q1--, r1 += d;
            }
            r1 -= m;

            q0 = r1 / d1;
            r0 = r1 - d1 * q0;
            r0 <<= biH;
            r0 |= ( X.p[i - 1] << biH ) >> biH;

            m = q0 * d0;
            if( r0 < m )
            {
                q0--, r0 += d;
                while( r0 >= d && r0 < m )
                    q0--, r0 += d;
            }
            r0 -= m;

            Z.p[i - t - 1] = ( q1 << biH ) | q0;
#endif
        }

        Z.p[i - t - 1]++;
        do
        {
            Z.p[i - t - 1]--;

            CHK( mpi_lset( &T1, 0 ) );
            T1.p[0] = (t < 1) ? 0 : Y.p[t - 1];
            T1.p[1] = Y.p[t];
            CHK( mpi_mul_int( &T1, &T1, Z.p[i - t - 1] ) );

            CHK( mpi_lset( &T2, 0 ) );
            T2.p[0] = (i < 2) ? 0 : X.p[i - 2];
            T2.p[1] = (i < 1) ? 0 : X.p[i - 1];
            T2.p[2] = X.p[i];
        }
        while( mpi_cmp_mpi( &T1, &T2 ) > 0 );

        CHK( mpi_mul_int( &T1, &Y, Z.p[i - t - 1] ) );
        CHK( mpi_shift_l( &T1,  biL * (i - t - 1) ) );
        CHK( mpi_sub_mpi( &X, &X, &T1 ) );

        if( mpi_cmp_int( &X, 0 ) < 0 )
        {
            CHK( mpi_copy( &T1, &Y ) );
            CHK( mpi_shift_l( &T1, biL * (i - t - 1) ) );
            CHK( mpi_add_mpi( &X, &X, &T1 ) );
            Z.p[i - t - 1]--;
        }
    }

    if( Q != NULL )
    {
        mpi_copy( Q, &Z );
        Q->s = A->s * B->s;
    }

    if( R != NULL )
    {
        mpi_shift_r( &X, k );
        mpi_copy( R, &X );

        R->s = A->s;
        if( mpi_cmp_int( R, 0 ) == 0 )
            R->s = 1;
    }

cleanup:

    mpi_free( &X, &Y, &Z, &T1, &T2, NULL );
    return( ret );
}

/*
 * Division by int: A = Q * b + R
 *
 * Returns 0 if successful
 *         1 if memory allocation failed
 *         ERR_MPI_DIVISION_BY_ZERO if b == 0
 */
int mpi_div_int( mpi *Q, mpi *R, mpi *A, int b )
{
    mpi _B;
    t_int p[1];

    p[0] = ( b < 0 ) ? -b : b;
    _B.s = ( b < 0 ) ? -1 : 1;
    _B.n = 1;
    _B.p = p;

    return( mpi_div_mpi( Q, R, A, &_B ) );
}

/*
 * Modulo: R = A mod B
 */
int mpi_mod_mpi( mpi *R, mpi *A, mpi *B )
{
    int ret;

    CHK( mpi_div_mpi( NULL, R, A, B ) );

    while( mpi_cmp_int( R, 0 ) < 0 )
      CHK( mpi_add_mpi( R, R, B ) );

    while( mpi_cmp_mpi( R, B ) >= 0 )
      CHK( mpi_sub_mpi( R, R, B ) );

cleanup:

    return( ret );
}

/*
 * Modulo: r = A mod b
 */
int mpi_mod_int( t_int *r, mpi *A, int b )
{
    int i;
    t_int x, y, z;

    if( b == 0 )
        return( ERR_MPI_DIVISION_BY_ZERO );

    if( b < 0 )
        b = -b;

    /*
     * handle trivial cases
     */
    if( b == 1 ) { *r = 0;           return( 0 ); }
    if( b == 2 ) { *r = A->p[0] & 1; return( 0 ); }

    /*
     * general case
     */
    for( i = A->n - 1, y = 0; i >= 0; i-- )
    {
        x  = A->p[i];
        y  = ( y << biH ) | ( x >> biH );
        z  = y / b;
        y -= z * b;

        x <<= biH;
        y  = ( y << biH ) | ( x >> biH );
        z  = y / b;
        y -= z * b;
    }

    *r = y;

    return( 0 );
}

/*
 * Fast Montgomery initialization (thanks to Tom St Denis)
 */
static void mpi_montg_init( t_int *mm, mpi *N )
{
    t_int x, m0 = N->p[0];

    x  = m0;
    x += ((m0 + 2) & 4) << 1;
    x *= (2 - (m0 * x));

    if( biL >= 16 ) x *= (2 - (m0 * x));
    if( biL >= 32 ) x *= (2 - (m0 * x));
    if( biL >= 64 ) x *= (2 - (m0 * x));

    *mm = ~x + 1;
}

/*
 * Montgomery multiplication: A = A * B * R^-1 mod N  (HAC 14.36)
 */
static void mpi_montmul( mpi *A, mpi *B, mpi *N, t_int mm, mpi *T )
{
    int i, n, m;
    t_int u0, u1, *d;

    memset( T->p, 0, ciL * T->n );

    d = T->p;
    n = N->n;
    m = ( B->n < n ) ? B->n : n;

    for( i = 0; i < n; i++ )
    {
        /*
         * T = (T + u0*B + u1*N) / 2^biL
         */
        u0 = A->p[i];
        u1 = ( d[0] + u0 * B->p[0] ) * mm;

        mpi_mul_hlp( m, B->p, d, u0 );
        mpi_mul_hlp( n, N->p, d, u1 );

        *d++ = u0; d[n + 1] = 0;
    }

    memcpy( A->p, d, ciL * (n + 1) );

    if( mpi_cmp_abs( A, N ) >= 0 )
        mpi_sub_hlp( n, N->p, A->p );
    else
        /* prevent timing attacks */
        mpi_sub_hlp( n, A->p, T->p );
}

/*
 * Montgomery reduction: A = A * R^-1 mod N
 */
static void mpi_montred( mpi *A, mpi *N, t_int mm, mpi *T )
{
    t_int z = 1;
    mpi U;

    U.n = U.s = z;
    U.p = &z;

    mpi_montmul( A, &U, N, mm, T );
}

/*
 * Sliding-window exponentiation: X = A^E mod N  (HAC 14.85)
 */
int mpi_exp_mod( mpi *X, mpi *A, mpi *E, mpi *N, mpi *_RR )
{
    int ret, i, j, wsize, wbits;
    int bufsize, nblimbs, nbits;
    t_int ei, mm, state;
    mpi RR, T, W[64];

    if( mpi_cmp_int( N, 0 ) < 0 || ( N->p[0] & 1 ) == 0 )
        return( ERR_MPI_INVALID_PARAMETER );

    /*
     * Init temps and window size
     */
    mpi_montg_init( &mm, N );
    mpi_init( &RR, &T, NULL );
    memset( W, 0, sizeof( W ) );

    i = mpi_size( E );

    wsize = ( i > 671 ) ? 6 : ( i > 239 ) ? 5 :
            ( i >  79 ) ? 4 : ( i >  23 ) ? 3 : 1;

    j = N->n + 1;
    CHK( mpi_grow( X, j ) );
    CHK( mpi_grow( &W[1],  j ) );
    CHK( mpi_grow( &T, j * 2 ) );

    /*
     * If 1st call, pre-compute R^2 mod N
     */
    if( _RR == NULL || _RR->p == NULL )
    {
        CHK( mpi_lset( &RR, 1 ) );
        CHK( mpi_shift_l( &RR, N->n * 2 * biL ) );
        CHK( mpi_mod_mpi( &RR, &RR, N ) );

        if( _RR != NULL )
            memcpy( _RR, &RR, sizeof( mpi ) );
    }
    else
        memcpy( &RR, _RR, sizeof( mpi ) );

    /*
     * W[1] = A * R^2 * R^-1 mod N = A * R mod N
     */
    if( mpi_cmp_mpi( A, N ) >= 0 )
        mpi_mod_mpi( &W[1], A, N );
    else   mpi_copy( &W[1], A );

    mpi_montmul( &W[1], &RR, N, mm, &T );

    /*
     * X = R^2 * R^-1 mod N = R mod N
     */
    CHK( mpi_copy( X, &RR ) );
    mpi_montred( X, N, mm, &T );

    if( wsize > 1 )
    {
        /*
         * W[1 << (wsize - 1)] = W[1] ^ (wsize - 1)
         */
        j =  1 << (wsize - 1);

        CHK( mpi_grow( &W[j], N->n + 1 ) );
        CHK( mpi_copy( &W[j], &W[1]    ) );

        for( i = 0; i < wsize - 1; i++ )
            mpi_montmul( &W[j], &W[j], N, mm, &T );
    
        /*
         * W[i] = W[i - 1] * W[1]
         */
        for( i = j + 1; i < (1 << wsize); i++ )
        {
            CHK( mpi_grow( &W[i], N->n + 1 ) );
            CHK( mpi_copy( &W[i], &W[i - 1] ) );

            mpi_montmul( &W[i], &W[1], N, mm, &T );
        }
    }

    nblimbs = E->n;
    bufsize = 0;
    nbits   = 0;
    wbits   = 0;
    state   = 0;

    while( 1 )
    {
        if( bufsize == 0 )
        {
            if( nblimbs-- == 0 )
                break;

            bufsize = sizeof( t_int ) << 3;
        }

        bufsize--;

        ei = (E->p[nblimbs] >> bufsize) & 1;

        /*
         * skip leading 0s
         */
        if( ei == 0 && state == 0 )
            continue;

        if( ei == 0 && state == 1 )
        {
            /*
             * out of window, square X
             */
            mpi_montmul( X, X, N, mm, &T );
            continue;
        }

        /*
         * add ei to current window
         */
        state = 2;

        nbits++;
        wbits |= (ei << (wsize - nbits));

        if( nbits == wsize )
        {
            /*
             * X = X^wsize R^-1 mod N
             */
            for( i = 0; i < wsize; i++ )
                mpi_montmul( X, X, N, mm, &T );

            /*
             * X = X * W[wbits] R^-1 mod N
             */
            mpi_montmul( X, &W[wbits], N, mm, &T );

            state--;
            nbits = 0;
            wbits = 0;
        }
    }

    /*
     * process the remaining bits
     */
    for( i = 0; i < nbits; i++ )
    {
        mpi_montmul( X, X, N, mm, &T );

        wbits <<= 1;

        if( (wbits & (1 << wsize)) != 0 )
            mpi_montmul( X, &W[1], N, mm, &T );
    }

    /*
     * X = A^E * R * R^-1 mod N = A^E mod N
     */
    mpi_montred( X, N, mm, &T );

cleanup:

    for( i = (1 << (wsize - 1)); i < (1 << wsize); i++ )
        mpi_free( &W[i], NULL );

    if( _RR != NULL )
         mpi_free( &W[1], &T, NULL );
    else mpi_free( &W[1], &T, &RR, NULL );

    return( ret );
}

/*
 * Greatest common divisor: G = gcd(A, B)  (HAC 14.54)
 */
int mpi_gcd( mpi *G, mpi *A, mpi *B )
{
    int ret, count;
    mpi TG, TA, TB;

    mpi_init( &TG, &TA, &TB, NULL );

    CHK( mpi_lset( &TG, 1 ) );
    CHK( mpi_copy( &TA, A ) );
    CHK( mpi_copy( &TB, B ) );

    TA.s = TB.s = 1;

    count = ( mpi_lsb( &TA ) < mpi_lsb( &TB ) )
            ? mpi_lsb( &TA ) : mpi_lsb( &TB );

    CHK( mpi_shift_l( &TG, count ) );
    CHK( mpi_shift_r( &TA, count ) );
    CHK( mpi_shift_r( &TB, count ) );

    while( mpi_cmp_int( &TA, 0 ) != 0 )
    {
        while( ( TA.p[0] & 1 ) == 0 ) CHK( mpi_shift_r( &TA, 1 ) );
        while( ( TB.p[0] & 1 ) == 0 ) CHK( mpi_shift_r( &TB, 1 ) );

        if( mpi_cmp_mpi( &TA, &TB ) >= 0 )
        {
            CHK( mpi_sub_abs( &TA, &TA, &TB ) );
            CHK( mpi_shift_r( &TA, 1 ) );
        }
        else
        {
            CHK( mpi_sub_abs( &TB, &TB, &TA ) );
            CHK( mpi_shift_r( &TB, 1 ) );
        }
    }

    CHK( mpi_mul_mpi( G, &TG, &TB ) );

cleanup:

    mpi_free( &TB, &TA, &TG, NULL );
    return( ret );
}

/*
 * Modular inverse: X = A^-1 mod N  (HAC 14.61 / 14.64)
 */
int mpi_inv_mod( mpi *X, mpi *A, mpi *N )
{
    int ret;
    mpi G, TA, TU, U1, U2, TB, TV, V1, V2;

    if( mpi_cmp_int( N, 0 ) <= 0 )
        return( ERR_MPI_INVALID_PARAMETER );

    mpi_init( &TA, &TU, &U1, &U2, &G,
              &TB, &TV, &V1, &V2, NULL );

    CHK( mpi_gcd( &G, A, N ) );

    if( mpi_cmp_int( &G, 1 ) != 0 )
    {
        ret = ERR_MPI_NOT_INVERTIBLE;
        goto cleanup;
    }

    CHK( mpi_mod_mpi( &TA, A, N ) );
    CHK( mpi_copy( &TU, &TA ) );
    CHK( mpi_copy( &TB, N ) );
    CHK( mpi_copy( &TV, N ) );

    CHK( mpi_lset( &U1, 1 ) );
    CHK( mpi_lset( &U2, 0 ) );
    CHK( mpi_lset( &V1, 0 ) );
    CHK( mpi_lset( &V2, 1 ) );

    do
    {
        while( ( TU.p[0] & 1 ) == 0 )
        {
            CHK( mpi_shift_r( &TU, 1 ) );

            if( ( U1.p[0] & 1 ) != 0 || ( U2.p[0] & 1 ) != 0 )
            {
                CHK( mpi_add_mpi( &U1, &U1, &TB ) );
                CHK( mpi_sub_mpi( &U2, &U2, &TA ) );
            }

            CHK( mpi_shift_r( &U1, 1 ) );
            CHK( mpi_shift_r( &U2, 1 ) );
        }

        while( ( TV.p[0] & 1 ) == 0 )
        {
            CHK( mpi_shift_r( &TV, 1 ) );

            if( ( V1.p[0] & 1 ) != 0 || ( V2.p[0] & 1 ) != 0 )
            {
                CHK( mpi_add_mpi( &V1, &V1, &TB ) );
                CHK( mpi_sub_mpi( &V2, &V2, &TA ) );
            }

            CHK( mpi_shift_r( &V1, 1 ) );
            CHK( mpi_shift_r( &V2, 1 ) );
        }

        if( mpi_cmp_mpi( &TU, &TV ) >= 0 )
        {
            CHK( mpi_sub_mpi( &TU, &TU, &TV ) );
            CHK( mpi_sub_mpi( &U1, &U1, &V1 ) );
            CHK( mpi_sub_mpi( &U2, &U2, &V2 ) );
        }
        else
        {
            CHK( mpi_sub_mpi( &TV, &TV, &TU ) );
            CHK( mpi_sub_mpi( &V1, &V1, &U1 ) );
            CHK( mpi_sub_mpi( &V2, &V2, &U2 ) );
        }
    }
    while( mpi_cmp_int( &TU, 0 ) != 0 );

    while( mpi_cmp_int( &V1, 0 ) < 0 )
      CHK( mpi_add_mpi( &V1, &V1, N ) );

    while( mpi_cmp_mpi( &V1, N ) >= 0 )
      CHK( mpi_sub_mpi( &V1, &V1, N ) );

    CHK( mpi_copy( X, &V1 ) );

cleanup:

    mpi_free( &V2, &V1, &TV, &TB, &G,
              &U2, &U1, &TU, &TA, NULL );

    return( ret );
}

static const int small_prime[] =
{
       3,  113,  271,  443,  619,  821,  1013,  1213,
       5,  127,  277,  449,  631,  823,  1019,  1217,
       7,  131,  281,  457,  641,  827,  1021,  1223,
      11,  137,  283,  461,  643,  829,  1031,  1229,
      13,  139,  293,  463,  647,  839,  1033,  1231,
      17,  149,  307,  467,  653,  853,  1039,  1237,
      19,  151,  311,  479,  659,  857,  1049,  1249,
      23,  157,  313,  487,  661,  859,  1051,  1259,
      29,  163,  317,  491,  673,  863,  1061,  1277,
      31,  167,  331,  499,  677,  877,  1063,  1279,
      37,  173,  337,  503,  683,  881,  1069,  1283,
      41,  179,  347,  509,  691,  883,  1087,  1289,
      43,  181,  349,  521,  701,  887,  1091,  1291,
      47,  191,  353,  523,  709,  907,  1093,  1297,
      53,  193,  359,  541,  719,  911,  1097,  1301,
      59,  197,  367,  547,  727,  919,  1103,  1303,
      61,  199,  373,  557,  733,  929,  1109,  1307,
      67,  211,  379,  563,  739,  937,  1117,  1319,
      71,  223,  383,  569,  743,  941,  1123,  1321,
      73,  227,  389,  571,  751,  947,  1129,  1327,
      79,  229,  397,  577,  757,  953,  1151,  1361,
      83,  233,  401,  587,  761,  967,  1153,  1367,
      89,  239,  409,  593,  769,  971,  1163,  1373,
      97,  241,  419,  599,  773,  977,  1171,  1381,
     101,  251,  421,  601,  787,  983,  1181,  1399,
     103,  257,  431,  607,  797,  991,  1187,  1409,
     107,  263,  433,  613,  809,  997,  1193,  1423,
     109,  269,  439,  617,  811, 1009,  1201,  -110
};

/*
 * Miller-Rabin primality test  (HAC 4.24)
 */
int mpi_is_prime( mpi *X )
{
    int ret, i, j, s, xs;
    mpi W, R, T, A, RR;

    if( mpi_cmp_int( X, 0 ) == 0 )
        return( 0 );

    mpi_init( &W, &R, &T, &A, &RR, NULL );
    xs = X->s; X->s = 1;

    /*
     * test trivial factors first
     */
    if( ( X->p[0] & 1 ) == 0 )
        return( ERR_MPI_IS_COMPOSITE );

    for( i = 0; small_prime[i] > 0; i++ )
    {
        t_int r;

        if( mpi_cmp_int( X, small_prime[i] ) <= 0 )
            return( 0 );

        CHK( mpi_mod_int( &r, X, small_prime[i] ) );

        if( r == 0 )
            return( ERR_MPI_IS_COMPOSITE );
    }

    /*
     * W = |X| - 1
     * R = W >> lsb( W )
     */
    CHK( mpi_sub_int( &W, X, 1 ) );
    CHK( mpi_copy( &R, &W ) );
    CHK( mpi_shift_r( &R, s = mpi_lsb( &W ) ) );

    for( i = 0; i < 8; i++ )
    {
        /*
         * pick a random A, 1 < A < |X| - 1
         */
        CHK( mpi_grow( &A, X->n ) );

        for( j = 0; j < A.n; j++ )
            A.p[j] = (t_int) rand() * rand();

        CHK( mpi_shift_r( &A, mpi_size( &A ) -
                              mpi_size( &W ) + 1 ) );
        A.p[0] |= 3;

        /*
         * A = A^R mod |X|
         */
        CHK( mpi_exp_mod( &A, &A, &R, X, &RR ) );

        if( mpi_cmp_mpi( &A, &W ) == 0 ||
            mpi_cmp_int( &A,  1 ) == 0 )
            continue;

        j = 1;
        while( j < s && mpi_cmp_mpi( &A, &W ) != 0 )
        {
            /*
             * A = A * A mod |X|
             */
            CHK( mpi_mul_mpi( &T, &A, &A ) );
            CHK( mpi_mod_mpi( &A, &T, X  ) );

            if( mpi_cmp_int( &A, 1 ) == 0 )
                break;

            j++;
        }

        /*
         * not prime if A != |X| - 1 or A == 1
         */
        if( mpi_cmp_mpi( &A, &W ) != 0 || j < s )
        {
            ret = ERR_MPI_IS_COMPOSITE;
            break;
        }
    }

cleanup:

    X->s = xs;
    mpi_free( &A, &T, &R, &W, NULL );
    return( ret );
}

/*
 * Prime number generation
 */
int mpi_gen_prime( mpi *X, int nbits, int dh_flag,
                   int (*rng_f)(void *), void *rng_d )
{
    int ret, k, n;
    mpi Y;

    if( nbits < 3 )
        return( ERR_MPI_INVALID_PARAMETER );

    mpi_init( &Y, NULL );

    n = BITS_TO_LIMBS( nbits );

    CHK( mpi_grow( X, n ) );
    CHK( mpi_lset( X, 0 ) );

    for( k = 0; k < X->n; k++ )
        X->p[k] = rng_f( rng_d )
                * rng_f( rng_d );

    k = mpi_size( X );

    if( k < nbits ) CHK( mpi_shift_l( X, nbits - k ) );
    if( k > nbits ) CHK( mpi_shift_r( X, k - nbits ) );

    X->p[0] |= 3;

    if( dh_flag == 0 )
    {
        while( ( ret = mpi_is_prime( X ) ) != 0 )
        {
            if( ret != ERR_MPI_IS_COMPOSITE )
                goto cleanup;

            CHK( mpi_add_int( X, X, 2 ) );
        }
    }
    else
    {
        CHK( mpi_sub_int( &Y, X, 1 ) );
        CHK( mpi_shift_r( &Y, 1 ) );

        while( 1 )
        {
            if( ( ret = mpi_is_prime( X ) ) == 0 )
            {
                if( ( ret = mpi_is_prime( &Y ) ) == 0 )
                    break;

                if( ret != ERR_MPI_IS_COMPOSITE )
                    goto cleanup;
            }

            if( ret != ERR_MPI_IS_COMPOSITE )
                goto cleanup;

            CHK( mpi_add_int( &Y, X, 1 ) );
            CHK( mpi_add_int(  X, X, 2 ) );
            CHK( mpi_shift_r( &Y, 1 ) );
        }
    }

cleanup:

    mpi_free( &Y, NULL );
    return( ret );
}

#ifdef SELF_TEST
/*
 * Checkup routine
 */
int mpi_self_test( void )
{
    int ret;
    mpi A, E, N, X, Y, U, V;

    mpi_init( &A, &E, &N, &X, &Y, &U, &V, NULL );

    CHK( mpi_read( &A, "EFE021C2645FD1DC586E69184AF4A31E" \
                       "D5F53E93B5F123FA41680867BA110131" \
                       "944FE7952E2517337780CB0DB80E61AA" \
                       "E7C8DDC6C5C6AADEB34EB38A2F40D5E6", 16 ) );
    CHK( mpi_read( &E, "B2E7EFD37075B9F03FF989C7C5051C20" \
                       "34D2A323810251127E7BF8625A4F49A5" \
                       "F3E27F4DA8BD59C47D6DAABA4C8127BD" \
                       "5B5C25763222FEFCCFC38B832366C29E", 16 ) );
    CHK( mpi_read( &N, "0066A198186C18C10B2F5ED9B522752A" \
                       "9830B69916E535C8F047518A889A43A5" \
                       "94B6BED27A168D31D4A52F88925AA8F5", 16 ) );

    CHK( mpi_mul_mpi( &X, &A, &N ) );
    CHK( mpi_read( &U, "602AB7ECA597A3D6B56FF9829A5E8B85" \
                       "9E857EA95A03512E2BAE7391688D264A" \
                       "A5663B0341DB9CCFD2C4C5F421FEC814" \
                       "8001B72E848A38CAE1C65F78E56ABDEF" \
                       "E12D3C039B8A02D6BE593F0BBBDA56F1" \
                       "ECF677152EF804370C1A305CAF3B5BF1" \
                       "30879B56C61DE584A0F53A2447A51E", 16 ) );

    printf( "  MPI test #1 (mul_mpi): " );
    if( mpi_cmp_mpi( &X, &U ) != 0 )
    {
        printf( "failed\n" );
        return( 1 );
    }
    printf( "passed\n" );

    CHK( mpi_div_mpi( &X, &Y, &A, &N ) );
    CHK( mpi_read( &U, "256567336059E52CAE22925474705F39A94", 16 ) );
    CHK( mpi_read( &V, "6613F26162223DF488E9CD48CC132C7A" \
                       "0AC93C701B001B092E4E5B9F73BCD27B" \
                       "9EE50D0657C77F374E903CDFA4C642", 16 ) );

    printf( "  MPI test #2 (div_mpi): " );
    if( mpi_cmp_mpi( &X, &U ) != 0 ||
        mpi_cmp_mpi( &Y, &V ) != 0 )
    {
        printf( "failed\n" );
        return( 1 );
    }
    printf( "passed\n" );

    CHK( mpi_exp_mod( &X, &A, &E, &N, NULL ) );
    CHK( mpi_read( &U, "36E139AEA55215609D2816998ED020BB" \
                       "BD96C37890F65171D948E9BC7CBAA4D9" \
                       "325D24D6A3C12710F10A09FA08AB87", 16 ) );

    printf( "  MPI test #3 (exp_mod): " );
    if( mpi_cmp_mpi( &X, &U ) != 0 )
    {
        printf( "failed\n" );
        return( 1 );
    }
    printf( "passed\n" );

    CHK( mpi_inv_mod( &X, &A, &N ) );
    CHK( mpi_read( &U, "003A0AAEDD7E784FC07D8F9EC6E3BFD5" \
                       "C3DBA76456363A10869622EAC2DD84EC" \
                       "C5B8A74DAC4D09E03B5E0BE779F2DF61", 16 ) );

    printf( "  MPI test #4 (inv_mod): " );
    if( mpi_cmp_mpi( &X, &U ) != 0 )
    {
        printf( "failed\n" );
        return( 1 );
    }
    printf( "passed\n" );

cleanup:

    if( ret != 0 )
        printf( "Unexpected error, return code = %d\n", ret );

    mpi_free( &V, &U, &Y, &X, &N, &E, &A, NULL );

    printf( "\n" );
    return( 0 );
}
#else
int mpi_self_test( void )
{
    return( 0 );
}
#endif
