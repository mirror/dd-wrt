/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <asterisk.h>
#include <asterisk/utils.h>			/* ast_slinear_saturated_add() */

#include "mixbuffer.h"

#/* */
EXPORT_DEF void mixb_attach(struct mixbuffer * mb, struct mixstream * stream)
{
	stream->entry.next = NULL;
	stream->used = 0;
	stream->write = mb->rb.read;
	AST_LIST_INSERT_TAIL(&mb->streams, stream, entry);
	mb->attached++;
}

#/* */
EXPORT_DEF void mixb_detach(struct mixbuffer * mb, struct mixstream * stream)
{
	mb->attached--;
	AST_LIST_REMOVE(&mb->streams, stream, entry);
}

#/* TODO: move up */
static void * saturated_sum(void* s1, const void *s2, size_t n)
{
	short* s11 = s1;
	short* s22 = (short*)s2;

	/* FIXME: odd bytes */
	for(n /= 2; n; n--, s11++, s22++)
		ast_slinear_saturated_add(s11, s22);

	return s1;
}

#/* function not update rb */
static inline size_t mixb_mix_write(struct mixbuffer * mb, struct mixstream * stream, const char * data, size_t len)
{
	size_t rv;
	/* save global state */
	size_t save_write = mb->rb.write;
	size_t save_used = mb->rb.used;
	
	/* load local state */
	mb->rb.write = stream->write;
	mb->rb.used = stream->used;

	rv = rb_write_core(&mb->rb, data, len, saturated_sum);

	/* update local state */
	stream->write = mb->rb.write;
	stream->used = mb->rb.used;

	/* restore global state */
	mb->rb.used = save_used;
	mb->rb.write = save_write;

	return rv;
}

#/* */
EXPORT_DEF size_t mixb_write(struct mixbuffer * mb, struct mixstream * stream, const char * data, size_t len)
{
	/* local state: how many data you fit? */
	size_t max_mix = mixb_free(mb, stream);

	if(max_mix < len)
		len = max_mix;

	if(len > 0)
	{
		max_mix = mb->rb.used - stream->used;
		if(len > max_mix)
		{
			/* optitional Mix followed by copy */
			if(max_mix)
				mixb_mix_write(mb, stream, data, max_mix);
			rb_write(&mb->rb, data + max_mix, len - max_mix);

			/* save local state */
			stream->write = mb->rb.write;
			stream->used = mb->rb.used;
		}
		else
		{
			/* Mix only */
			mixb_mix_write(mb, stream, data, len);
		}
	}

	return len;
}

#/* */
EXPORT_DEF size_t mixb_read_upd(struct mixbuffer * mb, size_t len)
{
	struct mixstream * stream;

	// NOTE: change used, read and also can change write
	size_t rv = rb_read_upd(&mb->rb, len);

	// all streams has one read but differ used
	AST_LIST_TRAVERSE(&mb->streams, stream, entry) {
		if(stream->used > len)
			stream->used -= len;
		else
			stream->used = 0;
		stream->write = mb->rb.read + stream->used;
		if(stream->write >= mb->rb.size)
			stream->write -= mb->rb.size;
	}

	return rv;
}
