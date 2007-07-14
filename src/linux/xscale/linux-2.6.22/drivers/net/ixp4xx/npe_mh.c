/*
 * npe_mh.c - NPE message handler.
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#include <linux/ixp_npe.h>
#include <linux/slab.h>

#define MAX_RETRY 200

struct npe_mh_msg {
	union {
		u8 byte[8]; /* Very desciptive name, I know ... */
		u32 data[2];
	} u;
};

/*
 * The whole code in this function must be reworked.
 * It is in a state that works but is not rock solid
 */
static int send_message(struct npe_info *npe, struct npe_mh_msg *msg)
{
	int i,j;
	u32 send[2], recv[2];

	for (i=0; i<2; i++)
		send[i] = be32_to_cpu(msg->u.data[i]);

	if ((npe_reg_read(npe, IX_NPEDL_REG_OFFSET_STAT) &
				IX_NPEMH_NPE_STAT_IFNE))
		return -1;

	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_FIFO, send[0]);
	for(i=0; i<MAX_RETRY; i++) {
		/* if the IFNF status bit is unset then the inFIFO is full */
		if (npe_reg_read(npe, IX_NPEDL_REG_OFFSET_STAT) &
				IX_NPEMH_NPE_STAT_IFNF)
			break;
	}
	if (i>=MAX_RETRY)
		return -1;
	npe_reg_write(npe, IX_NPEDL_REG_OFFSET_FIFO, send[1]);
	i=0;
	while (!(npe_reg_read(npe, IX_NPEDL_REG_OFFSET_STAT) &
					IX_NPEMH_NPE_STAT_OFNE)) {
		if (i++>MAX_RETRY) {
			printk("Waiting for Output FIFO NotEmpty failed\n");
			return -1;
		}
	}
	//printk("Output FIFO Not Empty. Loops: %d\n", i);
	j=0;
	while (npe_reg_read(npe, IX_NPEDL_REG_OFFSET_STAT) &
					IX_NPEMH_NPE_STAT_OFNE) {
		recv[j&1] = npe_reg_read(npe,IX_NPEDL_REG_OFFSET_FIFO);
		j++;
	}
	if ((recv[0] != send[0]) || (recv[1] != send[1])) {
		if (send[0] || send[1]) {
			/* all CMDs return the complete message as answer,
			 * only GETSTATUS returns the ImageID of the NPE
			 */
			printk("Unexpected answer: "
				"Send %08x:%08x Ret %08x:%08x\n",
				send[0], send[1], recv[0], recv[1]);
		}
	}
	return 0;
}

#define CMD  0
#define PORT 1
#define MAC  2

#define IX_ETHNPE_NPE_GETSTATUS			0x00
#define IX_ETHNPE_EDB_SETPORTADDRESS            0x01
#define IX_ETHNPE_GETSTATS			0x04
#define IX_ETHNPE_RESETSTATS			0x05
#define IX_ETHNPE_FW_SETFIREWALLMODE            0x0E
#define IX_ETHNPE_VLAN_SETRXQOSENTRY            0x0B
#define IX_ETHNPE_SETLOOPBACK_MODE		0x12

#define logical_id(mp) (((mp)->npe_id << 4) | ((mp)->port_id & 0xf))

int npe_mh_status(struct npe_info *npe)
{
	struct npe_mh_msg msg;

	memset(&msg, 0, sizeof(msg));
	msg.u.byte[CMD] = IX_ETHNPE_NPE_GETSTATUS;
	return send_message(npe, &msg);
}

int npe_mh_setportaddr(struct npe_info *npe, struct mac_plat_info *mp,
		u8 *macaddr)
{
	struct npe_mh_msg msg;

	msg.u.byte[CMD] = IX_ETHNPE_EDB_SETPORTADDRESS;
	msg.u.byte[PORT] = mp->eth_id;
	memcpy(msg.u.byte + MAC, macaddr, 6);

	return send_message(npe, &msg);
}

int npe_mh_disable_firewall(struct npe_info *npe, struct mac_plat_info *mp)
{
	struct npe_mh_msg msg;

	memset(&msg, 0, sizeof(msg));
	msg.u.byte[CMD] = IX_ETHNPE_FW_SETFIREWALLMODE;
	msg.u.byte[PORT] = logical_id(mp);

	return send_message(npe, &msg);
}

int npe_mh_npe_loopback_mode(struct npe_info *npe, struct mac_plat_info *mp,
		int enable)
{
	struct npe_mh_msg msg;

	memset(&msg, 0, sizeof(msg));
	msg.u.byte[CMD] = IX_ETHNPE_SETLOOPBACK_MODE;
	msg.u.byte[PORT] = logical_id(mp);
	msg.u.byte[3] = enable ? 1 : 0;

	return send_message(npe, &msg);
}

int npe_mh_set_rxqid(struct npe_info *npe, struct mac_plat_info *mp, int qid)
{
	struct npe_mh_msg msg;
	int i, ret;

	memset(&msg, 0, sizeof(msg));
	msg.u.byte[CMD] = IX_ETHNPE_VLAN_SETRXQOSENTRY;
	msg.u.byte[PORT] = logical_id(mp);
	msg.u.byte[5] = qid | 0x80;
	msg.u.byte[7] = qid<<4;
	for(i=0; i<8; i++) {
		msg.u.byte[3] = i;
		if ((ret = send_message(npe, &msg)))
			return ret;
	}
	return 0;
}

int npe_mh_get_stats(struct npe_info *npe, struct mac_plat_info *mp, u32 phys,
		int reset)
{
	struct npe_mh_msg msg;
	memset(&msg, 0, sizeof(msg));
	msg.u.byte[CMD] = reset ? IX_ETHNPE_RESETSTATS : IX_ETHNPE_GETSTATS;
	msg.u.byte[PORT] = logical_id(mp);
	msg.u.data[1] = cpu_to_npe32(cpu_to_be32(phys));

	return send_message(npe, &msg);
}


EXPORT_SYMBOL(npe_mh_status);
EXPORT_SYMBOL(npe_mh_setportaddr);
EXPORT_SYMBOL(npe_mh_disable_firewall);
EXPORT_SYMBOL(npe_mh_set_rxqid);
EXPORT_SYMBOL(npe_mh_npe_loopback_mode);
EXPORT_SYMBOL(npe_mh_get_stats);
