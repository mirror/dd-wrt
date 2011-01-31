#ifndef __STATES_H
#define __STATES_H

#include "message.h"
#include "asn1/asn_incl.h"
#include "ilmid.h"

/* Ilmi states */
#define S1      0
#define S2      1
#define S3      2
#define S4      3
#define S5      4
#define S6      5
#define S7      6
#define S8      7
#define S9      8
#define STATES  9

int state_stopped( int fd, int itf, Msgs *msgs );
int state_failing( int fd, int itf, Msgs *msgs );
int state_establishing( int fd, int itf, Msgs *msgs );
int state_config( int fd, int itf, Msgs *msgs );
int state_retrievePrefixes( int fd, int itf, Msgs *msgs );
int state_registerPrefixes( int fd, int itf, Msgs *msgs );
int state_retrieveAddresses( int fd, int itf, Msgs *msgs );
int state_registerAddresses( int fd, int itf, Msgs *msgs );
int state_verify( int fd, int itf, Msgs *msgs );


#endif

