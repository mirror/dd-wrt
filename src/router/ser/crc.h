/* $Id: crc.h,v 1.3 2003/04/10 12:37:50 janakj Exp $*/

#ifndef _CRC_H_
#define _CRC_H_

#include "str.h"

#define CRC16_LEN	4

extern unsigned long int crc_32_tab[];
extern unsigned short int ccitt_tab[];
extern unsigned short int crc_16_tab[];

unsigned short crcitt_string( char *s, int len );
void crcitt_string_array( char *dst, str src[], int size );

#endif /* _CRC_H_ */

