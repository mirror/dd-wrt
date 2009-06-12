//=================================================================
//
//        i18nmb.c
//
//        General testcase for C library multibyte functions
//
//=================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jjohnstn
// Contributors:  
// Date:          2000-11-24
// Description:   Contains general testcode for C library multibyte character functions
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <pkgconf/libc_i18n.h>     // Configuration header
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <cyg/infra/testcase.h>
#include <string.h>

// FUNCTIONS

int
main( int argc, char *argv[] )
{

#ifdef CYGFUN_LIBC_I18N_LOCALE_C_JIS
  unsigned char in_jis[] =
    "abcd\x1b$B\x23\x25\x45\x46\x1b(B23";
  unsigned char fmt_jis[] = 
    "abcd\x1b$B\x23\x25\x45\x46\x1b(B%d";
  
  unsigned char * testjis[] = { 
    "abcd\x1b$B\x23\x25\x45\x46\x1b(B",
    "abcd\x1b$B\x23\x25\x1b(Befg",
    "\x1b$B\x23\x7f\x1b(B",
    "\x1b$B\x44\x45\x46\x1b(B",
    "\x1b$B",
    "\x1b$B\x1b(B",
    "\x1b$B\x44\x45",
  };

  int mblenjis0[] = { 1, 1, 1, 1, 5, 5, 0};
  int mblenjis1[] = { 1, 1, 1, 1, 8, 1, 1, 1, 0 };
  int mblenjis2[] = { -1 };
  int mblenjis3[] = { 5, -1 };
  int mblenjis4[] = { -1 };
  int mblenjis5[] = { -1 };
  int mblenjis6[] = { -1 };

  int *mblenjis[] = {
    mblenjis0,
    mblenjis1,
    mblenjis2,
    mblenjis3,
    mblenjis4,
    mblenjis5,
    mblenjis6,
  };

  wchar_t mbtowcjis0[] = { 
    (wchar_t)'a', 
    (wchar_t)'b',
    (wchar_t)'c',
    (wchar_t)'d',
    (wchar_t)0x2325,
    (wchar_t)0x4546,
    (wchar_t)0
  };
#endif /* CYGFUN_LIBC_I18N_LOCALE_C_JIS */

#ifdef CYGFUN_LIBC_I18N_LOCALE_C_SJIS
  unsigned char in_sjis[] = 
    "ab\x81\x40\x9f\x7e\xe0\x80\xef\xfcgh23";
  unsigned char fmt_sjis[] = 
    "ab\x81\x40\x9f\x7e\xe0\x80\xef\xfcgh%d";

  unsigned char * testsjis[] = {
    "ab\x81\x40\x9f\x7e\xe0\x80\xef\xfcgh",
    "\x80\x40",
    "\xa0\x40",
    "\xdf\x40",
    "\xf0\x40",
    "\x84\x3f",
    "\x85\x7f",
    "\x86\xfd",
    "\x81",
  };

  int mblensjis0[] = { 1, 1, 2, 2, 2, 2, 1, 1, 0};
  int mblensjis1[] = { 1, 1, 0 };
  int mblensjis2[] = { 1, 1, 0 };
  int mblensjis3[] = { 1, 1, 0 };
  int mblensjis4[] = { 1, 1, 0 };
  int mblensjis5[] = { -1 };
  int mblensjis6[] = { -1 };
  int mblensjis7[] = { -1 };
  int mblensjis8[] = { -1 };

  int *mblensjis[] = {
    mblensjis0,
    mblensjis1,
    mblensjis2,
    mblensjis3,
    mblensjis4,
    mblensjis5,
    mblensjis6,
    mblensjis7,
    mblensjis8,
  };

  wchar_t mbtowcsjis0[] = { 
    (wchar_t)'a', 
    (wchar_t)'b',
    (wchar_t)0x8140,
    (wchar_t)0x9f7e,
    (wchar_t)0xe080,
    (wchar_t)0xeffc,
    (wchar_t)'g',
    (wchar_t)'h',
    (wchar_t)0
  };
#endif /* CYGFUN_LIBC_I18N_LOCALE_C_SJIS */
    
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_EUCJP
  unsigned char in_eucjp[] = 
    "ab\xa1\xa1\xa2\xfe\xe0\xfd\xef\xfcgh23";
  unsigned char fmt_eucjp[] = 
    "ab\xa1\xa1\xa2\xfe\xe0\xfd\xef\xfcgh%d";
 
  unsigned char * testeucjp[] = {
    "ab\xa1\xa1\xa2\xfe\xe0\xfd\xef\xfcgh",
    "\x80\xa1",
    "\xa0\xa1",
    "\xff\xa1",
    "\x40\xa1",
    "\xa3\x3f",
    "\xa4\x7f",
    "\xa5\xff",
    "\xb1",
  };

  int mbleneucjp0[] = { 1, 1, 2, 2, 2, 2, 1, 1, 0};
  int mbleneucjp1[] = { 1, -1 };
  int mbleneucjp2[] = { 1, -1 };
  int mbleneucjp3[] = { 1, -1 };
  int mbleneucjp4[] = { 1, -1 };
  int mbleneucjp5[] = { -1 };
  int mbleneucjp6[] = { -1 };
  int mbleneucjp7[] = { -1 };
  int mbleneucjp8[] = { -1 };

  int *mbleneucjp[] = {
    mbleneucjp0,
    mbleneucjp1,
    mbleneucjp2,
    mbleneucjp3,
    mbleneucjp4,
    mbleneucjp5,
    mbleneucjp6,
    mbleneucjp7,
    mbleneucjp8,
  };

  wchar_t mbtowceucjp0[] = { 
    (wchar_t)'a', 
    (wchar_t)'b',
    (wchar_t)0xa1a1,
    (wchar_t)0xa2fe,
    (wchar_t)0xe0fd,
    (wchar_t)0xeffc,
    (wchar_t)'g',
    (wchar_t)'h',
    (wchar_t)0
  };
#endif /* CYGFUN_LIBC_I18N_LOCALE_C_EUCJP */
  
  wchar_t wcbuff[100];
  char buff[100];
  
  char *ptr, *locale;
  int result, i, j, k;
  wchar_t *wcptr;

  k=k; locale=locale; // silence warnings
  
  CYG_TEST_INIT();
  
  CYG_TEST_INFO( "Starting tests from testcase " __FILE__ " for C library "
		 "<stdlib.h> multi-byte character functions" );
  
  setlocale (LC_ALL, "C");

  CYG_TEST_PASS_FAIL( MB_CUR_MAX == 1, "MB_CUR_MAX");

  CYG_TEST_PASS_FAIL( mblen(NULL, 0) == 0, "mblen(NULL, 0)");

  CYG_TEST_PASS_FAIL( mbtowc(NULL, NULL, 0) == 0, "mbtowc(NULL, NULL, 0)");

  CYG_TEST_PASS_FAIL( wctomb(NULL, 0) == 0, "wctomb(NULL, 0)");
 
  ptr = "abcdefghijklmnop";
  result = 1;
  j = 0;
  while (result > 0) 
    {
      result = mblen (ptr, MB_CUR_MAX);
      ptr += result;
      ++j;
    }

  CYG_TEST_PASS_FAIL( result == 0, "mblen (ptr, MB_CUR_MAX)");
  CYG_TEST_PASS_FAIL( j == strlen("abcdefghijklmnop") + 1, "j");

  ptr = "1234567890";
  wcptr = wcbuff;
  memset(wcbuff, 'K', 40);
  result = 1;
  while (result > 0) 
    {
      result = mbtowc (wcptr, ptr, MB_CUR_MAX);
      CYG_TEST_PASS_FAIL( *wcptr == (wchar_t)*ptr, "*wcptr");
      ptr += result;
      wcptr++;
      ++j;
    }  

  CYG_TEST_PASS_FAIL( wcptr - wcbuff == 11, "wcptr - wcbuff");
  CYG_TEST_PASS_FAIL( wcbuff[10] == 0, "wcbuff[10]");
 
  memset (buff, 'K', 40);
  ptr = buff;
  wcptr = wcbuff;
  result = 1;

  for (i = 0; i < 11; ++i)
    {
      result = wctomb (ptr, *wcptr);
      ptr += result;
      wcptr++;
    }  

  CYG_TEST_PASS_FAIL( strcmp("1234567890", buff) == 0, "strcmp(\"1234567890\", buff");

  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, "abcdef", 7);
  CYG_TEST_PASS_FAIL( result == 6, "mbstowcs (wcbuff, \"abcdef\", 7)");
  CYG_TEST_PASS_FAIL(*((char *)(&wcbuff[7])) == 'K', "*((char *)(&wcbuff[7]))");
  CYG_TEST_PASS_FAIL( wcbuff[6] == 0, "wcbuff[6]"); 

  memset(buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 20);
  CYG_TEST_PASS_FAIL( result == 6, "wcstombs (buff, wcbuff, 20)");
  CYG_TEST_PASS_FAIL( strcmp(buff, "abcdef") == 0, "strcmp(buff, \"abcdef\")");
   
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_JIS

  CYG_TEST_INFO( "Testing C-JIS locale" );

  locale = setlocale (LC_CTYPE, "C-JIS");
  CYG_TEST_PASS_FAIL( locale != NULL, "setlocale (LC_CTYPE, \"C-JIS\")");
  
  CYG_TEST_PASS_FAIL( strcmp(locale, "C-JIS") == 0, "strcmp(locale, \"C-JIS\")");

  locale = setlocale (LC_CTYPE, NULL);
  CYG_TEST_PASS_FAIL( strcmp(locale, "C-JIS") == 0, "setlocale(LC_CTYPE, NULL)");

  CYG_TEST_PASS_FAIL( MB_CUR_MAX == 8, "MB_CUR_MAX");

  CYG_TEST_PASS_FAIL( mblen(NULL, 0) != 0, "mblen(NULL, 0)");

  CYG_TEST_PASS_FAIL( mbtowc(NULL, NULL, 0) != 0, "mbtowc(NULL, NULL, 0)");

  CYG_TEST_PASS_FAIL( wctomb(NULL, 0) != 0, "wctomb(NULL, 0)");

  for (i = 0; i < sizeof(testjis) / sizeof(char *); ++i)
    {
      ptr = testjis[i];
      result = 1;
      j = 0;
      while (result > 0) 
        {
          result = mblen (ptr, MB_CUR_MAX);
	  CYG_TEST_PASS_FAIL( result == mblenjis[i][j], "mblen (ptr, MB_CUR_MAX)");
          ptr += result;
          ++j;
        }  
    }
  
  ptr = testjis[0];
  wcptr = wcbuff;
  memset(wcbuff, 'K', 40);
  result = 1;
  j = 0;
  while (result > 0) 
    {
      result = mbtowc (wcptr, ptr, MB_CUR_MAX);
      CYG_TEST_PASS_FAIL( result == mblenjis0[j], "mbtowc (wcptr, ptr, MB_CUR_MAX)");
      CYG_TEST_PASS_FAIL( *wcptr == mbtowcjis0[j], "*wcptr");
      ptr += result;
      wcptr++;
      ++j;
    }  

  CYG_TEST_PASS_FAIL( wcptr - wcbuff == 7, "wcptr - wcbuff");
  CYG_TEST_PASS_FAIL( wcbuff[6] == 0, "wcbuff[6]");
 
  memset (buff, 'K', 40);
  ptr = buff;
  wcptr = wcbuff;
  result = 1;

  for (i = 0; i < 7; ++i)
    {
      result = wctomb (ptr, *wcptr);
      ptr += result;
      wcptr++;
    }  

  CYG_TEST_PASS_FAIL( strcmp(testjis[0], buff) == 0, "strcmp(testjis[0], buff");

  result = wctomb (buff, (wchar_t)0x1111);
  CYG_TEST_PASS_FAIL( result == -1, "wctomb(buff, (wchar_t)0x1111)");

  result = mbstowcs (wcbuff, testjis[3], 8);
  CYG_TEST_PASS_FAIL( result == -1, "mbstowcs(wcbuff, testjis[3], 8)");
  
  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testjis[0], 8);
  CYG_TEST_PASS_FAIL( result == 6, "mbstowcs(wcbuff, testjis[0], 8)");
  CYG_TEST_PASS_FAIL( *((char *)(&wcbuff[7])) == 'K', "*((char *)(&wcbuff[7]))");
  CYG_TEST_PASS_FAIL( wcbuff[6] == 0, "wcbuff[6]");

  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testjis[0], 6);
  CYG_TEST_PASS_FAIL( result == 6, "mbstowcs (wcbuff, testjis[0], 6)");
  CYG_TEST_PASS_FAIL( *((char *)(&wcbuff[6])) == 'K', "*((char *)(&wcbuff[6]))");

  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testjis[0], 7);
  CYG_TEST_PASS_FAIL( result == 6, "mbstowcs (wcbuff, testjis[0], 7)");
  CYG_TEST_PASS_FAIL(*((char *)(&wcbuff[7])) == 'K', "*((char *)(&wcbuff[7]))");
  CYG_TEST_PASS_FAIL( wcbuff[6] == 0, "wcbuff[6]"); 

  memset(buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 20);
  CYG_TEST_PASS_FAIL( result == 14, "wcstombs (buff, wcbuff, 20)");
  CYG_TEST_PASS_FAIL( strcmp(buff, testjis[0]) == 0, "strcmp(buff, testjis[0])");

  memset (buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 14);
  CYG_TEST_PASS_FAIL( result == 14, "wcstombs (buff, wcbuff, 14)");
  CYG_TEST_PASS_FAIL( memcmp (buff, testjis[0], 14) == 0, "memcmp (buff, testjis[0], 14)");
  CYG_TEST_PASS_FAIL( buff[14] == 'K', "buff[14]");

  memset (buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 15);
  CYG_TEST_PASS_FAIL( result == 14, "wcstombs (buff, wcbuff, 15)");
  CYG_TEST_PASS_FAIL( memcmp (buff, testjis[0], 14) == 0, "memcmp (buff, testjis[0], 14)");
  CYG_TEST_PASS_FAIL( buff[14] == '\0', "buff[14]");

  result = wcstombs (buff, (wchar_t *)"\x23\x10", 2);
  CYG_TEST_PASS_FAIL( result == -1, "wcstombs (buff, (wchar_t *)\"\x23\x10\", 2)");

  result = sscanf (in_jis, fmt_jis, &k);
  CYG_TEST_PASS_FAIL( result == 1, "sscanf (in_jis, fmt_jis, &k)");
  CYG_TEST_PASS_FAIL( k == 23, "k"); 

  sprintf (buff, fmt_jis, 23);
  CYG_TEST_PASS_FAIL( strcmp(buff, in_jis) == 0, "sprintf(buff, fmt_jis, 23)");

  locale = setlocale(LC_ALL, "C");
  locale = setlocale(LC_CTYPE, NULL);

  CYG_TEST_PASS_FAIL( strcmp(locale, "C") == 0, "previous locale");
  CYG_TEST_PASS_FAIL( MB_CUR_MAX == 1, "MB_CUR_MAX");

