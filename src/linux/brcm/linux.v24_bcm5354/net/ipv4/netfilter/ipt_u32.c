/* Kernel module to match u32 packet content. */

/* 
U32 tests whether quantities of up to 4 bytes extracted from a packet 
have specified values.  The specification of what to extract is general 
enough to find data at given offsets from tcp headers or payloads.

 --u32 tests
 The argument amounts to a program in a small language described below.
 tests := location = value |  tests && location = value
 value := range | value , range
 range := number | number : number
  a single number, n, is interpreted the same as n:n
  n:m is interpreted as the range of numbers >=n and <=m
 location := number | location operator number
 operator := & | << | >> | @

 The operators &, <<, >>, && mean the same as in c.  The = is really a set
 membership operator and the value syntax describes a set.  The @ operator
 is what allows moving to the next header and is described further below.

 *** Until I can find out how to avoid it, there are some artificial limits
 on the size of the tests:
 - no more than 10 ='s (and 9 &&'s) in the u32 argument
 - no more than 10 ranges (and 9 commas) per value
 - no more than 10 numbers (and 9 operators) per location

 To describe the meaning of location, imagine the following machine that
 interprets it.  There are three registers:
  A is of type char*, initially the address of the IP header
  B and C are unsigned 32 bit integers, initially zero

  The instructions are:
   number	B = number;
   		C = (*(A+B)<<24)+(*(A+B+1)<<16)+(*(A+B+2)<<8)+*(A+B+3)
   &number	C = C&number
   <<number	C = C<<number
   >>number	C = C>>number
   @number	A = A+C; then do the instruction number
  Any access of memory outside [skb->head,skb->end] causes the match to fail.
  Otherwise the result of the computation is the final value of C.

 Whitespace is allowed but not required in the tests.
 However the characters that do occur there are likely to require
 shell quoting, so it's a good idea to enclose the arguments in quotes.

Example:
 match IP packets with total length >= 256
 The IP header contains a total length field in bytes 2-3.
 --u32 "0&0xFFFF=0x100:0xFFFF" 
 read bytes 0-3
 AND that with FFFF (giving bytes 2-3),
 and test whether that's in the range [0x100:0xFFFF]

Example: (more realistic, hence more complicated)
 match icmp packets with icmp type 0
 First test that it's an icmp packet, true iff byte 9 (protocol) = 1
 --u32 "6&0xFF=1 && ...
 read bytes 6-9, use & to throw away bytes 6-8 and compare the result to 1
 Next test that it's not a fragment.
  (If so it might be part of such a packet but we can't always tell.)
  n.b. This test is generally needed if you want to match anything
  beyond the IP header.
 The last 6 bits of byte 6 and all of byte 7 are 0 iff this is a complete
 packet (not a fragment).  Alternatively, you can allow first fragments
 by only testing the last 5 bits of byte 6.
 ... 4&0x3FFF=0 && ...
 Last test: the first byte past the IP header (the type) is 0
 This is where we have to use the @syntax.  The length of the IP header
 (IHL) in 32 bit words is stored in the right half of byte 0 of the
 IP header itself.
 ... 0>>22&0x3C@0>>24=0"
 The first 0 means read bytes 0-3,
 >>22 means shift that 22 bits to the right.  Shifting 24 bits would give
   the first byte, so only 22 bits is four times that plus a few more bits.
 &3C then eliminates the two extra bits on the right and the first four 
 bits of the first byte.
 For instance, if IHL=5 then the IP header is 20 (4 x 5) bytes long.
 In this case bytes 0-1 are (in binary) xxxx0101 yyzzzzzz, 
 >>22 gives the 10 bit value xxxx0101yy and &3C gives 010100.
 @ means to use this number as a new offset into the packet, and read
 four bytes starting from there.  This is the first 4 bytes of the icmp
 payload, of which byte 0 is the icmp type.  Therefore we simply shift
 the value 24 to the right to throw out all but the first byte and compare
 the result with 0.

Example: 
 tcp payload bytes 8-12 is any of 1, 2, 5 or 8
 First we test that the packet is a tcp packet (similar to icmp).
 --u32 "6&0xFF=6 && ...
 Next, test that it's not a fragment (same as above).
 ... 0>>22&0x3C@12>>26&0x3C@8=1,2,5,8"
 0>>22&3C as above computes the number of bytes in the IP header.
 @ makes this the new offset into the packet, which is the start of the
 tcp header.  The length of the tcp header (again in 32 bit words) is
 the left half of byte 12 of the tcp header.  The 12>>26&3C
 computes this length in bytes (similar to the IP header before).
 @ makes this the new offset, which is the start of the tcp payload.
 Finally 8 reads bytes 8-12 of the payload and = checks whether the
 result is any of 1, 2, 5 or 8
*/

