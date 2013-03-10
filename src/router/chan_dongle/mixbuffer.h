/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/

#ifndef CHAN_DONGLE_MIXBUFFER_H_INCLUDED
#define CHAN_DONGLE_MIXBUFFER_H_INCLUDED

#include <asterisk.h>
#include <asterisk/linkedlists.h>		/* AST_LIST_ENTRY() AST_LIST_HEAD_NOLOCK() */

#include "ringbuffer.h"

struct mixstream {
	AST_LIST_ENTRY(mixstream)		entry;
	size_t					used;			/*!< number of bytes used */
	size_t					write;			/*!< write position */
};

struct mixbuffer {
	AST_LIST_HEAD_NOLOCK(,mixstream)	streams;	/*!< list of stream descriptions */
	struct ringbuffer			rb;		/*!< base */
	unsigned				attached;	/*!< number of attached streams */
	};

/* initialize mixbuffer */
INLINE_DECL void mixb_init(struct mixbuffer * mb, void * buf, size_t len)
{
	AST_LIST_HEAD_INIT_NOLOCK(&mb->streams);
	rb_init(&mb->rb, buf, len);
	mb->attached = 0;
}

/* attach stream to mix buffer */
EXPORT_DECL void mixb_attach(struct mixbuffer * mb, struct mixstream * stream);

/* detach stream from mix buffer */
EXPORT_DECL void mixb_detach(struct mixbuffer * mb, struct mixstream * stream);

/* get amount of free bytes in buffer for specified stream */
INLINE_DECL size_t mixb_free (const struct mixbuffer * mb, const struct mixstream * stream)
{
	return mb->rb.size - stream->used;
}

/* get bytes used i.e. now may bytes can read */
INLINE_DECL size_t mixb_used(const struct mixbuffer * mb)
{
	return rb_used(&mb->rb);
}

/* advice read position */
EXPORT_DECL size_t mixb_read_upd(struct mixbuffer * mb, size_t len);

/* add data to mix buffer for specified stream */
EXPORT_DECL size_t mixb_write(struct mixbuffer * mb, struct mixstream * stream, const char * data, size_t len);

/* get data pointer and sizes in iov for all available for reading data in buffer */
INLINE_DECL int mixb_read_all_iov (const struct mixbuffer * mb, struct iovec iov[2])
{
	return rb_read_all_iov(&mb->rb, iov);
}

/* get data pointer and sizes in iov only for first len bytes */
INLINE_DECL int mixb_read_n_iov (const struct mixbuffer * mb, struct iovec iov[2], size_t len)
{
	return rb_read_n_iov(&mb->rb, iov, len);
}

/* get number of attached streams */
INLINE_DECL int mixb_streams (const struct mixbuffer * mb)
{
	return mb->attached;
}

#endif /* CHAN_DONGLE_MIXBUFFER_H_INCLUDED */
