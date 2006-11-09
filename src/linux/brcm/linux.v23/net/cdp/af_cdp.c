/********************************************************************

	linux/net/cdp/af_cdp.c
	Implements a Cisco Discovery Protocol handler.

 	This code is derived from protocol specifications by Cisco Systems
	and various code culled from the Kernel source tree, mostly the IPX
	code.

	Unless otherwise commented, all revisions by Chris Crowther.

	Revision 0.1.0:	Initial coding.
	Revision 0.1.1:	Incoming CDP packet handling working, prefix's and
			addresses need handling still.
	Revision 0.1.2:	010928 tb
			Code cleanup
			Fixed several memory leaks
	Revision 0.1.3: 011221 tb
			/proc/net/cdp_neighbors now looks like "sh cdp ne de"
	Revision 0.2.0: 011230 tb
			debug is an option to the module
			more decoding of addresses
			stricter error checking
	Revision 0.2.1: 020103 tb
			fixed extra garbage in debug output
			fixed address debug output
	Revision 0.2.2: 020109 tb
			fixed ipx/appletalk output
			they are reversed in CISCO documentation...
	Revision 0.2.3: 020307 tb
			fixup for 2.4.18:
				deleted	get_fast_time()
				added	do_gettimeofday()
			proper format for appletalk and IPX addresses

	Portions Copyright (c) 2001-2002 Chris Crowther and tom burkart.
	The authors admit no liability nor provide any warranty for this
	software.  This material is provided "AS-IS" and at no charge.

	This software is released under the the GNU Public Licence (GPL).


	Implementation notes:
	The neighbor list is implemented as a doubly linked list which gives
	a slight speed advantage during list member deletion.

 *******************************************************************/

#define MOD_NAME	"cdp"
#define MOD_VERSION	"0.2.3"
#define MOD_RELDATE	"2002/03/07"

#include <linux/config.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/sockios.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/init.h>

#include <asm/byteorder.h>
//#include <asm/checksum.h>	/* for csum_partial( char * , int, unsigned int ) */
#include <asm/system.h>

#include <net/psnap.h>
#include <net/cdp.h>


/* module info */
MODULE_AUTHOR( "Chris Crowther <chrisc@shad0w.org.uk> and tom burkart <tom@aussec.com>" );
MODULE_DESCRIPTION( "Linux Cisco Discovery Protocol" );
MODULE_LICENSE( "GPL" );

#ifdef CONFIG_CDP_DEBUG
static int debug = 2;		/* 0 quiet, 8 very verbose */

MODULE_PARM( debug, "i" );
MODULE_PARM_DESC( debug, "Linux CDP debug level (0-8)" );

#define	DEBUG(n, args...)	if (debug>=(n)) printk( KERN_DEBUG args )
#else
#define	DEBUG(n, args...)
#endif


/* globals */
#ifdef	SINGLE_LIST
static	struct	s_cdp_neighbor	*cdp_neighbors;
#else
static struct s_cdp_neighbors cdp_neighbors;
#endif
static struct timer_list cdp_poll_neighbors_timer;

/* init module function/globals */
static struct datalink_proto *pSNAP_datalink;
static unsigned char cdp_snap_id[5] = { 0x0, 0x0, 0x0c, 0x20, 0x00 };
static const char banner[] __initdata =
	KERN_INFO MOD_NAME ": Linux Cisco Discovery Protocol " MOD_VERSION "\n"
	KERN_INFO MOD_NAME ": " MOD_RELDATE " C. Crowther and t. burkart\n";

static const unsigned char proto_apollo_id[] =
	{ 0xaa, 0xaa, 0x03, 0x0, 0x0, 0x0, 0x80, 0x19 };
static const unsigned char proto_apple_id[] =
	{ 0xaa, 0xaa, 0x03, 0x0, 0x0, 0x0, 0x80, 0x9b };
static const unsigned char proto_decnet_id[] =
	{ 0xaa, 0xaa, 0x03, 0x0, 0x0, 0x0, 0x60, 0x03 };
static const unsigned char proto_ipv6_id[] =
	{ 0xaa, 0xaa, 0x03, 0x0, 0x0, 0x0, 0x08, 0x0 };
