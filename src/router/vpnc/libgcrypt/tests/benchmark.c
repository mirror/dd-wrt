/* benchmark.c - for libgcrypt
 *	Copyright (C) 2002, 2004 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef _WIN32
#include <sys/times.h>
#endif
#include <gcrypt.h>

#define PGM "benchmark"
#define BUG() do {fprintf ( stderr, "Ooops at %s:%d\n", __FILE__ , __LINE__ );\
		  exit(2);} while(0)


/* Helper for the start and stop timer. */
static clock_t started_at, stopped_at;


static void
start_timer (void)
{
#ifdef _WIN32
  started_at = stopped_at = clock ();
#else
  struct tms tmp;

  times (&tmp);
  started_at = stopped_at = tmp.tms_utime;
#endif
}

static void
stop_timer (void)
{
#ifdef _WIN32
  stopped_at = clock ();
#else
  struct tms tmp;

  times (&tmp);
  stopped_at = tmp.tms_utime;
#endif
}

static const char *
elapsed_time (void)
{
  static char buf[50];

  sprintf (buf, "%5.0fms",
           (((double) (stopped_at - started_at))/CLOCKS_PER_SEC)*10000000);
  return buf;
}


static void
random_bench (void)
{
  char buf[128];
  int i;

  printf ("%-10s", "random");

  start_timer ();
  for (i=0; i < 100; i++)
    gcry_randomize (buf, sizeof buf, GCRY_STRONG_RANDOM);
  stop_timer ();
  printf (" %s", elapsed_time ());

  start_timer ();
  for (i=0; i < 100; i++)
    gcry_randomize (buf, 8, GCRY_STRONG_RANDOM);
  stop_timer ();
  printf (" %s", elapsed_time ());

  putchar ('\n');
}



static void
md_bench ( const char *algoname )
{
  int algo;
  gcry_md_hd_t hd;
  int i;
  char buf[1000];
  gcry_error_t err = GPG_ERR_NO_ERROR;

  if (!algoname)
    {
      for (i=1; i < 400; i++)
        if ( !gcry_md_test_algo (i) )
          md_bench (gcry_md_algo_name (i));
      return;
    }

  algo = gcry_md_map_name (algoname);
  if (!algo)
    {
      fprintf (stderr, PGM ": invalid hash algorithm `%s'\n", algoname);
      exit (1);
    }

  err = gcry_md_open (&hd, algo, 0);
  if (err)
    {
      fprintf (stderr, PGM ": error opening hash algorithm `%s'\n", algoname);
      exit (1);
    }

  for (i=0; i < sizeof buf; i++)
    buf[i] = i;

  printf ("%-12s", gcry_md_algo_name (algo));

  start_timer ();
  for (i=0; i < 1000; i++)
    gcry_md_write (hd, buf, sizeof buf);
  gcry_md_final (hd);
  stop_timer ();
  printf (" %s", elapsed_time ());

  gcry_md_reset (hd);
  start_timer ();
  for (i=0; i < 10000; i++)
    gcry_md_write (hd, buf, sizeof buf/10);
  gcry_md_final (hd);
  stop_timer ();
  printf (" %s", elapsed_time ());

  gcry_md_reset (hd);
  start_timer ();
  for (i=0; i < 1000000; i++)
    gcry_md_write (hd, "", 1);
  gcry_md_final (hd);
  stop_timer ();
  printf (" %s", elapsed_time ());

  gcry_md_close (hd);
  putchar ('\n');
}

