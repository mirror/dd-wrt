/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "morse.h"
#include <linux/ieee80211.h>
#include "debug.h"

/**
 * morse_page_slice_control_set_page_index() - sets page index in page control field.
 *
 * @control: pointer to page control field in page slice element
 * @val: value of page index field to be set in page control.
 */
static inline void morse_page_slice_control_set_page_index(u8 *control, u8 val)
{
	*control |= ((*control & ~PAGE_SLICE_CONTROL_PAGE_INDEX) |
					BMSET(val, PAGE_SLICE_CONTROL_PAGE_INDEX));
}

/**
 * morse_page_slice_control_set_page_slice_length() - sets page slice length in page control field.
 *
 * @control: pointer to page control field in page slice element
 * @val: value of page slice length field to be set in page control.
 */
static inline void morse_page_slice_control_set_page_slice_length(u8 *control, u8 val)
{
	*control |= ((*control & ~PAGE_SLICE_CONTROL_PAGE_SLICE_LENGTH) |
					BMSET(val, PAGE_SLICE_CONTROL_PAGE_SLICE_LENGTH));
}

/**
 * morse_page_slice_control_set_page_slice_count() - sets page slice count in page control field.
 *
 * @control: pointer to page control field in page slice element
 * @val: value of page slice count field to be set in page control.
 */
static inline void morse_page_slice_control_set_page_slice_count(u8 *control, u8 val)
{
	*(u16 *)control |= ((*(u16 *)control & ~PAGE_SLICE_CONTROL_PAGE_SLICE_COUNT) |
					BMSET(val, PAGE_SLICE_CONTROL_PAGE_SLICE_COUNT));
}

/**
 * morse_page_slice_control_set_block_offset() - sets block offset in page control field.
 *
 * @control: pointer to page control field in page slice element
 * @val: value of block offset field to be set in page control.
 */
static inline void morse_page_slice_control_set_block_offset(u8 *control, u8 val)
{
	*(u16 *)control |= ((*(u16 *)control & ~PAGE_SLICE_CONTROL_BLOCK_OFFSET) |
					BMSET(val, PAGE_SLICE_CONTROL_BLOCK_OFFSET));
}

/**
 * morse_page_slice_control_set_tim_offset() - sets tim offset in page control field.
 *
 * @control: pointer to page control field in page slice element
 * @val: value of tim offset field to be set in page control.
 */
static inline void morse_page_slice_control_set_tim_offset(u8 *control, u8 val)
{
	*control |= ((*control & ~PAGE_SLICE_CONTROL_TIM_OFFSET) |
					BMSET(val, PAGE_SLICE_CONTROL_TIM_OFFSET));
}

/**
 * morse_insert_page_slice_element() - Inserts page slicing element into ies_mask.
 *
 * @mors_vif: pointer to driver's virtual interface
 * @ies_mask: array of pointers to information elements
 */
static void morse_insert_page_slice_element(struct morse_vif *mors_vif,
					    struct dot11ah_ies_mask *ies_mask)
{
	struct ie_element *element;
	u32 element_size;
	struct page_slice_element *page_slice_elem;
	struct page_slicing *page_slicing_data = &mors_vif->page_slicing_info;
	u32 page_bitmap = 0;
	u32 page_bitmap_size;
	bool update_bitmap = false;
	u8 block_idx = page_slicing_data->tim_bitmap_ctrl_offset / S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;
	u8 offset = 0;
	u8 last_block_idx;
	u8 no_of_blocks = 0;
	u8 page_period = page_slicing_data->page_period;
	u8 page_bitmap_byte_offset = block_idx / PAGE_BITMAP_NUMBER_OF_BLOCKS_PER_BYTE;
	u8 page_bitmap_first_block = page_bitmap_byte_offset *
						PAGE_BITMAP_NUMBER_OF_BLOCKS_PER_BYTE;

	last_block_idx = (page_slicing_data->tim_bitmap_ctrl_offset +
		page_slicing_data->tim_virtual_map_len - 1) / S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;