static const unsigned char proto_ipx_id[] =
	{ 0xaa, 0xaa, 0x03, 0x0, 0x0, 0x0, 0x81, 0x37 };
static const unsigned char proto_vines_id[] =
	{ 0xaa, 0xaa, 0x03, 0x0, 0x0, 0x0, 0x80, 0xc4 };
static const unsigned char proto_xns_id[] =
	{ 0xaa, 0xaa, 0x03, 0x0, 0x0, 0x0, 0x06, 0x0 };

/********************************************************************
	linked list functions
 *******************************************************************/

/* traverse the list and find the one with the given MAC address */
struct s_cdp_neighbor *cdp_find_neighbor( unsigned char *remote_ethernet ) {

	struct s_cdp_neighbor *p = cdp_neighbors.head;

	while ( p != NULL ) {
		DEBUG( 5, MOD_NAME " find: Neighbor   %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
			p->remote_ethernet[0],
			p->remote_ethernet[1],
			p->remote_ethernet[2],
			p->remote_ethernet[3],
			p->remote_ethernet[4],
			p->remote_ethernet[5] );
		if ( !memcmp( remote_ethernet, p->remote_ethernet, ETH_ALEN )) {
			return p;
		}
		else {
			p = p->next;
		}
	}

	/* no match found */
	return NULL;
}

/* free the element pointed to */
void cdp_free_neighbor( struct s_cdp_neighbor *p ) {
	if ( p->remote_ethernet )	kfree( p->remote_ethernet );
	if ( p->local_iface )		kfree( p->local_iface );
	if ( p->cdp_version )		kfree( p->cdp_version );
	if ( p->cdp_deviceID )		kfree( p->cdp_deviceID );
	if ( p->cdp_address )		kfree( p->cdp_address );
	if ( p->cdp_platform )		kfree( p->cdp_platform );
	if ( p->cdp_portID )		kfree( p->cdp_portID );
	if ( p->cdp_prefix )		kfree( p->cdp_prefix );
	kfree( p );
}

/* removes a neighbor element from the list, then frees it */
void cdp_delete_neighbor( struct s_cdp_neighbor *p ) {
	DEBUG( 6, MOD_NAME " delete: entry at %p\n", p );
	if ( p->prev )
		p->prev->next = p->next;

	if ( p->next )
		p->next->prev = p->prev;

	if ( cdp_neighbors.head == p ) {
		if ( p->next ) {
			cdp_neighbors.head = p->next;
			p->next->prev = NULL;
		}
		else
			cdp_neighbors.head = NULL;
	}

	if ( cdp_neighbors.foot == p ) {
		if ( p->prev ) {
			cdp_neighbors.foot = p->prev;
			p->prev->next = NULL;
		}
		else
			cdp_neighbors.foot = NULL;
	}

	cdp_free_neighbor( p );
}

