//==========================================================================
//
//        clockcnv.cxx
//
//        Clock Converter test
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
// Copyright (C) 2003 Gary Thomas
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     hmt
// Contributors:  hmt
// Date:          2000-01-24
// Description:   Tests the Kernel Real Time Clock Converter subsystem
// 
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>

#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/thread.hxx>

#include <cyg/infra/testcase.h>

#include <cyg/kernel/clock.inl>
#include <cyg/kernel/thread.inl>

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK

#include <cyg/infra/diag.h>

static void null_printf(const char *, ...)
{ /* nothing */ }

#define PRINTF0 diag_printf
#define nPRINTF0 null_printf

#define nPRINTF1 diag_printf
#define PRINTF1 null_printf

#define nPRINTF2 diag_printf
#define PRINTF2 null_printf


#define NTHREADS 1
#include "testaux.hxx"

static struct { cyg_uint32 ns; double scale; } ns_tickers[] = {
    { 70000000, 7.0 },                  // 7cS
    { 50000000, 5.0 },                  // 5cS
    { 45000000, 4.5 },                  // 4.5cS
    { 30000000, 3.0 },                  // 3cS
    { 20000000, 2.0 },                  // 2cS
    { 10000000, 1.0 },                  // 1cS - no change
    {  5000000, 0.5 },                  // 1/2 a cS
    {  4900000, 0.49 },                 // a bit below
    {  3333333, 0.3333333 },            // 1/3 cS
    {  1250000, 0.125 },                // 800Hz
    {  1000000, 0.1 },                  // 1000Hz
    {   909090, 0.0909090 },            // 1100Hz
    {   490000, 0.049 },                // 490uS
    {   333333, 0.0333333 },            // 1/30 cS, 1/3mS
    {    49000, 0.0049 },               // 49uS
    {    33333, 0.0033333 },            // 1/30 mS
    {     4900, 0.00049 },              // 4.9uS
    // now some outlandish ones
    {      170, 0.000017 },             // 170nS
    {       11, 0.0000011 },            // 11nS
    { 1000000000u, 100.0 },             // one second
    { 1234567777u, 123.4567777 },       // 1.234... seconds
    { 4294967291u, 429.4967291 },       // 4.3 seconds, nearly maxint.
    // now some which are prime in the nS per tick field
    {   909091, 0.0909091 },            // also 1100Hz - but 909091 is a prime!
    // and some eye-pleasing primes from the www - if they're not actually
    // prime, don't blame me.   http://www.rsok.com/~jrm/printprimes.html
    {  1000003, 0.1000003 },
    {  1477771, 0.1477771 },
    {  2000003, 0.2000003 },
    {  2382001, 0.2382001 },
    {  3333133, 0.3333133 },
    {  3999971, 0.3999971 },    
    {  5555591, 0.5555591 },
    {  6013919, 0.6013919 },
    // That's enough
};

