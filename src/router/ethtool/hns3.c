/* Copyright (c) 2023 Huawei Corporation */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "internal.h"

/* distinguish drive register data of earlier versions */
#define HNS3_REG_MAGIC_NUMBER 0x686e733372656773 /* hns3regs */
#define HNS3_REG_RSV_NAME "reserved"
#define HNS3_REG_UNKNOW_NAME "unknown"
#define HNS3_REG_UNKNOW_VALUE_LEN 4

struct hns3_reg_tlv {
	u16 tag;
	u16 len;
};

struct hns3_reg_header {
	u64 magic_number;
	u8 is_vf;
	u8 rsv[7];
};

struct hns3_reg_info {
	const char *name;
	u16 value_len;
};

struct hns3_regs_group {
	const char *group_name;
	const struct hns3_reg_info *regs;
	u16 regs_count;
};

enum hns3_reg_tag {
	HNS3_TAG_CMDQ = 0,
	HNS3_TAG_COMMON,
	HNS3_TAG_RING,
	HNS3_TAG_TQP_INTR,
	HNS3_TAG_QUERY_32_BIT,
	HNS3_TAG_QUERY_64_BIT,
	HNS3_TAG_DFX_BIOS_COMMON,
	HNS3_TAG_DFX_SSU_0,
	HNS3_TAG_DFX_SSU_1,
	HNS3_TAG_DFX_IGU_EGU,
	HNS3_TAG_DFX_RPU_0,
	HNS3_TAG_DFX_RPU_1,
	HNS3_TAG_DFX_NCSI,
	HNS3_TAG_DFX_RTC,
	HNS3_TAG_DFX_PPP,
	HNS3_TAG_DFX_RCB,
	HNS3_TAG_DFX_TQP,
	HNS3_TAG_DFX_SSU_2,
	HNS3_TAG_DFX_RPU_TNL,
	HNS3_TAG_MAX,
};

const bool hns3_reg_is_repeat_tag_array[] = {
	[HNS3_TAG_RING] = true,
	[HNS3_TAG_TQP_INTR] = true,
	[HNS3_TAG_DFX_RPU_TNL] = true,
};

const struct hns3_reg_info pf_cmdq_regs[] = {
	{"comm_nic_csq_baseaddr_l", 4},
	{"comm_nic_csq_baseaddr_h", 4},
	{"comm_nic_csq_depth", 4},
	{"comm_nic_csq_tail", 4},
	{"comm_nic_csq_head", 4},
	{"comm_nic_crq_baseaddr_l", 4},
	{"comm_nic_crq_baseaddr_h", 4},
	{"comm_nic_crq_depth", 4},
	{"comm_nic_crq_tail", 4},
	{"comm_nic_crq_head", 4},
	{"comm_vector0_cmdq_src", 4},
	{"comm_cmdq_intr_sts", 4},
	{"comm_cmdq_intr_en", 4},
	{"comm_cmdq_intr_gen", 4},
};

const struct hns3_reg_info pf_common_regs[] = {
	{"misc_vector_base", 4},
	{"pf_other_int", 4},
	{"misc_reset_sts", 4},
	{"misc_vector_int_sts", 4},
	{"global_reset", 4},
	{"fun_rst_ing", 4},
	{"gro_en", 4},
};

const struct hns3_reg_info pf_ring_regs[] = {
	{"ring_rx_addr_l", 4},
	{"ring_rx_addr_h", 4},
	{"ring_rx_bd_num", 4},
	{"ring_rx_bd_length", 4},
	{"ring_rx_merge_en", 4},
	{"ring_rx_tail", 4},
	{"ring_rx_head", 4},
	{"ring_rx_fbd_num", 4},
	{"ring_rx_offset", 4},
	{"ring_rx_fbd_offset", 4},
	{"ring_rx_stash", 4},
	{"ring_rx_bd_err", 4},
	{"ring_tx_addr_l", 4},
	{"ring_tx_addr_h", 4},
	{"ring_tx_bd_num", 4},
	{"ring_tx_priority", 4},
	{"ring_tx_tc", 4},
	{"ring_tx_merge_en", 4},
	{"ring_tx_tail", 4},
	{"ring_tx_head", 4},
	{"ring_tx_fbd_num", 4},
	{"ring_tx_offset", 4},
	{"ring_tx_ebd_num", 4},
	{"ring_tx_ebd_offset", 4},
	{"ring_tx_bd_err", 4},
	{"ring_en", 4},
};

const struct hns3_reg_info pf_tqp_intr_regs[] = {
	{"tqp_intr_ctrl", 4},
	{"tqp_intr_gl0", 4},
	{"tqp_intr_gl1", 4},
	{"tqp_intr_gl2", 4},
	{"tqp_intr_rl", 4},
};

