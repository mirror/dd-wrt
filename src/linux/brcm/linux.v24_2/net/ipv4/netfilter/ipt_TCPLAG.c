/* ipt_TCPLAG.c -- kernel module to implement TCPLAG target into netfilter
 * Copyright (C) 2002 Telford Tendys <telford@triode.net.au>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * This collects packets and attempts to make them into pairs
 * based on its own knowledge of how typical network conversations
 * operate. Once it has a pair, it logs the time between them.
 */
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/spinlock.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <linux/netfilter_ipv4/ip_tables.h>

#include <net/route.h>
#include <linux/netfilter_ipv4/ipt_TCPLAG.h>

#if 0
#define DEBUGP printk
#else
#define DEBUGP(format, args...)
#endif

/*
 * We need one spinlock for the hash table.
 */
static spinlock_t hash_lock = SPIN_LOCK_UNLOCKED;

typedef struct timeval timeval_T;

/*
 * Linked lists of events in the connection,
 * these store the SEQ numbers and the newest is always
 * at the start of the linked list, then they get older
 * down to the end of the linked list (this is not perfect
 * if packets get out of order but we don't worry about fine
 * details like that).
 *
 * Matching any event wipes out that event and also all other
 * events down the chain (i.e. all older events).
 * This keeps the linked list as short as possible.
 */
typedef struct tcplag_event_S
{
	struct tcplag_event_S *next;
	u16 source_port;
	u16 dest_port;
	u32 expected_ACK;
	struct timeval stamp;
} tcplag_event_T;

/*
 * This stores the connection statistics
 * We define connections more loosely than TCP/IP does,
 * because we only consider the two hosts, not the ports
 * Also, we list the host-pairs in low,high order which
 * means that we don't care who originated the connection.
 */
typedef struct tcplag_hash_S
{
	u32 low_ip;
	u32 high_ip;
	struct timeval lag_l_SEQ_h_ACK; /* l sends some data and h acknowledges that (sum of lag times) */
	struct timeval lag_h_SEQ_l_ACK; /* h sends some data and l acknowledges that (sum of lag times) */
	tcplag_event_T *h_ACK_list;     /* Try to match ACK packets coming from h in this list */
	tcplag_event_T *l_ACK_list;     /* Try to match ACK packets coming from l in this list */
	time_t stamp;                   /* When this bucket got added to the table */
	u16 count_l_SEQ_h_ACK;          /* Increment for each event */
	u16 count_h_SEQ_l_ACK;          /* Increment for each event */
} tcplag_hash_T;

static tcplag_hash_T **hashtab = 0;
static u32 hashsize = 0;
static u16 max_seconds = 30; /* Empty a hash bucket after this time */
static u32 reaper_ix = 0;

static void divide_down( timeval_T *T, int c )
{
	int remainder;

	T->tv_usec /= c;
	remainder = T->tv_sec % c; /* Only works properly with positive numbers */
	remainder *= 1000000;
	T->tv_usec == remainder;
	T->tv_sec /= c;
}

int diff_timeval( timeval_T *tv1, timeval_T *tv2 )
{
	register long x;

	x = tv1->tv_sec - tv2->tv_sec;
	if( x ) return( x );
	x = tv1->tv_usec - tv2->tv_usec;
	return( x );
}

void sprint_timeval( char *buf, timeval_T *tv )
{
	if( tv->tv_sec )
		sprintf( buf, "%lu%06lu", tv->tv_sec, tv->tv_usec );
	else
		sprintf( buf, "%lu", tv->tv_usec );
}

/*
 * This generates the log messages through printk()
 *
 * There is really no particular interest in the port numbers at this stage,
 * they are only useful for matching up the request with the reply.
 * The IP numbers are useful because some sites may be slower than others
 * or may travel different routes, etc (OK, in theory changing the port number
 * could also change the route but I don't like that sort of theory).
 *
 * The tags are:
 *
 * LIP=          The IP number of the side with the lowest lag
 * RIP=          The IP number of the side with the highest lag
 * LLAG=         The average time (in us) between RIP->LIP SEQ and LIP->RIP ACK
 * RLAG=         The average time (in us) between LIP->RIP SEQ and RIP->LIP ACK
 */
