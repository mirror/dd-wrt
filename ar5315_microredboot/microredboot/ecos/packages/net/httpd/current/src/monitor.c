/* =================================================================
 *
 *      monitor.c
 *
 *      An HTTP system monitor 
 *
 * ================================================================= 
 * ####ECOSGPLCOPYRIGHTBEGIN####
 * -------------------------------------------
 * This file is part of eCos, the Embedded Configurable Operating
 * System.
 * Copyright (C) 2002 Nick Garnett.
 * Copyright (C) 2003 Andrew Lunn
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
 *  Contributors: nickg@calivar.com, andrew.lunn@ascom.ch
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
#include <pkgconf/net.h>
#include <pkgconf/httpd.h>
#include <pkgconf/kernel.h>

#include <cyg/infra/cyg_trac.h>        /* tracing macros */
#include <cyg/infra/cyg_ass.h>         /* assertion macros */

#include <cyg/httpd/httpd.h>

#include <cyg/kernel/kapi.h>
#ifdef CYGPKG_KERNEL_INSTRUMENT
#include <cyg/kernel/instrmnt.h>
#include <cyg/kernel/instrument_desc.h>
#endif

#include <unistd.h>
#include <ctype.h>

/* ================================================================= */
/* Include all the necessary network headers by hand. We need to do
 * this so that _KERNEL is correctly defined, or not, for specific
 * headers so we can use both the external API and get at internal
 * kernel structures.
 */

#define _KERNEL
#include <sys/param.h>
#undef _KERNEL
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <netdb.h>
#define _KERNEL

#include <sys/sysctl.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <net/route.h>
#include <net/if_dl.h>

#include <sys/protosw.h>
#include <netinet/in_pcb.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/ip_var.h>
#include <netinet/icmp_var.h>
#include <netinet/udp_var.h>
#include <netinet/tcp_var.h>
#ifdef CYGPKG_NET_INET6
#include <netinet/ip6.h>
#include <net/if_var.h>
#include <netinet6/ip6_var.h>
#include <netinet6/in6_var.h>
#include <netinet/icmp6.h>
#endif


#include <sys/mbuf.h>

#include <cyg/io/eth/eth_drv_stats.h>

/* ================================================================= */
/* Use this when a thread appears to have no name.
 */

#define NULL_THREAD_NAME "----------"

/* ================================================================= */
/* Draw navigation bar
 *
 * This draws a simple table containing links to the various monitor
 * pages at the current position in the HTML stream.
 */

static void draw_navbar( FILE *client )
{
    html_para_begin( client, "" );
    
    html_table_begin( client, "border cellpadding=\"4\"" );
    {
        html_table_row_begin(client, "" );
        {
            html_table_data_begin( client, "" );
            html_image( client, "/monitor/ecos.gif", "eCos logo", "" );
            html_table_data_begin( client, "" );
            html_url( client, "Threads", "/monitor/threads.html");
            html_table_data_begin( client, "" );
            html_url( client, "Interrupts", "/monitor/interrupts.html");
            html_table_data_begin( client, "" );
            html_url( client, "Memory", "/monitor/memory.html");
            html_table_data_begin( client, "" );
            html_url( client, "Network", "/monitor/network.html");
#ifdef CYGPKG_KERNEL_INSTRUMENT
            html_table_data_begin( client, "" );
            html_url( client, "Instrumentation", "/monitor/instrument.html");
#endif
        }
        html_table_row_end( client );
    }
    html_table_end( client );
}

/* ================================================================= */
/* Index page
 *
 * A simple introductory page matching "/monitor" and
 * "/monitor/index.html".
 */

static char monitor_index_blurb[] =
"<p>This is the eCos System Monitor. It presents a simple web monitor "
"and control interface for eCos systems.\n"
"<p>Use the navigation bar at the bottom of each page to explore."
;

static cyg_bool cyg_monitor_index( FILE * client, char *filename,
                                   char *formdata, void *arg )
{
    html_begin(client);

    html_head(client,"eCos System Monitor", "");    

    html_body_begin(client,"");
    {
        html_heading(client, 2, "eCos System Monitor" );

        fputs( monitor_index_blurb, client );
        
        draw_navbar(client);
    }
    html_body_end(client);

    html_end(client);
    
    return 1;
}

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_entry,
                       "/monitor",
                       cyg_monitor_index,
                       NULL );

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_index_entry,
                       "/monitor/index.html",
                       cyg_monitor_index,
                       NULL );

/* ================================================================= */
/* Thread Monitor
 *
 * Uses the kapi thread info API to enumerate all the current threads
 * and generate a table showing their state.
 */

static cyg_bool cyg_monitor_threads( FILE * client, char *filename,
                                     char *formdata, void *arg )
{
    
    html_begin(client);

/*    html_head(client,"Thread Monitor", "<meta http-equiv=\"refresh\" content=\"10\">"); */
    html_head(client,"eCos Thread Monitor", "");    

    html_body_begin(client,"");
    {
        html_heading(client, 2, "Thread Monitor" );

        html_table_begin( client, "border" );
        {
            cyg_handle_t thread = 0;
            cyg_uint16 id = 0;
        
            html_table_header( client, "Id", "" );
            html_table_header( client, "State", "" );
            html_table_header( client, "Set<br>Priority", "" );
            html_table_header( client, "Current<br>Priority", "" );
            html_table_header( client, "Name", "" );
            html_table_header( client, "Stack Base", "" );
            html_table_header( client, "Stack Size", "" );
#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
            html_table_header( client, "Stack Used", "" );
#endif
            /* Loop over the threads, and generate a table row for
             * each.
             */
            while( cyg_thread_get_next( &thread, &id ) )
            {
                cyg_thread_info info;
                char *state_string;

                cyg_thread_get_info( thread, id, &info );
                
                if( info.name == NULL )
                    info.name = NULL_THREAD_NAME;

                /* Translate the state into a string.
                 */
                if( info.state == 0 )
                    state_string = "RUN";
                else if( info.state & 0x04 )
                    state_string = "SUSP";
                else switch( info.state & 0x1b )
                {
                case 0x01: state_string = "SLEEP"; break;
                case 0x02: state_string = "CNTSLEEP"; break;
                case 0x08: state_string = "CREATE"; break;
                case 0x10: state_string = "EXIT"; break;
                default: state_string = "????"; break;
                }

                /* Now generate the row.
                 */
                html_table_row_begin(client, "" );
                {
                    html_table_data_begin( client, "" );
                    fprintf( client, "<a href=\"/monitor/thread-%04x.html\">%04x</a>\n", id,id);
                    html_table_data_begin( client, "" );
                    fprintf( client, "%s", state_string);
                    html_table_data_begin( client, "" );
                    fprintf( client, "%d", info.set_pri);
                    html_table_data_begin( client, "" );
                    fprintf( client, "%d", info.cur_pri);
                    html_table_data_begin( client, "" );
                    fputs( info.name, client );
                    html_table_data_begin( client, "" );
                    fprintf( client, "%08x", info.stack_base );
                    html_table_data_begin( client, "" );
                    fprintf( client, "%d", info.stack_size );
#ifdef CYGFUN_KERNEL_THREADS_STACK_MEASUREMENT
                    html_table_data_begin( client, "" );
                    fprintf( client, "%d", info.stack_used );
#endif
                }
                html_table_row_end(client);
                
            }
        }
        html_table_end( client );

        draw_navbar(client);
    }
    html_body_end(client);

    html_end(client);
    
    return 1;
}

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_show_threads,
                       "/monitor/threads.htm*",
                       cyg_monitor_threads,
                       NULL );

