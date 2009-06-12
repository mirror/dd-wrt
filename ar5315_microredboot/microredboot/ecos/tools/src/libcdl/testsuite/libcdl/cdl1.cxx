//==========================================================================
//
//      cdl1.cxx
//
//      Basic testing of the CDL utility functions.                                                        
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1999, 2000 Red Hat, Inc.
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
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                1999-01-06
// Description:         There are a number of utility functions in the
//                      Cdl class to handle data validation, various
//                      conversions, etc.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cdlconfig.h>
#include <cdl.hxx>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/testcase.h>
#include <cstdlib>
#include <cmath>

static bool
test_is_valid_property_id(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_is_valid_property_id", "success %d");

    if (!Cdl::is_valid_property_id(CdlPropertyId_ActiveIf)) {
        CYG_TEST_FAIL("ActiveIf should be a valid property id");
        result = false;
    }
    if (!Cdl::is_valid_property_id(CdlPropertyId_DefaultValue)) {
        CYG_TEST_FAIL("DefaultValue should be a valid property id");
        result = false;
    }
    if (!Cdl::is_valid_property_id(CdlPropertyId_Wizard)) {
        CYG_TEST_FAIL("Wizard should be a valid property id");
        result = false;
    }
    if (Cdl::is_valid_property_id(CdlPropertyId_Unknown)) {
        CYG_TEST_FAIL("is_valid_property_id() accepted an invalid number");
        result = false;
    }
    // This test could give spurious negatives in the very long term
    union {
        int           x;
        CdlPropertyId id;
    } dummy;
    dummy.x = 1234;
    if (Cdl::is_valid_property_id(dummy.id)) {
        CYG_TEST_FAIL("is_valid_property_id() accepted an invalid number");
        result = false;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_is_valid_property_class(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_is_valid_property_class", "success %d");

    if (!Cdl::is_valid_property_class(CdlPropertyClass_Minimal)) {
        CYG_TEST_FAIL("Minimal should be a valid property class");
        result = false;
    }
    if (!Cdl::is_valid_property_class(CdlPropertyClass_TclCode)) {
        CYG_TEST_FAIL("TclCode should be a valid property class");
        result = false;
    }
    if (!Cdl::is_valid_property_class(CdlPropertyClass_GoalExpression)) {
        CYG_TEST_FAIL("GoalExpression should be a valid property class");
        result = false;
    }
    if (Cdl::is_valid_property_class(CdlPropertyClass_Unknown)) {
        CYG_TEST_FAIL("is_valid_property_class() accepted an invalid number");
        result = false;
    }
    // This test could give spurious negatives in the very long term
    union {
        int                     x;
        CdlPropertyClass        id;
    } dummy;
    dummy.x = 1234;
    if (Cdl::is_valid_property_class(dummy.id)) {
        CYG_TEST_FAIL("is_valid_property_class() accepted an invalid number");
        result = false;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_is_valid_object_type(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_is_valid_object_type", "success %d");

    if (!Cdl::is_valid_object_type(CdlObjectType_Package)) {
        CYG_TEST_FAIL("Package should be a valid object type");
        result = false;
    }
    if (!Cdl::is_valid_object_type(CdlObjectType_Component)) {
        CYG_TEST_FAIL("Component should be a valid object type");
        result = false;
    }
    if (!Cdl::is_valid_object_type(CdlObjectType_Option)) {
        CYG_TEST_FAIL("Option should be a valid object type");
        result = false;
    }
    if (Cdl::is_valid_object_type(CdlObjectType_Unknown)) {
        CYG_TEST_FAIL("is_valid_object_type() accepted an invalid number");
        result = false;
    }
    // This test could give spurious negatives in the very long term
    union {
        int             x;
        CdlObjectType   id;
    } dummy;
    dummy.x = 1234;
    if (Cdl::is_valid_object_type(dummy.id)) {
        CYG_TEST_FAIL("is_valid_object_type() accepted an invalid number");
        result = false;
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_is_valid_option_flavor(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_is_valid_option_flavor", "success %d");

    if (!Cdl::is_valid_option_flavor(CdlOptionFlavor_Bool)) {
        CYG_TEST_FAIL("Bool should be a valid option flavor");
        result = false;
    }
    if (!Cdl::is_valid_option_flavor(CdlOptionFlavor_Enum)) {
        CYG_TEST_FAIL("Enum should be a valid option flavor");
        result = false;
    }
    if (!Cdl::is_valid_option_flavor(CdlOptionFlavor_Count)) {
        CYG_TEST_FAIL("Count should be a valid option flavor");
        result = false;
    }
    if (Cdl::is_valid_option_flavor(CdlOptionFlavor_Unknown)) {
        CYG_TEST_FAIL("is_valid_option_flavor() accepted an invalid number");
        result = false;
    }
    // This test could give spurious negatives in the very long term
    union {
        int             x;
        CdlOptionFlavor id;
    } dummy;
    dummy.x = 1234;
    if (Cdl::is_valid_option_flavor(dummy.id)) {
        CYG_TEST_FAIL("is_valid_option_flavor() accepted an invalid number");
        result = false;
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_is_valid_value_source(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_is_valid_value_source", "success %d");

    if (!Cdl::is_valid_value_source(CdlValueSource_Default)) {
        CYG_TEST_FAIL("Default is a valid value source");
        result = false;
    }
    if (!Cdl::is_valid_value_source(CdlValueSource_User)) {
        CYG_TEST_FAIL("User is a valid value source");
        result = false;
    }
    if (!Cdl::is_valid_value_source(CdlValueSource_Wizard)) {
        CYG_TEST_FAIL("Wizard is a valid value source");
        result = false;
    }
    if (!Cdl::is_valid_value_source(CdlValueSource_Inferred)) {
        CYG_TEST_FAIL("Inferred is a valid value source");
        result = false;
    }
    if (Cdl::is_valid_value_source(CdlValueSource_Unknown)) {
        CYG_TEST_FAIL("is_valid_value_source() accepted an invalid number");
        result = false;
    }
    // This test could give spurious negatives in the very long term
    union {
        int             x;
        CdlValueSource  id;
    } dummy;
    dummy.x = 1234;
    if (Cdl::is_valid_value_source(dummy.id)) {
        CYG_TEST_FAIL("is_valid_value_source() accepted an invalid number");
        result = false;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_string_to_integer(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_string_to_integer", "success %d");

    // Note that there is no robust way of specifying 64 bit constants.
    // Some compilers will use a LL suffix, but not all. When a 64 bit
    // constant is needed this has to be achieved using arithmetic,
    // leaving the compiler to do the hard work.
    //
    // Compiler bogosity: VC++ does not appear the more normal way
    // of initializing an array of structures using nested braces.
    struct conversion_data {
        bool            ok;
        const char*     source;
        cdl_int         expected;
    } data[] = {
        {  true,          "0",            0 },
        {  true,          "1",            1 },
        {  true,         "-1",           -1  },
        {  true,        "0x0",            0  },
        {  true,        "0x1",            1  },
        {  true,         "01",            1  },
        {  true,       "1234",         1234  },
        {  true,      "-4567",        -4567  },
        {  true, "2147483647",   2147483647  },
        {  true,"-2147483648",  ((cdl_int) -2147483647) - 1  },
        {  true, "0x7fffffff",   2147483647  },
        {  true,"-0x80000000",  ((cdl_int) -2147483647) -1  },
        {  true, "0x12ABCDEF",    313249263  },
        {  true, "12345678987654321", ((cdl_int) 111111111) * ((cdl_int) 111111111) },
        { false,          "A",            0  },
        { false,         "0A",            0  },
        { false,      "1234*",            0  },
        { false,   "finished",            0  }
    };

    for (int i = 0; 0 != strcmp("finished", data[i].source); i++) {
        cdl_int     res;
        std::string source = std::string(data[i].source);
        if (data[i].ok) {
            if (!Cdl::string_to_integer(source, res)) {
                std::string msg = "the string \"" + source + "\" was not converted";
                CYG_TEST_FAIL(msg.c_str());
                result = false;
            } else if (res != data[i].expected) {
                std::string msg = "the string \"" + source + "\" was converted incorrectly";
                CYG_TEST_FAIL(msg.c_str());
                result = false;
            }
        } else {
            if (Cdl::string_to_integer(source, res)) {
                std::string msg = "the string \"" + source + "\" is invalid but was still converted";
                CYG_TEST_FAIL(msg.c_str());
                result = false;
            }
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_integer_to_string(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_integer_to_string", "success %d");
    
    // Note that there is no robust way of specifying 64 bit constants.
    // Some compilers will use a LL suffix, but not all. When a 64 bit
    // constant is needed this has to be achieved using arithmetic,
    // leaving the compiler to do the hard work.
    //
    // Integer to string conversions cannot fail.
    //
    // Compiler bogosity: VC++ does not appear the more normal way
    // of initializing an array of structures using nested braces.
    struct conversion_data {
        cdl_int         source;
        const char*     expected;
    } data[] = {
        {                0,           "0"  },
        {                1,           "1"  },
        {               -1,          "-1"  },
        {               10,          "10"  },
        {             1234,        "1234"  },
        {          -456789,     "-456789"  },
        {       2147483647,  "2147483647"  },
        { ((cdl_int) 111111111) * ((cdl_int) 111111111), "12345678987654321"  },
        {      (cdl_int) 0, "finished"     }
    };

    for (int i = 0; 0 != strcmp("finished", data[i].expected); i++) {
        std::string res;
        if (!Cdl::integer_to_string(data[i].source, res)) {
            std::string msg = "the integer \"" + std::string(data[i].expected) + "\" was not converted";
            CYG_TEST_FAIL(msg.c_str());
            result = false;
        } else if (res != data[i].expected) {
            std::string msg = "the string \"" + std::string(data[i].expected) + "\" was converted incorrectly";
            CYG_TEST_FAIL(msg.c_str());
            result = false;
        }
    }

    // Just a few more tests. Try converting some sequences to a string and back
    // again.
    for (int j = 0; j < 4; j++) {
        cdl_int starting_values[] = { 1, 3, -1, -2 };
        cdl_int current_value     = starting_values[j];

        for (int k = 0; k < 60; k++) {
            current_value <<= 1;
            cdl_int     int_tmp;
            std::string str_tmp;
            if (!Cdl::integer_to_string(current_value, str_tmp)) {
                CYG_TEST_FAIL("unable to convert valid integer to string");
                result = false;
                break;
            }
            if (!Cdl::string_to_integer(str_tmp, int_tmp)) {
                CYG_TEST_FAIL("unable to convert string back to integer");
                result = false;
                break;
            }
            if (current_value != int_tmp) {
                CYG_TEST_FAIL("integer conversion to/from strings not idempotent");
                result = false;
                break;
            }
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_string_to_bool(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_string_to_bool", "success %d");

    // Legal values for true  include 1 and "true".
    // Legal values for false include 0 and "false".
    // A random string should fail to convert.
    bool res;
    if (!Cdl::string_to_bool("1", res)) {
        CYG_TEST_FAIL("the string \"1\" was not converted");
        result = false;
    } else if (res != true) {
        CYG_TEST_FAIL("the string \"1\" did not convert to true");
        result = false;
    }
    if (!Cdl::string_to_bool("true", res)) {
        CYG_TEST_FAIL("the string \"true\" was not converted");
        result = false;
    } else if (res != true) {
        CYG_TEST_FAIL("the string \"true\" did not convert to true");
        result = false;
    }
    if (!Cdl::string_to_bool("0", res)) {
        CYG_TEST_FAIL("the string \"0\" was not converted");
        result = false;
    } else if (res != false) {
        CYG_TEST_FAIL("the string \"0\" did not convert to false");
        result = false;
    }
    if (!Cdl::string_to_bool("false", res)) {
        CYG_TEST_FAIL("the string \"false\" was not converted");
        result = false;
    } else if (res != false) {
        CYG_TEST_FAIL("the string \"false\" did not convert to false");
        result = false;
    }
    if (Cdl::string_to_bool("not a boolean string", res)) {
        CYG_TEST_FAIL("a spurious string was converted to a boolean");
        result = false;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

static bool
test_bool_to_string(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_bool_to_string", "success %d");

    std::string str;
    if (!Cdl::bool_to_string(true, str)) {
        CYG_TEST_FAIL("bool_to_string() failed for `true'");
        result = false;
    } else if ("1" != str) {
        CYG_TEST_FAIL("boolean value true translated incorrectly");
        result = false;
    }
    if (!Cdl::bool_to_string(false,str)) {
        CYG_TEST_FAIL("bool_to_string() failed for `true'");
        result = false;
    } else if ("0" != str) {
        CYG_TEST_FAIL("boolean value false translated incorrectly");
        result = false;
    }
    // There is no easy way to test failure conditions. The trick
    // of sticking a random number into a union will not work, there
    // are absolutely no guarantees about the internal implementation
    // of the bool data type in C++
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// This test is not intended to be comprehensive. In particular it is
// not very easy to validate the results, issues like locales get in the way.
static bool
test_double_to_string(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_double_to_string", "success %d");

    std::string str;
    if (!Cdl::double_to_string(0.0, str)) {
        CYG_TEST_FAIL("double_to_string() failed for 0.0");
        result = false;
    } else if (('0' != str[0]) && (('-' != str[0]) && ('0' != str[1]))) {
        fprintf(stderr, "result of conversion is %s\n", str.c_str());
        CYG_TEST_FAIL("double_to_string() returned strange result for 0.0");
        result = false;
    }
    
    if (!Cdl::double_to_string(3.141592, str)) {
        CYG_TEST_FAIL("double_to_string() failed for pi");
        result = false;
    } else if (('3' != str[0]) || ('1' != str[2]) || ('4' != str[3]) || ('1' != str[4])) {
        CYG_TEST_FAIL("double_to_string() returned strange result for pi");
        result = false;
    }

    if (!Cdl::double_to_string(-1.23456789E15, str)) {
        CYG_TEST_FAIL("double_to_string() failed for large but legal value");
        result = false;
    } else if (('-' != str[0]) && ('1' != str[1]) && (10 >= str.size())) {
        CYG_TEST_FAIL("double_to_string() returned strange result for large but legal value");
        result = false;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// This test is not intended to be comprehensive.
static bool
test_string_to_double(void)
{
    bool result = true;
    CYG_REPORT_FUNCNAMETYPE("test_string_to_double", "success %d");

    double val;
    if (!Cdl::string_to_double("0.0", val)) {
        CYG_TEST_FAIL("string_to_double() failed for 0.0");
        result = false;
    } else if (fabs(val) > 0.000001) {
        CYG_TEST_FAIL("string_to_double() produced strange result for 0.0");
        result = false;
    }
    if (!Cdl::string_to_double("3.141592", val)) {
        CYG_TEST_FAIL("string_to_double() failed for pi");
        result = false;
    } else if (fabs(val - 3.141592) > 0.000001) {
        CYG_TEST_FAIL("string_to_double() produced strange result for pi");
        result = false;
    }
    if (!Cdl::string_to_double("-1.23456789E15", val)) {
        CYG_TEST_FAIL("string_to_double() failed for large but legal value");
        result = false;
    } else if (fabs(val - -1.23456789E15) > 0.000001) {
        CYG_TEST_FAIL("string_to_double() produced strange result for large but legal value");
        result = false;
    }
    if (Cdl::string_to_double("random junk", val)) {
        CYG_TEST_FAIL("string_to_double() succeeded for junk data");
        result = false;
    }
    if (Cdl::string_to_double("1.23456789E1234", val)) {
        CYG_TEST_FAIL("string_to_double() succeeded for impossibly large value");
        result = false;
    }
    if (Cdl::string_to_double("1.0 and then some", val)) {
        CYG_TEST_FAIL("string_to_double() succeeded for number followed by junk");
        result = false;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

int
main(int argc, char** argv)
{
    CYG_REPORT_FUNCNAMETYPE("main", "result %d");
    
    if (test_is_valid_property_id()) {
        CYG_TEST_PASS("is_valid_property_id()");
    }
    if (test_is_valid_property_class()) {
        CYG_TEST_PASS("is_valid_property_class()");
    }
    if (test_is_valid_object_type()) {
        CYG_TEST_PASS("is_valid_object_type()");
    }
    if (test_is_valid_option_flavor()) {
        CYG_TEST_PASS("is_valid_option_flavor");
    }
    if (test_is_valid_value_source()) {
        CYG_TEST_PASS("is_valid_value_source");
    }
    if (test_string_to_integer()) {
        CYG_TEST_PASS("string_to_integer");
    }
    if (test_string_to_bool()) {
        CYG_TEST_PASS("string to bool");
    }
    if (test_integer_to_string()) {
        CYG_TEST_PASS("integer_to_string");
    }
    if (test_bool_to_string()) {
        CYG_TEST_PASS("bool_to_string");
    }
    if (test_string_to_double()) {
        CYG_TEST_PASS("string_to_double");
    }
    if (test_double_to_string()) {
        CYG_TEST_PASS("double_to_string");
    }
    
    CYG_REPORT_RETVAL(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