static void output( tcplag_hash_T *H, int level, const char *prefix )
{
	struct timeval ltm, rtm;
	u32 local_ip, remote_ip;
	char r_buf[ 20 ], l_buf[ 20 ];
/*
 * We can't make sense of a connection that only passes data one way,
 * In principle, at least the SYN and FIN should go both ways so we
 * should get a few hits for every connection.
 */
	if( 0 == H->count_l_SEQ_h_ACK || 0 == H->count_h_SEQ_l_ACK ) return;
/*
 * Calculate average times by dividing down
 */
	divide_down( &H->lag_l_SEQ_h_ACK, H->count_l_SEQ_h_ACK );
	divide_down( &H->lag_h_SEQ_l_ACK, H->count_h_SEQ_l_ACK );
/*
 * Sort these two by the lag so the the local is always the short lag
 */
	if( diff_timeval( &H->lag_l_SEQ_h_ACK, &H->lag_h_SEQ_l_ACK ) > 0 )
	{
		local_ip    = H->low_ip;
		remote_ip   = H->high_ip;
		rtm.tv_sec  = H->lag_l_SEQ_h_ACK.tv_sec;
		rtm.tv_usec = H->lag_l_SEQ_h_ACK.tv_usec;
		ltm.tv_sec  = H->lag_h_SEQ_l_ACK.tv_sec;
		ltm.tv_usec = H->lag_h_SEQ_l_ACK.tv_usec;
	}
	else
	{
		local_ip    = H->high_ip;
		remote_ip   = H->low_ip;
		ltm.tv_sec  = H->lag_l_SEQ_h_ACK.tv_sec;
		ltm.tv_usec = H->lag_l_SEQ_h_ACK.tv_usec;
		rtm.tv_sec  = H->lag_h_SEQ_l_ACK.tv_sec;
		rtm.tv_usec = H->lag_h_SEQ_l_ACK.tv_usec;
	}
/*
 * Don't use a spinlock on the output,
 * it is not guaranteed safe because some OTHER printk could
 * split our log message so we want only one single printk.
 *
 * We use sprintf() to partially pre-digest the output
 *
 * Actually, neither this not the main netfilter LOG target is
 * really safe from printk() overlap, basically syslog cannot
 * be regarded as a guaranteed data output channel. It is good
 * enough for most purposes.
 */
	sprint_timeval( l_buf, &ltm );
	sprint_timeval( r_buf, &rtm );
	printk( "<%d>%sLIP=%u.%u.%u.%u RIP=%u.%u.%u.%u LLAG=%s RLAG=%s\n",
			level & 7, prefix,
			NIPQUAD( local_ip ), NIPQUAD( remote_ip ),
			l_buf, r_buf );
}

/*
 * The reaper rolls through the hash table looking for old.
 * Log entries are only generated at the reaping time
 * (which means all log entries are out-of-date)
 */
static void reaper( time_t now, int level, const char *prefix )
{
	int i;

	now -= max_seconds;
	if( !hashsize ) return;
	if( !hashtab ) return;
	for( i = 0; i < 10; i++ )
	{
		if( ++reaper_ix >= hashsize ) reaper_ix = 0; 

//		DEBUGP( KERN_WARNING "reaper checking %u\n", reaper_ix );

		if( hashtab[ reaper_ix ])
		{
			tcplag_hash_T *found = 0;

			spin_lock_bh( &hash_lock );
			if( hashtab[ reaper_ix ])
			{
				if( now > hashtab[ reaper_ix ]->stamp )
				{
					DEBUGP( KERN_WARNING "reaper found expired entry\n" );
					found = hashtab[ reaper_ix ];
					hashtab[ reaper_ix ] = 0;
				}
			}
			spin_unlock_bh( &hash_lock );

			if( found )
			{
				output( found, level, prefix );
				kfree( found );
			}
		}
	}
}

/*
 * Convert the connection characteristics into a number
 * (not including the timestamp) FIXME: this is a sucky hash function
 */
static u32 make_hash( tcplag_hash_T *connection )
{
	register u32 r;

	r = connection->low_ip;
	r += connection->high_ip;
	return( r );
}

static int compare_connections( tcplag_hash_T *con1, tcplag_hash_T *con2 )
{
	int x;

	x = con1->low_ip - con2->low_ip; if( x ) return( x );
	x = con1->high_ip - con2->high_ip;
	return( x );
}

