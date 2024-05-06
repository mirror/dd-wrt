#include "usr/log.h"
#include <stdarg.h>
#include <stdio.h>

void pr_err(const char *format, ...)
{
	va_list args;

	fprintf(stderr, "Error: ");

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);

	fprintf(stderr, "\n");
}