/* update a neighbor entry with newer values */
int cdp_update_neighbor( struct sk_buff *skb, struct s_cdp_neighbor *p ) {

	/* We store all data locally in host byte order, which means we need
	 * to run ntoh[l/s] over all the numeric values (except single bytes
	 * of course).  It's safe to always run this since it has no effect
	 * on the data if the host byte order is the same network byte order
	 * (as is the case with Big Endian systems).
	 */

	/* temporary holders for packet data */
	unsigned short type = 0, length = 0, checksum, i;
	unsigned char *data = NULL;
	unsigned char proto_ver;
	#ifdef CONFIG_CDP_DEBUG
	unsigned long flags;
	#endif

	/* If we have less than 4 bytes of data, we don't have a valid CDP
	 * packet, return NULL so we know to toast the record
	 */
	if( skb->len < 4 )
		return 0;

	/* copy the timestamp from the skb to the neighbor struct so we know
	 * when it arrived, and therefore know when to expire it
	 */
	memcpy( &(p->timestamp), &(skb->stamp), sizeof(struct timeval) );
	memcpy( &proto_ver, (skb->data)++, 1 );
	memcpy( &(p->cdp_ttl), (skb->data)++, 1 );
	memcpy( &checksum, (skb->data)++, 2 ); (skb->data)++;
	checksum = ntohs( checksum );

	/* FIXME I haven't got around to actually checking this yet - we
	 * should really do it now
	 */
	/* csum_partial( data, length, sum ); */

	/* FIXME  compare the checksum here as a kludge to see if there's
	 * any change in data
	 */
	if ( p->cdp_checksum == checksum ) {
		DEBUG( 6, MOD_NAME " update: checksum same... not updating\n" );
		return 1;
	}
	/* update the checksum */
	p->cdp_checksum = checksum;

	/* mark the thing as in use - this is really for true updates only */
	p->cdp_proto_ver = 0;

	if ( p->remote_ethernet == NULL &&
			(p->remote_ethernet =
				(unsigned char *)kmalloc(
					sizeof(unsigned char)*ETH_ALEN,
					GFP_ATOMIC ) )
			 == NULL ) {
		printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for neighbor key member.\n" );
		return 0;
	}
	else {
		memcpy( p->remote_ethernet,
			(char *)(skb->mac.ethernet->h_source),
			ETH_ALEN );
	}

	if ( p->local_iface == NULL &&
			(p->local_iface =
				(char *)kmalloc(
					IFNAMSIZ,
					GFP_ATOMIC ) )
			== NULL ) {
		printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for local interface member.\n" );
		kfree( p->remote_ethernet );
		return 0;
	}
	else {
		strncpy( p->local_iface, (char *)(skb->dev->name), IFNAMSIZ );
	}

	/* only go while we're at least 4 less than the tail value - we can't
	 * actually go equal to it, since we must have at least 4 bytes
	 * left for our type and length fields
	 */
	while ( (skb->tail-skb->data) > 4 ) {
		type = 0;
		length = 0;

		memcpy( &type, (skb->data)++, 2 ); (skb->data)++;
		memcpy( &length, (skb->data)++, 2 ); (skb->data)++;
		/* make sure the byte order is right */
		type = ntohs( type );
		/* total length includes the "type" and "length" fields */
		length = ntohs( length ) - 4;

		data = (unsigned char *)kmalloc(
				sizeof(unsigned char)*length,
				GFP_ATOMIC );
		memcpy( data, (skb->data), length );

		/* jump to the end of this section of data */
		skb->data += length;

		DEBUG( 6, MOD_NAME " update: Data: Type = %.4X, Length = %d bytes\n",
			type, length );

		/* process the data */
		switch (type) {
#if 0
			case CDP_TYPE_IPPREFIX:
				if ( p->cdp_prefix ) kfree( p->cdp_prefix );
				p->cdp_prefix = (char *)kmalloc(
						sizeof(char)*length,
						GFP_ATOMIC );
				if ( p->cdp_prefix )
					memcpy( p->cdp_prefix, data, length );
				else
					printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for prefix.\n" );
				break;
#endif

			case CDP_TYPE_PLATFORM:
				if ( p->cdp_platform ) kfree( p->cdp_platform );
				p->cdp_platform = (char *)kmalloc(
						sizeof(char)*length+1,
						GFP_ATOMIC );
				if ( p->cdp_platform ) {
					memcpy( p->cdp_platform, data, length );
					*(p->cdp_platform+length) = '\000';
				}
				else
					printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for platform.\n" );
				break;

			case CDP_TYPE_VERSION:
				if ( p->cdp_version ) kfree( p->cdp_version );
				p->cdp_version = (char *)kmalloc( sizeof(char)*length+1, GFP_ATOMIC );
				if ( p->cdp_version ) {
					memcpy( p->cdp_version, data, length );
					*(p->cdp_version+length) = '\000';
				}
				else
					printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for version.\n" );
				break;

			case CDP_TYPE_CAPABILITIES:
				if ( length == 4 ) {
					memcpy( &(p->cdp_capabilities), data, length );
					p->cdp_capabilities = ntohl( p->cdp_capabilities );
					DEBUG( 7, MOD_NAME " update: Capabilities = %.8lx\n", p->cdp_capabilities );
				}
				else
					DEBUG( 7, MOD_NAME " update: Capabilities had length %i\n", length );
				break;

			case CDP_TYPE_PORTID:
				if ( p->cdp_portID ) kfree( p->cdp_portID );
				p->cdp_portID = (char *)kmalloc(
						sizeof(char)*length+1,
						GFP_ATOMIC );
				if ( p->cdp_portID ) {
					memcpy( p->cdp_portID, data, length );
					*(p->cdp_portID+length) = '\000';
				}
				else
					printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for portID.\n" );
				break;

			case CDP_TYPE_ADDRESS:
				if ( p->cdp_address ) kfree( p->cdp_address );
				p->cdp_address = (char *)kmalloc(
						sizeof(char)*length,
						GFP_ATOMIC );
				if ( p->cdp_address )
					memcpy( p->cdp_address, data, length );
				else
					printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for address.\n" );
				break;

			case CDP_TYPE_DEVICEID:
				if ( p->cdp_deviceID ) kfree( p->cdp_deviceID );
				p->cdp_deviceID = (char *)kmalloc(
						sizeof(char)*length+1,
						GFP_ATOMIC );
				if ( p->cdp_deviceID ) {
					memcpy( p->cdp_deviceID, data, length );
					*(p->cdp_deviceID+length) = '\000';
				}
				else
					printk( KERN_CRIT MOD_NAME " update: Could not allocate memory for deviceID.\n" );
				break;

			default:
				/* do nothing if we don't know what type it is */
				#ifdef CONFIG_CDP_DEBUG
				if ( debug >= 1 ) {
					save_flags( flags );
					cli();
					printk( KERN_DEBUG MOD_NAME " update: Unknown type %#.4x, length %i\n" KERN_DEBUG, type, length );
					for ( i=0; i<length; ++i ) {
						printk( " %.2x", data[i] );
						if ( i%8 == 7 )
							printk( "\n" KERN_DEBUG );
					}
					printk( "\n" );
					restore_flags( flags );
				}
				#endif
				break;
		}

		/* free the temporary data store */
		kfree( data );

	}

	/* the "not busy any more" marker */
	p->cdp_proto_ver = proto_ver;
	DEBUG( 6, MOD_NAME " update: Ver %i, TTL %i, Checksum %#.4x\n",
		p->cdp_proto_ver, p->cdp_ttl, p->cdp_checksum );

	return 1;
}

