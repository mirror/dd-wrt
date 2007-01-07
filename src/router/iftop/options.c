/*
 * options.c:
 *
 *
 */

#include "config.h"

#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "iftop.h"
#include "options.h"
#include "cfgfile.h"
#include "integers.h"

#if !defined(HAVE_INET_ATON) && defined(HAVE_INET_PTON)
#   define inet_aton(a, b)  inet_pton(AF_INET, (a), (b))
#endif

options_t options;

char optstr[] = "+i:f:nNF:hpbBPm:c:";

/* Global options. */

/* Selecting an interface on which to listen: */

/* This is a list of interface name prefixes which are `bad' in the sense
 * that they don't refer to interfaces of external type on which we are
 * likely to want to listen. We also compare candidate interfaces to lo. */
static char *bad_interface_names[] = {
            "lo:",
            "lo",
            "stf",     /* pseudo-device 6to4 tunnel interface */
            "gif",     /* psuedo-device generic tunnel interface */
            "dummy",
            "vmnet",
            NULL        /* last entry must be NULL */
        };

config_enumeration_type sort_enumeration[] = {
	{ "2s", OPTION_SORT_DIV1 },
	{ "10s", OPTION_SORT_DIV2 },
	{ "40s", OPTION_SORT_DIV3 },
	{ "source", OPTION_SORT_SRC },
	{ "destination", OPTION_SORT_SRC },
	{ NULL, -1 }
};

config_enumeration_type linedisplay_enumeration[] = {
	{ "two-line", OPTION_LINEDISPLAY_TWO_LINE },
	{ "one-line-both", OPTION_LINEDISPLAY_ONE_LINE_BOTH },
	{ "one-line-sent", OPTION_LINEDISPLAY_ONE_LINE_SENT },
	{ "one-line-received", OPTION_LINEDISPLAY_ONE_LINE_RECV },
	{ NULL, -1 }
};

config_enumeration_type showports_enumeration[] = {
	{ "off", OPTION_PORTS_OFF },
	{ "source-only", OPTION_PORTS_SRC },
	{ "destination-only", OPTION_PORTS_DEST },
	{ "on", OPTION_PORTS_ON },
	{ NULL, -1 }
};

static int is_bad_interface_name(char *i) {
    char **p;
    for (p = bad_interface_names; *p; ++p)
        if (strncmp(i, *p, strlen(*p)) == 0)
            return 1;
    return 0;
}

/* This finds the first interface which is up and is not the loopback
 * interface or one of the interface types listed in bad_interface_names. */
static char *get_first_interface(void) {
    struct if_nameindex * nameindex;
    char *i = NULL;
    int j = 0;
    /* Use if_nameindex(3) instead? */

    nameindex = if_nameindex();
    if(nameindex == NULL) {
        return NULL;
    }

    while(nameindex[j].if_index != 0) {
        if (strcmp(nameindex[j].if_name, "lo") != 0 && !is_bad_interface_name(nameindex[j].if_name)) {
            i = xstrdup(nameindex[j].if_name);
            break;
        }
        j++;
    }
    if_freenameindex(nameindex);
    return i;
}

void options_set_defaults() {
    char *s;
    /* Should go through the list of interfaces, and find the first one which
     * is up and is not lo or dummy*. */
    options.interface = get_first_interface();
    if (!options.interface)
        options.interface = "eth0";

    options.filtercode = NULL;
    options.netfilter = 0;
    inet_aton("10.0.1.0", &options.netfilternet);
    inet_aton("255.255.255.0", &options.netfiltermask);
    options.dnsresolution = 1;
    options.portresolution = 1;
#ifdef NEED_PROMISCUOUS_FOR_OUTGOING
    options.promiscuous = 1;
    options.promiscuous_but_choosy = 1;
#else
    options.promiscuous = 0;
    options.promiscuous_but_choosy = 0;
#endif
    options.showbars = 1;
    options.showports = OPTION_PORTS_OFF;
    options.aggregate_src = 0;
    options.aggregate_dest = 0;
    options.paused = 0;
    options.showhelp = 0;
    options.bandwidth_in_bytes = 0;
    options.sort = OPTION_SORT_DIV2;
    options.screenfilter = NULL;
    options.freezeorder = 0;
    options.linedisplay = OPTION_LINEDISPLAY_TWO_LINE;
    options.screen_offset = 0;
    options.show_totals = 0;
    options.max_bandwidth = 0; /* auto */
    options.log_scale = 0;
    options.bar_interval = 1;

    /* Figure out the name for the config file */
    s = getenv("HOME");
    if(s != NULL) {
        int i = strlen(s) + 9 + 1;
        options.config_file = xmalloc(i);
        snprintf(options.config_file,i,"%s/.iftoprc",s);
    }
    else {
        options.config_file = xstrdup("iftoprc");
    }
    options.config_file_specified = 0;
    
}