#endif /* CYGFUN_LIBC_I18N_LOCALE_C_JIS */

#ifdef CYGFUN_LIBC_I18N_LOCALE_C_SJIS

  CYG_TEST_INFO( "Testing C-SJIS locale" );

  locale = setlocale (LC_CTYPE, "C-SJIS");
  CYG_TEST_PASS_FAIL( locale != NULL, "setlocale (LC_CTYPE, \"C-SJIS\")");
  
  CYG_TEST_PASS_FAIL( strcmp(locale, "C-SJIS") == 0, "setlocale(LC_CTYPE, \"C-SJIS\")");

  locale = setlocale (LC_CTYPE, NULL);
  CYG_TEST_PASS_FAIL( strcmp(locale, "C-SJIS") == 0, "setlocale(LC_CTYPE, NULL)");

  CYG_TEST_PASS_FAIL( MB_CUR_MAX == 2, "MB_CUR_MAX");

  CYG_TEST_PASS_FAIL( mblen(NULL, 0) == 0, "mblen(NULL, 0)");

  CYG_TEST_PASS_FAIL( mbtowc(NULL, NULL, 0) == 0, "mbtowc(NULL, NULL, 0)");

  CYG_TEST_PASS_FAIL( wctomb(NULL, 0) == 0, "wctomb(NULL, 0)");

  for (i = 0; i < sizeof(testsjis) / sizeof(char *); ++i)
    {
      ptr = testsjis[i];
      result = 1;
      j = 0;
      while (result > 0) 
        {
          result = mblen (ptr, MB_CUR_MAX);
	  CYG_TEST_PASS_FAIL( result == mblensjis[i][j], "mblen (ptr, MB_CUR_MAX)");
          ptr += result;
          ++j;
        }  
    }
  
  ptr = testsjis[0];
  wcptr = wcbuff;
  memset(wcbuff, 'K', 40);
  result = 1;
  j = 0;
  while (result > 0) 
    {
      result = mbtowc (wcptr, ptr, MB_CUR_MAX);
      CYG_TEST_PASS_FAIL( result == mblensjis0[j], "mbtowc (wcptr, ptr, MB_CUR_MAX)");
      CYG_TEST_PASS_FAIL( *wcptr == mbtowcsjis0[j], "*wcptr");
      ptr += result;
      wcptr++;
      ++j;
    }  

  CYG_TEST_PASS_FAIL( wcptr - wcbuff == 9, "wcptr - wcbuff");
  CYG_TEST_PASS_FAIL( wcbuff[8] == 0, "wcbuff[8]");
 
  memset (buff, 'K', 40);
  ptr = buff;
  wcptr = wcbuff;
  result = 1;

  for (i = 0; i < 9; ++i)
    {
      result = wctomb (ptr, *wcptr);
      ptr += result;
      wcptr++;
    }  

  CYG_TEST_PASS_FAIL( strcmp(testsjis[0], buff) == 0, "strcmp(testsjis[0], buff");

  result = wctomb (buff, (wchar_t)0x1111);
  CYG_TEST_PASS_FAIL( result == -1, "wctomb(buff, (wchar_t)0x1111)");

  result = mbstowcs (wcbuff, testsjis[5], 8);
  CYG_TEST_PASS_FAIL( result == -1, "mbstowcs(wcbuff, testjis[3], 8)");
  
  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testsjis[0], 20);
  CYG_TEST_PASS_FAIL( result == 8, "mbstowcs(wcbuff, testsjis[0], 20)");
  CYG_TEST_PASS_FAIL( *((char *)(&wcbuff[9])) == 'K', "*((char *)(&wcbuff[9]))");
  CYG_TEST_PASS_FAIL( wcbuff[8] == 0, "wcbuff[8]");

  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testsjis[0], 8);
  CYG_TEST_PASS_FAIL( result == 8, "mbstowcs (wcbuff, testsjis[0], 8)");
  CYG_TEST_PASS_FAIL( *((char *)(&wcbuff[8])) == 'K', "*((char *)(&wcbuff[8]))");

  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testsjis[0], 9);
  CYG_TEST_PASS_FAIL( result == 8, "mbstowcs (wcbuff, testsjis[0], 9)");
  CYG_TEST_PASS_FAIL(*((char *)(&wcbuff[9])) == 'K', "*((char *)(&wcbuff[9]))");
  CYG_TEST_PASS_FAIL( wcbuff[8] == 0, "wcbuff[8]"); 

  memset(buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 20);
  CYG_TEST_PASS_FAIL( result == 12, "wcstombs (buff, wcbuff, 20)");
  CYG_TEST_PASS_FAIL( strcmp(buff, testsjis[0]) == 0, "strcmp(buff, testjis[0])");

  memset (buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 12);
  CYG_TEST_PASS_FAIL( result == 12, "wcstombs (buff, wcbuff, 12)");
  CYG_TEST_PASS_FAIL( memcmp (buff, testsjis[0], 12) == 0, "memcmp (buff, testsjis[0], 12)");
  CYG_TEST_PASS_FAIL( buff[12] == 'K', "buff[12]");

  memset (buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 13);
  CYG_TEST_PASS_FAIL( result == 12, "wcstombs (buff, wcbuff, 12)");
  CYG_TEST_PASS_FAIL( memcmp (buff, testsjis[0], 13) == 0, "memcmp (buff, testsjis[0], 13)");
  CYG_TEST_PASS_FAIL( buff[12] == '\0', "buff[12]");

  result = wcstombs (buff, (wchar_t *)"\x23\x10", 2);
  CYG_TEST_PASS_FAIL( result == -1, "wcstombs (buff, (wchar_t *)\"\x23\x10\", 2)");

  result = sscanf (in_sjis, fmt_sjis, &k);
  CYG_TEST_PASS_FAIL( result == 1, "sscanf (in_sjis, fmt_sjis, &k)");
  CYG_TEST_PASS_FAIL( k == 23, "k"); 

  sprintf (buff, fmt_sjis, 23);
  CYG_TEST_PASS_FAIL( strcmp(buff, in_sjis) == 0, "sprintf(buff, fmt_sjis, 23)");

  locale = setlocale(LC_ALL, "C");
  locale = setlocale(LC_CTYPE, NULL);

  CYG_TEST_PASS_FAIL( strcmp(locale, "C") == 0, "previous locale");
  CYG_TEST_PASS_FAIL( MB_CUR_MAX == 1, "MB_CUR_MAX");

