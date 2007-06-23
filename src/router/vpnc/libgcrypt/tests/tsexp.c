/* tsexp.c  -  S-expression regression tests
 *	Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../src/gcrypt.h"

#define PGMNAME "tsexp"

static int verbose;
static int error_count;

static void
info (const char *format, ...)
{
  va_list arg_ptr;

  if (verbose)
    {
      va_start( arg_ptr, format ) ;
      vfprintf (stderr, format, arg_ptr );
      va_end(arg_ptr);
    }
}

static void
fail ( const char *format, ... )
{
    va_list arg_ptr ;

    fputs (PGMNAME ": ", stderr);
    va_start( arg_ptr, format ) ;
    vfprintf (stderr, format, arg_ptr );
    va_end(arg_ptr);
    error_count++;
}


/* fixme: we need better tests */
static void
basic (void)
{
  int pass;
  gcry_sexp_t sexp;
  int idx;
  char *secure_buffer;
  size_t secure_buffer_len;
  const char *string;
  static struct {
    const char *token;
    const char *parm;
  } values[] = {
    { "public-key", NULL },
    { "dsa", NULL },
    { "dsa", "p" },
    { "dsa", "y" },
    { "dsa", "q" },
    { "dsa", "g" },
    { NULL }
  };

  info ("doing some pretty pointless tests\n"); 

  secure_buffer_len = 99;
  secure_buffer = gcry_xmalloc_secure (secure_buffer_len);
  memset (secure_buffer, 'G', secure_buffer_len);

  for (pass=0;;pass++)
    {
      switch (pass)
        {
        case 0:
          string = ("(public-key (dsa (p #41424344#) (y this_is_y) "
                    "(q #61626364656667#) (g %m)))");
          
          if ( gcry_sexp_build (&sexp, NULL, string,
                                gcry_mpi_set_ui (NULL, 42)) )
            {
              fail (" scanning `%s' failed\n", string);
              return;
            }
          break;
          
        case 1:
          string = ("(public-key (dsa (p #41424344#) (y this_is_y) "
                    "(q %b) (g %m)))");
      
          if ( gcry_sexp_build (&sexp, NULL, string, 
                                15, "foo\0\x01\0x02789012345",
                                gcry_mpi_set_ui (NULL, 42)) )
            {
              fail (" scanning `%s' failed\n", string);
              return;
            }
          break;

        case 2:
          string = ("(public-key (dsa (p #41424344#) (y silly_y_value) "
                    "(q %b) (g %m)))");
      
          if ( gcry_sexp_build (&sexp, NULL, string, 
                                secure_buffer_len, secure_buffer,
                                gcry_mpi_set_ui (NULL, 17)) )
            {
              fail (" scanning `%s' failed\n", string);
              return;
            }
          if (!gcry_is_secure (sexp))
            fail ("gcry_sexp_build did not switch to secure memory\n");
          break;
          
        default:
          return; /* Ready. */
        }


      /* now find something */
      for (idx=0; values[idx].token; idx++)
        {
          const char *token = values[idx].token;
          const char *parm = values[idx].parm;
          gcry_sexp_t s1, s2;
          gcry_mpi_t a;
          const char *p;
          size_t n;

          s1 = gcry_sexp_find_token (sexp, token, strlen(token) );
          if (!s1)
            {
              fail ("didn't found `%s'\n", token);
              continue;
            }

          p = gcry_sexp_nth_data (s1, 0, &n);
          if (!p)
            {
              fail ("no car for `%s'\n", token);
              continue;
            }
          info ("car=`%.*s'\n", (int)n, p);

          s2 = gcry_sexp_cdr (s1);
          if (!s2) 
            {
              fail ("no cdr for `%s'\n", token);
              continue;
            }

          p = gcry_sexp_nth_data (s2, 0, &n);
          if (p)
            {
              fail ("data at car of `%s'\n", token);
              continue;
            }

          if (parm)
            {
              s2 = gcry_sexp_find_token (s1, parm, strlen (parm));
              if (!s2)
                {
                  fail ("didn't found `%s'\n", parm);
                  continue;
                }
              p = gcry_sexp_nth_data (s2, 0, &n);
              if (!p) 
                {
                  fail("no car for `%s'\n", parm );
                  continue;
                }
              info ("car=`%.*s'\n", (int)n, p);
              p = gcry_sexp_nth_data (s2, 1, &n);
              if (!p) 
                {
                  fail("no cdr for `%s'\n", parm );
                  continue;
                }
              info ("cdr=`%.*s'\n", (int)n, p);
          
              a = gcry_sexp_nth_mpi (s2, 0, GCRYMPI_FMT_USG);
              if (!a)
                {
                  fail("failed to cdr the mpi for `%s'\n", parm);
                  continue;
                }
            }
        }
     
      gcry_sexp_release (sexp);
      sexp = NULL;
    }
  gcry_free (secure_buffer);
}


