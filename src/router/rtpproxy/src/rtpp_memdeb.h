/*
 * Copyright (c) 2014 Sippy Software, Inc., http://www.sippysoft.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*#undef malloc
#define malloc(n) rtpp_memdeb_malloc((n), __FILE__, __LINE__, __func__)
#undef free
#define free(p) rtpp_memdeb_free((p), __FILE__, __LINE__, __func__)
#undef realloc
#define realloc(p,n) rtpp_memdeb_realloc((p), (n), __FILE__, __LINE__, __func__)
#undef strdup
#define strdup(p) rtpp_memdeb_strdup((p), __FILE__, __LINE__, __func__)
#undef asprintf
#define asprintf(pp, fmt, args...) rtpp_memdeb_asprintf((pp), (fmt), __FILE__, __LINE__, __func__, ## args)
#undef vasprintf
#define vasprintf(pp, fmt, vl) rtpp_memdeb_vasprintf((pp), (fmt), __FILE__, __LINE__, __func__, (vl))

void *rtpp_memdeb_malloc(size_t, const char *, int, const char *);
void rtpp_memdeb_free(void *, const char *, int, const char *);
void *rtpp_memdeb_realloc(void *, size_t,  const char *, int, const char *);
char *rtpp_memdeb_strdup(const char *, const char *, int, const char *);
int rtpp_memdeb_asprintf(char **, const char *, const char *, int, const char *, ...);
*/
//#include <stdarg.h>

//int rtpp_memdeb_vasprintf(char **, const char *, const char *, int, const char *, va_list);

//#define RTPP_CHECK_LEAKS 	1
