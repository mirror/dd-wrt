/*
 * asn_int.h
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

#ifndef _asn_int_h_
#define _asn_int_h_

typedef long int AsnInt;
typedef unsigned long int UAsnInt;



AsnLen BEncAsnInt PROTO((BUF_TYPE b, AsnInt* data));

void BDecAsnInt PROTO((BUF_TYPE b, AsnInt* result, AsnLen* bytesDecoded,
                       ENV_TYPE env));

AsnLen BEncAsnIntContent PROTO((BUF_TYPE b, AsnInt* data));

void BDecAsnIntContent PROTO((BUF_TYPE b,
                       AsnTag tag,
                       AsnLen elmtLen, 
                       AsnInt*  result,
                       AsnLen* bytesDecoded,
                       ENV_TYPE env));

/* do nothing  */
#define FreeAsnInt(v)

void PrintAsnInt PROTO((FILE* f, AsnInt* v, unsigned short int indent));




AsnLen BEncUAsnInt PROTO((BUF_TYPE b, UAsnInt* data));

void BDecUAsnInt PROTO((BUF_TYPE b, UAsnInt* result, AsnLen* bytesDecoded,
                       ENV_TYPE env));

AsnLen BEncUAsnIntContent PROTO((BUF_TYPE b, UAsnInt* data));

void BDecUAsnIntContent PROTO((BUF_TYPE b,
                             AsnTag tagId,
                             AsnLen len,
                             UAsnInt* result,
                             AsnLen* bytesDecoded,
                             ENV_TYPE env));

/* do nothing  */
#define FreeUAsnInt(v) 

void PrintUAsnInt PROTO((FILE* f, UAsnInt* v, unsigned short int indent));


#endif /* conditional include */
