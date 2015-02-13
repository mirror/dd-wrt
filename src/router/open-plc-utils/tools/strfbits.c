/*====================================================================*
 *
 *   size_t strfbits (char  buffer [], size_t length, char const * operands [], char const * operator, unsigned flagword);
 *
 *   format.h
 *
 *   format buffer with an enumerated list of the bits in a flagword;
 *   each flagword bit position corresponds to a string in operands[]
 *   and operator is the string separating formatted operands;
 *
 *   enumeration continues until all bits are enumerated or operands
 *   are exhausted or the buffer fills;
 *
 *   for example, the following formats buffer with the literal string
 *   "one, three, five, six" since those bits are set;
 *
 *   char buffer[100];
 *   char const operator = ", ";
 *   char const *operands[] =
 *   {
 *      "zero",
 *      "one",
 *      "two",
 *      "three",
 *      "four",
 *      "five",
 *      "six",
 *      "seven",
 *      "eight",
 *      "nine",
 *      "ten",
 *      (char *)(0)
 *   };
 *   flag_t flags = 0x006C;
 *
 *   strfbits (buffer, sizeof(buffer), operands, operator, flags);
 *
 *   we decrement length before starting to reserve room for the NUL
 *   terminator; not room ... no write; we then add length to buffer
 *   before to compute the terminator address then subtract it after
 *   to compute the buffer start; this minimizes indexing and offset
 *   calculations within the loop;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef STRFBITS_SOURCE
#define STRFBITS_SOURCE

#include <unistd.h>

#include "../tools/memory.h"
#include "../tools/flags.h"

size_t strfbits (char buffer [], size_t length, char const * operands [], char const * operator, unsigned flagword)

{
	char * string = (char *)(buffer);
	char const *separator = "";
	if (length--)
	{
		buffer += length;
		while ((*operands) && (flagword))
		{
			if (flagword & 1)
			{
				char const *symbol;
				for (symbol = separator; (*symbol) && (string < buffer); symbol++)
				{
					*string++ = *symbol;
				}
				for (symbol = *operands; (*symbol) && (string < buffer); symbol++)
				{
					*string++ = *symbol;
				}
				separator = operator;
			}
			flagword >>= 1;
			operands++;
		}
		*string = (char) (0);
		buffer -= length;
	}
	return (string - (char *)(buffer));
}


#endif

