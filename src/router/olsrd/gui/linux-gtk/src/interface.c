
/*
 * OLSR ad-hoc routing table management protocol GUI front-end
 * Copyright (C) 2003 Andreas Tonnesen (andreto@ifi.uio.no)
 *
 * This file is part of olsr.org.
 *
 * uolsrGUI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * uolsrGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with olsr.org; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "common.h"
#include "interface.h"
#include "pixmaps.h"

/* Global widgets */

/* GtkWidget *text1; */

GdkBitmap *mask;

int packet_list_size = 0;
int route_list_size = 0;
int node_list_size = 0;

GtkWidget *main_window;

GtkWidget *
create_main_window(void)
{
  GtkWidget *notebook1;
  GtkWidget *frame3;
  GtkWidget *frame4;
  GtkWidget *hbox1;
  GtkWidget *vbox1;
  GtkWidget *scrolledwindow1;
  GtkWidget *node_label1;
  GtkWidget *node_label2;
  GtkWidget *node_label3;
  GtkWidget *node_label4;
  GtkWidget *node_label7;
  GtkWidget *node_label8;
  GtkWidget *node_label9;
  GtkWidget *mid_frame;
  GtkWidget *mpr_frame;
  GtkWidget *hna_frame;
  GtkWidget *mid_scrolledwindow;
  GtkWidget *mpr_scrolledwindow;
  GtkWidget *hna_scrolledwindow;
  GtkWidget *Main;
  GtkWidget *label_routes;
  GtkWidget *hbox2;
  GtkWidget *frame2;
  GtkWidget *scrolledwindow4;
  GtkWidget *label17;
  GtkWidget *label18;
  GtkWidget *label19;
  GtkWidget *scrolledwindow3;
  GtkWidget *label13;
  GtkWidget *label14;
  GtkWidget *label15;
  GtkWidget *label16;
  GtkWidget *label_packets;
  //GtkWidget *empty_notebook_page2;
  GtkWidget *label3;
  GtkWidget *net_vbox;
  GtkWidget *pack_vbox;
  GtkWidget *pack_disp_vbox;
  GtkWidget *disp_frame;
  GtkWidget *route_frame;
  GtkWidget *route_stats_frame;
  GtkWidget *route_scrolledwindow;
  GtkWidget *route_label1;
  GtkWidget *route_label2;
  GtkWidget *route_label3;
  GtkWidget *route_label4;
  GtkWidget *route_hbox1;

  GtkWidget *traffic_label;

  GtkWidget *settings_label;
  GtkWidget *settings_hbox1;

  GtkWidget *about_hbox1;
  GtkWidget *about_label;

  GtkWidget *empty1;

  GdkPixmap *unik_logo_gdk;
  GtkWidget *unik_logo;

  /*
   *The main window
   */

  main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data(GTK_OBJECT(main_window), "main_window", main_window);
  gtk_window_set_title(GTK_WINDOW(main_window), (olsrd_version));
  gtk_window_set_default_size(GTK_WINDOW(main_window), 600, 550);
  gtk_signal_connect(GTK_OBJECT(main_window), "destroy", GTK_SIGNAL_FUNC(gui_shutdown),
                     //GTK_SIGNAL_FUNC(gtk_main_quit),
                     NULL);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER); /* Position window in center */
  gtk_window_set_policy(GTK_WINDOW(main_window), FALSE, TRUE, TRUE);    /* No user-resizing */

  /*
   *Initialize the pixmaps
   */
  unik_logo_gdk =
    gdk_pixmap_colormap_create_from_xpm_d(NULL, gtk_widget_get_colormap(main_window), &mask, NULL, (gchar **) logo_xpm);

  unik_logo = gtk_pixmap_new(unik_logo_gdk, mask);

  /*
   *The notebook
   */

  notebook1 = gtk_notebook_new();
  gtk_widget_ref(notebook1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "notebook1", notebook1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(notebook1);
  gtk_container_add(GTK_CONTAINER(main_window), notebook1);

  /*
   *The first vbox
   */
  vbox1 = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(vbox1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "vbox1", vbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(vbox1);
  //gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 1);
  gtk_container_add(GTK_CONTAINER(notebook1), vbox1);

  /*
   *The nodes frame
   */
  frame3 = gtk_frame_new("Registered nodes:");
  gtk_widget_ref(frame3);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "frame3", frame3, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(frame3);
  gtk_box_pack_start(GTK_BOX(vbox1), frame3, TRUE, TRUE, 0);
  gtk_widget_set_size_request(frame3, -1, 300);
  gtk_container_set_border_width(GTK_CONTAINER(frame3), 1);

  /*
   *The scrolled window to contain the node list
   */

  scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_ref(scrolledwindow1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "scrolledwindow1", scrolledwindow1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(scrolledwindow1);
  gtk_container_add(GTK_CONTAINER(frame3), scrolledwindow1);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  //gtk_box_pack_start (GTK_BOX (frame3), scrolledwindow1, TRUE, TRUE, 0);
  //gtk_widget_set_usize (scrolledwindow1, -2, 332);

  /*
   *The node list
   */

  node_list = gtk_clist_new(7);
  gtk_widget_ref(node_list);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "node_list", node_list, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_list);
  gtk_container_add(GTK_CONTAINER(scrolledwindow1), node_list);
  gtk_clist_set_column_width(GTK_CLIST(node_list), 0, 150);     /* IP */
  //gtk_clist_set_column_justification(GTK_CLIST(node_list), 0, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(node_list), 1, 150);     /* gateway */
  //gtk_clist_set_column_justification(GTK_CLIST(node_list), 1, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(node_list), 2, 50);      /* hopcount */
  gtk_clist_set_column_justification(GTK_CLIST(node_list), 2, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(node_list), 3, 80);      /* dev */
  gtk_clist_set_column_justification(GTK_CLIST(node_list), 3, GTK_JUSTIFY_CENTER);

  gtk_clist_set_column_width(GTK_CLIST(node_list), 4, 70);      /* timer */
  gtk_clist_set_column_justification(GTK_CLIST(node_list), 4, GTK_JUSTIFY_CENTER);
  //gtk_clist_set_column_width (GTK_CLIST (node_list), 7, 100); /* last about */
  //gtk_clist_set_column_justification(GTK_CLIST(node_list), 7, GTK_JUSTIFY_CENTER);

  gtk_clist_set_column_width(GTK_CLIST(node_list), 5, 40);      /* MID */
  gtk_clist_set_column_justification(GTK_CLIST(node_list), 5, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(node_list), 6, 40);      /* HNA */
  gtk_clist_set_column_justification(GTK_CLIST(node_list), 6, GTK_JUSTIFY_CENTER);

  gtk_clist_column_titles_show(GTK_CLIST(node_list));

  /*
   *Row selection callback
   */
  gtk_signal_connect(GTK_OBJECT(node_list), "select_row", GTK_SIGNAL_FUNC(selection_made), NULL);

  /*
   *Column selection callback
   */
  gtk_signal_connect(GTK_OBJECT(node_list), "click_column", GTK_SIGNAL_FUNC(column_clicked_callback), NULL);

  node_label1 = gtk_label_new("Dest");
  gtk_widget_ref(node_label1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "IP", node_label1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_label1);
  gtk_clist_set_column_widget(GTK_CLIST(node_list), 0, node_label1);

  node_label2 = gtk_label_new("Gateway");
  gtk_widget_ref(node_label2);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "hops", node_label2, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_label2);
  gtk_clist_set_column_widget(GTK_CLIST(node_list), 1, node_label2);

  node_label3 = gtk_label_new("Metric");
  gtk_widget_ref(node_label3);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "info", node_label3, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_label3);
  gtk_clist_set_column_widget(GTK_CLIST(node_list), 2, node_label3);

  node_label4 = gtk_label_new("Device");
  gtk_widget_ref(node_label4);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "Device", node_label4, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_label4);
  gtk_clist_set_column_widget(GTK_CLIST(node_list), 3, node_label4);

  node_label7 = gtk_label_new("Timer");
  gtk_widget_ref(node_label7);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "LMF", node_label7, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_label7);
  gtk_clist_set_column_widget(GTK_CLIST(node_list), 4, node_label7);

  /*
     node_label8 = gtk_label_new ("LMA");
     gtk_widget_ref (node_label8);
     gtk_object_set_data_full (GTK_OBJECT (main_window), "LMA", node_label8,
     (GtkDestroyNotify) gtk_widget_unref);
     gtk_widget_show (node_label8);
     gtk_clist_set_column_widget (GTK_CLIST (node_list), 7, node_label8);
   */

  node_label8 = gtk_label_new("MID");
  gtk_widget_ref(node_label8);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "MID", node_label8, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_label8);
  gtk_clist_set_column_widget(GTK_CLIST(node_list), 5, node_label8);

  node_label9 = gtk_label_new("HNA");
  gtk_widget_ref(node_label9);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "HNA", node_label9, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(node_label9);
  gtk_clist_set_column_widget(GTK_CLIST(node_list), 6, node_label9);

  gtk_clist_column_titles_active(GTK_CLIST(node_list));

  gtk_widget_show_now(node_list);

  /*
   *Row selection callback
   */
  gtk_signal_connect(GTK_OBJECT(node_list), "select_row", GTK_SIGNAL_FUNC(node_selection), NULL);

  /*
   *The first hbox
   */
  hbox1 = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(hbox1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "hbox1", hbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(hbox1);
  gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
  //gtk_container_add (GTK_CONTAINER (notebook1), hbox1);
  gtk_container_set_border_width(GTK_CONTAINER(hbox1), 4);
  gtk_widget_set_size_request(hbox1, -1, 200);

  /*
   *The net-info frame
   */
  frame4 = gtk_frame_new("Info:");
  gtk_widget_ref(frame4);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "frame4", frame4, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(frame4);
  gtk_box_pack_start(GTK_BOX(hbox1), frame4, TRUE, TRUE, 0);
  //gtk_widget_set_size_request(frame4, 200, -1);
  gtk_container_set_border_width(GTK_CONTAINER(frame4), 1);

  /*
   *The net-info hbox
   */
  net_vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(net_vbox);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "net_vbox", net_vbox, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(net_vbox);
  gtk_container_add(GTK_CONTAINER(frame4), net_vbox);

  /*
   *The net-info label field
   */
  net_label = gtk_label_new(NULL);
  gtk_widget_ref(net_label);
  gtk_misc_set_alignment((GtkMisc *) net_label, 0, 0);
  //gtk_label_set_justify((GtkLabel *)net_label,GTK_JUSTIFY_LEFT);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "net_label", net_label, (GtkDestroyNotify) gtk_widget_unref);

  //set_net_info("Not connected...");
  gtk_widget_show(net_label);
  gtk_box_pack_start(GTK_BOX(net_vbox), net_label, TRUE, TRUE, 0);

  //gtk_container_add (GTK_CONTAINER (frame4), net_label);

  /*
   *The connect button
   */

  connect_button = gtk_button_new_with_label("Connect to host");
  gtk_widget_ref(connect_button);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "connect_button", connect_button, (GtkDestroyNotify) gtk_widget_unref);
  /* Connect the "clicked" signal of the button to our callback */
  gtk_signal_connect(GTK_OBJECT(connect_button), "clicked", GTK_SIGNAL_FUNC(connect_callback), NULL);
  gtk_widget_show(connect_button);
  gtk_box_pack_start(GTK_BOX(net_vbox), connect_button, FALSE, FALSE, 1);
  gtk_container_set_border_width(GTK_CONTAINER(connect_button), 5);

  /*
   *The node MPR info frame
   */
  mpr_frame = gtk_frame_new("MPR:");
  gtk_widget_ref(mpr_frame);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "mpr_frame", mpr_frame, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(mpr_frame);
  gtk_box_pack_start(GTK_BOX(hbox1), mpr_frame, FALSE, FALSE, 0);
  //gtk_widget_set_size_request(mid_frame, 125, -1);
  gtk_container_set_border_width(GTK_CONTAINER(mpr_frame), 1);

  /*
   *The scrolledwindow to contain the MPR node info
   */
  mpr_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_ref(mpr_scrolledwindow);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "mpr_scrolledwindow", mpr_scrolledwindow, (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show(mpr_scrolledwindow);
  gtk_container_add(GTK_CONTAINER(mpr_frame), mpr_scrolledwindow);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(mpr_scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width(GTK_CONTAINER(mpr_scrolledwindow), 3);

  /*
   *The node MID info frame
   */
  mid_frame = gtk_frame_new("MID:");
  gtk_widget_ref(mid_frame);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "mid_frame", mid_frame, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(mid_frame);
  gtk_box_pack_start(GTK_BOX(hbox1), mid_frame, FALSE, FALSE, 0);
  //gtk_widget_set_size_request(mid_frame, 125, -1);
  gtk_container_set_border_width(GTK_CONTAINER(mid_frame), 1);

  /*
   *The scrolledwindow to contain the MID node info
   */
  mid_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_ref(mid_scrolledwindow);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "mid_scrolledwindow", mid_scrolledwindow, (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show(mid_scrolledwindow);
  gtk_container_add(GTK_CONTAINER(mid_frame), mid_scrolledwindow);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(mid_scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width(GTK_CONTAINER(mid_scrolledwindow), 3);

  /*
   *The MPR list
   */
  mpr_list = gtk_clist_new(1);
  gtk_widget_ref(mpr_list);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "mpr_list", mpr_list, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(mpr_list);
  gtk_container_add(GTK_CONTAINER(mpr_scrolledwindow), mpr_list);
  gtk_clist_set_column_width(GTK_CLIST(mpr_list), 0, 120);      /* IP */
  gtk_clist_column_titles_hide(GTK_CLIST(mpr_list));

  /*
   *The MID list
   */
  mid_list = gtk_clist_new(1);
  gtk_widget_ref(mid_list);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "mid_list", mid_list, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(mid_list);
  gtk_container_add(GTK_CONTAINER(mid_scrolledwindow), mid_list);
  gtk_clist_set_column_width(GTK_CLIST(mid_list), 0, 120);      /* IP */
  gtk_clist_column_titles_hide(GTK_CLIST(mid_list));

  /*
   *The node HNA info frame
   */
  hna_frame = gtk_frame_new("HNA:");
  gtk_widget_ref(hna_frame);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "hna_frame", hna_frame, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(hna_frame);
  gtk_box_pack_start(GTK_BOX(hbox1), hna_frame, FALSE, FALSE, 0);
  //gtk_widget_set_size_request(mid_frame, 125, -1);
  gtk_container_set_border_width(GTK_CONTAINER(hna_frame), 1);

  /*
   *The HNA scrolled window
   */
  hna_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_ref(hna_scrolledwindow);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "hna_scrolledwindow", hna_scrolledwindow, (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show(hna_scrolledwindow);
  gtk_container_add(GTK_CONTAINER(hna_frame), hna_scrolledwindow);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(hna_scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_set_border_width(GTK_CONTAINER(hna_scrolledwindow), 3);

  /*
   *The HNA list
   */
  hna_list = gtk_clist_new(1);
  gtk_widget_ref(hna_list);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "hna_list", hna_list, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(hna_list);
  gtk_container_add(GTK_CONTAINER(hna_scrolledwindow), hna_list);
  gtk_clist_set_column_width(GTK_CLIST(hna_list), 0, 120);      /* IP */
  gtk_clist_column_titles_hide(GTK_CLIST(hna_list));

  /*
   *The "main" notebook page
   */
  Main = gtk_label_new("Main");
  gtk_widget_ref(Main);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "Main", Main, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(Main);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook1), 0), Main);

  /*
   *The main hbox of the Packet page
   */

  hbox2 = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(hbox2);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "hbox2", hbox2, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(hbox2);
  gtk_container_add(GTK_CONTAINER(notebook1), hbox2);

  /*
   *The packet hbox
   */
  pack_vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(pack_vbox);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "pack_vbox", pack_vbox, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(pack_vbox);
  gtk_box_pack_start(GTK_BOX(hbox2), pack_vbox, TRUE, TRUE, 0);

  /*
   *The packet frame
   */

  frame2 = gtk_frame_new("Packet");
  gtk_widget_ref(frame2);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "frame2", frame2, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(frame2);
  gtk_box_pack_start(GTK_BOX(pack_vbox), frame2, TRUE, TRUE, 0);        /* Do not expand */

  /*
   *Packet list scrolled window
   */
  scrolledwindow4 = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_ref(scrolledwindow4);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "scrolledwindow4", scrolledwindow4, (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show(scrolledwindow4);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow4), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  gtk_container_add(GTK_CONTAINER(frame2), scrolledwindow4);

  /*
   *The packet list
   */

  packet_list = gtk_clist_new(3);
  gtk_widget_ref(packet_list);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "packet_list", packet_list, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(packet_list);
  gtk_container_add(GTK_CONTAINER(scrolledwindow4), packet_list);
  gtk_clist_set_column_width(GTK_CLIST(packet_list), 0, 80);    /* Type */
  gtk_clist_set_column_width(GTK_CLIST(packet_list), 1, 150);   /* Origin IP */
  gtk_clist_set_column_width(GTK_CLIST(packet_list), 2, 20);    /* size */
  gtk_clist_column_titles_show(GTK_CLIST(packet_list));

  label17 = gtk_label_new("Type");
  gtk_widget_ref(label17);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label17", label17, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label17);
  gtk_clist_set_column_widget(GTK_CLIST(packet_list), 0, label17);

  label18 = gtk_label_new("Origin");
  gtk_widget_ref(label18);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label18", label18, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label18);
  gtk_clist_set_column_widget(GTK_CLIST(packet_list), 1, label18);

  label19 = gtk_label_new("Size");
  gtk_widget_ref(label19);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label19", label19, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label19);
  gtk_clist_set_column_widget(GTK_CLIST(packet_list), 2, label19);

  /*
   *Row selection callback
   */
  gtk_signal_connect(GTK_OBJECT(packet_list), "select_row", GTK_SIGNAL_FUNC(packet_selection), NULL);

  /*
   *The packet button
   */

  packet_button = gtk_button_new_with_label("Grab packets");
  gtk_widget_ref(packet_button);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "packet_button", packet_button, (GtkDestroyNotify) gtk_widget_unref);

  /* Connect the "clicked" signal of the button to our callback */
  gtk_signal_connect(GTK_OBJECT(packet_button), "clicked", GTK_SIGNAL_FUNC(packet_callback), NULL);
  gtk_widget_show(packet_button);
  gtk_box_pack_start(GTK_BOX(pack_vbox), packet_button, FALSE, FALSE, 5);

  /*
   *The packet disp hbox
   */
  pack_disp_vbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_ref(pack_disp_vbox);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "pack_disp_vbox", pack_disp_vbox, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(pack_disp_vbox);
  gtk_box_pack_start(GTK_BOX(hbox2), pack_disp_vbox, TRUE, TRUE, 0);

  /*
   *The packet disp frame
   */

  disp_frame = gtk_frame_new("Packet content");
  gtk_widget_ref(disp_frame);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "disp_frame", disp_frame, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(disp_frame);
  gtk_box_pack_start(GTK_BOX(pack_disp_vbox), disp_frame, TRUE, TRUE, 0);       /* Do not expand */

  /*
   *Scrolled window for the packet display
   *list
   */

  scrolledwindow3 = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_ref(scrolledwindow3);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "scrolledwindow3", scrolledwindow3, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(scrolledwindow3);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow3), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  gtk_container_add(GTK_CONTAINER(disp_frame), scrolledwindow3);

  //gtk_box_pack_start (GTK_BOX (disp_frame), scrolledwindow3, TRUE, TRUE, 0);

  /*
   *The packet display list
   */
  packet_content_list = gtk_clist_new(4);
  gtk_widget_ref(packet_content_list);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "packet_content_list", packet_content_list,
                           (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show(packet_content_list);
  gtk_container_add(GTK_CONTAINER(scrolledwindow3), packet_content_list);
  gtk_clist_set_column_width(GTK_CLIST(packet_content_list), 0, 70);    /* 0-7 */
  gtk_clist_set_column_justification(GTK_CLIST(packet_content_list), 0, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(packet_content_list), 1, 70);    /* 8-15 */
  gtk_clist_set_column_justification(GTK_CLIST(packet_content_list), 1, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(packet_content_list), 2, 70);    /* 16-23 */
  gtk_clist_set_column_justification(GTK_CLIST(packet_content_list), 2, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(packet_content_list), 3, 70);    /* 24-31 */
  gtk_clist_set_column_justification(GTK_CLIST(packet_content_list), 3, GTK_JUSTIFY_CENTER);
  gtk_clist_column_titles_show(GTK_CLIST(packet_content_list));

  label13 = gtk_label_new("0 - 7");
  gtk_widget_ref(label13);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label13", label13, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label13);
  gtk_clist_set_column_widget(GTK_CLIST(packet_content_list), 0, label13);

  label14 = gtk_label_new("8 - 15");
  gtk_widget_ref(label14);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label14", label14, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label14);
  gtk_clist_set_column_widget(GTK_CLIST(packet_content_list), 1, label14);

  label15 = gtk_label_new("16 - 23");
  gtk_widget_ref(label15);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label15", label15, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label15);
  gtk_clist_set_column_widget(GTK_CLIST(packet_content_list), 2, label15);

  label16 = gtk_label_new("24 - 31");
  gtk_widget_ref(label16);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label16", label16, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label16);
  gtk_clist_set_column_widget(GTK_CLIST(packet_content_list), 3, label16);

  //gtk_clist_set_selection_mode(GTK_CLIST (packet_content_list), GTK_SELECTION_NONE); /* no selections */

  /*
   *The packet button
   */

  packet_disp_button = gtk_button_new_with_label("Display hex");
  gtk_widget_ref(packet_disp_button);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "packet_disp_button", packet_disp_button, (GtkDestroyNotify) gtk_widget_unref);

  /* Connect the "clicked" signal of the button to our callback */
  gtk_signal_connect(GTK_OBJECT(packet_disp_button), "clicked", GTK_SIGNAL_FUNC(packet_disp_callback), NULL);
  gtk_widget_show(packet_disp_button);
  gtk_box_pack_start(GTK_BOX(pack_disp_vbox), packet_disp_button, FALSE, FALSE, 5);

  /*
   *The "packets" notebook
   */

  label_packets = gtk_label_new("Packets");
  gtk_widget_ref(label_packets);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label_packets", label_packets, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label_packets);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook1), 1), label_packets);

  /*
   *The route hbox
   */
  route_hbox1 = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(route_hbox1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_hbox1", route_hbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_hbox1);
  //gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 1);
  gtk_container_add(GTK_CONTAINER(notebook1), route_hbox1);

  /*
   *The routes frame
   */

  route_frame = gtk_frame_new("OLSR routes in kernel:");
  gtk_widget_ref(route_frame);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_frame", route_frame, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_frame);

  //gtk_container_add (GTK_CONTAINER (notebook1), route_frame);
  gtk_widget_set_size_request(route_frame, 200, -1);
  gtk_box_pack_start(GTK_BOX(route_hbox1), route_frame, TRUE, TRUE, 0); /* Do not expand */

  /*
   *Scrolled window for the packet display
   *list
   */

  route_scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_ref(route_scrolledwindow);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_scrolledwindow", route_scrolledwindow,
                           (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_scrolledwindow);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(route_scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  gtk_container_add(GTK_CONTAINER(route_frame), route_scrolledwindow);

  //gtk_box_pack_start (GTK_BOX (route_frame), scrolledwindow3, TRUE, TRUE, 0);

  /*
   *The routes display list
   */
  route_list = gtk_clist_new(4);
  gtk_widget_ref(route_list);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_list", route_list, (GtkDestroyNotify) gtk_widget_unref);

  gtk_widget_show(route_list);
  gtk_container_add(GTK_CONTAINER(route_scrolledwindow), route_list);
  gtk_clist_set_column_width(GTK_CLIST(route_list), 0, 120);    /* dest */
  //gtk_clist_set_column_justification(GTK_CLIST (route_list), 0, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(route_list), 1, 120);    /* gw */
  //gtk_clist_set_column_justification(GTK_CLIST (route_list), 1, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(route_list), 2, 50);     /* weight */
  gtk_clist_set_column_justification(GTK_CLIST(route_list), 2, GTK_JUSTIFY_CENTER);
  gtk_clist_set_column_width(GTK_CLIST(route_list), 3, 70);     /* interface */
  gtk_clist_set_column_justification(GTK_CLIST(route_list), 3, GTK_JUSTIFY_CENTER);
  gtk_clist_column_titles_show(GTK_CLIST(route_list));

  route_label1 = gtk_label_new("Destination");
  gtk_widget_ref(route_label1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_label1", route_label1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_label1);
  gtk_clist_set_column_widget(GTK_CLIST(route_list), 0, route_label1);

  route_label2 = gtk_label_new("Gateway");
  gtk_widget_ref(route_label2);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_label2", route_label2, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_label2);
  gtk_clist_set_column_widget(GTK_CLIST(route_list), 1, route_label2);

  route_label3 = gtk_label_new("Weight");
  gtk_widget_ref(route_label3);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_label3", route_label3, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_label3);
  gtk_clist_set_column_widget(GTK_CLIST(route_list), 2, route_label3);

  route_label4 = gtk_label_new("Interface");
  gtk_widget_ref(route_label4);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_label4", route_label4, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_label4);
  gtk_clist_set_column_widget(GTK_CLIST(route_list), 3, route_label4);

  //gtk_clist_set_selection_mode(GTK_CLIST (route_list), GTK_SELECTION_NONE); /* no selections */

  /*
   *The routes stats frame
   */

  route_stats_frame = gtk_frame_new("Stats:");
  gtk_widget_ref(route_stats_frame);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "route_stats_frame", route_stats_frame, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(route_stats_frame);

  //gtk_container_add (GTK_CONTAINER (notebook1), route_frame);
  gtk_box_pack_start(GTK_BOX(route_hbox1), route_stats_frame, TRUE, TRUE, 1);

  /*
   *The "routes" notebook
   */
  label_routes = gtk_label_new("Routes");
  gtk_widget_ref(label_routes);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "label_routes", label_routes, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label_routes);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook1), 2), label_routes);

  empty1 = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(empty1);
  gtk_container_add(GTK_CONTAINER(notebook1), empty1);

  /*
   *The "traffic" notebook
   */
  traffic_label = gtk_label_new("Traffic");
  gtk_widget_ref(traffic_label);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "traffic_label", traffic_label, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(traffic_label);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook1), 3), traffic_label);

  /*
   *The settings hbox
   */
  settings_hbox1 = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(settings_hbox1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "settings_hbox1", settings_hbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(settings_hbox1);
  //gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 1);
  gtk_container_add(GTK_CONTAINER(notebook1), settings_hbox1);

  /*
   *The settings-info label field
   */
  info_label = gtk_label_new(NULL);
  gtk_widget_ref(info_label);
  gtk_misc_set_alignment((GtkMisc *) info_label, 0, 0);
  //gtk_label_set_justify((GtkLabel *)net_label,GTK_JUSTIFY_LEFT);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "info_label", info_label, (GtkDestroyNotify) gtk_widget_unref);

  //set_net_info("Not connected...");
  gtk_widget_show(info_label);
  gtk_box_pack_start(GTK_BOX(settings_hbox1), info_label, TRUE, TRUE, 0);

  /*
   *The "settings" notebook
   */
  settings_label = gtk_label_new("Settings");
  gtk_widget_ref(settings_label);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "settings_label", settings_label, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(settings_label);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook1), 4), settings_label);

  /*
   *The "about" hbox
   */
  about_hbox1 = gtk_hbox_new(FALSE, 0);
  gtk_widget_ref(about_hbox1);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "about_hbox1", about_hbox1, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(about_hbox1);
  //gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 1);
  gtk_container_add(GTK_CONTAINER(notebook1), about_hbox1);
  gtk_container_set_border_width(GTK_CONTAINER(about_hbox1), 10);

  /*
   *The about label field
   */
  about_label = gtk_label_new(NULL);
  gtk_widget_ref(about_label);
  gtk_misc_set_alignment((GtkMisc *) about_label, 0, 0);
  gtk_label_set_justify((GtkLabel *) about_label, GTK_JUSTIFY_CENTER);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "about_label", about_label, (GtkDestroyNotify) gtk_widget_unref);

  //set_net_info("Not connected...");
  gtk_widget_show(about_label);
  gtk_box_pack_start(GTK_BOX(about_hbox1), unik_logo, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(about_hbox1), about_label, TRUE, TRUE, 0);
  gtk_widget_show(unik_logo);

  gtk_label_set_text((GtkLabel *) about_label, "OLSRD-GUI by Andreas Tonnesen (andreto@ifi.uio.no)");

  /*
   *The "about" notebook
   */
  label3 = gtk_label_new("About");
  gtk_widget_ref(label3);
  gtk_object_set_data_full(GTK_OBJECT(main_window), "About", label3, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(label3);
  gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook1), gtk_notebook_get_nth_page(GTK_NOTEBOOK(notebook1), 5), label3);

  return main_window;

}

