//{{{  Banner                                                   

//============================================================================
//
//      interp.cxx
//
//      Provide access to Tcl interpreters
//
//============================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2004 eCosCentric Limited
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
// Author(s):	bartv
// Contact(s):	bartv
// Date:	1999/01/20
// Version:	0.02
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
// the class definitions rely on these headers. It also brings
// in <tcl.h>
#include <cdlcore.hxx>

//}}}

//{{{  Statics                                                  

// ----------------------------------------------------------------------------
// This key is used for accessing AssocData in the Tcl interpreters,
// specifically the CdlInterpreter object.
char* CdlInterpreterBody::cdlinterpreter_assoc_data_key = "__cdlinterpreter";

CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlInterpreterBody);

//}}}
//{{{  CdlInterpreter:: creation                                

// ----------------------------------------------------------------------------
// Default constructor. This will only get invoked via the make() static
// member.

CdlInterpreterBody::CdlInterpreterBody(Tcl_Interp* tcl_interp_arg)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter:: default constructor");
    CYG_REPORT_FUNCARG2XV(this, tcl_interp_arg);
    CYG_PRECONDITIONC(0 != tcl_interp_arg);
    
    tcl_interp          = tcl_interp_arg;
    owns_interp         = false;
    parent              = 0;
    toplevel            = 0;
    transaction         = 0;
    loadable            = 0;
    container           = 0;
    node                = 0;
    context             = "";
    error_fn_ptr        = 0;
    warning_fn_ptr      = 0;
    current_commands    = 0;
    cdl_result          = false;
    
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    cdlinterpreterbody_cookie   = CdlInterpreterBody_Magic;

    Tcl_SetAssocData(tcl_interp, cdlinterpreter_assoc_data_key, 0, static_cast<ClientData>(this));

    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Create a new CDL interpreter. The underlying Tcl interpreter can be
// supplied by the caller, or else a suitable interpreter will be created
// with default settings. This default interpreter will only support Tcl,
// not Tk. There is no call to any AppInit() function, no support for
// autoloading packages, the "unknown" command is not implemented, and
// no command files will be read in.
//
// It is convenient to provide immediate access to two Tcl variables,
// cdl_version and cdl_interactive.

CdlInterpreter
CdlInterpreterBody::make(Tcl_Interp* tcl_interp_arg)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::make", "interpreter %p");
    CYG_REPORT_FUNCARG1XV(tcl_interp_arg);

    Tcl_Interp* tcl_interp = tcl_interp_arg;
    if (0 == tcl_interp) {
        tcl_interp = Tcl_CreateInterp();
        if (0 == tcl_interp) {
            throw std::bad_alloc();
        }
    } else {
        // Make sure that this Tcl interpreter is not already used
        // for another CdlInterpreter object.
        ClientData tmp = Tcl_GetAssocData(tcl_interp, cdlinterpreter_assoc_data_key, 0);
        if (0 != tmp) {
            CYG_FAIL("Attempt to use a Tcl interpreter for multiple CDL interpreters");
            throw std::bad_alloc();
        }
    }
    
    CdlInterpreter result = 0;
    try {
        result = new CdlInterpreterBody(tcl_interp);
        
        std::string version = Cdl::get_library_version();
        if (0 == Tcl_SetVar(tcl_interp, "cdl_version", CDL_TCL_CONST_CAST(char*,version.c_str()), TCL_GLOBAL_ONLY)) {
            throw std::bad_alloc();
        }
        if (0 == Tcl_SetVar(tcl_interp, "cdl_interactive", CDL_TCL_CONST_CAST(char*, (Cdl::is_interactive() ? "1" : "0")),
                            TCL_GLOBAL_ONLY)) {
            throw std::bad_alloc();
        }
    }
    catch(std::bad_alloc) {
        if (0 == tcl_interp_arg) {
            Tcl_DeleteInterp(tcl_interp);
        }
        throw;
    }
    if (0 == tcl_interp_arg) {
        result->owns_interp     = true;
    }
    CYG_POSTCONDITION_CLASSC(result);
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Given a toplevel and a loadable, create a new slave interpreter
// for that loadable. There should be master interpreter associated
// with the toplevel already.
//
// FIXME: do slave interpreters automatically see cdl_version and
// cdl_interactive?

CdlInterpreter
CdlInterpreterBody::create_slave(CdlLoadable loadable_arg, bool safe)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::create_slave", "slave %p");
    CYG_REPORT_FUNCARG3XV(this, loadable_arg, safe);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION(0 == parent, "slave interpreters cannot be created inside slaves");
    CYG_PRECONDITION(0 != toplevel, "CDL's slave interpreters need an associated toplevel");
    CYG_PRECONDITION_CLASSC(loadable_arg);

    // Slave interpreters need a name. Use a counter to create them uniquely.
    static cdl_int      next_slave = 1;
    std::string         slave_name;
    Cdl::integer_to_string(next_slave++, slave_name);
    slave_name = "slave" + slave_name;

    // FIXME: creating a slave that is not safe appears to fail.
