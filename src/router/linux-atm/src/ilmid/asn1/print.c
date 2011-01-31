/*
 * print.c - library routines for printing ASN.1 values.
 *
 * Copyright (C) 1992 Michael Sample and the University of British Columbia
 *
 * This library is free software; you can redistribute it and/or
 * modify it provided that this copyright/license information is retained
 * in original form.
 *
 * If you modify this file, you must clearly indicate your changes.
 *
 * This source code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "asn_config.h"
#include "print.h"

unsigned short int stdIndentG = 4;


void
Indent PARAMS((f, i),
FILE* f _AND_
unsigned short int i)
{
    for( ; i > 0; i--)
        fputc(' ', f);  /* this may be slow */
}