/*
 *Add a node to the node list
 */
void
route_list_add(char *dest, char *gw, char *metric, char *dev)
{
  gchar *tmp[4] = { dest, gw, dev, metric };
  route_list_size++;

  gtk_clist_freeze(GTK_CLIST(route_list));

  gtk_clist_append(GTK_CLIST(route_list), tmp);

  gtk_clist_thaw(GTK_CLIST(route_list));

}

/*
 *Update the entyr with IP 'addr'
 */
void
route_list_update(char *addr)
{

}

/*
 *Delete a node from the node list
 */
int
route_list_del(char *dest)
{
  int i = 0;
  char *ip;

  gtk_clist_freeze(GTK_CLIST(route_list));

  for (i = 0; i < route_list_size; i++) {
    gtk_clist_get_text(GTK_CLIST(route_list), i, 0, (gchar **) & ip);
    if (strcmp(dest, ip) == 0) {
      //printf("Found %d\n", i);
      gtk_clist_remove(GTK_CLIST(route_list), i);
      route_list_size--;
      gtk_clist_thaw(GTK_CLIST(route_list));
      return 1;
    }
  }

  gtk_clist_thaw(GTK_CLIST(route_list));
  return 0;
}

/*
 *Remove a node from the list
 */
int
remove_nodes_list(union olsr_ip_addr *node)
{
  char *ip;
  char *in_ip = ip_to_string(node);
  int i;

  for (i = 0; i < node_list_size; i++) {
    gtk_clist_get_text(GTK_CLIST(node_list), i, 0, (gchar **) & ip);
    if (strcmp(in_ip, ip) == 0) {
      //printf("Found entry!\n");
      gtk_clist_remove(GTK_CLIST(node_list), i);
      node_list_size--;
      return 1;
    }
  }

  return 0;
}

