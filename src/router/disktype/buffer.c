/*
 * buffer.c
 * Manages data reading and caching.
 *
 * Copyright (c) 2003 Christoph Pfisterer
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 */

#include "global.h"

#define PROFILE 0

/*
 * constants
 */

/* this defines 4K chunks, should only be changed upwards */
#define CHUNKBITS (12)
#define CHUNKSIZE (1<<CHUNKBITS)
#define CHUNKMASK (CHUNKSIZE-1)

/* the minimum block size for block-oriented sources, maximum is the
   chunk size */
#define MINBLOCKSIZE (256)

/* a simple hash function */
#define HASHSIZE (13)
#define HASHFUNC(start) (((start)>>CHUNKBITS) % HASHSIZE)

/* convenience */
#define MINIMUM(a,b) (((a) < (b)) ? (a) : (b))
#define MAXIMUM(a,b) (((a) > (b)) ? (a) : (b))

/*
 * types
 */

typedef struct chunk {
  /* file positions
     start never changes and is CHUNKSIZE-aligned, it identifies the chunk
     len is between 0 and CHUNKSIZE
     end is kept synchronized to (start + len)
  */
  u8 start, end, len;
  void *buf;
  /* links within a hash bucket, organized as a ring list */
  struct chunk *next, *prev;
} CHUNK;

typedef struct cache {
  /* chunks stored as a hash table of ring lists */
  CHUNK *hashtab[HASHSIZE];
  /* temporary buffer for requests involving several chunks */
  void *tempbuf;
} CACHE;

/*
 * helper functions
 */

static CHUNK * ensure_chunk(SOURCE *s, CACHE *cache, u8 start);
static CHUNK * get_chunk_alloc(CACHE *cache, u8 start);

/*
 * retrieve a piece of the source, entry point for detection
 */

u8 get_buffer(SECTION *section, u8 pos, u8 len, void **buf)
{
  SOURCE *s;

  /* get source info */
  s = section->source;
  pos += section->pos;

  return get_buffer_real(s, pos, len, NULL, buf);
}

/*
 * actual retrieval, entry point for layering
 */

