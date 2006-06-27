#ifndef _IP_POOL_H
#define _IP_POOL_H

/***************************************************************************/
/*  This program is free software; you can redistribute it and/or modify   */
/*  it under the terms of the GNU General Public License as published by   */
/*  the Free Software Foundation; either version 2 of the License, or	   */
/*  (at your option) any later version.					   */
/*									   */
/*  This program is distributed in the hope that it will be useful,	   */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of	   */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	   */
/*  GNU General Public License for more details.			   */
/*									   */
/*  You should have received a copy of the GNU General Public License	   */
/*  along with this program; if not, write to the Free Software	       	   */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA*/
/***************************************************************************/

/* A sockopt of such quality has hardly ever been seen before on the open
 * market!  This little beauty, hardly ever used: above 64, so it's
 * traditionally used for firewalling, not touched (even once!) by the
 * 2.0, 2.2 and 2.4 kernels!
 *
 * Comes with its own certificate of authenticity, valid anywhere in the
 * Free world!
 *
 * Rusty, 19.4.2000
 */
#define SO_IP_POOL 81

typedef int ip_pool_t;			/* pool index */
#define IP_POOL_NONE	((ip_pool_t)-1)

struct ip_pool_request {
	int op;
	ip_pool_t index;
	u_int32_t addr;
	u_int32_t addr2;
};

/* NOTE: I deliberately break the first cut ippool utility. Nobody uses it. */

#define IP_POOL_BAD001		0x00000010

#define IP_POOL_FLUSH		0x00000011	/* req.index, no arguments */
#define IP_POOL_INIT		0x00000012	/* from addr to addr2 incl. */
#define IP_POOL_DESTROY		0x00000013	/* req.index, no arguments */
#define IP_POOL_ADD_ADDR	0x00000014	/* add addr to pool */
#define IP_POOL_DEL_ADDR	0x00000015	/* del addr from pool */
#define IP_POOL_HIGH_NR		0x00000016	/* result in req.index */
#define IP_POOL_LOOKUP		0x00000017	/* result in addr and addr2 */
#define IP_POOL_USAGE		0x00000018	/* result in addr */
#define IP_POOL_TEST_ADDR	0x00000019	/* result (0/1) returned */

#ifdef __KERNEL__

/* NOTE: ip_pool_match() and ip_pool_mod() expect ADDR to be host byte order */
extern int ip_pool_match(ip_pool_t pool, u_int32_t addr);
extern int ip_pool_mod(ip_pool_t pool, u_int32_t addr, int isdel);

#endif

#endif /*_IP_POOL_H*/
