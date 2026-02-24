/*
 * Copyright 2020 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <linux/types.h>
#include <linux/ieee80211.h>

#include "dot11ah.h"
#include "tim.h"
#include "debug.h"
#include "../morse.h"

/* TODO: ADE for AIDs > 7 (not tested in WFA nor advertised in marketing material) */
#define ADE_AID_LIMIT		(7)

/**
 * State structure for parsing from 11n TIM to S1G TIM
 */
struct tim_to_s1g_parse_state {
	/** S1G TIM to fill */
	struct dot11ah_s1g_tim_ie *s1g_tim;

	/** 11n TIM virtual bitmap */
	const u8 *virtual_map_11n;

	/** Current index into S1G TIM partial virtual bitmap (ie. length used so far) */
	u16 index_s1g;

	/** Current length of virtual_map_11n */
	s16 length_11n;

	/**
	 * Octet offset for virtual_map_11n. It gives the current octet virtual_map_11n is pointing
	 * at in the full length 11n TIM (assuming bit 0 octet 0 in the full length 11n TIM is AID
	 * 0).
	 *
	 * E.g. If octet_offset_11n is 5, virtual_map_11n[0] will be the 5th octet of the full TIM
	 * bitmap. So if (virtual_map_11n[0] & (1<<2)) == TRUE, and octet_offset_11n == 5, traffic
	 * will be buffered for the STA with AID (5*8)+2 = 42.
	 */
	u8 octet_offset_11n;
};

/* Copy in octet and advance the index */
static inline void s1g_tim_append_octet(struct tim_to_s1g_parse_state *state, u8 octet)
{
	if (likely(state->index_s1g < sizeof(state->s1g_tim->encoded_block_info)))
		state->s1g_tim->encoded_block_info[state->index_s1g++] = octet;
}

/* Return a pointer to the current octet and advance the index */
static inline u8 *s1g_tim_reserve_octet(struct tim_to_s1g_parse_state *state)
{
	if (likely(state->index_s1g < sizeof(state->s1g_tim->encoded_block_info)))
		return &state->s1g_tim->encoded_block_info[state->index_s1g++];
	else
		return NULL;
}

/**
 * Call in 11n->s1g parsing functions to advance consumed octets in 11n tim.
 */
static void consume_11n_tim_octets(struct tim_to_s1g_parse_state *state, u8 num_octets)
{
	state->length_11n -= num_octets;
	state->octet_offset_11n += num_octets;
	state->virtual_map_11n += num_octets;

	/*
	 * Trim any holes at the front of the 11n tim.
	 */
	while (state->length_11n >= 1 && state->virtual_map_11n[0] == 0) {
		state->virtual_map_11n++;
		state->octet_offset_11n++;
		state->length_11n--;
	}
}

/*
 * Store the incoming S1G AID (13bits) into non-S1G TIM.
 * Limited to max 2007 stations to fit within non-S1G TIM (linux/mac80211 limit).
 *
 * Return the octet number of the AID stored, else negative number.
 */
static int morse_dot11_store_aid_into_tim(struct ieee80211_tim_ie *tim, u16 aid)
{
	u8 bitmap_offset;
	u8 octet_number;

	if (aid > AID_LIMIT)
		goto error;

	/*
	 * Caclulate what octet this AID falls in (0-250),
	 * where:
	 *		aid[15:3] = octet number in TIM
	 *		aid[2:0]  = bit position in octet
	 */
	octet_number = (aid >> 3);

	/*
	 * bitmap_offset is the value of the bitmap offset field * 2, and thus is always even.
	 * It can be thought of as N1 in Section 9.4.2.5.1 line ~50 of the spec
	 * (IEEE P802.11-REVme/D0.2), which is analogous to an 'octet offset' of the 11n TIM.
	 *
	 * Since this octet offset must be even, if the first sleeping station has
	 * an AID that would fall into an odd numbered octet, the first byte of the virtual map will
	 * be 0.
	 */
	if (tim->virtual_map[0] == 0 && tim->virtual_map[1] == 0) {
		/* First entry, set the offset based off octet number */
		bitmap_offset = (octet_number & IEEE80211_TIM_BITMAP_OFFSET);
		tim->bitmap_ctrl = tim->bitmap_ctrl | bitmap_offset;

	} else {
		/*
		 * Other entry; retrieve offset and make sure we haven't found an AID behind it.
		 */
		bitmap_offset = (tim->bitmap_ctrl & IEEE80211_TIM_BITMAP_OFFSET);
		if (octet_number < bitmap_offset)
			goto error;
	}

	/* Adjust our octet number based on our configured offset */
	octet_number = octet_number - bitmap_offset;
	if (octet_number >= DOT11_MAX_TIM_VIRTUAL_MAP_LENGTH)
		goto error;

	/* set the bit in the octet. */
	tim->virtual_map[octet_number] |= (0x01 << (aid & 0x07));

	return octet_number;
error:
	dot11ah_err("Error (aid %d) %s failed\n", aid, __func__);
	return -EINVAL;
}

