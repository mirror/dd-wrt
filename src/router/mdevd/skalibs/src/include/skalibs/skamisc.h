/* ISC license. */

#ifndef SKALIBS_SKAMISC_H
#define SKALIBS_SKAMISC_H

#include <string.h>

#include <skalibs/buffer.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>

extern stralloc satmp ;

extern int skagetln (buffer *, stralloc *, char) ;
extern int skagetln_loose (buffer *, stralloc *, char) ;
extern int skagetln_nofill (buffer *, stralloc *, char) ;
extern int skagetlnsep (buffer *, stralloc *, char const *, size_t) ;
extern int skagetlnsep_loose (buffer *, stralloc *, char const *, size_t) ;
extern int skagetlnmaxsep (buffer *, stralloc *, size_t, char const *, size_t) ;
extern int getlnmax (buffer *, char *, size_t, size_t *, char) ;
extern int getlnmaxsep (buffer *, char *, size_t, size_t *, char const *, size_t) ;

extern int sauniquename (stralloc *) ;

extern int string_quote_options (stralloc *, char const *, size_t, uint32_t) ;
#define string_quote(sa, s, len) string_quote_options(sa, s, (len), 0)
#define string_quote_nospace(sa, s, len) string_quote_options(sa, s, (len), 1)
#define string_quotes(sa, s) string_quote(sa, (s), strlen(s))

extern int string_quote_nodelim_mustquote_options (stralloc *, char const *, size_t, char const *, size_t, uint32_t) ;
#define string_quote_nodelim_mustquote(sa, s, len, delim, delimlen) string_quote_nodelim_mustquote_options(sa, s, len, delim, (delimlen), 0)
#define string_quote_nodelim_mustquote_nospace(sa, s, len, delim, delimlen) string_quote_nodelim_mustquote_options(sa, s, len, delim, (delimlen), 1)
#define string_quote_nodelim(sa, s, len) string_quote_nodelim_mustquote(sa, s, (len), "\"", 1)
#define string_quote_nodelim_nospace(sa, s, len) string_quote_nodelim_mustquote_nospace(sa, s, (len), "\"", 1)

extern int string_unquote (char *, size_t *, char const *, size_t, size_t *) ;
extern ssize_t string_unquote_nodelim (char *, char const *, size_t) ;
extern int string_unquote_withdelim (char *, size_t *, char const *, size_t, size_t *, char const *, size_t) ;

extern int string_format (stralloc *, char const *, char const *, char const *const *) ;
extern int string_index (char *, size_t, size_t, char, genalloc *) ;

#endif
