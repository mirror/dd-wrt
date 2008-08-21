/*
 * iptable.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <rc.h>
#include <iptables.h>

static char range_buf[500] = "";

// iptc_handle_t handle = NULL;
/*
 * Modify from iptables-standalone.c
 */
/*
 * int iptables_main (int argc, char *argv[]) { int ret = 0; char *table =
 * "filter"; iptc_handle_t handle = NULL;
 * 
 * program_name = "iptables"; program_version = NETFILTER_VERSION;
 * 
 * ret = do_command(argc, argv, &table, &handle);
 * 
 * if (ret) ret = iptc_commit(&handle);
 * 
 * if (!ret) fprintf(stderr, "iptables: %s\n", iptc_strerror(errno));
 * 
 * return !ret; } 
 */
int set_rule( char *_argv[], int flag )
{
    int ret = 0;

    /*
     * int _argc = 0; char *table = "filter"; iptc_handle_t handle = NULL;
     * #ifdef DEBUG char buf[120]; strcpy(buf,""); #endif
     * 
     * program_name = "iptables"; program_version = NETFILTER_VERSION;
     * 
     * do{ #ifdef DEBUG sprintf(buf+strlen(buf),"%s ",_argv[_argc]); #endif
     * _argc++; }while(_argv[_argc] != NULL);
     * 
     * #ifdef DEBUG printf("rule[%d]=[ %s]%s\n",rule_count,buf,(flag == 0 ?
     * "(no)" : "")); rule_count ++; #endif
     * 
     * if(!flag) return ret;
     * 
     * ret = do_command(_argc, _argv, &table, &handle);
     * 
     * if (ret) ret = iptc_commit(&handle);
     * 
     * if (!ret) fprintf(stderr, "iptables: %s\n", iptc_strerror(errno));
     * 
     */ return !ret;
}

int sameaddr( unsigned char *sin, unsigned char *ein )
{
    int i;

    for( i = 0; i < 4; i++ )
	if( sin[i] != ein[i] )
	    return 0;
    return 1;
}

void
getse( unsigned char *ip, unsigned char *nets, unsigned char *nete,
       int bitlen )
{
    int i, j, len, remain;
    unsigned char mask;

    len = bitlen / 8;
    remain = bitlen % 8;

    for( i = 0; i < len; i++ )
	nets[i] = nete[i] = ip[i];

    nets[i] = 0;
    nete[i] = 0;

    for( j = 0; j < 8; j++ )
    {
	mask = 0x80 >> j;
	if( j < remain )
	{
	    if( mask & ip[i] )
	    {
		nets[i] |= mask;
		nete[i] |= mask;
	    }
	}
	else
	    nete[i] |= mask;
    }

    i++;

    for( ; i < 4; i++ )
    {
	nets[i] = 0x00;
	nete[i] = 0xff;
    }
}

int count_bits( unsigned char *sin, unsigned char *ein )
// return the same bits count from beginning
{
    int i, j, sbit, ebit;
    int len = 0;
    unsigned char mask;

    for( i = 0; i < 4; i++ )
    {
	if( sin[i] == ein[i] )
	    len += 8;
	else
	{
	    mask = 0x80;
	    for( j = 0; j < 8; j++ )
	    {
		sbit = ( mask & sin[i] );
		ebit = ( mask & ein[i] );
		if( sbit == ebit )
		    len++;
		else
		    break;
		mask >>= 1;
	    }
	    break;
	}
    }
    return len;
}

