/*
* Copyright (C) 2006-2010 Freescale Semiconductor, Inc. All Rights Reserved.
*/

/*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

////////////////////////////////////////////////////////////////////////////////
//! \addtogroup rom_nand_boot
//! @{
//
//
//! \file rom_nand_hamming_code_ecc.c
//! \brief hamming code ecc functions.
//!
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//   Includes and external references
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "rom_nand_hamming_code_ecc.h"

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief compares three NCBs and return error if no two are identical else
//!        returns the one that matches with at least another copy.
//!
//! a local function
//!
//! This function compares the three NCB structures and their corresponding
//! parity bits. If any two matches then search ends. If none matches then it
//! returns an error.
//!
//! \param[in]  pNCBCopy1  pointer to first NCB structure
//! \param[in]  pNCBCopy2  pointer to second NCB structure
//! \param[in]  pNCBCopy3  pointer to third NCB structure
//! \param[in]  pP1 pointer to first set of Parity bits
//! \param[in]  pP2 pointer to second set of Parity bits
//! \param[in]  pP3 pointer to third set of Parity bits
//! \param[out] pu8HammingCopy to return either 1, 2 or 3 that qualified for
//!             Humming code analysis.
//!
//! \retval    0
//! \retval    -1
//!
////////////////////////////////////////////////////////////////////////////////
int TripleRedundancyCheck(uint8_t * pNCBCopy1, uint8_t * pNCBCopy2,
			  uint8_t * pNCBCopy3, uint8_t * pP1,
			  uint8_t * pP2, uint8_t * pP3,
			  uint8_t * pu8HammingCopy)
{
	int nError;

	// compare 1 and 2 copies of NCB
	nError = 0;
	nError = memcmp(pNCBCopy1, pNCBCopy2, NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES);
	if (nError == 0)
		nError = memcmp(pP1, pP2, NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES);

	if (nError == 0) {
		// 1 and 2 are identical so lets go with 1
		*pu8HammingCopy = 1;
		return 0;
	}
	// compare 1 and 3 copies of NCB
	nError = 0;
	nError = memcmp(pNCBCopy1, pNCBCopy3, NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES);
	if (nError == 0)
		nError = memcmp(pP1, pP3, NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES);

	if (nError == 0) {
		// 1 and 3 are identical so lets go with 1
		*pu8HammingCopy = 1;
		return 0;
	}
	// compare 2 and 3 copies of NCB
	nError = 0;
	nError = memcmp(pNCBCopy2, pNCBCopy3, NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES);
	if (nError == 0)
		nError = memcmp(pP2, pP3, NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES);

	if (nError == 0) {
		// 2 and 3 are identical so lets go with 2
		*pu8HammingCopy = 2;
		return 0;
	}

	return -1;
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief count number of 1s and return true if they occur even number of times
//!        in the given byte.
//!
//! xor all the bits of u8, if even number of 1s in u8, then the result is 0.
//!
//! \param[in]  u8 // input byte
//!
//! \retval    true, if even number of 1s in u8
//! \retval    false, if odd number of 1s in u8
//!
////////////////////////////////////////////////////////////////////////////////
int IsNumOf1sEven(uint8_t u8)
{
	int i, nCountOf1s = 0;

	for (i = 0; i < 8; i++) {
		nCountOf1s ^= ((u8 & (1 << i)) >> i);
	}
	return !nCountOf1s;
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief calculates parity using Hsiao Code and Hamming code
//!
//! \param[in]  d, given 16 bits integer
//! \param[out] p, pointer to uint8_t for parity
//!
//! \retval    none.
//!
////////////////////////////////////////////////////////////////////////////////
void CalculateParity(uint16_t d, uint8_t * p)
{
//  p[0] = d[15] ^ d[12] ^ d[11] ^ d[ 8] ^ d[ 5] ^ d[ 4] ^ d[ 3] ^ d[ 2];
//  p[1] = d[13] ^ d[12] ^ d[11] ^ d[10] ^ d[ 9] ^ d[ 7] ^ d[ 3] ^ d[ 1];
//  p[2] = d[15] ^ d[14] ^ d[13] ^ d[11] ^ d[10] ^ d[ 9] ^ d[ 6] ^ d[ 5];
//  p[3] = d[15] ^ d[14] ^ d[13] ^ d[ 8] ^ d[ 7] ^ d[ 6] ^ d[ 4] ^ d[ 0];
//  p[4] = d[12] ^ d[ 9] ^ d[ 8] ^ d[ 7] ^ d[ 6] ^ d[ 2] ^ d[ 1] ^ d[ 0];
//  p[5] = d[14] ^ d[10] ^ d[ 5] ^ d[ 4] ^ d[ 3] ^ d[ 2] ^ d[ 1] ^ d[ 0];

	uint8_t Bit0 = (d & (1 << 0)) ? 1 : 0;
	uint8_t Bit1 = (d & (1 << 1)) ? 1 : 0;
	uint8_t Bit2 = (d & (1 << 2)) ? 1 : 0;
	uint8_t Bit3 = (d & (1 << 3)) ? 1 : 0;
	uint8_t Bit4 = (d & (1 << 4)) ? 1 : 0;
	uint8_t Bit5 = (d & (1 << 5)) ? 1 : 0;
	uint8_t Bit6 = (d & (1 << 6)) ? 1 : 0;
	uint8_t Bit7 = (d & (1 << 7)) ? 1 : 0;
	uint8_t Bit8 = (d & (1 << 8)) ? 1 : 0;
	uint8_t Bit9 = (d & (1 << 9)) ? 1 : 0;
	uint8_t Bit10 = (d & (1 << 10)) ? 1 : 0;
	uint8_t Bit11 = (d & (1 << 11)) ? 1 : 0;
	uint8_t Bit12 = (d & (1 << 12)) ? 1 : 0;
	uint8_t Bit13 = (d & (1 << 13)) ? 1 : 0;
	uint8_t Bit14 = (d & (1 << 14)) ? 1 : 0;
	uint8_t Bit15 = (d & (1 << 15)) ? 1 : 0;

	*p = 0;

	*p |= ((Bit15 ^ Bit12 ^ Bit11 ^ Bit8 ^ Bit5 ^ Bit4 ^ Bit3 ^ Bit2) << 0);
	*p |=
	    ((Bit13 ^ Bit12 ^ Bit11 ^ Bit10 ^ Bit9 ^ Bit7 ^ Bit3 ^ Bit1) << 1);
	*p |=
	    ((Bit15 ^ Bit14 ^ Bit13 ^ Bit11 ^ Bit10 ^ Bit9 ^ Bit6 ^ Bit5) << 2);
	*p |= ((Bit15 ^ Bit14 ^ Bit13 ^ Bit8 ^ Bit7 ^ Bit6 ^ Bit4 ^ Bit0) << 3);
	*p |= ((Bit12 ^ Bit9 ^ Bit8 ^ Bit7 ^ Bit6 ^ Bit2 ^ Bit1 ^ Bit0) << 4);
	*p |= ((Bit14 ^ Bit10 ^ Bit5 ^ Bit4 ^ Bit3 ^ Bit2 ^ Bit1 ^ Bit0) << 5);
}

////////////////////////////////////////////////////////////////////////////////
//! \brief pre calculated array of syndromes using Hsiao code.
//!
//! The table consists of 22 entries, first 16 entries for each bit of error in
//! 16-bit data and the next 6 entries for 6-bit parity.
//!
//! The logic used to calculate this table is explained in the code below:
//! \code
//!
//! for(j=0; j<22; j++) { // for each error location
//!
//!     // d is 16-bit data and p is 6-bit parity
//!     // initialize received vector
//!     for(i=0;i<22;i++) {
//!         if(i<16)
//!             r[i] = d[i];
//!         else
//!             r[i] = p[i-16];
//!     }
//!     // inject error
//!     r[j]=r[j]^0x1;
//!
//!     // compute syndrome
//!     s[0] = r[16] ^ r[15] ^ r[12] ^ r[11] ^ r[8]  ^ r[5]  ^ r[4] ^ r[3] ^ r[2];
//!     s[1] = r[17] ^ r[13] ^ r[12] ^ r[11] ^ r[10] ^ r[9]  ^ r[7] ^ r[3] ^ r[1];
//!     s[2] = r[18] ^ r[15] ^ r[14] ^ r[13] ^ r[11] ^ r[10] ^ r[9] ^ r[6] ^ r[5];
//!     s[3] = r[19] ^ r[15] ^ r[14] ^ r[13] ^ r[8]  ^ r[7]  ^ r[6] ^ r[4] ^ r[0];
//!     s[4] = r[20] ^ r[12] ^ r[9]  ^ r[8]  ^ r[7]  ^ r[6]  ^ r[2] ^ r[1] ^ r[0];
//!     s[5] = r[21] ^ r[14] ^ r[10] ^ r[5]  ^ r[4]  ^ r[3]  ^ r[2] ^ r[1] ^ r[0];
//! }
//! \endcode
////////////////////////////////////////////////////////////////////////////////
static const uint8_t au8SyndTable[] = {
	0x38, 0x32, 0x31, 0x23, 0x29, 0x25, 0x1C, 0x1A, 0x19, 0x16, 0x26, 0x07,
	0x13, 0x0E, 0x2C, 0x0D, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20
};

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief looks up for a match in syndrome table array.
//!
//! \param[in]  u8Synd given syndrome to match in the table
//! \param[out] pu8BitToFlip pointer to return the index of array that matches
//!             with given syndrome
//!
//! \retval    0 if a match is found
//! \retval    -1 no match found
//!
////////////////////////////////////////////////////////////////////////////////
int TableLookupSingleErrors(uint8_t u8Synd, uint8_t * pu8BitToFlip)
{
	uint8_t i;
	for (i = 0; i < 22; i++) {
		if (au8SyndTable[i] == u8Synd) {
			*pu8BitToFlip = i;
			return 0;
		}
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
//!
//! \brief evaluate NCB block with Hamming Codes
//!
//! This function evaluates NCB Block with Hamming codes and if single bit error
//! occurs then it is fixed, if double error occurs then it returns an error
//!
//! \param[in] pNCB, NCB block
//! \param[in] pParityBlock, block of parity codes, every 6 bits for every 16 bits of
//!            data in NCB block
//!
//! \retval    0, if no error or 1 bit error that is fixed.
//! \retval    ERROR_ROM_NAND_DRIVER_NCB_HAMMING_DOUBLE_ERROR, double error occured
//!            that cannot be fixed.
//!
////////////////////////////////////////////////////////////////////////////////
int HammingCheck(uint8_t * pNCB, uint8_t * pParityBlock)
{
	uint16_t *p16Data = (uint16_t *) pNCB;
	uint8_t P;
	uint8_t NP;
	int i, j = 0;
	int nBitPointer = 0;
	uint8_t u8Syndrome, u8BitToFlip;
	int retStatus = 0;

	P = 0;	// kill warning
	for (i = 0; i < 256; i++) {
		// the problem here is to read 6 bits from an 8-bit byte array for each parity code.
		// The parity code is either present in 6 lsbs, or partial in one byte and remainder
		// in the next byte or it can be 6 msbs. The code below tries to collect all 6 bits
		// into one byte, leaving upper 2 bits 0.
		switch (nBitPointer) {
		case 0:
			// if nBitPointer is 0, that means, we are at the start of a new byte.
			// we can straight away read 6 lower bits from the byte for the parity.
			P = (pParityBlock[j] & 0x3F);
			nBitPointer = 2;
			break;
		case 2:
			// if nBitPointer is 2, that means, we need to read 2 MSb from jth byte of
			//parity block and remaining 4 from next byte.
			P = ((pParityBlock[j] & 0xC0) >> 6);	// read 2 MSbs and moved them to left 6 times
			j++;	//go to next byte
			P |= ((pParityBlock[j] & 0x0F) << 2);	// read 4 from LSb, move it up 2 times so that all 6 bits are aligned in a byte.
			nBitPointer = 4;
			break;
		case 4:
			// if nBitPointer is 4, that means, we need to read 4 MSbs from jth byte of
			//parity block and remaining 2 from next byte.
			P = ((pParityBlock[j] & 0xF0) >> 4);	// read 4 MSbs and moved them to left 4 times
			j++;	//goto the next byte
			P |= ((pParityBlock[j] & 0x03) << 4);	// read 2 LSbs and moved them to right 4 times so that all 6 bits are aligned in a byte
			nBitPointer = 6;
			break;
		case 6:
			// if nBitPointer is 6, that means, we need to read 6 MSbs from jth byte of
			// parity block
			P = ((pParityBlock[j] & 0xFC) >> 2);	// read 6 MSbs and moved them to left 2 times.
			nBitPointer = 0;
			j++;	// go to the next byte
			break;
		};

		// calculate new parity out of 16-bit data
		if ((*p16Data == 0) || (*p16Data == 0xFFFF)) {
			// this is for optimization purpose
			NP = 0;
		} else {
			CalculateParity(*p16Data, &NP);
		}

		// calculate syndrome by XORing parity read from NAND and new parity NP just calculated.
		u8Syndrome = NP ^ P;

		// if syndrome is 0, that means the data is good.
		if (u8Syndrome == 0) {
			// data is good. fetch next 16bits
			p16Data++;
			continue;
		}
		// check for double bit errors, which is the case when we have even number of 1s in the syndrome
		if (IsNumOf1sEven(u8Syndrome)) {
			// found a double error, can't fix it, return
			return -1;
		}

		// this is a single bit error and can be fixed
		retStatus = TableLookupSingleErrors(u8Syndrome, &u8BitToFlip);
		if (retStatus != 0) {
			return retStatus;
		}

		if (u8BitToFlip < 16) {
			// error is in data bit u8BitToFlip, flip that bit to correct it
			*p16Data ^= (0x1 << u8BitToFlip);
		} else {
			// the error is a 1 bit error and is in parity so we do not worry fixing it.

//               u8BitToFlip -= 16;
//
//                // error is in parity
//
//                switch(nBitPointer)
//                {
//                case 0:
//                    // all bits are in j-1 byte (bits 2..7) of parity block.
//                    pParityBlock[j-1] ^= 1 << (u8BitToFlip+2);
//                    break;
//                case 2:
//                    // all bits are in j byte (bits 0..5) of parity block
//                    pParityBlock[j] ^= 1 << (u8BitToFlip);
//                    break;
//                case 4:
//                    // 2 bits are in j-1 byte (bits 6 and 7) of parity block
//                    // 4 bits are in j byte (bits 0..3) of parity block
//                    if( u8BitToFlip < 2 )
//                    {
//                        // either 6 or 7 bit of j-1 byte to flip.
//                        pParityBlock[j-1] ^= 1 << (u8BitToFlip+6);
//                    }
//                    else
//                    {
//                        // either one of 0..3 bits of j byte to flip.
//                        pParityBlock[j] ^= 1 << (u8BitToFlip-2);
//                    }
//                    break;
//                case 6:
//                    if( u8BitToFlip < 4 )
//                    {
//                        // 4 bits are in j-1 byte (bits 4 to 7) of parity block
//                        pParityBlock[j-1] ^= 1 << (u8BitToFlip+4);
//                    }
//                    else
//                    {
//                        // 2 bits are in j byte (bits 0 and 1) of parity block
//                        pParityBlock[j] ^= 1 << (u8BitToFlip-4);
//                    }
//                    break;
//                };
		}

		// fetch next 16bits
		p16Data++;
	}
	return retStatus;
}

// eof rom_nand_hamming_code_ecc.c
//! @}
