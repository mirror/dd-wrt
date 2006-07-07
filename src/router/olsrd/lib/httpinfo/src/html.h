/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
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
 * $Id: html.h,v 1.6 2005/03/14 21:28:15 kattemat Exp $
 */

/*
 * Dynamic linked library for the olsr.org olsr daemon
 */

#ifndef _HTML_H
#define _HTML_H

static const char *httpinfo_css[] =
{
  "A {text-decoration: none}\n",
  "TH{text-align: left}\n",
  "H1, H3, TD, TH {font-family: Helvetica; font-size: 80%%}\n",
  "h2\n {\nfont-family: Helvetica;\n font-size: 14px;text-align: center;\n",
  "line-height: 16px;\ntext-decoration: none;\nborder: 1px solid #ccc;\n",
  "margin: 5px;\nbackground: #ececec;\n}\n",
  "hr\n{\nborder: none;\npadding: 1px;\nbackground: url(grayline.gif) repeat-x bottom;\n}\n",
  "#maintable\n{\nmargin: 0px;\npadding: 5px;\nborder-left: 1px solid #ccc;\n",
  "border-right: 1px solid #ccc;\nborder-bottom: 1px solid #ccc;\n}\n",
  "#footer\n{\nfont-size: 10px;\nline-height: 14px;\ntext-decoration: none;\ncolor: #666;\n}\n",
  "#hdr\n{\nfont-size: 14px;\ntext-align: center;\nline-height: 16px;\n",
  "text-decoration: none;\nborder: 1px solid #ccc;\n",
  "margin: 5px;\nbackground: #ececec;\n}\n",
  "#container\n{\nwidth: 500px;\npadding: 30px;\nborder: 1px solid #ccc;\nbackground: #fff;\n}\n",
  "#tabnav\n{\nheight: 20px;\nmargin: 0;\npadding-left: 10px;\n",
  "background: url(grayline.gif) repeat-x bottom;\n}\n",
  "#tabnav li\n{\nmargin: 0;\npadding: 0;\ndisplay: inline;\nlist-style-type: none;\n}\n",
  "#tabnav a:link, #tabnav a:visited\n{\nfloat: left;\nbackground: #ececec;\n",
  "font-size: 12px;\nline-height: 14px;\nfont-weight: bold;\npadding: 2px 10px 2px 10px;\n",
  "margin-right: 4px;\nborder: 1px solid #ccc;\ntext-decoration: none;\ncolor: #777;\n}\n",
  "#tabnav a:link.active, #tabnav a:visited.active\n{\nborder-bottom: 1px solid #fff;\n",
  "background: #ffffff;\ncolor: #000;\n}\n",
  "#tabnav a:hover\n{\nbackground: #777777;\ncolor: #ffffff;\n}\n",
  ".input_text\n{\nbackground: #E5E5E5;\nmargin-left: 5px; margin-top: 0px;\n",
  "text-align: left;\n\nwidth: 100px;\npadding: 0px;\ncolor: #000000;\n",
  "text-decoration: none;\nfont-family: verdana;\nfont-size: 12px;\n",
  "border: 1px solid #ccc;\n}\n", 
  ".input_button\n{\nbackground: #B5D1EE;\nmargin-left: 5px;\nmargin-top: 0px;\n",
  "text-align: center;\nwidth: 120px;\npadding: 0px;\ncolor: #000000;\n",
  "text-decoration: none;\nfont-family: verdana;\nfont-size: 12px;\n",
  "border: 1px solid #000;\n}\n",
  NULL
};



static const char *http_ok_head[] =
{
  "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n",
  "<HEAD>\n",
  "<META http-equiv=\"Content-type\" content=\"text/html; charset=ISO-8859-1\">\n",
  "<TITLE>olsr.org httpinfo plugin</TITLE>\n",
#ifndef SVEN_OLA
  "<link rel=\"icon\" href=\"images/favicon.ico\" type=\"image/x-icon\">\n",
  "<link rel=\"shortcut icon\" href=\"images/favicon.ico\" type=\"image/x-icon\">\n",
#endif
  "<link rel=\"stylesheet\" type=\"text/css\" href=\"httpinfo.css\">\n",
  "</HEAD>\n",
  "<body bgcolor=\"#ffffff\" text=\"#000000\">\n",
#ifndef SVEN_OLA
  "<table align=\"center\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"800\">"
  "<tbody><tr bgcolor=\"#ffffff\">",
  "<td align=\"left\" height=\"69\" valign=\"middle\" width=\"80%\">",
  "<font color=\"black\" face=\"timesroman\" size=\"6\">&nbsp;&nbsp;&nbsp;olsr.org OLSR daemon</font></td>",
  "<td align=\"right\" height=\"69\" valign=\"middle\" width=\"20%\">",
  "<img src=\"/logo.gif\" alt=\"olsrd logo\"></td>",
  "</tr>",
  "<p>",
  "</table>",
  "<!-- END HEAD -->\n\n",
#endif
  NULL
};



