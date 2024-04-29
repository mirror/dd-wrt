/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/of_platform.h>
#include "nss_crypto_ce5.h"

/*
 * nss_crypto_hw_deinit()
 *	Engine deinit routine
 */
void nss_crypto_hw_deinit(struct platform_device *pdev)
{
	/*
	 * TODO: Add support for putting HW into reset
	 */
	return;
}

/*
 * nss_crypto_hw_init()
 *	Engine init routine
 */
int nss_crypto_hw_init(struct platform_device *pdev)
{
	if (of_find_compatible_node(NULL, NULL, "qcom,ce5")) {
		return nss_crypto_ce5_init(pdev);
	}

	return 0;
}
