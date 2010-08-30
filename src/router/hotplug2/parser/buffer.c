#include "buffer.h"

void buffer_init(struct buffer_t *buffer) {
	buffer->bufsize = 64;
	buffer->bufpos = 0;
	buffer->buf = malloc(buffer->bufsize);
}

void buffer_push(struct buffer_t *buffer, int c) {
	buffer->buf[buffer->bufpos++] = (char)c;
	if (buffer->bufpos >= buffer->bufsize) {
		buffer->bufsize *= 2;
		buffer->buf = realloc(buffer->buf, buffer->bufsize);
	}
}

void buffer_flush(struct buffer_t *buffer) {
	buffer->bufpos = 0;
}

void buffer_clear(struct buffer_t *buffer) {
	free(buffer->buf);
	buffer->bufsize = buffer->bufpos = 0;
}