#if 0    
    Tcl_Interp* slave = Tcl_CreateSlave(interp, CDL_TCL_CONST_CAST(char*, slave_name.c_str()), safe);
#else
    Tcl_Interp* slave = Tcl_CreateInterp();
#endif
    if (0 == slave) {
        throw std::bad_alloc();
    }
 
    CdlInterpreter result = 0;
    try {
        result = new CdlInterpreterBody(slave);
    }
    catch(std::bad_alloc) {
        Tcl_DeleteInterp(slave);
        throw;
    }
    result->owns_interp = true;
#if 0    
    try {
        slaves.push_back(result);
    }
    catch(std::bad_alloc) {
        delete result;
        throw;
    }
#endif
    
    result->parent      = this;
    result->set_toplevel(toplevel);
    result->loadable    = loadable_arg;
    result->set_variable("cdl_version", get_variable("cdl_version"));
    result->set_variable("cdl_interactive", get_variable("cdl_interactive"));
    
    CYG_POSTCONDITION_CLASSC(result);
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Given an existing interpreter, turn it into a safe one. This is a one-way
// transformation.
void
CdlInterpreterBody::make_safe(void)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::make_safe");
    CYG_PRECONDITION_THISC();

    if (0 != Tcl_MakeSafe(tcl_interp)) {
        throw std::bad_alloc();
    }
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlInterpreter:: destructor                              

// ----------------------------------------------------------------------------
// Default destructor. It is necessary to worry about any slave
// interpreters, but otherwise there are no complications.

CdlInterpreterBody::~CdlInterpreterBody()
{
    CYG_REPORT_FUNCNAME("CdlInterpreter:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    
    cdlinterpreterbody_cookie   = CdlInterpreterBody_Invalid;
    parent                      = 0;
    toplevel                    = 0;
    transaction                 = 0;
    loadable                    = 0;
    container                   = 0;
    node                        = 0;
    context                     = "";
    error_fn_ptr                = 0;
    warning_fn_ptr              = 0;
    current_commands            = 0;
    cdl_result                  = false;
    
    // Make sure slave interpreters get deleted before the current one
    for (std::vector<CdlInterpreter>::iterator i = slaves.begin(); i != slaves.end(); i++) {
        delete *i;
        *i = 0;
    }

    Tcl_DeleteAssocData(tcl_interp, cdlinterpreter_assoc_data_key);
    if (owns_interp) {
        Tcl_DeleteInterp(tcl_interp);
    }
    owns_interp = false;
    tcl_interp  = 0;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlInterpreter:: check_this()                            

// ----------------------------------------------------------------------------
// check_this().

bool
CdlInterpreterBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlInterpreterBody_Magic != cdlinterpreterbody_cookie)
        return false;

    CYGDBG_MEMLEAK_CHECKTHIS();
    
    switch(zeal) {
      case cyg_system_test  :
      case cyg_extreme      :
          if (slaves.size() > 0) {
              for (std::vector<CdlInterpreter>::const_iterator i = slaves.begin(); i != slaves.end(); i++) {
                  if (!(*i)->check_this(cyg_quick)) {
                      return false;
                  }
              }
          }
      case cyg_thorough     :
          if ((0 != toplevel) && !toplevel->check_this(cyg_quick)) {
              return false;
          }
          if ((0 != transaction) && !transaction->check_this(cyg_quick)) {
              return false;
          }
          if ((0 != loadable) && !loadable->check_this(cyg_quick)) {
              return false;
          }
          if ((0 != container) && !container->check_this(cyg_quick)) {
              return false;
          }
          if ((0 != node) && !node->check_this(cyg_quick)) {
              return false;
          }
      case cyg_quick        :
          // For now only the toplevel interpreter should have slaves.
          if ((0 != parent) && (slaves.size() > 0)) {
              return false;
          }
          if( 0 == tcl_interp) {
              return false;
          }
      case cyg_trivial      :
      case cyg_none         :
          break;
    }
    return true;
}

//}}}
//{{{  CdlInterpreter:: set_toplevel() etc.                     

// ----------------------------------------------------------------------------
// Keep track of the current toplevel, container, etc. This gives commands
// added to the Tcl interpreter a simple way of figuring out the current
// state of the world so that properties get added to the right node, etc.
//
// set_toplevel() should only be called once, for the master interpreter
// associated with a toplevel. All slave interpreters inherit this value.
//
// There is no set_loadable(), instead the loadable field is filled in
// by create_slave() and cannot be changed.