/* 9.4.2.5.2 Block Bitmap Mode */
static int morse_dot11_s1g_to_tim_parse_block_mode(struct ieee80211_tim_ie *tim,
	u8 *tim_len,
	const u8 *block_info,
	u16 block_offset,
	u16 page_index,
	bool inverse_bitmap)
{
	int index = 0;
	int octet_number;
	int pos_m;
	int pos_q;
	u8 block_bitmap;

	/* Process the Encoded Block Information.
	 * The block starts with a single block bitmap byte,
	 * followed by variable length subblocks
	 */

	/* First octet presents the current block bitmap */
	block_bitmap = block_info[index];

	/* Move to next octet in the bitmap (start of subblocks) */
	index++;

	/* Now loop on the subblocks (n bytes), where n = number of 1's in block_bitmap */
	for (pos_m = 0; pos_m < S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK; pos_m++) {
		if ((block_bitmap >> pos_m) & 0x01) {
			u8 subblock = inverse_bitmap ?
				~block_info[index] : block_info[index];

			for (pos_q = 0; pos_q < S1G_TIM_NUM_AID_PER_SUBBLOCK; pos_q++) {
				if ((subblock >> pos_q) & 0x01) {
					u16 aid = 0;
					/* AID[0:12] constructed by concatenating:
					 *   > pos_q (AID[0:2]),
					 *   > pos_m (AID[3:5]),
					 *   > Block Offset field (AID[6:10]),
					 *   > Page Index field (AID[11:12])
					 * in sequence from LSB to MSB.
					 */
					aid = aid | ((pos_q        <<  0) & 0x0007);
					aid = aid | ((pos_m        <<  3) & 0x0038);
					aid = aid | ((block_offset <<  6) & 0x07C0);
					aid = aid | ((page_index   << 11) & 0x1800);
					/* Map this AID into 11n TIM (tim->virtual_map) */
					octet_number = morse_dot11_store_aid_into_tim(tim, aid);

					if (octet_number > *tim_len) {
						*tim_len = octet_number;
					} else if (octet_number < 0) {
						dot11ah_err("Failed to store aid into tim %d\n",
							    aid);
						return -1;
					}
				}
			}

			/* Move to next octet in the bitmap */
			index++;
		}
	}

	return index;
}

/*
 * 9.4.2.5.3 Single AID Mode
 */
static int morse_dot11_s1g_to_tim_parse_single_mode(struct ieee80211_tim_ie *tim, u8 *tim_len,
						    const u8 *block_info,
						    u16 block_offset,
						    u16 page_index,
						    bool inverse_bitmap)
{
	u16 aid = 0;
	u8 single_aid = 0;
	u8 octet_number = 0;

	single_aid = block_info[0];

	if (inverse_bitmap) {
		dot11ah_err("Inverse bitmap not supported for Single AID mode\n");
		return 1;
	}

	/* AID[0:12] constructed by concatenating:
	 *   > Single AID subfield (AID[0:5]),
	 *   > Block Offset field (AID[6:10]),
	 *   > Page Index field (AID[11:12])
	 * in sequence from LSB to MSB.
	 */
	aid = aid | (single_aid           & 0x003F);
	aid = aid | ((block_offset <<  6) & 0x07C0);
	aid = aid | ((page_index   << 11) & 0x1800);

	/* Now we have this AID, need to map it into 11n TIM (tim->virtual_map) */
	octet_number = morse_dot11_store_aid_into_tim(tim, aid);

	if (octet_number > *tim_len)
		*tim_len = octet_number;

	/* Block Info is presented only with a single byte */
	return 1;
}

/*
 * 9.4.2.5.4 OLB Mode
 */
static int morse_dot11_s1g_to_tim_parse_olb_mode(struct ieee80211_tim_ie *tim, u8 *tim_len,
						 const u8 *block_info,
						 u16 block_offset,
						 u16 page_index,
						 bool inverse_bitmap)
{
	int index = 0;
	int pos_q;
	int subblock_m;
	u8 length;
	s16 octet_number;

	/* Process the Encoded Block Information. The block starts with a single block length byte,
	 * followed by variable length subblocks
	 */

	/* First octet presents the current block bitmap */
	length = block_info[index];

	/* Move to next octet in the bitmap */
	index++;

	/* Now loop on the subblocks (n bytes), where n = length */
	for (subblock_m = 0; subblock_m < length; subblock_m++) {
		u8 subblock = inverse_bitmap ? (~block_info[index]) : block_info[index];

		for (pos_q = 0; pos_q < S1G_TIM_NUM_AID_PER_SUBBLOCK; pos_q++) {
			if ((subblock >> pos_q) & 0x01) {
				u16 aid = 0;
				int block_k = block_offset + (subblock_m / 8);

				/* AID[0:12] constructed by concatenating:
				 *   > pos_q (AID[0:2]),
				 *   > Subblock offset m mod 8 (AID[3:5]),
				 *   TODO: Block K overflow? offset is 5 bit and m/8 is 5 bits.
				 *   Sum needs 6 bits. The only way to be safe is assuming
				 *   length < 8 (so m/8 is always 0).
				 *   > Block K (i.e., Block Offset + [m / 8]) (AID[6:10]),
				 *   > Page Index field (AID[11:12])
				 * in sequence from LSB to MSB.
				 */
				aid = aid | (pos_q & 0x0007);
				aid = aid | (((subblock_m % 8) << 3) & 0x0038);
				aid = aid | ((block_k << 6) & 0x07C0);
				aid = aid | ((page_index << 11) & 0x1800);
				/* Now we have this AID, need to map it into 11n TIM */
				octet_number = morse_dot11_store_aid_into_tim(tim, aid);

				if (octet_number > *tim_len) {
					*tim_len = octet_number;

				} else if (octet_number < 0) {
					dot11ah_err("Failed to store aid into tim %d\n", aid);
					return -1;
				}
			}
		}

		/* Move to next octet in the bitmap */
		index++;
	}

	/* index should be equal to length + 1 (the length field byte itself + subblocks count */
	return index;
}

