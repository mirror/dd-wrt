/*
     This file is part of libmicrohttpd
     Copyright (C) 2020 Christian Grothoff
     Copyright (C) 2020-2023 Evgeny Grin (Karlson2k)

     libmicrohttpd is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 3, or (at your
     option) any later version.

     libmicrohttpd is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with libmicrohttpd; see the file COPYING.  If not, write to the
     Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
*/
/**
 * @file test_postprocessor_md.c
 * @brief  Testcase for postprocessor, keys with no value
 * @author Markus Doppelbauer
 * @author Karlson2k (Evgeny Grin)
 */
#include "mhd_options.h"
#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */
#include "microhttpd.h"
#include "internal.h"
#include "postprocessor.h"
#if 0
#include "mhd_panic.c"
#include "mhd_str.c"
#include "internal.c"
#endif


#define DEBUG 0

/* local replacement for MHD functions */

_MHD_EXTERN enum MHD_Result
MHD_lookup_connection_value_n (struct MHD_Connection *connection,
                               enum MHD_ValueKind kind,
                               const char *key,
                               size_t key_size,
                               const char **value_ptr,
                               size_t *value_size_ptr)
{
  (void) connection; (void) kind; (void) key; /* Mute compiler warnings */
  (void) key_size;

  *value_ptr = "";
  if (NULL != value_size_ptr)
    *value_size_ptr = 0;
  return MHD_NO;
}


#define ARRAY_LENGTH(array)     (sizeof(array) / sizeof(array[0]))


static unsigned int found;


static enum MHD_Result
post_data_iterator (void *cls,
                    enum MHD_ValueKind kind,
                    const char *key,
                    const char *filename,
                    const char *content_type,
                    const char *transfer_encoding,
                    const char *data,
                    uint64_t off,
                    size_t size)
{
  (void) cls; (void) kind; (void) filename; /* Mute compiler warnings */
  (void) content_type; (void) transfer_encoding;
  (void) off; /* FIXME: shouldn't be checked? */
#if DEBUG
  fprintf (stderr,
           "%s\t%s\n",
           key,
           data);
#endif
  if (0 == strcmp (key, "xxxx"))
  {
    if ( (4 != size) ||
         (0 != memcmp (data, "xxxx", 4)) )
      exit (1);
    found |= 1;
  }
  if (0 == strcmp (key, "yyyy"))
  {
    if ( (4 != size) ||
         (0 != memcmp (data, "yyyy", 4)) )
      exit (1);
    found |= 2;
  }
  if (0 == strcmp (key, "zzzz"))
  {
    if (0 != size)
      exit (1);
    found |= 4;
  }
  if (0 == strcmp (key, "aaaa"))
  {
    if (0 != size)
      exit (1);
    found |= 8;
  }
  return MHD_YES;
}


static enum MHD_Result
post_data_iterator2 (void *cls,
                     enum MHD_ValueKind kind,
                     const char *key,
                     const char *filename,
                     const char *content_type,
                     const char *transfer_encoding,
                     const char *data,
                     uint64_t off,
                     size_t size)
{
  static char seen[16];
  (void) cls; (void) kind; (void) filename; /* Mute compiler warnings */
  (void) content_type; (void) transfer_encoding;

#if DEBUG
  printf ("%s\t%s@ %" PRIu64 "\n",
          key,
          data,
          off);
#endif
  if (0 == strcmp (key, "text"))
  {
    if (off + size > sizeof (seen))
      exit (6);
    memcpy (&seen[off],
            data,
            size);
    if ( (10 == off + size) &&
         (0 == memcmp (seen, "text, text", 10)) )
      found |= 1;
  }
  return MHD_YES;
}