u8 get_buffer_real(SOURCE *s, u8 pos, u8 len, void *inbuf, void **outbuf)
{
  CACHE *cache;
  CHUNK *c;
  u8 end, first_chunk, last_chunk, curr_chunk, got, tocopy;
  void *mybuf;

  /* sanity check */
  if (len == 0 || (inbuf == NULL && outbuf == NULL))
    return 0;

  /* get source info */
  if (s->size_known && pos >= s->size) {
    /* error("Request for data beyond end of file (pos %llu)", pos); */
    return 0;
  }
  end = pos + len;
  if (s->size_known && end > s->size) {
    /* truncate to known end of file */
    end = s->size;
    len = end - pos;
  }

  /* get cache head */
  cache = (CACHE *)s->cache_head;
  if (cache == NULL) {
    /* allocate and initialize new cache head */
    cache = (CACHE *)malloc(sizeof(CACHE));
    if (cache == NULL)
      bailout("Out of memory");
    memset(cache, 0, sizeof(CACHE));
    s->cache_head = (void *)cache;
  }
  /* free old temp buffer if present */
  if (cache->tempbuf != NULL) {
    free(cache->tempbuf);
    cache->tempbuf = NULL;
  }

  /* calculate involved chunks */
  first_chunk = pos & ~CHUNKMASK;
  last_chunk = (end - 1) & ~CHUNKMASK;

  if (last_chunk == first_chunk) {
    /* just get the matching chunk */
    c = ensure_chunk(s, cache, first_chunk);
    /* NOTE: first_chunk == c->start */

    if (pos >= c->end)  /* chunk is incomplete and doesn't have our data */
      return 0;

    /* calculate return data */
    len = MINIMUM(len, c->end - pos);  /* guaranteed to be > 0 */
    mybuf = c->buf + (pos - c->start);
    if (inbuf)
      memcpy(inbuf, mybuf, len);
    if (outbuf)
      *outbuf = mybuf;

    return len;

  } else {

    /* prepare a buffer for concatenation */
    if (inbuf) {
      mybuf = inbuf;
    } else {

#if PROFILE
      printf("Temporary buffer for request %llu:%llu\n", pos, len);
#endif

      /* allocate one temporarily, will be free()'d at the next call */
      cache->tempbuf = malloc(len);
      if (cache->tempbuf == NULL) {
	error("Out of memory, still going");
	return 0;
      }
      mybuf = cache->tempbuf;
    }

    /* draw data from all covered chunks */
    got = 0;
    for (curr_chunk = first_chunk; curr_chunk <= last_chunk;
	 curr_chunk += CHUNKSIZE) {
      /* get that chunk */
      c = ensure_chunk(s, cache, curr_chunk);
      /* NOTE: curr_chunk == c->start */

      if (pos > curr_chunk) {
	/* copy from middle of chunk */
	/* NOTE:
	   - we're in the first chunk, i.e. pos < curr_chunk + CHUNKSIZE
	   - pos >= curr_chunk
	   - got == 0
	   - we must read to the end (else it would be the one-chunk case),
	     i.e. pos + len > curr_chunk + CHUNKSIZE
	   BUT: the chunk may be only partially filled
	*/
	if (c->end > pos) {
	  tocopy = c->end - pos;
	  memcpy(mybuf, c->buf + (pos & CHUNKMASK), tocopy);
	} else
	  tocopy = 0;
      } else {
	/* copy from start of chunk */
	tocopy = MINIMUM(c->len, len - got);  /* c->len can be zero */
	if (tocopy)
	  memcpy(mybuf + got, c->buf, tocopy);
      }
      got += tocopy;

      /* stop after an incomplete chunk (possibly-okay for the last one,
	 not-so-nice for earlier ones, but treated all the same) */
      if (c->len < CHUNKSIZE)
	break;
    }

    /* calculate return data */
    len = MINIMUM(len, got);  /* may be zero */
    if (outbuf)
      *outbuf = mybuf;
    return len;

  }
}

static CHUNK * ensure_chunk(SOURCE *s, CACHE *cache, u8 start)
{
  CHUNK *c;
  u8 pos, rel_start, rel_end;
  u8 toread, result, curr_chunk;

  c = get_chunk_alloc(cache, start);
  if (c->len >= CHUNKSIZE || (s->size_known && c->end >= s->size)) {
    /* chunk is complete  or  complete until EOF */
    return c;
  }

  if (s->sequential) {
    /* sequential source: ensure all data before this chunk was read */

    if (s->seq_pos < start) {
      /* try to read data between seq_pos and start */
      curr_chunk = s->seq_pos & ~CHUNKMASK;
      while (curr_chunk < start) {  /* runs at least once, due to the if()
				       and the formula of curr_chunk */
	ensure_chunk(s, cache, curr_chunk);
	curr_chunk += CHUNKSIZE;
	if (s->seq_pos < curr_chunk)
	  break;  /* it didn't work out... */
      }

      /* re-check precondition since s->size may have changed */
      if (s->size_known && c->end >= s->size)
	return c;  /* there is no more data to read */
    }

    if (s->seq_pos != c->end)   /* c->end is where we'll continue reading */
      return c;  /* we're not in a sane state, give up */
  }

  /* try to read the missing piece */
  if (s->read_block != NULL) {
    /* use block-oriented read_block() method */

    if (s->blocksize < MINBLOCKSIZE ||
	s->blocksize > CHUNKSIZE ||
	((s->blocksize & (s->blocksize - 1)) != 0)) {
      bailout("Internal error: Invalid block size %d", s->blocksize);
    }

    for (rel_start = 0; rel_start < CHUNKSIZE; rel_start = rel_end) {
      rel_end = rel_start + s->blocksize;
      if (c->len >= rel_end)
	continue;  /* already read */
      pos = c->start + rel_start;
      if (s->size_known && pos >= s->size)
	break;     /* whole block is past end of file */

      /* read it */
      if (s->read_block(s, pos, c->buf + rel_start)) {
	/* success */
	c->len = rel_end;
	c->end = c->start + c->len;
      } else {
	/* failure */
	c->len = rel_start;  /* this is safe as it can only mean a shrink */
	c->end = c->start + c->len;
	/* note the new end of file if necessary */
	if (!s->size_known || s->size > c->end) {
	  s->size_known = 1;
	  s->size = c->end;
	}
	break;
      }
    }

  } else {
    /* use byte-oriented read_bytes() method */

    if (s->size_known && s->size < c->start + CHUNKSIZE) {
      /* do not try to read beyond known end of file */
      toread = s->size - c->end;
      /* toread cannot be zero or negative due to the preconditions */
    } else {
      toread = CHUNKSIZE - c->len;
    }
    result = s->read_bytes(s, c->start + c->len, toread,
			   c->buf + c->len);
    if (result > 0) {
      /* adjust offsets */
      c->len += result;
      c->end = c->start + c->len;
      if (s->sequential)
	s->seq_pos += result;
    }
    if (result < toread) {
      /* we fell short, so it must have been an error or end-of-file */
      /* make sure we don't try again */
      if (!s->size_known || s->size > c->end) {
	s->size_known = 1;
	s->size = c->end;
      }
    }
  }

  return c;
}