static void entry0( CYG_ADDRWORD data )
{
    // First just try it with the clock as default:
    Cyg_Clock *rtc = Cyg_Clock::real_time_clock;

    Cyg_Clock::converter cv, rcv;
    Cyg_Clock::cyg_resolution res;

    unsigned int counter = 0;
    unsigned int skipped = 0;

    unsigned int i;
    for ( i = 0; i < sizeof( ns_tickers )/sizeof( ns_tickers[0] ); i++ ) {

        unsigned int lcounter = 0;
        unsigned int lskipped = 0;

        rtc->get_other_to_clock_converter( ns_tickers[i].ns, &cv );
        rtc->get_clock_to_other_converter( ns_tickers[i].ns, &rcv );
        
        PRINTF1( "ns per tick: %d\n", ns_tickers[i].ns );
        PRINTF1( "  converter: * %d / %d * %d / %d\n",
                     (int)cv.mul1, (int)cv.div1, (int)cv.mul2,(int) cv.div2 );
        PRINTF1( "   reverser: * %d / %d * %d / %d\n",
                     (int)rcv.mul1, (int)rcv.div1, (int)rcv.mul2, (int)rcv.div2 );

        double d = 1.0;
        d *= (double)cv.mul1;
        d /= (double)cv.div1;
        d *= (double)cv.mul2;
        d /= (double)cv.div2;
        d *= (double)rcv.mul1;
        d /= (double)rcv.div1;
        d *= (double)rcv.mul2;
        d /= (double)rcv.div2;
        PRINTF1( "       composite product %d.%d\n",
                     (int)d, ((int)(d * 1000000) % 1000000 ) );
        d -= 1.0;
        CYG_TEST_CHECK( d < +0.0001, "Overflow in composite product" );
        CYG_TEST_CHECK( d > -0.0001, "Underflow in composite product" );

        res = rtc->get_resolution();

        double factor_other_to_clock;
        double factor_clock_to_other;

        // res.dividend/res.divisor is the number of nS in a system
        // clock tick.  So:
        d = (double)res.dividend/(double)res.divisor;

        factor_other_to_clock = ns_tickers[i].scale * 1.0e7 / d ;
        factor_clock_to_other = d / (ns_tickers[i].scale * 1.0e7);

        unsigned int j;
        for ( j = 1; j < 100; j++ ) {
            cyg_uint64 delay;
            if (cyg_test_is_simulator)
                j += 30;                // test fewer values
                                   /* tr.b..m..k.. */

#ifdef CYGPKG_HAL_V85X_V850_CEB
            j += 30;                    // test fewer values
#endif

            for ( delay = j; delay < 1000000000000ll; delay *= 10 ) {
                // get the converted result
                cyg_uint64 result = Cyg_Clock::convert( delay, &cv );

                counter++; lcounter++;
                if ( (double)delay * (double)cv.mul1 > 1.6e+19 ||
                     (double)delay * (double)rcv.mul1 > 1.6e+19 ) {
                    // in silly territory now, give up.
                    // (that's MAXINT squared, approx.)
                    skipped++; lskipped++; 
                    continue; // so the counter is accurate
                }

                PRINTF2( "delay %x%08x to result %x%08x\n", 
                             (int)(delay >> 32), (int)delay,
                             (int)(result >> 32), (int)result );

                // get what it should be in double maths
                double delta = factor_other_to_clock * (double)delay;
                if ( delta > 1000.0 ) {
                    delta = (double)result - delta;
                    delta /= (double)result;
                    CYG_TEST_CHECK( delta <= +0.01,
                          "Proportional overflow in conversion to" );
                    CYG_TEST_CHECK( delta >= -0.01,
                          "Proportional underflow in conversion to" );
                }
                else {
                    cyg_uint64 lo = (cyg_uint64)(delta); // assume TRUNCATION
                    cyg_uint64 hi = lo + 1;
                    CYG_TEST_CHECK( hi >= result,
                                    "Range overflow in conversion to" );
                    CYG_TEST_CHECK( lo <= result,
                                    "Range underflow in conversion to" );
                }

                // get the converted result
                result = Cyg_Clock::convert( delay, &rcv );

                PRINTF2( "delay %x%08x from result %x%08x\n", 
                             (int)(delay >> 32), (int)delay,
                             (int)(result >> 32), (int)result );

                // get what it should be in double maths
                delta = factor_clock_to_other * (double)delay;
                if ( delta > 1000.0 ) {
                    delta = (double)result - delta;
                    delta /= (double)result;
                    CYG_TEST_CHECK( delta <= +0.01,
                          "Proportional overflow in conversion from" );
                    CYG_TEST_CHECK( delta >= -0.01,
                          "Proportional underflow in conversion from" );
                }
                else {
                    cyg_uint64 lo = (cyg_uint64)(delta); // assume TRUNCATION
                    cyg_uint64 hi = lo + 1;
                    CYG_TEST_CHECK( hi >= result,
                                    "Range overflow in conversion from" );
                    CYG_TEST_CHECK( lo <= result,
                                    "Range underflow in conversion from" );
                }

                if (cyg_test_is_simulator)
                    break;
            }
        }
        PRINTF0( "INFO:<%d nS/tick: tested %d values, skipped %d because of overflow>\n",
                 ns_tickers[i].ns, lcounter, lskipped );
    }

    PRINTF0( "INFO:<tested %d values, total skipped %d because of overflow>\n",
             counter, skipped );

    CYG_TEST_PASS_FINISH("ClockCnv OK");
}

void clockcnv_main( void )
{
    CYG_TEST_INIT();
    new_thread(entry0, (CYG_ADDRWORD)&thread_obj[0]);
    Cyg_Scheduler::start();
}

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    clockcnv_main();
}

#else // def CYGVAR_KERNEL_COUNTERS_CLOCK

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_NA( "Kernel real-time clock disabled");
}

#endif // def CYGVAR_KERNEL_COUNTERS_CLOCK

// EOF clockcnv.cxx
