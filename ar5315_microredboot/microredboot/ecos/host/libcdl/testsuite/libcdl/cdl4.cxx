//==========================================================================
//
//      cdl4.cxx
//
//      Tests for the Cdl::compare_versions() function.
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
// Description:         A variety of tests for Cdl::compare_versions()
//
//####DESCRIPTIONEND####
//==========================================================================

#include <cstdio>
#include <cdlconfig.h>
#include <cdl.hxx>
#include <cyg/infra/testcase.h>
#include <cstdlib>

static struct version_data {
    char*       arg1;
    char*       arg2;
    int         expected;
} versions[] = {
    { "current",        "v0.1",         -1 },
    { "v0.1",           "current",       1 },
    { "current",        "current",       0 },
    { "v0.1",           "v0.1",          0 },
    { "v0.1",           "V0.1",          0 },
    { "v0_1",           "V0.1",          0 },
    { "V0-1",           "v0_1",          0 },
    { "v0.2",           "v0.1",         -1 },
    { "v0.1",           "v0.2",          1 },
    { "v0.10",          "v0.2",         -1 },
    { "0.2",            "0.10",          1 },
    { "19990122",       "19981224",     -1 },
    { "19990122",       "19990124",      1 },
    { "ss20000101",     "ss19991231",   -1 },
    { "ss19700101",     "ss20380704",    1 },
    { "v0.1.2",         "v0.1.3",        1 },
    { "v0.1.3",         "v0.1.2",       -1 },
    { "v1.0",           "v0.1.3",       -1 },
    { "v0.2.5",         "v2.1",          1 },
    { "v1.0beta2",      "v1.0",          1 },
    { "v0.6",           "v1.0b",         1 },
    { "v1.0",           "v1.0a",        -1 },
    { "v1.0.p1",        "v1.0",         -1 },
    { "v1.0",           "v1.0.p2",       1 },
    { "v1.1",           "v1.0,p3",      -1 },
    { "finished",       "done",          0 }
};

int
main(int argc, char** argv)
{
    bool        ok = true;

    for (int i = 0; 0 != strcmp("finished", versions[i].arg1); i++) {
        if (Cdl::compare_versions(versions[i].arg1, versions[i].arg2) != versions[i].expected) {
            
            std::string result = "comparison of " + std::string(versions[i].arg1) + " and " +
                std::string(versions[i].arg2) + " gave wrong result";
            CYG_TEST_FAIL(result.c_str());
            ok = false;
        }
    }
    if (ok) {
        CYG_TEST_PASS("all version comparisons successful");
    }
    return EXIT_SUCCESS;
}
