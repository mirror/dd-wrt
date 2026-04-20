/*
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * ppe_vp_tx_to_ppe()
 *      API for PPE VP user to enqueue packet for PPE processing
 *
 * @param[IN] vp_num   VP num corresponding to its interface.
 * @param[IN] skb  Socket buffer to be enqueued.
 *
 * @return
 * true if packet is consumed by the API or false if the packet is not consumed.
 */
bool ppe_vp_tx_to_ppe(int32_t vp_num, struct sk_buff *skb);

/*
 * ppe_vp_tx_to_ppe_by_dev()
 *      API for PPE VP user to enqueue packet for PPE processing on a particular dev.
 *
 * @param[IN] dev	net device on which the packet has to xmit.
 * @param[IN] skb	Socket buffer to be enqueued.
 *
 * @return
 * true if packet is consumed by the API or false if the packet is not consumed.
 */
bool ppe_vp_tx_to_ppe_by_dev(struct net_device *dev, struct sk_buff *skb);
