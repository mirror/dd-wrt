/*
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */

#include <stdio.h>
#include "IxNpeMicrocode.c"


main (int argc, char *argv[])
{
    FILE* ixMicrocodeBinary;
    unsigned int testWord = 0xAABBCCDD;
    unsigned int arrayElements = sizeof(IxNpeMicrocode_array) / sizeof(unsigned int);
    unsigned int x, convertedValue;
    unsigned int IxNpeMicrocode_bigEndianArray[arrayElements];

    /* Endianness test - write a word to RAM and read back as a byte 
                         if byte is the LSB then host is little endian */
    if ((testWord & 0x000000FF) == (unsigned int) (*((unsigned char *) (&testWord))))
    {
	for (x=0; x<arrayElements; x++)
	{
	    /* Convert to format suitable for reading as 32-bit unsigned int by big endian processor */
	    convertedValue = (((IxNpeMicrocode_array[x] & 0xFF000000) >> 24) |
			      ((IxNpeMicrocode_array[x] & 0x00FF0000) >> 8)  |
			      ((IxNpeMicrocode_array[x] & 0x0000FF00) << 8)  |
			      ((IxNpeMicrocode_array[x] & 0x000000FF) << 24));
	    IxNpeMicrocode_bigEndianArray[x] = convertedValue;
	}
    }	
    /* Check to make sure host really is big-endian */
    else if (((testWord & 0xFF000000) >> 24) != (unsigned int) (*((unsigned char *) (&testWord))))
    {
	printf ("ERROR:  Cannot determine host endianness\n");
	return 1;
    }
    else  /* Copy array directly */
    {
	for (x=0; x<arrayElements; x++)
	{
	    IxNpeMicrocode_bigEndianArray[x] = IxNpeMicrocode_array[x];
	}
    }

    /* Write array to file in binary format */
    if ((ixMicrocodeBinary = fopen ("IxNpeMicrocode.dat", "w+b")) == NULL)
    {
	printf ("ERROR:  Could not open file for writing\n");
	return 1;
    }
    fwrite (IxNpeMicrocode_bigEndianArray, sizeof(IxNpeMicrocode_bigEndianArray), 1, ixMicrocodeBinary);
    fclose (ixMicrocodeBinary);
    return 0;
}
