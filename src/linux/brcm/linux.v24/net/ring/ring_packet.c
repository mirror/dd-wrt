/*
 *
 * (C) 2004 - Luca Deri <deri@ntop.org>
 *
 */

#include <linux/version.h>
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/socket.h>
#include <linux/skbuff.h>
#include <linux/rtnetlink.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/init.h>
#include <linux/filter.h>
#include <linux/ring.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#include <net/xfrm.h>
#else
#include <linux/poll.h>
#endif
#include <net/sock.h>
#include <asm/io.h>        /* needed for virt_to_phys()    */

/* #define RING_DEBUG */

/* ************************************************* */

struct ring_list {
  struct sock      *sk;
  struct ring_list *next;
};

struct ring_opt {
  struct net_device *ring_netdev;

  /* Packet buffers */
  unsigned long order;

  /* Ring Slots */
  unsigned long ring_memory;
  FlowSlotInfo *slots_info; /* Basically it points to ring_memory */
  char *ring_slots;  /* Basically it points to ring_memory+sizeof(FlowSlotInfo) */

  /* Packet Sampling */
  u_int pktToSample, sample_rate;

  /* BPF Filter */
  struct sk_filter *bpfFilter;
  /* struct sock_fprog fprog; */

  /* Locks */
  atomic_t num_ring_slots_waiters;
  wait_queue_head_t ring_slots_waitqueue;
  rwlock_t ring_index_lock;

  /* Indexes (Internal) */
  u_int insert_page_id, insert_slot_id;
};

/* ************************************************* */

/* List of all ring sockets. */
static struct ring_list *ring_table;
static rwlock_t ring_mgmt_lock = RW_LOCK_UNLOCKED;

/* ********************************** */

/* Forward */
static struct proto_ops ring_ops;
static int my_ring_handler(struct sk_buff *skb, u_char recv_packet);

/* Extern */

/* ********************************** */

/* Defaults */
static u_int bucket_len = 128, num_slots = 4096, sample_rate = 1, transparent_mode = 1, enable_tx_capture = 1;

MODULE_PARM(bucket_len, "i");
MODULE_PARM_DESC(bucket_len, "Number of ring buckets");
MODULE_PARM(num_slots,  "i");
MODULE_PARM_DESC(num_slots,  "Number of ring slots");
MODULE_PARM(sample_rate, "i");
MODULE_PARM_DESC(sample_rate, "Ring packet sample rate");
MODULE_PARM(transparent_mode, "i");
MODULE_PARM_DESC(transparent_mode, "Set to 1 to set transparent mode (slower but backwards compatible)");
MODULE_PARM(enable_tx_capture, "i");
MODULE_PARM_DESC(enable_tx_capture, "Set to 1 to capture outgoing packets");

/* ********************************** */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#define ring_sk(__sk) ((struct ring_opt *)(__sk)->sk_protinfo)
#else
#define ring_sk(__sk) ((__sk)->protinfo.pf_ring)
#endif
#define _rdtsc()      ({ uint64_t x; asm volatile("rdtsc" : "=A" (x)); x; })


/* ********************************** */

static void ring_sock_destruct(struct sock *sk) {

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
  skb_queue_purge(&sk->sk_receive_queue);

  if (!sock_flag(sk, SOCK_DEAD)) {
#ifdef RING_DEBUG
    printk("Attempt to release alive ring socket: %p\n", sk);
#endif
    return;
  }

  BUG_TRAP(!atomic_read(&sk->sk_rmem_alloc));
  BUG_TRAP(!atomic_read(&sk->sk_wmem_alloc));
#else

  BUG_TRAP(atomic_read(&sk->rmem_alloc)==0);
  BUG_TRAP(atomic_read(&sk->wmem_alloc)==0);

  if (!sk->dead) {
#ifdef RING_DEBUG
    printk("Attempt to release alive ring socket: %p\n", sk);
#endif
    return;
  }
#endif

  kfree(ring_sk(sk));

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
  MOD_DEC_USE_COUNT;
#endif
}

