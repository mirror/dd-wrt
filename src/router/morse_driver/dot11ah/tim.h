#ifndef _MORSE_DOT11AH_TIM_H_
#define _MORSE_DOT11AH_TIM_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/types.h>
#include <linux/ieee80211.h>

/**
 * S1G TIM definitions
 *
 * These flags are used with the @flags member of &ieee80211_tx_info.
 *
 * @IEEE80211_S1G_TIM_BLOCK_CTL_ENC_MODE: encoding mode
 * @IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP: inverse bitmap
 * @IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET: block offset mask
 * @IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE: S1G bitcamp control page slice mask
 * @IEEE80211_S1G_TIM_BITMAP_PAGE_INDEX: S1G bitmap control page index mask
 */
#define IEEE80211_S1G_TIM_BLOCK_CTL_ENC_MODE		GENMASK(1, 0)
#define IEEE80211_S1G_TIM_BLOCK_CTL_ENC_MODE_SHIFT	(0)

#define IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP	GENMASK(2, 2)
#define IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP_SHIFT	(2)

#define IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET	GENMASK(7, 3)
#define IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET_SHIFT	(3)

#define IEEE80211_TIM_BITMAP_TRAFFIC_INDICATION		GENMASK(0, 0)
#define IEEE80211_TIM_BITMAP_TRAFFIC_INDICATION_SHIFT	(0)

#define IEEE80211_TIM_BITMAP_OFFSET			GENMASK(7, 1)
#define IEEE80211_TIM_BITMAP_OFFSET_SHIFT		(1)

#define IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE		GENMASK(5, 1)
#define IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE_SHIFT	(1)

#define IEEE80211_S1G_TIM_BITMAP_PAGE_INDEX		GENMASK(7, 6)
#define IEEE80211_S1G_TIM_BITMAP_PAGE_INDEX_SHIFT	(6)

/* Block offset field comes from bits 6-10 in the AID (ie. AID[6:10]) */
#define S1G_TIM_AID_TO_BLOCK_OFFSET(_aid)		(((_aid) & GENMASK(10, 6)) >> 6)

/* 11n tim */
#define DOT11_MAX_TIM_VIRTUAL_MAP_LENGTH		(251)

#define S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK			(8)

#define S1G_TIM_NUM_AID_PER_SUBBLOCK			(8)
#define S1G_TIM_NUM_AID_PER_BLOCK \
				(S1G_TIM_NUM_AID_PER_SUBBLOCK * S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK)

#define S1G_TIM_MAX_BLOCK_SIZE				(256)

/* TIM element shall have the page slice number 31 when the entire page indicated by the
 * Page Index subfield is encoded in the TIM element.
 */
#define S1G_TIM_PAGE_SLICE_ENTIRE_PAGE			(31)

enum dot11ah_tim_encoding_mode {
	ENC_MODE_BLOCK = 0x00,
	ENC_MODE_AID = 0x01,
	ENC_MODE_OLB = 0x02,
	ENC_MODE_ADE = 0x03,
	ENC_MODE_UNKNOWN = 0xFF
};

struct dot11ah_s1g_tim_ie {
	u8 dtim_count;
	u8 dtim_period;
	u8 bitmap_control;
	/* TODO: for now set it to max expected length, later should alloc dynamically */
	u8 encoded_block_info[S1G_TIM_MAX_BLOCK_SIZE];
} __packed;

/**
 * morse_dot11_tim_to_s1g() - convert non S1G TIM to S1G TIM
 *
 * @s1g_tim: pointer to S1G TIM (after conversion).
 * @tim: pointer to 11n TIM element data.
 * @tim_virtual_map_length: length of TIM partial virtual bitmap.
 * @enc_mode: TIM encoding mode.
 * @inverse_bitmap: inverse mode.
 * @max_aid: Largest AID currently in use by associated STA.
 * @page_slice_no: Number of page slice belonging to a page included in TIM.
 * @page_index: Index of the page being included in the TIM.
 *
 * Return: The length of the S1G TIM element.
 */
int morse_dot11_tim_to_s1g(struct dot11ah_s1g_tim_ie *s1g_tim,
			   const struct ieee80211_tim_ie *tim,
			   u8 tim_virtual_map_length,
			   enum dot11ah_tim_encoding_mode enc_mode,
			   bool inverse_bitmap,
			   u16 max_aid,
			   u8 page_slice_no,
			   u8 page_index);

int morse_dot11_s1g_to_tim(struct ieee80211_tim_ie *tim, const struct dot11ah_s1g_tim_ie *s1g_tim,
			   size_t total_len);

/**
 * morse_dot11ah_insert_s1g_tim() - translate to S1G TIM and insert into ies_mask
 *
 * @vif: The VIF the IE was received on.
 * @ies_mask: Contains array of information elements.
 * @page_slice_no: Number of page slice belonging to a page included in TIM.
 * @page_index: Index of the page being served in the TIM.
 *
 * Translate ies_mask existing 802.11n TIM element into S1G one and insert it
 * back into the ies_mask after cleaning the original 802.11n TIM element.
 */
void morse_dot11ah_insert_s1g_tim(struct ieee80211_vif *vif,
				  struct dot11ah_ies_mask *ies_mask,
				  u8 page_slice_no,
				  u8 page_index);

#endif  /* !_MORSE_DOT11AH_TIM_H_ */
