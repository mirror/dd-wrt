#include "usr/argp/userspace-types.h"

#include <stdarg.h>
#include <stdio.h>

bool show_csv_header(bool no_headers, bool csv)
{
	return !no_headers && csv;
}

bool show_footer(bool no_headers, bool csv)
{
	return !no_headers && !csv;
}

/**
 * Prints something like this: "+---------+--------+". The arguments are the
 * number of dashes (minus 2, intended for padding) for every column.
 *
 * Because of stupid C quirks, you have to surround the arguments with zeroes.
 * For example, to print "+-----+----+---+", you have to call the function like
 * this: `print_table_separator(0, 3, 2, 1, 0);`. Just pretend that they are
 * the table borders or whatever.
 */
void print_table_separator(int junk, ...)
{
	unsigned int cursor;
	unsigned int i;
	va_list args;

	va_start(args, junk);
	printf("+");

	cursor = va_arg(args, unsigned int);
	while (cursor != 0) {
		for (i = 0; i < cursor + 2; i++)
			printf("-");
		printf("+");

		cursor = va_arg(args, unsigned int);
	}

	va_end(args);
	printf("\n");
}
