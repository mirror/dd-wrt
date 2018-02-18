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

#include "plugin_util.h"
#include "olsr.h"
#include "defs.h"

#include <arpa/inet.h>

int
set_plugin_port(const char *value, void *data, set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  char *endptr;
  const unsigned int port = strtoul(value, &endptr, 0);
  if (*endptr != '\0' || endptr == value) {
    OLSR_PRINTF(0, "Illegal port number \"%s\"", value);
    return 1;
  }
  if (port > 65535) {
    OLSR_PRINTF(0, "Port number %u out of range", port);
    return 1;
  }
  if (data != NULL) {
    int *v = data;
    *v = port;
    OLSR_PRINTF(1, "%s port number %u\n", "Got", port);
  } else {
    OLSR_PRINTF(0, "%s port number %u\n", "Ignored", port);
  }
  return 0;
}

int
set_plugin_ipaddress(const char *value, void *data, set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  char buf[INET6_ADDRSTRLEN];
  union olsr_ip_addr ip_addr;
  if (inet_pton(olsr_cnf->ip_version, value, &ip_addr) <= 0) {
    OLSR_PRINTF(0, "Illegal IP address \"%s\"", value);
    return 1;
  }
  inet_ntop(olsr_cnf->ip_version, &ip_addr, buf, sizeof(buf));
  if (data != NULL) {
    union olsr_ip_addr *v = data;
    *v = ip_addr;
    OLSR_PRINTF(1, "%s IP address %s\n", "Got", buf);
  } else {
    OLSR_PRINTF(0, "%s IP address %s\n", "Ignored", buf);
  }
  return 0;
}

/**
 * CAREFUL: this functions sets a boolean in an integer!!!
 */
int
set_plugin_boolean(const char *value, void *data, set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  bool *v = data;
  if (strcasecmp(value, "yes") == 0 || strcasecmp(value, "true") == 0) {
    *v = 1;
  } else if (strcasecmp(value, "no") == 0 || strcasecmp(value, "false") == 0) {
    *v = 0;
  } else {
    return 1;
  }
  return 0;
}

int
set_plugin_int(const char *value, void *data, set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  char *endptr;
  const int theint = strtol(value, &endptr, 0);
  if (*endptr != '\0' || endptr == value) {
    OLSR_PRINTF(0, "Illegal int \"%s\"", value);
    return 1;
  }
  if (data != NULL) {
    int *v = data;
    *v = theint;
    OLSR_PRINTF(1, "%s int %d\n", "Got", theint);
  } else {
    OLSR_PRINTF(0, "%s int %d\n", "Ignored", theint);
  }
  return 0;
}

int
set_plugin_long(const char *value, void *data, set_plugin_parameter_addon addon __attribute__ ((unused)))
{
  char *endptr;
  const long thelong = strtol(value, &endptr, 0);
  if (*endptr != '\0' || endptr == value) {
    OLSR_PRINTF(0, "Illegal long \"%s\"", value);
    return 1;
  }
  if (data != NULL) {
    long *v = data;
    *v = thelong;
    OLSR_PRINTF(1, "%s long %ld\n", "Got", thelong);
  } else {
    OLSR_PRINTF(0, "%s long %ld\n", "Ignored", thelong);
  }
  return 0;
}

int
set_plugin_string(const char *value, void *data, set_plugin_parameter_addon addon)
{
  if (data != NULL) {
    char *v = data;
    if ((unsigned)strlen(value) >= addon.ui) {
      OLSR_PRINTF(0, "String too long \"%s\"", value);
      return 1;
    }
    strscpy(v, value, addon.ui);
    OLSR_PRINTF(1, "%s string %s\n", "Got", value);
  } else {
    OLSR_PRINTF(0, "%s string %s\n", "Ignored", value);
  }
  return 0;
}

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
