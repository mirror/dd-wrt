//========================================================================
//
//      locale.cxx
//
//      Implementation of ISO C internationalisation (i18n) locales
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jlarmour
// Contributors: jjohnstn
// Date:         2000-04-18
// Purpose:     
// Description: 
// Usage:       
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libc_i18n.h>     // Configuration header

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Common tracing support
#include <cyg/infra/cyg_ass.h>     // Common assertion support
#include <locale.h>                // struct lconv
#include <string.h>                // several string functions
#include <limits.h>                // CHAR_MAX
#include "internal.h"              // locale type definitions

// CONSTANTS

// define the "C" locale
static const Cyg_libc_locale_t
C_locale = { 
  "C",
  { ".", "", "", "", "", "", "", "", "", "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX
  },
  1,
  NULL,
  NULL,
};

// define the "C-EUCJP" locale (C locale with Japanese EUCJP mb support)
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_EUCJP
static const Cyg_libc_locale_t
C_EUCJP_locale = { 
  "C-EUCJP",
  { ".", "", "", "", "", "", "", "", "", "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX
  },
  2,
  &__mbtowc_jp,
  &__wctomb_jp,
};
#endif

// define the "C-SJIS" locale (C locale with Japanese SJIS mb support)
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_SJIS
static const Cyg_libc_locale_t
C_SJIS_locale = { 
  "C-SJIS",
  { ".", "", "", "", "", "", "", "", "", "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX
  },
  2,
  &__mbtowc_jp,
  &__wctomb_jp,
};
#endif

// define the "C-JIS" locale (C locale with Japanese JIS mb support)
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_JIS
static const Cyg_libc_locale_t
C_JIS_locale = { 
  "C-JIS",
  { ".", "", "", "", "", "", "", "", "", "",
    CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
    CHAR_MAX, CHAR_MAX
  },
  8,
  &__mbtowc_jp,
  &__wctomb_jp,
};
#endif

// only one locale now, but leave room for expansion
static const Cyg_libc_locale_t *all_locales[] = { &C_locale,
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_EUCJP
						  &C_EUCJP_locale,
#endif
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_SJIS
						  &C_SJIS_locale,
#endif
#ifdef CYGFUN_LIBC_I18N_LOCALE_C_JIS
						  &C_JIS_locale,
#endif
};

// GLOBALS

// the maximum size of a multibyte character including state info
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
int __mb_cur_max = 1;
#endif

// the current locales. Our default is the C locale
static const Cyg_libc_locale_t *current_collate_locale  = &C_locale;
const        Cyg_libc_locale_t *__current_ctype_locale  = &C_locale;
static const Cyg_libc_locale_t *current_monetary_locale = &C_locale;
static const Cyg_libc_locale_t *current_numeric_locale  = &C_locale;
static const Cyg_libc_locale_t *current_time_locale     = &C_locale;

// FUNCTIONS

static const Cyg_libc_locale_t *
find_locale_data( const char *locale_str, cyg_ucount32 checklen )
{
    CYG_REPORT_FUNCNAMETYPE( "find_locale_data", "returning %08x" );
    CYG_REPORT_FUNCARG1( "locale_str=%s", locale_str );

    const Cyg_libc_locale_t *temp_locale, *curr_locale=NULL;
    cyg_ucount32 i;

    // is it "" i.e. use the default?
    if (*locale_str=='\0') {
        curr_locale = &C_locale;
        CYG_REPORT_RETVAL( curr_locale );
        return curr_locale;
    } // if

    for (i=0; i<sizeof(all_locales)/sizeof(Cyg_libc_locale_t *); i++ ) {

        temp_locale = all_locales[i];

        if ( !strncmp(temp_locale->name, locale_str, checklen) )
            curr_locale = temp_locale;
    } // for

    CYG_REPORT_RETVAL( curr_locale );
    return curr_locale;
} // find_locale_data()

typedef int (*mbtowc_fn_type)(wchar_t *, const char *, size_t, int *);
// routine used to export mbtowc function to I/O routines
externC mbtowc_fn_type
__get_current_locale_mbtowc_fn ()
{
    if (__current_ctype_locale->mbtowc_fn)
      return __current_ctype_locale->mbtowc_fn;
    return &__mbtowc_c;
}

