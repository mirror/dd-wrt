/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * Map CPU code to exception code.
 *
 * TODO: This logic is different for tunnel exception.
 * Need to be fixed with tunnel implementation.
 */
#define PPE_DRV_CC_TO_EXP(cc) ((cc) - 1)

/*
 * ppe_drv_cc
 *	Instance structure for cpu code management
 */
struct ppe_drv_cc {
	ppe_drv_cc_callback_t cb;	/* Per cc registered callback */
	void *app_data;			/* Associated app data */
	bool flush;			/* Flush the connection */
};

void ppe_drv_cc_entries_free(struct ppe_drv_cc *cc);
struct ppe_drv_cc *ppe_drv_cc_entries_alloc(void);