/* add a new neighbor */
struct s_cdp_neighbor *cdp_add_neighbor( struct sk_buff *skb ) {

	struct s_cdp_neighbor *p;

	if ( (p = (struct s_cdp_neighbor *)kmalloc(
				sizeof(struct s_cdp_neighbor),
				GFP_ATOMIC ) )
			== NULL ) {
		printk( KERN_CRIT MOD_NAME " add: Cannot allocate memory for new neighbor.\n" );
		return NULL;
	}
	else {
		DEBUG( 5, MOD_NAME " add: Allocated neighbor at: %p\n", p );

		/* initialise all the fields so they have some data */
		p->remote_ethernet = NULL;
		p->local_iface = NULL;
		p->cdp_proto_ver = 0;		/* make this illegal so that we know
						 * this record is incomplete */
		p->cdp_ttl = 15;		/* a default TTL so it will time out */
		p->cdp_checksum = 0;
		p->cdp_address = NULL;
		p->cdp_version = NULL;
		p->cdp_deviceID = NULL;
		p->cdp_capabilities = 0L;
		p->cdp_platform = NULL;
		p->cdp_portID = NULL;
		p->cdp_prefix = NULL;
		p->next = p->prev = NULL;

		/* Try and assign values, if we can't, blow the struct away */
		if ( !cdp_update_neighbor( skb, p ) ) {
			cdp_free_neighbor( p );
			return NULL;
		}

#ifdef	SINGLE_LIST
		/* add it to the head of the list */
		p->next = cdp_neighbors;
		cdp_neighbors = p;
#else
		/* add it at the end of the list */
		if ( cdp_neighbors.head == NULL ) {
			cdp_neighbors.foot = cdp_neighbors.head = p;
		}
		else {
			p->prev = cdp_neighbors.foot;
			cdp_neighbors.foot->next = p;
			cdp_neighbors.foot = p;
		}
#endif
	}

