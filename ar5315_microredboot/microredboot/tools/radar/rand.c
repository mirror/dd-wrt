#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>

void
set_random_seed(int test)
{
struct timeval t;
long int r;

    gettimeofday(&t, 0);
    srandom(t.tv_usec);
    if (test)
        srandom(0);
    r = random();
    srandom(r);
}

unsigned int
pickanumber(unsigned int lo, unsigned int hi)
{
long int r, d, x, i;

    r = random();		// pick a number from [0, RAND_MAX]
    d = r&0xf;                  // mask down to a 4-bit number
    for (i=0; i<d; i++)         // choose n more numbers and keep the last one
	r = random();
    d = hi - lo + 1;		// d is the integer range we want
    d = RAND_MAX/d;		// divide RAND_MAX into d buckets
    x = r/d + lo;		
    return x;
}

#define NHIST	10
#define	TEST	10000000
long int buckets[NHIST];
void
history()
{
int i, r;

    for (i=0; i<TEST; i++) {
	r = pickanumber(0, NHIST-1);
	if (r<0 || r>NHIST-1) {
	    printf("error r was %d\n", r);
	    continue;
	}
	buckets[r]++;
    }
    for (r=i=0; i<NHIST; i++) {
	printf("[%d]\t%d\n", i, buckets[i]);
	r += buckets[i];
    }
    printf("total %d\n", r);
}