static const char *html_tabs[] =
{
  "<table align=\"center\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"800\">\n",
  "<tr bgcolor=\"#ffffff\"><td>\n",
  "<ul id=\"tabnav\">\n",
  "<!-- TAB ELEMENTS -->",
  "<li><a href=\"%s\" %s>%s</a></li>\n",
  "</ul>\n",
  "</td></tr>\n",
  "<tr><td>\n",
  NULL
};





static const char *http_ok_tail[] =
{
    "\n<!-- START TAIL -->\n\n",
    "<div id=\"footer\">\n\n",
    "<p><center>\n",
    "(C)2005 Andreas T&oslash;nnesen<br>\n",
    "<a href=\"http://www.olsr.org/\">http://www.olsr.org</a>\n",
    "</center>\n",
    "</div>\n",
    "</body></html>\n",
    NULL
};

static const char *cfgfile_body[] =
{
    "\n\n",
    "<b>This is a automatically generated configuration\n",
    "file based on the current olsrd configuration of this node.<br>\n",
    "<hr>\n",
    "<pre>\n",
    "<!-- CFGFILE -->",
    "</pre>\n<hr>\n",
    NULL
};


static const char *about_frame[] =
{
    "<b>" PLUGIN_NAME " version " PLUGIN_VERSION "</b><br>\n"
    "by Andreas T&oslash;nnesen (C)2005.<br>\n",
#ifdef ADMIN_INTERFACE
    "Compiled <i>with experimental admin interface</i> " __DATE__ "<hr>\n"
#else
    "Compiled " __DATE__ "<hr>\n"
#endif
    "This plugin implements a HTTP server that supplies\n",
    "the client with various dynamic web pages representing\n",
    "the current olsrd status.<br>The different pages include:\n",
    "<ul>\n<li><b>Configuration</b> - This page displays information\n",
    "about the current olsrd configuration. This includes various\n",
    "olsr settings such as IP version, MID/TC redundancy, hysteresis\n",
    "etc. Information about the current status of the interfaces on\n",
    "which olsrd is configured to run is also displayed. Loaded olsrd\n",
    "plugins are shown with their plugin parameters. Finally all local\n",
    "HNA entries are shown. These are the networks that the local host\n",
    "will anounce itself as a gateway to.</li>\n",
    "<li><b>Routes</b> - This page displays all routes currently set in\n",
    "the kernel <i>by olsrd</i>. The type of route is also displayed(host\n",
    "or HNA).</li>\n",
    "<li><b>Links/Topology</b> - This page displays all information about\n",
    "links, neighbors, topology, MID and HNA entries.</li>\n",
    "<li><b>All</b> - Here all the previous pages are displayed as one.\n",
    "This is to make all information available as easy as possible(for example\n",
    "for a script) and using as few resources as possible.</li>\n",
#ifdef ADMIN_INTERFACE
    "<li><b>Admin</b> - This page is highly experimental(and unsecure)!\n",
    "As of now it is not working at all but it provides a impression of\n",
    "the future possibilities of httpinfo. This is to be a interface to\n",
    "changing olsrd settings in realtime. These settings include various\n"
    "\"basic\" settings and local HNA settings.\n",
#endif
    "<li><b>About</b> - this help page.</li>\n</ul>",
    "<hr>\n",
    "Send questions or comments to\n",
    "<a href=\"mailto:olsr-users@olsr.org\">olsr-users@olsr.org</a> or\n",
    "<a href=\"mailto:andreto-at-olsr.org\">andreto-at-olsr.org</a><br>\n"
    "Official olsrd homepage: <a href=\"http://www.olsr.org/\">http://www.olsr.org</a><br>\n",
    NULL
};



static const char *http_frame[] =
{
  "<div id=\"maintable\">\n",
  "<!-- BODY -->",
  "</div>\n",
  NULL
};


#endif
