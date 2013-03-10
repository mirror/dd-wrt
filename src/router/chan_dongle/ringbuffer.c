/* 
   Copyright (C) 2009 - 2010
   
   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org
   
   Dmitry Vagin <dmitry2004@yandex.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include "memmem.h"
#include <string.h>			/* memchr() */

#include "ringbuffer.h"

EXPORT_DEF int rb_memcmp (const struct ringbuffer* rb, const char* mem, size_t len)
{
	size_t tmp;

	if (rb->used > 0 && len > 0 && rb->used >= len)
	{
		if ((rb->read + len) > rb->size)
		{
			tmp = rb->size - rb->read;
			if (memcmp (rb->buffer + rb->read, mem, tmp) == 0)
			{
				len -= tmp;
				mem += tmp;

				if (memcmp (rb->buffer, mem, len) == 0)
				{
					return 0;
				}
			}
		}
		else 
		{
			if (memcmp (rb->buffer + rb->read, mem, len) == 0)
			{
				return 0;
			}
		}

		return 1;
	}

	return -1;
}

/* ============================ READ ============================= */
EXPORT_DEF int rb_read_all_iov (const struct ringbuffer* rb, struct iovec iov[2])
{
	if (rb->used > 0)
	{
		if ((rb->read + rb->used) > rb->size)
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = rb->size - rb->read;
			iov[1].iov_base = rb->buffer;
			iov[1].iov_len  = rb->used - iov[0].iov_len;
			return 2;
		}
		else
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = rb->used;
			iov[1].iov_len  = 0;
			return 1;
		}
	}

	return 0;
}

EXPORT_DEF int rb_read_n_iov (const struct ringbuffer* rb, struct iovec iov[2], size_t len)
{
	if (rb->used < len)
	{
		return 0;
	}

	if (len > 0)
	{
		if ((rb->read + len) > rb->size)
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = rb->size - rb->read;
			iov[1].iov_base = rb->buffer;
			iov[1].iov_len  = len - iov[0].iov_len;
			return 2;
		}
		else
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = len;
			iov[1].iov_len  = 0;
			return 1;
		}
	}

	return 0;
}

EXPORT_DEF int rb_read_until_char_iov (const struct ringbuffer* rb, struct iovec iov[2], char c)
{
	void* p;

	if (rb->used > 0)
	{
		if ((rb->read + rb->used) > rb->size)
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = rb->size - rb->read;
			if ((p = memchr (iov[0].iov_base, c, iov[0].iov_len)) != NULL)
			{
				iov[0].iov_len = p - iov[0].iov_base;
				iov[1].iov_len = 0;
				return 1;
			}
		
			if ((p = memchr (rb->buffer, c, rb->used - iov[0].iov_len)) != NULL)
			{
				iov[1].iov_base = rb->buffer;
				iov[1].iov_len = p - rb->buffer;
				return 2;
			}
		}
		else 
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = rb->used;
			if ((p = memchr (iov[0].iov_base, c, iov[0].iov_len)) != NULL)
			{
				iov[0].iov_len = p - iov[0].iov_base;
				iov[1].iov_len = 0;
				return 1;
			}
		}
	}

	return 0;
}

EXPORT_DEF int rb_read_until_mem_iov (const struct ringbuffer* rb, struct iovec iov[2], const void* mem, size_t len)
{
	size_t	i;
	void*	p;

	if (len == 1)
	{
		return rb_read_until_char_iov (rb, iov, *((char*) mem));
	}

	if (rb->used > 0 && len > 0 && rb->used >= len)
	{
		if ((rb->read + rb->used) > rb->size)
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = rb->size - rb->read;
			if (iov[0].iov_len >= len)
			{
				if ((p = memmem (iov[0].iov_base, iov[0].iov_len, mem, len)) != NULL)
				{
					iov[0].iov_len = p - iov[0].iov_base;
					iov[1].iov_len = 0;
					return 1;
				}

				i = 1;
				iov[1].iov_base = iov[0].iov_base + iov[0].iov_len - len + 1;
			}
			else
			{
				i = len - iov[0].iov_len;
				iov[1].iov_base = iov[0].iov_base;
			}


			while (i < len)
			{
				if (memcmp (iov[1].iov_base, mem, len - i) == 0 && memcmp (rb->buffer, mem + i, i) == 0)
				{
					iov[0].iov_len = iov[1].iov_base - iov[0].iov_base;
					iov[1].iov_len = 0;
					return 1;
				}

				if (rb->used == iov[0].iov_len + i)
				{
					return 0;
				}

				iov[1].iov_base++;
				i++;
			}

			if (rb->used >= iov[0].iov_len + len)
			{
				if ((p = memmem (rb->buffer, rb->used - iov[0].iov_len, mem, len)) != NULL)
				{
					if (p == rb->buffer)
					{
						iov[1].iov_len = 0;
						return 1;
					}
				
					iov[1].iov_base = rb->buffer;
					iov[1].iov_len = p - rb->buffer;
					return 2;
				}
			}
		}
		else 
		{
			iov[0].iov_base = rb->buffer + rb->read;
			iov[0].iov_len  = rb->used;
			if ((p = memmem (iov[0].iov_base, iov[0].iov_len, mem, len)) != NULL)
			{
				iov[0].iov_len = p - iov[0].iov_base;
				iov[1].iov_len = 0;

				return 1;
			}
		}
	}

	return 0;
}

