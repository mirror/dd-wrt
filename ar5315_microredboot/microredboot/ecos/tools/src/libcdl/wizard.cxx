//{{{  Banner                           

//============================================================================
//
//     wizard.cxx
//
//     Implementation of the CdlWizard class
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

//{{{  Statics                          

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlWizardBody);

//}}}
//{{{  Constructor                      

// ----------------------------------------------------------------------------
// Constructor. The real work is actually done in the parse routine.
CdlWizardBody::CdlWizardBody(std::string name_arg)
    : CdlNodeBody(name_arg),
      CdlParentableBody(),
      CdlUserVisibleBody()
{
    CYG_REPORT_FUNCNAME("CdlWizardBody:: constructor");
    CYG_REPORT_FUNCARG1XV(this);

    cdlwizardbody_cookie = CdlWizardBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destructor                       

// ----------------------------------------------------------------------------
// The real work is done in the base classes.
CdlWizardBody::~CdlWizardBody()
{
    CYG_REPORT_FUNCNAME("CdlWizardBody:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlwizardbody_cookie = CdlWizardBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  parse_wizard()                   

// ----------------------------------------------------------------------------
// Parsing a wizard definition.

int
CdlWizardBody::parse_wizard(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlWizard::parse_wizard", "result %d");
    CYG_REPORT_FUNCARG1("argc %d", argc);
    CYG_PRECONDITION_CLASSC(interp);

    int         result          = TCL_OK;
    std::string diag_argv0      = CdlParse::get_tcl_cmd_name(argv[0]);

    CdlLoadable  loadable       = interp->get_loadable();
    CdlContainer parent         = interp->get_container();       
    CdlToplevel  toplevel       = interp->get_toplevel();
    CYG_ASSERT_CLASSC(loadable);        // There should always be a loadable during parsing
    CYG_ASSERT_CLASSC(parent);
    CYG_ASSERT_CLASSC(toplevel);

    // The new wizard should be created and added to the loadable
    // early on. If there is a parsing error it will get cleaned up
    // automatically as a consequence of the loadable destructor.
    // However it is necessary to validate the name first. Errors
    // should be reported via CdlParse::report_error(), which
    // may result in an exception.
    CdlWizard    new_wizard     = 0;
    try {
    
        // Currently there are no command-line options. This may change in future.
        if (3 != argc) {
            CdlParse::report_error(interp, "", std::string("Incorrect number of arguments to `") + diag_argv0 +
                                   "'\nExpecting name and properties list.");
        } else if (!Tcl_CommandComplete(CDL_TCL_CONST_CAST(char*, argv[2]))) {
            CdlParse::report_error(interp, "", std::string("Invalid property list for cdl_wizard `") + argv[1]+ "'.");
        } else if (0 != toplevel->lookup(argv[1])) {
            CdlParse::report_error(interp, "", std::string("Wizard `") + argv[1] +
                                   "' cannot be loaded.\nThe name is already in use.");
        } else {
            new_wizard = new CdlWizardBody(argv[1]);
            toplevel->add_node(loadable, parent, new_wizard);

            // At this stage new_wizard has been created and added to the hierarchy.
            // The main work now is to add the properties.
    
            // Push the wizard as the current base object early on.
            // This aids diagnostics.
            CdlNode old_node = 0;

            std::string tcl_result;
            std::vector<CdlInterpreterCommandEntry>  new_commands;
            std::vector<CdlInterpreterCommandEntry>* old_commands = 0;
            static CdlInterpreterCommandEntry commands[] =
            {
                CdlInterpreterCommandEntry("init_proc",          &parse_init_proc       ),
                CdlInterpreterCommandEntry("decoration_proc",    &parse_decoration_proc ),
                CdlInterpreterCommandEntry("screen",             &parse_screen          ),
                CdlInterpreterCommandEntry("confirm_proc",       &parse_confirm_proc    ),
                CdlInterpreterCommandEntry("cancel_proc",        &parse_cancel_proc     ),
                CdlInterpreterCommandEntry("",                   0                      ),
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
            old_node = interp->push_node(new_wizard);
            old_commands = interp->push_commands(new_commands);
            result = interp->eval(argv[2], tcl_result);
            interp->pop_node(old_node);
            interp->pop_commands(old_commands);
        
            if (TCL_OK == result) {
                // Even if there were errors, they were not fatal. There may
                // now be a number of properties for this option, and some
                // validation should take place. Start with the base classes.
                new_wizard->CdlNodeBody::check_properties(interp);
                new_wizard->CdlUserVisibleBody::check_properties(interp);
                new_wizard->CdlParentableBody::check_properties(interp);

                // The init_proc and decoration_proc properties are
                // optional. The confirm_proc and cancel_proc properties
                // are compulsory, and there should be at least one screen
                // definition.
                if (new_wizard->count_properties(CdlPropertyId_InitProc) > 1) {
                    CdlParse::report_error(interp, "", "A wizard should have only one `init_proc' property.");
                }
                if (new_wizard->count_properties(CdlPropertyId_DecorationProc) > 1) {
                    CdlParse::report_error(interp, "", "A wizard should have only one `decoration_proc' property.");
                }
                if (new_wizard->count_properties(CdlPropertyId_ConfirmProc) != 1) {
                    CdlParse::report_error(interp, "", "A wizard should have one `confirm_proc' property.");
                }
                if (new_wizard->count_properties(CdlPropertyId_CancelProc) != 1) {
                    CdlParse::report_error(interp, "", "A wizard should have one `cancel_proc' property.");
                }
                if (new_wizard->count_properties(CdlPropertyId_Screen) < 1) {
                    CdlParse::report_error(interp, "", "A wizard should have at least one `screen' property.");
                }

                // It is necessary to check that all the screen properties have unique numbers
                const std::vector<CdlProperty>& properties = new_wizard->get_properties();
                std::vector<CdlProperty>::const_iterator prop_i, prop_j;
                for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
                    if (CdlPropertyId_Screen != (*prop_i)->get_property_name()) {
                        continue;
                    }
                    CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(*prop_i);
                    CYG_ASSERT_CLASSC(tclprop);
                    cdl_int num1 = tclprop->get_number();

                    for (prop_j = ++prop_i; prop_j != properties.end(); prop_j++) {
                        if (CdlPropertyId_Screen != (*prop_j)->get_property_name()) {
                            continue;
                        }
                        CdlProperty_TclCode tclprop2 = dynamic_cast<CdlProperty_TclCode>(*prop_j);
                        CYG_ASSERT_CLASSC(tclprop2);
                        cdl_int num2 = tclprop2->get_number();

                        if (num1 == num2) {
                            std::string tmp = "";
                            Cdl::integer_to_string(num1, tmp);
                            CdlParse::report_error(interp, "", "Duplicate definition of screen `" + tmp + "'.");
                            break;
                        }
                    } 
               }

                // There is no restriction on what screen numbers can be
                // defined (screens may appear and disappear during
                // development). It would be nice to validate that the
                // Tcl fragments never switch to an invalid screen, but
                // there is no easy way to do that.
            }
        }
            
    } catch(...) {
        if (0 != new_wizard) {
            delete new_wizard;
        }
        throw;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: cancel_proc <tclcode>

int
CdlWizardBody::parse_cancel_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_cancel_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_CancelProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: confirm_proc <tclcode>

int
CdlWizardBody::parse_confirm_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_confirm_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_ConfirmProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// syntax: decoration_proc <tclcode>

int
CdlWizardBody::parse_decoration_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_decoration_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_DecorationProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: init_proc <tclcode>

int
CdlWizardBody::parse_init_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_init_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_InitProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// The screen property is a little bit special and cannot be handled by the
// generic parsers. There are two arguments, a number and some Tcl code.

int
CdlWizardBody::parse_screen(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAME("CdlParse::parse_screen");
    CYG_REPORT_FUNCARG1("argc %d", argc);
    CYG_PRECONDITION_CLASSC(interp);
    
    CdlProperty_TclCode new_property = 0;
    cdl_int number = 0;
    
    try {
        std::vector<std::pair<std::string,std::string> > options;
        int data_index      = CdlParse::parse_options(interp, std::string("property ") + argv[0], 0, argc, argv, 1, options);
        if ((data_index + 2) != argc) {
            CdlParse::report_property_parse_error(interp, argv[0], std::string("Invalid number of arguments.\n") +
                                         "    Expecting two arguments, a number and some Tcl code.");
        } else if (!Cdl::string_to_integer(argv[data_index], number)) {
            CdlParse::report_property_parse_error(interp, argv[0], std::string(argv[data_index]) + " is not a valid number.");
        } else if (!Tcl_CommandComplete(CDL_TCL_CONST_CAST(char*, argv[data_index + 1]))) {
            CdlParse::report_property_parse_error(interp, argv[0], "incomplete Tcl code fragment.");
        } else {
            
            CdlNode current_node = interp->get_node();
            CYG_ASSERTC(0 != current_node);
            new_property = CdlProperty_TclCodeBody::make(current_node, CdlPropertyId_Screen, number, argv[data_index + 1],
                                                         argc, argv, options);
        }
    } catch(...) {
        if (0 != new_property) {
            delete new_property;
        }
        throw;
    }
    
    return TCL_OK;
}

//}}}
//{{{  Persistence                      

// ----------------------------------------------------------------------------
// For now there is no information in a wizard that should end up in a
// save file, but it is still desirable to override the base class
// member function.

void
CdlWizardBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlWizard::save");
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
CdlWizardBody::has_init_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlWizard::has_init_proc", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_InitProc);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlWizardBody::has_decoration_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlWizard::has_decoration_proc", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = has_property(CdlPropertyId_DecorationProc);
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlWizardBody::has_screen(cdl_int screen) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlWizard::has_screen", "result %d");
    CYG_REPORT_FUNCARG2XV(this, (int) screen);
    CYG_PRECONDITION_THISC();

    bool result = false;
    const std::vector<CdlProperty>& properties = get_properties();
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if (CdlPropertyId_Screen != (*prop_i)->get_property_name()) {
            continue;
        }
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(*prop_i);
        CYG_ASSERT_CLASSC(tclprop);
        if (screen == tclprop->get_number()) {
            result = true;
            break;
        }
    }
    CYG_REPORT_RETVAL(result);
    return result;
}

const cdl_tcl_code&
CdlWizardBody::get_init_proc() const
{
    CYG_REPORT_FUNCNAME("CdlWizard::get_init_proc");
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
CdlWizardBody::get_decoration_proc() const
{
    CYG_REPORT_FUNCNAME("CdlWizard::get_decoration_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    static cdl_tcl_code null_result = "";
    cdl_tcl_code& result = null_result;
    CdlProperty prop = get_property(CdlPropertyId_DecorationProc);
    if (0 != prop) {
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
        CYG_ASSERT_CLASSC(tclprop);
        result = tclprop->get_code();
    }

    CYG_REPORT_RETURN();
    return result;
}

const cdl_tcl_code&
CdlWizardBody::get_confirm_proc() const
{
    CYG_REPORT_FUNCNAME("CdlWizard::get_display_proc");
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
CdlWizardBody::get_cancel_proc() const
{
    CYG_REPORT_FUNCNAME("CdlWizard::get_display_proc");
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

cdl_int
CdlWizardBody::get_first_screen_number() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlWizard::get_first_screen_number", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // The result should really be initialized to MININT, but that is
    // slightly tricky given that we are dealing with cdl_int's rather
    // than plain int's. Instead a separate flag gives the same
    // effect.
    bool        result_set = false;
    cdl_int     result = -1;

    const std::vector<CdlProperty>& properties = get_properties();
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if (CdlPropertyId_Screen != (*prop_i)->get_property_name()) {
            continue;
        }
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(*prop_i);
        cdl_int its_num = tclprop->get_number();
        if (!result_set || (its_num < result)) {
            result = its_num;
            result_set = true;
        }
    }

    CYG_REPORT_RETVAL((int) result);
    return result;
}

const cdl_tcl_code&
CdlWizardBody::get_first_screen() const
{
    CYG_REPORT_FUNCNAME("CdlWizard::get_first_screen");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // The result should really be initialized to MININT, but that is
    // slightly tricky given that we are dealing with cdl_int's rather
    // than plain int's. Instead a separate flag gives the same
    // effect.
    static cdl_tcl_code null_result     = "";
    bool                result_set      = false;
    cdl_tcl_code&       result          = null_result;
    cdl_int             result_num      = -1;

    const std::vector<CdlProperty>& properties = get_properties();
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if (CdlPropertyId_Screen != (*prop_i)->get_property_name()) {
            continue;
        }
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(*prop_i);
        cdl_int its_num = tclprop->get_number();
        if (!result_set || (its_num < result_num)) {
            result     = tclprop->get_code();
            result_num = its_num;
            result_set = true;
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

const cdl_tcl_code&
CdlWizardBody::get_screen(cdl_int screen) const
{
    CYG_REPORT_FUNCNAME("CdlWizard::get_screen");
    CYG_REPORT_FUNCARG2XV(this, (int)screen);
    CYG_PRECONDITION_THISC();

    static cdl_tcl_code null_result     = "";
    cdl_tcl_code&       result          = null_result;

    const std::vector<CdlProperty>& properties = get_properties();
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = properties.begin(); prop_i != properties.end(); prop_i++) {
        if (CdlPropertyId_Screen != (*prop_i)->get_property_name()) {
            continue;
        }
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(*prop_i);
        if (tclprop->get_number() == screen) {
            result     = tclprop->get_code();
            break;
        }
    }

    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  check_this()                     

// ----------------------------------------------------------------------------
// check_this(). There is very little data associated with a wizard,
// most of the checks happen in the base classes.
bool
CdlWizardBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlWizardBody_Magic != cdlwizardbody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlUserVisibleBody::check_this(zeal);
}

//}}}
//{{{  misc                             

// ----------------------------------------------------------------------------

std::string
CdlWizardBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlWizard::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "wizard";
}

//}}}
