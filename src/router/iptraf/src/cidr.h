
/*
 * cidr.h - prototypes for cidr.c
 *
 * Copyright (c) Gerard Paul Java 2003
 *
 * This module contains functions that deal with CIDR address/mask notation.
 *
 * This module may be freely used for any purpose, commercial or otherwise,
 * In any product that uses this module, the following notice must appear:
 *
 *     Includes software developed by Gerard Paul Java
 *     Copyright (c) Gerard Paul Java 2003
 */

unsigned long cidr_get_mask(unsigned int maskbits);
char *cidr_get_quad_mask(unsigned int maskbits);
unsigned int cidr_get_maskbits(unsigned long mask);
void cidr_split_address(char *cidr_addr, char *addresspart,
                        unsigned int *maskbits);