EXPORT_DEF size_t rb_read_upd (struct ringbuffer* rb, size_t len)
{
	size_t s;

	if (rb->used < len)
	{
		len = rb->used;
	}

	if (len > 0)
	{
		rb->used -= len;

		if (rb->used == 0)
		{
			rb->read  = 0;
			rb->write = 0;
		}
		else
		{
			s = rb->read + len;

			if (s >= rb->size)
			{
				rb->read = s - rb->size;
			}
			else
			{
				rb->read = s;
			}
		}
	}

	return len;
}

/* unused
static size_t rb_read (struct ringbuffer* rb, char* buf, size_t len)
{
	size_t s;

	if (rb->used < len)
	{
		len = rb->used;
	}

	if (len > 0)
	{
		s = rb->read + len;
		if (s > rb->size)
		{
			memmove (buf, rb->buffer + rb->read, rb->size - rb->read);
			memmove (buf + rb->size - rb->read, rb->buffer, s - rb->size);
			rb->read = s - rb->size;
		}
		else
		{
			memmove (buf, rb->buffer + rb->read, len);
			if (s == rb->size)
			{
				rb->read = 0;
			}
			else
			{
				rb->read = s;
			}
		}

		rb->used -= len;

		if (rb->used == 0)
		{
			rb->read  = 0;
			rb->write = 0;
		}
	}

	return len;
}
*/

/* ============================ WRITE ============================ */

EXPORT_DEF int rb_write_iov (const struct ringbuffer* rb, struct iovec iov[2])
{
	size_t free;

	free = rb_free (rb);
	if (free > 0)
	{
		if ((rb->write + free) > rb->size)
		{
			iov[0].iov_base = rb->buffer + rb->write;
			iov[0].iov_len  = rb->size - rb->write;
			iov[1].iov_base = rb->buffer;
			iov[1].iov_len  = free - iov[0].iov_len;

			return 2;
		}
		else
		{
			iov[0].iov_base = rb->buffer + rb->write;
			iov[0].iov_len  = free;

			return 1;
		}
	}

	return 0;
}

EXPORT_DEF size_t rb_write_upd (struct ringbuffer* rb, size_t len)
{
	size_t free;
	size_t s;

	free = rb_free (rb);
	if (free < len)
	{
		len = free;
	}

	if (len > 0)
	{
		s = rb->write + len;

		if (s > rb->size)
		{
			rb->write = s - rb->size;
		}
		else
		{
			rb->write = s;
		}

		rb->used += len;
	}

	return len;
}

EXPORT_DEF size_t rb_write_core (struct ringbuffer* rb, const char* buf, size_t len, rb_write_f method)
{
	size_t	free;
	size_t	s;

	free = rb_free (rb);
	if (free < len)
	{
		len = free;
	}

	if (len > 0)
	{
		s = rb->write + len;

		if (s > rb->size)
		{
			(*method) (rb->buffer + rb->write, buf, rb->size - rb->write);
			(*method) (rb->buffer, buf + rb->size - rb->write, s - rb->size);
			rb->write = s - rb->size;
		}
		else
		{
			(*method) (rb->buffer + rb->write, buf, len);
			if (s == rb->size)
			{
				rb->write = 0;
			}
			else
			{
				rb->write = s;
			}
		}

		rb->used += len;
	}

	return len;
}
