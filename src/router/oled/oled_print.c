/* $Id: oled_test.c 19454 2008-05-06 06:48:10Z saulius $ */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "oled.h"

// ------------------------------------------------------------
int main(int argc, char *argv[])
{ 
	int i;

	oled_port_prepare();
	oled_reset();
	oled_init();
	oled_clear();
	oled_flip_horizontal(0);
	oled_flip_vertical(0);
	oled_brightness(0xFF);

	if (argc < 2) {
		fprintf(stderr, "Usage: %s \"string1\" \"string2\"..\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	for (i = 1; i < argc; i++) {
		oled_print2(0, 2 * (i - 1), argv[i]);
	}

	oled_close();	
	exit(EXIT_SUCCESS);
}

