//==========================================================================
//
//      cdl3.cxx
//
//      Basic testing of the CdlInterpreter class
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
// Date:                1999-01-21
// Description:         This test case deals with simple uses of the
//                      CdlInterpreter class, independent from any
//                      configuration data.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cstdio>
#include <cdlconfig.h>
#include <cdl.hxx>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/testcase.h>
#include <cstdlib>

static CdlInterpreter interp = 0;

static int
extra_command1(ClientData data, Tcl_Interp* tcl_interp, int argc, char** argv)
{
    if (static_cast<CdlInterpreter>(data) != interp) {
        char* msg = "ClientData does not correspond to interpreter";
        Tcl_SetResult(tcl_interp, msg, TCL_STATIC);
        CYG_TEST_FAIL(msg);
        return TCL_ERROR;
    }
    if ((3 != argc) ||
        (0 != strcmp("extra_command1", argv[0])) ||
        (0 != strcmp("first_arg",      argv[1])) ||
        (0 != strcmp("second_arg",     argv[2]))) {
        char* msg = "Wrong arguments passed to extra_command1";
        interp->set_result(msg);
        CYG_TEST_FAIL(msg);
        return TCL_ERROR;
    }
    interp->set_result("To be or not to be");
    return TCL_OK;
}

static int
extra_command2(ClientData data, Tcl_Interp* tcl_interp, int argc, char** argv)
{
    if (0 != data) {
        char*msg = "ClientData should be zero here";
        Tcl_SetResult(tcl_interp, msg, TCL_STATIC);
        CYG_TEST_FAIL(msg);
        return TCL_ERROR;
    }
    if ((2 != argc) ||
        (0 != strcmp("extra_command2", argv[0])) ||
        (0 != strcmp("third_arg",      argv[1]))) {
        char* msg = "Wrong arguments passed to extra_command2";
        Tcl_SetResult(tcl_interp, msg, TCL_STATIC);
        CYG_TEST_FAIL(msg);
        return TCL_ERROR;
    }
    Tcl_SetResult(tcl_interp, "That is the question", TCL_STATIC);
    return TCL_OK;
}

