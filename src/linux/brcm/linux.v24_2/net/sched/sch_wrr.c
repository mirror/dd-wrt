/*-----------------------------------------------------------------------------
Weighted Round Robin scheduler.
  
Written by Christian Worm Mortensen, cworm@it-c.dk.

Introduction
============
This module implements a weighted round robin queue with build-in classifier.
The classifier currently map each MAC or IP address (configurable either MAC
or IP and either source or destination) to different classes. Each such class 
is called a band. Whan using MAC addresses only bridged packets can be 
classified other packets go to a default MAC address.

Each band has a weight value, where 0<weight<=1. The bandwidth each band
get is proportional to the weight as can be deduced from the next section.


The queue
=========
Each band has a penalty value. Bands having something to sent are kept in
a heap according to this value. The band with the lowest penalty value
is in the root of the heap. The penalty value is a 128 bit number. Initially 
no bands are in the heap.

Two global 64 bit values counter_low_penal and couter_high_penal are initialized
to 0 and to 2^63 respectively.

Enqueing:
  The packet is inserted in the queue for the band it belongs to. If the band 
  is not in the heap it is inserted into it. In this case, the upper 64 bits 
  of its penalty value is set to the same as for the root-band of the heap. 
  If the heap is empty 0 is used. The lower 64 bit is set to couter_low_penal
  and couter_low_penal is incremented by 1.
  
Dequing:
  If the heap is empty we have nothing to send. 
  
  If the root band has a non-empty queue a packet is dequeued from that.
  The upper 64 bit of the penalty value of the band is incremented by the 
  packet size divided with the weight of the band. The lower 64 bit is set to 
  couter_high_penal and couter_high_penal is incremented by 1.

  If the root element for some reason has an  empty queue it is removed from 
  the heap and we try to dequeue again.

The effect of the heap and the upper 64 bit of the penalty values is to 
implement a weighted round robin queue. The effect of counter_low_penal,
counter_high_penal and the lower 64 bit of the penalty value is primarily to
stabilize the queue and to give better quality of service to machines only 
sending a packet now and then. For example machines which have a single 
interactive connection such as telnet or simple text chatting.


Setting weight
==============
The weight value can be changed dynamically by the queue itself. The weight 
value and how it is changed is described by the two members weight1 and 
weight2 which has type tc_wrr_class_weight and which are in each class. And 
by the two integer value members of the qdisc called penalfact1 and penalfact2.
The structure is defined as:

  struct tc_wrr_class_weight {
    // All are represented as parts of (2^64-1).
    __u64 val;  // Current value                        (0 is not valid)
    __u64 decr; // Value pr bytes                       (2^64-1 is not valid)
    __u64 incr; // Value pr seconds                     (2^64-1 is not valid)
    __u64 min;  // Minimal value                        (0 is not valid)
    __u64 max;  // Minimal value                        (0 is not valid)

    // The time where the above information was correct:
    time_t tim;
  };
    
The weight value used by the dequeue operations is calculated as 
weight1.val*weight2.val. weight1 and weight2 and handled independently and in the 
same way as will be described now.

Every second, the val parameter is incremented by incr.

Every time a packet is transmitted the value is increment by decr times
the packet size. Depending on the value of the weight_mode parameter it
is also mulitplied with other numbers. This makes it possible to give 
penalty to machines transferring much data.

-----------------------------------------------------------------------------*/

#include <linux/config.h>
#include <linux/module.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/bitops.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/socket.h>
#include <linux/sockios.h>
#include <linux/in.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/if_ether.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/notifier.h>
#include <net/ip.h>
#include <net/route.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/pkt_sched.h>

#include <linux/if_arp.h>
#include <linux/version.h>

// There seems to be problems when calling functions from userspace when
// using vmalloc and vfree.
//#define my_malloc(size) vmalloc(size)
//#define my_free(ptr)   vfree(ptr)
#define my_malloc(size) kmalloc(size,GFP_KERNEL)
#define my_free(ptr)    kfree(ptr)

// Kernel depend stuff:
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
  #define KERNEL22
#endif

#ifdef KERNEL22  
  #define LOCK_START start_bh_atomic();
  #define LOCK_END   end_bh_atomic();
  #define ENQUEUE_SUCCESS 1
  #define ENQUEUE_FAIL    0
  #ifdef CONFIG_IP_MASQUERADE      
    #include <net/ip_masq.h>
    #define MASQ_SUPPORT
  #endif
#else
  #define LOCK_START sch_tree_lock(sch);
  #define LOCK_END   sch_tree_unlock(sch);
  #define ENQUEUE_SUCCESS 0
  #define ENQUEUE_FAIL    NET_XMIT_DROP
  #ifdef CONFIG_IP_NF_CONNTRACK
    #include <linux/netfilter_ipv4/ip_conntrack.h>
    #define MASQ_SUPPORT
  #endif
#endif  

#include "proxydict.c"

// The penalty (priority) type:
typedef u64 penalty_base_t;
#define penalty_base_t_max ((penalty_base_t)-1)
typedef struct penalty_t {
  penalty_base_t ms;
  penalty_base_t ls;
} penalty_t;
#define penalty_leq(a,b) (a.ms<b.ms || (a.ms==b.ms && a.ls<=b.ls))
#define penalty_le(a,b)  (a.ms<b.ms || (a.ms==b.ms && a.ls<b.ls))
static penalty_t penalty_max={penalty_base_t_max,penalty_base_t_max};

