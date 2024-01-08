/* Return backtrace of current program state.
   Copyright (C) 2008-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kazu Hirata <kazu@codesourcery.com>, 2008.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
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
};

static _Unwind_Reason_Code (*unwind_backtrace)(_Unwind_Trace_Fn, void *);
static _Unwind_VRS_Result (*unwind_vrs_get)(_Unwind_Context *, _Unwind_VRS_RegClass, _uw, _Unwind_VRS_DataRepresentation, void *);

static void *libgcc_handle;

static void init(void)
{
	libgcc_handle = dlopen("libgcc_s.so.1", RTLD_LAZY);

	if (libgcc_handle == NULL)
		return;

	unwind_backtrace = dlsym(libgcc_handle, "_Unwind_Backtrace");
	unwind_vrs_get = dlsym(libgcc_handle, "_Unwind_VRS_Get");
	if (unwind_vrs_get == NULL)
		unwind_backtrace = NULL;
}

/* This function is identical to "_Unwind_GetGR", except that it uses
   "unwind_vrs_get" instead of "_Unwind_VRS_Get".  */
static inline _Unwind_Word unwind_getgr(_Unwind_Context *context, int regno)
{
	_uw val;
	unwind_vrs_get(context, _UVRSC_CORE, regno, _UVRSD_UINT32, &val);
	return val;
}

/* This macro is identical to the _Unwind_GetIP macro, except that it
   uses "unwind_getgr" instead of "_Unwind_GetGR".  */
#define unwind_getip(context) (unwind_getgr(context, 15) & ~(_Unwind_Word)1)

static _Unwind_Reason_Code backtrace_helper(struct _Unwind_Context *ctx, void *a)
{
	struct trace_arg *arg = a;

	/* We are first called with address in the __backtrace function.
     Skip it.  */
	if (arg->cnt != -1)
		arg->array[arg->cnt] = (void *)unwind_getip(ctx);
	if (++arg->cnt == arg->size)
		return _URC_END_OF_STACK;
	return _URC_NO_REASON;
}

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