/* ================================================================= */
/* Thread edit page
 *
 * A page on which the thread's state may be edited. This tests forms
 * handling.
 */

static char thread_edit_blurb[] =
"<p>This table contains an entry for each property of the thread "
"that you can edit. The properties are:\n"
"<p><b>State:</b> Change thread's state. The <em>Suspend</em> button "
"will suspend the thread. The <em>Run</em> button will undo any previous "
"suspends. The <em>Release</em> button will release the thread out of "
"any sleep it is currently in.\n"
"<p><b>Priority:</b> Change the thread's priority.\n"
"<p>Once the new state has been selected, press the <em>Submit</em> "
"button to make the change, or <em>Reset</em> to clear it."
;

static cyg_bool cyg_monitor_thread_edit( FILE * client, char *filename,
                                         char *formdata, void *arg )
{
    /* If any form data has been supplied, then change the thread's
     * state accordingly.
     */
    if( formdata != NULL )
    {
        char *formlist[6];
        char *state;
        char *pri_string;
        char *id_string;
        cyg_handle_t thread = 0;
        cyg_uint16 id;

        /* Parse the data */
        cyg_formdata_parse( formdata, formlist, 6 );

        /* Get the thread id from the hidden control */
        id_string = cyg_formlist_find( formlist, "thread");

        sscanf( id_string, "%04hx", &id );

        thread = cyg_thread_find( id );

        /* If there is a pri field, change the priority */
        pri_string = cyg_formlist_find( formlist, "pri");

        if( pri_string != NULL )
        {
            cyg_priority_t pri;

            sscanf( pri_string, "%d", &pri );

            cyg_thread_set_priority( thread, pri );
        }

        /* If there is a state field, change the thread state */
        state = cyg_formlist_find( formlist, "state");

        if( state != NULL )
        {
            if( strcmp( state, "run" ) == 0 )
                cyg_thread_resume( thread );
            if( strcmp( state, "suspend" ) == 0 )
                cyg_thread_suspend( thread );
            if( strcmp( state, "release" ) == 0 )
                cyg_thread_release( thread );
        }
    }

    /* Now generate a page showing the current thread state, and
     * including form controls to change it.
     */
    
    html_begin(client);

    html_head(client,"eCos Thread Editor", "");

    html_body_begin(client,"");
    {
        cyg_uint16 id;
        cyg_thread_info info;
        cyg_handle_t thread = 0;
        char idbuf[16];
        
        sscanf( filename, "/monitor/thread-%04hx.html", &id );

        thread = cyg_thread_find( id );
        cyg_thread_get_info(thread, id, &info );
        
        html_heading(client, 2, "Thread State Editor" );

        html_para_begin( client, "" );
        
        fprintf( client, "Editing Thread %04x %s\n",id,
                 info.name?info.name:NULL_THREAD_NAME);

        fputs( thread_edit_blurb, client );
        
        html_form_begin( client, filename, "" );        
        {
            html_table_begin( client, "border" );
            {
                html_table_header( client, "Property", "" );
                html_table_header( client, "Value", "" );

                html_table_row_begin(client, "" );
                {
                    html_table_data_begin( client, "" );
                    fputs( "State", client );

                    html_table_data_begin( client, "" );

                    html_form_input_radio( client, "state", "run", (info.state&0x04)==0 );
                    fputs( "Run", client );
                    html_form_input_radio( client, "state", "suspend", (info.state&0x04)!=0 );
                    fputs( "Suspend", client );
                    html_form_input_radio( client, "state", "release", 0 );
                    fputs( "Release", client );
                }
                html_table_row_end( client );

                html_table_row_begin(client, "" );
                {
                    html_table_data_begin( client, "" );
                    fputs( "Priority", client );

                    html_table_data_begin( client, "" );
                    fprintf(client,"<input type=\"text\" name=\"pri\" size=\"10\" value=\"%d\">\n",
                            info.set_pri);                    
                }
                html_table_row_end( client );

            }
            html_table_end( client );

            /* Add submit and reset buttons */
            html_form_input(client, "submit", "submit", "Submit", "");                
            html_form_input(client, "reset", "reset", "Reset", "");

            /* Pass the thread ID through to our next incarnation */
            sprintf( idbuf, "%04x", id );
            html_form_input_hidden(client, "thread", idbuf );            

        }
        html_form_end( client );
        
        draw_navbar(client);
    }
    html_body_end(client);

    html_end(client);
    
    return 1;
}

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_thread_edit_entry,
                       "/monitor/thread-*",
                       cyg_monitor_thread_edit,
                       NULL );

/* ================================================================= */
/* Interrupt monitor
 *
 * At present this just generates a table showing which interrupts
 * have an ISR attached.
 */

static cyg_bool cyg_monitor_interrupts( FILE * client, char *filename,
                                        char *formdata, void *arg )
{
    html_begin(client);

    html_head(client,"eCos Interrupt Monitor", "");

    html_body_begin(client,"");
    {
        html_heading(client, 2, "Interrupt Monitor" );

        html_table_begin( client, "border" );
        {
            int i;
            int maxint = CYGNUM_HAL_ISR_MAX;

#ifdef CYGPKG_HAL_I386
            maxint = CYGNUM_HAL_ISR_MIN+16;
#endif
            
            html_table_header( client, "ISR", "" );
            html_table_header( client, "State", "" );
            
            for( i = CYGNUM_HAL_ISR_MIN; i <= maxint ; i++ )
            {
                cyg_bool_t inuse;
                HAL_INTERRUPT_IN_USE( i, inuse );

                html_table_row_begin(client, "" );
                {
                    html_table_data_begin( client, "" );
                    fprintf( client, "%d", i);
                    html_table_data_begin( client, "" );
                    fprintf( client, "%s", inuse?"In Use":"Free");
                }
                html_table_row_end( client );
            }
        }
        html_table_end( client );

        draw_navbar(client);
    }
    html_body_end(client);

    html_end(client);
    
    return 1;
    
}

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_interrupts_entry,
                       "/monitor/interrupts.htm*",
                       cyg_monitor_interrupts,
                       NULL );

/* ================================================================= */
/* Memory monitor
 *
 * Generates a table showing a 256 byte page of memory. Form controls
 * allow changes to the base address and display element size.
 */

