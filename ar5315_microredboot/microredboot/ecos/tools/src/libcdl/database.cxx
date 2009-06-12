//{{{  Banner                                                   

//============================================================================
//
//      database.cxx
//
//      Temporary implementation of the CdlPackagesDatabase class
//      Implementations of the temporary CdlTargetsDatabase and
//      CdlTemplatesDatabase classes.
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
// Date:        1999/01/21
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

// strcmp() is useful when dealing with Tcl strings.
#include <cstring>

//}}}

//{{{  Statics                                                  

// ----------------------------------------------------------------------------
// Some test cases may want to read in a file other than
// "ecos.db", e.g. to facilitate testing the error conditions.
char*
CdlPackagesDatabaseBody::database_name = "ecos.db";

// Should warnings be issued for minor database inconsistencies?
bool CdlPackagesDatabaseBody::verbose_mode      = false;

// The new_package etc. commands need to store the name of the
// current package so that subsequent commands can do the right thing.
// Using constant strings as the key avoids typo problems.
const char*     dbparser_pkgname                = "::dbparser_pkgname";
const char*     dbparser_pkgdata                = "__cdl_dbparser_pkgdata";
const char*     dbparser_targetname             = "::dbparser_targetname";
const char*     dbparser_targetdata             = "__cdl_dbparser_targetdata";
const char*     dbparser_component_repository   = "::component_repository";
const char*     dbparser_database_key           = "__dbparser_key";       // for assoc data
const char*     template_description_key        = "__cdl_extract_template_description"; // ditto
const char*     template_packages_key           = "__cdl_extract_template_packages";

// These are useful for generating diagnostics.
static std::string diag_package = std::string("package ");
static std::string diag_target  = std::string("target ");

CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlPackagesDatabaseBody);

//}}}
//{{{  Tcl commands for the parser                              

//{{{  CdlDbParser class                                

// ----------------------------------------------------------------------------
// Commands that get invoked from inside the Tcl interpreter. These
// need access to the internals of the database objects, which can be
// achieved by making them static members of a CdlDbParser class.

class CdlDbParser {
  public:
    static int new_package(CdlInterpreter, int, const char*[]);
    static int package_description(CdlInterpreter, int, const char*[]);
    static int package_alias(CdlInterpreter, int, const char*[]);
    static int package_directory(CdlInterpreter, int, const char*[]);
    static int package_script(CdlInterpreter, int, const char*[]);
    static int package_hardware(CdlInterpreter, int, const char*[]);

    static int new_target(CdlInterpreter, int, const char*[]);
    static int target_description(CdlInterpreter, int, const char*[]);
    static int target_alias(CdlInterpreter, int, const char*[]);
    static int target_packages(CdlInterpreter, int, const char*[]);
    static int target_enable(CdlInterpreter, int, const char*[]);
    static int target_disable(CdlInterpreter, int, const char*[]);
    static int target_set_value(CdlInterpreter, int, const char*[]);
};

//}}}
//{{{  CdlDbParser::package-related                     

// ----------------------------------------------------------------------------
// package <name> <body>

