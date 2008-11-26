/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/igmp.h>
#include <linux/if_ether.h>
#include <linux/list.h>
#include <linux/inetdevice.h>
#include <net/checksum.h>

/* To enable debugging uncomment CONFIG_IGMPS_DEBUG */
#define CONFIG_IGMPS_DEBUG

#ifdef CONFIG_IGMPS_DEBUG
#define IGMPSDEBUG(format, args...)  printk(format , ## args)
#else
#define IGMPSDEBUG(format, args...)
#endif

/* APIs exported by the "mv_gwy_main.c" */
extern int mv_gtw_enable_igmp(void); 
extern int mv_gtw_set_mac_addr_to_switch(unsigned char *mac_addr, unsigned char db, unsigned int ports_mask, unsigned char op);

/* MAC node data structure (a member of "ipm_llist") */
typedef struct mac_node 
{
    struct list_head ip_list;
    unsigned char eth_addr[ETH_ALEN];
    unsigned int port_bitmap;
    struct list_head list_node;
}_mac_node;

/* IP node data structure */
typedef struct ip_node 
{
    unsigned int ip_port_bitmap;
    unsigned int ip_addr;
    struct list_head ip_list_node;
}_ip_node;

static int _igmp_add_mac_node( unsigned char temp_addr[ETH_ALEN], unsigned char vlan_dbnum,
				__u32		igmp_addr,
				__u32		port_bitmap);
			
/* Linked list of the MAC addresses */
static LIST_HEAD(ipm_llist);