CdlToplevel
CdlInterpreterBody::get_toplevel() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_toplevel", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlToplevel result = toplevel;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlTransaction
CdlInterpreterBody::get_transaction() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_transaction", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlTransaction result = transaction;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlLoadable
CdlInterpreterBody::get_loadable() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter:get_loadable", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlLoadable result = loadable;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlContainer
CdlInterpreterBody::get_container() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_container", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlContainer result = container;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlNode
CdlInterpreterBody::get_node() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_node", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlNode result = node;
    CYG_REPORT_RETVAL(result);
    return result;
}

std::string
CdlInterpreterBody::get_context() const
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::get_context");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return context;
}

CdlDiagnosticFnPtr
CdlInterpreterBody::get_error_fn_ptr() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_error_fn_ptr", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlDiagnosticFnPtr result = error_fn_ptr;
    CYG_REPORT_RETVAL(result);
    return result;
}

CdlDiagnosticFnPtr
CdlInterpreterBody::get_warning_fn_ptr() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_warning_fn_ptr", "result %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlDiagnosticFnPtr result = warning_fn_ptr;
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::set_toplevel(CdlToplevel new_toplevel)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::set_toplevel");
    CYG_REPORT_FUNCARG2XV(this, new_toplevel);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION(0 == toplevel, "changing toplevels is not allowed");
    CYG_PRECONDITION_CLASSC(new_toplevel);

    toplevel = new_toplevel;
    CYG_REPORT_RETURN();
}

void
CdlInterpreterBody::set_transaction(CdlTransaction new_transaction)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::set_transaction");
    CYG_REPORT_FUNCARG2XV(this, new_transaction);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_ZERO_OR_CLASSC(new_transaction);

    transaction = new_transaction;
    CYG_REPORT_RETURN();
}

CdlContainer
CdlInterpreterBody::push_container(CdlContainer new_container)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::push_container", "result %p");
    CYG_REPORT_FUNCARG2XV(this, new_container);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(new_container);

    CdlContainer result = container;
    container = new_container;
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::pop_container(CdlContainer old_container)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::pop_container");
    CYG_REPORT_FUNCARG2XV(this, old_container);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_ZERO_OR_CLASSC(old_container);
    CYG_PRECONDITIONC(0 != container);

    container = old_container;

    CYG_REPORT_RETURN();
}

CdlNode
CdlInterpreterBody::push_node(CdlNode new_node)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::push_node", "result %p");
    CYG_REPORT_FUNCARG2XV(this, new_node);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(new_node);

    CdlNode result = node;
    node = new_node;
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::pop_node(CdlNode old_node)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::pop_node");
    CYG_REPORT_FUNCARG2XV(this, old_node);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 != node);
    CYG_PRECONDITION_ZERO_OR_CLASSC(old_node);

    node = old_node;

    CYG_REPORT_RETURN();
}

std::string
CdlInterpreterBody::push_context(std::string new_context)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::push_context");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != new_context);

    std::string result = context;
    context = new_context;
    return result;
}

void
CdlInterpreterBody::pop_context(std::string old_context)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::pop_context");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != context);

    context = old_context;

    CYG_REPORT_RETURN();
}

CdlDiagnosticFnPtr
CdlInterpreterBody::push_error_fn_ptr(CdlDiagnosticFnPtr new_fn_ptr)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::push_error_fn_ptr", "result %p");
    CYG_REPORT_FUNCARG2XV(this, new_fn_ptr);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 != new_fn_ptr);

    CdlDiagnosticFnPtr result = error_fn_ptr;
    error_fn_ptr = new_fn_ptr;
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::pop_error_fn_ptr(CdlDiagnosticFnPtr old_fn_ptr)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::pop_error_fn_ptr");
    CYG_REPORT_FUNCARG2XV(this, old_fn_ptr);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 != error_fn_ptr);

    error_fn_ptr = old_fn_ptr;

    CYG_REPORT_RETURN();
}

CdlDiagnosticFnPtr
CdlInterpreterBody::push_warning_fn_ptr(CdlDiagnosticFnPtr new_fn_ptr)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::push_warning_fn_ptr", "result %p");
    CYG_REPORT_FUNCARG2XV(this, new_fn_ptr);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 != new_fn_ptr);

    CdlDiagnosticFnPtr result = warning_fn_ptr;
    warning_fn_ptr = new_fn_ptr;
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::pop_warning_fn_ptr(CdlDiagnosticFnPtr old_fn_ptr)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::pop_warning_fn_ptr");
    CYG_REPORT_FUNCARG2XV(this, old_fn_ptr);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC(0 != warning_fn_ptr);

    warning_fn_ptr = old_fn_ptr;

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlInterpreter:: get information                         

// ----------------------------------------------------------------------------
// Get hold of the underlying Tcl interpreter. This makes it easier to
// use miscellaneous Tcl library facilities such as Tcl_SplitList()
Tcl_Interp*
CdlInterpreterBody::get_tcl_interpreter(void) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_tcl_interpreter", "interpreter %p");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    Tcl_Interp* result = tcl_interp;
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  CdlInterpreter:: eval()                                  

