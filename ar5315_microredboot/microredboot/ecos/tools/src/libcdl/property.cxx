//{{{  Banner                           

//============================================================================
//
//     property.cxx
//
//     Implementation of the CdlProperty class and derived classes.
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002 Bart Veer
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
//============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   bartv
// Contact(s):  bartv
// Date:        1999/01/29
// Version:     0.02
// Description: The CdlProperty class is used to hold the bulk of the
//              actual data in a CDL script. The CdlOption and other
//              higher-level classes are essentially just named containers
//              of properties.
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

// <cdl.hxx> defines everything implemented in this module.
// It implicitly supplies <string>, <vector> and <map> because
// the class definitions rely on these headers.
#include <cdlcore.hxx>

//}}}

//{{{  Statics                          

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlPropertyBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_MinimalBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_StringBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_TclCodeBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_ReferenceBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_StringVectorBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_ExpressionBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_ListExpressionBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlProperty_GoalExpressionBody);

//}}}
//{{{  CdlProperty base class           

// ----------------------------------------------------------------------------

CdlPropertyBody::CdlPropertyBody(CdlNode node, std::string name_arg, int argc, const char* argv_arg[],
                                 std::vector<std::pair<std::string,std::string> >& options_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty:: constructor");
    CYG_REPORT_FUNCARG3XV(this, node, argc);
    CYG_PRECONDITIONC(argc > 0);
    CYG_PRECONDITION_CLASSC(node);

    name = name_arg;
    argv.push_back(CdlParse::get_tcl_cmd_name(argv_arg[0]));
    for (int i = 1; i < argc; i++) {
        argv.push_back(argv_arg[i]);
    }
    options = options_arg;
    node->properties.push_back(this);
    cdlpropertybody_cookie = CdlPropertyBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlPropertyBody::~CdlPropertyBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();
    
    cdlpropertybody_cookie      = CdlPropertyBody_Invalid;
    name                        = "";
    argv.clear();
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

std::string
CdlPropertyBody::get_property_name(void) const
{
    CYG_REPORT_FUNCNAME("CdlProperty::get_property_id");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return name;
}

int
CdlPropertyBody::get_argc(void) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlProperty::get_argc", "argc %d");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    int result = argv.size();
    CYG_REPORT_RETVAL(result);
    return result;
}

const std::vector<std::string>&
CdlPropertyBody::get_argv(void) const
{
    CYG_REPORT_FUNCNAME("CdlProperty::get_argv");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return argv;
}

const std::vector<std::pair<std::string,std::string> >&
CdlPropertyBody::get_options() const
{
    CYG_REPORT_FUNCNAME("CdlProperty::get_options");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return options;
}

