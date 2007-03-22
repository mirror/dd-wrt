
/***

error.c - Error-handling subroutines

Written by Gerard Paul Java
Copyright (c) Gerard Paul Java 1997, 1998

This software is open source; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License in the included COPYING file for
details.

***/

#include <curses.h>
#include <panel.h>
#include <winops.h>
#include <msgboxes.h>
#include "deskman.h"
#include "attrs.h"
#include "error.h"
#include "log.h"

void write_error(char *msg, int daemonized)
{
    int response;

    if (daemonized)
        write_daemon_err(msg);
    else
        tx_errbox(msg, ANYKEY_MSG, &response);
}
