/*
#-------------------------------------------------------------------------------
#                                                                                                                         
# $Id: netstat-nat.h,v 1.3 2002/09/08 20:23:48 mardan Exp $     
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>


#define MAX_CONN 	2048	/* maximum connections in array */
#define ROWS		6
#define ROW_SIZE	70


void protocol_tcp(char *line);
void protocol_udp(char *line);
void protocol_udp_ass(char *line);
void protocol_udp_unr(char *line);
void protocol_icmp_unr(char *line);
void protocol_icmp_rep(char *line);
void check_src_dst(char *protocol, char *src_ip, char *dst_ip, char *src_port, char *dst_port, char *status);
void store_data(char *protocol, char *src_ip, char *dst_ip, char *src_port, char *dst_port, char *status);
void extract_ip(char *gen_buffer);
void display_help();
int lookup_hostname(char *r_host);
int lookup_ip(char *hostname);
int match(char *string, char *pattern);
int check_if_source(char *host);
int check_if_destination(char *host);
void lookup_portname(char *port,char *proto);


/* The End */
