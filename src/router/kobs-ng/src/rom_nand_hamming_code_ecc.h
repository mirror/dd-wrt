////////////////////////////////////////////////////////////////////////////////
//! \addtogroup rom_nand_boot
//! @{
//
// Copyright (c) 2006 SigmaTel, Inc.
//
//! \file rom_nand_hamming_code_ecc.h
//! \brief This file provides header info for hamming code ecc.
//!
////////////////////////////////////////////////////////////////////////////////
#ifndef _ROM_NAND_HAMMING_CODE_ECC_H
#define _ROM_NAND_HAMMING_CODE_ECC_H

#include <stdint.h>
    
////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
//!< Bytes per NCB data block
#define NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES        (512) 
//! Size of a parity block in bytes for all 16-bit data blocks present inside one 512 byte NCB block.
#define NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES      ((((512*8)/16)*6)/8) 
//! Offset to first copy of NCB in a NAND page
#define NAND_HC_ECC_OFFSET_FIRST_DATA_COPY            (0) 
//! Offset to second copy of NCB in a NAND page
#define NAND_HC_ECC_OFFSET_SECOND_DATA_COPY           (NAND_HC_ECC_OFFSET_FIRST_DATA_COPY+NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES) 
//! Offset to third copy of NCB in a NAND page
#define NAND_HC_ECC_OFFSET_THIRD_DATA_COPY            (NAND_HC_ECC_OFFSET_SECOND_DATA_COPY+NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES)
//! Offset to first copy of Parity block in a NAND page
#define NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY          (NAND_HC_ECC_OFFSET_THIRD_DATA_COPY+NAND_HC_ECC_SIZEOF_DATA_BLOCK_IN_BYTES)
//! Offset to second copy of Parity block in a NAND page
#define NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY         (NAND_HC_ECC_OFFSET_FIRST_PARITY_COPY+NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES)
//! Offset to third copy of Parity block in a NAND page
#define NAND_HC_ECC_OFFSET_THIRD_PARITY_COPY          (NAND_HC_ECC_OFFSET_SECOND_PARITY_COPY+NAND_HC_ECC_SIZEOF_PARITY_BLOCK_IN_BYTES)
    
#define BITMASK_HAMMINGCHECKED_ALL_THREE_COPIES 0x7	//!< to indicate all three copies of NCB in first page are processed with Hamming codes.
#define BITMASK_HAMMINGCHECKED_FIRST_COPY       0x1	//!< to indicate first copy of NCB is processed with Hamming codes.
#define BITMASK_HAMMINGCHECKED_SECOND_COPY      0x2	//!< to indicate second copy of NCB is processed with Hamming codes.
#define BITMASK_HAMMINGCHECKED_THIRD_COPY       0x4	//!< to indicate third copy of NCB is processed with Hamming codes.
int TripleRedundancyCheck(uint8_t * pNCBCopy1, uint8_t * pNCBCopy2,
			   uint8_t * pNCBCopy3,  uint8_t * pP1, uint8_t * pP2,
			   uint8_t * pP3,  uint8_t * pu8HammingCopy);
int IsNumOf1sEven(uint8_t u8);
void CalculateParity(uint16_t d, uint8_t * p);
int TableLookupSingleErrors(uint8_t u8Synd, uint8_t * pu8BitToFlip);
int HammingCheck(uint8_t * pNCB, uint8_t * pParityBlock);

#endif	/*  */
////////////////////////////////////////////////////////////////////////////////
// End of file
////////////////////////////////////////////////////////////////////////////////
// @}
