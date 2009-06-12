//{{{  Banner                                                   

//============================================================================
//
//      cdlmisc.cxx
//
//      Implementation of the various CDL utility member functions.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This file is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
//============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contact(s):  bartv
// Date:        1998/03/04
// Version:     0.01
//
//####DESCRIPTIONEND####
//============================================================================

//}}}
//{{{  #include's                                               

// ----------------------------------------------------------------------------
#include "cdlconfig.h"

// Get the infrastructure types, assertions, tracing and similar
// facilities.
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>

// <cdlcore.hxx> defines everything implemented in this module.
// It implicitly supplies <string>, <vector> and <map> because
// the class definitions rely on these headers.
#include <cdlcore.hxx>

// For access to the isdigit(), isupper(), tolower(), ... functions
#include <cctype>

// For access to sprintf(), specifically double to string conversions.
#include <cstdio>

// For access to strtod()
#include <cstdlib>

// strtod() involves errno...
#include <cerrno>

// For access to fmod()
#include <cmath>

// For access to DBL_DIG
#include <cfloat>

//}}}

//{{{  Cdl::is_valid_xxx()                                      

// ---------------------------------------------------------------------------

bool
Cdl::is_valid_value_flavor(CdlValueFlavor data)
{
    bool result = false;

    switch(data) {
      case CdlValueFlavor_None     :
      case CdlValueFlavor_Bool     :
      case CdlValueFlavor_BoolData :
      case CdlValueFlavor_Data     :
        result = true;
        break;

      default:
        break;
    }

    return result;
}

bool
Cdl::is_valid_value_source(CdlValueSource data)
{
    bool result = false;

    switch(data) {
      case CdlValueSource_Default         :
      case CdlValueSource_User            :
      case CdlValueSource_Wizard          :
      case CdlValueSource_Inferred        :
        result = true;
        break;

      default:
        break;
    }

    return result;
}

// ----------------------------------------------------------------------------
// For now CDL names are restricted to what is acceptable to the C
// preprocessor. This may cause problems in future, e.g. i18n.

bool
Cdl::is_valid_cdl_name(const std::string& name)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::is_valid_cdl_name", "result %d");

    bool result = is_valid_c_preprocessor_symbol(name);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
