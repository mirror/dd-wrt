//=================================================================
//
//        rand3.c
//
//        Testcase for C library rand()
//
//=================================================================
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors:  
// Date:          2000-04-30
// Description:   Contains testcode for C library rand() function. This tests
//                that random numbers are distributed well between 0 and
//                RAND_MAX
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <stdlib.h>
#include <cyg/infra/testcase.h>


// CONSTANTS

#define NUM_BUCKETS 1000     // how many categories to define
#define TEST_LENGTH 200000   // how many samples to take - careful
                             // when reducing this since it also reduces
                             // BUCKET_DIFF_TOLERANCE below. If you reduce
                             // it too low, BUCKET_DIFF_TOLERANCE will need
                             // a fudge factor

#define BUCKET_SIZE    (RAND_MAX / NUM_BUCKETS)  // number space allocated
                                                 // to bucket from 0..RAND_MAX
#define NUM_PER_BUCKET (TEST_LENGTH/NUM_BUCKETS) // Expected number that went
                                                 // into each bucket at end

// how much the buckets can vary at the end.
#define BUCKET_DIFF_TOLERANCE (NUM_PER_BUCKET/4) // allowed to vary 25%



// FUNCTIONS

static __inline__ int
my_abs(int i)
{
    return (i < 0) ? -i : i;
} // my_abs()

int
main(int argc, char *argv[])
{
    // divide the space from 0..RAND_MAX into NUM_BUCKETS categories *BUT*
    // RAND_MAX / NUM_BUCKETS may not divide exactly so we leave space for
    // the bits left over, in case there are any! So we add 1.

    static cyg_uint8 rand_bucket[NUM_BUCKETS+1];    
    cyg_ucount32 count;                        // loop variable
    int r;                                     // temp for rand() variable

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "rand() function");

    CYG_TEST_INFO("This test tests the distribution of random numbers and");
    CYG_TEST_INFO("may take some time");

    for ( count=0; count < TEST_LENGTH; ++count ) {
        r = rand();
        ++rand_bucket[ r / BUCKET_SIZE ];
        if ((count%10000)==0)
            CYG_TEST_STILL_ALIVE(count, "Still testing...");
    } // for

    for ( count=0; count < NUM_BUCKETS; ++count ) {
        cyg_ucount32 diff;

        diff = my_abs( rand_bucket[count] - NUM_PER_BUCKET );
        if ( diff > BUCKET_DIFF_TOLERANCE )
            break;
    } // for

    // if the previous loop completed, we may want to check the "extra"
    // bucket (see the comment at the top) that may have some bits in if
    // RAND_MAX doesn't split into NUM_BUCKETS evenly. The number of random
    // digits that fell into that bucket would be expected to be proportional
    // to the ratio of the remainder of (RAND_MAX % NUM_BUCKETS) to
    // NUM_BUCKETS. 
    if (count == NUM_BUCKETS) {
        cyg_ucount32 rem;
        cyg_ucount32 last_bucket_expected;
        cyg_ucount32 diff;

        rem = RAND_MAX % NUM_BUCKETS;

        last_bucket_expected = (rem * NUM_PER_BUCKET) / BUCKET_SIZE;

        diff = my_abs(last_bucket_expected - rand_bucket[count]);
        CYG_TEST_PASS_FAIL(diff <= BUCKET_DIFF_TOLERANCE,
                           "Upper bound fencepost test");
    }
    CYG_TEST_PASS_FAIL( (count >= NUM_BUCKETS),
                        "even distribution of rand()");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for "
                    "C library rand() function");
} // main()


// EOF rand3.c
