//==========================================================================
//
//      ttrace1.cxx
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
// Description:         By default all tracing should be disabled, but
//                      they should compile just fine. This module uses all
//                      of the trace macros. As a fringe benefit, all
//                      of the macros get checked for correct argument usage.
//
//####DESCRIPTIONEND####
//==========================================================================


#include <cyg/infra/testcase.h>
#include <cyg/infra/cyg_trac.h>
#include <cstdlib>

#ifdef CYGDBG_USE_TRACING
# error Tracing should not be enabled by default.
#endif
#ifdef CYGDBG_INFRA_DEBUG_FUNCTION_REPORTS
# error Function reporting should not be enabled by default.
#endif

#define CYG_TRACE_USER_BOOL  (invalid_expression)
#define CYG_REPORT_USER_BOOL (invalid_expression)

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
fn12(int now)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1(fmt, now);
}

void
fn13(int now, int is)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2(fmt, now, is);
}

void
fn14(int now, int is, int the)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3(fmt, now, is, the);
}

void
fn15(int now, int is, int the, int winter)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4(fmt, now, is, the, winter);
}

void
fn16(int now, int is, int the, int winter, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5(fmt, now, is, the, winter, of);
}

void
fn17(int now, int is, int the, int winter, int of, int our)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6(fmt, now, is, the, winter, of, our);
}

void
fn18(int now, int is, int the, int winter, int of, int our, int discontent)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7(fmt, now, is, the, winter, of, our, discontent);
}

void
fn19(int now, int is, int the, int winter, int of, int our, int discontent, int made)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8(fmt, now, is, the, winter, of, our, discontent, made);
}

void
fn20(int now)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1X(now);
}

void
fn21(int now, int is)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2X(now, is);
}

void
fn22(int now, int is, int the)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3X(now, is, the);
}

void
fn23(int now, int is, int the, int winter)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4X(now, is, the, winter);
}

void
fn24(int now, int is, int the, int winter, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5X(now, is, the, winter, of);
}

void
fn25(int now, int is, int the, int winter, int of, int our)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6X(now, is, the, winter, of, our);
}

void
fn26(int now, int is, int the, int winter, int of, int our, int discontent)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7X(now, is, the, winter, of, our, discontent);
}

void
fn27(int now, int is, int the, int winter, int of, int our, int discontent, int made)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8X(now, is, the, winter, of, our, discontent, made);
}

void
fn28(int now)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1Y(now);
}

void
fn29(int now, int is)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2Y(now, is);
}

void
fn30(int now, int is, int the)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3Y(now, is, the);
}

void
fn31(int now, int is, int the, int winter)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4Y(now, is, the, winter);
}

void
fn32(int now, int is, int the, int winter, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5Y(now, is, the, winter, of);
}

void
fn33(int now, int is, int the, int winter, int of, int our)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6Y(now, is, the, winter, of, our);
}

void
fn34(int now, int is, int the, int winter, int of, int our, int discontent)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7Y(now, is, the, winter, of, our, discontent);
}

void
fn35(int now, int is, int the, int winter, int of, int our, int discontent, int made)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8D(now, is, the, winter, of, our, discontent, made);
}

void
fn36(int now)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1D(now);
}

void
fn37(int now, int is)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2D(now, is);
}

void
fn38(int now, int is, int the)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3D(now, is, the);
}

void
fn39(int now, int is, int the, int winter)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4D(now, is, the, winter);
}

void
fn40(int now, int is, int the, int winter, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5D(now, is, the, winter, of);
}

void
fn41(int now, int is, int the, int winter, int of, int our)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6D(now, is, the, winter, of, our);
}

void
fn42(int now, int is, int the, int winter, int of, int our, int discontent)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7D(now, is, the, winter, of, our, discontent);
}

void
fn43(int now, int is, int the, int winter, int of, int our, int discontent, int made)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8D(now, is, the, winter, of, our, discontent, made);
}

void
fn44(int now)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1XV(now);
}

void
fn45(int now, int is)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2XV(now, is);
}

void
fn46(int now, int is, int the)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3XV(now, is, the);
}

void
fn47(int now, int is, int the, int winter)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4XV(now, is, the, winter);
}

void
fn48(int now, int is, int the, int winter, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5XV(now, is, the, winter, of);
}

void
fn49(int now, int is, int the, int winter, int of, int our)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6XV(now, is, the, winter, of, our);
}

