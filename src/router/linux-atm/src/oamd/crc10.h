/* 
 *	crc10.h	CRC10
 *
 *	Written 2003 by Jorge Boncompte, DTI2
 *	
 *	The CRC10 table and sum routine are from the f4loopbackd.c source
*/

#ifndef CRC10_H
#define CRC10_H

#define POLYNOMIAL 0x633

unsigned short crc10(unsigned char *payload);
int crc10_check(unsigned char *payload);

#endif