//-----------------------------------------------------------------------------
// A generel heap.

struct heap;
struct heap_element;

// Initializes an empty heap:
//   he:   A pointer to an unintialized heap structure identifying the heap
//   size: Maximal number of elements the heap can contain
//   poll: An array of size "size" used by the heap.     
static void heap_init(struct heap* he,int size, struct heap_element* poll);

// Each element in the heap is identified by a user-assigned id which
// should be a non negative integer less than the size argument
// given to heap_init.
static void heap_insert(struct heap*, int id, penalty_t);
static void heap_remove(struct heap*, int id);
static void heap_set_penalty(struct heap*, int id, penalty_t);

// Retreviewing information:
static char      heap_empty(struct heap*); // Heap empty?
static char      heap_contains(struct heap*, int id); // Does heap contain 
                                                      // the given id?
static int       heap_root(struct heap*);  // Returns the id of the root
static penalty_t heap_get_penalty(struct heap*, int id); // Returns penaly
                                                         // of root node

//--------------------
// Heap implementation

struct heap_element {
  penalty_t penalty;
  int id;             // The user-assigned id of this element
  int id2idx;         // Maps from user-assigned ids to indices in root_1
};

struct heap {
  struct heap_element* root_1;
  int elements;
};

// Heap implementation:
static void heap_init(struct heap* h, int size, struct heap_element* poll) {
  int i;
  
  h->elements=0;
  h->root_1=poll-1;
  
  for(i=0; i<size; i++) poll[i].id2idx=0;
};

static char heap_empty(struct heap* h) {
  return h->elements==0;
}

static char heap_contains(struct heap* h, int id) {
  return h->root_1[id+1].id2idx!=0;
}

static int heap_root(struct heap* h) {
  return h->root_1[1].id;
}

static penalty_t heap_get_penalty(struct heap* h, int id) {
  return h->root_1[ h->root_1[id+1].id2idx ].penalty;
}

static void heap_penalty_changed_internal(struct heap* h,int idx);

static void heap_set_penalty(struct heap* h, int id, penalty_t p) {
  int idx=h->root_1[id+1].id2idx;
  h->root_1[idx].penalty=p;
  heap_penalty_changed_internal(h,idx);
}

static void heap_insert(struct heap* h, int id, penalty_t p) {
  // Insert at the end of the heap:
  h->elements++;
  h->root_1[h->elements].id=id;
  h->root_1[h->elements].penalty=p;
  h->root_1[id+1].id2idx=h->elements;
  
  // And put it in the right position:
  heap_penalty_changed_internal(h,h->elements);
}

static void heap_remove(struct heap* h, int id) {
  int idx=h->root_1[id+1].id2idx;
  int mvid;
  h->root_1[id+1].id2idx=0;
  
  if(h->elements==idx)  { h->elements--; return; }
  
  mvid=h->root_1[h->elements].id;
  h->root_1[idx].id=mvid;
  h->root_1[idx].penalty=h->root_1[h->elements].penalty;
  h->root_1[mvid+1].id2idx=idx;
  
  h->elements--;
  heap_penalty_changed_internal(h,idx);
}  

static void heap_swap(struct heap* h, int idx0, int idx1) {
  penalty_t tmp_p;
  int       tmp_id;
  int       id0,id1;
  
  // Simple content:
  tmp_p=h->root_1[idx0].penalty;
  tmp_id=h->root_1[idx0].id;
  h->root_1[idx0].penalty=h->root_1[idx1].penalty;
  h->root_1[idx0].id=h->root_1[idx1].id;
  h->root_1[idx1].penalty=tmp_p;
  h->root_1[idx1].id=tmp_id;
  
  // Update reverse pointers:
  id0=h->root_1[idx0].id;
  id1=h->root_1[idx1].id;
  h->root_1[id0+1].id2idx=idx0;
  h->root_1[id1+1].id2idx=idx1;
}

static void heap_penalty_changed_internal(struct heap* h,int cur) {
  if(cur==1 || penalty_leq(h->root_1[cur>>1].penalty,h->root_1[cur].penalty)) {
    // We are in heap order upwards - so we should move the element down
    for(;;) {
      int nxt0=cur<<1;
      int nxt1=nxt0+1;
      penalty_t pen_c=h->root_1[cur].penalty;
      penalty_t pen_0=nxt0<=h->elements ? h->root_1[nxt0].penalty : penalty_max;
      penalty_t pen_1=nxt1<=h->elements ? h->root_1[nxt1].penalty : penalty_max;
    
      if(penalty_le(pen_0,pen_c) && penalty_leq(pen_0,pen_1)) {
        // Swap with child 0:
	heap_swap(h,cur,nxt0);
        cur=nxt0;
      } else if(penalty_le(pen_1,pen_c)) {
        // Swap with child 1:
	heap_swap(h,cur,nxt1);
	cur=nxt1;
      } else {
        // Heap in heap order:
        return;
      }
    }
  } else {
    // We are not in heap order upwards (and thus we must be it downwards).
    // We move up:
    while(cur!=1) { // While not root
      int nxt=cur>>1;
      if(penalty_leq(h->root_1[nxt].penalty,h->root_1[cur].penalty)) return;
      heap_swap(h,cur,nxt);
      cur=nxt;
    }
  }
};

//-----------------------------------------------------------------------------
// Classification based on MAC or IP adresses. Note that of historical reason
// these are prefixed with mac_ since originally only MAC bases classification
// was supported.
//
// This code should be in a separate filter module - but it isn't.