void
fn50(int now, int is, int the, int winter, int of, int our, int discontent)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7XV(now, is, the, winter, of, our, discontent);
}

void
fn51(int now, int is, int the, int winter, int of, int our, int discontent, int made)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8XV(now, is, the, winter, of, our, discontent, made);
}

void
fn52(int now)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1YV(now);
}

void
fn53(int now, int is)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2YV(now, is);
}

void
fn54(int now, int is, int the)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3YV(now, is, the);
}

void
fn55(int now, int is, int the, int winter)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4YV(now, is, the, winter);
}

void
fn56(int now, int is, int the, int winter, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5YV(now, is, the, winter, of);
}

void
fn57(int now, int is, int the, int winter, int of, int our)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6YV(now, is, the, winter, of, our);
}

void
fn58(int now, int is, int the, int winter, int of, int our, int discontent)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7YV(now, is, the, winter, of, our, discontent);
}

void
fn59(int now, int is, int the, int winter, int of, int our, int discontent, int made)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8YV(now, is, the, winter, of, our, discontent, made);
}

void
fn60(int now)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG1DV(now);
}

void
fn61(int now, int is)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG2DV(now, is);
}

void
fn62(int now, int is, int the)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG3DV(now, is, the);
}

void
fn63(int now, int is, int the, int winter)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG4DV(now, is, the, winter);
}

void
fn64(int now, int is, int the, int winter, int of)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG5DV(now, is, the, winter, of);
}

void
fn65(int now, int is, int the, int winter, int of, int our)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG6DV(now, is, the, winter, of, our);
}

void
fn66(int now, int is, int the, int winter, int of, int our, int discontent)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG7DV(now, is, the, winter, of, our, discontent);
}

void
fn67(int now, int is, int the, int winter, int of, int our, int discontent, int made)
{
    CYG_REPORT_FUNCTION();
    CYG_REPORT_FUNCARG8DV(now, is, the, winter, of, our, discontent, made);
}

