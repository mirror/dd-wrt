//{{{  Banner                           

//============================================================================
//
//     dialog.cxx
//
//     Implementation of the CdlDialog class
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
// Date:        1999/03/01
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

//}}}

//{{{  Class statics                    

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlDialogBody);

// The application code can specify whether or not it supports custom
// dialogs. This affects the internals of e.g.
// CdlValuable::get_widget_hint(). For now err on the side of caution.

bool CdlDialogBody::dialogs_enabled     = false;

void
CdlDialogBody::disable_dialogs()
{
    CYG_REPORT_FUNCNAME("CdlDialog::disable_dialogs");
    dialogs_enabled = false;
    CYG_REPORT_RETURN();
}

void
CdlDialogBody::enable_dialogs()
{
    CYG_REPORT_FUNCNAME("CdlDialog::enable_dialogs");
    dialogs_enabled = true;
    CYG_REPORT_RETURN();
}

bool
CdlDialogBody::dialogs_are_enabled()
{
    CYG_REPORT_FUNCNAMETYPE("CdlDialog::dialogs_are_enabled", "result %d");
    bool result = dialogs_enabled;
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Constructor                      

// ----------------------------------------------------------------------------
// Constructor. The real work is actually done in the parse routine.
//
// There is no data associated with a custom dialog object.
CdlDialogBody::CdlDialogBody(std::string name_arg)
    : CdlNodeBody(name_arg),
      CdlParentableBody(),
      CdlUserVisibleBody()
{
    CYG_REPORT_FUNCNAME("CdlDialogBody:: constructor");
    CYG_REPORT_FUNCARG1XV(this);

    cdldialogbody_cookie = CdlDialogBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destructor                       

// ----------------------------------------------------------------------------
// The real work is done in the base classes.

CdlDialogBody::~CdlDialogBody()
{
    CYG_REPORT_FUNCNAME("CdlDialogBody:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdldialogbody_cookie = CdlDialogBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  parse_dialog()                   

// ----------------------------------------------------------------------------
// Parsing a dialog definition.
int
CdlDialogBody::parse_dialog(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDialog::parse_dialog", "result %d");
    CYG_REPORT_FUNCARG1("argc %d", argc);
    CYG_PRECONDITION_CLASSC(interp);
    
    std::string diag_argv0      = CdlParse::get_tcl_cmd_name(argv[0]);

    CdlLoadable  loadable       = interp->get_loadable();
    CdlContainer parent         = interp->get_container();       
    CdlToplevel  toplevel       = interp->get_toplevel();
    CYG_ASSERT_CLASSC(loadable);        // There should always be a loadable during parsing
    CYG_ASSERT_CLASSC(parent);
    CYG_ASSERT_CLASSC(toplevel);

    // The new dialog should be created and added to the loadable
    // early on. If there is a parsing error it will get cleaned up
    // automatically as a consequence of the loadable destructor.
    // However it is necessary to validate the name first. Errors
    // should be reported via CdlParse::report_error(), which
    // may result in an exception.
    CdlDialog    new_dialog     = 0;
    CdlNode      old_node       = 0;
    std::vector<CdlInterpreterCommandEntry>* old_commands = 0;
    int          result         = TCL_OK;
    
    // Currently there are no command-line options. This may change in future.
    if (3 != argc) {
        CdlParse::report_error(interp, "", std::string("Incorrect number of arguments to `") + diag_argv0 +
                               "'\nExpecting name and properties list.");
    } else if (!Tcl_CommandComplete(CDL_TCL_CONST_CAST(char*, argv[2]))) {
        CdlParse::report_error(interp, "", std::string("Invalid property list for cdl_dialog `") + argv[1] + "'.");
    } else if (0 != toplevel->lookup(argv[1])) {
        CdlParse::report_error(interp, "", std::string("Dialog `") + argv[1] + "' cannot be loaded.\n" +
                               "The name is already in use.");
    } else {

        try {
            new_dialog = new CdlDialogBody(argv[1]);
            toplevel->add_node(loadable, parent, new_dialog);

            // Push the dialog as the current base object early on.
            // This aids diagnostics.
            old_node = interp->push_node(new_dialog);

            std::string tcl_result;
            std::vector<CdlInterpreterCommandEntry>  new_commands;
            static CdlInterpreterCommandEntry commands[] =
            {
                CdlInterpreterCommandEntry("init_proc",          &CdlWizardBody::parse_init_proc    ),
                CdlInterpreterCommandEntry("update_proc",        &CdlDialogBody::parse_update_proc  ),
                CdlInterpreterCommandEntry("display_proc",       &CdlDialogBody::parse_display_proc ),
                CdlInterpreterCommandEntry("confirm_proc",       &CdlWizardBody::parse_confirm_proc ),
                CdlInterpreterCommandEntry("cancel_proc",        &CdlWizardBody::parse_cancel_proc  ),
                CdlInterpreterCommandEntry("",                   0                                  ),
            };
            int i;
            for (i = 0; 0 != commands[i].command; i++) {
                new_commands.push_back(commands[i]);
            }
            CdlParentableBody::add_property_parsers(new_commands);
            CdlUserVisibleBody::add_property_parsers(new_commands);
            CdlNodeBody::add_property_parsers(new_commands);
    
            // Now evaluate the body. If an error occurs then typically
            // this will be reported via CdlParse::report_error(),
            // but any exceptions will have been intercepted and
            // turned into a Tcl error.
            old_commands = interp->push_commands(new_commands);
            result = interp->eval(argv[2], tcl_result);
            interp->pop_commands(old_commands);
            old_commands = 0;
            interp->pop_node(old_node);
            old_node = 0;
        
            if (TCL_OK == result) {
                // Even if there were errors, they were not fatal. There may
                // now be a number of properties for this option, and some
                // validation should take place. Start with the base classes.
                new_dialog->CdlNodeBody::check_properties(interp);
                new_dialog->CdlUserVisibleBody::check_properties(interp);
                new_dialog->CdlParentableBody::check_properties(interp);

                // The init_proc and update_proc properties are optional.
                // The display_proc, confirm_proc and cancel_proc properties
                // are compulsory.
                if (new_dialog->count_properties(CdlPropertyId_InitProc) > 1) {
                    CdlParse::report_error(interp, "", "A dialog should have only one `init_proc' property.");
                }
                if (new_dialog->count_properties(CdlPropertyId_UpdateProc) > 1) {
                    CdlParse::report_error(interp, "", "A dialog should have only one `update_proc' property.");
                }
                if (new_dialog->count_properties(CdlPropertyId_DisplayProc) != 1) {
                    CdlParse::report_error(interp, "", "A dialog should have one `display_proc' property.");
                }
                if (new_dialog->count_properties(CdlPropertyId_ConfirmProc) != 1) {
                    CdlParse::report_error(interp, "", "A dialog should have one `confirm_proc' property.");
                }
                if (new_dialog->count_properties(CdlPropertyId_CancelProc) != 1) {
                    CdlParse::report_error(interp, "", "A dialog should have one `cancel_proc' property.");
                }
            }
            
        } catch(...) {
            if (0 != new_dialog) {
                delete new_dialog;
            }
            if (0 != old_node) {
                interp->pop_node(old_node);
            }
            if (0 != old_commands) {
                interp->pop_commands(old_commands);
            }
            throw;
        }
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
// Syntax: display_proc <tclcode>
int
CdlDialogBody::parse_display_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_display_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_DisplayProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: update_proc <tclcode>
int
CdlDialogBody::parse_update_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_update_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_UpdateProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Persistence                      

// ----------------------------------------------------------------------------
// For now there is no information in a custom dialog that should end
// up in a save file, but it is still desirable to override the base
// class member function.

void
CdlDialogBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlDialog::save");
    CYG_REPORT_FUNCARG5XV(this, interp, chan, indentation, minimal);
    CYG_PRECONDITION_THISC();

    CYG_UNUSED_PARAM(CdlInterpreter, interp);
    CYG_UNUSED_PARAM(Tcl_Channel, chan);
    CYG_UNUSED_PARAM(int, indentation);
    CYG_UNUSED_PARAM(bool, minimal);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Data access                      

// ----------------------------------------------------------------------------
bool
CdlDialogBody::has_init_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlDialog::has_init_proc", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_InitProc);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlDialogBody::has_update_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlDialog::has_update_proc", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_UpdateProc);
    CYG_REPORT_RETVAL(result);
    return result;
}

const cdl_tcl_code&
CdlDialogBody::get_init_proc() const
{
    CYG_REPORT_FUNCNAME("CdlDialog::get_init_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    static cdl_tcl_code null_result = "";
    cdl_tcl_code& result = null_result;
    CdlProperty prop = get_property(CdlPropertyId_InitProc);
    if (0 != prop) {
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
        CYG_ASSERT_CLASSC(tclprop);
        result = tclprop->get_code();
    }

    CYG_REPORT_RETURN();
    return result;
}

const cdl_tcl_code&
CdlDialogBody::get_update_proc() const
{
    CYG_REPORT_FUNCNAME("CdlDialog::get_update_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    static cdl_tcl_code null_result = "";
    cdl_tcl_code& result = null_result;
    CdlProperty prop = get_property(CdlPropertyId_UpdateProc);
    if (0 != prop) {
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
        CYG_ASSERT_CLASSC(tclprop);
        result = tclprop->get_code();
    }

    CYG_REPORT_RETURN();
    return result;
}

const cdl_tcl_code&
CdlDialogBody::get_display_proc() const
{
    CYG_REPORT_FUNCNAME("CdlDialog::get_display_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty prop = get_property(CdlPropertyId_DisplayProc);
    CYG_ASSERT_CLASSC(prop);
    CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
    CYG_ASSERT_CLASSC(tclprop);

    const cdl_tcl_code& result = tclprop->get_code();

    CYG_REPORT_RETURN();
    return result;
}

const cdl_tcl_code&
CdlDialogBody::get_confirm_proc() const
{
    CYG_REPORT_FUNCNAME("CdlDialog::get_display_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty prop = get_property(CdlPropertyId_ConfirmProc);
    CYG_ASSERT_CLASSC(prop);
    CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
    CYG_ASSERT_CLASSC(tclprop);

    const cdl_tcl_code& result = tclprop->get_code();

    CYG_REPORT_RETURN();
    return result;
}

const cdl_tcl_code&
CdlDialogBody::get_cancel_proc() const
{
    CYG_REPORT_FUNCNAME("CdlDialog::get_display_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlProperty prop = get_property(CdlPropertyId_CancelProc);
    CYG_ASSERT_CLASSC(prop);
    CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
    CYG_ASSERT_CLASSC(tclprop);

    const cdl_tcl_code& result = tclprop->get_code();

    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  check_this()                     

// ----------------------------------------------------------------------------
// check_this(). There is very little data associated with a dialog itself.
// most of the checks happen in the base class.

bool
CdlDialogBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlDialogBody_Magic != cdldialogbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlUserVisibleBody::check_this(zeal);
}

//}}}
//{{{  misc                             

// ----------------------------------------------------------------------------
std::string
CdlDialogBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlDialog::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "dialog";
}

//}}}
