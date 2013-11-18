/* $Id: floattest.c 3855 2006-11-10 21:59:21Z ckuethe $ */
#include <stdio.h>

/*
 * Copyright (c) 2006 Chris Kuethe <chris.kuethe@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * this simple program tests to see whether your system can do proper
 * single and double precision floating point. This is apparently Very
 * Hard To Do(tm) on embedded systems, judging by the number of broken
 * ARM toolchains I've seen... :(
 *
 * compile with: gcc -O -o floattest floattest.c
 *     (use whatever -O level you like)
 */

int main( void );
int test_single( void );
int test_double( void );

int main(){
	int i;

	if ((i = test_single())){
		printf("WARNING: Single-precision "
			"floating point math might be broken (test %d)\n", i);
		return i;
	}

	if ((i = test_double())){
		printf("WARNING: Double-precision "
			"floating point math might be broken (test %d)\n", i);
		return i;
	}

	printf("floating point math appears to work\n");
	return 0;
}

int test_single(){
	static float f;
	static int i;

	/* addition test */
	f = 1.0;
	for(i = 0; i < 10; i++)
		f += (1<<i);
	if (f != 1024.0)
		return 1;

	/* subtraction test */
	f = 1024.0;
	for(i = 0; i < 10; i++)
		f -= (1<<i);
	if (f != 1.0)
		return 2;

	/* multiplication test */
	f = 1.0;
	for(i = 1; i < 10; i++)
		f *= i;
	if (f != 362880.0)
		return 3;

	/* division test */
	f = 362880.0;
	for(i = 1; i < 10; i++)
		f /= i;
	if (f != 1.0)
		return 4;

	/* multiply-accumulate test */
	f = 0.5;
	for(i = 1; i < 1000000; i++){
		f += 2.0;
		f *= 0.5;
	}
	if (f != 2.0)
		return 5;

	/* divide-subtract test */
	f = 2.0;
	for(i = 1; i < 1000000; i++){
		f /= 0.5;
		f -= 2.0;
	}
	if (f != 2.0)
		return 6;

	/* add-multiply-subtract-divide test */
	f = 1000000.0;
	for(i = 1; i < 1000000; i++)
		f = ((((f + 1.5) * 0.5) - 1.25) / 0.5);
	if (f != 1.0)
		return 7;

	/* multiply-add-divide-subtract test */
	f = 1.0;
	for(i = 1; i < 1000000; i++)
		f = ((((f * 5.0) + 3.0) / 2.0) - 3.0);
	if (f != 1.0)
		return 8;

	/* subtract-divide-add-multiply test */
	f = 8.0;
	for(i = 1; i < 1000000; i++)
		f = ((((f - 5.0) / 2.0) + 2.5) * 2.0);
	if (f != 8.0)
		return 9;

	/* divide-subtract-multiply-add test */
	f = 42.0;
	for(i = 1; i < 1000000; i++)
		f = ((((f / 6.0) - 5.0) * 19.75 ) + 2.5);
	if (f != 42.0)
		return 10;

	return 0;
}


int test_double(){
	static double f;
	static int i;

	/* addition test */
	f = 1.0;
	for(i = 0; i < 10; i++)
		f += (1<<i);
	if (f != 1024.0)
		return 1;

	/* subtraction test */
	f = 1024.0;
	for(i = 0; i < 10; i++)
		f -= (1<<i);
	if (f != 1.0)
		return 2;

	/* multiplication test */
	f = 1.0;
	for(i = 1; i < 10; i++)
		f *= i;
	if (f != 362880.0)
		return 3;

	/* division test */
	f = 362880.0;
	for(i = 1; i < 10; i++)
		f /= i;
	if (f != 1.0)
		return 4;

	/* multiply-accumulate test */
	f = 0.5;
	for(i = 1; i < 1000000; i++){
		f += 2.0;
		f *= 0.5;
	}
	if (f != 2.0)
		return 5;

	/* divide-subtract test */
	f = 2.0;
	for(i = 1; i < 1000000; i++){
		f /= 0.5;
		f -= 2.0;
	}
	if (f != 2.0)
		return 6;

	/* add-multiply-subtract-divide test */
	f = 1000000.0;
	for(i = 1; i < 1000000; i++)
		f = ((((f + 1.5) * 0.5) - 1.25) / 0.5);
	if (f != 1.0)
		return 7;

	/* multiply-add-divide-subtract test */
	f = 1.0;
	for(i = 1; i < 1000000; i++){
		f = ((((f * 5.0) + 3.0) / 2.0) - 3.0);
	}
	if (f != 1.0)
		return 8;

	/* subtract-divide-add-multiply test */
	f = 8.0;
	for(i = 1; i < 1000000; i++)
		f = ((((f - 5.0) / 2.0) + 2.5) * 2.0);
	if (f != 8.0)
		return 9;

	/* divide-subtract-multiply-add test */
	f = 42.0;
	for(i = 1; i < 1000000; i++)
		f = ((((f / 6.0) - 5.0) * 19.75 ) + 2.5);
	if (f != 42.0)
		return 10;

	return 0;
}
