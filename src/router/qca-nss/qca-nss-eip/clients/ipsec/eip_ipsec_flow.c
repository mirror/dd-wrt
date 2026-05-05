/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/ip.h>
#include <ppe_acl.h>
#include <eip_ipsec.h>
#include <eip.h>
#include <ppe_drv_sc.h>
#include <ecm_notifier.h>
#include "eip_ipsec_priv.h"

/*
 * __flow_tuple_print()
 *	Print IPsec tuple information with formatted context tag.
 */
/*
 * __flow_tuple_print()
 *      Print IPsec tuple information with formatted context tag.
 */
static void __flow_tuple_print(const struct eip_ipsec_tuple *t,
                                     const char *fmt, ...)
{
        char tag[EIP_IPSEC_TAG_MAX];
        va_list ap;

        if (!t || !fmt)
                return;

        va_start(ap, fmt);
        vsnprintf(tag, sizeof(tag), fmt, ap);
        va_end(ap);

        if (t->ip_version == 4) {
                pr_debug("%s: tuple v4: src=%pI4 dst=%pI4 spi=0x%x proto=%u sport=%u dport=%u\n",
                         tag, &t->src_ip[0], &t->dest_ip[0],
                         t->esp_spi, t->protocol, t->sport, t->dport);
        } else if (t->ip_version == 6) {
                pr_debug("%s: tuple v6: src=%pI6c dst=%pI6c spi=0x%x proto=%u sport=%u dport=%u\n",
                         tag, (const u8 *)t->src_ip, (const u8 *)t->dest_ip,
                         t->esp_spi, t->protocol, t->sport, t->dport);
        }
}

/*
 * __flow_tuple_from_ecm()
 *	Fill EIP flow information from ECM notifier information.
 */
static void __flow_tuple_from_ecm(struct ecm_notifier_connection_data *ecm_info,
		struct eip_ipsec_tuple *tuple, bool ipsec)
{
	struct in6_addr *sip6, *dip6;
	struct in_addr *sip, *dip;

	memset(tuple, 0, sizeof(*tuple));
	tuple->ip_version = ecm_info->tuple.ip_ver;
	tuple->protocol = ecm_info->tuple.protocol;

	if (ipsec) {
		sip = &ecm_info->tuple.dest.in;
		dip = &ecm_info->tuple.src.in;
		sip6 = &ecm_info->tuple.dest.in6;
		dip6 = &ecm_info->tuple.src.in6;
		tuple->sport = ecm_info->tuple.dst_port;
		tuple->dport = ecm_info->tuple.src_port;
	} else {
		sip = &ecm_info->tuple.src.in;
		dip = &ecm_info->tuple.dest.in;
		sip6 = &ecm_info->tuple.src.in6;
		dip6 = &ecm_info->tuple.dest.in6;
		tuple->sport = ecm_info->tuple.src_port;
		tuple->dport = ecm_info->tuple.dst_port;
	}

	switch (tuple->ip_version) {
	case 6:
		tuple->src_ip[3] = sip6->in6_u.u6_addr32[3];
		tuple->src_ip[2] = sip6->in6_u.u6_addr32[2];
		tuple->src_ip[1] = sip6->in6_u.u6_addr32[1];
		tuple->src_ip[0] = sip6->in6_u.u6_addr32[0];

		tuple->dest_ip[3] = dip6->in6_u.u6_addr32[3];
		tuple->dest_ip[2] = dip6->in6_u.u6_addr32[2];
		tuple->dest_ip[1] = dip6->in6_u.u6_addr32[1];
		tuple->dest_ip[0] = dip6->in6_u.u6_addr32[0];
		break;

	case 4:
		tuple->src_ip[0] = sip->s_addr;
		tuple->dest_ip[0] = dip->s_addr;
		break;

	default:
		pr_err("%px: Invalid IP version %d\n", ecm_info, tuple->ip_version);
		break;
	}
}