static cyg_bool cyg_monitor_memory( FILE * client, char *filename,
                                    char *formdata, void *arg )
{
    char *formlist[10];
    cyg_uint32 base = 0;
    unsigned int datasize = 1;
    int size = 256;
    char *p;
    bool valid_base = true;

    
    cyg_formdata_parse( formdata, formlist, 10 );

    p = cyg_formlist_find( formlist, "base" );

    /* If the page is requested without a 'base' parameter, do not attempt
     * to access any memory locations to prevent illegal memory accesses
     * on targets where '0' is not a valid address.
     */
    if( p != NULL )
        sscanf( p, "%x", &base );
    else
        valid_base = false;

    p = cyg_formlist_find( formlist, "datasize" );

    if( p != NULL )
        sscanf( p, "%x", &datasize );

    if( cyg_formlist_find( formlist, "next" ) != NULL )
        base += size;

    if( cyg_formlist_find( formlist, "prev" ) != NULL )
        base -= size;
        
    html_begin(client);

    html_head(client,"eCos Memory Monitor", "");

    html_body_begin(client,"");
    {
        html_heading(client, 2, "Memory Monitor" );

        html_form_begin( client, "/monitor/memory.html", "" );
        {

            /* Text input control for base address
             */
            html_para_begin( client, "" );
            fprintf(client,
                    "Base Address: 0x<input type=\"text\" name=\"base\" size=\"10\" value=\"%x\">\n",
                    base);

            fprintf(client,
                    "WARNING: entering an illegal base address can crash the system.\n");

            /* A little menu for the element size
             */
            html_para_begin( client, "" );

            fputs( "Element Size: ", client );
            html_form_select_begin( client, "datasize", "" );
            html_form_option( client, "1", "bytes", datasize==1 );
            html_form_option( client, "2", "words", datasize==2 );
            html_form_option( client, "4", "dwords", datasize==4 );
            html_form_select_end( client );
                        
            html_para_begin( client, "" );

            /* Submit and reset buttons
             */
            html_form_input(client, "submit", "submit", "Submit", "");                
            html_form_input(client, "reset", "reset", "Reset", "");

            html_para_begin( client, "" );

            /* Previous page button */
            html_form_input(client, "submit", "prev", "Prev Page", "");

            /* Switch to monospaced font */
            cyg_html_tag_begin( client, "font", "face=\"monospace\"" );
            
            html_table_begin( client, "" );

            if (valid_base == true) {
                cyg_addrword_t loc;
                cyg_addrword_t oloc;
                
                for( oloc = loc = base; loc <= (base+size) ; loc++ )
                {
                    if( ( loc % 16 ) == 0 )
                    {
                        if( loc != base )
                        {
                            html_table_data_begin( client, "" );
                            for( ; oloc < loc; oloc++ )
                            {
                                char c = *(char *)oloc;
                                if( !isprint(c) )
                                    c = '.';
                                putc( c, client );
                            }
                            html_table_row_end( client );
                        }
                        if( loc == (base+size) )
                            break;
                        html_table_row_begin(client, "" );
                        html_table_data_begin( client, "" );
                        fprintf( client, "%08x:",loc);
                    }

                    html_table_data_begin( client, "" );

                    if( (loc % datasize) == 0 )
                    {
                        switch( datasize )
                        {
                        case 1: fprintf( client, "%02x", *(cyg_uint8  *)loc ); break;
                        case 2: fprintf( client, "%04x", *(cyg_uint16 *)loc ); break;
                        case 4: fprintf( client, "%08x", *(cyg_uint32 *)loc ); break;
                        }
                    }
                }
            }
            html_table_end( client );
            cyg_html_tag_end( client, "font" );
            
            html_form_input(client, "submit", "next", "Next Page", "");            
        }
        html_form_end( client );

        draw_navbar(client);
    }
    html_body_end(client);

    html_end(client);
    
    return 1;
    
}

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_memory_entry,
                       "/monitor/memory.htm*",
                       cyg_monitor_memory,
                       NULL );

/* ================================================================= */
/* Network Monitor
 *
 * This function generates a page containing information about the
 * network interfaces and the protocols.
 */

static cyg_bool cyg_monitor_network( FILE * client, char *filename,
                                     char *formdata, void *arg )
{
    struct ifaddrs *iflist, *ifp;

    getifaddrs(&iflist);
    
    html_begin(client);

    html_head(client,"eCos Network Monitor", "");

    html_body_begin(client,"");
    {
        html_heading(client, 2, "Network Monitor" );

        html_heading(client, 3, "Interfaces" );        

        html_table_begin( client, "border" );
        {
            char addr[64];
            int i;
            ifp = iflist;

            html_table_header( client, "Interface", "" );
            html_table_header( client, "Status", "" );

            while( ifp != (struct ifaddrs *)NULL) 
            {
                if (ifp->ifa_addr->sa_family != AF_LINK) 
                {
                  
                  html_table_row_begin(client, "" );
                  {
                        html_table_data_begin( client, "" );
                        fprintf( client, "%s", ifp->ifa_name);

                        html_table_data_begin( client, "" );                        
                        html_table_begin( client, "" );
                        {
                            /* Get the interface's flags and display
                             * the interesting ones.
                             */
                            
                            html_table_row_begin(client, "" );
                            fprintf( client, "<td>Flags<td>\n" );
                            for( i = 0; i < 16; i++ )
                              {
                                switch( ifp->ifa_flags & (1<<i) )
                                  {
                                  default: break;
                                  case IFF_UP: fputs( " UP", client ); break;
                                  case IFF_BROADCAST: fputs( " BROADCAST", client ); break;
                                  case IFF_DEBUG: fputs( " DEBUG", client ); break;
                                  case IFF_LOOPBACK: fputs( " LOOPBACK", client ); break;
                                  case IFF_PROMISC: fputs( " PROMISCUOUS", client ); break;
                                  case IFF_RUNNING: fputs( " RUNNING", client ); break;
                                  case IFF_SIMPLEX: fputs( " SIMPLEX", client ); break;
                                  case IFF_MULTICAST: fputs( " MULTICAST", client ); break;
                                  }
                              }
                            html_table_row_end( client );                
                            
                            html_table_row_begin(client, "" );
                            getnameinfo(ifp->ifa_addr, sizeof(*ifp->ifa_addr),
                                        addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
                            fprintf( client, "<td>Address<td>%s\n", addr);
                            html_table_row_end( client );
                            
                            if (ifp->ifa_netmask) 
                            {
                              html_table_row_begin(client, "" );
                              getnameinfo(ifp->ifa_netmask, sizeof(*ifp->ifa_netmask),
                                          addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
                              fprintf( client, "<td>Mask<td>%s\n", addr); 
                              html_table_row_end( client );
                            }

                            if (ifp->ifa_broadaddr) 
                            {
                              html_table_row_begin(client, "" );
                              getnameinfo(ifp->ifa_broadaddr, sizeof(*ifp->ifa_broadaddr),
                                          addr, sizeof(addr), NULL, 0, NI_NUMERICHOST);
                              fprintf( client, "<td>Broadcast<td>%s\n", addr); 
                              html_table_row_end( client );
                            }
                        }
                        html_table_end( client );   
                    }
                    html_table_row_end( client );

                }
                ifp = ifp->ifa_next;
            }
        }
        html_table_end( client );

        /* Now the protocols. For each of the main protocols: IP,
         * ICMP, UDP, TCP print a table of useful information derived
         * from the in-kernel data structures. Note that this only
         * works for the BSD stacks.
         */
        
        html_para_begin( client, "" );
        html_heading(client, 3, "Protocols" );

        html_para_begin( client, "" );
        html_table_begin( client, "border");
        {
            html_table_header( client, "IPv4", "" );
#ifdef CYGPKG_NET_INET6
            html_table_header( client, "IPv6", "" );
#endif            
            html_table_header( client, "ICMPv4", "" );
#ifdef CYGPKG_NET_INET6
            html_table_header( client, "ICMPv6", "" );
#endif            
            html_table_header( client, "UDP", "" );
            html_table_header( client, "TCP", "" );

            html_table_row_begin(client, "" );
            {
                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Received" );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Total",
                             ipstat.ips_total );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Bad",
                             ipstat.ips_badsum+
                             ipstat.ips_tooshort+
                             ipstat.ips_toosmall+
                             ipstat.ips_badhlen+
                             ipstat.ips_badlen+
                             ipstat.ips_noproto+
                             ipstat.ips_toolong
                        );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Reassembled",
                             ipstat.ips_reassembled );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Delivered",
                             ipstat.ips_delivered );

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Sent" );                    
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Total",
                             ipstat.ips_localout );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Raw",
                             ipstat.ips_rawout );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Fragmented",
                             ipstat.ips_fragmented );
                }
                html_table_end( client );
