/*
 * nibble_alloc.h - handles buffer allocation
 * MS 91
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
 */

#ifndef _nibble_alloc_h_
#define _nibble_alloc_h_

typedef struct NibbleBuf
{
    char* start;
    char* end;
    char* curr;
    struct NibbleBuf* next;
} NibbleBuf;


typedef struct NibbleMem
{
    NibbleBuf* firstNibbleBuf;
    NibbleBuf* currNibbleBuf;
    unsigned long int incrementSize;
} NibbleMem;



void InitNibbleMem PROTO((unsigned long int initialSize,
                          unsigned long int incrementSize));

void ShutdownNibbleMem();

void ServiceNibbleFault PROTO((unsigned long int size));

void* NibbleAlloc PROTO((unsigned long int size));

void ResetNibbleMem();


#endif /* conditional include */
