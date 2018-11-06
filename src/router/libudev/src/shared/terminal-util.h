#pragma once

/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#include "macro.h"
#include "time-util.h"

#define ANSI_HIGHLIGHT_ON "\x1B[1;39m"
#define ANSI_RED_ON "\x1B[31m"
#define ANSI_HIGHLIGHT_RED_ON "\x1B[1;31m"
#define ANSI_GREEN_ON "\x1B[32m"
#define ANSI_HIGHLIGHT_GREEN_ON "\x1B[1;32m"
#define ANSI_HIGHLIGHT_YELLOW_ON "\x1B[1;33m"
#define ANSI_HIGHLIGHT_BLUE_ON "\x1B[1;34m"
#define ANSI_HIGHLIGHT_OFF "\x1B[0m"
#define ANSI_ERASE_TO_END_OF_LINE "\x1B[K"

int open_terminal(const char *name, int mode);
