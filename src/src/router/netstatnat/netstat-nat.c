/*
#-------------------------------------------------------------------------------
#                                                                                                                         
# $Id: netstat-nat.c,v 1.17 2002/09/12 19:32:12 mardan Exp $     
#       
# $Log: netstat-nat.c,v $
# Revision 1.17  2002/09/12 19:32:12  mardan
# Added display local connections to NAT box self
# Updated README
# Small changes in Makefile
#
# Revision 1.16  2002/09/08 20:23:48  mardan
# Added sort by connection option. (source/destination IP/port)
# Updated README and man-page.
#
# Revision 1.15  2002/08/07 19:25:59  mardan
# Fixed bug, displayed wrong icmp connection in state REPLIED (dest was gateway).
#
# Revision 1.14  2002/08/07 19:02:54  mardan
# Fixed 'icmp' bug. Segmentation fault occured when displaying NATed icmp connections.
#
# Revision 1.13  2002/08/06 19:32:54  mardan
# Added small feature: no header output.
# Lots of code cleanup.
#
# Revision 1.12  2002/08/03 00:22:22  mardan
# Added portname resolving based on the listed names in 'services'.
# Re-arranged the layout.
# Added a Makefile and a header file.
# Updated the README.
#
# Revision 1.11  2002/07/12 20:05:54  mardan
# Added argument for extended view of hostnames.
# Moved display-code into one function.
# Removed most unnessacery code.
# Updated README
#
# Revision 1.10  2002/07/10 19:58:33  mardan
# Added filtering by destination-host, re-arranged some code to work properly.
# Tested DNAT icmp and udp.(pls report if any bugs occur)
# Fixed a few declaration bugs.
#
# Revision 1.9  2002/07/09 20:00:36  mardan
# Added fully DNAT support (udp & icmp not fully tested yet, but should work),
# including argument support for (S)(D)NAT selection.
# Re-arranged layout code, can possible merged into one function.
# Some few minor changes.
# Started to work on destination-host selection.
#
# Revision 1.8  2002/07/07 20:27:47  mardan
# Added display by source host/IP.
# Made a few fixes/changes.
# Updated the REAMDE.
#
# Revision 1.7  2002/06/30 19:55:41  mardan
# Added README and COPYING (license) FILES.
#
# Revision 1.6  2002/06/23 16:27:26  mardan
# Finished udp.
# Maybe some layout changes in future? therwise tool is finished.
#
# Revision 1.5  2002/06/23 14:07:46  mardan
# Added protocol arg option.
# Todo: udp protocol
#
# Revision 1.4  2002/06/23 12:57:35  mardan
# Added ident strings for test :-)
#
# Revision 1.3  2002/06/23 12:47:08  mardan
# Fixed resolved hostname hussle-up/layout
# Moved all source code into netstat-nat.c
#
# Revision 1.2  2002/06/23 11:56:09  mardan
# Added NAT icmp display.
# Still need to do udp (more states possible)
# Really need to fix resolved hostnames display, still hussled up.
#
# Revision 1.1.1.1  2002/05/04 01:08:06  mardan
# Initial import of netstat-nat, the C version.
# Array pointers really needs to be fixed, still lots of other bugs..
# So far only TCP displayed.
# No commandline args for e.g. no_nameresolving, protocol.
#
#
#                                                                                                                  
# Copyright (c) 2002 by D.Wijsman (mardan@tweegy.demon.nl). 
# All rights reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
# 
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#	       
#                                                                                                                         
#-------------------------------------------------------------------------------
*/


#include "netstat-nat.h"


static char const rcsid[] = "$Id: netstat-nat.c,v 1.17 2002/09/12 19:32:12 mardan Exp $";
char SRC_IP[35];
char DST_IP[35];
int SNAT = 1;
int DNAT = 1;
int LOCAL = 0;
char connection_table[MAX_CONN][ROWS][ROW_SIZE];
int connection_index = 0;


