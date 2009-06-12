#ifndef CYGONCE_HAL_PPC_FADS_QUICC_SMC2_H
#define CYGONCE_HAL_PPC_FADS_QUICC_SMC2_H
//=============================================================================
//####UNSUPPORTEDBEGIN####
//
// -------------------------------------------
// This source file has been contributed to eCos/Red Hat. It may have been
// changed slightly to provide an interface consistent with those of other 
// files.
//
// The functionality and contents of this file is supplied "AS IS"
// without any form of support and will not necessarily be kept up
// to date by Red Hat.
//
// The style of programming used in this file may not comply with the
// eCos programming guidelines. Please do not use as a base for other
// files.
//
// All inquiries about this file, or the functionality provided by it,
// should be directed to the 'ecos-discuss' mailing list (see
// http://sourceware.cygnus.com/ecos/intouch.html for details).
//
// Contributed by: Kevin Hester <khester@opticworks.com>
// Maintained by:  <Unmaintained>
// See also:
//   Motorola's "Example Software Initializing the SMC as a UART" package 
//   (smc2.zip) at:
//    http://www.mot.com/SPS/RISC/netcomm/tools/index.html#MPC860_table
// -------------------------------------------
//
//####UNSUPPORTEDEND####
//=============================================================================

extern void  cyg_smc2_init(unsigned long baudRate);
extern void  cyg_smc2_putchar(char);
extern char  cyg_smc2_getchar(void);

#endif /* CYGONCE_HAL_PPC_FADS_QUICC_SMC2_H */