static int compare_events( tcplag_event_T *ev1, tcplag_event_T *ev2 )
{
	int x;

	DEBUGP( "Comparing sequence %u to %u\n", ev1->expected_ACK, ev2->expected_ACK );
	x = ev1->expected_ACK - ev2->expected_ACK;
	if( x ) return( x );
	DEBUGP( "Comparing source port %u to %u\n", ev1->source_port, ev2->source_port );
	x = ev1->source_port - ev2->source_port;
	if( x ) return( x );
	DEBUGP( "Comparing destination port %u to %u\n", ev1->dest_port, ev2->dest_port );
	x = ev1->dest_port - ev2->dest_port;
	return( x );
}

/*
 * Go to the hash table and either find an existing connection that
 * matches correctly or inject a new connection into the table.
 * Once the connection is OK, chain the event onto the linked list.
 */
static void hash_insert( tcplag_hash_T *connection, tcplag_event_T *event, int direction )
{
	u32 h, i;

	if( !event ) return; /* Just to be safe */
	if( !hashsize ) return;
	if( !hashtab ) return;

	h = make_hash( connection );
	h %= hashsize;

	DEBUGP( KERN_WARNING "hash_insert( %u )\n", h );

	spin_lock_bh( &hash_lock );
	for( i = 0; i < hashsize; i++, ({ if( ++h >= hashsize ) { h = 0; }}))
	{
		tcplag_hash_T *co_new = 0;
/*
 * Consider existing entry
 */
		if( hashtab[ h ])
		{
			if( compare_connections( hashtab[ h ], connection )) continue;
			co_new = hashtab[ h ];
			DEBUGP( KERN_WARNING "Existing connection at %u\n", h );
			goto add_link;
		}
/*
 * Use empty slot for new entry
 */
		if( !hashtab[ h ])
		{
			co_new = kmalloc( sizeof( tcplag_hash_T ), GFP_ATOMIC );
			memset( co_new, 0, sizeof( tcplag_hash_T ));
			co_new->low_ip = connection->low_ip;
			co_new->high_ip = connection->high_ip;
			co_new->stamp = event->stamp.tv_sec;
			hashtab[ h ] = co_new;
			DEBUGP( KERN_WARNING "Added connection to table at %u\n", h );
 add_link:
			{
				tcplag_event_T *ev_new;

				ev_new = kmalloc( sizeof( tcplag_event_T ), GFP_ATOMIC );
				memcpy( ev_new, event, sizeof( tcplag_event_T ));
				if( direction )
				{
					ev_new->next = co_new->h_ACK_list;
					co_new->h_ACK_list = ev_new;
					DEBUGP( KERN_WARNING "Connection at %u, direction is h_ACK_list\n", h );
				}
				else
				{
					ev_new->next = co_new->l_ACK_list;
					co_new->l_ACK_list = ev_new;
					DEBUGP( KERN_WARNING "Connection at %u, direction is l_ACK_list\n", h );
				}
			}
			goto done;
		}
	}
 done:
	spin_unlock_bh( &hash_lock );
}

/*
 * Search the hash table for a matching connection,
 * if we can't find one of those then we are stuffed.
 *
 * Once a connection has been found, scan along the list for
 * a matching SEQ number and if that is found then calculate
 * the lag, update the counters and cut the chain at the
 * point where the matching SEQ is found.
 */
