//==========================================================================
//
//      ttrace2.cxx
//
//      Trace test case                                                                
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
// Date:                1999-01-06
// Purpose:
// Description:         This file tests all the trace macros for the case
//                      where tracing and function reporting are enabled.
//
//####DESCRIPTIONEND####
//==========================================================================


#define CYGDBG_USE_TRACING
#define CYGDBG_INFRA_DEBUG_FUNCTION_REPORTS
#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_trac.h>
#include <cstdlib>

bool tracing_is_enabled(void);
bool reporting_is_enabled(void);

#define CYG_TRACE_USER_BOOL  (tracing_is_enabled())
#define CYG_REPORT_USER_BOOL (reporting_is_enabled())


void
fn1(void)
{
    CYG_REPORT_FUNCTION();
}

void
fn2(void)
{
    CYG_REPORT_FUNCTYPE("printf-style format string");
}

void
fn3(void)
{
    CYG_REPORT_FUNCNAME("fn3");
}

void
fn4(void)
{
    CYG_REPORT_FUNCNAMETYPE("fn4", "printf-style format string");
}

void
fn5(void)
{
    CYG_REPORT_FUNCTIONC();
}

void
fn6(void)
{
    CYG_REPORT_FUNCTYPEC("printf-style format string");
}

void
fn7(void)
{
    CYG_REPORT_FUNCNAMEC("fn7");
}

void
fn8(void)
{
    CYG_REPORT_FUNCNAMETYPEC("fn8", "printf-style format string");
}

void
fn9(void)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_RETURN();
}

void
fn10(void)
{
    CYG_REPORT_FUNCTYPE("result is %d");
    CYG_REPORT_RETVAL(42);
}

void
fn11(void)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARGVOID();
}

void
fn12(int glorious)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1("%d", glorious);
}

void
fn13(int glorious, int summer)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2("%d %d", glorious, summer);
}

void
fn14(int glorious, int summer, int by)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3("%d %d %d", glorious, summer, by);
}

void
fn15(int glorious, int summer, int by, int thisse)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4("%d %d %d %d", glorious, summer, by, thisse);
}

void
fn16(int glorious, int summer, int by, int thisse, int son)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5("%d %d %d %d %d", glorious, summer, by, thisse, son);
}

void
fn17(int glorious, int summer, int by, int thisse, int son, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6("%d %d %d %d %d %d", glorious, summer, by, thisse, son, of);
}

void
fn18(int glorious, int summer, int by, int thisse, int son, int of, int york)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7("%d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york);
}

void
fn19(int glorious, int summer, int by, int thisse, int son, int of, int york, int stop)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8("%d %d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york, stop);
}

void
fn20(int glorious)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1X(glorious);
}

void
fn21(int glorious, int summer)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2X(glorious, summer);
}

void
fn22(int glorious, int summer, int by)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3X(glorious, summer, by);
}

void
fn23(int glorious, int summer, int by, int thisse)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4X(glorious, summer, by, thisse);
}

void
fn24(int glorious, int summer, int by, int thisse, int son)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5X(glorious, summer, by, thisse, son);
}

void
fn25(int glorious, int summer, int by, int thisse, int son, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6X(glorious, summer, by, thisse, son, of);
}

void
fn26(int glorious, int summer, int by, int thisse, int son, int of, int york)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7X(glorious, summer, by, thisse, son, of, york);
}

void
fn27(int glorious, int summer, int by, int thisse, int son, int of, int york, int stop)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8X(glorious, summer, by, thisse, son, of, york, stop);
}

void
fn28(int glorious)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1Y(glorious);
}

void
fn29(int glorious, int summer)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2Y(glorious, summer);
}

void
fn30(int glorious, int summer, int by)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3Y(glorious, summer, by);
}

void
fn31(int glorious, int summer, int by, int thisse)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4Y(glorious, summer, by, thisse);
}

void
fn32(int glorious, int summer, int by, int thisse, int son)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5Y(glorious, summer, by, thisse, son);
}

void
fn33(int glorious, int summer, int by, int thisse, int son, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6Y(glorious, summer, by, thisse, son, of);
}

void
fn34(int glorious, int summer, int by, int thisse, int son, int of, int york)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7Y(glorious, summer, by, thisse, son, of, york);
}