/* Main process routine */
int mv_gtw_igmp_snoop_process(struct sk_buff* skb, unsigned char port, unsigned char vlan_dbnum)     
{
    struct iphdr      *ipheader;
    struct igmphdr    *igmpheader;
    __u32             igmp_addr, igmp_addr_host;
    unsigned char     mac_found, ip_found, ip_port_found;
    unsigned char     temp_mac_addr[ETH_ALEN];
    struct list_head  *l, *ip_l;
    __u32             temp_port_bitmap = 0;
    _mac_node         *mac_node_entry;
    _ip_node          *ip_node_entry;
    __u16 	      old_igmp_csum, new_igmp_csum;

    ipheader = (struct iphdr*)(skb->data);

    /* First of all, let's filter out non-IGMP traffic */
    if (ipheader->protocol != IPPROTO_IGMP)
        goto out;
		
    /* Get the pointer to the IGMP header and its data */
    igmpheader = (struct igmphdr*)((__u32)ipheader + (__u32)(ipheader->ihl * 4));

    /* Filter out unsupported IGMP messages */
    if ((igmpheader->type != IGMP_HOST_MEMBERSHIP_REPORT) &&
	(igmpheader->type != IGMPV2_HOST_MEMBERSHIP_REPORT) &&
	(igmpheader->type != IGMP_HOST_LEAVE_MESSAGE))    	
	goto out;
    
    /* Filter out non-multicast messages */
    if (!MULTICAST(igmpheader->group))
    {
    	printk(KERN_ERR "\nIGMP snoop: Non-multicast group address in IGMP header\n");
	goto out;
    }
    
    /* According to "draft-ietf-magma-snoop-12.txt" local multicast messages (224.0.0.x) must be flooded to all ports */
    /* So, don't do anything with such messages */
    if (LOCAL_MCAST(igmpheader->group))
    {
    	IGMPSDEBUG(KERN_INFO "\nIGMP snoop: Local IGMP messages (224.0.0.x) must be flooded \n");
	goto out;
    }
    
    /* According to RFC 2236 IGMP LEAVE messages should be sent to ALL-ROUTERS address (224.0.0.2) */
    if (igmpheader->type == IGMP_HOST_LEAVE_MESSAGE)
    {
    	if (ntohl(ipheader->daddr) != 0xE0000002)
	{
		IGMPSDEBUG(KERN_INFO "\nIGMP snoop: Ignore IGMP LEAVE message sent to non-ALL-ROUTERS address (224.0.0.2) \n");
		goto out;
	}	
    }
    else
    {
    	/* According to RFC 2236 Membership Report (JOIN) IGMP messages should be sent to the IGMP group address */
	if (ipheader->daddr != igmpheader->group)
    	{
    		IGMPSDEBUG(KERN_INFO "\nIGMP snoop: Ignore IGMP JOIN message with different destination IP(%u.%u.%u.%u) and IGMP group address(%u.%u.%u.%u) \n",
			NIPQUAD(ipheader->daddr), NIPQUAD(igmpheader->group));
		goto out;
   	 }
    }
    
    /* According to RFC 2236 the IGMP checksum must be checked before processing */
    /* Before re-calculation of the IGMP checksum one needs to zero the checksum field */
    old_igmp_csum = igmpheader->csum;
    igmpheader->csum = 0;
	 
    /* Calculate a new checksum */
    new_igmp_csum = ip_compute_csum((void *)igmpheader, sizeof(struct igmphdr));
	
    /* Restore the old checksum */
    igmpheader->csum = old_igmp_csum;
	
    if (old_igmp_csum != new_igmp_csum)
    {
	printk(KERN_ERR "\nIGMP snoop: Wrong IGMP message checksum\n");
	goto out;
    }	
        
    igmp_addr = igmpheader->group;
    igmp_addr_host = ntohl(igmpheader->group);

    /* Build the multicast MAC address */
    temp_mac_addr[0] = 0x01;
    temp_mac_addr[1] = 0x00;
    temp_mac_addr[2] = 0x5E;
    temp_mac_addr[3] = (igmp_addr_host>>16) & 0x000000FF;
    temp_mac_addr[4] = (igmp_addr_host>>8) & 0x000000FF;
    temp_mac_addr[5] = igmp_addr_host & 0x000000FF;

    /* Set the bit corresponding to the source port */
    temp_port_bitmap = 1 << port;
        
    switch(igmpheader->type)
    { 
	case IGMP_HOST_MEMBERSHIP_REPORT:
	case IGMPV2_HOST_MEMBERSHIP_REPORT:	
	    IGMPSDEBUG(KERN_INFO "\n\nIGMP snoop: %s JOIN (if=\"%s\", group=%u.%u.%u.%u, port=%d, vlanDB=%d)\n",
		(igmpheader->type == IGMP_HOST_MEMBERSHIP_REPORT) ? "IGMPv1" : "IGMPv2",
		 skb->dev->name, NIPQUAD(igmp_addr), port, vlan_dbnum);
			
	    /* If the DB (linked list) is empty, create it and add the MAC node  */
	    if (list_empty(&ipm_llist)) {
		IGMPSDEBUG(KERN_INFO "IGMP snoop: Empty MAC node list \n");
		if (_igmp_add_mac_node(temp_mac_addr, vlan_dbnum, igmp_addr, port) != 0) {
		    printk(KERN_ERR "IGMP snoop: Failed to add MAC node \n");
		    goto out;
		}
	    }
	    else {
		/* Non-empty MAC node list */
		l = ipm_llist.next;
		mac_found = 0;

		IGMPSDEBUG(KERN_INFO "IGMP snoop: Searching for MAC(01:00:5e:%x:%x:%x) ...\n", 
		   temp_mac_addr[3], temp_mac_addr[4], temp_mac_addr[5]); 

		/* Check all entries of the MAC node list */
		do {
		    mac_node_entry = list_entry(l, struct mac_node, list_node);	
	
		    /* Check whether the node's MAC address is equal to the current one */
		    if (memcmp(temp_mac_addr,mac_node_entry->eth_addr,ETH_ALEN) == 0) {

			IGMPSDEBUG(KERN_INFO "IGMP snoop: MAC address was found in the list\n");
			mac_found = 1; /* MAC node was found */
	
			/* Now look for ip entry */
			ip_l = mac_node_entry->ip_list.next;
			ip_found = 0;
	
			/* Check all entries of the IP address list */
			IGMPSDEBUG(KERN_INFO "IGMP snoop: Searching for IP(%u.%u.%u.%u) ...\n", NIPQUAD(igmp_addr));

			do {
			    ip_node_entry = list_entry(ip_l, struct ip_node, ip_list_node);	
	
			    if (ip_node_entry->ip_addr != igmp_addr) {
				/* Another IP address was found, continue */	
				ip_l = ip_l->next;
			    }
			    else {
				/* Current IP address is already in the list */
				IGMPSDEBUG(KERN_INFO "IGMP snoop: IP address was found in the list\n");
				ip_found = 1;
	
				/* Check whether the port bit is already set in the IP node bitmap */
				if ((ip_node_entry->ip_port_bitmap & temp_port_bitmap) == 0) {
				    IGMPSDEBUG(KERN_INFO "IGMP snoop: Port %d is not yet set in IP entry port bitmap\n", port);

				    /* Bit is not set, add a new port to ip entry*/
				    ip_node_entry->ip_port_bitmap |= temp_port_bitmap;
	
				    /* Check whether there is a need to update the MAC entry ? */
				    if ((mac_node_entry->port_bitmap & temp_port_bitmap) == 0) {
					/* Bit is not set yet and therefore the switch was not instructed */	
					mac_node_entry->port_bitmap |= temp_port_bitmap;
	
					/* Instruct the switch to add the MAC address + port */
					IGMPSDEBUG(KERN_INFO "IGMP snoop: Add port (%d) and MAC address (01:00:5e:%d:%d:%d) to switch DB (dbnum=%d)\n", 
					   port, mac_node_entry->eth_addr[3], mac_node_entry->eth_addr[4], mac_node_entry->eth_addr[5],vlan_dbnum);

					if (mv_gtw_set_mac_addr_to_switch(temp_mac_addr, vlan_dbnum, mac_node_entry->port_bitmap, 1) != 0)
					    printk(KERN_ERR "IGMP snoop: mv_gtw_set_mac_addr_to_switch() failed \n");
				    }
				}
			    }
			} while((ip_l != &mac_node_entry->ip_list) && (ip_found==0));
		
			/* Check whether the IP node was found */
			if (ip_found == 0) {
			    /* Add a new ip node */
			    _ip_node *new_ip_node;
						
			    IGMPSDEBUG(KERN_INFO "IGMP snoop: Add a new IP node to the list (ip=%u.%u.%u.%u port=%d)\n",NIPQUAD(igmp_addr),port);

			    new_ip_node = kmalloc(sizeof(_ip_node), GFP_KERNEL);
			    if (new_ip_node == NULL) {
			    	printk(KERN_ERR "IGMP snoop: out-of-memory\n");
				goto out;
			    }
	
			    /* Fill the IP node with data */
			    INIT_LIST_HEAD(&new_ip_node->ip_list_node);
			    new_ip_node->ip_addr = igmp_addr;
			    new_ip_node->ip_port_bitmap = temp_port_bitmap;
	
			    list_add_tail(&new_ip_node->ip_list_node, &mac_node_entry->ip_list);
	
			    /* if need to update the mac entry ? */
			    if ((mac_node_entry->port_bitmap & temp_port_bitmap) == 0) {
				mac_node_entry->port_bitmap |= temp_port_bitmap;
				/* Instruct the switch to add the MAC address + port */
				IGMPSDEBUG(KERN_INFO "IGMP snoop: Add port (%d) and MAC address (01:00:5e:%d:%d:%d) to switch DB (dbnum=%d)\n", 
				   port,mac_node_entry->eth_addr[3], mac_node_entry->eth_addr[4], mac_node_entry->eth_addr[5],vlan_dbnum);

				if (mv_gtw_set_mac_addr_to_switch(temp_mac_addr,vlan_dbnum,mac_node_entry->port_bitmap, 1) != 0)
					printk(KERN_ERR "IGMP snoop: mv_gtw_set_mac_addr_to_switch() failed \n");
			    }
			}    /* if (ip_found == 0)  */
		    }  /* if (memcmp(temp_mac_addr,mac_node_entry->eth_addr,ETH_ALEN) == 0)  */

		    else {
			/* Another MAC address is found in the MAC node linked list */
			l = l->next;
		    }
		} while ((l != &ipm_llist) && (mac_found == 0));
	
		/* If the MAC entry was not found, than add a mac entry to the DB */
		if (mac_found == 0) {
		    if (_igmp_add_mac_node(temp_mac_addr, vlan_dbnum, igmp_addr, port) != 0) {
			printk(KERN_ERR "Failed to add MAC node \n");
			goto out;
		    }
		}
	    }
	    break;

	case IGMP_HOST_LEAVE_MESSAGE:
	    IGMPSDEBUG(KERN_INFO "\n\nIGMP snoop: IGMPv2 LEAVE (if=\"%s\", group=%u.%u.%u.%u, port=%d, vlanDB=%d)\n",
	    skb->dev->name, NIPQUAD(igmp_addr), port, vlan_dbnum);

	    /* Nothing to be done if the MAC node list is empty*/
	    if (list_empty(&ipm_llist))
		break;
	
	    /* Search for the MAC address in the list */
	    IGMPSDEBUG(KERN_INFO "IGMP snoop: Searching for MAC(01:00:5e:%x:%x:%x) ...\n", 
	    temp_mac_addr[3], temp_mac_addr[4], temp_mac_addr[5]); 
	    l = ipm_llist.next;
	    mac_found = 0;

	    do {
		mac_node_entry = list_entry(l, struct mac_node, list_node);

		if (memcmp(temp_mac_addr, mac_node_entry->eth_addr, ETH_ALEN) == 0) {

		    /* MAC address match */
		    IGMPSDEBUG(KERN_INFO "IGMP snoop: MAC address was found in the list\n");
		    mac_found = 1;
		
		    /* Now look for the IP entry */
		    ip_l = mac_node_entry->ip_list.next;
		    ip_found = 0;
		    ip_port_found = 0;

		    /* First of all, check whether the common bitamp has this bit set */
		    if ((mac_node_entry->port_bitmap & temp_port_bitmap) != 0) {

			IGMPSDEBUG(KERN_INFO "IGMP snoop: Searching for IP(%u.%u.%u.%u) ...\n", NIPQUAD(igmp_addr));
								
			do {
			    ip_node_entry = list_entry(ip_l, struct ip_node, ip_list_node);
						
			    if (ip_node_entry->ip_addr != igmp_addr) {
				/* Another IP multicast address but need to check if the port to be  */
				/* removed is set. */ 
				IGMPSDEBUG(KERN_INFO "IGMP snoop: Port %d is used by IP(%u.%u.%u.%u) ...\n", 
				    port, NIPQUAD(ip_node_entry->ip_addr)); 

				/* check if the port to be removed is present in the IP entry bitmap */	
				if((ip_node_entry->ip_port_bitmap & temp_port_bitmap) != 0) {
				    ip_port_found = 1;
				}
			    }

			    else {
				/* The multicast IP address to remove was found */
				IGMPSDEBUG(KERN_INFO "IGMP snoop: IP address was found in the list\n");
				ip_found = 1;
		
				/* Check whether the port is set in the IP entry bitmap */
				if ((ip_node_entry->ip_port_bitmap & temp_port_bitmap) != 0) {
				    /* Remove the port from the IP entry's bitmap */
				    ip_node_entry->ip_port_bitmap &= ~temp_port_bitmap;
			
				    /* If there are no more bits set to 1, delete the IP entry node */
				    if (ip_node_entry->ip_port_bitmap == 0) {
					IGMPSDEBUG(KERN_INFO "IGMP snoop: Last port on IP node, delete the IP entry\n");
					ip_l = ip_l->next;
			
					list_del(&ip_node_entry->ip_list_node);
			
					/* MUST FREE MEMORY */
					kfree(ip_node_entry);
					continue;
				    }
				}
			    }
		
			    ip_l = ip_l->next;
		
			} while (ip_l != &mac_node_entry->ip_list); /* walkthrough of IP list */
		
		    }

		    /* Check whether we need to update the MAC entry */
		    if ((ip_found == 1) && (ip_port_found == 0)) {
			/* If the port to remove is not associated with any IP address, */
			/* instruct the switch component to remove it */
			if ((mac_node_entry->port_bitmap & temp_port_bitmap) != 0) {
			    mac_node_entry->port_bitmap ^= temp_port_bitmap;
		
			    /* Send the command to the switch */
			    IGMPSDEBUG(KERN_INFO "IGMP snoop: Send port bitmap (0x%x) and MAC address (01:00:5e:%d:%d:%d) to switch DB (dbnum=%d)\n", 
			    mac_node_entry->port_bitmap,mac_node_entry->eth_addr[3], mac_node_entry->eth_addr[4], mac_node_entry->eth_addr[5],vlan_dbnum);

			    /* Is it the last port of the MAC entry ? */ 
			    if (mac_node_entry->port_bitmap == 0) {
				IGMPSDEBUG(KERN_INFO "IGMP snoop: Last port on MAC node, delete the MAC entry\n");

				if (mv_gtw_set_mac_addr_to_switch(temp_mac_addr, vlan_dbnum, mac_node_entry->port_bitmap, 0) != 0)
				    printk(KERN_ERR "IGMP snoop: mv_gtw_set_mac_addr_to_switch() failed \n");

				list_del(&mac_node_entry->list_node);
				/* MUST FREE MEMORY */
				kfree(mac_node_entry);
			    }
			    else {
				if (mv_gtw_set_mac_addr_to_switch(temp_mac_addr, vlan_dbnum, mac_node_entry->port_bitmap, 1) != 0)
				    printk(KERN_ERR "IGMP snoop: mv_gtw_set_mac_addr_to_switch() failed \n");
			    }
		    	}
		    }
		} /* handling of the found MAC address match */
		
		else {
		    l = l->next;
		}
	    } while ((l != &ipm_llist) && (mac_found==0));
	
	    break;
	
	default:
	    break;	
    }
	
out:
    return 0; 
}



