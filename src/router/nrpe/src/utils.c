/****************************************************************************
 *
 * UTILS.C - NRPE Utility Functions
 *
 * License: GPL
 * Copyright (c) 1999-2006 Ethan Galstad (nagios@nagios.org)
 *
 * Last Modified: 12-11-2006
 *
 * Description:
 *
 * This file contains common network functions used in nrpe and check_nrpe.
 *
 * License Information:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ****************************************************************************/

#include "../include/common.h"
#include "../include/utils.h"

static unsigned long crc32_table[256];



/* build the crc table - must be called before calculating the crc value */
void generate_crc32_table(void){
	unsigned long crc, poly;
	int i, j;

	poly=0xEDB88320L;
	for(i=0;i<256;i++){
		crc=i;
		for(j=8;j>0;j--){
			if(crc & 1)
				crc=(crc>>1)^poly;
			else
				crc>>=1;
		        }
		crc32_table[i]=crc;
                }

	return;
        }


/* calculates the CRC 32 value for a buffer */
unsigned long calculate_crc32(char *buffer, int buffer_size){
	register unsigned long crc;
	int this_char;
	int current_index;

	crc=0xFFFFFFFF;

	for(current_index=0;current_index<buffer_size;current_index++){
		this_char=(int)buffer[current_index];
		crc=((crc>>8) & 0x00FFFFFF) ^ crc32_table[(crc ^ this_char) & 0xFF];
	        }

	return (crc ^ 0xFFFFFFFF);
        }


/* fill a buffer with semi-random data */
void randomize_buffer(char *buffer,int buffer_size){
	FILE *fp;
	int x;
	int seed;

	/**** FILL BUFFER WITH RANDOM ALPHA-NUMERIC CHARACTERS ****/

	/***************************************************************
	   Only use alpha-numeric characters becase plugins usually
	   only generate numbers and letters in their output.  We
	   want the buffer to contain the same set of characters as
	   plugins, so its harder to distinguish where the real output
	   ends and the rest of the buffer (padded randomly) starts.
	***************************************************************/

	/* try to get seed value from /dev/urandom, as its a better source of entropy */
	fp=fopen("/dev/urandom","r");
	if(fp!=NULL){
		seed=fgetc(fp);
		fclose(fp);
	        }

	/* else fallback to using the current time as the seed */
	else
		seed=(int)time(NULL);

	srand(seed);
	for(x=0;x<buffer_size;x++)
		buffer[x]=(int)'0'+(int)(72.0*rand()/(RAND_MAX+1.0));

	return;
        }


/* opens a connection to a remote host/tcp port */
int my_tcp_connect(char *host_name,int port,int *sd){
	int result;

	result=my_connect(host_name,port,sd,"tcp");

	return result;
        }


/* opens a tcp or udp connection to a remote host */
int my_connect(char *host_name,int port,int *sd,char *proto){
	struct sockaddr_in servaddr;
	struct hostent *hp;
	struct protoent *ptrp;
	int result;

	bzero((char *)&servaddr,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(port);

	/* try to bypass using a DNS lookup if this is just an IP address */
	if(!my_inet_aton(host_name,&servaddr.sin_addr)){

		/* else do a DNS lookup */
		hp=gethostbyname((const char *)host_name);
		if(hp==NULL){
			printf("Invalid host name '%s'\n",host_name);
			return STATE_UNKNOWN;
		        }

		memcpy(&servaddr.sin_addr,hp->h_addr,hp->h_length);
	        }

	/* map transport protocol name to protocol number */
	if(((ptrp=getprotobyname(proto)))==NULL){
		printf("Cannot map \"%s\" to protocol number\n",proto);
		return STATE_UNKNOWN;
	        }

	/* create a socket */
	*sd=socket(PF_INET,(!strcmp(proto,"udp"))?SOCK_DGRAM:SOCK_STREAM,ptrp->p_proto);
	if(*sd<0){
		printf("Socket creation failed\n");
		return STATE_UNKNOWN;
	        }

	/* open a connection */
	result=connect(*sd,(struct sockaddr *)&servaddr,sizeof(servaddr));
	if(result<0){
		switch(errno){  
		case ECONNREFUSED:
			printf("Connection refused by host\n");
			break;
		case ETIMEDOUT:
			printf("Timeout while attempting connection\n");
			break;
		case ENETUNREACH:
			printf("Network is unreachable\n");
			break;
		default:
			printf("Connection refused or timed out\n");
		        }

		return STATE_CRITICAL;
	        }

	return STATE_OK;
        }



/* This code was taken from Fyodor's nmap utility, which was originally taken from
   the GLIBC 2.0.6 libraries because Solaris doesn't contain the inet_aton() funtion. */
int my_inet_aton(register const char *cp, struct in_addr *addr){
	register unsigned int val;	/* changed from u_long --david */
	register int base, n;
	register char c;
	u_int parts[4];
	register u_int *pp = parts;

	c=*cp;

	for(;;){

		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit((int)c))
			return (0);
		val=0;
		base=10;

		if(c=='0'){
			c=*++cp;
			if(c=='x'||c=='X')
				base=16,c=*++cp;
			else
				base=8;
		        }

		for(;;){
			if(isascii((int)c) && isdigit((int)c)){
				val=(val*base)+(c -'0');
				c=*++cp;
			        } 
			else if(base==16 && isascii((int)c) && isxdigit((int)c)){
				val=(val<<4) | (c+10-(islower((int)c)?'a':'A'));
				c = *++cp;
			        } 
			else
				break;
		        }

		if(c=='.'){

			/*
			 * Internet format:
			 *	a.b.c.d
			 *	a.b.c	(with c treated as 16 bits)
			 *	a.b	(with b treated as 24 bits)
			 */
			if(pp>=parts+3)
				return (0);
			*pp++=val;
			c=*++cp;
		        } 
		else
			break;
	        }

	/* Check for trailing characters */
	if(c!='\0' && (!isascii((int)c) || !isspace((int)c)))
		return (0);

	/* Concoct the address according to the number of parts specified */
	n=pp-parts+1;
	switch(n){

	case 0:
		return (0);		/* initial nondigit */

	case 1:				/* a -- 32 bits */
		break;

	case 2:				/* a.b -- 8.24 bits */
		if(val>0xffffff)
			return (0);
		val|=parts[0]<<24;
		break;

	case 3:				/* a.b.c -- 8.8.16 bits */
		if(val>0xffff)
			return (0);
		val|=(parts[0]<< 24) | (parts[1]<<16);
		break;

	case 4:				/* a.b.c.d -- 8.8.8.8 bits */
		if(val>0xff)
			return (0);
		val|=(parts[0]<<24) | (parts[1]<<16) | (parts[2]<<8);
		break;
	        }

	if(addr)
		addr->s_addr=htonl(val);

	return (1);
        }


