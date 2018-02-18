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

#include "olsrd_conf_checksum.h"

#include "olsrd_conf.h"
#include "../superfasthash.h"

#include <stdio.h>

#include <string.h>

static uint32_t configuration_checksum;
static char configuration_checksum_str[(sizeof(configuration_checksum) * 2) + 1];

#define CLI_START     "*CLI START*"
#define CLI_START_LEN (sizeof(CLI_START) - 1)
#define CLI_END       "*CLI END*"
#define CLI_END_LEN   (sizeof(CLI_END) - 1)

void olsrd_config_checksum_init(void) {
  configuration_checksum = 0;
  memset(configuration_checksum_str, 0, sizeof(configuration_checksum_str));
}

void olsrd_config_checksum_final(void) {
  snprintf(configuration_checksum_str, sizeof(configuration_checksum_str), "%08x", configuration_checksum);
  configuration_checksum_str[sizeof(configuration_checksum_str) - 1] = '\0';
}

void olsrd_config_checksum_get(size_t *len, char ** str) {
  if (len) {
    *len = sizeof(configuration_checksum);
  }

  if (str) {
    *str = configuration_checksum_str;
  }
}

void olsrd_config_checksum_add_cli(int argc, char *argv[]) {
  int i = 1;

  if (!argc || !argv) {
    return;
  }

  olsrd_config_checksum_add(CLI_START, CLI_START_LEN);

  for (i = 0; i < argc; ++i) {
    if (!argv[i]) {
      break;
    }

    olsrd_config_checksum_add(argv[i], strlen(argv[i]));
  }

  olsrd_config_checksum_add(CLI_END, CLI_END_LEN);
}

void olsrd_config_checksum_add(const char *str, size_t len) {
  if (!str || !len) {
    return;
  }

  configuration_checksum = hash_inc(str, len, configuration_checksum);
  configuration_checksum = hash_inc("\n", 1, configuration_checksum);
}
