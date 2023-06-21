/* Rational - Rational number class with overflow detection
   Copyright (C) 2005-2023 Antonio Diaz Diaz.

   This library is free software. Redistribution and use in source and
   binary forms, with or without modification, are permitted provided
   that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
   notice, this list of conditions, and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions, and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <string>

#include "rational.h"

#ifndef LLONG_MAX
#define LLONG_MAX 0x7FFFFFFFFFFFFFFFLL
#endif
#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX - 1LL)
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX 0xFFFFFFFFFFFFFFFFULL
#endif


namespace {

long long gcd( long long n, long long m )	// Greatest Common Divisor
  {
  if( n < 0 ) n = -n;
  if( m < 0 ) m = -m;

  while( true )
    {
    if( m ) n %= m; else return n;
    if( n ) m %= n; else return m;
    }
  }


std::string overflow_string( const int n )
  { if( n > 0 ) return "+INF"; if( n < 0 ) return "-INF"; return "NAN"; }

int overflow_value( const long long n, const bool negate = false )
  {
  if( negate )
    { if( n > 0 ) return -INT_MAX; if( n < 0 ) return INT_MAX; return 0; }
  else
    { if( n > 0 ) return INT_MAX; if( n < 0 ) return -INT_MAX; return 0; }
  }

} // end namespace


void Rational::normalize( long long n, long long d )
  {
  if( d == 0 ) { num = overflow_value( n ); den = 0; return; }  // set error
  if( n == 0 ) { num = 0; den = 1; return; }
  if( d != 1 )
    {
    const long long tmp = gcd( n, d );
    n /= tmp; d /= tmp;
    }

  if( n <= INT_MAX && n >= -INT_MAX && d <= INT_MAX && d >= -INT_MAX )
    { if( d >= 0 ) { num = n; den = d; } else { num = -n; den = -d; } }
  else
    { num = overflow_value( n, d < 0 ); den = 0; }
  }


void Rational::normalize()
  {
  if( den == 0 ) { num = overflow_value( num ); return; }
  if( num == 0 ) { den = 1; return; }
  if( den != 1 )
    {
    const int tmp = gcd( num, den );
    num /= tmp; den /= tmp;
    }
  if( num < -INT_MAX )
    { num = overflow_value( den, true ); den = 0; return; }
  if( den < 0 )
    {
    if( den < -INT_MAX )
      { num = overflow_value( num, true ); den = 0; return; }
    num = -num; den = -den;
    }
  }


Rational Rational::inverse() const
  {
  if( den <= 0 ) return *this;			// no op on error
  Rational tmp;
  if( num > 0 ) { tmp.num = den; tmp.den = num; }
  else if( num < 0 ) { tmp.num = -den; tmp.den = -num; }
  else { tmp.num = overflow_value( den ); tmp.den = 0; }	// set error
  return tmp;
  }


Rational & Rational::operator+=( const Rational & r )
  {
  if( den <= 0 ) return *this;			// no op on error
  if( r.den <= 0 ) { num = r.num; den = 0; return *this; }	// set error

  long long new_den = den; new_den *= r.den;
  long long new_num1 = num; new_num1 *= r.den;
  long long new_num2 = r.num; new_num2 *= den;
  normalize( new_num1 + new_num2, new_den );
  return *this;
  }


Rational & Rational::operator*=( const Rational & r )
  {
  if( den <= 0 ) return *this;			// no op on error
  if( r.den <= 0 ) { num = r.num; den = 0; return *this; }	// set error

  long long new_num = num; new_num *= r.num;
  long long new_den = den; new_den *= r.den;
  normalize( new_num, new_den );
  return *this;
  }


int Rational::round() const
  {
  if( den <= 0 ) return num;
  int result = num / den;
  const int rest = std::abs( num ) % den;
  if( rest > 0 && rest >= den - rest )
    { if( num >= 0 ) ++result; else --result; }
  return result;
  }


/* Recognized formats: 123 123/456 123.456 .123 12% 12/3% 12.3% .12%
   Values may be preceded by an optional '+' or '-' sign.
   Return the number of chars read from 's', or 0 if input is invalid.
   In case of invalid input, the Rational is not changed.
*/
int Rational::parse( const char * const s )
  {
  if( !s || !s[0] ) return 0;
  long long n = 0, d = 1;		// restrain intermediate overflow
  int c = 0;
  bool minus = false;

  while( std::isspace( s[c] ) ) ++c;
  if( s[c] == '+' ) ++c;
  else if( s[c] == '-' ) { ++c; minus = true; }
  if( !std::isdigit( s[c] ) && s[c] != '.' ) return 0;

  while( std::isdigit( s[c] ) )
    {
    if( ( LLONG_MAX - (s[c] - '0') ) / 10 < n ) return 0;
    n = (n * 10) + (s[c] - '0'); ++c;
    }

  if( s[c] == '.' )
    {
    ++c; if( !std::isdigit( s[c] ) ) return 0;
    while( std::isdigit( s[c] ) )
      {
      if( ( LLONG_MAX - (s[c] - '0') ) / 10 < n || LLONG_MAX / 10 < d )
        return 0;
      n = (n * 10) + (s[c] - '0'); d *= 10; ++c;
      }
    }
  else if( s[c] == '/' )
    {
    ++c; d = 0;
    while( std::isdigit( s[c] ) )
      {
      if( ( LLONG_MAX - (s[c] - '0') ) / 10 < d ) return 0;
      d = (d * 10) + (s[c] - '0'); ++c;
      }
    if( d == 0 ) return 0;
    }

  if( s[c] == '%' )
    {
    ++c;
    if( n % 100 == 0 ) n /= 100;
    else if( n % 10 == 0 && LLONG_MAX / 10 >= d ) { n /= 10; d *= 10; }
    else if( LLONG_MAX / 100 >= d ) d *= 100;
    else return 0;
    }

  if( minus ) n = -n;
  Rational tmp; tmp.normalize( n, d );
  if( !tmp.error() ) { *this = tmp; return c; }
  return 0;
  }


