#if defined (__SVR4) && defined (__sun) /* Solaris */
#include <netinet/in.h>
#include <net/route.h>
struct rt_msg {
        struct rt_msghdr hdr;
        struct sockaddr_in addrs[RTAX_MAX];
};
#endif /* Solaris */
void routing_init(char *ip);
void routing_start(void);
void routing_end(void);
