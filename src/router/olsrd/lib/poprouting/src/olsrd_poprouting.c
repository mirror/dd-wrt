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

#include "olsrd_poprouting.h"

#include <unistd.h>

#include "info/info_types.h"
#include "info/http_headers.h"
#include "olsr.h"
#include "scheduler.h"

float timer = 0.0f;
float hello_mult = 10.0f;
float tc_mult = 60.0f;

unsigned long long get_supported_commands_mask(void) {
  return SIW_POPROUTING;
}

bool isCommand(const char *str, unsigned long long siw) {
  bool r;
  const char *prefix, s[2] = "=";
  char *string, *cmd, *s_timer;
  string = strdup(str);
  switch (siw) {
    case SIW_POPROUTING_HELLO:
      prefix = "/helloTimer";
      break;

    case SIW_POPROUTING_TC:
      prefix = "/tcTimer";
      break;

    case SIW_POPROUTING_HELLO_MULT:
      prefix = "/helloTimerMult";
      break;

    case SIW_POPROUTING_TC_MULT:
      prefix = "/tcTimerMult";
      break;

    default:
      return false;
  }
  cmd = strtok(string, s);
  s_timer = strtok(NULL, s);
  if (s_timer) {
    timer = atof(s_timer);
  } else {
    timer = 0;
  }

  r = !strcmp(cmd, prefix) && (timer >= 0);

  free(string);

  return r;
}

void output_error(struct autobuf *abuf, unsigned int status, const char * req __attribute__((unused)), bool http_headers) {
  if (http_headers || (status == INFO_HTTP_OK)) {
    return;
  }

  /* !http_headers && !INFO_HTTP_OK */

  if (status == INFO_HTTP_NOCONTENT) {
    /* wget can't handle output of zero length */
    abuf_puts(abuf, "\n");
  } else {
    abuf_appendf(abuf, "error: %s\n", httpStatusToReply(status));
  }
}

void set_hello_timer(struct autobuf *abuf) {
  struct olsr_if *in;
  assert(abuf);
  in = olsr_cnf->interfaces;
  if (timer != 0.0f) {
    char response[20];
    sprintf(response, "hello:%.2f\n", (double) in->cnf->hello_params.emission_interval);
    abuf_puts(abuf, response);
    return;
  }

  while (in != NULL ) {
    olsr_printf(1, "(POPROUTING) Setting Hello Timer=%f for interface %s\n", (double) timer, in->name);
    //olsr_change_timer(in->interf->hello_gen_timer, timer * MSEC_PER_SEC, 0, 1);
    in->interf->hello_gen_timer->timer_period = timer * MSEC_PER_SEC;
    in->interf->hello_gen_timer->timer_jitter_pct = POPROUTING_JITTER; // Jitter to 5%
    in->cnf->hello_params.emission_interval = timer;
    in->cnf->hello_params.validity_time = timer * hello_mult;
    in->interf->valtimes.hello = reltime_to_me(in->cnf->hello_params.validity_time * MSEC_PER_SEC);
    in->interf->hello_etime = in->cnf->hello_params.emission_interval * MSEC_PER_SEC;
    in = in->next;
  }
  abuf_puts(abuf, "0\n");
  return;
}

void set_tc_timer(struct autobuf *abuf) {
  struct olsr_if *in;
  in = olsr_cnf->interfaces;
  if (timer != 0.0f) {
    char response[20];
    sprintf(response, "tc:%.2f\n", (double) in->cnf->tc_params.emission_interval);
    abuf_puts(abuf, response);
    return;
  }

  while (in != NULL ) {
    olsr_printf(1, "(POPROUTING) Setting Tc Timer=%f for interface %s\n", (double) timer, in->name);
    //olsr_change_timer(in->interf->tc_gen_timer, timer * MSEC_PER_SEC, 0, 1);
    in->interf->tc_gen_timer->timer_period = timer * MSEC_PER_SEC;
    in->interf->tc_gen_timer->timer_jitter_pct = POPROUTING_JITTER; // Jitter to 5%
    in->cnf->tc_params.emission_interval = timer;
    in->cnf->tc_params.validity_time = timer * tc_mult;
    in->interf->valtimes.tc = reltime_to_me(in->cnf->tc_params.validity_time * MSEC_PER_SEC);
    in = in->next;
  }
  abuf_puts(abuf, "0\n");
  return;
}

void set_tc_timer_mult(struct autobuf *abuf) {
  float current_timer;
  struct olsr_if *in;
  assert(abuf);
  in = olsr_cnf->interfaces;
  if (timer != 0.0f) {
    char response[20];
    sprintf(response, "tc_mult:%.2f\n", (double) tc_mult);
    abuf_puts(abuf, response);
    return;
  } else {
    tc_mult = timer;
  }

  while (in != NULL ) {
    olsr_printf(1, "(POPROUTING) Setting Tc Timer Mult=%f for interface %s\n", (double) tc_mult, in->name);
    current_timer = in->cnf->tc_params.emission_interval;
    in->cnf->tc_params.validity_time = current_timer * tc_mult;
    in->interf->valtimes.tc = reltime_to_me(in->cnf->tc_params.validity_time * MSEC_PER_SEC);
    in = in->next;
  }
  abuf_puts(abuf, "0\n");
  return;
}

void set_hello_timer_mult(struct autobuf *abuf) {
  float current_timer;
  struct olsr_if *in;
  assert(abuf);
  in = olsr_cnf->interfaces;
  if (timer != 0.0f) {
    char response[20];
    sprintf(response, "hello_mult:%.2f\n", (double) hello_mult);
    abuf_puts(abuf, response);
    return;
  } else {
    hello_mult = timer;
  }

  while (in != NULL ) {
    olsr_printf(1, "(POPROUTING) Setting Hello Timer Mult=%f for interface %s\n", (double) hello_mult, in->name);
    current_timer = in->cnf->hello_params.emission_interval;
    in->cnf->hello_params.validity_time = current_timer * hello_mult;
    in->interf->valtimes.hello = reltime_to_me(in->cnf->hello_params.validity_time * MSEC_PER_SEC);
    in = in->next;
  }
  abuf_puts(abuf, "0\n");
  return;
}