int main(int argc, char *argv[])
    {
    const char *args = "hnp:s:d:SDxor:L";
    static int SORT_ROW = 1;
    static int EXT_VIEW = 0;
    static int RESOLVE = 1;
    static int no_hdr = 0;
    static char PROTOCOL[4];
    FILE *f;
    char line[200];
    char src[50];
    char dst[50];
    char buf[100];
    char buf2[100];
    
    char *pa[MAX_CONN][ROWS];
    char *store;
    int index,a,b,c,j,r;
    
    // check parameters
    while ((c = getopt(argc, argv, args)) != -1 ) {
	switch (c) {
	case 'h':
	    display_help();
	    return 1;
	case '?':
	    display_help();
	    return 1;
	case 'n':
	    RESOLVE = 0;
	    break;
	case 'p':
	    strcpy (PROTOCOL, optarg);
	    break;
	case 's':
	    strcpy (SRC_IP, optarg);
	    lookup_ip(SRC_IP);
	    break;
	case 'd':
	    strcpy (DST_IP, optarg);
	    lookup_ip(DST_IP);
	    break;    
	case 'S':
	    DNAT = 0;
	    break;
	case 'D':
	    SNAT = 0;
	    break;
	case 'L':
	    SNAT = 0;
	    DNAT = 0;
	    LOCAL = 1;
	    break;
	case 'x':
	    EXT_VIEW = 1;
	    break;
	case 'o':
	    no_hdr = 1;
	    break;
	case 'r':
	    if (optarg == NULL || optarg == '\0') {
		display_help();
		return 1;
		}
	    if (strcmp(optarg, "scr") == 0) SORT_ROW = 1; //default
	    if (strcmp(optarg, "dst") == 0) SORT_ROW = 2;
	    if (strcmp(optarg, "src-port") == 0) SORT_ROW = 3; 
	    if (strcmp(optarg, "dst-port") == 0) SORT_ROW = 4; 
	    if (strcmp(optarg, "state") == 0) SORT_ROW = 5;
	    break; 
	}
    }
    
    // some checking for IPTables and read file
    f=fopen("/proc/net/ip_conntrack","r");
    if (!f) {
	printf("Make sure netfilter/IPtables is enabled by kernel or modules.\n");
	return 1;
	}
    
    // process conntrack table
    if (!no_hdr) {
	if (!EXT_VIEW) {
	    printf("%-6s%-31s%-31s%-6s\n","Proto","NATed Address","Foreign Address","State");
	} else {
	    printf("%-6s%-41s%-41s%-6s\n","Proto","NATed Address","Foreign Address","State");
	    }
	}

    while (fgets(line,1000,f)!=NULL) 
	{
	if ((!strcmp(PROTOCOL, "tcp")) || (!strcmp(PROTOCOL, ""))) {
	    if(match(line, "tcp")) {
		protocol_tcp(line);
		}
	    }
	    
	if ((!strcmp(PROTOCOL, "udp")) || (!strcmp(PROTOCOL, ""))) {
	    if((match(line, "udp")) && (match(line, "UNREPLIED"))) {
		protocol_udp_unr(line);
		}
	    if((match(line, "udp")) && (match(line, "ASSURED"))) {
		protocol_udp_ass(line);
		}
	    if((match(line, "udp")) && (!match(line, "ASSURED")) && (!match(line, "UNREPLIED"))) {
		protocol_udp(line); 
		}
	    }
	        
	if ((!strcmp(PROTOCOL, "icmp")) || (!strcmp(PROTOCOL, ""))) {
	    if((match(line, "icmp")) && (match(line, "UNREPLIED"))) {
		protocol_icmp_unr(line);
		}
	    if((match(line, "icmp")) && (!match(line, "UNREPLIED"))) {
		protocol_icmp_rep(line);
		}
	    }
	}
    fclose(f);
    
    // create array pointed to main connection array
    for (index = 0; index < connection_index; index++)	{
	for (j=0; j<ROWS; j++) {
	    pa[index][j] = &connection_table[index][j][0];
	    }
	}
    // sort by protocol and defined row
    for (a = 0;a < connection_index-1; a++) {
	for (b = a+1; b < connection_index; b++) {
	    r = strcmp(pa[a][0], pa[b][0]);
	    if (r > 0) {
		for (j=0;j<ROWS-1;j++) {
		    store = pa[a][j];
		    pa[a][j] = pa[b][j];
		    pa[b][j] = store;
		    }
		}	    
	    if (r == 0) {
		if (strcmp(pa[a][SORT_ROW], pa[b][SORT_ROW]) > 0) {
		    for (j=0;j<ROWS;j++) {
			store = pa[a][j];
			pa[a][j] = pa[b][j];
			pa[b][j] = store;
			}
		    }
		}
	    }
	}
    // print connections
    for (index = 0; index < connection_index; index++) {  
	if (RESOLVE) {
	    lookup_hostname(pa[index][1]);
	    lookup_hostname(pa[index][2]);
	    if (strlen(pa[index][3]) > 0 || strlen(pa[index][4]) > 0) {
		lookup_portname(pa[index][3], pa[index][0]);
		lookup_portname(pa[index][4], pa[index][0]);
	    	}
	    }
	if (!EXT_VIEW) {
	    strcpy(buf ,""); 
	    strncat(buf, pa[index][1], 29 - strlen(pa[index][3]));    
	    sprintf(buf2, "%s:%s", buf, pa[index][3]);
	    sprintf(src , "%-31s", buf2);
	    strcpy(buf ,""); 
	    strncat(buf, pa[index][2], 29 - strlen(pa[index][4]));    
	    sprintf(buf2, "%s:%s", buf, pa[index][4]);
	    sprintf(dst , "%-31s", buf2);
	} else {
	    strcpy(buf ,""); 
	    strncat(buf, pa[index][1], 39 - strlen(pa[index][3]));    
	    sprintf(buf2, "%s:%s", buf, pa[index][3]);
	    sprintf(src , "%-41s", buf2);
	    strcpy(buf ,""); 
	    strncat(buf, pa[index][2], 39 - strlen(pa[index][4]));    
	    sprintf(buf2, "%s:%s", buf, pa[index][4]);
	    sprintf(dst , "%-41s", buf2);
	    }
	printf("%-6s%s%s%-11s\n", pa[index][0], src, dst, pa[index][5]);
	}
    return(0);
    }