	return p;
}



/********************************************************************
	proc fs functions
 ********************************************************************/

/* print the address data contained in the CDP packet */
int cdp_print_addresses( unsigned char *p, char *buffer ) {

/* 4 bytes - no of addresses
 * 1 byte proto type
 * 1 byte no of bytes of proto id following
 * 1-8 bytes proto id
 * 2 bytes length of address following
 * ? bytes of address
 */
	unsigned long no_of_addresses;
	unsigned short proto_id_len, addr_len;
	short index = 4;
	int len, i, printaddress = 1;
	/* initialise the next two to shut the compiler up */
	int format = 0;			/* 0 = hex, 1 = decimal */
	char separator = 0;		/* char between address octets */
	#ifdef CONFIG_CDP_DEBUG
	unsigned long flags;
	#endif

	len = sprintf( buffer, "Device addresses:\n" );
	memcpy( &no_of_addresses, p, 4 );
	no_of_addresses = ntohl( no_of_addresses );
	while ( no_of_addresses-- ) {
		proto_id_len = p[index+1];
		memcpy( &addr_len, p+index+proto_id_len+2, 2 );
		addr_len = ntohs( addr_len );
		format = -1;
		separator = ' ';

		/* FIXME - proper address handling */
		if ( *(p+index) == 1 && proto_id_len == 1 ) {
			if ( *(p+index+2) == 0xcc ) {
				len += sprintf( buffer+len, "  IP address:" );
				separator = '.';
				format = 1;
			}
			else if ( *(p+index+2) == 0x81 ) {
				len += sprintf( buffer+len, "  ISO CLNS address:" );
				separator = ' ';
				format = 1;
			}
		}
		else if ( *(p+index) == 2 && proto_id_len == 8 ) {
			if ( !memcmp( p+index+2, proto_apollo_id, 8 ) ) {
				len += sprintf( buffer+len, "  Apollo Domain address:" );
				separator = ' ';
				format = 1;
			}
			else if ( !memcmp( p+index+2, proto_apple_id, 8 ) ) {
				len += sprintf( buffer+len, "  Appletalk address:" );
				/* it is supposed to be 3, else std print */
				if ( addr_len == 3 ) {
					len += sprintf( buffer+len, " %u.%i",
						(unsigned int)p[ index+proto_id_len+4 ] * 256 + (unsigned int)p[ index+proto_id_len+5 ],
						p[ index+proto_id_len+6 ]
					);
					printaddress = 0;
				}
				else {
					separator = '.';
					format = 1;
				}
			}
			else if ( !memcmp( p+index+2, proto_decnet_id, 8 ) ) {
				len += sprintf( buffer+len, "  DECNET IV address:" );
				separator = ' ';
				format = 1;
			}
			else if ( !memcmp( p+index+2, proto_ipv6_id, 8 ) ) {
				len += sprintf( buffer+len, "  IPv6 address:" );
				separator = ':';
				format = 0;
			}
			else if ( !memcmp( p+index+2, proto_ipx_id, 8 ) ) {
				len += sprintf( buffer+len, "  Novell address:" );
				/* have 80 bits of address */
				if ( addr_len == 10 ) {
					len += sprintf( buffer+len,
						" %02x%02x%02x%02x.%02x%02x.%02x%02x.%02x%02x",
						p[ index+proto_id_len+4 ],
						p[ index+proto_id_len+5 ],
						p[ index+proto_id_len+6 ],
						p[ index+proto_id_len+7 ],
						p[ index+proto_id_len+8 ],
						p[ index+proto_id_len+9 ],
						p[ index+proto_id_len+10 ],
						p[ index+proto_id_len+11 ],
						p[ index+proto_id_len+12 ],
						p[ index+proto_id_len+13 ]
					);
					printaddress = 0;
				}
				else {
					separator = ' ';
					format = 0;
				}
			}
			else if ( !memcmp( p+index+2, proto_vines_id, 8 ) ) {
				len += sprintf( buffer+len, "  Banyan VINES address:" );
				separator = ' ';
				format = 1;
			}
			else if ( !memcmp( p+index+2, proto_xns_id, 8 ) ) {
				len += sprintf( buffer+len, "  XNS address:" );
				separator = ' ';
				format = 1;
			}
		}
		else {
			len += sprintf( buffer+len, "  Unknown address:" );
		}
#ifdef CONFIG_CDP_DEBUG
		if ( debug >= 2 && format < 0 ) {
			save_flags( flags );
			cli();
			printk( KERN_DEBUG MOD_NAME " paddr: proto_id %d:", proto_id_len );
			for ( i=0; i<proto_id_len; ++i )
				printk( " %.2x", p[index+2+i] );
			printk( "\n" );
			restore_flags( flags );
		}
#endif

		/* now print the address */
		if ( printaddress )
			for ( i=0; i<addr_len; ++i )
				len += sprintf( buffer+len,
					format?"%c%i":"%c%.2x",
					(i==0)?' ':separator,
					p[ index+4+proto_id_len+i ] );
		len += sprintf( buffer+len, "\n" );

		/* now bump the index up to the next entry */
		index += proto_id_len + addr_len + 4;
	}
	return len;
}


