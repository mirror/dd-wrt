/*
   Internal file viewer for the Midnight Commander
   Function for work with growing bufers

   Copyright (C) 1994-2014
   Free Software Foundation, Inc.

   Written by:
   Miguel de Icaza, 1994, 1995, 1998
   Janne Kukonlehto, 1994, 1995
   Jakub Jelinek, 1995
   Joseph M. Hinkle, 1996
   Norbert Warmuth, 1997
   Pavel Machek, 1998
   Roland Illig <roland.illig@gmx.de>, 2004, 2005
   Slava Zanko <slavazanko@google.com>, 2009
   Andrew Borodin <aborodin@vmail.ru>, 2009
   Ilia Maslakov <il.smind@gmail.com>, 2009

   This file is part of the Midnight Commander.

   The Midnight Commander is free software: you can redistribute it
   and/or modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the License,
   or (at your option) any later version.

   The Midnight Commander is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <errno.h>

#include "lib/global.h"
#include "lib/vfs/vfs.h"
#include "lib/util.h"
#include "lib/widget.h"         /* D_NORMAL */

#include "internal.h"

/* Block size for reading files in parts */
#define VIEW_PAGE_SIZE ((size_t) 8192)

/*** global variables ****************************************************************************/

/*** file scope macro definitions ****************************************************************/

/*** file scope type declarations ****************************************************************/

/*** file scope variables ************************************************************************/

/*** file scope functions ************************************************************************/
/* --------------------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------------------- */
/*** public functions ****************************************************************************/
/* --------------------------------------------------------------------------------------------- */

void
mcview_growbuf_init (mcview_t * view)
{
    view->growbuf_in_use = TRUE;
    view->growbuf_blockptr = g_ptr_array_new ();
    view->growbuf_lastindex = VIEW_PAGE_SIZE;
    view->growbuf_finished = FALSE;
}

/* --------------------------------------------------------------------------------------------- */

void
mcview_growbuf_free (mcview_t * view)
{
#ifdef HAVE_ASSERT_H
    assert (view->growbuf_in_use);
#endif

    g_ptr_array_foreach (view->growbuf_blockptr, (GFunc) g_free, NULL);

    (void) g_ptr_array_free (view->growbuf_blockptr, TRUE);

    view->growbuf_blockptr = NULL;
    view->growbuf_in_use = FALSE;
}

/* --------------------------------------------------------------------------------------------- */

off_t
mcview_growbuf_filesize (mcview_t * view)
{
#ifdef HAVE_ASSERT_H
    assert (view->growbuf_in_use);
#endif

    if (view->growbuf_blockptr->len == 0)
        return 0;
    else
        return ((off_t) view->growbuf_blockptr->len - 1) * VIEW_PAGE_SIZE + view->growbuf_lastindex;
}

/* --------------------------------------------------------------------------------------------- */
/** Copies the output from the pipe to the growing buffer, until either
 * the end-of-pipe is reached or the interval [0..ofs) of the growing
 * buffer is completely filled.
 */

void
mcview_growbuf_read_until (mcview_t * view, off_t ofs)
{
    ssize_t nread;
    gboolean short_read;

#ifdef HAVE_ASSERT_H
    assert (view->growbuf_in_use);
#endif

    if (view->growbuf_finished)
        return;

    short_read = FALSE;
    while (mcview_growbuf_filesize (view) < ofs || short_read)
    {
        byte *p;
        size_t bytesfree;

        if (view->growbuf_lastindex == VIEW_PAGE_SIZE)
        {
            /* Append a new block to the growing buffer */
            byte *newblock = g_try_malloc (VIEW_PAGE_SIZE);
            if (newblock == NULL)
                return;

            g_ptr_array_add (view->growbuf_blockptr, newblock);
            view->growbuf_lastindex = 0;
        }
        p = g_ptr_array_index (view->growbuf_blockptr,
                               view->growbuf_blockptr->len - 1) + view->growbuf_lastindex;

        bytesfree = VIEW_PAGE_SIZE - view->growbuf_lastindex;

        if (view->datasource == DS_STDIO_PIPE)
        {
            nread = fread (p, 1, bytesfree, view->ds_stdio_pipe);
            if (nread == 0)
            {
                view->growbuf_finished = TRUE;
                (void) pclose (view->ds_stdio_pipe);
                mcview_display (view);
                close_error_pipe (D_NORMAL, NULL);
                view->ds_stdio_pipe = NULL;
                return;
            }
        }
        else
        {
#ifdef HAVE_ASSERT_H
            assert (view->datasource == DS_VFS_PIPE);
#endif
            do
            {
                nread = mc_read (view->ds_vfs_pipe, p, bytesfree);
            }
            while (nread == -1 && errno == EINTR);
            if (nread == -1 || nread == 0)
            {
                view->growbuf_finished = TRUE;
                (void) mc_close (view->ds_vfs_pipe);
                view->ds_vfs_pipe = -1;
                return;
            }
        }
        short_read = ((size_t) nread < bytesfree);
        view->growbuf_lastindex += nread;
    }
}

/* --------------------------------------------------------------------------------------------- */

gboolean
mcview_get_byte_growing_buffer (mcview_t * view, off_t byte_index, int *retval)
{
    off_t pageno;
    off_t pageindex;

    if (retval != NULL)
        *retval = -1;

    pageno = byte_index / VIEW_PAGE_SIZE;
    pageindex = byte_index % VIEW_PAGE_SIZE;

#ifdef HAVE_ASSERT_H
    assert (view->growbuf_in_use);
#endif

    if (pageno < 0)
        return FALSE;

    mcview_growbuf_read_until (view, byte_index + 1);
    if (view->growbuf_blockptr->len == 0)
        return FALSE;
    if (pageno < (off_t) view->growbuf_blockptr->len - 1)
    {
        if (retval != NULL)
            *retval = *((byte *) (g_ptr_array_index (view->growbuf_blockptr, pageno) + pageindex));
        return TRUE;
    }
    if (pageno == (off_t) view->growbuf_blockptr->len - 1
        && pageindex < (off_t) view->growbuf_lastindex)
    {
        if (retval != NULL)
            *retval = *((byte *) (g_ptr_array_index (view->growbuf_blockptr, pageno) + pageindex));
        return TRUE;
    }
    return FALSE;
}

/* --------------------------------------------------------------------------------------------- */

char *
mcview_get_ptr_growing_buffer (mcview_t * view, off_t byte_index)
{
    off_t pageno = byte_index / VIEW_PAGE_SIZE;
    off_t pageindex = byte_index % VIEW_PAGE_SIZE;

#ifdef HAVE_ASSERT_H
    assert (view->growbuf_in_use);
#endif

    if (pageno < 0)
        return NULL;

    mcview_growbuf_read_until (view, byte_index + 1);
    if (view->growbuf_blockptr->len == 0)
        return NULL;
    if (pageno < (off_t) view->growbuf_blockptr->len - 1)
        return (char *) (g_ptr_array_index (view->growbuf_blockptr, pageno) + pageindex);
    if (pageno == (off_t) view->growbuf_blockptr->len - 1
        && pageindex < (off_t) view->growbuf_lastindex)
        return (char *) (g_ptr_array_index (view->growbuf_blockptr, pageno) + pageindex);
    return NULL;
}

/* --------------------------------------------------------------------------------------------- */
