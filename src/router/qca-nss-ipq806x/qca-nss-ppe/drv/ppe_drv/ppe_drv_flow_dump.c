/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/debugfs.h>
#include <linux/version.h>
#include "ppe_drv.h"

typedef uint32_t ip_addr_t[4];

/*
 * Debugfs dentry object.
 */
static struct dentry *ppe_drv_flow_dump_dentry;

/*
 * Character device stuff - used to communicate status back to user space
 */
static int ppe_drv_flow_dump_dev_major_id = 0;			/* Major ID of registered char dev from which we can dump out state to userspace */


/*
 * ppe_drv_flow_dump_write_reset()
 *	Reset the msg buffer, specifying a new initial prefix
 *
 * Returns 0 on success
 */
int ppe_drv_flow_dump_write_reset(struct ppe_drv_flow_dump_instance *fdi, char *prefix)
{
	int result;

	fdi->msgp = fdi->msg;
	fdi->msg_len = 0;

	result = snprintf(fdi->prefix, PPE_DRV_FLOW_DUMP_FILE_PREFIX_SIZE, "%s", prefix);
	if ((result < 0) || (result >= PPE_DRV_FLOW_DUMP_FILE_PREFIX_SIZE)) {
		return -1;
	}

	fdi->prefix_level = 0;
	fdi->prefix_levels[fdi->prefix_level] = result;

	return 0;
}

/*
 * ppe_drv_flow_dump_prefix_add()
 *	Add another level to the prefix
 *
 * Returns 0 on success
 */
int ppe_drv_flow_dump_prefix_add(struct ppe_drv_flow_dump_instance *fdi, char *prefix)
{
	int pxsz;
	int pxremain;
	int result;

	pxsz = fdi->prefix_levels[fdi->prefix_level];
	pxremain = PPE_DRV_FLOW_DUMP_FILE_PREFIX_SIZE - pxsz;

	result = snprintf(fdi->prefix + pxsz, pxremain, ".%s", prefix);
	if ((result < 0) || (result >= pxremain)) {
		return -1;
	}

	fdi->prefix_level++;
	ppe_drv_assert(fdi->prefix_level < PPE_IF_MAP_FILE_PREFIX_LEVELS_MAX, "Bad prefix handling\n");
	fdi->prefix_levels[fdi->prefix_level] = pxsz + result;

	return 0;
}

/*
 * ppe_drv_flow_dump_prefix_index_add()
 *	Add another level (numeric) to the prefix
 *
 * Returns 0 on success
 */
int ppe_drv_flow_dump_prefix_index_add(struct ppe_drv_flow_dump_instance *fdi, uint32_t index)
{
	int pxsz;
	int pxremain;
	int result;

	pxsz = fdi->prefix_levels[fdi->prefix_level];
	pxremain = PPE_DRV_FLOW_DUMP_FILE_PREFIX_SIZE - pxsz;
	result = snprintf(fdi->prefix + pxsz, pxremain, ".%u", index);
	if ((result < 0) || (result >= pxremain)) {
		return -1;
	}

	fdi->prefix_level++;
	fdi->prefix_levels[fdi->prefix_level] = pxsz + result;

	return 0;
}

/*
 * ppe_drv_flow_dump_prefix_remove()
 *	Remove level from the prefix
 *
 * Returns 0 on success
 */
int ppe_drv_flow_dump_prefix_remove(struct ppe_drv_flow_dump_instance *fdi)
{
	int pxsz;

	fdi->prefix_level--;
	ppe_drv_assert(fdi->prefix_level >= 0, "Bad prefix handling\n");
	pxsz = fdi->prefix_levels[fdi->prefix_level];
	fdi->prefix[pxsz] = 0;

	return 0;
}

/*
 * ppe_drv_flow_dump_write()
 *	Write out to the message buffer, prefix is added automatically.
 *
 * Returns 0 on success
 */
