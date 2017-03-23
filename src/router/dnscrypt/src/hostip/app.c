
#include <config.h>
#include <sys/types.h>
#ifdef _WIN32
# include <winsock2.h>
# include <ws2tcpip.h>
# ifndef _WIN32_IE
#  define _WIN32_IE 0x400
# endif
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <event2/event.h>
#include <event2/dns.h>
#include <event2/util.h>

#include "app.h"
#include "options.h"

static AppContext app_context;

#ifndef INET6_ADDRSTRLEN
# define INET6_ADDRSTRLEN 46U
#endif

static void
query_cb_err_print(const int err)
{
    if (err == DNS_ERR_UNKNOWN || err == DNS_ERR_TRUNCATED) {
        fprintf(stderr, "Response might be truncated.\n"
                "If you just started dnscrypt-proxy, this is expected; you may want to retry.\n");
    } else {
        fprintf(stderr, "[%s]\n", evdns_err_to_string(err));
    }
    exit(1);
}

static void
ipv4_query_cb(int result, char type, int count, int ttl,
              void * const ips_, void * const app_context_)
{
    char            ip_s_buf[INET6_ADDRSTRLEN + 1U];
    AppContext     *app_context = app_context_;
    struct in_addr *ips = ips_;
    const char     *ip_s;
    int             i = 0;

    (void) ttl;
    if (result != DNS_ERR_NONE) {
        query_cb_err_print(result);
    }
    assert(type == DNS_IPv4_A);
    assert(count >= 0);
    while (i < count) {
        ip_s = evutil_inet_ntop(AF_INET, &ips[i], ip_s_buf, sizeof ip_s_buf);
        if (ip_s != NULL) {
            puts(ip_s);
        }
        i++;
    }
    event_base_loopexit(app_context->event_loop, NULL);
}

static void
ipv6_query_cb(int result, char type, int count, int ttl,
              void * const ips_, void * const app_context_)
{
    char             ip_s_buf[INET6_ADDRSTRLEN + 1U];
    AppContext      *app_context = app_context_;
    struct in6_addr *ips = ips_;
    const char      *ip_s;
    int              i = 0;

    (void) ttl;
    if (result != DNS_ERR_NONE) {
        query_cb_err_print(result);
    }
    assert(type == DNS_IPv6_AAAA);
    assert(count >= 0);
    while (i < count) {
        ip_s = evutil_inet_ntop(AF_INET6, &ips[i], ip_s_buf, sizeof ip_s_buf);
        if (ip_s != NULL) {
            puts(ip_s);
        }
        i++;
    }
    event_base_loopexit(app_context->event_loop, NULL);
}

int main(int argc, char *argv[])
{
    struct evdns_base *evdns_base;

    if (options_parse(&app_context, argc, argv) != 0) {
        return 1;
    }
#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    if ((app_context.event_loop = event_base_new()) == NULL) {
        perror("event_base_new");
        return 1;
    }
    if ((evdns_base = evdns_base_new(app_context.event_loop, 0)) == NULL) {
        perror("evdns_base");
        return 1;
    }
    evdns_base_set_option(evdns_base, "use-tcp", "on-tc");
    evdns_base_set_option(evdns_base, "randomize-case", "0");
    if (evdns_base_nameserver_ip_add(evdns_base,
                                     app_context.resolver_ip) != 0) {
        fprintf(stderr, "Unable to use [%s] as a resolver\n",
                app_context.resolver_ip);
        return 1;
    }
    if (app_context.want_ipv6 != 0) {
        evdns_base_resolve_ipv6(evdns_base, app_context.host_name,
                                DNS_QUERY_NO_SEARCH,
                                ipv6_query_cb, &app_context);
    } else {
        evdns_base_resolve_ipv4(evdns_base, app_context.host_name,
                                DNS_QUERY_NO_SEARCH,
                                ipv4_query_cb, &app_context);
    }
    event_base_dispatch(app_context.event_loop);
    event_base_free(app_context.event_loop);

    return 0;
}