/*
 * 9.4.2.5.5 ADE Mode
 * TODO: ADE for AIDs > 7 (not tested in WFA nor advertised in marketing material)
 */
static int morse_dot11_s1g_to_tim_parse_ade_mode(struct ieee80211_tim_ie *tim,
						 const u8 *block_info,
						 u16 block_offset,
						 u16 page_index,
						 bool inverse_bitmap)
{
	int i;
	int index = 0;
	u16 aid = 0;
	u8 diff_aid;
	u8 ewl = 0;
	u8 length;
	u8 bits[256]; /* temp bit array for easier WL traverse */
	const u8 *bytes = NULL;
	int number_encoded_words;

	/* Process the Encoded Block Information. The block starts with EWL subfield [b0:b2] then
	 * number of differential aid's [b3:b7], followed by 'length' diff_aid's and a padding
	 */
	ewl = block_info[index] & 0x07;
	length = (block_info[index] & 0xF8) >> 3;

	/* Move to next octet in the bitmap */
	index++;

	/* Special case 1: if all AIDs in the ADE blocks are paged, AP sets the Inverse Bitmap to 1
	 * and ADE block consists only EWL and Length fields, where both EWL and Length Field are
	 * set to 0s.
	 */
	if (inverse_bitmap && ewl == 0 && length == 0) {
		/* TODO: should retrieve all AID's in this ADE block as per last paragraph in
		 * 9.4.2.5.5, but for simplicity we will only assume 8 AID's are encoded (as the
		 * virtual_map is only one byte anyway for now)
		 */
		for (i = 0; i < 8; i++) {
			aid = (page_index * 2048) + (block_offset * 64) + i;
			/* Now we have this AID, need to map it into 11n TIM (tim->virtual_map) */
			if (aid > ADE_AID_LIMIT)
				goto error;

			morse_dot11_store_aid_into_tim(tim, aid);
		}

		/* Only a single byte is used (for ewl and length) */
		return 1;
	}

	/* Special case 2: if all but one AIDs in the ADE blocks are paged, AP sets the Inverse
	 * Bitmap to 1 and ADE block consists only one Diff_AID subfield. The AP sets the EWL to 7
	 * and the Length subfield to one. Diff_AID subfield is set to:
	 * (AID (Page Index 2048 + Block Offset 64)).
	 */
	if (inverse_bitmap && ewl == 7 && length == 1) {
		u16 excluded_aid;

		diff_aid = block_info[index];
		excluded_aid =  diff_aid + (page_index * 2048) + (block_offset * 64);

		/* TODO: should retrieve all AID's in this ADE block as per last paragraph in
		 * 9.4.2.5.5, but for simplicity we will only assume 8 AID's are encoded (as the
		 * virtual_map is only one byte anyway for now)
		 */
		for (i = 0; i < 8; i++) {
			aid = (page_index * 2048) + (block_offset * 64) + i;
			/* Exclude AID marked as unpaged, map other AIDs into TIM as before */
			if (aid > ADE_AID_LIMIT)
				goto error;

			if (aid != excluded_aid)
				morse_dot11_store_aid_into_tim(tim, aid);
		}

		/* Two bytes are used (1st for ewl and length, and second for single diff_aid) */
		return 2;
	}

	/* For all other cases, AP sorts all AIDi, i = 1, 2...n in an ascending order and calculates
	 * the AID differential values according to
	 *	> Diff_AID1 = AID1 (Page Index 2048 + Block Offset 64)
	 *	> Diff_AIDi = AIDi AIDi 1, i = 2 ... n.
	 */

	/* Number of bits for each word (diff_aid) = EWL + 1 */
	ewl += 1;

	/* For easier looping, create a bit array (length x 8) out of the byte array */
	bytes = &block_info[index];
	for (i = 0; i < length * 8; i++)
		bits[i] = ((1 << (i % 8)) & bytes[i / 8]) >> (i % 8);

	number_encoded_words = (length * 8) / ewl;

	/* Example:
	 * ewl = 3, length = 5
	 * bytes [LSB to MSB]: 0x5F, 0x31, 0x22, 0x10, 0x7a
	 * bites: 1 1 1 1 1 0 1 0 1 0 0 0 1 1 0 0 0 1 0 0 0 1 0 0 0 0 0 0 1 0 0 0 0 1 0 1 1 1 1 0
	 * number_encoded_words = (5 * 8) / 3 = 13
	 */

	/* Now loop over the array to extract all diff_aids (each ewl bits) */
	for (i = 0; i < number_encoded_words; i++) {
		int j = 0;

		diff_aid = 0;

		for (j = 0; j < ewl; j++)
			diff_aid = diff_aid | (bits[(i * ewl) + j] << j);

		if (i == 0)	/* first word */
			aid = diff_aid + (page_index * 2048) + (block_offset * 64);
		else /* all other words */
			aid = diff_aid + aid;

		if (aid > ADE_AID_LIMIT)
			goto error;

		morse_dot11_store_aid_into_tim(tim, aid);
	}

	/* Variable length specifies the total length of the current ADE block in octets, excluding
	 * EWL and Length subfields.
	 */
	return length + 1;

error:
	dot11ah_err("ADE mode is not supported for AIDs larger than 7.\n");
	return length;
}

