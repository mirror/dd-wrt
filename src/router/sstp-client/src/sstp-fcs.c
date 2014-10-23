/* Fast Frame Check Sequence (FCS) Implementation, for HDLC-like framing of
 * PPP.  Adapted by C. Scott Ananian <cananian@alumni.princeton.edu>
 * from RFC1662:
 *
 * C.2.  16-bit FCS Computation Method
 *
 *  The following code provides a table lookup computation for
 *  calculating the Frame Check Sequence as data arrives at the
 *  interface.  This implementation is based on [7], [8], and [9].
 *
 *  [7]   Perez, "Byte-wise CRC Calculations", IEEE Micro, June 1983.
 *
 *  [8]   Morse, G., "Calculating CRC's by Bits and Bytes", Byte,
 *        September 1986.
 *
 *  [9]   LeVan, J., "A Fast CRC", Byte, November 1987.
 */

#include <config.h>
#include <string.h>
#include <sys/types.h>
#include "sstp-private.h"


/*
 * FCS lookup table as calculated by the table generator.
 */
static uint16_t fcstab[256] = 
{
   0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
   0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
   0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
   0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
   0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
   0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
   0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
   0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
   0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
   0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
   0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
   0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
   0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
   0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
   0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
   0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
   0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
   0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
   0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
   0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
   0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
   0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
   0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
   0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
   0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
   0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
   0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
   0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
   0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
   0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
   0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
   0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


/*!
 * @brief Calculate a new fcs given the current fcs and the data.
 */
uint16_t sstp_frame_check(uint16_t fcs, const unsigned char *cp, int len)
{
    while (len--)
    {
        fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];
    }

    return (fcs);
}


status_t sstp_frame_decode(const unsigned char *buf, int *length, 
    unsigned char *frame, int *size)
{
    unsigned int index = 0;
    unsigned int pos   = 0;
    unsigned int ret   = 0;

    /* Skip the start of the frame */
    while (buf[index] == HDLC_FLAG)
    {
        index++;
    }

    do
    {
        unsigned int escape = 0;

        /* Incase we encounter escapes */
        if (buf[index] == HDLC_ESCAPE)
        {
            escape = HDLC_TRANSPARENCY;
            index++;
        }
    
        /* Copy character to the output */
        if (pos < *size)
        {
            frame[pos++] = buf[index] ^ escape;
        } 

        /* Received incomplete frame */
        if (index >= *length)
        {
            return SSTP_OVERFLOW;
        }

    } while (buf[++index] != HDLC_FLAG);

    /* Account for the FCS field */
    *size   = (pos - sizeof(uint16_t));
    *length = index;

    /* Skip short packets */
    if (pos < 4)
    {
        return SSTP_FAIL;
    }

    /* Calculate checksum and compare */
    ret = sstp_frame_check(PPPINITFCS16, frame, pos);
    if (PPPGOODFCS16 != ret)    // 0xf0b8
    {
        return SSTP_FAIL;
    }
    
    return SSTP_OKAY;
}


status_t sstp_frame_encode(const unsigned char *source, int ilen, 
        unsigned char *frame, int *flen)
{
    uint16_t fcs = 0;
    int pos = 0;
    int i   = 0;

    fcs  = sstp_frame_check(PPPINITFCS16, source, ilen);
    fcs ^= PPPINITFCS16;

    /* Set the start of frame marker */
    frame[pos++] = HDLC_FLAG;

    /* Escape the payload */
    for (i = 0; i < ilen + 2; i++)
    {
        unsigned char c = 0;
        
        /* Normal case of iterating the source */
        if (i < ilen)
        {
            c = source[i];
        }

        /* Handle the two-byte checksum (first) */
        if (i == (ilen+0))
        {
            c = ((fcs >> 0) & 0xFF);
        }

        /* Handle the two-byte checksum (second) */
        if (i == (ilen+1))
        {
            c = ((fcs >> 8) & 0xFF);
        }

        /* Buffer overflow */
        if (*flen < (pos+3))
        {
            return SSTP_OVERFLOW;
        }
    
        /* Excape character if needed */
        if ((c <  HDLC_TRANSPARENCY) || 
            (c == HDLC_FLAG)         ||
            (c == HDLC_ESCAPE))
        {
            frame[pos++] = (HDLC_ESCAPE);
            frame[pos++] = (c ^ HDLC_TRANSPARENCY);
            continue;
        }

        /* Character does not need escaping */
        frame[pos++] = c;
    }

    /* Set the End of Frame marker */
    frame[pos++] = HDLC_FLAG;

    /* Set the return position */
    *flen = pos;

    return SSTP_OKAY;
}


#ifdef __SSTP_UNIT_TEST_FCS

#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    int flen = 0;
    int clen = 0;
    int  ret = 0;
    unsigned char *frame = NULL;
    unsigned char *check = NULL;
    unsigned char byte[] = 
    {
        0xff, 0x03, 0xc0, 0x21, 0x01, 0x01, 0x00, 0x18, 0x01, 0x04,
        0x05, 0x78, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06,
        0x37, 0x67, 0x24, 0xc2, 0x07, 0x02, 0x08, 0x02
    };

    /* Allocate stack space */
    flen  = (sizeof(byte) << 1) + 4;
    frame = alloca(flen);
    if (!frame)
    {
        printf("Could not allocate memory for frame\n");
        return EXIT_FAILURE;
    }

    clen  = sizeof(byte) << 1;
    check = alloca(clen);
    if (!check)
    {
        printf("Could not allocate memory for check\n");
        return EXIT_FAILURE;
    }

    ret = sstp_frame_encode(byte, sizeof(byte), frame, &flen);
    if (SSTP_OKAY != ret)
    {
        printf("Could not encode frame\n");
        return EXIT_FAILURE;
    }

    printf("Frame encoded successfully in %d bytes\n", flen);

    ret = sstp_frame_decode(frame, &flen, check, &clen);
    if (SSTP_OKAY != ret)
    {
        printf("Could not decode frame\n");
        return EXIT_FAILURE;
    }

    if (clen != sizeof(byte))
    {
        printf("The number of bytes are different, %d != %d\n", clen,
            (int)sizeof(byte));
        return EXIT_FAILURE;
    }

    if (memcmp(byte, check, sizeof(byte)))
    {
        printf("The bytes encoded is not what was decoded\n");
        return EXIT_FAILURE;
    }

    printf("Frame decoded successfully in %d bytes\n", clen);
    return EXIT_SUCCESS;
}

#endif  /* #ifdef __TEST_SSTP_FCS */


