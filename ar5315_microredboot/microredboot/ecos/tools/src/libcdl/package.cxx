//{{{  Banner                           

//============================================================================
//
//     package.cxx
//
//     Implementation of the CdlPackage class
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

// <cdl.hxx> defines everything implemented in this module.
// It implicitly supplies <string>, <vector> and <map> because
// the class definitions rely on these headers.
#include <cdl.hxx>

//}}}

//{{{  Statics                          

// ----------------------------------------------------------------------------
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlPackageBody);

//}}}
//{{{  Constructor                      

// ----------------------------------------------------------------------------
// Constructor. The real work is actually done in the base classes
// and the parser.
CdlPackageBody::CdlPackageBody(std::string name_arg, CdlConfiguration toplevel, std::string dir)
    : CdlNodeBody(name_arg),
      CdlContainerBody(),
      CdlUserVisibleBody(),
      CdlValuableBody(CdlValueFlavor_BoolData),
      CdlParentableBody(),
      CdlBuildableBody(),
      CdlDefinableBody(),
      CdlLoadableBody(toplevel, dir),
      CdlBuildLoadableBody(),
      CdlDefineLoadableBody()
{
    CYG_REPORT_FUNCNAME("CdlPackageBody:: constructor");
    CYG_REPORT_FUNCARG1XV(this);

    loaded_for_template   = false;
    loaded_for_hardware   = false;
    cdlpackagebody_cookie = CdlPackageBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Destructor                       

// ----------------------------------------------------------------------------
// Most of the work is done in the base classes.

CdlPackageBody::~CdlPackageBody()
{
    CYG_REPORT_FUNCNAME("CdlPackageBody:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    loaded_for_template   = false;
    loaded_for_hardware   = false;
    cdlpackagebody_cookie = CdlPackageBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  parse_package()                  

// ----------------------------------------------------------------------------
// Parsing a package definition. This routine gets invoked directly from the
// Tcl interpreter, when the cdl_package command is encountered. The
// command takes two arguments, a name and a body of properties, and there
// should only be one cdl_package command per package.
//
// At the point that the cdl_package command is executed the CdlPackage
// object should already exist, and in fact it should be the current
// interpreter's loadable. Obviously the name should be checked. The
// main purpose of the cdl_package command is to fill in some extra
// information in the form of properties.
//
// A package is a buildable, valuable, uservisible, ... object so it
// inherits properties from all three. In practice some of the
// properties from the base classes are not actually legal, but that
// will be caught by the validation code. Additional properties
// relevant to a package are: parent, license_proc, install_proc, and
// wizard. It is harmless (but unnecessary) to allow components etc.
// to be defined inside a package definition.

int
CdlPackageBody::parse_package(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackageBody::parse_package", "result %d");
    CYG_REPORT_FUNCARG1("argc %d", argc);
    CYG_PRECONDITION_CLASSC(interp);
    
    std::string  diag_argv0      = CdlParse::get_tcl_cmd_name(argv[0]);

    CdlLoadable  loadable       = interp->get_loadable();
    CdlPackage   package        = dynamic_cast<CdlPackage>(loadable);
    CdlContainer parent         = package->get_parent();       
    CdlToplevel  toplevel       = interp->get_toplevel();
    std::string filename        = interp->get_context();
 
    CYG_ASSERT_CLASSC(loadable);        // There should always be a loadable during parsing
    CYG_ASSERT_CLASSC(package);         // And packages are the only loadable for software CDL
    CYG_ASSERT_CLASSC(toplevel);
    CYG_ASSERTC(dynamic_cast<CdlToplevel>(parent) == toplevel);    // The package cannot have been reparented yet.
    CYG_ASSERTC("" != filename);
    CYG_UNUSED_PARAM(CdlContainer, parent);
    CYG_UNUSED_PARAM(CdlToplevel, toplevel);

    // There should be no current node, in fact the cdl_package command
    // can only exist at the toplevel of the original script courtesy
    // of commands being pushed and popped.
    CYG_ASSERTC(0 == interp->get_node());
    
    // Also, the package should be the current container.
    CYG_ASSERTC(package == dynamic_cast<CdlPackage>(interp->get_container()));
    
    // Declare these outside the scope of the try statement, to allow
    // goto calls for the error handling.
    const std::vector<CdlProperty>& properties = package->get_properties();

    CdlInterpreterBody::NodeSupport interp_node(interp, package);
    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("hardware",           &parse_hardware                    ),
        CdlInterpreterCommandEntry("license_proc",       &parse_license_proc                ),
        CdlInterpreterCommandEntry("install_proc",       &parse_install_proc                ),
        CdlInterpreterCommandEntry("cdl_component",      &CdlComponentBody::parse_component ),
        CdlInterpreterCommandEntry("cdl_option",         &CdlOptionBody::parse_option       ),
        CdlInterpreterCommandEntry("cdl_interface",      &CdlInterfaceBody::parse_interface ),
        CdlInterpreterCommandEntry("cdl_dialog",         &CdlDialogBody::parse_dialog       ),
        CdlInterpreterCommandEntry("cdl_wizard",         &CdlWizardBody::parse_wizard       ),
        CdlInterpreterCommandEntry("",                   0                                  )
    };
    std::vector<CdlInterpreterCommandEntry>  new_commands;
    int i;
    
    // All parsing errors may result in an exception, under the control of
    // application code. This exception must not pass through the Tcl interpreter.
    int result = TCL_OK;
    try {

        // Currently there are no options. This may change in future.
        if (3 != argc) {
            CdlParse::report_error(interp, "",
                                   std::string("Incorrect number of arguments to `") + diag_argv0 +
                                   "'\nExpecting name and properties list.");
        } else if (argv[1] != loadable->get_name()) {
            CdlParse::report_error(interp, "",
                                   std::string("Incorrect package name in CDL script.\n") +
                                   "This package is `" + loadable->get_name() + "'\n" +
                                   "The CDL script `" + filename + "' defines a package `" + argv[1] + "'.");
        } else if (0 != properties.size()) {
            CdlParse::report_error(interp, "",
                                   std::string("Duplicate cdl_package commands for package `") + argv[1] + "'.");
        } else if (!Tcl_CommandComplete(CDL_TCL_CONST_CAST(char*, argv[2]))) {
            CdlParse::report_error(interp, "",
                                   std::string("Invalid property list for cdl_package `") + argv[1] + "'.");
        } else {

            for (i = 0; 0 != commands[i].command; i++) {
                new_commands.push_back(commands[i]);
            }
        
            CdlBuildLoadableBody::add_property_parsers(new_commands);
            CdlBuildableBody::add_property_parsers(new_commands);
            CdlDefineLoadableBody::add_property_parsers(new_commands);
            CdlDefinableBody::add_property_parsers(new_commands);
            CdlParentableBody::add_property_parsers(new_commands);
            CdlValuableBody::add_property_parsers(new_commands);
            CdlUserVisibleBody::add_property_parsers(new_commands);
            CdlNodeBody::add_property_parsers(new_commands);

            // Now evaluate the body. If an error occurs then typically
            // this will be reported via CdlParse::report_error(),
            // but any exceptions will have been intercepted and
            // turned into a Tcl error.
            CdlInterpreterBody::CommandSupport interp_cmds(interp, new_commands);
            result = interp->eval(argv[2]);
            if (TCL_OK == result) {

                // Even if there were errors, they were not fatal. There may
                // now be a number of properties for this package, and some
                // validation should take place. Start with the base classes.
                package->CdlNodeBody::check_properties(interp);
                package->CdlUserVisibleBody::check_properties(interp);
                package->CdlValuableBody::check_properties(interp);
                package->CdlParentableBody::check_properties(interp);
                package->CdlBuildableBody::check_properties(interp);
                package->CdlBuildLoadableBody::check_properties(interp);
                package->CdlDefinableBody::check_properties(interp);
                package->CdlDefineLoadableBody::check_properties(interp);

                // Some of the properties in the base classes are not actually
                // appropriate. A package is valuable, but it can only be
                // modified by loading and unloading. Many of the value-related
                // properties do not make sense.
                if (package->count_properties(CdlPropertyId_Flavor) > 0) {
                    CdlParse::report_error(interp, "", "A package should not have a `flavor' property.");
                }
                if (package->count_properties(CdlPropertyId_EntryProc) > 0) {
                    CdlParse::report_error(interp, "", "A package should not have an `entry_proc' property.");
                }
                if (package->count_properties(CdlPropertyId_CheckProc) > 0) {
                    CdlParse::report_error(interp, "", "A package should not have a `check_proc' property.");
                }
                // BLV: this reasoning is faulty, it should be possible to
                // control the enabled aspect via an expression. That would
                // need option processing for the default_value property.
                if (package->count_properties(CdlPropertyId_DefaultValue) > 0) {
                    CdlParse::report_error(interp, "", "A package should not have a `default_value' property.");
                }
                if (package->count_properties(CdlPropertyId_LegalValues) > 0) {
                    CdlParse::report_error(interp, "", "A package should not have a `legal_values' property.");
                }
                if (package->count_properties(CdlPropertyId_Calculated) > 0) {
                    CdlParse::report_error(interp, "", "A package should not have a `calculated' property.");
                }
                if (package->count_properties(CdlPropertyId_Dialog) > 0) {
                    CdlParse::report_error(interp, "", "A package should not have a `dialog' property.");
                }

                // There should be at most one each of license_proc, install_proc, include_dir,
                // export_to, library, makefile, and wizard.
                if (package->count_properties(CdlPropertyId_LicenseProc) > 1) {
                    CdlParse::report_error(interp, "", "A package should have at most one `license_proc' property.");
                }
                if (package->count_properties(CdlPropertyId_InstallProc) > 1) {
                    CdlParse::report_error(interp, "", "A package should have at most one `install_proc' property.");
                }
            }
        }
        
    } catch (std::bad_alloc e) {
        // Errors at this stage should be reported via Tcl, not via C++
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Out of memory"));
        result = TCL_ERROR;
    } catch (CdlParseException e) {
        interp->set_result(e.get_message());
        result = TCL_ERROR;
    } catch(...) {
        interp->set_result(CdlParse::construct_diagnostic(interp, "internal error", "", "Unexpected C++ exception"));
        result = TCL_ERROR;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  Package properties               

// ----------------------------------------------------------------------------
// Syntax: hardware
int
CdlPackageBody::parse_hardware(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_hardware", "result %d");

    int result = CdlParse::parse_minimal_property(interp, argc, argv, CdlPropertyId_Hardware, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: install_proc <tclcode>
int
CdlPackageBody::parse_install_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_install_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_InstallProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
// Syntax: license_proc <tclcode>

int
CdlPackageBody::parse_license_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_license_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_LicenseProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

bool
CdlPackageBody::is_hardware_package() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackage::is_hardware_package", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    if (has_property(CdlPropertyId_Hardware)) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlPackageBody::has_install_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackage::has_install_proc", "result 5d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    if (has_property(CdlPropertyId_InstallProc)) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

const cdl_tcl_code&
CdlPackageBody::get_install_proc() const
{
    CYG_REPORT_FUNCNAME("CdlPackage::get_install_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    static cdl_tcl_code null_result = "";
    cdl_tcl_code& result = null_result;
    CdlProperty prop = get_property(CdlPropertyId_InstallProc);
    if (0 != prop) {
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
        result = tclprop->get_code();
    }

    CYG_REPORT_RETURN();
    return result;
}

bool
CdlPackageBody::has_license_proc() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackage::has_install_proc", "result 5d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    if (has_property(CdlPropertyId_LicenseProc)) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

const cdl_tcl_code&
CdlPackageBody::get_license_proc() const
{
    CYG_REPORT_FUNCNAME("CdlPackage::get_install_proc");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    static cdl_tcl_code null_result = "";
    cdl_tcl_code& result = null_result;
    CdlProperty prop = get_property(CdlPropertyId_LicenseProc);
    if (0 != prop) {
        CdlProperty_TclCode tclprop = dynamic_cast<CdlProperty_TclCode>(prop);
        result = tclprop->get_code();
    }

    CYG_REPORT_RETURN();
    return result;
}

//}}}
//{{{  Propagation support              

// ----------------------------------------------------------------------------
void
CdlPackageBody::update(CdlTransaction transaction, CdlUpdate update)
{
    CYG_REPORT_FUNCNAME("CdlPackage::update");

    this->CdlValuableBody::update(transaction, update);
    this->CdlContainerBody::update(transaction, update);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Persistence support              

// ----------------------------------------------------------------------------

void
CdlPackageBody::initialize_savefile_support(CdlToplevel toplevel)
{
    CYG_REPORT_FUNCNAME("CdlPackage::initialize_savefile_support");

    toplevel->add_savefile_command("cdl_package", 0, &savefile_package_command);
    CdlValuableBody::initialize_savefile_support(toplevel, "cdl_package");
}

void
CdlPackageBody::save(CdlInterpreter interp, Tcl_Channel chan, int indentation, bool minimal)
{
    CYG_REPORT_FUNCNAME("CdlPackage::save");
    CYG_REPORT_FUNCARG5XV(this, interp, chan, indentation, minimal);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    // For a minimal save there is sufficient data in the savefile header
    // to allow the package to be loaded. It is still necessary to output
    // a cdl_package command if there were additional savefile strings.
    if (!minimal || this->has_additional_savefile_information()) {
    
        // Start with the UserVisible data, which will result in a suitable set
        // of comments before the package definition itself.
        this->CdlUserVisibleBody::save(interp, chan, indentation, minimal);

        // Now output the line "cdl_package <name> {"
        // The name is guaranteed to be a valid C preprocessor symbol, so it
        // is not going to need any quoting.
        std::string data = std::string(indentation, ' ') + "cdl_package " + get_name() + " {\n";
        std::string indent_string = std::string(indentation + 4, ' ');

        // The value associated with a package cannot be changed simply
        // by editing the savefile. Add a comment to that effect.
        if (!minimal) {
            data += indent_string + "# Packages cannot be added or removed, nor can their version be changed,\n";
            data += indent_string + "# simply by editing their value. Instead the appropriate configuration\n";
            data += indent_string + "# should be used to perform these actions.\n\n";
        }

        // Output the command and the comment.
        interp->write_data(chan, data);
    
        // Deal with the value
        this->CdlValuableBody::save(interp, chan, indentation + 4, false, minimal);

        // And with any unrecognised data
        this->CdlNodeBody::save(interp, chan, indentation + 4, minimal);
    
        // Close the cdl_package body. A blank line is added here.
        data = "};\n\n";
        
        interp->write_data(chan, data);
    }
    
    // Packages are containers, so dump the contents as well.
    this->CdlContainerBody::save(interp, chan, indentation, minimal);
    
    CYG_REPORT_RETURN();
}

int
CdlPackageBody::savefile_package_command(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackage::savefile_package_command", "result %d");
    CYG_PRECONDITION_CLASSC(interp);

    int result = TCL_OK;
    CdlToplevel toplevel = interp->get_toplevel();
    CYG_ASSERT_CLASSC(toplevel);
    CdlConfiguration config = dynamic_cast<CdlConfiguration>(toplevel);
    CYG_ASSERT_CLASSC(config);

    std::vector<CdlInterpreterCommandEntry> subcommands;
    std::vector<CdlInterpreterCommandEntry>* toplevel_commands = 0;
    CdlNode old_node = 0;
    
    try {
        
        if (3 != argc) {
            CdlParse::report_error(interp, "", "Invalid cdl_package command in savefile, expecting two arguments.");
        } else {

            CdlNode current_node = config->lookup(argv[1]);
            if (0 == current_node) {
                CdlParse::report_error(interp, "",
                                       std::string("The savefile contains a cdl_package command for `") +
                                       argv[1] + "' which has not been loaded.");
            } else {
                config->get_savefile_subcommands("cdl_package", subcommands);
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
CdlPackageBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlPackageBody_Magic != cdlpackagebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
 
    return CdlNodeBody::check_this(zeal)                &&
           CdlContainerBody::check_this(zeal)           &&
           CdlLoadableBody::check_this(zeal)            &&
           CdlUserVisibleBody::check_this(zeal)         &&
           CdlValuableBody::check_this(zeal)            &&
           CdlParentableBody::check_this(zeal)          &&
           CdlBuildableBody::check_this(zeal)           &&
           CdlBuildLoadableBody::check_this(zeal)       &&
           CdlDefinableBody::check_this(zeal)           &&
           CdlDefineLoadableBody::check_this(zeal);
}

//}}}
//{{{  Misc                             

// ----------------------------------------------------------------------------

std::string
CdlPackageBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlPackage::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "package";
}

// BLV: there is an argument for forcing hardware packages to
// send their configuration data to a single header file
// <pkgconf/hardware.h>, so that any code can #include a
// single file to get hold of the hardware details. This
// is suppressed for now while the details are sorted out.

std::string
CdlPackageBody::get_config_header() const
{
    CYG_REPORT_FUNCNAME("CdlPackage::get_config_header");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_CLASSC(this);

    std::string result = "";
#if 0    
    if (has_property(CdlPropertyId_Hardware)) {
        result = "hardware.h";
    } else {
        result = CdlDefineLoadableBody::get_config_header();
    }
#else
    result = CdlDefineLoadableBody::get_config_header();
#endif  

    CYG_REPORT_RETURN();
    return result;
}

bool
CdlPackageBody::belongs_to_template() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackage::belongs_to_template", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = loaded_for_template;
    
    CYG_REPORT_RETVAL(result);
    return result;
}

bool
CdlPackageBody::belongs_to_hardware() const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackage::belongs_to_hardware", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = loaded_for_hardware;
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
