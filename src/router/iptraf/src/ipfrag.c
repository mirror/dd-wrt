/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

ipfrag.c - module that handles fragmented IP packets.

This module is necessary to maintain accurate counts in case
fragmented IP packets are received.  TCP and UDP headers are not copied
in fragments.

This module is based on RFC 815, but does not really reassemble packets.
The routines here merely accumulate packet sizes and pass them off to
the IP traffic monitor routine.

***/

#include "iptraf-ng-compat.h"

#include "ipfrag.h"
#include "list.h"

struct fragdescent {
	unsigned int min;
	unsigned int max;
	struct fragdescent *prev_entry;
	struct fragdescent *next_entry;
};

struct fragent {
	struct list_head fragent_list;
	unsigned long s_addr;
	in_port_t s_port;
	unsigned long d_addr;
	in_port_t d_port;
	unsigned int id;
	unsigned int protocol;
	int firstin;
	struct fragdescent *fragdesclist;
	struct fragdescent *fragdesctail;
	unsigned int bcount;
};

LIST_HEAD(frag_head);

static struct fragent *addnewdgram(struct iphdr *packet)
{
	struct fragent *ptmp;

	ptmp = xmallocz(sizeof(struct fragent));
	list_add_tail(&ptmp->fragent_list, &frag_head);

	ptmp->fragdesclist = xmalloc(sizeof(struct fragdescent));
	ptmp->fragdesclist->min = 0;
	ptmp->fragdesclist->max = 65535;
	ptmp->fragdesclist->next_entry = NULL;
	ptmp->fragdesclist->prev_entry = NULL;
	ptmp->fragdesctail = ptmp->fragdesclist;

	ptmp->s_addr = packet->saddr;
	ptmp->d_addr = packet->daddr;
	ptmp->protocol = packet->protocol;
	ptmp->id = packet->id;

	return ptmp;
}

static struct fragdescent *addnewhole(struct fragent *frag)
{
	struct fragdescent *ptmp;

	ptmp = xmalloc(sizeof(struct fragdescent));

	if (frag->fragdesclist == NULL) {
		frag->fragdesclist = ptmp;
		ptmp->prev_entry = NULL;
	}
	if (frag->fragdesctail != NULL) {
		frag->fragdesctail->next_entry = ptmp;
		ptmp->prev_entry = frag->fragdesctail;
	}
	ptmp->next_entry = NULL;
	frag->fragdesctail = ptmp;

	return ptmp;
}

static struct fragent *searchfrags(unsigned long saddr, unsigned long daddr,
				   unsigned int protocol, unsigned int id)
{
	struct fragent *ftmp;

	list_for_each_entry(ftmp, &frag_head, fragent_list) {
		if ((saddr == ftmp->s_addr) && (daddr == ftmp->d_addr)
		    && (protocol == ftmp->protocol) && (id == ftmp->id))
			return ftmp;
	}

	return NULL;
}

static void deldgram(struct fragent *ftmp)
{
	list_del(&ftmp->fragent_list);
	free(ftmp);
}

/* destroy hole descriptor list */
static void destroyholes(struct fragent *ftmp)
{
	struct fragdescent *dtmp = ftmp->fragdesclist;

	while (dtmp != NULL) {
		struct fragdescent *ntmp = dtmp->next_entry;

		free(dtmp);
		dtmp = ntmp;
	}
}

void destroyfraglist(void)
{
	struct fragent *entry, *tmp;

	list_for_each_entry_safe(entry, tmp, &frag_head, fragent_list) {
		destroyholes(entry);
		deldgram(entry);
	}
}

/*
 * Process IP fragment.  Returns number of bytes to report to the traffic
 * monitor or 0 for an error condition.
 */

unsigned int processfragment(struct iphdr *packet, in_port_t *sport,
			     in_port_t *dport, int *firstin)
{
	struct fragent *ftmp;
	struct fragdescent *dtmp;
	struct fragdescent *ntmp;
	char *tpacket;

	unsigned int offset;
	unsigned int lastbyte;
	unsigned int retval;

	/* Determine appropriate hole descriptor list */

	ftmp =
	    searchfrags(packet->saddr, packet->daddr, packet->protocol,
			packet->id);

	if (ftmp == NULL)	/* No such datagram for this frag yet */
		ftmp = addnewdgram(packet);

	if (ftmp == NULL)
		return 0;

	/* 
	 * At this point, ftmp should contain the address of the appropriate
	 * descriptor list.
	 */

	dtmp = ftmp->fragdesclist;	/* Point to hole descriptors */
	offset = ipv4_frag_offset(packet);
	lastbyte = (offset + (ntohs(packet->tot_len) - (packet->ihl) * 4)) - 1;

	if (ipv4_is_first_fragment(packet)) {	/* first fragment? */
		ftmp->firstin = 1;
		tpacket = ((char *) (packet)) + (packet->ihl * 4);
		if (packet->protocol == IPPROTO_TCP) {
			ftmp->s_port = ntohs(((struct tcphdr *) tpacket)->source);
			ftmp->d_port = ntohs(((struct tcphdr *) tpacket)->dest);
		} else if (packet->protocol == IPPROTO_UDP) {
			ftmp->s_port = ntohs(((struct udphdr *) tpacket)->source);
			ftmp->d_port = ntohs(((struct udphdr *) tpacket)->dest);
		}
	}
	while (dtmp != NULL) {
		if ((offset <= dtmp->max) && (lastbyte >= dtmp->min))
			break;

		dtmp = dtmp->next_entry;
	}

	if (dtmp != NULL) {	/* Duplicate/overlap or something out of the
				   loopback interface */
		/* 
		 * Delete current entry from hole descriptor list
		 */

		if (dtmp->prev_entry != NULL)
			dtmp->prev_entry->next_entry = dtmp->next_entry;
		else
			ftmp->fragdesclist = dtmp->next_entry;

		if (dtmp->next_entry != NULL)
			dtmp->next_entry->prev_entry = dtmp->prev_entry;
		else
			ftmp->fragdesctail = dtmp->prev_entry;

		/*
		 * Memory for the hole descriptor will not be released yet.
		 */

		if (offset > dtmp->min) {
			/*
			 * If offset in fragment is greater than offset in the descriptor,
			 * create a new hole descriptor.
			 */

			ntmp = addnewhole(ftmp);
			ntmp->min = dtmp->min;
			ntmp->max = offset - 1;
		}
		if ((lastbyte < dtmp->max)
		    && ipv4_more_fragments(packet)) {
			/*
			 * If last byte in fragment is less than the last byte of the
			 * hole descriptor, and more fragments, create a new hole
			 * descriptor.
			 */

			ntmp = addnewhole(ftmp);
			ntmp->min = lastbyte + 1;
			ntmp->max = dtmp->max;
		}
		free(dtmp);

	}
	*firstin = ftmp->firstin;

	ftmp->bcount += ntohs(packet->tot_len);

	if (ftmp->firstin) {
		*sport = ftmp->s_port;
		*dport = ftmp->d_port;
		retval = ftmp->bcount;
		ftmp->bcount = 0;

		if (ftmp->fragdesclist == NULL)
			deldgram(ftmp);

		return retval;
	} else
		return 0;
}