const struct hns3_reg_info query_32_bit_regs[] = {
	{"ssu_common_err_int", 4},
	{"ssu_port_based_err_int", 4},
	{"ssu_fifo_overflow_int", 4},
	{"ssu_ets_tcg_int", 4},
	{"ssu_bp_status_0", 4},
	{"ssu_bp_status_1", 4},
	{"ssu_bp_status_2", 4},
	{"ssu_bp_status_3", 4},
	{"ssu_bp_status_4", 4},
	{"ssu_bp_status_5", 4},
	{"ssu_mac_tx_pfc_ind", 4},
	{"ssu_mac_rx_pfc_ind", 4},
	{"ssu_rx_oq_drop_pkt_cnt", 4},
	{"ssu_tx_oq_drop_pkt_cnt", 4},
};

const struct hns3_reg_info query_64_bit_regs[] = {
	{"ppp_get_rx_pkt_cnt", 8},
	{"ppp_get_tx_pkt_cnt", 8},
	{"ppp_send_uc_prt2host_pkt_cnt", 8},
	{"ppp_send_uc_prt2prt_pkt_cnt", 8},
	{"ppp_send_uc_host2host_pkt_cnt", 8},
	{"ppp_send_uc_host2prt_pkt_cnt", 8},
	{"ppp_send_mc_from_prt_cnt", 8},
};

const struct hns3_reg_info dfx_bios_common_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"bp_cpu_state", 4},
	{"dfx_msix_info_nic_0", 4},
	{"dfx_msix_info_nic_1", 4},
	{"dfx_msix_info_nic_2", 4},
	{"dfx_msix_info_nic_3", 4},
	{"dfx_msix_info_roc_0", 4},
	{"dfx_msix_info_roc_1", 4},
	{"dfx_msix_info_roc_2", 4},
	{"dfx_msix_info_roc_3", 4},
	{HNS3_REG_RSV_NAME, 8},
};

const struct hns3_reg_info dfx_ssu_0_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"ssu_ets_port_status", 4},
	{"ssu_ets_tcg_status", 4},
	{HNS3_REG_RSV_NAME, 4},
	{HNS3_REG_RSV_NAME, 4},
	{"ssu_bp_status_0", 4},
	{"ssu_bp_status_1", 4},
	{"ssu_bp_status_2", 4},
	{"ssu_bp_status_3", 4},
	{"ssu_bp_status_4", 4},
	{"ssu_bp_status_5", 4},
	{"ssu_mac_tx_pfc_ind", 4},
	{"mac_ssu_rx_pfc_ind", 4},
	{"btmp_ageing_st_b0", 4},
	{"btmp_ageing_st_b1", 4},
	{"btmp_ageing_st_b2", 4},
	{HNS3_REG_RSV_NAME, 8},
	{"full_drop_num", 4},
	{"part_drop_num", 4},
	{"ppp_key_drop_num", 4},
	{"ppp_rlt_drop_num", 4},
	{"lo_pri_unicast_rlt_drop_num", 4},
	{"hi_pri_multicast_rlt_drop_num", 4},
	{"lo_pri_multicast_rlt_drop_num", 4},
	{"ncsi_packet_curr_buffer_cnt", 4},
	{HNS3_REG_RSV_NAME, 12},
	{"ssu_mb_rd_rlt_drop_cnt", 4},
	{"ssu_ppp_mac_key_num", 8},
	{"ssu_ppp_host_key_num", 8},
	{"ppp_ssu_mac_rlt_num", 8},
	{"ppp_ssu_host_rlt_num", 8},
	{"ncsi_rx_packet_in_cnt", 8},
	{"ncsi_tx_packet_out_cnt", 8},
	{"ssu_key_drop_num", 4},
	{"mb_uncopy_num", 4},
	{"rx_oq_drop_pkt_cnt", 4},
	{"tx_oq_drop_pkt_cnt", 4},
	{"bank_unbalance_drop_cnt", 4},
	{"bank_unbalance_rx_drop_cnt", 4},
	{"nic_l2_err_drop_pkt_cnt", 4},
	{"roc_l2_err_drop_pkt_cnt", 4},
	{"nic_l2_err_drop_pkt_cnt_rx", 4},
	{"roc_l2_err_drop_pkt_cnt_rx", 4},
	{"rx_oq_glb_drop_pkt_cnt", 4},
	{HNS3_REG_RSV_NAME, 4},
	{"lo_pri_unicast_cur_cnt", 4},
	{"hi_pri_multicast_cur_cnt", 4},
	{"lo_pri_multicast_cur_cnt", 4},
	{HNS3_REG_RSV_NAME, 12},
};

