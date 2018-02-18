/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#ifdef __linux__

#include "egressFile.h"

/* Plugin includes */

/* OLSRD includes */
#include "olsr_cfg.h"
#include "gateway_costs.h"
#include "gateway.h"
#include "scheduler.h"
#include "ipcalc.h"
#include "log.h"

/* System includes */
#include <unistd.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/stat.h>
#include <assert.h>
#include <net/if.h>

/** the maximum length of a line that is read from the file */
#define LINE_LENGTH 256

/** regular expression describing a comment */
static const char * regexComment = "^([[:space:]]*|[[:space:]#]+.*)$";

/**
 * regular expression describing an egress line.
 *
 * # interface=uplink (Kbps),downlink (Kbps),path cost,gateway
 */
static const char * regexEgress = "^[[:space:]]*" //
        "([^[:space:]=]+)"                     /* 01: interface, mandatory, can NOT be empty */
        "[[:space:]]*=[[:space:]]*"            /* --: field separator */
        "([[:digit:]]*)"                       /* 02: requireNetwork, mandatory, can be empty */
        "[[:space:]]*,[[:space:]]*"            /* --: field separator */
        "([[:digit:]]*)"                       /* 03: requireGateway, mandatory, can be empty */
        "[[:space:]]*,[[:space:]]*"            /* --: field separator */
        "([[:digit:]]*)"                       /* 04: uplink, mandatory, can be empty */
        "[[:space:]]*,[[:space:]]*"            /* --: field separator */
        "([[:digit:]]*)"                       /* 05: downlink, mandatory, can be empty */
        "("                                    /* 06: (the rest is optional) */
        "[[:space:]]*,[[:space:]]*"            /* --: field separator */
        "([[:digit:]]*)"                       /* 07: path cost, optional, can be empty */
        "("                                    /* 08: (the rest is optional) */
        "[[:space:]]*,[[:space:]]*"            /* --: field separator */
        "(|([[:digit:]\\.:]+)/([[:digit:]]+))" /* 09: network, optional, can be empty, 09=ip/x 10=ip 11=x */
        "("                                    /* 12: (the rest is optional) */
        "[[:space:]]*,[[:space:]]*"            /* --: field separator */
        "([[:digit:]\\.:]*)"                   /* 13: gateway, optional, can be empty */
        ")?"                                   /* 12 */
        ")?"                                   /* 08 */
        ")?"                                   /* 06 */
        "[[:space:]]*$";

/** the number of matches in regexEgress */
#define REGEX_EGRESS_LINE_MATCH_COUNT (1 /* 00 */ + 13)

/** the compiled regular expression describing a comment */
static regex_t compiledRegexComment;

/** the compiled regular expression describing an egress line */
static regex_t compiledRegexEgress;

/** true when the file reader has been started */
static bool started = false;

/** type to hold the cached stat result */
typedef struct _CachedStat {
#if defined(__linux__) && !defined(__ANDROID__)
  struct timespec timeStamp; /* Time of last modification (full resolution) */
#else
  time_t timeStamp; /* Time of last modification (second resolution) */
#endif
} CachedStat;

/** the cached stat result */
static CachedStat cachedStat;

/** the malloc-ed buffer in which to store a line read from the file */
static char * line = NULL;

/* forward declaration */
static bool readEgressFile(const char * fileName);

/*
 * Error Reporting
 */

/** the maximum length of an error report */
#define ERROR_LENGTH 1024

/** true when errors have been reported, used to reduce error reports */
static bool reportedErrors = false;

/**
 * Report an error.
 *
 * @param useErrno
 * when true then errno is used in the error message; the error reason is also
 * reported.
 * @param lineNo
 * the line number of the caller
 * @param format
 * a pointer to the format string
 * @param ...
 * arguments to the format string
 */