#endif /* CYGFUN_LIBC_I18N_LOCALE_C_SJIS */

#ifdef CYGFUN_LIBC_I18N_LOCALE_C_EUCJP

  CYG_TEST_INFO( "Testing C-EUCJP locale" );

  locale = setlocale (LC_CTYPE, "C-EUCJP");
  CYG_TEST_PASS_FAIL( locale != NULL, "setlocale (LC_CTYPE, \"C-EUCJP\")");
  
  CYG_TEST_PASS_FAIL( strcmp(locale, "C-EUCJP") == 0, "setlocale(LC_CTYPE, \"C-EUCJP\")");

  locale = setlocale (LC_CTYPE, NULL);
  CYG_TEST_PASS_FAIL( strcmp(locale, "C-EUCJP") == 0, "current locale");

  CYG_TEST_PASS_FAIL( MB_CUR_MAX == 2, "MB_CUR_MAX");

  CYG_TEST_PASS_FAIL( mblen(NULL, 0) == 0, "mblen(NULL, 0)");

  CYG_TEST_PASS_FAIL( mbtowc(NULL, NULL, 0) == 0, "mbtowc(NULL, NULL, 0)");

  CYG_TEST_PASS_FAIL( wctomb(NULL, 0) == 0, "wctomb(NULL, 0)");

  for (i = 0; i < sizeof(testeucjp) / sizeof(char *); ++i)
    {
      ptr = testeucjp[i];
      result = 1;
      j = 0;
      while (result > 0) 
        {
          result = mblen (ptr, MB_CUR_MAX);
	  CYG_TEST_PASS_FAIL( result == mbleneucjp[i][j], "mblen (ptr, MB_CUR_MAX)");
          ptr += result;
          ++j;
        }  
    }
  
  ptr = testeucjp[0];
  wcptr = wcbuff;
  memset(wcbuff, 'K', 40);
  result = 1;
  j = 0;
  while (result > 0) 
    {
      result = mbtowc (wcptr, ptr, MB_CUR_MAX);
      CYG_TEST_PASS_FAIL( result == mbleneucjp0[j], "mbtowc (wcptr, ptr, MB_CUR_MAX)");
      CYG_TEST_PASS_FAIL( *wcptr == mbtowceucjp0[j], "*wcptr");
      ptr += result;
      wcptr++;
      ++j;
    }  

  CYG_TEST_PASS_FAIL( wcptr - wcbuff == 9, "wcptr - wcbuff");
  CYG_TEST_PASS_FAIL( wcbuff[8] == 0, "wcbuff[8]");
 
  memset (buff, 'K', 40);
  ptr = buff;
  wcptr = wcbuff;
  result = 1;

  for (i = 0; i < 9; ++i)
    {
      result = wctomb (ptr, *wcptr);
      ptr += result;
      wcptr++;
    }  

  CYG_TEST_PASS_FAIL( strcmp(testeucjp[0], buff) == 0, "strcmp(testeucjp[0], buff");

  result = wctomb (buff, (wchar_t)0x1111);
  CYG_TEST_PASS_FAIL( result == -1, "wctomb(buff, (wchar_t)0x1111)");

  result = mbstowcs (wcbuff, testeucjp[5], 8);
  CYG_TEST_PASS_FAIL( result == -1, "mbstowcs(wcbuff, testjis[3], 8)");
  
  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testeucjp[0], 20);
  CYG_TEST_PASS_FAIL( result == 8, "mbstowcs(wcbuff, testeucjp[0], 20)");
  CYG_TEST_PASS_FAIL( *((char *)(&wcbuff[9])) == 'K', "*((char *)(&wcbuff[9]))");
  CYG_TEST_PASS_FAIL( wcbuff[8] == 0, "wcbuff[8]");

  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testeucjp[0], 8);
  CYG_TEST_PASS_FAIL( result == 8, "mbstowcs (wcbuff, testeucjp[0], 8)");
  CYG_TEST_PASS_FAIL( *((char *)(&wcbuff[8])) == 'K', "*((char *)(&wcbuff[8]))");

  memset (wcbuff, 'K', 40);
  result = mbstowcs (wcbuff, testeucjp[0], 9);
  CYG_TEST_PASS_FAIL( result == 8, "mbstowcs (wcbuff, testeucjp[0], 9)");
  CYG_TEST_PASS_FAIL(*((char *)(&wcbuff[9])) == 'K', "*((char *)(&wcbuff[9]))");
  CYG_TEST_PASS_FAIL( wcbuff[8] == 0, "wcbuff[8]"); 

  memset(buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 20);
  CYG_TEST_PASS_FAIL( result == 12, "wcstombs (buff, wcbuff, 20)");
  CYG_TEST_PASS_FAIL( strcmp(buff, testeucjp[0]) == 0, "strcmp(buff, testjis[0])");

  memset (buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 12);
  CYG_TEST_PASS_FAIL( result == 12, "wcstombs (buff, wcbuff, 12)");
  CYG_TEST_PASS_FAIL( memcmp (buff, testeucjp[0], 12) == 0, "memcmp (buff, testeucjp[0], 12)");
  CYG_TEST_PASS_FAIL( buff[12] == 'K', "buff[12]");

  memset (buff, 'K', 20);
  result = wcstombs (buff, wcbuff, 13);
  CYG_TEST_PASS_FAIL( result == 12, "wcstombs (buff, wcbuff, 12)");
  CYG_TEST_PASS_FAIL( memcmp (buff, testeucjp[0], 13) == 0, "memcmp (buff, testeucjp[0], 13)");
  CYG_TEST_PASS_FAIL( buff[12] == '\0', "buff[12]");

  result = wcstombs (buff, (wchar_t *)"\x23\x10", 2);
  CYG_TEST_PASS_FAIL( result == -1, "wcstombs (buff, (wchar_t *)\"\x23\x10\", 2)");

  result = sscanf (in_eucjp, fmt_eucjp, &k);
  CYG_TEST_PASS_FAIL( result == 1, "sscanf (in_eucjp, fmt_eucjp, &k)");
  CYG_TEST_PASS_FAIL( k == 23, "k"); 

  sprintf (buff, fmt_eucjp, 23);
  CYG_TEST_PASS_FAIL( strcmp(buff, in_eucjp) == 0, "sprintf(buff, fmt_eucjp, 23)");

  locale = setlocale(LC_ALL, "C");
  locale = setlocale(LC_CTYPE, NULL);

  CYG_TEST_PASS_FAIL( strcmp(locale, "C") == 0, "previous locale");
  CYG_TEST_PASS_FAIL( MB_CUR_MAX == 1, "MB_CUR_MAX");

#endif /* CYGFUN_LIBC_I18N_LOCALE_C_EUCJP */


  CYG_TEST_FINISH( "Finished tests from testcase " __FILE__ " for C library "
                     "<stdlib.h> multibyte character functions" );
  
  return 0;

}  