void strip(char *buffer){
	int x;
	int index;

	for(x=strlen(buffer);x>=1;x--){
		index=x-1;
		if(buffer[index]==' ' || buffer[index]=='\r' || buffer[index]=='\n' || buffer[index]=='\t')
			buffer[index]='\x0';
		else
			break;
	        }

	return;
        }


/* sends all data - thanks to Beej's Guide to Network Programming */
int sendall(int s, char *buf, int *len){
	int total=0;
	int bytesleft=*len;
	int n=0;

	/* send all the data */
	while(total<*len){

		/* send some data */
		n=send(s,buf+total,bytesleft,0);

		/* break on error */
		if(n==-1)
			break;

		/* apply bytes we sent */
		total+=n;
		bytesleft-=n;
	        }

	/* return number of bytes actually send here */
	*len=total;

	/* return -1 on failure, 0 on success */
	return n==-1?-1:0;
        }


/* receives all data - modelled after sendall() */
int recvall(int s, char *buf, int *len, int timeout){
	int total=0;
	int bytesleft=*len;
	int n=0;
	time_t start_time;
	time_t current_time;
	
	/* clear the receive buffer */
	bzero(buf,*len);

	time(&start_time);

	/* receive all data */
	while(total<*len){

		/* receive some data */
		n=recv(s,buf+total,bytesleft,0);

		/* no data has arrived yet (non-blocking socket) */
		if(n==-1 && errno==EAGAIN){
			time(&current_time);
			if(current_time-start_time>timeout)
				break;
			sleep(1);
			continue;
		        }

		/* receive error or client disconnect */
		else if(n<=0)
			break;

		/* apply bytes we received */
		total+=n;
		bytesleft-=n;
	        }

	/* return number of bytes actually received here */
	*len=total;

	/* return <=0 on failure, bytes received on success */
	return (n<=0)?n:total;
        }


/* fixes compiler problems under Solaris, since strsep() isn't included */
/* this code is taken from the glibc source */
char *my_strsep (char **stringp, const char *delim){
	char *begin, *end;

	begin = *stringp;
	if (begin == NULL)
		return NULL;

	/* A frequent case is when the delimiter string contains only one
	   character.  Here we don't need to call the expensive `strpbrk'
	   function and instead work using `strchr'.  */
	if(delim[0]=='\0' || delim[1]=='\0'){
		char ch = delim[0];

		if(ch=='\0')
			end=NULL;
		else{
			if(*begin==ch)
				end=begin;
			else
				end=strchr(begin+1,ch);
			}
		}

	else
		/* Find the end of the token.  */
		end = strpbrk (begin, delim);

	if(end){

		/* Terminate the token and set *STRINGP past NUL character.  */
		*end++='\0';
		*stringp=end;
		}
	else
		/* No more delimiters; this is the last token.  */
		*stringp=NULL;

	return begin;
	}


/* show license */
void display_license(void){

	printf("This program is released under the GPL (see below) with the additional\n");
	printf("exemption that compiling, linking, and/or using OpenSSL is allowed.\n\n");

	printf("This program is free software; you can redistribute it and/or modify\n");
	printf("it under the terms of the GNU General Public License as published by\n");
	printf("the Free Software Foundation; either version 2 of the License, or\n");
	printf("(at your option) any later version.\n\n");
	printf("This program is distributed in the hope that it will be useful,\n");
	printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
	printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
	printf("GNU General Public License for more details.\n\n");
	printf("You should have received a copy of the GNU General Public License\n");
	printf("along with this program; if not, write to the Free Software\n");
	printf("Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n");

	return;
        }



