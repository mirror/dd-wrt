/* Copyright (c) 2013
 *      Mike Gerwitz (mtg@gnu.org)
 *
 * This file is part of GNU screen.
 *
 * GNU screen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * <https://www.gnu.org/licenses>.
 *
 ****************************************************************
 */

#include "config.h"

#include "winmsgbuf.h"

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


/* Allocate and initialize to the empty string a new window message buffer. The
 * return value must be freed using wmbc_free. */
WinMsgBuf *wmb_create(void)
{
	WinMsgBuf *w = malloc(sizeof(WinMsgBuf));
	if (w == NULL)
		return NULL;

	w->buf = malloc(WINMSGBUF_SIZE);
	if (w->buf == NULL) {
		free(w);
		return NULL;
	}

	w->size = WINMSGBUF_SIZE;
	wmb_reset(w);
	return w;
}

/* Attempts to expand the buffer to hold at least MIN bytes. The new size of the
 * buffer is returned, which may be unchanged from the original size if
 * additional memory could not be allocated. */
size_t wmb_expand(WinMsgBuf *wmb, size_t min)
{
	size_t size = wmb->size;

	if (size >= min)
		return size;

	/* keep doubling the buffer until we reach at least the requested size; this
	 * ensures that we'll always be a power of two (so long as the original
	 * buffer size was) and doubling will help cut down on excessive allocation
	 * requests on large buffers */
	while (size < min) {
		size *= 2;
	}

	void *p = realloc(wmb->buf, size);
	if (p == NULL) {
		/* reallocation failed; maybe the caller can do without? */
		return wmb->size;
	}

	/* realloc already handled the free for us */
	wmb->buf = p;
	wmb->size = size;
	return size;
}

/* Add a rendition to the buffer */
void wmb_rendadd(WinMsgBuf *wmb, uint64_t r, int offset)
{
	/* TODO: lift arbitrary limit; dynamically allocate */
	if (wmb->numrend >= MAX_WINMSG_REND)
		return;

	wmb->rend[wmb->numrend] = r;
	wmb->rendpos[wmb->numrend] = offset;
	wmb->numrend++;
}

/* Retrieve buffer size. This returns the total size of the buffer, not how much
 * has been used. */
size_t wmb_size(const WinMsgBuf *wmb)
{
	return wmb->size;
}

/* Retrieve a pointer to the raw buffer contents. This should not be used to
 * modify the buffer. */
const char *wmb_contents(const WinMsgBuf *wmb)
{
	return wmb->buf;
}

/* Initializes window buffer to the empty string; useful for re-using an
 * existing buffer without allocating a new one. */
void wmb_reset(WinMsgBuf *w)
{
	*w->buf = '\0';
	w->numrend = 0;
}

/* Deinitialize and free memory allocated to the given window buffer */
void wmb_free(WinMsgBuf *w)
{
	free(w->buf);
	free(w);
}


/* Allocate and initialize a buffer context for the given buffer. The return
 * value must be freed using wmbc_free. */
WinMsgBufContext *wmbc_create(WinMsgBuf *w)
{
	if (w == NULL)
		return NULL;

	WinMsgBufContext *c = malloc(sizeof(WinMsgBufContext));
	if (c == NULL)
		return NULL;

	c->buf = w;
	c->p = w->buf;
	return c;
}

/* Rewind pointer to the first byte of the buffer. */
void wmbc_rewind(WinMsgBufContext *wmbc)
{
	wmbc->p = wmbc->buf->buf;
}

/* Place pointer at terminating null character. */
void wmbc_fastfw0(WinMsgBufContext *wmbc)
{
	wmbc->p += strlen(wmbc->p);
}

/* Place pointer just past the last byte in the buffer, ignoring terminating null
 * characters. The next write will trigger an expansion. */
void wmbc_fastfw_end(WinMsgBufContext *wmbc)
{
	wmbc->p = wmbc->buf->buf + wmbc->buf->size;
}

/* Attempts buffer expansion and updates context pointers appropriately. The
 * result is true if expansion succeeded, otherwise false. */
static bool _wmbc_expand(WinMsgBufContext *wmbc, size_t size)
{
	size_t offset = wmbc_offset(wmbc);

	if (wmb_expand(wmbc->buf, size) < size) {
		return false;
	}

	/* the buffer address may have changed; re-calculate pointer address */
	wmbc->p = wmbc->buf->buf + offset;
	return true;
}

/* Sets a character at the current buffer position and increments the pointer.
 * The terminating null character is not retained. The buffer will be
 * dynamically resized as needed. */
void wmbc_putchar(WinMsgBufContext *wmbc, char c)
{
	/* attempt to accomodate this character, but bail out silenty if it cannot
	 * fit */
	if (!wmbc_bytesleft(wmbc)) {
		if (!_wmbc_expand(wmbc, wmbc->buf->size + 1)) {
			return;
		}
	}

	*wmbc->p++ = c;
}