/*
 * eip_ipsecget_dev()
 *	Get IPsec dev.
 */
static inline struct net_device *__flow_get_dev(struct ecm_notifier_connection_data *data, bool *ipsec)
{
	if (eip_ipsec_dev_is_nss(data->to_dev)) {
		*ipsec = false;
		return data->to_dev;
	}

	if (eip_ipsec_dev_is_nss(data->from_dev)) {
		*ipsec = true;
		return data->from_dev;
	}

	return NULL;
}

/*
 * eip_ipsec_flow_notifier()
 *	ECM notifier callback.
 */
static int eip_ipsec_flow_notifier(struct notifier_block *nb, unsigned long action, void *dt)
{
	struct ecm_notifier_connection_data *data = dt;
	struct eip_ipsec_tuple tuple = {0};
	struct net_device *ipsec_dev;
	struct eip_ipsec_sa *sa = NULL;
	bool ipsec;

	if (!data) {
		pr_debug("Invalid data from ECM\n");
		return NOTIFY_OK;
	}

	/* Check if flow is IPsec and get device */
	ipsec_dev = __flow_get_dev(data, &ipsec);
	if (!ipsec_dev) {
		pr_debug("Flow is non-IPsec flow\n");
		return NOTIFY_OK;
	}

	/* Skip tunnel outer flows */
	if (data->is_tun_outer) {
		pr_debug("The flow is encapsulated flow\n");
		return NOTIFY_OK;
	}

	/* Fill flow information */
	__flow_tuple_from_ecm(data, &tuple, ipsec);

	switch (action) {
	case ECM_NOTIFIER_ACTION_CONNECTION_ADDED:

		/* Return for non-PPE flows */
		if (data->ae_type != ECM_AE_TYPE_PPE) {
			pr_debug("AE type is not PPE, AE_TYPE : %u\n", data->ae_type);
			return NOTIFY_OK;
		}

		sa = eip_ipsec_sa_ref_get_encap(ipsec_dev);
		if (!sa) {
			pr_debug("%px: Failed to get SA\n", ipsec_dev);
			return NOTIFY_OK;
		}

		if (eip_ipsec_flow_add(ipsec_dev, &tuple, sa)) {
			pr_debug("%px : Failed to add flow\n", ipsec_dev);
		}
		eip_ipsec_sa_deref(sa);
		break;

	case ECM_NOTIFIER_ACTION_CONNECTION_REMOVED:
		eip_ipsec_flow_del(ipsec_dev, &tuple, true);
		break;

	default:
		pr_debug("%px: ECM action is not supported\n", ipsec_dev);
		break;
	}

	return NOTIFY_OK;
}

/*
 * eip_flow_notifier
 *	ECM Notifier callback.
 */
static struct notifier_block eip_flow_notifier = {
	.notifier_call = eip_ipsec_flow_notifier,
};

/*
 * __flow_fill_tuple()
 *	Fill flow tuple information from SA tuple information.
 */
static inline void __flow_fill_tuple(struct eip_ipsec_tuple *tuple, struct eip_flow_tuple *flow_tuple, bool is_encap)
{
	int i;

	flow_tuple->ip_ver = tuple->ip_version;
	flow_tuple->ip_proto = tuple->protocol;

	if (flow_tuple->ip_ver == 4) {
		flow_tuple->src_ip[0] = ntohl(tuple->src_ip[0]);
		flow_tuple->dst_ip[0] = ntohl(tuple->dest_ip[0]);
	} else {
		for (i = 0; i < 4; i++) {
			flow_tuple->src_ip[i] = ntohl(tuple->src_ip[i]);
			flow_tuple->dst_ip[i] = ntohl(tuple->dest_ip[i]);
		}
	}

	if (is_encap) {
		/* Encap direction - use ports but clear SPI */
		flow_tuple->src_port = tuple->sport;
		flow_tuple->dst_port = tuple->dport;
		flow_tuple->spi = 0;
	} else {
		/* Decap direction - use SPI but clear port numbers */
		flow_tuple->spi = ntohl(tuple->esp_spi);
		flow_tuple->src_port = 0;
		flow_tuple->dst_port = 0;
	}
}

