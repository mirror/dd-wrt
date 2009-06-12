//{{{  Banner                           

//==========================================================================
//
//      build.cxx
//
//      libcdl support for building and for header file generation
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 2002, 2003 Bart Veer
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
//==========================================================================
//#####DESCRIPTIONBEGIN####                                             
//
// Author(s):           bartv
// Contributors:        bartv
// Date:                1999-06-018
//
//####DESCRIPTIONEND####
//==========================================================================

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
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlBuildLoadableBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlBuildableBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlDefineLoadableBody);
CYGDBG_DEFINE_MEMLEAK_COUNTER(CdlDefinableBody);

//}}}
//{{{  CdlBuildableBody                 

//{{{  Basics                           

// ----------------------------------------------------------------------------
// There is little data specific to a buildable. The only distinguishing
// feature is the set of properties that are supported, plus a handful
// of functions to extract that information.

CdlBuildableBody::CdlBuildableBody()
{
    CYG_REPORT_FUNCNAME("CdlBuildable:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    // There is no data to initialize yet
    cdlbuildablebody_cookie = CdlBuildableBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlBuildableBody::~CdlBuildableBody()
{
    CYG_REPORT_FUNCNAME("CdlBuildable:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlbuildablebody_cookie = CdlBuildableBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------

std::string
CdlBuildableBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlBuildable::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "buildable";
}

// ----------------------------------------------------------------------------

bool
CdlBuildableBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlBuildableBody_Magic != cdlbuildablebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlNodeBody::check_this(zeal);
}

//}}}
//{{{  Add and check property parsers   

// ----------------------------------------------------------------------------

void
CdlBuildableBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlBuildable::add_property_parsers");

    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("compile",            &CdlBuildableBody::parse_compile    ),
        CdlInterpreterCommandEntry("object",             &CdlBuildableBody::parse_object     ),
        CdlInterpreterCommandEntry("make_object",        &CdlBuildableBody::parse_make_object),
        CdlInterpreterCommandEntry("make",               &CdlBuildableBody::parse_make       ),
        CdlInterpreterCommandEntry("build_proc",         &CdlBuildableBody::parse_build_proc ),
        CdlInterpreterCommandEntry("",                   0                                   ),
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

void
CdlBuildableBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlBuildable::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    // There are no real constraints on the number of compile
    // properties etc.
    // TODO: check that the relevant sources files exist,
    //       unless marked appropriately (build_proc can create
    //       new source files).
    
    CdlNodeBody::check_properties(interp);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Property parsers                 

// ----------------------------------------------------------------------------
// Syntax: compile <file1 file2 ...>
//
// There are a couple of checks that could be performed here:
//
// 1) does each listed file actually exist? Unfortunately that approach
//    falls foul of build_proc, which is allowed to generate source files
//    on the fly.
//
// 2) does the file have a recognised suffix such as .c or .cxx. This
//    relies libcdl having some way of knowing how to treat different
//    files.
//
// For now there are no validity checks.
//
// A future extension may allow dependencies to be listed, as an
// option. This would allow component vendors to specify that
// particular custom build steps should happen before particular
// compilations, a more robust approach than the current priority
// scheme.

int
CdlBuildableBody::parse_compile(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_compile", "result %d");
    static char* options[] = {
        "library:",
        0
    };

    int result = CdlParse::parse_stringvector_property(interp, argc, argv, CdlPropertyId_Compile, options, 0, true);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// A utility to break a custom build step down into its three components.
//
// A custom build step takes the form:
//     target : deps
//         rules
//
// This utility function takes a single string of this form and breaks
// it down into its constituent parts. 
//
// NOTE: this will need lots of extra code in future to allow for
// escaped characters, spaces in filenames, etc. For now just keep
// things simple.

bool
CdlBuildableBody::split_custom_build_step(std::string str_data, std::string& target, std::string& deps, std::string& rules,
                                          std::string& error_msg)
{
    CYG_REPORT_FUNCNAMETYPE("CdlBuildable::split_custom_build_step", "result %d");

    target      = "";
    deps        = "";
    rules       = "";
    error_msg   = "";

    const char* data  = str_data.c_str();

    // Skip any leading white space, and make sure that this leaves some real data.
    while (('\0' != *data) && isspace(*data)) {
        data++;
    }
    if ('\0' == *data) {
        error_msg = "no data in custom build_step";
        CYG_REPORT_RETVAL(false);
        return false;
    }

    // Now extract the target. This consists of any sequence of characters
    // upto space, tab, colon.
    for ( ; ('\0' != *data) && (':' != *data) && (' ' != *data) && ('\t' != *data); data++) {
        target += *data;
    }
    // Discard any spaces or tabs, they are of no interest
    while ((' ' == *data) || ('\t' == *data)) {
        data++;
    }
    // The current character should be a colon
    if (':' != *data) {
        error_msg = "expecting a colon `;' after the target `" + target + "'";
        CYG_REPORT_RETVAL(false);
        return false;
    }

    // Move past the colon, and skip any further spaces or tabs
    data++;
    while (('\0' != *data) && ((' ' == *data) || ('\t' == *data))) {
        data++;
    }

    // Everything from here until the end of line should be part of the deps field,
    // including white space. 
    while (('\0' != *data) && ('\n' != *data) && (';' != *data)) {
        deps += *data++;
    }

    if ("" == deps) {
        error_msg = "expecting dependency list after `" + target + ":'";
        CYG_REPORT_RETVAL(false);
        return false;
    }

    // Having some rules is compulsory.
    if ('\0' == *data) {
        error_msg = "expecting one or more rules after the dependency list";
        CYG_REPORT_RETVAL(false);
        return false;
    } else {
        // We are currently at \n or ;, move on to the actual rules
        data++;
    }

    // Rules consist of one or more lines. Any leading white space on a given
    // line should be discarded.
    while ('\0' != *data) {
        // Processing the current rule. Skip leading spaces and tabs
        while ((' ' == *data) || ('\t' == *data)) {
            data++;
        }
        // Now add everything up to the next newline or EOD to the rules.
        while (('\0' != *data) && ('\n' != *data)) {
            rules += *data++;
        }

        // Terminate this line of the rules with a newline, even if that
        // character is absent from the raw data.
        rules += '\n';

        // And ignore the newline in the raw data itself
        if ('\n' == *data) {
            data++;
        }
    }

    // Better make sure that there are some rules. All of the looping above
    // may just have left white space
    if ("" == rules) {
        error_msg = "no rules provided";
        CYG_REPORT_RETVAL(false);
        return false;
    }

    // Everything is ok.
 
    CYG_REPORT_RETVAL(true);
    return true;
}


// ----------------------------------------------------------------------------
// syntax: make <target> <rules>
//
// There is rather a lot of checking to be done.
//
// 1) the priority should be valid. In particular it should be a number
//    within a reasonable range.
//
// 2) the rules should take the form:
//    <target> : <deps> ;|\n rules
//
//    Where the target should be a single file, identical to the
//    first property argument.

static void
parse_make_final_check(CdlInterpreter interp, CdlProperty_String prop)
{
    CYG_REPORT_FUNCNAME("parse_make_final_check");
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITION_CLASSC(prop);

    std::string prio_string = prop->get_option("priority");
    if ("" != prio_string) {
        cdl_int tmp = 1;
        if (!Cdl::string_to_integer(prio_string, tmp)) {
            CdlParse::report_property_parse_error(interp, prop,
                                                  "Invalid priority option, priorities should be simple numbers.");
        } else {
            if ((tmp < 1) || (tmp > 1024)) {
                CdlParse::report_property_parse_error(interp, prop,
                                                      "Invalid priority value, priorities should be in the range 1 to 1024.");
            }
        }
    }
    
    std::string data    = prop->get_string();
    std::string target;
    std::string deps;
    std::string rules;
    std::string error_msg;

    if (!CdlBuildableBody::split_custom_build_step(data, target, deps, rules, error_msg)) {
        CdlParse::report_property_parse_error(interp, prop, "Invalid custom build step, " + error_msg);
    }
}

int
CdlBuildableBody::parse_make(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_make", "result %d");
    static char* options[] = {
        "priority:",
        0
    };

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Make, options, &parse_make_final_check);
    
    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
// syntax: make_object <target> <rules>
//
// The rules here are much the same as for the "make" property.

static void
parse_make_object_final_check(CdlInterpreter interp, CdlProperty_String prop)
{
    CYG_REPORT_FUNCNAME("parse_make_object_final_check");
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITION_CLASSC(prop);


    std::string prio_string = prop->get_option("priority");
    if ("" != prio_string) {
        cdl_int tmp = 1;
        if (!Cdl::string_to_integer(prio_string, tmp)) {
            CdlParse::report_property_parse_error(interp, prop,
                                                  "Invalid priority option, priorities should be simple numbers.");
        } else {
            if ((tmp < 1) || (tmp > 1024)) {
                CdlParse::report_property_parse_error(interp, prop,
                                                      "Invalid priority value, priorities should be in the range 1 to 1024.");
            }
        }
    }
    
    std::string data    = prop->get_string();
    std::string target;
    std::string deps;
    std::string rules;
    std::string error_msg;

    if (!CdlBuildableBody::split_custom_build_step(data, target, deps, rules, error_msg)) {
        CdlParse::report_property_parse_error(interp, prop, "Invalid custom build step, " + error_msg);
    }
}

int
CdlBuildableBody::parse_make_object(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_make_object", "result %d");
    static char* options[] = {
        "library:",
        "priority:",
        0
    };

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_MakeObject, options,
                                                 &parse_make_object_final_check);
    
    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
// Syntax: object <file1> <file2> ...

int
CdlBuildableBody::parse_object(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_object", "result %d");
    static char* options[] = {
        "library:",
        0
    };

    int result = CdlParse::parse_stringvector_property(interp, argc, argv, CdlPropertyId_Object, options, 0, true);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: build_proc { tcl code }

int
CdlBuildableBody::parse_build_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_build_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_BuildProc, 0, 0);

    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  update_build_info()              

// ----------------------------------------------------------------------------
// Most of the work is done in update_all_build_info(). The update_build_info()
// merely checks the active and enabled state first.
void
CdlBuildableBody::update_build_info(CdlBuildInfo_Loadable& build_info, std::string library) const
{
    CYG_REPORT_FUNCNAME("CdlBuildable::update_build_info");
    CYG_REPORT_FUNCARG2XV(this, &build_info);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != library);

    if (!is_active()) {
        CYG_REPORT_RETURN();
        return;
    }

    CdlConstValuable valuable = dynamic_cast<CdlConstValuable>(this);
    if (0 != valuable) {
        if (!valuable->is_enabled()) {
            CYG_REPORT_RETURN();
            return;
        }
    }

    update_all_build_info(build_info, library);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  update_all_build_info()          

// ----------------------------------------------------------------------------
// There are four properties to be considered, each of which may occur
// multiple times: "compile", "object", "make_object", and "make".
// Each of these will result in separate additions to the build_info
// structure.

void
CdlBuildableBody::update_all_build_info(CdlBuildInfo_Loadable& build_info, std::string package_library) const
{
    CYG_REPORT_FUNCNAME("CdlBuildable::update_all_build_info");
    CYG_REPORT_FUNCARG2XV(this, &build_info);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITIONC("" != package_library);

    // Get some information about the owning loadable first.
    CdlLoadable loadable        = get_owner();
    CYG_ASSERT_CLASSC(loadable);
    std::string directory       = loadable->get_directory();
    CYG_ASSERTC("" != directory);
    CdlInterpreter interp       = loadable->get_interpreter();
    CYG_ASSERT_CLASSC(interp);

    // The interpreter needs some information about the locations
    // of various things. This code has to be kept in step with
    // CdlLoadable::find_relative_file()
    interp->set_variable("::cdl_topdir", get_toplevel()->get_directory());
    interp->set_variable("::cdl_pkgdir", directory);

    // For many packages the sources will reside in a src subdirectory.
    // For simple packages the sources may live directly at the toplevel
    bool has_src_subdir    = loadable->has_subdirectory("src");

    // NOTE: the object property is not yet supported
    std::vector<CdlProperty> compile_properties;
    get_properties(CdlPropertyId_Compile, compile_properties);
    std::vector<CdlProperty> makeobject_properties;
    get_properties(CdlPropertyId_MakeObject, makeobject_properties);
    std::vector<CdlProperty> make_properties;
    get_properties(CdlPropertyId_Make, make_properties);
    std::vector<CdlProperty>::const_iterator prop_i;

    for (prop_i = compile_properties.begin(); prop_i != compile_properties.end(); prop_i++) {
        CdlProperty_StringVector compile_prop = dynamic_cast<CdlProperty_StringVector>(*prop_i);
        CYG_LOOP_INVARIANT_CLASSC(compile_prop);

        // Does this property have a library option?
        std::string current_library = compile_prop->get_option("library");
        if ("" == current_library) {
            current_library = package_library;
        }

        const std::vector<std::string>& files   = compile_prop->get_strings();
        std::vector<std::string>::const_iterator file_i;

        for (file_i = files.begin(); file_i != files.end(); file_i++) {

            // For each listed file, try to find it. If this is unsuccessful
            // then assume that the file will be generated later on.
            std::string path = loadable->find_relative_file(*file_i, "src");
            if ("" == path) {
                if (has_src_subdir) {
                    path = "src/" + *file_i;
                } else {
                    path = *file_i;
                }
            }

            // Now check whether or not the specified file is already present.
            std::vector<CdlBuildInfo_Compile>::const_iterator info_i;
            for (info_i = build_info.compiles.begin(); info_i != build_info.compiles.end(); info_i++) {
                if ((current_library == info_i->library) && (path == info_i->source)) {
                    break;
                }
            }
            if (info_i == build_info.compiles.end()) {
                CdlBuildInfo_Compile new_info;
                new_info.library    = current_library;
                new_info.source     = path;
                build_info.compiles.push_back(new_info);
            }
        }
    }

    for (prop_i = makeobject_properties.begin(); prop_i != makeobject_properties.end(); prop_i++) {
        CdlProperty_String prop = dynamic_cast<CdlProperty_String>(*prop_i);
        CYG_LOOP_INVARIANT_CLASSC(prop);

        // Does thie property have a library option?
        std::string current_library = prop->get_option("library");
        if ("" == current_library) {
            current_library = package_library;
        }

        // How about a priority field? The default priority for make_object is 100
        // We can rely on the validation done during the parsing process
        cdl_int priority = 100;
        std::string priority_option = prop->get_option("priority");
        if ("" != priority_option) {
            Cdl::string_to_integer(priority_option, priority);
        }

        // What we need now is the separate target, deps, and rules. These
        // can be obtained via a utility. The raw data will have been validated
        // already.
        std::string raw_data = prop->get_string();
        std::string target;
        std::string deps;
        std::string rules;
        std::string error_msg;
        bool result;

        result = CdlBuildableBody::split_custom_build_step(raw_data, target, deps, rules, error_msg);
        CYG_ASSERTC(true == result);

        // Construct a local object, then copy it into the vector
        CdlBuildInfo_MakeObject local_copy;
        local_copy.priority     = priority;
        local_copy.library      = current_library;
        local_copy.object       = target;
        local_copy.deps         = deps;
        local_copy.rules        = rules;

        build_info.make_objects.push_back(local_copy);
    }
    
    for (prop_i = make_properties.begin(); prop_i != make_properties.end(); prop_i++) {
        CdlProperty_String prop = dynamic_cast<CdlProperty_String>(*prop_i);
        CYG_LOOP_INVARIANT_CLASSC(prop);

        // Is there a priority field? The default priority for make is
        // 300 We can rely on the validation done during the parsing
        // process
        cdl_int priority = 300;
        std::string priority_option = prop->get_option("priority");
        if ("" != priority_option) {
            Cdl::string_to_integer(priority_option, priority);
        }

        // What we need now is the separate target, deps, and rules. These
        // can be obtained via a utility. The raw data will have been validated
        // already.
        std::string raw_data = prop->get_string();
        std::string target;
        std::string deps;
        std::string rules;
        std::string error_msg;
        bool result;

        result = CdlBuildableBody::split_custom_build_step(raw_data, target, deps, rules, error_msg);
        CYG_ASSERTC(true == result);

        // Construct a local object, then copy it into the vector
        CdlBuildInfo_Make local_copy;
        local_copy.priority     = priority;
        local_copy.target       = target;
        local_copy.deps         = deps;
        local_copy.rules        = rules;

        build_info.makes.push_back(local_copy);
    }
    
    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  CdlBuildLoadableBody             

//{{{  Class variables                          

// ----------------------------------------------------------------------------
// This variable controls the default library that should be generated.
// Some applications may wish to override this.
char* CdlBuildLoadableBody::default_library_name        = "libtarget.a";

// The pattern that should be used to identify header files.
// FIXME: this information should come out of a data file
char* CdlBuildLoadableBody::default_headers_glob_pattern = "*.h *.hxx *.inl *.si *.inc";

//}}}
//{{{  The simple stuff                         

// ----------------------------------------------------------------------------

CdlBuildLoadableBody::CdlBuildLoadableBody()
    : CdlLoadableBody()
{
    CYG_REPORT_FUNCNAME("CdlBuildLoadable:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    // There is no data to initialize
    cdlbuildloadablebody_cookie = CdlBuildLoadableBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlBuildLoadableBody::~CdlBuildLoadableBody()
{
    CYG_REPORT_FUNCNAME("CdlBuildLoadable:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdlbuildloadablebody_cookie = CdlBuildLoadableBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------

std::string
CdlBuildLoadableBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlBuildLoadable::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "build_loadable";
}

// ----------------------------------------------------------------------------

bool
CdlBuildLoadableBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlBuildLoadableBody_Magic != cdlbuildloadablebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlContainerBody::check_this(zeal) && CdlNodeBody::check_this(zeal);
}

//}}}
//{{{  Property parsers                         

// ----------------------------------------------------------------------------

void
CdlBuildLoadableBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlBuildLoadable::add_property_parsers");

    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("library",            &CdlBuildLoadableBody::parse_library       ),
        CdlInterpreterCommandEntry("makefile",           &CdlBuildLoadableBody::parse_makefile      ),
        CdlInterpreterCommandEntry("include_dir",        &CdlBuildLoadableBody::parse_include_dir   ),
        CdlInterpreterCommandEntry("include_files",      &CdlBuildLoadableBody::parse_include_files ),
        CdlInterpreterCommandEntry("",                   0                                          )
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
    
    CYG_REPORT_RETURN();
}

void
CdlBuildLoadableBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlBuildLoadable::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    CdlNodeBody::check_properties(interp);
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// syntax: library <filename>
//
// NOTE: there should probably be a check that the library name is in
// a valid format, i.e. libxxx.a

int
CdlBuildLoadableBody::parse_library(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_library", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Library, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// syntax: makefile <filename>
//
// NOTE: possibly there should be a check that the makefile exists.
// Do we want to allow build_proc's to generate makefiles?
int
CdlBuildLoadableBody::parse_makefile(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_makefile", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Makefile, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// syntax: include_dir <directory name>
int
CdlBuildLoadableBody::parse_include_dir(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_include_dir", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_IncludeDir, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: include_files <file1 file2 ...>
//
// This lists the header files that should be copied into the install tree
// as part of the build operation. In the absence of an include_files property
// there should be an include subdirectory, and all files in that subdirectory
// are assumed to be exportable headers.
//
// NOTE: add a finalizer to check that the files exist or get created.

int
CdlBuildLoadableBody::parse_include_files(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_include_files", "result %d");

    int result = CdlParse::parse_stringvector_property(interp, argc, argv, CdlPropertyId_IncludeFiles, 0, 0, true);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  update_build_info()                      

// ----------------------------------------------------------------------------
// This utility routine takes care of filling in a Buildinfo_Loadable
// structure with the appropriate header file information. This involves
// the following:
//
// 1) there may be an include_dir property. This affects the destination
//     of any header files that are listed.
//
// 2) the loadable may or may not have an include subdirectory. For
//    non-trivial packages the include subdirectory provides a clean
//    way of separating interface and implementation. For simple
//    packages it is too heavyweight.
//
// 3) there may be one or more include_files property. If so then these
//    specify all the files that should be exported.
//
// 4) otherwise if there is an include subdirectory then we need to
//    know all the header files in that subdirectory. A Tcl script is
//    used for this.
//
// 5) otherwise all the header files below the package directory itself
//    are of interest.

static void
update_header_file_info(CdlConstBuildLoadable loadable, CdlBuildInfo_Loadable& build_info)
{
    CYG_REPORT_FUNCNAME("update_header_file_info");
    CYG_REPORT_FUNCARG2XV(loadable, &build_info);
    CYG_PRECONDITION_CLASSC(loadable);

    std::string dest_dir = "";
    CdlProperty include_dir_prop = loadable->get_property(CdlPropertyId_IncludeDir);
    if (0 != include_dir_prop) {
        CdlProperty_String strprop = dynamic_cast<CdlProperty_String>(include_dir_prop);
        CYG_ASSERT_CLASSC(strprop);
        dest_dir = strprop->get_string();
    }

    bool has_include_subdir = loadable->has_subdirectory("include");

    std::vector<CdlProperty> include_file_properties;
    loadable->get_properties(CdlPropertyId_IncludeFiles, include_file_properties);
    if (include_file_properties.size() > 0) {
        std::vector<CdlProperty>::const_iterator prop_i;
        for (prop_i = include_file_properties.begin(); prop_i != include_file_properties.end(); prop_i++) {
            
            CdlProperty_StringVector strvprop = dynamic_cast<CdlProperty_StringVector>(*prop_i);
            CYG_ASSERT_CLASSC(strvprop);

            const std::vector<std::string>& filenames = strvprop->get_strings();
            std::vector<std::string>::const_iterator file_i;
            for (file_i = filenames.begin(); file_i != filenames.end(); file_i++) {
                std::string path = loadable->find_relative_file(*file_i, "include");
                // Assume that the header file will be generated by a build_proc
                if ("" == path) {
                    if (has_include_subdir) {
                        path = "include/" + *file_i;
                    } else {
                        path = *file_i;
                    }
                }
                CdlBuildInfo_Header local_copy;
                local_copy.source       = path;
                local_copy.destination  = "";
                if ("" != dest_dir) {
                    local_copy.destination = dest_dir + "/";
                }
                // At this stage "path" may begin with "include/", which should not
                // be present in the destination.
                const char* tmp = path.c_str();
                if (0 == strncmp("include/", tmp, 8)) {
                    local_copy.destination += &(tmp[8]);
                } else {
                    local_copy.destination += path;
                }
                build_info.headers.push_back(local_copy);
            }
        }
        CYG_REPORT_RETURN();
        return;
    }

    // It is necessary to search for the appropriate files.
    CdlInterpreter interp = loadable->get_interpreter();
    std::string    path   = loadable->get_toplevel()->get_directory() + "/" + loadable->get_directory();
    if (has_include_subdir) {
        std::vector<std::string> files;
        std::vector<std::string>::const_iterator file_i;
        interp->locate_all_files(path + "/include", files);
        for (file_i = files.begin(); file_i != files.end(); file_i++) {
            // NOTE: for now discard any header files in the pkgconf subdirectory
            if (0 == strncmp("pkgconf/", file_i->c_str(), 8)) {
                continue;
            }
            if ('~' == *(file_i->rbegin())) {
                continue;
            }
            CdlBuildInfo_Header local_copy;
            local_copy.source   = "include/" + *file_i;
            local_copy.destination      = "";
            if ("" != dest_dir) {
                local_copy.destination = dest_dir + "/";
            }
            local_copy.destination += *file_i;
            build_info.headers.push_back(local_copy);
        }
    } else {
        // Look for all header files, which for now means files with
        // a .h, .hxx, .inl or .inc extension.
        // FIXME: the definition of what constitutes a header file
        // should not be hard-wired here.
        std::vector<std::string> files;
        std::vector<std::string>::const_iterator file_i;
        interp->locate_all_files(path, files);
        for (file_i = files.begin(); file_i != files.end(); file_i++) {

            // Problems with libstdc++ versions, use C comparisons instead.
            const char* c_string = file_i->c_str();
            unsigned int len = strlen(c_string);
            if (((len >= 2) && (0 == strncmp(c_string + len - 2, ".h", 2)))        ||
                ((len >= 4) && (0 == strncmp(c_string + len - 4, ".hxx", 4)))      ||
                ((len >= 4) && (0 == strncmp(c_string + len - 4, ".inl", 4)))      ||
                ((len >= 4) && (0 == strncmp(c_string + len - 4, ".inc", 4)))) {
                
                CdlBuildInfo_Header local_copy;
                local_copy.source = *file_i;
                local_copy.destination = "";
                if ("" != dest_dir) {
                    local_copy.destination = dest_dir + "/";
                }
                local_copy.destination += *file_i;
                build_info.headers.push_back(local_copy);
            }
        }
    }

    CYG_REPORT_RETURN();
    return;
}

// ----------------------------------------------------------------------------
// Updating a loadable build's info involves two steps. First, there
// is some information associated with the loadable as a whole such as
// header files. Second, each buildable in the loadable (including itself)
// may contain properties such as compile etc. This is all handled via
// a CdlBuildable member function.

void
CdlBuildLoadableBody::update_build_info(CdlBuildInfo& build_info) const
{
    CYG_REPORT_FUNCNAME("CdlBuildLoadable::update_build_info");
    CYG_REPORT_FUNCARG2XV(this, &build_info);
    CYG_PRECONDITION_THISC();

    // It is not possible to disable a loadable itself: either the
    // loadable is present or it is not (although in some cases users
    // may be able to change versions). However, because of reparenting
    // it is possible for a loadable to be below a disabled container,
    // and hence it is still necessary to check whether or not the
    // loadable is active.
    if (!is_active()) {
        CYG_REPORT_RETURN();
        return;
    }

    // Time to add a new CdlBuildInfo_Loadable object to the current
    // vector. The name and directory can be filled in straightaway,
    // the vectors will all be initialized to empty.
    CdlBuildInfo_Loadable tmp_info;
    build_info.entries.push_back(tmp_info);
    CdlBuildInfo_Loadable& this_info = *(build_info.entries.rbegin());
    this_info.name      = get_name();
    this_info.directory = get_directory();

    // Take care of the header files
    update_header_file_info(this, this_info);
    
    // Work out the library name appropriate for this loadable.
    // There may be a library property, otherwise the global default
    // should be used.
    std::string loadable_library = default_library_name;
    if (has_property(CdlPropertyId_Library)) {
        CdlProperty_String strprop = dynamic_cast<CdlProperty_String>(get_property(CdlPropertyId_Library));
        loadable_library = strprop->get_string();
    }
    
    const std::vector<CdlNode>& contents = get_owned();
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
        CdlBuildable buildable = dynamic_cast<CdlBuildable>(*node_i);
        if (0 != buildable) {
            buildable->update_build_info(this_info, loadable_library);
        }
    }
    
    CYG_REPORT_RETURN();
}

// This is much the same as the above, but there is no test for
// active either at the loadable level or for the individual buildables.
void
CdlBuildLoadableBody::update_all_build_info(CdlBuildInfo& build_info) const
{
    CYG_REPORT_FUNCNAME("CdlBuildLoadable::update_all_build_info");
    CYG_REPORT_FUNCARG2XV(this, &build_info);
    CYG_PRECONDITION_THISC();

    CdlBuildInfo_Loadable tmp_info;
    build_info.entries.push_back(tmp_info);
    CdlBuildInfo_Loadable& this_info = *(build_info.entries.rbegin());
    this_info.name      = get_name();
    this_info.directory = get_directory();

    std::string loadable_library = default_library_name;
    if (has_property(CdlPropertyId_Library)) {
        CdlProperty_String strprop = dynamic_cast<CdlProperty_String>(get_property(CdlPropertyId_Library));
        loadable_library = strprop->get_string();
    }
    
    const std::vector<CdlNode>& contents = get_owned();
    std::vector<CdlNode>::const_iterator node_i;
    for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
        CdlBuildable buildable = dynamic_cast<CdlBuildable>(*node_i);
        if (0 != buildable) {
            buildable->update_build_info(this_info, loadable_library);
        }
    }
    
    CYG_REPORT_RETURN();
    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  Version number #define's         

// ----------------------------------------------------------------------------
// Given a package xxxPKG_A_B_C with a version V1_2_3, generate additional
// #define's of the form:
//
//   #define xxxNUM_A_B_C_VERSION_MAJOR 1
//   #define xxxNUM_A_B_C_VERSION_MINOR 2
//   #define xxxNUM_A_B_C_VERSION_RELEASE 3
//
// The goal here is to allow application code to cope with API
// changes (which of course should be a rare event but cannot be
// eliminated completely). C preprocessor #if statements are
// essentially limited to numerical values, so there is no easy
// way of coping with V1_2_3 at the preprocessor level. However it
// is possible to cope with VERSION_NUMBER #define's.
//
// Note that only application code and third party packages are
// affected. 
//
// These #define's go into system.h, alongside the main definition of
// the package. There seems to be little point in putting them in the
// package's own configuration header.
//
// There are three problems. First, what should be done for packages
// which do not follow the naming conventions? Given a completely
// random package rather than something like xxxPKG_..., what symbol
// names should be used? Basically, if the package does not follow the
// naming convention then there is no safe way of generating new
// symbols. Any names that are chosen might clash. Of course even for
// packages that do follow the naming convention a clash is still
// possible, just a lot less likely.
//
// Conclusion: if a package does not follow the naming convention, do
// not generate version #define's for it.
//
// Second, what happens if a different version numbering scheme is
// used? For example the release number might be absent. Version
// numbering schemes might change between releases, but application
// code may still check the #define's.
//
// Third and related, what should happen for "current" and anoncvs? Do
// we want to look at what other versions are installed and bump one
// of the numbers?
//
// Conclusion: the version #define's always have to be generated,
// even if they are not present in the version string, to allow
// application code to test these symbols anyway. A safe default is
// necessary, and -1 is probably the best bet. For example, if
// the version is bumped from 1.3.287 to 1.4 then the release number
// for the latter is set to -1. Another possible default would be
// 0, but that could cause problems for packages that start counting
// from 0 (not a common practice, but...)
//
// This leaves the question of what to do about "current". Chances are
// that "current" comes from anoncvs and is always more recent than
// any official release, so when comparing versions "current" should
// always be greater than anything else. This can be achieved by using
// a sufficiently large number for the major version. In practice
// it is cleaner to have another #define to indicate the current
// version, and then define package versions to match, i.e.:
//
//   #define CYGNUM_VERSION_CURRENT 0x7fffff00
//   ...
//   #define xxxNUM_A_B_C_VERSION_MAJOR   CYGNUM_VERSION_CURRENT
//   #define xxxNUM_A_B_C_VERSION_MINOR   -1
//   #define xxxNUM_A_B_C_VERSION_RELEASE -1
//
// All comparisons should now work sensibly. Leaving a little bit
// of slack for VERSION_CURRENT seems like a good precaution.

static void
system_h_add_version_header(Tcl_Channel system_h)
{
    CYG_REPORT_FUNCNAME("system_h_add_version_header");
    Tcl_Write(system_h, "#define CYGNUM_VERSION_CURRENT 0x7fffff00\n", -1);
    CYG_REPORT_RETURN();
}

static void
system_h_add_package_versioning(Tcl_Channel system_h, std::string name, std::string value)
{
    CYG_REPORT_FUNCNAME("system_h_add_package_versioning");

    char name_buf[256];
    char line_buf[512];
    
    // The first thing to check is that the package name can be used
    // as the basis for the version symbols.
    bool ok = false;
    unsigned int i;
    for (i = 0; i < name.size(); i++) {
        if ('_' == name[i]) {
            if (3 < i) {
                if ((name[i-3] == 'P') && (name[i-2] == 'K') && (name[i-1] == 'G')) {
                    ok = true;
                }
            }
            break;
        }
    }
    if (name.size() >= 256) {
        ok = false;
    }
    if (!ok) {
        CYG_REPORT_RETURN();
        return;
    }

    strcpy(name_buf, name.c_str());
    
    // Change from xxxPKG to xxxNUM
    name_buf[i - 3] = 'N';
    name_buf[i - 2] = 'U';
    name_buf[i - 1] = 'M';

    // Now determine the version strings.
    std::string major   = "-1";
    std::string minor   = "-1";
    std::string release = "-1";
    if ("current" == value) {
        major   = "CYGNUM_VERSION_CURRENT";
    } else {
        Cdl::split_version_string(value, major, minor, release);
    }

    sprintf(line_buf, "#define %s_VERSION_MAJOR %s\n", name_buf, major.c_str());
    Tcl_Write(system_h, line_buf, -1);
    sprintf(line_buf, "#define %s_VERSION_MINOR %s\n", name_buf, minor.c_str());
    Tcl_Write(system_h, line_buf, -1);
    sprintf(line_buf, "#define %s_VERSION_RELEASE %s\n", name_buf, release.c_str());
    Tcl_Write(system_h, line_buf, -1);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlDefinableBody                 

//{{{  Basics                                           

// ----------------------------------------------------------------------------

CdlDefinableBody::CdlDefinableBody()
{
    CYG_REPORT_FUNCNAME("CdlDefinable:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    // There is no data to initialize
    cdldefinablebody_cookie = CdlDefinableBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlDefinableBody::~CdlDefinableBody()
{
    CYG_REPORT_FUNCNAME("CdlDefinable:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdldefinablebody_cookie = CdlDefinableBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------

std::string
CdlDefinableBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlDefinable::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "definable";
}

// ----------------------------------------------------------------------------

bool
CdlDefinableBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlDefinableBody_Magic != cdldefinablebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlNodeBody::check_this(zeal);
}

//}}}
//{{{  add_property_parser() and check_properties()     

// ----------------------------------------------------------------------------

void
CdlDefinableBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlDefinable::add_property_parsers");

    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("no_define",          &parse_no_define     ),
        CdlInterpreterCommandEntry("define",             &parse_define        ),
        CdlInterpreterCommandEntry("define_format",      &parse_define_format ),
        CdlInterpreterCommandEntry("define_proc",        &parse_define_proc   ),
        CdlInterpreterCommandEntry("if_define",          &parse_if_define     ),
        CdlInterpreterCommandEntry("",                   0                    )
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

void
CdlDefinableBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlDefinable::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    // There should be at most one each of no_define and define_format.
    if (count_properties(CdlPropertyId_NoDefine) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one no_define property.");
    }
    if (count_properties(CdlPropertyId_DefineFormat) > 1) {
        CdlParse::report_error(interp, "", "There should be at most one define_format property.");
    }
    if (has_property(CdlPropertyId_NoDefine) && has_property(CdlPropertyId_DefineFormat)) {
        CdlParse::report_error(interp, "", "The no_define and define_format properties are mutually exclusive.");
    }
    // FIXME: the define_format property only makes sense for certain
    // flavors. However the flavor property may not have been processed yet.
    
    CdlNodeBody::check_properties(interp);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  Definable properties                             

// ----------------------------------------------------------------------------
// Syntax: no_define
int
CdlDefinableBody::parse_no_define(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_no_define", "result %d");

    int result = CdlParse::parse_minimal_property(interp, argc, argv, CdlPropertyId_NoDefine, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// syntax: define <symbol>

// The argument to "define" should be a valid C preprocessor symbol.
static void
parse_define_final_check(CdlInterpreter interp, CdlProperty_String prop)
{
    CYG_REPORT_FUNCNAME("parse_define_final_check");
    CYG_PRECONDITION_CLASSC(prop);
    CYG_PRECONDITION_CLASSC(interp);
    
    const std::string& str = prop->get_string();

    if (!Cdl::is_valid_c_preprocessor_symbol(str)) {
        CdlParse::report_property_parse_error(interp, prop, str + " is not a valid C preprocessor symbol");
    }

    // There may be a file option. At this stage the only valid filename
    // that can be used here is system.h
    std::string file_option = prop->get_option("file");
    if (("" != file_option) && ("system.h" != file_option)) {
        CdlParse::report_property_parse_error(interp, prop, "Invalid -file option " + file_option);
    }

    // FIXME: validate the format string
    
    CYG_REPORT_RETURN();
}

int
CdlDefinableBody::parse_define(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_define", "result %d");

    static char* options[] = {
        "file:",
        "format:",
        0
    };
    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_Define, options, &parse_define_final_check);
    
    CYG_REPORT_RETVAL(result);
    return result;
}


// ----------------------------------------------------------------------------
// syntax: define_format <string>
//
// FIXME: it is possible to apply some checks to the string, e.g. that there
// is only one conversion operation.
//
// FIXME: also check that the flavor is sensible, define_format has no effect
// for none or bool
//
// FIXME: enforce mutual exclusion with no_define

int
CdlDefinableBody::parse_define_format(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_format", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_DefineFormat, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}
// ----------------------------------------------------------------------------
// syntax: define_proc <tclcode>
int
CdlDefinableBody::parse_define_proc(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_define_proc", "result %d");

    int result = CdlParse::parse_tclcode_property(interp, argc, argv, CdlPropertyId_DefineProc, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

// ----------------------------------------------------------------------------
// Syntax: if_define sym1 sym2

static void
parse_if_define_final_check(CdlInterpreter interp, CdlProperty_StringVector prop)
{
    CYG_REPORT_FUNCNAME("parse_if_define_final_check");
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITION_CLASSC(prop);
    
    // There should be exactly two entries in the vector, and both of them should be
    // valid preprocessor symbols.
    const std::vector<std::string>& strings     = prop->get_strings();

    if (2 != strings.size()) {
        CdlParse::report_property_parse_error(interp, prop, "There should be exactly two arguments.");
    }
    if (!Cdl::is_valid_c_preprocessor_symbol(strings[0])) {
        CdlParse::report_property_parse_error(interp, prop, strings[0] + " is not a valid C preprocessor symbol.");
    }
    if (!Cdl::is_valid_c_preprocessor_symbol(strings[1])) {
        CdlParse::report_property_parse_error(interp, prop, strings[1] + " is not a valid C preprocessor symbol.");
    }
    
    // There may be a file option. At this stage the only valid filename
    // that can be used here is system.h
    std::string file_option = prop->get_option("file");
    if (("" != file_option) && ("system.h" != file_option)) {
        CdlParse::report_property_parse_error(interp, prop, "Invalid -file option " + file_option);
    }
}

int
CdlDefinableBody::parse_if_define(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_if_define", "result %d");

    char* options[] = {
        "file:",
        0
    };
    int result = CdlParse::parse_stringvector_property(interp, argc, argv, CdlPropertyId_IfDefine, options,
                                                       &parse_if_define_final_check, false);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  generate_config_header()                         

// ----------------------------------------------------------------------------
// This code needs to allow for the following properties.
//
// 1) no_define. This suppresses the default #define generation. 
//
// 2) define_format <format_string.
//
// 3) define [-file <filename>][-format <format_string>] symbol
//
// 4) define_proc
//
// 5) if_define

void
CdlDefinableBody::generate_config_header(Tcl_Channel this_hdr, Tcl_Channel system_h) const
{
    CYG_REPORT_FUNCNAME("CdlDefinable::generate_config_header");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlLoadable    loadable = get_owner();
    CdlInterpreter interp   = loadable->get_interpreter();
    
    // This definable is known to be active. However it may or may not be enabled.
    CYG_PRECONDITIONC(is_active());

    std::string      name   = get_name();
    CdlValueFlavor   flavor = CdlValueFlavor_Bool;
    std::string      value  = "1";
    CdlConstValuable valuable        = dynamic_cast<CdlConstValuable>(this);
    if (0 != valuable) {
        // It is always possible to check the enabled() flag. 
        if (!valuable->is_enabled()) {
            CYG_REPORT_RETURN();
            return;
        }
        // The value is only valid for BoolData and Data flavors, and may
        // not have been provided. If there is no value then this option
        // should not generate a #define
        flavor = valuable->get_flavor();
        if ((CdlValueFlavor_BoolData == flavor) || (CdlValueFlavor_Data == flavor)) {
            value = valuable->get_value();
        }
    }

    // Flavor and value are now both set to sensible strings.
    // First, check the no_define property. If this is present then the default
    // #define generation should be suppressed.
    if (!has_property(CdlPropertyId_NoDefine)) {

        // OK, it is necessary to generate at least one #define.
        // If this node is actually a loadable then the #define should go
        // into system.h, otherwise into the current header
        Tcl_Channel chan = this_hdr;
        if (dynamic_cast<CdlConstLoadable>((CdlConstNode)this) == loadable) {
            chan = system_h;
        }

        // For flavors None and Bool, there should be just one #define
        if ((CdlValueFlavor_None == flavor) || (CdlValueFlavor_Bool == flavor)) {
            std::string define = "#define " + name + " 1\n";
            Tcl_Write(chan, const_cast<char*>(define.c_str()), -1);
        } else {
            // If there is a format string then that controls the default
            // value display.
            if (!has_property(CdlPropertyId_DefineFormat)) {
                std::string define = "#define " + name + " " + value + "\n";
                Tcl_Write(chan, const_cast<char*>(define.c_str()), -1);
            } else {
                CdlProperty_String strprop = dynamic_cast<CdlProperty_String>(get_property(CdlPropertyId_DefineFormat));
                CYG_ASSERT_CLASSC(strprop);
                std::string format = strprop->get_string();
                std::string cmd = "return \"#define " + name + " [format " + format + " " + value + "]\n\"";
                std::string define;
                if (TCL_OK != interp->eval(cmd, define)) {
                    throw CdlInputOutputException("Internal error executing tcl fragment to process define_format property");
                }
                Tcl_Write(chan, const_cast<char*>(define.c_str()), -1);
            }

            // There may also be a separate #define of the form <name>_<value>,
            // if that is a valid preprocessor symbol.
            std::string tmp = name + "_" + value;
            if (Cdl::is_valid_c_preprocessor_symbol(tmp)) {
                tmp = "#define "+ tmp + "\n";
                Tcl_Write(chan, const_cast<char*>(tmp.c_str()), -1);
            }
            
            // For loadables, add additional version information to system_h
            if (dynamic_cast<CdlConstLoadable>((CdlConstNode)this) == loadable) {
                system_h_add_package_versioning(system_h, name, value);
            }
        }
    }

    // Next, check for any additional define properties
    std::vector<CdlProperty> define_props;
    get_properties(CdlPropertyId_Define, define_props);
    std::vector<CdlProperty>::const_iterator prop_i;
    for (prop_i = define_props.begin(); prop_i != define_props.end(); prop_i++) {
        CdlProperty_String strprop = dynamic_cast<CdlProperty_String>(*prop_i);
        CYG_ASSERT_CLASSC(strprop);
        std::string symbol = strprop->get_string();

        std::string file = strprop->get_option("file");
        Tcl_Channel chan = this_hdr;
        if (("" != file) && ("system.h" == file)) {
            chan = system_h;
        }

        if ((CdlValueFlavor_None == flavor) || (CdlValueFlavor_Bool == flavor)) {
            std::string define = "#define " + symbol + " 1\n";
            Tcl_Write(chan, const_cast<char*>(define.c_str()), -1);
        } else {
            std::string format = strprop->get_option("format");
            if ("" == format) {
                std::string define = "#define " + symbol + " " + value + "\n";
                Tcl_Write(chan, const_cast<char*>(define.c_str()), -1);
            } else {
                std::string cmd = "return \"#define " + symbol + " [format " + format + " " + value + "]\n\"";
                std::string define;
                if (TCL_OK != interp->eval(cmd, define)) {
                    throw CdlInputOutputException("Internal error executing tcl fragment to process format option");
                }
                Tcl_Write(chan, const_cast<char*>(define.c_str()), -1);
            }

            std::string tmp = symbol + "_" + value;
            if (Cdl::is_valid_c_preprocessor_symbol(tmp)) {
                tmp = "#define " + tmp + "\n";
                Tcl_Write(chan, const_cast<char*>(tmp.c_str()), -1);
            }
        }
    }

    // Now check for if_define properties
    std::vector<CdlProperty> if_define_props;
    get_properties(CdlPropertyId_IfDefine, if_define_props);
    for (prop_i = if_define_props.begin(); prop_i != if_define_props.end(); prop_i++) {
        CdlProperty_StringVector strprop = dynamic_cast<CdlProperty_StringVector>(*prop_i);
        CYG_ASSERT_CLASSC(strprop);
        CYG_ASSERTC(2 == strprop->get_number_of_strings());

        std::string sym1 = strprop->get_string(0);
        std::string sym2 = strprop->get_string(1);

        Tcl_Channel chan = this_hdr;
        std::string file = strprop->get_option("file");
        if (("" != file) && ("system.h" == file)) {
            chan = system_h;
        }
        std::string data = "#ifdef " + sym1 + "\n# define " + sym2 + " 1\n#endif\n";
        Tcl_Write(chan, const_cast<char*>(data.c_str()), -1);
    }

    // And define_proc properties
    std::vector<CdlProperty> define_proc_props;
    get_properties(CdlPropertyId_DefineProc, define_proc_props);
    for (prop_i = define_proc_props.begin(); prop_i != define_proc_props.end(); prop_i++) {
        CdlProperty_TclCode codeprop = dynamic_cast<CdlProperty_TclCode>(*prop_i);
        CYG_ASSERT_CLASSC(codeprop);

        cdl_tcl_code code       = codeprop->get_code();
        std::string  result;
        if (TCL_OK != interp->eval(code, result)) {
            throw CdlInputOutputException("Error evaluating define_proc property for " + name + "\n" + result);
        }
    }
    
    
    CYG_REPORT_RETURN();
}

//}}}

//}}}
//{{{  CdlDefineLoadableBody            

//{{{  Basics                           

// ----------------------------------------------------------------------------

CdlDefineLoadableBody::CdlDefineLoadableBody()
{
    CYG_REPORT_FUNCNAME("CdlDefineLoadable:: default constructor");
    CYG_REPORT_FUNCARG1XV(this);

    cdldefineloadablebody_cookie = CdlDefineLoadableBody_Magic;
    CYGDBG_MEMLEAK_CONSTRUCTOR();
    
    CYG_POSTCONDITION_THISC();
    CYG_REPORT_RETURN();
}

CdlDefineLoadableBody::~CdlDefineLoadableBody()
{
    CYG_REPORT_FUNCNAME("CdlDefineLoadable:: destructor");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    cdldefineloadablebody_cookie = CdlDefineLoadableBody_Invalid;
    CYGDBG_MEMLEAK_DESTRUCTOR();
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------

std::string
CdlDefineLoadableBody::get_class_name() const
{
    CYG_REPORT_FUNCNAME("CdlDefineLoadable::get_class_name");
    CYG_PRECONDITION_THISC();
    CYG_REPORT_RETURN();
    return "define_loadable";
}

// ----------------------------------------------------------------------------

bool
CdlDefineLoadableBody::check_this(cyg_assert_class_zeal zeal) const
{
    if (CdlDefineLoadableBody_Magic != cdldefineloadablebody_cookie) {
        return false;
    }
    CYGDBG_MEMLEAK_CHECKTHIS();
    return CdlLoadableBody::check_this(zeal) && CdlNodeBody::check_this(zeal);
}

//}}}
//{{{  Property parsing                 

// ----------------------------------------------------------------------------

void
CdlDefineLoadableBody::add_property_parsers(std::vector<CdlInterpreterCommandEntry>& parsers)
{
    CYG_REPORT_FUNCNAME("CdlDefineLoadable::add_property_parsers");

    static CdlInterpreterCommandEntry commands[] =
    {
        CdlInterpreterCommandEntry("define_header",      &parse_define_header),
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

void
CdlDefineLoadableBody::check_properties(CdlInterpreter interp)
{
    CYG_REPORT_FUNCNAME("CdlDefineLoadable::check_properties");
    CYG_REPORT_FUNCARG2XV(this, interp);
    CYG_PRECONDITION_THISC();
    CYG_PRECONDITION_CLASSC(interp);

    // There should be at most one define_header property
    int count = count_properties(CdlPropertyId_DefineHeader);
    if (count> 1) {
        CdlParse::report_error(interp, "", "There should be at most one define_header property.");
    }
    // FIXME: filename validation
    
    CdlNodeBody::check_properties(interp);
    
    CYG_REPORT_RETURN();
}

// ----------------------------------------------------------------------------
// syntax: define_header <header file name>
int
CdlDefineLoadableBody::parse_define_header(CdlInterpreter interp, int argc, const char* argv[])
{
    CYG_REPORT_FUNCNAMETYPE("parse_define_header", "result %d");

    int result = CdlParse::parse_string_property(interp, argc, argv, CdlPropertyId_DefineHeader, 0, 0);
    
    CYG_REPORT_RETVAL(result);
    return result;
}

//}}}
//{{{  generate_config_header()         

// ----------------------------------------------------------------------------
void
CdlDefineLoadableBody::generate_config_header(Tcl_Channel this_hdr, Tcl_Channel system_h) const
{
    CYG_REPORT_FUNCNAME("CdlDefineLoadable::generate_config_header");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    CdlInterpreter interp       = get_interpreter();
    Tcl_RegisterChannel(interp->get_tcl_interpreter(), this_hdr);
    Tcl_RegisterChannel(interp->get_tcl_interpreter(), system_h);

    CdlInterpreterBody::ContextSupport(interp, std::string("Package ") + this->get_name() + ", header file generation");
    
    try {
        interp->set_variable("::cdl_header", Tcl_GetChannelName(this_hdr));
        interp->set_variable("::cdl_system_header", Tcl_GetChannelName(system_h));

        const std::vector<CdlNode>& contents = get_owned();
        std::vector<CdlNode>::const_iterator node_i;
        for (node_i = contents.begin(); node_i != contents.end(); node_i++) {
            CdlDefinable definable = dynamic_cast<CdlDefinable>(*node_i);
            if (0 == definable) {
                continue;
            }
            if (!definable->is_active()) {
                continue;
            }
            definable->generate_config_header(this_hdr, system_h);
        }
    
        interp->unset_variable("::cdl_header");
        interp->unset_variable("::cdl_system_header");
    } catch(...) {
        Tcl_UnregisterChannel(interp->get_tcl_interpreter(), this_hdr);
        Tcl_UnregisterChannel(interp->get_tcl_interpreter(), system_h);
        throw;
    }
    
    Tcl_UnregisterChannel(interp->get_tcl_interpreter(), this_hdr);
    Tcl_UnregisterChannel(interp->get_tcl_interpreter(), system_h);
    
    CYG_REPORT_RETURN();
}

//}}}
//{{{  get_config_headers()             

// ----------------------------------------------------------------------------
// What header file should be generated for this loadable?
//
// If there is a define_header property then this should be used.
// Otherwise a filename is constructed from the loadable's name.

std::string
CdlDefineLoadableBody::get_config_header() const
{
    CYG_REPORT_FUNCNAME("CdlDefineLoadable::get_config_headers");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();

    std::string result = "";
    CdlProperty prop = get_property(CdlPropertyId_DefineHeader);
    if (0 != prop) {
        CdlProperty_String string_prop = dynamic_cast<CdlProperty_String>(prop);
        CYG_ASSERT_CLASSC(string_prop);
        result = string_prop->get_string();
    } else {
        std::string tmp = get_name();
        result = Cdl::get_short_form(tmp);
        result += ".h";
    }
    CYG_REPORT_RETURN();
    return result;
}

//}}}

//}}}
//{{{  CdlToplevel                      

//{{{  CdlToplevel::get_build_info()            

// ----------------------------------------------------------------------------
// Essentially this code involves iterating over the loadables vector,
// looking for BuildLoadables and invoking their update_build_info()
// member function. In addition, if there is currently some data in
// the build_info vector (probably from a previous call) then that
// must be cleared.

void
CdlToplevelBody::get_build_info(CdlBuildInfo& build_info)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_build_info");
    CYG_REPORT_FUNCARG2XV(this, &build_info);
    CYG_PRECONDITION_THISC();

    if (0 != build_info.entries.size()) {
        build_info.entries.clear();
    }

    const std::vector<CdlLoadable>&          loadables   = get_loadables();
    std::vector<CdlLoadable>::const_iterator load_i;
    for (load_i = loadables.begin(); load_i != loadables.end(); load_i++) {
        CdlConstBuildLoadable bl = dynamic_cast<CdlConstBuildLoadable>(*load_i);
        if (0 != bl) {
            bl->update_build_info(build_info);
        }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlToplevel::get_all_build_info()        

// ----------------------------------------------------------------------------
// This is just like get_build_info(), but calls a different
// BuildLoadable member.

void
CdlToplevelBody::get_all_build_info(CdlBuildInfo& build_info)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::get_all_build_info");
    CYG_REPORT_FUNCARG2XV(this, &build_info);
    CYG_PRECONDITION_THISC();

    if (0 != build_info.entries.size()) {
        build_info.entries.clear();
    }

    const std::vector<CdlLoadable>&          loadables   = get_loadables();
    std::vector<CdlLoadable>::const_iterator load_i;
    for (load_i = loadables.begin(); load_i != loadables.end(); load_i++) {
        CdlConstBuildLoadable bl = dynamic_cast<CdlConstBuildLoadable>(*load_i);
        if (0 != bl) {
            bl->update_all_build_info(build_info);
        }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlToplevel::generate_config_headers()   

// ----------------------------------------------------------------------------
// Generating the config headers. This involves the following steps:
//
// 1) for every DefineLoadable, find out what header file should
//    be generated. Note that some loadables may share a header file.
//
// 2) create a temporary version of system.h. Note that it is not
//    a good idea to just overwrite an existing system.h, chances
//    are pretty good that the file will not have changed, and
//    updating it unnecessarily will result in unnecessary rebuilds
//    due to header file dependencies.
//
// 3) for each file that should be generated, create a temporary
//    version and allow all applicable loadables to update it.

// A utility to compare two files and do the right thing.
// This requires some simple Tcl code.
static void
compare_and_copy(CdlInterpreter interp, std::string file1, std::string file2)
{
    CYG_REPORT_FUNCNAME("compare_and_copy");
    CYG_PRECONDITION_CLASSC(interp);
    CYG_PRECONDITIONC("" != file1);
    CYG_PRECONDITIONC("" != file2);
    CYG_PRECONDITIONC(file1 != file2);

    static char compare_and_copy_script[] = "\
if {[file exists \"$::cdl_compare_and_copy_file2\"] == 0} {                                   \n\
    catch { file rename -- $::cdl_compare_and_copy_file1 $::cdl_compare_and_copy_file2}       \n\
    return                                                                                    \n\
}                                                                                             \n\
set fd [open \"$::cdl_compare_and_copy_file1\" r]                                             \n\
set data1 [read $fd]                                                                          \n\
close $fd                                                                                     \n\
set fd [open \"$::cdl_compare_and_copy_file2\" r]                                             \n\
set data2 [read $fd]                                                                          \n\
close $fd                                                                                     \n\
if {$data1 == $data2} {                                                                       \n\
    file delete \"$::cdl_compare_and_copy_file1\"                                             \n\
} else {                                                                                      \n\
    catch { file rename -force -- $::cdl_compare_and_copy_file1 $::cdl_compare_and_copy_file2 } \n\
}                                                                                             \n\
";

    interp->set_variable("::cdl_compare_and_copy_file1", file1);
    interp->set_variable("::cdl_compare_and_copy_file2", file2);
    std::string tcl_result;
    if (TCL_OK != interp->eval(compare_and_copy_script, tcl_result)) {
        throw CdlInputOutputException("internal error manipulating temporary header " + file1 + " and target " + file2 +
            "\n" + tcl_result);
    }
}

void
CdlToplevelBody::generate_config_headers(std::string directory)
{
    CYG_REPORT_FUNCNAME("CdlToplevel::generate_config_headers");
    CYG_REPORT_FUNCARG1XV(this);
    CYG_PRECONDITION_THISC();
    CYG_ASSERTC("" != directory);

    // Replace any backslashes in the path with forward slashes. The
    // latter are used throughout the library
    // NOTE: this is not i18n-friendly.
    for (unsigned int i = 0; i < directory.size(); i++) {
        if ('\\' == directory[i]) {
            directory[i] = '/';
        }
    }
    
    CdlInterpreter interp = get_interpreter();
    std::string    tcl_result;
    if ((TCL_OK != interp->eval("file isdirectory \"" + directory + "\"", tcl_result)) ||
        (tcl_result != "1")) {
        throw CdlInputOutputException("target " + directory + " is not a valid existing directory.");
    }
    
    std::vector<std::pair<CdlDefineLoadable, std::string> > headers;
    const std::vector<CdlLoadable>& loadables = get_loadables();
    std::vector<CdlLoadable>::const_iterator load_i;
    for (load_i = loadables.begin(); load_i != loadables.end(); load_i++) {
        CdlDefineLoadable tmp = dynamic_cast<CdlDefineLoadable>(*load_i);
        if (0 != tmp) {
            std::string hdr = tmp->get_config_header();
            headers.push_back(std::make_pair(tmp, hdr));
        }
    }

    static char banner_format[] =
"#ifndef CYGONCE_PKGCONF_%s\n\
#define CYGONCE_PKGCONF_%s\n\
/*\n\
 * File <pkgconf/%s>\n\
 *\n\
 * This file is generated automatically by the configuration\n\
 * system. It should not be edited. Any changes to this file\n\
 * may be overwritten.\n\
 */\n\
\n";
#ifdef _WIN32
    // Create three channels which Tcl will use for standard streams
    // if these streams do not already exist. This avoids a Tcl
    // problem which can prevent closure of system.h. (FIXME)
    Tcl_Channel stdin_chan = Tcl_OpenFileChannel(interp->get_tcl_interpreter(), "nul", "w", 0666);
    Tcl_Channel stdout_chan = Tcl_OpenFileChannel(interp->get_tcl_interpreter(), "nul", "w", 0666);
    Tcl_Channel stderr_chan = Tcl_OpenFileChannel(interp->get_tcl_interpreter(), "nul", "w", 0666);
    Tcl_RegisterChannel(0, stdin_chan);
    Tcl_RegisterChannel(0, stdout_chan);
    Tcl_RegisterChannel(0, stderr_chan);
#endif
    // Assume for now that files __libcdl_file1 and __libcdl_file2 are
    // legal on all platforms of interest, and that nobody is going to
    // export to these.
    std::string system_h_name = directory + "/__libcdl_file1";
    Tcl_Channel system_h = Tcl_OpenFileChannel(interp->get_tcl_interpreter(),
                                               const_cast<char*>(system_h_name.c_str()), "w", 0666);
    if (0 == system_h) {
        throw CdlInputOutputException("Unable to open file " + system_h_name + "\n" +
                                      interp->get_result());
    }
    // The channel will be registered and unregistered in several
    // different interpreters. This call prevents the channel from
    // disappearing prematurely.
    Tcl_RegisterChannel(0, system_h);

    // Make sure that this operation is undone if necessary.
    try {
        // Now fill in system.h with the appropriate data. Start with the banner.
        char local_buf[512];
        sprintf(local_buf, banner_format, "SYSTEM_H", "SYSTEM_H", "system.h");
        Tcl_Write(system_h, local_buf, -1);

        // Add generic version information
        system_h_add_version_header(system_h);
        
        // The rest of system.h will be filled in by the following loop.
        //
        // Walk down the previously constructed headers vector, create
        // appropriate files, and let each DefineLoadable fill in the
        // file for itself.
        std::vector<std::pair<CdlDefineLoadable, std::string> >::iterator outer_i;
        std::vector<std::pair<CdlDefineLoadable, std::string> >::iterator inner_i;
        for (outer_i = headers.begin(); outer_i != headers.end(); outer_i++) {
            if ("" == outer_i->second) {
                continue;
            }
            std::string target_name = outer_i->second;
            std::string header_name = directory + "/__libcdl_file2";
            Tcl_Channel header_h = Tcl_OpenFileChannel(interp->get_tcl_interpreter(),
                                                       const_cast<char*>(header_name.c_str()), "w", 0666);
            if (0 == header_h) {
                throw CdlInputOutputException("Unable to open file " + header_name + "\n" +
                                              interp->get_result());
            }
            // The channel may be used in several different interpreters, so
            // do an extra register operation
            Tcl_RegisterChannel(0, header_h);

            try {
                // Output the banner. This requires an all-upper-case version of the
                // header name.
                std::string upper_case;
                for (unsigned int i = 0; i < target_name.size(); i++) {
                    if (islower(target_name[i])) {
                        upper_case += toupper(target_name[i]);
                    } else if ('.' == target_name[i]) {
                        upper_case += '_';
                    } else {
                        upper_case += target_name[i];
                    }
                }
                sprintf(local_buf, banner_format, upper_case.c_str(), upper_case.c_str(), target_name.c_str());
                Tcl_Write(header_h, local_buf, -1);

                // Now iterate over all the loadables looking for ones which
                // should generate #define's for this header, and invoke the
                // appropriate member function.
                for (inner_i = outer_i; inner_i != headers.end(); inner_i++) {
                    if (inner_i->second == target_name) {
                        inner_i->first->generate_config_header(header_h, system_h);
                        inner_i->second = "";
                    }
                }
        
                // The header file has now been updated. Close it and decide whether
                // or not to replace the old version
                Tcl_Write(header_h, "\n#endif\n", -1);
            } catch(...) {
                Tcl_UnregisterChannel(0, header_h);
                throw;
            }
            Tcl_UnregisterChannel(0, header_h);
            compare_and_copy(interp, header_name, directory + "/" + target_name);
        }
        
        Tcl_Write(system_h, "\n#endif\n", -1);
    } catch(...) {
        Tcl_UnregisterChannel(0, system_h);
        throw;
    }
    
    // This call to UnregisterChannel automatically closes the
    // channel, there is no need for an explicit Tcl_Close() call.
    Tcl_UnregisterChannel(0, system_h);
#ifdef _WIN32
    Tcl_UnregisterChannel(0, stderr_chan);
    Tcl_UnregisterChannel(0, stdout_chan);
    Tcl_UnregisterChannel(0, stdin_chan);
#endif
    compare_and_copy(interp, system_h_name, directory +"/system.h");
}

//}}}
//{{{  CdlToplevel::get_config_headers()        

// ----------------------------------------------------------------------------
// Return details of the header files that should be generated. This
// allows higher-level code to detect files that should no longer
// be present, amongst other uses.
//
// The main complication is that some packages may wish to share the
// same header file, especially hardware packages.

void
CdlToplevelBody::get_config_headers(std::vector<std::string>& headers)
{
    CYG_REPORT_FUNCNAME("CdlToplevelBody::get_config_headers");
    CYG_REPORT_FUNCARG2XV(this, &headers);
    CYG_PRECONDITION_THISC();

    // Allow the vector argument to be re-used in multiple calls.
    // Strictly speaking this is better done at the application
    // level, but the behaviour is consistent with get_build_info();
    headers.clear();

    // There will always be a system.h header file with details
    // of the loadables.
    // FIXME: the name of this file should probably be controllable
    headers.push_back("system.h");

    // Now check each loadable and adds its header file, assuming
    // this is unique.
    const std::vector<CdlLoadable>& loadables = get_loadables();
    std::vector<CdlLoadable>::const_iterator i;
    for (i = loadables.begin(); i != loadables.end(); i++) {
        CdlDefineLoadable current = dynamic_cast<CdlDefineLoadable>(*i);
        if (0 != current) {
            std::string its_file = current->get_config_header();
            CYG_LOOP_INVARIANTC("" != its_file);
            if (std::find(headers.begin(), headers.end(), its_file) == headers.end()) {
                headers.push_back(its_file);
            }
        }
    }

    CYG_REPORT_RETURN();
}

//}}}
//{{{  CdlToplevel::generate_build_tree()       

void
CdlToplevelBody::generate_build_tree(std::string build_tree, std::string install_tree)
{
}

//}}}

//}}}
