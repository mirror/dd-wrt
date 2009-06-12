//===========================================================================
//
//      mbtowc_jp.cxx
//
//      Internal __mbtowc_jp() routine 
//
//===========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jjohnstn
// Contributors:  jjohnstn
// Date:          2000-11-16
// Purpose:       Provide internal use __mbtowc_jp() routine
// Description:   Japanese locale version of mbtowc()
// Usage:       
//
//####DESCRIPTIONEND####
//
//===========================================================================
//
// This code was taken from newlib/libc/stdlib/mbtowc_r.c


// CONFIGURATION

#include <pkgconf/libc_i18n.h>     // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions
#include <cyg/infra/cyg_trac.h>    // Tracing support
#include <cyg/infra/cyg_ass.h>     // Assertion support
#include <locale.h>
#include <stdlib.h>                // Header for this file
#include <string.h>                // strcmp definition
#include <stddef.h>                // size_t definition
#include "internal.h"              // internal __isxxxx macros

// TRACE

#if defined(CYGDBG_USE_TRACING) && defined(CYGNUM_LIBC_STDLIB_MBTOWC_TRACE_LEVEL)
static int mbtowc_trace = CYGNUM_LIBC_STDLIB_MBTOWC_TRACE_LEVEL;
# define TL1 (0 < mbtowc_trace)
#else
# define TL1 (0)
#endif

// STATICS


#ifdef CYGFUN_LIBC_I18N_LOCALE_C_JIS
typedef enum { ESCAPE, DOLLAR, BRACKET, AT, B, J, 
               NUL, JIS_CHAR, OTHER, JIS_C_NUM } JIS_CHAR_TYPE;
typedef enum { ASCII, A_ESC, A_ESC_DL, JIS, JIS_1, JIS_2, J_ESC, J_ESC_BR,
               J2_ESC, J2_ESC_BR, DONE, INV, JIS_S_NUM } JIS_STATE; 
typedef enum { COPY_A, COPY_J, COPY_J2, MAKE_A, MAKE_J, NOOP, EMPTY, ERROR } JIS_ACTION;

/************************************************************************************** 
 * state/action tables for processing JIS encoding
 * Where possible, switches to JIS are grouped with proceding JIS characters and switches
 * to ASCII are grouped with preceding JIS characters.  Thus, maximum returned length
 * is 2 (switch to JIS) + 2 (JIS characters) + 2 (switch back to ASCII) = 6.
 *************************************************************************************/

static JIS_STATE JIS_state_table[JIS_S_NUM][JIS_C_NUM] = {
/*              ESCAPE   DOLLAR    BRACKET   AT       B       J        NUL      JIS_CHAR  OTHER */
/* ASCII */   { A_ESC,   DONE,     DONE,     DONE,    DONE,   DONE,    DONE,    DONE,     DONE },
/* A_ESC */   { DONE,    A_ESC_DL, DONE,     DONE,    DONE,   DONE,    DONE,    DONE,     DONE },
/* A_ESC_DL */{ DONE,    DONE,     DONE,     JIS,     JIS,    DONE,    DONE,    DONE,     DONE }, 
/* JIS */     { J_ESC,   JIS_1,    JIS_1,    JIS_1,   JIS_1,  JIS_1,   INV,     JIS_1,    INV },
/* JIS_1 */   { INV,     JIS_2,    JIS_2,    JIS_2,   JIS_2,  JIS_2,   INV,     JIS_2,    INV },
/* JIS_2 */   { J2_ESC,  DONE,     DONE,     DONE,    DONE,   DONE,    INV,     DONE,     DONE },
/* J_ESC */   { INV,     INV,      J_ESC_BR, INV,     INV,    INV,     INV,     INV,      INV },
/* J_ESC_BR */{ INV,     INV,      INV,      INV,     ASCII,  ASCII,   INV,     INV,      INV },
/* J2_ESC */  { INV,     INV,      J2_ESC_BR,INV,     INV,    INV,     INV,     INV,      INV },
/* J2_ESC_BR*/{ INV,     INV,      INV,      INV,     DONE,   DONE,    INV,     INV,      INV },
};

