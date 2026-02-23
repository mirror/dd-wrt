/* output.c

   Copyright (C) 2002, 2003, 2009 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "output.h"

#include "base16.h"

/* For TMP_ALLOC */ 
#include "nettle-internal.h"

static void
sexp_put_hash_char(struct sexp_output *output, uint8_t c)
{
  output->hash->update (output->ctx, 1, &c);
}

void
sexp_output_init(struct sexp_output *output, FILE *f,
		 const struct nettle_hash *hash,
		 unsigned width, int prefer_hex)
{
  output->f = f;
  output->line_width = width;
  output->prefer_hex = prefer_hex;
  output->hash = hash;
  if (output->hash)
    {
      output->ctx = xalloc (output->hash->context_size);
      output->hash->init (output->ctx);
      output->put_char = sexp_put_hash_char;
    }
  else
    {
      output->ctx = NULL;
      output->put_char = NULL;
    }
  output->pos = 0;
  output->soft_newline = 0;
}

static void
sexp_put_raw_char(struct sexp_output *output, uint8_t c)
{
  if (putc(c, output->f) < 0)
    die("Write failed: %s\n", strerror(errno));

  output->pos++;
  output->soft_newline = 0;
}

void
sexp_put_newline(struct sexp_output *output,
		 unsigned indent)
{
  if (output->soft_newline)
    output->soft_newline = 0;
  else
    {
      unsigned i;

      sexp_put_raw_char(output, '\n');
      output->pos = 0;
  
      for(i = 0; i < indent; i++)
	sexp_put_raw_char(output, ' ');
  
      output->pos = indent;
    }
}

/* Put a newline, but only if it is followed by another newline,
   collaps to one newline only. */
void
sexp_put_soft_newline(struct sexp_output *output,
		      unsigned indent)
{
  sexp_put_newline(output, indent);
  output->soft_newline = 1;
}

void
sexp_put_char(struct sexp_output *output, uint8_t c)
{
  if (output->put_char)
    {
      if (output->line_width
	  && output->pos >= output->line_width
	  && output->pos >= (output->coding_indent + 10))
	sexp_put_newline(output, output->coding_indent);
      output->put_char (output, c);
    }
  else
    sexp_put_raw_char(output, c);
}

void
sexp_put_data(struct sexp_output *output,
	      unsigned length, const uint8_t *data)
{
  unsigned i;

  for (i = 0; i<length; i++)
    sexp_put_char(output, data[i]);
}

static void
sexp_put_length(struct sexp_output *output, 
		unsigned length)
{
  unsigned digit = 1;

  for (;;)
    {
      unsigned next = digit * 10;
      if (next > length)
	break;
      digit = next;
    }

  for (; digit; length %= digit, digit /= 10)
    sexp_put_char(output, '0' + length / digit);
}

static void
sexp_put_base16 (struct sexp_output *output, uint8_t c)
{
  char encoded[2];
  base16_encode_single (encoded, c);
  sexp_put_raw_char (output, encoded[0]);
  sexp_put_raw_char (output, encoded[1]);
}

void
sexp_put_base16_start (struct sexp_output *output)
{
  assert (!output->put_char);
  output->coding_indent = output->pos;
  output->put_char = sexp_put_base16;
}

void
sexp_put_base16_end (struct sexp_output *output)
{
  output->put_char = NULL;
}

static void
sexp_put_base64 (struct sexp_output *output, uint8_t c)
{
  char encoded[2];
  unsigned done;

  done = base64_encode_single (&output->base64, encoded, c);
  sexp_put_raw_char (output, encoded[0]);
  if (done > 1)
    sexp_put_raw_char (output, encoded[1]);
}

void
sexp_put_base64_start (struct sexp_output *output)
{
  assert (!output->put_char);
  output->coding_indent = output->pos;
  base64_encode_init (&output->base64);
  output->put_char = sexp_put_base64;
}

void
sexp_put_base64_end(struct sexp_output *output)
{
  char encoded[BASE64_ENCODE_FINAL_LENGTH];
  unsigned done;

  done = base64_encode_final (&output->base64, encoded);
  output->put_char = NULL;
  sexp_put_data (output, done, (const uint8_t*) encoded);
}

void
sexp_put_string(struct sexp_output *output, enum sexp_mode mode,
		struct nettle_buffer *string)
{
  if (!string->size)
    sexp_put_data(output, 2,
		  (const uint8_t *) ((mode == SEXP_ADVANCED) ? "\"\"": "0:"));

  else if (mode == SEXP_ADVANCED)
    {
      unsigned i;
      int token = (string->contents[0] < '0' || string->contents[0] > '9');
      int quote_friendly = 1;
#define CONTROL_SIZE 0x20
      static const char escape_names[CONTROL_SIZE] =
	{
	  0,0,0,0,0,0,0,0, 'b','t','n',0,'f','r',0,0,
	  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0
	};

      for (i = 0; i<string->size; i++)
	{
	  uint8_t c = string->contents[i];
	  
	  if (token & !TOKEN_CHAR(c))
	    token = 0;
	  
	  if (quote_friendly)
	    {
	      if (c >= 0x7f)
		quote_friendly = 0;
	      else if (c < CONTROL_SIZE && !escape_names[c])
		quote_friendly = 0;
	    }
	}
      
      if (token)
	sexp_put_data(output, string->size, string->contents);

      else if (quote_friendly)
	{
	  sexp_put_char(output, '"');

	  for (i = 0; i<string->size; i++)
	    {
	      int escape = 0;
	      uint8_t c = string->contents[i];

	      assert(c < 0x7f);
	      
	      if (c == '\\' || c == '"')
		escape = 1;
	      else if (c < CONTROL_SIZE)
		{
		  escape = 1;
		  c = escape_names[c];
		  assert(c);
		}
	      if (escape)
		sexp_put_char(output, '\\');

	      sexp_put_char(output, c);
	    }
	  
	  sexp_put_char(output, '"');
	}
      else if (output->prefer_hex)
	{
	  sexp_put_char(output, '#');
	  sexp_put_base16_start (output);
	  sexp_put_data(output, string->size, string->contents);
	  sexp_put_base16_end (output);
	  sexp_put_char(output, '#');
	}
      else
	{
	  sexp_put_char(output, '|');
	  sexp_put_base64_start (output);
	  sexp_put_data(output, string->size, string->contents);
	  sexp_put_base64_end (output);
	  sexp_put_char(output, '|');
	}
#undef CONTROL_SIZE
    }
  else
    {
      sexp_put_length(output, string->size);
      sexp_put_char(output, ':');
      sexp_put_data(output, string->size, string->contents);
    }
}

void
sexp_put_digest(struct sexp_output *output)
{
  TMP_DECL(digest, uint8_t, NETTLE_MAX_HASH_DIGEST_SIZE);
  TMP_ALLOC(digest, output->hash->digest_size);
  
  assert(output->hash);

  output->hash->digest(output->ctx, digest);

  output->put_char = NULL;

  sexp_put_base16_start (output);
  sexp_put_data (output, output->hash->digest_size, digest);
  sexp_put_base16_end (output);

  output->put_char = sexp_put_hash_char;
}

