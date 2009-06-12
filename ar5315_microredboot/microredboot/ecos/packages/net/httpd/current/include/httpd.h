#ifndef CYGONCE_NET_HTTPD_HTTPD_H
#define CYGONCE_NET_HTTPD_HTTPD_H
/* =================================================================
 *
 *      httpd.h
 *
 *      A simple embedded HTTP server
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating
 * System.
 * Copyright (C) 2002 Nick Garnett
 * 
 * eCos is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 or (at your option)
 * any later version.
 * 
 * eCos is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with eCos; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * As a special exception, if other files instantiate templates or
 * use macros or inline functions from this file, or you compile this
 * file and link it with other works to produce a work based on this
 * file, this file does not by itself cause the resulting work to be
 * covered by the GNU General Public License. However the source code
 * for this file must still be made available in accordance with
 * section (3) of the GNU General Public License.
 * 
 * This exception does not invalidate any other reasons why a work
 * based on this file might be covered by the GNU General Public
 * License.
 *
 * -------------------------------------------
 * ####ECOSGPLCOPYRIGHTEND####
 * =================================================================
 * #####DESCRIPTIONBEGIN####
 * 
 *  Author(s):    nickg@calivar.com
 *  Contributors: nickg@calivar.com
 *  Date:         2002-10-14
 *  Purpose:      
 *  Description:  
 *               
 * ####DESCRIPTIONEND####
 * 
 * =================================================================
 */

#include <pkgconf/system.h>
#include <pkgconf/isoinfra.h>
#include <pkgconf/httpd.h>

#include <cyg/hal/hal_tables.h>

#include <stdio.h>

/* ================================================================= */
/* Start daemon explicitly
 */

#ifndef CYGNUM_HTTPD_SERVER_AUTO_START

__externC void cyg_httpd_startup(void);

#endif

/* ================================================================= */
/* Lookup Table
 *
 * 
 */

typedef cyg_bool cyg_httpd_handler(FILE *client, char *filename,
                              char *formdata, void *arg);

struct cyg_httpd_table_entry
{
    char                *pattern;
    cyg_httpd_handler   *handler;
    void                *arg;
} CYG_HAL_TABLE_TYPE;

typedef struct cyg_httpd_table_entry cyg_httpd_table_entry;

#define CYG_HTTPD_TABLE_ENTRY( __name, __pattern, __handler, __arg ) \
cyg_httpd_table_entry __name CYG_HAL_TABLE_ENTRY( httpd_table ) = { __pattern, __handler, __arg } 

/* ================================================================= */
/* Useful handler functions
 */

/* ----------------------------------------------------------------- */
/*
 */

__externC cyg_bool cyg_httpd_send_html( FILE *client, char *filename,
                                        char *request, void *arg );

/* ----------------------------------------------------------------- */
/*
 */

typedef struct
{
    char        *content_type;
    cyg_uint32  content_length;
    cyg_uint8   *data;
} cyg_httpd_data;

__externC cyg_bool cyg_httpd_send_data( FILE *client, char *filename,
                                        char *request, void *arg );

#define CYG_HTTPD_DATA( __name, __type, __length, __data ) \
cyg_httpd_data __name = { __type, __length, __data }

/* ================================================================= */
/* HTTP and HTML helper macros and functions
 */

/* ----------------------------------------------------------------- */
/* HTTP header support
 *
 * cyg_http_start() sends an HTTP header with the given content type
 * and length. cyg_http_finish() terminates an HTTP send.
 * html_begin() starts an HTML document, and html_end() finishes it.
 */

__externC void cyg_http_start( FILE *client, char *content_type,
                               int content_length );
__externC void cyg_http_finish( FILE *client );

#define html_begin(__client)                            \
        cyg_http_start( __client, "text/html", 0 );     \
        html_tag_begin( __client, "html", "" )

#define html_end( __client )                    \
        html_tag_end( __client, "html" );       \
        cyg_http_finish( __client )

/* ----------------------------------------------------------------- */
/*
 */


__externC void cyg_html_tag_begin( FILE *client, char *tag, char *attr );
__externC void cyg_html_tag_end( FILE *client, char *tag );

#define html_tag_begin( __client, __tag, __attr ) \
        cyg_html_tag_begin( __client, __tag, __attr )