const struct hns3_reg_info dfx_ssu_1_regs[] = {
	{"prt_id", 4},
	{"packet_tc_curr_buffer_cnt_0", 4},
	{"packet_tc_curr_buffer_cnt_1", 4},
	{"packet_tc_curr_buffer_cnt_2", 4},
	{"packet_tc_curr_buffer_cnt_3", 4},
	{"packet_tc_curr_buffer_cnt_4", 4},
	{"packet_tc_curr_buffer_cnt_5", 4},
	{"packet_tc_curr_buffer_cnt_6", 4},
	{"packet_tc_curr_buffer_cnt_7", 4},
	{"packet_curr_buffer_cnt", 4},
	{HNS3_REG_RSV_NAME, 8},
	{"rx_packet_in_cnt", 8},
	{"rx_packet_out_cnt", 8},
	{"tx_packet_in_cnt", 8},
	{"tx_packet_out_cnt", 8},
	{"roc_rx_packet_in_cnt", 8},
	{"roc_tx_packet_out_cnt", 8},
	{"rx_packet_tc_in_cnt_0", 8},
	{"rx_packet_tc_in_cnt_1", 8},
	{"rx_packet_tc_in_cnt_2", 8},
	{"rx_packet_tc_in_cnt_3", 8},
	{"rx_packet_tc_in_cnt_4", 8},
	{"rx_packet_tc_in_cnt_5", 8},
	{"rx_packet_tc_in_cnt_6", 8},
	{"rx_packet_tc_in_cnt_7", 8},
	{"rx_packet_tc_out_cnt_0", 8},
	{"rx_packet_tc_out_cnt_1", 8},
	{"rx_packet_tc_out_cnt_2", 8},
	{"rx_packet_tc_out_cnt_3", 8},
	{"rx_packet_tc_out_cnt_4", 8},
	{"rx_packet_tc_out_cnt_5", 8},
	{"rx_packet_tc_out_cnt_6", 8},
	{"rx_packet_tc_out_cnt_7", 8},
	{"tx_packet_tc_in_cnt_0", 8},
	{"tx_packet_tc_in_cnt_1", 8},
	{"tx_packet_tc_in_cnt_2", 8},
	{"tx_packet_tc_in_cnt_3", 8},
	{"tx_packet_tc_in_cnt_4", 8},
	{"tx_packet_tc_in_cnt_5", 8},
	{"tx_packet_tc_in_cnt_6", 8},
	{"tx_packet_tc_in_cnt_7", 8},
	{"tx_packet_tc_out_cnt_0", 8},
	{"tx_packet_tc_out_cnt_1", 8},
	{"tx_packet_tc_out_cnt_2", 8},
	{"tx_packet_tc_out_cnt_3", 8},
	{"tx_packet_tc_out_cnt_4", 8},
	{"tx_packet_tc_out_cnt_5", 8},
	{"tx_packet_tc_out_cnt_6", 8},
	{"tx_packet_tc_out_cnt_7", 8},
	{HNS3_REG_RSV_NAME, 8},
};

const struct hns3_reg_info dfx_igu_egu_regs[] = {
	{"prt_id", 4},
	{"igu_rx_err_pkt", 4},
	{"igu_rx_no_sof_pkt", 4},
	{"egu_tx_1588_short_pkt", 4},
	{"egu_tx_1588_pkt", 4},
	{"egu_tx_err_pkt", 4},
	{"igu_rx_out_l2_pkt", 4},
	{"igu_rx_out_l3_pkt", 4},
	{"igu_rx_out_l4_pkt", 4},
	{"igu_rx_in_l2_pkt", 4},
	{"igu_rx_in_l3_pkt", 4},
	{"igu_rx_in_l4_pkt", 4},
	{"igu_rx_el3e_pkt", 4},
	{"igu_rx_el4e_pkt", 4},
	{"igu_rx_l3e_pkt", 4},
	{"igu_rx_l4e_pkt", 4},
	{"igu_rx_rocee_pkt", 4},
	{"igu_rx_out_udp0_pkt", 4},
	{"igu_rx_in_udp0_pkt", 4},
	{"mul_car_drop_pkt_cnt", 8},
	{"bro_car_drop_pkt_cnt", 8},
	{HNS3_REG_RSV_NAME, 4},
	{"igu_rx_oversize_pkt", 8},
	{"igu_rx_undersize_pkt", 8},
	{"igu_rx_out_all_pkt", 8},
	{"igu_tx_out_all_pkt", 8},
	{"igu_rx_uni_pkt", 8},
	{"igu_rx_multi_pkt", 8},
	{"igu_rx_broad_pkt", 8},
	{"egu_tx_out_all_pkt", 8},
	{"egu_tx_uni_pkt", 8},
	{"egu_tx_multi_pkt", 8},
	{"egu_tx_broad_pkt", 8},
	{"igu_tx_key_num", 8},
	{"igu_rx_non_tun_pkt", 8},
	{"igu_rx_tun_pkt", 8},
	{HNS3_REG_RSV_NAME, 8},
};

const struct hns3_reg_info dfx_rpu_0_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"fsm_dfx_st0", 4},
	{"fsm_dfx_st1", 4},
	{"rpu_rx_pkt_drop_cnt", 4},
	{"buf_wait_timeout", 4},
	{"buf_wait_timeout_qid", 4},
};