int
main(int argc, char** argv)
{
    CYG_TRACE0(junk, fmt);
    CYG_TRACE1(junk, fmt, now);
    CYG_TRACE2(junk, fmt, now, is);
    CYG_TRACE3(junk, fmt, now, is, the);
    CYG_TRACE4(junk, fmt, now, is, the, winter);
    CYG_TRACE5(junk, fmt, now, is, the, winter, of);
    CYG_TRACE6(junk, fmt, now, is, the, winter, of, our);
    CYG_TRACE7(junk, fmt, now, is, the, winter, of, our, discontent);
    CYG_TRACE8(junk, fmt, now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE0B(fmt);
    CYG_TRACE1B(fmt, now);
    CYG_TRACE2B(fmt, now, is);
    CYG_TRACE3B(fmt, now, is, the);
    CYG_TRACE4B(fmt, now, is, the, winter);
    CYG_TRACE5B(fmt, now, is, the, winter, of);
    CYG_TRACE6B(fmt, now, is, the, winter, of, our);
    CYG_TRACE7B(fmt, now, is, the, winter, of, our, discontent);
    CYG_TRACE8B(fmt, now, is, the, winter, of, our, discontent, made);

    CYG_TRACE1X(junk, now);
    CYG_TRACE2X(junk, now, is);
    CYG_TRACE3X(junk, now, is, the);
    CYG_TRACE4X(junk, now, is, the, winter);
    CYG_TRACE5X(junk, now, is, the, winter, of);
    CYG_TRACE6X(junk, now, is, the, winter, of, our);
    CYG_TRACE7X(junk, now, is, the, winter, of, our, discontent);
    CYG_TRACE8X(junk, now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1Y(junk, now);
    CYG_TRACE2Y(junk, now, is);
    CYG_TRACE3Y(junk, now, is, the);
    CYG_TRACE4Y(junk, now, is, the, winter);
    CYG_TRACE5Y(junk, now, is, the, winter, of);
    CYG_TRACE6Y(junk, now, is, the, winter, of, our);
    CYG_TRACE7Y(junk, now, is, the, winter, of, our, discontent);
    CYG_TRACE8Y(junk, now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1D(junk, now);
    CYG_TRACE2D(junk, now, is);
    CYG_TRACE3D(junk, now, is, the);
    CYG_TRACE4D(junk, now, is, the, winter);
    CYG_TRACE5D(junk, now, is, the, winter, of);
    CYG_TRACE6D(junk, now, is, the, winter, of, our);
    CYG_TRACE7D(junk, now, is, the, winter, of, our, discontent);
    CYG_TRACE8D(junk, now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1XV(junk, now);
    CYG_TRACE2XV(junk, now, is);
    CYG_TRACE3XV(junk, now, is, the);
    CYG_TRACE4XV(junk, now, is, the, winter);
    CYG_TRACE5XV(junk, now, is, the, winter, of);
    CYG_TRACE6XV(junk, now, is, the, winter, of, our);
    CYG_TRACE7XV(junk, now, is, the, winter, of, our, discontent);
    CYG_TRACE8XV(junk, now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1YV(junk, now);
    CYG_TRACE2YV(junk, now, is);
    CYG_TRACE3YV(junk, now, is, the);
    CYG_TRACE4YV(junk, now, is, the, winter);
    CYG_TRACE5YV(junk, now, is, the, winter, of);
    CYG_TRACE6YV(junk, now, is, the, winter, of, our);
    CYG_TRACE7YV(junk, now, is, the, winter, of, our, discontent);
    CYG_TRACE8YV(junk, now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1DV(junk, now);
    CYG_TRACE2DV(junk, now, is);
    CYG_TRACE3DV(junk, now, is, the);
    CYG_TRACE4DV(junk, now, is, the, winter);
    CYG_TRACE5DV(junk, now, is, the, winter, of);
    CYG_TRACE6DV(junk, now, is, the, winter, of, our);
    CYG_TRACE7DV(junk, now, is, the, winter, of, our, discontent);
    CYG_TRACE8DV(junk, now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1XB(now);
    CYG_TRACE2XB(now, is);
    CYG_TRACE3XB(now, is, the);
    CYG_TRACE4XB(now, is, the, winter);
    CYG_TRACE5XB(now, is, the, winter, of);
    CYG_TRACE6XB(now, is, the, winter, of, our);
    CYG_TRACE7XB(now, is, the, winter, of, our, discontent);
    CYG_TRACE8XB(now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1YB(now);
    CYG_TRACE2YB(now, is);
    CYG_TRACE3YB(now, is, the);
    CYG_TRACE4YB(now, is, the, winter);
    CYG_TRACE5YB(now, is, the, winter, of);
    CYG_TRACE6YB(now, is, the, winter, of, our);
    CYG_TRACE7YB(now, is, the, winter, of, our, discontent);
    CYG_TRACE8YB(now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1DB(now);
    CYG_TRACE2DB(now, is);
    CYG_TRACE3DB(now, is, the);
    CYG_TRACE4DB(now, is, the, winter);
    CYG_TRACE5DB(now, is, the, winter, of);
    CYG_TRACE6DB(now, is, the, winter, of, our);
    CYG_TRACE7DB(now, is, the, winter, of, our, discontent);
    CYG_TRACE8DB(now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1XVB(now);
    CYG_TRACE2XVB(now, is);
    CYG_TRACE3XVB(now, is, the);
    CYG_TRACE4XVB(now, is, the, winter);
    CYG_TRACE5XVB(now, is, the, winter, of);
    CYG_TRACE6XVB(now, is, the, winter, of, our);
    CYG_TRACE7XVB(now, is, the, winter, of, our, discontent);
    CYG_TRACE8XVB(now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1YVB(now);
    CYG_TRACE2YVB(now, is);
    CYG_TRACE3YVB(now, is, the);
    CYG_TRACE4YVB(now, is, the, winter);
    CYG_TRACE5YVB(now, is, the, winter, of);
    CYG_TRACE6YVB(now, is, the, winter, of, our);
    CYG_TRACE7YVB(now, is, the, winter, of, our, discontent);
    CYG_TRACE8YVB(now, is, the, winter, of, our, discontent, made);
    
    CYG_TRACE1DVB(now);
    CYG_TRACE2DVB(now, is);
    CYG_TRACE3DVB(now, is, the);
    CYG_TRACE4DVB(now, is, the, winter);
    CYG_TRACE5DVB(now, is, the, winter, of);
    CYG_TRACE6DVB(now, is, the, winter, of, our);
    CYG_TRACE7DVB(now, is, the, winter, of, our, discontent);
    CYG_TRACE8DVB(now, is, the, winter, of, our, discontent, made);

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
    
    CYG_TEST_PASS_FINISH("disabled tracing does nothing");
    return 0;
}