/*
 *If the node passed as a parameter exists then
 *update it. If not add it to the list
 */
void
update_nodes_list(struct node *node)
{
  int i = 0;
  char *ip;
  int found = 0;
  char *dest;
  char *tmp[9] = { "", "", "", "", "", "", "", "", "" };
  char timer[20];
  struct tm *time_st;
  char itoa_buf[10];

  if (memcmp(&node->addr, &main_addr, ipsize) == 0)
    dest = "local";
  else
    dest = ip_to_string(&node->addr);

  gtk_clist_freeze(GTK_CLIST(node_list));

  while ((i < node_list_size) && !found) {
    gtk_clist_get_text(GTK_CLIST(node_list), i, 0, (gchar **) & ip);
    if (strcmp(dest, ip) == 0)
      found = 1;
    i++;
  }

  /* Update node */
  if (found) {
    i--;                        /* Go backt to the right row */
    //printf("Updating %s\n\n", ip_to_string(&node->addr));
    /* don't update main addr */
    /* Gateway */
    if (memcmp(&node->addr, &main_addr, ipsize) != 0) {
      if (memcmp(&node->gw_addr, &null_addr, ipsize) != 0)
        gtk_clist_set_text(GTK_CLIST(node_list), i, 1, ip_to_string(&node->gw_addr));
      /* Weigth */
      if (node->hopcount != 0) {
        gui_itoa(node->hopcount, itoa_buf);
        gtk_clist_set_text(GTK_CLIST(node_list), i, 2, itoa_buf);
      }
      /* Device */
      gtk_clist_set_text(GTK_CLIST(node_list), i, 3, &node->dev[0]);
    }

    /* Timer */
    if (node->timer.tv_usec) {
      memset(&timer[0], 0, 20);
      time_st = localtime((time_t *) & node->timer.tv_sec);
      sprintf(&timer[0], "%02d:%02d:%02d", time_st->tm_hour, time_st->tm_min, time_st->tm_sec);
      gtk_clist_set_text(GTK_CLIST(node_list), i, 4, &timer[0]);
    }

    /* MID */
    if (node->mid.next != &node->mid)
      gtk_clist_set_text(GTK_CLIST(node_list), i, 5, "yes");
    else
      gtk_clist_set_text(GTK_CLIST(node_list), i, 5, "no");
    /* HNA */
    if (node->hna.next != &node->hna)
      gtk_clist_set_text(GTK_CLIST(node_list), i, 6, "yes");
    else
      gtk_clist_set_text(GTK_CLIST(node_list), i, 6, "no");

  }
  /* Add new node */
  else {
    i = node_list_size;
    /* Create entry */
    gtk_clist_insert(GTK_CLIST(node_list), i, tmp);
    /* Main address */
    gtk_clist_set_text(GTK_CLIST(node_list), i, 0, dest);
    if (memcmp(&node->addr, &main_addr, ipsize) == 0) {
      if (memcmp(&node->gw_addr, &null_addr, ipsize) != 0)
        gtk_clist_set_text(GTK_CLIST(node_list), i, 1, ip_to_string(&node->gw_addr));
      /* Weigth */
      if (node->hopcount != 0) {
        gui_itoa(node->hopcount, itoa_buf);
        gtk_clist_set_text(GTK_CLIST(node_list), i, 2, itoa_buf);
      }
      /* Device */
      gtk_clist_set_text(GTK_CLIST(node_list), i, 3, &node->dev[0]);
    }

    /* MID */
    if (node->mid.next != &node->mid)
      gtk_clist_set_text(GTK_CLIST(node_list), i, 5, "yes");
    else
      gtk_clist_set_text(GTK_CLIST(node_list), i, 5, "no");
    /* HNA */
    if (node->hna.next != &node->hna)
      gtk_clist_set_text(GTK_CLIST(node_list), i, 6, "yes");
    else
      gtk_clist_set_text(GTK_CLIST(node_list), i, 6, "no");

    node_list_size++;
  }

  gtk_clist_thaw(GTK_CLIST(node_list));

}

