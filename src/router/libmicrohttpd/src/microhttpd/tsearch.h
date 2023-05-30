/*-
 * Written by J.T. Conklin <jtc@netbsd.org>
 * Public domain.
 *
 *	$NetBSD: search.h,v 1.12 1999/02/22 10:34:28 christos Exp $
 */

#ifndef _TSEARCH_H_
#define _TSEARCH_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

void    *tdelete (const void *, void **,
                  int (*)(const void *, const void *));

void    *tfind (const void *, void * const *,
                int (*)(const void *, const void *));

void    *tsearch (const void *, void **,
                  int (*)(const void *, const void *));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_TSEARCH_H_ */