static int request_complete( tcplag_hash_T *connection, tcplag_event_T *event, int direction )
{
	u32 h, i;

	if( !event ) return( 0 );
	if( !hashsize ) return( 0 );
	if( !hashtab ) return( 0 );
	h = make_hash( connection );
	h %= hashsize;

	DEBUGP( KERN_WARNING "request_complete( %u )\n", h );

	for( i = 0; i < hashsize; i++ )
	{
		tcplag_hash_T *found = 0;

		if( !hashtab[ h ]) return( 0 );

		spin_lock_bh( &hash_lock );
		if( hashtab[ h ])
		{
			if( !compare_connections( hashtab[ h ], connection ))
			{
				tcplag_event_T *ev, **evroot;
				timeval_T *tv;
				u16 *cn;

				found = hashtab[ h ];
				if( direction )
				{
					evroot = &found->h_ACK_list;
					tv = &found->lag_l_SEQ_h_ACK;
					cn = &found->count_l_SEQ_h_ACK;
					DEBUGP( KERN_WARNING "Connection at %u, direction is h_ACK_list\n", h );
				}
				else
				{
					evroot = &found->l_ACK_list;
					tv = &found->lag_h_SEQ_l_ACK;
					cn = &found->count_h_SEQ_l_ACK;
					DEBUGP( KERN_WARNING "Connection at %u, direction is l_ACK_list\n", h );
				}
				for( ev = *evroot; ev; ev = ev->next )
				{
					if( !compare_events( ev, event ))
					{
/*
 * Calculate the lag (in two parts) and add that to the collection
 */
						event->stamp.tv_sec -= ev->stamp.tv_sec;
						event->stamp.tv_usec -= ev->stamp.tv_usec;
						if( event->stamp.tv_usec < 0 )
						{
							event->stamp.tv_usec += 1000000;
							event->stamp.tv_sec++;
						}
						if( event->stamp.tv_sec < 0 )
						{
							DEBUGP( KERN_WARNING "Negative lag detected\n" );
						}
						else
						{
							tv->tv_sec += event->stamp.tv_sec;
							tv->tv_usec += event->stamp.tv_usec;
							++*cn;
							DEBUGP( KERN_WARNING "Found a match, added %lu.%06lu"
									" (accumulator is up to %lu.%06lu, %u events)\n",
									event->stamp.tv_sec,
									event->stamp.tv_usec,
									tv->tv_sec, tv->tv_usec, *cn );
						}
/*
 * Truncate the linked list.
 *
 * Visit each event in the list and return the memory to the pool.
 *
 * If a host is making multiple connections to the same remote host
 * then this truncation will result in some requests not being
 * monitored. Statistically we will still get some reasonable number
 * of measurements and multiple simultaneous connections between host
 * pairs don't happen all that often.
 */
						*evroot = 0;
						while( ev )
						{
							tcplag_event_T *ev_next = ev->next;
							DEBUGP( KERN_WARNING "Shitcan %u\n", ev->expected_ACK );
							kfree( ev );
							ev = ev_next;
						}
/*
 * TODO: overflow limit for *cn, force premature output() if necessary
 * (and drop this connection from the hash table)
 */
						break;
					}
				}
				goto done;
			}
		}
 done:
		spin_unlock_bh( &hash_lock );

		if( found ) return( 1 );
		if( ++h >= hashsize ) h = 0;
	}	
	return( 0 );
}

/*
 * Here is our target data:
 *
 * pskb      --  The packet itself (see linux/skbuff.h for breakdown)
 *
 * hooknum   --
 *
 * in        --  The device that this packet came in on
 *               (depending on the chain this may or may not exist)
 *
 * out       --  The device that this packet is just about to go
 *               out onto (again existance depends on the chain)
 *
 * targinfo  --  Our private data (handed through from iptables command util)
 *
 * userinfo  --  Some more data
 *
 */