int
main(int argc, char** argv)
{
    // Start by creating a simple interpreter using default settings
    bool ok = true;
    interp = CdlInterpreterBody::make();
    if (0 == interp) {
        CYG_TEST_FAIL_FINISH("Unable to create a new interpreter with default settings");
    }
    if ((!interp->check_this(cyg_quick)) ||
        (!interp->check_this(cyg_extreme))) {
        CYG_TEST_FAIL("check_this() failed");
        ok = false;
    }
    if ((0 != interp->get_configuration()) ||
        (0 != interp->get_package())) {
        CYG_TEST_FAIL("a new interpreter should not be associated with any package or configuration");
        ok = false;
    }

    // Now try evaluating a very simple script
    std::string str_result;
    if ((TCL_OK != interp->eval("expr 2 * 2 * 2\n", str_result)) ||
        ("8" != str_result)) {
        CYG_TEST_FAIL("simple command execution failed");
        ok = false;
    }
    // And something a bit more interesting
    std::string fibonaci = "                                                            \n\
proc fibonaci { arg } {                                                                 \n\
    if { $arg < 3 } {                                                                   \n\
        return 1                                                                        \n\
    } else {                                                                            \n\
        set result [expr [fibonaci [expr $arg - 1]] + [fibonaci [expr $arg - 2]]]       \n\
        return $result                                                                  \n\
    }                                                                                   \n\
}                                                                                       \n\
return [fibonaci 10]                                                                    \n\
";
    if ((TCL_OK != interp->eval(fibonaci, str_result)) ||
        ("55" != str_result)) {
        CYG_TEST_FAIL("full script execution failed");
        ok = false;
    }

    // A new interpreter should not know about "extra_command1"
    if ((TCL_OK != interp->eval("info command extra_command1", str_result)) ||
        ("" != str_result)) {
        CYG_TEST_FAIL("new interpreter should not have an extra_command1 command");
        ok = false;
    }
    try {
        interp->add_command("extra_command1", &extra_command1, 0);
        if ((TCL_OK != interp->eval("info command extra_command1", str_result)) ||
            ("extra_command1" != str_result)) {
            CYG_TEST_FAIL("interpreter does not know the new extra_command1 command");
            ok = false;
        }
        if ((TCL_OK != interp->eval("extra_command1 first_arg second_arg", str_result)) ||
            ("To be or not to be" != str_result)) {
            CYG_TEST_FAIL("execution of a new command failed");
            ok = false;
        }
        interp->remove_command("extra_command1");
    }
    catch(std::bad_alloc e) {
        CYG_TEST_FAIL("unable to add a new command to the interpreter");
        ok = false;
    }

    // Check the variables support
    try {
        interp->set_variable("some_variable", "random_value");
    }
    catch(std::bad_alloc e) {
        CYG_TEST_FAIL("unable to set a global variable inside the interpreter");
        ok = false;
    }
    if ("random_value" != interp->get_variable("some_variable")) {
        CYG_TEST_FAIL("unable to retrieve global variable setting");
        ok = false;
    }
    // Make absolutely sure the variable exists
    if ((TCL_OK != interp->eval("info exists ::some_variable", str_result)) ||
        ("1" != str_result)) {
        CYG_TEST_FAIL("Tcl code does not know about new global variable");
    }
    interp->unset_variable("some_variable");
    if ("" != interp->get_variable("some_variable")) {
        CYG_TEST_FAIL("variable still exists after it has been removed");
        ok = false;
    }
    
    // Check the AssocData support
    if (0 != interp->get_assoc_data("some_key")) {
        CYG_TEST_FAIL("unexpected assoc data already present");
        ok = false;
    }
    interp->set_assoc_data("some_key", static_cast<ClientData>(&ok), 0);
    if (&ok != static_cast<bool*>(interp->get_assoc_data("some_key"))) {
        CYG_TEST_FAIL("unable to retrieve assoc data");
        ok = false;
    }
    interp->delete_assoc_data("some_key");
    if (0 != interp->get_assoc_data("some_key")) {
        CYG_TEST_FAIL("assoc data still present after retrieval");
        ok = false;
    }

    // Make the interpreter safe. This should remove the file command.
    if ((TCL_OK != interp->eval("info command file", str_result)) ||
        ("file" != str_result)) {
        CYG_TEST_FAIL("interpreter should still have the file command");
        ok = false;
    }
    interp->make_safe();
    if ((TCL_OK != interp->eval("info command file", str_result)) ||
        ("" != str_result)) {
        CYG_TEST_FAIL("interpreter should no longer have the file command");
        ok = false;
    }

    // This is just a compilation check to make sure that interpreters can
    // be deleted.
#ifdef CYGBLD_LIBCDL_USE_SMART_POINTERS
    interp.destroy();
#else
    delete interp;
    interp = 0;
#endif    
    
    if (ok) {
        CYG_TEST_PASS("interpreter with default settings is functional");
    }
    ok = true;

    // Now try to create a new Tcl interpreter, register the extra command
    // at the Tcl level, and turn it into a CdlInterpreter
    Tcl_Interp* tcl_interp = Tcl_CreateInterp();
    if (0 == tcl_interp) {
        CYG_TEST_FAIL_FINISH("unable to create new Tcl interpreter");
    }
    if (0 == Tcl_CreateCommand(tcl_interp, "extra_command2", &extra_command2, 0, 0)) {
        CYG_TEST_FAIL_FINISH("unable to add new command to Tcl interpreter");
    }
    
    // And turn the Tcl interpreter into a Cdl one.
    interp = CdlInterpreterBody::make(tcl_interp);
    if (0 == interp) {
        CYG_TEST_FAIL_FINISH("unable to create new CDL interpreter using existing Tcl one");
        ok = false;
    }
    if (!interp->check_this(cyg_quick)) {
        CYG_TEST_FAIL_FINISH("new CDL interpreter fails checks");
        ok = false;
    }

    // The new interpreter should not have an extra_command1 command.
    if ((TCL_OK != interp->eval("info command extra_command1", str_result)) ||
        ("" != str_result)) {
        CYG_TEST_FAIL("new interpreter should not have an extra_command1 command");
        ok = false;
    }
    // But it should still have an extra_command2 command
    if ((TCL_OK != interp->eval("info command extra_command2", str_result)) ||
        ("extra_command2" != str_result)) {
        CYG_TEST_FAIL("new interpreter should have an extra_command2 command");
        ok = false;
    }
    // And that command should work
    if ((TCL_OK != interp->eval("extra_command2 third_arg", str_result)) ||
        ("That is the question" != str_result)) {
        CYG_TEST_FAIL("extra_command2 command not functional");
        ok = false;
    }
    // Get rid of the CdlInterpreter object. The Tcl interpreter should still
    // be valid.
#ifdef CYGBLD_LIBCDL_USE_SMART_POINTERS
    interp.destroy();
#else
    delete interp;
    interp = 0;
#endif
    if (Tcl_InterpDeleted(tcl_interp)) {
        CYG_TEST_FAIL("Tcl interpreter deleted when it should still be around");
        ok = false;
    }
    
    if (ok) {
        CYG_TEST_PASS("custom interpreter is functional");
    }
    
    return EXIT_SUCCESS;
}
