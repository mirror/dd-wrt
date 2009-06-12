#ifndef CYGONCE_LIBM_VECTOR_SUPPORT_H
#define CYGONCE_LIBM_VECTOR_SUPPORT_H
//========================================================================
//
//      vector_support.h
//
//      Support for testing of the math library using test vectors
//
//========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  jlarmour
// Date:          1999-01-21
// Purpose:     
// Description: 
// Usage:         #include "vectors/vector_support.h"
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libm.h>      // Configuration header
#include <pkgconf/isoinfra.h>  // CYGINT_ISO_MAIN_STARTUP

// INCLUDES

#include <cyg/infra/cyg_type.h>   // Common type definitions and support
#include <cyg/infra/testcase.h>   // Test infrastructure
#include <math.h>                 // Header for this package
#include <sys/ieeefp.h>           // Cyg_libm_ieee_double_shape_type
#include <cyg/infra/diag.h>

#ifndef CYGSEM_LIBM_COMPAT_IEEE_ONLY
# include <errno.h>                // For Cyg_ErrNo
#endif

// CONSTANTS

#define PROBLEM_THRESHOLD 10      // Number of test vectors allowed to fail
                                  // before we give up completely

// HOW TO START TESTS

#if CYGINT_ISO_MAIN_STARTUP

# define START_TEST( test ) test(0)

#elif defined(CYGFUN_KERNEL_API_C)

# include <cyg/hal/hal_arch.h>
# include <cyg/kernel/kapi.h>

# define STACKSIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

static cyg_uint8 stack[STACKSIZE];
static cyg_handle_t thr_handle;
static cyg_thread thr;

# define START_TEST( test ) CYG_MACRO_START \
    cyg_thread_create( 4, &test, (cyg_addrword_t)0, "test", \
                       &stack[0],  STACKSIZE, &thr_handle, &thr ); \
    cyg_thread_resume( thr_handle ); \
    cyg_scheduler_start(); \
    CYG_MACRO_END

externC int main( int, char ** );

externC void
cyg_user_start( void )
{
    (void) main(0, NULL);
} // cyg_user_start()

#else // !defined(CYGFUN_KERNEL_API_C)

externC int main( int, char ** );

externC void
cyg_user_start( void )
{
    (void) main(0, NULL);
} // cyg_user_start()

# define START_TEST( test ) test(0)

#endif

// TYPE DEFINITIONS


typedef enum {
    CYG_LIBM_TEST_VEC_NONE,         // this indicates whether the "double"
    CYG_LIBM_TEST_VEC_INT,          // is being used to store a double, an
    CYG_LIBM_TEST_VEC_DOUBLE,       // int, or its not being used at all!
    CYG_LIBM_TEST_VEC_INT_P,        // int pointer
    CYG_LIBM_TEST_VEC_DOUBLE_P      // double pointer
} Cyg_libm_test_arg_type;
                              

// Define a type for a test vector record

typedef struct {
    cyg_ucount32 vector_num;        // id number of this test vector record
    
    // if any of the following arguments are ints rather than doubles, then
    // use the lsw part to store it

    cyg_uint32 arg1_msw;   // first argument
    cyg_uint32 arg1_lsw;
    cyg_uint32 arg2_msw;   // second argument
    cyg_uint32 arg2_lsw;
    cyg_uint32 result_msw; // expected return value
    cyg_uint32 result_lsw;

#ifndef CYGSEM_LIBM_COMPAT_IEEE_ONLY
    Cyg_ErrNo errno_val;           // expected value of errno. 0==unchanged
#else
    cyg_uint32 errno_val;
#endif

    double tolerance;              // relative amount that it is allowed
                                   // to vary. i.e. the value should be
                                   // plus or minus result*tolerance

    int matherr_type;              // if testing the matherr() function,
                                   // what type should the exception be
} Cyg_libm_test_double_vec_t;


// types to cope with the different forms of function to be called by the
// test vector

typedef double (*ddfn)( double, double );
typedef double (*difn)( double, int );
typedef double (*dfn)( double );
typedef double (*ddpfn)( double, double *);
typedef double (*dipfn)( double, int *);


// STATIC FUNCTIONS

// equivalent of abs() for doubles. We want to be independent of fabs()

static double
my_abs_d( double x )
{
    Cyg_libm_ieee_double_shape_type t;

    t.value = x;

    t.number.sign = 0;

    return t.value;
} // my_abs_d()


