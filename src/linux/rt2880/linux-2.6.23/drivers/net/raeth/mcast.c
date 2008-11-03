#include <linux/autoconf.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <asm/semaphore.h>


#if defined(CONFIG_RALINK_RT3052_MP) || defined(CONFIG_RALINK_RT3052_MP2)

#define MAX_MCAST_ENTRY	    16
#define AGEING_TIME	    5  //Unit: Sec
#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2], \
    ((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5]

//#define MCAST_DEBUG
#ifdef MCAST_DEBUG
#define MCAST_PRINT(fmt, args...) printk(KERN_INFO fmt, ## args)
#else
#define MCAST_PRINT(fmt, args...) { }
#endif

typedef struct {
    uint8_t	src_mac[6];
    uint8_t	dst_mac[6];
    uint32_t	valid;
    uint32_t	use_count;
    unsigned long ageout;
} mcast_entry;

mcast_entry mcast_tbl[MAX_MCAST_ENTRY];
atomic_t mcast_entry_num=ATOMIC_INIT(0);
DECLARE_MUTEX(mtbl_lock);

uint32_t inline is_multicast_pkt(uint8_t *mac)
{
    if(mac[0]==0x01) {
	return 1;
    }else{
	return 0;
    }
}

int32_t inline mcast_entry_get(uint8_t *src_mac, uint8_t *dst_mac) 
{
    int i=0;

    for(i=0;i<MAX_MCAST_ENTRY;i++) {
	if(memcmp(mcast_tbl[i].src_mac,src_mac, 6)==0 &&
		memcmp(mcast_tbl[i].dst_mac, dst_mac, 6)==0 &&
		mcast_tbl[i].valid == 1) {
	    return i;
	}
    }
    return -1;
}

int inline __add_mcast_entry(uint8_t *src_mac, uint8_t *dst_mac)
{
    int i=0;

    // use empty or ageout entry
    for(i=0;i<MAX_MCAST_ENTRY;i++) {
	if( mcast_tbl[i].valid==0 ||
		time_after(jiffies, mcast_tbl[i].ageout)) {

	    if(mcast_tbl[i].valid==0) {
		atomic_inc(&mcast_entry_num);
	    }

	    memcpy(mcast_tbl[i].src_mac, src_mac, 6);
	    memcpy(mcast_tbl[i].dst_mac, dst_mac, 6);
	    mcast_tbl[i].valid=1;
	    mcast_tbl[i].use_count=1;
	    mcast_tbl[i].ageout=jiffies + AGEING_TIME * HZ;
	   
	    return 1;
	}
    }

    MCAST_PRINT("RAETH: Multicast Table is FULL!!\n");
    return 0;
}

int inline mcast_entry_ins(uint8_t *src_mac, uint8_t *dst_mac) 
{
    int entry_num=0, ret=0;

    down(&mtbl_lock);
    if((entry_num = mcast_entry_get(src_mac, dst_mac)) >=0) {
	mcast_tbl[entry_num].use_count++;
	mcast_tbl[entry_num].ageout=jiffies + AGEING_TIME * HZ;
	MCAST_PRINT("%s: Update %0X:%0X:%0X:%0X:%0X:%0X's use_count=%d\n" \
		,__FUNCTION__, MAC_ARG(dst_mac), mcast_tbl[entry_num].use_count);
	ret = 1;
    }else { //if entry not found, create new entry.
	MCAST_PRINT("%s: Create new entry %0X:%0X:%0X:%0X:%0X:%0X\n", \
		__FUNCTION__, MAC_ARG(dst_mac));
	ret = __add_mcast_entry(src_mac,dst_mac);
    }
    
    up(&mtbl_lock);
    return ret;

}


/*
 * Return:
 *	    0: entry found
 *	    1: entry not found
 */
int inline mcast_entry_del(uint8_t *src_mac, uint8_t *dst_mac) 
{
    int entry_num;

    down(&mtbl_lock);
    if((entry_num = mcast_entry_get(src_mac, dst_mac)) >=0) {
	if((--mcast_tbl[entry_num].use_count)==0) {
	    MCAST_PRINT("%s: %0X:%0X:%0X:%0X:%0X:%0X (entry_num=%d)\n", \
		    __FUNCTION__, MAC_ARG(dst_mac), entry_num);
	    mcast_tbl[entry_num].valid=0;
	    atomic_dec(&mcast_entry_num);
	}
	up(&mtbl_lock);
	return 0;
    }else { 
	/* this multicast packet was not sent by meself, just ignore it */
	up(&mtbl_lock);
	return 1;
    }
}

/* 
 * Return
 *	    0: drop packet
 *	    1: continue
 */
int32_t mcast_rx(struct sk_buff * skb)
{
    struct ethhdr *eth=(struct ethhdr *)(skb->data-ETH_HLEN);

    /* if we do not send multicast packet before, 
     * we don't need to check re-inject multicast packet.
     */
    if (atomic_read(&mcast_entry_num)==0) {
	return 1;
    }


    if(is_multicast_pkt(eth->h_dest)) {
	MCAST_PRINT("%s: %0X:%0X:%0X:%0X:%0X:%0X\n", __FUNCTION__, \
		MAC_ARG(eth->h_dest));
	return mcast_entry_del(eth->h_source, eth->h_dest);
    }

    return 1;
}


int32_t mcast_tx(struct sk_buff *skb)
{
    struct ethhdr *eth = (struct ethhdr *) skb->data;


    if(is_multicast_pkt(eth->h_dest)) {
	MCAST_PRINT("%s: %0X:%0X:%0X:%0X:%0X:%0X\n", __FUNCTION__,\
	       	MAC_ARG(eth->h_dest));
	mcast_entry_ins(eth->h_source, eth->h_dest);
    }

    return 1;
}

#endif
