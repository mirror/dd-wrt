/* Rubin encoder/decoder header       */
/* work started at   : aug   3, 1994  */
/* last modification : aug  15, 1994  */
/* $Id: //depot/sw/releases/linuxsrc/src/redboot_cobra/ecos/packages/fs/jffs2/current/src/compr_rubin.h#3 $ */

#include "pushpull.h"

#define RUBIN_REG_SIZE   16
#define UPPER_BIT_RUBIN    (((long) 1)<<(RUBIN_REG_SIZE-1))
#define LOWER_BITS_RUBIN   ((((long) 1)<<(RUBIN_REG_SIZE-1))-1)


struct rubin_state {
	unsigned long p;		
	unsigned long q;	
	unsigned long rec_q;
	long bit_number;
	struct pushpull pp;
	int bit_divider;
	int bits[8];
};
