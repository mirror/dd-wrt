//{{{  Banner                           

//============================================================================
//
//     interface.cxx
//
//     Implementation of the CdlInterface class
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
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlInterfaceBody);

//}}}
//{{{  Constructor                      

// ----------------------------------------------------------------------------
CdlInterfaceBody::CdlInterfaceBody(std::string name_arg, bool generated_arg)
    : CdlNodeBody(name_arg),
      CdlUserVisibleBody(),
      CdlValuableBody(CdlValueFlavor_Data),
      CdlParentableBody(),
      CdlBuildableBody(),
      CdlDefinableBody()
{
    CYG_REPORT_FUNCNAME("CdlInterfaceBody:: constructor");
    CYG_REPORT_FUNCARG1XV(this);

    generated = generated_arg;
    cdlinterfacebody_cookie = CdlInterfaceBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();

    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destructor                       

// ----------------------------------------------------------------------------
CdlInterfaceBody::~CdlInterfaceBody()
{
    CYG_REPORT_FUNCNAME("CdlInterfaceBody:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlinterfacebody_cookie = CdlInterfaceBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  parse_interface()                

// ----------------------------------------------------------------------------
// Parsing an interface definition. This is basically the same as parsing
// an option, component, or package.

int
CdlInterfaceBody::parse_interface(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterface::parse_interface", "result %d");
    CYG_REPORT_FUNCARG1("argc %d", argc);
    CYG_PRECONDITION_CLASSC(interp);
    
    std::string  diag_argv0      = CdlParse::get_tcl_cmd_name(argv[0]);

    CdlLoadable  loadable       = interp->get_loadable();
    CdlContainer parent         = interp->get_container();       
    CdlToplevel  toplevel       = interp->get_toplevel();
    CYG_ASSERT_CLASSC(loadable);        // There should always be a loadable during parsing
    CYG_ASSERT_CLASSC(parent);
    CYG_ASSERT_CLASSC(toplevel);

    // The new interface should be created and added to the loadable.
    // early on. If there is a parsing error it will get cleaned up
    // automatically as a consequence of the loadable destructor.
    // However it is necessary to validate the name first. Errors
    // should be reported via CdlParse::report_error(), which
    // may result in an exception.
    CdlInterface new_interface  = 0;
    bool         ok             = true;
    int          result         = TCL_OK;
    try {
    
        // Currently there are no command-line options. This may change in future.
        if (3 != argc) {
            CdlParse::report_error(interp, "", std::string("Incorrect number of arguments to `") + diag_argv0 +
                                   "'\nExpecting name and properties list.");
            ok = false;
        } else if (!Tcl_CommandComplete(CDL_TCL_CONST_CAST(char*, argv[2]))) {
            CdlParse::report_error(interp, "", std::string("Invalid property list for cdl_interface `") + argv[1] + "'.");
            ok = false;
        } else if (0 != toplevel->lookup(argv[1])) {
            // FIXME: interfaces can be generated implicitly because of an
            // unresolved implements property. This code should look for
            // an existing auto-generated interface object and replace it
            // if necessary.
            CdlParse::report_error(interp, "", std::string("Interface `") + argv[1] +
                                   "' cannot be loaded.\nThe name is already in use.");
            ok = false;
        } else {
            new_interface = new CdlInterfaceBody(argv[1], false);
            toplevel->add_node(loadable, parent, new_interface);
        }

        if (!ok) {
            // Just because this component cannot be created, that is no
            // reason to abort the whole parsing process.
            CYG_REPORT_RETVAL(TCL_OK);
            return TCL_OK;
        }
    } catch(std::bad_alloc e) {
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Out of memory"));
        result = TCL_ERROR;
    } catch(CdlParseException e) {
        interp->set_result(e.get_message());
        result = TCL_ERROR;
    } catch(...) {
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Unexpected C++ exception"));
        result = TCL_ERROR;
    }
    if (TCL_OK != result) {
        CYG_REPORT_RETVAL(result);
        return result;
    }

    // At this stage new_interface has been created and added to the
    // hierarchy. The main work now is to add the properties.
    
    // Push the option as the current node early on. This aids
    // diagnostics.
    CdlNode old_node = interp->push_node(new_interface);

    // Declare these outside the scope of the try statement, to allow
    // goto calls for the error handling.
    std::string tcl_result;
    std::vector<CdlInterpreterCommandEntry>  new_commands;
    std::vector<CdlInterpreterCommandEntry>* old_commands = 0;
    static CdlInterpreterCommandEntry   commands[] =
    {
        CdlInterpreterCommandEntry("", 0)
    };
    int i;
    
    // All parsing errors may result in an exception, under the control of
    // application code. This exception must not pass through the Tcl interpreter.
    try {

        for (i = 0; 0 != commands[i].command; i++) {
            new_commands.push_back(commands[i]);
        }
        CdlDefinableBody::add_property_parsers(new_commands);
        CdlBuildableBody::add_property_parsers(new_commands);
        CdlParentableBody::add_property_parsers(new_commands);
        CdlValuableBody::add_property_parsers(new_commands);
        CdlUserVisibleBody::add_property_parsers(new_commands);
        CdlNodeBody::add_property_parsers(new_commands);
    
        // Now evaluate the body. If an error occurs then typically
        // this will be reported via CdlParse::report_error(),
        // but any exceptions will have been intercepted and
        // turned into a Tcl error.
        old_commands = interp->push_commands(new_commands);
        result = interp->eval(argv[2], tcl_result);
        interp->pop_commands(old_commands);
        
        if (TCL_OK == result) {
            // Even if there were errors, they were not fatal. There may
            // now be a number of properties for this option, and some
            // validation should take place. Start with the base classes.
            new_interface->CdlNodeBody::check_properties(interp);
            new_interface->CdlUserVisibleBody::check_properties(interp);
            new_interface->CdlValuableBody::check_properties(interp);
            new_interface->CdlParentableBody::check_properties(interp);
            new_interface->CdlBuildableBody::check_properties(interp);
            new_interface->CdlDefinableBody::check_properties(interp);

            // The flavor "none" makes no sense for interfaces.
            // The flavor "bool" makes very little sense, but may be useful
            // in weird cases. Both booldata and data make sense.
            // The default flavor is "data", because interfaces are
            // essentially just counters.
            if (new_interface->has_property(CdlPropertyId_Flavor)) {
                if (CdlValueFlavor_None == new_interface->get_flavor()) {
                    CdlParse::report_error(interp, "", "An interface should not have the `none' flavor.");
                }
            }
            
            // Interfaces cannot be modified directly by the user, so
            // there is no point in entry_proc, check_proc, dialog or
            // wizard
            if (new_interface->has_property(CdlPropertyId_EntryProc)) {
                CdlParse::report_error(interp, "", "An interface should not have an `entry_proc' property.");
            }
            if (new_interface->has_property(CdlPropertyId_CheckProc)) {
                CdlParse::report_error(interp, "", "An interface should not have a `check_proc' property.");
            }
            if (new_interface->has_property(CdlPropertyId_Dialog)) {
                CdlParse::report_error(interp, "", "An interface should not have a `dialog' property.");
            }
            if (new_interface->has_property(CdlPropertyId_Wizard)) {
                CdlParse::report_error(interp, "", "An interface should not have a `wizard' property.");
            }
            // Calculated does not make sense, an interface is implicitly calculated
            // Nor does default_value.
            if (new_interface->has_property(CdlPropertyId_Calculated)) {
                CdlParse::report_error(interp, "", "An interface should not have a `calculated' property.");
            }
            if (new_interface->has_property(CdlPropertyId_DefaultValue)) {
                CdlParse::report_error(interp, "", "An interface should not have a `default_value' property.");
            }
            // active_if might make sense, as a way of controlling
            // whether or not a #define will be generated.
            
            // legal_values, requires and implements are all sensible
            // properties for an interface.

            // group may or may not make sense, allow it for now.

            // For the uservisible base class, allow all of display,
            // doc and description. At worst these are harmless

            // For the parentable base class, allow parent. Again this
            // is harmless.

            // Also allow all of the definable and buildable
            // properties.
        }
    } catch (std::bad_alloc e) {
        // Errors at this stage should be reported via Tcl, not via C++.
        // However there is no point in continuing with the parsing operation,
        // just give up.
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Out of memory"));
        result = TCL_ERROR;
    } catch (CdlParseException e) {
        interp->set_result(e.get_message());
        result = TCL_ERROR;
    } catch(...) {
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Unexpected C++ exception"));
        result = TCL_ERROR;
    }

    // Restore the interpreter to its prior state.
    interp->pop_node(old_node);
    if (0 != old_commands) {
        interp->pop_commands(old_commands);
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Persistence support              

// ----------------------------------------------------------------------------

void
CdlInterfaceBody::initialize_savefile_support(CdlToplevel toplevel)
{
    CYG_REPORT_FUNCNAME("CdlInterface::initialize_savefile_support");

    toplevel->add_savefile_command("cdl_interface", 0, &savefile_interface_command);
    CdlValuableBody::initialize_savefile_support(toplevel, "cdl_interface");

    CYG_REPORT_RETURN();
}

void
CdlInterfaceBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlInterface::save");
    CYG_REPORT_FUNCARG5XV(this, interp, chan, indentation, minimal);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    // Interfaces contain only calculated data, so for a minimal save
    // there is no point in storing any of the data.
    if (!minimal || this->has_additional_savefile_information()) {
        // Start with the UserVisible data, which will result in a suitable set
        // of comments before the package definition itself.
        this->CdlUserVisibleBody::save(interp, chan, indentation, minimal);

        // Now output the line "cdl_interface <name> {"
        // The name is guaranteed to be a valid C preprocessor symbol, so it
        // is not going to need any quoting.
        std::string data = std::string(indentation, ' ') + "cdl_interface " + get_name() + " {\n";

        // Start with details of everything that implements this interface.
        if (!minimal) {
            const std::vector<CdlReferrer>& referrers = this->get_referrers();
            std::vector<CdlReferrer>::const_iterator ref_i;
            int real_referrers = 0;
            for (ref_i = referrers.begin(); ref_i != referrers.end(); ref_i++) {
                CdlNode     node = ref_i->get_source();
                CdlProperty prop = ref_i->get_source_property();

                CdlValuable valuable = dynamic_cast<CdlValuable>(node);
                if ((0 != valuable) && (CdlPropertyId_Implements == prop->get_property_name())) {
                    real_referrers++;
                    data += std::string(indentation, ' ') + "    # Implemented by " + valuable->get_name() + ", " +
                        (valuable->is_active()  ? "active"  : "inactive") + ", " +
                        (valuable->is_enabled() ? "enabled" : "disabled") + '\n';
                }
            }
            if (0 == real_referrers) {
                data += std::string(indentation, ' ') + "    # No options implement this inferface\n";
            }
        }
        interp->write_data(chan, data);

        // Deal with the value
        this->CdlValuableBody::save(interp, chan, indentation + 4, false, minimal);

        // Close the cdl_interface body. A blank line is added here.
        data = "};\n\n";
        interp->write_data(chan, data);
    }
    
    CYG_REPORT_RETURN();
}

int
CdlInterfaceBody::savefile_interface_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterface::savefile_interface_command", "result %d");
    CYG_PRECONDITION_CLASSC(interp);

    int result = TCL_OK;
    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);

    std::vector<CdlInterpreterCommandEntry> subcommands;
    std::vector<CdlInterpreterCommandEntry>* toplevel_commands = 0;
    CdlNode old_node = 0;
    
    try {
        
        if (3 != argc) {
            CdlParse::report_error(interp, "", "Invalid cdl_interface command in savefile, expecting two arguments.");
        } else {

            CdlNode current_node = toplevel->lookup(argv[1]);
            if (0 == current_node) {
                // FIXME: save value in limbo
                CdlParse::report_error(interp, "",
                                       std::string("The savefile contains a cdl_interface command for an unknown interface `")
                                       + argv[1] + "'.");
            } else {
                toplevel->get_savefile_subcommands("cdl_interface", subcommands);
                toplevel_commands = interp->push_commands(subcommands);
                old_node = interp->push_node(current_node);
                
                std::string tcl_result;
                result = interp->eval(argv[2], tcl_result);
            
                interp->pop_commands(toplevel_commands);
                toplevel_commands = 0;
                interp->pop_node(old_node);
                old_node = 0;
            }
        }
    } catch(...) {
        if (0 != old_node) {
            interp->pop_node(old_node);
        }
        if (0 != toplevel_commands) {
            interp->pop_commands(toplevel_commands);
        }
        throw;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  check_this()                     

// ----------------------------------------------------------------------------

bool
CdlInterfaceBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlInterfaceBody_Magic != cdlinterfacebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlNodeBody::check_this(zeal)        &&
           CdlUserVisibleBody::check_this(zeal) &&
           CdlValuableBody::check_this(zeal)    &&
           CdlParentableBody::check_this(zeal)  &&
           CdlBuildableBody::check_this(zeal)   &&
           CdlDefinableBody::check_this(zeal);
}

//}}}
//{{{  recalculate()                    

// ----------------------------------------------------------------------------
// There has been a change in the configuration which may affect the value
// of an interface. This can happen for a variety of reasons. For simplicity
// the entire value is just recalculated, with no attempt at optimisation.
// This may have to change in future.

void
CdlInterfaceBody::recalculate(CdlTransaction transaction)
{
    CYG_REPORT_FUNCNAME("CdlInterface::recalculate");

    const CdlValue& old_value = transaction->get_whole_value(this);

    cdl_int count = 0;
    std::vector<CdlValuable> implementers;
    std::vector<CdlValuable>::const_iterator valuable_i;
    this->get_implementers(implementers);
    for (valuable_i = implementers.begin(); valuable_i != implementers.end(); valuable_i++) {
        if (transaction->is_active(*valuable_i)) {
            const CdlValue& implementer_value = transaction->get_whole_value(*valuable_i);
            if (implementer_value.is_enabled()) {
                count++;
            }
        }
    }

    // What to do with the count depends on the flavor.
    switch(this->get_flavor()) {
      case CdlValueFlavor_Bool :
        {
            bool new_bool = (count > 0);
            if (new_bool != old_value.is_enabled()) {
                CdlValue new_value = old_value;
                new_value.set_enabled(new_bool, CdlValueSource_Default);
                transaction->set_whole_value(this, old_value, new_value);
            }
            break;
        }
      case CdlValueFlavor_BoolData:
        {
            // The only thing that actually needs checking is the count value.
            // Iff that has changed then the boolean part may need changing as well.
            if (count != old_value.get_integer_value()) {
                CdlValue new_value = old_value;
                new_value.set_enabled_and_value(count > 0, count, CdlValueSource_Default);
                transaction->set_whole_value(this, old_value, new_value);
            }

            break;
        }
      case CdlValueFlavor_Data:
        {
            if (count != old_value.get_integer_value()) {
                CdlValue new_value = old_value;
                new_value.set_integer_value(count, CdlValueSource_Default);
                transaction->set_whole_value(this, old_value, new_value);
            }
            break;
        }
          
      default:
        break;
    }
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  misc                             

// ----------------------------------------------------------------------------

bool
CdlInterfaceBody::is_modifiable() const
{
    CYG_REPORT_FUNCNAME("CdlInterface::is_modifiable (false)");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return false;
}

std::string
CdlInterfaceBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlInterface::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "interface";
}

void
CdlInterfaceBody::get_implementers(std::vector<CdlValuable>& implementers) const
{
    CYG_REPORT_FUNCNAME("CdlInterface::get_implementers");
    CYG_PRECONDITION_THISC();

    const std::vector<CdlReferrer>& referrers = this->get_referrers();
    std::vector<CdlReferrer>::const_iterator ref_i;
    for (ref_i = referrers.begin(); ref_i != referrers.end(); ref_i++) {
        CdlNode     node = ref_i->get_source();
        CdlProperty prop = ref_i->get_source_property();

        CdlValuable valuable = dynamic_cast<CdlValuable>(node);
        if ((0 != valuable) && (CdlPropertyId_Implements == prop->get_property_name())) {
            implementers.push_back(valuable);
        }
    }
    
    CYG_REPORT_RETURN();
}

// Interfaces are somewhat peculiar, in that they can be defined implicitly
// simply by occurring in an "implements" property, or explicitly inside
// a CDL script. Worse, they can switch between these two states when
// loadables are added or removed.

bool
CdlInterfaceBody::was_generated() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlInterface::was_generated", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = this->generated;
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
