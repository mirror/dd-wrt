/* network.h for compatibility with the other eCos network stacks */

#include <lwip/sys.h>           /* lwIP stack includes */
#define LWIP_COMPAT_SOCKETS 1
#include <lwip/sockets.h>
#include <lwip/inet.h>