static enum MHD_Result
post_data_iterator3 (void *cls,
                     enum MHD_ValueKind kind,
                     const char *key,
                     const char *filename,
                     const char *content_type,
                     const char *transfer_encoding,
                     const char *data,
                     uint64_t off,
                     size_t size)
{
  (void) cls; (void) kind; (void) filename; /* Mute compiler warnings */
  (void) content_type; (void) transfer_encoding;
  (void) off; /* FIXME: shouldn't be checked? */
#if DEBUG
  fprintf (stderr,
           "%s\t%s\n",
           key,
           data);
#endif
  if (0 == strcmp (key, "y"))
  {
    if ( (1 != size) ||
         (0 != memcmp (data, "y", 1)) )
      exit (1);
    found |= 1;
  }
  return MHD_YES;
}


static enum MHD_Result
post_data_iterator4 (void *cls,
                     enum MHD_ValueKind kind,
                     const char *key,
                     const char *filename,
                     const char *content_type,
                     const char *transfer_encoding,
                     const char *data,
                     uint64_t off,
                     size_t size)
{
  (void) cls; (void) kind; (void) key; /* Mute compiler warnings */
  (void) filename; (void) content_type; (void) transfer_encoding;
  (void) off; /* FIXME: shouldn't be checked? */
#if DEBUG
  fprintf (stderr,
           "%s\t%s\n",
           key,
           data);
#endif
  if (NULL != memchr (data, 'M', size))
  {
    found |= 1;
  }
  return MHD_YES;
}


static enum MHD_Result
post_data_iterator5 (void *cls,
                     enum MHD_ValueKind kind,
                     const char *key,
                     const char *filename,
                     const char *content_type,
                     const char *transfer_encoding,
                     const char *data,
                     uint64_t off,
                     size_t size)
{
  (void) cls; (void) kind; (void) key; /* Mute compiler warnings */
  (void) filename; (void) content_type; (void) transfer_encoding;
  (void) data; (void) off; (void) size;

  found++;
  return MHD_YES;
}