/*
 *Add a packet to the packet list
 */
void
packet_list_add(char *type, char *from, char *length)
{
  gchar *nfo[3] = { type, from, length };

  //if(!freeze_packets)
  //{
  if (packet_list_size >= MAXPACKS)
    gtk_clist_remove(GTK_CLIST(packet_list), MAXPACKS - 1);
  else
    packet_list_size++;

  gtk_clist_prepend(GTK_CLIST(packet_list), nfo);

  //}

}

void
set_net_info(gchar * info, int disp_button)
{
  gchar title[255];

  memset(&title[0], 0, 255);
  gtk_label_set_text((GtkLabel *) info_label, info);
  gtk_label_set_text((GtkLabel *) net_label, "Connected");

  strcat(title, olsrd_version);
  strcat(title, " - ");
  strcat(title, ip_to_string(&main_addr));

  gtk_window_set_title(GTK_WINDOW(main_window), title);

  if (disp_button)
    gtk_widget_show(connect_button);
  else
    gtk_widget_hide(connect_button);
}

void
set_net_info_offline()
{
  gtk_label_set_text((GtkLabel *) net_label, "Connection refused...");
  gtk_widget_show(connect_button);
}

void
set_net_info_connecting()
{
  gtk_label_set_text((GtkLabel *) net_label, "Connecting...");
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
