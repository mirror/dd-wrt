//{{{  Banner                           

//============================================================================
//
//      value.cxx
//
//      Implementation of value-related CDL classes.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
// Copyright (C) 1999, 2000, 2001 Red Hat, Inc.
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
// Date:        1999/07/12
// Version:     0.02
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

//}}}

//{{{  Statics                          

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlValue);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlListValue);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlValuableBody);

//}}}
//{{{  CdlSimpleValue class             

//{{{  Constructors                     

// ----------------------------------------------------------------------------

CdlSimpleValue::CdlSimpleValue()
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    value               = "0";
    int_value           = 0;
    double_value        = 0.0;
    valid_flags         = int_valid | double_valid | string_valid;
    format              = CdlValueFormat_Default;

    CYG_REPORT_RETURN();
}

CdlSimpleValue::CdlSimpleValue(std::string val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: string constructor");
    CYG_REPORT_FUNCARG1XV(this);

    value               = val;
    int_value           = 0;
    double_value        = 0.0;
    valid_flags         = string_valid;
    format              = CdlValueFormat_Default;
    
    CYG_REPORT_RETURN();
}

CdlSimpleValue::CdlSimpleValue(cdl_int val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: int constructor");
    CYG_REPORT_FUNCARG1XV(this);

    value               = "0";
    int_value           = val;
    double_value        = 0.0;
    valid_flags         = int_valid;
    format              = CdlValueFormat_Default;
    
    CYG_REPORT_RETURN();
}

CdlSimpleValue::CdlSimpleValue(double val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: double constructor");
    CYG_REPORT_FUNCARG1XV(this);

    value               = "0";
    int_value           = 0;
    double_value        = val;
    valid_flags         = double_valid;
    format              = CdlValueFormat_Default;

    CYG_REPORT_RETURN();
}

CdlSimpleValue::CdlSimpleValue(bool val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: bool constructor");
    CYG_REPORT_FUNCARG2XV(this, val);

    value               = (val) ? "1" : "0";
    int_value           = (val) ? 1 : 0;
    double_value        = 0.0;
    valid_flags         = string_valid | int_valid;
    format              = CdlValueFormat_Default;
    
    CYG_REPORT_RETURN();
}

CdlSimpleValue::CdlSimpleValue(const CdlSimpleValue& original)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: copy constructor");
    CYG_REPORT_FUNCARG2XV(this, &original);

    value               = original.value;
    int_value           = original.int_value;
    double_value        = original.double_value;
    valid_flags         = original.valid_flags;
    format              = original.format;
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destructor                       

// ----------------------------------------------------------------------------

CdlSimpleValue::~CdlSimpleValue()
{
    CYG_REPORT_FUNCNAME("CdlsimpleValue:: destructor");
    CYG_REPORT_FUNCARG1XV(this);

    value               = "";
    int_value           = 0;
    double_value        = 0.0;
    valid_flags         = 0;
    format              = CdlValueFormat_Default;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Assignment operators             

// ----------------------------------------------------------------------------

CdlSimpleValue&
CdlSimpleValue::operator=(const CdlSimpleValue& original)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: assignment operator");
    CYG_REPORT_FUNCARG2XV(this, &original);

    if (this != &original) {
        value           = original.value;
        int_value       = original.int_value;
        double_value    = original.double_value;
        valid_flags     = original.valid_flags;
        format          = original.format;
    }
    
    CYG_REPORT_RETURN();
    return *this;
}

CdlSimpleValue&
CdlSimpleValue::operator=(std::string val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: string assignment");
    CYG_REPORT_FUNCARG1XV(this);

    value               = val;
    int_value           = 0;
    double_value        = 0.0;
    valid_flags         = string_valid;
    format              = CdlValueFormat_Default;

    CYG_REPORT_RETURN();
    return *this;
}

CdlSimpleValue&
CdlSimpleValue::operator=(cdl_int val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: integer assignment");
    CYG_REPORT_FUNCARG1XV(this);

    value               = "";
    int_value           = val;
    double_value        = 0.0;
    valid_flags         = int_valid;
    format              = CdlValueFormat_Default;
    
    CYG_REPORT_RETURN();
    return *this;
}

CdlSimpleValue&
CdlSimpleValue::operator=(double val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: double assignment");
    CYG_REPORT_FUNCARG1XV(this);

    value               = "";
    int_value           = 0;
    double_value        = val;
    valid_flags         = double_valid;
    format              = CdlValueFormat_Default;

    CYG_REPORT_RETURN();
    return *this;
}

// ----------------------------------------------------------------------------
// Converting a boolean into a simple value. This is sufficiently common
// to warrant its own member function, and in addition it avoids
// ambiguity when assigning 0.

CdlSimpleValue&
CdlSimpleValue::operator=(bool val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: bool assignment");
    CYG_REPORT_FUNCARG1XV(this);

    value               = (val) ? "1" : "0";
    int_value           = (val) ? 1 : 0;
    double_value        = 0.0;
    valid_flags         = string_valid | int_valid;
    format              = CdlValueFormat_Default;

    CYG_REPORT_RETURN();
    return *this;
}

//}}}
//{{{  CdlValuable -> CdlSimpleValue    

// ----------------------------------------------------------------------------
// This routine bridges the gap between the full data held in the CdlValuable
// object and the basic information needed for expression evaluation.

void
CdlSimpleValue::eval_valuable(CdlEvalContext& context, CdlValuable valuable, CdlSimpleValue& result)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue:: valuable assignment");
    CYG_REPORT_FUNCARG3XV(&context, valuable, &result);
    CYG_PRECONDITION_CLASSC(valuable);

    // If the valuable is not currently active then its value is
    // always zero for the purposes of expression evaluation.
    // FIXME: this check should be on a per-transaction basis.
    if (((0 != context.transaction) && !context.transaction->is_active(valuable)) ||
        ((0 == context.transaction) && !valuable->is_active())) {
        
        result.value           = "0";
        result.int_value       = 0;
        result.double_value    = 0.0;
        result.valid_flags     = string_valid | int_valid;
        result.format          = CdlValueFormat_Default;
        CYG_REPORT_RETURN();
        return;
    }

    // Get hold of the underlying CdlValue object
    const CdlValue& val = (0 != context.transaction) ?
        context.transaction->get_whole_value(valuable) : valuable->get_whole_value();
        
    // Otherwise the value depends on the flavor.
    switch(val.get_flavor()) {
      case CdlValueFlavor_None :
      {
        // This could be treated as an error, but since valuables with flavor
        // none are permanently enabled a constant "1" is a better result.
        result.value           = "1";
        result.int_value       = 1;
        result.double_value    = 0.0;
        result.valid_flags     = string_valid | int_valid;
        result.format          = CdlValueFormat_Default;
        break;
      }
      case CdlValueFlavor_Bool :
      {
        bool enabled           = val.is_enabled();
        result.value           = (enabled) ? "1" : "0";
        result.int_value       = (enabled) ?  1  :  0;
        result.double_value    = 0.0;
        result.valid_flags     = string_valid | int_valid;
        result.format          = CdlValueFormat_Default;
        break;
      }
      case CdlValueFlavor_BoolData :
      {
        if (!val.is_enabled()) {
                    
            result.value        = "0";
            result.int_value    = 0;
            result.double_value = 0.0;
            result.valid_flags  = string_valid | int_valid;
            result.format       = CdlValueFormat_Default;
                    
        } else {

            // Just use a copy constructor, let the compiler optimise things.
            result = val.get_simple_value();
        }
        break;
      }
      case CdlValueFlavor_Data :
      {
        // Just like BoolData, but with no need to check the enabled flag.
        result = val.get_simple_value();
        break;
      }
      default:
      {
        CYG_FAIL("Valuable object with an unknown flavor encountered.");
      }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Getting the value                

// ----------------------------------------------------------------------------
// Some of these calls involve conversion operators.

std::string
CdlSimpleValue::get_value() const
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::get_value");
    CYG_REPORT_FUNCARG1XV(this);

    if (!(valid_flags & string_valid)) {
        if (valid_flags & int_valid) {
            Cdl::integer_to_string(int_value, value, format);
        } else if (valid_flags & double_valid) {
            Cdl::double_to_string(double_value, value, format);
        } else {
            CYG_FAIL("Attempt to use uninitialized SimpleValue");
        }
        valid_flags |= string_valid;
    }

    CYG_REPORT_RETURN();
    return value;
}

bool
CdlSimpleValue::has_integer_value() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlSimpleValue::has_integer_value", "result %d");
    CYG_REPORT_FUNCARG1XV(this);

    if (!(valid_flags & (int_valid | int_invalid))) {
        if (valid_flags & double_valid) {
            if (Cdl::double_to_integer(double_value, int_value)) {
                valid_flags |= int_valid;
            } else {
                valid_flags |= int_invalid;
            }
        } else if (valid_flags & string_valid) {
            if (Cdl::string_to_integer(value, int_value)) {
                valid_flags |= int_valid;
            } else {
                valid_flags |= int_invalid;
            }
        } else {
            CYG_FAIL("Attempt to use uninitialized SimpleValue");
        }
    }
    
    bool result = (valid_flags & int_valid);
    CYG_REPORT_RETVAL(result);
    return result;
}

cdl_int
CdlSimpleValue::get_integer_value() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlsimpleValue::get_integer_value", "result %ld");
    CYG_REPORT_FUNCARG1XV(this);

    cdl_int result = 0;
    if ((valid_flags & int_valid) || has_integer_value()) {
        result = int_value;
    }

    CYG_REPORT_RETVAL((int) result);
    return result;
}

bool
CdlSimpleValue::has_double_value() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlSimpleValue::has_double_value", "result %d");
    CYG_REPORT_FUNCARG1XV(this);

    if (!(valid_flags & (double_valid | double_invalid))) {
        if (valid_flags & int_valid) {
            Cdl::integer_to_double(int_value, double_value);
            valid_flags |= double_valid;
        } else if (valid_flags & string_valid) {
            if (Cdl::string_to_double(value, double_value)) {
                valid_flags |= double_valid;
            } else {
                valid_flags |= double_invalid;
            }
        } else {
            CYG_FAIL("Attempt to use uninitialized SimpleValue");
        }
    }
    bool result = (valid_flags & double_valid);
    CYG_REPORT_RETVAL(result);
    return result;
}

double
CdlSimpleValue::get_double_value() const
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::get_double_value");
    CYG_REPORT_FUNCARG1XV(this);

    double result = 0.0;
    if ((valid_flags & double_valid) || has_double_value()) {
        result = double_value;
    }

    CYG_REPORT_RETURN();
    return result;
}

bool
CdlSimpleValue::get_bool_value() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlSimpleValue::get_bool_value", "result %d");
    CYG_REPORT_FUNCARG1XV(this);

    bool result = false;
    if (valid_flags & int_valid) {
        if (0 != int_value) {
            result = true;
        }
    } else if (valid_flags & double_valid) {
        // Leave it to the compiler to decide what is valid
        result = double_value;
    } else if (valid_flags & string_valid) {
        // string_to_bool copes with "1", "true", and a few other cases.
        // If the current value does not match any of these then
        // true corresponds to a non-empty string.
        if (!Cdl::string_to_bool(value, result)) {
            if ("" == value) {
                result = false;
            } else {
                result = true;
            }
        }
    } else {
        // No value defined, default to false.
        result = false;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Updating the value               

// ----------------------------------------------------------------------------
// Normally the assignment operators will be used for this instead.

void
CdlSimpleValue::set_value(std::string val, CdlValueFormat new_format)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::set_value (string)");
    CYG_REPORT_FUNCARG1XV(this);

    value               = val;
    int_value           = 0;
    double_value        = 0.0;
    valid_flags         = string_valid;
    format              = new_format;
}


void
CdlSimpleValue::set_integer_value(cdl_int val, CdlValueFormat new_format)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::set_integer_value");
    CYG_REPORT_FUNCARG2XV(this, (int) val);

    value               = "";
    int_value           = val;
    double_value        = 0.0;
    valid_flags         = int_valid;
    format              = new_format;

    CYG_REPORT_RETURN();
}


void
CdlSimpleValue::set_double_value(double val, CdlValueFormat new_format)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::set_double_value");
    CYG_REPORT_FUNCARG1XV(this);

    value               = "";
    int_value           = 0;
    double_value        = val;
    valid_flags         = double_valid;
    format              = new_format;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Value format support             

// ----------------------------------------------------------------------------

CdlValueFormat
CdlSimpleValue::get_value_format() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlSimpleValue::get_value_format", "result %d");
    CYG_REPORT_FUNCARG1XV(this);

    CdlValueFormat result = format;
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlSimpleValue::set_value_format(CdlValueFormat new_format)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::set_value_format");
    CYG_REPORT_FUNCARG2XV(this, new_format);

    format      = new_format;
    
    CYG_REPORT_RETURN();
}

void
CdlSimpleValue::set_value_format(CdlSimpleValue& other_val)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::set_value_format (simple val)");
    CYG_REPORT_FUNCARG2XV(this, &other_val);

    format = other_val.format;

    CYG_REPORT_RETURN();
}

// This gets used for binary operators, e.g. A + B
// If A has a non-default format then that gets used.
// Otherwise B's format gets used, which may or may not be default.
//
// e.g. 0x1000 + 4 -> 0x1004
//      10 + 0x100 -> 0x10A
//      10 + 32    -> 42