int ppe_drv_flow_dump_write(struct ppe_drv_flow_dump_instance *fdi, char *name, char *fmt, ...)
{
	int remain;
	char *ptr;
	int result;
	va_list args;

	remain = PPE_DRV_FLOW_DUMP_FILE_BUFFER_SIZE - fdi->msg_len;
	ptr = fdi->msg + fdi->msg_len;
	result = snprintf(ptr, remain, "%s.%s=", fdi->prefix, name);
	if ((result < 0) || (result >= remain)) {
		return -1;
	}

	fdi->msg_len += result;
	remain -= result;
	ptr += result;

	va_start(args, fmt);
	result = vsnprintf(ptr, remain, fmt, args);
	va_end(args);
	if ((result < 0) || (result >= remain)) {
		return -2;
	}

	fdi->msg_len += result;
	remain -= result;
	ptr += result;

	result = snprintf(ptr, remain, "\n");
	if ((result < 0) || (result >= remain)) {
		return -3;
	}

	fdi->msg_len += result;
	return 0;
}

/*
 * ppe_flow_dump_v6_get()
 *	Prepare a connection message for ipv6
 */
int ppe_flow_dump_v6_get(struct ppe_drv_flow_dump_instance *fdi, struct list_head *conn_head)
{
	int result;

	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn *cn;
	struct ppe_drv_v6_conn_flow *pcf;
	struct ppe_drv_v6_conn_flow *pcr;

	spin_lock_bh(&p->lock);

	if (!list_empty(conn_head)) {
		list_for_each_entry(cn, conn_head, list) {
			pcf = &cn->pcf;

			/*
			 * Extract information from the connection for inclusion into the message
			 */
			if ((result = ppe_drv_flow_dump_prefix_index_add(fdi, fdi->flow_cnt))) {
				goto ppe_drv_flow_dump_write_error;
			}

			/*
			 * Flow direction Information
			 */
			if ((result = ppe_drv_flow_dump_prefix_add(fdi, "flow"))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "sip_address", "%pI6", &pcf->dump_match_src_ip[4]))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "sport", "%u", pcf->match_src_ident))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "dip_address", "%pI6", &pcf->dump_match_dest_ip[4]))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "dport", "%u", pcf->match_dest_ident))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "ppe_tx_port", "%u", pcf->tx_port->port))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "ppe_rx_port", "%u", pcf->rx_port->port))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if (pcf->in_port_if) {

				if ((pcf->in_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_iface_index", "%u", pcf->in_port_if->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf->in_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_l3_if_index", "%u", pcf->in_port_if->l3->l3_if_index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf->in_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_vsi_index", "%u", pcf->in_port_if->vsi->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if (pcf->in_port_if->dev) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_netdev_name", "%s", pcf->in_port_if->dev->name))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}
			}

			if (pcf->eg_port_if) {

				if ((pcf->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_iface_index", "%u", pcf->eg_port_if->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf->eg_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_l3_if_index", "%u", pcf->eg_port_if->l3->l3_if_index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_vsi_index", "%u", pcf->eg_port_if->vsi->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if (pcf->eg_port_if->dev) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_netdev_name", "%s", pcf->eg_port_if->dev->name))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}
			}

			if (pcf->pf) {

				if ((result = ppe_drv_flow_dump_write(fdi, "hw_flow_index", "%u", pcf->pf->index))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if (pcf->pf->nh) {
					if ((result = ppe_drv_flow_dump_write(fdi, "nexthop_index", "%u", pcf->pf->nh->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if (pcf->pf->host) {
					if ((result = ppe_drv_flow_dump_write(fdi, "host_index", "%u", pcf->pf->host->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "service_code", "%u", pcf->pf->service_code))) {
					goto ppe_drv_flow_dump_write_error;
				}
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "tx_packets", "%u", pcf->tx_packets))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "tx_bytes", "%u", pcf->tx_bytes))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "rx_packets", "%u", pcf->rx_packets))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "rx_bytes", "%u", pcf->rx_bytes))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_prefix_remove(fdi))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "protocol", "%u", pcf->match_protocol))) {
				goto ppe_drv_flow_dump_write_error;
			}

			/*
			 * Return flow direction Information
			 */
			pcr = &cn->pcr;

			if (pcr) {

				if ((result = ppe_drv_flow_dump_prefix_add(fdi, "return"))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "dump_sip_address", "%pI6", &pcr->dump_match_src_ip[4]))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "sport", "%u", pcr->match_src_ident))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "dip_address", "%pI6", &pcr->dump_match_dest_ip[4]))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "dport", "%u", pcr->match_dest_ident))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "ppe_tx_port", "%u", pcr->tx_port->port))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "ppe_rx_port", "%u", pcr->rx_port->port))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if (pcr->in_port_if) {

					if ((pcr->in_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "in_iface_index", "%u", pcr->in_port_if->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((pcr->in_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "in_l3_if_index", "%u", pcr->in_port_if->l3->l3_if_index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((pcr->in_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "in_vsi_index", "%u", pcr->in_port_if->vsi->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if (pcr->in_port_if->dev) {
						if ((result = ppe_drv_flow_dump_write(fdi, "in_netdev_name", "%s", pcr->in_port_if->dev->name))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}
				}

				if (pcr->eg_port_if) {

					if ((pcr->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_iface_index", "%u", pcr->eg_port_if->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((pcr->eg_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_l3_if_index", "%u", pcr->eg_port_if->l3->l3_if_index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((pcr->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_vsi_index", "%u", pcr->eg_port_if->vsi->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if (pcr->eg_port_if->dev) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_netdev_name", "%s", pcr->eg_port_if->dev->name))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}
				}

				if (pcr->pf) {

					if ((result = ppe_drv_flow_dump_write(fdi, "hw_flow_index", "%u", pcr->pf->index))) {
						goto ppe_drv_flow_dump_write_error;
					}

					if (pcr->pf->nh) {
						if ((result = ppe_drv_flow_dump_write(fdi, "nexthop_index", "%u", pcr->pf->nh->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if (pcr->pf->host) {
						if ((result = ppe_drv_flow_dump_write(fdi, "host_index", "%u", pcr->pf->host->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((result = ppe_drv_flow_dump_write(fdi, "service_code", "%u", pcr->pf->service_code))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "tx_packets", "%u", pcr->tx_packets))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "tx_bytes", "%u", pcr->tx_bytes))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "rx_packets", "%u", pcr->rx_packets))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "rx_bytes", "%u\n", pcr->rx_bytes))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_prefix_remove(fdi))) {
					goto ppe_drv_flow_dump_write_error;
				}
			}

			if ((result = ppe_drv_flow_dump_prefix_remove(fdi))) {
				goto ppe_drv_flow_dump_write_error;
			}

			fdi->flow_cnt++;
		}
	}

	spin_unlock_bh(&p->lock);

	return ppe_drv_flow_dump_prefix_remove(fdi);

ppe_drv_flow_dump_write_error :
	spin_unlock_bh(&p->lock);
	return result;
}

/*
 * ppe_flow_dump_v4_get()
 *	Prepare a connection message for ipv4
 */
int ppe_flow_dump_v4_get(struct ppe_drv_flow_dump_instance *fdi, struct list_head *conn_head)
{
	int result;

	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v4_conn *cn;
	struct ppe_drv_v4_conn_flow *pcf_v4;
	struct ppe_drv_v4_conn_flow *pcr_v4;

	spin_lock_bh(&p->lock);

	if (!list_empty(conn_head)) {
		list_for_each_entry(cn, conn_head, list) {
			pcf_v4 = &cn->pcf;

			/*
			 * Extract information from the connection for inclusion into the message
			 */
			if ((result = ppe_drv_flow_dump_prefix_index_add(fdi, fdi->flow_cnt))) {
				goto ppe_drv_flow_dump_write_error;
			}

			/*
			 * Flow direction Information
			 */
			if ((result = ppe_drv_flow_dump_prefix_add(fdi, "flow"))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "sip_address", "%pI4", &pcf_v4->dump_match_src_ip))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "sip_address_nat", "%pI4", &pcf_v4->dump_xlate_src_ip))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "sport", "%u", pcf_v4->match_src_ident))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "sport_nat", "%u", pcf_v4->xlate_src_ident))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "dip_address", "%pI4", &pcf_v4->dump_match_dest_ip))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "dip_address_nat", "%pI4", &pcf_v4->dump_xlate_dest_ip))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "dport", "%u", pcf_v4->match_dest_ident))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "dport_nat", "%u", pcf_v4->xlate_dest_ident))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "ppe_tx_port", "%u", pcf_v4->tx_port->port))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "ppe_rx_port", "%u", pcf_v4->rx_port->port))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if (pcf_v4->in_port_if) {

				if ((pcf_v4->in_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_iface_index", "%u", pcf_v4->in_port_if->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf_v4->in_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_l3_if_index", "%u", pcf_v4->in_port_if->l3->l3_if_index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf_v4->in_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_vsi_index", "%u", pcf_v4->in_port_if->vsi->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if (pcf_v4->in_port_if->dev) {
					if ((result = ppe_drv_flow_dump_write(fdi, "in_netdev_name", "%s", pcf_v4->in_port_if->dev->name))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}
			}

			if (pcf_v4->eg_port_if) {

				if ((pcf_v4->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_iface_index", "%u", pcf_v4->eg_port_if->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf_v4->eg_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_l3_if_index", "%u", pcf_v4->eg_port_if->l3->l3_if_index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((pcf_v4->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_vsi_index", "%u", pcf_v4->eg_port_if->vsi->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if (pcf_v4->eg_port_if->dev) {
					if ((result = ppe_drv_flow_dump_write(fdi, "eg_netdev_name", "%s", pcf_v4->eg_port_if->dev->name))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}
			}

			if (pcf_v4->pf) {

				if ((result = ppe_drv_flow_dump_write(fdi, "hw_flow_index", "%u", pcf_v4->pf->index))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if (pcf_v4->pf->nh) {
					if ((result = ppe_drv_flow_dump_write(fdi, "nexthop_index", "%u", pcf_v4->pf->nh->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if (pcf_v4->pf->host) {
					if ((result = ppe_drv_flow_dump_write(fdi, "host_index", "%u", pcf_v4->pf->host->index))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "service_code", "%u", pcf_v4->pf->service_code))) {
					goto ppe_drv_flow_dump_write_error;
				}
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "tx_packets", "%u", pcf_v4->tx_packets))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "tx_bytes", "%u", pcf_v4->tx_bytes))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "rx_packets", "%u", pcf_v4->rx_packets))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "rx_bytes", "%u", pcf_v4->rx_bytes))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_prefix_remove(fdi))) {
				goto ppe_drv_flow_dump_write_error;
			}

			if ((result = ppe_drv_flow_dump_write(fdi, "protocol", "%u", pcf_v4->match_protocol))) {
				goto ppe_drv_flow_dump_write_error;
			}

			/*
			 * Return flow direction Information
			 */
			pcr_v4 = &cn->pcr;

			if (pcr_v4) {

				if ((result = ppe_drv_flow_dump_prefix_add(fdi, "return"))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "sip_address", "%pI4", &pcr_v4->dump_match_src_ip))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "sip_address_nat", "%pI4", &pcr_v4->dump_xlate_src_ip))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "sport", "%u", pcr_v4->match_src_ident))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "sport_nat", "%u", pcr_v4->xlate_src_ident))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "dip_address", "%pI4", &pcr_v4->dump_match_dest_ip))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "dip_address_nat", "%pI4", &pcr_v4->dump_xlate_dest_ip))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "dport", "%u", pcr_v4->match_dest_ident))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "dport_nat", "%u", pcr_v4->xlate_dest_ident))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "ppe_tx_port", "%u", pcr_v4->tx_port->port))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "ppe_rx_port", "%u", pcr_v4->rx_port->port))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if (pcr_v4->in_port_if) {

					if ((pcr_v4->in_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "in_iface_index", "%u", pcr_v4->in_port_if->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((pcr_v4->in_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
						if (pcr_v4->in_port_if->l3) {
							if ((result = ppe_drv_flow_dump_write(fdi, "in_l3_if_index", "%u", pcr_v4->in_port_if->l3->l3_if_index))) {
								goto ppe_drv_flow_dump_write_error;
							}
						}
					}

					if ((pcr_v4->in_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
						if (pcr_v4->in_port_if->vsi) {
							if ((result = ppe_drv_flow_dump_write(fdi, "in_vsi_index", "%u", pcr_v4->in_port_if->vsi->index))) {
								goto ppe_drv_flow_dump_write_error;
							}
						}
					}

					if (pcr_v4->in_port_if->dev) {
						if ((result = ppe_drv_flow_dump_write(fdi, "in_netdev_name", "%s", pcr_v4->in_port_if->dev->name))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}
				}

				if (pcr_v4->eg_port_if) {

					if ((pcr_v4->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_iface_index", "%u", pcr_v4->eg_port_if->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((pcr_v4->eg_port_if->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) == PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_l3_if_index", "%u", pcr_v4->eg_port_if->l3->l3_if_index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((pcr_v4->eg_port_if->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) == PPE_DRV_IFACE_FLAG_VSI_VALID) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_vsi_index", "%u", pcr_v4->eg_port_if->vsi->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if (pcr_v4->eg_port_if->dev) {
						if ((result = ppe_drv_flow_dump_write(fdi, "eg_netdev_name", "%s", pcr_v4->eg_port_if->dev->name))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}
				}

				if (pcr_v4->pf) {

					if ((result = ppe_drv_flow_dump_write(fdi, "hw_flow_index", "%u", pcr_v4->pf->index))) {
						goto ppe_drv_flow_dump_write_error;
					}

					if (pcr_v4->pf->nh) {
						if ((result = ppe_drv_flow_dump_write(fdi, "nexthop_index", "%u", pcr_v4->pf->nh->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if (pcr_v4->pf->host) {
						if ((result = ppe_drv_flow_dump_write(fdi, "host_index", "%u", pcr_v4->pf->host->index))) {
							goto ppe_drv_flow_dump_write_error;
						}
					}

					if ((result = ppe_drv_flow_dump_write(fdi, "service_code", "%u", pcr_v4->pf->service_code))) {
						goto ppe_drv_flow_dump_write_error;
					}
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "tx_packets", "%u", pcr_v4->tx_packets))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "tx_bytes", "%u", pcr_v4->tx_bytes))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "rx_packets", "%u", pcr_v4->rx_packets))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_write(fdi, "rx_bytes", "%u\n", pcr_v4->rx_bytes))) {
					goto ppe_drv_flow_dump_write_error;
				}

				if ((result = ppe_drv_flow_dump_prefix_remove(fdi))) {
					goto ppe_drv_flow_dump_write_error;
				}
			}

			if ((result = ppe_drv_flow_dump_prefix_remove(fdi))) {
				goto ppe_drv_flow_dump_write_error;
			}

			fdi->flow_cnt++;
		}
	}

	spin_unlock_bh(&p->lock);

	return ppe_drv_flow_dump_prefix_remove(fdi);

ppe_drv_flow_dump_write_error :
	spin_unlock_bh(&p->lock);
	return result;
}