int
CdlDbParser::new_package(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::new_package", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    CdlPackagesDatabase db      = static_cast<CdlPackagesDatabase>(interp->get_assoc_data(dbparser_database_key));
    CYG_INVARIANT_CLASSC(CdlPackagesDatabaseBody, db);
    
    if (3 != argc) {
        if (argc < 2) {
            CdlParse::report_error(interp, "", "Invalid package command, missing name and contents.");
        } else if (argc == 2) {
            CdlParse::report_error(interp, diag_package + argv[1], "Invalid package command, missing body.");
        } else {
            CdlParse::report_error(interp, diag_package + argv[1],
                                   "Invalid package command, expecting just name and body.");
        }
        CYG_REPORT_RETVAL(TCL_OK);
        return TCL_OK;
    }
    std::string pkg_name        = argv[1];

    // Better make sure that this is not a duplicate definition.
    if (std::find(db->package_names.begin(), db->package_names.end(), pkg_name) != db->package_names.end()) {
        CdlParse::report_warning(interp, diag_package + pkg_name, "Duplicate package entry, ignoring second occurence.");
        CYG_REPORT_RETVAL(TCL_OK);
        return TCL_OK;
    }
    
    // The package data is constructed locally. It only gets added to
    // the database in the absence of errors.
    bool package_ok      = true;
    int  old_error_count = CdlParse::get_error_count(interp);
        
    CdlPackagesDatabaseBody::package_data package;
    package.description = "";
    package.directory   = "";
    package.script      = "";
    package.hardware    = false;
    
    // aliases and versions are vectors and will take care of themselves
    // And the name had better be valid as well.
    if (!Cdl::is_valid_cdl_name(pkg_name)) {
        CdlParse::report_error(interp, diag_package + pkg_name, "This is not a valid CDL name.");
    }
    
    // Sort out the commands, then invoke the script in argv[2]. There is
    // no need to worry about error recovery here, any errors will be
    // fatal anyway.
    CdlInterpreterCommandEntry commands[] = {
        CdlInterpreterCommandEntry("description", &CdlDbParser::package_description ),
        CdlInterpreterCommandEntry("alias",       &CdlDbParser::package_alias       ),
        CdlInterpreterCommandEntry("directory",   &CdlDbParser::package_directory   ),
        CdlInterpreterCommandEntry("script",      &CdlDbParser::package_script      ),
        CdlInterpreterCommandEntry("hardware",    &CdlDbParser::package_hardware    ),
        CdlInterpreterCommandEntry("",            0                                 )
    };
    CdlInterpreterBody::CommandSupport  cmds(interp, commands);
    CdlInterpreterBody::VariableSupport interp_name(interp, dbparser_pkgname, pkg_name);
    CdlInterpreterBody::AssocSupport    interp_data(interp, dbparser_pkgdata, static_cast<ClientData>(&package));
    int result = interp->eval(argv[2]);
    if (TCL_OK == result) {
        
        // The body has been parsed OK. Check that it is valid.
        if ("" == package.directory) {
            CdlParse::report_error(interp, diag_package + pkg_name, "Missing directory specification.");
        }
        if ("" == package.script) {
            CdlParse::report_error(interp, diag_package + pkg_name, "Missing script specification.");
        }
        if (0 == package.aliases.size()) {
            CdlParse::report_error(interp, diag_package + pkg_name, "At least one alias should be supplied.");
        }

        // Additional checks. Is the package directory actually present?
        // Note that there are scenarios where a package may be listed
        // in the database but not installed, e.g. an anoncvs checkout
        // of selected modules.
        if ("" != package.directory) {
            std::string repo = interp->get_variable(dbparser_component_repository);
            CYG_ASSERTC("" != repo);

            std::string pkgdir = repo + "/" + package.directory;
            if (!interp->is_directory(pkgdir)) {
                if (CdlPackagesDatabaseBody::verbose_mode) {
                    CdlParse::report_warning(interp, diag_package + pkg_name,
                                             std::string("This package is not present in the component repository.\n"
                                                         "There is no directory `") + pkgdir + "'.");
                }
                package_ok = false;
            } else {
                
                // Now look for version subdirectories. There should be at least one.
                std::vector<std::string> subdirs;
                unsigned int i;
                interp->locate_subdirs(pkgdir, subdirs);
                std::sort(subdirs.begin(), subdirs.end(), Cdl::version_cmp());
                
                for (i = 0; i < subdirs.size(); i++) {
                    if (("CVS" == subdirs[i]) || ("cvs" == subdirs[i])) {
                        continue;
                    }
                    if ("" != package.script) {
                        if (!(interp->is_file(pkgdir + "/" + subdirs[i] + "/cdl/" + package.script) ||
                              interp->is_file(pkgdir + "/" + subdirs[i] + "/" + package.script))) {
                            CdlParse::report_warning(interp, diag_package + pkg_name,
                                                     std::string("Version subdirectory `") + subdirs[i] +
                                                     "' does not have a CDL script `" + package.script + "'.");
                            continue;
                        }
                    }
                    package.versions.push_back(subdirs[i]);
                }
                if (0 == package.versions.size()) {
                    CdlParse::report_warning(interp, diag_package + pkg_name,
                                             "This package does not have any valid version subdirectories.");
                    package_ok = false;
                }
            }
        }
    }

    // If the package is still ok, now is the time to add it to the database.
    if (package_ok && (old_error_count == CdlParse::get_error_count(interp))) {
        db->package_names.push_back(pkg_name);
        db->packages[pkg_name] = package;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

// Syntax: description <text>
int
CdlDbParser::package_description(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::package_description", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_pkgname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::package_data* package =
        static_cast<CdlPackagesDatabaseBody::package_data*>(interp->get_assoc_data(dbparser_pkgdata));

    if (2 != argc) {
        CdlParse::report_error(interp, diag_package + name, "Invalid description, expecting a single string.");
    } else if ("" != package->description) {
        CdlParse::report_warning(interp, diag_package + name, "A package should have only one description.");
    } else {
        package->description = argv[1];
    }
    
    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: alias <list>
// For example: alias { "This is an alias" another_alias dummy_name }
int
CdlDbParser::package_alias(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::package_alias", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_pkgname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::package_data* package =
        static_cast<CdlPackagesDatabaseBody::package_data*>(interp->get_assoc_data(dbparser_pkgdata));

    // There should be one argument, a list of valid packages.
    // Also, the alias command should be used only once
    if (2 != argc) {
        CdlParse::report_error(interp, diag_package + name,
                               "The alias command should be followed by a list of known aliases.");
    } else if (0 < package->aliases.size()) {
        CdlParse::report_warning(interp, diag_package + name, "There should be only one list of aliases.");
    } else {
        int          list_count     = 0;
        const char** list_entries   = 0;
        Tcl_Interp* tcl_interp      = interp->get_tcl_interpreter();
        if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, argv[1]), &list_count, CDL_TCL_CONST_CAST(char***, &list_entries))) {
            CdlParse::report_error(interp, diag_package + name, Tcl_GetStringResult(tcl_interp));
        } else {
            if (0 == list_count) {
                CdlParse::report_error(interp, diag_package + name, "At least one alias should be supplied.");
            } else {
                for (int i = 0; i < list_count; i++) {
                    package->aliases.push_back(list_entries[i]);
                }
            }
            Tcl_Free((char*)list_entries);
        }
    }
    
    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: directory <path>
// The path is of course relative to the component repository.
int
CdlDbParser::package_directory(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::package_directory", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_pkgname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::package_data* package =
        static_cast<CdlPackagesDatabaseBody::package_data*>(interp->get_assoc_data(dbparser_pkgdata));

    // There should be exactly one argument, and the directory command
    // should be used only once.
    if (2 != argc) {
        CdlParse::report_error(interp, diag_package + name, "Only one directory can be specified.");
    } else if ("" != package->directory) {
        CdlParse::report_warning(interp, diag_package + name, "A package can be located in only one directory.");
    } else {
        package->directory = argv[1];
    }
    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: hardware
// There are no arguments.
int
CdlDbParser::package_hardware(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::package_hardware", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_pkgname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::package_data* package =
        static_cast<CdlPackagesDatabaseBody::package_data*>(interp->get_assoc_data(dbparser_pkgdata));

    if (1 != argc) {
        CdlParse::report_error(interp, diag_package + name, "There should be no further data after hardware.");
    } else if (package->hardware) {
        CdlParse::report_warning(interp, diag_package + name, "The hardware property should be specified only once");
    } else {
        package->hardware    = true;
    }
    
    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: script <filename>
int
CdlDbParser::package_script(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::package_script", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_pkgname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::package_data* package =
        static_cast<CdlPackagesDatabaseBody::package_data*>(interp->get_assoc_data(dbparser_pkgdata));

    // There should be exactly one argument, and the script command
    // should be used only once
    if (2 != argc) {
        CdlParse::report_error(interp, diag_package + name, "Only one CDL script can be specified.");
    } else if ("" != package->script) {
        CdlParse::report_warning(interp, diag_package + name, "A package can have only one starting CDL script.");
    } else {
        package->script = argv[1];
    }

    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

//}}}
//{{{  CdlDbParser::target-related                      

// ----------------------------------------------------------------------------
// target <name> <body>

int
CdlDbParser::new_target(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::new_target", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    CdlPackagesDatabase db      = static_cast<CdlPackagesDatabase>(interp->get_assoc_data(dbparser_database_key));
    CYG_INVARIANT_CLASSC(CdlPackagesDatabaseBody, db);
    
    if (3 != argc) {
        if (argc < 2) {
            CdlParse::report_error(interp, "", "Invalid target command, missing name and contents.");
        } else if (argc == 2) {
            CdlParse::report_error(interp, diag_target + argv[1], "Invalid target command, missing body.");
        } else {
            CdlParse::report_error(interp, diag_target + argv[1], "Invalid target command, expecting just name and body.");
        }
        CYG_REPORT_RETVAL(TCL_OK);
        return TCL_OK;
    }
    
    std::string target_name     = argv[1];

    // Better make sure that this is not a duplicate definition.
    if (std::find(db->target_names.begin(), db->target_names.end(), target_name) != db->target_names.end()) {
        CdlParse::report_warning(interp, diag_target + target_name,
                                 "Duplicate target entry, ignoring second occurence.");
        CYG_REPORT_RETVAL(TCL_OK);
        return TCL_OK;
    }

    // The target data is constructed locally. It only gets added to the
    // database in the absence of errors.
    bool target_ok = true;
    int old_error_count = CdlParse::get_error_count(interp);

    CdlPackagesDatabaseBody::target_data target;
    target.description = "";
    // aliases, packages and compiler_flags are vectors and will take care of themselves

    // Sort out the commands, then invoke the script in argv[2]. There is
    // no need to worry about error recovery here, any errors will be
    // fatal anyway.
    CdlInterpreterCommandEntry commands[] = {
        CdlInterpreterCommandEntry("description",    &CdlDbParser::target_description    ),
        CdlInterpreterCommandEntry("alias",          &CdlDbParser::target_alias          ),
        CdlInterpreterCommandEntry("packages",       &CdlDbParser::target_packages       ),
        CdlInterpreterCommandEntry("enable",         &CdlDbParser::target_enable         ),
        CdlInterpreterCommandEntry("disable",        &CdlDbParser::target_disable        ),
        CdlInterpreterCommandEntry("set_value",      &CdlDbParser::target_set_value      ),
        CdlInterpreterCommandEntry("",               0                                   )
    };
    CdlInterpreterBody::CommandSupport  interp_cmds(interp, commands);
    CdlInterpreterBody::VariableSupport interp_name(interp, dbparser_targetname, target_name);
    CdlInterpreterBody::AssocSupport    interp_data(interp, dbparser_targetdata, static_cast<ClientData>(&target));
    int result = interp->eval(argv[2]);
    if (TCL_OK == result) {
        
        if (0 == target.aliases.size()) {
            CdlParse::report_error(interp, diag_target + target_name, "At least one alias should be supplied.");
        }
        
        // There is no check for > 0 hardware packages. This is an unlikely
        // scenario but should be allowed for.
        // Add this target to the list.
    }

    if (target_ok && (old_error_count == CdlParse::get_error_count(interp))) {
        db->target_names.push_back(target_name);
        db->targets[target_name] = target;
    }
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// Syntax: description <text>
int
CdlDbParser::target_description(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::target_description", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_targetname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::target_data* target =
        static_cast<CdlPackagesDatabaseBody::target_data*>(interp->get_assoc_data(dbparser_targetdata));

    if (2 != argc) {
        CdlParse::report_error(interp, diag_target + name, "The target description should be a single string.");
    } else if ("" != target->description) {
        CdlParse::report_warning(interp, diag_target + name, "A target should have only one description.");
    } else {
        target->description = argv[1];
    }
    
    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: alias <list>
// For example: alias { "This is an alias" another_alias dummy_name }
int
CdlDbParser::target_alias(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::target_alias", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_targetname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::target_data* target =
        static_cast<CdlPackagesDatabaseBody::target_data*>(interp->get_assoc_data(dbparser_targetdata));

    // There should be one argument, a list of valid aliases
    // The alias command should be used only once
    if (2 != argc) {
        CdlParse::report_error(interp, diag_target + name, "The alias command should be followed by a list of known aliases");
    } else if (0 < target->aliases.size()) {
        CdlParse::report_warning(interp, diag_target + name, "There should be only one list of aliases.");
    } else {
        int          list_count     = 0;
        const char** list_entries   = 0;
        Tcl_Interp* tcl_interp      = interp->get_tcl_interpreter();
        if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, argv[1]), &list_count, CDL_TCL_CONST_CAST(char***, &list_entries))) {
            CdlParse::report_error(interp, diag_target + name, Tcl_GetStringResult(tcl_interp));
        } else {
            if (0 == list_count) {
                CdlParse::report_error(interp, diag_target + name, "At least one alias should be supplied.");
            } else {
                for (int i = 0; i < list_count; i++) {
                    target->aliases.push_back(list_entries[i]);
                }
            }
            Tcl_Free((char*)list_entries);
        }
    }
    
    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: packages <list> ...
// For example: packages { CYGPKG_HAL_XXX CYGPKG_HAL_YYY }
int
CdlDbParser::target_packages(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::target_packages", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_targetname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::target_data* target =
        static_cast<CdlPackagesDatabaseBody::target_data*>(interp->get_assoc_data(dbparser_targetdata));

    // There should be one argument, a list of valid packages.
    // The packages command should be used only once
    if (2 != argc) {
        CdlParse::report_error(interp, diag_target + name, "`packages' should be followed by a list of known packages.");
    } else if (0 < target->packages.size()) {
        CdlParse::report_warning(interp, diag_target + name, "There should be only one list of packages.");
    } else {
        int          list_count     = 0;
        const char** list_entries   = 0;
        Tcl_Interp* tcl_interp      = interp->get_tcl_interpreter();
        if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, argv[1]), &list_count, CDL_TCL_CONST_CAST(char***, &list_entries))) {
            CdlParse::report_error(interp, diag_target + name, Tcl_GetStringResult(tcl_interp));
        } else {
            // Allow for a dummy target spec, just in case it proves useful.
            if (0 != list_count) {
                for (int i = 0; i < list_count; i++) {
                    target->packages.push_back(list_entries[i]);
                }
            }
            Tcl_Free((char*)list_entries);
        }
    }
    
    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: enable { opt1 opt2 ... }
// For example: enable { CYGPKG_HAL_ARM_CL7xxx_7211 }
int
CdlDbParser::target_enable(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::target_enable", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_targetname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::target_data* target =
        static_cast<CdlPackagesDatabaseBody::target_data*>(interp->get_assoc_data(dbparser_targetdata));

    // There should be one argument, a list of valid flags.
    if (2 != argc) {
        CdlParse::report_error(interp, diag_target + name, "`enable' should be followed by a list of CDL options.");
    } else {
        int          list_count     = 0;
        const char** list_entries   = 0;
        Tcl_Interp* tcl_interp      = interp->get_tcl_interpreter();
        if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, argv[1]), &list_count, CDL_TCL_CONST_CAST(char***, &list_entries))) {
            CdlParse::report_error(interp, diag_target + name, Tcl_GetStringResult(tcl_interp));
        } else {
            for (int i = 0; i < list_count; i++) {
                target->enable.push_back(list_entries[i]);
            }
            Tcl_Free((char *) list_entries);
        }
    }

    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

                           