#include <linux/module.h>
#include <linux/skbuff.h>

#include <linux/netfilter_ipv4/ipt_u32.h>
#include <linux/netfilter_ipv4/ip_tables.h>

/* #include <asm-i386/timex.h> for timing */

MODULE_AUTHOR("Don Cohen <don@isis.cs3-inc.com>");
MODULE_DESCRIPTION("IP tables u32 matching module");
MODULE_LICENSE("GPL");

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
	const struct ipt_u32 *data = matchinfo;
	int testind, i;
	unsigned char* origbase = (char*)skb->nh.iph;
	unsigned char* base = origbase;
	unsigned char* head = skb->head;
	unsigned char* end = skb->end;
	int nnums, nvals;
	u_int32_t pos, val;
	/* unsigned long long cycles1, cycles2, cycles3, cycles4;
	   cycles1 = get_cycles(); */

	for (testind=0; testind < data->ntests; testind++) {
		base = origbase; /* reset for each test */
		pos = data->tests[testind].location[0].number;
		if (base+pos+3 > end || base+pos < head) 
			return 0;
		val = (base[pos]<<24) + (base[pos+1]<<16) +
			(base[pos+2]<<8) + base[pos+3];
		nnums = data->tests[testind].nnums;
		for (i=1; i < nnums; i++) {
			u_int32_t number = data->tests[testind].location[i].number;
			switch (data->tests[testind].location[i].nextop) {
			case IPT_U32_AND: 
				val = val & number; 
				break;
			case IPT_U32_LEFTSH: 
				val = val << number;
				break;
			case IPT_U32_RIGHTSH: 
				val = val >> number; 
				break;
			case IPT_U32_AT:
				base = base + val;
				pos = number;
				if (base+pos+3 > end || base+pos < head) 
					return 0;
				val = (base[pos]<<24) + (base[pos+1]<<16) +
					(base[pos+2]<<8) + base[pos+3];
				break;
			}
		}
		nvals = data->tests[testind].nvalues;
		for (i=0; i < nvals; i++) {
			if ((data->tests[testind].value[i].min <= val) &&
			    (val <= data->tests[testind].value[i].max))	{
				break;
			}
		}
		if (i >= data->tests[testind].nvalues) {
			/* cycles2 = get_cycles(); 
			   printk("failed %d in %d cycles\n", testind, 
				  cycles2-cycles1); */
			return 0;
		}
	}
	/* cycles2 = get_cycles();
	   printk("succeeded in %d cycles\n", cycles2-cycles1); */
	return 1;
}

static int
checkentry(const char *tablename,
           const struct ipt_ip *ip,
           void *matchinfo,
           unsigned int matchsize,
           unsigned int hook_mask)
{
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_u32)))
		return 0;
	return 1;
}

static struct ipt_match u32_match
= { { NULL, NULL }, "u32", &match, &checkentry, NULL, THIS_MODULE };

static int __init init(void)
{
	return ipt_register_match(&u32_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&u32_match);
}

module_init(init);
module_exit(fini);
