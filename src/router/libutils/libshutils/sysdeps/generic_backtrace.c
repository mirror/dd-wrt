/* Return backtrace of current program state.
   Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <dlfcn.h>
#include <stdlib.h>
#include <unwind.h>

#define __libc_once_define(CLASS, NAME) CLASS int NAME = 0
#define __libc_once(ONCE_CONTROL, INIT_FUNCTION) \
	do {                                     \
		if ((ONCE_CONTROL) == 0) {       \
			INIT_FUNCTION();         \
			(ONCE_CONTROL) = 1;      \
		}                                \
	} while (0)

struct trace_arg {
	void **array;
	_Unwind_Word cfa;
	int cnt;
	int size;
};

static _Unwind_Reason_Code (*unwind_backtrace)(_Unwind_Trace_Fn, void *);
static _Unwind_Ptr (*unwind_getip)(struct _Unwind_Context *);
static _Unwind_Word (*unwind_getcfa)(struct _Unwind_Context *);
static void *libgcc_handle;

/* Dummy version in case libgcc_s does not contain the real code.  */
static _Unwind_Word dummy_getcfa(struct _Unwind_Context *ctx __attribute__((unused)))
{
	return 0;
}

static void init(void)
{
	libgcc_handle = dlopen("libgcc_s.so.1", RTLD_LAZY);

	if (libgcc_handle == NULL)
		return;

	unwind_backtrace = dlsym(libgcc_handle, "_Unwind_Backtrace");
	unwind_getip = dlsym(libgcc_handle, "_Unwind_GetIP");
	if (unwind_getip == NULL)
		unwind_backtrace = NULL;
	unwind_getcfa = (dlsym(libgcc_handle, "_Unwind_GetCFA") ?: dummy_getcfa);
}

static _Unwind_Reason_Code backtrace_helper(struct _Unwind_Context *ctx, void *a)
{
	struct trace_arg *arg = a;

	/* We are first called with address in the __backtrace function.
     Skip it.  */
	if (arg->cnt != -1) {
		arg->array[arg->cnt] = (void *)unwind_getip(ctx);

		/* Check whether we make any progress.  */
		_Unwind_Word cfa = unwind_getcfa(ctx);

		if (arg->cnt > 0 && arg->array[arg->cnt - 1] == arg->array[arg->cnt] && cfa == arg->cfa)
			return _URC_END_OF_STACK;
		arg->cfa = cfa;
	}
	if (++arg->cnt == arg->size)
		return _URC_END_OF_STACK;
	return _URC_NO_REASON;
}

static int backtrace(void **array, int size)
{
	struct trace_arg arg = { .array = array, .cfa = 0, .size = size, .cnt = -1 };

	if (size <= 0)
		return 0;

	__libc_once_define(static, once);

	__libc_once(once, init);
	if (unwind_backtrace == NULL)
		return 0;

	unwind_backtrace(backtrace_helper, &arg);

	/* _Unwind_Backtrace seems to put NULL address above
     _start.  Fix it up here.  */
	if (arg.cnt > 1 && arg.array[arg.cnt - 1] == NULL)
		--arg.cnt;
	return arg.cnt != -1 ? arg.cnt : 0;
}

/* Free all resources if necessary.  */
static void backtrace_release(void)
{
	unwind_backtrace = NULL;
	if (libgcc_handle != NULL) {
		dlclose(libgcc_handle);
		libgcc_handle = NULL;
	}
}