// Syntax: disable { opt1 opt2 ... }
// For example: disable { CYGPKG_HAL_ARM_CL7xxx_7111 }
int
CdlDbParser::target_disable(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::target_disable", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_targetname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::target_data* target =
        static_cast<CdlPackagesDatabaseBody::target_data*>(interp->get_assoc_data(dbparser_targetdata));

    // There should be one argument, a list of valid flags.
    if (2 != argc) {
        CdlParse::report_error(interp, diag_target + name, "`disable' should be followed by a list of CDL options.");
    } else {
        int          list_count     = 0;
        const char** list_entries   = 0;
        Tcl_Interp* tcl_interp      = interp->get_tcl_interpreter();
        if (TCL_OK != Tcl_SplitList(tcl_interp, CDL_TCL_CONST_CAST(char*, argv[1]), &list_count, CDL_TCL_CONST_CAST(char***, &list_entries))) {
            CdlParse::report_error(interp, diag_target + name, Tcl_GetStringResult(tcl_interp));
        } else {
            for (int i = 0; i < list_count; i++) {
                target->disable.push_back(list_entries[i]);
            }
            Tcl_Free((char *) list_entries);
        }
    }

    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

// Syntax: set_value <option> <value>
// For example: set_value CYGHWR_MEMSIZE 0x100000
int
CdlDbParser::target_set_value(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("CdlDbParser::target_set_value", "result %d");
    CYG_REPORT_FUNCARG1XV(argc);
    CYG_PRECONDITION_CLASSC(interp);

    std::string name = interp->get_variable(dbparser_targetname);
    CYG_ASSERTC("" != name);
    CdlPackagesDatabaseBody::target_data* target =
        static_cast<CdlPackagesDatabaseBody::target_data*>(interp->get_assoc_data(dbparser_targetdata));

    // There should be one argument, a list of valid flags.
    if (3 != argc) {
        CdlParse::report_error(interp, diag_target + name, "`set_value' should be followed by an option name and its value.");
    } else {
        target->set_values.push_back(std::make_pair(std::string(argv[1]), std::string(argv[2])));
    }

    CYG_REPORT_RETVAL(TCL_OK);
    return TCL_OK;
}

//}}}

//}}}
//{{{  CdlPackagesDatabase:: creation                           

// ----------------------------------------------------------------------------
// The exported interface is make(). The hard work is done inside the
// constructor.

CdlPackagesDatabase
CdlPackagesDatabaseBody::make(std::string repo, CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn)
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackagesDatabase::make", "database %p");

    // Where is the component repository? The location may come from the
    // parent or from an environment variable ECOS_REPOSITORY
    if ("" == repo) {
        char *env = getenv("ECOS_REPOSITORY");
        if (0 == env) {
            throw CdlInputOutputException(std::string("No component repository specified and no ") +
                                          std::string("ECOS_REPOSITORY environment variable"));
        } else {
            repo = env;
        }
    }

    // Replace any backslashes in the repository with forward slashes.
    // The latter are used throughout the library
    // NOTE: this is not i18n-friendly.
    for (unsigned int i = 0; i < repo.size(); i++) {
        if ('\\' == repo[i]) {
            repo[i] = '/';
        }
    }
    CdlPackagesDatabase result = new CdlPackagesDatabaseBody(repo, error_fn, warn_fn);
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------

CdlPackagesDatabaseBody::CdlPackagesDatabaseBody(std::string repo, CdlDiagnosticFnPtr error_fn, CdlDiagnosticFnPtr warn_fn)
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase:: constructor");
    CYG_PRECONDITIONC("" != repo);

    // There will be calls to check_this() while the database is evaluated,
    // so make sure that the database is valid first.
    component_repository                = repo;
    cdlpackagesdatabasebody_cookie      = CdlPackagesDatabaseBody_Magic;
    
    // We want to read in the entire packages file. Portability problems
    // can be largely eliminated by using a Tcl interpreter for this, but
    // under Windows there is a problem if the pathname is a cygwin one.
    // For now it is assumed that the supplied pathname is acceptable to
    // Tcl.
    //
    // No attempt is made at this stage to use a safe interpreter.
    // Some file I/O operations are needed while processing the data,
    // for example to check that a package is actually installed.
    // Additional file I/O may prove useful in future, e.g. to create
    // some or all of a database on the fly. Obviously some
    // restrictions are desirable (no modify access to the repository,
    // no network capabilities, and so on.) These have to be added
    // in future.

    CdlInterpreter interp = CdlInterpreterBody::make();

    try {
        
        CdlInterpreterBody::ContextSupport context(interp, database_name);
        CdlInterpreterCommandEntry commands[] =
        {
            CdlInterpreterCommandEntry("package",  &CdlDbParser::new_package  ),
            CdlInterpreterCommandEntry("target",   &CdlDbParser::new_target   ),
            CdlInterpreterCommandEntry("",         0                          )
        };
        CdlInterpreterBody::CommandSupport cmds(interp, commands);
        CdlInterpreterBody::DiagSupport diag(interp, error_fn, warn_fn);
        CdlInterpreterBody::AssocSupport assoc(interp, dbparser_database_key, static_cast<ClientData>(this));
        CdlInterpreterBody::VariableSupport var(interp, dbparser_component_repository, repo);
        interp->add_command("unknown", &CdlParse::unknown_command);
        CdlParse::clear_error_count(interp);

        // Ignore errors at this stage, instead check error count at the end.
        (void) interp->eval_file(component_repository + "/" + database_name);
    
        // Now start looking for templates. These should reside in the
        // templates subdirectory of the component repository. Each template
        // should be in its own directory, and inside each directory should
        // be versioned template files with a .ect extension.
        std::string templates_dir = repo + "/" + "templates";
        std::vector<std::string> subdirs;
        interp->locate_subdirs(templates_dir, subdirs);

        unsigned int i;
        for (i = 0; i < subdirs.size(); i++) {
            // Do not add the template to the known ones until we are sure there is
            // at least one valid template.
            std::vector<std::string> files;
            interp->locate_files(templates_dir + "/" + subdirs[i], files);
            unsigned int j;
            for (j = 0; j < files.size(); j++) {
                if ((4 < files[j].size()) && (".ect" == files[j].substr(files[j].size() - 4))) {
                    break;
                }
            }
            if (j != files.size()) {
                this->template_names.push_back(subdirs[i]);
                for ( ; j < files.size(); j++) {
                    if ((4 < files[j].size()) && (".ect" == files[j].substr(files[j].size() - 4))) {
                        this->templates[subdirs[i]].versions.push_back(files[j].substr(0, files[j].size() - 4));
                    }
                }
            }
        }

        // Consistency checks. All target-specific packages should
        // have the hardware attribute. Also, all the packages should
        // exist. Problems only result in warnings and only when
        // operating in verbose mode, to allow for somewhat
        // inconsistent repositories e.g. an anoncvs tree.
        if (CdlPackagesDatabaseBody::verbose_mode) {
            std::vector<std::string>::const_iterator name_i;
            std::vector<std::string>::const_iterator name_j;
            for (name_i = target_names.begin(); name_i != target_names.end(); name_i++) {
                for (name_j = targets[*name_i].packages.begin(); name_j != targets[*name_i].packages.end(); name_j++) {
                    if (std::find(package_names.begin(), package_names.end(), *name_j) == package_names.end()) {
                        CdlParse::report_warning(interp, diag_target + *name_i,
                                                 std::string("This target refers to an unknown package `") + *name_j + "'.");
                    }
                    if (!packages[*name_j].hardware) {
                        CdlParse::report_warning(interp, diag_target + *name_i,
                                                 std::string("This target refers to a package `") + *name_j +
                                                 "' that is not hardware-specific.");
                    }
                }
            }
        }
        // Now, were there any errors while reading in the database?
        // If so it is necessary to throw an exception here, to make sure
        // that things get cleaned up properly.
        int error_count = CdlParse::get_error_count(interp);
        if (0 != error_count) {
            throw CdlInputOutputException("Invalid package database.");
        }
    } catch(...) {
        // Something has gone wrong. Clear out all of the data accumulated so far, as well
        // as the interpreter.
        delete interp;
        package_names.clear();
        target_names.clear();
        template_names.clear();
        packages.clear();
        targets.clear();
        templates.clear();
        throw;
    }

    delete interp;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlPackagesDatabase:: destructor                         

// ----------------------------------------------------------------------------
CdlPackagesDatabaseBody::~CdlPackagesDatabaseBody()
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase:: default destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlpackagesdatabasebody_cookie      = CdlPackagesDatabaseBody_Invalid;
    component_repository                = "";
    package_names.clear();
    target_names.clear();
    template_names.clear();
    packages.clear();
    targets.clear();
    templates.clear();

    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlPackagesDatabase:: check_this()                       

// ----------------------------------------------------------------------------

bool
CdlPackagesDatabaseBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlPackagesDatabaseBody_Magic != cdlpackagesdatabasebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();

    switch(zeal) {
      case cyg_system_test :
      case cyg_extreme :
      {
          std::vector<std::string>::const_iterator              names_i;
          std::map<std::string,package_data>::const_iterator    pkgs_i;
          
          // Every entry in the names vector should have an entry in the packages vector.
          for (names_i = package_names.begin(); names_i != package_names.end(); names_i++) {
              if (packages.find(*names_i) == packages.end()) {
                  return false;
              }
          }
          // The inverse should be true as well
          for (pkgs_i = packages.begin(); pkgs_i != packages.end(); pkgs_i++) {
              if (std::find(package_names.begin(), package_names.end(), pkgs_i->first) == package_names.end()) {
                  return false;
              }
          }
          
          // Repeat for targets.
          std::map<std::string,target_data>::const_iterator     targets_i;
          for (names_i = target_names.begin(); names_i != target_names.end(); names_i++) {
              if (targets.find(*names_i) == targets.end()) {
                  return false;
              }
          }
          for (targets_i = targets.begin(); targets_i != targets.end(); targets_i++) {
              if (std::find(target_names.begin(), target_names.end(), targets_i->first) == target_names.end()) {
                  return false;
              }
          }

          // And for templates
          std::map<std::string,template_data>::const_iterator    templates_i;
          for (names_i = template_names.begin(); names_i != template_names.end(); names_i++) {
              if (templates.find(*names_i) == templates.end()) {
                  return false;
              }
          }
          // The inverse should be true as well
          for (templates_i = templates.begin(); templates_i != templates.end(); templates_i++) {
              if (std::find(template_names.begin(), template_names.end(), templates_i->first) == template_names.end()) {
                  return false;
              }
          }
          
          // Possibly the package directories should be validated as
          // well, not to mention the various version subdirectories,
          // but doing file I/O inside an assertion is excessive.
      }
      case cyg_thorough :
      case cyg_quick:
          if ("" == component_repository) {
              return false;
          }
      case cyg_trivial:
      case cyg_none :
        break;
    }

    return true;
}