static void
canon_len (void)
{
  static struct {
    size_t textlen; /* length of the buffer */
    size_t expected;/* expected length or 0 on error and then ... */
    size_t erroff;  /* ... and at this offset */
    gcry_error_t errcode;    /* ... with this error code */
    char *text; 
  } values[] = {
    { 14, 13, 0, GPG_ERR_NO_ERROR, "(9:abcdefghi) " },
    { 16, 15, 0, GPG_ERR_NO_ERROR, "(10:abcdefghix)" },
    { 14,  0,14, GPG_ERR_SEXP_STRING_TOO_LONG, "(10:abcdefghi)" },
    { 15,  0, 1, GPG_ERR_SEXP_ZERO_PREFIX, "(010:abcdefghi)" },
    {  2,  0, 0, GPG_ERR_SEXP_NOT_CANONICAL, "1:"},
    {  4,  0, 4, GPG_ERR_SEXP_STRING_TOO_LONG, "(1:)"},
    {  5,  5, 0, GPG_ERR_NO_ERROR, "(1:x)"},
    {  2,  2, 0, GPG_ERR_NO_ERROR, "()"},
    {  4,  2, 0, GPG_ERR_NO_ERROR, "()()"},
    {  4,  4, 0, GPG_ERR_NO_ERROR, "(())"},
    {  3,  0, 3, GPG_ERR_SEXP_STRING_TOO_LONG, "(()"},
    {  3,  0, 1, GPG_ERR_SEXP_BAD_CHARACTER, "( )"},
    {  9,  9, 0, GPG_ERR_NO_ERROR, "(3:abc())"},
    { 10,  0, 6, GPG_ERR_SEXP_BAD_CHARACTER, "(3:abc ())"},
    /* fixme: we need much more cases */
    { 0 },
  };
  int idx;
  gcry_error_t errcode;
  size_t n, erroff;

  info ("checking canoncial length test function\n");
  for (idx=0; values[idx].text; idx++)
    {
      n = gcry_sexp_canon_len ((const unsigned char*)values[idx].text,
                               values[idx].textlen, 
                               &erroff, &errcode);
      
      if (n && n == values[idx].expected)
        ; /* success */
      else if (!n && !values[idx].expected)
        { /* we expected an error - check that this is the right one */
          if (values[idx].erroff != erroff)
            fail ("canonical length test %d - wrong error offset %u\n",
                  idx, (unsigned int)erroff);
          if (gcry_err_code (errcode) != values[idx].errcode)
            fail ("canonical length test %d - wrong error code %d\n",
                  idx, errcode);
        }
      else
        fail ("canonical length test %d failed - n=%u, off=%u, err=%d\n",
              idx, (unsigned int)n, (unsigned int)erroff, errcode);
    }
}