const struct hns3_reg_info dfx_rpu_1_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"fifo_dfx_st0", 4},
	{"fifo_dfx_st1", 4},
	{"fifo_dfx_st2", 4},
	{"fifo_dfx_st3", 4},
	{"fifo_dfx_st4", 4},
	{"fifo_dfx_st5", 4},
	{HNS3_REG_RSV_NAME, 20},
};

const struct hns3_reg_info dfx_ncsi_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"ncsi_egu_tx_fifo_sts", 4},
	{"ncsi_pause_status", 4},
	{"ncsi_rx_ctrl_dmac_err_cnt", 4},
	{"ncsi_rx_ctrl_smac_err_cnt", 4},
	{"ncsi_rx_ctrl_cks_err_cnt", 4},
	{"ncsi_rx_ctrl_pkt_cnt", 4},
	{"ncsi_rx_pt_dmac_err_cnt", 4},
	{"ncsi_rx_pt_smac_err_cnt", 4},
	{"ncsi_rx_pt_pkt_cnt", 4},
	{"ncsi_rx_fcs_err_cnt", 4},
	{"ncsi_tx_ctrl_dmac_err_cnt", 4},
	{"ncsi_tx_ctrl_smac_err_cnt", 4},
	{"ncsi_tx_ctrl_pkt_cnt", 4},
	{"ncsi_tx_pt_dmac_err_cnt", 4},
	{"ncsi_tx_pt_smac_err_cnt", 4},
	{"ncsi_tx_pt_pkt_cnt", 4},
	{"ncsi_tx_pt_pkt_trun_cnt", 4},
	{"ncsi_tx_pt_pkt_err_cnt", 4},
	{"ncsi_tx_ctrl_pkt_err_cnt", 4},
	{"ncsi_rx_ctrl_pkt_trun_cnt", 4},
	{"ncsi_rx_ctrl_pkt_cflit_cnt", 4},
	{HNS3_REG_RSV_NAME, 8},
	{"ncsi_mac_rx_octets_ok", 4},
	{"ncsi_mac_rx_octets_bad", 4},
	{"ncsi_mac_rx_uc_pkts", 4},
	{"ncsi_mac_rx_mc_pkts", 4},
	{"ncsi_mac_rx_bc_pkts", 4},
	{"ncsi_mac_rx_pkts_64octets", 4},
	{"ncsi_mac_rx_pkts_65to127octets", 4},
	{"ncsi_mac_rx_pkts_128to255octets", 4},
	{"ncsi_mac_rx_pkts_256to511octets", 4},
	{"ncsi_mac_rx_pkts_512to1023octets", 4},
	{"ncsi_mac_rx_pkts_1024to1518octets", 4},
	{"ncsi_mac_rx_pkts_1519tomaxoctets", 4},
	{"ncsi_mac_rx_fcs_errors", 4},
	{"ncsi_mac_rx_long_errors", 4},
	{"ncsi_mac_rx_jabber_errors", 4},
	{"ncsi_mac_rx_runt_err_cnt", 4},
	{"ncsi_mac_rx_short_err_cnt", 4},
	{"ncsi_mac_rx_filt_pkt_cnt", 4},
	{"ncsi_mac_rx_octets_total_filt", 4},
	{"ncsi_mac_tx_octets_ok", 4},
	{"ncsi_mac_tx_octets_bad", 4},
	{"ncsi_mac_tx_uc_pkts", 4},
	{"ncsi_mac_tx_mc_pkts", 4},
	{"ncsi_mac_tx_bc_pkts", 4},
	{"ncsi_mac_tx_pkts_64octets", 4},
	{"ncsi_mac_tx_pkts_65to127octets", 4},
	{"ncsi_mac_tx_pkts_128to255octets", 4},
	{"ncsi_mac_tx_pkts_256to511octets", 4},
	{"ncsi_mac_tx_pkts_512to1023octets", 4},
	{"ncsi_mac_tx_pkts_1024to1518octets", 4},
	{"ncsi_mac_tx_pkts_1519tomaxoctets", 4},
	{"ncsi_mac_tx_underrun", 4},
	{"ncsi_mac_tx_crc_error", 4},
	{"ncsi_mac_tx_pause_frames", 4},
	{"ncsi_mac_rx_pad_pkts", 4},
	{"ncsi_mac_rx_pause_frames", 4},
};

const struct hns3_reg_info dfx_rtc_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"lge_igu_afifo_dfx_0", 4},
	{"lge_igu_afifo_dfx_1", 4},
	{"lge_igu_afifo_dfx_2", 4},
	{"lge_igu_afifo_dfx_3", 4},
	{"lge_igu_afifo_dfx_4", 4},
	{"lge_igu_afifo_dfx_5", 4},
	{"lge_igu_afifo_dfx_6", 4},
	{"lge_igu_afifo_dfx_7", 4},
	{"lge_egu_afifo_dfx_0", 4},
	{"lge_egu_afifo_dfx_1", 4},
	{"lge_egu_afifo_dfx_2", 4},
	{"lge_egu_afifo_dfx_3", 4},
	{"lge_egu_afifo_dfx_4", 4},
	{"lge_egu_afifo_dfx_5", 4},
	{"lge_egu_afifo_dfx_6", 4},
	{"lge_egu_afifo_dfx_7", 4},
	{"cge_igu_afifo_dfx_0", 4},
	{"cge_igu_afifo_dfx_1", 4},
	{"cge_egu_afifo_dfx_0", 4},
	{"cge_egu_afifo_dfx_1", 4},
	{HNS3_REG_RSV_NAME, 12},
};