/* 9.4.2.5.2 Block Bitmap Mode */
static void morse_dot11_tim_to_s1g_parse_block_mode(struct tim_to_s1g_parse_state *state,
						    bool inverse_bitmap, u16 max_aid)
{
	int i;
	int j;
	u8 num_subblocks;
	u16 aid_base = state->octet_offset_11n * 8;
	u8 subblocks_to_block_boundary = S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK -
			(state->octet_offset_11n % S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK);

	/*
	 * This will hold the MAX of 8 subblocks before copying back to the s1g_tim struct.
	 */
	u8 block_ctrl;
	u8 block_offset;
	u8 *block_bitmap;
	u8 subblocks[S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK];

	/* On inverse mode, all subblocks are inverted */
	memset(subblocks, inverse_bitmap ? 0xFF : 0x00, sizeof(subblocks));

	/* Set Block Control, block[0] (bit0:bit2) */
	block_ctrl = ENC_MODE_BLOCK |
		(inverse_bitmap << IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP_SHIFT);

	/* AID[0:12] constructed by concatenating:
	 *   > pos_q (AID[0:2]),
	 *   > pos_m (AID[3:5]),
	 *   > Block Offset field (AID[6:10]),
	 *   > Page Index field (AID[11:12]) <<-- Caller already set to zero for AID's < 2008
	 * in sequence from LSB to MSB.
	 */
	block_offset = S1G_TIM_AID_TO_BLOCK_OFFSET(aid_base);

	/* Fill in the Block Offset (b3:b7) & Block control (b0:b2) in first byte of the block */
	s1g_tim_append_octet(state, block_ctrl |
			     (block_offset << IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET_SHIFT));

	/* fill out block_bitmap & subblocks from 11n tim virtual map */
	for (i = 0; i < state->length_11n && i < subblocks_to_block_boundary; i++) {
		u8 temp = state->virtual_map_11n[i];

		for (j = 0; temp != 0; temp >>= 1, j++) {
			/* bit found */
			if (temp & 0x1) {
				/* Work out actual AID (to account for bitmap_offset in 11n tim) */
				u16 aid = aid_base + (i * S1G_TIM_NUM_AID_PER_SUBBLOCK) + j;

				/* convert aid to positions */
				u8 pos_m = ((aid >> 3) & 0x7);
				u8 pos_q = (aid & 0x7);

				/* set/clear the bit in the corresponding subblock */
				if (inverse_bitmap)
					subblocks[pos_m] &= ~(0x1 << pos_q);
				else
					subblocks[pos_m] |= (0x1 << pos_q);
			}
		}
	}

	consume_11n_tim_octets(state, i);

	/* Save the location of block_bitmap for later */
	block_bitmap = s1g_tim_reserve_octet(state);
	if (unlikely(!block_bitmap))
		return;

	/*
	 * Copy in subblocks
	 * Clamp max sub-block based on max AID (for inverse mode)
	 */
	num_subblocks = min((u8)(((max_aid - (block_offset * S1G_TIM_NUM_AID_PER_BLOCK)) >> 3) + 1),
		(u8)ARRAY_SIZE(subblocks));

	for (i = 0; i < num_subblocks; i++) {
		/* Only include subblocks that have info */
		if (subblocks[i] != 0) {
			s1g_tim_append_octet(state, subblocks[i]);

			/* set the bit in the block_bitmap to indicate the subblock is present */
			*block_bitmap |= (0x1 << i);
		}
	}
}

/* 9.4.2.5.3 Single AID Mode
 * This mode will try to consume an entire byte. Therefore it will add an encoded block for every
 * bit set in the virtual map byte it selects. It is up to the caller to make sure only one bit is
 * set in the virtual map byte, else reap the consequences of inefficency.
 */