/*
 * __flow_get_esph_offset()
 *	Return ESP header offset packet.
 */
static inline u32 __flow_get_esph_offset(u32 sa_flags)
{
	u32 offset;

	offset = (sa_flags & EIP_IPSEC_FLAG_IPV6) ? sizeof(struct ipv6hdr) : sizeof(struct iphdr);
	offset += (sa_flags & EIP_IPSEC_FLAG_UDP) ? sizeof(struct udphdr) : 0;

	return offset;
}

/*
 * eip_ipsec_config_acl_for_spi_match()
 *	Create and configure ACL rule for SPI match.
 */
s16 eip_ipsec_flow_match_spi(__le32 spi, struct net_device *dev, u32 sa_flags)
{
	s16 acl_rule_id = EIP_IPSEC_INVALID_ACL_RULE_ID;
	struct ppe_acl_rule_match_udf *udf;
	struct ppe_acl_rule *acl_rule;
	u32 esp_offset;
	ppe_acl_ret_t ret;
	bool is_ipv6;

	esp_offset = __flow_get_esph_offset(sa_flags);
	is_ipv6 = !!(sa_flags & EIP_IPSEC_FLAG_IPV6);

	acl_rule = kzalloc(sizeof(struct ppe_acl_rule), GFP_ATOMIC);
	if (!acl_rule) {
		pr_err("%px: Failed to allocate ACL rule\n", dev);
		return acl_rule_id;
	}

	udf = &acl_rule->rules[PPE_ACL_RULE_MATCH_TYPE_UDF].rule.udf;

	/* Fill common part of ACL */
	acl_rule->cmn.cmn_flags = PPE_ACL_RULE_CMN_FLAG_NO_RULEID | PPE_ACL_RULE_CMN_FLAG_POST_RT_EN;
	acl_rule->valid_flags = PPE_ACL_RULE_MATCH_TYPE_UDF_VALID;
	acl_rule->userspace_rule = false;
	acl_rule->rules[PPE_ACL_RULE_MATCH_TYPE_UDF].rule_flags = 0;

	udf->udf_a.min = upper_16_bits(spi);
	udf->udf_a.mask_max = U16_MAX;
	udf->udf_a.valid_op = PPE_ACL_UDF_OP_MASK;
	udf->udf_a.offset = esp_offset;
	udf->udf_a.offset_base = PPE_ACL_UDF_OFFSET_BASE_L3;
	udf->udf_a.proto = is_ipv6 ? PPE_ACL_UDF_IPV6 : PPE_ACL_UDF_IPV4;

	udf->udf_b.min = lower_16_bits(spi);
	udf->udf_b.mask_max = U16_MAX;
	udf->udf_b.valid_op = PPE_ACL_UDF_OP_MASK;
	udf->udf_b.offset = esp_offset + 2;
	udf->udf_b.offset_base = PPE_ACL_UDF_OFFSET_BASE_L3;
	udf->udf_b.proto = is_ipv6 ? PPE_ACL_UDF_IPV6 : PPE_ACL_UDF_IPV4;

	/* Fill ACL action information */
	acl_rule->action.flags |= PPE_ACL_RULE_ACTION_FLAG_SERVICE_CODE_EN;
	acl_rule->action.flags |= PPE_ACL_RULE_ACTION_FLAG_DEST_INFO_CHANGE_EN;
	acl_rule->action.service_code = PPE_DRV_SC_IPSEC_PPE2EIP_DECAP;

	strscpy(acl_rule->action.dst.dev_name, dev->name, IFNAMSIZ);
	acl_rule->action.fwd_cmd = PPE_ACL_FWD_CMD_FWD;

	/* Fill ACL rule bind information */
	acl_rule->dev_type = PPE_ACL_RULE_DEV_TYPE_SC;
	acl_rule->dev.sc = PPE_DRV_SC_IPSEC_PPE2EIP_ACL_MATCH;

	ret = ppe_acl_rule_create(acl_rule);
	if (ret != PPE_ACL_RET_SUCCESS) {
		pr_err("ACL rule creation failed for SPI: 0x%x. Returned error code: %d\n", spi, ret);
		goto out;
	}

	acl_rule_id = acl_rule->rule_id;

out:
	kfree(acl_rule);
	return acl_rule_id;
}

