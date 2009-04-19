
/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * Copyright (c) 2004, Thomas Lopatic (thomas@olsr.org)
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

#include "src/link.h"
#include "src/plugin.h"
#include "src/lib.h"
#include "src/os_unix.h"
#include "src/http.h"
#include "src/glua.h"
#include "src/glua_ext.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

static void sigHand(int sig) __attribute__ ((noreturn));
static void
sigHand(int sig __attribute__ ((unused)))
{
  httpShutdown();
  exit(0);
}

static void
usage(void)
{
  fprintf(stderr, "usage: tas [--address ip-address] [--port port-number]\n");
  fprintf(stderr, "           [--work-dir work-directory] [--root-dir root-directory]\n");
  fprintf(stderr, "           [--password password] [--user user] [--sess-time session-timeout]\n");
  fprintf(stderr, "           [--index-file index-file] [--pub-dir public-directory]\n");
  fprintf(stderr, "           [--quantum quantum] [--mess-time message-timeout]\n");
  fprintf(stderr, "           [--mess-limit message-queue-limit]\n");
}

int
main(int ac, char *av[])
{
  int i;

  httpInit();

  for (i = 1; i < ac; i++) {
    if (strcmp(av[i], "--address") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing address parameter\n");
        usage();
        return 1;
      }

      if (httpSetAddress(av[i], NULL, (set_plugin_parameter_addon) {
                         0}
          ) < 0) {
        fprintf(stderr, "cannot set address\n");
        return 1;
      }
    }

    else if (strcmp(av[i], "--port") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing port parameter\n");
        usage();
        return 1;
      }

      if (httpSetPort(av[i], NULL, (set_plugin_parameter_addon) {
                      0}
          ) < 0) {
        fprintf(stderr, "cannot set port\n");
        return 1;
      }
    }

    else if (strcmp(av[i], "--work-dir") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing work directory parameter\n");
        usage();
        return 1;
      }

      if (httpSetWorkDir(av[i], NULL, (set_plugin_parameter_addon) {
                         0}
          ) < 0) {
        fprintf(stderr, "cannot set work directory\n");
        return 1;
      }
    }

    else if (strcmp(av[i], "--root-dir") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing root directory parameter\n");
        usage();
        return 1;
      }

      if (httpSetRootDir(av[i], NULL, (set_plugin_parameter_addon) {
                         0}
          ) < 0) {
        fprintf(stderr, "cannot set root directory\n");
        return 1;
      }
    }

    else if (strcmp(av[i], "--index-file") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing index file parameter\n");
        usage();
        return 1;
      }

      httpSetIndexFile(av[i], NULL, (set_plugin_parameter_addon) {
                       0}
      );
    }

    else if (strcmp(av[i], "--user") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing user parameter\n");
        usage();
        return 1;
      }

      httpSetUser(av[i], NULL, (set_plugin_parameter_addon) {
                  0}
      );
    }

    else if (strcmp(av[i], "--password") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing password parameter\n");
        usage();
        return 1;
      }

      httpSetPassword(av[i], NULL, (set_plugin_parameter_addon) {
                      0}
      );
    }

    else if (strcmp(av[i], "--sess-time") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing timeout parameter\n");
        usage();
        return 1;
      }

      if (httpSetSessTime(av[i], NULL, (set_plugin_parameter_addon) {
                          0}
          ) < 0) {
        fprintf(stderr, "cannot set session timeout\n");
        return 1;
      }
    }

    else if (strcmp(av[i], "--pub-dir") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing public directory parameter\n");
        usage();
        return 1;
      }

      httpSetPubDir(av[i], NULL, (set_plugin_parameter_addon) {
                    0}
      );
    }

    else if (strcmp(av[i], "--quantum") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing quantum parameter\n");
        usage();
        return 1;
      }

      if (httpSetQuantum(av[i], NULL, (set_plugin_parameter_addon) {
                         0}
          ) < 0) {
        fprintf(stderr, "cannot set quantum\n");
        return 1;
      }
    }

    else if (strcmp(av[i], "--mess-time") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing timeout parameter\n");
        usage();
        return 1;
      }

      if (httpSetMessTime(av[i], NULL, (set_plugin_parameter_addon) {
                          0}
          ) < 0) {
        fprintf(stderr, "cannot set message timeout\n");
        return 1;
      }
    }

    else if (strcmp(av[i], "--mess-limit") == 0) {
      if (++i == ac) {
        fprintf(stderr, "missing limit parameter\n");
        usage();
        return 1;
      }

      if (httpSetMessLimit(av[i], NULL, (set_plugin_parameter_addon) {
                           0}
          ) < 0) {
        fprintf(stderr, "cannot set message queue limit\n");
        return 1;
      }
    }

    else {
      fprintf(stderr, "invalid argument: %s\n", av[i]);
      usage();
      return 1;
    }
  }

  signal(SIGINT, sigHand);
  signal(SIGTERM, sigHand);

  if (httpSetup() < 0) {
    fprintf(stderr, "cannot set up HTTP server\n");
    return 1;
  }

  while (1) {
    if (httpService(10) < 0) {
      fprintf(stderr, "cannot run HTTP server\n");
      return 1;
    }

    usleep(100000);
  }

  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
