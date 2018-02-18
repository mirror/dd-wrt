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

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */

#include "olsr.h"
#include "olsrd_httpinfo.h"
#include "olsr_cfg.h"
#include "admin_interface.h"
#include "net_olsr.h"
#include "ipcalc.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char admin_basic_setting_int[] =
  "<td><strong>%s</strong></td><td><input type=\"text\" name=\"%s\" maxlength=\"%d\" class=\"input_text\" value=\"%d\"></td>\n";
static const char admin_basic_setting_float[] =
  "<td><strong>%s</strong></td><td><input type=\"text\" name=\"%s\" maxlength=\"%d\" class=\"input_text\" value=\"%0.2f\"></td>\n";
static const char admin_basic_setting_string[] =
  "<td><strong>%s</strong></td><td><input type=\"text\" name=\"%s\" maxlength=\"%d\" class=\"input_text\" value=\"%s\"></td>\n";

static const char admin_frame_prolog[] =
  "<strong>Administrator interface</strong><hr>\n" "<h2>Change basic settings</h2>\n"
  "<form action=\"set_values\" method=\"post\">\n" "<table width=\"100%%\">\n";

static const char admin_frame_mid[] =
  "</table>\n<br>\n" "<center><input type=\"submit\" value=\"Submit\" class=\"input_button\">\n"
  "<input type=\"reset\" value=\"Reset\" class=\"input_button\"></center>\n" "</form>\n" "<h2>Add/remove local HNA entries</h2>\n"
  "<form action=\"set_values\" method=\"post\">\n" "<table width=\"100%%\"><tr><td><strong>Network:</strong></td>\n"
  "<td><input type=\"text\" name=\"hna_new_net\" maxlength=\"16\" class=\"input_text\" value=\"0.0.0.0\"></td>\n"
  "<td><strong>Netmask/Prefix:</strong></td>\n"
  "<td><input type=\"text\" name=\"hna_new_netmask\" maxlength=\"16\" class=\"input_text\" value=\"0.0.0.0\"></td>\n"
  "<td><input type=\"submit\" value=\"Add entry\" class=\"input_button\"></td></form>\n" "</table><hr>\n"
  "<form action=\"set_values\" method=\"post\">\n" "<table width=\"100%%\">\n"
  "<tr><th width=50 halign=\"middle\">Delete</th><th>Network</th><th>Netmask</th></tr>\n";

static const char admin_frame_epilog[] =
  "</table>\n<br>\n" "<center><input type=\"submit\" value=\"Delete selected\" class=\"input_button\"></center>\n" "</form>\n";

