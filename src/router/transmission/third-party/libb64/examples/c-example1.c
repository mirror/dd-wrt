/*
c-example1.c - libb64 example code

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64

This is a short example of how to use libb64's C interface to encode
and decode a string directly.
The main work is done between the START/STOP ENCODING and DECODING lines.

Note that this is extremely simple; you will almost never have all the data
in a string ready to encode/decode!
Because we all the data to encode/decode in a string, and know its length,
we can get away with a single en/decode_block call.
For a more realistic example, see c-example2.c
*/

#include <b64/cencode.h>
#include <b64/cdecode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

/* arbitrary buffer size */
#define SIZE 100

char* encode(const char* input)
{
	/* set up a destination buffer large enough to hold the encoded data */
	char* output = (char*)malloc(SIZE);
	/* keep track of our encoded position */
	char* c = output;
	/* store the number of bytes encoded by a single call */
	size_t cnt = 0;
	/* we need an encoder state */
	base64_encodestate s;
	
	/*---------- START ENCODING ----------*/
	/* initialise the encoder state */
	base64_init_encodestate(&s);
	/* gather data from the input and send it to the output */
	cnt = base64_encode_block(input, strlen(input), c, &s);
	c += cnt;
	/* since we have encoded the entire input string, we know that 
	   there is no more input data; finalise the encoding */
	cnt = base64_encode_blockend(c, &s);
	c += cnt;
	/*---------- STOP ENCODING  ----------*/
	
	/* we want to print the encoded data, so null-terminate it: */
	*c = 0;
	
	return output;
}

char* decode(const char* input)
{
	/* set up a destination buffer large enough to hold the encoded data */
	char* output = (char*)malloc(SIZE);
	/* keep track of our decoded position */
	char* c = output;
	/* store the number of bytes decoded by a single call */
	size_t cnt = 0;
	/* we need a decoder state */
	base64_decodestate s;
	
	/*---------- START DECODING ----------*/
	/* initialise the decoder state */
	base64_init_decodestate(&s);
	/* decode the input data */
	cnt = base64_decode_block(input, strlen(input), c, &s);
	c += cnt;
	/* note: there is no base64_decode_blockend! */
	/*---------- STOP DECODING  ----------*/
	
	/* we want to print the decoded data, so null-terminate it: */
	*c = 0;
	
	return output;
}


int main(void)
{
	const char* input = "hello world";
	char* encoded;
	char* decoded;
	
	/* encode the data */
	encoded = encode(input);
	printf("encoded: %s", encoded); /* encoded data has a trailing newline */

	/* decode the data */
	decoded = decode(encoded);
	printf("decoded: %s\n", decoded);
	
	/* compare the original and decoded data */
	assert(strcmp(input, decoded) == 0);
	
	free(encoded);
	free(decoded);
	return 0;
}


