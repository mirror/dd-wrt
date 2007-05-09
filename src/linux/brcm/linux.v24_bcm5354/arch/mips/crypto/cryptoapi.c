/* $Id: cryptoapi.c,v 1.1.2.1 2004/05/23 12:29:03 nikki Exp $
 *
 * crypto/cryptoapi.c
 *
 * Written by Alexander Kjeldaas <astor@fast.no> 1998-11-15
 *
 * 2000-10-15 Harald Welte <laforge@gnumonks.org>
 * 		- ported to Linux 2.4 
 * 
 * Copyright (C)  1998 Alexander Kjeldaas.
 *                2001 Herbert Valerio Riedel
 *
 * This module is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version. 
 *
 */

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/config.h>
#include <linux/string.h>
#include <linux/crypto.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_KMOD
#include <linux/kmod.h>
#endif /* CONFIG_KMOD */

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#ifdef MODULE_DESCRIPTION
MODULE_DESCRIPTION ("CryptoAPI core module");
#endif

static struct proc_dir_entry *proc_crypto;

static struct cipher_context *
default_realloc_cipher_context(struct cipher_context *old_cx,
			       struct cipher_implementation *,
			       int max_key_len);

static struct digest_context *
default_realloc_digest_context(struct digest_context *old_cx,
			       struct digest_implementation *);

static void default_wipe_context(struct cipher_context *cx);

static void default_free_cipher_context(struct cipher_context *cx);
static void default_free_digest_context(struct digest_context *cx);

static void default_lock(void);
static void default_unlock(void);

/* ciphers */
static int default_encrypt(struct cipher_context *cx, 
			   const u8 *in, u8 *out, int size);
static int default_encrypt_atomic(struct cipher_context *cx, 
				  const u8 *in, u8 *out, int size);
static int default_encrypt_iv(struct cipher_context *cx, 
			      const u8 *in, u8 *out, int size,
			      const u8 *iv);
static int default_encrypt_atomic_iv(struct cipher_context *cx, 
				     const u8 *in, u8 *out, int size,
				     const u8 *iv);

static int default_decrypt(struct cipher_context *cx, 
			   const u8 *in, u8 *out, int size);
static int default_decrypt_atomic(struct cipher_context *cx, 
				  const u8 *in, u8 *out, int size);

static int default_decrypt_iv(struct cipher_context *cx, 
			      const u8 *in, u8 *out, int size, const u8 *iv);
static int default_decrypt_atomic_iv(struct cipher_context *cx, 
				     const u8 *in, u8 *out, int size,
				     const u8 *iv);

static int default_set_key(struct cipher_context *cx, 
			   const u8 *key, int key_len);
static int default_set_key_atomic(struct cipher_context *cx, 
				  const u8 *key, int key_len);

/* digests */
static int default_open (struct digest_context *cx);
static int default_open_atomic (struct digest_context *cx);

static int default_update(struct digest_context *cx, const u8 *in, int size);
static int default_update_atomic(struct digest_context *cx, 
				 const u8 *in, int size);

static int default_digest(struct digest_context *cx, u8 *out);
static int default_digest_atomic(struct digest_context *cx, u8 *out);

static int default_close(struct digest_context *cx, u8 *out);
static int default_close_atomic(struct digest_context *cx, u8 *out);

static int default_hmac(struct digest_context *cx, const u8 *key, int key_len,
				const u8 *in, int size, u8 *hmac);
static int default_hmac_atomic(struct digest_context *cx, const u8 *key, int key_len,
			       const u8 *in, int size, u8 *hmac);

#ifdef CONFIG_PROC_FS
static int cipher_read_proc(char *page, char **start, off_t off,
			    int count, int *eof, void *data);
static int digest_read_proc(char *page, char **start, off_t off,
			    int count, int *eof, void *data);
static void crypto_create_proc(void);
static void crypto_delete_proc(void);
#endif /* CONFIG_PROC_FS */

static LIST_HEAD(ciphers);
static LIST_HEAD(digests);