static void
cipher_bench ( const char *algoname )
{
  static int header_printed;
  int algo;
  gcry_cipher_hd_t hd;
  int i;
  int keylen, blklen;
  char key[128];
  char outbuf[1000], buf[1000];
  size_t buflen;
  static struct { int mode; const char *name; int blocked; } modes[] = {
    { GCRY_CIPHER_MODE_ECB, "ECB", 1 },
    { GCRY_CIPHER_MODE_CBC, "CBC", 1 },
    { GCRY_CIPHER_MODE_CFB, "CFB", 0 },
    { GCRY_CIPHER_MODE_CTR, "CTR", 0 },
    { GCRY_CIPHER_MODE_STREAM, "STREAM", 0 },
    {0}
  };
  int modeidx;
  gcry_error_t err = GPG_ERR_NO_ERROR;


  if (!algoname)
    {
      for (i=1; i < 400; i++)
        if ( !gcry_cipher_test_algo (i) )
          cipher_bench (gcry_cipher_algo_name (i));
      return;
    }


  if (!header_printed)
    {
      printf ("%-10s", "");
      for (modeidx=0; modes[modeidx].mode; modeidx++)
        printf (" %-15s", modes[modeidx].name );
      putchar ('\n');
      printf ("%-10s", "");
      for (modeidx=0; modes[modeidx].mode; modeidx++)
        printf (" ---------------" );
      putchar ('\n');
      header_printed = 1;
    }

  algo = gcry_cipher_map_name (algoname);
  if (!algo)
    {
      fprintf (stderr, PGM ": invalid cipher algorithm `%s'\n", algoname);
      exit (1);
    }

  keylen = gcry_cipher_get_algo_keylen (algo);
  if (!keylen)
    {
      fprintf (stderr, PGM ": failed to get key length for algorithm `%s'\n",
	       algoname);
      exit (1);
    }
  if ( keylen > sizeof key )
    {
        fprintf (stderr, PGM ": algo %d, keylength problem (%d)\n",
                 algo, keylen );
        exit (1);
    }
  for (i=0; i < keylen; i++)
    key[i] = i + (clock () & 0xff);

  blklen = gcry_cipher_get_algo_blklen (algo);
  if (!blklen)
    {
      fprintf (stderr, PGM ": failed to get block length for algorithm `%s'\n",
	       algoname);
      exit (1);
    }

  printf ("%-10s", gcry_cipher_algo_name (algo));
  fflush (stdout);

  for (modeidx=0; modes[modeidx].mode; modeidx++)
    {
      if ((blklen > 1 && modes[modeidx].mode == GCRY_CIPHER_MODE_STREAM)
          | (blklen == 1 && modes[modeidx].mode != GCRY_CIPHER_MODE_STREAM))
        {
          printf ("                " );
          continue;
        }

      for (i=0; i < sizeof buf; i++)
        buf[i] = i;

      err = gcry_cipher_open (&hd, algo, modes[modeidx].mode, 0);
      if (err)
        {
          fprintf (stderr, PGM ": error opening cipher `%s'\n", algoname);
          exit (1);
        }
      
      err = gcry_cipher_setkey (hd, key, keylen);
      if (err)
      { 
          fprintf (stderr, "gcry_cipher_setkey failed: %s\n",
		   gpg_strerror (err));
          gcry_cipher_close (hd);
          exit (1);
        }

      buflen = sizeof buf;
      if (modes[modeidx].blocked)
        buflen = (buflen / blklen) * blklen;

      start_timer ();
      for (i=err=0; !err && i < 1000; i++)
        err = gcry_cipher_encrypt ( hd, outbuf, buflen, buf, buflen);
      stop_timer ();
      printf (" %s", elapsed_time ());
      fflush (stdout);
      gcry_cipher_close (hd);
      if (err)
        { 
          fprintf (stderr, "gcry_cipher_encrypt failed: %s\n",
                   gpg_strerror (err) );
          exit (1);
        }

      err = gcry_cipher_open (&hd, algo, modes[modeidx].mode, 0);
      if (err)
        {
          fprintf (stderr, PGM ": error opening cipher `%s'/n", algoname);
          exit (1);
        }
      
      err = gcry_cipher_setkey (hd, key, keylen);
      if (err)
        { 
          fprintf (stderr, "gcry_cipher_setkey failed: %s\n",
                   gpg_strerror (err));
          gcry_cipher_close (hd);
          exit (1);
        }

      start_timer ();
      for (i=err=0; !err && i < 1000; i++)
        err = gcry_cipher_decrypt ( hd, outbuf, buflen,  buf, buflen);
      stop_timer ();
      printf (" %s", elapsed_time ());
      fflush (stdout);
      gcry_cipher_close (hd);
      if (err)
        { 
          fprintf (stderr, "gcry_cipher_decrypt failed: %s\n",
                   gpg_strerror (err) );
          exit (1);
        }
    }

  putchar ('\n');
}


