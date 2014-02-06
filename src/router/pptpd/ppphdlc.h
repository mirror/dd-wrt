/*
 * ppphdlc.h
 *
 * Copied from C. S. Ananians ppp_fcs.h
 */

#ifndef _PPTPD_PPPHDLC_H
#define _PPTPD_PPPHDLC_H

#define PPPINITFCS16    0xffff  /* Initial FCS value */
#define PPPGOODFCS16    0xf0b8  /* Good final FCS value */

extern u_int16_t fcstab[256];

#endif  /* !_PPTPD_PPPHDLC_H */
