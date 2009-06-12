/*
 * Padraig's pr3930 mips emulator 
 */

#ifndef IMAGELOADER_H_
#define IMAGELOADER_H_

#include "mips_types.h"

/* load a hex mips image into memory from a file */
int load_srec_image(char*,u_word);

#endif
