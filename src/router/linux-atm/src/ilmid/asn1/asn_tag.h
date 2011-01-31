/*
 * asn_tag.h
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


#ifndef _asn_tag_h_
#define _asn_tag_h_

typedef unsigned long int AsnTag;

/*
 * The MAKE_TAG_ID macro generates the TAG_ID rep for the
 * the given class/form/code (rep'd in integer for)
 * if the class/form/code are constants the the compiler (should)
 * calculate the tag completely --> zero runtime overhead.
 * This is good for efficiently comparing tags in switch statements
 * (decoding) etc.  because run-time bit fiddling (eliminated) minimized
 */

#define TB sizeof(AsnTag)   /* Tag Id's byte len */
#define UL unsigned long

#define MAKE_TAG_ID( cl, fm, cd)\
( (((UL)(cl)) << ((TB -1) * 8)) | ( ((UL)(fm)) << ((TB -1) * 8)) | (MAKE_TAG_ID_CODE(((UL)(cd)))))

#define MAKE_TAG_ID_CODE(cd)\
( (cd < 31) ?  (MAKE_TAG_ID_CODE1(cd)):\
      ((cd < 128)?  (MAKE_TAG_ID_CODE2(cd)):\
         ((cd < 16384)?  (MAKE_TAG_ID_CODE3(cd)):\
           (MAKE_TAG_ID_CODE4(cd)))))

#define MAKE_TAG_ID_CODE1(cd)  (cd << ((TB -1) * 8))
#define MAKE_TAG_ID_CODE2(cd)  ((31 << ((TB -1) * 8)) | (cd << ((TB-2) * 8)))
#define MAKE_TAG_ID_CODE3(cd)  ((31 << ((TB -1) * 8))\
                                | ((cd & 0x3f80) << 9)\
                                | ( 0x0080 << ((TB-2) * 8))\
                                | ((cd & 0x007F) << ((TB-3)* 8)) )

#define MAKE_TAG_ID_CODE4(cd)  ((31 << ((TB -1) * 8))\
                                | ((cd & 0x1fc000) << 2)\
                                | ( 0x0080 << ((TB-2) * 8))\
                                | ((cd & 0x3f80) << 1)\
                                | ( 0x0080 << ((TB-3) * 8))\
                                | ((cd & 0x007F) << ((TB-4)*8)) )

       
    
typedef enum
{
    ANY_CLASS = -2,
    NULL_CLASS = -1,
    UNIV = 0,
    APPL = (1 << 6),
    CNTX = (2 << 6),
    PRIV = (3 << 6)
} BER_CLASS;

#ifdef notdef
typedef enum
{
    ANY_FORM = -2,
    NULL_FORM = -1,
    PRIM = 0,
    CONS = (1 << 5)
} BER_FORM;
#else
#define ANY_FORM       -2
#define NULL_FORM      -1
#define PRIM           0
#define CONS           (1L << 5)
#endif


typedef enum
{
    NO_TAG_CODE = 0,
    BOOLEAN_TAG_CODE = 1,
    INTEGER_TAG_CODE,
    BITSTRING_TAG_CODE,
    OCTETSTRING_TAG_CODE,
    NULLTYPE_TAG_CODE,
    OID_TAG_CODE,
    OD_TAG_CODE,
    EXTERNAL_TAG_CODE,
    REAL_TAG_CODE,
    ENUM_TAG_CODE,
    SEQ_TAG_CODE =  16,
    SET_TAG_CODE,
    NUMERICSTRING_TAG_CODE,
    PRINTABLESTRING_TAG_CODE,
    TELETEXSTRING_TAG_CODE,
    VIDEOTEXSTRING_TAG_CODE,
    IA5STRING_TAG_CODE,
    UTCTIME_TAG_CODE,
    GENERALIZEDTIME_TAG_CODE,
    GRAPHICSTRING_TAG_CODE,
    VISIBLESTRING_TAG_CODE,
    GENERALSTRING_TAG_CODE
} BER_UNIV_CODE;

#define TT61STRING_TAG_CODE TELETEXSTRING_TAG_CODE
#define ISO646STRING_TAG_CODE VISIBLESTRING_TAG_CODE


/*
 * the TAG_ID_[CLASS/FORM/CODE] macros are not
 * super fast - try not to use during encoding/decoding
 */
#define TAG_ID_CLASS( tid) ( (tid & (0xC0 << ((TB-1) *8))) >> ((TB -1) * 8))
#define TAG_ID_FORM( tid)  ( (tid & (0x20 << ((TB-1) *8))) >> ((TB -1) * 8))

/*
 * TAG_IS_CONS evaluates to true if the given AsnTag type
 * tag has the constructed bit set.
 */
#define TAG_IS_CONS(tag) ((tag) & (CONS << ((TB-1) *8)))


/* not a valid tag - usually the first EOC octet */
#define EOC_TAG_ID 0



/*
 * tag encoders.  given constant values for class form &
 * code in the  source, these can be optimized by the compiler
 * (e.g.  do the shifts and bitwise ands & ors etc)
 *
 * This is the prototype that the following BEncTag routines
 * would use if they were routines.  They return the number of 
 * octets written to the buffer.
 *
 * 
 *AsnLen BEncTag PROTO((BUF_TYPE b, BER_CLASS class, BER_FORM form, int code));
 *
 * WARNING: these are FRAGILE macros (What people will do for performance!)
 *          Be careful of situations like:
 *            if (foo)
 *                  encLen += BEncTag1(...);
 *          Use {}'s to enclose any ASN.1 related routine that you are
 *          treating as a single statement in your code.
 */
#define BEncTag1( b, class, form, code)\
    1;\
    BufPutByteRvs( b, (class) | (form) | (code));

#define BEncTag2( b, class, form, code)\
    2;\
    BufPutByteRvs( b, code);\
    BufPutByteRvs( b, (class) | (form) | 31);

#define BEncTag3( b, class, form, code)\
    3;\
    BufPutByteRvs( b, (code) & 0x7F);\
    BufPutByteRvs( b, 0x80 | ((code) >> 7));\
    BufPutByteRvs( b, (class) | (form) | 31);

#define BEncTag4( b, class, form, code)\
    4;\
    BufPutByteRvs( b, (code) & 0x7F);\
    BufPutByteRvs( b, 0x80 | ((code) >> 7));\
    BufPutByteRvs( b, 0x80 | ((code) >> 14));\
    BufPutByteRvs( b, (class) | (form) | 31);

#define BEncTag5( b, class, form, code)\
    5;\
    BufPutByteRvs( b, (code) & 0x7F);\
    BufPutByteRvs( b, 0x80 | ((code) >> 7));\
    BufPutByteRvs( b, 0x80 | ((code) >> 14));\
    BufPutByteRvs( b, 0x80 | ((code) >> 21));\
    BufPutByteRvs( b, (class) | (form) | 31);


/* the following are protos for routines ins asn_tag.c */


AsnTag BDecTag PROTO((BUF_TYPE  b, AsnLen*   bytesDecoded, ENV_TYPE env));

#endif /* conditional include */