/* ********************************** */

static void ring_insert(struct sock *sk) {
  struct ring_list *next;

#ifdef RING_DEBUG
  printk("RING: ring_insert()\n");
#endif

  write_lock(&ring_mgmt_lock);
  next = kmalloc(sizeof(struct ring_list), GFP_ATOMIC);
  if(next != NULL) {
    next->sk = sk;
    next->next = ring_table;
    ring_table = next;
  }

  write_unlock(&ring_mgmt_lock);
}

/* ********************************** */

static void ring_remove(struct sock *sk) {
  struct ring_list *ptr, *prev = NULL;

  ptr = ring_table;

  while(ptr != NULL) {
    if(ptr->sk == sk) {
      struct ring_list *next = ptr->next;
      kfree(ptr);

      if(prev != NULL)
	prev->next = next;
      else
	ring_table = next;

      break;
    } else {
      prev = ptr;
      ptr = ptr->next;
    }
  }
}

/* ********************************** */

static FlowSlot* get_insert_slot(struct ring_opt *pfr) {
#ifdef RING_DEBUG
  printk("get_insert_slot(%d)\n", pfr->slots_info->insert_idx);
#endif

  if(pfr->ring_slots != NULL) {
    FlowSlot *slot = (FlowSlot*)&(pfr->ring_slots[pfr->slots_info->insert_idx*pfr->slots_info->slot_len]);
    return(slot);
  } else
    return(NULL);
}

/* ********************************** */

static FlowSlot* get_remove_slot(struct ring_opt *pfr) {
#ifdef RING_DEBUG
  printk("get_remove_slot(%d)\n", pfr->slots_info->remove_idx);
#endif

  if(pfr->ring_slots != NULL)
    return((FlowSlot*)&(pfr->ring_slots[pfr->slots_info->remove_idx*pfr->slots_info->slot_len]));
  else
    return(NULL);
}

/* ********************************** */

