/* ISC license. */

#include <errno.h>
#include <skalibs/error.h>

int error_temp (int e)
{
  if (error_isagain(e)) return 1 ;
  switch (e)
  {
    case 0 :
    case EINTR :
    case ENOMEM :
    case ETXTBSY :
    case EIO :
    case ETIMEDOUT :
    case ENOBUFS :
#ifdef EDEADLK
    case EDEADLK :
#endif
#ifdef EBUSY
    case EBUSY :
#endif
#ifdef ENFILE
    case ENFILE :
#endif
#ifdef EFBIG
    case EFBIG :
#endif
#ifdef ENOSPC
    case ENOSPC :
#endif
#ifdef ENETDOWN
    case ENETDOWN :
#endif
#ifdef ENETUNREACH
    case ENETUNREACH :
#endif
#ifdef ENETRESET
    case ENETRESET :
#endif
#ifdef ECONNABORTED
    case ECONNABORTED :
#endif
#ifdef ECONNRESET
    case ECONNRESET :
#endif
#ifdef ETOOMANYREFS
    case ETOOMANYREFS :
#endif
#ifdef ECONNREFUSED
    case ECONNREFUSED :
#endif
#ifdef EHOSTDOWN
    case EHOSTDOWN :
#endif
#ifdef EHOSTUNREACH
    case EHOSTUNREACH :
#endif
#ifdef EPROCLIM
    case EPROCLIM :
#endif
#ifdef EUSERS
    case EUSERS :
#endif
#ifdef EDQUOT
    case EDQUOT :
#endif
#ifdef ESTALE
    case ESTALE :
#endif
#ifdef ENOLCK
    case ENOLCK :
#endif
      return 1 ;
    default : return 0 ;
  }
}