//}}}
//{{{  CdlPackagesDatabase:: misc                               

// ----------------------------------------------------------------------------

std::string
CdlPackagesDatabaseBody::get_component_repository() const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_component_repository");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return component_repository;
}

void
CdlPackagesDatabaseBody::set_verbose(bool new_mode)
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::set_verbose");
    CYG_REPORT_FUNCARG1XV(new_mode);

    verbose_mode = new_mode;
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlPackagesDatabase:: get package information            

// ----------------------------------------------------------------------------

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_packages(void) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_packages");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return package_names;
}

bool
CdlPackagesDatabaseBody::is_known_package(std::string name) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackagesDatabase::is_known_package", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    if (std::find(package_names.begin(), package_names.end(), name) != package_names.end()) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

const std::string&
CdlPackagesDatabaseBody::get_package_description(std::string pkg_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_package_description");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::map<std::string,package_data>::const_iterator pkgs_i = packages.find(pkg_name);
    if (pkgs_i != packages.end()) {
        CYG_REPORT_RETURN();
        return pkgs_i->second.description;
    }
    
    CYG_FAIL("Invalid package name passed to CdlPackagesDatabase::get_package_description()");
    static std::string dummy = "";
    return dummy;
}

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_package_aliases(std::string pkg_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_package_aliases");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::map<std::string,package_data>::const_iterator pkgs_i = packages.find(pkg_name);
    if (pkgs_i != packages.end()) {
        CYG_REPORT_RETURN();
        return pkgs_i->second.aliases;
    }
    
    CYG_FAIL("Invalid package name passed to CdlPackagesDatabase::get_package_aliases()");
    static std::vector<std::string> dummy;
    return dummy;
}

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_package_versions(std::string pkg_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_package_versions");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::map<std::string,package_data>::const_iterator pkgs_i = packages.find(pkg_name);
    if (pkgs_i != packages.end()) {
        CYG_REPORT_RETURN();
        return pkgs_i->second.versions;
    }
    
    CYG_FAIL("Invalid package name passed to CdlPackagesDatabase::get_package_versions()");
    static std::vector<std::string> dummy;
    return dummy;
}