static void add_skb_to_ring(struct sk_buff *skb, struct ring_opt *pfr, u_char recv_packet) {
  FlowSlot *theSlot;
  int idx, displ;

  if(recv_packet)
    displ = SKB_DISPLACEMENT;
  else
    displ = 0;

  write_lock(&pfr->ring_index_lock);

  pfr->slots_info->tot_pkts++;

  /* BPF Filtering (from af_packet.c) */
  if(pfr->bpfFilter != NULL) {
    unsigned res = 1, len;

    len = skb->len-skb->data_len;

    skb->data -= displ;
    res = sk_run_filter(skb, pfr->bpfFilter->insns, pfr->bpfFilter->len);
    skb->data += displ;

    if(res == 0) {
      /* Filter failed */
      write_unlock(&pfr->ring_index_lock);

#ifdef RING_DEBUG
      printk("add_skb_to_ring(skb): Filter failed [len=%d][tot=%d][insertIdx=%d][pkt_type=%d][cloned=%d]\n",
	 (int)skb->len, pfr->slots_info->tot_pkts,
	 pfr->slots_info->insert_idx,
	 skb->pkt_type, skb->cloned);
#endif

      return;
    }
  }

  /* ************************** */

  if(pfr->sample_rate > 1) {
    if(pfr->pktToSample == 0)
      pfr->pktToSample = pfr->sample_rate;
    else {
      pfr->pktToSample--;
      write_unlock(&pfr->ring_index_lock);

#ifdef RING_DEBUG
      printk("add_skb_to_ring(skb): sampled packet [len=%d][tot=%d][insertIdx=%d][pkt_type=%d][cloned=%d]\n",
	     (int)skb->len, pfr->slots_info->tot_pkts,
	     pfr->slots_info->insert_idx,
	     skb->pkt_type, skb->cloned);
#endif
      return;
    }
  }

#ifdef RING_DEBUG
  printk("add_skb_to_ring(skb) [len=%d][tot=%d][insertIdx=%d][pkt_type=%d][cloned=%d]\n",
	 (int)skb->len, pfr->slots_info->tot_pkts,
	 pfr->slots_info->insert_idx,
	 skb->pkt_type, skb->cloned);
#endif

  idx = pfr->slots_info->insert_idx;
  theSlot = get_insert_slot(pfr);

  if((theSlot != NULL) && (theSlot->slot_state == 0)) {
    struct pcap_pkthdr *hdr;
    unsigned int bucketSpace;
    char *bucket;


    /* Update Index */
    idx++;

    if(idx == pfr->slots_info->tot_slots)
      pfr->slots_info->insert_idx = 0;
    else
      pfr->slots_info->insert_idx = idx;

    write_unlock(&pfr->ring_index_lock);

    bucketSpace = pfr->slots_info->slot_len
#ifdef RING_MAGIC
      - sizeof(u_char)
#endif
      - sizeof(u_char)  /* flowSlot.slot_state */
      - sizeof(struct pcap_pkthdr)
      - 1 /* 10 */ /* safe boundary */;

    bucket = &theSlot->bucket;
    hdr = (struct pcap_pkthdr*)bucket;

    if(skb->stamp.tv_sec == 0) do_gettimeofday(&skb->stamp);

    hdr->ts.tv_sec = skb->stamp.tv_sec, hdr->ts.tv_usec = skb->stamp.tv_usec;
    hdr->caplen    = skb->len+displ;

    if(hdr->caplen > bucketSpace)
      hdr->caplen = bucketSpace;

    hdr->len = skb->len+displ;
    memcpy(&bucket[sizeof(struct pcap_pkthdr)], skb->data-displ, hdr->caplen);

#ifdef RING_DEBUG
    {
      static unsigned int lastLoss = 0;

      if(pfr->slots_info->tot_lost && (lastLoss != pfr->slots_info->tot_lost)) {
	printk("add_skb_to_ring(%d): [bucketSpace=%d]"
	       "[hdr.caplen=%d][skb->len=%d]"
	       "[pcap_pkthdr=%d][removeIdx=%d][loss=%d][page=%d][slot=%d]\n",
	       idx-1, bucketSpace, hdr->caplen, skb->len,
	       sizeof(struct pcap_pkthdr),
	       pfr->slots_info->remove_idx,
	       pfr->slots_info->tot_lost,
	       pfr->insert_page_id, pfr->insert_slot_id);

	lastLoss = pfr->slots_info->tot_lost;
      }
    }
#endif

    pfr->slots_info->tot_insert++;
    theSlot->slot_state = 1;
  } else {
    pfr->slots_info->tot_lost++;
    write_unlock(&pfr->ring_index_lock);

#ifdef RING_DEBUG
    printk("add_skb_to_ring(skb): packet lost [loss=%d][removeIdx=%d][insertIdx=%d]\n",
	   pfr->slots_info->tot_lost, pfr->slots_info->remove_idx, pfr->slots_info->insert_idx);
#endif
  }

  /* wakeup in case of poll() */
  if(waitqueue_active(&pfr->ring_slots_waitqueue))
    wake_up_interruptible(&pfr->ring_slots_waitqueue);
}

/* ********************************** */