static void morse_dot11_tim_to_s1g_parse_single_mode(struct tim_to_s1g_parse_state *state,
						     bool inverse_bitmap)
{
	int remainder;
	u16 aid_base = state->octet_offset_11n * 8;
	u8 block_ctrl;
	u8 block_offset = 0;
	u8 bitmap;

	/*
	 * Set Block Control, block[0] (bit0:bit2)
	 */
	block_ctrl = ENC_MODE_AID |
		(inverse_bitmap << IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP_SHIFT);

	/* AID[0:12] constructed by concatenating:
	 *   > Single AID subfield (AID[0:5]),
	 *   > Block Offset field (AID[6:10]),
	 *   > Page Index field (AID[11:12]) <<-- Caller already set to zero for AID's < 2008
	 * in sequence from LSB to MSB.
	 */

	bitmap = state->virtual_map_11n[0];
	consume_11n_tim_octets(state, 1);

	/*
	 * Inverse single AID mode, ie. every station except for the specified one has data
	 * buffered, is not supported as the use case is almost non-existent & can be easily covered
	 * by other encoding schemes.
	 *
	 * Do this here (after we consume 11n TIM bytes) so we don't get stuck in an infinite loop.
	 */
	if (inverse_bitmap) {
		dot11ah_err("Inverse Single AID mode is not supported for transmit.");
		return;
	}

	block_offset = S1G_TIM_AID_TO_BLOCK_OFFSET(aid_base);

	for (remainder = 0; remainder < 8; remainder++) {
		if ((bitmap >> remainder) & 0x01) {
			u8 single_aid = (aid_base | remainder) & 0x003F;

			s1g_tim_append_octet(state, block_ctrl | (block_offset <<
						IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET_SHIFT));

			s1g_tim_append_octet(state, single_aid);
		}
	}
}

/* 9.4.2.5.4 OLB Mode */
static void morse_dot11_tim_to_s1g_parse_olb_mode(struct tim_to_s1g_parse_state *state,
						  bool inverse_bitmap, u16 max_aid)
{
	int i;
	int j;
	u16 aid_base = state->octet_offset_11n * 8;
	u8 block_ctrl;
	u8 block_offset;
	u8 num_subblocks = 0;
	u8 subblocks[S1G_TIM_MAX_BLOCK_SIZE];

	u8 start_idx = 0, stop_idx = 0;
	u8 empty_front_subblocks = 0, empty_front_blocks = 0;

	/* AID[0:12] constructed by concatenating:
	 *   > pos_q (AID[0:2]),
	 *   > Subblock offset m mod 8 (AID[3:5]),
	 *   > Block K (i.e., Block Offset + [m / 8]) (AID[6:10]),
	 *   > Page Index field (AID[11:12])
	 * in sequence from LSB to MSB.
	 *
	 * From the spec:
	 * The Length subfield is 1 octet. A Length subfield equal to n indicates that the Encoded
	 * Block Information field contains n contiguous subblocks in ascending order from multiple
	 * blocks starting from the first subblock of the block in position Block Offset.
	 *
	 *
	 * OLB may contain empty subblocks at the start if the first AID is at the top of a block
	 * boundary.
	 * OLB has a limitation where for aids/subblocks close to the upper block boundary, all
	 * subblocks lower than it in the block will still have to be included.
	 * E.g.
	 * s1g block:|                 1                    |                  2                   |
	 * 11n tim:  0x00 0x00 0x00 0x00 0x00 0x00 0xF1 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00
	 *                                           ^
	 *
	 * Will OLB encode as: 0x07 0x00 0x00 0x00 0x00 0x00 0x00 0xF1
	 *
	 * Note that this will have a length of 7 with most subblocks being 0 as we are only able to
	 * offset by the block.
	 *
	 * This encoding should only really be used when num sleeping stations > max that can be
	 * displayed by block mode, or there is a long sequence of contiguous subblocks with bits
	 * set.
	 */

	memset(subblocks, inverse_bitmap ? 0xFF : 0x00, sizeof(subblocks));

	/*
	 * Set Block Control, block[0] (bit0:bit2)
	 */
	block_ctrl = ENC_MODE_OLB |
		(inverse_bitmap << IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP_SHIFT);

	block_offset = S1G_TIM_AID_TO_BLOCK_OFFSET(aid_base);
	start_idx = block_offset * S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;

	/*
	 * Walk the 11n tim and copy
	 */
	for (i = 0; i < state->length_11n && i < ARRAY_SIZE(subblocks); i++) {
		u8 temp = state->virtual_map_11n[i];

		for (j = 0; temp != 0; temp >>= 1, j++) {
			/* bit found */
			if (temp & 0x1) {
				/*
				 * Work out actual AID (to account for bitmap_offset in 11n tim)
				 */
				u16 aid = aid_base + (i * S1G_TIM_NUM_AID_PER_SUBBLOCK) + j;

				/* convert aid to positions */
				u8 pos_m = (aid >> 3);
				u8 pos_q = (aid & 0x7);

				/* set/clear the bit in the corresponding subblock */
				if (inverse_bitmap) {
					subblocks[pos_m] &= ~(0x1 << pos_q);
				} else {
					subblocks[pos_m] |= (0x1 << pos_q);
					/* Store largest used subblock */
					if (pos_m >= stop_idx)
						stop_idx = pos_m + 1;
				}
			}
		}
	}

	consume_11n_tim_octets(state, i);

	/* See if we can trim the length */
	if (inverse_bitmap) {
		/* Subblock of max AID is the stop index in inverse mode */
		stop_idx = ((max_aid) >> 3) + 1;

		/* Count the number of empty starting subblocks */
		for (i = start_idx; i < stop_idx; i++) {
			if (subblocks[i] != 0)
				break;
			empty_front_subblocks++;
		}

		/* Can only advance by a block (8 bytes) at a time */
		empty_front_blocks = empty_front_subblocks / S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK;

		if (empty_front_blocks) {
			/* Update the offset */
			block_offset += empty_front_blocks;
			start_idx += (S1G_TIM_NUM_SUBBLOCKS_PER_BLOCK * empty_front_blocks);
		}

		/* Try to trim the tail, as stop_idx is set by max AID */
		while (stop_idx > start_idx && subblocks[stop_idx - 1] == 0)
			stop_idx--;
	}

	num_subblocks = stop_idx - start_idx;

	/* insert the data into the encoded block info, if we have any */
	if (num_subblocks) {
		s1g_tim_append_octet(state, block_ctrl | (block_offset <<
						IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET_SHIFT));

		s1g_tim_append_octet(state, num_subblocks);

		for (i = start_idx; i < stop_idx; i++)
			s1g_tim_append_octet(state, subblocks[i]);
	}
}

