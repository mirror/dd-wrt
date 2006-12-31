/* op.h - message processor opcodes */
 
/* Written 1995,1996 by Werner Almesberger, EPFL-LRC */
 

#ifndef OP_H
#define OP_H

/* construction */

#define OP_INVALID	0 /* crash here */
#define OP_COPY		1 /* copy <jump>,<src>,<size> */
#define OP_COPYVAR	2 /* copy <index>,<src>,<maxsize(bytes)> */
#define OP_BEGIN_LEN	3 /* begin_length <jump>,<src>,<size> */
#define OP_END_LEN	4 /* end_length */
#if 0
#define OP_CASE		4 /* case <groups>, { <group>,<offset>,... { <op>,...,
			     jump }, ... } */
#endif
#define OP_JUMP		5 /* jump <addr> */
#define OP_END		6 /* end */
#define OP_IFGROUP	7 /* ifgroup <group>,<jump> */

/* parsing */

#define OP_MULTI	8 /* multi <size>,case,... */
#define OP_CASE		9 /* case <jump>,<src>,<size>,<groups>, { <pattern>,
			     <group>,<offset>,... { <op>,...,jump }, ... } */
#define OP_IFEND       10 /* ifend <addr> */
#define OP_DUMP	       11 /* dump <index>; dumper only */
#define OP_BEGIN_REC   12 /* begin_recovery <id>,<group>,<addr> */
#define OP_END_REC     13 /* end_recovery */
#define OP_ABORT       14 /* abort <id> */

#endif