static void
do_powm ( const char *n_str, const char *e_str, const char *m_str)
{
  gcry_mpi_t e, n, msg, cip;
  gcry_error_t err;
  int i;

  err = gcry_mpi_scan (&n, GCRYMPI_FMT_HEX, n_str, 0, 0);
  if (err) BUG ();
  err = gcry_mpi_scan (&e, GCRYMPI_FMT_HEX, e_str, 0, 0);
  if (err) BUG ();
  err = gcry_mpi_scan (&msg, GCRYMPI_FMT_HEX, m_str, 0, 0);
  if (err) BUG ();

  cip = gcry_mpi_new (0);

  start_timer ();
  for (i=0; i < 1000; i++)
    gcry_mpi_powm (cip, msg, e, n);
  stop_timer ();
  printf (" %s", elapsed_time ()); fflush (stdout);
/*    { */
/*      char *buf; */

/*      if (gcry_mpi_aprint (GCRYMPI_FMT_HEX, (void**)&buf, NULL, cip)) */
/*        BUG (); */
/*      printf ("result: %s\n", buf); */
/*      gcry_free (buf); */
/*    } */
  gcry_mpi_release (cip);
  gcry_mpi_release (msg);
  gcry_mpi_release (n);
  gcry_mpi_release (e);
}


static void
mpi_bench (void)
{
  printf ("%-10s", "powm"); fflush (stdout);

  do_powm (
"20A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E4",
           "29", 
"B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8"
           );
  do_powm (
           "20A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E41071F0246879A442658FBD18C1771571E7073EEEB2160BA0CBFB3404D627069A6CFBD53867AD2D9D40231648000787B5C84176B4336144644AE71A403CA40716",
           "29", 
           "B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8FC4EE4BB446B83E48ABED7DB81CBF5E81DE4759E8D68AC985846D999F96B0D8A80E5C69D272C766AB8A23B40D50A4FA889FBC2BD2624222D8EB297F4BAEF8593847"
           );
  do_powm (
           "20A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E41071F0246879A442658FBD18C1771571E7073EEEB2160BA0CBFB3404D627069A6CFBD53867AD2D9D40231648000787B5C84176B4336144644AE71A403CA4071620A94417D4D5EF2B2DA99165C7DC87DADB3979B72961AF90D09D59BA24CB9A10166FDCCC9C659F2B9626EC23F3FA425F564A072BA941B03FA81767CC289E41071F0246879A442658FBD18C1771571E7073EEEB2160BA0CBFB3404D627069A6CFBD53867AD2D9D40231648000787B5C84176B4336144644AE71A403CA40716",
           "29", 
           "B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8FC4EE4BB446B83E48ABED7DB81CBF5E81DE4759E8D68AC985846D999F96B0D8A80E5C69D272C766AB8A23B40D50A4FA889FBC2BD2624222D8EB297F4BAEF8593847B870187A323F1ECD5B8A0B4249507335A1C4CE8394F38FD76B08C78A42C58F6EA136ACF90DFE8603697B1694A3D81114D6117AC1811979C51C4DD013D52F8FC4EE4BB446B83E48ABED7DB81CBF5E81DE4759E8D68AC985846D999F96B0D8A80E5C69D272C766AB8A23B40D50A4FA889FBC2BD2624222D8EB297F4BAEF8593847"
           );

  putchar ('\n');


}


int
main( int argc, char **argv )
{
  if (argc)
    { argc--; argv++; }

  if ( !argc )
    {
      md_bench (NULL);
      putchar ('\n');
      cipher_bench (NULL);
      putchar ('\n');
      mpi_bench ();
      putchar ('\n');
      random_bench ();
    }
  else if ( !strcmp (*argv, "--help"))
     fputs ("usage: benchmark [md|cipher|random|mpi [algonames]]\n", stdout);
  else if ( !strcmp (*argv, "random"))
    {
      random_bench ();
    }
  else if ( !strcmp (*argv, "md"))
    {
      if (argc == 1)
        md_bench (NULL);
      else
        for (argc--, argv++; argc; argc--, argv++)
          md_bench ( *argv );
    }
  else if ( !strcmp (*argv, "cipher"))
    {
      if (argc == 1)
        cipher_bench (NULL);
      else
        for (argc--, argv++; argc; argc--, argv++)
          cipher_bench ( *argv );
    }
  else if ( !strcmp (*argv, "mpi"))
    {
        mpi_bench ();
    }
  else
    {
      fprintf (stderr, PGM ": bad arguments\n");
      return 1;
    }
  
  return 0;
}