/*
 * 9.4.2.5.5 ADE Mode
 * TODO: ADE for AIDs > 7 (not tested in WFA nor advertised in marketing material)
 *		support for multiple encoded blocks / looping over 11n tim
 */
static void morse_dot11_tim_to_s1g_parse_ade_mode(struct tim_to_s1g_parse_state *state,
						  bool inverse_bitmap)
{
	int i;
	int remainder;
	int aid_index = 0;
	u8 block_offset;

	u16 aid_base = state->octet_offset_11n * 8;

	/* this will hold the MAX of 8 AID's before copying back to the s1g_tim struct */
	u16 diff_aid_list[8] = { 0 };

	/* AID[0:12] constructed by concatenating:
	 *	> AID1 = Diff_AID1 + (Page Index 2048 + Block Offset 64)
	 *	> AIDi = Diff_AIDi + AIDi 1, i = 2 ... n.
	 */

	/* Note: we have two variables in the first equation (Block Offset and Diff_AID). We will
	 * assume the diff_aid is always < 64 (bits 0:5), hence Block Offset field is AID[6:10].
	 */
	block_offset = S1G_TIM_AID_TO_BLOCK_OFFSET(aid_base);

	if (state->length_11n > 1)
		dot11ah_err("ADE encoding not supported for AIDs larger than 8\n");

	/* Fill in the Block Offset (b3:b7) in first byte of the block */
	s1g_tim_append_octet(state, ENC_MODE_ADE | (block_offset <<
						IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET_SHIFT));

	/* Loop on the virtual_map to extract the active aids and construct the diff_aid array
	 */
	for (remainder = 0; remainder < 8; remainder++) {
		if ((state->virtual_map_11n[0] >> remainder) & 0x01) {
			/* For active AID's use only the first 6 bits for Diff_AID */
			u8 diff_aid = (aid_base | remainder) % 64;

			if (aid_index == 0)
				diff_aid_list[aid_index] = diff_aid;
			else
				diff_aid_list[aid_index] = diff_aid - diff_aid_list[aid_index - 1];

			aid_index++;
		}
	}

	consume_11n_tim_octets(state, 1);

	if (aid_index > 0) {
		/* For simplicity, use one octet for each diff_aid, hence the EWL field (len of word
		 * in bits) will be 0x7, and the total length in octets = number of encoded AIDs
		 */
		s1g_tim_append_octet(state, 0x07 /* EWL */ | (aid_index << 3) /* Length */);

		for (i = 0; i < aid_index; i++)
			s1g_tim_append_octet(state, diff_aid_list[i]);
	}
}

/*
 * Convert S1G TIM to Non-S1G TIM.
 * The output Non-S1G map is limited only to the first 8 AIDs.
 * Also, any incoming AID that is larger than 2008 is dropped.
 */