// -- NATed protocols

// NATed tcp protocol
void protocol_tcp(char *line)
    {
    char *token;
    char *buf[35];
    int count;
    token = strtok(line," ");
    count = 0;

    while(token != NULL) {
	buf[count] = token;
	if(!strlen(buf[count]))
	    buf[count]="";
	count++;
	token = strtok(NULL," ");
	}
    if ((match(buf[4], "=")) && (match(buf[5], "=")) && (match(buf[8], "=")) && (match(buf[9], "="))) {
	extract_ip(buf[4]);
	extract_ip(buf[9]);
	extract_ip(buf[5]);
	extract_ip(buf[8]);
	if (SNAT) {
	    if ((!strcmp(buf[4],buf[9])==0) && (strcmp(buf[5],buf[8])==0)) {		
		check_src_dst(buf[0],buf[4],buf[8],buf[6],buf[7],buf[3]);
		}
	    }
	if (DNAT) {
	    if ((strcmp(buf[4],buf[9])==0) && (!strcmp(buf[5],buf[8])==0)) {		
		check_src_dst(buf[0],buf[4],buf[8],buf[6],buf[7],buf[3]);
		}
	    }
	if (LOCAL) {
	    if ((strcmp(buf[4],buf[9])==0) && (strcmp(buf[5],buf[8])==0)) {		
		check_src_dst(buf[0],buf[4],buf[8],buf[6],buf[7],buf[3]);
		}
	    }
	}
    }