int
build_admin_body(char *buf, uint32_t bufsize __attribute__ ((unused)))
{
  int size = 0;
  size += snprintf(&buf[size], bufsize - size, admin_frame_prolog);

  size += snprintf(&buf[size], bufsize - size, "<tr>\n");

  size += snprintf(&buf[size], bufsize - size, admin_basic_setting_int, "Debug level:", "debug_level", 2, olsr_cnf->debug_level);
  size += snprintf(&buf[size], bufsize - size, admin_basic_setting_float, "Pollrate:", "pollrate", 4, (double)olsr_cnf->pollrate);
  size += snprintf(&buf[size], bufsize - size, admin_basic_setting_string, "TOS:", "tos", 6, "TBD");

  size += snprintf(&buf[size], bufsize - size, "</tr>\n" "<tr>\n");

  size +=
    snprintf(&buf[size], bufsize - size, admin_basic_setting_int, "TC redundancy:", "tc_redundancy", 1, olsr_cnf->tc_redundancy);
  size += snprintf(&buf[size], bufsize - size, admin_basic_setting_int, "MPR coverage:", "mpr_coverage", 1, olsr_cnf->mpr_coverage);
  size += snprintf(&buf[size], bufsize - size, admin_basic_setting_int, "Willingness:", "willingness", 1, olsr_cnf->willingness);

  size += snprintf(&buf[size], bufsize - size, "</tr>\n" "<tr>\n");

  if (olsr_cnf->use_hysteresis) {
    size +=
      snprintf(&buf[size], bufsize - size, admin_basic_setting_float, "Hyst scaling:", "hyst_scaling", 4,
    		  (double)olsr_cnf->hysteresis_param.scaling);

    size +=
      snprintf(&buf[size], bufsize - size, admin_basic_setting_float, "Lower thr:", "hyst_lower", 4,
    		  (double)olsr_cnf->hysteresis_param.thr_low);
    size +=
      snprintf(&buf[size], bufsize - size, admin_basic_setting_float, "Upper thr:", "hyst_upper", 4,
    		  (double)olsr_cnf->hysteresis_param.thr_high);
  } else {
    size += snprintf(&buf[size], bufsize - size, "<td>Hysteresis disabled</td>\n");
  }

  size += snprintf(&buf[size], bufsize - size, "</tr>\n" "<tr>\n");

  if (olsr_cnf->lq_level) {
    size += snprintf(&buf[size], bufsize - size, admin_basic_setting_int, "LQ level:", "lq_level", 1, olsr_cnf->lq_level);
    size += snprintf(&buf[size], bufsize - size, admin_basic_setting_float, "LQ aging:", "lq_aging", 2, (double)olsr_cnf->lq_aging);
  } else {
    size += snprintf(&buf[size], bufsize - size, "<td>LQ disabled</td>\n");
  }

  size += snprintf(&buf[size], bufsize - size, "</tr>\n" "<tr>\n");
  size += snprintf(&buf[size], bufsize - size, "</tr>\n");

  size += snprintf(&buf[size], bufsize - size, admin_frame_mid);

  if (olsr_cnf->hna_entries) {
    struct ip_prefix_list *hna;
    for (hna = olsr_cnf->hna_entries; hna; hna = hna->next) {
      struct ipaddr_str netbuf;
      olsr_ip_to_string(&netbuf, &hna->net.prefix);
      size +=
        snprintf(&buf[size], bufsize - size,
                 "<tr><td halign=\"middle\"><input type=\"checkbox\" name=\"del_hna%s*%d\" class=\"input_checkbox\"></td><td>%s</td><td>%d</td></tr>\n",
                 netbuf.buf, hna->net.prefix_len, netbuf.buf, hna->net.prefix_len);
    }
  }
  size += snprintf(&buf[size], bufsize - size, admin_frame_epilog);
  return size;
}

#ifdef ADMIN_INTERFACE