static void die(char *msg) {
    fprintf(stderr, msg);
    exit(1);
}

static void set_max_bandwidth(char* arg) {
    char* units;
    long long mult = 1;
    long long value;
    units = arg + strspn(arg, "0123456789");
    if(strlen(units) > 1) {
        die("Invalid units\n");
    }
    if(strlen(units) == 1) {
        if(*units == 'k' || *units == 'K') {
            mult = 1024;
        }
        else if(*units == 'm' || *units == 'M') {
            mult = 1024 * 1024;
        }
        else if(*units == 'g' || *units == 'G') {
            mult = 1024 * 1024 * 1024;
        }
    }
    *units = '\0';
    if(sscanf(arg, "%lld", &value) != 1) {
        die("Error reading max bandwidth\n");
    }
    options.max_bandwidth = value * mult;
}

static void set_net_filter(char* arg) {
    char* mask;

    mask = strchr(arg, '/');
    if (mask == NULL) {
        die("Could not parse net/mask\n");
    }
    *mask = '\0';
    mask++;
    if (inet_aton(arg, &options.netfilternet) == 0)
        die("Invalid network address\n");
    /* Accept a netmask like /24 or /255.255.255.0. */
    if (mask[strspn(mask, "0123456789")] == '\0') {
        /* Whole string is numeric */
        int n;
        n = atoi(mask);
        if (n > 32) {
            die("Invalid netmask");
        }
        else {
            if(n == 32) {
              /* This needs to be special cased, although I don't fully 
               * understand why -pdw 
               */
              options.netfiltermask.s_addr = htonl(0xffffffffl);
            }
            else {
              u_int32_t mm = 0xffffffffl;
              mm >>= n;
              options.netfiltermask.s_addr = htonl(~mm);
            }
        }
    } 
    else if (inet_aton(mask, &options.netfiltermask) == 0) {
        die("Invalid netmask\n");
    }
    options.netfilternet.s_addr = options.netfilternet.s_addr & options.netfiltermask.s_addr;

    options.netfilter = 1;

}

/* usage:
 * Print usage information. */
static void usage(FILE *fp) {
    fprintf(fp,
"iftop: display bandwidth usage on an interface by host\n"
"\n"
"Synopsis: iftop -h | [-npbBP] [-i interface] [-f filter code] [-N net/mask]\n"
"\n"
"   -h                  display this message\n"
"   -n                  don't do hostname lookups\n"
"   -N                  don't convert port numbers to services\n"
"   -p                  run in promiscuous mode (show traffic between other\n"
"                       hosts on the same network segment)\n"
"   -b                  don't display a bar graph of traffic\n"
"   -B                  Display bandwidth in bytes\n"
"   -i interface        listen on named interface\n"
"   -f filter code      use filter code to select packets to count\n"
"                       (default: none, but only IP packets are counted)\n"
"   -F net/mask         show traffic flows in/out of network\n"
"   -P                  show ports as well as hosts\n"
"   -m limit            sets the upper limit for the bandwidth scale\n"
"   -c config file      specifies an alternative configuration file\n"
"\n"
"iftop, version " IFTOP_VERSION "\n"
"copyright (c) 2002 Paul Warren <pdw@ex-parrot.com> and contributors\n"
            );
}

void options_read_args(int argc, char **argv) {
    int opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, optstr)) != -1) {
        switch (opt) {
            case 'h':
                usage(stdout);
                exit(0);

            case 'n':
                config_set_string("dns-resolution","false");
                break;

            case 'N':
                config_set_string("port-resolution","false");
                break;

            case 'i':
                config_set_string("interface", optarg);
                break;

            case 'f':
                config_set_string("filter-code", optarg);
                break;

            case 'p':
                config_set_string("promiscuous", "true");
                break;

            case 'P':
                config_set_string("port-display", "on");
                break;

            case 'F':
                config_set_string("net-filter", optarg);
                break;
            
            case 'm':
                config_set_string("max-bandwidth", optarg);
                break;

            case 'b':
                config_set_string("show-bars", "true");
                break;

            case 'B':
                config_set_string("use-bytes", "true");
                break;

            case 'c':
                xfree(options.config_file);
                options.config_file = xstrdup(optarg);
                options.config_file_specified = 1;
                break;

            case '?':
                fprintf(stderr, "iftop: unknown option -%c\n", optopt);
                usage(stderr);
                exit(1);

            case ':':
                fprintf(stderr, "iftop: option -%c requires an argument\n", optopt);
                usage(stderr);
                exit(1);
        }
    }

    if (optind != argc) {
        fprintf(stderr, "iftop: found arguments following options\n");
        fprintf(stderr, "*** some options have changed names since v0.9 ***\n");
        usage(stderr);
        exit(1);
    }
}

/* options_config_get_string:
 * Gets a value from the config, sets *value to a copy of the value, if
 * found.  Leaves the option unchanged otherwise. */