#ifdef CYGPKG_NET_INET6
                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Received" );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Total",
                             ip6stat.ip6s_total );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Bad",
                             ip6stat.ip6s_tooshort+
                             ip6stat.ip6s_toosmall
                        );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Reassembled",
                             ip6stat.ip6s_reassembled );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Delivered",
                             ip6stat.ip6s_delivered );

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Sent" );                    
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Total",
                             ip6stat.ip6s_localout );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Raw",
                             ip6stat.ip6s_rawout );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Fragmented",
                             ip6stat.ip6s_fragmented );
                }
                html_table_end( client );
#endif
                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Received" );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "ECHO",
                             icmpstat.icps_inhist[ICMP_ECHO] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "ECHO REPLY",
                             icmpstat.icps_inhist[ICMP_ECHOREPLY] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "UNREACH",
                             icmpstat.icps_inhist[ICMP_UNREACH] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "REDIRECT",
                             icmpstat.icps_inhist[ICMP_REDIRECT] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Other",
                             icmpstat.icps_inhist[ICMP_SOURCEQUENCH]+
                             icmpstat.icps_inhist[ICMP_ROUTERADVERT]+
                             icmpstat.icps_inhist[ICMP_ROUTERSOLICIT]+
                             icmpstat.icps_inhist[ICMP_TIMXCEED]+
                             icmpstat.icps_inhist[ICMP_PARAMPROB]+
                             icmpstat.icps_inhist[ICMP_TSTAMP]+
                             icmpstat.icps_inhist[ICMP_TSTAMPREPLY]+
                             icmpstat.icps_inhist[ICMP_IREQ]+
                             icmpstat.icps_inhist[ICMP_IREQREPLY]+
                             icmpstat.icps_inhist[ICMP_MASKREQ]+
                             icmpstat.icps_inhist[ICMP_MASKREPLY]
                        );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Bad",
                             icmpstat.icps_badcode+
                             icmpstat.icps_tooshort+
                             icmpstat.icps_checksum+
                             icmpstat.icps_badlen+
                             icmpstat.icps_bmcastecho
                        );

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Sent" );                    
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "ECHO",
                             icmpstat.icps_outhist[ICMP_ECHO] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "ECHO REPLY",
                             icmpstat.icps_outhist[ICMP_ECHOREPLY] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "UNREACH",
                             icmpstat.icps_outhist[ICMP_UNREACH] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "REDIRECT",
                             icmpstat.icps_outhist[ICMP_REDIRECT] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Other",
                             icmpstat.icps_inhist[ICMP_SOURCEQUENCH]+                             
                             icmpstat.icps_outhist[ICMP_ROUTERADVERT]+
                             icmpstat.icps_outhist[ICMP_ROUTERSOLICIT]+
                             icmpstat.icps_outhist[ICMP_TIMXCEED]+
                             icmpstat.icps_outhist[ICMP_PARAMPROB]+
                             icmpstat.icps_outhist[ICMP_TSTAMP]+
                             icmpstat.icps_outhist[ICMP_TSTAMPREPLY]+
                             icmpstat.icps_outhist[ICMP_IREQ]+
                             icmpstat.icps_outhist[ICMP_IREQREPLY]+
                             icmpstat.icps_outhist[ICMP_MASKREQ]+
                             icmpstat.icps_outhist[ICMP_MASKREPLY]
                        );
                }
                html_table_end( client );

#ifdef CYGPKG_NET_INET6
                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Received" );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "ECHO",
                             icmp6stat.icp6s_inhist[ICMP_ECHO] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "ECHO REPLY",
                             icmp6stat.icp6s_inhist[ICMP_ECHOREPLY] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "UNREACH",
                             icmp6stat.icp6s_inhist[ICMP_UNREACH] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "REDIRECT",
                             icmp6stat.icp6s_inhist[ICMP_REDIRECT] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Other",
                             icmp6stat.icp6s_inhist[ICMP_SOURCEQUENCH]+
                             icmp6stat.icp6s_inhist[ICMP_ROUTERADVERT]+
                             icmp6stat.icp6s_inhist[ICMP_ROUTERSOLICIT]+
                             icmp6stat.icp6s_inhist[ICMP_TIMXCEED]+
                             icmp6stat.icp6s_inhist[ICMP_PARAMPROB]+
                             icmp6stat.icp6s_inhist[ICMP_TSTAMP]+
                             icmp6stat.icp6s_inhist[ICMP_TSTAMPREPLY]+
                             icmp6stat.icp6s_inhist[ICMP_IREQ]+
                             icmp6stat.icp6s_inhist[ICMP_IREQREPLY]+
                             icmp6stat.icp6s_inhist[ICMP_MASKREQ]+
                             icmp6stat.icp6s_inhist[ICMP_MASKREPLY]
                        );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Bad",
                             icmp6stat.icp6s_badcode+
                             icmp6stat.icp6s_tooshort+
                             icmp6stat.icp6s_checksum+
                             icmp6stat.icp6s_badlen
                        );

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Sent" );                    
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "ECHO",
                             icmp6stat.icp6s_outhist[ICMP_ECHO] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "ECHO REPLY",
                             icmp6stat.icp6s_outhist[ICMP_ECHOREPLY] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "UNREACH",
                             icmp6stat.icp6s_outhist[ICMP_UNREACH] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "REDIRECT",
                             icmp6stat.icp6s_outhist[ICMP_REDIRECT] );
                    fprintf( client, "<tr><td>%s<td>%lld</tr>\n", "Other",
                             icmp6stat.icp6s_inhist[ICMP_SOURCEQUENCH]+                             
                             icmp6stat.icp6s_outhist[ICMP_ROUTERADVERT]+
                             icmp6stat.icp6s_outhist[ICMP_ROUTERSOLICIT]+
                             icmp6stat.icp6s_outhist[ICMP_TIMXCEED]+
                             icmp6stat.icp6s_outhist[ICMP_PARAMPROB]+
                             icmp6stat.icp6s_outhist[ICMP_TSTAMP]+
                             icmp6stat.icp6s_outhist[ICMP_TSTAMPREPLY]+
                             icmp6stat.icp6s_outhist[ICMP_IREQ]+
                             icmp6stat.icp6s_outhist[ICMP_IREQREPLY]+
                             icmp6stat.icp6s_outhist[ICMP_MASKREQ]+
                             icmp6stat.icp6s_outhist[ICMP_MASKREPLY]
                        );
                }
                html_table_end( client );
#endif
                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Received" );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Total",
                             udpstat.udps_ipackets );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Bad",
                             udpstat.udps_hdrops+
                             udpstat.udps_badsum+
                             udpstat.udps_badlen+
                             udpstat.udps_noport+
                             udpstat.udps_noportbcast+
                             udpstat.udps_fullsock
                        );
                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Sent" );                    
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Total",
                             udpstat.udps_opackets );
                }
                html_table_end( client );

                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Connections" );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Initiated",
                             tcpstat.tcps_connattempt );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Accepted",
                             tcpstat.tcps_accepts );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Established",
                             tcpstat.tcps_connects );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Closed",
                             tcpstat.tcps_closed );

                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Received" );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Packets",
                             tcpstat.tcps_rcvtotal );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Data Packets",
                             tcpstat.tcps_rcvpack );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Bytes",
                             tcpstat.tcps_rcvbyte );
                    
                    fprintf( client, "<tr><td><b>%s:</b><td></tr>\n", "Sent" );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Packets",
                             tcpstat.tcps_sndtotal );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Data Packets",
                             tcpstat.tcps_sndpack );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Bytes",
                             tcpstat.tcps_sndbyte );

                    
                }
                html_table_end( client );
                
                
            }
            html_table_row_end( client );

        }
        html_table_end( client );

        html_para_begin( client, "" );
        html_heading(client, 3, "Mbufs" );

        html_table_begin( client, "border" );
        {
            html_table_header( client, "Summary", "" );
            html_table_header( client, "Types", "" );

            html_table_row_begin( client, "" );
            {
                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Mbufs",
                             mbstat.m_mbufs );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Clusters",
                             mbstat.m_clusters );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Free Clusters",
                             mbstat.m_clfree );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Drops",
                             mbstat.m_drops );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Waits",
                             mbstat.m_wait );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Drains",
                             mbstat.m_drain );