const struct hns3_reg_info dfx_ppp_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"drop_from_prt_pkt_cnt", 4},
	{"drop_from_host_pkt_cnt", 4},
	{"drop_tx_vlan_proc_cnt", 4},
	{"drop_mng_cnt", 4},
	{"drop_fd_cnt", 4},
	{"drop_no_dst_cnt", 4},
	{"drop_mc_mbid_full_cnt", 4},
	{"drop_sc_filtered", 4},
	{"ppp_mc_drop_pkt_cnt", 4},
	{"drop_pt_cnt", 4},
	{"drop_mac_anti_spoof_cnt", 4},
	{"drop_ig_vfv_cnt", 4},
	{"drop_ig_prtv_cnt", 4},
	{"drop_cnm_pfc_pause_cnt", 4},
	{"drop_torus_tc_cnt", 4},
	{"drop_torus_lpbk_cnt", 4},
	{"ppp_hfs_sts", 4},
	{"ppp_mc_rslt_sts", 4},
	{"ppp_p3u_sts", 4},
	{HNS3_REG_RSV_NAME, 4},
	{"ppp_umv_sts_0", 4},
	{"ppp_umv_sts_1", 4},
	{"ppp_vfv_sts", 4},
	{"ppp_gro_key_cnt", 4},
	{"ppp_gro_info_cnt", 4},
	{"ppp_gro_drop_cnt", 4},
	{"ppp_gro_out_cnt", 4},
	{"ppp_gro_key_match_data_cnt", 4},
	{"ppp_gro_key_match_tcam_cnt", 4},
	{"ppp_gro_info_match_cnt", 4},
	{"ppp_gro_free_entry_cnt", 4},
	{"ppp_gro_inner_dfx_signal", 4},
	{HNS3_REG_RSV_NAME, 12},
	{"get_rx_pkt_cnt", 8},
	{"get_tx_pkt_cnt", 8},
	{"send_uc_prt2host_pkt_cnt", 8},
	{"send_uc_prt2prt_pkt_cnt", 8},
	{"send_uc_host2host_pkt_cnt", 8},
	{"send_uc_host2prt_pkt_cnt", 8},
	{"send_mc_from_prt_cnt", 8},
	{"send_mc_from_host_cnt", 8},
	{"ssu_mc_rd_cnt", 8},
	{"ssu_mc_drop_cnt", 8},
	{"ssu_mc_rd_pkt_cnt", 8},
	{"ppp_mc_2host_pkt_cnt", 8},
	{"ppp_mc_2prt_pkt_cnt", 8},
	{"ntsnos_pkt_cnt", 8},
	{"ntup_pkt_cnt", 8},
	{"ntlcl_pkt_cnt", 8},
	{"nttgt_pkt_cnt", 8},
	{"rtns_pkt_cnt", 8},
	{"rtlpbk_pkt_cnt", 8},
	{"nr_pkt_cnt", 8},
	{"rr_pkt_cnt", 8},
	{"mng_tbl_hit_cnt", 8},
	{"fd_tbl_hit_cnt", 8},
	{"fd_lkup_cnt", 8},
	{"bc_hit_cnt", 8},
	{"um_tbl_uc_hit_cnt", 8},
	{"um_tbl_mc_hit_cnt", 8},
	{"um_tbl_snq_hit_cnt", 8},
	{HNS3_REG_RSV_NAME, 8},
	{"fwd_bonding_hit_cnt", 8},
	{"promis_tbl_hit_cnt", 8},
	{"get_tunl_pkt_cnt", 8},
	{"get_bmc_pkt_cnt", 8},
	{"send_uc_prt2bmc_pkt_cnt", 8},
	{"send_uc_host2bmc_pkt_cnt", 8},
	{"send_uc_bmc2host_pkt_cnt", 8},
	{"send_uc_bmc2prt_pkt_cnt", 8},
	{"ppp_mc_2bmc_pkt_cnt", 8},
	{HNS3_REG_RSV_NAME, 24},
	{"rx_default_host_hit_cnt", 8},
	{"lan_pair_cnt", 8},
	{"um_tbl_mc_hit_pkt_cnt", 8},
	{"mta_tbl_hit_pkt_cnt", 8},
	{"promis_tbl_hit_pkt_cnt", 8},
	{HNS3_REG_RSV_NAME, 16},
};

