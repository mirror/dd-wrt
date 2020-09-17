// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

extern void	*xcalloc(size_t nelem, size_t elsize);
extern void	xfree(void *ptr);
extern void	*xmalloc(size_t size);
extern void	*xrealloc(void *ptr, size_t size);
extern char	*xstrdup(const char *s1);