void
fn35(int glorious, int summer, int by, int thisse, int son, int of, int york, int stop)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8D(glorious, summer, by, thisse, son, of, york, stop);
}

void
fn36(int glorious)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1D(glorious);
}

void
fn37(int glorious, int summer)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2D(glorious, summer);
}

void
fn38(int glorious, int summer, int by)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3D(glorious, summer, by);
}

void
fn39(int glorious, int summer, int by, int thisse)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4D(glorious, summer, by, thisse);
}

void
fn40(int glorious, int summer, int by, int thisse, int son)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5D(glorious, summer, by, thisse, son);
}

void
fn41(int glorious, int summer, int by, int thisse, int son, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6D(glorious, summer, by, thisse, son, of);
}

void
fn42(int glorious, int summer, int by, int thisse, int son, int of, int york)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7D(glorious, summer, by, thisse, son, of, york);
}

void
fn43(int glorious, int summer, int by, int thisse, int son, int of, int york, int stop)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8D(glorious, summer, by, thisse, son, of, york, stop);
}

void
fn44(int glorious)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1XV(glorious);
}

void
fn45(int glorious, int summer)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2XV(glorious, summer);
}

void
fn46(int glorious, int summer, int by)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3XV(glorious, summer, by);
}

void
fn47(int glorious, int summer, int by, int thisse)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4XV(glorious, summer, by, thisse);
}

void
fn48(int glorious, int summer, int by, int thisse, int son)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5XV(glorious, summer, by, thisse, son);
}

void
fn49(int glorious, int summer, int by, int thisse, int son, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6XV(glorious, summer, by, thisse, son, of);
}

void
fn50(int glorious, int summer, int by, int thisse, int son, int of, int york)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7XV(glorious, summer, by, thisse, son, of, york);
}

void
fn51(int glorious, int summer, int by, int thisse, int son, int of, int york, int stop)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8XV(glorious, summer, by, thisse, son, of, york, stop);
}

void
fn52(int glorious)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1YV(glorious);
}

void
fn53(int glorious, int summer)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2YV(glorious, summer);
}

void
fn54(int glorious, int summer, int by)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3YV(glorious, summer, by);
}

void
fn55(int glorious, int summer, int by, int thisse)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4YV(glorious, summer, by, thisse);
}

void
fn56(int glorious, int summer, int by, int thisse, int son)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5YV(glorious, summer, by, thisse, son);
}

void
fn57(int glorious, int summer, int by, int thisse, int son, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6YV(glorious, summer, by, thisse, son, of);
}

void
fn58(int glorious, int summer, int by, int thisse, int son, int of, int york)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7YV(glorious, summer, by, thisse, son, of, york);
}

void
fn59(int glorious, int summer, int by, int thisse, int son, int of, int york, int stop)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8YV(glorious, summer, by, thisse, son, of, york, stop);
}

void
fn60(int glorious)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1DV(glorious);
}

void
fn61(int glorious, int summer)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2DV(glorious, summer);
}

void
fn62(int glorious, int summer, int by)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3DV(glorious, summer, by);
}

void
fn63(int glorious, int summer, int by, int thisse)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4DV(glorious, summer, by, thisse);
}

void
fn64(int glorious, int summer, int by, int thisse, int son)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5DV(glorious, summer, by, thisse, son);
}

void
fn65(int glorious, int summer, int by, int thisse, int son, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6DV(glorious, summer, by, thisse, son, of);
}

void
fn66(int glorious, int summer, int by, int thisse, int son, int of, int york)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7DV(glorious, summer, by, thisse, son, of, york);
}

void
fn67(int glorious, int summer, int by, int thisse, int son, int of, int york, int stop)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8DV(glorious, summer, by, thisse, son, of, york, stop);
}

