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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

extern GtkWidget *net_label;
extern GtkWidget *info_label;
extern GtkTextBuffer *textBuffer;
extern GtkWidget *connect_button;
extern GtkWidget *packet_list;
extern GtkWidget *packet_content_list;
extern GtkWidget *packet_button;
extern GtkWidget *packet_disp_button;
extern GtkWidget *node_list;
extern GtkWidget *mid_list;
extern GtkWidget *mpr_list;
extern GtkWidget *hna_list;
extern GtkWidget *route_list;

void fill_clist(GtkCList *);

void selection_made(GtkWidget * clist, gint row, gint column, GdkEventButton * event, gpointer data);

void set_net_info_connecting(void);

void column_clicked_callback(GtkWidget *, gint);

void connect_callback(GtkWidget * widget, gpointer data);

void packet_callback(GtkWidget * widget, gpointer data);

void packet_disp_callback(GtkWidget * widget, gpointer data);

void packet_selection(GtkWidget * clist, gint row, gint column, GdkEventButton * event, gpointer data);

void node_selection(GtkWidget * clist, gint row, gint column, GdkEventButton * event, gpointer data);

void gui_shutdown(GtkObject *, gpointer);

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
