/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __SMBD_BUFFER_POOL_H__
#define __SMBD_BUFFER_POOL_H__

void *smbd_find_buffer(size_t size);
void smbd_release_buffer(void *buffer);

void *smbd_alloc(size_t size);
void smbd_free(void *ptr);

void smbd_free_request(void *addr);
void *smbd_alloc_request(size_t size);
void smbd_free_response(void *buffer);
void *smbd_alloc_response(size_t size);

void *smbd_realloc_response(void *ptr, size_t old_sz, size_t new_sz);

void smbd_free_file_struct(void *filp);
void *smbd_alloc_file_struct(void);

void smbd_destroy_buffer_pools(void);
int smbd_init_buffer_pools(void);

#endif /* __SMBD_BUFFER_POOL_H__ */