static unsigned int target( struct sk_buff **pskb,
							unsigned int hooknum,
							const struct net_device *in,
							const struct net_device *out,
							const void *targinfo,
							void *userinfo )
{
	struct iphdr *iph = ( *pskb )->nh.iph;
	const struct ipt_tcplag *el = targinfo;
	tcplag_hash_T connection;
	tcplag_event_T event;
	int direction;
/*
 * We know we are dealing with IP here
 * Fill in all the obvious fields
 */
	if( iph->saddr > iph->daddr )
	{
		direction = 0;
		connection.high_ip = iph->saddr;
		connection.low_ip = iph->daddr;
	}
	else
	{
		direction = 1;
		connection.low_ip = iph->saddr;
		connection.high_ip = iph->daddr;
	}
	do_gettimeofday( &event.stamp );
/*
 * Do a bit of cleaning
 */
	reaper( event.stamp.tv_sec, el->level, el->prefix );

	DEBUGP( KERN_WARNING "got packet %lu %lu %s %s\n",
			event.stamp.tv_sec,
			event.stamp.tv_usec,
			in ? in->name : "none", out ? out->name : "none" );
/*
 * Now start looking at the details
 *
 * First step is to identify this packet to see if it is 
 * the sort of packet that we are interested in.
 * Don't hold any locks while we are doing this because often
 * we will just let the packet go without any further consideration.
 */
	switch( iph->protocol )
	{
		case IPPROTO_TCP:
		{
			struct tcphdr *tcp;

			if( ntohs( iph->frag_off ) & IP_OFFSET )
			{
				DEBUGP( KERN_WARNING "ignoring fragment\n" );
				break;
			}
			tcp = (struct tcphdr *)((u32 *)iph + iph->ihl );
			event.source_port = ntohs( tcp->source );
			event.dest_port = ntohs( tcp->dest );
/*
 * Every packet should have a valid SEQ number so use this to
 * generate an ACK number. This works along the formula:
 * -- Start with the SEQ number
 * -- For SYN or FIN add 1 to that number
 * -- For data packet, add the data length to that number
 */

/*
 * Data length requires a bit of fiddling around
 */
			{
				unsigned int data_len;
				if( tcp->syn || tcp->fin )
				{
					data_len = 1; /* Not real data, the SEQ clicks forward by 1 */
				}
				else
				{
					data_len = ntohs( iph->tot_len );
					data_len -= 4 * iph->ihl;  /* Subtract away IP header & options */
					data_len -= 4 * tcp->doff; /* Subtract away TCP header & options */
				}
				
				DEBUGP( KERN_WARNING "Data length calculated at %u\n", data_len );

				if( data_len ) /* Only track events that demand an ACK */
				{
					event.expected_ACK = ntohl( tcp->seq ) + data_len;
					hash_insert( &connection, &event, direction );
				}
				else
				{
					DEBUGP( "Don't bother to insert this, ACK not required\n" );
				}
			}

			if( tcp->ack )
			{
/*
 * Now we consider the matching of an existing event.
 * Reverse the port numbers and change the ACK number to the actual ACK number
 * Note that the direction is reversed because the reply will be going
 * the opposite way to the request.
 */
				event.expected_ACK = ntohl( tcp->ack_seq );
				event.dest_port = ntohs( tcp->source );
				event.source_port = ntohs( tcp->dest );
				request_complete( &connection, &event, !direction );
			}
			else
			{
				DEBUGP( "Don't bother to check this, ACK not valid\n" );
			}
		}
	}
	return( IPT_CONTINUE );
}

/*
 * return( 0 ) if there is a problem with this entry (i.e. kick it out of the kernel)
 * return( 1 ) if the entry is suitable
 *
 * tablename     --  
 *
 * e             --  
 *
 * targinfo      --  Our private data block (handed to us from iptables plug-in)
 *
 * targinfosize  --  The size of our private data block
 *
 * hook_mask     --  
 *
 *
 * Not much can go wrong for us, any illegal flags are harmlessly ignored,
 * all possible flag combos make sense. All we check for is correct data size.
 */
static int checkentry( const char *tablename,
					   const struct ipt_entry *e,
					   void *targinfo,
					   unsigned int targinfosize,
					   unsigned int hook_mask )
{
	const struct ipt_tcplag *el = targinfo;

	if( targinfosize != IPT_ALIGN( sizeof( struct ipt_tcplag )))
	{
		DEBUGP( "TCPLAG: targinfosize %u != %u\n", targinfosize,
				IPT_ALIGN( sizeof( struct ipt_tcplag )));
		return( 0 );
	}
	if( el->prefix[ 14 ]) return( 0 ); /* Be sure to have terminated string */
	return( 1 );
}

static struct ipt_target reg =
{
	{ 0, 0 },
	"TCPLAG",
	&target,
	&checkentry,
	0,
    THIS_MODULE
};

static int __init init( void )
{
	if( ipt_register_target( &reg )) return( -EINVAL );
	hashsize = 123; /* should be configurable */
	hashtab = kmalloc( sizeof( void * ) * hashsize, GFP_ATOMIC );
	memset( hashtab, 0, sizeof( void * ) * hashsize );
	return( 0 );
}

/*
 * This should not need locks (in theory)
 * because it can only get punted after it is no longer
 * chained into any of the netfilter lists.
 */
static void __exit fini( void )
{
	int i;

	ipt_unregister_target( &reg );
/*
 * Put back kernel memory
 */
	for( i = 0; i < hashsize; i++ )
	{
		tcplag_hash_T *p;

		if(( p = hashtab[ i ]))
		{
			tcplag_event_T *ev, *evn;

			hashtab[ i ] = 0;
			for( ev = p->h_ACK_list; ev; ev = evn )
			{
				evn = ev->next;
				kfree( ev );
			}
			for( ev = p->l_ACK_list; ev; ev = evn )
			{
				evn = ev->next;
				kfree( ev );
			}
			kfree( p );
		}
	}
	kfree( hashtab );
}

module_init(init);
module_exit(fini);