// Interface:

struct mac_head;

// Initialices/destroys the structure we maintain.
// Returns -1 on error
static int  mac_init(struct mac_head*, int max_macs, char srcaddr, 
                     char usemac, char usemasq, void* proxyremap);
static void mac_done(struct mac_head*);
static void mac_reset(struct mac_head*);

// Classify a packet. Returns a number n where 0<=n<max_macs. Or -1 if
// the packet should be dropped.
static int mac_classify(struct mac_head*, struct sk_buff *skb);

//-------------
// Implementation:

struct mac_addr {
  unsigned char addr[ETH_ALEN]; // Address of this band (last two are 0 on IP)
  unsigned long lastused;       // Last time a packet was encountered
  int class;                    // Classid of this band (0<=classid<max_macs)
};

static int mac_compare(const void* a, const void* b) {
  return memcmp(a,b,ETH_ALEN);
}

struct mac_head {
  int mac_max;    // Maximal number of MAC addresses/classes allowed
  int mac_cur;    // Current number of MAC addresses/classes
  int mac_reused; // Number of times we have reused a class with a new 
                  // address.
  u64 incr_time;
  char srcaddr;   // True if we classify on the source address of packets,
                  // else we use destination address.
  char usemac;    // If true we use mac, else we use IP
  char usemasq;   // If true we try to demasqgrade
  struct mac_addr* macs; // Allocated mac_max elements, used max_cur
  char* cls2mac;         // Mapping from classnumbers to addresses -
                         // there is 6 bytes in each entry
                          
  void* proxyremap; // Information on proxy remapping of data or 0
};

// This is as the standard C library function with the same name:
static const void* bsearch(const void* key, const void* base, int nmemb, 
                           size_t size, 
			   int (*compare)(const void*, const void*)) {	
  int m_idx;
  const void* m_ptr;
  int i;
  
  if(nmemb<=0) return 0;
  
  m_idx=nmemb>>1;
  m_ptr=((const char*)base)+m_idx*size;
  
  i=compare(key,m_ptr);
  if(i<0) // key is less
    return bsearch(key,base,m_idx,size,compare);
  else if(i>0)
    return bsearch(key,((const char*)m_ptr)+size,nmemb-m_idx-1,size,compare);
    
  return m_ptr;
}

static int mac_init(struct mac_head* h, int max_macs, char srcaddr, 
                    char usemac, char usemasq,void* proxyremap) {
  h->mac_cur=0;
  h->mac_reused=0;
  h->incr_time=0;
  h->srcaddr=srcaddr;
  h->usemac=usemac;
  h->usemasq=usemasq;
  h->mac_max=max_macs;
  h->proxyremap=proxyremap;

  h->macs=(struct mac_addr*)
    my_malloc( sizeof(struct mac_addr)*max_macs);
  h->cls2mac=(char*)my_malloc( 6*max_macs);
  if(!h->macs || !h->cls2mac) {
    if(h->macs) my_free(h->macs);
    if(h->cls2mac) my_free(h->cls2mac);
    return -1;
  }
  return 0;
}

static void mac_done(struct mac_head* h) {
  my_free(h->macs);
  my_free(h->cls2mac);
}

static void mac_reset(struct mac_head* h) {
  h->mac_cur=0;
  h->mac_reused=0;
  h->incr_time=0;
}

static int lookup_mac(struct mac_head* h, unsigned char* addr) {
  int i;
  int class;
  
  // First try to find the address in the table:  
  struct mac_addr* m=(struct mac_addr*)
    bsearch(addr,h->macs,h->mac_cur,sizeof(struct mac_addr),mac_compare);
  if(m) {
    // Found:
    m->lastused=h->incr_time++;
    return m->class;
  }
  
  // Okay - the MAC adress was not in table
  if(h->mac_cur==h->mac_max) {
    // And the table is full - delete the oldest entry:

    // Find the oldest entry:
    int lowidx=0;
    int i;
    for(i=1; i<h->mac_cur; i++) 
      if(h->macs[i].lastused < h->macs[lowidx].lastused) lowidx=i;
    
    class=h->macs[lowidx].class;
    
    // And delete it:
    memmove(&h->macs[lowidx],&h->macs[lowidx+1],
            (h->mac_cur-lowidx-1)*sizeof(struct mac_addr));
    h->mac_reused++;
    h->mac_cur--;
  } else {
    class=h->mac_cur;
  }
  
  // The table is now not full - find the position we should put the address in:
  for(i=0; i<h->mac_cur; i++) if(mac_compare(addr,&h->macs[i])<0) break;
  
  // We should insert at position i:
  memmove(&h->macs[i+1],&h->macs[i],(h->mac_cur-i)*sizeof(struct mac_addr));
  m=&h->macs[i];
  memcpy(m->addr,addr,ETH_ALEN);
  m->lastused=h->incr_time++;
  m->class=class;
  h->mac_cur++;
  
  // Finally update the cls2mac variabel:
  memcpy(h->cls2mac+ETH_ALEN*class,addr,ETH_ALEN);
  
  return m->class;
}