const struct hns3_reg_info dfx_rcb_regs[] = {
	{HNS3_REG_RSV_NAME, 4},
	{"fsm_dfx_st0", 4},
	{"fsm_dfx_st1", 4},
	{"fsm_dfx_st2", 4},
	{"fifo_dfx_st0", 4},
	{"fifo_dfx_st1", 4},
	{"fifo_dfx_st2", 4},
	{"fifo_dfx_st3", 4},
	{"fifo_dfx_st4", 4},
	{"fifo_dfx_st5", 4},
	{"fifo_dfx_st6", 4},
	{"fifo_dfx_st7", 4},
	{"fifo_dfx_st8", 4},
	{"fifo_dfx_st9", 4},
	{"fifo_dfx_st10", 4},
	{"fifo_dfx_st11", 4},
	{"q_credit_vld_0", 4},
	{"q_credit_vld_1", 4},
	{"q_credit_vld_2", 4},
	{"q_credit_vld_3", 4},
	{"q_credit_vld_4", 4},
	{"q_credit_vld_5", 4},
	{"q_credit_vld_6", 4},
	{"q_credit_vld_7", 4},
	{"q_credit_vld_8", 4},
	{"q_credit_vld_9", 4},
	{"q_credit_vld_10", 4},
	{"q_credit_vld_11", 4},
	{"q_credit_vld_12", 4},
	{"q_credit_vld_13", 4},
	{"q_credit_vld_14", 4},
	{"q_credit_vld_15", 4},
	{"q_credit_vld_16", 4},
	{"q_credit_vld_17", 4},
	{"q_credit_vld_18", 4},
	{"q_credit_vld_19", 4},
	{"q_credit_vld_20", 4},
	{"q_credit_vld_21", 4},
	{"q_credit_vld_22", 4},
	{"q_credit_vld_23", 4},
	{"q_credit_vld_24", 4},
	{"q_credit_vld_25", 4},
	{"q_credit_vld_26", 4},
	{"q_credit_vld_27", 4},
	{"q_credit_vld_28", 4},
	{"q_credit_vld_29", 4},
	{"q_credit_vld_30", 4},
	{"q_credit_vld_31", 4},
	{"gro_bd_serr_cnt", 4},
	{"gro_context_serr_cnt", 4},
	{"rx_stash_cfg_serr_cnt", 4},
	{"rcb_tx_mem_serr_cnt", 4},
	{"gro_bd_merr_cnt", 4},
	{"gro_context_merr_cnt", 4},
	{"rx_stash_cfg_merr_cnt", 4},
	{"rcb_tx_mem_merr_cnt", 4},
	{HNS3_REG_RSV_NAME, 16},
};

const struct hns3_reg_info dfx_tqp_regs[] = {
	{"q_num", 4},
	{"rcb_cfg_rx_ring_tail", 4},
	{"rcb_cfg_rx_ring_head", 4},
	{"rcb_cfg_rx_ring_fbdnum", 4},
	{"rcb_cfg_rx_ring_offset", 4},
	{"rcb_cfg_rx_ring_fbdoffset", 4},
	{"rcb_cfg_rx_ring_pktnum_record", 4},
	{"rcb_cfg_tx_ring_tail", 4},
	{"rcb_cfg_tx_ring_head", 4},
	{"rcb_cfg_tx_ring_fbdnum", 4},
	{"rcb_cfg_tx_ring_offset", 4},
	{"rcb_cfg_tx_ring_ebdnum", 4},
};

const struct hns3_reg_info dfx_ssu_2_regs[] = {
	{"oq_index", 4},
	{"queue_cnt", 4},
	{HNS3_REG_RSV_NAME, 16},
};

const struct hns3_reg_info vf_cmdq_regs[] = {
	{"comm_nic_csq_baseaddr_l", 4},
	{"comm_nic_csq_baseaddr_h", 4},
	{"comm_nic_csq_depth", 4},
	{"comm_nic_csq_tail", 4},
	{"comm_nic_csq_head", 4},
	{"comm_nic_crq_baseaddr_l", 4},
	{"comm_nic_crq_baseaddr_h", 4},
	{"comm_nic_crq_depth", 4},
	{"comm_nic_crq_tail", 4},
	{"comm_nic_crq_head", 4},
	{"comm_vector0_cmdq_src", 4},
	{"comm_vector0_cmdq_state", 4},
	{"comm_cmdq_intr_en", 4},
	{"comm_cmdq_intr_gen", 4},
};

const struct hns3_reg_info vf_common_regs[] = {
	{"misc_vector_base", 4},
	{"rst_ing", 4},
	{"gro_en", 4},
};

