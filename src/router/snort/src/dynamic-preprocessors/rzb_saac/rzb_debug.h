#ifndef NRT_DEBUG_H
#define NRT_DEBUG_H

#define D_CRITICAL  0xFFFF

#define D_EMERG     0x0001
#define D_WARN      0x0002
#define D_DEBUG     0x0004
#define D_INFO      0x0008
#define D_ALLLVL    0x00FF
#define D_CRIT      0x0080

#define D_CLIENT    0x0100
#define D_SERVER    0X0200
#define D_DETECT    0x0400
#define D_PACKET    0x0800
#define D_FILE      0x1000
#define D_ALERT     0x2000
#define D_ALLCOMP   0xFF00

#define D_ALLDEBUG  0xFFFF

#define DEBUG
#ifdef DEBUG
#define DEBUGLEVEL D_ALLDEBUG //((D_ALLCOMP & ~D_PACKET) | D_CRIT)// (D_ALLDEBUG & ~D_PACKET)
#define DEBUGOUT(flag, code) if((flag & DEBUGLEVEL & 0xFF00) && (flag & DEBUGLEVEL & 0x00FF)) code 
#else
#define DEBUGOUT(flag, code) 
#endif

#define PACKETDUMPSIZE 256

void prettyprint(const unsigned char *, unsigned int);


#endif