int valid_ip_checksum(struct iphdr* ip, int size) {
  __u16 header_len=ip->ihl<<2;
  __u16 c=0;
  __u16* ipu=(u16*)ip;
  int a;
  
  // We require 4 bytes in the packet since we access the port numbers:  
  if((size<header_len) || size<sizeof(struct iphdr)+4) return 0;
  
  for(a=0; a<(header_len>>1); a++, ipu++) {
    if(a!=5) { // If not the checksum field
      __u16 oldc=c;	
      c+=(*ipu); 
      if(c<oldc) c++;	  
    }
  }
  
  return ip->check==(__u16)~c;
}   
 
static int mac_classify(struct mac_head* head, struct sk_buff *skb)
{
  // We set this to the address we map to. In case we map to an IP
  // address the last two entries are set to 0.
  unsigned char addr[ETH_ALEN];
  
  
  // This is the size of the network part of the packet, I think:
  int size=((char*)skb->data+skb->len)-((char*)skb->nh.iph);

  // Set a default value for the address:
  memset(addr,0,ETH_ALEN);
  
  // Accept IP-ARP traffic with big-enough packets:
  if(ntohs(skb->protocol)==ETH_P_ARP && 
           ntohs(skb->nh.arph->ar_pro)==ETH_P_IP) {
    // Map all ARP trafic to a default adress to make sure
    // it goes through
  } else if ((ntohs(skb->protocol)==ETH_P_IP) && 
          valid_ip_checksum(skb->nh.iph,size)) {
    // Accept IP packets which have correct checksum.
	     
    // This is the IP header:
    struct iphdr* iph=skb->nh.iph;
    
    // And this is the port numbers:
    const __u16 *portp = (__u16 *)&(((char *)iph)[iph->ihl*4]);
    __u16 sport=portp[0];
    __u16 dport=portp[1];
    
    // We will set this to the IP address of the packet that should be
    // accounted to:
    unsigned ipaddr;
    
    // Used below:    
    ProxyRemapBlock* prm;
    
    // Set ipaddr:
    if(head->srcaddr) 
      ipaddr=iph->saddr;
    else
      ipaddr=iph->daddr;
      
#ifdef MASQ_SUPPORT
    // Update ipaddr if packet is masqgraded:
    if(head->usemasq) {
      #ifdef KERNEL22
        struct ip_masq* src;

        // HACK!:
        //   ip_masq_in_get must be called for packets comming from the outside
        //   to the firewall. We have a a packet which is comming from the 
	//   firewall to the outside - so we switch the parameters:
        if((src=ip_masq_in_get(
  	       iph->protocol,
	       iph->daddr,dport,
	       iph->saddr,sport))) {
          // Use masqgraded address:
  	  ipaddr=src->saddr;
	
	  // It seems like we must put it back:
	  ip_masq_put(src);
        }
      #else
        // Thanks to Rusty Russell for help with the following code:
        enum ip_conntrack_info ctinfo;
        struct ip_conntrack *ct;
        ct = ip_conntrack_get(skb, &ctinfo);
        if (ct) {
          if(head->srcaddr)
            ipaddr=ct->tuplehash[CTINFO2DIR(ctinfo)].tuple.src.ip;
  	  else
            ipaddr=ct->tuplehash[CTINFO2DIR(ctinfo)].tuple.dst.ip;
        }
      #endif
    }
#endif    

    // Set prm based on ipaddr:
    prm=0;
    if(head->proxyremap) {
      if(head->srcaddr) {
        prm=proxyLookup(head->proxyremap,ipaddr,sport,skb->nh.iph->protocol);
      } else {
        prm=proxyLookup(head->proxyremap,ipaddr,dport,skb->nh.iph->protocol);
      }
    }
    
    // And finally set addr to the address:
    memset(addr,0,ETH_ALEN);
    if(prm) {
      // This package should be remapped:
      if(head->usemac) 
        memcpy(addr,prm->macaddr,ETH_ALEN);
      else {
        memcpy(addr,&prm->caddr,sizeof(unsigned));
      }
    } else {
      // This packet should not be remapped:
      if(head->usemac) {	
        // We should find MAC address of packet.
	// Unfortunatly, this is not always available.
	// On bridged packets it always is, however..
	#ifdef KERNEL22
          if(skb->pkt_bridged) {
            if(head->srcaddr) {
              memcpy(addr,skb->mac.ethernet->h_source,ETH_ALEN);
            } else {
              memcpy(addr,skb->mac.ethernet->h_dest,ETH_ALEN);
  	    }
	  }
	#endif	
      } else {
        memcpy(addr,&ipaddr,4);              
      } 
    }
  } else {
    // All other traffic is dropped - this ensures that packets
    // we consider probably have valid addresses so we don't
    // get to many strange addresses into our table. And that we
    // don't use bandwidth on strange packets..
    return -1;
  }
  
  return lookup_mac(head,addr);
}

//-----------------------------------------------------------------------------
// The qdisc itself

// Pr-class information.
struct wrrc_sched_data {
  struct Qdisc* que;                   // The queue for this class
  struct tc_wrr_class_modf class_modf; // Information about the class.
  
  // For classes in the heap this is the priority value priosum
  // was updated with for this class:
  u64 priosum_val;
};  

// Pr-qdisc information:
struct wrr_sched_data
{
  // A heap containing all the bands that will send something
  struct heap h;
  struct heap_element* poll; // bandc elements
  
  // The sum of the prioities of the elements in the heap where
  // a priority of 1 is saved as 2^32
  u64 priosum;
  
  // A class for each band
  struct wrrc_sched_data* bands; // bandc elements
  
  // Information maintained by the proxydict module of 0 if we
  // have no proxy remapping
  void* proxydict;
  
