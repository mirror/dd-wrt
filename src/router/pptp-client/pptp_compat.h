/* pptp_compat.h ... Compatibility functions
 *
 */

#if defined (__SVR4) && defined (__sun) /* Solaris */
#include <sys/termios.h>

#define u_int8_t  uint8_t
#define u_int16_t uint16_t
#define u_int32_t uint32_t

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffffU
#endif

int daemon(int nochdir, int noclose);
int openpty(int *amaster, int *aslave, char *name, struct termios *termp, struct winsize * winp); 
#endif /* Solaris */
