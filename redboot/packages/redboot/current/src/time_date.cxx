//==========================================================================
//
//        time_date.cxx
//
//        RedBoot time/date commands
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Gary Thomas
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     gthomas
// Contributors:  
// Date:          2003-09-10
// Description:   
//####DESCRIPTIONEND####
// -------------------------------------------------------------------------

#include <redboot.h>
#include <pkgconf/hal.h>
#include <pkgconf/wallclock.h>

#include <cyg/infra/diag.h>
#include <cyg/io/wallclock.hxx>              // The WallClock API
#include <cyg/io/wallclock/wallclock.inl>    // Helper functions (avoids LIBC)

RedBoot_cmd("date", 
            "Show/Set the time of day", 
            "[YYYY/MM/DD HH:MM:SS]",
            do_time_date
    );

static bool
verify(cyg_uint32 val, int min, int max, char *id)
{
    if (((int)val < min) || ((int)val > max)) {
        diag_printf("%s is out of range - must be [%d..%d]\n", id, min, max);
        return false;
    }
    return true;
}

void
do_time_date(int argc, char *argv[])
{
    cyg_uint32 now = Cyg_WallClock::wallclock->get_current_time();
    cyg_uint32 year, month, mday, hour, minute, second;
    char *sp;
    bool ok = true;

    if (argc == 1) {
        // Just show the current time/date
        _simple_mkdate(now, &year, &month, &mday, &hour, &minute, &second);
        diag_printf("%04d/%02d/%02d %02d:%02d:%02d\n", 
                    year, month, mday, hour, minute, second);
    } else if (argc == 3) {
        sp = argv[1];
        if (!parse_num(sp, (unsigned long *)&year, &sp, "/") ||
            !parse_num(sp, (unsigned long *)&month, &sp, "/") ||
            !parse_num(sp, (unsigned long *)&mday, &sp, "/")) {
            ok = false;
        }
        sp = argv[2];
        if (!parse_num(sp, (unsigned long *)&hour, &sp, ":") ||
            !parse_num(sp, (unsigned long *)&minute, &sp, ":") ||
            !parse_num(sp, (unsigned long *)&second, &sp, ":")) {
            ok = false;
        }
        if (ok) {
            // Verify values make some sense, then set the hardware
            if (year < 100) year += 2000;
            ok = ok && verify(year, 1970, 2034, "year");
            ok = ok && verify(month, 1, 12, "month");
            ok = ok && verify(mday, 1, 31, "day");
            ok = ok && verify(hour, 0, 23, "hour");
            ok = ok && verify(minute, 0, 59, "minute");
            ok = ok && verify(second, 0, 59, "second");
            if (ok) {
                now = _simple_mktime(year, month, mday, hour, minute, second);
                Cyg_WallClock::wallclock->set_current_time(now);
            }
        }
    } else {
        ok = false;
    }
    if (!ok) {
        diag_printf("usage: date [YYYY/MM/DD HH:MM:SS]\n");
    }

}

// -------------------------------------------------------------------------
// EOF time_date.cxx