// NATed udp protocol
void protocol_udp(char *line) 
    {
    char *token;
    char *buf[35];
    int count;
    token = strtok(line," ");
    count = 0;
    
    while(token != NULL) {
	buf[count] = token;
	if(!strlen(buf[count]))
	    buf[count]="";
	count++;
	token = strtok(NULL," ");
 	}
    if ((match(buf[2], "=")) && (match(buf[3], "=")) && (match(buf[6], "=")) && (match(buf[6], "="))) {
        extract_ip(buf[2]);
	extract_ip(buf[3]);
	extract_ip(buf[6]);
	extract_ip(buf[7]);
	if (SNAT) {
	    if ((!strcmp(buf[2],buf[7])==0) && (strcmp(buf[3],buf[6])==0)) {	
		check_src_dst(buf[0],buf[2],buf[6],buf[4],buf[5]," ");
		}    
	    }
	if (DNAT) {
	    if ((strcmp(buf[2],buf[7])==0) && (!strcmp(buf[3],buf[6])==0)) {	
		check_src_dst(buf[0],buf[2],buf[6],buf[4],buf[5]," ");
		}    
	    }	
	if (LOCAL) {
	    if ((strcmp(buf[2],buf[7])==0) && (strcmp(buf[3],buf[6])==0)) {	
		check_src_dst(buf[0],buf[2],buf[6],buf[4],buf[5]," ");
		}    
	    }	
	
	}
    }

void protocol_udp_ass(char *line) 
    {
    char *token;
    char *buf[35];
    int count;
    token = strtok(line," ");
    count = 0;
    
    while(token != NULL) {
	buf[count] = token;
	if(!strlen(buf[count]))
	    buf[count]="";
	count++;
	token = strtok(NULL," ");
 	}
    if ((match(buf[3], "=")) && (match(buf[4], "=")) && (match(buf[7], "=")) && (match(buf[8], "="))) {
        extract_ip(buf[3]);
	extract_ip(buf[4]);
	extract_ip(buf[7]);
	extract_ip(buf[8]);
	if(SNAT) {
	    if ((!strcmp(buf[3],buf[8])==0) && (strcmp(buf[4],buf[7])==0)) {	
		check_src_dst(buf[0],buf[3],buf[7],buf[5],buf[6],buf[11]);
		}    
	    }
	if(DNAT) {
	    if ((strcmp(buf[3],buf[8])==0) && (!strcmp(buf[4],buf[7])==0)) {	
		check_src_dst(buf[0],buf[3],buf[7],buf[5],buf[6],buf[11]);
		}    
	    }
	if(LOCAL) {
	    if ((strcmp(buf[3],buf[8])==0) && (strcmp(buf[4],buf[7])==0)) {	
		check_src_dst(buf[0],buf[3],buf[7],buf[5],buf[6],buf[11]);
		}    
	    }
	}
    }

void protocol_udp_unr(char *line) 
    {
    char *token;
    char *buf[35];
    int count;
    token = strtok(line," ");
    count = 0;
    
    while(token != NULL) {
	buf[count] = token;
	if(!strlen(buf[count]))
	    buf[count]="";
	count++;
	token = strtok(NULL," ");
 	}
    if ((match(buf[3], "=")) && (match(buf[4], "=")) && (match(buf[8], "=")) && (match(buf[9], "="))) {
        extract_ip(buf[3]);
	extract_ip(buf[4]);
	extract_ip(buf[8]);
	extract_ip(buf[9]);
	if (SNAT) {
	    if ((!strcmp(buf[3],buf[9])==0) && (strcmp(buf[4],buf[8])==0)) {	
		check_src_dst(buf[0],buf[3],buf[8],buf[5],buf[6],buf[7]);
		}    
	    }
	if (DNAT) {
	    if ((strcmp(buf[3],buf[9])==0) && (!strcmp(buf[4],buf[8])==0)) {	
		check_src_dst(buf[0],buf[3],buf[8],buf[5],buf[6],buf[7]);
		}    
	    }
	if (LOCAL) {
	    if ((strcmp(buf[3],buf[9])==0) && (strcmp(buf[4],buf[8])==0)) {	
		check_src_dst(buf[0],buf[3],buf[8],buf[5],buf[6],buf[7]);
		}    
	    }
	}
    }