Cdl::is_valid_c_preprocessor_symbol(const std::string& symbol)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::is_valid_c_preprocessor_symbol", "result %d");

    bool result = true;
    if ("" == symbol) {
        result = false;
    } else {
        // A valid preprocessor symbol should begin with either an underscore
        // or a letter. It should then be followed by some number of underscores,
        // letters, or digits.
        //
        // In some locales isalpha() may succeed for characters which are not
        // legal for C preprocessor symbols. Instead ASCII is assumed here.
        if (('_' != symbol[0]) &&
            !(('a' <= symbol[0]) && (symbol[0] <= 'z')) &&
            !(('A' <= symbol[0]) && (symbol[0] <= 'Z'))) {
            
            result = false;
        } else {
            for (unsigned int i = 1; i < symbol.size(); i++) {
                if (('_' != symbol[i]) &&
                    !(('a' <= symbol[i]) && (symbol[i] <= 'z')) &&
                    !(('A' <= symbol[i]) && (symbol[i] <= 'Z')) &&
                    !(('0' <= symbol[i]) && (symbol[i] <= '9'))) {
                    
                    result = false;
                    break;
                }
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Cdl::xxx_to_yyy() - strings, ints, doubles, ...          

// ---------------------------------------------------------------------------
// Conversion routines between strings, integers, doubles, bools, ...
//
// Conversions to/from integers are complicated somewhat because the
// data type in question is cdl_int. In the initial implementation this
// is 64 bits. In the long term it will be arbitrary precision and
// the conversion routines will need to be reimplemented.
//
// ASCII rather than EBCDIC is assumed.
//
// Some of the routines may fail, e.g. string to integer conversions.
// Others are guaranteed to succeed. 

//{{{  string_to_integer()                      

// ----------------------------------------------------------------------------
bool
Cdl::string_to_integer(std::string data, cdl_int& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::string_to_integer", "success %d");

    bool negative       = false;
    bool seen_plus      = false;
    bool seen_minus     = false;
    
    // Life is a bit easier if I can check for '\0'
    const char* ptr           = data.c_str();

    // Not essential but harmless.
    while (isspace(*ptr))
        ptr++;

    if ('+' == *ptr) {
        if (seen_plus) {
            target = 0;
            CYG_REPORT_RETVAL(false);
            return false;
        }
        seen_plus = true;
        ptr++;
    }
    
    if ('-' == *ptr) {
        if (seen_minus) {
            target = 0;
            CYG_REPORT_RETVAL(false);
            return false;
        }
        seen_minus = true;
        negative = true;
        ptr++;
    }

    cdl_int acc = 0;
    if ('0' == *ptr) {
        // This happens sufficiently often to be worth a special case.
        if ('\0' == ptr[1]) {
            target = 0;
            CYG_REPORT_RETVAL(true);
            return true;
        }
        // Hex is always worth supporting. 
        if (('x' == ptr[1]) || ('X' == ptr[1])) {
            ptr++; ptr++;
            if (!isxdigit(*ptr)) {
                CYG_REPORT_RETVAL(false);
                return false;
            }
            while (isxdigit(*ptr)) {
                cdl_int new_acc = acc * 16;
                if (isdigit(*ptr)) {
                    new_acc += (*ptr - '0');
                } else if (('a' <= *ptr) && (*ptr <= 'f')) {
                    new_acc += (*ptr + 10 - 'a');
                } else if (('A' <= *ptr) && (*ptr <= 'F')) {
                    new_acc += (*ptr + 10 - 'A');
                } else {
                    CYG_FAIL("this platform's implementation of isxdigit() is broken");
                }
                if (new_acc < acc) {
                    CYG_REPORT_RETVAL(false);
                    return false;
                }
                acc = new_acc;
                ptr++;
            }
            if ('\0' != *ptr) {
                CYG_REPORT_RETVAL(false);
                return false;
            }
            if (negative) {
                cdl_int new_acc = 0 - acc;
                if (new_acc > 0) {
                    CYG_REPORT_RETVAL(false);
                    return false;
                } else {
                    acc = new_acc;
                }
            }
            target = acc;
            CYG_REPORT_RETVAL(true);
            return true;
        }

        // Octal? Oh well, might as well be complete.
        if (('0' <= ptr[1]) && (ptr[1] <= '7')) {
            ptr++;
            do {
                cdl_int new_acc = 8 * acc;
                new_acc += (*ptr - '0');
                if (new_acc < acc) {
                    CYG_REPORT_RETVAL(false);
                    return false;
                }
                acc = new_acc;
                ptr++;
            } while (('0' <= *ptr) && (*ptr <= '7'));
            if ('\0' != *ptr) {
                CYG_REPORT_RETVAL(false);
                return false;
            }
            if (negative) {
                cdl_int new_acc = 0 - acc;
                if (new_acc > 0) {
                    CYG_REPORT_RETVAL(false);
                    return false;
                }
                else {
                    acc = new_acc;
                }
            }
            target = acc;
            CYG_REPORT_RETVAL(true);
            return true;
        }

        // Drop through for the case of a decimal.
    }

    while(isdigit(*ptr)) {
        cdl_int new_acc = 10 * acc;
        new_acc += (*ptr - '0');
        if (new_acc < acc) {
            CYG_REPORT_RETVAL(false);
            return false;
        }
        acc = new_acc;
        ptr++;
    }
    if ('\0' != *ptr) {
        CYG_REPORT_RETVAL(false);
        return false;
    }
    if (negative) {
        cdl_int new_acc = 0 - acc;
        if (new_acc > 0) {
            CYG_REPORT_RETVAL(false);
            return false;
        } else {
            acc = new_acc;
        }
    }
    target = acc;
    CYG_REPORT_RETVAL(true);
    return true;
}

//}}}
//{{{  string_to_double()                       

// ----------------------------------------------------------------------------
// There is no point in doing this the hard way, just use standard
// library calls.
//
// There is an obvious question as to how much precision can get lost
// doing the conversion to a string. In practice this should not matter
// too much, since the expression handling code generally keeps the
// original double precision lying around to be re-used. However it may
// be desirable to keep the libcdl behaviour in synch with Tcl's
// tcl_precision variable.

bool
Cdl::string_to_double(std::string value, double& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::string_to_double", "success %d");

    bool        result      = true;
    const char* start_ptr   = value.c_str();
    char*       end_ptr;
    int         old_errno   = errno;
    
    errno                   = 0;
    double conv             = strtod(start_ptr, &end_ptr);
    if (0 != errno) {
        CYG_ASSERT(ERANGE == errno, "standard-compliant C library");
        result = false;
    } else if ('\0' != *end_ptr) {
        result = false;
    } else {
        target = conv;
        result = true;
    }
     
    errno = old_errno;
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  string_to_bool()                         

// ----------------------------------------------------------------------------
// Conversions to and from bools. The only real issue here is exactly
// what strings should be accepted as synonyms for true and false.
// It is not actually clear that these functions are useful.
bool
Cdl::string_to_bool(std::string data, bool& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::string_to_bool", "success %d");

    // Arguably there should be a precondition ( "" != data )
    bool result = false;

    // What is truth ?
    if (( data == "1"   ) || (data == "true") ||
        ( data == "True") || (data == "TRUE") ) {
        result = true;
        target = true;
    } else if ((data == "0"    ) || (data == "false") ||
               (data == "False") || (data == "FALSE") ) {
        result = true;
        target = false;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}

//{{{  integer_to_string()                      

// ----------------------------------------------------------------------------

std::string
Cdl::integer_to_string(cdl_int value, CdlValueFormat format)
{
    std::string result;
    Cdl::integer_to_string(value, result, format);
    return result;
}

void
Cdl::integer_to_string(cdl_int value, std::string& target, CdlValueFormat format)
{
    CYG_REPORT_FUNCNAME("Cdl::integer_to_string");
    CYG_REPORT_FUNCARG2XV((long) value, format);

    // Optimise this special case.
    if (0 == value) {
        if (CdlValueFormat_Hex == format) {
            target = "0x0";
        } else {
            target = "0";
        }
        CYG_REPORT_RETVAL(true);
        return;
    }

    // A local buffer to construct partial strings. This avoids
    // unnecessary std::string reallocation.
    // 64 bits and three bits at a time for octal numbers gives 21 digits,
    // plus spares for the leading '0' and the terminator.
    char local_buf[24];
    char *local_ptr = &(local_buf[23]);
    *local_ptr-- = '\0';
    
    if (CdlValueFormat_Hex == format) {

        // Output the data as 0x... with either 8 or 16 digits,
        // depending on the size.
        int i;
        for (i = 0; i < 8; i++) {
            int tmp = (int) (value & 0x0F);
            value   = value >> 4;
            if (tmp < 10) {
                *local_ptr-- = '0' + tmp;
            } else {
                *local_ptr-- = 'A' + (tmp - 10);
            }
        }
        // Beware of right shifts that preserve the sign bit.
        {
            int tmp1 = (value & 0x0FFFF);
            int tmp2 = ((value >> 16) & 0x0FFFF);
            value    = (tmp2 << 16) + tmp1;
        }
        if (value != 0) {
            for (i = 0; i < 8; i++) {
                int tmp = (int) (value & 0x0F);
                value   = value >> 4;
                if (tmp < 10) {
                    *local_ptr-- = '0' + tmp;
                } else {
                    *local_ptr-- = 'A' + (tmp - 10);
                }
            }
        }
        *local_ptr-- = 'x';
        *local_ptr   = '0';
        target = std::string(local_ptr);
        
    } else if (CdlValueFormat_Octal == format) {

        // Simply output the data three bits at a time, do not worry about any
        // particular width restrictions. However it is necessary to worry
        // about masking.
        cdl_int mask = 0x1FFFFFFF;
        mask = (mask << 16) | 0x0FFFF;
        mask = (mask << 16) | 0x0FFFF;

        target = "";
        while (value > 0) {
            int tmp = value & 0x07;
            value   = (value >> 3) & mask;
            *local_ptr-- = '0' + tmp;
        }
        *local_ptr = '0';
        target = std::string(local_ptr);
        
    } else {
        // A simple decimal number      
        // Switch to positive arithmetic.
        bool negative = false;
        if (value < 0) {
            negative = true;
            value    = 0 - value;
            // Only MININT cannot be converted using the above line
            if (value < 0) {
                target = "-9223372036854775808";
                CYG_REPORT_RETVAL(true);
                return;
            }
        }

        while (value > 0) {
            int rem = (int) (value % 10);
            value   = value / 10;
            *local_ptr-- = '0' + rem;
        }
        if (negative) {
            *local_ptr-- = '-';
        }
        local_ptr++;
        target = std::string(local_ptr);
    }
    
    CYG_REPORT_RETURN();
    return;
}

//}}}
//{{{  double_to_string()                       

// ----------------------------------------------------------------------------

std::string
Cdl::double_to_string(double value, CdlValueFormat format)
{
    std::string result;
    Cdl::double_to_string(value, result, format);
    return result;
}

void
Cdl::double_to_string(double value, std::string& result, CdlValueFormat format)
{
    CYG_REPORT_FUNCNAME("Cdl::double_to_String");

    char buf[256];      // This should be plenty :-)
    sprintf(buf, "%.*G", DBL_DIG, value);
    result = buf;

    CYG_UNUSED_PARAM(CdlValueFormat, format);
    CYG_REPORT_RETURN();
}

//}}}
//{{{  bool_to_string()                         

// ----------------------------------------------------------------------------
// Should the string results be 1/0 or true/false? Not that
// it really matters. The testcase in cdl1.cxx expects 1/0.
std::string
Cdl::bool_to_string(bool value)
{
    std::string result;
    Cdl::bool_to_string(value, result);
    return result;
}

void
Cdl::bool_to_string(bool value, std::string& target)
{
    CYG_REPORT_FUNCNAME("Cdl::bool_to_string");
    CYG_REPORT_FUNCARG1( "value arg %ld", (long) value);

    if (value)
        target = "1";
    else
        target = "0";

    CYG_REPORT_RETURN();
}

//}}}

//{{{  integer_to_double()                      

// ----------------------------------------------------------------------------
// Currently integer to double cannot fail, although there may well be loss
// of accurary. Eventually cdl_int may be an arbitrary precision integer
// in which case conversion to double is not guaranteed.
double
Cdl::integer_to_double(cdl_int value)
{
    CYG_REPORT_FUNCNAME("Cdl::integer_to_double");

    double result = (double) value;

    CYG_REPORT_RETURN();
    return result;
}

void
Cdl::integer_to_double(cdl_int value, double& target)
{
    CYG_REPORT_FUNCNAME("Cdl::integer_to_double");

    target = (double) value;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  double_to_integer()                      

// Conversion from double to integer is only allowed if there is no loss
// of data. modf() is useful here
bool
Cdl::double_to_integer(double value, cdl_int& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::double_to_integer", "result %d");

    bool   result = false;
    
    double integral;
    double frac;

    frac = modf(value, &integral);
    if (0.0 == frac) {
        // Looking good, but integral may still be too big.
        cdl_int tmp = (cdl_int) integral;
        if (tmp == value) {
            // No fraction, no loss of data, everything looking good
            target = tmp;
            result = true;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}

//}}}
//{{{  Cdl::xxx_to_yyy() - CDL-specific data types              

// ----------------------------------------------------------------------------
// Conversions between strings and flavors.

static struct {
    char*               name;
    CdlValueFlavor      flavor;
} valid_flavors[] = {
    { "none",           CdlValueFlavor_None     },
    { "bool",           CdlValueFlavor_Bool     },
    { "booldata",       CdlValueFlavor_BoolData },
    { "data",           CdlValueFlavor_Data     },
    { 0,                CdlValueFlavor_Invalid  }
};

bool
Cdl::string_to_flavor(std::string name, CdlValueFlavor& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::string_to_flavor", "success %d");

    bool result = false;

    // First convert the argument to lower case. Arguably this is incorrect,
    // Tcl is a case-sensitive language, but the conversion is unlikely ever
    // to be harmfull.
    for (std::string::iterator str_i = name.begin(); str_i != name.end(); str_i++) {
        if (isupper(*str_i)) {
            *str_i = tolower(*str_i);
        }
    }
           
    // Now look for a match in the table.
    int match           = -1;
    int i;
    const char* c_str   = name.c_str();
    int len             = strlen(c_str);

    for (i = 0; 0 != valid_flavors[i].name; i++) {
        if (0 == strncmp(c_str, valid_flavors[i].name, len)) {
            // Check for an ambiguous string match.
            // This cannot actually happen with the current flavor names.
            if ( -1 != match) {
                break;
            }
            match = i;
        }
        
    }
    if (-1 != match) {
        target = valid_flavors[match].flavor;
        result = true;
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
Cdl::flavor_to_string(CdlValueFlavor flavor, std::string& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::flavor_to_string", "success %d");

    bool result = false;

    for (int i = 0; 0 != valid_flavors[i].name; i++) {
        if (flavor == valid_flavors[i].flavor) {
            target = valid_flavors[i].name;
            result = true;
            break;
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Similar support for value sources.

static struct {
    char*               name;
    CdlValueSource      source;
} valid_sources[] = {
    { "default",        CdlValueSource_Default  },
    { "inferred",       CdlValueSource_Inferred },
    { "wizard",         CdlValueSource_Wizard   },
    { "user",           CdlValueSource_User     },
    { 0,                CdlValueSource_Invalid  }
};

bool
Cdl::string_to_source(std::string name, CdlValueSource& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::string_to_source", "success %d");

    bool result = false;

    // First convert the argument to lower case. Arguably this is incorrect,
    // Tcl is a case-sensitive language, but the conversion is unlikely ever
    // to be harmfull.
    for (std::string::iterator str_i = name.begin(); str_i != name.end(); str_i++) {
        if (isupper(*str_i)) {
            *str_i = tolower(*str_i);
        }
    }
           
    // Now look for a match in the table.
    int match           = -1;
    int i;
    const char* c_str   = name.c_str();
    int len             = strlen(c_str);

    for (i = 0; 0 != valid_sources[i].name; i++) {
        if (0 == strncmp(c_str, valid_sources[i].name, len)) {
            // Check for an ambiguous string match.
            // This cannot actually happen with the current source names.
            if ( -1 != match) {
                break;
            }
            match = i;
        }
        
    }
    if (-1 != match) {
        target = valid_sources[match].source;
        result = true;
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
Cdl::source_to_string(CdlValueSource source, std::string& target)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::source_to_string", "success %d");

    bool result = false;

    for (int i = 0; 0 != valid_sources[i].name; i++) {
        if (source == valid_sources[i].source) {
            target = valid_sources[i].name;
            result = true;
            break;
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Cdl::get_library_version()                               

// ----------------------------------------------------------------------------
// The version of the library actually lives inside configure.in. It gets
// exported into cdlconfig.h
std::string
Cdl::get_library_version(void)
{
    return std::string(CYGNUM_LIBCDL_VERSION);
}

//}}}
//{{{  Cdl::set_interactive()                                   

// ----------------------------------------------------------------------------
// Some CDL scripts and some bits of the library may want to adapt depending
// on whether or not the application is running fully interactively or in
// batch mode. The primary distinction is that a batch program should never
// attempt to obtain user input, whether via Tk widgets or by other means.

bool Cdl::interactive   = false;

void
Cdl::set_interactive(bool value)
{
    CYG_REPORT_FUNCNAME("Cdl::set_interactive");
    CYG_REPORT_FUNCARG1D(value);

    interactive = value;
}

bool
Cdl::is_interactive(void)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::is_interactive", "interactive %d");
    CYG_REPORT_RETVAL(interactive);
    return interactive;
}

//}}}
//{{{  version support()                                        

// ----------------------------------------------------------------------------
// Packages may need to impose constraints on which versions of other
// packages they can coexist with. This requires some way of achieving
// a partial ordering of version numbers. Unfortunately there are many
// different ways of specifying a version number, and we cannot impose
// a single model on all third party package developers. Instead this
// routine performs some semi-intelligent comparisons of two version
// strings which should work in the vast majority of cases.
//
// The return value is as per strcmp(), -1 if the first entry is
// smaller (i.e. the more recent and hence hopefully the first in
// a list), +1 if the second entry is smaller, 0 if the two are
// identical.
//
// There is a big ambiguity between "derived" versions and "experimental"
// versions. Something like v0.3beta is experimental, i.e. it is older
// than the full release v0.3. On the other hand v0.3.p1 is a patched
// version of v0.3 and hence newer. This code uses the presence or otherwise
// of a separator to decide between the two cases.

// A utility routine which checks whether or not a character counts
// as a separator. Currently the characters . - and _ are all accepted
// as field separators.
//
// Arguably - should not be accepted as a separator. Instead if it preceeds
// a digit it could be interpreted as part of a prerelease number.

static bool
is_separator(int ch)
{
    return ('.' == ch) || ('-' == ch) || ('_' == ch);
}
    
int
Cdl::compare_versions(std::string arg1, std::string arg2)
{
    CYG_REPORT_FUNCNAMETYPE("Cdl::compare_versions", "result %d");

    if (arg1 == arg2) {
        CYG_REPORT_RETVAL(0);
        return 0;
    }

    // The version number "current" is special, it always indicates the most
    // recent version e.g. as checked out from a CVS repository.
    if ("current" == arg1) {
        CYG_REPORT_RETVAL(-1);
        return -1;
    }
    if ("current" == arg2) {
        CYG_REPORT_RETVAL(1);
        return 1;
    }

    const char* ptr1 = arg1.c_str();
    const char* ptr2 = arg2.c_str();      
    int num1    = 0;
    int num2    = 0;

    // If both strings start with 'v' or 'V', skip this. A minor variation in
    // case at the start of a string should be ignored.
    if ((('v' == *ptr1) || ('V' == *ptr1)) &&
        (('v' == *ptr2) || ('V' == *ptr2))) {
        ptr1++;
        ptr2++;
    }

    // Now process the rest of the version string, one unit at a time.
    while (1) {

        if (('\0' == *ptr1) && ('\0' == *ptr2)) {
            // Both strings have terminated at the same time. There
            // may have been some spurious leading zeroes in numbers,
            // or irrelevant differences in the separators.
            CYG_REPORT_RETVAL(0);
            return 0;
        }
        
        if ('\0' == *ptr1) {
            // The first string has ended first. If the second string currently
            // points at a separator then arg2 is a derived version, e.g.
            // v0.3.p1, and hence newer. Otherwise arg2 is an experimental
            // version v0.3beta and hence older.
            if (is_separator(*ptr2)) {
                CYG_REPORT_RETVAL(1);
                return 1;
            } else {
                CYG_REPORT_RETVAL(-1);
                return -1;
            }
        }

        if ('\0' == *ptr2) {
            // As per the previous test.
            if (is_separator(*ptr1)) {
                CYG_REPORT_RETVAL(-1);
                return -1;
            } else {
                CYG_REPORT_RETVAL(1);
                return 1;
            }
        }

        // If both strings currently point at numerical data, do a conversion and
        // a numerical comparison.
        if (isdigit(*ptr1) && isdigit(*ptr2)) {
            num1 = 0;
            num2 = 0;
            // Strictly speaking there should be some overflow tests here, but it
            // is not worth the trouble.
            do {
                num1 = (10 * num1) + (*ptr1++ - '0');
            } while(isdigit(*ptr1));
            do {
                num2 = (10 * num2) + (*ptr2++ - '0');
            } while(isdigit(*ptr2));
            // v2.0 is newer than v1.0
            if (num1 < num2) {
                CYG_REPORT_RETVAL(1);
                return 1;
            } else if (num1 > num2) {
                CYG_REPORT_RETVAL(-1);
                return -1;
            } else {
                continue;
            }
        }

        // Non-numerical data. If the two characters are the same then
        // move on. Note: this has to happen after numerical conversions
        // to distinguish v10.0 and v1.0
        if (*ptr1 == *ptr2) {
            ptr1++; ptr2++;
            continue;
        }

        // If both strings are currently at a separator then move on. All
        // separators can be used interchangeably.
        if (is_separator(*ptr1) && is_separator(*ptr2)) {
            ptr1++; ptr2++;
            continue;
        }

        // If only one string is at a separator, special action
        // is needed. v1.1alpha is interpreted as earlier than
        // v1.1, but v1.1.3 is a later release.
        if (is_separator(*ptr1)) {
            return -1;
        } else if (is_separator(*ptr2)) {
            return 1;
        }
        
        // Two different characters, e.g. v1.0alpha vs. v1.0beta
        if (*ptr1 < *ptr2) {
            CYG_REPORT_RETVAL(1);
            return 1;
        } else {
            CYG_REPORT_RETVAL(-1);
            return -1;
        }
    }

    // Not reachable.
}

// ----------------------------------------------------------------------------
// Given a version string, extract major, minor and release numbers.
// Some or all of these may be absent. Basically the code just
// iterates through the string looking for sequences of numbers.

static void
version_extract_number(const std::string& version, unsigned int& index, std::string& result)
{
    CYG_REPORT_FUNCNAME("version_extract_number");

    // The calling code is expected to supply a sensible default.
    // Search for a digit
    for ( ; index < version.size(); index++) {
        if (isdigit(version[index])) {
            break;
        }
    }
    if (index != version.size()) {
        result = "";
        if ((index > 0) && ('-' == version[index-1])) {
            result = "-";
        }
        do {
            result += version[index++];
        } while ((index < version.size()) && isdigit(version[index]));
    }
    
    CYG_REPORT_RETURN();
}

void
Cdl::split_version_string(const std::string& version, std::string& major, std::string& minor, std::string& release)
{
    CYG_REPORT_FUNCNAME("CdlMisc::split_version_string");

    unsigned int index = 0;
    version_extract_number(version, index, major);
    version_extract_number(version, index, minor);
    version_extract_number(version, index, release);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Cdl::get_short_form()                                    

// ----------------------------------------------------------------------------
// It is occasionally useful to take a full CDL name such as CYgpkg_KERNEL
// and turn it into a short form such as "kernel". This involves discarding
// everything up to and including the first underscore, then lowercasing
// all subsequent characters.
std::string
Cdl::get_short_form(const std::string& original)
{
    CYG_REPORT_FUNCNAME("CdlMisc::get_short_form");

    std::string  result = "";
    unsigned int size = original.size();
    unsigned int i;
    for (i = 0; i < size; i++) {
        if ('_' == original[i]) {
            i++;
            break;
        }
    }

    // Either at end of string, or just past the first underscore
    for ( ; i < size; i++) {
        if (isupper(original[i])) {
            result += tolower(original[i]);
        } else {
            result += original[i];
        }
    }
    
    CYG_REPORT_RETURN();
    return result;
}

//}}}