/*
 * ppe_drv_flow_dump_v4_get()
 *	Prepare a connection message ipv4
 */
static bool ppe_drv_flow_dump_v4_get(struct ppe_drv_flow_dump_instance *fdi, struct list_head *conn_head)
{
	int result;

	if ((result = ppe_drv_flow_dump_write_reset(fdi, "conn_v4"))) {
		return result;
	}

	ppe_drv_trace("Prep conn msg for %px\n", fdi);

	return ppe_flow_dump_v4_get(fdi,conn_head);
}

/*
 * ppe_drv_flow_dump_v6_get()
 *	Prepare a connection message for ipv6
 */
static bool ppe_drv_flow_dump_v6_get(struct ppe_drv_flow_dump_instance *fdi, struct list_head *conn_head)
{
	int result;

	if ((result = ppe_drv_flow_dump_write_reset(fdi, "conn_v6"))) {
		return result;
	}

	ppe_drv_trace("Prep conn msg for %px\n", fdi);

	return ppe_flow_dump_v6_get(fdi,conn_head);
}

/*
 * ppe_drv_flow_dump_dev_open()
 *	Opens the special char device file which we use to dump our state.
 */
static int ppe_drv_flow_dump_dev_open(struct inode *inode, struct file *file)
{
	struct ppe_drv_flow_dump_instance *fdi;
	struct ppe_drv *p = &ppe_drv_gbl;

	ppe_drv_info("flow_dump open\n");

	/*
	 * Allocate state information for the reading
	 */
	ppe_drv_assert(file->private_data == NULL, "unexpected double open: %px?\n", file->private_data);

	fdi = (struct ppe_drv_flow_dump_instance *)kzalloc(sizeof(struct ppe_drv_flow_dump_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!fdi) {
		return -ENOMEM;
	}

	file->private_data = fdi;
	fdi->dump_en = true;
	fdi->flow_cnt = 0;

	/*
	 * Initializing head pointers of connections
	 */
	fdi->cn_v4 = NULL;
	fdi->cn_tun_v4 = NULL;
	fdi->cn_v6 = NULL;
	fdi->cn_tun_v6 = NULL;

	spin_lock_bh(&p->lock);

	if (!list_empty(&p->conn_v4)) {
		fdi->cn_v4 = &p->conn_v4;
	}

	if (!list_empty(&p->conn_tun_v4)) {
		fdi->cn_tun_v4 = &p->conn_tun_v4;
	}

	if (!list_empty(&p->conn_v6)) {
		fdi->cn_v6 = &p->conn_v6;
	}

	if (!list_empty(&p->conn_tun_v6)) {
		fdi->cn_tun_v6 = &p->conn_tun_v6;
	}

	spin_unlock_bh(&p->lock);

	return 0;
}

/*
 * ppe_drv_flow_dump_dev_release()
 *	Called when a process closes the device file.
 */
static int ppe_drv_flow_dump_dev_release(struct inode *inode, struct file *file)
{
	struct ppe_drv_flow_dump_instance *fdi;

	fdi = (struct ppe_drv_flow_dump_instance *)file->private_data;

	if (fdi) {
		kfree(fdi);
	}

	return 0;
}

/*
 * ppe_drv_flow_dump_dev_read()
 *	Called to read the state
 */
static ssize_t ppe_drv_flow_dump_dev_read(struct file *file,	/* see include/linux/fs.h   */
		char *buffer,				/* buffer to fill with data */
		size_t length,				/* length of the buffer     */
		loff_t *offset)				/* Doesn't apply - this is a char file */
{
	struct ppe_drv_flow_dump_instance *fdi;
	int bytes_read = 0;

	fdi = (struct ppe_drv_flow_dump_instance *)file->private_data;
	if (!fdi) {
		return -ENOMEM;
	}

	ppe_drv_assert(file->private_data == NULL, "unexpected double open: %px?\n", file->private_data);
	/*
	 * If there is still some message remaining to be output then complete that first
	 */
	if (fdi->msg_len) {
		goto char_device_read_output;
	}

	if (fdi->cn_v4) {
		if (ppe_drv_flow_dump_v4_get(fdi, fdi->cn_v4)) {
			ppe_drv_warn("Failed to create ppe state conn_v4_msg\n");
			return -EIO;
		}

		fdi->cn_v4 = NULL;
		goto char_device_read_output;
	}

	if (fdi->cn_tun_v4) {
		if (ppe_drv_flow_dump_v4_get(fdi, fdi->cn_tun_v4)) {
			ppe_drv_warn("Failed to create ppe state conn_tun_v4_msg\n");
			return -EIO;
		}

		fdi->cn_tun_v4 = NULL;
		goto char_device_read_output;
	}

	if (fdi->cn_v6) {
		if (ppe_drv_flow_dump_v6_get(fdi, fdi->cn_v6)) {
			ppe_drv_warn("Failed to create ppe state conn_v6_msg\n");
			return -EIO;
		}

		fdi->cn_v6 = NULL;
		goto char_device_read_output;
	}

	if (fdi->cn_tun_v6) {
		if (ppe_drv_flow_dump_v6_get(fdi, fdi->cn_tun_v6)) {
			ppe_drv_warn("Failed to create ppe state conn_tun_v6_msg\n");
			return -EIO;
		}

		fdi->cn_tun_v6 = NULL;
		goto char_device_read_output;
	}

	return 0;

char_device_read_output:
	/*
	 * If supplied buffer is small we limit what we output
	 */
	bytes_read = fdi->msg_len;
	if (bytes_read > length) {
		bytes_read = length;
	}

	if (copy_to_user(buffer, fdi->msgp, bytes_read)) {
		return -EIO;
	}

	fdi->msg_len -= bytes_read;
	fdi->msgp += bytes_read;

	ppe_drv_trace("flow dump read, bytes_read %u bytes\n", bytes_read);

	/*
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;
}

/*
 * ppe_drv_flow_dump_dev_write()
 * 	write file operation handler
 */
static ssize_t ppe_drv_flow_dump_dev_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	/*
	 * Not supported.
	 */

	return -EINVAL;
}

/*
 * File operations used in the char device
 *	NOTE: The char device is a simple file that allows us to dump our connection tracking state
 */
static struct file_operations ppe_drv_flow_dump_fops = {
	.read = ppe_drv_flow_dump_dev_read,
	.write = ppe_drv_flow_dump_dev_write,
	.open = ppe_drv_flow_dump_dev_open,
	.release = ppe_drv_flow_dump_dev_release
};

/*
 * ppe_drv_flow_dump_exit()
 * 	unregister character device for flow dump.
 */
void ppe_drv_flow_dump_exit(void)
{
	unregister_chrdev(ppe_drv_flow_dump_dev_major_id, "ppe_flow_dump");

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ppe_drv_flow_dump_dentry) {
		debugfs_remove_recursive(ppe_drv_flow_dump_dentry);
	}

}

