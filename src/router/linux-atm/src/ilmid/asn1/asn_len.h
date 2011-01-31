/*
 * asn_len.h
 *
 * Warning: many of these routines are MACROs for performance reasons
 *          - be carful where you use them.  Don't use more than one per
 *          assignment statement -
 *          (eg itemLen += BEncEoc(b) + BEncFoo(b) ..; this 
 *           will break the code)
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


#ifndef _asn_len_h_
#define _asn_len_h_

typedef unsigned long int AsnLen;

/* max unsigned value  - used for internal rep of indef len */
#define INDEFINITE_LEN  ~0L 


#ifdef USE_INDEF_LEN

#define BEncEocIfNec(b) BEncEoc(b); 

/*
 * include len for EOC  (2 must be first due to BEncIndefLen 
 * - ack! ugly macros!)
 */
#define BEncConsLen(b, len) 2 + BEncIndefLen(b)


#else  /* use definite length - faster?/smaller encodings */


/* do nothing since only using definite lens */
#define BEncEocIfNec(b)  

#define BEncConsLen(b, len) BEncDefLen(b, len)


#endif 



/*
 * writes indefinite length byte to buffer. 'returns' encoded len (1)
 */
#define BEncIndefLen( b)\
    1;\
    BufPutByteRvs( b, 0x80);


#define BEncEoc( b)\
    2;\
    BufPutByteRvs(b, 0);\
    BufPutByteRvs(b, 0);


/*
 * use if you know the encoded length will be 0 >= len <= 127
 * Eg for booleans, nulls, any resonable integers and reals
 *
 * NOTE: this particular Encode Routine does NOT return the length
 * encoded (1).
 */
#define BEncDefLenTo127(b, len)\
    BufPutByteRvs( b, (unsigned char) len)

#define BDEC_2ND_EOC_OCTET(b, bytesDecoded, env)\
{\
    if ((BufGetByte(b) != 0) || BufReadError(b)) {\
        Asn1Error("ERROR - second octet of EOC not zero\n");\
        longjmp(env, -28);}\
     (*bytesDecoded)++;\
}


AsnLen BEncDefLen PROTO((BUF_TYPE  b, AsnLen len));

AsnLen BDecLen PROTO((BUF_TYPE b, AsnLen*  bytesDecoded, ENV_TYPE env));

/*
AsnLen BEncEoc PROTO((BUF_TYPE b));
*/
void BDecEoc PROTO((BUF_TYPE b, AsnLen* bytesDecoded, ENV_TYPE env));


#endif /* conditional include */