/* do the data display for /proc/net/cdp_neighbors query */
int cdp_get_neighbor_info( char *buffer, char **start, off_t offset, int length ) {
	struct s_cdp_neighbor *p = cdp_neighbors.head;
	struct timeval sometime;
	unsigned long flags;
	int len=0;
	off_t pos=0;
	off_t begin=0;

	do_gettimeofday( &sometime );

	save_flags( flags );
	cli();

	while ( p ) {
		/* only process it when the version is not zero */
		if ( p->cdp_proto_ver ) {
			if ( p->local_iface )
				len += sprintf(
					buffer+len,
					"Port (Our Port): %s\n",
					p->local_iface );
			if ( p->cdp_deviceID )
				len += sprintf(
					buffer+len,
					"Device-ID: %s\n",
					p->cdp_deviceID );
			if ( p->cdp_address )
				len += cdp_print_addresses( p->cdp_address, buffer+len );
			len += sprintf(
					buffer+len,
					"Holdtime: %d sec\nCapabilities:",
					(int)(p->cdp_ttl+p->timestamp.tv_sec-sometime.tv_sec) );
			if ( p->cdp_capabilities & CDP_CAPABILITY_L3R )
				len += sprintf( buffer+len, " Router" );
			if ( p->cdp_capabilities & CDP_CAPABILITY_L2TB )
				len += sprintf( buffer+len, " Transparent_Bridge" );
			if ( p->cdp_capabilities & CDP_CAPABILITY_L2SRB )
				len += sprintf( buffer+len, " SR_Bridge" );
			if ( p->cdp_capabilities & CDP_CAPABILITY_L2SW )
				len += sprintf( buffer+len, " Switch" );
			if ( p->cdp_capabilities & CDP_CAPABILITY_L3TXRX )
				len += sprintf( buffer+len, " Host" );
			if ( p->cdp_capabilities & CDP_CAPABILITY_IGRP )
				len += sprintf( buffer+len, " IGRP" );
			if ( p->cdp_capabilities & CDP_CAPABILITY_L1 )
				len += sprintf( buffer+len, " Repeater" );
			len += sprintf( buffer+len, "\n" );
			if ( p->cdp_version )
				len += sprintf(
					buffer+len,
					"Version:\n%s\n",
					p->cdp_version );
			if ( p->cdp_platform )
				len += sprintf(
					buffer+len,
					"Platform: %s\n",
					p->cdp_platform );
			if ( p->cdp_portID )
				len += sprintf(
					buffer+len,
					"Port-ID (Port on Neighbor's Device): %s\n",
					p->cdp_portID );
		}
		p = p->next;
		if ( p )
			len += sprintf( buffer+len, "\n" );

		pos = begin + len;
		if( pos < offset ) {
			len=0;
			begin=pos;
		}
		if( pos > offset+length )
			break;
	}

	restore_flags( flags );

	*start = buffer + (offset-begin);
	len -= offset - begin;
	if ( len > length )
		len=length;

	return len;
}