static JIS_ACTION JIS_action_table[JIS_S_NUM][JIS_C_NUM] = {
/*              ESCAPE   DOLLAR    BRACKET   AT       B        J        NUL      JIS_CHAR  OTHER */
/* ASCII */   { NOOP,    COPY_A,   COPY_A,   COPY_A,  COPY_A,  COPY_A,  EMPTY,   COPY_A,  COPY_A},
/* A_ESC */   { COPY_A,  NOOP,     COPY_A,   COPY_A,  COPY_A,  COPY_A,  COPY_A,  COPY_A,  COPY_A},
/* A_ESC_DL */{ COPY_A,  COPY_A,   COPY_A,   MAKE_J,  MAKE_J,  COPY_A,  COPY_A,  COPY_A,  COPY_A},
/* JIS */     { NOOP,    NOOP,     NOOP,     NOOP,    NOOP,    NOOP,    ERROR,   NOOP,    ERROR },
/* JIS_1 */   { ERROR,   NOOP,     NOOP,     NOOP,    NOOP,    NOOP,    ERROR,   NOOP,    ERROR },
/* JIS_2 */   { NOOP,    COPY_J2,  COPY_J2,  COPY_J2, COPY_J2, COPY_J2, ERROR,   COPY_J2, COPY_J2},
/* J_ESC */   { ERROR,   ERROR,    NOOP,     ERROR,   ERROR,   ERROR,   ERROR,   ERROR,   ERROR },
/* J_ESC_BR */{ ERROR,   ERROR,    ERROR,    ERROR,   NOOP,    NOOP,    ERROR,   ERROR,   ERROR },
/* J2_ESC */  { ERROR,   ERROR,    NOOP,     ERROR,   ERROR,   ERROR,   ERROR,   ERROR,   ERROR },
/* J2_ESC_BR*/{ ERROR,   ERROR,    ERROR,    ERROR,   COPY_J,  COPY_J,  ERROR,   ERROR,   ERROR },
};
#endif // CYGFUN_LIBC_I18N_LOCALE_C_JIS

// FUNCTIONS

