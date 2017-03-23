
#ifndef __APP_H__
#define __APP_H__ 1

#include <event2/event.h>

typedef struct AppContext_ {
    struct event_base *event_loop;
    const char        *host_name;
    const char        *resolver_ip;
    _Bool              want_ipv6;
} AppContext;

#endif
