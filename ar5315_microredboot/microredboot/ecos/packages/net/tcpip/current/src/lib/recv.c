//==========================================================================
//
//      lib/recv.c
//
//      recv() system call
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
// Author(s):    gthomas,andrew.lunn@ascom.ch
// Contributors: gthomas
// Date:         2001-11-01
// Purpose:      
// Description:  
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================


#include <sys/param.h>
#include <cyg/io/file.h>
#include <sys/socket.h>
#include <sys/socketvar.h>

extern ssize_t recvfrom (int, void *, size_t, int, struct sockaddr *, socklen_t *);

ssize_t	
recv(int s, void *buf, size_t buflen, int flags)
{
    
    return(recvfrom(s,buf,buflen,flags,NULL,0));
}