int
__mbtowc_jp ( wchar_t *pwc, const char *s, size_t n, int *state )
{
  wchar_t dummy;
  unsigned char *t = (unsigned char *)s;
  int retval;
  const char *cur_locale = __current_ctype_locale->name;

  CYG_REPORT_FUNCNAMETYPE( "__mbtowc_jp", "returning %d" );
  CYG_REPORT_FUNCARG4( "pwc=%08x, s=%08x, n=%ud, state=%08x", pwc, s, n, state );
  
  if (pwc != NULL)
    CYG_CHECK_DATA_PTR( pwc, "pwc is not a valid pointer!" );
  if (s != NULL)
    CYG_CHECK_DATA_PTR( s, "s is not a valid pointer!" );
  CYG_CHECK_DATA_PTR( state, "state is not a valid pointer!" );
 
  if (pwc == NULL)
    pwc = &dummy;

  if (s != NULL && n == 0)
    {
      retval = -1;
      CYG_REPORT_RETVAL (retval);
      return retval;
    }

  if (cur_locale == NULL ||
      (strlen (cur_locale) <= 1))
    { /* fall-through */ }
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_SJIS
  else if (!strcmp (cur_locale, "C-SJIS"))
    {
      int char1;
      if (s == NULL)
	{
	  retval = 0;
	  CYG_REPORT_RETVAL (retval);
	  return retval;  /* not state-dependent */
	}
      char1 = *t;
      if (_issjis1 (char1))
        {
          int char2 = t[1];
          if (n <= 1)
            retval = -1;
          else if (_issjis2 (char2))
            {
              *pwc = (((wchar_t)*t) << 8) + (wchar_t)(*(t+1));
              retval = 2;
            }
          else  
            retval = -1;
	  CYG_REPORT_RETVAL (retval);
	  return retval;
        }
    }
#endif /* CYGFUN_LIBC_I18N_LOCALE_C_SJIS */
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_EUCJP
  else if (!strcmp (cur_locale, "C-EUCJP"))
    {
      int char1;
      if (s == NULL)
	{
	  retval = 0;  /* not state-dependent */
	  CYG_REPORT_RETVAL (retval);
	  return retval;  /* not state-dependent */
	}
      char1 = *t;
      if (_iseucjp (char1))
        {
          int char2 = t[1];     
          if (n <= 1)
            retval = -1;
          if (_iseucjp (char2))
            {
              *pwc = (((wchar_t)*t) << 8) + (wchar_t)(*(t+1));
              retval = 2;
            }
          else
            retval = -1;
	  CYG_REPORT_RETVAL (retval);
	  return retval;
        }
    }
#endif /* CYGFUN_LIBC_I18N_LOCALE_C_EUCJP */ 
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_JIS
  else if (!strcmp (cur_locale, "C-JIS"))
    {
      JIS_STATE curr_state;
      JIS_ACTION action;
      JIS_CHAR_TYPE ch;
      unsigned char *ptr;
      int i, curr_ch;
 
      if (s == NULL)
        {
          *state = 0;
	  retval = 1;
	  CYG_REPORT_RETVAL (retval);
	  return retval;  /* state-dependent */
        }

      curr_state = (*state == 0 ? ASCII : JIS);
      ptr = t;

      for (i = 0; i < (int)n; ++i)
        {
          curr_ch = t[i];
          switch (curr_ch)
            {
	    case ESC_CHAR:
              ch = ESCAPE;
              break;
	    case '$':
              ch = DOLLAR;
              break;
            case '@':
              ch = AT;
              break;
            case '(':
	      ch = BRACKET;
              break;
            case 'B':
              ch = B;
              break;
            case 'J':
              ch = J;
              break;
            case '\0':
              ch = NUL;
              break;
            default:
              if (_isjis (curr_ch))
                ch = JIS_CHAR;
              else
                ch = OTHER;
	    }

          action = JIS_action_table[curr_state][ch];
          curr_state = JIS_state_table[curr_state][ch];
        
          switch (action)
            {
            case NOOP:
              break;
            case EMPTY:
              *state = 0;
              *pwc = (wchar_t)0;
	      retval = i;
	      CYG_REPORT_RETVAL (retval);
	      return retval;
            case COPY_A:
	      *state = 0;
              *pwc = (wchar_t)*ptr;
	      retval = i + 1;
	      CYG_REPORT_RETVAL (retval);
	      return retval;
             case COPY_J:
              *state = 0;
              *pwc = (((wchar_t)*ptr) << 8) + (wchar_t)(*(ptr+1));
	      retval = i + 1;
	      CYG_REPORT_RETVAL (retval);
	      return retval;
             case COPY_J2:
              *state = 1;
              *pwc = (((wchar_t)*ptr) << 8) + (wchar_t)(*(ptr+1));
              retval = (ptr - t) + 2;
	      CYG_REPORT_RETVAL (retval);
	      return retval;
	    case MAKE_A:
            case MAKE_J:
              ptr = (unsigned char *)(t + i + 1);
              break;
            case ERROR:
            default:
              retval = -1;
	      CYG_REPORT_RETVAL (retval);
	      return retval;
	    }

        }

      retval = -1;  /* n < bytes needed */
      CYG_REPORT_RETVAL (retval);
      return retval;
    }
#endif /* CYGFUN_LIBC_I18N_LOCALE_C_JIS */

  /* otherwise this must be the "C" locale or unknown locale */
  if (s == NULL)
    retval = 0;  /* not state-dependent */
  else
    {
      if (pwc)
	*pwc = (wchar_t)*t;
      retval = (*t != '\0');
    }
  CYG_REPORT_RETVAL (retval);
  return retval;
} // __mbtowc_jp()

// EOF mbtowc_jp.cxx
