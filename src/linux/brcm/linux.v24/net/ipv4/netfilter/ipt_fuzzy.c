/*
 *  This module implements a simple TSK FLC 
 * (Takagi-Sugeno-Kang Fuzzy Logic Controller) that aims
 * to limit , in an adaptive and flexible way , the packet rate crossing 
 * a given stream . It serves as an initial and very simple (but effective)
 * example of how Fuzzy Logic techniques can be applied to defeat DoS attacks.
 *  As a matter of fact , Fuzzy Logic can help us to insert any "behavior"  
 * into our code in a precise , adaptive and efficient manner. 
 *  The goal is very similar to that of "limit" match , but using techniques of
 * Fuzzy Control , that allow us to shape the transfer functions precisely ,
 * avoiding over and undershoots - and stuff like that .
 *
 *
 * 2002-08-10  Hime Aguiar e Oliveira Jr. <hime@engineer.com> : Initial version.
 * 2002-08-17  : Changed to eliminate floating point operations .
 * 2002-08-23  : Coding style changes .
*/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/random.h>
#include <net/tcp.h>
#include <linux/spinlock.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_fuzzy.h>

/*
 Packet Acceptance Rate - LOW and Packet Acceptance Rate - HIGH
 Expressed in percentage
*/

#define PAR_LOW		1/100
#define PAR_HIGH	1

static spinlock_t fuzzy_lock = SPIN_LOCK_UNLOCKED ;

MODULE_AUTHOR("Hime Aguiar e Oliveira Junior <hime@engineer.com>");
MODULE_DESCRIPTION("IP tables Fuzzy Logic Controller match module");
MODULE_LICENSE("GPL");

static  u_int8_t mf_high(u_int32_t tx,u_int32_t mini,u_int32_t maxi)
{
	if (tx >= maxi) return 100;

	if (tx <= mini) return 0;

	return ( (100*(tx-mini)) / (maxi-mini) ) ;
}

static u_int8_t mf_low(u_int32_t tx,u_int32_t mini,u_int32_t maxi)
{
	if (tx <= mini) return 100;

	if (tx >= maxi) return 0;

	return ( (100*( maxi - tx ))  / ( maxi - mini ) ) ;

}

static int
ipt_fuzzy_match(const struct sk_buff *pskb,
	       const struct net_device *in,
	       const struct net_device *out,
	       const void *matchinfo,
	       int offset,
	       const void *hdr,
	       u_int16_t datalen,
	       int *hotdrop)
{
	/* From userspace */
	
	struct ipt_fuzzy_info *info = (struct ipt_fuzzy_info *) matchinfo;

	u_int8_t random_number;
	unsigned long amount ;
	u_int8_t howhigh , howlow ;
	

	spin_lock_bh(&fuzzy_lock) ; /* Rise the lock */

	info->bytes_total += pskb->len ;
	info->packets_total++ ;

	info->present_time = jiffies ;
	
	if ( info->present_time >= info->previous_time )
		amount = info->present_time - info->previous_time ;
	else { 
	       	/* There was a transition : I choose to re-sample 
		   and keep the old acceptance rate...
	        */

		amount = 0 ;
		info->previous_time = info->present_time ;
		info->bytes_total = info->packets_total = 0;
	     };
	
	if (  amount > HZ/10 ) /* More than 100 ms elapsed ... */
		{

	info->mean_rate = (u_int32_t) ( ( HZ * info->packets_total )  \
		  		        / amount ) ;

		info->previous_time = info->present_time ;
		info->bytes_total = info->packets_total = 0 ;

       howhigh = mf_high(info->mean_rate,info->minimum_rate,info->maximum_rate);
       howlow  = mf_low(info->mean_rate,info->minimum_rate,info->maximum_rate);

    info->acceptance_rate = (u_int8_t) \
		           ( howhigh*PAR_LOW + PAR_HIGH*howlow ) ;

    /* In fact , the above defuzzification would require a denominator
       proportional to (howhigh+howlow) but , in this particular case ,
       that expression is constant .
        An imediate consequence is that it isn't necessary to call 
       both mf_high and mf_low - but to keep things understandable ,
       I did so .
     */ 

		}
	
	spin_unlock_bh(&fuzzy_lock) ; /* Release the lock */


	if ( info->acceptance_rate < 100 )
	{		 
		get_random_bytes((void *)(&random_number), 1);

		/*  If within the acceptance , it can pass => don't match */
		if ( random_number <= (255 * info->acceptance_rate) / 100 )
			return 0 ;
		else
			return 1; /* It can't pass ( It matches ) */
	} ;

	return 0; /* acceptance_rate == 100 % => Everything passes ... */
	
}

static int
ipt_fuzzy_checkentry(const char *tablename,
		   const struct ipt_ip *e,
		   void *matchinfo,
		   unsigned int matchsize,
		   unsigned int hook_mask)
{
	
	const struct ipt_fuzzy_info *info = matchinfo;

	if (matchsize != IPT_ALIGN(sizeof(struct ipt_fuzzy_info))) {
		printk("ipt_fuzzy: matchsize %u != %u\n", matchsize,
		       IPT_ALIGN(sizeof(struct ipt_fuzzy_info)));
		return 0;
	}

if ((info->minimum_rate < MINFUZZYRATE ) || (info->maximum_rate > MAXFUZZYRATE)
	|| (info->minimum_rate >= info->maximum_rate ))
		{
		printk("ipt_fuzzy: BAD limits , please verify !!!\n");
		return 0;
		}

	return 1;
}

static struct ipt_match ipt_fuzzy_reg = { 
	{NULL, NULL},
	"fuzzy",
	ipt_fuzzy_match,
	ipt_fuzzy_checkentry,
	NULL,
	THIS_MODULE };

static int __init init(void)
{
	if (ipt_register_match(&ipt_fuzzy_reg))
		return -EINVAL;

	return 0;
}

static void __exit fini(void)
{
	ipt_unregister_match(&ipt_fuzzy_reg);
}

module_init(init);
module_exit(fini);