// ----------------------------------------------------------------------------
// Evaluate a Cdl script held in a string. The result of this evaluation, 
// e.g. TCL_OK, is returned directly. The string result is made available
// in an in-out parameter.
//
// According to the spec the underlying Tcl_Eval() routine needs to be able
// to make temporary changes to the script, so the latter must be held in
// writable memory. This requires a copy operation.

int
CdlInterpreterBody::eval(std::string script, std::string& str_result)
{
    CYG_REPORT_FUNCNAMETYPE("CdInterpreter::eval", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    int result  = TCL_OK;
    int size    = script.size();

    // Distinguish between results set by the Tcl interpreter and results
    // set by CDL-related commands running in that interpreter.
    cdl_result = false;
    
    if (size < 2048) {
        char buf[2048];
        script.copy(buf, size, 0);
        buf[size] = '\0';
        result = Tcl_Eval(tcl_interp, buf);
    } else {
        char* buf = static_cast<char*>(malloc(script.size() + 1));
        if (0 == buf) {
            this->set_result(CdlParse::construct_diagnostic(this, "internal error", "", "Out of memory"));
            result = TCL_ERROR;
        } else {
            script.copy(buf, size, 0);
            buf[size] = '\0';
            result = Tcl_Eval(tcl_interp, buf);
            free(buf);
        }
    }

    // The distinction between TCL_OK and TCL_RETURN is probably not worth
    // worrying about.
    if (TCL_RETURN == result) {
        result = TCL_OK;
    }
    
    // If we have an error condition that was raised by the Tcl
    // interpreter rather than by the library, it needs to be
    // raised up to the library level. That way the error count
    // etc. are kept accurate.
    if ((TCL_OK != result) && !cdl_result) {
        const char* tcl_result = Tcl_GetStringResult(tcl_interp);
        if ((0 == tcl_result) || ('\0' == tcl_result[0])) {
            tcl_result = "Internal error, no additional information available.";
        }
        CdlParse::report_error(this, "", tcl_result);
    }
    
    str_result = Tcl_GetStringResult(tcl_interp);
    CYG_REPORT_RETVAL(result);
    return result;
}

// Ditto for Tcl Code that comes from a CDL file. Currently this is held
// as a string. In future it may be appropriate to store a byte-compiled
// version as well.
int
CdlInterpreterBody::eval_cdl_code(const cdl_tcl_code script, std::string& str_result)
{
    CYG_REPORT_FUNCNAMETYPE("CdInterpreter::eval_cdl_code", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    int result  = TCL_OK;
    int size    = script.size();
    // Distinguish between results set by the Tcl interpreter and results
    // set by CDL-related commands running in that interpreter.
    cdl_result = false;
    
    if (size < 2048) {
        char buf[2048];
        script.copy(buf, size, 0);
        buf[size] = '\0';
        result = Tcl_Eval(tcl_interp, buf);
    } else {
        char* buf = static_cast<char*>(malloc(script.size() + 1));
        if (0 == buf) {
            this->set_result(CdlParse::construct_diagnostic(this, "internal error", "", "Out of memory"));
            result = TCL_ERROR;
        } else {
            script.copy(buf, size, 0);
            buf[size] = '\0';
            result = Tcl_Eval(tcl_interp, buf);
            free(buf);
        }
    }
    // The distinction between TCL_OK and TCL_RETURN is probably not worth
    // worrying about.
    if (TCL_RETURN == result) {
        result = TCL_OK;
    }
    
    // If we have an error condition that was raised by the Tcl
    // interpreter rather than by the library, it needs to be
    // raised up to the library level. That way the error count
    // etc. are kept accurate.
    if ((TCL_OK != result) && !cdl_result) {
        const char* tcl_result = Tcl_GetStringResult(tcl_interp);
        if ((0 == tcl_result) || ('\0' == tcl_result[0])) {
            tcl_result = "Internal error, no additional information available.";
        }
        CdlParse::report_error(this, "", tcl_result);
    }
    
    str_result = Tcl_GetStringResult(tcl_interp);
    CYG_REPORT_RETVAL(result);
    return result;
}

// Ditto for evaluating an entire file.
int
CdlInterpreterBody::eval_file(std::string script, std::string& str_result)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::eval_file", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != script);

    // Distinguish between results set by the Tcl interpreter and results
    // set by CDL-related commands running in that interpreter.
    cdl_result = false;
    
    int result = Tcl_EvalFile(tcl_interp, CDL_TCL_CONST_CAST(char*, script.c_str()));
    // The distinction between TCL_OK and TCL_RETURN is probably not worth
    // worrying about.
    if (TCL_RETURN == result) {
        result = TCL_OK;
    }

    // If we have an error condition that was raised by the Tcl
    // interpreter rather than by the library, it needs to be
    // raised up to the library level. That way the error count
    // etc. are kept accurate.
    if ((TCL_OK != result) && !cdl_result) {
        const char* tcl_result = Tcl_GetStringResult(tcl_interp);
        if ((0 == tcl_result) || ('\0' == tcl_result[0])) {
            tcl_result = "Internal error, no additional information available.";
        }
        CdlParse::report_error(this, "", tcl_result);
    }
    
    str_result = Tcl_GetStringResult(tcl_interp);
    CYG_REPORT_RETVAL(result);
    return result;
}

// Variants for when the result string is of no interest
int
CdlInterpreterBody::eval(std::string script)
{
    std::string result_string;
    return this->eval(script, result_string);
}

int
CdlInterpreterBody::eval_cdl_code(const cdl_tcl_code script)
{
    std::string result_string;
    return this->eval_cdl_code(script, result_string);
}

int
CdlInterpreterBody::eval_file(std::string filename)
{
    std::string result_string;
    return this->eval_file(filename, result_string);
}

//}}}
//{{{  CdlInterpreter:: set_result()                            

// ----------------------------------------------------------------------------
// Provide a way of setting an interpreter's result from a command implemented
// in C++.

void
CdlInterpreterBody::set_result(std::string result)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::set_result");
    CYG_PRECONDITION_THISC();

    Tcl_SetResult(tcl_interp, const_cast<char*>(result.c_str()), TCL_VOLATILE);
    this->cdl_result = true;
    
    CYG_REPORT_RETURN();
}