static struct transform_group transforms[MAX_TRANSFORM] = {
	/* digest */
	{ TRANSFORM_DIGEST, "digest", RW_LOCK_UNLOCKED, &digests, 
#ifdef CONFIG_PROC_FS
	  NULL, &digest_read_proc 
#endif /* CONFIG_PROC_FS */
	},
	/* cipher */
	{ TRANSFORM_CIPHER, "cipher", RW_LOCK_UNLOCKED, &ciphers, 
#ifdef CONFIG_PROC_FS
	  NULL, &cipher_read_proc 
#endif /* CONFIG_PROC_FS */
	}
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0) && defined(CONFIG_PROC_FS)
static struct proc_dir_entry *
create_proc_read_entry(const char *name,
		       mode_t mode, struct proc_dir_entry *base, 
		       read_proc_t *read_proc, void * data)
{
	struct proc_dir_entry *res=create_proc_entry(name,mode,base);
	if (res) {
		res->read_proc=read_proc;
		res->data=data;
	}
	return res;
}
#endif /* defined(CONFIG_PROC_FS) */


/**
 * find_transform_by_name - Find transform implementation
 * @name: The name of the transform.
 * @tgroup: The identifier for the transform group the transform belongs to.
 *
 * Returns a ptr to the transform on success, NULL on failure.
 * Valid tgroup values are:
 *
 * %TRANSFORM_CIPHER - When looking for ciphers
 *
 * %TRANSFORM_DIGEST - When looking for digests
 *
 * You might want to use the wrapper-functions
 * find_cipher_by_name(const char *name), and
 * find_digest_by_name(const char *name) instead of this one.
 */
struct transform_implementation *
find_transform_by_name(const char *name, int tgroup, int atomicapi)
{
	struct list_head *tmp;
	struct transform_group *tg;
#ifdef CONFIG_KMOD
	char module_name[200];
	char *p;
	int n;
#endif /* CONFIG_KMOD */

	if (tgroup >= MAX_TRANSFORM)
		return NULL;
	tg = &transforms[tgroup];

#ifdef CONFIG_KMOD
	n = strlen(name) + strlen(tg->tg_name) + 3;
	if (n >= 200) n = 200;
	snprintf(module_name, n, "%s-%s-", tg->tg_name, name);
retry:
#endif /* CONFIG_KMOD */
	read_lock(&tg->tg_lock);
	for (tmp = tg->tg_head->next; tmp != tg->tg_head; tmp = tmp->next) {
		struct transform_implementation *t;
		t = list_entry(tmp, struct transform_implementation, t_list);
		if (strcmp(t->t_name, name) == 0) {
			if (!atomicapi || t->t_atomicapi) {
				read_unlock(&tg->tg_lock);
				return t;
			}
		}
	}

	/* transform not found */
	read_unlock(&tg->tg_lock);

#ifdef CONFIG_KMOD
	/* We try loading more and more general modules in succession.
	 * For example, if the module_name initially is set to
	 * "cipher-blowfish-cbc", we first try a module called
	 * "cipher-blowfish-cbc", then "cipher-blowfish" */
	if ((p = strrchr(module_name, '-')) != NULL) {
		*p = 0;
		printk(KERN_DEBUG "cryptoapi: trying %s\n", module_name);
		request_module(module_name);
		goto retry;
	}
#endif /* CONFIG_KMOD */
	return NULL;
}

/**
 * register_transform - Register new transform.
 * @ti: Initialized transform implementation struct.
 * @tgroup: The identifier for the transform group the transform
 *          should belong to.
 *
 * Adds a transform from the crypto API. ti->t_group is set to point
 * to the correct transform group according to tgroup, the transform
 * is added to the group's transform-list, and a /proc files are
 * created if CONFIG_PROC_FS=y Returns 0 on success.  Valid tgroup
 * values are:
 *
 * %TRANSFORM_CIPHER - When adding ciphers
 *
 * %TRANSFORM_DIGEST - When adding digests
 *
 */
int 
register_transform(struct transform_implementation *ti, int tgroup)
{
	int err = 0;
	struct transform_group *tg;
	
	if (tgroup >= MAX_TRANSFORM) {
		return -1;
	}
	INIT_LIST_HEAD(&ti->t_list);
	tg = ti->t_group = &transforms[tgroup];
	write_lock(&ti->t_group->tg_lock);
	list_add(&ti->t_list, ti->t_group->tg_head);
	write_unlock(&ti->t_group->tg_lock);
	if (!err) {
		MOD_INC_USE_COUNT;
//		printk(KERN_INFO "cryptoapi: Registered %s (%d)\n", 
//			ti->t_name, ti->t_flags);
#ifdef CONFIG_PROC_FS
		ti->t_proc = create_proc_read_entry(ti->t_name,
						    S_IFREG|S_IRUGO,
						    tg->tg_proc_parent_dir,
						    tg->read_proc, (void *)ti);
#endif /* CONFIG_PROC_FS */
	}
	return err;
}

