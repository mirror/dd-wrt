/* SCTP kernel reference Implementation
 *
 * Copyright (c) 2003 Hewlett-Packard Company
 * 
 * This file is part of the SCTP kernel reference Implementation
 * 
 * This header represents the structures and constants needed to backport
 * lksctp from Linux kernel 2.5 to 2.4 This file also has some code that
 * has been taken from the source base of Linux kernel 2.5 
 * 
 * The SCTP reference implementation is free software; 
 * you can redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * The SCTP reference implementation is distributed in the hope that it 
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 ************************
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU CC; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.  
 * 
 */

#ifndef __net_sctp_compat_h__
#define __net_sctp_compat_h__

#include <linux/version.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/seq_file.h>

/*
 * The following defines are for compatibility with 2.5
 */
/*
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define DEFINE_SNMP_STAT(type, name)	\
	type name[NR_CPUS * 2]
#define DECLARE_SNMP_STAT(type, name)	\
	extern type name[]
#define SNMP_DEC_STATS(mib, field) ((mib)[2*smp_processor_id()+!in_softirq()].field--)

#define sctp_sk(__sk) (&(((struct sock *)__sk)->tp_pinfo.af_sctp))
#define inet_sk(__sk) (&(((struct sock *)__sk)->protinfo.af_inet))
#define inet6_sk(__sk) (&(((struct sock *)__sk)->net_pinfo.af_inet6))

#define virt_addr_valid(x)	VALID_PAGE(virt_to_page((x)))
#define sock_owned_by_user(sk)  ((sk)->lock.users)
#define sk_set_owner(x, y)
#define __unsafe(x) MOD_INC_USE_COUNT
#define dst_pmtu(x) ((x)->pmtu)

void sctp_hash_digest(const char *key, const int in_key_len,
		      const char *text, const int text_len,
		      __u8 *digest);
/*
 * find last bit set.
 */
static __inline__ int fls(int x)
{
	int r = 32;
	
	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

#endif /* __net_sctp_compat_h__ */