bool
CdlInterpreterBody::result_set_by_cdl()
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::result_set_by_cdl", "result %d");
    CYG_PRECONDITION_THISC();

    bool result = this->cdl_result;
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Also allow the result to be extracted again.
std::string
CdlInterpreterBody::get_result()
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::get_result");
    CYG_PRECONDITION_THISC();

    std::string result = Tcl_GetStringResult(tcl_interp);

    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  CdlInterpreter:: add and remove commands                 

// ----------------------------------------------------------------------------
// This is the Tcl command proc that gets used for all CdlInterpreter
// commands. The ClientData field will be a CdlInterpreterCommand,
// i.e. a function pointer. That function needs a pointer to the
// CdlInterpreter object, which can be accessed via AssocData.
int
CdlInterpreterBody::tcl_command_proc(ClientData data, Tcl_Interp* tcl_interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::tcl_command_proc", "result %d");
    CYG_REPORT_FUNCARG3XV(data, tcl_interp, argc);
    CYG_PRECONDITIONC(0 != data);

    int result = TCL_OK;

    union {
        ClientData            data;
        CdlInterpreterCommand command;
    } x;
    x.data = data;
    CdlInterpreterCommand command = x.command;

    data = Tcl_GetAssocData(tcl_interp, cdlinterpreter_assoc_data_key, 0);
    CdlInterpreter interp = static_cast<CdlInterpreter>(data);
    CYG_ASSERT_CLASSC(interp);

    try {
        result = (*command)(interp, argc, argv);
    } catch(std::bad_alloc e) {
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Out of memory."));
        result = TCL_ERROR;
    } catch(CdlStringException e) {
        interp->set_result(e.get_message());
        result = TCL_ERROR;
    } catch(...) {
        CYG_FAIL("Unexpected C++ exception");
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Unexpected C++ exception."));
        result = TCL_ERROR;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::add_command(std::string name, CdlInterpreterCommand command)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::add_command");
    CYG_REPORT_FUNCARG2XV(this, command);

    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);
    CYG_CHECK_FUNC_PTRC(command);

    union {
        CdlInterpreterCommand command;
        ClientData            data;
    } x;
    x.command = command;

    // Tcl 8.4 involves some incompatible API changes
#if (TCL_MAJOR_VERSION > 8) || ((TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 4))
    if (0 == Tcl_CreateCommand(tcl_interp, CDL_TCL_CONST_CAST(char*, name.c_str()), &tcl_command_proc, x.data, 0)) {
        throw std::bad_alloc();
    }
#else
    if (0 == Tcl_CreateCommand(tcl_interp, CDL_TCL_CONST_CAST(char*, name.c_str()),
                               (int (*)(ClientData,Tcl_Interp*, int, char*[])) &tcl_command_proc,
                               x.data, 0)) {
        throw std::bad_alloc();
    }
#endif
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Remove a command from an interpreter. This is just a wrapper for the
// Tcl_DeleteCommand() routine.

void
CdlInterpreterBody::remove_command(std::string name)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::remove_command");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    if (0 != Tcl_DeleteCommand(tcl_interp, CDL_TCL_CONST_CAST(char*, name.c_str()))) {
        CYG_FAIL("attempt to delete non-existant command");
    }
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// It is also possible to add and remove whole sets of commands in one go,
// keeping track of the current set.