int
main(int argc, char** argv)
{
    int glorious    = 0;
    int summer      = 1;
    int by          = 2;
    int thisse      = 4;
    int son         = 5;
    int of          = 6;
    int york        = 7;
    int stop        = 8;
    
    CYG_TRACE0(true, "no argument here");
    CYG_TRACE1(true, "%d", glorious);
    CYG_TRACE2(true, "%d %d", glorious, summer);
    CYG_TRACE3(true, "%d %d %d", glorious, summer, by);
    CYG_TRACE4(true, "%d %d %d %d", glorious, summer, by, thisse);
    CYG_TRACE5(true, "%d %d %d %d %d", glorious, summer, by, thisse, son);
    CYG_TRACE6(true, "%d %d %d %d %d %d", glorious, summer, by, thisse, son, of);
    CYG_TRACE7(true, "%d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8(true, "%d %d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE0(false, "no argument here");
    CYG_TRACE1(false, "%d", glorious);
    CYG_TRACE2(false, "%d %d", glorious, summer);
    CYG_TRACE3(false, "%d %d %d", glorious, summer, by);
    CYG_TRACE4(false, "%d %d %d %d", glorious, summer, by, thisse);
    CYG_TRACE5(false, "%d %d %d %d %d", glorious, summer, by, thisse, son);
    CYG_TRACE6(false, "%d %d %d %d %d %d", glorious, summer, by, thisse, son, of);
    CYG_TRACE7(false, "%d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8(false, "%d %d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE0B("no argument here");
    CYG_TRACE1B("%d", glorious);
    CYG_TRACE2B("%d %d", glorious, summer);
    CYG_TRACE3B("%d %d %d", glorious, summer, by);
    CYG_TRACE4B("%d %d %d %d", glorious, summer, by, thisse);
    CYG_TRACE5B("%d %d %d %d %d", glorious, summer, by, thisse, son);
    CYG_TRACE6B("%d %d %d %d %d %d", glorious, summer, by, thisse, son, of);
    CYG_TRACE7B("%d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8B("%d %d %d %d %d %d %d %d", glorious, summer, by, thisse, son, of, york, stop);

    CYG_TRACE1X(true, glorious);
    CYG_TRACE2X(true, glorious, summer);
    CYG_TRACE3X(true, glorious, summer, by);
    CYG_TRACE4X(true, glorious, summer, by, thisse);
    CYG_TRACE5X(true, glorious, summer, by, thisse, son);
    CYG_TRACE6X(true, glorious, summer, by, thisse, son, of);
    CYG_TRACE7X(true, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8X(true, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1Y(true, glorious);
    CYG_TRACE2Y(true, glorious, summer);
    CYG_TRACE3Y(true, glorious, summer, by);
    CYG_TRACE4Y(true, glorious, summer, by, thisse);
    CYG_TRACE5Y(true, glorious, summer, by, thisse, son);
    CYG_TRACE6Y(true, glorious, summer, by, thisse, son, of);
    CYG_TRACE7Y(true, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8Y(true, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1D(true, glorious);
    CYG_TRACE2D(true, glorious, summer);
    CYG_TRACE3D(true, glorious, summer, by);
    CYG_TRACE4D(true, glorious, summer, by, thisse);
    CYG_TRACE5D(true, glorious, summer, by, thisse, son);
    CYG_TRACE6D(true, glorious, summer, by, thisse, son, of);
    CYG_TRACE7D(true, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8D(true, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1XV(true, glorious);
    CYG_TRACE2XV(true, glorious, summer);
    CYG_TRACE3XV(true, glorious, summer, by);
    CYG_TRACE4XV(true, glorious, summer, by, thisse);
    CYG_TRACE5XV(true, glorious, summer, by, thisse, son);
    CYG_TRACE6XV(true, glorious, summer, by, thisse, son, of);
    CYG_TRACE7XV(true, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8XV(true, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1YV(true, glorious);
    CYG_TRACE2YV(true, glorious, summer);
    CYG_TRACE3YV(true, glorious, summer, by);
    CYG_TRACE4YV(true, glorious, summer, by, thisse);
    CYG_TRACE5YV(true, glorious, summer, by, thisse, son);
    CYG_TRACE6YV(true, glorious, summer, by, thisse, son, of);
    CYG_TRACE7YV(true, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8YV(true, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1DV(true, glorious);
    CYG_TRACE2DV(true, glorious, summer);
    CYG_TRACE3DV(true, glorious, summer, by);
    CYG_TRACE4DV(true, glorious, summer, by, thisse);
    CYG_TRACE5DV(true, glorious, summer, by, thisse, son);
    CYG_TRACE6DV(true, glorious, summer, by, thisse, son, of);
    CYG_TRACE7DV(true, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8DV(true, glorious, summer, by, thisse, son, of, york, stop);

    CYG_TRACE1X(false, glorious);
    CYG_TRACE2X(false, glorious, summer);
    CYG_TRACE3X(false, glorious, summer, by);
    CYG_TRACE4X(false, glorious, summer, by, thisse);
    CYG_TRACE5X(false, glorious, summer, by, thisse, son);
    CYG_TRACE6X(false, glorious, summer, by, thisse, son, of);
    CYG_TRACE7X(false, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8X(false, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1Y(false, glorious);
    CYG_TRACE2Y(false, glorious, summer);
    CYG_TRACE3Y(false, glorious, summer, by);
    CYG_TRACE4Y(false, glorious, summer, by, thisse);
    CYG_TRACE5Y(false, glorious, summer, by, thisse, son);
    CYG_TRACE6Y(false, glorious, summer, by, thisse, son, of);
    CYG_TRACE7Y(false, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8Y(false, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1D(false, glorious);
    CYG_TRACE2D(false, glorious, summer);
    CYG_TRACE3D(false, glorious, summer, by);
    CYG_TRACE4D(false, glorious, summer, by, thisse);
    CYG_TRACE5D(false, glorious, summer, by, thisse, son);
    CYG_TRACE6D(false, glorious, summer, by, thisse, son, of);
    CYG_TRACE7D(false, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8D(false, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1XV(false, glorious);
    CYG_TRACE2XV(false, glorious, summer);
    CYG_TRACE3XV(false, glorious, summer, by);
    CYG_TRACE4XV(false, glorious, summer, by, thisse);
    CYG_TRACE5XV(false, glorious, summer, by, thisse, son);
    CYG_TRACE6XV(false, glorious, summer, by, thisse, son, of);
    CYG_TRACE7XV(false, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8XV(false, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1YV(false, glorious);
    CYG_TRACE2YV(false, glorious, summer);
    CYG_TRACE3YV(false, glorious, summer, by);
    CYG_TRACE4YV(false, glorious, summer, by, thisse);
    CYG_TRACE5YV(false, glorious, summer, by, thisse, son);
    CYG_TRACE6YV(false, glorious, summer, by, thisse, son, of);
    CYG_TRACE7YV(false, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8YV(false, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1DV(false, glorious);
    CYG_TRACE2DV(false, glorious, summer);
    CYG_TRACE3DV(false, glorious, summer, by);
    CYG_TRACE4DV(false, glorious, summer, by, thisse);
    CYG_TRACE5DV(false, glorious, summer, by, thisse, son);
    CYG_TRACE6DV(false, glorious, summer, by, thisse, son, of);
    CYG_TRACE7DV(false, glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8DV(false, glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1XB(glorious);
    CYG_TRACE2XB(glorious, summer);
    CYG_TRACE3XB(glorious, summer, by);
    CYG_TRACE4XB(glorious, summer, by, thisse);
    CYG_TRACE5XB(glorious, summer, by, thisse, son);
    CYG_TRACE6XB(glorious, summer, by, thisse, son, of);
    CYG_TRACE7XB(glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8XB(glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1YB(glorious);
    CYG_TRACE2YB(glorious, summer);
    CYG_TRACE3YB(glorious, summer, by);
    CYG_TRACE4YB(glorious, summer, by, thisse);
    CYG_TRACE5YB(glorious, summer, by, thisse, son);
    CYG_TRACE6YB(glorious, summer, by, thisse, son, of);
    CYG_TRACE7YB(glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8YB(glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1DB(glorious);
    CYG_TRACE2DB(glorious, summer);
    CYG_TRACE3DB(glorious, summer, by);
    CYG_TRACE4DB(glorious, summer, by, thisse);
    CYG_TRACE5DB(glorious, summer, by, thisse, son);
    CYG_TRACE6DB(glorious, summer, by, thisse, son, of);
    CYG_TRACE7DB(glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8DB(glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1XVB(glorious);
    CYG_TRACE2XVB(glorious, summer);
    CYG_TRACE3XVB(glorious, summer, by);
    CYG_TRACE4XVB(glorious, summer, by, thisse);
    CYG_TRACE5XVB(glorious, summer, by, thisse, son);
    CYG_TRACE6XVB(glorious, summer, by, thisse, son, of);
    CYG_TRACE7XVB(glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8XVB(glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1YVB(glorious);
    CYG_TRACE2YVB(glorious, summer);
    CYG_TRACE3YVB(glorious, summer, by);
    CYG_TRACE4YVB(glorious, summer, by, thisse);
    CYG_TRACE5YVB(glorious, summer, by, thisse, son);
    CYG_TRACE6YVB(glorious, summer, by, thisse, son, of);
    CYG_TRACE7YVB(glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8YVB(glorious, summer, by, thisse, son, of, york, stop);
    
    CYG_TRACE1DVB(glorious);
    CYG_TRACE2DVB(glorious, summer);
    CYG_TRACE3DVB(glorious, summer, by);
    CYG_TRACE4DVB(glorious, summer, by, thisse);
    CYG_TRACE5DVB(glorious, summer, by, thisse, son);
    CYG_TRACE6DVB(glorious, summer, by, thisse, son, of);
    CYG_TRACE7DVB(glorious, summer, by, thisse, son, of, york);
    CYG_TRACE8DVB(glorious, summer, by, thisse, son, of, york, stop);

    fn1();
    fn2();
    fn3();
    fn4();
    fn5();
    fn6();
    fn7();
    fn8();
    fn9();
    fn10();
    fn11();
    fn12(1);
    fn13(2,   3);
    fn14(4,   5,  6);
    fn15(7,   8,  9, 10);
    fn16(11, 12, 13, 14, 15);
    fn17(16, 17, 18, 19, 20, 21);
    fn18(22, 23, 24, 25, 26, 27, 28);
    fn19(29, 30, 31, 32, 33, 34, 35, 36);
    fn20(1);
    fn21(2,   3);
    fn22(4,   5,  6);
    fn23(7,   8,  9, 10);
    fn24(11, 12, 13, 14, 15);
    fn25(16, 17, 18, 19, 20, 21);
    fn26(22, 23, 24, 25, 26, 27, 28);
    fn27(29, 30, 31, 32, 33, 34, 35, 36);
    fn28(1);
    fn29(2,   3);
    fn30(4,   5,  6);
    fn31(7,   8,  9, 10);
    fn32(11, 12, 13, 14, 15);
    fn33(16, 17, 18, 19, 20, 21);
    fn34(22, 23, 24, 25, 26, 27, 28);
    fn35(29, 30, 31, 32, 33, 34, 35, 36);
    fn36(1);
    fn37(2,   3);
    fn38(4,   5,  6);
    fn39(7,   8,  9, 10);
    fn40(11, 12, 13, 14, 15);
    fn41(16, 17, 18, 19, 20, 21);
    fn42(22, 23, 24, 25, 26, 27, 28);
    fn43(29, 30, 31, 32, 33, 34, 35, 36);
    fn44(1);
    fn45(2,   3);
    fn46(4,   5,  6);
    fn47(7,   8,  9, 10);
    fn48(11, 12, 13, 14, 15);
    fn49(16, 17, 18, 19, 20, 21);
    fn50(22, 23, 24, 25, 26, 27, 28);
    fn51(29, 30, 31, 32, 33, 34, 35, 36);
    fn52(1);
    fn53(2,   3);
    fn54(4,   5,  6);
    fn55(7,   8,  9, 10);
    fn56(11, 12, 13, 14, 15);
    fn57(16, 17, 18, 19, 20, 21);
    fn58(22, 23, 24, 25, 26, 27, 28);
    fn59(29, 30, 31, 32, 33, 34, 35, 36);
    fn60(1);
    fn61(2,   3);
    fn62(4,   5,  6);
    fn63(7,   8,  9, 10);
    fn64(11, 12, 13, 14, 15);
    fn65(16, 17, 18, 19, 20, 21);
    fn66(22, 23, 24, 25, 26, 27, 28);
    fn67(29, 30, 31, 32, 33, 34, 35, 36);
    
    CYG_TEST_PASS_FINISH("enabled tracing only slows things down");
    return 0;
}
// ----------------------------------------------------------------------------
// These functions allow "dynamic" control over tracing and reporting.
// The assumption is that the compiler does not know enough about rand()
// to be able to optimise this away.
bool
tracing_is_enabled(void)
{
    return rand() >= 0;
}

bool
reporting_is_enabled(void)
{
    return rand() >= 0;
}