/*
 * eip_ipsec_flow_add()
 *	Add flow to EIP HW.
 */
int eip_ipsec_flow_add(struct net_device *ndev, struct eip_ipsec_tuple *flow_tuple, struct eip_ipsec_sa *sa)
{
	struct eip_flow_info flow_info = {0};
	struct eip_flow_tuple *tuple;
	struct eip_ipsec_dev *eid;
	bool is_encap;

	tuple = &flow_info.tuple;

	is_encap = !!(sa->flags & EIP_IPSEC_FLAG_ENC);
	__flow_fill_tuple(flow_tuple, tuple, is_encap);

	/* Setup flow info */
	eid = netdev_priv(ndev);
	flow_info.tr = sa->tr;

	if (eip_flow_add(eid->tun, &flow_info)) {
		__flow_tuple_print(flow_tuple, "%px: Flow add failed for %s direction", ndev, is_encap ? "encap" : "decap");
		return -EINVAL;
	}

	__flow_tuple_print(flow_tuple, "%px: Flow added successfully for %s direction", ndev, is_encap ? "encap" : "decap");
	return 0;
}

/*
 * eip_ipsec_flow_del()
 *	Delete flow from EIP HW.
 */
void eip_ipsec_flow_del(struct net_device *ndev, struct eip_ipsec_tuple *flow_tuple, bool is_encap)
{
	struct eip_flow_info flow_info = {0};
	struct eip_flow_tuple *tuple;
	struct eip_ipsec_dev *eid;

	eid = netdev_priv(ndev);
	tuple = &flow_info.tuple ;

	__flow_fill_tuple(flow_tuple, tuple, is_encap);
	eip_flow_del(eid->tun, &flow_info);
	__flow_tuple_print(flow_tuple, "%px: Flow deleted for %s direction", ndev, (is_encap ? "encap" : "decap"));
}

/*
 * eip_ipsec_flow_init()
 *	Initialize flow module and register ECM notifier.
 */
int eip_ipsec_flow_init(void)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	int err = 0;

	/* Register ECM notifier for flow add/delete */
	if (eip_feature_check(drv->ctx, EIP_OFFLOAD_INNER_FLOW)) {
		err = ecm_notifier_register_connection_notify(&eip_flow_notifier);
		if (err) {
			pr_err("%px: Failed to register notifier with ECM, error(%d)\n", drv, err);
			return err;
		}
		pr_info("%px: Registered ECM notifier for IPsec flow offload\n", drv);

		/*
		 * TODO: Analyze queue-to-service mapping and remove.
		 */
		/* Configure PPE2EIP encap and decap service code's queue */
		ppe_drv_eip_sc_queue_config_set();
		pr_info("%px: Configured PPE2EIP service code queue mappings\n", drv);
	}

	return 0;
}

/*
 * eip_ipsec_flow_deinit()
 *	Deinitialize flow module and unregister ECM notifier.
 */
void eip_ipsec_flow_deinit(void)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;

	/* Unregister ecm notifier */
	if (eip_feature_check(drv->ctx, EIP_OFFLOAD_INNER_FLOW)) {
		ecm_notifier_unregister_connection_notify(&eip_flow_notifier);
		pr_info("%px: Unregistered ECM notifier for IPsec flow offload\n", drv);
	}
}