const std::string&
CdlPackagesDatabaseBody::get_package_directory(std::string pkg_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_package_directory");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::map<std::string,package_data>::const_iterator pkgs_i = packages.find(pkg_name);
    if (pkgs_i != packages.end()) {
        CYG_REPORT_RETURN();
        return pkgs_i->second.directory;
    }
    
    CYG_FAIL("Invalid package name passed to CdlPackagesDatabase::get_package_directory()");
    static std::string dummy = "";
    return dummy;
}

const std::string&
CdlPackagesDatabaseBody::get_package_script(std::string pkg_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_package_script");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::map<std::string,package_data>::const_iterator pkgs_i = packages.find(pkg_name);
    if (pkgs_i != packages.end()) {
        CYG_REPORT_RETURN();
        return pkgs_i->second.script;
    }
    
    CYG_FAIL("Invalid package name passed to CdlPackagesDatabase::get_package_script()");
    static std::string dummy = "";
    return dummy;
}

bool
CdlPackagesDatabaseBody::is_hardware_package(std::string pkg_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::is_hardware_package");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::map<std::string,package_data>::const_iterator pkgs_i = packages.find(pkg_name);
    if (pkgs_i != packages.end()) {
        CYG_REPORT_RETURN();
        return pkgs_i->second.hardware;
    }
    
    CYG_FAIL("Invalid package name passed to CdlPackagesDatabase::is_hardware_package()");
    return false;
}