/* Initialization function */
int mv_gtw_igmp_snoop_init(void)
{
    /* Enable the HW-based IGMP snooping */
    mv_gtw_enable_igmp();
    return 0;
}

/* Exit function */
int mv_gtw_igmp_snoop_exit(void)
{
    struct list_head *l, *ip_l;
    _mac_node *node_entry;
    _ip_node   *ip_node_entry;

    /* should free all kmalloc allocated memory from llists */
    if (list_empty(&ipm_llist))
        goto out;

    l = ipm_llist.next;
    do
    {
        node_entry = list_entry(l, struct mac_node, list_node);
        IGMPSDEBUG(KERN_INFO "IGMP snoop: Free MAC node (mac=01:00:5e:%x:%x:%x)\n", 
               node_entry->eth_addr[3], node_entry->eth_addr[4], node_entry->eth_addr[5]); 

	/* now look for ip entry */
        ip_l = node_entry->ip_list.next;
        
	do
        {
            ip_node_entry = list_entry(ip_l, struct ip_node, ip_list_node);
            IGMPSDEBUG(KERN_INFO "IGMP snoop: 	Free IP node (ip=%d.%d.%d.%d)\n", 
					   NIPQUAD(ip_node_entry->ip_addr));
                         
            ip_l = ip_l->next;
            list_del(&ip_node_entry->ip_list_node);

             /* MUST FREE MEMORY */
            kfree(ip_node_entry);
	    
        }while(ip_l != &node_entry->ip_list); /* walkthrough of IP list */

	l = l->next;
        
	list_del(&node_entry->list_node);
        kfree(node_entry);

    }while(l != &ipm_llist);

out:
    IGMPSDEBUG(KERN_INFO "IGMP snoop: Exit completed\n");
    
    return 0;
}