int morse_dot11_s1g_to_tim(struct ieee80211_tim_ie *tim, const struct dot11ah_s1g_tim_ie *s1g_tim,
			   size_t total_len)
{
	int index = 0;
	u8 enc_mode;
	u8 page_slice = 0;
	u16 page_index = 0;
	u16 block_offset = 0;
	int block_info_len;
	int length = sizeof(*tim);
	bool inverse_bitmap;
	u8 tim_virtual_bitmap_max_octet = 0;
	int res;

	if (!tim || !s1g_tim)
		return length;

	if (total_len < 2)
		return length;

	tim->dtim_count = s1g_tim->dtim_count;
	tim->dtim_period = s1g_tim->dtim_period;

	/* No blocks encoded in this element, return */
	if (total_len < 3)
		return length;

	/*
	 * Prepare an empty TIM (in case of errors)
	 */
	tim->bitmap_ctrl = 0;
	tim->virtual_map[0] = 0;

	/* Copy Broadcast Traffic */
	tim->bitmap_ctrl = (s1g_tim->bitmap_control & IEEE80211_TIM_BITMAP_TRAFFIC_INDICATION);

	/* The number of blocks is unknown, so will use the actual length in bytes
	 * to loop over the bitmap (i.e., while length > 0)
	 *
	 * Note: total_len (input) indicates the number of octets in the element excluding
	 * the Element ID and Length fields. Hence: actual bitmap length is calculated as:
	 *
	 * Encoded Block Info Length = Element length
	 *				- DTIM Count (1byte)
	 *				- DITM Period (1byte)
	 *				- Bitmap Control (1byte)
	 */
	block_info_len = total_len - 3;

	page_index = (s1g_tim->bitmap_control & IEEE80211_S1G_TIM_BITMAP_PAGE_INDEX) >>
		IEEE80211_S1G_TIM_BITMAP_PAGE_INDEX_SHIFT;

	/*
	 * The Page Slice Number subfield indicates which page slice is encoded in the Partial
	 * Virtual Bitmap field when the subfield is in the range of 0 to 30. If the Page Slice
	 * Number subfield is 31, then the entire page indicated by the Page Index subfield value is
	 * encoded in the Partial Virtual Bitmap field of the TIM elements with the same page index.
	 */

	page_slice = (s1g_tim->bitmap_control & IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE) >>
		IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE_SHIFT;

	/*
	 * If all bits in virtual bitmap are 0, the Partial Virtual Bitmap field is not present in
	 * the TIM element and the Length field of the TIM element is set to 3. In such a case, it
	 * only makes sense if the page_slice is set to 31 (i.e., the entire page is set).
	 */
	if (block_info_len == 0) {
		if (page_slice == S1G_TIM_PAGE_SLICE_ENTIRE_PAGE) {
			/* Clear all. We have page_slice 31 but nothing is in partial bitmap */
			tim->bitmap_ctrl = 0;
			tim->virtual_map[0] = 0;
		}
		return length;
	}

	while (index < block_info_len) {
		/* Parse the encoding mode and block offset, using the first byte of block
		 * > Block Control: bits 0:2
		 *   > Encoding Mode: bits 0:1
		 *   > Inverse Bitmap: bit 2
		 * > Block Offset : bits 3:7
		 */
		enc_mode =
			(s1g_tim->encoded_block_info[index] &
			 IEEE80211_S1G_TIM_BLOCK_CTL_ENC_MODE) >>
				IEEE80211_S1G_TIM_BLOCK_CTL_ENC_MODE_SHIFT;

		inverse_bitmap =
			(s1g_tim->encoded_block_info[index] &
			 IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP) >>
				IEEE80211_S1G_TIM_BLOCK_CTL_INVERSE_BMAP_SHIFT;

		block_offset =
			(s1g_tim->encoded_block_info[index] &
			 IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET) >>
				IEEE80211_S1G_TIM_BLOCK_CTL_BLOCK_OFFSET_SHIFT;

		/* Advance one byte (Block Control and Block Offset); now point to Block Info */
		index++;
		switch (enc_mode) {
		case ENC_MODE_BLOCK:
			res = morse_dot11_s1g_to_tim_parse_block_mode(tim,
				&tim_virtual_bitmap_max_octet,
				&s1g_tim->encoded_block_info[index], block_offset,
				page_index, inverse_bitmap);
			if (res < 0)
				goto error;
			else
				index += res;
			break;
		case ENC_MODE_AID:
			index += morse_dot11_s1g_to_tim_parse_single_mode(tim,
				&tim_virtual_bitmap_max_octet,
				&s1g_tim->encoded_block_info[index], block_offset,
				page_index, inverse_bitmap);
			break;
		case ENC_MODE_OLB:
			res = morse_dot11_s1g_to_tim_parse_olb_mode(tim,
				&tim_virtual_bitmap_max_octet,
				&s1g_tim->encoded_block_info[index], block_offset,
				page_index, inverse_bitmap);
			if (res < 0)
				goto error;
			else
				index += res;
			break;
		case ENC_MODE_ADE:
			index += morse_dot11_s1g_to_tim_parse_ade_mode(tim,
				&s1g_tim->encoded_block_info[index], block_offset, page_index,
				inverse_bitmap);
			break;
		default:
			goto error;
		}
	}

	return length + tim_virtual_bitmap_max_octet;
error:
	dot11ah_err("Error %s failed\n", __func__);
	return length;
}

