/*
   Copyright (C) 2010 bg <bg_one@mail.ru>
*/
#ifndef CHAN_DONGLE_EXPORT_H_INCLUDED
#define CHAN_DONGLE_EXPORT_H_INCLUDED

#ifdef BUILD_SINGLE

#define EXPORT_DEF		static
#define EXPORT_DECL		static
#define INLINE_DECL		static inline
#else /* BUILD_SINGLE */

#define EXPORT_DEF
#define EXPORT_DECL		extern
#define INLINE_DECL		static inline

#endif /* BUILD_SINGLE */
#endif /* CHAN_DONGLE_EXPORT_H_INCLUDED */
