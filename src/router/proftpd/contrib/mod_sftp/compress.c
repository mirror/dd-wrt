/*
 * ProFTPD - mod_sftp compression
 * Copyright (c) 2008 TJ Saunders
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * $Id: compress.c,v 1.3 2009/11/22 22:18:13 castaglia Exp $
 */

#include "mod_sftp.h"

#include <zlib.h>

#include "msg.h"
#include "packet.h"
#include "crypto.h"
#include "compress.h"

struct sftp_compress {
  int use_zlib;
  int stream_ready;
};

/* We need to keep the old compression contexts around, so that we can handle
 * N arbitrary packets to/from the client using the old contexts, as during
 * rekeying.  Thus we have two read compression contexts, two write compression
 * contexts. The compression idx variable indicates which of the contexts is
 * currently in use.
 */

static struct sftp_compress read_compresses[] = {
  { FALSE, FALSE },
  { FALSE, FALSE }
};
static z_stream read_streams[2];

static struct sftp_compress write_compresses[] = {
  { FALSE, FALSE },
  { FALSE, FALSE }
};
static z_stream write_streams[2];

static unsigned int read_comp_idx = 0;
static unsigned int write_comp_idx = 0;

static unsigned int get_next_read_index(void) {
  if (read_comp_idx == 1)
    return 0;

  return 1;
}

static unsigned int get_next_write_index(void) {
  if (write_comp_idx == 1)
    return 0;

  return 1;
}

static void switch_read_compress(int flags) {
  struct sftp_compress *comp;
  z_stream *stream;

  comp = &(read_compresses[read_comp_idx]);
  stream = &(read_streams[read_comp_idx]);

  /* First we can free up the read stream, kept from rekeying. */
  if (comp->use_zlib == flags &&
      comp->stream_ready) {
  
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "done decompressing data: decompressed %" PR_LU " bytes to %" PR_LU
      " bytes of data (%.2f)", (pr_off_t) stream->total_in,
      (pr_off_t) stream->total_out,
      stream->total_in == 0 ? 0.0 :
        (float) stream->total_out / stream->total_in);

    inflateEnd(stream);
    comp->use_zlib = FALSE;
    comp->stream_ready = FALSE;

    /* Now we can switch the index. */
    if (read_comp_idx == 1) {
      read_comp_idx = 0;
      return;
    }

    read_comp_idx = 1;
  }
}

static void switch_write_compress(int flags) {
  struct sftp_compress *comp; 
  z_stream *stream;
 
  comp = &(write_compresses[write_comp_idx]);
  stream = &(write_streams[write_comp_idx]);
 
  /* First we can free up the write stream, kept from rekeying. */
  if (comp->use_zlib == flags &&
      comp->stream_ready) {
 
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "done compressing data: compressed %" PR_LU " bytes to %" PR_LU
      " bytes of data (%.2f)", (pr_off_t) stream->total_in,
      (pr_off_t) stream->total_out,
      stream->total_in == 0 ? 0.0 :
        (float) stream->total_out / stream->total_in);

    deflateEnd(stream);
    comp->use_zlib = FALSE;
    comp->stream_ready = FALSE;

    /* Now we can switch the index. */
    if (write_comp_idx == 1) {
      write_comp_idx = 0;
      return;
    }

    write_comp_idx = 1;
  }
}

const char *sftp_compress_get_read_algo(void) {
  struct sftp_compress *comp;

  comp = &(read_compresses[read_comp_idx]);

  if (comp->use_zlib) {
    if (comp->use_zlib == SFTP_COMPRESS_FL_NEW_KEY) {
      return "zlib";
    }

    if (comp->use_zlib == SFTP_COMPRESS_FL_AUTHENTICATED) {
      return "zlib@openssh.com";
    }
  }

  return "none";
}

