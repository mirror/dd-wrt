/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

/*
 * nss_ovpnmgr_debugfs.h
 */
#ifndef __NSS_OVPNMGR_DEBUGFS__H
#define __NSS_OVPNMGR_DEBUGFS__H

#define NSS_OVPNMGR_DEBUGFS_MAX_NAME_SIZE 30

int nss_ovpnmgr_debugfs_init(void);
void nss_ovpnmgr_debugfs_create(struct nss_ovpnmgr_app *app);
void nss_ovpnmgr_debugfs_remove(struct dentry *app_dentry);
void nss_ovpnmgr_debugfs_cleanup(void);
#endif /* __NSS_OVPNMGR_DEBUGFS__H */