/* Return a string representing the value 'num/den' in decimal point format
   with 'prec' decimals.
   'iwidth' is the minimum width of the integer part, prefixed with spaces
   if needed.
   If 'prec' is negative, produce only the decimals needed.
   If 'rounding', round up the last digit if the next one would be >= 5.
*/
std::string Rational::to_decimal( const unsigned iwidth, int prec,
                                  const bool rounding ) const
  {
  if( den <= 0 ) return overflow_string( num );

  std::string s;
  int ipart = std::abs( num / den );
  const bool truncate = ( prec < 0 );
  if( prec < 0 ) prec = -prec;

  do { s += ( ipart % 10 ) + '0'; ipart /= 10; } while( ipart > 0 );
  if( num < 0 ) s += '-';
  if( iwidth > s.size() ) s.append( iwidth - s.size(), ' ' );
  std::reverse( s.begin(), s.end() );
  long long rest = std::abs( num ) % den;
  if( prec > 0 && ( rest > 0 || !truncate ) )
    {
    s += '.';
    while( prec > 0 && ( rest > 0 || !truncate ) )
      { rest *= 10; s += ( rest / den ) + '0'; rest %= den; --prec; }
    }
  if( rounding && rest * 2 >= den )		// round last decimal up
    for( int j = s.size() - 1; j >= 0; --j )
      {
      if( s[j] == '.' ) continue;
      if( s[j] >= '0' && s[j] < '9' ) { ++s[j]; break; }
      if( s[j] == '9' ) s[j] = '0';
      if( j > 0 && s[j-1] == '.' ) continue;
      if( j > 0 && s[j-1] == ' ' ) { s[j-1] = '1'; break; }
      if( j > 1 && s[j-2] == ' ' && s[j-1] == '-' )
        { s[j-2] = '-'; s[j-1] = '1'; break; }
      // no prev digit, prepend '1' to the first digit
      if( j == 0 || s[j-1] < '0' || s[j-1] > '9' )
        { s.insert( s.begin() + j, '1' ); break; }
      }
  return s;
  }


/* Return a string representing the value 'num/den' in fractional form.
   'width' is the minimum width to be produced, prefixed with spaces if
   needed.
*/
std::string Rational::to_fraction( const unsigned width ) const
  {
  if( den <= 0 ) return overflow_string( num );

  std::string s;
  int n = std::abs( num ), d = den;

  do { s += ( d % 10 ) + '0'; d /= 10; } while( d > 0 );
  s += '/';
  do { s += ( n % 10 ) + '0'; n /= 10; } while( n > 0 );
  if( num < 0 ) s += '-';
  if( width > s.size() ) s.append( width - s.size(), ' ' );
  std::reverse( s.begin(), s.end() );
  return s;
  }