  // Always incrementning counters, we always have that any value of
  // counter_low_penal < any value of counter_high_penal.
  penalty_base_t counter_low_penal;
  penalty_base_t counter_high_penal;
  
  // Penalty updating:
  struct tc_wrr_qdisc_modf qdisc_modf;
  
  // Statistics:
  int packets_requed;
  
  // The filter:
  struct mac_head filter;  
  int bandc; // Number of bands
};

// Priority handling.
//   weight is in interval [0..2^32]
//   priosum has whole numbers in the upper and fragments in the lower 32 bits. 
static void weight_transmit(struct tc_wrr_class_weight* p, 
                            struct tc_wrr_qdisc_weight q,
			    unsigned heapsize,
			    u64 priosum, u64 weight,
			    unsigned size) {

  unsigned long now=jiffies/HZ;
  
  // Penalty for transmitting:
  u64 change,old;
  u32 divisor;
  
  change=0;
  switch(q.weight_mode) {
    case 1: change=p->decr*size; break;
    case 2: change=p->decr*size*heapsize; break;
    case 3: // Note: 64 bit division is not always available..
      divisor=(u32)(weight>>16);
      if(divisor<=0) divisor=1;
      change=p->decr*size*(((u32)(priosum>>16))/divisor); break;
  }
  old=p->val;
  p->val-=change;
  if(p->val>old || p->val<p->min) p->val=p->min;
  
  // Credit for time went:
  change=(now-p->tim)*p->incr;
  p->tim=now;
  old=p->val;
  p->val+=change;
  if(p->val<old || p->val>p->max) p->val=p->max;
}  

static void weight_setdefault(struct tc_wrr_class_weight* p) {
  p->val=(u64)-1;
  p->decr=0;
  p->incr=0;
  p->min=(u64)-1;
  p->max=(u64)-1;
  p->tim=jiffies/HZ;  
}

static void weight_setvalue(struct tc_wrr_class_weight* dst, 
                            struct tc_wrr_class_weight* src) {
  if(src->val!=0) {
    dst->val=src->val;
    dst->tim=jiffies/HZ;
  }
  if(src->min!=0) dst->min=src->min;
  if(src->max!=0) dst->max=src->max;
  if(src->decr!=((u64)-1)) dst->decr=src->decr;
  if(src->incr!=((u64)-1)) dst->incr=src->incr;
  if(dst->val<dst->min) dst->val=dst->min;
  if(dst->val>dst->max) dst->val=dst->max;
}

static void wrr_destroy(struct Qdisc *sch)
{
  struct wrr_sched_data *q=(struct wrr_sched_data *)sch->data;
  int i;
  
  // Destroy our filter:
  mac_done(&q->filter);

  // Destroy all our childre ques:
  for(i=0; i<q->bandc; i++) 
    qdisc_destroy(q->bands[i].que);
    
  // And free memory:
  my_free(q->bands);
  my_free(q->poll);  
  if(q->proxydict) my_free(q->proxydict);
  
  MOD_DEC_USE_COUNT;
}

static int wrr_init(struct Qdisc *sch, struct rtattr *opt)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  int i,maciniterr;
  char crterr;
  struct tc_wrr_qdisc_crt *qopt;
  
  // Parse options:
  if (!opt) return -EINVAL; // Options must be specified
  if (opt->rta_len < RTA_LENGTH(sizeof(*qopt))) return -EINVAL;
  qopt = RTA_DATA(opt);

  if(qopt->bands_max>8192 || qopt->bands_max<2) {
    // More than 8192 queues or less than 2? That cannot be true - it must be 
    // an error...
    return -EINVAL;
  }
  
  if(qopt->proxy_maxconn<0 || qopt->proxy_maxconn>20000) {
    // More than this number of maximal concurrent connections is unrealistic
    return -EINVAL;
  }

#ifndef MASQ_SUPPORT
  if(qopt->usemasq) { 
    return -ENOSYS;
  }
#endif  

#ifndef KERNEL22
  if(qopt->usemac) { // Not supported - please fix this!
    return -ENOSYS;
  }
#endif      

  q->bandc=qopt->bands_max;
  q->qdisc_modf=qopt->qdisc_modf;
  
  // Create structures:
  q->poll=(struct heap_element*)
           my_malloc( sizeof(struct heap_element)*q->bandc);
  q->bands=(struct wrrc_sched_data*)
           my_malloc( sizeof(struct wrrc_sched_data)*q->bandc);
  
  if(qopt->proxy_maxconn>0) {
    q->proxydict=my_malloc(proxyGetMemSize(qopt->proxy_maxconn));
  } else {
    q->proxydict=0;
  }
  
  // Init mac module:
  maciniterr=mac_init(&q->filter,qopt->bands_max,qopt->srcaddr,
                      qopt->usemac,qopt->usemasq,q->proxydict);

  // See if we got the memory we wanted:
  if(!q->poll || !q->bands || 
    (qopt->proxy_maxconn>0 && !q->proxydict) || maciniterr<0) {
    if(q->poll) my_free(q->poll);
    if(q->bands) my_free(q->bands);
    if(q->proxydict) my_free(q->proxydict);
    if(maciniterr>=0) mac_done(&q->filter);
    return -ENOMEM;
  }
  
  // Initialize proxy:
  if(q->proxydict) {
    proxyInitMem(q->proxydict,qopt->proxy_maxconn);
  }
    
  // Initialize values:    
  q->counter_low_penal=0;
  q->counter_high_penal=penalty_base_t_max>>1;
  q->packets_requed=0;
  
  // Initialize empty heap:
  heap_init(&q->h,q->bandc,q->poll);
  q->priosum=0;
  
  // Initialize each band:
  crterr=0;
  for (i=0; i<q->bandc; i++) {
    weight_setdefault(&q->bands[i].class_modf.weight1);
    weight_setdefault(&q->bands[i].class_modf.weight2);
    if(!crterr) {
      struct Qdisc *child=qdisc_create_dflt(sch->dev, &pfifo_qdisc_ops);
      if(child)
        q->bands[i].que = child;
      else {
        // Queue couldn't be created :-(
        crterr=1;
      }
    }
    if(crterr) q->bands[i].que = &noop_qdisc;
  }
      
  MOD_INC_USE_COUNT;
  
  if(crterr) {
    // Destroy again:
    wrr_destroy(sch);
    return -ENOMEM;
  }
  
  return 0;
}