int options_config_get_string(const char *name, char** value) {
    char *s;
    s = config_get_string(name);
    if(s != NULL) {
        *value = xstrdup(s);
        return 1;
    }
    return 0;
}

int options_config_get_bool(const char *name, int* value) {
    if(config_get_string(name)) {
        *value = config_get_bool(name);
        return 1;
    }
    return 0;
}

int options_config_get_int(const char *name, int* value) {
    if(config_get_string(name)) {
        config_get_int(name, value);
        return 1;
    }
    return 0;
}

int options_config_get_enum(char *name, config_enumeration_type* enumeration, int *result) {
    int i;
    if(config_get_string(name)) {
        if(config_get_enum(name, enumeration, &i)) {
            *result = i; 
            return 1;
        }
    }
    return 0;
}

int options_config_get_promiscuous() {
    if(config_get_string("promiscuous")) {
        options.promiscuous = config_get_bool("promiscuous");
        if(options.promiscuous) {
            /* User has explicitly requested promiscuous mode, so don't be
             * choosy */
            options.promiscuous_but_choosy = 0;
        }
        return 1;
    }
    return 0;
}

int options_config_get_bw_rate(char *directive, long long* result) {
    char* units;
    long long mult = 1;
    long long value;
    char *s;
    s = config_get_string(directive);
    if(s) {
        units = s + strspn(s, "0123456789");
        if(strlen(units) > 1) {
            fprintf(stderr, "Invalid units in value: %s\n", s);
            return 0;
        }
        if(strlen(units) == 1) {
            if(*units == 'k' || *units == 'K') {
                mult = 1024;
            }
            else if(*units == 'm' || *units == 'M') {
                mult = 1024 * 1024;
            }
            else if(*units == 'g' || *units == 'G') {
                mult = 1024 * 1024 * 1024;
            }
            else if(*units == 'b' || *units == 'B') {
                /* bits => mult = 1 */
            }
            else {
                fprintf(stderr, "Invalid units in value: %s\n", s);
                return 0;
            }
        }
        *units = '\0';
        if(sscanf(s, "%lld", &value) != 1) {
            fprintf(stderr, "Error reading rate: %s\n", s);
        }
        options.max_bandwidth = value * mult;
        return 1;
    }
    return 0;
}

/*
 * Read the net filter option.  
 */
int options_config_get_net_filter() {
    char* s;
    s = config_get_string("net-filter");
    if(s) {
        char* mask;

        mask = strchr(s, '/');
        if (mask == NULL) {
            fprintf(stderr, "Could not parse net/mask: %s\n", s);
            return 0;
        }
        *mask = '\0';
        mask++;
        if (inet_aton(s, &options.netfilternet) == 0) {
            fprintf(stderr, "Invalid network address: %s\n", s);
            return 0;
        }
        /* Accept a netmask like /24 or /255.255.255.0. */
        if (mask[strspn(mask, "0123456789")] == '\0') {
            /* Whole string is numeric */
            int n;
            n = atoi(mask);
            if (n > 32) {
                fprintf(stderr, "Invalid netmask: %s\n", s);
            }
            else {
                if(n == 32) {
                  /* This needs to be special cased, although I don't fully 
                   * understand why -pdw 
                   */
                  options.netfiltermask.s_addr = htonl(0xffffffffl);
                }
                else {
                  u_int32_t mm = 0xffffffffl;
                  mm >>= n;
                  options.netfiltermask.s_addr = htonl(~mm);
                }
            }
        } 
        else if (inet_aton(mask, &options.netfiltermask) == 0) {
            fprintf(stderr, "Invalid netmask: %s\n", s);
        }
        options.netfilternet.s_addr = options.netfilternet.s_addr & options.netfiltermask.s_addr;
        options.netfilter = 1;
        return 1;
    }
    return 0;
}


void options_make() {
    options_config_get_string("interface", &options.interface);
    options_config_get_bool("dns-resolution", &options.dnsresolution);
    options_config_get_bool("port-resolution", &options.portresolution);
    options_config_get_string("filter-code", &options.filtercode);
    options_config_get_bool("show-bars", &options.showbars);
    options_config_get_promiscuous();
    options_config_get_bool("hide-source", &options.aggregate_src);
    options_config_get_bool("hide-destination", &options.aggregate_dest);
    options_config_get_bool("use-bytes", &options.bandwidth_in_bytes);
    options_config_get_enum("sort", sort_enumeration, (int*)&options.sort);
    options_config_get_enum("line-display", linedisplay_enumeration, (int*)&options.linedisplay);
    options_config_get_bool("show-totals", &options.show_totals);
    options_config_get_bool("log-scale", &options.log_scale);
    options_config_get_bw_rate("max-bandwidth", &options.max_bandwidth);
    options_config_get_enum("port-display", showports_enumeration, (int*)&options.showports);
    options_config_get_string("screen-filter", &options.screenfilter);
    options_config_get_net_filter();
};