std::vector<CdlInterpreterCommandEntry>*
CdlInterpreterBody::push_commands(std::vector<CdlInterpreterCommandEntry>& new_commands)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::push_commands", "result %p");
    CYG_REPORT_FUNCARG2XV(this, &new_commands);
    CYG_PRECONDITION_THISC();

    std::vector<CdlInterpreterCommandEntry>* result = current_commands;
    std::vector<CdlInterpreterCommandEntry>::iterator i;
    
    // First uninstall all the old commands, if any
    if (0 != current_commands) {
        for (i = current_commands->begin(); i != current_commands->end(); i++) {
            remove_command(i->name);
        }
    }

    // Now install the new commands
    for (i = new_commands.begin(); i != new_commands.end(); i++) {
        add_command(i->name, i->command);
    }

    // Remember the current set in case of a subsequent push operation
    current_commands = &new_commands;

    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::pop_commands(std::vector<CdlInterpreterCommandEntry>* original_commands)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::pop_commands");
    CYG_REPORT_FUNCARG2XV(this, &original_commands);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION(0 != current_commands, "no pop without a previous push please");

    std::vector<CdlInterpreterCommandEntry>::iterator i;
    // Uninstall the most recent set of commands
    for (i = current_commands->begin(); i != current_commands->end(); i++) {
        remove_command(i->name);
    }

    // Reinstall the previous set, if any
    if (0 != original_commands) {
        for (i = original_commands->begin(); i != original_commands->end(); i++) {
            add_command(i->name, i->command);
        }
    }
    current_commands = original_commands;
    CYG_REPORT_RETURN();
}

std::vector<CdlInterpreterCommandEntry>*
CdlInterpreterBody::get_pushed_commands() const
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::get_pushed_commands");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return current_commands;
}

//}}}
//{{{  CdlInterpreter:: variables                               

// ----------------------------------------------------------------------------
// Provide some more stubs, this time for accessing Tcl global variables.
void
CdlInterpreterBody::set_variable(std::string name, std::string value)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::set_variable");
    CYG_REPORT_FUNCARG2("this %p, name %s", this, name.c_str());
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);
    if (0 == Tcl_SetVar(tcl_interp, CDL_TCL_CONST_CAST(char*, name.c_str()), CDL_TCL_CONST_CAST(char*, value.c_str()), TCL_GLOBAL_ONLY)) {
        throw std::bad_alloc();
    }
    CYG_REPORT_RETURN();
}

void
CdlInterpreterBody::unset_variable(std::string name)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::unset_variable");
    CYG_REPORT_FUNCARG2("this %p, name %s", this, name.c_str());
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    Tcl_UnsetVar(tcl_interp, CDL_TCL_CONST_CAST(char*, name.c_str()), TCL_GLOBAL_ONLY);
    CYG_REPORT_RETURN();
}

std::string
CdlInterpreterBody::get_variable(std::string name)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::get_variable");
    CYG_REPORT_FUNCARG2("this %p, name %s", this, name.c_str());
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    std::string result = "";
    const char *tmp = Tcl_GetVar(tcl_interp, CDL_TCL_CONST_CAST(char*, name.c_str()), TCL_GLOBAL_ONLY);
    if (0 != tmp) {
        result = tmp;
    }
    
    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  CdlInterpreter:: assoc data                              

// ----------------------------------------------------------------------------
// Associated data. It is useful to be able to store some C++ data with
// Tcl interpreters, so that the implementations of various commands
// can retrieve details of the current state. Tcl provides the necessary
// underlying support via routines Tcl_SetAssocData() etc., and the
// routines here are just stubs for the underlying Tcl ones.

void
CdlInterpreterBody::set_assoc_data(const char* key, ClientData data, Tcl_InterpDeleteProc* del_proc)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::set_assoc_data");
    CYG_REPORT_FUNCARG3("this %p, key %s, data %p", this, key, data);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC((0 != key) && ('\0' != key[0]));

    Tcl_SetAssocData(tcl_interp, CDL_TCL_CONST_CAST(char*, key), del_proc, data);
    CYG_REPORT_RETURN();
}

ClientData
CdlInterpreterBody::get_assoc_data(const char* key)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::get_assoc_data", "result %p");
    CYG_REPORT_FUNCARG2("this %p, key %s", this, key);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC((0 != key) && ('\0' != key[0]));

    ClientData result = Tcl_GetAssocData(tcl_interp, CDL_TCL_CONST_CAST(char*, key), 0);
    CYG_REPORT_RETVAL(result);
    return result;
}

void
CdlInterpreterBody::delete_assoc_data(const char* key)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::delete_assoc_data");
    CYG_REPORT_FUNCARG2("this %p, key %s", this, key);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC((0 != key) && ('\0' != key[0]));

    Tcl_DeleteAssocData(tcl_interp, CDL_TCL_CONST_CAST(char*, key));
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlInterpreter:: file I/O                                