static void wrr_reset(struct Qdisc* sch)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  int i;
  
  // Reset own values:
  q->counter_low_penal=0;
  q->counter_high_penal=penalty_base_t_max>>1;
  q->packets_requed=0;
  
  // Reset filter:
  mac_reset(&q->filter);
  
  // Reinitialize heap:
  heap_init(&q->h,q->bandc,q->poll);
  q->priosum=0;
  
  // Reset all bands:
  for (i=0; i<q->bandc; i++) {
    weight_setdefault(&q->bands[i].class_modf.weight1);
    weight_setdefault(&q->bands[i].class_modf.weight2);
    qdisc_reset(q->bands[i].que);
  }
  
  // Reset proxy remapping information:
  if(q->proxydict)
    proxyInitMem(q->proxydict,proxyGetMaxConn(q->proxydict));
}

static int wrr_enqueue(struct sk_buff *skb, struct Qdisc* sch)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  int retvalue=ENQUEUE_FAIL;
  
  // The packet is in skb.
  int band=mac_classify(&q->filter,skb);

  if(band>=0) {
    // Enque packet for this band:
    struct Qdisc* qdisc = q->bands[band].que;

    if ((retvalue=qdisc->enqueue(skb, qdisc)) == ENQUEUE_SUCCESS) {
      // Successfull
      sch->stats.bytes += skb->len;
      sch->stats.packets++;
      sch->q.qlen++;
        
      // Insert band into heap if not already there:
      if(!heap_contains(&q->h,band)) {        
        penalty_t p;
	if(!heap_empty(&q->h)) 
  	  p.ms=heap_get_penalty(&q->h,heap_root(&q->h)).ms;
	else
	  p.ms=0;
	p.ls=q->counter_low_penal++;
        heap_insert(&q->h,band,p);
	q->bands[band].priosum_val=
	  ((q->bands[band].class_modf.weight1.val>>48)+1)*
          ((q->bands[band].class_modf.weight2.val>>48)+1);
	q->priosum+=q->bands[band].priosum_val;
      }
    }
  } else {
    // If we decide not to enque it seems like we also need to free the packet:
    kfree_skb(skb);
  }
  
  if(retvalue!=ENQUEUE_SUCCESS) {
    // Packet not enqued:
    sch->stats.drops++;
  }
  
  return retvalue;
}

static struct sk_buff *wrr_dequeue(struct Qdisc* sch)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  struct sk_buff* skb;
  int band;
  u64 weight,priosum;
  struct wrrc_sched_data* b;
  
  // Return if heap is empty:
  if(heap_empty(&q->h)) return 0;
  
  // Find root element:
  band=heap_root(&q->h);
  
  // Find priority of this element in interval [1;2^32]
  b=&q->bands[band];
  weight=((b->class_modf.weight1.val>>48)+1)*
         ((b->class_modf.weight2.val>>48)+1); //weight is in interval [1;2^32]
  priosum=q->priosum;
  q->priosum-=q->bands[band].priosum_val;
  
  // Deque the packet from the root:
  skb=q->bands[band].que->dequeue(q->bands[band].que);  
  
  if(skb) {
    // There was a packet in this que.
    unsigned adjlen;
    penalty_t p;
    
    // Find length of packet adjusted with priority:
    adjlen=(u32)(weight>>(32-16));
    if(adjlen==0) adjlen=1;
    adjlen=(skb->len<<16)/adjlen;
	   
    // Update penalty information for this class:
    weight_transmit(&b->class_modf.weight1,q->qdisc_modf.weight1,q->h.elements,priosum,weight,skb->len);
    weight_transmit(&b->class_modf.weight2,q->qdisc_modf.weight2,q->h.elements,priosum,weight,skb->len);
    q->bands[band].priosum_val=((b->class_modf.weight1.val>>48)+1)*
                               ((b->class_modf.weight2.val>>48)+1);    
    q->priosum+=q->bands[band].priosum_val;	       
        
    // And update the class in the heap
    p=heap_get_penalty(&q->h,band);    
    p.ms+=adjlen;  
    p.ls=q->counter_high_penal++; 
    heap_set_penalty(&q->h,band,p);
    
    // Return packet:
    sch->q.qlen--;
    return skb;
  }
  
  // No packet - so machine should be removed from heap:
  heap_remove(&q->h,band);
  
  return 0;
}