	/* Update page bitmap by going through the every block i.e, 64 AIDs
	 * (8 sub blocks - 8 octets).
	 */
	for (; block_idx <= last_block_idx; block_idx++) {
		/* First & last octet(i.e., 1st and last block) of TIM PVB will have traffic
		 * buffered for atleast one STA. So set the corresponding bit for these two
		 * blocks in page bitmap.
		 */
		if (block_idx == page_slicing_data->block_offset ||
			block_idx == last_block_idx) {
			update_bitmap = true;
			/* mac80211's TIM PVB may not have all 8 octets (subblocks) of 1st block.
			 * So increase offset based on the bitmap offset.
			 */
			if (block_idx == page_slicing_data->block_offset)
				offset = (block_idx + 1) * S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK -
							page_slicing_data->tim_bitmap_ctrl_offset;
		} else {
			/* set the block in the page bitmap even if one AID (belonging to the block)
			 * is set.
			 */
			if (*(u64 *)(page_slicing_data->tim_virtual_map + offset))
				update_bitmap = true;

			/* Move to next block */
			offset += S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;
		}
		if (update_bitmap) {
			page_bitmap |= 1 << (block_idx - page_bitmap_first_block);
			no_of_blocks++;
		}
		/* Reset update flag */
		update_bitmap = false;
	}

	/* Store the number of blocks to be scheduled during the page period */
	page_slicing_data->total_number_of_blocks = no_of_blocks;

	/* Calculate how many page slices(count) would be required in this page period
	 * based on the number of octets in TIM partial virtual bitmap.
	 * 1 Page contains 32 blocks,
	 * 1 Block contains 8 subblocks,
	 * 1 subblock represents 8 AIDs (8 bits) i.e, one octet in TIM partial virtual
	 * bitmap.
	 * Page slice length indicates number of blocks inlcuded in each TIM/Page slice.
	 */
	page_slicing_data->page_slice_length = no_of_blocks / page_period;
	if (!page_slicing_data->page_slice_length) {
		/* Schedule 1 block per slice if number blocks are less than page period */
		page_slicing_data->page_slice_length = 1;
		page_slicing_data->page_slice_count = no_of_blocks;
	} else {
		page_slicing_data->page_slice_count = page_period;
	}

	/* Calculate the page bitmap size based on the number of blocks to indicate in the
	 * page slice element.
	 */
	page_bitmap_size = (last_block_idx - page_slicing_data->block_offset) /
							PAGE_BITMAP_NUMBER_OF_BLOCKS_PER_BYTE + 1;
	element_size = sizeof(*page_slice_elem) + page_bitmap_size;
	element = morse_dot11_ies_create_ie_element(ies_mask,
						    WLAN_EID_PAGE_SLICE,
						    element_size,
						    true,
						    true);

	if (!element) {
		/* Reset the length */
		page_slicing_data->tim_virtual_map_len = 0;
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, 1);
		return;
	}

	/* Fill in the page slice element fields */
	page_slice_elem = (struct page_slice_element *)element->ptr;
	page_slice_elem->page_period = page_period;

	/* Update page slice control fields */
	morse_page_slice_control_set_page_index(page_slice_elem->page_slice_control,
						page_slicing_data->page_index);
	morse_page_slice_control_set_page_slice_length(page_slice_elem->page_slice_control,
						       page_slicing_data->page_slice_length);
	morse_page_slice_control_set_page_slice_count(page_slice_elem->page_slice_control,
						      page_slicing_data->page_slice_count);
	morse_page_slice_control_set_block_offset(&page_slice_elem->page_slice_control[1],
						  page_slicing_data->block_offset);
	morse_page_slice_control_set_tim_offset(&page_slice_elem->page_slice_control[2],
						page_slicing_data->tim_offset);

	if (page_bitmap_size) {
		memcpy(element->ptr + sizeof(*page_slice_elem),
		       (u8 *)&page_bitmap,
		       page_bitmap_size);
		page_slicing_data->page_bitmap = page_bitmap;
	}
}

static u8 morse_page_slicing_find_next_block(struct page_slicing *page_slicing_data,
										u8 no_of_blocks)
{
	u64 block_bitmap = *(u64 *)(page_slicing_data->tim_virtual_map +
							page_slicing_data->tim_virtual_map_index);
	u8 len_in_octets = 0;

	while (!block_bitmap) {
		/* when searching for 1st block, ignore blocks without any AID set*/
		if (!no_of_blocks)
			page_slicing_data->tim_virtual_map_index += S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;
		else /* Move to next block */
			len_in_octets += S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;

		block_bitmap = *(u64 *)(page_slicing_data->tim_virtual_map +
					page_slicing_data->tim_virtual_map_index + len_in_octets);

		if (page_slicing_data->tim_virtual_map_index + len_in_octets >
				page_slicing_data->tim_virtual_map_len)
			break;
	}

	/* Update length for first block as we ignore all blocks without any aid set */
	if (!no_of_blocks)
		len_in_octets = S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;
	else
		len_in_octets += S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;
	return len_in_octets;
}