//}}}
//{{{  CdlPackagesDatabase:: get target information             

// ----------------------------------------------------------------------------

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_targets(void) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_targets");
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return target_names;
}

bool
CdlPackagesDatabaseBody::is_known_target(std::string name) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackagesDatabase::is_known_target", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    if (std::find(target_names.begin(), target_names.end(), name) != target_names.end()) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

const std::string&
CdlPackagesDatabaseBody::get_target_description(std::string target_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_target_description");
    CYG_PRECONDITION_THISC();

    std::map<std::string,target_data>::const_iterator target_i = targets.find(target_name);
    if (target_i != targets.end()) {
        CYG_REPORT_RETURN();
        return target_i->second.description;
    }
    
    CYG_FAIL("Invalid target name passed to CdlPackagesDatabase::get_target_description()");
    static std::string dummy = "";
    return dummy;
}

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_target_aliases(std::string target_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_target_aliases");
    CYG_PRECONDITION_THISC();

    std::map<std::string,target_data>::const_iterator target_i = targets.find(target_name);
    if (target_i != targets.end()) {
        CYG_REPORT_RETURN();
        return target_i->second.aliases;
    }
    
    CYG_FAIL("Invalid target name passed to CdlPackagesDatabase::get_target_aliases()");
    static std::vector<std::string> dummy;
    return dummy;
}

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_target_packages(std::string target_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_target_packages");
    CYG_PRECONDITION_THISC();

    std::map<std::string,target_data>::const_iterator target_i = targets.find(target_name);
    if (target_i != targets.end()) {
        CYG_REPORT_RETURN();
        return target_i->second.packages;
    }
    
    CYG_FAIL("Invalid target name passed to CdlPackagesDatabase::get_target_packages()");
    static std::vector<std::string> dummy;
    return dummy;
}

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_target_enables(std::string target_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_target_enables");
    CYG_PRECONDITION_THISC();

    std::map<std::string,target_data>::const_iterator target_i = this->targets.find(target_name);
    if (target_i != this->targets.end()) {
        CYG_REPORT_RETURN();
        return target_i->second.enable;
    }
    
    CYG_FAIL("Invalid target name passed to CdlPackagesDatabase::get_target_enables()");
    static std::vector<std::string> dummy;
    return dummy;
}

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_target_disables(std::string target_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_target_disables");
    CYG_PRECONDITION_THISC();

    std::map<std::string,target_data>::const_iterator target_i = this->targets.find(target_name);
    if (target_i != this->targets.end()) {
        CYG_REPORT_RETURN();
        return target_i->second.disable;
    }
    
    CYG_FAIL("Invalid target name passed to CdlPackagesDatabase::get_target_disables()");
    static std::vector<std::string> dummy;
    return dummy;
}