static int wrr_requeue(struct sk_buff *skb, struct Qdisc* sch)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  struct Qdisc* qdisc;
  int ret;
  
  // Find band we took it from:
  int band=mac_classify(&q->filter,skb);
  if(band<0) { 
    // Who should now free the pakcet?
    printk(KERN_DEBUG "sch_wrr: Oops - packet requed could never have been queued.\n");
    sch->stats.drops++; 
    return ENQUEUE_FAIL; 
  }

  q->packets_requed++;
  
  // Try to requeue it on that machine:
  qdisc=q->bands[band].que;

  if((ret=qdisc->ops->requeue(skb,qdisc))==ENQUEUE_SUCCESS) {
    // On success:
    sch->q.qlen++;
    
    // We should restore priority information - but we don't
    //
    // p=heap_get_penalty(&q->h,band);
    // ...
    // heap_set_penalty(&q->h,band,p);
    
    return ENQUEUE_SUCCESS;
  } else {
    sch->stats.drops++;
    return ret;
  }
}    

static int wrr_drop(struct Qdisc* sch)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;

  // Ugly... Drop button up in heap:
  int i;

  for(i=q->h.elements; i>=1; i--) {
    int band=q->h.root_1[i].id;
    if(q->bands[band].que->ops->drop(q->bands[band].que)) {
      // On success
      sch->q.qlen--;
      return 1;
    }
  }

  return 0;
}

static int wrr_dump(struct Qdisc *sch, struct sk_buff *skb)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  unsigned char	*b = skb->tail;
  struct tc_wrr_qdisc_stats opt;

  opt.qdisc_crt.qdisc_modf=q->qdisc_modf;
  opt.qdisc_crt.srcaddr=q->filter.srcaddr;
  opt.qdisc_crt.usemac=q->filter.usemac;
  opt.qdisc_crt.usemasq=q->filter.usemasq;
  opt.qdisc_crt.bands_max=q->filter.mac_max;  
  opt.nodes_in_heap=q->h.elements;
  opt.bands_cur=q->filter.mac_cur;
  opt.bands_reused=q->filter.mac_reused;
  opt.packets_requed=q->packets_requed;
  opt.priosum=q->priosum;

  if(q->proxydict) {
    opt.qdisc_crt.proxy_maxconn=proxyGetMaxConn(q->proxydict);
    opt.proxy_curconn=proxyGetCurConn(q->proxydict);
  } else {
    opt.qdisc_crt.proxy_maxconn=0;
    opt.proxy_curconn=0;
  }
  
  RTA_PUT(skb, TCA_OPTIONS, sizeof(opt), &opt);
  return skb->len;
  
rtattr_failure: // seems like RTA_PUT jump to this label..
  skb_trim(skb, b - skb->data);
  return -1;  
}

static int wrr_tune_std(struct Qdisc *sch, struct rtattr *opt) 
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  struct tc_wrr_qdisc_modf_std *qopt = RTA_DATA(opt);
  
  if(opt->rta_len < RTA_LENGTH(sizeof(*qopt))) return -EINVAL;  

  LOCK_START
  
  if(qopt->change_class) {
    int idx=lookup_mac(&q->filter,qopt->addr);
    weight_setvalue
      (&q->bands[idx].class_modf.weight1,&qopt->class_modf.weight1);
    weight_setvalue
      (&q->bands[idx].class_modf.weight2,&qopt->class_modf.weight2);
  } else {
    if(qopt->qdisc_modf.weight1.weight_mode!=-1)
       q->qdisc_modf.weight1.weight_mode=qopt->qdisc_modf.weight1.weight_mode;
    if(qopt->qdisc_modf.weight2.weight_mode!=-1)
       q->qdisc_modf.weight2.weight_mode=qopt->qdisc_modf.weight2.weight_mode;
  }
  
  LOCK_END
  
  return 0;
}

static int wrr_tune_proxy(struct Qdisc *sch, struct rtattr *opt) 
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  struct tc_wrr_qdisc_modf_proxy *qopt = RTA_DATA(opt);
  int i;

  // Return if we are not configured with proxy support:
  if(!q->proxydict) return -ENOSYS;

  // Return if not enough data given:
  if(opt->rta_len<RTA_LENGTH(sizeof(*qopt)) ||
     opt->rta_len<
       RTA_LENGTH(sizeof(*qopt)+sizeof(ProxyRemapBlock)*qopt->changec))
       return -EINVAL;

  LOCK_START;
  
  if(qopt->reset) {
    proxyInitMem(q->proxydict,proxyGetMaxConn(q->proxydict));
  }
  
  // Do all the changes:
  for(i=0; i<qopt->changec; i++) {
    proxyConsumeBlock(q->proxydict,&((ProxyRemapBlock*)&qopt->changes)[i]);
  }
  
  LOCK_END;
  
  return 0;
}

static int wrr_tune(struct Qdisc *sch, struct rtattr *opt) {
  if(((struct tc_wrr_qdisc_modf_std*)RTA_DATA(opt))->proxy) {
    return wrr_tune_proxy(sch,opt);
  } else {
    return wrr_tune_std(sch,opt);
  }    
}

//-----------------------------------------------------------------------------
// Classes.
//  External and internal IDs are equal. They are the band number plus 1.

// Replace a class with another:
static int wrr_graft(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
	             struct Qdisc **old)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  if(arg>q->bandc || arg==0)  return -EINVAL;
  arg--;
  
  if (new == NULL)
    new = &noop_qdisc;