// NATed icmp protocol
void protocol_icmp_unr(char *line) 
    {
    char *token;
    char *buf[35];
    int count;
    token = strtok(line," ");
    count = 0;
    
    while(token != NULL) {
	buf[count] = token;
	if(!strlen(buf[count]))
	    buf[count]="";
	count++;
	token = strtok(NULL," ");
 	}
    if ((match(buf[3], "=")) && (match(buf[4], "=")) && (match(buf[9], "=")) && (match(buf[10], "="))) {
        extract_ip(buf[3]);
        extract_ip(buf[10]);
        extract_ip(buf[4]);
        extract_ip(buf[9]);
        if (SNAT) {
	    if ((!strcmp(buf[3],buf[10])==0) && (strcmp(buf[4],buf[9])==0)) {	
		check_src_dst(buf[0],buf[3],buf[9]," "," ",buf[8]);
    		}
	    }
	if (DNAT) {
	    if ((strcmp(buf[3],buf[10])==0) && (!strcmp(buf[4],buf[9])==0)) {	
    		check_src_dst(buf[0],buf[3],buf[9]," "," ",buf[8]);
		}    
	    }
	if (LOCAL) {
	    if ((strcmp(buf[3],buf[10])==0) && (strcmp(buf[4],buf[9])==0)) {	
    		check_src_dst(buf[0],buf[3],buf[9]," "," ",buf[8]);
		}    
	    }
	}
    }

void protocol_icmp_rep(char *line) 
    {
    char *token;
    char *buf[35];
    int count;
    token = strtok(line," ");
    count = 0;
    
    while(token != NULL) {
	buf[count] = token;
	if(!strlen(buf[count]))
	    buf[count]="";
	count++;
	token = strtok(NULL," ");
 	}
    if ((match(buf[3], "=")) && (match(buf[4], "=")) && (match(buf[8], "=")) && (match(buf[9], "="))) {
        extract_ip(buf[3]);
	extract_ip(buf[9]);
	extract_ip(buf[4]);
	extract_ip(buf[8]);
	if (SNAT) {
	    if ((!strcmp(buf[3],buf[9])==0) && (strcmp(buf[4],buf[8])==0)) {	
		check_src_dst(buf[0],buf[3],buf[8]," "," ","REPLIED");
		}
	    }
	if (DNAT) {
	    if ((strcmp(buf[3],buf[9])==0) && (!strcmp(buf[4],buf[8])==0)) {	
		check_src_dst(buf[0],buf[3],buf[8]," "," ","REPLIED");
		}
	    }
	if (LOCAL) {
	    if ((strcmp(buf[3],buf[9])==0) && (strcmp(buf[4],buf[8])==0)) {	
		check_src_dst(buf[0],buf[3],buf[8]," "," ","REPLIED");
		}
	    }
	}
    }
    
// -- End of NATed protocols

// -- Internal used functions
// Check filtering by source and destination IP
void check_src_dst(char *protocol, char *src_ip, char *dst_ip, char *src_port, char *dst_port, char *status) 
    {
    if ((check_if_source(src_ip)) && (strcmp(DST_IP,"")==0)) {
	store_data(protocol,src_ip,dst_ip,src_port,dst_port,status);
	}
    else if ((check_if_destination(dst_ip)) && (strcmp(SRC_IP,"")==0)) {
	store_data(protocol,src_ip,dst_ip,src_port,dst_port,status);
	}
    else if ((check_if_destination(dst_ip)) && (check_if_source(src_ip))) {
	store_data(protocol,src_ip,dst_ip,src_port,dst_port,status);
	}
    }

