/*
 * asn_len.c - BER encode, decode and utilities for ASN.1 lengths.
 *
 *   indefinite lens are representd by the highest AsnLen
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


AsnLen
BEncDefLen PARAMS((b, len),
BUF_TYPE b _AND_
AsnLen len)
{
    /*
     * unrolled for efficiency
     * check each possibitlity of the 4 byte integer
     */
    if (len < 128)
    {
        BufPutByteRvs( b, len);
        return (1);
    }
    else if (len < 256)
    {
        BufPutByteRvs( b, len);
        BufPutByteRvs( b, 0x81);
        return(2);
    }
    else if (len < 65536) 
    {
        BufPutByteRvs( b, len);
        BufPutByteRvs( b, len >> 8);
        BufPutByteRvs( b, 0x82);
        return (3);
    }
    else if (len < 16777216)
    {
        BufPutByteRvs( b, len);
        BufPutByteRvs( b, len >> 8);
        BufPutByteRvs( b, len >> 16);
        BufPutByteRvs( b, 0x83);
        return (4);
    }
    else
    {
        BufPutByteRvs( b, len);
        BufPutByteRvs( b, len >> 8);
        BufPutByteRvs( b, len >> 16);
        BufPutByteRvs( b, len >> 24);
        BufPutByteRvs( b, 0x84);
        return (5);
    }
} /*  BEncDefLen */


/*
 * non unrolled version 
 */
AsnLen
BEncDefLen2 PARAMS( (b, len),
BUF_TYPE  b _AND_
long int  len)
{
    int i;
    unsigned long int j;

    if (len < 128)
    {
        BufPutByteRvs(b, len);
        return(1);
    }
    else 
    {
        for (i = 0, j = len; j > 0; j >>= 8, i++)
            BufPutByteRvs(b, j);
        
        BufPutByteRvs(b, 0x80 | i);
        return(i + 1);
    }
    
} /*  BEncDefLen2 */



/*
 * decodes and returns an ASN.1 length
 */
AsnLen
BDecLen PARAMS( (b, bytesDecoded, env),
BUF_TYPE b _AND_
unsigned long int*  bytesDecoded _AND_
jmp_buf env)
{
    AsnLen len;
    AsnLen byte;
    int lenBytes;

    byte = (unsigned long int) BufGetByte(b);

    if (BufReadError(b))
    {
        Asn1Error("BDecLen: ERROR - decoded past end of data\n");
        longjmp(env, -13);
    }

    (*bytesDecoded)++;
    if (byte < 128)   /* short length */
        return(byte);

    else if (byte == (AsnLen) 0x080)  /* indef len indicator */
        return(INDEFINITE_LEN);
  
    else  /* long len form */
    {
        /*
         * strip high bit to get # bytes left in len
         */
        lenBytes = byte & (AsnLen) 0x7f;

        if (lenBytes > sizeof(AsnLen))
        {
            Asn1Error("BDecLen: ERROR - length overflow\n");
            longjmp(env, -14);
        }

        (*bytesDecoded) += lenBytes;
               
        for (len = 0; lenBytes > 0; lenBytes--)
            len = (len << 8) | (AsnLen) BufGetByte(b);


        if (BufReadError(b))
        {
            Asn1Error("BDecLen: ERROR - decoded past end of data\n");
            longjmp(env, -15);
        }
        
        return(len);
    }
    /* not reached */
} /* BDecLen */


/* MACRO
AsnLen
BEncEoc PARAMS((b),
BUF_TYPE b)
{
    BufPutByteRvs(b, 0);
    BufPutByteRvs(b, 0);
    return(2);
}   BEncEoc */

/*
 * Decodes an End of Contents (EOC) marker from the given buffer.
 * Flags and error if the octets are non-zero or if a read error
 * occurs.  Increments bytesDecoded by the length of the EOC marker.
 */
void
BDecEoc PARAMS((b, bytesDecoded, env),
BUF_TYPE b _AND_
AsnLen* bytesDecoded _AND_
jmp_buf env)
{
    if ( (BufGetByte(b) != 0) || (BufGetByte(b) != 0) || BufReadError(b))
    {
        Asn1Error("BDecEoc: ERROR - non zero byte in EOC or end of data reached\n");
        longjmp(env, -16);
    }
    (*bytesDecoded) += 2;

}  /* BDecEoc */