static int my_ring_handler(struct sk_buff *skb, u_char recv_packet) {
  struct sock *skElement;
  int rc = 0;
#ifdef RING_DEBUG
  int num=0;
#endif
  struct ring_list *ptr;
#ifdef PROFILING
  uint64_t rdt = _rdtsc(), rdt1, rdt2;
#endif

  if((!skb) /* Invalid skb */
     || ((!enable_tx_capture) && (!recv_packet))) /*
						    An outgoing packet is about to be sent out
						    but we decided not to handle transmitted
						    packets.
						  */
    return(0);

#ifdef RING_DEBUG
  if(0) {
    printk("my_ring_handler() [len=%d][dev=%s]\n", skb->len,
	   skb->dev->name == NULL ? "<NULL>" : skb->dev->name);
  }
#endif

  read_lock(&ring_mgmt_lock);

#ifdef PROFILING
  rdt1 = _rdtsc();
#endif

  ptr = ring_table;
  while(ptr != NULL) {
    struct ring_opt *pfr;

    skElement = ptr->sk;

    pfr = ring_sk(skElement);
    if((pfr != NULL)
       && (pfr->ring_slots != NULL)
       && (pfr->ring_netdev == skb->dev)) {
      /* We've found the ring where the packet can be stored */
      add_skb_to_ring(skb, pfr, recv_packet);

      /* DO NOT DISABLE THE MAIN NETWORK INTERFACE !!!! */
      rc = 1; /* Ring found: we've done our job */
    }

    ptr = ptr->next;
#ifdef RING_DEBUG
    num++;
#endif
  }

#ifdef RING_DEBUG
  /* printk("# loops: %d\n", num); */
#endif

#ifdef PROFILING
  rdt1 = _rdtsc()-rdt1;
#endif

  read_unlock(&ring_mgmt_lock);

#ifdef PROFILING
  rdt2 = _rdtsc();
#endif

  if(transparent_mode) rc = 0;

  if(rc != 0)
    dev_kfree_skb(skb); /* Free the skb */

#ifdef PROFILING
  rdt2 = _rdtsc()-rdt2;
  rdt = _rdtsc()-rdt;

#ifdef RING_DEBUG
  printk("# cycles: %d [lock costed %d %d%%][free costed %d %d%%]\n",
	 (int)rdt, rdt-rdt1,
	 (int)((float)((rdt-rdt1)*100)/(float)rdt),
	 rdt2,
	 (int)((float)(rdt2*100)/(float)rdt));
#endif
#endif

  return(rc); /*  0 = packet not handled */
}

/* ********************************** */

static int ring_create(struct socket *sock, int protocol) {
  struct sock *sk;
  struct ring_opt *pfr;
  int err;

#ifdef RING_DEBUG
  printk("RING: ring_create()\n");
#endif

  /* Are you root, superuser or so ? */
  if(!capable(CAP_NET_ADMIN))
    return -EPERM;

  if(sock->type != SOCK_RAW)
    return -ESOCKTNOSUPPORT;

  if(protocol != htons(ETH_P_ALL))
    return -EPROTONOSUPPORT;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
  MOD_INC_USE_COUNT;
#endif

  err = -ENOMEM;
  sk = sk_alloc(PF_RING, GFP_KERNEL, 1
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
		, NULL
#endif
		);
  if (sk == NULL)
    goto out;

  sock->ops = &ring_ops;
  sock_init_data(sock, sk);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
  sk_set_owner(sk, THIS_MODULE);
#endif

  err = -ENOMEM;
  pfr = ring_sk(sk) = kmalloc(sizeof(*pfr), GFP_KERNEL);
  if (!pfr) {
    sk_free(sk);
    goto out;
  }
  memset(pfr, 0, sizeof(*pfr));
  init_waitqueue_head(&pfr->ring_slots_waitqueue);
  pfr->ring_index_lock = RW_LOCK_UNLOCKED;
  atomic_set(&pfr->num_ring_slots_waiters, 0);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
  sk->sk_family       = PF_RING;
  sk->sk_destruct     = ring_sock_destruct;
#else
  sk->family          = PF_RING;
  sk->destruct        = ring_sock_destruct;
  sk->num             = protocol;
#endif

  ring_insert(sk);
  return(0);
 out:
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
  MOD_DEC_USE_COUNT;
#endif
  return err;
}

/* *********************************************** */

