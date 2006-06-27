/*  Copyright (C) 2003 by Venkat Pothamsetty, vpothams@cisco.com */

/* This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version
   2 of the License, or (at your option) any later version.
*/


/* Kernel module to match MODBUS parameters. */
#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/tcp.h>
#include <linux/netfilter_ipv4/ipt_modbus.h>
#include <linux/netfilter_ipv4/ip_tables.h>

EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");

/* 
The reference Modbus header template.  This template is
matched up against the Modbus packet that comes in, and
extract the headers we want 
*/
   
struct modbus_tcp
{

struct modbus_hdr
{
    __u16 transaction_id;
    __u16 protocol_id;
    __u16 length;
  }modbus_h;
  
  struct modbus_data
  {
    __u8 unit_id;
    __u8 func_code;
    __u16 ref_num;
    __u16 word_cnt;
    __u8 byte_cnt;
  }modbus_d;
  
};


/* 
The function which checks whether the length feild that
the user is interested matches with that of the packet we got 
*/

static inline int
length_check(int flags, u_int16_t packet_length_s,u_int16_t length_min,u_int16_t length_max, int invert)
{
  u_int16_t packet_length;

  packet_length=ntohs(packet_length_s);
  
  if(flags && ((length_min == 0) && (packet_length > length_max)) || ((length_max == 0) && (packet_length < length_min)) || ((length_max == length_min) && (packet_length == length_max)))
    return 1 ^ invert;

  return 0 ^ invert;
}

/* 
The function which checks whether the function code that
the user is interested matches with that of the packet we got 
*/

static inline int
func_code_check(int flags, u_int16_t packet_code,u_int16_t given_code_min,u_int16_t given_code_max, int invert)
{

  if(flags && (packet_code >= given_code_min) && (packet_code <= given_code_max))
    /* Ex-Or the return for a !*/
    return 1 ^ invert;
  
  else
    return 0 ^ invert;
}

/* 
The function which checks whether the unit ID that
the user is interested matches with that of the packet we got 
*/

static inline int
unitid_check(int flags, u_int16_t packet_unitid,u_int16_t given_unitid,  int invert)
{

  if(flags && (packet_unitid == given_unitid))
    return 1 ^ invert;
  else
    return 0 ^ invert;
}

/* 
The function which checks whether the reference number that
the user is interested matches with that of the packet we got 
*/

static inline int
refnum_check(int flags, u_int16_t packet_refnum,u_int16_t given_refnum,int invert)
{

  if(flags && (packet_refnum == given_refnum))
    return 1 ^ invert;
  else
    return 0 ^ invert;
}

/* 
Triggers when a packet comes in matching the registeres 
match 
*/

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      const void *hdr,
      u_int16_t datalen,
      int *hotdrop)
{
  
  const struct iphdr *iph;
  const struct tcphdr *tcph;
  u_int8_t tcplen;

  /* Examine the TCP header, which is 32 bytes after the IP
     header.  "hdr" points to just after IP header */
  const struct modbus_tcp *modbus;
  const struct ipt_modbus *modbusinfo = matchinfo;
  const struct modbus_data *data;
  

  iph = skb->nh.iph;
  //tcplen = (skb)->len - iph->ihl*4;
  
  tcph = (void *)iph + iph->ihl*4;

  /* TCP header length caluculation*/
  tcplen = tcph->doff*4;

  /* Match our structure to the data part */
  modbus = hdr+tcplen;

  
  //printk(KERN_DEBUG "lengths: %d-%d-%d:%d\n",iph->ihl,ntohs(iph->tot_len),tcplen, 20+tcplen);
  
  /* If length is less then the total of IP and TCP header, that
     should be part of three way handshake .. allow it ... its
     a hack and needs to be fixed */
  if (ntohs(iph->tot_len) == 20+tcplen) {
    if(modbusinfo->allow_tcp == 1)
      return 1;
    else
      return 0;
  }
  
  else
    {
  
  /* Return the "OR"s of all the parameters given.  If any
     of the given parameters is true, the whole thing is true */ 
      
      //printk(KERN_DEBUG "Modbus Filter==funccode match:%d-unitid match:%d-refnum match:%d--length match:%d:result: %d\n",func_code_check(modbusinfo->funccode_flags,(modbus->modbus_d).func_code, modbusinfo->func_code[0],modbusinfo->func_code[1], modbusinfo->invflags_funccode), unitid_check(modbusinfo->unitid_flags,(modbus->modbus_d).unit_id, modbusinfo->unit_id, modbusinfo->invflags_unitid), refnum_check(modbusinfo->refnum_flags,(modbus->modbus_d).ref_num, modbusinfo->ref_num, modbusinfo->invflags_refnum), length_check(modbusinfo->length_flags,(modbus->modbus_h).length, modbusinfo->length[0],modbusinfo->length[1], modbusinfo->invflags_length), func_code_check(modbusinfo->funccode_flags,(modbus->modbus_d).func_code, modbusinfo->func_code[0],modbusinfo->func_code[1], modbusinfo->invflags_funccode) || unitid_check(modbusinfo->unitid_flags,(modbus->modbus_d).unit_id, modbusinfo->unit_id,modbusinfo->invflags_unitid) || refnum_check(modbusinfo->refnum_flags,(modbus->modbus_d).ref_num, modbusinfo->ref_num,modbusinfo->invflags_refnum) || length_check(modbusinfo->length_flags,(modbus->modbus_h).length, modbusinfo->length[0],modbusinfo->length[1], modbusinfo->invflags_length));
      
      
      return (func_code_check(modbusinfo->funccode_flags,(modbus->modbus_d).func_code, modbusinfo->func_code[0],modbusinfo->func_code[1], modbusinfo->invflags_funccode) || unitid_check(modbusinfo->unitid_flags,(modbus->modbus_d).unit_id, modbusinfo->unit_id,modbusinfo->invflags_unitid) || refnum_check(modbusinfo->refnum_flags,(modbus->modbus_d).ref_num, modbusinfo->ref_num,modbusinfo->invflags_refnum) || length_check(modbusinfo->length_flags,(modbus->modbus_h).length, modbusinfo->length[0],modbusinfo->length[1], modbusinfo->invflags_length));
      
    }
}

/* Part of standard netfilter code */
static int
checkentry(const char *tablename,
	   const struct ipt_ip *ip,
	   void *matchinfo,
	   unsigned int matchinfosize,
	   unsigned int hook_mask)
{
  const struct ipt_modbus *modbusinfo = matchinfo;
  
  if (ip->proto != 6){
    printk(KERN_DEBUG "Not a TCP packet\n");
    return 0;
  }
  
  if (matchinfosize != IPT_ALIGN(sizeof(struct ipt_modbus))) {
    return 0;
  }
  
  return 1;
}

static struct ipt_match modbus_match
= { { NULL, NULL }, "modbus", &match, &checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
  return ipt_register_match(&modbus_match);
}

static void __exit cleanup(void)
{
  ipt_unregister_match(&modbus_match);
}

module_init(init);
module_exit(cleanup);