static void
back_and_forth_one (int testno, const char *buffer, size_t length)
{
  gcry_error_t rc;
  gcry_sexp_t se, se1;
  size_t n, n1;
  char *p1;

  rc = gcry_sexp_new (&se, buffer, length, 1);
  if (rc)
    {
      fail ("baf %d: gcry_sexp_new failed: %s\n", testno, gpg_strerror (rc));
      return;
    }
  n1 = gcry_sexp_sprint (se, GCRYSEXP_FMT_CANON, NULL, 0);
  if (!n1)
    {
      fail ("baf %d: get required length for canon failed\n", testno);
      return;
    }
  p1 = gcry_xmalloc (n1);
  n = gcry_sexp_sprint (se, GCRYSEXP_FMT_CANON, p1, n1);
  if (n1 != n+1) /* sprints adds an extra 0 but dies not return it */
    {
      fail ("baf %d: length mismatch for canon\n", testno);
      return;
    }
  rc = gcry_sexp_create (&se1, p1, n, 0, gcry_free);
  if (rc)
    {
      fail ("baf %d: gcry_sexp_create failed: %s\n",
            testno, gpg_strerror (rc));
      return;
    }
  gcry_sexp_release (se1);
  
  /* FIXME: we need a lot more tests */

  gcry_sexp_release (se);
}



static void
back_and_forth (void)
{
  static struct { char *buf; int len; } tests[] = {
    { "(7:g34:fgh1::2:())", 0 },
    { "(7:g34:fgh1::2:())", 18 },
    { NULL, 0 }
  };
  int idx;

  for (idx=0; tests[idx].buf; idx++)
    back_and_forth_one (idx, tests[idx].buf, tests[idx].len);
}


static void
check_sscan (void)
{
  static struct {
    const char *text;
    gcry_error_t expected_err;  
  } values[] = {
    /* Bug reported by Olivier L'Heureux 2003-10-07 */
    { "(7:sig-val(3:dsa"
      "(1:r20:\x7e\xff\xd5\xba\xc9\xc9\xa4\x9b\xd4\x26\x8b\x64"
      "\x06\x7a\xcf\x42\x7b\x6c\x51\xfb)"
      "(1:s21:\x01\x8c\x6c\x6f\x37\x1a\x8d\xfd\x5a\xb3\x2a\x3d"
      "\xc5\xae\x23\xed\x32\x62\x30\x62\x3e)))",
      GPG_ERR_NO_ERROR },
    { "(7:sig-val(3:dsa"
      "(1:r20:\x7e\xff\xd5\xba\xc9\xc9\xa4\x9b\xd4\x26\x8b\x64"
      "\x06\x7a\xcf\x42\x7b\x6c\x51\xfb)"
      "(1:s21:\x01\x8c\x6c\x6f\x37\x1a\x8d\xfd\x5a\xb3\x2a\x3d"
      "\xc5\xae\x23\xed\x32\x62\x30\x62\x3e))",
      GPG_ERR_SEXP_UNMATCHED_PAREN },
    { "(7:sig-val(3:dsa"
      "(1:r20:\x7e\xff\xd5\xba\xc9\xc9\xa4\x9b\xd4\x26\x8b\x64"
      "\x06\x7a\xcf\x42\x7b\x6c\x51\xfb)"
      "(1:s21:\x01\x8c\x6c\x6f\x37\x1a\x8d\xfd\x5a\xb3\x2a\x3d"
      "\xc5\xae\x23\xed\x32\x62\x30\x62\x3e))))",
      GPG_ERR_SEXP_UNMATCHED_PAREN },
    { NULL, 0 }
  };
  int idx;
  gcry_error_t err;
  gcry_sexp_t s;

  info ("checking gcry_sexp_sscan\n");
  for (idx=0; values[idx].text; idx++)
    {
      err = gcry_sexp_sscan (&s, NULL,
                             values[idx].text,
                             strlen (values[idx].text));
      if (gpg_err_code (err) != values[idx].expected_err)
        fail ("gcry_sexp_sscan test %d failed: %s\n", idx, gpg_strerror (err));
      gcry_sexp_release (s);
    }
}




int
main (int argc, char **argv)
{
  if (argc > 1 && !strcmp (argv[1], "-v"))
    verbose = 1;

  gcry_control (GCRYCTL_DISABLE_SECMEM_WARN);
  gcry_control (GCRYCTL_INIT_SECMEM, 16384, 0);

  basic ();
  canon_len ();
  back_and_forth ();
  check_sscan ();
  
  return error_count? 1:0;
}