static CHUNK * get_chunk_alloc(CACHE *cache, u8 start)
{
  int hpos;
  CHUNK *chain, *trav, *c;

  /* get hash bucket (chain) */
  hpos = HASHFUNC(start);
  chain = cache->hashtab[hpos];

  if (chain == NULL) {
    /* bucket is empty, allocate new chunk */
    c = (CHUNK *)malloc(sizeof(CHUNK));
    if (c == NULL)
      bailout("Out of memory");
    c->buf = malloc(CHUNKSIZE);
    if (c->buf == NULL)
      bailout("Out of memory");
    c->start = start;
    c->end = start;
    c->len = 0;
    /* create a new ring list */
    c->prev = c->next = c;
    cache->hashtab[hpos] = c;
    return c;
  }

  /* travel the ring list, looking for the wanted chunk */
  trav = chain;
  do {
    if (trav->start == start)  /* found existing chunk */
      return trav;
    trav = trav->next;
  } while (trav != chain);

  /* not found, allocate new chunk */
  c = (CHUNK *)malloc(sizeof(CHUNK));
  if (c == NULL)
    bailout("Out of memory");
  c->buf = malloc(CHUNKSIZE);
  if (c->buf == NULL)
    bailout("Out of memory");
  c->start = start;
  c->end = start;
  c->len = 0;
  /* add to ring list before chain, becomes new head */
  c->prev = chain->prev;
  c->next = chain;
  c->prev->next = c;
  c->next->prev = c;
  cache->hashtab[hpos] = c;
  return c;
}

/*
 * dispose of a source
 */

void close_source(SOURCE *s)
{
  CACHE *cache;
  int hpos;
  CHUNK *chain, *trav, *nexttrav;

  /* drop the cache */
  cache = (CACHE *)s->cache_head;
  if (cache != NULL) {
#if PROFILE
    printf("Cache profile:\n");
#endif
    if (cache->tempbuf != NULL)
      free(cache->tempbuf);
    for (hpos = 0; hpos < HASHSIZE; hpos++) {
#if PROFILE
      printf(" hash position %d:", hpos);
#endif
      chain = cache->hashtab[hpos];
      if (chain != NULL) {
	trav = chain;
	do {
#if PROFILE
	  printf(" %lluK", trav->start >> 10);
	  if (trav->len != CHUNKSIZE)
	    printf(":%llu", trav->len);
#endif
	  nexttrav = trav->next;
	  free(trav->buf);
	  free(trav);
	  trav = nexttrav;
	} while (trav != chain);
#if PROFILE
	printf("\n");
#endif
      }
#if PROFILE
      else
	printf(" empty\n");
#endif
    }
  }

  /* type-specific cleanup */
  if (s->close != NULL)
    (*s->close)(s);

  /* release memory for the structure */
  free(s);
}

/* EOF */
