/*
 * asn_null.h
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
 *
 */


#ifndef _asn_null_h_
#define _asn_null_h_

typedef char AsnNull;


AsnLen BEncAsnNull PROTO((BUF_TYPE b, AsnNull* data));

void BDecAsnNull PROTO((BUF_TYPE b, AsnNull* result, AsnLen* bytesDecoded,
                       ENV_TYPE env));


/* 'return' length of encoded NULL value, 0 */
#define BEncAsnNullContent(b, data) 0

void BDecAsnNullContent PROTO((BUF_TYPE    b,
                               AsnTag tag,
                               AsnLen len,
                               AsnNull* result,
                               AsnLen* bytesDecoded,
                               ENV_TYPE env));

 /* do nothing */
#define FreeAsnNull(v)

void PrintAsnNull PROTO((FILE* f, AsnNull * b, unsigned short int indent));

#endif /* conditional include */
