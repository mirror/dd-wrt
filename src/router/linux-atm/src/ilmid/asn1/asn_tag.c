/*
 * asn_tag.c - BER encode, decode and untility routines for ASN.1 Tags.
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


/*
 * Returns an AsnTag.  An AsnTag is simply an encoded tag
 * shifted to fill up an unsigned long int (first tag byte
 * in most sig byte of long int)
 * This rep permits easy case stmt comparison of tags.
 * NOTE: The unsigned long rep for tag BREAKS if the 
 *       the tag's code is over 2^21 (very unlikely)
 *
 * RETURNS 0 if decoded a 0 byte (ie first byte of an EOC)
 */
AsnTag
BDecTag PARAMS( (b, bytesDecoded, env),
BUF_TYPE  b _AND_
AsnLen* bytesDecoded _AND_
jmp_buf env)
{
    AsnTag tagId;
    AsnTag tmpTagId;
    int i;

    tagId = ((AsnTag)BufGetByte(b)) << ((sizeof(AsnTag)-1)*8);
    (*bytesDecoded)++;

    /* check if long tag format (ie code > 31) */
    if ( (tagId & (((AsnTag) 0x1f) << ((sizeof(AsnTag)-1)*8))) == 
          (((AsnTag)0x1f) << ((sizeof(AsnTag)-1)*8)))
    {
        i = 2;
        do
        {
            tmpTagId = (AsnTag) BufGetByte(b);
            tagId |= (tmpTagId << ((sizeof(AsnTag)-i)*8));
            (*bytesDecoded)++;
            i++;
        }
        while ((tmpTagId & (AsnTag)0x80) && (i < sizeof(AsnTag)));
        
        /*
         * check for tag that is too long 
         */
        if (i > (sizeof(AsnTag)+1))
        {
            Asn1Error("BDecTag: ERROR - tag value overflow\n");
            longjmp(env, -25);
        }
    }

    if (BufReadError(b))
    {
        Asn1Error("BDecTag: ERROR - decoded past the end of data\n");
        longjmp(env, -26);
    }

    return(tagId);

}  /* BDecTag */

