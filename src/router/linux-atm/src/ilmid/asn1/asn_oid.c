/*
 * asn_oid.c - BER encode, decode, print and free routines for the 
 *             ASN.1 OBJECT IDENTIFIER type.
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
#include <stdlib.h>
#include <string.h>

#include "asn_config.h"
#include "asn_len.h"
#include "asn_tag.h"
#include "asn_octs.h"
#include "asn_oid.h"


/*
 * encodes universal TAG LENGTH and Contents of and ASN.1 OBJECT ID
 */
AsnLen
BEncAsnOid PARAMS((b, data),
BUF_TYPE     b _AND_
AsnOid* data)
{
    AsnLen len;

    len =  BEncAsnOidContent(b, data);
    len += BEncDefLen(b, len);
    len += BEncTag1(b, UNIV, PRIM, OID_TAG_CODE);
    return(len);
}  /* BEncAsnOid */


/* 
 * decodes universal TAG LENGTH and Contents of and ASN.1 OBJECT ID
 */
void
BDecAsnOid PARAMS((b, result, bytesDecoded, env),
BUF_TYPE   b _AND_
AsnOid*    result _AND_
AsnLen* bytesDecoded _AND_
jmp_buf env)
{
    AsnTag tag;
    AsnLen elmtLen;

    if ( (tag =BDecTag(b, bytesDecoded, env)) !=
        MAKE_TAG_ID(UNIV, PRIM, INTEGER_TAG_CODE))
    {
         Asn1Error("BDecAsnOid: ERROR - wrong tag on OBJECT IDENTIFIER.\n");
         longjmp(env, -40);
    }

    elmtLen = BDecLen (b, bytesDecoded, env);
    BDecAsnOidContent( b, tag, elmtLen, result, bytesDecoded, env);

}  /* BDecAsnOid */



/*
 * Decodes just the content of the OID.
 * AsnOid is handled the same as a primtive octet string
 */
void
BDecAsnOidContent PARAMS((b, tagId, len, result, bytesDecoded, env),
BUF_TYPE b _AND_
AsnTag tagId _AND_
AsnLen len _AND_
AsnOid* result _AND_
AsnLen* bytesDecoded _AND_
jmp_buf env)
{
    result->octetLen = len;
    result->octs =  Asn1Alloc(len);
    BufCopy(result->octs, b, len);
    if (BufReadError(b))
    {
         Asn1Error("BDecAsnOidContent: ERROR - decoded past end of data\n");
         longjmp(env, -21);
    }
    (*bytesDecoded) += len;
}  /* BDecAsnOidContent */



/*
 * Prints the given OID to the given FILE* in ASN.1 Value Notation.
 * Since the internal rep of an OID is 'encoded', this routine 
 * decodes each individual arc number to print it.
 */
void
PrintAsnOid PARAMS((f,v, indent),
FILE* f _AND_
AsnOid* v _AND_
unsigned short int indent)
{
    unsigned short int firstArcNum;
    unsigned long int arcNum;
    int i;

    fprintf(f,"{");

    /* un-munge first two arc numbers */
    for (arcNum = 0, i=0; (i < v->octetLen) && (v->octs[i] & 0x80);i++)
        arcNum = (arcNum << 7) + (v->octs[i] & 0x7f);

    arcNum = (arcNum << 7) + (v->octs[i] & 0x7f);
    i++;
    firstArcNum = arcNum/40;
    if (firstArcNum > 2)
        firstArcNum = 2;

    fprintf(f,"%d %lu", firstArcNum, arcNum - (firstArcNum * 40));

    for (; i < v->octetLen ; )
    {
        for (arcNum = 0; (i < v->octetLen) && (v->octs[i] & 0x80);i++)
            arcNum = (arcNum << 7) + (v->octs[i] & 0x7f);

        arcNum = (arcNum << 7) + (v->octs[i] & 0x7f);
        i++;
        fprintf(f," %lu", arcNum);
    }
    fprintf(f,"}");

} /* PrintAsnOid */





/*
 * given an OID, figures out the length for the encoded version
 */