/*
 * ppe_drv_flow_dump_init()
 * 	register a character device for flow dump.
 */
int ppe_drv_flow_dump_init(struct dentry *dentry)
{
	int dev_id = -1;

	ppe_drv_flow_dump_dentry = debugfs_create_dir("ppe_drv_flow_dump", dentry);
	if (!ppe_drv_flow_dump_dentry) {
		ppe_drv_warn("Failed to create ppe flow dump directory in debugfs\n");
		return -1;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	if (!debugfs_create_u32("ppe_flow_dump", S_IRUGO, ppe_drv_flow_dump_dentry,
				(u32 *)&ppe_drv_flow_dump_dev_major_id)) {
		ppe_drv_warn("Failed to create ppe flow dump dev major file in debugfs\n");
		goto init_cleanup;
	}
#else
	debugfs_create_u32("ppe_flow_dump", S_IRUGO, ppe_drv_flow_dump_dentry,
                                (u32 *)&ppe_drv_flow_dump_dev_major_id);
#endif

	/*
	 * Register a char device that we will use to provide a dump of our state
	 */
	dev_id = register_chrdev(0, "ppe_drv_flow_dump", &ppe_drv_flow_dump_fops);
	if (dev_id < 0) {
		ppe_drv_warn("Failed to register chrdev %u\n", dev_id);
		goto init_cleanup;
	}

	ppe_drv_flow_dump_dev_major_id = dev_id;
	ppe_drv_trace("registered chr dev major id assigned %u\n", ppe_drv_flow_dump_dev_major_id);

	return 0;

init_cleanup:
	debugfs_remove_recursive(ppe_drv_flow_dump_dentry);
	return dev_id;
}