/* Add a new MAC node to the list of multicast MAC address structures */
static int _igmp_add_mac_node(	unsigned char	temp_mac_addr[ETH_ALEN],
				unsigned char	vlan_dbnum,
				__u32		igmp_addr,
				__u32		port)
{
	/* should use label , or shorten the code in any other way, I know!!! */
	_mac_node *new_mac_node;	
	_ip_node  *new_ip_node;

	IGMPSDEBUG(KERN_INFO "IGMP snoop: Add a new MAC node (mac=01:00:5e:%x:%x:%x, ip=%d.%d.%d.%d, port=%d)\n", 
				temp_mac_addr[3], temp_mac_addr[4], temp_mac_addr[5], NIPQUAD(igmp_addr), port);

	/* Allocate the memory chunk for the MAC node */
	new_mac_node = kmalloc(sizeof(_mac_node), GFP_KERNEL);
	if (new_mac_node == NULL) 
	{
		printk(KERN_ERR "out-of-memory\n");
		return -ENOMEM;
	}

	/* Initialize the list node */
	INIT_LIST_HEAD(&new_mac_node->list_node);

	/* Copy the MAC address and port bitmap */
	memcpy(new_mac_node->eth_addr, temp_mac_addr, ETH_ALEN);
	new_mac_node->port_bitmap = (1 << port);

	/* Init the head of the IP address list */
	INIT_LIST_HEAD(&new_mac_node->ip_list);

	/* Create the first element of the IP address list */
	new_ip_node = kmalloc(sizeof(_ip_node), GFP_KERNEL);
	if (new_ip_node == NULL) 
	{
		printk(KERN_ERR "out-of-memory\n");
		kfree(new_mac_node);
		return -ENOMEM;
	}

	/* Fill the IP address element with data */
	INIT_LIST_HEAD(&new_ip_node->ip_list_node);	
	new_ip_node->ip_addr = igmp_addr;
	new_ip_node->ip_port_bitmap = (1 << port);

	/* Add the IP address element to the list */
	list_add_tail(&new_ip_node->ip_list_node, &new_mac_node->ip_list);

	/* Add the MAC node element to the list */
	list_add_tail(&new_mac_node->list_node, &ipm_llist);

	/* Instruct the network driver to add the corresponding entry to the switch table */
	mv_gtw_set_mac_addr_to_switch(temp_mac_addr, vlan_dbnum, new_mac_node->port_bitmap, 1);

	return 0;
}