static int ring_release(struct socket *sock)
{
  struct sock *sk = sock->sk;
  struct ring_opt *pfr = ring_sk(sk);

  if(!sk)
    return 0;

#ifdef RING_DEBUG
  printk("RING: called ring_release\n");
#endif

  write_lock_bh(&ring_mgmt_lock);

#ifdef RING_DEBUG
  printk("RING: ring_release entered\n");
#endif

  ring_remove(sk);

  sock_orphan(sk);
  sock->sk = NULL;

  /* Free the ring buffer */
  if(pfr->ring_memory) {
    struct page *page, *page_end;

    page_end = virt_to_page(pfr->ring_memory + (PAGE_SIZE << pfr->order) - 1);
    for(page = virt_to_page(pfr->ring_memory); page <= page_end; page++)
      ClearPageReserved(page);

    free_pages(pfr->ring_memory, pfr->order);
  }

  kfree(pfr);
  ring_sk(sk) = NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
  skb_queue_purge(&sk->sk_write_queue);
#endif
  sock_put(sk);

#ifdef RING_DEBUG
  printk("RING: ring_release leaving\n");
#endif

  write_unlock_bh(&ring_mgmt_lock);

  return 0;
}

/* ********************************** */
/*
 * We create a ring for this socket and bind it to the specified device
 */
static int packet_ring_bind(struct sock *sk, struct net_device *dev)
{
  u_int the_slot_len;
  u_int32_t tot_mem;
  struct ring_opt *pfr = ring_sk(sk);
  struct page *page, *page_end;

  if(!dev) return(-1);

#ifdef RING_DEBUG
  printk("RING: packet_ring_bind(%s) called\n", dev->name);
#endif

  /* **********************************************

  *************************************
  *                                   *
  *        FlowSlotInfo               *
  *                                   *
  ************************************* <-+
  *        FlowSlot                   *   |
  *************************************   |
  *        FlowSlot                   *   |
  *************************************   +- num_slots
  *        FlowSlot                   *   |
  *************************************   |
  *        FlowSlot                   *   |
  ************************************* <-+

  ********************************************** */

  the_slot_len = sizeof(u_char)    /* flowSlot.slot_state */
                 + sizeof(u_short) /* flowSlot.slot_len   */
                 + bucket_len      /* flowSlot.bucket     */;

  tot_mem = sizeof(FlowSlotInfo) + num_slots*the_slot_len;

  /*
    Calculate the value of the order parameter used later.
    See http://www.linuxjournal.com/article.php?sid=1133
  */
  for(pfr->order = 0;(PAGE_SIZE << pfr->order) < tot_mem; pfr->order++)  ;

  /*
     We now try to allocate the memory as required. If we fail
     we try to allocate a smaller amount or memory (hence a
     smaller ring).
  */
  while((pfr->ring_memory = __get_free_pages(GFP_ATOMIC, pfr->order)) == 0)
    if(pfr->order-- == 0)
      break;

  if(pfr->order == 0) {
#ifdef RING_DEBUG
    printk("ERROR: not enough memory\n");
#endif
    return(-1);
  } else {
#ifdef RING_DEBUG
    printk("RING: succesfully allocated %lu KB [tot_mem=%d][order=%ld]\n",
	   PAGE_SIZE >> (10 - pfr->order), tot_mem, pfr->order);
#endif
  }

  tot_mem = PAGE_SIZE << pfr->order;
  memset((char*)pfr->ring_memory, 0, tot_mem);

  /* Now we need to reserve the pages */
  page_end = virt_to_page(pfr->ring_memory + (PAGE_SIZE << pfr->order) - 1);
  for(page = virt_to_page(pfr->ring_memory); page <= page_end; page++)
    SetPageReserved(page);

  pfr->slots_info = (FlowSlotInfo*)pfr->ring_memory;
  pfr->ring_slots = (char*)(pfr->ring_memory+sizeof(FlowSlotInfo));

  pfr->slots_info->version     = RING_FLOWSLOT_VERSION;
  pfr->slots_info->slot_len    = the_slot_len;
  pfr->slots_info->tot_slots   = (tot_mem-sizeof(FlowSlotInfo))/the_slot_len;
  pfr->slots_info->tot_mem     = tot_mem;
  pfr->slots_info->sample_rate = sample_rate;

#ifdef RING_DEBUG
  printk("RING: allocated %d slots [slot_len=%d][tot_mem=%u]\n",
	 pfr->slots_info->tot_slots, pfr->slots_info->slot_len,
	 pfr->slots_info->tot_mem);
#endif

#ifdef RING_MAGIC
 {
   int i;

   for(i=0; i<pfr->slots_info->tot_slots; i++) {
     unsigned long idx = i*pfr->slots_info->slot_len;
     FlowSlot *slot = (FlowSlot*)&pfr->ring_slots[idx];

     /* printk("RING: Setting RING_MAGIC_VALUE into slot %d [displacement=%lu]\n", i, idx); */
     slot->magic = RING_MAGIC_VALUE; slot->slot_state = 0;
   }
 }
#endif

  pfr->insert_page_id = 1, pfr->insert_slot_id = 0;

  /*
    IMPORTANT
    Leave this statement here as last one. In fact when
    the ring_netdev != NULL the socket is ready to be used.
  */
  pfr->ring_netdev = dev;

  return(0);
}

