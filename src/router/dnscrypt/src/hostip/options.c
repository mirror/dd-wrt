
#include <config.h>
#include <sys/types.h>

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "app.h"
#include "options.h"

#ifndef DEFAULT_RESOLVER_IP
# define DEFAULT_RESOLVER_IP "8.8.8.8"
#endif

static struct option getopt_long_options[] = {
    { "ipv6", 0, NULL, '6' },
    { "help", 0, NULL, 'h' },
    { "resolver-address", 1, NULL, 'r' },
    { "version", 0, NULL, 'V' },
    { NULL, 0, NULL, 0 }
};
static const char   *getopt_options = "6hr:V";

static void
options_version(void)
{
    puts("hostip v" PACKAGE_VERSION);
}

static void
options_usage(void)
{
    puts("Usage: hostip [-6] [-r resolver_ip[:port]] host_name\n"
         "  -6, --ipv6: ask for AAAA records\n"
         "  -h, --help: show usage\n"
         "  -r, --resolver-address=<ip>: the resolver IP address\n"
         "  -V, --version: show version number\n"
         "\n"
         "Example: hostip www.example.com\n");
}

static
void options_init_with_default(AppContext * const app_context)
{
    app_context->host_name = NULL;
    app_context->resolver_ip = DEFAULT_RESOLVER_IP;
    app_context->want_ipv6 = 0;
}

static int
options_apply(AppContext * const app_context)
{
    if (app_context->resolver_ip == NULL) {
        options_usage();
        exit(1);
    }
    return 0;
}

int
options_parse(AppContext * const app_context, int argc, char *argv[])
{
    int opt_flag;
    int option_index = 0;

    options_init_with_default(app_context);
    while ((opt_flag = getopt_long(argc, argv,
                                   getopt_options, getopt_long_options,
                                   &option_index)) != -1) {
        switch (opt_flag) {
        case '6':
            app_context->want_ipv6 = 1;
            break;
        case 'h':
            options_usage();
            exit(0);
        case 'r':
            app_context->resolver_ip = optarg;
            break;
        case 'V':
            options_version();
            exit(0);
        default:
            options_usage();
            exit(1);
        }
    }
    argc -= optind;
    argv += optind;
    if (argc != 1 || *argv == NULL) {
        options_usage();
        exit(1);
    }
    app_context->host_name = *argv;
    options_apply(app_context);

    return 0;
}