const std::vector<std::pair<std::string, std::string> >&
CdlPackagesDatabaseBody::get_target_set_values(std::string target_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_target_set_values");
    CYG_PRECONDITION_THISC();

    std::map<std::string,target_data>::const_iterator target_i = this->targets.find(target_name);
    if (target_i != this->targets.end()) {
        CYG_REPORT_RETURN();
        return target_i->second.set_values;
    }
    
    CYG_FAIL("Invalid target name passed to CdlPackagesDatabase::get_target_values()");
    static std::vector<std::pair<std::string, std::string> > dummy;
    return dummy;
}

//}}}
//{{{  CdlPackagesDatabase:: get template information           

// ----------------------------------------------------------------------------
// Templates are different from packages and targets. The ecos.db file
// does not contain all the information in one convenient file, instead
// it is necessary to trawl through a templates sub-directory of the
// component repository. There are no aliases. Descriptions can be obtained
// only by executing the template file in a suitable interpreter.

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_templates(void) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_templates");
    CYG_PRECONDITION_THISC();

    CYG_REPORT_RETURN();
    return template_names;
}

bool
CdlPackagesDatabaseBody::is_known_template(std::string name) const
{
    CYG_REPORT_FUNCNAMETYPE("CdlPackagesDatabase::is_known_template", "result %d");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    bool result = false;
    if (std::find(template_names.begin(), template_names.end(), name) != template_names.end()) {
        result = true;
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_template_versions(std::string template_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_template_versions");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::map<std::string,template_data>::const_iterator template_i = templates.find(template_name);
    if (template_i != templates.end()) {
        CYG_REPORT_RETURN();
        return template_i->second.versions;
    }
    
    CYG_FAIL("Invalid template name passed to CdlPackagesDatabase::get_template_versions()");
    static std::vector<std::string> dummy;
    return dummy;
}

std::string
CdlPackagesDatabaseBody::get_template_filename(std::string template_name, std::string version_name) const
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_template_filename");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // Given the way templates are identified, the filename can be determined
    // easily by concatenating a few strings. The only complication is that
    // version_name may be an empty string, indicating that the most recent
    // version should be used.
    std::map<std::string,template_data>::const_iterator template_i = templates.find(template_name);
    if (template_i == templates.end()) {
        CYG_FAIL("Invalid template name passed to CdlPackagesDatabase::get_template_filename");
        CYG_REPORT_RETURN();
        return "";
    }
    if ("" == version_name) {
        CYG_ASSERTC(0 != template_i->second.versions.size());
        version_name = template_i->second.versions[0];
    } else {
        std::vector<std::string>::const_iterator vsn_i = std::find(template_i->second.versions.begin(),
                                                                   template_i->second.versions.end(), version_name);
        if (vsn_i == template_i->second.versions.end()) {
            CYG_FAIL("Invalid template version passed to CdlPackagesDatabase::get_template_filename");
            CYG_REPORT_RETURN();
            return "";
        }
    }

    std::string result = component_repository + "/templates/" + template_name + "/" + version_name + ".ect";
    CYG_REPORT_RETURN();
    return result;
}

// ----------------------------------------------------------------------------
// The descriptions now live in an eCos savefile, i.e. a Tcl script, so
// extracting them can be relatively expensive and needs to happen on
// a just-in-time basis.

const std::string
CdlPackagesDatabaseBody::get_template_description(std::string template_name, std::string version_name)
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_template_description");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    // Is this a known template?
    std::map<std::string, struct template_data>::iterator template_i = templates.find(template_name);
    if (template_i == templates.end()) {
        CYG_FAIL("Invalid template name passed to CdlPackagesDatabase::get_template_description");
        CYG_REPORT_RETURN();
        return "";
    }

    // Is it a known version of the template?
    if ("" == version_name) {
        CYG_ASSERTC(0 != template_i->second.versions.size());
        version_name = template_i->second.versions[0];
    } else {
        if (std::find(template_i->second.versions.begin(), template_i->second.versions.end(), version_name) ==
            template_i->second.versions.end()) {

            CYG_FAIL("Invalid template version passed to CdlPackagesDatabase::get_template_description");
            CYG_REPORT_RETURN();
            return "";
        }
    }

    // We have a valid template and version. Has the version file in
    // question been read in yet?
    std::map<std::string, struct template_version_data>::iterator version_i;
    version_i = template_i->second.version_details.find(version_name);
    if (version_i != template_i->second.version_details.end()) {
        CYG_REPORT_RETURN();
        return version_i->second.description;
    }
    
    std::string filename = this->get_template_filename(template_name, version_name);
    if ("" == filename) {
        CYG_REPORT_RETURN();
        return "";
    } 
    extract_template_details(filename, template_i->second.version_details[version_name].description,
                             template_i->second.version_details[version_name].packages);
    CYG_REPORT_RETURN();
    return template_i->second.version_details[version_name].description;
}

// ----------------------------------------------------------------------------
// Similarly extracting package information needs to happen on a
// just-in-time basis.
const std::vector<std::string>&
CdlPackagesDatabaseBody::get_template_packages(std::string template_name, std::string version_name)
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_template_packages");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    static std::vector<std::string> dummy;
    
    // Is this a known template?
    std::map<std::string, struct template_data>::iterator template_i = templates.find(template_name);
    if (template_i == templates.end()) {
        CYG_FAIL("Invalid template name passed to CdlPackagesDatabase::get_template_packages");
        CYG_REPORT_RETURN();
        return dummy;
    }

    // Is it a known version of the template?
    if ("" == version_name) {
        CYG_ASSERTC(0 != template_i->second.versions.size());
        version_name = template_i->second.versions[0];
    } else {
        if (std::find(template_i->second.versions.begin(), template_i->second.versions.end(), version_name) ==
            template_i->second.versions.end()) {

            CYG_FAIL("Invalid template version passed to CdlPackagesDatabase::get_packages");
            CYG_REPORT_RETURN();
            return dummy;
        }
    }

    // We have a valid template and version. Has the version file in
    // question been read in yet?
    std::map<std::string, struct template_version_data>::iterator version_i;
    version_i = template_i->second.version_details.find(version_name);
    if (version_i != template_i->second.version_details.end()) {
        CYG_REPORT_RETURN();
        return version_i->second.packages;
    }
    
    std::string filename = this->get_template_filename(template_name, version_name);
    if ("" == filename) {
        CYG_REPORT_RETURN();
        return dummy;
    } 
    extract_template_details(filename, template_i->second.version_details[version_name].description,
                             template_i->second.version_details[version_name].packages);
    CYG_REPORT_RETURN();
    return template_i->second.version_details[version_name].packages;
}