/* ************************************* */

/* Bind to a device */
static int ring_bind(struct socket *sock, struct sockaddr *sa, int addr_len)
{
  struct sock *sk=sock->sk;
  struct net_device *dev = NULL;

#ifdef RING_DEBUG
  printk("RING: ring_bind() called\n");
#endif

  /*
   *	Check legality
   */
  if (addr_len != sizeof(struct sockaddr))
    return -EINVAL;
  if (sa->sa_family != PF_RING)
    return -EINVAL;

  /* Safety check: add trailing zero if missing */
  sa->sa_data[sizeof(sa->sa_data)-1] = '\0';

#ifdef RING_DEBUG
  printk("RING: searching device %s\n", sa->sa_data);
#endif

  if((dev = __dev_get_by_name(sa->sa_data)) == NULL) {
#ifdef RING_DEBUG
    printk("RING: search failed\n");
#endif
    return(-EINVAL);
  } else
    return(packet_ring_bind(sk, dev));
}

/* ************************************* */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
volatile void* virt_to_kseg(volatile void* address) {
  pte_t *pte;

  pte = pte_offset_map(pmd_offset(pgd_offset_k((unsigned long) address),
				  (unsigned long) address),
		       (unsigned long) address);
  return((volatile void*)pte_page(*pte));
}
#else /* 2.4 */

/* http://www.scs.ch/~frey/linux/memorymap.html */
volatile void *virt_to_kseg(volatile void *address)
{
  pgd_t *pgd; pmd_t *pmd; pte_t *ptep, pte;
  unsigned long va, ret = 0UL;

  va=VMALLOC_VMADDR((unsigned long)address);

  /* get the page directory. Use the kernel memory map. */
  pgd = pgd_offset_k(va);

  /* check whether we found an entry */
  if (!pgd_none(*pgd))
    {
      /* get the page middle directory */
      pmd = pmd_offset(pgd, va);
      /* check whether we found an entry */
      if (!pmd_none(*pmd))
	{
	  /* get a pointer to the page table entry */
	  ptep = pte_offset(pmd, va);
	  pte = *ptep;
	  /* check for a valid page */
	  if (pte_present(pte))
	    {
	      /* get the address the page is refering to */
	      ret = (unsigned long)page_address(pte_page(pte));
	      /* add the offset within the page to the page address */
	      ret |= (va & (PAGE_SIZE -1));
	    }
	}
    }
  return((volatile void *)ret);
}
#endif

/* ************************************* */