void
CdlSimpleValue::set_value_format(CdlSimpleValue& val1, CdlSimpleValue& val2)
{
    CYG_REPORT_FUNCNAME("CdlSimpleValue::set_value_format");
    CYG_REPORT_FUNCARG3XV(this, &val1, &val2);

    format = (CdlValueFormat_Default != val1.format) ? val1.format : val2.format;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Comparison operators             

// ----------------------------------------------------------------------------

bool
CdlSimpleValue::operator==(const CdlSimpleValue& other) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlSimpleValue:: operator==", "result %d");
    CYG_REPORT_FUNCARG2XV(this, &other);

    bool result = false;
    
    if (has_integer_value()) {
        if (other.has_integer_value()) {
            cdl_int val1 = get_integer_value();
            cdl_int val2 = other.get_integer_value();
            result = (val1 == val2);
        }
    } else if (has_double_value()) {
        if (other.has_double_value()) {
            double val1 = get_double_value();
            double val2 = other.get_double_value();
            result = (val1 == val2);
        }
    } else {
        std::string val1 = get_value();
        std::string val2 = other.get_value();
        result = (val1 == val2);
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlSimpleValue::operator!=(const CdlSimpleValue& other) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlSimpleValue:: operator!=", "result %d");
    CYG_REPORT_FUNCARG2XV(this, &other);

    bool result = true;
    if (has_integer_value()) {
        if (other.has_integer_value()) {
            cdl_int val1 = get_integer_value();
            cdl_int val2 = other.get_integer_value();
            result = (val1 != val2);
        }
    } else if (has_double_value()) {
        if (other.has_double_value()) {
            double val1 = get_double_value();
            double val2 = other.get_double_value();
            result = (val1 != val2);
        }
    } else {
        std::string val1 = get_value();
        std::string val2 = other.get_value();
        result = (val1 != val2);
    }


    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}

//}}}
//{{{  CdlValue class                   

// ----------------------------------------------------------------------------
// This should really be a class static constant, but VC++ does not implement
// that part of the language. A constant here avoids the need for lots of
// occurrences of 4 throughout the value-related routines.

static const int CdlValue_number_of_sources = 4;

//{{{  Constructors                             

// ----------------------------------------------------------------------------
// The default flavor depends on the type of entity being created. For
// example CDL options are boolean by default, but packages are booldata.
// The intelligence to do the right thing lives in set_flavor().

CdlValue::CdlValue(CdlValueFlavor flavor_arg)
{
    CYG_REPORT_FUNCNAME("CdlValue:: constructor");
    CYG_REPORT_FUNCARG1XV(this);

    current_source = CdlValueSource_Default;
    source_valid[CdlValueSource_Default]        = true;
    source_valid[CdlValueSource_Inferred]       = false;
    source_valid[CdlValueSource_Wizard]         = false;
    source_valid[CdlValueSource_User]           = false;
    enabled[CdlValueSource_Default]             = false;
    enabled[CdlValueSource_Inferred]            = false;
    enabled[CdlValueSource_Wizard]              = false;
    enabled[CdlValueSource_User]                = false;

    // The SimpleValues will initialize themselves.
    
    cdlvalue_cookie         = CdlValue_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    // This cannot happen until after the object is valid.
    set_flavor(flavor_arg);
        
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Copy constructor. This is not really required, a default
// member-wise copy would be fine and more efficient, but it would
// lose tracing and assertion.

CdlValue::CdlValue(const CdlValue& original)
{
    CYG_REPORT_FUNCNAME("CdlValue:: copy constructor");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_INVARIANT_CLASSOC(CdlValue, original);

    flavor              = original.flavor;
    current_source      = original.current_source;
    for (int i = 0; i < CdlValue_number_of_sources; i++) {
        source_valid[i] = original.source_valid[i];
        enabled[i]      = original.enabled[i];
        values[i]       = original.values[i];
    }

    cdlvalue_cookie = CdlValue_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Assignment operator. Again this is not required, the default would be
// fine and more efficient, but tracing and assertions are good things.

CdlValue& CdlValue::operator=(const CdlValue& original)
{
    CYG_REPORT_FUNCNAME("CdlValue:: assignment operator");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_INVARIANT_CLASSOC(CdlValue, original);

    if (this != &original) {
        flavor          = original.flavor;
        current_source  = original.current_source;
        for (int i = 0; i < CdlValue_number_of_sources; i++) {
            source_valid[i]     = original.source_valid[i];
            enabled[i]          = original.enabled[i];
            values[i]           = original.values[i];
        }
    }

    cdlvalue_cookie = CdlValue_Magic;
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
    return *this;
}

//}}}
//{{{  Destructor                               

// ----------------------------------------------------------------------------

CdlValue::~CdlValue()
{
    CYG_REPORT_FUNCNAME("CdlValue:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlvalue_cookie     = CdlValue_Invalid;
    flavor              = CdlValueFlavor_Invalid;
    current_source      = CdlValueSource_Invalid;
    for (int i = 0; i < CdlValue_number_of_sources; i++) {
        source_valid[i]         = false;
        enabled[i]              = false;
        // The CdlSimpleValue array will take care of itself.
    }
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

//}}}
//{{{  check_this()                             

// ----------------------------------------------------------------------------
bool
CdlValue::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlValue_Magic != cdlvalue_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    if (!source_valid[CdlValueSource_Default]) {
        return false;
    }

    if ((CdlValueFlavor_None == flavor) || (CdlValueFlavor_Data == flavor)) {
        for (int i = 0; i < CdlValue_number_of_sources; i++) {
            if (!enabled[i]) {
                return false;
            }
        }
    }
    for (int i = 0; i < CdlValue_number_of_sources; i++) {
        if (source_valid[i]) {
            if (!values[i].check_this(zeal)) {
                return false;
            }
        }
    }
    
    return true;
}

//}}}
//{{{  Flavor manipulation                      

// ----------------------------------------------------------------------------
// Get hold of the current flavor.
CdlValueFlavor
CdlValue::get_flavor(void) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValue::get_flavor", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlValueFlavor result = flavor;
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// set_flavor() may be invoked once or twice for a given entity. The first
// time is from inside the constructor with the default flavor for this
// particular class of entity. It may then be called again if the
// entity has a "flavor" property that overrides this. All old data
// will be lost, so evaluating a default value etc. should be done after
// the call to set_flavor(), and there should be no subsequent calls to
// set_flavor().

void
CdlValue::set_flavor(CdlValueFlavor flavor_arg)
{
    CYG_REPORT_FUNCNAME("CdlValue:: set_flavor");
    CYG_REPORT_FUNCARG2XV(this, flavor_arg);
    
    // No precondition here, set_flavor() is called from inside the constructor
    CYG_PRECONDITIONC((CdlValueFlavor_None     == flavor_arg) || \
                      (CdlValueFlavor_Bool     == flavor_arg) || \
                      (CdlValueFlavor_BoolData == flavor_arg) || \
                      (CdlValueFlavor_Data     == flavor_arg));

    flavor = flavor_arg;
    switch(flavor) {
      case CdlValueFlavor_None :
        {
            // All value sources are enabled, but "default" remains
            // the only valid one. All data parts are set to "1",
            // although that should not really matter.
            enabled[CdlValueSource_Default]     = true;
            enabled[CdlValueSource_Inferred]    = true;
            enabled[CdlValueSource_Wizard]      = true;
            enabled[CdlValueSource_User]        = true;
            
            CdlSimpleValue simple_val((cdl_int) 1);
            values[CdlValueSource_Default]      = simple_val;
            values[CdlValueSource_Inferred]     = simple_val;
            values[CdlValueSource_Wizard]       = simple_val;
            values[CdlValueSource_User]         = simple_val;
            break;
        }
          
      case CdlValueFlavor_Bool :
        {
            // All value sources start out as disabled, but with a
            // constant data part of 1. Users can only control the
            // boolean part. This is consistent with header file
            // generation: no #define is generated for disabled
            // options, but if the option is enabled then the data
            // part will be used for the value.
            enabled[CdlValueSource_Default]     = false;
            enabled[CdlValueSource_Inferred]    = false;
            enabled[CdlValueSource_Wizard]      = false;
            enabled[CdlValueSource_User]        = false;

            // BLV - keep the data part at 0 for now. There is too
            // much confusion in the code between value as a string
            // representation, and value as the data part of the
            // bool/data pair. This needs to be fixed, but it requires
            // significant API changes.
#if 0            
            CdlSimpleValue simple_val(cdl_int(1));
#else
            CdlSimpleValue simple_val(cdl_int(0));
#endif            
            values[CdlValueSource_Default]      = simple_val;
            values[CdlValueSource_Inferred]     = simple_val;
            values[CdlValueSource_Wizard]       = simple_val;
            values[CdlValueSource_User]         = simple_val;
            break;
        }
          
      case CdlValueFlavor_BoolData :
        {
            // All value sources start out as disabled, just like
            // booleans. Nothing is known about the data part.
            enabled[CdlValueSource_Default]       = false;
            enabled[CdlValueSource_Inferred]      = false;
            enabled[CdlValueSource_Wizard]        = false;
            enabled[CdlValueSource_User]          = false;
            break;
        }
          
      case CdlValueFlavor_Data :
        {
            // All value sources start out as enabled, and cannot be
            // changed. Nothing is known about the data part.
            enabled[CdlValueSource_Default]       = true;
            enabled[CdlValueSource_Inferred]      = true;
            enabled[CdlValueSource_Wizard]        = true;
            enabled[CdlValueSource_User]          = true;
            break;
        }

      default :
        break;
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Source manipulation                      

// ----------------------------------------------------------------------------

void
CdlValue::set_source(CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValue::set_source");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_INVARIANT_THISC(CdlValue);
    CYG_PRECONDITIONC((0 <= source) && (source <= CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    current_source = source;

    CYG_REPORT_RETURN();
}

CdlValueSource
CdlValue::get_source(void) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValue::get_source", "source %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlValueSource result = current_source;
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValue::has_source(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValue::has_source", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));

    bool result = source_valid[source];
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Invalidate a specific source. If that source happens to be the current one,
// switch to the highest-priority valid source.

void
CdlValue::invalidate_source(CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValue::invalidate_source");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(CdlValueSource_Default != source);
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));

    if (CdlValueSource_Default != source) {
        source_valid[source]        = false;
        if (current_source == source) {
            if (source_valid[CdlValueSource_User]) {
                current_source = CdlValueSource_User;
            } else if (source_valid[CdlValueSource_Wizard]) {
                current_source = CdlValueSource_Wizard;
            } else if (source_valid[CdlValueSource_Inferred]) {
                current_source = CdlValueSource_Inferred;
            } else {
                current_source = CdlValueSource_Default;
            }
        }
    }
    
    CYG_POSTCONDITIONC(source_valid[current_source]);
}

//}}}
//{{{  Retrieving the data                      

// ----------------------------------------------------------------------------
// Check the enabled flag for the appropriate source. The specified source
// is normally provided by a default argument CdlValueSource_Current, which
// 99.9...% of the time is what we are after.
//
// Note that this member can be used even for entities of flavor none
// and data, and the result will be true. However it is not legal to
// disable such entities.

bool
CdlValue::is_enabled(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValue::is_enabled", "enabled %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    if (CdlValueSource_Current == source) {
        source = current_source;
    }
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    bool result = enabled[source];
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Access to the value field.

std::string
CdlValue::get_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValue::get_value");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    if (CdlValueSource_Current == source) {
        source = current_source;
    }
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    std::string result = values[source].get_value();
    CYG_REPORT_RETURN();
    return result;
}

bool
CdlValue::has_integer_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValue::has_integer_value", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_INVARIANT_THISC(CdlValue);

    if (CdlValueSource_Current == source) {
        source = current_source;
    }
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    bool result = values[source].has_integer_value();
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValue::has_double_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValue::has_value", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_INVARIANT_THISC(CdlValue);
    
    if (CdlValueSource_Current == source) {
        source = current_source;
    }
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    bool result = values[source].has_double_value();
    CYG_REPORT_RETVAL(result);
    return result;
}

cdl_int
CdlValue::get_integer_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValue::get_integer_value", "value %ld");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    if (CdlValueSource_Current == source) {
        source = current_source;
    }
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    cdl_int result = values[source].get_integer_value();
    CYG_REPORT_RETVAL(result);
    return result;
}

double
CdlValue::get_double_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValue::get_double_value");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    if (CdlValueSource_Current == source) {
        source = current_source;
    }
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    double result = values[source].get_double_value();
    CYG_REPORT_RETURN();
    return result;
}

CdlSimpleValue
CdlValue::get_simple_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValue::get_simple_value");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    if (CdlValueSource_Current == source) {
        source = current_source;
    }
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    CYG_PRECONDITIONC(source_valid[source]);

    CYG_REPORT_RETURN();
    return values[source];
}

//}}}
//{{{  Value modification                       

// ----------------------------------------------------------------------------

void
CdlValue::set_enabled(bool val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValue::set_enabled");
    CYG_REPORT_FUNCARG3XV(this, val, source);
    CYG_INVARIANT_THISC(CdlValue);
    CYG_PRECONDITIONC((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor));
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));

    enabled[source] = val;
    source_valid[source] = true;
    if (source > current_source) {
        current_source = source;
    }
    
    CYG_REPORT_RETURN();
}

