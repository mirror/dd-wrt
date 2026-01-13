/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2025 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "pound.h"
#include "extern.h"
#include "watcher.h"

void
watchpoint_set_mode (struct watchpoint *wp, enum watcher_mode mode)
{
  /*
   * Nothing to do: the mode will be set in watcher_setup, prior to
   * arming the watcher job.
   */
}

int
watcher_setup (void)
{
  struct watchpoint *wp;
  DLIST_FOREACH (wp, &watch_head, link)
    {
      if (wp->type == WATCH_FILE)
	watchpoint_set_compat_mode (wp);
    }
  return 0;
}
