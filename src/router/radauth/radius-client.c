/* Really simple radius authenticator
 *
 * Copyright (c) 2004 Michael Gernoth <michael@gernoth.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "radius.h"

int main(int argc, char** argv)
{
int trycount = 10,i=1;
	if ( argc < 6 )
	{
		fprintf(stderr,"radius-client - A simple radius authenticator\n");
		fprintf(stderr,"(C) 2004 Michael Gernoth,\n");
		fprintf(stderr,"(c) 2005 some modifications by Sebastian Gottschall\n");
		fprintf(stderr,"Usage: %s user pass server port shared-secret\n",argv[0]);
		exit(1);
	}
	while((trycount--)>0)
	{
	    if (radius(argv[3],atoi(argv[4]),argv[1],argv[2],argv[5]))
	    {
	    	    printf("Accept\n");
		    break;
	    } else {
		    sleep(1);
		    printf("Reject on try %d\n",i++);
	    }
	}
	exit(0);
}
