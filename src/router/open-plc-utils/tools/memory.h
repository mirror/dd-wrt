/*====================================================================*
 *
 *   memory.h - memory encode/decode definitions and delcaration;
 *
 *   this file is a subset of the original that includes only those
 *   definitions and declaration needed for toolkit programs;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef MEMORY_HEADER
#define MEMORY_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <netinet/in.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"
#include "../tools/endian.h"

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define IPv4_LEN 4
#define IPv6_LEN 16

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#ifndef offset
#define offset(struct, member) (signed)(&struct.member)-(signed)(&struct)
#endif

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#define HEXSTRING(string, memory) hexstring (string, sizeof (string), memory, sizeof (memory)) 
#define HEXDUMP_HEADER "-------- 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ----------------\n"
#define REGVIEW32_HEADER "REGISTER CONTENTS 31----24 23----16 15----08 07----00\n"
#define REGVIEW16_HEADER "ADDR DATA 5432 1098 7654 3210\n"
#define ADDRSIZE 8

/*====================================================================*
 *   macro expansions;
 *--------------------------------------------------------------------*/

#define NEW(object) (object *)(emalloc(sizeof(object)))
#define STR(length) (char *)(emalloc((length)+1))

/*====================================================================*
 *   memory increment/decrement functions;
 *--------------------------------------------------------------------*/

signed strincr (void * memory, size_t extent, byte min, byte max);
signed strdecr (void * memory, size_t extent, byte min, byte max);
signed memincr (void * memory, size_t extent);
signed memdecr (void * memory, size_t extent);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

bool memseek (void const * memory, size_t extent, void const * buffer, size_t length);

/*====================================================================*
 *   memory allocation functions;
 *--------------------------------------------------------------------*/

void * emalloc (size_t length);

/*====================================================================*
 *   memory movement functions;
 *--------------------------------------------------------------------*/

void memswap (void *, void *, size_t extent);
void reverse (void * memory, size_t extent);

/*====================================================================*
 *   memory validation functions (deprecated);
 *--------------------------------------------------------------------*/

uint32_t checksum_32 (const uint32_t memory [], size_t extent, uint32_t checksum);
uint32_t fdchecksum_32 (int fd, size_t extent, uint32_t checksum);

/*====================================================================*
 *   memory validation functions;
 *--------------------------------------------------------------------*/

uint32_t checksum32 (void const * memory, size_t extent, uint32_t checksum);
uint32_t fdchecksum32 (int fd, size_t extent, uint32_t checksum);

/*====================================================================*
 *   memory encode functions;
 *--------------------------------------------------------------------*/

size_t memencode (void * memory, size_t extent, char const * format, char const * string);
size_t hexencode (void * memory, size_t extent, char const * string);
size_t hexcopy (void * memory, size_t extent, char const * string);
size_t decencode (void * memory, size_t extent, char const * string);
size_t memdecode (void const * memory, size_t extent, char const * format, char const * string);
size_t hexdecode (void const * memory, size_t extent, char buffer [], size_t length);
size_t decdecode (void const * memory, size_t extent, char buffer [], size_t length);
char * hexstring (char buffer [], size_t length, void const * memory, size_t extent);
char * decstring (char buffer [], size_t length, void const * memory, size_t extent);
char * hexoffset (char buffer [], size_t length, off_t offset);
size_t bytespec (char const * string, void * memory, size_t extent);
size_t dataspec (char const * string, void * memory, size_t extent);
size_t ipv4spec (char const * string, void * memory);
size_t ipv6spec (char const * string, void * memory);

/*====================================================================*
 *   memory input functions;
 *--------------------------------------------------------------------*/

size_t hexload (void * memory, size_t extent, FILE *fp);

/*====================================================================*
 *   memory output functions;
 *--------------------------------------------------------------------*/

void hexwrite (signed fd, void const * memory, size_t extent);
void hexsave (void const * memory, size_t extent, size_t column, FILE *fp);
void hexdump (void const * memory, size_t offset, size_t extent, FILE *fp);
void bindump (void const * memory, size_t offset, size_t extent, FILE *fp);
void hexview (void const * memory, size_t offset, size_t extent, FILE *fp);
void hexpeek (void const * memory, size_t origin, size_t offset, size_t extent, size_t window, FILE * fp);
void regview16 (void const * memory, size_t offset, size_t extent, FILE *fp);
void regview32 (void const * memory, size_t offset, size_t extent, FILE *fp);
void hexout (void const * memory, size_t extent, char c, char e, FILE *fp);
void decout (void const * memory, size_t extent, char c, char e, FILE *fp);
void binout (void const * memory, size_t extent, char c, char e, FILE *fp);
void chrout (void const * memory, size_t extent, char c, char e, FILE *fp);
void memout (void const * memory, size_t extent, char const * format, unsigned group, char c, char e, FILE *fp);

/*====================================================================*
 *   end definitions;
 *--------------------------------------------------------------------*/

#endif