/********************************************************************
	network functions
 ********************************************************************/

/* deletes timed out neighbor entries (ie TTL expired) */
static void SMP_TIMER_NAME(cdp_check_expire)( unsigned long unused ) {
	struct timeval sometime;
	struct s_cdp_neighbor *p = cdp_neighbors.head;
	unsigned	long	flags;

	mod_timer( &cdp_poll_neighbors_timer, jiffies + CDP_POLL /* now + CDP_POLL */ );

	do_gettimeofday( &sometime );
	save_flags( flags );
	cli();

	/* now go through the neighbor list */
	while ( p ) {
		if ( (p->timestamp.tv_sec+p->cdp_ttl) < sometime.tv_sec ) {
			if ( p->next ) {
				p = p->next;
				cdp_delete_neighbor( p->prev );
			}
			else {
				cdp_delete_neighbor( p );
				break;
			}
		}
		else
			p = p->next;
	}
	restore_flags( flags );
}
SMP_TIMER_DEFINE( cdp_check_expire, cdp_expire_task );

/* function triggered by arriving CDP packets, as determined by the SNAP ID */
int cdp_rcv( struct sk_buff *skb, struct net_device *dev, struct packet_type *pt ) {

	struct s_cdp_neighbor *p = NULL;

	DEBUG( 5, MOD_NAME " rcv: Packet from %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
		skb->mac.ethernet->h_source[0],
		skb->mac.ethernet->h_source[1],
		skb->mac.ethernet->h_source[2],
		skb->mac.ethernet->h_source[3],
		skb->mac.ethernet->h_source[4],
		skb->mac.ethernet->h_source[5] );

	p = cdp_find_neighbor( skb->mac.ethernet->h_source );

	if ( p ) {
		DEBUG( 5, MOD_NAME " rcv: Updating neighbor at: %p\n", p );
		if ( !cdp_update_neighbor( skb, p ) ) {
			/* update failed - delete entry */
			DEBUG( 5, MOD_NAME " rcv: Update of neighbor failed - deleting\n" );
			cdp_delete_neighbor( p );
		}
	}
	else {
		DEBUG( 5, MOD_NAME " rcv: Creating new neighbor.\n" );
		cdp_add_neighbor( skb );
	}

	/* we're done with the packet now */
	/* don't need to free it as it gets done in p8022.c */

	return 0;
}



/********************************************************************
	module init and exit functions
 ********************************************************************/

static int __init cdp_init( void ) {
	pSNAP_datalink = register_snap_client( cdp_snap_id, cdp_rcv );
	if ( !pSNAP_datalink ) {
		printk( KERN_CRIT MOD_NAME ": Unable to register with SNAP\n" );
		return 0;
	}

	/* initialise the neighbors linked list */
#ifdef	SINGLE_LIST
	cdp_neighbors = NULL;
#else
	cdp_neighbors.head = cdp_neighbors.foot = NULL;
#endif

	/* initialise and register timer */
	cdp_poll_neighbors_timer.function = cdp_check_expire;
	cdp_poll_neighbors_timer.expires = jiffies + CDP_POLL;
	add_timer( &cdp_poll_neighbors_timer );

	/* create /proc/net entry, if proc fs is enabled */
#ifdef CONFIG_PROC_FS
        proc_net_create( "cdp_neighbors", 0, cdp_get_neighbor_info );
#endif
	printk( banner );
	return 0;
}


static void __exit cdp_quit( void ) {

	struct s_cdp_neighbor *p = cdp_neighbors.head;

	unregister_snap_client( cdp_snap_id );
	pSNAP_datalink = NULL;

#ifdef CONFIG_PROC_FS
	proc_net_remove( "cdp_neighbors" );
#endif

	/* remove timers */
	del_timer( &cdp_poll_neighbors_timer );

	/* Unallocate all the neighbor entries */
	while ( p ) {
		cdp_delete_neighbor( p );
		p = cdp_neighbors.head;
	}

       cdp_neighbors.foot = cdp_neighbors.head = NULL;

       printk( KERN_INFO MOD_NAME ": Exiting\n" );
}

module_init( cdp_init );
module_exit( cdp_quit );