int
process_param(char *key, char *value)
{
  static union olsr_ip_addr curr_hna_net;
  static bool curr_hna_ok = false;

  if (!strcmp(key, "debug_level")) {
    int ival = atoi(value);
    if ((ival < 0) || (ival > 9))
      return -1;

    olsr_cnf->debug_level = ival;
    return 1;
  }

  if (!strcmp(key, "tc_redundancy")) {
    int ival = atoi(value);
    if ((ival < 0) || (ival > 3))
      return -1;

    olsr_cnf->tc_redundancy = ival;
    return 1;
  }

  if (!strcmp(key, "mpr_coverage")) {
    int ival = atoi(value);
    if (ival < 0)
      return -1;

    olsr_cnf->mpr_coverage = ival;
    return 1;
  }

  if (!strcmp(key, "willingness")) {
    int ival = atoi(value);
    if ((ival < 0) || (ival > 7))
      return -1;

    olsr_cnf->willingness = ival;
    return 1;
  }

  if (!strcmp(key, "lq_level")) {
    int ival = atoi(value);
    if ((ival < 0) || (ival > 2))
      return -1;

    olsr_cnf->lq_level = ival;
    return 1;
  }

  if (!strcmp(key, "lq_wsize")) {
    int ival = atoi(value);
    if ((ival < 0) || (ival > 10))
      return -1;

    olsr_cnf->lq_wsize = ival;
    return 1;
  }

  if (!strcmp(key, "hyst_scaling")) {
    float fval = 1.1;
    sscanf(value, "%f", &fval);
    if ((fval < 0.0) || (fval > 1.0))
      return -1;

    olsr_cnf->hysteresis_param.scaling = fval;
    return 1;
  }

  if (!strcmp(key, "hyst_scaling")) {
    float fval = 1.1;
    sscanf(value, "%f", &fval);
    if ((fval < 0.0) || (fval > 1.0))
      return -1;

    olsr_cnf->hysteresis_param.scaling = fval;
    return 1;
  }

  if (!strcmp(key, "hyst_lower")) {
    float fval = 1.1;
    sscanf(value, "%f", &fval);
    if ((fval < 0.0) || (fval > 1.0))
      return -1;

    olsr_cnf->hysteresis_param.thr_low = fval;
    return 1;
  }

  if (!strcmp(key, "hyst_upper")) {
    float fval = 1.1;
    sscanf(value, "%f", &fval);
    if ((fval < 0.0) || (fval > 1.0))
      return -1;

    olsr_cnf->hysteresis_param.thr_high = fval;
    return 1;
  }

  if (!strcmp(key, "pollrate")) {
    float fval = 1.1;
    sscanf(value, "%f", &fval);
    if ((fval < 0.0) || (fval > 1.0))
      return -1;

    olsr_cnf->pollrate = fval;
    return 1;
  }

  if (!strcmp(key, "hna_new_net")) {
    if (inet_pton(olsr_cnf->ipsize, value, &curr_hna_net.v4) == 0) {
      fprintf(stderr, "Failed converting new HNA net %s\n", value);
      return -1;
    }
    curr_hna_ok = true;
    return 1;
  }

  if (!strcmp(key, "hna_new_netmask")) {
    struct in_addr in;
    uint8_t prefixlen;

    if (!curr_hna_ok)
      return -1;

    curr_hna_ok = false;

    if (inet_aton(value, &in) == 0) {
      fprintf(stderr, "Failed converting new HNA netmask %s\n", value);
      return -1;
    }
    prefixlen = netmask_to_prefix((uint8_t *) & in, olsr_cnf->ipsize);
    if (prefixlen == UCHAR_MAX) {
      fprintf(stderr, "Failed converting new HNA netmask %s\n", value);
      return -1;
    }
    ip_prefix_list_add(&olsr_cnf->hna_entries, &curr_hna_net, prefixlen);
    return 1;
  }

  if (!strncmp(key, "del_hna", 7) && !strcmp(value, "on")) {
    struct in_addr net, mask;
    char ip_net[16], ip_mask[16];
    int seperator = 0;
    uint8_t prefixlen;

    while (key[7 + seperator] != '*') {
      seperator++;
    }
    memcpy(ip_net, &key[7], seperator);
    ip_net[seperator] = 0;
    memcpy(ip_mask, &key[7 + seperator + 1], 16);
    olsr_printf(1, "Deleting HNA %s/%s\n", ip_net, ip_mask);

    if (inet_aton(ip_net, &net) == 0) {
      fprintf(stderr, "Failed converting HNA net %s for deletion\n", ip_net);
      return -1;
    }

    if (inet_aton(ip_mask, &mask) == 0) {
      fprintf(stderr, "Failed converting HNA netmask %s for deletion\n", ip_mask);
      return -1;
    }
    prefixlen = netmask_to_prefix((uint8_t *) & mask, olsr_cnf->ipsize);
    if (prefixlen == UCHAR_MAX) {
      fprintf(stderr, "Failed converting new HNA netmask %s\n", value);
      return -1;
    }
    ip_prefix_list_add(&olsr_cnf->hna_entries, &curr_hna_net, prefixlen);
    return 1;
  }

  return 0;
}

int
process_set_values(char *data, uint32_t data_size, char *buf, uint32_t bufsize __attribute__ ((unused)))
{
  int size = 0;
  int val_start, key_start;
  uint32_t i;

  size += sprintf(buf, "<html>\n<head><title>olsr.org httpinfo plugin</title></head>\n<body>\n");

  key_start = 0;
  val_start = 0;

  for (i = 0; i < data_size; i++) {
    if (data[i] == '=') {
      data[i] = '\0';
      val_start = i + 1;
    }

    if (data[i] == '&') {
      data[i] = '\0';
      if (!process_param(&data[key_start], &data[val_start])) {
        size +=
          snprintf(&buf[size], bufsize - size, "<h2>FAILED PROCESSING!</h2><br>Key: %s Value: %s<br>\n", &data[key_start],
                   &data[val_start]);
        return -1;
      }

      key_start = i + 1;
    }
  }

  if (!process_param(&data[key_start], &data[val_start])) {
    size +=
      snprintf(&buf[size], bufsize - size, "<b>FAILED PROCESSING!</b><br>Key: %s Value: %s<br>\n", &data[key_start],
               &data[val_start]);
    return -1;
  }

  size +=
    snprintf(&buf[size], bufsize - size,
             "<h2>UPDATE SUCESSFULL!</h2><br>Press BACK and RELOAD in your browser to return to the plugin<br>\n</body>\n</html>\n");
  size += snprintf(&buf[size], bufsize - size, "\n</body>\n</html>\n");

  return size;
}
#endif /* ADMIN_INTERFACE */

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
