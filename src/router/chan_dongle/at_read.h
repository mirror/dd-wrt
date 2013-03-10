/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_AT_READ_H_INCLUDED
#define CHAN_DONGLE_AT_READ_H_INCLUDED

#include "at_response.h"		/* at_res_t */
#include "export.h"			/* EXPORT_DECL EXPORT_DEF */

struct pvt;
struct ringbuffer;
struct iovec;

EXPORT_DECL int at_wait (int fd, int* ms);
EXPORT_DECL ssize_t at_read (int fd, const char * dev, struct ringbuffer* rb);
EXPORT_DECL int at_read_result_iov (const char * dev, int * read_result, struct ringbuffer* rb, struct iovec * iov);
EXPORT_DECL at_res_t at_read_result_classification (struct ringbuffer * rb, size_t len);

#endif /* CHAN_DONGLE_AT_READ_H_INCLUDED */
