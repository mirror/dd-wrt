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
 * @file test_postprocessor.c
 * @brief  Testcase for postprocessor, keys with no value
 * @author Markus Doppelbauer
 */
#include "MHD_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <microhttpd.h>

#define DEBUG 0

#define ARRAY_LENGTH(array)     (sizeof(array) / sizeof(array[0]))

/**
 * Handler for fatal errors.
 */
MHD_PanicCallback mhd_panic;

/**
 * Closure argument for #mhd_panic.
 */
void *mhd_panic_cls;


enum MHD_Result
MHD_lookup_connection_value_n (struct MHD_Connection *connection,
                               enum MHD_ValueKind kind,
                               const char *key,
                               size_t key_size,
                               const char **value_ptr,
                               size_t *value_size_ptr)
{
  (void) connection; (void) kind; (void) key; /* Mute compiler warnings */
  (void) key_size; (void) value_ptr; (void) value_size_ptr;
  return MHD_NO;
}


#include "mhd_str.c"
#include "internal.c"
#include "postprocessor.c"


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
        "xxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxx%2Cxxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cx%2C%2Cx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxx%2C%2C%2Cxxxxxxxxx%2Cxxxxxxxx%2C"
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
      "t=xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxx%2C%2Cx%2Cxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxx%2C%2Cx%2Cxx%2C%2Cxxxx%2Cxxx%2C%2Cx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxx",
      /* one chunk: second line is dropped */
      "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cx%2C%2Cx%2Cxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2Cx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxx%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxxx%2Cxxxxxxxxxx%2Cxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxx%2Cxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxx%2Cxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxx%2Cx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cx%2Cxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxx%2Cxxxxxx%2Cxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2Cxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2C%2C%2C%2C%2C%2C%2Cxxxxxxxxxxxxxx%2C%2C%2C%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2Cxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx%2CxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxZZZZZZZZZZZZZ%2C%2C%2C%2C"
      "%E2%80%A2MMMMMMMM%2C%2C%2C%2CMMMMMMMMMMMMMMMMMMMM%2CMMMMMMMMMMMMMMMMMMMMMMM%2CMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM%2C%2C%2C%2CMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM%2CMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM%2CMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
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
      "XXXXXXXXXXXX=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX&XXXXXX=&XXXXXXXXXXXXXX=XXXX+&XXXXXXXXXXXXXXX=XXXXXXXXX&XXXXXXXXXXXXX=XXXX%XX%XXXXXX&XXXXXXXXXXX=XXXXXXXXX&XXXXXXXXXXXXX=XXXXXXXXXX&XXXXXXXXXXXXXXX=XX&XXXXXXXXXXXXXXX=XXXXXXXXX&XXXXXXXXXXXXX=XXXXXX&XXXXXXXXXXX=XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
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
