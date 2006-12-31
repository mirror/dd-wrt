/*
 * asn_octs.h
 *
 * MS 92
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


#ifndef _asn_octs_h_
#define _asn_octs_h_

typedef struct AsnOcts
{
    unsigned long int octetLen;
    char* octs;
} AsnOcts;

#define ASNOCTS_PRESENT(aocts) ((aocts)->octs != NULL)



AsnLen BEncAsnOcts PROTO((BUF_TYPE b, AsnOcts* data));

void BDecAsnOcts PROTO((BUF_TYPE b, AsnOcts* result, AsnLen* bytesDecoded,
                       ENV_TYPE env));

AsnLen BEncAsnOctsContent PROTO((BUF_TYPE b, AsnOcts* octs));

void BDecAsnOctsContent PROTO((BUF_TYPE b,
                               AsnLen len,
                               AsnTag tagId,
                               AsnOcts* result,
                               AsnLen* bytesDecoded,
                               ENV_TYPE env));

void FreeAsnOcts PROTO((AsnOcts* o));

void PrintAsnOcts PROTO((FILE* f, AsnOcts *o, unsigned short int indent));

int AsnOctsEquiv PROTO((AsnOcts* o1, AsnOcts* o2));


#endif /* conditional include */