const struct hns3_reg_info vf_ring_regs[] = {
	{"ring_rx_addr_l", 4},
	{"ring_rx_addr_h", 4},
	{"ring_rx_bd_num", 4},
	{"ring_rx_bd_length", 4},
	{"ring_rx_merge_en", 4},
	{"ring_rx_tail", 4},
	{"ring_rx_head", 4},
	{"ring_rx_fbd_num", 4},
	{"ring_rx_offset", 4},
	{"ring_rx_fbd_offset", 4},
	{"ring_rx_stash", 4},
	{"ring_rx_bd_err", 4},
	{"ring_tx_addr_l", 4},
	{"ring_tx_addr_h", 4},
	{"ring_tx_bd_num", 4},
	{"ring_tx_priority", 4},
	{"ring_tx_tc", 4},
	{"ring_tx_merge_en", 4},
	{"ring_tx_tail", 4},
	{"ring_tx_head", 4},
	{"ring_tx_fbd_num", 4},
	{"ring_tx_offset", 4},
	{"ring_tx_ebd_num", 4},
	{"ring_tx_ebd_offset", 4},
	{"ring_tx_bd_err", 4},
	{"ring_en", 4},
};

const struct hns3_reg_info vf_tqp_intr_regs[] = {
	{"tqp_intr_ctrl", 4},
	{"tqp_intr_gl0", 4},
	{"tqp_intr_gl1", 4},
	{"tqp_intr_gl2", 4},
	{"tqp_intr_rl", 4},
};

const struct hns3_regs_group pf_regs_groups[] = {
	[HNS3_TAG_CMDQ] = {"cmdq_regs", pf_cmdq_regs, ARRAY_SIZE(pf_cmdq_regs)},
	[HNS3_TAG_COMMON] = {"common_regs", pf_common_regs,
			     ARRAY_SIZE(pf_common_regs)},
	[HNS3_TAG_RING] = {"ring_regs", pf_ring_regs, ARRAY_SIZE(pf_ring_regs)},
	[HNS3_TAG_TQP_INTR] = {"tqp_intr_regs", pf_tqp_intr_regs,
			       ARRAY_SIZE(pf_tqp_intr_regs)},
	[HNS3_TAG_QUERY_32_BIT] = {"dfx_32_regs", query_32_bit_regs,
				   ARRAY_SIZE(query_32_bit_regs)},
	[HNS3_TAG_QUERY_64_BIT] = {"dfx_64_regs", query_64_bit_regs,
				   ARRAY_SIZE(query_64_bit_regs)},
	[HNS3_TAG_DFX_BIOS_COMMON] = {"dfx_bios_common_regs",
				      dfx_bios_common_regs,
				      ARRAY_SIZE(dfx_bios_common_regs)},
	[HNS3_TAG_DFX_SSU_0] = {"dfx_ssu_0_regs", dfx_ssu_0_regs,
				ARRAY_SIZE(dfx_ssu_0_regs)},
	[HNS3_TAG_DFX_SSU_1] = {"dfx_ssu_1_regs", dfx_ssu_1_regs,
				ARRAY_SIZE(dfx_ssu_1_regs)},
	[HNS3_TAG_DFX_IGU_EGU] = {"dfx_igu_egu_regs", dfx_igu_egu_regs,
				  ARRAY_SIZE(dfx_igu_egu_regs)},
	[HNS3_TAG_DFX_RPU_0] = {"dfx_rpu_0_regs", dfx_rpu_0_regs,
				ARRAY_SIZE(dfx_rpu_0_regs)},
	[HNS3_TAG_DFX_RPU_1] = {"dfx_rpu_1_regs", dfx_rpu_1_regs,
				ARRAY_SIZE(dfx_rpu_1_regs)},
	[HNS3_TAG_DFX_NCSI] = {"dfx_ncsi_regs", dfx_ncsi_regs,
			       ARRAY_SIZE(dfx_ncsi_regs)},
	[HNS3_TAG_DFX_RTC] = {"dfx_rtc_regs", dfx_rtc_regs,
			      ARRAY_SIZE(dfx_rtc_regs)},
	[HNS3_TAG_DFX_PPP] = {"dfx_ppp_regs", dfx_ppp_regs,
			      ARRAY_SIZE(dfx_ppp_regs)},
	[HNS3_TAG_DFX_RCB] = {"dfx_rcb_regs", dfx_rcb_regs,
			      ARRAY_SIZE(dfx_rcb_regs)},
	[HNS3_TAG_DFX_TQP] = {"dfx_tqp_regs", dfx_tqp_regs,
			      ARRAY_SIZE(dfx_tqp_regs)},
	[HNS3_TAG_DFX_SSU_2] = {"dfx_ssu_2_regs", dfx_ssu_2_regs,
				ARRAY_SIZE(dfx_ssu_2_regs)},
	[HNS3_TAG_DFX_RPU_TNL] = {"dfx_rpu_tnl", dfx_rpu_0_regs,
				  ARRAY_SIZE(dfx_rpu_0_regs)},
};

const struct hns3_regs_group vf_regs_groups[] = {
	[HNS3_TAG_CMDQ] = {"cmdq_regs", vf_cmdq_regs, ARRAY_SIZE(vf_cmdq_regs)},
	[HNS3_TAG_COMMON] = {"common_regs", vf_common_regs,
			     ARRAY_SIZE(vf_common_regs)},
	[HNS3_TAG_RING] = {"ring_regs", vf_ring_regs, ARRAY_SIZE(vf_ring_regs)},
	[HNS3_TAG_TQP_INTR] = {"tqp_intr_regs", vf_tqp_intr_regs,
			       ARRAY_SIZE(vf_tqp_intr_regs)},
};

