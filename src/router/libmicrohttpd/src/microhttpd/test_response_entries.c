/*
     This file is part of libmicrohttpd
     Copyright (C) 2021 Karlson2k (Evgeny Grin)

     This test_response_entries.c file is in the public domain
*/

/**
 * @file test_response_entries.c
 * @brief  Test adding and removing response headers
 * @author Karlson2k (Evgeny Grin)
 */
#include "mhd_options.h"
#include "platform.h"
#include <string.h>
#include <microhttpd.h>


static int
expect_str (const char *actual, const char *expected)
{
  if (expected == actual)
    return ! 0;
  if (NULL == actual)
  {
    fprintf (stderr, "FAILED: result: NULL\n" \
             "        expected: \"%s\"",
             expected);
    return 0;
  }
  if (NULL == expected)
  {
    fprintf (stderr, "FAILED: result: \"%s\"\n" \
             "        expected: NULL",
             actual);
    return 0;
  }
  if (0 != strcmp (actual, expected))
  {
    fprintf (stderr, "FAILED: result: \"%s\"\n" \
             "        expected: \"%s\"",
             actual, expected);
    return 0;
  }
  return ! 0;
}


int
main (int argc,
      char *const *argv)
{
  (void) argc;
  (void) argv; /* Unused. Silence compiler warning. */
  struct MHD_Response *r;

  r = MHD_create_response_from_buffer (0, "", MHD_RESPMEM_PERSISTENT);
  if (NULL == r)
  {
    fprintf (stderr, "Cannot create a response.\n");
    return 1;
  }

  /* ** Test basic header functions ** */

  /* Add first header */
  if (MHD_YES != MHD_add_response_header (r, "Header-Type-A", "value-a1"))
  {
    fprintf (stderr, "Cannot add header A1.\n");
    MHD_destroy_response (r);
    return 2;
  }
  if (! expect_str (MHD_get_response_header (r, "Header-Type-A"), "value-a1"))
  {
    MHD_destroy_response (r);
    return 2;
  }
  /* Add second header with the same name */
  if (MHD_YES != MHD_add_response_header (r, "Header-Type-A", "value-a2"))
  {
    fprintf (stderr, "Cannot add header A2.\n");
    MHD_destroy_response (r);
    return 2;
  }
  /* Value of the first header must be returned */
  if (! expect_str (MHD_get_response_header (r, "Header-Type-A"), "value-a1"))
  {
    MHD_destroy_response (r);
    return 2;
  }
  /* Remove the first header */
  if (MHD_YES != MHD_del_response_header (r, "Header-Type-A", "value-a1"))
  {
    fprintf (stderr, "Cannot remove header A1.\n");
    MHD_destroy_response (r);
    return 2;
  }
  /* Value of the ex-second header must be returned */
  if (! expect_str (MHD_get_response_header (r, "Header-Type-A"), "value-a2"))
  {
    MHD_destroy_response (r);
    return 2;
  }
  if (MHD_YES != MHD_add_response_header (r, "Header-Type-A", "value-a3"))
  {
    fprintf (stderr, "Cannot add header A2.\n");
    MHD_destroy_response (r);
    return 2;
  }
  /* Value of the ex-second header must be returned */
  if (! expect_str (MHD_get_response_header (r, "Header-Type-A"), "value-a2"))
  {
    MHD_destroy_response (r);
    return 2;
  }
  /* Remove the last header */
  if (MHD_YES != MHD_del_response_header (r, "Header-Type-A", "value-a3"))
  {
    fprintf (stderr, "Cannot add header A2.\n");
    MHD_destroy_response (r);
    return 2;
  }
  if (! expect_str (MHD_get_response_header (r, "Header-Type-A"), "value-a2"))
  {
    MHD_destroy_response (r);
    return 2;
  }
  if (! expect_str (MHD_get_response_header (r, "Header-Type-B"), NULL))
  {
    MHD_destroy_response (r);
    return 2;
  }
  if (MHD_NO != MHD_del_response_header (r, "Header-Type-C", "value-a3"))
  {
    fprintf (stderr, "Removed non-existing header.\n");
    MHD_destroy_response (r);
    return 2;
  }
  if (MHD_NO != MHD_del_response_header (r, "Header-Type-A", "value-c"))
  {
    fprintf (stderr, "Removed non-existing header value.\n");
    MHD_destroy_response (r);
    return 2;
  }

  /* ** Test "Connection:" header ** */

  if (MHD_YES != MHD_add_response_header (r, "Connection", "a,b,c,d,e"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with simple values.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "a, b, c, d, e"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "e,b,c,d,a"))
  {
    fprintf (stderr,
             "Cannot remove \"Connection\" header with simple values.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }

  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "i,k,l,m,n,o,p,close"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with simple values and \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, i, k, l, m, n, o, p"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "i,k,l,m,n,o,p,close"))
  {
    fprintf (stderr,
             "Cannot remove \"Connection\" header with simple values and \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }

  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "1,2,3,4,5,6,7,close"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with simple values and \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, 1, 2, 3, 4, 5, 6, 7"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "8,9,close"))
  {
    fprintf (stderr,
             "Cannot add second \"Connection\" header with simple values and \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, 1, 2, 3, 4, 5, 6, 7, 8, 9"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "1,3,5,7,9"))
  {
    fprintf (stderr,
             "Cannot remove part of \"Connection\" header with simple values.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, 2, 4, 6, 8"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "10,12"))
  {
    fprintf (stderr,
             "Cannot add third \"Connection\" header with simple values.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, 2, 4, 6, 8, 10, 12"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "12  ,10  ,8  ,close"))
  {
    fprintf (stderr,
             "Cannot remove part of \"Connection\" header with simple values and \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "2, 4, 6"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\" only.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, 2, 4, 6"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "4  ,5,6,7  8,"))
  {
    fprintf (stderr,
             "Cannot remove part of \"Connection\" header with simple values and non-existing tokens.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "close, 2"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\" only.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "close, 2"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "close, 10, 12, 22, nothing"))
  {
    fprintf (stderr,
             "Cannot remove part of \"Connection\" header with \"close\" and non-existing tokens.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "2"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "2"))
  {
    fprintf (stderr,
             "Cannot remove part of \"Connection\" header with simple values and non-existing tokens.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "close"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "close"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot remove \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }

  if (MHD_YES != MHD_add_response_header (r, "Connection", "close,other-token"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, other-token"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close, new-token"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, other-token, new-token"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "close, new-token"))
  {
    fprintf (stderr, "Cannot remove tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "other-token"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "other-token"))
  {
    fprintf (stderr, "Cannot remove tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }

  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "close, one-long-token"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, one-long-token"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, one-long-token"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "one-long-token,close"))
  {
    fprintf (stderr, "Cannot remove tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }

  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "close"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "close, additional-token"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, additional-token"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "additional-token,close"))
  {
    fprintf (stderr, "Cannot remove tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }


  if (MHD_YES != MHD_add_response_header (r, "Connection", "token-1,token-2"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "token-1, token-2"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "token-3"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "token-1, token-2, token-3"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, token-1, token-2, token-3"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, token-1, token-2, token-3"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close, token-4"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, token-1, token-2, token-3, token-4"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection", "close"))
  {
    fprintf (stderr, "Cannot remove tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "token-1, token-2, token-3, token-4"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "close, token-5"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with \"close\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, token-1, token-2, token-3, token-4, token-5"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_NO != MHD_del_response_header (r, "Connection",
                                         "non-existing, token-9"))
  {
    fprintf (stderr,
             "Non-existing tokens successfully removed from \"Connection\" header.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, token-1, token-2, token-3, token-4, token-5"))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_NO != MHD_add_response_header (r, "Connection",
                                         ",,,,,,,,,,,,    ,\t\t\t, , , "))
  {
    fprintf (stderr,
             "Empty token was added successfully to \"Connection\" header.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "close, token-1, token-2, token-3, token-4, token-5"))
  {
    fprintf (stderr, "Cannot remove tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }
  if (MHD_NO != MHD_add_response_header (r, "Connection",
                                         ",,,,,,,,,,,,    ,\t\t\t, , , "))
  {
    fprintf (stderr,
             "Empty token was added successfully to \"Connection\" header.\n");
    MHD_destroy_response (r);
    return 3;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 3;
  }

  if (MHD_NO != MHD_add_response_header (r, "Connection", "keep-Alive"))
  {
    fprintf (stderr,
             "Successfully added \"Connection\" header with \"keep-Alive\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "keep-Alive, Close"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with \"keep-Alive, Close\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "close"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_NO != MHD_add_response_header (r, "Connection", "keep-Alive"))
  {
    fprintf (stderr,
             "Successfully added \"Connection\" header with \"keep-Alive\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "keep-Alive, Close"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with \"keep-Alive, Close\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "close"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "close, additional-token"))
  {
    fprintf (stderr, "Cannot add \"Connection\" header with "
             "\"close, additional-token\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, additional-token"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_NO != MHD_add_response_header (r, "Connection", "keep-Alive"))
  {
    fprintf (stderr,
             "Successfully added \"Connection\" header with \"keep-Alive\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, additional-token"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "additional-token,close"))
  {
    fprintf (stderr, "Cannot remove tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 4;
  }

  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "Keep-aLive, token-1"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with \"Keep-aLive, token-1\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), "token-1"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "Keep-aLive, token-2"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with \"Keep-aLive, token-2\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "token-1, token-2"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection",
                                          "Keep-aLive, token-3, close"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with \"Keep-aLive, token-3, close\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, token-1, token-2, token-3"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "close"))
  {
    fprintf (stderr, "Cannot remove \"close\" tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "token-1, token-2, token-3"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_add_response_header (r, "Connection", "Keep-aLive, close"))
  {
    fprintf (stderr,
             "Cannot add \"Connection\" header with \"Keep-aLive, token-3, close\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"),
                    "close, token-1, token-2, token-3"))
  {
    MHD_destroy_response (r);
    return 4;
  }
  if (MHD_YES != MHD_del_response_header (r, "Connection",
                                          "close, token-1, Keep-Alive, token-2, token-3"))
  {
    fprintf (stderr, "Cannot remove \"close\" tokens from \"Connection\".\n");
    MHD_destroy_response (r);
    return 4;
  }
  if (! expect_str (MHD_get_response_header (r, "Connection"), NULL))
  {
    MHD_destroy_response (r);
    return 4;
  }

  if (MHD_YES != MHD_add_response_header (r, "Date",
                                          "Wed, 01 Apr 2015 00:00:00 GMT"))
  {
    fprintf (stderr,
             "Cannot add \"Date\" header with \"Wed, 01 Apr 2015 00:00:00 GMT\".\n");
    MHD_destroy_response (r);
    return 5;
  }
  if (! expect_str (MHD_get_response_header (r, "Date"),
                    "Wed, 01 Apr 2015 00:00:00 GMT"))
  {
    MHD_destroy_response (r);
    return 5;
  }
  if (MHD_YES != MHD_add_response_header (r, "Date",
                                          "Thu, 01 Apr 2021 00:00:00 GMT"))
  {
    fprintf (stderr,
             "Cannot add \"Date\" header with \"Thu, 01 Apr 2021 00:00:00 GMT\".\n");
    MHD_destroy_response (r);
    return 5;
  }
  if (! expect_str (MHD_get_response_header (r, "Date"),
                    "Thu, 01 Apr 2021 00:00:00 GMT"))
  {
    MHD_destroy_response (r);
    return 5;
  }
  if (MHD_YES != MHD_del_response_header (r, "Date",
                                          "Thu, 01 Apr 2021 00:00:00 GMT"))
  {
    fprintf (stderr, "Cannot remove \"Date\" header.\n");
    MHD_destroy_response (r);
    return 5;
  }
  if (! expect_str (MHD_get_response_header (r, "Date"), NULL))
  {
    MHD_destroy_response (r);
    return 5;
  }

  if (MHD_YES != MHD_add_response_header (r, MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                          "chunked"))
  {
    fprintf (stderr,
             "Cannot add \"" MHD_HTTP_HEADER_TRANSFER_ENCODING \
             "\" header with \"chunked\".\n");
    MHD_destroy_response (r);
    return 6;
  }
  if (! expect_str (MHD_get_response_header (r,
                                             MHD_HTTP_HEADER_TRANSFER_ENCODING),
                    "chunked"))
  {
    MHD_destroy_response (r);
    return 6;
  }
  if (MHD_YES != MHD_add_response_header (r, MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                          "chunked"))
  {
    fprintf (stderr,
             "Cannot add \"" MHD_HTTP_HEADER_TRANSFER_ENCODING \
             "\" second header with \"chunked\".\n");
    MHD_destroy_response (r);
    return 6;
  }
  if (! expect_str (MHD_get_response_header (r,
                                             MHD_HTTP_HEADER_TRANSFER_ENCODING),
                    "chunked"))
  {
    MHD_destroy_response (r);
    return 6;
  }
  if (MHD_NO != MHD_add_response_header (r, MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                         "identity"))
  {
    fprintf (stderr,
             "Successfully added \"" MHD_HTTP_HEADER_TRANSFER_ENCODING \
             "\" header with \"identity\".\n");
    MHD_destroy_response (r);
    return 6;
  }
  if (! expect_str (MHD_get_response_header (r,
                                             MHD_HTTP_HEADER_TRANSFER_ENCODING),
                    "chunked"))
  {
    MHD_destroy_response (r);
    return 6;
  }
  if (MHD_YES != MHD_del_response_header (r, MHD_HTTP_HEADER_TRANSFER_ENCODING,
                                          "chunked"))
  {
    fprintf (stderr, "Cannot remove \"" MHD_HTTP_HEADER_TRANSFER_ENCODING \
             "\" header.\n");
    MHD_destroy_response (r);
    return 6;
  }
  if (! expect_str (MHD_get_response_header (r,
                                             MHD_HTTP_HEADER_TRANSFER_ENCODING),
                    NULL))
  {
    MHD_destroy_response (r);
    return 6;
  }

  MHD_destroy_response (r);
  printf ("All tests has been successfully passed.\n");
  return 0;
}
