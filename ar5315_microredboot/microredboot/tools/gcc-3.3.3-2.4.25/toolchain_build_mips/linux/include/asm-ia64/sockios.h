#ifndef _ASM_IA64_SOCKIOS_H
#define _ASM_IA64_SOCKIOS_H

/*
 * Socket-level I/O control calls.  This mostly mirrors the Linux/x86
 * version.
 *
 * Copyright (C) 1998, 1999 Hewlett-Packard Co
 * Copyright (C) 1998, 1999 David Mosberger-Tang <davidm@hpl.hp.com>
 */
#define FIOSETOWN 	0x8901
#define SIOCSPGRP	0x8902
#define FIOGETOWN	0x8903
#define SIOCGPGRP	0x8904
#define SIOCATMARK	0x8905
#define SIOCGSTAMP	0x8906		/* Get stamp */

#endif /* _ASM_IA64_SOCKIOS_H */