// ----------------------------------------------------------------------------
// Tcl provides file I/O facilities that are already known to be portable
// to the platforms of interest.

bool
CdlInterpreterBody::is_directory(std::string name)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::is_directory", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    bool result = false;
    std::string command = "file isdirectory \"" + name + "\"";
    std::string tcl_result;
    if ((TCL_OK == this->eval(command, tcl_result)) && ("1" == tcl_result)) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlInterpreterBody::is_file(std::string name)
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterpreter::is_file", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != name);

    bool result = false;
    std::string command = "file isfile \"" + name + "\"";
    std::string tcl_result;
    if ((TCL_OK == this->eval(command, tcl_result)) && ("1" == tcl_result)) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

void
CdlInterpreterBody::locate_subdirs(std::string directory, std::vector<std::string>& result)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::locate_subdirs");
    CYG_REPORT_FUNCARG2XV(this, &result);
    CYG_PRECONDITION_THISC();
    
    static char locate_subdirs_script[] = "\
set pattern [file join \"$::cdl_locate_subdirs_path\" *]    \n\
set result {}                                               \n\
foreach entry [glob -nocomplain -- $pattern] {              \n\
    if ([file isdirectory $entry]) {                        \n\
        set entry [file tail $entry]                        \n\
        if {($entry != \"CVS\") && ($entry != \"cvs\")} {   \n\
            lappend result $entry                           \n\
        }                                                   \n\
    }                                                       \n\
}                                                           \n\
return $result                                              \n\
";
    
    std::string                 tcl_result = "";
    set_variable("::cdl_locate_subdirs_path", directory);
    if (TCL_OK != eval(locate_subdirs_script, tcl_result)) {
        CYG_FAIL("Internal error evaluating Tcl script");
    }

    int             count;
    const char**    array;
    if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, tcl_result.c_str()), &count, CDL_TCL_CONST_CAST(char***, &array))) {
        throw std::bad_alloc();
    }
    for (int i = 0; i < count; i++) {
        result.push_back(array[i]);
    }
    Tcl_Free((char*) array);

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Locating all subdirs requires some simple recursion
void
CdlInterpreterBody::locate_all_subdirs(std::string directory, std::vector<std::string>& result)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::locate_all_subdirs");
    CYG_REPORT_FUNCARG2XV(this, &result);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != directory);

    std::vector<std::string> subdirs;
    locate_subdirs(directory, subdirs);
    std::vector<std::string>::const_iterator i, j;

    for (i = subdirs.begin(); i != subdirs.end(); i++) {
        result.push_back(*i);
        std::vector<std::string> its_subdirs;
        locate_all_subdirs(directory + "/" + *i, its_subdirs);
        for (j = its_subdirs.begin(); j != its_subdirs.end(); j++) {
            result.push_back(*i + "/" + *j);
        }
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Locating the files in a particular subdirectory. This requires another
// simple Tcl script.
void
CdlInterpreterBody::locate_files(std::string directory, std::vector<std::string>& result)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::locate_files");
    CYG_REPORT_FUNCARG2XV(this, &result);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != directory);

    static char locate_files_script[] = "\
set pattern [file join \"$::cdl_locate_files_path\" *]  \n\
set result {}                                           \n\
foreach entry [glob -nocomplain -- $pattern] {          \n\
    if ([file isdirectory $entry]) {                    \n\
        continue                                        \n\
    }                                                   \n\
    lappend result [file tail $entry]                   \n\
 }                                                      \n\
return $result                                          \n\
";
 
    std::string                 tcl_result;
    set_variable("::cdl_locate_files_path", directory);
    if (TCL_OK != eval(locate_files_script, tcl_result)) {
        CYG_FAIL("Internal error evaluating Tcl script");
    }
    int             count;
    const char**    array;
    if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, tcl_result.c_str()), &count, CDL_TCL_CONST_CAST(char***, &array))) {
        throw std::bad_alloc();
    }
    for (int i = 0; i < count; i++) {
        result.push_back(array[i]);
    }
    Tcl_Free((char*) array);

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Locating all files can be achieved by combining locate_all_subdirs()
// and locate_files().
void
CdlInterpreterBody::locate_all_files(std::string directory, std::vector<std::string>& result)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::locate_all_files");
    CYG_REPORT_FUNCARG2XV(this, &result);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != directory);

    std::vector<std::string> files;
    std::vector<std::string>::const_iterator i, j;
    locate_files(directory, files);
    for (i = files.begin(); i != files.end(); i++) {
        result.push_back(*i);
    }
    
    std::vector<std::string> all_subdirs;
    locate_all_subdirs(directory, all_subdirs);
    for (i = all_subdirs.begin(); i != all_subdirs.end(); i++) {
        std::vector<std::string> subdir_files;
        locate_files(directory + "/" + *i, subdir_files);
        for (j = subdir_files.begin(); j != subdir_files.end(); j++) {
            result.push_back(*i + "/" + *j);
        }
    }

    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// Write some data to a file, throwing an I/O exception on failure. This