static void hns3_dump_reg_hex(const char *regs_name, const u8 *regs_data,
			     u16 value_len, u32 name_max_len)
{
	if (strcmp(regs_name, HNS3_REG_RSV_NAME) == 0)
		return;

	fprintf(stdout, "  %-*s : ", name_max_len, regs_name);
	if (value_len == 4) /* 4 bytes register */
		fprintf(stdout, "0x%08x\n", *(u32 *)regs_data);
	if (value_len == 8) /* 8 bytes register */
		fprintf(stdout, "0x%016llx\n", *(u64 *)regs_data);
}

static u32 hns3_get_group_regs_name_max_len(const struct hns3_regs_group *group)
{
	const struct hns3_reg_info *reg;
	u32 max_len = 0;
	u16 i;

	for (i = 0; i < group->regs_count; i++) {
		reg = &group->regs[i];
		max_len = strlen(reg->name) > max_len ?
			  strlen(reg->name) : max_len;
	}

	return max_len;
}

static const char *hns3_get_group_name(const struct hns3_regs_group *group,
				       u32 tag)
{
	static u32 pre_tag = HNS3_TAG_MAX;
	static char group_name[256];
	static u32 index;

	if (!hns3_reg_is_repeat_tag_array[tag])
		return group->group_name;

	if (tag != pre_tag)
		index = 0;

	pre_tag = tag;
	sprintf(group_name, "%s_%u", group->group_name, index++);
	return group_name;
}

static void hns3_dump_reg_group(const struct hns3_regs_group *group, u32 tag,
			       u32 expected_len, const u8 *regs_data)
{
	u32 name_max_len = hns3_get_group_regs_name_max_len(group);
	const struct hns3_reg_info *reg;
	u32 dump_offset = 0;
	u16 i;

	fprintf(stdout, "[%s]\n", hns3_get_group_name(group, tag));
	for (i = 0; i < group->regs_count && dump_offset < expected_len; i++) {
		reg = &group->regs[i];
		hns3_dump_reg_hex(reg->name, regs_data + dump_offset,
				  reg->value_len, name_max_len);
		dump_offset += reg->value_len;
	}

	/* the driver may add new register.
	 * In this case, the register name is unknown.
	 * The register can be parsed as unknown:value format.
	 */
	while (dump_offset < expected_len) {
		hns3_dump_reg_hex(HNS3_REG_UNKNOW_NAME, regs_data + dump_offset,
				  HNS3_REG_UNKNOW_VALUE_LEN, name_max_len);
		dump_offset += HNS3_REG_UNKNOW_VALUE_LEN;
	}
}

static void hns3_dump_as_groups(const struct hns3_regs_group *groups,
				const u8 *regs_data, u32 regs_len)
{
	u32 tlv_size = sizeof(struct hns3_reg_tlv);
	const struct hns3_reg_tlv *tlv;
	u32 dump_offset = 0;

	while (dump_offset < regs_len) {
		tlv = (const struct hns3_reg_tlv *)(regs_data + dump_offset);
		hns3_dump_reg_group(&groups[tlv->tag], tlv->tag,
				    tlv->len - tlv_size,
				    regs_data + dump_offset + tlv_size);
		dump_offset += tlv->len;
	}
}

static bool hns3_dump_validate(const u8 *regs_data, u32 regs_len)
{
	u32 tlv_size = sizeof(struct hns3_reg_tlv);
	const struct hns3_reg_tlv *tlv;
	u32 dump_offset = 0;

	while (dump_offset < regs_len) {
		tlv = (const struct hns3_reg_tlv *)(regs_data + dump_offset);

		/* register value length is 4 bytes or 8 bytes */
		if ((tlv->len - tlv_size) % 4)
			return false;

		if (tlv->tag >= HNS3_TAG_MAX)
			return false;

		dump_offset += tlv->len;
	}

	return dump_offset == regs_len;
}

int hns3_dump_regs(struct ethtool_drvinfo *info __maybe_unused,
		   struct ethtool_regs *regs)
{
	const struct hns3_regs_group *groups = pf_regs_groups;
	u32 header_len = sizeof(struct hns3_reg_header);
	const struct hns3_reg_header *header;

	/* must contain header and register data */
	if (regs->len <= header_len)
		return -ENODATA;

	header = (struct hns3_reg_header *)regs->data;
	if (header->magic_number != HNS3_REG_MAGIC_NUMBER)
		return -EOPNOTSUPP;

	if (!hns3_dump_validate(regs->data + header_len,
				regs->len - header_len))
		return -EINVAL;

	if (header->is_vf)
		groups = vf_regs_groups;

	hns3_dump_as_groups(groups, regs->data + header_len,
			    regs->len - header_len);
	return 0;
}