__attribute__ ((format(printf, 3, 4)))
static void egressFileError(bool useErrno, int lineNo, const char *format, ...) {
  char str[ERROR_LENGTH];
  char *strErr = NULL;

  if (reportedErrors) {
    return;
  }

  if (useErrno) {
    strErr = strerror(errno);
  }

  if ((format == NULL ) || (*format == '\0')) {
    olsr_syslog(OLSR_LOG_ERR, "%s@%d: %s\n", __FILE__, lineNo, useErrno ? strErr : "Unknown error");
  } else {
    va_list arglist;

    va_start(arglist, format);
    vsnprintf(str, sizeof(str), format, arglist);
    va_end(arglist);

    str[sizeof(str) - 1] = '\0'; /* Ensures null termination */

    if (useErrno) {
      olsr_syslog(OLSR_LOG_ERR, "%s@%d: %s: %s\n", __FILE__, lineNo, str, strErr);
    } else {
      olsr_syslog(OLSR_LOG_ERR, "%s@%d: %s\n", __FILE__, lineNo, str);
    }
  }
}

/*
 * Helpers
 */

/**
 * Read an (olsr_ip_addr) IP address from a string:
 * First tries to parse the value as an IPv4 address, and if not successful
 * tries to parse it as an IPv6 address.
 *
 * @param str
 * The string to convert to an (olsr_ip_addr) IP address
 * @param dst
 * A pointer to the location where to store the (olsr_ip_addr) IP address upon
 * successful conversion. Not touched when errors are reported.
 * @param dstSet
 * A pointer to the location where to store the flag that signals whether the
 * IP address is set. Not touched when errors are reported.
 * @param dstIpVersion
 * A pointer to the location where to store the IP version of the IP address.
 * Not touched when errors are reported.
 *
 * @return
 * - true on success
 * - false otherwise
 */
static bool readIPAddress(const char * str, union olsr_ip_addr * dst, bool * dstSet, int * dstIpVersion) {
  int conversion;
  union olsr_ip_addr ip;
  int ip_version;

  assert(str);
  assert(dst);
  assert(dstSet);
  assert(dstIpVersion);

  /* try IPv4 first */
  ip_version = AF_INET;
  memset(&ip, 0, sizeof(ip));
  conversion = inet_pton(ip_version, str, &ip.v4);

  if (conversion != 1) {
    /* now try IPv6: IPv4 conversion was not successful */
    ip_version = AF_INET6;
    memset(&ip, 0, sizeof(ip));
    conversion = inet_pton(ip_version, str, &ip.v6);
  }

  if (conversion != 1) {
    return false;
  }

  *dst = ip;
  *dstSet = true;
  *dstIpVersion = ip_version;
  return true;
}

/**
 * Read an unsigned long long number from a value string.
 * An empty string results in a value of zero.
 *
 * @param value
 * The string to convert to a number
 * @param valueNumber
 * A pointer to the location where to store the number upon successful conversion.
 * Not touched when errors are reported.
 *
 * @return
 * - true on success
 * - false otherwise
 */
