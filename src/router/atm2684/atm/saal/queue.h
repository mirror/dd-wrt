/* queue.h - Quick and very very dirty buffer and queue handling */

/* Written 1995-1996 by Werner Almesberger, EPFL-LRC */
 

#ifndef QUEUE_H
#define QUEUE_H

typedef struct _buffer {
    void *data;
    int length; /* TX buffer: of the entire PDU; RX buffer: of the data */
    int key; /* SD.N(S) */
    int extra; /* SD.N(PS) */
    struct _buffer *next,*prev;
} BUFFER;

typedef struct {
    BUFFER *first,*last;
} QUEUE;


BUFFER *buffer_create(int length,int key);
BUFFER *buffer_clone(BUFFER *b);
void buffer_discard(BUFFER *b);
void queue_init(QUEUE *q);
void queue_put(QUEUE *q,BUFFER *b);
void queue_remove(QUEUE *q,BUFFER *b);
BUFFER *queue_peek(QUEUE *q);
BUFFER *queue_get(QUEUE *q);
BUFFER *queue_lookup(QUEUE *q,int key);
void queue_clear(QUEUE *q);

#endif