/**
 * unregister_transform - Unregister new transform.
 * @ti: Initialized transform implementation struct.
 *
 * Removes a transform from the crypto API.  Returns 0 on success,
 * non-zero on failure to remove /proc entry.
 *
 */
int 
unregister_transform(struct transform_implementation *ti)
{
	int ret = 0;

	if (!list_empty(&ti->t_list)) {
		write_lock(&ti->t_group->tg_lock);
		list_del(&ti->t_list);
		write_unlock(&ti->t_group->tg_lock);
		ret = 0;
	}

#ifdef CONFIG_PROC_FS
	if (ti->t_proc) {
		ti->t_proc = NULL;
		remove_proc_entry(ti->t_name, ti->t_group->tg_proc_parent_dir);
	}
#endif /* CONFIG_PROC_FS */

	if (!ret) {
		MOD_DEC_USE_COUNT;
//		printk(KERN_INFO "cryptoapi: Unregistered %s (%d)\n",
//		       ti->t_name, ti->t_flags);
	}

	return ret;
}

int 
register_cipher(struct cipher_implementation *ci)
{
	if (!ci->realloc_context) 
		ci->realloc_context = default_realloc_cipher_context;

	if (!ci->wipe_context) 
		ci->wipe_context = default_wipe_context;

	if (!ci->free_context) 
		ci->free_context = default_free_cipher_context;

	if (!ci->lock) 
		ci->lock = default_lock;

	if (!ci->unlock) 
		ci->unlock = default_unlock;

	if (ci->_encrypt && ci->_decrypt && ci->_set_key) {
		ci->encrypt_iv = default_encrypt_iv;
		ci->encrypt_atomic_iv = default_encrypt_atomic_iv;
		ci->decrypt_iv = default_decrypt_iv;
		ci->decrypt_atomic_iv = default_decrypt_atomic_iv;

		ci->encrypt = default_encrypt;
		ci->encrypt_atomic = default_encrypt_atomic;
		ci->decrypt = default_decrypt;
		ci->decrypt_atomic = default_decrypt_atomic;
		ci->set_key = default_set_key;
		ci->set_key_atomic = default_set_key_atomic;
		ci->trans.t_atomicapi = 1;
	}

	if (!ci->encrypt || !ci->decrypt || !ci->set_key) {
		return -EINVAL;
	}
	return register_transform((struct transform_implementation *)ci,
				  TRANSFORM_CIPHER);
}

int 
register_digest(struct digest_implementation *di)
{
	if (!di->realloc_context)
		di->realloc_context = default_realloc_digest_context;

	if (!di->free_context)
		di->free_context = default_free_digest_context;

	if (!di->lock)
		di->lock = default_lock;

	if (!di->unlock)
		di->unlock = default_unlock;

	if (di->_open && di->_update && di->_digest &&
	    di->_close && di->_hmac) 
	{
		di->open = default_open;
		di->open_atomic = default_open_atomic;

		di->update = default_update;
		di->update_atomic = default_update_atomic;

		di->digest = default_digest;
		di->digest_atomic = default_digest_atomic;

		di->close = default_close;
		di->close_atomic = default_close_atomic;

		di->hmac = default_hmac;
		di->hmac_atomic = default_hmac_atomic;
		di->trans.t_atomicapi = 1;
	}

	if (!di->open || !di->update || !di->digest || !di->close)
		return -EINVAL;

	return register_transform((struct transform_implementation *)di,
				  TRANSFORM_DIGEST);
}

int 
unregister_cipher(struct cipher_implementation *ci)
{
	return unregister_transform((struct transform_implementation *)ci);
}

int 
unregister_digest(struct digest_implementation *ci)
{
	return unregister_transform((struct transform_implementation *)ci);
}

struct cipher_context *
default_realloc_cipher_context(struct cipher_context *old_cx,
			       struct cipher_implementation *ci,
			       int max_key_len)
{
	struct cipher_context *cx;
	/* Default ciphers need the same amount of memory for any key
           size */
	if (old_cx) {
		return old_cx;
	}
	cx = kmalloc(sizeof(struct cipher_context) +
		     ci->key_schedule_size, GFP_KERNEL);
	if (!cx) {
		return NULL;
	}
	cx->ci = ci;
	cx->keyinfo = (void *)((char *)cx)+sizeof(struct cipher_context);
	(void) max_key_len; /* Make gcc happy */
	return cx;
}