void store_data(char *protocol, char *src_ip, char *dst_ip, char *src_port, char *dst_port, char *status)  
    {
    char *split;
    char status_b[15];
    char protocol_b[5];
    char *token;
    char *buff;
    char msg[50];
    
    //ports
    if ((match(src_port, "=")) && (match(dst_port, "="))) {
	//source port
	split = strtok(src_port,"=");
	split = strtok(NULL,"=");
	src_port = split;
	strcpy(connection_table[connection_index][3], src_port);
	//destination port
	split = strtok(dst_port,"=");
	split = strtok(NULL,"=");
	dst_port = split;
	strcpy(connection_table[connection_index][4], dst_port);
	}
    //protocol
    strncpy(protocol_b, protocol, 5);
    strcpy(connection_table[connection_index][0], protocol);
    //IP
    if (strcmp(protocol_b,"icmp")!=0) {
	strcpy(connection_table[connection_index][1], src_ip);
	strcpy(connection_table[connection_index][2], dst_ip);
    } else {
	strcpy(connection_table[connection_index][1], src_ip);
	strcpy(connection_table[connection_index][2], dst_ip);
        }
    //status
    strcpy(status_b,status);
    token = strtok(status_b,"[");
    buff = token;
    token = strtok(NULL,"[");
    token = strtok(buff,"]");
    buff = token;
    token = strtok(NULL,"]");
    sprintf(msg, "%s",buff);
    strcpy(connection_table[connection_index][5], msg);
    connection_index++;
    }

void lookup_portname(char *port, char *proto)
    {
    char *buf_portname;
    char buf_port[10];
    int portnr;
    struct servent *service;
    
    strcpy(buf_port,port);
    portnr = htons(atoi(buf_port));
    if ((service = getservbyport(portnr, proto))) {
	strcpy(port,service->s_name);
	}
    }

void extract_ip(char *gen_buffer) 
    {
    char *split;
    split = strtok(gen_buffer,"=");
    split = strtok(NULL,"=");
    strcpy(gen_buffer,split);
    }

int lookup_hostname(char *r_host) 
    {
    int addr;
    struct hostent *hp;
    char **p;
    char *hostname;
    addr = inet_addr (r_host);
    hp=gethostbyaddr((char *) &addr, sizeof (addr), AF_INET);
    if (hp == NULL) {
	return 0;
	}

    for (p = hp->h_addr_list; *p!=0;p++){
	struct in_addr in;
	char **q;
	(void)memcpy(&in.s_addr, *p, sizeof(in.s_addr));
	strcpy(r_host, "");
	strcpy(r_host,hp->h_name);
	}
    return 0;
    }


int lookup_ip(char *hostname)
    {
    char *ip;
    struct hostent *hp;
    struct in_addr ip_addr;
    hp = gethostbyname(hostname);
    if(!hp) {
	printf("Unknown host: %s\n",hostname);
	exit(-1);
	}
    ip_addr = *(struct in_addr *)(hp->h_addr);
    ip = inet_ntoa(*(struct in_addr *)(hp->h_addr));
    strcpy(hostname,ip);
    return 1;
    }

int match(char *string, char *pattern) 
    {
    return strstr(string, pattern) ? 1 : 0;
    }

int check_if_source(char *host) 
    {
    if ((strcmp(host,SRC_IP)==0) || (strcmp(SRC_IP, "")==0)) {
	return 1;
	}
    return 0;
    }

int check_if_destination(char *host) 
    {
    if ((strcmp(host,DST_IP)==0) || (strcmp(DST_IP, "")==0)) {
	return 1;
	}
    return 0;
    }

void display_help()
    {
    printf("args: -h: displays this help\n");
    printf("      -n: don't resolve host/portnames\n");
    printf("      -p tcp | udp | icmp  : display connections by protocol\n");
    printf("      -s <source-host>     : display connections by source\n");
    printf("      -d <destination-host>: display connections by destination\n");
    printf("      -S: display SNAT connections\n");
    printf("      -D: display DNAT connections (default: SNAT & DNAT)\n");
    printf("      -L: display only connections to NAT box self (disables SNAT & DNAT)\n"); 
    printf("      -x: extended hostnames view\n");
    printf("      -r src | dst | src-port | dst-port | state : sort connections\n");
    printf("      -o: strip output header\n");
    }

    
// -- End of internal used functions

// -- The End --