// ----------------------------------------------------------------------------
// Extracting the description and package information involves running
// the script through a Tcl interpreter extended with the appropriate
// commands. Most of the savefile information is irrelevant and is handled
// by extract_ignore(). The commands of interest are cdl_configuration and
// its sub-commands description and package.

static int
extract_ignore(CdlInterpreter interp, int argc, const char* argv[])
{
    return TCL_OK;
}

static int
extract_cdl_configuration(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("extract_cdl_configuration", "result %d");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_PRECONDITION_CLASSC(interp);

    int result = TCL_OK;
    
    // usage: cdl_configuration <name> <body>
    if (3 != argc) {
        interp->set_result("Invalid cdl_configuration command in template, expecting two arguments");
        result = TCL_ERROR;
    } else {
        // Ignore the first argument for now.
        std::string tmp;
        result = interp->eval(argv[2], tmp);
        
        // After processing the cdl_configuration command the description and
        // package information should be known. There is no point in processing
        // the rest of the file.
        if (TCL_OK == result) {
            interp->set_result("OK");
            result = TCL_ERROR;
        }
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

static int
extract_cdl_description(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("extract_cdl_description", "result %d");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_PRECONDITION_CLASSC(interp);

    int result = TCL_OK;
    
    // usage: package <name>
    if (2 != argc) {
        interp->set_result("Invalid description command in template, expecting just one argument");
        result = TCL_ERROR;
    } else {
        ClientData client_data = interp->get_assoc_data(template_description_key);
        CYG_ASSERTC(0 != client_data);
        std::string* result_ptr = static_cast<std::string*>(client_data);
        *result_ptr = argv[1];
    }

    CYG_REPORT_RETVAL(result);
    return result;
}

static int
extract_cdl_package(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("extract_cdl_package", "result %d");
    CYG_REPORT_FUNCARG2XV(interp, argc);
    CYG_PRECONDITION_CLASSC(interp);

    int result = TCL_OK;
    
    // usage: package <name> <version>
    if (2 > argc) {
        interp->set_result("Invalid package command in template, expecting two arguments");
        result = TCL_ERROR;
    } else {
        ClientData client_data = interp->get_assoc_data(template_packages_key);
        CYG_ASSERTC(0 != client_data);
        std::vector<std::string>* result_ptr = static_cast<std::vector<std::string>*>(client_data);
        result_ptr->push_back(argv[1]);
    }
    CYG_REPORT_RETVAL(result);
    return result;
}


void
CdlPackagesDatabaseBody::extract_template_details(std::string filename, std::string& description,
                                                      std::vector<std::string>& packages)
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::extract_template_description");

    CdlInterpreter interp = CdlInterpreterBody::make();
    interp->set_assoc_data(template_description_key, static_cast<ClientData>(&description));
    interp->set_assoc_data(template_packages_key,    static_cast<ClientData>(&packages));
    static CdlInterpreterCommandEntry extract_commands[] =
    {
        CdlInterpreterCommandEntry("cdl_savefile_version",  &extract_ignore                 ),
        CdlInterpreterCommandEntry("cdl_savefile_command",  &extract_ignore                 ),
        CdlInterpreterCommandEntry("cdl_configuration",     &extract_cdl_configuration      ),
        CdlInterpreterCommandEntry("hardware",              &extract_ignore                 ),
        CdlInterpreterCommandEntry("template",              &extract_ignore                 ),
        CdlInterpreterCommandEntry("description",           &extract_cdl_description        ),
        CdlInterpreterCommandEntry("package",               &extract_cdl_package            ),
        CdlInterpreterCommandEntry("unknown",               &extract_ignore                 ),
        CdlInterpreterCommandEntry("",                      0                               )
    };
    std::vector<CdlInterpreterCommandEntry> new_commands;
    for (int i = 0; 0 != extract_commands[i].command; i++) {
        new_commands.push_back(extract_commands[i]);
    }
    interp->push_commands(new_commands);

    std::string tmp;
    int result = interp->eval_file(filename, tmp);
    // Special escape mechanism, see extract_cdl_configuration() above
    if ((TCL_ERROR == result) && ("OK" == tmp)) {
        result = TCL_OK;
    }
#if 0    
    if (TCL_OK != result) {
        // No obvious way of recovering just yet
    }
#endif
    delete interp;
                           
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlPackagesDatabase:: get_valid_cflags()                 

// ----------------------------------------------------------------------------

const std::vector<std::string>&
CdlPackagesDatabaseBody::get_valid_cflags()
{
    CYG_REPORT_FUNCNAME("CdlPackagesDatabase::get_valid_compiler_flags");

    static std::vector<std::string> result_vec;
    static const char* valid_flags[] = {
        "ARCHFLAGS",  "CARCHFLAGS",  "CXXARCHFLAGS",  "LDARCHFLAGS",
        "ERRFLAGS",   "CERRFLAGS",   "CXXERRFLAGS",   "LDERRFLAGS",
        "LANGFLAGS",  "CLANGFLAGS",  "CXXLANGFLAGS",  "LDLANGFLAGS",
        "DBGFLAGS",   "CDBGFLAGS",   "CXXDBGFLAGS",   "LDDBGFLAGS",
        "EXTRAFLAGS", "CEXTRAFLAGS", "CXXEXTRAFLAGS", "LDEXTRAFLAGS",
        0
    };

    if (0 == result_vec.size()) {
        for (int i = 0; 0 != valid_flags[i]; i++) {
            result_vec.push_back(valid_flags[i]);
        }
    }
    CYG_REPORT_RETURN();
    return result_vec;
}

//}}}
