/* Return backtrace of current program state.
   Copyright (C) 1998-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
	int cnt, size;
	void *lastebp, *lastesp;
};

static _Unwind_Reason_Code (*unwind_backtrace)(_Unwind_Trace_Fn, void *);
static _Unwind_Ptr (*unwind_getip)(struct _Unwind_Context *);
static _Unwind_Ptr (*unwind_getcfa)(struct _Unwind_Context *);
static _Unwind_Ptr (*unwind_getgr)(struct _Unwind_Context *, int);
static void *libgcc_handle;

static void init(void)
{
	libgcc_handle = dlopen("libgcc_s.so.1", RTLD_LAZY);

	if (libgcc_handle == NULL)
		return;

	unwind_backtrace = dlsym(libgcc_handle, "_Unwind_Backtrace");
	unwind_getip = dlsym(libgcc_handle, "_Unwind_GetIP");
	unwind_getcfa = dlsym(libgcc_handle, "_Unwind_GetCFA");
	unwind_getgr = dlsym(libgcc_handle, "_Unwind_GetGR");
	if (unwind_getip == NULL || unwind_getgr == NULL || unwind_getcfa == NULL) {
		unwind_backtrace = NULL;
		dlclose(libgcc_handle);
		libgcc_handle = NULL;
	}
}

static _Unwind_Reason_Code backtrace_helper(struct _Unwind_Context *ctx, void *a)
{
	struct trace_arg *arg = a;

	/* We are first called with address in the __backtrace function.
     Skip it.  */
	if (arg->cnt != -1)
		arg->array[arg->cnt] = (void *)unwind_getip(ctx);
	if (++arg->cnt == arg->size)
		return _URC_END_OF_STACK;

	/* %ebp is DWARF2 register 5 on IA-32.  */
	arg->lastebp = (void *)unwind_getgr(ctx, 5);
	arg->lastesp = (void *)unwind_getcfa(ctx);
	return _URC_NO_REASON;
}

/* This is a global variable set at program start time.  It marks the
   highest used stack address.  */
extern void *__libc_stack_end;

/* This is the stack layout we see with every stack frame
   if not compiled without frame pointer.

            +-----------------+        +-----------------+
    %ebp -> | %ebp last frame--------> | %ebp last frame--->...
            |                 |        |                 |
            | return address  |        | return address  |
            +-----------------+        +-----------------+

   First try as far to get as far as possible using
   _Unwind_Backtrace which handles -fomit-frame-pointer
   as well, but requires .eh_frame info.  Then fall back to
   walking the stack manually.  */

struct layout {
	struct layout *ebp;
	void *ret;
};

static int backtrace(void **array, int size)
{
	struct trace_arg arg = { .array = array, .size = size, .cnt = -1 };

	if (size <= 0)
		return 0;

	__libc_once_define(static, once);

	__libc_once(once, init);
	if (unwind_backtrace == NULL)
		return 0;

	unwind_backtrace(backtrace_helper, &arg);

	if (arg.cnt > 1 && arg.array[arg.cnt - 1] == NULL)
		--arg.cnt;
	else if (arg.cnt < size) {
		struct layout *ebp = (struct layout *)arg.lastebp;

		while (arg.cnt < size) {
			/* Check for out of range.  */
			if ((void *)ebp < arg.lastesp || (void *)ebp > __libc_stack_end || ((long)ebp & 3))
				break;

			array[arg.cnt++] = ebp->ret;
			ebp = ebp->ebp;
		}
	}
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