struct digest_context *
default_realloc_digest_context(struct digest_context *old_cx,
			       struct digest_implementation *di)
{
	struct digest_context *cx;

	if (old_cx)
		di->free_context (old_cx);

	cx = kmalloc(sizeof (struct digest_context) +
		      di->working_size, GFP_KERNEL);
	if (!cx)
		return NULL;

	cx->di = di;

	/* let digest_info point behind the context */
	cx->digest_info = (void *)((char *)cx) + sizeof(struct digest_context);

	return cx;
}

void
default_wipe_context(struct cipher_context *cx)
{
	struct cipher_implementation *ci = cx->ci;
	u32 *keyinfo = cx->keyinfo;
	memset(cx->keyinfo, 0, ci->key_schedule_size);
	memset(cx, 0, sizeof(struct cipher_context));
	cx->ci = ci;
	cx->keyinfo = keyinfo;
}

void
default_free_cipher_context(struct cipher_context *cx)
{
	kfree(cx);
}

void
default_free_digest_context(struct digest_context *cx)
{
	kfree(cx);
}

void 
default_lock (void)
{
}

void
default_unlock (void)
{
}

static int 
default_encrypt(struct cipher_context *cx, const u8 *in, u8 *out, int size)
{
	return cx->ci->_encrypt(cx, in, out, size, 0, cx->iv);
}

static int 
default_encrypt_atomic(struct cipher_context *cx, const u8 *in, u8 *out, 
		       int size)
{
	return cx->ci->_encrypt(cx, in, out, size, 1, cx->iv);
}

static int 
default_encrypt_iv(struct cipher_context *cx, const u8 *in, u8 *out, int size,
		const u8 *iv)
{
	return cx->ci->_encrypt(cx, in, out, size, 0, iv);
}

static int 
default_encrypt_atomic_iv(struct cipher_context *cx, const u8 *in, u8 *out, 
		          int size, const u8 *iv)
{
	return cx->ci->_encrypt(cx, in, out, size, 1, iv);
}

static int 
default_decrypt(struct cipher_context *cx, const u8 *in, u8 *out, int size)
{
	return cx->ci->_decrypt(cx, in, out, size, 0, cx->iv);
}

static int 
default_decrypt_atomic(struct cipher_context *cx, const u8 *in, u8 *out, 
		       int size)
{
	return cx->ci->_decrypt(cx, in, out, size, 1, cx->iv);
}

static int 
default_decrypt_iv(struct cipher_context *cx, const u8 *in, u8 *out, int size,
		const u8 *iv)
{
	return cx->ci->_decrypt(cx, in, out, size, 0, iv);
}

static int 
default_decrypt_atomic_iv(struct cipher_context *cx, const u8 *in, u8 *out, 
		       int size, const u8 *iv)
{
	return cx->ci->_decrypt(cx, in, out, size, 1, iv);
}


static int
default_set_key(struct cipher_context *cx, const u8 *key, int key_len)
{
	return cx->ci->_set_key(cx, key, key_len, 0);
}

static int
default_set_key_atomic(struct cipher_context *cx, const u8 *key, 
		       int key_len)
{
	return cx->ci->_set_key(cx, key, key_len, 1);
}

static int
default_open (struct digest_context *cx)
{
  return cx->di->_open (cx, 0);
}

static int
default_open_atomic (struct digest_context *cx)
{
	return cx->di->_open (cx, 1);
}

static int
default_update(struct digest_context *cx, const u8 *in, int size)
{
	return cx->di->_update (cx, in, size, 0);
}

static int
default_update_atomic(struct digest_context *cx, const u8 *in, int size)
{
	return cx->di->_update (cx, in, size, 1);
}

static int
default_digest(struct digest_context *cx, u8 *out)
{
	return cx->di->_digest (cx, out, 0);
}

static int
default_digest_atomic(struct digest_context *cx, u8 *out)
{
	return cx->di->_digest (cx, out, 1);
}

static int
default_close(struct digest_context *cx, u8 *out)
{
	return cx->di->_close (cx, out, 0);
}

static int
default_close_atomic(struct digest_context *cx, u8 *out)
{
	return cx->di->_close (cx, out, 1);
}