// functionality is needed whenever savefile data is generated, it is
// convenient to have a utility function to do the job. Also, performing
// the Tcl_Write involves passing const data as a non-const argument:
// if this ever causes problems in future it is a good idea to isolate
// the problem here.

void
CdlInterpreterBody::write_data(Tcl_Channel chan, std::string data)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::write_data");
    CYG_REPORT_FUNCARG2XV(this, chan);
    CYG_PRECONDITION_THISC();

    if (-1 == Tcl_Write(chan, CDL_TCL_CONST_CAST(char*, data.data()), data.size())) {
        std::string msg = "Unexpected error writing to file " + this->get_context() + " : " + Tcl_PosixError(tcl_interp);
        throw CdlInputOutputException(msg);
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlInterpreter:: quote() etc.                            

// ----------------------------------------------------------------------------
// Given a string, quote it in such a way that the Tcl interpreter will
// process it as a single word, but keep the result as human-readable
// as possible. If there are no special characters then just return the
// string itself. Otherwise quoting is necessary.
//
// The choice is between braces and double quotes. Generally braces
// are better and more consistent, but there is a problem if the
// string argument itself contains braces. These could be
// backslash-escaped, but the Tcl interpreter will not automatically
// remove the backslashes so we would end up with a discrepancy
// between the original data and what is seen by the parser. In this
// case quote marks have to be used instead.
//
// NOTE: this code may not behave sensibly when it comes to i18n
// issues.

std::string
CdlInterpreterBody::quote(std::string src)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::quote");

    std::string  result = "";
    bool         contains_specials = false;
    unsigned int i;

    if (0 == src.size()) {
        // An empty string. The best way to represent this is an empty
        // set of quotes.
        result = "\"\"";
        CYG_REPORT_RETURN();
        return result;
    }
    
    if ('#' == src[0]) {
        contains_specials = true;
    }
    
    for (i = 0; (i < src.size()) && !contains_specials; i++) {
        if (isspace(src[i])) {
            contains_specials = true;
            break;
        }
        switch(src[i]) {
          case '{':
          case '}':
          case '\\':
          case '$':
          case '"':
          case '[':
          case ']':
          case '#':
          case ';':
              contains_specials = true;
              break;
            
          default:
              break;
        }
    }

    if (!contains_specials) {
        result = src;
    } else{
        // If the data is a multiline item, it is better to start it in column 0.
        // Unfortunately there is the question of what to do with the opening
        // quote. Putting it on the current line, followed by a backslash-escaped
        // newline, introduces a space into the string. If the string begins with
        // a space anyway then arguably this would be harmless, but it could
        // be confusing to the user. Putting the opening double quote into column 0
        // means that the first line of data is indented relative to the rest of
        // the data, but still appears to be the best alternative.
        if (src.find('\n') != std::string::npos) {
            result += "\\\n";
        }
        result += '\"';
        for (i = 0; i < src.size(); i++) {
            switch(src[i]) {
              case '\\':
              case '$':
              case '"':
              case '[':
              case ']':
                  result += '\\';
                  result += src[i];
                  break;
                  
              default:
                result += src[i];
                break;
            }
        }
        result += '\"';
    }
    
    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
// Given some data which may be multiline, return a string which corresponds
// to that data turned into a comment.

std::string
CdlInterpreterBody::multiline_comment(const std::string& orig, int first_indent, int second_indent)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::multiline_comment");

    std::string indent  = std::string(first_indent, ' ') + "# " + std::string(second_indent, ' ');
    std::string result  = "";
    bool indent_needed = true;
    
    std::string::const_iterator str_i;
    for (str_i = orig.begin(); str_i != orig.end(); str_i++) {
        if (indent_needed) {
            result += indent;
            indent_needed = false;
        }
        result += *str_i;
        if ('\n' == *str_i) {
            indent_needed = true;
        }
    }
    
    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
// Given some data, append it to the current line and add additional commented
// and indented lines as required.
std::string
CdlInterpreterBody::extend_comment(const std::string& orig, int first_indent, int second_indent)
{
    CYG_REPORT_FUNCNAME("CdlInterpreter::extend_comment");

    std::string indent  = std::string(first_indent, ' ') + "# " + std::string(second_indent, ' ');
    std::string result = "";
    bool indent_needed = false;
    
    std::string::const_iterator str_i;
    for (str_i = orig.begin(); str_i != orig.end(); str_i++) {
        if (indent_needed) {
            result += indent;
            indent_needed = false;
        }
        result += *str_i;
        if ('\n' == *str_i) {
            indent_needed = true;
        }
    }
    
    CYG_REPORT_RETURN();
    return result;
}

//}}}
