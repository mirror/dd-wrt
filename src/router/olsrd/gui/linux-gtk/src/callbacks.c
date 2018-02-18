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
#include "interface.h"

void
selection_made(GtkWidget * clist, gint row, gint column __attribute__((unused)),
		GdkEventButton * event __attribute__((unused)),
		gpointer data __attribute__((unused)))
{
  gchar *ip, *hops, *gw, *dev;
  const gchar text[100];
  /* Get the text that is stored in the selected row and column
   * which was clicked in. We will receive it as a pointer in the
   * argument text.
   */
  gtk_clist_get_text(GTK_CLIST(clist), row, 0, &ip);
  gtk_clist_get_text(GTK_CLIST(clist), row, 1, &gw);
  gtk_clist_get_text(GTK_CLIST(clist), row, 2, &hops);
  gtk_clist_get_text(GTK_CLIST(clist), row, 3, &dev);

  /* Just prints some information about the selected row */
  sprintf((char *)&text[0], "IP:%s\nGATEWAY:%s\nHOPCOUNT:%s\nINTERFACE:%s\n", ip, gw, hops, dev);

  //gtk_text_buffer_set_text (textBuffer, text, -1);

  //gtk_text_view_set_buffer((GtkTextView *)text1, textBuffer);

  //gtk_widget_show (text1);

  return;
}

void
node_selection(GtkWidget * clist __attribute__((unused)), gint row,
		gint column __attribute__((unused)),
		GdkEventButton * event __attribute__((unused)),
		gpointer data __attribute__((unused)))
{

  gchar *text;
  struct node *host;
  struct mid *mids;
  struct mpr *mprs;
  struct hna *hnas;
  int i;
  gchar *tmpshit[1] = { (gchar *)"" };

  /*
   *Clear the lists
   */
  gtk_clist_clear(GTK_CLIST(mid_list));
  gtk_clist_clear(GTK_CLIST(mpr_list));
  gtk_clist_clear(GTK_CLIST(hna_list));
  i = 0;

  /*
   *Get th IP address
   */
  gtk_clist_get_text(GTK_CLIST(node_list), row, 0, &text);

  /* Get the node */
  if (strncmp(text, "local", sizeof("local")) == 0)
    host = find_node_t(&main_addr);
  else
    host = find_node(text);

  if (host) {
    /* Timeout the registered MPRs for this node */
    time_out_mprs(&host->addr);

    /* Get mpr pointer AFTER timeout....(another waisted hour...) */
    mprs = host->mpr.next;
    mids = host->mid.next;
    hnas = host->hna.next;

    while (mids != &host->mid) {
      gtk_clist_append(GTK_CLIST(mid_list), tmpshit);
      gtk_clist_set_text(GTK_CLIST(mid_list), i, 0, ip_to_string(&mids->alias));
      i++;
      mids = mids->next;
    }

    i = 0;

    while (mprs != &host->mpr) {
      //printf("ADDING MPR : %s\n", ip_to_string(&mprs->addr));fflush(stdout);
      gtk_clist_append(GTK_CLIST(mpr_list), tmpshit);
      gtk_clist_set_text(GTK_CLIST(mpr_list), i, 0, ip_to_string(&mprs->addr));
      i++;
      mprs = mprs->next;
    }
    i = 0;

    while (hnas != &host->hna) {
      gtk_clist_append(GTK_CLIST(hna_list), tmpshit);
      gtk_clist_set_text(GTK_CLIST(hna_list), i, 0, ip_to_string(&hnas->net));
      i++;
      hnas = hnas->next;
    }

  } else {
    printf("Could not find info about %s!\n", text);
  }

}

void
packet_selection(GtkWidget * clist __attribute__((unused)), gint row,
		gint column __attribute__((unused)),
		GdkEventButton * event __attribute__((unused)),
		gpointer data __attribute__((unused)))
{
  /* Fetch the packet from the cache */
  union olsr_message *pack;
  char *packet;
  int y, x;
  short size;
  char *content[4];
  int mem_size = 10;

  content[0] = (char *)malloc(mem_size);
  content[1] = (char *)malloc(mem_size);
  content[2] = (char *)malloc(mem_size);
  content[3] = (char *)malloc(mem_size);

  pack = get_packet(row);
  packet = (char *)pack;

  //printf("Got the packet at row %d...\n", row);

  gtk_clist_clear(GTK_CLIST(packet_content_list));

  size = ntohs(pack->v4.olsr_msgsize);

  for (y = 0; y < size; y += 4) {

    for (x = 0; x < 4; x++) {
      if (display_dec)
        sprintf(content[x], "%03i", (u_char) packet[y + x]);    /* Decimal format */
      else
        sprintf(content[x], "%02x", (u_char) packet[y + x]);    /* Hex format */
    }

    gtk_clist_append(GTK_CLIST(packet_content_list), content);
  }

  free(content[0]);
  free(content[1]);
  free(content[2]);
  free(content[3]);

}

void
column_clicked_callback(GtkWidget * list __attribute__((unused)),
		gint column __attribute__((unused)))
{

  //printf("You pressed %d\n",column);

}

extern struct sockaddr_in pin;

/*
 *Connect button callback
 */
void
connect_callback(GtkWidget * widget __attribute__((unused)),
		gpointer data __attribute__((unused)))
{
  ipc_connect(&pin);
}

/*
 *Packet button callback
 */
void
packet_callback(GtkWidget * widget __attribute__((unused)),
		gpointer data __attribute__((unused)))
{
  if (freeze_packets) {
    freeze_packets = 0;
    gtk_button_set_label(GTK_BUTTON(packet_button), "Freeze packets");
  } else {
    freeze_packets = 1;
    gtk_button_set_label(GTK_BUTTON(packet_button), "Grab packets");
  }
}

/*
 *Packet display button callback
 */
void
packet_disp_callback(GtkWidget * widget __attribute__((unused)),
		gpointer data __attribute__((unused)))
{
  if (display_dec) {
    display_dec = 0;
    gtk_button_set_label(GTK_BUTTON(packet_disp_button), "Display decimal");
  } else {
    display_dec = 1;
    gtk_button_set_label(GTK_BUTTON(packet_disp_button), "Display hex");
  }
}

__attribute__((noreturn))
void
gui_shutdown(GtkObject * object __attribute__((unused)),
		gpointer user_data __attribute__((unused)))
{
  printf("Shutting down...\n");

  if (ipc_close() < 0)
    printf("Could not close socket!\n");

  printf("BYE-BYE!\n");
  exit(EXIT_SUCCESS);

}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
