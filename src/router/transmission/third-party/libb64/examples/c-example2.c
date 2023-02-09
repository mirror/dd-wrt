/*
c-example2.c - libb64 example code

This is part of the libb64 project, and has been placed in the public domain.
For details, see http://sourceforge.net/projects/libb64

This is a short example of how to use libb64's C interface to encode
and decode a file directly.
The main work is done between the START/STOP ENCODING and DECODING lines.
The main difference between this code and c-example1.c is that we do not
know the size of the input file before hand, and so we use to iterate over
encoding and decoding the data.
*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <b64/cencode.h>
#include <b64/cdecode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

/* arbitrary buffer size */
#define SIZE 100

void encode(FILE* inputFile, FILE* outputFile)
{
	/* set up a destination buffer large enough to hold the encoded data */
	int size = SIZE;
	char* input = (char*)malloc(size);
	char* encoded = (char*)malloc(2*size); /* ~4/3 x input */
	/* we need an encoder and decoder state */
	base64_encodestate es;
	/* store the number of bytes encoded by a single call */
	size_t cnt = 0;
	
	/*---------- START ENCODING ----------*/
	/* initialise the encoder state */
	base64_init_encodestate(&es);
	es.chars_per_line = 72;
	/* gather data from the input and send it to the output */
	while (1)
	{
		cnt = fread(input, sizeof(char), size, inputFile);
		if (cnt == 0) break;
		cnt = base64_encode_block(input, cnt, encoded, &es);
		/* output the encoded bytes to the output file */
		fwrite(encoded, sizeof(char), cnt, outputFile);
	}
	/* since we have reached the end of the input file, we know that 
	   there is no more input data; finalise the encoding */
	cnt = base64_encode_blockend(encoded, &es);
	/* write the last bytes to the output file */
	fwrite(encoded, sizeof(char), cnt, outputFile);
	/*---------- STOP ENCODING  ----------*/
	
	free(encoded);
	free(input);
}

void decode(FILE* inputFile, FILE* outputFile)
{
	/* set up a destination buffer large enough to hold the decoded data */
	int size = SIZE;
	char* encoded = (char*)malloc(size);
	char* decoded = (char*)malloc(1*size); /* ~3/4 x encoded */
	/* we need an encoder and decoder state */
	base64_decodestate ds;
	/* store the number of bytes encoded by a single call */
	size_t cnt = 0;
	
	/*---------- START DECODING ----------*/
	/* initialise the encoder state */
	base64_init_decodestate(&ds);
	/* gather data from the input and send it to the output */
	while (1)
	{
		cnt = fread(encoded, sizeof(char), size, inputFile);
		if (cnt == 0) break;
		cnt = base64_decode_block(encoded, cnt, decoded, &ds);
		/* output the encoded bytes to the output file */
		fwrite(decoded, sizeof(char), cnt, outputFile);
	}
	/*---------- START DECODING  ----------*/
	
	free(encoded);
	free(decoded);
}

int main(int argc, char** argv)
{
	FILE* inputFile;
	FILE* encodedFile;
	FILE* decodedFile;
	
	if (argc < 4)
	{
		printf("please supply three filenames: input, encoded & decoded\n");
		exit(-1);
	}
	
	/* encode the input file */
	
	inputFile   = fopen(argv[1], "r");
	encodedFile = fopen(argv[2], "w");
	
	encode(inputFile, encodedFile);
	
	fclose(inputFile);
	fclose(encodedFile);

	/* decode the encoded file */
	
	encodedFile = fopen(argv[2], "r");
	decodedFile = fopen(argv[3], "w");
	
	decode(encodedFile, decodedFile);
	
	fclose(encodedFile);
	fclose(decodedFile);

	/* we leave the original vs. decoded data comparison to 
		diff in the Makefile */
		
	return 0;
}