static bool readULL(const char * value, unsigned long long * valueNumber) {
  char * endPtr = NULL;
  unsigned long valueNew;

  assert(value);
  assert(valueNumber);

  if (!value || !strlen(value)) {
    *valueNumber = 0;
    return true;
  }

  errno = 0;
  valueNew = strtoull(value, &endPtr, 10);

  if (!((endPtr != value) && (*value != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
    /* invalid conversion */
    return false;
  }

  *valueNumber = valueNew;
  return true;
}

/**
 * Strip EOL characters from the end of a string
 *
 * @param str the string to strip
 */
static void stripEols(char * str) {
  ssize_t len = strlen(str);
  while ((len > 0) && ((str[len - 1] == '\n') || (str[len - 1] == '\r'))) {
    len--;
  }
  str[len] = '\0';
}

/**
 * Find an egress interface in the configuration
 *
 * @param name the name of the egress interface
 * @return the pointer to the egress interface, NULL when not found
 */
struct sgw_egress_if * findEgressInterface(char * name) {
  if (name && (name[0] != '\0')) {
    struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
    while (egress_if) {
      if (!strcmp(egress_if->name, name)) {
        return egress_if;
      }
      egress_if = egress_if->next;
    }
  }

  return NULL ;
}

/**
 * Find an egress interface in the configuration by if_index
 *
 * @param if_index the index of the egress interface
 * @return the pointer to the egress interface, NULL when not found
 */
struct sgw_egress_if * findEgressInterfaceByIndex(int if_index) {
  if (if_index > 0) {
    struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
    while (egress_if) {
      if (egress_if->if_index == if_index) {
        return egress_if;
      }
      egress_if = egress_if->next;
    }
  }

  return NULL ;
}

/**
 * Calculate the costs from the bandwidth parameters
 *
 * @param bw the bandwidth parameters
 * @param up true when the interface is up
 * @return true when the costs changed
 */
bool egressBwCalculateCosts(struct egress_if_bw * bw, bool up) {
  int64_t costsPrevious = bw->costs;
  bw->costs = gw_costs_weigh(up, bw->path_cost, bw->egressUk, bw->egressDk);
  return (costsPrevious != bw->costs);
}

/**
 * Clear the bandwidth parameters
 * @param bw the bandwidth parameters
 * @param up true when the interface is up
 */
void egressBwClear(struct egress_if_bw * bw, bool up) {
  bw->requireNetwork = true;
  bw->requireGateway = true;
  bw->egressUk = 0;
  bw->egressDk = 0;
  bw->path_cost = 0;
  memset(&bw->network, 0, sizeof(bw->network));
  memset(&bw->gateway, 0, sizeof(bw->gateway));

  bw->networkSet = false;
  bw->gatewaySet = false;

  egressBwCalculateCosts(bw, up);
}

/*
 * Timer
 */

/** the timer for polling the egress file for changes */
static struct timer_entry *egress_file_timer;

/**
 * Timer callback to read the egress file
 *
 * @param unused unused
 */
static void egress_file_timer_callback(void *unused __attribute__ ((unused))) {
  if (readEgressFile(olsr_cnf->smart_gw_egress_file)) {
    doRoutesMultiGw(true, false, GW_MULTI_CHANGE_PHASE_RUNTIME);
  }
}

/*
 * Life Cycle
 */

/**
 * Initialises the egress file reader
 *
 * @return
 * - true upon success
 * - false otherwise
 */
bool startEgressFile(void) {
  int r;

  if (started) {
    return true;
  }

  line = malloc(LINE_LENGTH);
  if (!line) {
    egressFileError(false, __LINE__, "Could not allocate a line buffer");
    return false;
  }
  *line = '\0';

  r = regcomp(&compiledRegexComment, regexComment, REG_EXTENDED);
  if (r) {
    regerror(r, &compiledRegexComment, line, LINE_LENGTH);
    egressFileError(false, __LINE__, "Could not compile regex \"%s\" (%d = %s)", regexComment, r, line);

    free(line);
    line = NULL;
    return false;
  }

  r = regcomp(&compiledRegexEgress, regexEgress, REG_EXTENDED);
  if (r) {
    regerror(r, &compiledRegexEgress, line, LINE_LENGTH);
    egressFileError(false, __LINE__, "Could not compile regex \"%s\" (%d = %s)", regexEgress, r, line);

    regfree(&compiledRegexComment);
    free(line);
    line = NULL;
    return false;
  }

  memset(&cachedStat.timeStamp, 0, sizeof(cachedStat.timeStamp));

  readEgressFile(olsr_cnf->smart_gw_egress_file);

  olsr_set_timer(&egress_file_timer, olsr_cnf->smart_gw_egress_file_period, 0, true, &egress_file_timer_callback, NULL, NULL);

  started = true;
  return true;
}

/**
 * Cleans up the egress file reader.
 */
void stopEgressFile(void) {
  if (started) {
    olsr_stop_timer(egress_file_timer);
    egress_file_timer = NULL;

    regfree(&compiledRegexEgress);
    regfree(&compiledRegexComment);
    free(line);
    line = NULL;

    started = false;
  }
}

/*
 * File Reader
 */

/** the buffer with regex matches */
static regmatch_t pmatch[REGEX_EGRESS_LINE_MATCH_COUNT];

static void readEgressFileClear(void) {
  struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
  while (egress_if) {
    egress_if->bwPrevious = egress_if->bwCurrent;
    egress_if->bwCostsChanged = false;
    egress_if->bwNetworkChanged = false;
    egress_if->bwGatewayChanged = false;
    egress_if->bwChanged = false;

    egress_if->inEgressFile = false;

    egress_if = egress_if->next;
  }
}

/**
 * Read the egress file
 *
 * @param fileName the filename
 * @return true to indicate changes (any egress_if->bwChanged is true)
 */
static bool readEgressFile(const char * fileName) {
  int fd;
  struct stat statBuf;
  FILE * fp = NULL;
  void * mtim;
  unsigned int lineNumber = 0;

  bool changed = false;
  bool reportedErrorsLocal = false;
  const char * filepath = !fileName ? DEF_GW_EGRESS_FILE : fileName;

  fd = open(filepath, O_RDONLY);
  if (fd < 0) {
    /* could not open the file */
    memset(&cachedStat.timeStamp, 0, sizeof(cachedStat.timeStamp));
    readEgressFileClear();
    goto outerror;
  }

  if (fstat(fd, &statBuf)) {
    /* could not stat the file */
    memset(&cachedStat.timeStamp, 0, sizeof(cachedStat.timeStamp));
    readEgressFileClear();
    goto outerror;
  }

#if defined(__linux__) && !defined(__ANDROID__)
  mtim = &statBuf.st_mtim;
#else
  mtim = &statBuf.st_mtime;
#endif

  if (!memcmp(&cachedStat.timeStamp, mtim, sizeof(cachedStat.timeStamp))) {
    /* file did not change since last read */
    goto out;
  }

  fp = fdopen(fd, "r");
  if (!fp) {
    /* could not open the file */
    goto out;
  }

  memcpy(&cachedStat.timeStamp, mtim, sizeof(cachedStat.timeStamp));

  /* copy 'current' egress interfaces into 'previous' field */
  readEgressFileClear();

  while (fgets(line, LINE_LENGTH, fp)) {
    struct sgw_egress_if * egress_if = NULL;
    bool requireNetwork = true;
    bool requireGateway = true;
    unsigned long long uplink = DEF_EGRESS_UPLINK_KBPS;
    unsigned long long downlink = DEF_EGRESS_DOWNLINK_KBPS;
    unsigned long long pathCosts = DEF_EGRESS_PATH_COSTS;
    struct olsr_ip_prefix network;
    union olsr_ip_addr gateway;
    bool networkSet = false;
    bool gatewaySet = false;
    int networkIpVersion = AF_INET;
    int gatewayIpVersion = AF_INET;

    lineNumber++;

    if (!regexec(&compiledRegexComment, line, 0, NULL, 0)) {
      /* the line is a comment */
      continue;
    }

    memset(&network, 0, sizeof(network));
    memset(&gateway, 0, sizeof(gateway));

    stripEols(line);

    memset(pmatch, 0, sizeof(pmatch));
    if (regexec(&compiledRegexEgress, line, REGEX_EGRESS_LINE_MATCH_COUNT, pmatch, 0)) {
      egressFileError(false, __LINE__, "Egress speed file line %d uses invalid syntax: line is ignored (%s)", lineNumber, line);
      reportedErrorsLocal = true;
      continue;
    }

    /* iface: mandatory presence, guaranteed through regex match */
    {
      regoff_t len = pmatch[1].rm_eo - pmatch[1].rm_so;
      char * ifaceString = &line[pmatch[1].rm_so];
      line[pmatch[1].rm_eo] = '\0';

      if (len > IFNAMSIZ) {
        /* interface name is too long */
        egressFileError(false, __LINE__, "Egress speed file line %d: interface \"%s\" is too long: line is ignored", lineNumber, ifaceString);
        reportedErrorsLocal = true;
        continue;
      }

      egress_if = findEgressInterface(ifaceString);
      if (!egress_if) {
        /* not a known egress interface */
        egressFileError(false, __LINE__, "Egress speed file line %d: interface \"%s\" is not a configured egress interface: line is ignored", lineNumber,
            ifaceString);
        reportedErrorsLocal = true;
        continue;
      }
    }
    assert(egress_if);

    /* requireNetwork: mandatory presence, guaranteed through regex match */
    {
      regoff_t len = pmatch[2].rm_eo - pmatch[2].rm_so;
      char * requireNetworkString = &line[pmatch[2].rm_so];
      unsigned long long value = 1;
      line[pmatch[2].rm_eo] = '\0';

      if ((len > 0) && !readULL(requireNetworkString, &value)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: requireNetwork \"%s\" is not a valid number: line is ignored", lineNumber,
            requireNetworkString);
        reportedErrorsLocal = true;
        continue;
      } else {
        requireNetwork = (value != 0);
      }
    }

    /* requireGateway: mandatory presence, guaranteed through regex match */
    {
      regoff_t len = pmatch[3].rm_eo - pmatch[3].rm_so;
      char * requireGatewayString = &line[pmatch[3].rm_so];
      unsigned long long value = 1;
      line[pmatch[3].rm_eo] = '\0';

      if ((len > 0) && !readULL(requireGatewayString, &value)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: requireGateway \"%s\" is not a valid number: line is ignored", lineNumber,
            requireGatewayString);
        reportedErrorsLocal = true;
        continue;
      } else {
        requireGateway = (value != 0);
      }
    }

    /* uplink: mandatory presence, guaranteed through regex match */
    {
      regoff_t len = pmatch[4].rm_eo - pmatch[4].rm_so;
      char * uplinkString = &line[pmatch[4].rm_so];
      line[pmatch[4].rm_eo] = '\0';

      if ((len > 0) && !readULL(uplinkString, &uplink)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: uplink bandwidth \"%s\" is not a valid number: line is ignored", lineNumber, uplinkString);
        reportedErrorsLocal = true;
        continue;
      }
    }
    uplink = MIN(uplink, MAX_SMARTGW_SPEED);

    /* downlink: mandatory presence, guaranteed through regex match */
    {
      regoff_t len = pmatch[5].rm_eo - pmatch[5].rm_so;
      char * downlinkString = &line[pmatch[5].rm_so];
      line[pmatch[5].rm_eo] = '\0';

      if ((len > 0) && !readULL(downlinkString, &downlink)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: downlink bandwidth \"%s\" is not a valid number: line is ignored", lineNumber,
            downlinkString);
        reportedErrorsLocal = true;
        continue;
      }
    }
    downlink = MIN(downlink, MAX_SMARTGW_SPEED);

    /* path costs: optional presence */
    if (pmatch[7].rm_so != -1) {
      regoff_t len = pmatch[7].rm_eo - pmatch[7].rm_so;
      char * pathCostsString = &line[pmatch[7].rm_so];
      line[pmatch[7].rm_eo] = '\0';

      if ((len > 0) && !readULL(pathCostsString, &pathCosts)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: path costs \"%s\" is not a valid number: line is ignored", lineNumber, pathCostsString);
        reportedErrorsLocal = true;
        continue;
      }
    }
    pathCosts = MIN(pathCosts, UINT32_MAX);

    /* network: optional presence */
    if ((pmatch[9].rm_so != -1) && ((pmatch[9].rm_eo - pmatch[9].rm_so) > 0)) {
      /* network is present: guarantees IP and prefix presence */
      unsigned long long prefix_len;
      char * networkString = &line[pmatch[10].rm_so];
      char * prefixlenString = &line[pmatch[11].rm_so];
      line[pmatch[10].rm_eo] = '\0';
      line[pmatch[11].rm_eo] = '\0';

      if (!readIPAddress(networkString, &network.prefix, &networkSet, &networkIpVersion)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: network IP address \"%s\" is not a valid IP address: line is ignored", lineNumber,
            networkString);
        reportedErrorsLocal = true;
        continue;
      }

      if (!readULL(prefixlenString, &prefix_len)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: network prefix \"%s\" is not a valid number: line is ignored", lineNumber,
            prefixlenString);
        reportedErrorsLocal = true;
        continue;
      }

      if (prefix_len > ((networkIpVersion == AF_INET) ? 32 : 128)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: network prefix \"%s\" is not in the range [0, %d]: line is ignored", lineNumber,
            prefixlenString, ((networkIpVersion == AF_INET) ? 32 : 128));
        reportedErrorsLocal = true;
        continue;
      }

      network.prefix_len = prefix_len;
    }

    /* gateway: optional presence */
    if (pmatch[13].rm_so != -1) {
      regoff_t len = pmatch[13].rm_eo - pmatch[13].rm_so;
      char * gatewayString = &line[pmatch[13].rm_so];
      line[pmatch[13].rm_eo] = '\0';

      if ((len > 0) && !readIPAddress(gatewayString, &gateway, &gatewaySet, &gatewayIpVersion)) {
        egressFileError(false, __LINE__, "Egress speed file line %d: gateway IP address \"%s\" is not a valid IP address: line is ignored", lineNumber,
            gatewayString);
        reportedErrorsLocal = true;
        continue;
      }
    }

    /* check all IP versions are the same */
    if ((networkSet && gatewaySet) && (networkIpVersion != gatewayIpVersion)) {
      egressFileError(false, __LINE__, "Egress speed file line %d: network and gateway IP addresses must be of the same IP version: line is ignored",
          lineNumber);
      reportedErrorsLocal = true;
      continue;
    }

    /* check no IPv6 */
    if ((networkSet && networkIpVersion == AF_INET6) || //
        (gatewaySet && gatewayIpVersion == AF_INET6)) {
      egressFileError(false, __LINE__, "Egress speed file line %d: network and gateway IP addresses must not be IPv6 addresses: line is ignored", lineNumber);
      reportedErrorsLocal = true;
      continue;
    }

    /* ensure network is masked by netmask */
    if (networkSet) {
      /* assumes IPv4 */
      in_addr_t mask = (network.prefix_len == 0) ? 0 : (~0U << (32 - network.prefix_len));
      uint32_t masked = ntohl(network.prefix.v4.s_addr) & mask;
      network.prefix.v4.s_addr = htonl(masked);
    }

    if (!uplink || !downlink) {
      egressBwClear(&egress_if->bwCurrent, egress_if->upCurrent);
    } else {
      egress_if->bwCurrent.requireNetwork = requireNetwork;
      egress_if->bwCurrent.requireGateway = requireGateway;
      egress_if->bwCurrent.egressUk = uplink;
      egress_if->bwCurrent.egressDk = downlink;
      egress_if->bwCurrent.path_cost = pathCosts;
      egress_if->bwCurrent.network = network;
      egress_if->bwCurrent.gateway = gateway;

      egress_if->bwCurrent.networkSet = networkSet;
      egress_if->bwCurrent.gatewaySet = gatewaySet;

      egressBwCalculateCosts(&egress_if->bwCurrent, egress_if->upCurrent);
    }

    egress_if->inEgressFile = true;
  }

  fclose(fp);
  fp = NULL;

  reportedErrors = reportedErrorsLocal;

  outerror:

  /* clear absent egress interfaces and setup 'changed' status */
  {
    struct sgw_egress_if * egress_if = olsr_cnf->smart_gw_egress_interfaces;
    while (egress_if) {
      if (!egress_if->inEgressFile) {
        egressBwClear(&egress_if->bwCurrent, egress_if->upCurrent);
      }

      egress_if->bwCostsChanged = egressBwCostsChanged(egress_if);
      egress_if->bwNetworkChanged = egressBwNetworkChanged(egress_if);
      egress_if->bwGatewayChanged = egressBwGatewayChanged(egress_if);
      egress_if->bwChanged = egressBwChanged(egress_if);
      if (egress_if->bwChanged) {
        changed = true;
      }

      egress_if = egress_if->next;
    }
  }

  out: if (fp) {
    fclose(fp);
  }
  if (fd >= 0) {
    close(fd);
  }
  return changed;
}

#endif /* __linux__ */