int
main (int argc, char *argv[])
{
  struct MHD_PostProcessor *postprocessor;
  (void) argc; (void) argv;

  if (1)
  {
    found = 0;
    postprocessor = malloc (sizeof (struct MHD_PostProcessor)
                            + 0x1000 + 1);
    if (NULL == postprocessor)
      return 77;
    memset (postprocessor,
            0,
            sizeof (struct MHD_PostProcessor) + 0x1000 + 1);
    postprocessor->ikvi = &post_data_iterator;
    postprocessor->encoding = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    postprocessor->buffer_size = 0x1000;
    postprocessor->state = PP_Init;
    postprocessor->skip_rn = RN_Inactive;
    if (MHD_YES != MHD_post_process (postprocessor, "xxxx=xxxx", 9))
      exit (1);
    if (MHD_YES != MHD_post_process (postprocessor, "&yyyy=yyyy&zzzz=&aaaa=",
                                     22))
      exit (1);
    if (MHD_YES != MHD_post_process (postprocessor, "", 0))
      exit (1);
    if (MHD_YES !=
        MHD_destroy_post_processor (postprocessor))
      exit (3);
    if (found != 15)
      exit (2);
  }
  if (1)
  {
    found = 0;
    postprocessor = malloc (sizeof (struct MHD_PostProcessor)
                            + 0x1000 + 1);
    if (NULL == postprocessor)
      return 77;
    memset (postprocessor,
            0,
            sizeof (struct MHD_PostProcessor) + 0x1000 + 1);
    postprocessor->ikvi = post_data_iterator2;
    postprocessor->encoding = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    postprocessor->buffer_size = 0x1000;
    postprocessor->state = PP_Init;
    postprocessor->skip_rn = RN_Inactive;
    if (MHD_YES != MHD_post_process (postprocessor, "text=text%2", 11))
      exit (1);
    if (MHD_YES != MHD_post_process (postprocessor, "C+text", 6))
      exit (1);
    if (MHD_YES != MHD_post_process (postprocessor, "", 0))
      exit (1);
    if (MHD_YES != MHD_destroy_post_processor (postprocessor))
      exit (1);
    if (found != 1)
      exit (4);
  }
  if (1)
  {
    found = 0;
    postprocessor = malloc (sizeof (struct MHD_PostProcessor)
                            + 0x1000 + 1);
    if (NULL == postprocessor)
      return 77;
    memset (postprocessor,
            0,
            sizeof (struct MHD_PostProcessor) + 0x1000 + 1);
    postprocessor->ikvi = post_data_iterator3;
    postprocessor->encoding = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    postprocessor->buffer_size = 0x1000;
    postprocessor->state = PP_Init;
    postprocessor->skip_rn = RN_Inactive;
    {
      const char *chunk =
        "x=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxx%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2Cxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2Cxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxx%2C%2Cxxxxxx"
        "%2Cxxxx%2C%2Cxxxxx%2Cxxxxxxxxx%2C%2Cxxx%2Cxxxxxxxxxxx_xxxxxxxxxxxxxxx"
        "%2Cxxxxx%2Cxxxxxxxxxxx_xxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx"
        "%2Cxxxxxxxxxxx_xxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxx%2C%2Cxxxxx%2C%2Cx%2Cxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C"
        "xxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxx%2C%2Cxx%2C%2Cx%2Cxx%2C%2Cxxxx%2Cxxx"
        "%2C%2Cx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C"
        "%2Cxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "x%2C%2C%2C%2C%2C%2C%2Cxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xx%2Cxxxxx%2Cxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxx%2Cxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxx"
        "xx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxx"
        "xxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cxxxx"
        "xxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxx%2Cx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2"
        "Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxx%2Cxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cx"
        "xxxxxxxx%2Cxxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxx"
        "%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C"
        "%2C%2C%2C%2C%2Cxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%"
        "2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C"
        "%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2"
        "Cxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2"
        "C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%"
        "2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxxxxxxxxx"
        "xxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2Cxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2"
        "Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2"
        "C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cx%2C%"
        "2Cx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxx%2Cxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxx%2Cx"
        "xxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx"
        "xxx%2Cxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxx%2Cx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cx"
        "xxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxx"
        "xx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxx"
        "xxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx"
        "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C"
        "%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxx"
        "xxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxx"
        "xxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2C"
        "xxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxx"
        "xxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2"
        "Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxx"
        "xxx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx"
        "xxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx"
        "xxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxx"
        "xxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx"
        "xxxxxxxxx%2Cxxxxx%2Cxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx"
        "xxxxxxxxxxxxxxx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxx%2Cx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxx%2Cx%2Cxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%"
        "2Cxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxx%"
        "2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxx"
        "xx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxx"
        "xxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%"
        "2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxx"
        "x%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxx"
        "xxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C"
        "%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxx%2C%2C%2C%2Cxxxxxxx"
        "xxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%"
        "2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
        "xxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx"
        "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxx%2Cxxx"
        "xxxx%2Cxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxx%"
        "2C%2C%2Cxxxxxxxxx%2Cxxxxxxxx%2C"
        "&y=y&z=z";

      if (MHD_YES != MHD_post_process (postprocessor, chunk, strlen (chunk) ))
        exit (1);
    }
    if (MHD_YES != MHD_post_process (postprocessor, "", 0))
      exit (1);
    if (MHD_YES != MHD_destroy_post_processor (postprocessor))
      exit (1);
    if (found != 1)
      exit (5);
  }

  if (1)
  {
    unsigned i;
    const char *chunks[] = {
      "t=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxx%2C%2Cx%2Cxxxxxxxx"
      "xxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cxxxxx%"
      "2Cxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxx%2C%2Cx%2"
      "Cxx%2C%2Cxxxx%2Cxxx%2C%2Cx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "x%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2"
      "Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxx%2C%2C%2C%2Cxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxx"
      "xxxxxxxx%2C%2Cxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx"
      "xxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cx%2Cxxxxxxxxxx"
      "xxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xx%2Cxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxx"
      "xxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2C"
      "xxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C"
      "%2C%2C%2C%2Cxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C"
      "%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxx"
      "xxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2"
      "C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxxxxxxxxxxxx"
      "xxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2"
      "C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxx",
      /* one chunk: second line is dropped */
      "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C"
      "xxxxxxxxxxxxxxxx%2Cx%2C%2Cx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2Cxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxx"
      "xxxxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cx%2C%2C"
      "%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%"
      "2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxx"
      "xxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "x%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxx%2C%2C%2C%2C"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx"
      "xxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxx"
      "xxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxx"
      "%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx"
      "x%2Cxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxx"
      "xx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxx%2Cxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%"
      "2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx%2Cxx"
      "xxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cxxxx"
      "x%2Cxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%"
      "2Cx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxx%2Cx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xx%2Cx%2Cxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxx%2Cx"
      "xxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxx%2Cxxxxxxxxxxxxxx"
      "xxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%"
      "2C%2C%2C%2Cxxxxxxxxxxx%2C%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2"
      "Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx"
      "xxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2"
      "C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxx%2"
      "C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
      "xxxxxxxxxxxxxxxxxxxxxxxxxZZZZZZZZZZZZZ%2C%2C%2C%2C"
      "%E2%80%A2MMMMMMMM%2C%2C%2C%2CMMMMMMMMMMMMMMMMMMMM%2CMMMMMMMMMMMMMMMMMM"
      "MMMMM%2CMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMM%2C%2C%2C%2CMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM%2CMMMMMMMMMMMMMMMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM%2CMMMMMM"
      "MMMMMMMMMMMMMMMMMMMMMMMMMMMM"
      "zz",
      "",
    };
    postprocessor = malloc (sizeof(struct MHD_PostProcessor) + 131076 + 1);
    if (NULL == postprocessor)
      return 77;
    memset (postprocessor,
            0,
            sizeof (struct MHD_PostProcessor) + 0x1000 + 1);
    postprocessor->ikvi = post_data_iterator4;
    postprocessor->encoding = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    postprocessor->buffer_size = 131076;
    postprocessor->state = PP_Init;
    postprocessor->skip_rn = RN_Inactive;
    for (i = 0; i < ARRAY_LENGTH (chunks); ++i)
    {
      const char *chunk = chunks[i];
      if (MHD_YES != MHD_post_process (postprocessor, chunk, strlen (chunk) ))
        exit (1);
    }
    if (MHD_YES != MHD_destroy_post_processor (postprocessor))
      exit (1);
    if (found != 1)
      return 6;
  }
  if (1)
  {
    unsigned i;
    const char *chunks[] = {
      "XXXXXXXXXXXX=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
      "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX&XXXXXX=&XXXXXXXXXXXXXX=X"
      "XXX+&XXXXXXXXXXXXXXX=XXXXXXXXX&XXXXXXXXXXXXX=XXXX%XX%XXXXXX&XXXXXXXXXX"
      "X=XXXXXXXXX&XXXXXXXXXXXXX=XXXXXXXXXX&XXXXXXXXXXXXXXX=XX&XXXXXXXXXXXXXX"
      "X=XXXXXXXXX&XXXXXXXXXXXXX=XXXXXX&XXXXXXXXXXX=XXXXXXXXXXXXXXXXXXXXXXXXX"
      "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
      "XXXXXXXXXX",
      "&XXXXXXXX=XXXX",
      "",
    };
    postprocessor = malloc (sizeof(struct MHD_PostProcessor) + 131076 + 1);
    found = 0;
    if (NULL == postprocessor)
      return 77;
    memset (postprocessor,
            0,
            sizeof (struct MHD_PostProcessor) + 0x1000 + 1);
    postprocessor->ikvi = post_data_iterator5;
    postprocessor->encoding = MHD_HTTP_POST_ENCODING_FORM_URLENCODED;
    postprocessor->buffer_size = 131076;
    postprocessor->state = PP_Init;
    postprocessor->skip_rn = RN_Inactive;
    for (i = 0; i < ARRAY_LENGTH (chunks); ++i)
    {
      const char *chunk = chunks[i];
      if (MHD_YES != MHD_post_process (postprocessor, chunk, strlen (chunk) ))
        exit (1);
    }
    if (MHD_YES != MHD_destroy_post_processor (postprocessor))
      exit (1);
    if (found != 12)
      return 7;
  }


  return EXIT_SUCCESS;
}
