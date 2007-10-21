/*
 * HTTP Info plugin for the olsr.org OLSR daemon
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: admin_html.h,v 1.5 2007/10/20 20:41:04 bernd67 Exp $
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */



#ifndef ADMIN_HTML_H
#define ADMIN_HTML_H

static const char admin_frame_prolog[] =
    "<strong>Administrator interface</strong><hr>\n"
    "<h2>Change basic settings</h2>\n"
    "<form action=\"set_values\" method=\"post\">\n"
    "<table width=\"100%%\">\n";

static const char admin_frame_mid[] =
    "</table>\n<br>\n"
    "<center><input type=\"submit\" value=\"Submit\" class=\"input_button\">\n"
    "<input type=\"reset\" value=\"Reset\" class=\"input_button\"></center>\n"
    "</form>\n"
    "<h2>Add/remove local HNA entries</h2>\n"
    "<form action=\"set_values\" method=\"post\">\n"
    "<table width=\"100%%\"><tr><td><strong>Network:</strong></td>\n"
    "<td><input type=\"text\" name=\"hna_new_net\" maxlength=\"16\" class=\"input_text\" value=\"0.0.0.0\"></td>\n"
    "<td><strong>Netmask/Prefix:</strong></td>\n"
    "<td><input type=\"text\" name=\"hna_new_netmask\" maxlength=\"16\" class=\"input_text\" value=\"0.0.0.0\"></td>\n"
    "<td><input type=\"submit\" value=\"Add entry\" class=\"input_button\"></td></form>\n"
    "</table><hr>\n"
    "<form action=\"set_values\" method=\"post\">\n"
    "<table width=\"100%%\">\n"
  "<tr><th width=50 halign=\"middle\">Delete</th><th>Network</th><th>Netmask</th></tr>\n";

static const char admin_frame_epilog[] =
    "</table>\n<br>\n"
    "<center><input type=\"submit\" value=\"Delete selected\" class=\"input_button\"></center>\n"
    "</form>\n";

static const char admin_basic_setting_int[] = "<td><strong>%s</strong></td>\n<td> <input type=\"text\" name=\"%s\" maxlength=\"%d\" class=\"input_text\" value=\"%d\"></td>\n";
static const char admin_basic_setting_float[] = "<td><strong>%s</strong></td>\n<td> <input type=\"text\" name=\"%s\" maxlength=\"%d\" class=\"input_text\" value=\"%0.2f\"></td>\n";
static const char admin_basic_setting_string[] = "<td><strong>%s</strong></td>\n<td> <input type=\"text\" name=\"%s\" maxlength=\"%d\" class=\"input_text\" value=\"%s\"></td>\n";

#endif