#if defined(CYGPKG_NET_FREEBSD_STACK)
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Copy Fails",
                             mbstat.m_mcfail );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "Pullup Fails",
                             mbstat.m_mpfail );
#endif                    

                }
                html_table_end( client );

                html_table_data_begin( client, "valign=\"top\"" );                        
                html_table_begin( client, "" );
                {
                    u_long *mtypes;
#if defined(CYGPKG_NET_FREEBSD_STACK)
                    mtypes = mbtypes;
#else
                    mtypes = mbstat.m_mtypes;
#endif
                    
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "FREE",
                             mtypes[MT_FREE] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "DATA",
                             mtypes[MT_DATA] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "HEADER",
                             mtypes[MT_HEADER] );
#if !defined(CYGPKG_NET_FREEBSD_STACK)                    
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "SOCKET",
                             mtypes[MT_SOCKET] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "PCB",
                             mtypes[MT_PCB] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "RTABLE",
                             mtypes[MT_RTABLE] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "HTABLE",
                             mtypes[MT_HTABLE] );
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "ATABLE",
                             mtypes[MT_ATABLE] );
#endif
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "SONAME",
                             mtypes[MT_SONAME] );
#if !defined(CYGPKG_NET_FREEBSD_STACK)                                        
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "SOOPTS",
                             mtypes[MT_SOOPTS] );
#endif
                    fprintf( client, "<tr><td>%s<td>%ld</tr>\n", "FTABLE",
                             mtypes[MT_FTABLE] );

                    /* Ignore the rest for now... */
                    
                }
                html_table_end( client );
                
            }
            html_table_row_end( client );
            
        }
        html_table_end( client );
        
        
        draw_navbar(client);
    }
    html_body_end(client);

    html_end(client);
    
    freeifaddrs(iflist);
    return 1;
    
}

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_network_entry,
                       "/monitor/network.htm*",
                       cyg_monitor_network,
                       NULL );

        
/* ================================================================= */
/* Instrumentation Monitor
 *
 * If the CYGPKG_KERNEL_INSTRUMENT option is set, we generate a table
 * showing the current instrumentation buffer. If the FLAGS option is
 * enabled we also print a table giving control of the flags.
 * 
 */

#ifdef CYGPKG_KERNEL_INSTRUMENT

/* Instrumentation record. */
struct Instrument_Record
{
    CYG_WORD16  type;                   // record type
    CYG_WORD16  thread;                 // current thread id
    CYG_WORD    timestamp;              // 32 bit timestamp
    CYG_WORD    arg1;                   // first arg
    CYG_WORD    arg2;                   // second arg
};
typedef struct Instrument_Record Instrument_Record;

/* Instrumentation variables, these live in the
 * instrumentation files in the kernel
 */
__externC Instrument_Record       *instrument_buffer_pointer;
__externC Instrument_Record       instrument_buffer[];
__externC cyg_uint32              instrument_buffer_size;

#if defined(CYGDBG_KERNEL_INSTRUMENT_FLAGS) && \
    defined(CYGDBG_KERNEL_INSTRUMENT_MSGS)

static cyg_uint32 instrument_flags[(CYG_INSTRUMENT_CLASS_MAX>>8)+1];

#endif

static char cyg_monitor_instrument_blurb1[] =
"<p>Use the checkboxes to enable the events to be recorded. Click "
"the <em>Submit</em> button to start recording. Click the <em>Clear</em> "
"button to clear all instrumentation and to stop recording."
;

