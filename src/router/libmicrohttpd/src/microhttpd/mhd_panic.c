/*
  This file is part of libmicrohttpd
  Copyright (C) 2007-2018 Daniel Pittman and Christian Grothoff
  Copyright (C) 2014-2022 Evgeny Grin (Karlson2k)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/**
 * @file microhttpd/mhd_panic.h
 * @brief  MHD_panic() function and helpers
 * @author Daniel Pittman
 * @author Christian Grothoff
 * @author Karlson2k (Evgeny Grin)
 */

#include "mhd_panic.h"
#include "platform.h"
#include "microhttpd.h"

/**
 * Handler for fatal errors.
 */
MHD_PanicCallback mhd_panic = (MHD_PanicCallback) NULL;

/**
 * Closure argument for #mhd_panic.
 */
void *mhd_panic_cls = NULL;


/**
 * Default implementation of the panic function,
 * prints an error message and aborts.
 *
 * @param cls unused
 * @param file name of the file with the problem
 * @param line line number with the problem
 * @param reason error message with details
 */
_MHD_NORETURN static void
mhd_panic_std (void *cls,
               const char *file,
               unsigned int line,
               const char *reason)
{
  (void) cls; /* Mute compiler warning. */
#ifdef HAVE_MESSAGES
  fprintf (stderr,
           _ ("Fatal error in GNU libmicrohttpd %s:%u: %s\n"),
           file,
           line,
           reason);
#else  /* ! HAVE_MESSAGES */
  (void) file;   /* Mute compiler warning. */
  (void) line;   /* Mute compiler warning. */
  (void) reason; /* Mute compiler warning. */
#endif
  abort ();
}


/**
 * Sets the global error handler to a different implementation.
 *
 * @a cb will only be called in the case of typically fatal, serious internal
 * consistency issues or serious system failures like failed lock of mutex.
 *
 * These issues should only arise in the case of serious memory corruption or
 * similar problems with the architecture, there is no safe way to continue
 * even for closing of the application.
 *
 * The default implementation that is used if no panic function is set simply
 * prints an error message and calls `abort()`.
 * Alternative implementations might call `exit()` or other similar functions.
 *
 * @param cb new error handler or NULL to use default handler
 * @param cls passed to @a cb
 * @ingroup logging
 */
_MHD_EXTERN void
MHD_set_panic_func (MHD_PanicCallback cb,
                    void *cls)
{
  if ((MHD_PanicCallback) NULL != cb)
    mhd_panic = cb;
  else
    mhd_panic = &mhd_panic_std;

  mhd_panic_cls = cls;
}