int sftp_compress_set_read_algo(const char *algo) {
  unsigned int idx = read_comp_idx;

  if (read_compresses[idx].stream_ready) {
    /* If we have an existing stream, it means that we are currently
     * rekeying.
     */
    idx = get_next_read_index();
  }

  if (strcmp(algo, "zlib@openssh.com") == 0) {
    read_compresses[idx].use_zlib = SFTP_COMPRESS_FL_AUTHENTICATED;
    return 0;
  }

  if (strcmp(algo, "zlib") == 0) {
    read_compresses[idx].use_zlib = SFTP_COMPRESS_FL_NEW_KEY;
    return 0;
  }

  if (strcmp(algo, "none") == 0) {
    return 0;
  }

  errno = EINVAL;
  return -1;
}

int sftp_compress_init_read(int flags) {
  struct sftp_compress *comp;
  z_stream *stream;

  switch_read_compress(flags);

  comp = &(read_compresses[read_comp_idx]);
  stream = &(read_streams[read_comp_idx]);

  if (comp->use_zlib == flags &&
      !comp->stream_ready) {
    int zres;

    zres = inflateInit(stream);
    if (zres != Z_OK) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error preparing decompression stream (%d)", zres);
    }

    comp->stream_ready = TRUE;
  }

  return 0;
}

int sftp_compress_read_data(struct ssh2_packet *pkt) {
  struct sftp_compress *comp;
  z_stream *stream;

  comp = &(read_compresses[read_comp_idx]);
  stream = &(read_streams[read_comp_idx]);

  if (comp->use_zlib &&
      comp->stream_ready) {
    unsigned char buf[16382], *input;
    char *payload;
    uint32_t input_len, payload_len = 0, payload_sz;
    pool *sub_pool;
    int zres;

    if (pkt->payload_len == 0) {
      return 0;
    }

    sub_pool = make_sub_pool(pkt->pool);

    /* Use a copy of the payload, rather than the actual payload itself,
     * as zlib may alter the payload contents and then encounter an error.
     */
    input_len = pkt->payload_len;
    input = palloc(sub_pool, input_len);
    memcpy(input, pkt->payload, input_len);

    /* Try to guess at how big the uncompressed data will be.  Optimistic
     * estimate, for now, will be a factor of 8.
     */
    payload_sz = input_len * 8;
    payload = palloc(sub_pool, payload_sz);

    stream->next_in = input;
    stream->avail_in = input_len;

    while (1) {
      size_t copy_len = 0;

      pr_signals_handle();

      stream->next_out = buf;
      stream->avail_out = sizeof(buf);

      zres = inflate(stream, Z_SYNC_FLUSH);
      if (zres == Z_OK) {
        copy_len = sizeof(buf) - stream->avail_out;

        /* Allocate more space for the data if necessary. */
        if ((payload_len + copy_len) > payload_sz) {
          uint32_t new_sz;
          char *tmp;

          new_sz = payload_sz * 2;
          tmp = palloc(sub_pool, new_sz);
          memcpy(tmp, payload, payload_len);
          payload = tmp;
        }

        if (copy_len > 0) {
          memcpy(payload + payload_len, buf, copy_len);
          payload_len += copy_len;
        }

        continue;

      } else if (zres == Z_BUF_ERROR) {
        /* Make sure that pkt->payload has enough room for the uncompressed
         * data.  If not, allocate a larger buffer.
         */
        if (pkt->payload_len < payload_len) {
          pkt->payload = palloc(pkt->pool, payload_len);
        }

        memcpy(pkt->payload, payload, payload_len);
        pkt->payload_len = payload_len;
        break;
      }

      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unhandled zlib error (%d) while decompressing", zres);
      destroy_pool(sub_pool);
      return -1;
    }

    destroy_pool(sub_pool);
  }

  return 0;
}

const char *sftp_compress_get_write_algo(void) {
  struct sftp_compress *comp;

  comp = &(write_compresses[write_comp_idx]);

  if (comp->use_zlib) {
    if (comp->use_zlib == SFTP_COMPRESS_FL_NEW_KEY) {
      return "zlib";
    }

    if (comp->use_zlib == SFTP_COMPRESS_FL_AUTHENTICATED) {
      return "zlib@openssh.com";
    }
  }

  return "none";
}

