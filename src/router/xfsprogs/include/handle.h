// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (c) 1995, 2001-2003, 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __HANDLE_H__
#define __HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif

struct fsdmidata;
struct attrlist_cursor;
struct parent;

extern int  path_to_handle (char *__path, void **__hanp, size_t *__hlen);
extern int  path_to_fshandle (char *__path, void **__fshanp, size_t *__fshlen);
extern int  fd_to_handle (int fd, void **hanp, size_t *hlen);
extern int  handle_to_fshandle (void *__hanp, size_t __hlen, void **__fshanp,
				size_t *__fshlen);
extern void free_handle (void *__hanp, size_t __hlen);
extern int  open_by_fshandle (void *__fshanp, size_t __fshlen, int __rw);
extern int  open_by_handle (void *__hanp, size_t __hlen, int __rw);
extern int  readlink_by_handle (void *__hanp, size_t __hlen, void *__buf,
				size_t __bs);
extern int  attr_multi_by_handle (void *__hanp, size_t __hlen, void *__buf,
				  int __rtrvcnt, int __flags);
extern int  attr_list_by_handle (void *__hanp, size_t __hlen, void *__buf,
				 size_t __bufsize, int __flags,
				 struct attrlist_cursor *__cursor);
extern int  parents_by_handle(void *__hanp, size_t __hlen,
			      struct parent *__buf, size_t __bufsize,
			      unsigned int *__count);
extern int  parentpaths_by_handle(void *__hanp, size_t __hlen,
				  struct parent *__buf, size_t __bufsize,
				  unsigned int *__count);
extern int  fssetdm_by_handle (void *__hanp, size_t __hlen,
			       struct fsdmidata *__fsdmi);

void fshandle_destroy(void);

#ifdef __cplusplus
}
#endif

#endif	/* __HANDLE_H__ */
