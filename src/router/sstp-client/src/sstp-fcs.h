/* ppp_fcs.h ... header file for PPP-HDLC FCS 
 *               C. Scott Ananian <cananian@alumni.princeton.edu>
 *
 * $Id: ppp_fcs.h,v 1.2 2008/02/19 05:05:03 quozl Exp $
 */ 
#ifndef __SSTP_FCS_H__
#define __SSTP_FCS_H__


/*< Initial FCS value */
#define PPPINITFCS16        0xffff

/*< Good final FCS value */
#define PPPGOODFCS16        0xf0b8

#define HDLC_FLAG           0x7E
#define HDLC_ESCAPE         0x7D
#define HDLC_TRANSPARENCY   0x20


/*! 
 * @brief Calculate checksum of a frame per RFC1662
 */
uint16_t sstp_frame_check(uint16_t fcs, const unsigned char *cp, int len);


/*!
 * @brief Decode a frame from the buffer and decapsulate it
 */
status_t sstp_frame_decode(const unsigned char *buf, int *length, 
    unsigned char *frame, int *size);


status_t sstp_frame_encode(const unsigned char *source, int ilen, 
        unsigned char *frame, int *flen);

#endif /* #ifndef __SSTP_FCS_H__ */
