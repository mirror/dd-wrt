/*
 *  Copyright 2002 Tobias Ringstrom <tobias@ringstrom.mine.nu>
 *  Authentication Copyright 2002 Arcturus Networks Inc.
 *      by Norman Shulman <norm@arcturusnetworks.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/crypto.h>
#include <linux/compiler.h>

#include "ipsec_sa.h"

LIST_HEAD(ipsec_sa_list);
rwlock_t ipsec_sa_lock = RW_LOCK_UNLOCKED;

static void
ipsec_sa_free(struct ipsec_sa *sa)
{
	if (sa->cipher)
	{
		struct cipher_implementation *ci = sa->cipher->ci;

		ci->wipe_context(sa->cipher);
		ci->free_context(sa->cipher);
		ci->unlock();
	}

	if (sa->digest)
	{
		struct digest_implementation *di = sa->digest->di;

		memset(sa->digest_key, 0, sa->digest_keylen);
		di->free_context(sa->digest);
		di->unlock();
	}

	kfree(sa);
}

int
ipsec_sa_init(void)
{
	INIT_LIST_HEAD(&ipsec_sa_list);

	return 0;
}

void
ipsec_sa_destroy(void)
{
	while (!list_empty(&ipsec_sa_list))
	{
		struct ipsec_sa *sa = list_entry(ipsec_sa_list.next, struct ipsec_sa, list);

		list_del(&sa->list);
		if (atomic_read(&sa->users) != 0)
		{
			printk(KERN_WARNING "Deleting SA in use (spi=0x%x, users=%d)\n",
				   sa->spi, atomic_read(&sa->users));
		}
		ipsec_sa_free(sa);
	}
}

static int
add_cipher(struct ipsec_sa *sa,
		   const char *cipher_name,
		   const void *cipher_key,
		   int cipher_keylen)
{
	struct cipher_implementation *ci;

	if (cipher_name[0] == '\0')
	{
		sa->cipher = NULL;
		return 0;
	}

	/* Find a cipher with atomic capabilities. */
	if ((ci = find_cipher_by_name(cipher_name, 1)) == NULL)
		return -ENOENT;

	ci->lock();
	if ((sa->cipher = ci->realloc_context(NULL, ci, cipher_keylen)) == NULL)
	{
		ci->unlock ();
		return -ENOMEM;
	}

	if (ci->set_key(sa->cipher, cipher_key, cipher_keylen) < 0)
	{
		ci->wipe_context(sa->cipher);
		ci->free_context(sa->cipher);
		ci->unlock();
		return -EPROTO;
	}

	return 0;
}

static int
add_digest(struct ipsec_sa *sa,
		   const char *digest_name,
		   const void *digest_key,
		   int digest_keylen,
		   int digest_hmaclen)
{
	struct digest_implementation *di;

	if (digest_name[0] == '\0')
	{
		sa->digest = NULL;
		return 0;
	}

	if (digest_keylen > IPSEC_MAX_DIGEST_KEYLEN ||
		digest_hmaclen > digest_keylen)
	{
		return -EPROTO;
	}

	/* Find an auth with atomic capabilities. */
	if ((di = find_digest_by_name(digest_name, 1)) == NULL)
		return -ENOENT;

	di->lock();
	if ((sa->digest = di->realloc_context(NULL, di)) == NULL)
	{
		di->unlock();
		return -ENOMEM;
	}

	sa->digest_keylen = digest_keylen;
	memcpy(sa->digest_key, digest_key, digest_keylen);

	sa->digest_hmaclen = digest_hmaclen;

	return 0;
}

int
ipsec_sa_add(u32 dst,
			 u32 src,
			 u32 spi,
			 u32 flags,
			 const char *cipher,
			 const void *cipher_key,
			 int cipher_keylen,
			 const char *digest,
			 const void *digest_key,
			 int digest_keylen,
			 int digest_hmaclen)
{
	struct list_head *l;
	struct ipsec_sa *sa;
	unsigned long irqflags;
	int err;

	if (spi == IPSEC_SPI_ANY)
		return -EINVAL;

	sa = kmalloc(sizeof(*sa), GFP_KERNEL);
	if (sa == NULL)
		return -ENOMEM;

	memset(sa, 0, sizeof(struct ipsec_sa));
	atomic_set(&sa->users, 0);
	sa->dst = dst;
	sa->src = src;
	sa->spi = spi;

	if ((err = add_cipher(sa, cipher, cipher_key, cipher_keylen)) != 0)
		return err;

	if ((err = add_digest(sa, digest, digest_key, digest_keylen, digest_hmaclen)) != 0)
	{
		ipsec_sa_free(sa);
		return err;
	}

	write_lock_irqsave(&ipsec_sa_lock, irqflags);
	/* Check for duplicates */
	list_for_each(l, &ipsec_sa_list)
	{
		struct ipsec_sa *sa2 = list_entry(l, struct ipsec_sa, list);
		if (sa2->dst == dst && sa2->src == src && sa2->spi == spi)
		{
			ipsec_sa_free(sa);
			write_unlock_irqrestore(&ipsec_sa_lock, irqflags);
			return -EEXIST;
		}
	}
	list_add_tail(&sa->list, &ipsec_sa_list);
	write_unlock_irqrestore(&ipsec_sa_lock, irqflags);

	return 0;
}

struct ipsec_sa*
ipsec_sa_get(u32 dst, u32 src, u32 spi)
{
	struct list_head *l;
	unsigned long flags;

	read_lock_irqsave(&ipsec_sa_lock, flags);
	list_for_each(l, &ipsec_sa_list)
	{
		struct ipsec_sa *sa = list_entry(l, struct ipsec_sa, list);
		if (sa->dst == dst && sa->src == src &&
			(spi == IPSEC_SPI_ANY || sa->spi == spi))
		{
			atomic_inc(&sa->users);
			read_unlock_irqrestore(&ipsec_sa_lock, flags);
			return sa;
		}
	}
	read_unlock_irqrestore(&ipsec_sa_lock, flags);

	return NULL;
}

struct ipsec_sa*
ipsec_sa_get_num(int n)
{
	struct list_head *l;
	unsigned long flags;
	struct ipsec_sa *sa = NULL;

	read_lock_irqsave(&ipsec_sa_lock, flags);
	list_for_each(l, &ipsec_sa_list)
	{
		if (n-- == 0)
		{
			sa = list_entry(l, struct ipsec_sa, list);
			atomic_inc(&sa->users);
			break;
		}
	}
	read_unlock_irqrestore(&ipsec_sa_lock, flags);

	return sa;
}

void
ipsec_sa_put(struct ipsec_sa *sa)
{
	atomic_dec(&sa->users);
	if (unlikely(atomic_read(&sa->users) < 0))
		BUG();
}

int
ipsec_sa_del(u32 dst, u32 src, u32 spi)
{
	struct list_head *l, *tmp;
	unsigned long flags;
	int n = 0;

	write_lock_irqsave(&ipsec_sa_lock, flags);
	list_for_each_safe(l, tmp, &ipsec_sa_list)
	{
		struct ipsec_sa *sa = list_entry(l, struct ipsec_sa, list);
		/* Note that sa->users cannot increase while the write lock is held. */
		if ((dst == INADDR_ANY || sa->dst == dst) &&
			(src == INADDR_ANY || sa->src == src) &&
			(spi == IPSEC_SPI_ANY || sa->spi == spi) &&
			atomic_read(&sa->users) == 0)
		{
			list_del(&sa->list);
			ipsec_sa_free(sa);
			++n;
		}
	}
	write_unlock_irqrestore(&ipsec_sa_lock, flags);

	return n;
}
