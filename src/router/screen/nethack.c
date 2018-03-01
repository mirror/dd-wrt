/* Copyright (c) 2008, 2009
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 *      Micah Cowan (micah@cowan.name)
 *      Sadrul Habib Chowdhury (sadrul@users.sourceforge.net)
 * Copyright (c) 1993-2002, 2003, 2005, 2006, 2007
 *      Juergen Weigert (jnweiger@immd4.informatik.uni-erlangen.de)
 *      Michael Schroeder (mlschroe@immd4.informatik.uni-erlangen.de)
 * Copyright (c) 1987 Oliver Laumann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * http://www.gnu.org/licenses/, or contact Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 ****************************************************************
 */

#include "config.h"
#include "screen.h"

#ifdef NETHACK
extern int nethackflag;
#endif

struct nlstrans {
    char *from;
    char *to;
};

#ifdef NETHACK
static struct nlstrans nethacktrans[] = {
{"Cannot lock terminal - fork failed",
	 "Cannot fork terminal - lock failed"},
{"Got only %d bytes from %s",
	 "You choke on your food: %d bytes from %s"},
{"Copy mode - Column %d Line %d(+%d) (%d,%d)",
	"Welcome to hacker's treasure zoo - Column %d Line %d(+%d) (%d,%d)"},
{"First mark set - Column %d Line %d",
	"You drop a magic marker - Column %d Line %d"},
{"Copy mode aborted",
	"You escaped the dungeon."},
{"Filter removed.",
	"You have a sad feeling for a moment..."},
{"Window %d (%s) killed.",
	"You destroy poor window %d (%s)."},
{"Window %d (%s) is now being monitored for all activity.",
	"You feel like someone is watching you..."},
{"Window %d (%s) is no longer being monitored for activity.",
	"You no longer sense the watcher's presence."},
{"empty buffer",
	"Nothing happens."},
{"switched to audible bell.",
	"Suddenly you can't see your bell!"},
{"switched to visual bell.",
	"Your bell is no longer invisible."},
{"The window is now being monitored for %d sec. silence.",
	"You feel like someone is waiting for %d sec. silence..."},
{"The window is no longer being monitored for silence.",
	"You no longer sense the watcher's silence."},
{"No other window.",
	"You cannot escape from window %d!"},
{"Logfile \"%s\" closed.",
	"You put away your scroll of logging named \"%s\"." },
{"Error opening logfile \"%s\"",
	"You don't seem to have a scroll of logging named \"%s\"."},
{"Creating logfile \"%s\".",
	"You start writing on your scroll of logging named \"%s\"."},
{"Appending to logfile \"%s\".",
	"You add to your scroll of logging named \"%s\"."},
{"Detach aborted.",
	"The blast of disintegration whizzes by you!"},
{"Empty register.",
	"Nothing happens."},
{"[ Passwords don't match - checking turned off ]",
	"[ Passwords don't match - your armor crumbles away ]"},
{"Aborted because of window size change.",
	"KAABLAMM!!!  You triggered a land mine!"},
{"Out of memory.",
	"Who was that Maude person anyway?"},
{"getpwuid() can't identify your account!",
	"An alarm sounds through the dungeon...\nThe Keystone Kops are after you!"},
{"Must be connected to a terminal.",
	"You must play from a terminal."},
{"No Sockets found in %s.\n",
	"This room is empty (%s).\n"},
{"New screen...",
	"Be careful!  New screen tonight."},
{"Child has been stopped, restarting.",
	"You regain consciousness."},
{"There are screens on:",
	"Your inventory:"},
{"There is a screen on:",
	"Your inventory:"},
{"There are several screens on:",
	"Prove thyself worthy or perish:"},
{"There is a suitable screen on:",
	"You see here a good looking screen:"},
{"There are several suitable screens on:",
	"You may wish for a screen, what do you want?"},
{"%d socket%s wiped out.",
	"You hear %d distant explosion%s."},
{"Remove dead screens with 'screen -wipe'.",
	"The dead screen%s touch%s you. Try 'screen -wipe'."},
{"Illegal reattach attempt from terminal %s.",
	"'%s' tries to touch your session, but fails."},
{"Could not write %s",
	"%s is too hard to dig in"},
{0, 0}
};
#endif

const char *
DoNLS(from)
const char *from;
{
#ifdef NETHACK
  struct nlstrans *t;

  if (nethackflag)
    {
      for (t = nethacktrans; t->from; t++)
	if (strcmp(from, t->from) == 0)
	  return t->to;
    }
#endif
  return from;
}