/* Copies a string into the buffer, dynamically resizing the buffer as needed to
 * accomodate length N. If S is shorter than N characters in length, the
 * remaining bytes are filled will nulls. The context pointer is adjusted to the
 * terminating null byte. A pointer to the first copied character in the buffer
 * is returned; it shall not be used to modify the buffer. */
const char *wmbc_strncpy(WinMsgBufContext *wmbc, const char *s, size_t n)
{
	size_t l = wmbc_bytesleft(wmbc);

	/* silently fail in the event that we cannot accomodate */
	if (l < n) {
		size_t size = wmbc->buf->size + (n - l);
		if (!_wmbc_expand(wmbc, size)) {
			/* TODO: we should copy what can fit. */
			return NULL;
		}
	}

	char *p = wmbc->p;
	strncpy(wmbc->p, s, n);
	wmbc->p += n;
	return p;
}

/* Copies a string into the buffer, dynamically resizing the buffer as needed to
 * accomodate the length of the string sans its terminating null byte. The
 * context pointer is adjusted to the the terminiating null byte. A pointer to
 * the first copied character in the destination buffer is returned; it shall
 * not be used to modify the buffer. */
const char *wmbc_strcpy(WinMsgBufContext *wmbc, const char *s)
{
	return wmbc_strncpy(wmbc, s, strlen(s));
}

/* Write data to the buffer using a printf-style format string. If needed, the
 * buffer will be automatically expanded to accomodate the resulting string and
 * is therefore protected against overflows. */
int wmbc_printf(WinMsgBufContext *wmbc, const char *fmt, ...)
{
	va_list ap;
	size_t  n, max;

	/* to prevent buffer overflows, cap the number of bytes to the remaining
	 * buffer size */
	va_start(ap, fmt);
	max = wmbc_bytesleft(wmbc);
	n = vsnprintf(wmbc->p, max, fmt, ap);
	va_end(ap);

	/* more space is needed if vsnprintf returns a larger number than our max,
	 * in which case we should accomodate by dynamically resizing the buffer and
	 * trying again */
	if (n > max) {
		if (!_wmbc_expand(wmbc, wmb_size(wmbc->buf) + n - max)) {
			/* failed to allocate additional memory; this will simply have to do */
			wmbc_fastfw_end(wmbc);
			return max;
		}

		va_start(ap, fmt);
		size_t m = vsnprintf(wmbc->p, n + 1, fmt, ap);
		assert(m == n); /* this should never fail */
		va_end(ap);
	}

	wmbc_fastfw0(wmbc);
	return n;
}

/* Retrieve the 0-indexed offset of the context pointer into the buffer */
size_t wmbc_offset(WinMsgBufContext *wmbc)
{
	ptrdiff_t offset = wmbc->p - wmbc->buf->buf;

	/* when using wmbc_* functions (as one always should), the offset should
	 * always be within the bounds of the buffer or one byte outside of it
	 * (the latter case would require an expansion before writing) */
	assert(offset > -1);
	assert((size_t)offset <= wmbc->buf->size);

	return (size_t)offset;
}

/* Calculate the number of bytes remaining in the buffer relative to the current
 * position within the buffer */
size_t wmbc_bytesleft(WinMsgBufContext *wmbc)
{
	return wmbc->buf->size - wmbc_offset(wmbc);
}

/* Merges the contents of another null-terminated buffer and its renditions. The
 * return value is a pointer to the first character of WMB's buffer. */
const char *wmbc_mergewmb(WinMsgBufContext *wmbc, WinMsgBuf *wmb)
{
	const char *p;
	size_t offset = wmbc_offset(wmbc);

	/* import buffer contents into our own at our current position */
	assert(wmb);
	p = wmbc_strcpy(wmbc, wmb->buf);

	/* merge renditions, adjusting them to reflect their new offset */
	for (int i = 0; i < wmb->numrend; i++) {
		wmb_rendadd(wmbc->buf, wmb->rend[i], offset + wmb->rendpos[i]);
	}

	return p;
}

/* Write a terminating null byte to the buffer and return a pointer to the
 * buffer contents. This should not be used to modify the buffer. If buffer is
 * full and expansion fails, then the last byte in the buffer will be replaced
 * with the null byte. */
const char *wmbc_finish(WinMsgBufContext *wmbc)
{
	if (!wmbc_bytesleft(wmbc)) {
		size_t size = wmbc->buf->size + 1;
		if (wmb_expand(wmbc->buf, size) < size) {
			/* we must terminate the string or we may cause big problems for the
			 * caller; overwrite the last char :x */
			wmbc->p--;
		}
	}

	*wmbc->p = '\0';
	return wmb_contents(wmbc->buf);
}

/* Deinitializes and frees previously allocated context. The contained buffer
 * must be freed separately; this function will not do so for you. */
void wmbc_free(WinMsgBufContext *c)
{
	free(c);
}
