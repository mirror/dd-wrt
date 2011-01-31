#ifndef __WAIT_H
#define __WAIT_H
#include "ilmid.h"

void wait_for_prefix( int fd, int itf, Msgs *msgs );
Message *wait_for_sysgroup( int fd, int itf, Msgs *msgs );
Message *wait_for_attachment_point( int fd, int itf, Msgs *msgs );
Message *wait_for_config( int fd, int itf, Msgs *msgs );
Message *wait_for_status( int fd, int itf, Msgs *msgs );
Message *wait_for_setresponse( int fd, int itf, Msgs *msgs );
void reset_apoint( AttPoint *a );

#endif

