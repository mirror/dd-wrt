/*
 * mode.c - a simple function that returns "kbits" or "kbytes" based on a
 *          numeric argument.
 */

#include <string.h>
#include "options.h"

void dispmode(int mode, char *result)
{
    if (mode == KBITS)
        strcpy(result, "kbits");
    else
        strcpy(result, "kbytes");
}
