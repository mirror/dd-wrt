/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_MX32_SELINUX_RUNTIME

# define TEST_SECONTEXT
# include "fchownat.c"

#else

SKIP_MAIN_UNDEFINED("HAVE_MX32_SELINUX_RUNTIME")

#endif
