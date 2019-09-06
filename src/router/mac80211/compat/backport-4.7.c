/*
 * Copyright(c) 2016 Hauke Mehrtens <hauke@hauke-m.de>
 *
 * Backport functionality introduced in Linux 4.7.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/export.h>
#include <linux/list.h>
#include <linux/rcupdate.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <net/netlink.h>
#include <linux/firmware.h>
#include <linux/module.h>

/**
 * __nla_reserve_64bit - reserve room for attribute on the skb and align it
 * @skb: socket buffer to reserve room on
 * @attrtype: attribute type
 * @attrlen: length of attribute payload
 *
 * Adds a netlink attribute header to a socket buffer and reserves
 * room for the payload but does not copy it. It also ensure that this
 * attribute will be 64-bit aign.
 *
 * The caller is responsible to ensure that the skb provides enough
 * tailroom for the attribute header and payload.
 */
struct nlattr *__nla_reserve_64bit(struct sk_buff *skb, int attrtype,
                                  int attrlen, int padattr)
{
       if (nla_need_padding_for_64bit(skb))
               nla_align_64bit(skb, padattr);

       return __nla_reserve(skb, attrtype, attrlen);
}
EXPORT_SYMBOL_GPL(__nla_reserve_64bit);

/**
 * nla_reserve_64bit - reserve room for attribute on the skb and align it
 * @skb: socket buffer to reserve room on
 * @attrtype: attribute type
 * @attrlen: length of attribute payload
 *
 * Adds a netlink attribute header to a socket buffer and reserves
 * room for the payload but does not copy it. It also ensure that this
 * attribute will be 64-bit aign.
 *
 * Returns NULL if the tailroom of the skb is insufficient to store
 * the attribute header and payload.
 */
struct nlattr *nla_reserve_64bit(struct sk_buff *skb, int attrtype, int attrlen,
				 int padattr)
{
       size_t len;

       if (nla_need_padding_for_64bit(skb))
               len = nla_total_size_64bit(attrlen);
       else
               len = nla_total_size(attrlen);
       if (unlikely(skb_tailroom(skb) < len))
               return NULL;

       return __nla_reserve_64bit(skb, attrtype, attrlen, padattr);
}
EXPORT_SYMBOL_GPL(nla_reserve_64bit);

/**
 * __nla_put_64bit - Add a netlink attribute to a socket buffer and align it
 * @skb: socket buffer to add attribute to
 * @attrtype: attribute type
 * @attrlen: length of attribute payload
 * @data: head of attribute payload
 *
 * The caller is responsible to ensure that the skb provides enough
 * tailroom for the attribute header and payload.
 */
void __nla_put_64bit(struct sk_buff *skb, int attrtype, int attrlen,
                    const void *data, int padattr)
{
       struct nlattr *nla;

       nla = __nla_reserve_64bit(skb, attrtype, attrlen, padattr);
       memcpy(nla_data(nla), data, attrlen);
}
EXPORT_SYMBOL_GPL(__nla_put_64bit);

/**
 * nla_put_64bit - Add a netlink attribute to a socket buffer and align it
 * @skb: socket buffer to add attribute to
 * @attrtype: attribute type
 * @attrtype: attribute type
 * @attrlen: length of attribute payload
 * @data: head of attribute payload
 *
 * Returns -EMSGSIZE if the tailroom of the skb is insufficient to store
 * the attribute header and payload.
 */
int nla_put_64bit(struct sk_buff *skb, int attrtype, int attrlen,
                 const void *data, int padattr)
{
       size_t len;

       if (nla_need_padding_for_64bit(skb))
               len = nla_total_size_64bit(attrlen);
       else
               len = nla_total_size(attrlen);
       if (unlikely(skb_tailroom(skb) < len))
               return -EMSGSIZE;

       __nla_put_64bit(skb, attrtype, attrlen, data, padattr);
       return 0;
}
EXPORT_SYMBOL_GPL(nla_put_64bit);