int morse_dot11_tim_to_s1g(struct dot11ah_s1g_tim_ie *s1g_tim,
			   const struct ieee80211_tim_ie *tim,
			   u8 tim_virtual_map_length,
			   enum dot11ah_tim_encoding_mode enc_mode,
			   bool inverse_bitmap,
			   u16 max_aid,
			   u8 page_slice_no,
			   u8 page_index)
{
	u8 octet_offset;
	struct tim_to_s1g_parse_state state;
	int s1g_tim_length = 0;

	if (!s1g_tim || !tim)
		/* Account for max length we will send */
		return sizeof(*s1g_tim);

	/*
	 * If all bits in virtual bitmap are 0, the Partial Virtual Bitmap field is not present in
	 * the TIM element and the Length field of the TIM element is set to 3. If all bits in the
	 * virtual bitmap are 0 and all the bits of the Bitmap Control field are 0, both the Partial
	 * Virtual Bitmap field and the Bitmap Control field are not present in the TIM element and
	 * the Length field of the TIM element is set to 2. The Bitmap Control field is present if
	 * the Partial Virtual Bitmap field is present.
	 */
	s1g_tim_length = sizeof(*s1g_tim)
			- sizeof(s1g_tim->bitmap_control)
			- sizeof(s1g_tim->encoded_block_info);

	s1g_tim->dtim_count = tim->dtim_count;
	s1g_tim->dtim_period = tim->dtim_period;

	/* Let prepare an empty TIM (in case of errors) */
	s1g_tim->bitmap_control = 0;
	memset(s1g_tim->encoded_block_info, 0, S1G_TIM_MAX_BLOCK_SIZE);

	/* Set the traffic indicator bit, as per the incoming TIM element */
	s1g_tim->bitmap_control = (tim->bitmap_ctrl & IEEE80211_TIM_BITMAP_TRAFFIC_INDICATION);

	/* Which octet does the first TIM bitmap block represent? */
	octet_offset = (tim->bitmap_ctrl & IEEE80211_TIM_BITMAP_OFFSET);

	/* Initialise the parse state structure */
	state.s1g_tim = s1g_tim;
	state.index_s1g = 0;
	state.octet_offset_11n = octet_offset;
	state.length_11n = tim_virtual_map_length;
	state.virtual_map_11n = tim->virtual_map;

	/*
	 * Consume any empty octets at the start of the 11n TIM.
	 *	This can happen if the virtual map starts at an odd offset, or if we get passed an
	 *	empty TIM from linux.
	 */
	consume_11n_tim_octets(&state, 0);

	while (state.length_11n > 0 && state.index_s1g < sizeof(s1g_tim->encoded_block_info)) {
		switch (enc_mode) {
		case ENC_MODE_BLOCK:
			morse_dot11_tim_to_s1g_parse_block_mode(&state, inverse_bitmap, max_aid);
			break;

		case ENC_MODE_AID:
			morse_dot11_tim_to_s1g_parse_single_mode(&state, inverse_bitmap);
			break;

		case ENC_MODE_OLB:
			morse_dot11_tim_to_s1g_parse_olb_mode(&state, inverse_bitmap, max_aid);
			break;

		case ENC_MODE_ADE:
			morse_dot11_tim_to_s1g_parse_ade_mode(&state, inverse_bitmap);
			break;

		default:
			goto error;
		}
	}

	/* Only include the tim if we either have BC traffic, or the 11n tim had some bits set. */
	if (s1g_tim->bitmap_control || state.index_s1g > 0) {
		s1g_tim->bitmap_control |= (page_slice_no <<
					    IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE_SHIFT);
		s1g_tim->bitmap_control |= (page_index <<
					   IEEE80211_S1G_TIM_BITMAP_PAGE_INDEX_SHIFT);

		/* Bitmap Control field is present if the Partial Virtual Bitmap field is present */
		s1g_tim_length = s1g_tim_length + state.index_s1g + 1;
	}

	return s1g_tim_length;
error:
	dot11ah_err("Error %s failed\n", __func__);
	return s1g_tim_length;
}

void morse_dot11ah_insert_s1g_tim(struct ieee80211_vif *vif, struct dot11ah_ies_mask *ies_mask,
				  u8 page_slice_no, u8 page_index)
{
	int length;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct dot11ah_s1g_tim_ie s1g_tim_ie;
	const struct ieee80211_tim_ie *tim;
	enum dot11ah_tim_encoding_mode enc_mode;
	u8 tim_virtual_map_len_11n;
	bool inverse_bitmap;

	/* SW-4741: in IBSS, TIM element is not relevant and should not be inserted */
	if (vif->type == NL80211_IFTYPE_ADHOC)
		return;

	/* enc_mode here is 3 bits, carrying both encoding mode and inverse bitmap fields
	 * TODO: add inverse_bitmap field separate in morsectrl instead of muxing it with enc_mode
	 */
	enc_mode = mors_vif ? (mors_vif->custom_configs->enc_mode & 0x03) : 0;
	inverse_bitmap = mors_vif ? ((mors_vif->custom_configs->enc_mode & 0x04) >> 2) : 0;

	tim = (const struct ieee80211_tim_ie *)ies_mask->ies[WLAN_EID_TIM].ptr;

	/* 11n TIM is either 2 bytes (with no virtual map), or 3 bytes + virtual map */
	tim_virtual_map_len_11n = (ies_mask->ies[WLAN_EID_TIM].len <= 2) ?
			0 : (ies_mask->ies[WLAN_EID_TIM].len - 3);

	morse_dot11_clear_eid_from_ies_mask(ies_mask, WLAN_EID_TIM);

	length = morse_dot11_tim_to_s1g(&s1g_tim_ie,
					tim,
					tim_virtual_map_len_11n,
					enc_mode,
					inverse_bitmap,
					mors_vif->ap->largest_aid,
					page_slice_no,
					page_index);

	morse_dot11ah_insert_element(ies_mask, WLAN_EID_TIM, (u8 *)&s1g_tim_ie, length);
}
EXPORT_SYMBOL(morse_dot11ah_insert_s1g_tim);
