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

#include "common.h"
#include "ipc.h"
#include "main.h"

struct sockaddr_in pin;

int
main(int argc, char *argv[])
{
  struct hostent *hp;
  struct in_addr in;

#ifdef _WIN32
  WSADATA WsaData;
#endif /* _WIN32 */
  GtkWidget *main_window;

#ifdef _WIN32
  if (WSAStartup(0x0202, &WsaData)) {
    fprintf(stderr, "Could not initialize WinSock.\n");
    exit(EXIT_FAILURE);
  }
#endif /* _WIN32 */

  /* Get IP */
  if ((hp = gethostbyname(argc > 1 ? argv[1] : "localhost")) == 0) {
    fprintf(stderr, "Not a valid host \"%s\"\n", argv[1]);
    exit(EXIT_FAILURE);
  }

  in.s_addr = ((struct in_addr *)(void *)(hp->h_addr))->s_addr;
  printf("Address: %s\n", inet_ntoa(in));

  /* fill in the socket structure with host information */
  memset(&pin, 0, sizeof(pin));
  pin.sin_family = AF_INET;
  pin.sin_addr.s_addr = ((struct in_addr *)(void *)(hp->h_addr))->s_addr;
  pin.sin_port = htons(IPC_PORT);

  gtk_init(&argc, &argv);

  init_nodes();

  freeze_packets = 1;
  display_dec = 1;

  /* "Failsafe" values */
  ipversion = AF_INET;
  ipsize = sizeof(struct in_addr);

  main_window = create_main_window();
  gtk_widget_show(main_window);

  printf("Done building GUI\n");

  memset(&main_addr, 0, sizeof(union olsr_ip_addr));
  memset(&null_addr, 0, sizeof(union olsr_ip_addr));

  /* Terminate signal */
  signal(SIGINT, shutdown_);

  /* Init node timeout */
  nodes_timeout = NEIGHB_HOLD_TIME_NW;
  init_timer((olsr_u32_t) (nodes_timeout * 1000), &hold_time_nodes);

  ipc_connect(&pin);

  add_timeouts();

  gtk_main();
  return 0;
}

/*
 *Timeouts
 */

int
add_timeouts(void)
{

  /*
   *Check socket for messages every IPC_INTERVAL
   *milliseconds
   */
  gtk_timeout_add(IPC_INTERVAL, ipc_timeout, NULL);

  /*
   *Time out nodes
   */
  timeouts = 5;                 /* Wait 5 times befor starting timing out nodes */
  gtk_timeout_add(TOP_HOLD_TIME, time_out_nodes, NULL);

  return 1;
}

gint
ipc_timeout(gpointer data __attribute__((unused)))
{

  ipc_read();
  return 1;
}

__attribute__((noreturn))
void
shutdown_(int sig)
{
  int errNr = errno;
  printf("Cleaning up...\n");

  if (ipc_close() < 0)
    printf("Could not close socket!\n");

  printf("BYE-BYE!\n");
  errno = errNr;
  exit(sig);
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