bool
CdlPropertyBody::has_option(std::string name) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlProperty::has_option", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    std::vector<std::pair<std::string,std::string> >::const_iterator opt_i;
    for (opt_i = options.begin(); opt_i != options.end(); opt_i++) {
        if (name == opt_i->first) {
            result = true;
            break;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlPropertyBody::get_option(std::string name) const
{
    CYG_REPORT_FUNCNAME("CdlProperty::get_option");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = "";
    std::vector<std::pair<std::string,std::string> >::const_iterator opt_i;
    for (opt_i = options.begin(); opt_i != options.end(); opt_i++) {
        if (name == opt_i->first) {
            result = opt_i->second;
            break;
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
// Handling updates. This is a virtual function. The default
// implementation does nothing because not all properties contain
// references to other CDL entities.

void
CdlPropertyBody::update(CdlTransaction transact, CdlNode source, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlProperty::update");
    CYG_REPORT_FUNCARG5XV(this, transact, source, dest, change);
    CYG_PRECONDITION_THISC();

    CYG_UNUSED_PARAM(CdlTransaction, transact);
    CYG_UNUSED_PARAM(CdlNode, source);
    CYG_UNUSED_PARAM(CdlNode, dest);
    CYG_UNUSED_PARAM(CdlUpdate, change);
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
bool
CdlPropertyBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlPropertyBody_Magic != cdlpropertybody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    CYG_UNUSED_PARAM(cyg_assert_class_zeal, zeal);
    return true;
}

//}}}
//{{{  CdlProperty_Minimal class        

// ----------------------------------------------------------------------------

CdlProperty_Minimal
CdlProperty_MinimalBody::make(CdlNode node_arg, std::string name_arg, int argc_arg, const char* argv_arg[],
                              std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_MinimalBody(node_arg, name_arg, argc_arg, argv_arg, options_arg);
}

CdlProperty_MinimalBody::CdlProperty_MinimalBody(CdlNode node_arg, std::string name_arg, int argc_arg, const char* argv_arg[],
                                                 std::vector<std::pair<std::string,std::string> >& options_arg)
    : inherited(node_arg, name_arg, argc_arg, argv_arg, options_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_Minimal:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);

    cdlproperty_minimalbody_cookie = CdlProperty_MinimalBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_MinimalBody::~CdlProperty_MinimalBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_Minimal:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_minimalbody_cookie = CdlProperty_MinimalBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

bool
CdlProperty_MinimalBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_MinimalBody_Magic != cdlproperty_minimalbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited::check_this(zeal);
}

//}}}
//{{{  CdlProperty_String class         

// ----------------------------------------------------------------------------

CdlProperty_String
CdlProperty_StringBody::make(CdlNode node_arg, std::string name_arg, std::string value_arg, int argc_arg, const char* argv_arg[],
                             std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_StringBody(node_arg, name_arg, value_arg, argc_arg, argv_arg, options_arg);
}

CdlProperty_StringBody::CdlProperty_StringBody(CdlNode node_arg, std::string name_arg, std::string value_arg,
                                               int argc_arg, const char* argv_arg[],
                                               std::vector<std::pair<std::string,std::string> >& options_arg)
    : inherited(node_arg, name_arg, argc_arg, argv_arg, options_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_String:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);

    data = value_arg;
    cdlproperty_stringbody_cookie = CdlProperty_StringBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_StringBody::~CdlProperty_StringBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_String:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_stringbody_cookie = CdlProperty_StringBody_Invalid;
    data = "";
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

std::string
CdlProperty_StringBody::get_string(void) const
{
    CYG_REPORT_FUNCNAME("CdlProperty_String::get_string");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return data;
}

bool
CdlProperty_StringBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_StringBody_Magic != cdlproperty_stringbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited::check_this(zeal);
}

//}}}`
//{{{  CdlProperty_TclCode class        

// ----------------------------------------------------------------------------

CdlProperty_TclCode
CdlProperty_TclCodeBody::make(CdlNode node_arg, std::string name_arg, cdl_tcl_code code_arg,
                              int argc_arg, const char* argv_arg[],
                              std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_TclCodeBody(node_arg, name_arg, 0, code_arg, argc_arg, argv_arg, options_arg);
}

CdlProperty_TclCode
CdlProperty_TclCodeBody::make(CdlNode node_arg, std::string name_arg, cdl_int number_arg, cdl_tcl_code code_arg,
                              int argc_arg, const char* argv_arg[],
                              std::vector<std::pair<std::string,std::string> >& options_arg)                              
{
    return new CdlProperty_TclCodeBody(node_arg, name_arg, number_arg, code_arg, argc_arg, argv_arg, options_arg);
}


CdlProperty_TclCodeBody::CdlProperty_TclCodeBody(CdlNode node_arg, std::string name_arg,
                                                 cdl_int number_arg, cdl_tcl_code code_arg,
                                                 int argc_arg, const char* argv_arg[],
                                                 std::vector<std::pair<std::string,std::string> >& options_arg)
    : inherited(node_arg, name_arg, argc_arg, argv_arg, options_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_TclCode:: constructor");
    CYG_REPORT_FUNCARG2("this %p, number_arg %d", this, number_arg);

    code   = code_arg;
    number = number_arg;
    cdlproperty_tclcodebody_cookie = CdlProperty_TclCodeBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_TclCodeBody::~CdlProperty_TclCodeBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_TclCode:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_tclcodebody_cookie = CdlProperty_TclCodeBody_Invalid;
    code = cdl_tcl_code("");
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

const cdl_tcl_code&
CdlProperty_TclCodeBody::get_code(void) const
{
    CYG_REPORT_FUNCNAME("CdlProperty_TclCode::get_code");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return code;
}

cdl_int
CdlProperty_TclCodeBody::get_number(void) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlProperty_TclCode::get_number", "result %d");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdl_int result = number;
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlProperty_TclCodeBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_TclCodeBody_Magic != cdlproperty_tclcodebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited::check_this(zeal);
}

//}}}
//{{{  CdlProperty_StringVector class   

// ----------------------------------------------------------------------------

CdlProperty_StringVector
CdlProperty_StringVectorBody::make(CdlNode node_arg, std::string name_arg, const std::vector<std::string>& data_arg,
                                   int argc_arg, const char* argv_arg[],
                                   std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_StringVectorBody(node_arg, name_arg, data_arg, argc_arg, argv_arg, options_arg);
}

CdlProperty_StringVectorBody::CdlProperty_StringVectorBody(CdlNode node_arg, std::string name_arg,
                                                           const std::vector<std::string>& data_arg,
                                                           int argc_arg, const char* argv_arg[],
                                                           std::vector<std::pair<std::string,std::string> >& options_arg)
    : inherited(node_arg, name_arg, argc_arg, argv_arg, options_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_StringVector:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    
    data = data_arg;
    cdlproperty_stringvectorbody_cookie = CdlProperty_StringVectorBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_StringVectorBody::~CdlProperty_StringVectorBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_StringVector:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_stringvectorbody_cookie = CdlProperty_StringVectorBody_Invalid;
    data.clear();
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

std::string
CdlProperty_StringVectorBody::get_first_string(void) const
{
    CYG_REPORT_FUNCNAME("CdlProperty_StringVector::get_first_string");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    std::string result;
    if (0 == data.size()) {
        result = "";
    } else {
        result = data[0];
    }
    CYG_REPORT_RETURN();
    return result;
}

unsigned int
CdlProperty_StringVectorBody::get_number_of_strings() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlProperty_StringVector::get_number_of_strings", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    unsigned int result = data.size();
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlProperty_StringVectorBody::get_string(unsigned int index) const
{
    CYG_REPORT_FUNCNAME("CdlProperty_StringVector::get_string");
    CYG_REPORT_FUNCARG2XV(this, index);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(index < data.size());

    std::string result = data[index];
    CYG_REPORT_RETURN();
    return result;
}

const std::vector<std::string>&
CdlProperty_StringVectorBody::get_strings(void) const
{
    CYG_REPORT_FUNCNAME("CdlProperty_StringVector::get_strings");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return data;
}

bool
CdlProperty_StringVectorBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_StringVectorBody_Magic != cdlproperty_stringvectorbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited::check_this(zeal);
}

//}}}`
//{{{  CdlProperty_Reference class      

// ----------------------------------------------------------------------------
// This is pretty simple since most of the functionality is provided by the
// reference class.

CdlProperty_Reference
CdlProperty_ReferenceBody::make(CdlNode node_arg, std::string name_arg, std::string ref_arg, CdlUpdateHandler update_handler,
                                int argc_arg, const char* argv_arg[],
                                std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_ReferenceBody(node_arg, name_arg, ref_arg, update_handler, argc_arg, argv_arg, options_arg);
}

CdlProperty_ReferenceBody::CdlProperty_ReferenceBody(CdlNode node_arg, std::string name_arg, std::string ref_arg,
                                                     CdlUpdateHandler update_handler_arg,
                                                     int argc_arg, const char* argv_arg[],
                                                     std::vector<std::pair<std::string,std::string> >& options_arg)
    : CdlPropertyBody(node_arg, name_arg, argc_arg, argv_arg, options_arg),
      CdlReference(ref_arg),
      update_handler(update_handler_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_Reference:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);

    cdlproperty_referencebody_cookie = CdlProperty_ReferenceBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_ReferenceBody::~CdlProperty_ReferenceBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_Reference:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_referencebody_cookie = CdlProperty_ReferenceBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Reference handling. It is useful at this level to cope with the
// four cases of Loaded, Unloaded, Created, and Destroyed. In addition
// the property-specific update handler needs to be invoked.

void
CdlProperty_ReferenceBody::update(CdlTransaction transact, CdlNode source, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlProperty_Reference::update");
    CYG_REPORT_FUNCARG5XV(this, transact, source, dest, change);
    CYG_PRECONDITION_THISC();

    switch(change) {
        
      case CdlUpdate_Loaded :
      {
        // The source has just been loaded, try to resolve the reference.
        // Note that e.g. the parent property allow for a reference to ""
        // The necessary validation will have happened during parsing.
        CYG_ASSERTC(0 == dest);
        CdlToplevel toplevel = source->get_toplevel();
        std::string dest_name = get_destination_name();
        if ("" != dest_name) {
            dest = toplevel->lookup(dest_name);
            if (0 == dest) {
                CdlConflict_UnresolvedBody::make(transact, source, this, get_destination_name());
            } else {
                bind(source, this, dest);
            }
        }
        break;
      }

      case CdlUpdate_Unloading:
      {
        // The source is being unloaded. If the reference is currently bound,
        // unbind it. There is no point in creating a new conflict object.
        CYG_ASSERTC(0 == dest);
        dest = get_destination();
        if (0 != dest) {
            unbind(source, this);
        }
        break;
      }
      
      case CdlUpdate_Created:
      {
        // There is an existing conflict object, but the destination has
        // just been created. The old conflict object should be eliminated,
        // and the reference can now be bound.
        CYG_ASSERT_CLASSC(dest);
        
        bind(source, this, dest);

        CdlConflict conflict = transact->get_structural_conflict(source, this, &CdlConflict_UnresolvedBody::test);
        CYG_ASSERTC(0 != conflict);
        transact->clear_conflict(conflict);
        break;
      }
      case CdlUpdate_Destroyed :
      {
        // The destination is about to be destroyed. Eliminate the existing
        // binding and create a new conflict object.
        CYG_ASSERT_CLASSC(dest);
        unbind(source, this);
        CdlConflict_UnresolvedBody::make(transact, source, this, get_destination_name());
        break;
      }

      // Init, ValueChange and ActiveChange are of no interest.
      default:
        break;
    }

    (*update_handler)(transact, source, this, dest, change);
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
bool
CdlProperty_ReferenceBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_ReferenceBody_Magic != cdlproperty_referencebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited_property::check_this(zeal) && inherited_reference::check_this(zeal);
}

//}}}
//{{{  CdlProperty_Expression class     

// ----------------------------------------------------------------------------
// This is pretty simple since most of the functionality is provided by the
// base expression class.

CdlProperty_Expression
CdlProperty_ExpressionBody::make(CdlNode node_arg, std::string name_arg, CdlExpression expr_arg,
                                 CdlUpdateHandler update_handler_arg,
                                 int argc_arg, const char* argv_arg[],
                                 std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_ExpressionBody(node_arg, name_arg, expr_arg, update_handler_arg, argc_arg, argv_arg, options_arg);
}

CdlProperty_ExpressionBody::CdlProperty_ExpressionBody(CdlNode node_arg, std::string name_arg, CdlExpression expr_arg,
                                                       CdlUpdateHandler update_handler_arg,
                                                       int argc_arg, const char* argv_arg[],
                                                       std::vector<std::pair<std::string,std::string> >& options_arg)
    : CdlPropertyBody(node_arg, name_arg, argc_arg, argv_arg, options_arg),
      CdlExpressionBody(*expr_arg),
      update_handler(update_handler_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_Expression:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    
    cdlproperty_expressionbody_cookie = CdlProperty_ExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_ExpressionBody::~CdlProperty_ExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_Expression:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_expressionbody_cookie = CdlProperty_ExpressionBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

void
CdlProperty_ExpressionBody::update(CdlTransaction transact, CdlNode source, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlProperty_Expression::update");
    CYG_REPORT_FUNCARG5XV(this, transact, source, dest, change);
    CYG_PRECONDITION_THISC();

    // The CdlExpression update() member will take care of binding
    // and unbinding, as needed.
    if ((change & (CdlUpdate_Loaded | CdlUpdate_Unloading | CdlUpdate_Created | CdlUpdate_Destroyed)) != 0) {
        (void) CdlExpressionBody::update(transact, source, this, dest, change);
    }

    // Now invoke the per-property update handler to re-evaluate the
    // expression etc., as needed
    (*update_handler)(transact, source, this, dest, change);
    
    CYG_REPORT_RETURN();
}

bool
CdlProperty_ExpressionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_ExpressionBody_Magic != cdlproperty_expressionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited_property::check_this(zeal) && inherited_expression::check_this(zeal);
}

//}}}
//{{{  CdlProperty_ListExpression class 

// ----------------------------------------------------------------------------
// This is pretty simple since most of the functionality is provided by the
// base expression class.

CdlProperty_ListExpression
CdlProperty_ListExpressionBody::make(CdlNode node_arg, std::string name_arg, CdlListExpression expr_arg,
                                     CdlUpdateHandler update_handler_arg, 
                                     int argc_arg, const char* argv_arg[],
                                     std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_ListExpressionBody(node_arg, name_arg, expr_arg, update_handler_arg,
                                              argc_arg, argv_arg, options_arg);
}

CdlProperty_ListExpressionBody::CdlProperty_ListExpressionBody(CdlNode node_arg, std::string name_arg,
                                                               CdlListExpression expr_arg,
                                                               CdlUpdateHandler update_handler_arg,
                                                               int argc_arg, const char* argv_arg[],
                                                               std::vector<std::pair<std::string,std::string> >& options_arg)
    : CdlPropertyBody(node_arg, name_arg, argc_arg, argv_arg, options_arg),
      CdlListExpressionBody(*expr_arg),
      update_handler(update_handler_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_ListExpression:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    
    cdlproperty_listexpressionbody_cookie = CdlProperty_ListExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_ListExpressionBody::~CdlProperty_ListExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_ListExpression:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_listexpressionbody_cookie = CdlProperty_ListExpressionBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

void
CdlProperty_ListExpressionBody::update(CdlTransaction transact, CdlNode source, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlProperty_ListExpression::update");
    CYG_REPORT_FUNCARG4XV(this, source, dest, change);
    CYG_PRECONDITION_THISC();

    if ((change & (CdlUpdate_Loaded | CdlUpdate_Unloading | CdlUpdate_Created | CdlUpdate_Destroyed)) != 0) {
        bool handled = CdlListExpressionBody::update(transact, source, this, dest, change);
        CYG_UNUSED_PARAM(bool, handled);
        CYG_ASSERTC(handled);
    }

    // Now invoke the per-property update handler to re-evaluate
    // the lexpr as appropriate.
    (*update_handler)(transact, source, this, dest, change);
    
    CYG_REPORT_RETURN();
}

bool
CdlProperty_ListExpressionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_ListExpressionBody_Magic != cdlproperty_listexpressionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited_property::check_this(zeal) && inherited_expression::check_this(zeal);
}

//}}}
//{{{  CdlProperty_GoalExpression class 

// ----------------------------------------------------------------------------
// This is pretty simple since most of the functionality is provided by the
// base expression class.

CdlProperty_GoalExpression
CdlProperty_GoalExpressionBody::make(CdlNode node_arg, std::string name_arg, CdlGoalExpression expr_arg,
                                     CdlUpdateHandler update_handler_arg,
                                     int argc_arg, const char* argv_arg[],
                                     std::vector<std::pair<std::string,std::string> >& options_arg)
{
    return new CdlProperty_GoalExpressionBody(node_arg, name_arg, expr_arg, update_handler_arg,
                                              argc_arg, argv_arg, options_arg);
}

CdlProperty_GoalExpressionBody::CdlProperty_GoalExpressionBody(CdlNode node_arg, std::string name_arg,
                                                               CdlGoalExpression expr_arg,
                                                               CdlUpdateHandler update_handler_arg,
                                                               int argc_arg, const char* argv_arg[],
                                                               std::vector<std::pair<std::string,std::string> >& options_arg)
    : CdlPropertyBody(node_arg, name_arg, argc_arg, argv_arg, options_arg),
      CdlGoalExpressionBody(*expr_arg),
      update_handler(update_handler_arg)
{
    CYG_REPORT_FUNCNAME("CdlProperty_GoalExpression:: constructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    
    cdlproperty_goalexpressionbody_cookie = CdlProperty_GoalExpressionBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlProperty_GoalExpressionBody::~CdlProperty_GoalExpressionBody()
{
    CYG_REPORT_FUNCNAME("CdlProperty_GoalExpression:: destructor");
    CYG_REPORT_FUNCARG1("this %p", this);
    CYG_PRECONDITION_THISC();

    cdlproperty_goalexpressionbody_cookie = CdlProperty_GoalExpressionBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();

    CYG_REPORT_RETURN();
}

void
CdlProperty_GoalExpressionBody::update(CdlTransaction transact, CdlNode source, CdlNode dest, CdlUpdate change)
{
    CYG_REPORT_FUNCNAME("CdlProperty_GoalExpression::update");
    CYG_REPORT_FUNCARG4XV(this, source, dest, change);
    CYG_PRECONDITION_THISC();

    // The CdlExpression update() member will take care of binding and
    // unbinding, as needed.
    if ((change & (CdlUpdate_Loaded | CdlUpdate_Unloading | CdlUpdate_Created | CdlUpdate_Destroyed)) != 0) {
        CdlExpression expr = get_expression();
        bool handled = expr->update(transact, source, this, dest, change);
        CYG_UNUSED_PARAM(bool, handled);
        CYG_ASSERTC(handled);
    }

    // Now invoke the per-property update handler to re-evaluate
    // the gexpr as appropriate
    (*update_handler)(transact, source, this, dest, change);
    
    CYG_REPORT_RETURN();
}

bool
CdlProperty_GoalExpressionBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlProperty_GoalExpressionBody_Magic != cdlproperty_goalexpressionbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return inherited_property::check_this(zeal) && inherited_expression::check_this(zeal);
}

//}}}
