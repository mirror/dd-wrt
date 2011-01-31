/*
 * asn_null.c - BER encode, decode, print and free routines for the 
 *              ASN.1 NULL type.
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

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include "asn_config.h"
#include "asn_len.h"
#include "asn_tag.h"
#include "asn_null.h"



/*
 * encodes universal TAG LENGTH and Contents of and ASN.1 NULL
 */
AsnLen
BEncAsnNull PARAMS((b, data),
BUF_TYPE     b _AND_
AsnNull* data)
{
    AsnLen len;

    len =  BEncAsnNullContent(b, data);
    len += BEncDefLen(b, len);
    len += BEncTag1(b, UNIV, PRIM, NULLTYPE_TAG_CODE);
    return(len);
}  /* BEncAsnNull */


/* 
 * decodes universal TAG LENGTH and Contents of and ASN.1 NULL
 */
void
BDecAsnNull PARAMS((b, result, bytesDecoded, env),
BUF_TYPE b _AND_
AsnNull* result _AND_
AsnLen* bytesDecoded _AND_
jmp_buf env)
{
    AsnTag tag;
    AsnLen elmtLen;

    if ( (tag =BDecTag(b, bytesDecoded, env)) !=
        MAKE_TAG_ID(UNIV, PRIM, NULLTYPE_TAG_CODE))
    {
         Asn1Error("BDecAsnNull: ERROR wrong tag on NULL.\n");
         longjmp(env, -40);
    }

    elmtLen = BDecLen (b, bytesDecoded, env);
    BDecAsnNullContent( b, tag, elmtLen, result, bytesDecoded, env);

}  /* BDecAsnNull */



void
BDecAsnNullContent PARAMS((b, tagId, len, result, bytesDecoded, env),
BUF_TYPE b _AND_
AsnTag tagId _AND_
AsnLen len _AND_
AsnNull* result _AND_
AsnLen* bytesDecoded _AND_
jmp_buf env)
{
    if ( len != 0)
    {
        Asn1Error("BDecAsnNullContent: ERROR - NULL type's len must be 0\n");
        longjmp(env, -17);
    }
}  /* BDecAsnNullContent */

/*
 * Prints the NULL value to the given FILE* in Value Notation. 
 * ignores the indent.
 */
void
PrintAsnNull PARAMS((f,v, indent),
FILE* f _AND_
AsnNull* v _AND_
unsigned short int indent)
{
    fprintf(f,"NULL");
}