static cyg_bool
checkErrorAcceptable( Cyg_libm_ieee_double_shape_type is,
                      Cyg_libm_ieee_double_shape_type shouldbe,
                      double tolerance) // _relative_ amount it can vary
{
    Cyg_libm_ieee_double_shape_type temp_doub;

    // first do a short-circuit check if its identical
    if ( (is.parts.lsw == shouldbe.parts.lsw) &&
         (is.parts.msw == shouldbe.parts.msw) )
        return false;
    
    // now check special cases
    
    // +0 == -0
    if ( (is.parts.lsw == 0) && (shouldbe.parts.lsw == 0) &&
         ((is.parts.msw & 0x7fffffff) == 0) &&
         ((shouldbe.parts.msw & 0x7fffffff) == 0) )
        return false;

    // +-infinity == +-infinity
    if ((is.parts.lsw == 0) && (shouldbe.parts.lsw == 0) &&
        (is.number.fraction0 == 0) && (shouldbe.number.fraction0 == 0) &&
        (is.number.exponent == 2047) && (shouldbe.number.exponent == 2047))
    {
        return (is.number.sign != shouldbe.number.sign);
    } // if

    // both NaN. Assumes isnan works, but its pretty safe
    if ( isnan(is.value) && isnan(shouldbe.value) )
        return false;
    else if (isnan(is.value) || isnan(shouldbe.value) )
        return true;

    // check same sign. Even around small values close to 0 we would want
    // it to be on the right _side_ of 0
    if ( is.number.sign != shouldbe.number.sign )
        return true;

    // okay, now work out what tolerance means for us

    // find out what the difference is in the first place
    temp_doub.value = my_abs_d( shouldbe.value - is.value );

    // Check "both ways round" to deal with both under and overflow cases
    if ( ((temp_doub.value / tolerance) < my_abs_d(shouldbe.value)) ||
         (temp_doub.value < (my_abs_d(shouldbe.value) * tolerance)) )
        return false;
    else
        return true; // so its not close enough
    
} // checkErrorAcceptable()


