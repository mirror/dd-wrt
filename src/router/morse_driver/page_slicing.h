/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _MORSE_PAGE_SLICING_H_
#define _MORSE_PAGE_SLICING_H_

#include <linux/types.h>

/**
 * Number of blocks(maximum) in a page.
 */
#define NUMBER_OF_BLOCKS_PER_PAGE (32)

/**
 * Each bit in a byte represents a block and it requires 4 bytes to represent all 32
 * blocks of a page. It requires 1 byte to advertise 8 blocks in page bitmap.
 */
#define PAGE_BITMAP_NUMBER_OF_BLOCKS_PER_BYTE (8)

/* Bitmap for page slice control fields. Page slice control field format is specified in
 * section 9.4.2.192 Page Slice element of IEEE802.11-2020.
 */
#define PAGE_SLICE_CONTROL_PAGE_INDEX GENMASK(1, 0)
#define PAGE_SLICE_CONTROL_PAGE_SLICE_LENGTH GENMASK(6, 2)
#define PAGE_SLICE_CONTROL_PAGE_SLICE_COUNT GENMASK(11, 7)
#define PAGE_SLICE_CONTROL_BLOCK_OFFSET GENMASK(8, 4)
#define PAGE_SLICE_CONTROL_TIM_OFFSET GENMASK(4, 1)

struct page_slicing {
    /**
     * Page slicing enabled or not.
     */
	bool enabled;

    /**
     * Copy of the TIM information received from mac80211 as part of
     * ieee80211_beacon_get.
     */
	u8 tim_virtual_map[DOT11_MAX_TIM_VIRTUAL_MAP_LENGTH];

    /**
     * Length of the TIM virtual map taken from DTIM beacon. It is updated
     * after a TIM slice is inluded in every beacon.
     */
	u8 tim_virtual_map_len;

    /**
     * TIM Bitmap control offset(points to first octet) TIM bitmap from mac80211.
     */
	u8 tim_bitmap_ctrl_offset;

    /**
     * It points to the index of tim virtual bitmap for current TIM page slice.
     */
	u8 tim_virtual_map_index;

    /**
     * Indicates number of beacon intervals between successive beacons that carry
     * the page slice element for the associated page.
     */
	u8 page_period;

    /**
     * The minimum number of blocks included in each TIM for the associated page slice.
     * The final TIM in a set of page slices may contain more depending on the number of blocks
     * (see section 9.4.2.192 Page Slice element in IEEE802.11-2020).
     */
	u8 page_slice_length;

    /**
     * Indicates number of TIMs scheduled in one page period.
     */
	u8 page_slice_count;

    /**
     * Indicates the index of the current page being scheduled during beacon intervals within
     * a page period.
     */
	u8 page_index;

    /**
     * Indicates offset of the block in the first page slice from the first block
     * in the page assigned within the page period.
     */
	u8 block_offset;

    /**
     * Indicates number of beacon intervals from DTIM beacon frame to the beacon that
     * carries the first page slice element of a page to the beacon that carries
     * first page slice of the page indicated in page slice element (DTIM Beacon).
     */
	u8 tim_offset;

    /**
     * Page slice number to schedule at next TBTT.
     */
	u8 page_slice_no;

    /**
     * Total Number of blocks scheduled in page period
     */
	u8 total_number_of_blocks;

    /**
     * Indicates blocks that are scheduled in the page period
     */
	u32 page_bitmap;
};

/* Page slice element - fields format is specified in section 9.4.2.192 Page Slice element
 * of IEEE802.11-2020.
 */
struct page_slice_element {
    /**
     * Number of beacon intervals between successive beacons that carry page slice
     * element for associated page.
     */
	u8 page_period;

    /**
     * page slice control indicates page index, page slice length, page slice count,
     * block offset and TIM offset.
     */
	u8 page_slice_control[3];

    /**
     * Indicates presence of buffered data for each of the one or more blocks in
     * a page alice or all the assigned page slices within a page period.
     */
	u8 page_bitmap[];
} __packed;

/**
 * morse_page_slicing_process_tim_element() - Process tim element for page slicing
 *
 * @vif: pointer to vif
 * @ies_mask: contains array of information elements from beacon buffer.
 * @page_slice_no: Number of page slice belonging to a page being included in TIM element.
 * @page_index: Index of the page being included in the TIM.
 */
void morse_page_slicing_process_tim_element(struct ieee80211_vif *vif,
					    struct dot11ah_ies_mask *ies_mask,
					    u8 *page_slice_no,
					    u8 *page_index);

/**
 * morse_page_slicing_init() - Initialize page slicing configuration.
 *
 * @vif: pointer vif
 * @dtim_period: dtim interval configuration from mac80211.
 * @enable: Flag to enable or disable page slicing.
 */
void morse_page_slicing_init(struct ieee80211_vif *vif, u8 dtim_period, u8 enable);
#endif /* !_MORSE_PAGE_SLICING_H_*/
