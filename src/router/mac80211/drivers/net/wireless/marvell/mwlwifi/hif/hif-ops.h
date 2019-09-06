/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file defines host interface related operations. */

#ifndef _HIF_OPS_H_
#define _HIF_OPS_H_
static inline const char *mwl_hif_get_driver_name(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	return priv->hif.ops->driver_name;
}

static inline const char *mwl_hif_get_driver_version(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	return priv->hif.ops->driver_version;
}

static inline unsigned int mwl_hif_get_tx_head_room(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	return priv->hif.ops->tx_head_room;
}

static inline unsigned int mwl_hif_get_ampdu_num(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	return priv->hif.ops->ampdu_num;
}

static inline void mwl_hif_reset(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->reset)
		priv->hif.ops->reset(hw);
}

static inline int mwl_hif_init(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->init)
		return priv->hif.ops->init(hw);
	else
		return -ENOTSUPP;
}

static inline void mwl_hif_deinit(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->deinit)
		priv->hif.ops->deinit(hw);
}

static inline int mwl_hif_get_info(struct ieee80211_hw *hw,
				   char *buf, size_t size)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->get_info)
		return priv->hif.ops->get_info(hw, buf, size);
	else
		return 0;
}

static inline int mwl_hif_get_tx_status(struct ieee80211_hw *hw,
					char *buf, size_t size)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->get_tx_status)
		return priv->hif.ops->get_tx_status(hw, buf, size);
	else
		return 0;
}

static inline int mwl_hif_get_rx_status(struct ieee80211_hw *hw,
					char *buf, size_t size)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->get_rx_status)
		return priv->hif.ops->get_rx_status(hw, buf, size);
	else
		return 0;
}

static inline void mwl_hif_enable_data_tasks(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->enable_data_tasks)
		priv->hif.ops->enable_data_tasks(hw);
}

static inline void mwl_hif_disable_data_tasks(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->disable_data_tasks)
		priv->hif.ops->disable_data_tasks(hw);
}

static inline int mwl_hif_exec_cmd(struct ieee80211_hw *hw, unsigned short cmd)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->exec_cmd)
		return priv->hif.ops->exec_cmd(hw, cmd);
	else
		return -ENOTSUPP;
}

static inline int mwl_hif_get_irq_num(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->get_irq_num)
		return priv->hif.ops->get_irq_num(hw);
	else
		return -ENOTSUPP;
}

static inline irqreturn_t mwl_hif_irq_handler(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->irq_handler)
		return priv->hif.ops->irq_handler(hw);
	else
		return -ENOTSUPP;
}

static inline void mwl_hif_irq_enable(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->irq_enable)
		priv->hif.ops->irq_enable(hw);
}

static inline void mwl_hif_irq_disable(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->irq_disable)
		priv->hif.ops->irq_disable(hw);
}

static inline int mwl_hif_download_firmware(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->download_firmware)
		return priv->hif.ops->download_firmware(hw);
	else
		return -ENOTSUPP;
}

static inline void mwl_hif_timer_routine(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->timer_routine)
		priv->hif.ops->timer_routine(hw);
}

static inline void mwl_hif_tx_xmit(struct ieee80211_hw *hw,
				   struct ieee80211_tx_control *control,
				   struct sk_buff *skb)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->tx_xmit)
		priv->hif.ops->tx_xmit(hw, control, skb);
}

static inline void mwl_hif_tx_del_pkts_via_vif(struct ieee80211_hw *hw,
					       struct ieee80211_vif *vif)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->tx_del_pkts_via_vif)
		priv->hif.ops->tx_del_pkts_via_vif(hw, vif);
}

static inline void mwl_hif_tx_del_pkts_via_sta(struct ieee80211_hw *hw,
					       struct ieee80211_sta *sta)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->tx_del_pkts_via_sta)
		priv->hif.ops->tx_del_pkts_via_sta(hw, sta);
}

static inline void mwl_hif_tx_del_ampdu_pkts(struct ieee80211_hw *hw,
					     struct ieee80211_sta *sta, u8 tid)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->tx_del_ampdu_pkts)
		priv->hif.ops->tx_del_ampdu_pkts(hw, sta, tid);
}

static inline void mwl_hif_tx_del_sta_amsdu_pkts(struct ieee80211_hw *hw,
						 struct ieee80211_sta *sta)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->tx_del_sta_amsdu_pkts)
		priv->hif.ops->tx_del_sta_amsdu_pkts(hw, sta);
}

static inline void mwl_hif_tx_return_pkts(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->tx_return_pkts)
		priv->hif.ops->tx_return_pkts(hw);
}

static inline struct device_node *mwl_hif_device_node(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->get_device_node)
		return priv->hif.ops->get_device_node(hw);
	else
		return NULL;
}

static inline void mwl_hif_get_survey(struct ieee80211_hw *hw,
				      struct mwl_survey_info *survey_info)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->get_survey)
		priv->hif.ops->get_survey(hw, survey_info);
}

static inline int mwl_hif_reg_access(struct ieee80211_hw *hw, bool write)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->reg_access)
		return priv->hif.ops->reg_access(hw, write);
	else
		return -ENOTSUPP;
}

static inline void mwl_hif_set_sta_id(struct ieee80211_hw *hw,
				      struct ieee80211_sta *sta,
				      bool sta_mode, bool set)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->set_sta_id)
		priv->hif.ops->set_sta_id(hw, sta, sta_mode, set);
}

static inline void mwl_hif_process_account(struct ieee80211_hw *hw)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->process_account)
		priv->hif.ops->process_account(hw);
}

static inline int mwl_hif_mcast_cts(struct ieee80211_hw *hw, bool enable)
{
	struct mwl_priv *priv = hw->priv;

	if (priv->hif.ops->mcast_cts)
		return priv->hif.ops->mcast_cts(hw, enable);
	else
		return -ENOTSUPP;
}
#endif /* _HIF_OPS_H_ */