externC char *
setlocale( int category, const char *locale )
{
    CYG_REPORT_FUNCNAMETYPE("setlocale", "returning %08x");
    CYG_REPORT_FUNCARG2( "category=%d, locale=%s", category, locale );

    if (locale != NULL)
        CYG_CHECK_DATA_PTR( locale, "locale pointer is invalid!" );

    const char *str;

    // special case if locale==NULL, return current locale name
    if (locale==NULL) {

        CYG_TRACE0( true, "Getting current locale value" );

        switch (category) {

        case LC_COLLATE:
            str = current_collate_locale->name;
            break;
        case LC_CTYPE:
            str = __current_ctype_locale->name;
            break;
        case LC_MONETARY:
            str = current_monetary_locale->name;
            break;
        case LC_NUMERIC:
            str = current_numeric_locale->name;
            break;
        case LC_TIME:
            str = current_time_locale->name;
            break;
        case LC_ALL:

            // create static string to give a constructed string back
            // to the user. The size is the number of categories other
            // than LC_ALL times the maximum name size, and add a constant
            // for the delimiting octothorpes
            static char my_str[ CYGNUM_LIBC_I18N_MAX_LOCALE_NAME_SIZE*5+10 ];

            strcpy( my_str, "#" );
            strcat( my_str, current_collate_locale->name );
            strcat( my_str, "#" );
            strcat( my_str, __current_ctype_locale->name );
            strcat( my_str, "#" );
            strcat( my_str, current_monetary_locale->name );
            strcat( my_str, "#" );
            strcat( my_str, current_numeric_locale->name );
            strcat( my_str, "#" );
            strcat( my_str, current_time_locale->name );
            strcat( my_str, "#" );

            str = &my_str[0];
            break;
        default:
            str=NULL;
            CYG_FAIL("setlocale() passed bad category!" );
            break;

        } // switch

        CYG_REPORT_RETVAL( (char *)str);
        return (char *)str;
    } // if
        
    // we only get here if locale is non-NULL, i.e. we want to set it

    const Cyg_libc_locale_t *loc;
    cyg_bool default_locale = (*locale=='\0');

    CYG_TRACE0( true, "Setting current locale value" );

    switch( category ) {
    case LC_COLLATE:
        loc = find_locale_data( locale, CYGNUM_LIBC_I18N_MAX_LOCALE_NAME_SIZE );
        if (loc != NULL)      // found it
            current_collate_locale=loc;
        break;
        
    case LC_CTYPE:
        loc = find_locale_data( locale, CYGNUM_LIBC_I18N_MAX_LOCALE_NAME_SIZE );
        if (loc != NULL)      // found it
	  {
            __current_ctype_locale=loc;
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
	    __mb_cur_max = loc->mb_cur_max;
#endif
	  }
        break;
        
    case LC_MONETARY:
        loc = find_locale_data( locale, CYGNUM_LIBC_I18N_MAX_LOCALE_NAME_SIZE );
        if (loc != NULL)      // found it
            current_monetary_locale=loc;
        break;
        
    case LC_NUMERIC:
        loc = find_locale_data( locale, CYGNUM_LIBC_I18N_MAX_LOCALE_NAME_SIZE );
        if (loc != NULL)      // found it
            current_numeric_locale=loc;
        break;
        
    case LC_TIME:
        loc = find_locale_data( locale, CYGNUM_LIBC_I18N_MAX_LOCALE_NAME_SIZE );
        if (loc != NULL)      // found it
            current_time_locale=loc;
        break;
        
    case LC_ALL:
        // first try and match it exactly
        loc = find_locale_data( locale, CYGNUM_LIBC_I18N_MAX_LOCALE_NAME_SIZE );
        if (loc != NULL) {     // found it

            CYG_TRACE0(true, "Matched locale string exactly");
            current_collate_locale = __current_ctype_locale = loc;
            current_monetary_locale = current_numeric_locale = loc;
            current_time_locale = loc;
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
	    __mb_cur_max = loc->mb_cur_max;
#endif
        } // if
        else {
            CYG_TRACE0( true, "Attempting to parse string previously "
                        "returned from setlocale()" );
            // now try and see if it is a compound string returned
            // earlier by setlocale( LC_ALL, NULL );

            // Note we don't do much checking here. This could be
            // much more rigorous (but at the expense of speed/size?)

            const Cyg_libc_locale_t *temp_collate_locale,
                *temp_ctype_locale, *temp_monetary_locale,
                *temp_numeric_locale, *temp_time_locale;

            cyg_ucount32 token_len;

            str = &locale[0];
            if ( *str=='#' ) {
                ++str;
                token_len = strcspn( str, "#" );
                loc = find_locale_data( str, token_len );

                if (loc!=NULL) {
                    temp_collate_locale=loc;
                    str += token_len+1;
                    token_len = strcspn( str, "#" );
                    loc = find_locale_data( str, token_len );

                    if (loc!=NULL) {
                        temp_ctype_locale=loc;
                        str += token_len+1;
                        token_len = strcspn( str, "#" );
                        loc = find_locale_data( str, token_len );

                        if (loc!=NULL) {
                            temp_monetary_locale=loc;
                            str += token_len+1;
                            token_len = strcspn( str, "#" );
                            loc = find_locale_data( str, token_len );

                            if (loc!=NULL) {
                                temp_numeric_locale=loc;
                                str += token_len+1;
                                token_len = strcspn( str, "#" );
                                loc = find_locale_data( str, token_len );

                                if (loc!=NULL) {
                                    temp_time_locale=loc;
                                    str += token_len+1;
                                    token_len = strcspn( str, "#" );
                                    loc = find_locale_data( str, token_len );

                                    if (loc!=NULL) {
                      // if we've got this far and loc still isn't NULL,
                      // then everything's fine, and we've matched everything
                                        
                      current_collate_locale = temp_collate_locale;
                      __current_ctype_locale = temp_ctype_locale;
#ifdef CYGINT_LIBC_I18N_MB_REQUIRED
		      __mb_cur_max = temp_ctype_locale->mb_cur_max;
#endif
                      current_monetary_locale = temp_monetary_locale;
                      current_numeric_locale = temp_numeric_locale;
                      current_time_locale = temp_time_locale;

                                    } // if
                                } // if
                            } // if
                        } // if
                    } // if
                } // if
            } // if
            
        } // else
        break; // case LC_ALL
        
    default:
            CYG_FAIL("setlocale() passed bad category!" );
            loc=NULL;
            break;
    } // switch

    if (loc==NULL) {
        CYG_REPORT_RETVAL( NULL );
        return NULL;
    } // if
    else if (default_locale==true) {
        CYG_REPORT_RETVAL(C_locale.name);
        return (char *)C_locale.name;
    } // else if
    else {
        CYG_REPORT_RETVAL(locale);
        return (char *)locale;
    } // else
} // setlocale()

externC struct lconv *
localeconv( void )
{
    CYG_REPORT_FUNCNAMETYPE( "localeconv", "returning %08x" );
    CYG_REPORT_FUNCARGVOID();

    static struct lconv static_lconv;

    static_lconv.decimal_point = 
        current_numeric_locale->numdata.decimal_point;

    static_lconv.thousands_sep = 
        current_numeric_locale->numdata.thousands_sep;

    static_lconv.grouping = 
        current_numeric_locale->numdata.grouping;

    // we cheat a bit, but it should be worth it - a lot of these are
    // constants which optimise nicely
    cyg_ucount32 size_used;
    size_used = (char *)&static_lconv.int_curr_symbol -
                (char *)&static_lconv;

    memcpy( &(static_lconv.int_curr_symbol),
            &(current_monetary_locale->numdata.int_curr_symbol),
            sizeof(struct lconv) - size_used );
            

    CYG_REPORT_RETVAL( &static_lconv );
    return &static_lconv;
} // localeconv()

// EOF locale.cxx