void morse_page_slicing_process_tim_element(struct ieee80211_vif *vif,
					    struct dot11ah_ies_mask *ies_mask,
					    u8 *page_slice_no,
					    u8 *page_index)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct page_slicing *page_slicing_data = &mors_vif->page_slicing_info;
	struct ieee80211_tim_ie *tim_ie =
		(struct ieee80211_tim_ie *)ies_mask->ies[WLAN_EID_TIM].ptr;
	u8 dtim_count = tim_ie->dtim_count;
	u8 dtim_period = tim_ie->dtim_period;
	u8 bitmap_ctrl = tim_ie->bitmap_ctrl;
	u8 bitmap_offset = (bitmap_ctrl & IEEE80211_TIM_BITMAP_OFFSET);
	u8 first_aid_in_block = bitmap_offset * 8;
	u8 virtual_map_len;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	u8 len_in_octets;
	u8 no_of_blocks;
	u8 tim_len = 0;
	u8 *tim_pvb = page_slicing_data->tim_virtual_map;
	u8 page_slice_count;
	u8 tim_virtual_map_len;
	u8 tim_bitmap_ctrl_offset;
	struct ie_element *element;
	u8 remaining_tim_map_len;

	/* Calculate partial virtual bitmap length. 11n TIM contains minimum of 4 octets
	 * i.e dtim count(1 octet), dtim period(1 octet), bitmap ctrl(1 octet) and PVB[1].
	 * If IE length is 4 then check virtual map is not zero i.e, no traffic buffered
	 * for STAs. Otherwise the actual PVB size is IE length - 3.
	 */
	virtual_map_len = (ies_mask->ies[WLAN_EID_TIM].len == 4) ? tim_ie->virtual_map[0] != 0 :
						(ies_mask->ies[WLAN_EID_TIM].len - 3);

	/* Check if it is a DTIM or TIM beacon:
	 * DTIM Beacon: Save the partial virtual bitmap (PVB) and schedule the TIM into different
	 * slices, announce the page/TIM slicing information through page slicing element.
	 * PS Stations upon receiving the page slicing element, would calculate the target
	 * TIM beacon and schedule their wakeup accordingly.
	 *
	 * TIM Beacon: Update TIM element to include the TIM slice information as per the
	 * page slicing schedule announced in page slice element as part of DTIM beacon.
	 */
	if (!dtim_count && virtual_map_len) {
		/* Resest existing and save the new virtual bitmap */
		memset(tim_pvb,
		       0,
			   virtual_map_len);
		memcpy(tim_pvb,
		       tim_ie->virtual_map,
		       virtual_map_len);
		page_slicing_data->tim_virtual_map_len = virtual_map_len;
		page_slicing_data->tim_bitmap_ctrl_offset = bitmap_offset;
		page_slicing_data->page_slice_no = 0;
		page_slicing_data->tim_virtual_map_index = 0;

		/* Derive block offset based on the starting/first AID in the block */
		page_slicing_data->block_offset =
			S1G_TIM_AID_TO_BLOCK_OFFSET(first_aid_in_block);

		/* TIM in DTIM beacon contains the first page slice of page being scheduled */
		page_slicing_data->tim_offset = 0;

		/* Add page slice element */
		morse_insert_page_slice_element(mors_vif, ies_mask);
	}

	if (!page_slicing_data->tim_virtual_map_len) {
		/* Any traffic buffered after DTIM beacon, will be indicated only in next
		 * the DTIM interval. Update TIM length to avoid indicating PVB.
		 */
		ies_mask->ies[WLAN_EID_TIM].len = sizeof(*tim_ie) - 1;
		return;
	}

	bitmap_offset = 0;
	tim_virtual_map_len = page_slicing_data->tim_virtual_map_len;
	page_slice_count = page_slicing_data->page_slice_count;
	tim_bitmap_ctrl_offset = page_slicing_data->tim_bitmap_ctrl_offset;
	remaining_tim_map_len = (tim_virtual_map_len - page_slicing_data->tim_virtual_map_index);

	/* reallocate TIM if mac80211's TIM buffer doesn't have enough room  */
	if (virtual_map_len < remaining_tim_map_len) {
		virtual_map_len = remaining_tim_map_len;
		/* Add TIM IE size to allocate */
		remaining_tim_map_len += offsetof(struct ieee80211_tim_ie, virtual_map);
		morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_TIM);
		element = morse_dot11_ies_create_ie_element(ies_mask, WLAN_EID_TIM,
							remaining_tim_map_len, true, true);
		if (!element) {
			MORSE_ERR(mors, "Failed to allocate memory for TIM IE, len=%u\n",
								tim_virtual_map_len);
			return;
		}
		tim_ie = (struct ieee80211_tim_ie *)element->ptr;
		tim_ie->dtim_count = dtim_count;
		tim_ie->dtim_period = dtim_period;
	} else {
		/* Reset the pvb & bitmap control offset for page slice TIM */
		memset(tim_ie->virtual_map, 0, virtual_map_len);
		tim_ie->bitmap_ctrl = 0;
	}

	/* Update TIM element with page slice information that is being scheduled in the beacon
	 * going to be transmitted.
	 */
	*page_slice_no = page_slicing_data->page_slice_no++;

	for (no_of_blocks = 0; no_of_blocks < page_slicing_data->page_slice_length;
									no_of_blocks++) {
		/* Reset the length */
		len_in_octets = 0;

		/* First page slice - starting block will have atleast one AID set and
		 * TIM PVB may not have all sub blocks of starting block. So, calculate
		 * the number of subblocks to copy based on the bitmap offset.
		 */
		if (page_slicing_data->page_slice_no == 1 && !no_of_blocks) {
			len_in_octets = (page_slicing_data->block_offset + 1) *
				S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK - tim_bitmap_ctrl_offset;
			/* Copy only vmap_len of bytes if whole block is not present */
			if (len_in_octets > tim_virtual_map_len)
				len_in_octets = tim_virtual_map_len;
		} else if (page_slicing_data->page_slice_no == page_slice_count) {
			/* Copy remaining bitmap in last slice */
			len_in_octets = tim_virtual_map_len -
						page_slicing_data->tim_virtual_map_index;
		} else {
			/* Go through the bitmap until we find a block with atleast 1 AID
			 * set.
			 */
			len_in_octets =
				morse_page_slicing_find_next_block(page_slicing_data,
									no_of_blocks);
		}

		if ((tim_len + len_in_octets) > virtual_map_len) {
			/* Reset the virtual map len and index */
			page_slicing_data->tim_virtual_map_len = 0;
			page_slicing_data->tim_virtual_map_index = 0;
			break;
		}

		/* Copy the TIM Slice information into TIM partial virtual bitmap */
		memcpy(tim_ie->virtual_map + tim_len,
			tim_pvb + page_slicing_data->tim_virtual_map_index,
			len_in_octets);

		/* Update bitmap(byte) offset of first block */
		if (!no_of_blocks)
			bitmap_offset = page_slicing_data->tim_virtual_map_index +
							tim_bitmap_ctrl_offset;

		/* Update copied bitmap length */
		tim_len += len_in_octets;

		if (page_slicing_data->page_slice_no < page_slice_count) {
			/* Move to next block of bitmap */
			page_slicing_data->tim_virtual_map_index += len_in_octets;
		} else {
			/* Reset the virtual map len and index */
			page_slicing_data->tim_virtual_map_len = 0;
			page_slicing_data->tim_virtual_map_index = 0;
			/* Since we are pushing whole remaining bitmap in last page lice.
			 * Exit the loop after copy.
			 */
			break;
		}
	}

	ies_mask->ies[WLAN_EID_TIM].len = offsetof(struct ieee80211_tim_ie, virtual_map) +
										tim_len;

	if (!dtim_count)
		tim_ie->bitmap_ctrl = bitmap_ctrl & IEEE80211_TIM_BITMAP_TRAFFIC_INDICATION;

	tim_ie->bitmap_ctrl |= (bitmap_offset & IEEE80211_TIM_BITMAP_OFFSET);

	/* Update the page index */
	*page_index = page_slicing_data->page_index;
}

void morse_page_slicing_init(struct ieee80211_vif *vif, u8 dtim_period, u8 enable)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct page_slicing *page_slicing_data = &mors_vif->page_slicing_info;

	memset(page_slicing_data, 0, sizeof(struct page_slicing));

	/* Enable Page slicing only when dtim period > 1 */
	if (dtim_period == 1)
		page_slicing_data->enabled = false;
	else if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, PAGE_SLICING))
		page_slicing_data->enabled = enable;

	/* Initialize page period, page index and page slice length */
	page_slicing_data->page_period = dtim_period;
	/* Set page index to 0 as we are supporting upto 2007 STAs only (mac80211 limitation) */
	page_slicing_data->page_index = 0;
	/* Schedule a page in one DTIM interval. Check for dtim period value as it is
	 * observed that mac80211 is passing invalid dtim period (zero) as part of beacon
	 * interval configuration to driver in STA mode.
	 */
	if (dtim_period)
		page_slicing_data->page_slice_length = NUMBER_OF_BLOCKS_PER_PAGE / dtim_period;
	page_slicing_data->tim_virtual_map_len = 0;
	page_slicing_data->page_slice_no = 0;
}
