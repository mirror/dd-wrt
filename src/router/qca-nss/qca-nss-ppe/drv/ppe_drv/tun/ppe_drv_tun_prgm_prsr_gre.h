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
#ifndef _PPE_DRV_TUN_PRGM_PRSR__GRE_H_
#define _PPE_DRV_TUN_PRGM_PRSR_GRE_H_

/*
 * GRE tunnel Program Parser configure/deconfigure functions
 */
bool ppe_drv_tun_prgm_prsr_gre_deconfigure(struct ppe_drv_tun_prgm_prsr *pgm_psr);
bool ppe_drv_tun_prgm_prsr_gre_configure(struct ppe_drv_tun_prgm_prsr *program_parser);
#endif /* _PPE_DRV_TUN_PRGM_PRSR_GRE_H_ */