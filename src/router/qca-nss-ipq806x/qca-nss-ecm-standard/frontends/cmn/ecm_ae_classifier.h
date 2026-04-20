/*
 **************************************************************************
 * Copyright (c) 2021 The Linux Foundation.  All rights reserved.
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

extern struct ecm_ae_classifier_ops ae_ops;	/* Acceleration engine operations object */

extern void ecm_ae_classifier_select_info_fill(ip_addr_t src_ip, ip_addr_t dest_ip,
					int sport, int dport, int protocol, int ip_version,
					bool is_routed, bool is_multicast,
					struct ecm_ae_classifier_info *info);
extern bool ecm_ae_classifier_is_external(struct ecm_ae_classifier_ops *ops);
extern bool ecm_ae_classifier_is_fallback_enabled(struct ecm_ae_classifier_ops *ops);
