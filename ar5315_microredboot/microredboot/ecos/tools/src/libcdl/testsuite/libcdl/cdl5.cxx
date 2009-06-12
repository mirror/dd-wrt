//==========================================================================
//
//      cdl5.cxx
//
//      Basic test of the database class
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
// Date:                1999-01-22
// Description:         Test the database handling using the test data.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cstdio>
#include <cdlconfig.h>
#include <cdl.hxx>
#include <cyg/infra/testcase.h>
#include <cstdlib>
#include <algorithm>

#if 1
int
main(int argc, char** argv)
{
    // There should be one argument, the location of the component
    // repository. This is actually the data subdirectory in
    // libcdl/testsuite.
    if (2 != argc) {
        CYG_TEST_FAIL_FINISH("Missing command line argument to specify the component repository");
    }
    
    CdlDatabase database = 0;
    try {
        database = CdlDatabaseBody::make(argv[1]);
    }
    catch(std::bad_alloc e) {
        CYG_TEST_FAIL_FINISH("Out of memory when reading in the database");
    }
    catch(CdlInputOutputException e) {
        CYG_TEST_FAIL_FINISH(e.get_message().c_str());
    }

    bool ok = true;
    const std::vector<std::string>& packages = database->get_packages();
    if (2 > packages.size()) {
        CYG_TEST_FAIL("The database should have at least two packages");
        ok = false;
    }
    std::vector<std::string>::const_iterator srch = std::find(packages.begin(), packages.end(), "CYGPKG_PKG1");
    if (srch == packages.end()) {
        CYG_TEST_FAIL("There should be a package CYGPKG_PKG1");
        ok = false;
    }
    srch = std::find(packages.begin(), packages.end(), "CYGPKG_PKG2");
    if (srch == packages.end()) {
        CYG_TEST_FAIL("There should be a package CYGPKG_PKG2");
        ok = false;
    }

    const std::vector<std::string>& aliases     = database->get_package_aliases("CYGPKG_PKG1");
    const std::vector<std::string>& versions    = database->get_package_versions("CYGPKG_PKG1");
    const std::string& directory                = database->get_package_directory("CYGPKG_PKG1");
    if ("pkg1" != directory) {
        CYG_TEST_FAIL("Incorrect directory for CYGPKG_PKG1");
        ok = false;
    }
    if ((3 != aliases.size()) ||
        (aliases.end() == std::find(aliases.begin(), aliases.end(), "package1")) ||
        (aliases.end() == std::find(aliases.begin(), aliases.end(), "pkg1"))     ||
        (aliases.end() == std::find(aliases.begin(), aliases.end(), "another alias"))) {
        CYG_TEST_FAIL("Incorrect aliases for CYGPKG_PKG1");
        ok = false;
    }
    if ((2 != versions.size()) ||
        (versions.end() == std::find(versions.begin(), versions.end(), "current")) ||
        (versions.end() == std::find(versions.begin(), versions.end(), "v1.1"))) {
        CYG_TEST_FAIL("Versions of CYGPKG_PKG1 do not match expectations");
        ok = false;
    }

    if (ok) {
        CYG_TEST_PASS("Database ok");
    }
    return EXIT_SUCCESS;
}

#else

// Some more code to look at a packages database. This produces a simple
// dump.

int
main(int argc, char** argv)
{
    CdlDatabase database = 0;
    try {
        database = CdlDatabaseBody::make();
    }
    catch(std::bad_alloc e) {
        CYG_TEST_FAIL_FINISH("Out of memory reading in the database");
    }
    catch(CdlInputOutputException e) {
        CYG_TEST_FAIL_FINISH(e.get_message().c_str());
    }

    const std::vector<std::string>&             packages = database->get_packages();
    std::vector<std::string>::const_iterator    pkgs_i;
    std::vector<std::string>::const_iterator    data_i;

    printf("There are %d packages\n", packages.size());
    for (pkgs_i = packages.begin(); pkgs_i != packages.end(); pkgs_i++) {
        printf("Package %s\n", pkgs_i->c_str());

        const std::vector<std::string>& aliases         = database->get_package_aliases(*pkgs_i);
        const std::vector<std::string>& versions        = database->get_package_versions(*pkgs_i);
        const std::string&              directory       = database->get_package_directory(*pkgs_i);

        printf("  Directory : %s\n", directory.c_str());
        printf("  Aliases   :");
        for (data_i = aliases.begin(); data_i != aliases.end(); data_i++) {
            printf(" %s", data_i->c_str());
        }
        putchar('\n');
        printf("  Versions  :");
        for (data_i = versions.begin(); data_i != versions.end(); data_i++) {
            printf(" %s", data_i->c_str());
        }
        putchar('\n');
    }
    
    // stdout output is discarded if the test failures.
    CYG_TEST_FAIL("All data displayed.");
    return EXIT_FAILURE;
}
#endif