void subrange( unsigned char *sin, unsigned char *ein )
// recursive function to divide ip range
{
    unsigned char nets[4], nete[4];
    int bitlen, nextlen;

    bitlen = count_bits( sin, ein );

    if( bitlen == 32 )
    {
	sprintf( range_buf + strlen( range_buf ), "%u.%u.%u.%u/32 ", sin[0],
		 sin[1], sin[2], sin[3] );
	cprintf( "%u.%u.%u.%u/32\n", sin[0], sin[1], sin[2], sin[3] );
	return;
    }
    else if( bitlen == 31 )
    {
	sprintf( range_buf + strlen( range_buf ), "%u.%u.%u.%u/31 ", sin[0],
		 sin[1], sin[2], sin[3] );
	cprintf( "%u.%u.%u.%u/31\n", sin[0], sin[1], sin[2], sin[3] );
	return;
    }

    nextlen = bitlen + 1;

    getse( sin, nets, nete, bitlen );

    if( sameaddr( sin, nets ) && sameaddr( ein, nete ) )
    {
	sprintf( range_buf + strlen( range_buf ), "%u.%u.%u.%u/%d ", nets[0],
		 nets[1], nets[2], nets[3], bitlen );
	cprintf( "%u.%u.%u.%u/%d\n", nets[0], nets[1], nets[2], nets[3],
		 bitlen );
	return;
    }

    getse( sin, nets, nete, nextlen );

    if( sameaddr( sin, nete ) )
    {
	sprintf( range_buf + strlen( range_buf ), "%u.%u.%u.%u/32 ", sin[0],
		 sin[1], sin[2], sin[3] );
	cprintf( "%u.%u.%u.%u/32\n", sin[0], sin[1], sin[2], sin[3] );
    }
    else if( sameaddr( sin, nets ) )
    {
	sprintf( range_buf + strlen( range_buf ), "%u.%u.%u.%u/%d ", nets[0],
		 nets[1], nets[2], nets[3], nextlen );
	cprintf( "%u.%u.%u.%u/%d\n", nets[0], nets[1], nets[2], nets[3],
		 nextlen );
    }
    else			// continue check
	subrange( sin, nete );

    getse( ein, nets, nete, nextlen );

    if( sameaddr( ein, nets ) )
    {
	sprintf( range_buf + strlen( range_buf ), "%u.%u.%u.%u/32 ", ein[0],
		 ein[1], ein[2], ein[3] );
	cprintf( "%u.%u.%u.%u/32\n", ein[0], ein[1], ein[2], ein[3] );
    }
    else if( sameaddr( ein, nete ) )
    {
	sprintf( range_buf + strlen( range_buf ), "%u.%u.%u.%u/%d ", nets[0],
		 nets[1], nets[2], nets[3], nextlen );
	cprintf( "%u.%u.%u.%u/%d\n", nets[0], nets[1], nets[2], nets[3],
		 nextlen );
    }
    else			// continue check
	subrange( nets, ein );

}

char *range( char *start, char *end )
{

    unsigned char startipc[4], endipc[4];
    unsigned int startip[4], endip[4];

    int retcount = 0;
    int i;

    cprintf( "start=[%s] end=[%s]\n", start, end );

    strcpy( range_buf, "" );

    retcount =
	sscanf( start, "%u.%u.%u.%u", &startip[0], &startip[1], &startip[2],
		&startip[3] );

    retcount +=
	sscanf( end, "%u.%u.%u.%u", &endip[0], &endip[1], &endip[2],
		&endip[3] );

    if( retcount != 8 )
    {
	printf( "Error ip address!\n" );
    }

    for( i = 0; i < 4; i++ )
    {
	if( ( startip[i] > 255 ) || ( endip[i] > 255 )
	    || ( startip[i] > endip[i] ) )
	{
	    printf( "Out of range!\n" );
	}
	startipc[i] = ( unsigned char )startip[i];
	endipc[i] = ( unsigned char )endip[i];
    }

    subrange( startipc, endipc );

    cprintf( "range_buf=[%s]\n", range_buf );

    return ( char * )&range_buf;

}

#ifdef DEBUG_IPTABLE
int range_main( int argc, char *argv[] )
{
    /*
     * char *sub; char var[500], *next;
     * 
     * sub = range("192.168.1.1","192.168.1.100");
     * dprintf("range_buf=[%s]\n",sub);
     * 
     * foreach(var, sub, next){ dprintf("[%s]\n",var); char
     * *_argv[]={"iptables","-I","FORWARD","-o","eth1","-p","tcp","-m","tcp","--dport","500","-s",var,"-j","DROP",NULL};
     * set_rule(_argv,1);
     * 
     * }
     * 
     */ return 0;
}

int rule_main( int argc, char *argv[] )
{

    /*
     * int i; long start,end;
     * 
     * start=time(0); for(i=0;i<atoi(argv[1]);i++){ char
     * *_argv[]={"iptables","-A","FORWARD","-o","eth1","-p","tcp","-m","tcp","--dport","500","-s","192.168.1.100","-j","DROP",NULL};
     * set_rule(_argv,1); } //iptc_commit(&handle); end=time(0);
     * 
     * printf("start=[%ld] end=[%ld]\n",start,end);
     * 
     */ return 0;
}
#endif