AsnLen
EncodedOidLen PARAMS((oid),
OID* oid)
{
    AsnLen totalLen;
    unsigned long headArcNum;
    unsigned long tmpArcNum;
    OID* tmpOid;

    /*
     * oid must have at least 2 elmts
     */
    if (oid->next == NULL)
       return 0;

    headArcNum = (oid->arcNum * 40) + oid->next->arcNum;
    
    /*
     * figure out total encoded length of oid 
     */
    tmpArcNum = headArcNum;
    for ( totalLen = 1; (tmpArcNum >>= 7) != 0; totalLen++);
    for (tmpOid = oid->next->next; tmpOid != NULL; tmpOid = tmpOid->next)
    {
        totalLen++;
        tmpArcNum = tmpOid->arcNum;
        for (; (tmpArcNum >>= 7) != 0; totalLen++);
    }

    return (totalLen);

}  /* EncodedOidLen */


/*
 * given an oid list and a pre-allocated ENC_OID
 * (use EncodedOidLen to figure out byte length needed)
 * fills the ENC_OID with a BER encoded version
 * of the oid.
 */
void
BuildEncodedOid PARAMS( (oid, result),
OID* oid _AND_
AsnOid* result)
{
    unsigned long len;
    unsigned long headArcNum;
    unsigned long tmpArcNum;
    char*         buf;
    int           i;
    OID*          tmpOid;

    buf = result->octs;

    /*
     * oid must have at least 2 elmts
     */
    if (oid->next == NULL)
       return;
    /*
     * munge together first two arcNum
     * note first arcnum must be <= 2 
     * and second must be < 39 if first = 0 or 1
     * see (X.209) for ref to this stupidity
     */
    headArcNum = (oid->arcNum * 40) + oid->next->arcNum;

    tmpArcNum = headArcNum;

    /*
     * calc # bytes needed for head arc num
     */
    for (len = 0; (tmpArcNum >>= 7) != 0; len++);

    /*
     * write more signifcant bytes (if any) of head arc num
     * with 'more' bit set 
     */
    for (i=0 ; i < len; i++)
        *(buf++) = 0x80 | (headArcNum >> ((len-i)*7));

    /*
     * write least significant byte of head arc num
     */
    *(buf++) = 0x7f & headArcNum;


    /*
     * write following arc nums, if any 
     */
    for (tmpOid = oid->next->next; tmpOid != NULL; tmpOid = tmpOid->next)
    {    
        /*
         * figure out encoded length -1 of this arcNum
         */
        tmpArcNum = tmpOid->arcNum;
        for (len = 0; (tmpArcNum >>= 7) != 0; len++);
        
        
        /*
         * write more signifcant bytes (if any)
         * with 'more' bit set 
         */
        for (i=0 ; i < len; i++)
            *(buf++) = 0x80 | (tmpOid->arcNum >> ((len-i)*7));

        /*
         * write least significant byte
         */
        *(buf++) = 0x7f & tmpOid->arcNum;
    }
    
} /* BuildEncodedOid */


/*
 * convert an AsnOid into an OID (linked list)
 * NOT RECOMMENDED for use in protocol implementations
 */
void
UnbuildEncodedOid PARAMS( (eoid, result),
AsnOid* eoid _AND_
OID** result)
{
    OID** nextOid;
    OID* headOid;
    int arcNum;
    int i;
    int firstArcNum;
    int secondArcNum;
    
    for (arcNum = 0, i=0; (i < eoid->octetLen) && (eoid->octs[i] & 0x80);i++)
        arcNum = (arcNum << 7) + (eoid->octs[i] & 0x7f);

    arcNum = (arcNum << 7) + (eoid->octs[i] & 0x7f);
    i++;

    firstArcNum = arcNum / 40;
    if (firstArcNum > 2 )
        firstArcNum = 2;

    secondArcNum = arcNum - (firstArcNum * 40);

    headOid = (OID*)malloc(sizeof(OID));
    headOid->arcNum = firstArcNum;
    headOid->next = (OID*)malloc(sizeof(OID));
    headOid->next->arcNum = secondArcNum;
    nextOid = &headOid->next->next;
    
    for ( ; i < eoid->octetLen; )
    {
        for (arcNum = 0 ; (i < eoid->octetLen) && (eoid->octs[i] & 0x80);i++)
            arcNum = (arcNum << 7) + (eoid->octs[i] & 0x7f);

        arcNum = (arcNum << 7) + (eoid->octs[i] & 0x7f);
        i++;
        *nextOid = (OID*)malloc(sizeof(OID));
        (*nextOid)->arcNum = arcNum;
        nextOid = &(*nextOid)->next;
    }

    *result = headOid;

} /* UnbuildEncodedOid */
