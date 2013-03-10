#include <stdio.h>
#include "mixbuffer.h"
#include "helpers.h"

void hex_encode(unsigned char * bytes, unsigned length)
{
	for(; length; --length)
	{
		fprintf(stderr, "%02X", *bytes++);
	}
}

void check_result1(unsigned st, int lbuf, const struct mixbuffer * mixb, struct mixstream * lb)
{

	struct state {
		char			buffer[40];
		struct mixbuffer	mb;
		struct mixstream	st;
	};
	struct state states[50];

	if(st < ITEMS_OF(states))
	{
//		struct state * state = states + st;
/*
		state->rb.buffer = rb->buffer;
		int rv = memcmp(rb->buffer, state->buffer, sizeof(state->buffer)) == 0 
				&&
			memcmp(rb, &state->rb, sizeof(state->rb)) == 0 
				&&
			memcmp(lb, &state->lb, sizeof(state->lb)) == 0;
*/
		fprintf(stderr, "'");
		hex_encode(mixb->rb.buffer, mixb->rb.size);
		fprintf(stderr, "', %2u, %2u, %2u, %2u, %2u, %2u, %2d\n", 
			(unsigned)mixb->rb.size, 
			(unsigned)mixb->rb.used, 
			(unsigned)mixb->rb.read, 
			(unsigned)mixb->rb.write, 
			(unsigned)lb->write, 
			(unsigned)lb->used, 
			lbuf
			);
	}
}

#/* */
void test_suite1()
{
	unsigned i;
	char buffer[40];
	
	static const char x1[] = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00 };
	static const char x2[] = { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00 };
	static const char x3[] = { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00 };
	static const char x4[] = { 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00 };
//	static const char x5[] = { 0x05, 0x05, 0x05, 0x05, 0x05, 0x00 };
//	static const char x6[] = { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00 };
//	static const char x7[] = { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 };
//	static const char x8[] = { 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x00 };
//	static const char x9[] = { 0x10, 0x10, 0x10, 0x10, 0x00 };
	static const char * strings[] = {
		x1, x2, x3, x4, 
//		x5, x6, x7, x8, x9
		};

	struct mixbuffer mb;
	struct mixstream locals[5];

	memset(buffer, 0, sizeof(buffer));

	mixb_init(&mb, buffer, sizeof(buffer));
	for(i = 0; i < ITEMS_OF(locals); i++)
		mixb_attach(&mb, &locals[i]);

	fprintf(stderr, "Testing rb_overwrite()");

	
	fprintf(stderr, "Data                                                      size used read write write1 used1 idx\n");
	for(i = 0; i < 50; i++) {
		int idx = i % ITEMS_OF(strings);
		unsigned length = strlen(strings[idx]);
		int lbuf = i % ITEMS_OF(locals);

		if(mixb_free(&mb, &locals[lbuf]) < length)
			mixb_read_upd(&mb, length - mixb_free(&mb, &locals[lbuf]));
		
		mixb_write(&mb, &locals[lbuf], strings[idx], length);
		check_result1(i, lbuf, &mb, &locals[lbuf]);
	}

	for(i = 0; i < ITEMS_OF(locals); i++)
		mixb_detach(&mb, &locals[i]);
	
}

#/* */
int main(int argc, char * argv[])
{
	test_suite1();

	return 0;
}