// This allows us to run through any test vectors of form:
// double = fn(double, double)
// double = fn(double)
// double = fn(double, int)
// double = fn(double, int *)
// double = fn(double, double *)
//
// result type being non-double is left as a future enhancement
//
// This returns true if the tests all succeeded and false if any failed
//
static cyg_bool
doTestVec( CYG_ADDRESS func_ptr,
           Cyg_libm_test_arg_type arg1_type,
           Cyg_libm_test_arg_type arg2_type,
           Cyg_libm_test_arg_type result_type,
           const Cyg_libm_test_double_vec_t *vectors,
           cyg_ucount32 num_vectors )
{
    cyg_ucount32 problems=0;
    cyg_ucount32 i;
    Cyg_libm_ieee_double_shape_type arg1, arg2, result_wanted, ret;
    cyg_ucount32 alive_count = num_vectors / 10;

    if ((arg1_type != CYG_LIBM_TEST_VEC_DOUBLE) ||
        (result_type != CYG_LIBM_TEST_VEC_DOUBLE) ) {
        CYG_TEST_FAIL("Test vector arguments are not correct type!");
        return false;
    } // if

    switch (arg2_type) {
    case CYG_LIBM_TEST_VEC_DOUBLE:

        {
            ddfn fn = (ddfn) func_ptr;

            for (i=0;
                 (i < num_vectors) && (problems < PROBLEM_THRESHOLD);
                 i++) {

                if (0 == i % alive_count)
                    CYG_TEST_STILL_ALIVE(i, "Still crunching, please wait...");

                arg1.parts.msw = vectors[i].arg1_msw;
                arg1.parts.lsw = vectors[i].arg1_lsw;

                arg2.parts.msw = vectors[i].arg2_msw;
                arg2.parts.lsw = vectors[i].arg2_lsw;

                result_wanted.parts.msw = vectors[i].result_msw;
                result_wanted.parts.lsw = vectors[i].result_lsw;

                ret.value = (*fn)( arg1.value, arg2.value );

                if ((vectors[i].errno_val) != 0) {

#ifndef CYGSEM_LIBM_COMPAT_IEEE_ONLY
                    // In IEEE-mode we can't check the answer if this
                    // is an error case

                    if ((cyg_libm_get_compat_mode() !=
                        CYGNUM_LIBM_COMPAT_IEEE) &&
                        (errno != vectors[i].errno_val)) {

                        ++problems;
                        diag_printf("Vector #%d\n", i+1);
                        CYG_TEST_FAIL( "error not set correctly");

                    } // if
#endif

                    continue; // no point checking value in an error case
                } // if
                if (checkErrorAcceptable( ret, result_wanted,
                                          vectors[i].tolerance) ) {
                    ++problems;
                    diag_printf("Vector #%d\n", i+1);
                    CYG_TEST_FAIL( "Result out of tolerance");
                } // if
            } // for
            
        } // compound
            
        break;

    case CYG_LIBM_TEST_VEC_INT:

        {
            difn fn = (difn) func_ptr;
        
            for (i=0;
                 (i < num_vectors) && (problems < PROBLEM_THRESHOLD);
                 i++) {

                if (0 == i % alive_count)
                    CYG_TEST_STILL_ALIVE(i, "Still crunching, please wait...");

                arg1.parts.msw = vectors[i].arg1_msw;
                arg1.parts.lsw = vectors[i].arg1_lsw;

                result_wanted.parts.msw = vectors[i].result_msw;
                result_wanted.parts.lsw = vectors[i].result_lsw;

                ret.value = (*fn)( arg1.value, vectors[i].arg2_lsw );
                if (checkErrorAcceptable( ret, result_wanted,
                                          vectors[i].tolerance) ) {
                    ++problems;
                    diag_printf("Vector #%d\n", i+1);
                    CYG_TEST_FAIL( "Result out of tolerance");
                } // if
            } // for
            
        } // compound
            
        break;

    case CYG_LIBM_TEST_VEC_INT_P:

        {
            dipfn fn = (dipfn) func_ptr;
            int my_int;
            Cyg_libm_ieee_double_shape_type my_doub1, my_doub2;
        
            for (i=0;
                 (i < num_vectors) && (problems < PROBLEM_THRESHOLD);
                 i++) {

                if (0 == i % alive_count)
                    CYG_TEST_STILL_ALIVE(i, "Still crunching, please wait...");

                arg1.parts.msw = vectors[i].arg1_msw;
                arg1.parts.lsw = vectors[i].arg1_lsw;


                result_wanted.parts.msw = vectors[i].result_msw;
                result_wanted.parts.lsw = vectors[i].result_lsw;

                ret.value = (*fn)( arg1.value, &my_int );
                if (checkErrorAcceptable( ret, result_wanted,
                                          vectors[i].tolerance) ) {
                    ++problems;
                    diag_printf("Vector #%d\n", i+1);
                    CYG_TEST_FAIL( "Result out of tolerance");
                } // if

                my_doub1.value = (double) my_int;
                my_doub2.value = (double) (signed)vectors[i].arg2_lsw;
                
                if (checkErrorAcceptable( my_doub1, my_doub2,
                                          vectors[i].tolerance) ) {
                    ++problems;
                    diag_printf("Vector #%d\n", i+1);
                    CYG_TEST_FAIL( "Integer result out of tolerance");
                } // if

                
            } // for
            
        } // compound
            
        break;

    case CYG_LIBM_TEST_VEC_DOUBLE_P:

        {
            ddpfn fn = (ddpfn) func_ptr;
            Cyg_libm_ieee_double_shape_type my_doub1;
        
            for (i=0;
                 (i < num_vectors) && (problems < PROBLEM_THRESHOLD);
                 i++) {

                if (0 == i % alive_count)
                    CYG_TEST_STILL_ALIVE(i, "Still crunching, please wait...");

                arg1.parts.msw = vectors[i].arg1_msw;
                arg1.parts.lsw = vectors[i].arg1_lsw;

                arg2.parts.msw = vectors[i].arg2_msw;
                arg2.parts.lsw = vectors[i].arg2_lsw;


                result_wanted.parts.msw = vectors[i].result_msw;
                result_wanted.parts.lsw = vectors[i].result_lsw;

                ret.value = (*fn)( arg1.value, &my_doub1.value );
                if (checkErrorAcceptable( ret, result_wanted,
                                          vectors[i].tolerance) ) {
                    ++problems;
                    diag_printf("Vector #%d\n", i+1);
                    CYG_TEST_FAIL( "Result out of tolerance");
                } // if

                if (checkErrorAcceptable( my_doub1, arg2,
                                          vectors[i].tolerance) ) {
                    ++problems;
                    diag_printf("Vector #%d\n", i+1);
                    CYG_TEST_FAIL( "Returned double result out of "
                                   "tolerance");
                } // if

                
            } // for
            
        } // compound
            
        break;

    case CYG_LIBM_TEST_VEC_NONE:

        {
            dfn fn = (dfn) func_ptr;
        
            for (i=0;
                 (i < num_vectors) && (problems < PROBLEM_THRESHOLD);
                 i++) {

                if (0 == i % alive_count)
                    CYG_TEST_STILL_ALIVE(i, "Still crunching, please wait...");

                arg1.parts.msw = vectors[i].arg1_msw;
                arg1.parts.lsw = vectors[i].arg1_lsw;

                result_wanted.parts.msw = vectors[i].result_msw;
                result_wanted.parts.lsw = vectors[i].result_lsw;

                ret.value = (*fn)( arg1.value );
                if (checkErrorAcceptable( ret, result_wanted,
                                          vectors[i].tolerance) ) {
                    ++problems;
                    diag_printf("Vector #%d\n", i+1);
                    CYG_TEST_FAIL( "Result out of tolerance");
                } // if
            } // for
            
        } // compound
            
        break;

    default:
        CYG_TEST_FAIL("Second argument of unknown type!");
        return false;
    } // switch

    if (problems != 0)
        return false;
    else
        return true;

} // doTestVec()

#endif // CYGONCE_LIBM_VECTOR_SUPPORT_H multiple inclusion protection

// EOF vector_support.h
