/* Wrapper for gnulib getopt headers

   Copyright (C) 2025 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#ifndef NETTLE_GETOPT_H
#define NETTLE_GETOPT_H

#ifndef __BEGIN_DECLS
# ifdef __cplusplus
#  define __BEGIN_DECLS extern "C" {
# else
#  define __BEGIN_DECLS /* nothing */
# endif
#endif
#ifndef __END_DECLS
# ifdef __cplusplus
#  define __END_DECLS }
# else
#  define __END_DECLS /* nothing */
# endif
#endif

#ifndef __THROW
# define __THROW
#endif

#ifndef __getopt_argv_const
# define __getopt_argv_const
#endif

#define _GL_ARG_NONNULL(x)

#include "lib/getopt-core.h"
#include "lib/getopt-ext.h"

#endif /* NETTLE_GETOPT_H */