#define html_tag_end( __client, __tag ) cyg_html_tag_end( __client, __tag )


/* ----------------------------------------------------------------- */
/*
 */


#define html_head( __client, __title, __meta )                          \
{                                                                       \
    fprintf(__client, "<%s><%s>", "head", "title" );                    \
    fputs( __title, __client );                                         \
    fprintf(__client, "</%s>%s</%s>\n", "title", __meta, "head");       \
}

#define html_body_begin( __client, __attr )     \
        cyg_html_tag_begin( __client, "body", __attr );

#define html_body_end( __client )               \
        cyg_html_tag_end( __client, "body" );

#define html_heading( __client, __level, __heading ) \
    fprintf(__client,"<h%d>%s</h%d>\n",__level,__heading,__level);

/* ----------------------------------------------------------------- */
/*
 */


#define html_url( __client, __text, __link ) \
        fprintf( __client, "<a href=\"%s\">%s</a>\n",__link,__text);

#define html_para_begin( __client, __attr )     \
        cyg_html_tag_begin( __client, "p", __attr );

#define html_image( __client, __source, __alt, __attr )                 \
        fprintf( __client, "<%s %s=\"%s\" %s=\"%s\" %s>\n", "img",      \
                 "src",__source,                                        \
                 "alt",__alt,                                           \
                 (__attr)?(__attr):"" );

/* ----------------------------------------------------------------- */
/*
 */


#define html_table_begin( __client, __attr )     \
        cyg_html_tag_begin( __client, "table", __attr );

#define html_table_end( __client )               \
        cyg_html_tag_end( __client, "table" );

#define html_table_header( __client, __content, __attr )        \
{                                                               \
    cyg_html_tag_begin( __client, "th", __attr);                \
    fputs( __content, __client );                               \
    cyg_html_tag_end( __client, "th" );                         \
}

#define html_table_row_begin( __client, __attr )     \
        cyg_html_tag_begin( __client, "tr", __attr );

#define html_table_row_end( __client )               \
        cyg_html_tag_end( __client, "tr" );

#define html_table_data_begin( __client, __attr )     \
        cyg_html_tag_begin( __client, "td", __attr );

#define html_table_data_end( __client )               \
        cyg_html_tag_end( __client, "td" );

/* ----------------------------------------------------------------- */
/*
 */


#define html_form_begin( __client, __url, __attr )      \
        fprintf(__client, "<%s %s=\"%s\" %s>\n","form", \
                "action",__url,                         \
                (__attr)?(__attr):"" );

#define html_form_end( __client )               \
        cyg_html_tag_end( __client, "form" );

#define html_form_input( __client, __type, __name, __value, __attr )            \
{                                                                               \
    char *__lattr = (__attr);                                                   \
    fprintf(__client, "<%s %s=\"%s\" %s=\"%s\" %s=\"%s\" %s>\n","input",        \
            "type",__type,                                                      \
            "name",__name,                                                      \
            "value",__value,                                                    \
            __lattr?__lattr:"" );                                               \
}

#define html_form_input_radio( __client, __name, __value, __checked ) \
        html_form_input( __client, "radio", __name, __value, (__checked)?"checked":"" )

#define html_form_input_checkbox( __client, __name, __value, __checked ) \
        html_form_input( __client, "checkbox", __name, __value, (__checked)?"checked":"" )

#define html_form_input_hidden( __client, __name, __value ) \
        html_form_input( __client, "hidden", __name, __value, "" )

#define html_form_select_begin( __client, __name, __attr )      \
        fprintf( __client, "<%s %s=\"%s\" %s>\n","select",      \
                 "name",__name,                                 \
                 (__attr)?(__attr):"" );

#define html_form_option( __client, __value, __label, __selected )      \
        fprintf( __client, "<%s %s=\"%s\" %s>\n","option",              \
                 "value", __value,                                      \
                 (__selected)?"selected":"" );                          \
        fputs(__label, __client );

#define html_form_select_end( __client ) \
        cyg_html_tag_end( __client, "select" );


/* ================================================================= */
/*
 */


__externC void cyg_formdata_parse( char *data, char *list[], int size );

__externC char *cyg_formlist_find( char *list[], char *name );

/* ----------------------------------------------------------------- */
#endif /* CYGONCE_NET_HTTPD_HTTPD_H                                  */
/* end of httpd.c                                                    */
