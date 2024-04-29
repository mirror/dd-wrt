/*
 **************************************************************************
 * Copyright (c) 2014-2021, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#ifdef ECM_MULTICAST_ENABLE
struct ecm_db_multicast_tuple_instance *
ecm_db_multicast_tuple_instance_alloc(ip_addr_t origin,
				      ip_addr_t group,
				      uint16_t src_port,
				      uint16_t dst_port);
int ecm_db_multicast_tuple_instance_deref(struct ecm_db_multicast_tuple_instance *ti);
void ecm_db_multicast_connection_deref(struct ecm_db_multicast_tuple_instance *ti);
void ecm_db_multicast_tuple_instance_add(struct ecm_db_multicast_tuple_instance *ti,
					 struct ecm_db_connection_instance *ci);
struct ecm_db_multicast_tuple_instance *ecm_db_multicast_connection_find_and_ref(ip_addr_t origin,
										 ip_addr_t group);
struct ecm_db_multicast_tuple_instance *ecm_db_multicast_connection_get_and_ref_first(ip_addr_t group);
struct ecm_db_multicast_tuple_instance *ecm_db_multicast_connection_get_and_ref_next(struct ecm_db_multicast_tuple_instance *ti);
void ecm_db_multicast_tuple_instance_source_ip_get(struct ecm_db_multicast_tuple_instance *ti, ip_addr_t origin);
void ecm_db_multicast_tuple_instance_group_ip_get(struct ecm_db_multicast_tuple_instance *ti, ip_addr_t group);
uint32_t ecm_db_multicast_tuple_instance_flags_get(struct ecm_db_multicast_tuple_instance *ti);
void ecm_db_multicast_tuple_instance_flags_set(struct ecm_db_multicast_tuple_instance *ti, uint32_t flags);
void ecm_db_multicast_tuple_instance_flags_clear(struct ecm_db_multicast_tuple_instance *ti, uint32_t flags);
void ecm_db_multicast_tuple_instance_set_and_hold_l2_br_dev(struct ecm_db_multicast_tuple_instance *ti, struct net_device *l2_br_dev);
void ecm_db_multicast_tuple_instance_set_and_hold_l3_br_dev(struct ecm_db_multicast_tuple_instance *ti, struct net_device *l3_br_dev);
struct net_device *ecm_db_multicast_tuple_instance_get_l2_br_dev(struct ecm_db_multicast_tuple_instance *ti);
struct net_device *ecm_db_multicast_tuple_instance_get_l3_br_dev(struct ecm_db_multicast_tuple_instance *ti);

struct ecm_db_connection_instance *ecm_db_multicast_connection_get_from_tuple(struct ecm_db_multicast_tuple_instance *ti);
void ecm_db_multicast_connection_to_interfaces_deref_all(struct ecm_db_iface_instance *interfaces, int32_t *ifaces_first);
void ecm_db_multicast_connection_to_interfaces_clear(struct ecm_db_connection_instance *ci);
int32_t ecm_db_multicast_connection_to_interfaces_get_and_ref_all(struct ecm_db_connection_instance *ci,
								  struct ecm_db_iface_instance **interfaces,
								  int32_t **to_ifaces_first);
int ecm_db_multicast_connection_to_interfaces_reset(struct ecm_db_connection_instance *ci,
						    struct ecm_db_iface_instance *interfaces,
						    int32_t *new_first);
void ecm_db_multicast_connection_to_interfaces_update(struct ecm_db_connection_instance *ci,
						      struct ecm_db_iface_instance *interfaces,
						      int32_t *new_first, int *join_valid_idx);
void ecm_db_multicast_connection_data_totals_update(struct ecm_db_connection_instance *ci,
						    bool is_from, uint64_t size, uint64_t packets);
void ecm_db_multicast_connection_to_interfaces_clear_at_index(struct ecm_db_connection_instance *ci,
							      uint32_t index);
void ecm_db_multicast_connection_data_totals_update(struct ecm_db_connection_instance *ci,
						    bool is_from, uint64_t size, uint64_t packets);
void ecm_db_multicast_connection_interface_heirarchy_stats_update(struct ecm_db_connection_instance *ci,
								  uint64_t size, uint64_t packets);
bool ecm_db_multicast_connection_to_interfaces_set_check(struct ecm_db_connection_instance *ci);
int _ecm_db_multicast_tuple_instance_deref(struct ecm_db_multicast_tuple_instance *ti);
int ecm_db_multicast_to_interfaces_xml_state_get(struct ecm_db_connection_instance *ci, struct ecm_state_file_instance *sfi);
int ecm_db_multicast_connection_to_interfaces_get_count(struct ecm_db_connection_instance *ci);
#ifdef ECM_INTERFACE_OVS_BRIDGE_ENABLE
#ifdef ECM_CLASSIFIER_OVS_ENABLE
bool ecm_db_multicast_ovs_verify_to_list(struct ecm_db_connection_instance *ci, struct ecm_classifier_process_response *aci_pr);
void ecm_db_multicast_tuple_set_ovs_ingress_vlan(struct ecm_db_multicast_tuple_instance *ti, uint32_t *ingress_vlan_tag);
struct vlan_hdr ecm_db_multicast_tuple_get_ovs_ingress_vlan(struct ecm_db_multicast_tuple_instance *ti);
#endif
#endif
void ecm_db_multicast_connection_to_interfaces_leave(struct ecm_db_connection_instance *ci, struct ecm_multicast_if_update *mc_update);
#endif
