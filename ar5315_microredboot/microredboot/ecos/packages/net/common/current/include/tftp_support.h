//==========================================================================
//
//      include/tftp_support.h
//
//      TFTP support
//
//==========================================================================
//####BSDCOPYRIGHTBEGIN####
//
// -------------------------------------------
//
// Portions of this software may have been derived from OpenBSD or other sources,
// and are covered by the appropriate copyright disclaimers included herein.
//
// -------------------------------------------
//
//####BSDCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, andrew.lunn@ascom.ch
// Date:         2000-04-06
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _TFTP_SUPPORT_H_
#define _TFTP_SUPPORT_H_

#include <fcntl.h> // O_RDONLY
/*
 * File transfer modes
 */
#define TFTP_NETASCII   0              // Text files
#define TFTP_OCTET      1              // Binary files

/*
 * Errors
 */

// These initial 7 are passed across the net in "ERROR" packets.
#define	TFTP_ENOTFOUND   1   /* file not found */
#define	TFTP_EACCESS     2   /* access violation */
#define	TFTP_ENOSPACE    3   /* disk full or allocation exceeded */
#define	TFTP_EBADOP      4   /* illegal TFTP operation */
#define	TFTP_EBADID      5   /* unknown transfer ID */
#define	TFTP_EEXISTS     6   /* file already exists */
#define	TFTP_ENOUSER     7   /* no such user */
// These extensions are return codes in our API, *never* passed on the net.
#define TFTP_TIMEOUT     8   /* operation timed out */
#define TFTP_NETERR      9   /* some sort of network error */
#define TFTP_INVALID    10   /* invalid parameter */
#define TFTP_PROTOCOL   11   /* protocol violation */
#define TFTP_TOOLARGE   12   /* file is larger than buffer */

/*
 * Server support
 */

struct tftpd_fileops {
    int (*open)(const char *, int);
    int (*close)(int);
    int (*write)(int, const void *, int);
    int (*read)(int, void *, int);
};

__externC int tftpd_start(int, struct tftpd_fileops *);
__externC int tftpd_stop(int);

/*
 * Client support
 */

/* IPv4 and IPv6 */
__externC int tftp_client_get(char *, char *, int, char *, int, int, int *);
__externC int tftp_client_put(char *, char *, int, char *, int, int, int *);

/* IPv4 only */
__externC int tftp_get(char *, struct sockaddr_in *, char *, int, int, int *);
__externC int tftp_put(char *, struct sockaddr_in *, char *, int, int, int *);

#define TFTP_TIMEOUT_PERIOD  5          // Seconds between retries
#define TFTP_TIMEOUT_MAX    50          // Max timeouts over all blocks
#define TFTP_RETRIES_MAX     5          // retries per block before giving up

#endif // _TFTP_SUPPORT_H_
