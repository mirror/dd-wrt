//==========================================================================
//
//      alias.c
//
//      RedBoot - alias support
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Author(s):    gthomas
// Contributors: gthomas, jskov
// Date:         2002-05-15
// Purpose:      
// Description:  
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <redboot.h>

#ifdef CYGNUM_REDBOOT_FLASH_STRING_SIZE
# define MAX_STRING_LENGTH CYGNUM_REDBOOT_FLASH_STRING_SIZE
#else
# define MAX_STRING_LENGTH 32
#endif

// Lookup an alias. First try plain string aliases. If that fails try
// other types so allowing access to all configured values. This allows
// for alias (macro) expansion of normal 'fconfig' data, such as the
// board IP address.
static char *
lookup_alias(char *alias, char *alias_buf)
{
    if (0 == strcasecmp("FREEMEMLO", alias)) {
        diag_sprintf(alias_buf,"%p", ((CYG_ADDRWORD)mem_segments[0].start + 0x03ff) & ~0x03ff);
        return alias_buf;
    } else if (0 == strcasecmp("FREEMEMHI", alias)) {
        diag_sprintf(alias_buf,"%p", ((CYG_ADDRWORD)mem_segments[0].end) & ~0x03ff);
        return alias_buf;
    }

#ifdef CYGSEM_REDBOOT_FLASH_ALIASES
    return flash_lookup_alias(alias, alias_buf);
#else
    return NULL;
#endif
}

// Expand aliases, this is recursive. ie if one alias contains other
// aliases, these will also be expanded from the insertion point
// onwards.
//
// If 'iter' is zero, then quoted strings are not expanded
//
static bool
_expand_aliases(char *line, int len, int iter)
{
    char *lp = line;
    char *ms, *me, *ep;
    char *alias;
    char c;
    int offset, line_len, alias_len;
    char alias_buf[MAX_STRING_LENGTH];
    bool macro_found = false;

    if ((line_len = strlen(line)) != 0) {
        while (*lp) {
            c = *lp++;
            if ((c == '%') && (*lp == '{')) {
                // Found a macro/alias to expand
                ms = lp+1;
                lp += 2;
                while (*lp && (*lp != '}')) lp++;
                if (!*lp) {
                    diag_printf("Invalid macro/alias '%s'\n", ms);
                    line[0] = '\0';  // Destroy line
                    return false;
                }
                me = lp;
                *me = '\0';
                lp++;
                if ((alias = lookup_alias(ms,alias_buf)) != (char *)NULL) {
                    alias_len = strlen(alias);
                    // See if there is room in the line to expand this macro/alias
                    if ((line_len+alias_len) < len) {
                        // Make a hole by moving data within the line
                        offset = alias_len-strlen(ms)-2;  // Number of bytes being inserted
			ep = &lp[strlen(lp)-1];
			if (offset > 1) {
                            ep[offset] = '\0';
                            while (ep != (lp-1)) {
                                ep[offset-1] = *ep;
                                ep--;
                            }           
			} else {
                            if (offset <=0) {
                                while ((lp-1) != ep) {
                                    lp[offset-1] = *lp;
                                    lp++;
                                }
                                lp[offset-1]='\0';
                            }
			}
                        // Insert the macro/alias data
                        lp = ms-2;
                        while (*alias) {
                            if ((alias[0] == '%') && (alias[1] == '{')) macro_found = true;
                            *lp++ = *alias++;
                        }
                        line_len = strlen(line);
			lp = lp - alias_len;
                    } else {
                        diag_printf("No room to expand '%s'\n", ms);
                        line[0] = '\0';  // Destroy line
                        return false;
                    }
                } else {
                    diag_printf("Alias '%s' not defined\n", ms);
                    *me = '|';
                }
            } else if ((c == '"') && (iter == 0)) {
                // Skip quoted strings
                while (*lp && (*lp != '"')) lp++;
            }            
        }
    }
    return macro_found;
}

void
expand_aliases(char *line, int len)
{
    int iter = 0;

    while (_expand_aliases(line, len, iter++)) {
    }
}