int sftp_compress_set_write_algo(const char *algo) {
  unsigned int idx = write_comp_idx;

  if (write_compresses[idx].stream_ready) {
    /* If we have an existing stream, it means that we are currently
     * rekeying. 
     */
    idx = get_next_write_index();
  }

  if (strcmp(algo, "zlib@openssh.com") == 0) {
    write_compresses[idx].use_zlib = SFTP_COMPRESS_FL_AUTHENTICATED;
    return 0;
  }

  if (strcmp(algo, "zlib") == 0) {
    write_compresses[idx].use_zlib = SFTP_COMPRESS_FL_NEW_KEY;
    return 0;
  }

  if (strcmp(algo, "none") == 0) {
    return 0;
  }

  errno = EINVAL;
  return -1;
}

int sftp_compress_init_write(int flags) {
  struct sftp_compress *comp;
  z_stream *stream;

  switch_write_compress(flags);

  comp = &(write_compresses[write_comp_idx]);
  stream = &(write_streams[write_comp_idx]);

  if (comp->use_zlib == flags &&
      !comp->stream_ready) {
    int zres;

    zres = deflateInit(stream, Z_DEFAULT_COMPRESSION);
    if (zres != Z_OK) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error preparing compression stream (%d)", zres);
    }

    comp->stream_ready = TRUE;
  }

  return 0;
}

int sftp_compress_write_data(struct ssh2_packet *pkt) {
  struct sftp_compress *comp;
  z_stream *stream;

  comp = &(write_compresses[write_comp_idx]);
  stream = &(write_streams[write_comp_idx]);

  if (comp->use_zlib &&
      comp->stream_ready) {
    unsigned char buf[8192], *input;
    char *payload;
    uint32_t input_len, payload_len = 0, payload_sz;
    pool *sub_pool;
    int zres;

    if (pkt->payload_len == 0) {
      return 0;
    }

    sub_pool = make_sub_pool(pkt->pool);

    /* Use a copy of the payload, rather than the actual payload itself,
     * as zlib may alter the payload contents and then encounter an error.
     */
    input_len = pkt->payload_len;
    input = palloc(sub_pool, input_len);
    memcpy(input, pkt->payload, input_len);

    /* Try to guess at how small the compressed data will be.  Optimistic
     * estimate, for now, will be a factor of 2, with a minimum of 1K.
     */
    payload_sz = 1024;
    if ((input_len * 2) > payload_sz) {
      payload_sz = input_len * 2;
    }
    payload = palloc(sub_pool, payload_sz);

    stream->next_in = input;
    stream->avail_in = input_len;
    stream->avail_out = 0;

    while (stream->avail_out == 0) {
      size_t copy_len = 0;

      pr_signals_handle();

      stream->next_out = buf;
      stream->avail_out = sizeof(buf);

      zres = deflate(stream, Z_SYNC_FLUSH);

      switch (zres) {
        case Z_OK:
          copy_len = sizeof(buf) - stream->avail_out;

          /* Allocate more space for the data if necessary. */
          if ((payload_len + copy_len) > payload_sz) {
            uint32_t new_sz;
            char *tmp;

            new_sz = payload_sz * 2;
            tmp = palloc(sub_pool, new_sz);
            memcpy(tmp, payload, payload_len);
            payload = tmp;
          }

          memcpy(payload + payload_len, buf, copy_len);
          payload_len += copy_len;
          break;

        default:
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "unhandled zlib error (%d) while compressing", zres);
          destroy_pool(sub_pool);
          return -1;
      }
    }

    if (payload_len > 0) {
      if (pkt->payload_len < payload_len) {
        pkt->payload = palloc(pkt->pool, payload_len);
      }

      memcpy(pkt->payload, payload, payload_len);
      pkt->payload_len = payload_len;
    }

    destroy_pool(sub_pool);
    return 0;
  }

  return 0;
}