static cyg_bool cyg_monitor_instrument( FILE * client, char *filename,
                                        char *formdata, void *arg )
{

#if defined(CYGDBG_KERNEL_INSTRUMENT_FLAGS) && \
    defined(CYGDBG_KERNEL_INSTRUMENT_MSGS)

    /* Disable all instrumentation while we generate the page.
     * Otherwise we could swamp the information we are really after.
     */
    
    cyg_scheduler_lock();
    {
        struct instrument_desc_s *id = instrument_desc;
        CYG_WORD cl = 0, ev = 0;

        for( ; id->msg != 0; id++ )
        {
            if( id->num > 0xff )
            {
                cl = id->num>>8;
                instrument_flags[cl] = 0;
            }
            else
            {
                ev = id->num;
                cyg_instrument_disable( cl<<8, ev );
            }
        }
    }
    cyg_scheduler_unlock();

#endif
    
    /* If we have some form data, deal with it.
     */
    if( formdata != NULL )
    {
        char *list[40];
        
        cyg_formdata_parse( formdata, list, sizeof(list)/sizeof(*list) );

#if defined(CYGDBG_KERNEL_INSTRUMENT_FLAGS) && \
    defined(CYGDBG_KERNEL_INSTRUMENT_MSGS)
        
        if( cyg_formlist_find( list, "clear" ) != NULL )
        {
            /* If the clear button is set, then disable all instrumentation flags.
             */
            int i;
            for( i = 0; i < sizeof(instrument_flags)/sizeof(*instrument_flags); i++ )
                instrument_flags[i] = 0;
        }
        else
        {
            /* Otherwise all the set checkboxes have been reported to
             * us in the form of class=event inputs.
             */
            char **p;
            for( p = list; *p != NULL; p++ )
            {
                int cl,ev;
                if( sscanf( *p, "%02x=%02x", &cl, &ev ) == 2 )
                    instrument_flags[cl] |= 1<<ev;
            }
        }
#endif
        
        /* If the cleartable button has been pressed, clear all the
         * table entries. 
         */
        if( cyg_formlist_find( list, "cleartable" ) != NULL )
        {
            cyg_scheduler_lock();
            {
                Instrument_Record *ibp = instrument_buffer_pointer;
                do
                {
                    ibp->type = 0;
                    ibp++;
                    if( ibp == &instrument_buffer[instrument_buffer_size] )
                        ibp = instrument_buffer;
                    
                } while( ibp != instrument_buffer_pointer );
                
            }
            cyg_scheduler_unlock();
        }
    }
    
    /* Now start generating the HTML page.
     */
    html_begin(client);

    html_head(client,"eCos Instrumentation Buffer", "");

    html_body_begin(client,"");
    {
        html_heading(client, 2, "Instrumentation Buffer" );

        html_form_begin( client, "/monitor/instrument.html", "" );
        
#if defined(CYGDBG_KERNEL_INSTRUMENT_FLAGS) && \
    defined(CYGDBG_KERNEL_INSTRUMENT_MSGS)
        
        /* If we have the flags enabled, generate a table showing all
         * the flag names with a checkbox against each.
         */

        fputs( cyg_monitor_instrument_blurb1, client );
        
        html_para_begin( client, "" );        
        /* Add submit,clear and reset buttons */
        html_form_input(client, "submit", "submit", "Submit", "");                
        html_form_input(client, "submit", "clear", "Clear", "");                
        html_form_input(client, "reset", "reset", "Reset", "");
        
        html_table_begin( client, "border width=100%" );
        {
            struct instrument_desc_s *id = instrument_desc;
            CYG_WORD cl = 0, ev = 0;
            int clc = 0;
            char class[5], event[5];

            html_table_row_begin( client, "" );

            for( ; id->msg != 0; id++ )
            {
                if( id->num > 0xff )
                {
                    cl = id->num;
                    sprintf( class, "%02x", cl>>8 );
                    clc = 0;
                    
                    if( id != instrument_desc )
                    {
                        html_table_end( client );
                        html_table_row_end( client );
                        html_table_row_begin( client, "" );
                    }
                        
                    html_table_data_begin( client, "valign=\"top\" width=100%" );
                    html_table_begin( client, "width=100%" );
                }
                else
                {
                    ev = id->num;
                    sprintf( event, "%02x", ev );

                    if( (clc%4) == 0 )
                    {
                        if( clc != 0 )
                            html_table_row_end( client );

                        html_table_row_begin( client, "" );
                    }
                    clc++;
                    html_table_data_begin( client, "width=25%" );
                    html_form_input_checkbox( client, class, event,
                                              instrument_flags[cl>>8] & (1<<ev) );
                    fputs( id->msg, client );
                }
            }            
            html_table_end( client );
            html_table_row_end( client );
        }
        html_table_end( client );

        /* Add submit,clear and reset buttons */
        html_form_input(client, "submit", "submit", "Submit", "");                
        html_form_input(client, "submit", "clear", "Clear", "");                
        html_form_input(client, "reset", "reset", "Reset", "");

#endif

        html_para_begin( client, "" );

        html_form_input(client, "submit", "cleartable", "Clear Table", "");

        
        /*
         */
        
        html_table_begin( client, "border" );
        {
            Instrument_Record *ibp = instrument_buffer_pointer;

            html_table_header( client, "TIME", "" );
            html_table_header( client, "THREAD", "" );
            html_table_header( client, "EVENT", "" );
            html_table_header( client, "ARG1", "" );
            html_table_header( client, "ARG2", "" );

            do
            {
                if( ibp->type != 0 )
                {
                    html_table_row_begin(client, "" );
                    {
                        cyg_handle_t thread = cyg_thread_find(ibp->thread);
                        cyg_thread_info info;
                    
                        html_table_data_begin( client, "" );
                        fprintf( client, "%08x",ibp->timestamp);
                        html_table_data_begin( client, "" );
                        if( thread != 0 &&
                            cyg_thread_get_info( thread, ibp->thread, &info ) &&
                            info.name != NULL )
                            fputs( info.name, client );
                        else fprintf( client, "[%04x]", ibp->thread);
                        html_table_data_begin( client, "" );
#if defined(CYGDBG_KERNEL_INSTRUMENT_MSGS)
                        fprintf( client, "%s", cyg_instrument_msg(ibp->type));
#else
                        fprintf( client, "%04x", ibp->type );
#endif
                        html_table_data_begin( client, "" );
                        fprintf( client, "%08x", ibp->arg1);
                        html_table_data_begin( client, "" );
                        fprintf( client, "%08x", ibp->arg2);
                    }
                    html_table_row_end( client );
                }
                ibp++;
                if( ibp == &instrument_buffer[instrument_buffer_size] )
                    ibp = instrument_buffer;
                
            } while( ibp != instrument_buffer_pointer );
        }
        html_table_end( client );

        html_para_begin( client, "" );

        html_form_input(client, "submit", "cleartable", "Clear Table", "");
        
        html_form_end( client );
        
        draw_navbar(client);
    }
    html_body_end(client);

    html_end(client);


#if defined(CYGDBG_KERNEL_INSTRUMENT_FLAGS) && \
    defined(CYGDBG_KERNEL_INSTRUMENT_MSGS)

    /*
     */

    cyg_scheduler_lock();
    {
        struct instrument_desc_s *id = instrument_desc;
        CYG_WORD cl = 0, ev = 0;

        for( ; id->msg != 0; id++ )
        {
            if( id->num > 0xff )
                cl = id->num>>8;
            else
            {
                ev = id->num;
                if( (instrument_flags[cl] & (1<<ev)) != 0 )
                    cyg_instrument_enable( cl<<8, ev );
            }
        }
    }
    cyg_scheduler_unlock();
#endif
    
    return true;
}

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_instrument_entry,
                       "/monitor/instrument.htm*",
                       cyg_monitor_instrument,
                       NULL );

#endif

/* ================================================================= */
/* eCos Logo
 *
 * Define the logo as a byte array, and then define the data structure
 * and table entry to allow it to be fetched by the client.
 */

