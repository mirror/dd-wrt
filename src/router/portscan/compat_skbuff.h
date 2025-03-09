#pragma once
#include <linux/kconfig.h>
struct tcphdr;
struct udphdr;

#define skb_ifindex(skb) (skb)->skb_iif
#define skb_nfmark(skb) (((struct sk_buff *)(skb))->mark)

#if IS_ENABLED(CONFIG_NETWORK_SECMARK)
#	define skb_secmark(skb) ((skb)->secmark)
#else
#	define skb_secmark(skb) 0
#endif
