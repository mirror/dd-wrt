// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 */

#ifndef __KSMBD_BUFFER_POOL_H__
#define __KSMBD_BUFFER_POOL_H__

void *_ksmbd_find_buffer(size_t size, const char *func, int line);
#define ksmbd_find_buffer(size) _ksmbd_find_buffer(size, __func__, __LINE__)

void ksmbd_release_buffer(void *buffer);

static void *_ksmbd_alloc(size_t size, const char *func, int line);
#define ksmbd_alloc(size) _ksmbd_alloc(size, __func__, __LINE__)
static void *_ksmbd_zalloc(size_t size, const char *func, int line);
#define ksmbd_zalloc(size) _ksmbd_zalloc(size, __func__, __LINE__)
static void ksmbd_free(void *ptr);

void ksmbd_free_request(void *addr);

void *_ksmbd_alloc_request(size_t size, const char *func, int line);

#define ksmbd_alloc_request(size) _ksmbd_alloc_request(size, __func__ , __LINE__)

void ksmbd_free_response(void *buffer);
void *_ksmbd_alloc_response(size_t size, const char *func, int line);
#define ksmbd_alloc_response(size) _ksmbd_alloc_response(size, __func__ , __LINE__)

void *_ksmbd_realloc_response(void *ptr, size_t old_sz, size_t new_sz, const char *func, int line);
#define ksmbd_realloc_response(ptr, old_sz, new_sz) _ksmbd_realloc_response(ptr, old_sz, new_sz, __func__ , __LINE__)

void ksmbd_free_file_struct(void *filp);
void *ksmbd_alloc_file_struct(void);

void ksmbd_destroy_buffer_pools(void);
int ksmbd_init_buffer_pools(void);

#endif /* __KSMBD_BUFFER_POOL_H__ */