#ifdef KERNEL22
  *old = xchg(&q->bands[arg].que, new);
#else
  LOCK_START
  *old = q->bands[arg].que;
  q->bands[arg].que = new;
  qdisc_reset(*old);
  LOCK_END	
#endif

  return 0;
}

// Returns the qdisc for a class:
static struct Qdisc * wrr_leaf(struct Qdisc *sch, unsigned long arg)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  if(arg>q->bandc || arg==0)  return NULL;
  arg--;
  return q->bands[arg].que;
}

static unsigned long wrr_get(struct Qdisc *sch, u32 classid)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  unsigned long band = TC_H_MIN(classid);
  if(band>q->bandc || band==0) return 0;
  return band;
}

static void wrr_put(struct Qdisc *q, unsigned long cl)
{
  return;
}

static int wrr_delete(struct Qdisc *sch, unsigned long cl)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  if(cl==0 || cl>q->bandc) return -ENOENT;
  cl--;
  return 0;
}

static int wrr_dump_class(struct Qdisc *sch, unsigned long cl, 
                          struct sk_buff *skb, struct tcmsg *tcm)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  unsigned char *b = skb->tail;
  struct tc_wrr_class_stats opt;

  // Handle of this class:
  tcm->tcm_handle = sch->handle|cl;
  
  if(cl==0 || cl>q->bandc)
    goto rtattr_failure;
  cl--;  
  
  if(cl>=q->filter.mac_cur) {
    // Band is unused:
    memset(&opt,0,sizeof(opt));
    opt.used=0;
  } else {
    opt.used=1;
    opt.class_modf.weight1=q->bands[cl].class_modf.weight1;
    opt.class_modf.weight2=q->bands[cl].class_modf.weight2;
    weight_transmit(&opt.class_modf.weight1,q->qdisc_modf.weight1,0,0,0,0);
    weight_transmit(&opt.class_modf.weight2,q->qdisc_modf.weight2,0,0,0,0);
    memcpy(opt.addr,q->filter.cls2mac+cl*ETH_ALEN,ETH_ALEN);
    opt.usemac=q->filter.usemac;
    opt.heappos=q->h.root_1[cl+1].id2idx;
    if(opt.heappos!=0) { // Is in heap
      opt.penal_ls=heap_get_penalty(&q->h,cl).ls;  
      opt.penal_ms=heap_get_penalty(&q->h,cl).ms;
    } else {
      opt.penal_ls=0;
      opt.penal_ms=0;
    }
  }
    
  // Put quing information:
  RTA_PUT(skb, TCA_OPTIONS, sizeof(opt), &opt);
  return skb->len;

rtattr_failure:
  skb_trim(skb, b - skb->data);
  return -1;
}

static int wrr_change(struct Qdisc *sch, u32 handle, u32 parent, 
                      struct rtattr **tca, unsigned long *arg)
{
  unsigned long cl = *arg;
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  struct rtattr *opt = tca[TCA_OPTIONS-1];
  struct tc_wrr_class_modf *copt = RTA_DATA(opt);
  
  if(cl==0 || cl>q->bandc) return -EINVAL;
  cl--;  
  
  if (opt->rta_len < RTA_LENGTH(sizeof(*copt))) return -EINVAL;

  LOCK_START;
  
  weight_setvalue(&q->bands[cl].class_modf.weight1,&copt->weight1);
  weight_setvalue(&q->bands[cl].class_modf.weight2,&copt->weight2);
  
  LOCK_END;

  return 0;
}

static void wrr_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
  struct wrr_sched_data *q = (struct wrr_sched_data *)sch->data;
  int prio;

  if (arg->stop) return;

  for (prio = 1; prio <= q->bandc; prio++) {
    if (arg->count < arg->skip) {
      arg->count++;
      continue;
    }
    if (arg->fn(sch, prio, arg) < 0) {
      arg->stop = 1;
      break;
    }
    arg->count++;
  }
}

static struct tcf_proto ** wrr_find_tcf(struct Qdisc *sch, unsigned long cl)
{
  return NULL;
}

static unsigned long wrr_bind(struct Qdisc *sch, 
                              unsigned long parent, u32 classid)
{
  return wrr_get(sch, classid);
}

//-----------------------------------------------------------------------------
// Generel

static struct Qdisc_class_ops wrr_class_ops =
{
	wrr_graft,
	wrr_leaf,

	wrr_get,
	wrr_put,
	wrr_change,
	wrr_delete,
	wrr_walk,

	wrr_find_tcf,
	wrr_bind,
	wrr_put,

#if !defined(KERNEL22) || defined(CONFIG_RTNETLINK)
	wrr_dump_class,
#endif
};

struct Qdisc_ops wrr_qdisc_ops =
{
	NULL,
	&wrr_class_ops,
	"wrr",
	sizeof(struct wrr_sched_data),

	wrr_enqueue,
	wrr_dequeue,
	wrr_requeue,
	wrr_drop,

	wrr_init,
	wrr_reset,
	wrr_destroy,
	wrr_tune,

#if !defined(KERNEL22) || defined(CONFIG_RTNETLINK)
	wrr_dump,
#endif
};

#ifdef MODULE

int init_module(void)
{
	return register_qdisc(&wrr_qdisc_ops);
}

void cleanup_module(void) 
{
	unregister_qdisc(&wrr_qdisc_ops);
}

#ifndef KERNEL22
  MODULE_LICENSE("GPL");
#endif

#endif