static int
default_hmac (struct digest_context *cx, const u8 *key, int key_len,
	     const u8 *in, int size, u8 *hmac)
{
	if (!(cx && key && in && hmac)) {
		printk (KERN_ERR "%s: some parameter is null\n", __PRETTY_FUNCTION__);
		return -EINVAL;
	}

	return cx->di->_hmac (cx, key, key_len, in, size, hmac, 0);
}

static int
default_hmac_atomic (struct digest_context *cx, const u8 *key, int key_len,
		    const u8 *in, int size, u8 *hmac)
{
	if (!(cx && key && in && hmac)) {
		printk (KERN_ERR "%s: some parameter is null\n", __PRETTY_FUNCTION__);
		return -EINVAL;
	}

	return cx->di->_hmac (cx, key, key_len, in, size, hmac, 1);
}

#ifdef CONFIG_PROC_FS
static int cipher_read_proc(char *page, char **start, off_t off,
			    int count, int *eof, void *data)
{
	struct cipher_implementation *ci;
	int len = 0;
	
	ci = (struct cipher_implementation *)data;

	len = sprintf(page,
		      "cipher_name:       %s\n"
		      "cipher_flags:      %d\n"
		      "blocksize:         %d\n"
		      "keysize_mask:      0x%08x\n"
		      "ivsize:            %d\n"
		      "key_schedule_size: %d\n",
		      ci->trans.t_name, ci->trans.t_flags, 
		      ci->blocksize, ci->key_size_mask,
		      ci->ivsize, ci->key_schedule_size);
	*eof=1;

	return len;
}

static int digest_read_proc(char *page, char **start, off_t off,
			int count, int *eof, void *data)
{
	struct digest_implementation *ci;
	int len = 0;
	
	ci = (struct digest_implementation *)data;

	len = sprintf(page, "digest_name:       %s\n"
		      "digest_flags:      %d\n"
		      "blocksize:         %d\n"
		      "working_size:      %d\n",
		      ci->trans.t_name, ci->trans.t_flags, 
		      ci->blocksize, ci->working_size);
	*eof=1;

	return len;
}

static void crypto_create_proc(void)
{
	int i;

	proc_crypto = proc_mkdir("crypto", NULL);
	for (i = 0; i < (sizeof(transforms) / sizeof(struct transform_group));
	     i++) {
		transforms[i].tg_proc_parent_dir = 
			proc_mkdir(transforms[i].tg_name, proc_crypto);
	}
}

static void crypto_delete_proc(void)
{
	int i;

	for (i = 0; i < (sizeof(transforms) / sizeof(struct transform_group));
	     i++) {
		struct proc_dir_entry *p = transforms[i].tg_proc_parent_dir;
		remove_proc_entry(transforms[i].tg_name, p);
	}

	remove_proc_entry("crypto", NULL);
}
#endif /* CONFIG_PROC_FS */

static int __init
init_cryptoapi(void)
{
#ifdef CONFIG_PROC_FS
	crypto_create_proc();
#endif /* CONFIG_PROC_FS */

//	printk(KERN_INFO "cryptoapi: loaded\n");

	return 0;
}

static void __exit
cleanup_cryptoapi(void) 
{
#ifdef CONFIG_PROC_FS
	crypto_delete_proc();
#endif /* CONFIG_PROC_FS */

//	printk(KERN_INFO "cryptoapi: unloaded\n");
}

module_init(init_cryptoapi);
module_exit(cleanup_cryptoapi);

EXPORT_SYMBOL(find_transform_by_name);
EXPORT_SYMBOL(register_transform);
EXPORT_SYMBOL(unregister_transform);
EXPORT_SYMBOL(register_cipher);
EXPORT_SYMBOL(unregister_cipher);
EXPORT_SYMBOL(register_digest);
EXPORT_SYMBOL(unregister_digest);

/* 
 * Overrides for Emacs so that we follow Linus's tabbing style.
 * Emacs will notice this stuff at the end of the file and automatically
 * adjust the settings for this buffer only.  This must remain at the end
 * of the file.
 * ---------------------------------------------------------------------------     
 * Local variables:
 * c-indent-level: 8
 * c-brace-imaginary-offset: 0
 * c-brace-offset: -8
 * c-argdecl-indent: 8
 * c-label-offset: -8
 * c-continued-statement-offset: 8
 * c-continued-brace-offset: 0
 * End:
 */