static int ring_mmap(struct file *file,
		     struct socket *sock,
		     struct vm_area_struct *vma)
{
  struct sock *sk = sock->sk;
  struct ring_opt *pfr = ring_sk(sk);
  unsigned long size, start;
  u_int pagesToMap;
  char *ptr;

#ifdef RING_DEBUG
  printk("RING: ring_mmap() called\n");
#endif

  if(pfr->ring_memory == 0) {
#ifdef RING_DEBUG
    printk("RING: ring_mmap() failed: mapping area to an unbound socket\n");
#endif
    return -EINVAL;
  }

  size = (unsigned long)(vma->vm_end-vma->vm_start);

  if(size % PAGE_SIZE) {
#ifdef RING_DEBUG
    printk("RING: ring_mmap() failed: len is not multiple of PAGE_SIZE\n");
#endif
    return(-EINVAL);
  }

  /* if userspace tries to mmap beyond end of our buffer, fail */
  if(size > pfr->slots_info->tot_mem) {
#ifdef RING_DEBUG
    printk("proc_mmap() failed: area too large [%ld > %d]\n", size, pfr->slots_info->tot_mem);
#endif
    return(-EINVAL);
  }

  pagesToMap = size/PAGE_SIZE;

#ifdef RING_DEBUG
  printk("RING: ring_mmap() called. %d pages to map\n", pagesToMap);
#endif

#ifdef RING_DEBUG
  printk("RING: mmap [slot_len=%d][tot_slots=%d] for ring on device %s\n",
	 pfr->slots_info->slot_len, pfr->slots_info->tot_slots,
	 pfr->ring_netdev->name);
#endif

  /* we do not want to have this area swapped out, lock it */
  vma->vm_flags |= VM_LOCKED;
  start = vma->vm_start;

  /* Ring slots start from page 1 (page 0 is reserved for FlowSlotInfo) */
  ptr = (char*)(start+PAGE_SIZE);

  if(remap_page_range(
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
		      vma,
#endif
		      start,
		      __pa(pfr->ring_memory),
		      PAGE_SIZE*pagesToMap, vma->vm_page_prot)) {
#ifdef RING_DEBUG
    printk("remap_page_range() failed\n");
#endif
    return(-EAGAIN);
  }

#ifdef RING_DEBUG
  printk("proc_mmap(pagesToMap=%d): success.\n", pagesToMap);
#endif

  return 0;
}

/* ************************************* */

unsigned int ring_poll(struct file * file,
		       struct socket *sock, poll_table *wait)
{
  FlowSlot* slot;
  struct ring_opt *pfr = ring_sk(sock->sk);

#ifdef RING_DEBUG
  printk("poll called\n");
#endif

  slot = get_remove_slot(pfr);

  if((slot == NULL) || (slot->slot_state == 0))
    poll_wait(file, &pfr->ring_slots_waitqueue, wait);

#ifdef RING_DEBUG
  printk("poll returning %d\n", slot->slot_state);
#endif

  if((slot != NULL) && (slot->slot_state == 1))
    return(POLLIN | POLLRDNORM);
  else
    return 0;
}

/* ************************************* */

