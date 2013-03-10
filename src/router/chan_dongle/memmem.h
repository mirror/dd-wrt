/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_MEMMEM_H_INCLUDED
#define CHAN_DONGLE_MEMMEM_H_INCLUDED

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_MEMMEM

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */
#include <string.h>

#else /* HAVE_MEMMEM */

#include <sys/types.h>			/* size_t */
#include "export.h"			/* EXPORT_DECL EXPORT_DEF */

EXPORT_DECL void * memmem(const void *l, size_t l_len, const void *s, size_t s_len);

#endif /* HAVE_MEMMEM */
#endif /* CHAN_DONGLE_MANAGER_H_INCLUDED */