void
CdlValue::set_value(CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValue::set_value");
    CYG_REPORT_FUNCARG3XV(this, &val, source);
    CYG_INVARIANT_THISC(CdlValue);
    CYG_PRECONDITIONC((CdlValueFlavor_BoolData == flavor) || (CdlValueFlavor_Data == flavor));
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));
    
    values[source] = val;
    source_valid[source] = true;
    if (source > current_source) {
        current_source = source;
    }
    
    CYG_REPORT_RETURN();
}

void
CdlValue::set_enabled_and_value(bool enabled_arg, CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValue::set_enabled_and_value");
    CYG_REPORT_FUNCARG4XV(this, enabled_arg, &val, source);
    CYG_INVARIANT_THISC(CdlValue);
    CYG_PRECONDITIONC(CdlValueFlavor_BoolData == flavor);
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));

    enabled[source]      = enabled_arg;
    values[source]       = val;
    source_valid[source] = true;
    if (source > current_source) {
        current_source = source;
    }
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Given a SimpleValue, this member function does the right thing
// for the flavor.

void
CdlValue::set(CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValue::set");
    CYG_REPORT_FUNCARG3XV(this, &val, source);
    CYG_INVARIANT_THISC(CdlValue);
    CYG_ASSERTC((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor) || (CdlValueFlavor_Data == flavor));
    CYG_PRECONDITIONC((0 <= source) && (source < CdlValue_number_of_sources));

    switch(flavor) {
      case CdlValueFlavor_Bool:
        enabled[source] = val.get_bool_value();
        break;

      case CdlValueFlavor_BoolData:
        if (!val.get_bool_value()) {
            enabled[source] = false;
            values[source]  = (cdl_int) 0;
        } else {
            enabled[source] = true;
            values[source]  = val;
        }
        break;
                    
      case CdlValueFlavor_Data:
        values[source] = val;
        break;
                    
      default:
        CYG_FAIL("Unknown value flavor detected.");
    }
    
    source_valid[source] = true;
    if (source > current_source) {
        current_source = source;
    }

    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  CdlListValue class               

// ----------------------------------------------------------------------------
// List values. Most of this is straightforward.

CdlListValue::CdlListValue()
{
    CYG_REPORT_FUNCNAME("CdlListValue:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    // The only data fields are embedded objects which will have been
    // filled in already.
    cdllistvalue_cookie = CdlListValue_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlListValue::CdlListValue(const CdlListValue& original)
{
    CYG_REPORT_FUNCNAME("CdlListValue:: copy constructor");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_INVARIANT_CLASSOC(CdlListValue, original);
    
    // This may get expensive, but should not happen very often.
    table               = original.table;
    integer_ranges      = original.integer_ranges;
    double_ranges       = original.double_ranges;
    cdllistvalue_cookie = CdlListValue_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlListValue & CdlListValue::operator=(const CdlListValue& original)
{
    CYG_REPORT_FUNCNAME("CdlListValue:: assignment operator");
    CYG_REPORT_FUNCARG2XV(this, &original);
    CYG_INVARIANT_CLASSOC(CdlListValue, original);

    if (this != &original) {
        table.clear();
        integer_ranges.clear();
        double_ranges.clear();
        table          = original.table;
        integer_ranges = original.integer_ranges;
        double_ranges  = original.double_ranges;
    }
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
    return *this;
}

CdlListValue::~CdlListValue()
{
    CYG_REPORT_FUNCNAME("CdlListValue:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdllistvalue_cookie = CdlListValue_Invalid;
    table.clear();
    integer_ranges.clear();
    double_ranges.clear();
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Finding out about the current legal values. These routines can be
// used by GUI-related code to figure out a sensible widget to be used
// for a CDL entity. In nearly all cases life will be simple: either
// there will be a fixed set of legal values and the user merely has
// to choose one of these; or there will be a simple numerical range.
// Occasionally life may be more complicated, if the full generality
// of CDL list expressions is being used, and it will be necessary to
// use an entry box instead. Note that the entity's flavor may also
// affect the user interface.

const std::vector<CdlSimpleValue>&
CdlListValue::get_table(void) const
{
    CYG_REPORT_FUNCNAME("CdlListValue::get_table");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return table;
}

const std::vector<std::pair<cdl_int, cdl_int> >&
CdlListValue::get_integer_ranges(void) const
{
    CYG_REPORT_FUNCNAME("CdlListValue::get_integer_ranges");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return integer_ranges;
}

const std::vector<std::pair<double, double> >&
CdlListValue::get_double_ranges(void) const
{
    CYG_REPORT_FUNCNAME("CdlListValue::get_double_ranges");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return double_ranges;
}

// ----------------------------------------------------------------------------
// Membership. This can be quite complicated.
//
// 1) anything which has an integer representation must be checked against
//    the integer ranges and the vector of integer constants. It must
//    also be checked against the floating point ranges, since calculations
//    may have resulted in the fractional part disappearing, assuming that
//    the integer has a floating point representation.
//
// 2) similarly anything which has a floating point representation must
//    be checked against the floating point ranges and constant vector.
//    In addition it may have an empty fractional part in which case
//    integer comparisons have to be attempted as well.
//
// 3) string data needs to be tested first of all for integer and double
//    representations. If these fail then the comparison should be against
//    the string vector.
//
// For floating point data exact comparisons are of course meaningless,
// and arguably the vector of floating point constants is useless. The
// ranges vector is better, but still not ideal. It may be necessary
// to introduce an epsilon fudge factor.

bool
CdlListValue::is_member(CdlSimpleValue& val) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlListValue::is_member (CdlSimpleValue)", "result %d");
    CYG_REPORT_FUNCARG2XV(this, &val);
    CYG_PRECONDITION_THISC();

    bool result = false;
    if (val.has_integer_value()) {
        result = is_member(val.get_integer_value(), false);
    }
    if (!result && val.has_double_value()) {
        result = is_member(val.get_double_value(), false);
    }
    if (!result) {
        result = is_member(val.get_value());
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlListValue::is_member(std::string val, bool allow_conversions) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlListValue::is_member (string)", "result %d");
    CYG_REPORT_FUNCARG3XV(this, &val, allow_conversions);
    CYG_PRECONDITION_THISC();

    bool        result = false;
    if (allow_conversions) {
        cdl_int     integer_value;
        double      double_value;

        if (Cdl::string_to_integer(val, integer_value)) {
            result = is_member(integer_value, false);
        }
        if (!result && Cdl::string_to_double(val, double_value)) {
            result = is_member(double_value, false);
        }
    }
    if (!result) {
        for (std::vector<CdlSimpleValue>::const_iterator val_i = table.begin(); val_i != table.end(); val_i++) {
            if (val_i->get_value() == val) {
                result = true;
                break;
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlListValue::is_member(cdl_int val, bool allow_conversions) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlListValue::is_member (int)", "result %d");
    CYG_REPORT_FUNCARG3XV(this, &val, allow_conversions);
    CYG_PRECONDITION_THISC();

    bool result = false;
    for (std::vector<CdlSimpleValue>::const_iterator val_i = table.begin(); val_i != table.end(); val_i++) {
        if (val_i->has_integer_value() && (val_i->get_integer_value() == val)) {
            result = true;
            break;
        }
    }
    if (!result) {
        for (std::vector<std::pair<cdl_int,cdl_int> >::const_iterator i = integer_ranges.begin();
             i != integer_ranges.end(); i++) {
            if ((val >= i->first) && (val <= i->second)) {
                result = true;
                break;
            }
        }
    }
    if (!result && allow_conversions) {
        double double_value = Cdl::integer_to_double(val);
        result = is_member(double_value, false);
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlListValue::is_member(double val, bool allow_conversions) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlListValue::is_member (double)", "result %d");
    CYG_REPORT_FUNCARG3XV(this, &val, allow_conversions);
    CYG_PRECONDITION_THISC();

    bool result = false;
    for (std::vector<CdlSimpleValue>::const_iterator val_i = table.begin(); val_i != table.end(); val_i++) {
        if (val_i->has_double_value() && (val_i->get_double_value() == val)) {
            result = true;
            break;
        }
    }
    if (!result) {
        for (std::vector<std::pair<double,double> >::const_iterator i = double_ranges.begin();
             i != double_ranges.end(); i++) {
            if ((val >= i->first) && (val <= i->second)) {
                result = true;
                break;
            }
        }
    }
    if (!result && allow_conversions) {
        cdl_int integer_value;
        if (Cdl::double_to_integer(val, integer_value)) {
            result = is_member(integer_value, false);
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

bool
CdlListValue::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlListValue_Magic != cdllistvalue_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    // After construction the various vectors will still be empty, they
    // do not get filled in until a list expression is evaluated. No
    // further tests are possible here.
    return true;
}

//}}}

//{{{  dialog property                  

// ----------------------------------------------------------------------------
// Syntax: dialog <reference>

void
CdlValuableBody::dialog_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                       CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlValuable::dialog_update_handler");
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(source);
    CYG_PRECONDITION_CLASSC(prop);
    
    // The main update of interest is Loaded (iff dest != 0), and
    // Created. These updates indicate that the destination now exists,
    // so it is possible to check that the destination is a dialog.
    if (((CdlUpdate_Loaded == change) && (0 != dest)) ||
        (CdlUpdate_Created == change)) {

        CYG_ASSERT_CLASSC(dest);
        CdlDialog dialog = dynamic_cast<CdlDialog>(dest);
        if (0 == dialog) {
            std::string msg = dest->get_class_name() + " " + dest->get_name() +
                " cannot be used in a dialog property, it is not a custom dialog.";
            CdlConflict_DataBody::make(transaction, source, prop, msg);
        }
        
    } else if (CdlUpdate_Destroyed == change) {
        // If there was a data conflict object, it is no longer relevant
        transaction->clear_structural_conflicts(source, prop, &CdlConflict_DataBody::test);
    }

    CYG_REPORT_RETURN();
}

int
CdlValuableBody::parse_dialog(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_dialog", "result %d");

    int result = CdlParse::parse_reference_property(interp, argc, argv, CdlPropertyId_Dialog, 0, 0, false, &dialog_update_handler);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_dialog() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_dialog", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // It is not enough to have the property, the dialog reference must also be
    // resolved and go to a dialog.
    bool        result          = false;
    CdlProperty property        = get_property(CdlPropertyId_Dialog);
    if (0 != property) {
        CdlProperty_Reference ref_prop = dynamic_cast<CdlProperty_Reference>(property);
        CYG_ASSERTC(0 != ref_prop);

        CdlNode destination = ref_prop->get_destination();
        if (0 != destination) {
            CdlDialog dialog = dynamic_cast<CdlDialog>(destination);
            if (0 != dialog) {
                result = true;
            }
        }
    }
    CYG_REPORT_RETVAL(result);
    return result;
}


CdlDialog
CdlValuableBody::get_dialog() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_dialog", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlDialog   result          = 0;
    CdlProperty property        = get_property(CdlPropertyId_Dialog);
    if (0 != property) {
        CdlProperty_Reference ref_prop = dynamic_cast<CdlProperty_Reference>(property);
        CYG_ASSERTC(0 != ref_prop);

        CdlNode destination = ref_prop->get_destination();
        if (0 != destination) {
            result = dynamic_cast<CdlDialog>(destination);
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  wizard property                  

// ----------------------------------------------------------------------------
// Syntax: wizard <reference>

void
CdlValuableBody::wizard_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                       CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlValuable::wizard_update_handler");
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(source);
    CYG_PRECONDITION_CLASSC(prop);
    
    // The main update of interest is Loaded (iff dest != 0), and
    // Created. These updates indicate that the destination now exists,
    // so it is possible to check that the destination is a dialog.
    if (((CdlUpdate_Loaded == change) && (0 != dest)) ||
        (CdlUpdate_Created == change)) {

        CYG_ASSERT_CLASSC(dest);
        CdlWizard wizard = dynamic_cast<CdlWizard>(dest);
        if (0 == wizard) {
            std::string msg = dest->get_class_name() + " " + dest->get_name() +
                " cannot be used in a wizard property, it is not a wizard.";
            CdlConflict_DataBody::make(transaction, source, prop, msg);
        }
        
    } else if (CdlUpdate_Destroyed == change) {
        // If there was a data conflict object, it is no longer relevant
        transaction->clear_structural_conflicts(source, prop, &CdlConflict_DataBody::test);
    }

    CYG_REPORT_RETURN();
}

int
CdlValuableBody::parse_wizard(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_wizard", "result %d");

    int result = CdlParse::parse_reference_property(interp, argc, argv, CdlPropertyId_Wizard, 0, 0, false, &wizard_update_handler);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_wizard() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_wizard", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // It is not enough to have the property, the wizard reference
    // must also be resolved to a wizard object.
    bool        result          = false;
    CdlProperty property        = get_property(CdlPropertyId_Wizard);
    if (0 != property) {
        CdlProperty_Reference ref_prop = dynamic_cast<CdlProperty_Reference>(property);
        CYG_ASSERTC(0 != ref_prop);

        CdlNode destination = ref_prop->get_destination();
        if (0 != destination) {
            CdlWizard wizard = dynamic_cast<CdlWizard>(destination);
            CYG_ASSERTC(0 != wizard);
            CYG_UNUSED_PARAM(CdlWizard, wizard);
            result = true;
        }
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlWizard
CdlValuableBody::get_wizard() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_wizard", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlWizard   result          = 0;
    CdlProperty property        = get_property(CdlPropertyId_Wizard);
    if (0 != property) {
        CdlProperty_Reference ref_prop = dynamic_cast<CdlProperty_Reference>(property);
        CYG_ASSERTC(0 != ref_prop);

        CdlNode destination = ref_prop->get_destination();
        if (0 != destination) {
            result = dynamic_cast<CdlWizard>(destination);
            CYG_ASSERTC(0 != result);
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  legal_values property            

// ----------------------------------------------------------------------------
// Syntax: legal_values <list expression>

void
CdlValuableBody::legal_values_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                             CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("legal_values_update_handler");

    // Loaded and Unloading are of no immediate interest, reference
    // updating happens in the calling code.
    //
    // Any other change can affect the list expression and hence
    // invalidate the current value.
    if ((CdlUpdate_Loaded == change) || (CdlUpdate_Unloading == change)) {
        CYG_REPORT_RETURN();
        return;
    }

    CdlValuable valuable = dynamic_cast<CdlValuable>(source);
    CdlProperty_ListExpression lexpr = dynamic_cast<CdlProperty_ListExpression>(prop);
    CYG_ASSERT_CLASSC(valuable);
    CYG_ASSERT_CLASSC(lexpr);

    valuable->check_value(transaction);

    CYG_UNUSED_PARAM(CdlNode, dest);
    CYG_UNUSED_PARAM(CdlProperty_ListExpression, lexpr);
    CYG_REPORT_RETURN();
}

int
CdlValuableBody::parse_legal_values(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_legal_values", "result %d");

    int result = CdlParse::parse_listexpression_property(interp, argc, argv, CdlPropertyId_LegalValues, 0, 0,
                                                         &legal_values_update_handler);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_legal_values() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_legal_values", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_LegalValues);
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlProperty_ListExpression
CdlValuableBody::get_legal_values() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_legal_values", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty_ListExpression result = 0;
    CdlProperty       property          = get_property(CdlPropertyId_LegalValues);
    if (0 != property) {
        result = dynamic_cast<CdlProperty_ListExpression>(property);
        CYG_ASSERTC(0 != result);
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  default_value property           

// ----------------------------------------------------------------------------
// syntax: default_value <expr>

void
CdlValuableBody::default_value_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                              CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlValuable::default_value_update_handler");
    CYG_REPORT_FUNCARG5XV(transaction, source, prop, dest, change);

    // Loaded and unloading should be ignored.
    if ((CdlUpdate_Loaded == change) || (CdlUpdate_Unloading == change)) {
        CYG_REPORT_RETURN();
        return;
    }

    // Init, Created, Destroyed, ValueChange and ActiveChange should
    // all result in the expression being re-evaluated and the result
    // applied.
    CdlValuable valuable = dynamic_cast<CdlValuable>(source);
    CYG_ASSERTC(0 != valuable);
    CdlProperty_Expression expr = dynamic_cast<CdlProperty_Expression>(prop);
    CYG_ASSERTC(0 != expr);
    
    CdlSimpleValue val;

    try {
        
        CdlEvalContext context(transaction, source, prop);
        expr->eval(context, val);

        valuable->set(transaction, val, CdlValueSource_Default);

    } catch(CdlEvalException e) {

        
        // An EvalException conflict will have been created, so the
        // user knows that this default_value is not kosher. It is
        // still a good idea to make sure that the object retains a
        // sensible value.
        val = (cdl_int) 0;
        valuable->set(transaction, val, CdlValueSource_Default);
    }

    CYG_UNUSED_PARAM(CdlNode, dest);
    CYG_REPORT_RETURN();
}

int
CdlValuableBody::parse_default_value(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_default_value", "result %d");
    int result = CdlParse::parse_expression_property(interp, argc, argv, CdlPropertyId_DefaultValue, 0, 0,
                                                     &default_value_update_handler);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_default_value_expression() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_default_value_expression", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_DefaultValue);
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlProperty_Expression
CdlValuableBody::get_default_value_expression() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_default_value_expression", "result %");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty_Expression result = 0;
    CdlProperty property          = get_property(CdlPropertyId_DefaultValue);
    if (0 != property) {
        result = dynamic_cast<CdlProperty_Expression>(property);
        CYG_ASSERTC(0 != result);
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  calculated_property              

// ----------------------------------------------------------------------------
// Syntax: calculated <expression>

void
CdlValuableBody::calculated_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                           CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlValuable::default_value_update_handler");
    CYG_REPORT_FUNCARG5XV(transaction, source, prop, dest, change);

    // Loaded and unloading should be ignored.
    if ((CdlUpdate_Loaded == change) || (CdlUpdate_Unloading == change)) {
        CYG_REPORT_RETURN();
        return;
    }

    // Init, Created, Destroyed, ValueChange and ActiveChange should
    // all result in the expression being re-evaluated and the result
    // applied.
    CdlValuable valuable = dynamic_cast<CdlValuable>(source);
    CYG_ASSERTC(0 != valuable);
    CdlProperty_Expression expr = dynamic_cast<CdlProperty_Expression>(prop);
    CYG_ASSERTC(0 != expr);
    
    CdlSimpleValue val;

    try {
        
        CdlEvalContext context(transaction, source, prop);
        expr->eval(context, val);

        valuable->set(transaction, val, CdlValueSource_Default);

    } catch(CdlEvalException e) {

        
        // An EvalException conflict will have been created, so the
        // user knows that this default_value is not kosher. It is
        // still a good idea to make sure that the object retains a
        // sensible value.
        val = (cdl_int) 0;
        valuable->set(transaction, val, CdlValueSource_Default);
    }

    CYG_UNUSED_PARAM(CdlNode, dest);
    CYG_REPORT_RETURN();
}

// FIXME: check for flavor none?
int
CdlValuableBody::parse_calculated(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_calculated", "result %d");

    int result = CdlParse::parse_expression_property(interp, argc, argv, CdlPropertyId_Calculated, 0, 0,
                                                     &calculated_update_handler);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_calculated_expression() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_calculated_expression", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_Calculated);
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlProperty_Expression
CdlValuableBody::get_calculated_expression() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_calculated_expression", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty_Expression result   = 0;
    CdlProperty            property = get_property(CdlPropertyId_Calculated);
    if (0 != property) {
        result = dynamic_cast<CdlProperty_Expression>(property);
        CYG_ASSERTC(0 != result);
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  active_if property               

// ----------------------------------------------------------------------------
// Syntax:
//    active_if <goal expression>

void
CdlValuableBody::active_if_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                      CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlValuable::active_if_update_handler");
    CYG_REPORT_FUNCARG5XV(transaction, source, prop, dest, change);
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_PRECONDITION_CLASSC(source);
    CYG_PRECONDITION_CLASSC(prop);

    // Loaded should be ignored here, the world is still getting sorted out.
    // Unloading is of no interest, the source is disappearing anyway.
    if ((CdlUpdate_Loaded == change) || (CdlUpdate_Unloading == change)) {
        CYG_REPORT_RETURN();
        return;
    }

    // Any other change warrants re-evaluating the active status of the source.
    // This can be achieved via a test_active() call, although that may do
    // more work than is strictly necessary e.g. it may re-evaluate other
    // is_active properties. In practice it is unlikely that there will
    // be enough other constraints to warrant more efficient processing.
    bool old_state = transaction->is_active(source);
    bool new_state = source->test_active(transaction);
    if (old_state != new_state) {
        transaction->set_active(source, new_state);
    }
    
    CYG_UNUSED_PARAM(CdlNode, dest);
    CYG_REPORT_RETURN();
}

int
CdlValuableBody::parse_active_if(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_active_if", "result %d");

    int result = CdlParse::parse_goalexpression_property(interp, argc, argv, CdlPropertyId_ActiveIf, 0, 0,
                                                         &active_if_update_handler);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_active_if_conditions() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_active_if_conditions", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_ActiveIf);
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlValuableBody::get_active_if_conditions(std::vector<CdlProperty_GoalExpression>& result) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_active_if_conditions");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::vector<CdlProperty> properties;
    get_properties(CdlPropertyId_ActiveIf, properties);
    std::vector<CdlProperty>::const_iterator i;
    for (i = properties.begin(); i != properties.end(); i++) {
        CdlProperty_GoalExpression goal = dynamic_cast<CdlProperty_GoalExpression>(*i);
        CYG_ASSERTC(0 != goal);
        result.push_back(goal);
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  requires property                

// ----------------------------------------------------------------------------
// Syntax: requires <goal expression>

void
CdlValuableBody::requires_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                         CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlValuable::requires_update_handler");
    CYG_REPORT_FUNCARG5XV(transaction, source, prop, dest, change);
    CYG_PRECONDITION_CLASSC(transaction);

    // Loaded and Unloading are not of interest.
    if ((CdlUpdate_Loaded == change) || (CdlUpdate_Unloading == change)) {
        CYG_REPORT_RETURN();
        return;
    }
    
    // Any other change should cause normal handling. This happens in
    // a separate function because "requires" properties also need to
    // be checked when e.g. the source becomes inactive.
    CdlValuable valuable = dynamic_cast<CdlValuable>(source);
    CdlProperty_GoalExpression gexpr = dynamic_cast<CdlProperty_GoalExpression>(prop);
    CYG_ASSERT_CLASSC(valuable);
    CYG_ASSERT_CLASSC(gexpr);

    valuable->check_requires(transaction, gexpr);

    CYG_UNUSED_PARAM(CdlNode, dest);
    CYG_REPORT_RETURN();
}

int
CdlValuableBody::parse_requires(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_requires", "result %d");

    int result = CdlParse::parse_goalexpression_property(interp, argc, argv, CdlPropertyId_Requires, 0, 0,
                                                         &requires_update_handler);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_requires_goals() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_requires_goals", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_Requires);
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlValuableBody::get_requires_goals(std::vector<CdlProperty_GoalExpression>& result) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_requires_goals");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::vector<CdlProperty> properties;
    get_properties(CdlPropertyId_Requires, properties);
    std::vector<CdlProperty>::const_iterator i;
    for (i = properties.begin(); i != properties.end(); i++) {
        CdlProperty_GoalExpression goal = dynamic_cast<CdlProperty_GoalExpression>(*i);
        CYG_ASSERTC(0 != goal);
        result.push_back(goal);
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  implements property              

// ----------------------------------------------------------------------------
// Syntax: implements <reference to interface>

void
CdlValuableBody::implements_update_handler(CdlTransaction transaction, CdlNode source, CdlProperty prop, CdlNode dest,
                                           CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlValuable::implements_update_handler");
    CYG_REPORT_FUNCARG5XV(transaction, source, prop, dest, change);
    CYG_PRECONDITION_CLASSC(transaction);

    // Calculation of interface values happens inside
    // CdlInterfaceBody::recalculate(). That member function simply
    // checks all of the implementors and recalculates the value from
    // scratch. It needs to be invoked whenever there is a relevant
    // change to the implementors. Currently no attempt is made to
    // optimise interface updates, although this may have to change in
    // future.

    // Any changes to the interface itself can be ignored.
    if ((CdlUpdate_ValueChange == change) || (CdlUpdate_ActiveChange == change)) {
        CYG_REPORT_RETURN();
        return;
    }

    // The second stage init is irrelevant
    if (CdlUpdate_Init == change) {
        CYG_REPORT_RETURN();
        return;
    }

    // Possibilities:
    // 1) source is being loaded, dest valid
    // 2) source is being loaded, dest unknown
    // 3) source is being unloaded, dest valid
    // 4) source is being unloaded, dest unknown
    // 5) dest has been created
    // 6) dest is going away
    //
    // If we have a valid dest, it needs to be updated and any structural
    // conflicts have to be cleared.
    //
    // If there is no dest, the implements property remains unbound.
    // A suitable conflict is created in the base class.
    //
    // If the dest is invalid, a structural conflict has to be created.
    if (CdlUpdate_Destroyed == change) {
        // There is no need to do any clean-ups in the dest.
        dest = 0;
    }
    if (0 == dest) {
        transaction->clear_structural_conflicts(source, prop, &CdlConflict_DataBody::test);
    } else {
        CdlInterface interface = dynamic_cast<CdlInterface>(dest);

        if (0 == interface) {
            std::string msg = source->get_class_name() + " " + source->get_name() + " cannot implement " +
                dest->get_name() + "\n    The latter is not an interface.";
            CdlConflict_DataBody::make(transaction, source, prop, msg);
        } else {
            transaction->clear_structural_conflicts(source, prop, &CdlConflict_DataBody::test);
            interface->recalculate(transaction);
        }
    }
    
    CYG_REPORT_RETURN();
}

int
CdlValuableBody::parse_implements(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_implements", "result %d");

    int result = CdlParse::parse_reference_property(interp, argc, argv, CdlPropertyId_Implements, 0, 0, false,
                                                    &implements_update_handler);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlValuableBody::get_implemented_interfaces(std::vector<CdlInterface>& result) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_implemented_interfaces");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::vector<CdlProperty> properties;
    get_properties(CdlPropertyId_Implements, properties);
    std::vector<CdlProperty>::const_iterator i;
    for (i = properties.begin(); i != properties.end(); i++) {
        CdlProperty_Reference refprop = dynamic_cast<CdlProperty_Reference>(*i);
        CYG_ASSERTC(0 != refprop);
        CdlNode node = refprop->get_destination();
        if (0 != node) {
            CdlInterface interface = dynamic_cast<CdlInterface>(node);
            CYG_ASSERT_CLASSC(interface);
            result.push_back(interface);
        }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  Other properties                 

// ----------------------------------------------------------------------------
// Syntax: flavor <legal flavor>

static void
parse_flavor_final_check(CdlInterpreter interp, CdlProperty_String prop)
{
    CYG_REPORT_FUNCNAME("parse_flavor_final_check");
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITION_CLASSC(prop);
    
    const std::string& str = prop->get_string();
    std::string copy = std::string(str);
    CdlValueFlavor flavor;

    if (!Cdl::string_to_flavor(copy, flavor)) {
        CdlParse::report_property_parse_error(interp, prop, str + " is not a valid CDL flavor.");
    }
    
    CYG_REPORT_RETURN();
}


int
CdlValuableBody::parse_flavor(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_flavor", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Flavor, 0, &parse_flavor_final_check);
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// syntax: group <group name>
int
CdlValuableBody::parse_group(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_group", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Group, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: check_proc <tclcode>

int
CdlValuableBody::parse_check_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_check_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_CheckProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_check_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_check_proc", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_CheckProc);
    CYG_REPORT_RETVAL(result);
    return result;
}

cdl_tcl_code
CdlValuableBody::get_check_proc() const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_check_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdl_tcl_code result         = "";
    CdlProperty  property       = get_property(CdlPropertyId_CheckProc);
    if (0 != property) {
        CdlProperty_TclCode code_prop = dynamic_cast<CdlProperty_TclCode>(property);
        CYG_ASSERTC(0 != code_prop);
        result = code_prop->get_code();
    }
    
    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: entry_proc <tclcode>

int
CdlValuableBody::parse_entry_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_entry_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_EntryProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::has_entry_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_entry_proc", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_EntryProc);
    CYG_REPORT_RETVAL(result);
    return result;
}
cdl_tcl_code
CdlValuableBody::get_entry_proc() const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_entry_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdl_tcl_code result         = "";
    CdlProperty  property       = get_property(CdlPropertyId_EntryProc);
    if (0 != property) {
        CdlProperty_TclCode code_prop = dynamic_cast<CdlProperty_TclCode>(property);
        CYG_ASSERTC(0 != code_prop);
        result = code_prop->get_code();
    }

    CYG_REPORT_RETURN();
    return result;
}

//}}}

//{{{  CdlValuable misc                 

// ----------------------------------------------------------------------------
// Objects with flavor none are not modifiable. Also, objects with the
// calculated property are not modifiable. Everything else is ok.

bool
CdlValuableBody::is_modifiable() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuableBody::is_modifiable", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = true;
    if (CdlValueFlavor_None == get_flavor()) {
        result = false;
    } else if (has_property(CdlPropertyId_Calculated)) {
        result = false;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlValuable::get_widget_hint()   

// ----------------------------------------------------------------------------

void
CdlValuableBody::get_widget_hint(CdlWidgetHint& hint)
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_widget_hint");
    CYG_REPORT_FUNCARG2XV(this, &hint);
    CYG_PRECONDITION_THISC();

    // Start by resetting the hint to default values.
    hint.bool_widget  = CdlBoolWidget_None;
    hint.value_widget = CdlValueWidget_None;
    hint.radio_button_interface = "";

    // If the valuable is a loadable then it cannot be modified directly.
    // Changing the value means unloading and/or loading more data
    // into the configuration. This should always be handled via a
    // separate dialog, followed by a tree redisplay
    CdlConstLoadable loadable = dynamic_cast<CdlConstLoadable>(this);
    if (0 != loadable) {
        hint.value_widget = CdlValueWidget_Loadable;
        CYG_REPORT_RETURN();
        return;
    }
    
    // If the valuable is not modifiable then we are already done.
    CdlValueFlavor flavor = this->get_flavor();
    if ((CdlValueFlavor_None == flavor) || !this->is_modifiable()) {
        CYG_REPORT_RETURN();
        return;
    }

    // If there is a custom dialog and dialogs are enabled, use it.
    if (this->has_dialog() && CdlDialogBody::dialogs_are_enabled()) {
        if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
            hint.bool_widget = CdlBoolWidget_CustomDialog;
        }
        if ((CdlValueFlavor_Data == flavor) || (CdlValueFlavor_BoolData == flavor)) {
            hint.value_widget = CdlValueWidget_CustomDialog;
        }
        CYG_REPORT_RETURN();
        return;
    }
    
    // Process the bool part, if any
    if ((CdlValueFlavor_Bool == flavor) || (CdlValueFlavor_BoolData == flavor)) {
        
        // Default to a CheckButton
        hint.bool_widget = CdlBoolWidget_CheckButton;

        // Under some circumstances it is appropriate to use a radio button instead.
        // This is the case when there are several mutually exclusive entities.
        // Most of the time radio buttons should actually be handled by a single
        // option which has a list of legal values. There are a couple of cases
        // where this is not appropriate:
        //
        // 1) grouping. Some of the mutually exclusive entities could be containers.
        //    With clever use of a single option and some active_if properties it
        //    would be possible to get almost the same effect, but not quite.
        //
        // 2) external packages. It should be possible to have a third party package
        //    which could add e.g. a new scheduler.
        //
        // The implementation of this involves interfaces. Basically mutually
        // exclusive entities should implement the same interface, and that
        // interface should have an explicit requires $cdl_value == 1
        // In addition all of the options involved should have the same parent.
        // An entity may implement multiple interfaces, so they all have to be checked
        CdlInterface radio_interface = 0;
        std::vector<CdlProperty> implements = this->get_properties(CdlPropertyId_Implements);
        std::vector<CdlProperty>::const_iterator imp_i;
        for (imp_i = implements.begin(); (imp_i != implements.end()) && (0 == radio_interface); imp_i++) {
            CdlProperty_Reference refprop = dynamic_cast<CdlProperty_Reference>(*imp_i);
            CYG_ASSERT_CLASSC(refprop);

            CdlNode destnode = refprop->get_destination();
            if (0 == destnode) {
                continue;
            }
            CdlInterface interface = dynamic_cast<CdlInterface>(destnode);
            CYG_ASSERT_CLASSC(interface);
 
            std::vector<CdlProperty_GoalExpression> requires;
            std::vector<CdlProperty_GoalExpression>::const_iterator req_i;
            interface->get_requires_goals(requires);
            for (req_i = requires.begin(); req_i != requires.end(); req_i++) {

                CdlExpression expr = (*req_i)->get_expression();
                CdlSubexpression& subexpr = expr->sub_expressions[expr->first_subexpression];
                if (CdlExprOp_Equal != subexpr.op) {
                    continue;
                }
 
                CdlSubexpression& lhs = expr->sub_expressions[subexpr.lhs_index];
                CdlSubexpression& rhs = expr->sub_expressions[subexpr.rhs_index];
                CdlSubexpression* ref_operand = &lhs;

                // Allow for "a == 1" or "1 == a"
                if ((CdlExprOp_IntegerConstant == lhs.op) && (1 == lhs.constants.get_integer_value())) {
                    ref_operand = &rhs;
                } else if ((CdlExprOp_IntegerConstant == rhs.op) && (1 == rhs.constants.get_integer_value())) {
                    ref_operand = &lhs;
                } else {
                    continue;
                }

                if (CdlExprOp_Reference != ref_operand->op) {
                    continue;
                }
                CdlReference& ref = expr->references[ref_operand->reference_index];
                if (ref.get_destination() == interface) {
                    break;
                }
            }
            if (req_i == requires.end()) {
                continue;
            }

            CdlContainer parent = this->get_parent();
            CYG_ASSERT_CLASSC(parent);
 
            std::vector<CdlValuable> implementers;
            std::vector<CdlValuable>::const_iterator imp_i;
            interface->get_implementers(implementers);
            for (imp_i = implementers.begin(); imp_i != implementers.end(); imp_i++) {
                if (parent != (*imp_i)->get_parent()) {
                    break;
                }
            }

            if (imp_i == implementers.end()) {
                // An interface has been found that matches the constraints.
                radio_interface = interface;
            }
        }
        if (0 != radio_interface) {
            hint.bool_widget = CdlBoolWidget_Radio;
            hint.radio_button_interface = radio_interface->get_name();
        }
    }

    // Process the data part, if any
    if ((CdlValueFlavor_Data == flavor) || (CdlValueFlavor_BoolData == flavor)) {
        
        // Default to a simple entry box.
        hint.value_widget = CdlValueWidget_EntryBox;
        
        // If there is a legal_values list, this will normally indicate
        // which widget should be used.
        if (this->has_legal_values()) {
            // The legal_values expression needs to be evaluated and examined.
            // If the result is a simple numerical range then all we need to
            // figure out is whether to default to decimal, hex, octal or double.
            // Otherwise if the result is a simple list and all of the entries
            // are numerical, that is sufficient information. If a list with
            // non-numerical entries that is fine as well. Anything more complicated
            // needs to revert to an entry box.
            CdlProperty_ListExpression lexpr = this->get_legal_values();
            CdlEvalContext             context(0, this, lexpr);
            CdlListValue               val;

            try {
                lexpr->eval(context, val);
                const std::vector<CdlSimpleValue>& table = val.get_table();
                const std::vector<std::pair<cdl_int, cdl_int> >& int_ranges = val.get_integer_ranges();
                const std::vector<std::pair<double, double> >&   double_ranges = val.get_double_ranges();
                
                if ((0 == table.size()) && (0 == int_ranges.size()) && (1 == double_ranges.size())) {
                    
                    // A straightforward range of double precision numbers
                    hint.value_widget = CdlValueWidget_DoubleRange;
                    
                } else if ((0 == table.size()) && (1 == int_ranges.size()) && (0 == double_ranges.size())) {

                    // Bummer. The formatting information has been lost.
                    // To fix this the two sets of ranges should be collapsed into pairs of
                    // CdlSimpleValue's.
                    hint.value_widget = CdlValueWidget_DecimalRange;
                    
                } else if ((1 <= table.size() && (0 == int_ranges.size()) && (0 == double_ranges.size()))) {

                    // If all of the values are numerical, then we have a numeric set.
                    // Otherwise we have a string set.
                    bool all_numeric = true;
                    std::vector<CdlSimpleValue>::const_iterator tab_i;
                    for (tab_i = table.begin(); (tab_i != table.end()) && all_numeric; tab_i++) {
                        if (!tab_i->has_double_value() && !tab_i->has_integer_value()) {
                            all_numeric = false;
                        }
                    }
                    if (all_numeric) {
                        hint.value_widget = CdlValueWidget_NumericSet;
                    } else {
                        hint.value_widget = CdlValueWidget_StringSet;
                    }
                    
                } else {
                    // The list expression is a complex combination. Leave it as an entry box.
                    // In some cases it would be possible to do better, for example
                    //     legal_values -1 1 to 4 8 to 12
                    // Support for cases like these may get added in future, if such cases
                    // ever arise in practice.
                }
                
            } catch(...) {
                // Not a lot that can be done here, unfortunately
            }
        } else {
            // There is no legal_values property, so an entry box is probably the
            // right thing to use. There is a special case for multiline strings,
            // identified by a default_value expression that contains a newline.
            if (this->has_default_value_expression()) {
                CdlProperty_Expression expr = this->get_default_value_expression();
                CdlEvalContext         context(0, this, expr);
                CdlSimpleValue         val;
                try {
                    expr->eval(context, val);
                    std::string tmp = val.get_value();
                    if (std::string::npos != tmp.find('\n')) {
                        hint.value_widget = CdlValueWidget_MultilineString;
                    }
                } catch(...) {
                    // Not a lot that can be done here, unfortunately
                }
            }
        }
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlValuable get operations       

// ----------------------------------------------------------------------------
const CdlValue&
CdlValuableBody::get_whole_value() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_whole_value", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETVAL(&value);
    return value;
}

CdlValueFlavor
CdlValuableBody::get_flavor() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_flavor", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlValueFlavor result = value.get_flavor();
    CYG_REPORT_RETVAL((int) result);
    return result;
}

CdlValueSource
CdlValuableBody::get_source() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_source", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlValueSource result = value.get_source();
    CYG_REPORT_RETVAL((int) result);
    return result;
}

bool
CdlValuableBody::has_source(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_source", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    bool result = value.has_source(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::is_enabled(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::is_enabled", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    bool result = value.is_enabled(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlValuableBody::get_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_value");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    std::string result = value.get_value(source);
    CYG_REPORT_RETURN();
    return result;
}

bool
CdlValuableBody::has_integer_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_integer_value", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    bool result = value.has_integer_value(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

cdl_int
CdlValuableBody::get_integer_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_integer_value", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    cdl_int result = value.get_integer_value(source);
    CYG_REPORT_RETVAL((int) result);
    return result;
}

bool
CdlValuableBody::has_double_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_double_value", "result %d");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    bool result = value.has_double_value(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

double
CdlValuableBody::get_double_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_double_value");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    double result = value.get_double_value();
    CYG_REPORT_RETURN();
    return result;
}

CdlSimpleValue
CdlValuableBody::get_simple_value(CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_simple_value");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    CdlSimpleValue result = value.get_simple_value(source);
    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
CdlValueSource
CdlValuableBody::get_source(CdlTransaction transaction) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_source", "result %d");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    CdlValueSource result = transaction_value.get_source();
    CYG_REPORT_RETVAL((int) result);
    return result;
}

bool
CdlValuableBody::has_source(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_source", "result %d");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    bool result = transaction_value.has_source(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlValuableBody::is_enabled(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::is_enabled", "result %d");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    bool result = transaction_value.is_enabled(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlValuableBody::get_value(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_value");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    std::string result = transaction_value.get_value(source);
    CYG_REPORT_RETURN();
    return result;
}

bool
CdlValuableBody::has_integer_value(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_integer_value", "result %d");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    bool result = transaction_value.has_integer_value(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

cdl_int
CdlValuableBody::get_integer_value(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::get_integer_value", "result %d");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    cdl_int result = transaction_value.get_integer_value(source);
    CYG_REPORT_RETVAL((int) result);
    return result;
}

bool
CdlValuableBody::has_double_value(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::has_double_value", "result %d");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    bool result = transaction_value.has_double_value(source);
    CYG_REPORT_RETVAL(result);
    return result;
}

double
CdlValuableBody::get_double_value(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_double_value");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    double result = transaction_value.get_double_value();
    CYG_REPORT_RETURN();
    return result;
}

CdlSimpleValue
CdlValuableBody::get_simple_value(CdlTransaction transaction, CdlValueSource source) const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_simple_value");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& transaction_value = transaction->get_whole_value(this);
    CdlSimpleValue result = transaction_value.get_simple_value(source);
    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  CdlValuable internal modify ops  

// ----------------------------------------------------------------------------
// There has been a change to either the value itself or to the
// set of legal values. It is necessary to validate the current
// value, maintaining a suitable conflict object.
void
CdlValuableBody::check_value(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAME("CdlValuable::check_value");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    // Checking the value only makes sense for BoolData and Data
    // values.
    CdlValueFlavor flavor = value.get_flavor();
    if ((CdlValueFlavor_BoolData != flavor) && (CdlValueFlavor_Data != flavor)) {
        CYG_REPORT_RETURN();
        return;
    }

    // If the valuable is not currently active and enabled then it
    // does not matter whether or not the value is legal. Any old
    // conflicts should be destroyed.
    if (!(transaction->is_active(this) && this->is_enabled(transaction))) {
        transaction->clear_conflicts(this, &CdlConflict_IllegalValueBody::test);
        CYG_REPORT_RETURN();
        return;
    }

    // If there is a legal_values property, check membership.
    if (this->has_property(CdlPropertyId_LegalValues)) {
        CdlProperty_ListExpression lexpr = dynamic_cast<CdlProperty_ListExpression>(get_property(CdlPropertyId_LegalValues));
        CYG_ASSERT_CLASSC(lexpr);

        CdlSimpleValue val = this->get_simple_value(transaction);
        CdlEvalContext context(transaction, this, lexpr);
        try {
            if (!lexpr->is_member(context, val)) {
                if (!transaction->has_conflict(this, lexpr, &CdlConflict_IllegalValueBody::test)) {
                    CdlConflict_IllegalValueBody::make(transaction, this, lexpr);
                }
            
            } else {
                // Tne current value is legal. Get rid of any old conflicts.
                transaction->clear_conflicts(this, lexpr, &CdlConflict_IllegalValueBody::test);
            }
        } catch(CdlEvalException e) {
            // There should now be an EvalException conflict for this
            // node, so there is no point in having an IllegalValue conflict
            // as well.
            transaction->clear_conflicts(this, lexpr, &CdlConflict_IllegalValueBody::test);
        }              

        // FIXME: add support for check_proc
    }
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// There has been a change that may affect "requires" properties.
// Again do the necessary checking and maintain suitable conflict
// objects.
void
CdlValuableBody::check_requires(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAME("CdlValuable::check_requires");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    std::vector<CdlProperty> requires_properties;
    std::vector<CdlProperty>::const_iterator prop_i;
    get_properties(CdlPropertyId_Requires, requires_properties);
    for (prop_i = requires_properties.begin(); prop_i != requires_properties.end(); prop_i++) {

        CdlProperty_GoalExpression gexpr = dynamic_cast<CdlProperty_GoalExpression>(*prop_i);
        CYG_ASSERT_CLASSC(gexpr);
        this->check_requires(transaction, gexpr);
    }

    CYG_REPORT_RETURN();
}

void
CdlValuableBody::check_requires(CdlTransaction transaction, CdlProperty_GoalExpression gexpr)
{
    CYG_REPORT_FUNCNAME("CdlValuable::check_requires (property)");
    CYG_REPORT_FUNCARG3XV(this, transaction, gexpr);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    CYG_ASSERT_CLASSC(gexpr);

    // If the valuable is not currently active and enabled then the "requires"
    // properties are irrelevant, and any old conflicts should be destroyed.
    if (!transaction->is_active(this) || !this->is_enabled(transaction)) {
        transaction->clear_conflicts(this, gexpr, &CdlConflict_RequiresBody::test);
        CYG_REPORT_RETURN();
        return;
    }

    // What is the current value of the goal expression?
    try {
        CdlEvalContext context(transaction, this, gexpr);
        if (gexpr->eval(context)) {
            // The goal is satisfied.
            transaction->clear_conflicts(this, gexpr, &CdlConflict_RequiresBody::test);
        } else {
            // The goal is not satisfied. Make sure there is a conflict object.
            if (!transaction->has_conflict(this, gexpr, &CdlConflict_RequiresBody::test)) {
                CdlConflict_RequiresBody::make(transaction, this, gexpr);
            }
        }
    } catch(CdlEvalException e) {
        // There should now be an EvalException conflict associated with this node,
        // having a requires conflict as well serves no purpose
        transaction->clear_conflicts(this, gexpr, &CdlConflict_RequiresBody::test);
    }
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// The update handler. If there is a change to the value or active state
// then it is necessary to reevaluate any requires properties, and to
// check whether or not the value is legal wrt legal_values etc.
void
CdlValuableBody::update(CdlTransaction transaction, CdlUpdate update)
{
    CYG_REPORT_FUNCNAME("CdlValuable::update");
    CYG_REPORT_FUNCARG3XV(this, transaction, update);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    if ((CdlUpdate_ValueChange == update) || (CdlUpdate_ActiveChange == update)) {
        this->check_value(transaction);
        this->check_requires(transaction);
    }
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Should this node be active. In addition to the base class' checks that
// the parent is active and enabled, any active_if constraints need
// to be evaluated.

bool
CdlValuableBody::test_active(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::test_active", "result %d");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    bool result = true;
    if (!this->CdlNodeBody::test_active(transaction)) {
        result = false;
    }

    if (result) {
        std::vector<CdlProperty> active_if_properties;
        std::vector<CdlProperty>::const_iterator prop_i;
        
        this->get_properties(CdlPropertyId_ActiveIf, active_if_properties);
        for (prop_i = active_if_properties.begin(); result && (prop_i != active_if_properties.end()); prop_i++) {
            
            CdlProperty_GoalExpression gexpr = dynamic_cast<CdlProperty_GoalExpression>(*prop_i);
            CYG_ASSERT_CLASSC(gexpr);
            CdlEvalContext context(transaction, this, gexpr);
            try {
                if (!gexpr->eval(context)) {
                    result = false;
                }
            } catch(CdlEvalException e) {
                // Hmmm, an active_if property cannot be evaluated.
                // Tricky. If the node is inactive then its conflicts
                // are ignored, which would be a bad thing. For now
                // assume that the node is active, unless it was already
                // inactive for other reasons.
            }
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlValuable modify operations    

// ----------------------------------------------------------------------------
// Start with the non-transaction versions. These allocate a new transaction,
// perform their operation in the context of that transaction, and then
// commit the transaction.

void
CdlValuableBody::set_source(CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_source (no transaction)");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();
    
    CdlTransaction transaction = CdlTransactionBody::make(get_toplevel());
    this->set_source(transaction, source);
    transaction->body();
    delete transaction;

    CYG_REPORT_RETURN();
}

void
CdlValuableBody::invalidate_source(CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::invalidate_source (no transaction)");
    CYG_REPORT_FUNCARG2XV(this, source);
    CYG_PRECONDITION_THISC();

    CdlTransaction transaction = CdlTransactionBody::make(get_toplevel());
    this->invalidate_source(transaction, source);
    transaction->body();
    delete transaction;

    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set_enabled(bool val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_enabled (no transaction)");
    CYG_REPORT_FUNCARG3XV(this, val, source);
    CYG_PRECONDITION_THISC();
    
    CdlTransaction transaction = CdlTransactionBody::make(get_toplevel());
    this->set_enabled(transaction, val, source);
    transaction->body();
    delete transaction;

    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set_value(CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_value (no transaction)");
    CYG_REPORT_FUNCARG3XV(this, &val, source);
    CYG_PRECONDITION_THISC();

    CdlTransaction transaction = CdlTransactionBody::make(get_toplevel());
    this->set_value(transaction, val, source);
    transaction->body();
    delete transaction;

    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set_enabled_and_value(bool enabled_arg, CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_enabled_and_value (no transaction)");
    CYG_REPORT_FUNCARG4XV(this, enabled_arg, &val, source);
    CYG_PRECONDITION_THISC();

    CdlTransaction transaction = CdlTransactionBody::make(get_toplevel());
    this->set_enabled_and_value(transaction, enabled_arg, val, source);
    transaction->body();
    delete transaction;

    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set(CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set (no transaction)");
    CYG_REPORT_FUNCARG3XV(this, &val, source);
    CYG_PRECONDITION_THISC();

    CdlTransaction transaction = CdlTransactionBody::make(get_toplevel());
    this->set(transaction, val, source);
    transaction->body();
    delete transaction;

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// These member functions operate in the context of a transaction. The
// basic format is:
//
//  1) find out the state before the change
//  2) make a local CdlValue copy, and modify it.
//  3) update the value held in the transaction.
//
// Values checks etc. happen during propagation, mainly from inside
// the update handler. There is code in CdlTransaction::set_whole_value()
// to avoid unnecessary propagation.

void
CdlValuableBody::set_source(CdlTransaction transaction, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_source");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_ASSERTC((source == CdlValueSource_Default) || !has_property(CdlPropertyId_Calculated));
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    const CdlValue& old_value = transaction->get_whole_value(this);
    CdlValue        new_value = old_value;
    new_value.set_source(source);
    transaction->set_whole_value(this, old_value, new_value);

    CYG_REPORT_RETURN();
}

void
CdlValuableBody::invalidate_source(CdlTransaction transaction, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::invalidate_source");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    const CdlValue& old_value = transaction->get_whole_value(this);
    CdlValue        new_value = old_value;
    new_value.invalidate_source(source);
    transaction->set_whole_value(this, old_value, new_value);
    
    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set_enabled(CdlTransaction transaction, bool enabled_arg, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_enabled");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_ASSERTC((source == CdlValueSource_Default) || !has_property(CdlPropertyId_Calculated));
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);

    const CdlValue& old_value = transaction->get_whole_value(this);
    CdlValue        new_value = old_value;
    new_value.set_enabled(enabled_arg, source);
    transaction->set_whole_value(this, old_value, new_value);
    
    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set_value(CdlTransaction transaction, CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_enabled");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_ASSERTC((source == CdlValueSource_Default) || !has_property(CdlPropertyId_Calculated));
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    const CdlValue& old_value = transaction->get_whole_value(this);
    CdlValue        new_value = old_value;
    new_value.set_value(val, source);
    transaction->set_whole_value(this, old_value, new_value);
    
    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set_enabled_and_value(CdlTransaction transaction, bool enabled_arg, CdlSimpleValue& val,
                                       CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set_enabled");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_ASSERTC((source == CdlValueSource_Default) || !has_property(CdlPropertyId_Calculated));
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    const CdlValue& old_value = transaction->get_whole_value(this);
    CdlValue        new_value = old_value;
    new_value.set_enabled_and_value(enabled_arg, val, source);
    transaction->set_whole_value(this, old_value, new_value);
    
    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set(CdlTransaction transaction, CdlSimpleValue& val, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set");
    CYG_REPORT_FUNCARG3XV(this, transaction, source);
    CYG_ASSERTC((source == CdlValueSource_Default) || !has_property(CdlPropertyId_Calculated));
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    const CdlValue& old_value = transaction->get_whole_value(this);
    CdlValue        new_value = old_value;
    new_value.set(val, source);
    transaction->set_whole_value(this, old_value, new_value);
    
    CYG_REPORT_RETURN();
}

void
CdlValuableBody::set(CdlTransaction transaction, const CdlValue& val)
{
    CYG_REPORT_FUNCNAME("CdlValuable::set");
    CYG_REPORT_FUNCARG2XV(this, transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(transaction);
    
    const CdlValue& old_value = transaction->get_whole_value(this);
    CdlValue        new_value = val;
    transaction->set_whole_value(this, old_value, new_value);

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlValuable basics               

// ----------------------------------------------------------------------------
// The CdlValuable class implements the concept of CDL objects that take
// a value. There are lots of properties associated with that.

CdlValuableBody::CdlValuableBody(CdlValueFlavor flavor)
    : value(flavor)
{
    CYG_REPORT_FUNCNAME("CdlValuable:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    cdlvaluablebody_cookie = CdlValuableBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlValuableBody::~CdlValuableBody()
{
    CYG_REPORT_FUNCNAME("CdlValuableBody:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlvaluablebody_cookie = CdlValuableBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------

std::string
CdlValuableBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlValuable::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "valuable";
}

// ----------------------------------------------------------------------------
bool
CdlValuableBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlValuableBody_Magic != cdlvaluablebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    if (has_property(CdlPropertyId_Calculated) && (CdlValueSource_Default != value.get_source())) {
        CYG_FAIL("Calculated valuables can only have a default value.");
        return false;
    }
    
    return CdlNodeBody::check_this(zeal) && value.check_this(zeal);
}

//}}}
//{{{  CdlValuable parsing support      

// ----------------------------------------------------------------------------
// Parsing support. Adding the appropriate parsers is straightforward.

void
CdlValuableBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlValuable::add_property_parsers");

    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("active_if",          &parse_active_if    ),
        CdlInterpreterCommandEntry("calculated",         &parse_calculated   ),
        CdlInterpreterCommandEntry("check_proc",         &parse_check_proc   ),
        CdlInterpreterCommandEntry("default_value",      &parse_default_value),
        CdlInterpreterCommandEntry("dialog",             &parse_dialog       ),
        CdlInterpreterCommandEntry("entry_proc",         &parse_entry_proc   ),
        CdlInterpreterCommandEntry("flavor",             &parse_flavor       ),
        CdlInterpreterCommandEntry("group",              &parse_group        ),
        CdlInterpreterCommandEntry("implements",         &parse_implements   ),
        CdlInterpreterCommandEntry("legal_values",       &parse_legal_values ),
        CdlInterpreterCommandEntry("requires",           &parse_requires     ),
        CdlInterpreterCommandEntry("wizard",             &parse_wizard       ),
        CdlInterpreterCommandEntry("",                   0                   )
    };

    for (int i = 0; commands[i].command != 0; i++) {
        std::vector<CdlInterpreterCommandEntry>::const_iterator j;
        for (j = parsers.begin(); j != parsers.end(); j++) {
            if (commands[i].name == j->name) {
                if (commands[i].command != j->command) {
                    CYG_FAIL("Property names are being re-used");
                }
                break;
            }
        }
        if (j == parsers.end()) {
            parsers.push_back(commands[i]);
        }
    }
    CdlNodeBody::add_property_parsers(parsers);
    
    CYG_REPORT_RETURN();
}

// Validatation is quite a bit more complicated...
void
CdlValuableBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlValuable::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    // There should be at most one of flavor, entry_proc, check_proc,
    // default_value, legal_values, dialog, and calculated. There can
    // be any number of active_if, requires, and implements.
    // NOTE: should multiple entry_proc's and check_proc's be allowed?
    //       This could prove useful if there are a sensible number
    //       of library check_proc's.
    if (count_properties(CdlPropertyId_Flavor) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one flavor property.");
    }
    if (count_properties(CdlPropertyId_EntryProc) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one entry_proc property.");
    }
    if (count_properties(CdlPropertyId_CheckProc) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one check_proc property.");
    }
    if (count_properties(CdlPropertyId_DefaultValue) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one default_value property.");
    }
    if (count_properties(CdlPropertyId_LegalValues) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one legal_values property.");
    }
    if (count_properties(CdlPropertyId_Dialog) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one dialog property.");
    }
    if (count_properties(CdlPropertyId_Wizard) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one wizard property.");
    }
    if (count_properties(CdlPropertyId_Calculated) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one calculated property.");
    }

    // If there is a flavor property, update the flavor in the base class
    if (has_property(CdlPropertyId_Flavor)) {
        CdlProperty_String flavor_property = dynamic_cast<CdlProperty_String>(get_property(CdlPropertyId_Flavor));
        CYG_ASSERTC(0 != flavor_property);
        
        std::string flavor_string = flavor_property->get_string();
        CdlValueFlavor flavor;
        // The property parsing code should have caught any problems already.
        if (!Cdl::string_to_flavor(flavor_string, flavor)) {
            CdlParse::report_error(interp, "", "Invalid flavor " + flavor_string);
        } else {
            value.set_flavor(flavor);
        }

        // If the flavor is "none" then the entity is not modifiable,
        // and most of the properties do not make sense. However this
        // is not enforced at parse-time: temporarily switching to
        // flavor none may make sense during debugging.
        // FIXME: no longer correct
    }

    // For boolean entities legal_values does not make much sense.
    // In theory a legal_values property could be used to restrict
    // the value to just true or just false, but the same effect
    // can be achieved more sensibly with a "requires" property.
    //
    // check_proc is allowed, this can be used to check programatically
    // that the current value is legal.
    if (CdlValueFlavor_Bool == get_flavor()) {
        if (has_property(CdlPropertyId_LegalValues)) {
            CdlParse::report_error(interp, "", "The \"legal_values\" property is not applicable to boolean entities."); 
        }
    }

    // default_value and calculated are mutually exclusive
    if (has_property(CdlPropertyId_Calculated) && has_property(CdlPropertyId_DefaultValue)) {
        CdlParse::report_error(interp, "", "The properties \"default_value\" and \"calculated\" cannot be used together.");
    }

#if 0
    // Dialog is not mutually exclusive with entry_proc.
    // Custom dialogs may not be supported, in which case it is likely that
    // a text entry widget will be used and an entry_proc may well be
    // applicable.
    if (has_property(CdlPropertyId_Dialog) && has_property(CdlPropertyId_EntryProc)) {
        CdlParse::report_error(interp, "", "The properties \"dialog\" and \"entry_proc\" cannot be used together.");
    }
#endif    

    // All of the expressions may be invalid because of unresolved references,
    // ditto for implements and for dialog. 
    
    CdlNodeBody::check_properties(interp);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlValuable persistence support  

// ----------------------------------------------------------------------------
void
CdlValuableBody::initialize_savefile_support(CdlToplevel toplevel, std::string major_command)
{
    CYG_REPORT_FUNCNAME("CdlValuable::initialize_savefile_support");
    CYG_PRECONDITION_CLASSC(toplevel);
    CYG_PRECONDITIONC("" != major_command);

    toplevel->add_savefile_subcommand(major_command, "value_source", 0, &savefile_value_source_command);
    toplevel->add_savefile_subcommand(major_command, "user_value",   0, &savefile_user_value_command);
    toplevel->add_savefile_subcommand(major_command, "wizard_value", 0, &savefile_wizard_value_command);
    toplevel->add_savefile_subcommand(major_command, "inferred_value", 0, &savefile_inferred_value_command);

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Is a savefile entry actually needed for this valuable? When performing
// a minimal save there is no point in outputting valuables which have
// a default value.
bool
CdlValuableBody::value_savefile_entry_needed() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlValuable::value_savefile_entry_needed", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;

    if (this->is_modifiable()) {
        if (this->has_source(CdlValueSource_User) ||
            this->has_source(CdlValueSource_Wizard) ||
            this->has_source(CdlValueSource_Inferred)) {

            result = true;
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// This utility is useful for outputting a particular value source

static std::string one    = "1";     // Needed to avoid confusing the compiler
static std::string zero   = "0";

static std::string
value_to_string(CdlValuable valuable, CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("value_to_string");

    std::string data = "";
    
    switch(valuable->get_flavor()) {
      case CdlValueFlavor_Bool :
        data += (valuable->is_enabled(source) ? one : zero);
        break;
      case CdlValueFlavor_BoolData :
        data += (valuable->is_enabled(source) ? one : zero) + " " +
            CdlInterpreterBody::quote(valuable->get_value(source));
        break;
      case CdlValueFlavor_Data:
        data += CdlInterpreterBody::quote(valuable->get_value(source));
        break;
      default:
        CYG_FAIL("Invalid value flavor detected");
        break;
    }
    return data;
}

// Another utility to figure out the expected value source, given which
// sources are available.
static CdlValueSource
get_expected_source(CdlValuable valuable)
{
    CYG_REPORT_FUNCNAMETYPE("get_expected_source", "result %d");
    CYG_REPORT_FUNCARG1XV(valuable);

    CdlValueSource expected_source = CdlValueSource_Default;
        
    if (valuable->has_source(CdlValueSource_User)) {
        expected_source = CdlValueSource_User;
    } else if (valuable->has_source(CdlValueSource_Wizard)) {
        expected_source = CdlValueSource_Wizard;
    } else if (valuable->has_source(CdlValueSource_Inferred)) {
        expected_source = CdlValueSource_Inferred;
    }

    CYG_REPORT_RETVAL((int) expected_source);
    return expected_source;
}

// And another utility, to list the valuables listed in an expression.
// e.g. for an expression of the form
//
//      requires (AAA + BBB) > CCC
//
// this would produce:
//
//      AAA == 1
//      BBB == 2
//      CCC == 0
//
// No indentation happens here, instead the calling code is assumed
// to use multiline_comment()
static std::string
follow_expr_references(CdlProperty property, CdlExpression expr)
{
    CYG_REPORT_FUNCNAME("follow_expr_references");
    CYG_REPORT_FUNCARG1XV(expr);
    CYG_PRECONDITION_CLASSC(expr);

    std::string    data = "";
    CdlSimpleValue simple_value;
    std::vector<CdlReference>::const_iterator ref_i;
    
    for (ref_i = expr->references.begin(); ref_i != expr->references.end(); ref_i++) {
        const std::string& refname = ref_i->get_destination_name();
        CdlNode refnode = ref_i->get_destination();
        CdlValuable refvaluable = 0;
        if (0 != refnode) {
            refvaluable = dynamic_cast<CdlValuable>(refnode);
        }
        data += refname + " ";
        if (0 == refvaluable) {
            data += "(unknown) == 0";
        } else {
            CdlEvalContext context(0, refvaluable, property);
            CdlSimpleValue::eval_valuable(context, refvaluable, simple_value);
            data += "== " + CdlInterpreterBody::quote(simple_value.get_value());
        }
        data += '\n';
    }
    
    CYG_REPORT_RETURN();
    return data;
}

// ----------------------------------------------------------------------------

void
CdlValuableBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool modifiable, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlValuable::save");
    CYG_REPORT_FUNCARG5XV(this, interp, chan, indentation, minimal);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    std::string data = "";
    std::string indent_string = std::string(indentation, ' ');
    std::string tmp_value     = "";
    CdlSimpleValue simple_value;

    // If performing a minimal save, the only fields of interest are the
    // user value, the wizard value, the inferred value, and the value source.
    // Not all of these need be present.
    //
    // Having two places where these fields get output is unfortunate,
    // but the alternative is an awful lot of "if (minimal)" tests
    // in the main code.
    if (minimal) {
        
        if (modifiable) {
            if (this->has_source(CdlValueSource_User)) {
                data += indent_string + "user_value " + value_to_string(this, CdlValueSource_User) + "\n";
            }
            if (this->has_source(CdlValueSource_Wizard)) {
                data += indent_string + "wizard_value " + value_to_string(this, CdlValueSource_Wizard) + "\n";
            }
            if (this->has_source(CdlValueSource_Inferred)) {
                data += indent_string + "inferred_value " + value_to_string(this, CdlValueSource_Inferred) + "\n";
            }
            CdlValueSource expected_source = get_expected_source(this);
            if (expected_source != this->get_source()) {
                std::string current_source_string;
                if (!Cdl::source_to_string(this->get_source(), current_source_string)) {
                    CYG_FAIL("Invalid current value source detected");
                }
                data += indent_string + "value_source " + current_source_string + "\n";
            }
        }
        
    } else {
    
        // Right at the start, indicate whether or not this property is active.
        if (!this->is_active()) {
            data += indent_string + "# This option is not active\n";
            // If the entity is inactive because the parent is inactive or disabled,
            // say so here. This is in addition to any unsatisfied active_if
            // conditions, which will be reported below.
            CdlContainer parent = this->get_parent();
            if (!parent->is_active()) {
                data += indent_string + "# The parent " + parent->get_name() + " is not active\n";
            }
            CdlValuable tmp = dynamic_cast<CdlValuable>(parent);
            if ((0 != tmp) && !tmp->is_enabled()) {
                data += indent_string + "# The parent " + parent->get_name() + " is disabled\n";
            }
        }
        if (this->has_active_if_conditions()) {
            std::vector<CdlProperty_GoalExpression> active_if_conditions;
            this->get_active_if_conditions(active_if_conditions);
            std::vector<CdlProperty_GoalExpression>::const_iterator expr_i;
            for (expr_i = active_if_conditions.begin(); expr_i != active_if_conditions.end(); expr_i++) {
                data += indent_string + "# ActiveIf constraint: " +
                    CdlInterpreterBody::extend_comment((*expr_i)->get_original_string(), indentation, 4) +
                    '\n';

                CdlExpression expr = (*expr_i)->get_expression();
                data += CdlInterpreterBody::multiline_comment(follow_expr_references(*expr_i, expr), indentation, 4);
                CdlEvalContext context(0, this, *expr_i);
                bool active_if_value = false;
                try {
                    active_if_value = (*expr_i)->eval(context);
                } catch(CdlEvalException e) {
                    active_if_value = false;
                } catch(std::bad_alloc) {
                    throw;
                }
                data += indent_string + "#   --> " + (active_if_value ? one : zero) + "\n";
            }
        }
        
        // If there has been any information related to the active status,
        // add a blank line before we start worrying about values.
        if (0 < data.size()) {
            data += '\n';
        }

        if (CdlValueFlavor_None == this->get_flavor()) {
            data += indent_string + "# There is no associated value.\n";
        } else if (this->has_property(CdlPropertyId_Calculated)) {
            CdlProperty_Expression expr = this->get_calculated_expression();
            data += indent_string + "# Calculated value: " +
                CdlInterpreterBody::extend_comment(expr->get_original_string(), indentation, 4) + '\n';
            data += CdlInterpreterBody::multiline_comment(follow_expr_references(expr, expr), indentation, 4);
        } else if (!modifiable) {
            data += indent_string + "# This value cannot be modified here.\n";
        }
         
        // Output the flavor. This clutters up the savefile a bit.
        // However it is necessary so that the user can distinguish
        // between bool, booldata and data items
        switch(this->get_flavor()) {
          case CdlValueFlavor_Bool:
            data += indent_string + "# Flavor: bool\n";
            break;
          case CdlValueFlavor_BoolData:
            data += indent_string + "# Flavor: booldata\n";
            break;
          case CdlValueFlavor_Data:
            data += indent_string + "# Flavor: data\n";
            break;
          default:
            break;
        }
            
        // If the value is not modifiable, just list the current value.
        // This is not in a form that allows users to change it easily.
        if (!modifiable) {
            switch(this->get_flavor()) {
              case CdlValueFlavor_None :
                break;
              case CdlValueFlavor_Bool :
                data += indent_string + "# Current value: " + (this->is_enabled() ? one : zero) + '\n';
                break;
              case CdlValueFlavor_BoolData :
                data += indent_string + "# Current value: " + (this->is_enabled() ? one : zero) + " " +
                    CdlInterpreterBody::extend_comment(this->get_value(), indentation, 4) + '\n';
                break;
              case CdlValueFlavor_Data :
                data += indent_string + "# Current_value: " +
                    CdlInterpreterBody::extend_comment(this->get_value(), indentation, 4) + '\n';
                break;
              default:
                break;
            }
        
        } else if (CdlValueFlavor_None != this->get_flavor()) {

            // If there is a user value, output it. Otherwise output
            // a comment that allows users to edit the user value conveniently.
            // It is assumed that the user will want a value similar to the
            // default one, so that is provided as the starting point
            if (this->has_source(CdlValueSource_User)) {
                data += indent_string + "user_value " + value_to_string(this, CdlValueSource_User) + "\n";
            } else {
                data += indent_string + "# No user value, uncomment the following line to provide one.\n" +
                    indent_string + "# user_value " +
                    CdlInterpreterBody::extend_comment(value_to_string(this, CdlValueSource_Default), indentation, 0) + "\n";
            }
        
            // Output a wizard value iff there is one. There is little point
            // in letting users edit a wizard value, they should be running
            // the wizard itself.
            if (this->has_source(CdlValueSource_Wizard)) {
                data += indent_string + "# The wizard value should not be edited directly.\n" +
                    indent_string + "# Instead the wizard should be run again if necessary.\n";
                data += indent_string + "wizard_value " + value_to_string(this, CdlValueSource_Wizard) + "\n";
            }

            // List the inferred value. This needs to be a command,
            if (this->has_source(CdlValueSource_Inferred)) {
                data += indent_string + "# The inferred value should not be edited directly.\n";
                data += indent_string + "inferred_value " + value_to_string(this, CdlValueSource_Inferred) + "\n";
            }
        
            // Output the value source iff it is unusual. If the current
            // source is the highest priority one then there is no point
            // in outputting a command, but a comment is usual. The value
            // source needs to come after wizard and inferred values
            std::string    current_source_string;
            CdlValueSource expected_source = get_expected_source(this);
            CdlValueSource current_source  = this->get_source();
            if (!Cdl::source_to_string(current_source, current_source_string)) {
                CYG_FAIL("Invalid current value source detected");
            }
            if (this->get_source() == expected_source) {
                data += indent_string + "# value_source " + current_source_string + "\n";
            } else {
                data += indent_string + "value_source " + current_source_string + "\n";
            }

            // Always output the default value as a comment.
            data += indent_string + "# Default value: ";
            
            // If there is no default_value expression or if the expression involves
            // only constants, just output the current default value. Otherwise
            // output both the expression and the value
            CdlProperty prop = this->get_property(CdlPropertyId_DefaultValue);
            CdlProperty_Expression expr = dynamic_cast<CdlProperty_Expression>(prop);
            if ((0 == expr) || (0 == expr->references.size())) {
                // There is no default_value expression, so just output the current value
                data += CdlInterpreterBody::extend_comment(value_to_string(this, CdlValueSource_Default), indentation, 4)
                    + "\n";
            } else {
                data += CdlInterpreterBody::extend_comment(expr->get_original_string(), indentation, 4) + "\n";
                data += CdlInterpreterBody::multiline_comment(follow_expr_references(expr, expr), indentation, 4);
                data += indent_string + "#   --> " +
                    CdlInterpreterBody::extend_comment(value_to_string(this, CdlValueSource_Default), indentation, 4) + "\n";
            }
        }

        // If there is a legal_values property, add the details.
        if (this->has_property(CdlPropertyId_LegalValues)) {
            CdlProperty_ListExpression lexpr = this->get_legal_values();
            data += indent_string + "# Legal values: " +
                CdlInterpreterBody::extend_comment(lexpr->get_original_string(), indentation, 4) + '\n';
        
            std::vector<CdlExpression>::const_iterator expr_i;
            std::vector<std::pair<CdlExpression,CdlExpression> >::const_iterator ranges_i;
            for (expr_i = lexpr->data.begin(); expr_i != lexpr->data.end(); expr_i++) {
                data += CdlInterpreterBody::multiline_comment(follow_expr_references(lexpr, *expr_i), indentation, 4);
            }
            for (ranges_i = lexpr->ranges.begin(); ranges_i != lexpr->ranges.end(); ranges_i++) {
                data += CdlInterpreterBody::multiline_comment(follow_expr_references(lexpr, ranges_i->first), indentation, 4);
                data += CdlInterpreterBody::multiline_comment(follow_expr_references(lexpr, ranges_i->second), indentation, 4);
            }
        }
    
        // If there is a check_proc property, mention this.
        if (this->has_property(CdlPropertyId_CheckProc)) {
            data += indent_string + "# There is a check_proc routine that will check the value.\n";
        }

        // Output all requires properties
        if (this->has_property(CdlPropertyId_Requires)) {
            std::vector<CdlProperty_GoalExpression> requires_goals;
            this->get_requires_goals(requires_goals);
            std::vector<CdlProperty_GoalExpression>::const_iterator expr_i;
            for (expr_i = requires_goals.begin(); expr_i != requires_goals.end(); expr_i++) {
                data += indent_string + "# Requires: " +
                    CdlInterpreterBody::extend_comment((*expr_i)->get_original_string(), indentation, 4) + "\n";

                CdlExpression expr = (*expr_i)->get_expression();
                data += CdlInterpreterBody::multiline_comment(follow_expr_references(*expr_i, expr), indentation, 4);
                CdlEvalContext context(0, this, *expr_i);
                bool active_if_value = false;
                try {
                    active_if_value = (*expr_i)->eval(context);
                } catch(CdlEvalException e) {
                    active_if_value = false;
                } catch(std::bad_alloc) {
                    throw;
                }
                data += indent_string + "#   --> " + (active_if_value ? one : zero) + "\n";
            }
        }

        // Output all dependencies that other entities may have on this one.
        const std::vector<CdlReferrer>& referrers = this->get_referrers();
        if (0 != referrers.size()) {
            data += '\n' + indent_string + "# The following properties are affected by this value\n";
            std::vector<CdlReferrer>::const_iterator ref_i;
            for (ref_i = referrers.begin(); ref_i != referrers.end(); ref_i++) {
            
                CdlNode source = ref_i->get_source();
                CdlProperty source_prop = ref_i->get_source_property();
                std::string prop_id = source_prop->get_property_name();
            
                if ((prop_id == CdlPropertyId_ActiveIf)     ||
                    (prop_id == CdlPropertyId_Calculated)   ||
                    (prop_id == CdlPropertyId_DefaultValue) ||
                    (prop_id == CdlPropertyId_LegalValues)  ||
                    (prop_id == CdlPropertyId_Requires)) {
                
                    data += indent_string + "# " + source->get_class_name() + " " + source->get_name() + "\n";
                    data += indent_string + "#     " + prop_id + ": ";
                    if ((prop_id == CdlPropertyId_Calculated) || (prop_id == CdlPropertyId_DefaultValue)) {
                        CdlProperty_Expression expr = dynamic_cast<CdlProperty_Expression>(source_prop);
                        CYG_ASSERT_CLASSC(expr);
                        data += CdlInterpreterBody::extend_comment(expr->get_original_string(), indentation, 4);
                    } else if (prop_id == CdlPropertyId_LegalValues) {
                        CdlProperty_ListExpression lexpr = dynamic_cast<CdlProperty_ListExpression>(source_prop);
                        CYG_ASSERT_CLASSC(lexpr);
                        data += CdlInterpreterBody::extend_comment(lexpr->get_original_string(), indentation, 4);
                    } else if ((prop_id == CdlPropertyId_ActiveIf) || (prop_id == CdlPropertyId_Requires)) {
                        CdlProperty_GoalExpression gexpr = dynamic_cast<CdlProperty_GoalExpression>(source_prop);
                        CYG_ASSERT_CLASSC(gexpr);
                        data += CdlInterpreterBody::extend_comment(gexpr->get_original_string(), indentation, 4);
                    }
                    data += '\n';
                }
            }
        }
    }

    interp->write_data(chan, data);
    
    CYG_REPORT_RETURN();
}

int
CdlValuableBody::savefile_value_source_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlValuable::savefile_value_source_command");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_PRECONDITION_CLASSC(interp);

    CdlValuable valuable = dynamic_cast<CdlValuable>(interp->get_node());
    CYG_ASSERT_CLASSC(valuable);
    CdlTransaction transaction = interp->get_transaction();
    CYG_ASSERT_CLASSC(transaction);

    CdlValueSource source = CdlValueSource_Invalid;
    if ((2 != argc) || !Cdl::string_to_source(argv[1], source) || !valuable->has_source(transaction, source)) {
        std::string msg = "Invalid value_source command for ";
        msg += valuable->get_class_name() + " " + valuable->get_name() + "\n";
        if (CdlValueSource_Invalid == source) {
            msg += "Expecting one argument, which should \"user\", \"wizard\", \"inferred\" or \"default\"";
        } else {
            msg += "The specified value source is not valid.";
        }
        CdlParse::report_error(interp, "", msg);
    } else {
        valuable->set_source(transaction, source);
    }
    
    return TCL_OK;
}

int
CdlValuableBody::savefile_xxx_value_command(CdlInterpreter interp, int argc, const char* argv[], CdlValueSource source)
{
    CYG_REPORT_FUNCNAME("CdlValuable::savefile_xxx_value_command");
    CYG_REPORT_FUNCARG3XV(interp, argc, source);
    CYG_PRECONDITION_CLASSC(interp);

    CdlValuable valuable = dynamic_cast<CdlValuable>(interp->get_node());
    CYG_ASSERT_CLASSC(valuable);
    CdlTransaction transact = interp->get_transaction();
    CYG_ASSERT_CLASSC(transact);

    bool error = false;
    bool warn  = false;
    std::string msg = "";
    if (CdlValueFlavor_None == valuable->get_flavor()) {
        msg = "Options with flavor \"none\" cannot be modified.";
        error = true;
    } else if (!valuable->is_modifiable()) {
        msg = "This option is not user-modifiable.";
        error = true;
    } else {
        switch(valuable->get_flavor()) {
          case CdlValueFlavor_Bool :
              if (2 != argc) {
                  msg = "Invalid boolean value, expecting 0 or 1";
                  error = true;
              } else {
                  bool x;
                  Cdl::string_to_bool(argv[1], x);
                  valuable->set_enabled(transact, x, source);
              }
              break;
          case CdlValueFlavor_Data :
              if (2 != argc) {
                  msg = "Invalid data value, expecting a single string";
                  error = true;
              } else {
                  valuable->set_value(transact, argv[1], source);
              }
              break;
          case CdlValueFlavor_BoolData:
              if (3 != argc) {
                  msg = "Invalid booldata value, expecting a boolean followed by a string";
                  error = true;
              } else {
                  bool x;
                  Cdl::string_to_bool(argv[1], x);
                  valuable->set_enabled_and_value(transact, x, argv[2], source);
              }
              break;
          default:
            CYG_FAIL("Invalid value flavor detected");
            break;
        }
    }

    if (error || warn) {
        msg = std::string("Invalid value command for ") + valuable->get_class_name() + " " + valuable->get_name() + "\n"
            + msg;
        if (error) {
            CdlParse::report_error(interp, "", msg);
        } else {
            CdlParse::report_warning(interp, "", msg);
        }
    }

    return TCL_OK;
}

int
CdlValuableBody::savefile_user_value_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlValuable::savefile_user_value_command");
    int result = CdlValuableBody::savefile_xxx_value_command(interp, argc, argv, CdlValueSource_User);
    CYG_REPORT_RETURN();
    return result;
}

int
CdlValuableBody::savefile_wizard_value_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlValuable::savefile_wizard_value_command");
    int result = CdlValuableBody::savefile_xxx_value_command(interp, argc, argv, CdlValueSource_Wizard);
    CYG_REPORT_RETURN();
    return result;
}

int
CdlValuableBody::savefile_inferred_value_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlValuable::savefile_inferred_value_command");
    int result = CdlValuableBody::savefile_xxx_value_command(interp, argc, argv, CdlValueSource_Inferred);
    CYG_REPORT_RETURN();
    return result;
}

//}}}
