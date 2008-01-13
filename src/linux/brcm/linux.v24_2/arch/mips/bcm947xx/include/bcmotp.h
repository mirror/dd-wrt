/*
 * Write-once support for otp.
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#define OTP_HW_RGN	1
#define OTP_SW_RGN	2
#define OTP_CI_RGN	4
#define OTP_FUSE_RGN	8

extern void *otp_init(sb_t *sbh);

extern uint16 otpr(void *oh, chipcregs_t *cc, uint wn);
extern uint16 otpw(void *oh, chipcregs_t *cc, uint16 data, uint wn);

extern int otp_status(void *oh);
extern int otp_size(void *oh);

extern int otp_read_region(void *oh, int region, uint16 *data, uint wlen);
extern int otp_write_region(void *oh, int region, uint16 *data, uint wlen);

extern int otp_nvread(void *oh, char *data, uint *len);

#ifdef BCMNVRAMW
extern int otp_nvwrite(void *oh, uint16 *data, uint wlen);
#endif

#if defined(WLTEST)
extern int otp_dump(void *oh, int arg, char *buf, uint size);
#endif