static cyg_uint8 ecos_logo_gif[] = {
    0x47, 0x49, 0x46, 0x38, 0x39, 0x61, 0x6f, 0x00, 0x23, 0x00, 0xf7, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xea, 0x19, 0x00, 0xcc, 0x00, 0x00, 0xfd, 0xfd, 0xfd, 0x83, 0x00, 0x00, 0xf6,
    0xf6, 0xf6, 0x2f, 0x2f, 0x2f, 0xef, 0xef, 0xef, 0x19, 0x00, 0x19, 0x45, 0x45, 0x45, 0xbb, 0xbb,
    0xbb, 0xe2, 0x3e, 0x3e, 0xe8, 0xbb, 0xd3, 0x3e, 0x3e, 0x3e, 0xfb, 0xfb, 0xfb, 0xbe, 0xbe, 0xbe,
    0xf1, 0xed, 0xed, 0xe5, 0x7b, 0x8e, 0xe5, 0x6b, 0x7b, 0xcc, 0xcc, 0xcc, 0x98, 0x95, 0x98, 0xc6,
    0xc3, 0xc6, 0x19, 0x19, 0x19, 0xf1, 0xf1, 0xf1, 0x98, 0x98, 0x98, 0xd6, 0xd3, 0xd6, 0x83, 0x83,
    0x83, 0xde, 0xde, 0xde, 0xe5, 0x25, 0x25, 0x73, 0x6f, 0x73, 0xea, 0xea, 0xea, 0xe5, 0x8e, 0xa2,
    0xe8, 0xe8, 0xe8, 0x7f, 0x7b, 0x7f, 0xe2, 0x56, 0x66, 0x8b, 0x8b, 0x8b, 0x19, 0x19, 0x00, 0xf8,
    0xf8, 0xf8, 0x73, 0x73, 0x73, 0xd3, 0xd1, 0xd1, 0x9c, 0x9c, 0x9c, 0x50, 0x50, 0x50, 0xef, 0xce,
    0xea, 0x92, 0x8e, 0x92, 0x6f, 0x6f, 0x6f, 0xe2, 0x2f, 0x2f, 0x61, 0x61, 0x61, 0xe5, 0xe5, 0xe5,
    0xe8, 0x9f, 0xb8, 0x37, 0x2f, 0x37, 0x66, 0x66, 0x66, 0xe2, 0x4b, 0x50, 0x56, 0x50, 0x56, 0xa9,
    0xa5, 0xa9, 0xce, 0xce, 0xce, 0x56, 0x50, 0x50, 0xd9, 0xd9, 0xd9, 0x77, 0x73, 0x77, 0x25, 0x25,
    0x2f, 0x6b, 0x6b, 0x6b, 0x9f, 0x9f, 0x9f, 0x87, 0x83, 0x87, 0x3e, 0x37, 0x37, 0xf4, 0xf4, 0xf4,
    0x25, 0x25, 0x25, 0xc1, 0xc1, 0xc1, 0x83, 0x7f, 0x83, 0xe5, 0xe2, 0xe2, 0x4b, 0x45, 0x4b, 0xd1,
    0xce, 0xd1, 0xaf, 0xac, 0xaf, 0xcc, 0xc9, 0xcc, 0x5b, 0x56, 0x5b, 0xdb, 0xd9, 0xdb, 0x66, 0x61,
    0x66, 0xe2, 0xe2, 0xe2, 0xb8, 0xb5, 0xb8, 0x2f, 0x25, 0x2f, 0xc3, 0xc1, 0xc3, 0xe0, 0xde, 0xe0,
    0xd3, 0xd3, 0xd3, 0xde, 0xdb, 0xde, 0xac, 0xac, 0xac, 0xce, 0xcc, 0xce, 0x77, 0x6f, 0x73, 0x4b,
    0x45, 0x45, 0xc9, 0xc6, 0xc9, 0x45, 0x3e, 0x45, 0x61, 0x5b, 0x61, 0xb5, 0xb2, 0xb5, 0x3e, 0x00,
    0x00, 0x8b, 0x87, 0x87, 0x95, 0x92, 0x95, 0xa2, 0x9f, 0xa2, 0xb8, 0xb8, 0xb8, 0x7b, 0x77, 0x7b,
    0x9c, 0x98, 0x9c, 0x50, 0x4b, 0x50, 0x6f, 0x6b, 0x6f, 0x6b, 0x66, 0x6b, 0xac, 0xa9, 0xac, 0x8e,
    0x8b, 0x8e, 0x8e, 0x00, 0x00, 0xdb, 0xdb, 0xdb, 0xed, 0xed, 0xed, 0x50, 0x4b, 0x4b, 0x3e, 0x37,
    0x3e, 0x95, 0x95, 0x95, 0xa5, 0xa2, 0xa5, 0x9f, 0x9c, 0x9f, 0xc1, 0xbe, 0xbe, 0x2f, 0x25, 0x25,
    0xd3, 0xd1, 0xd3, 0xf4, 0xf1, 0xf4, 0xc6, 0xc6, 0xc6, 0x8e, 0x8e, 0x8e, 0x25, 0x19, 0x19, 0x66,
    0x61, 0x61, 0xb2, 0xaf, 0xb2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
    0x00, 0x00, 0x6f, 0x00, 0x23, 0x00, 0x40, 0x08, 0xff, 0x00, 0x03, 0x08, 0x1c, 0x28, 0x10, 0x41,
    0x8a, 0x17, 0x0e, 0x1c, 0x00, 0x58, 0xc8, 0xb0, 0xa1, 0xc3, 0x87, 0x10, 0x21, 0x12, 0x14, 0x28,
    0xa0, 0xa2, 0xc5, 0x8b, 0x18, 0x33, 0x6a, 0xdc, 0xb8, 0x91, 0x01, 0x83, 0x11, 0x31, 0x26, 0x12,
    0x44, 0x10, 0x91, 0xe1, 0x09, 0x91, 0x28, 0x03, 0x4c, 0xb9, 0x50, 0x72, 0xa1, 0x0a, 0x04, 0x29,
    0x09, 0x72, 0x6c, 0x10, 0x73, 0xe2, 0x88, 0x8d, 0x2e, 0x0c, 0xd4, 0x0c, 0x70, 0x73, 0x63, 0x80,
    0x88, 0x10, 0x76, 0x0a, 0x6d, 0x18, 0x94, 0x60, 0x03, 0x8e, 0x1a, 0x27, 0x4a, 0x40, 0x5a, 0x51,
    0xa9, 0xc5, 0x0e, 0x3b, 0x0d, 0x30, 0x40, 0xca, 0x40, 0x45, 0xc9, 0x14, 0x42, 0x45, 0x72, 0x60,
    0xf9, 0xf0, 0x64, 0xd6, 0x00, 0x19, 0x3b, 0xe8, 0xfc, 0x3a, 0x70, 0xe9, 0x45, 0x10, 0x59, 0x0d,
    0x70, 0x9c, 0x30, 0xf1, 0x44, 0xcb, 0xb7, 0x70, 0x4b, 0x8a, 0x64, 0x4a, 0xb7, 0xae, 0x5d, 0x01,
    0x6c, 0x51, 0xbe, 0x88, 0xcb, 0xb7, 0xef, 0xc2, 0xb9, 0x77, 0x03, 0x0b, 0xbe, 0x98, 0x37, 0xc0,
    0x81, 0x88, 0x58, 0x09, 0xde, 0xe0, 0xea, 0xf0, 0xc5, 0x44, 0x0e, 0x0f, 0x13, 0x13, 0x34, 0x30,
    0x21, 0x23, 0xdb, 0x8c, 0x63, 0x07, 0x4a, 0xcd, 0x88, 0xd6, 0xa9, 0x45, 0x9a, 0x04, 0x69, 0x60,
    0xec, 0xc0, 0x94, 0x41, 0x44, 0xaf, 0x03, 0x53, 0xf8, 0xdd, 0xd1, 0x10, 0xf5, 0xc0, 0xa9, 0x76,
    0x41, 0x0f, 0x24, 0xcd, 0xb1, 0xf3, 0xc0, 0x9e, 0x1c, 0x25, 0x88, 0xac, 0xac, 0x91, 0x41, 0x00,
    0xc8, 0x6f, 0x77, 0x48, 0x9e, 0x78, 0xc3, 0xaa, 0xdf, 0x03, 0x27, 0xa6, 0x40, 0x28, 0x2a, 0x13,
    0xe9, 0x08, 0xd9, 0x93, 0x41, 0xd0, 0xe6, 0xd8, 0x41, 0x42, 0x83, 0x15, 0x02, 0x0d, 0x34, 0xe0,
    0x8d, 0x34, 0x73, 0x6a, 0xc6, 0x7e, 0xc3, 0x3b, 0xff, 0x04, 0x3c, 0xb8, 0x3c, 0x5d, 0xdb, 0x03,
    0xb7, 0xc2, 0x7d, 0x91, 0x02, 0xa6, 0xe2, 0x13, 0xe0, 0xf9, 0x22, 0x3f, 0xe1, 0x9a, 0x22, 0xd3,
    0x09, 0xd0, 0x03, 0x18, 0x00, 0x01, 0x9b, 0x2e, 0x48, 0xec, 0x01, 0x34, 0x10, 0xc3, 0x04, 0xd3,
    0x69, 0xa4, 0xd3, 0x5e, 0x11, 0x39, 0x46, 0x96, 0x40, 0x53, 0x44, 0xa4, 0x02, 0x59, 0x1a, 0xe5,
    0x97, 0x95, 0x68, 0x17, 0xd1, 0x80, 0x92, 0x77, 0xfd, 0x69, 0x34, 0x81, 0x83, 0x6d, 0x25, 0xe4,
    0xe1, 0x87, 0x0e, 0x3c, 0x48, 0x90, 0x6a, 0x0c, 0x89, 0x48, 0x10, 0x77, 0x74, 0x85, 0x44, 0x10,
    0x6e, 0x11, 0x4e, 0xd4, 0x9f, 0x84, 0x2b, 0x10, 0x68, 0x97, 0x69, 0x10, 0x31, 0x27, 0xd0, 0x72,
    0x38, 0xe6, 0xa8, 0x63, 0x8e, 0x44, 0x4d, 0x74, 0xd4, 0x5d, 0x13, 0xa1, 0xe8, 0x13, 0x41, 0x31,
    0x60, 0xe4, 0x02, 0x7a, 0x22, 0xa9, 0xd5, 0xd1, 0x61, 0x35, 0x4e, 0x84, 0x80, 0x42, 0x0e, 0x5d,
    0x70, 0x83, 0x48, 0xac, 0xf5, 0xe8, 0xa3, 0x46, 0x2e, 0xac, 0x30, 0x9a, 0x48, 0x45, 0x66, 0xc4,
    0x80, 0x77, 0x01, 0x74, 0x69, 0x91, 0x75, 0x05, 0x0a, 0x80, 0xa4, 0x59, 0x5e, 0x92, 0x54, 0x92,
    0x94, 0x5f, 0x3d, 0xf9, 0x16, 0x09, 0x59, 0x6d, 0x44, 0x03, 0x98, 0x31, 0xfd, 0x48, 0x58, 0x56,
    0x20, 0x20, 0x45, 0x90, 0x71, 0xe2, 0xf5, 0x39, 0xde, 0x44, 0xe6, 0x05, 0xea, 0xdf, 0x4e, 0x1c,
    0x90, 0xe0, 0x40, 0x95, 0x7e, 0xf2, 0x45, 0x9e, 0xa0, 0xe6, 0x75, 0xc0, 0x80, 0x04, 0x00, 0x0a,
    0x85, 0x00, 0x82, 0x89, 0x86, 0xb7, 0x28, 0xa3, 0x82, 0x4e, 0x40, 0x27, 0x41, 0x94, 0x56, 0x2a,
    0xde, 0xa5, 0x98, 0x32, 0x5a, 0x58, 0x6a, 0x7d, 0xed, 0x90, 0x10, 0x93, 0x9e, 0x02, 0x00, 0x2a,
    0x47, 0x2e, 0x78, 0x14, 0x2a, 0x53, 0xb6, 0x75, 0xff, 0x0a, 0x91, 0x03, 0x53, 0xc6, 0x84, 0x00,
    0x09, 0x7c, 0x5d, 0x70, 0x82, 0x7b, 0x29, 0x21, 0x25, 0xc1, 0xa6, 0x03, 0x35, 0x90, 0x61, 0x46,
    0x2e, 0x48, 0x18, 0xec, 0xb0, 0x16, 0xb1, 0x45, 0x62, 0x44, 0x70, 0x92, 0xb5, 0x58, 0x49, 0x0a,
    0x0a, 0xb5, 0x91, 0xb1, 0x31, 0xa1, 0x79, 0x11, 0xb5, 0x03, 0x69, 0xc9, 0x51, 0x0c, 0x50, 0x3e,
    0x74, 0x01, 0xaf, 0x05, 0x9d, 0x80, 0xaa, 0x0a, 0x36, 0x0a, 0x24, 0x2b, 0x00, 0xdf, 0xa2, 0xb4,
    0x82, 0x04, 0x13, 0x4c, 0x00, 0x02, 0x80, 0x96, 0xa1, 0xb4, 0x5d, 0xbb, 0x20, 0xd0, 0xa9, 0xe4,
    0x98, 0x3e, 0xd2, 0x46, 0x43, 0x03, 0x76, 0x76, 0x24, 0x17, 0x41, 0xea, 0x31, 0x3b, 0x11, 0x9f,
    0xe8, 0xa2, 0xe4, 0xc2, 0x46, 0x2c, 0x0a, 0x30, 0xc2, 0x44, 0x06, 0x1c, 0xac, 0x21, 0xc3, 0xd7,
    0x4e, 0x24, 0xe6, 0x8c, 0x02, 0x2f, 0xe8, 0x64, 0x43, 0xcd, 0x36, 0x67, 0xd7, 0x44, 0xda, 0x72,
    0x64, 0x21, 0x41, 0x68, 0xea, 0x96, 0xd2, 0x76, 0x74, 0xd1, 0xf8, 0x50, 0xb9, 0x16, 0xff, 0xc4,
    0x50, 0xb9, 0xfd, 0x32, 0xe5, 0x42, 0x90, 0x74, 0x5d, 0x69, 0xd1, 0xc2, 0x3b, 0x21, 0x6b, 0x91,
    0xc9, 0x0e, 0x65, 0x2c, 0x10, 0xaa, 0xe2, 0x4d, 0xc1, 0x71, 0x60, 0x32, 0xb3, 0xea, 0x59, 0x46,
    0x20, 0x6d, 0x6a, 0xb3, 0x00, 0x38, 0x37, 0xb4, 0x83, 0x48, 0xdd, 0x46, 0x09, 0x62, 0x42, 0x4a,
    0x8b, 0xd4, 0x31, 0xb1, 0x18, 0x89, 0x1c, 0x2c, 0x47, 0xbe, 0x01, 0x7a, 0xde, 0xd0, 0x18, 0x31,
    0xd0, 0x74, 0x43, 0x0e, 0xa0, 0x34, 0x85, 0x0a, 0x87, 0x39, 0x40, 0x02, 0xb8, 0x02, 0xe1, 0xea,
    0x50, 0xd8, 0x49, 0x4a, 0x00, 0x1b, 0x0d, 0x9d, 0x71, 0x96, 0xe4, 0x04, 0x53, 0x75, 0x00, 0x37,
    0x4a, 0x05, 0x42, 0xa5, 0x9f, 0xdb, 0x15, 0x8d, 0x61, 0x2a, 0xd0, 0xd1, 0x0c, 0x2c, 0x0b, 0x51,
    0xad, 0x6d, 0xf2, 0xec, 0xd0, 0x70, 0x35, 0x61, 0x69, 0x71, 0xcb, 0x7a, 0x0b, 0x65, 0x2d, 0x46,
    0x21, 0x11, 0x0c, 0x91, 0xce, 0x29, 0x41, 0x80, 0x68, 0x4b, 0x0e, 0xa0, 0xac, 0x35, 0x96, 0xd8,
    0x96, 0xd5, 0x51, 0xe7, 0x8f, 0x63, 0x44, 0x73, 0x00, 0x6e, 0xc5, 0xa5, 0x02, 0x09, 0xa8, 0x93,
    0xf0, 0xc2, 0xe5, 0x7e, 0xae, 0x1a, 0xd6, 0x08, 0x12, 0xc4, 0x2e, 0x01, 0x85, 0xaf, 0x66, 0x64,
    0x75, 0x41, 0x5f, 0xa7, 0xaa, 0xe8, 0xe6, 0xb5, 0x0b, 0x3a, 0x3a, 0x4a, 0x37, 0xe4, 0xae, 0xfb,
    0xbf, 0x1a, 0xf7, 0x3e, 0x18, 0x0d, 0x91, 0x0e, 0x14, 0x10, 0x00, 0x3b
};

CYG_HTTPD_DATA( cyg_monitor_ecos_logo_data,
                "image/gif",
                sizeof(ecos_logo_gif), ecos_logo_gif );

CYG_HTTPD_TABLE_ENTRY( cyg_monitor_ecos_logo,
                       "/monitor/ecos.gif",
                       cyg_httpd_send_data,
                       &cyg_monitor_ecos_logo_data );

/*------------------------------------------------------------------ */
/* end of monitor.c                                                  */