/* Code taken/inspired from core/sock.c */
static int ring_setsockopt(struct socket *sock, int level, int optname,
			   char *optval, int optlen)
{
  struct ring_opt *pfr = ring_sk(sock->sk);
  int val, found = 0, ret = 0;

  if(optlen<sizeof(int))
    return(-EINVAL);

  if (get_user(val, (int *)optval))
    return -EFAULT;

  write_lock(&ring_mgmt_lock);

  switch(optname)
    {
    case SO_ATTACH_FILTER:
      found = 1;
      ret = -EINVAL;
      if (optlen == sizeof(struct sock_fprog)) {
	unsigned int fsize;
	struct sock_fprog fprog;

	ret = -EFAULT;
	if(copy_from_user(&fprog, optval, sizeof(fprog)))
	  break;

	fsize = sizeof(struct sock_filter) * fprog.len;
	pfr->bpfFilter = kmalloc(fsize, GFP_KERNEL);

	if(pfr->bpfFilter == NULL) {
	  ret = -ENOMEM;
	  break;
	}

	if(copy_from_user(pfr->bpfFilter->insns, fprog.filter, fsize))
	  break;

	pfr->bpfFilter->len = fprog.len;

	if(sk_chk_filter(pfr->bpfFilter->insns, pfr->bpfFilter->len) != 0) {
	  /* Bad filter specified */
	  kfree(pfr->bpfFilter);
	  pfr->bpfFilter = NULL;
	  break;
	}
      }
      ret = 0;
      break;

    case SO_DETACH_FILTER:
      found = 1;
      if(pfr->bpfFilter != NULL) {
	kfree(pfr->bpfFilter);
	pfr->bpfFilter = NULL;
	break;
      }
      ret = -ENONET;
      break;
    }

  write_unlock(&ring_mgmt_lock);

  if(found)
    return ret;
  else
    return(sock_setsockopt(sock, level, optname, optval, optlen));
}

/* ************************************* */

static int ring_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{

  switch(cmd)
    {
    case SIOCGIFFLAGS:
    case SIOCSIFFLAGS:
    case SIOCGIFCONF:
    case SIOCGIFMETRIC:
    case SIOCSIFMETRIC:
    case SIOCGIFMEM:
    case SIOCSIFMEM:
    case SIOCGIFMTU:
    case SIOCSIFMTU:
    case SIOCSIFLINK:
    case SIOCGIFHWADDR:
    case SIOCSIFHWADDR:
    case SIOCSIFMAP:
    case SIOCGIFMAP:
    case SIOCSIFSLAVE:
    case SIOCGIFSLAVE:
    case SIOCGIFINDEX:
    case SIOCGIFNAME:
    case SIOCGIFCOUNT:
    case SIOCSIFHWBROADCAST:
      return(dev_ioctl(cmd,(void *) arg));

    default:
      return -EOPNOTSUPP;
    }

  return 0;
}

/* ************************************* */

static struct proto_ops ring_ops = {
  .family	=	PF_RING,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
  .owner	=	THIS_MODULE,
#endif

  /* Operations that make no sense on ring sockets. */
  .connect	=	sock_no_connect,
  .socketpair	=	sock_no_socketpair,
  .accept	=	sock_no_accept,
  .getname	=	sock_no_getname,
  .listen	=	sock_no_listen,
  .shutdown	=	sock_no_shutdown,
  .sendpage	=	sock_no_sendpage,
  .sendmsg	=	sock_no_sendmsg,
  .recvmsg	=	sock_no_recvmsg,
  .getsockopt	=	sock_no_getsockopt,

  /* Now the operations that really occur. */
  .release	=	ring_release,
  .bind		=	ring_bind,
  .mmap		=	ring_mmap,
  .poll		=	ring_poll,
  .setsockopt	=	ring_setsockopt,
  .ioctl	=	ring_ioctl,
};

static struct net_proto_family ring_family_ops = {
  .family	=	PF_RING,
  .create	=	ring_create,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
  .owner	=	THIS_MODULE,
#endif
};

static void __exit ring_exit(void)
{
  while(ring_table != NULL) {
    struct ring_list *next = ring_table->next;
    kfree(ring_table);
    ring_table = next;
  }

  set_ring_handler(NULL);
  sock_unregister(PF_RING);
}

static int __init ring_init(void)
{
  ring_table = NULL;
  sock_register(&ring_family_ops);
  set_ring_handler(my_ring_handler);
  return 0;
}

module_init(ring_init);
module_exit(ring_exit);
MODULE_LICENSE("GPL");

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
MODULE_ALIAS_NETPROTO(PF_RING);
#endif
