#ifndef RULES_BUFFER_H
#define RULES_BUFFER_H 1

#include <stdlib.h>

struct buffer_t {
	char	*buf;
	size_t	bufsize;
	size_t	bufpos;
};

void buffer_init(struct buffer_t *);
void buffer_push(struct buffer_t *, int);
void buffer_flush(struct buffer_t *);
void buffer_clear(struct buffer_t *);

#endif /* RULES_BUFFER_H */
