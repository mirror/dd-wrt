/*
 * options.h:
 *
 */

#ifndef __OPTIONS_H_ /* include guard */
#define __OPTIONS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


typedef enum {
  OPTION_PORTS_OFF,
  OPTION_PORTS_SRC,
  OPTION_PORTS_DEST,
  OPTION_PORTS_ON
} option_port_t;

typedef enum {
  OPTION_SORT_DIV1,
  OPTION_SORT_DIV2,
  OPTION_SORT_DIV3,
  OPTION_SORT_SRC,
  OPTION_SORT_DEST
} option_sort_t;

typedef enum {
  OPTION_LINEDISPLAY_TWO_LINE,
  OPTION_LINEDISPLAY_ONE_LINE_BOTH,
  OPTION_LINEDISPLAY_ONE_LINE_RECV,
  OPTION_LINEDISPLAY_ONE_LINE_SENT
} option_linedisplay_t;

/* 
 * This structure has to be defined in the same order as the config 
 * directives in cfgfile.c.  Clearly this is EBW.
 */
typedef struct {
    /* interface on which to listen */
    char *interface;

    int dnsresolution;
    int portresolution;
    /* pcap filter code */
    char *filtercode;

    int showbars;
    option_port_t showports;

    int promiscuous;
    int promiscuous_but_choosy;
    int aggregate_src;
    int aggregate_dest;
    int paused;
    int showhelp;
    int bandwidth_in_bytes;
    option_sort_t sort;

    int bar_interval;

    char* screenfilter;
    int freezeorder;

    int screen_offset;

    option_linedisplay_t linedisplay;

    int show_totals;

    long long max_bandwidth;
    int log_scale;

    /* Cross network filter */
    int netfilter;
    struct in_addr netfilternet;
    struct in_addr netfiltermask;

    char *config_file;
    int config_file_specified;

} options_t;


void options_set_defaults();
void options_read(int argc, char **argv);

#endif /* __OPTIONS_H_ */
