// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/nospec.h>
#include <linux/hugetlb.h>
#include <linux/compat.h>
#include <linux/io_uring.h>

#include <uapi/linux/io_uring.h>

#include "io_uring.h"
#include "alloc_cache.h"
#include "openclose.h"
#include "rsrc.h"
#include "memmap.h"
#include "register.h"

struct io_rsrc_update {
	struct file			*file;
	u64				arg;
	u32				nr_args;
	u32				offset;
};

static void io_rsrc_buf_put(struct io_ring_ctx *ctx, struct io_rsrc_put *prsrc);
static int io_sqe_buffer_register(struct io_ring_ctx *ctx, struct iovec *iov,
				  struct io_mapped_ubuf **pimu,
				  struct page **last_hpage);

/* only define max */
#define IORING_MAX_FIXED_FILES	(1U << 20)
#define IORING_MAX_REG_BUFFERS	(1U << 14)

static const struct io_mapped_ubuf dummy_ubuf = {
	/* set invalid range, so io_import_fixed() fails meeting it */
	.ubuf = -1UL,
	.len = UINT_MAX,
};

int __io_account_mem(struct user_struct *user, unsigned long nr_pages)
{
	unsigned long page_limit, cur_pages, new_pages;

	if (!nr_pages)
		return 0;

	/* Don't allow more pages than we can safely lock */
	page_limit = rlimit(RLIMIT_MEMLOCK) >> PAGE_SHIFT;

	cur_pages = atomic_long_read(&user->locked_vm);
	do {
		new_pages = cur_pages + nr_pages;
		if (new_pages > page_limit)
			return -ENOMEM;
	} while (!atomic_long_try_cmpxchg(&user->locked_vm,
					  &cur_pages, new_pages));
	return 0;
}

static void io_unaccount_mem(struct io_ring_ctx *ctx, unsigned long nr_pages)
{
	if (ctx->user)
		__io_unaccount_mem(ctx->user, nr_pages);

	if (ctx->mm_account)
		atomic64_sub(nr_pages, &ctx->mm_account->pinned_vm);
}

static int io_account_mem(struct io_ring_ctx *ctx, unsigned long nr_pages)
{
	int ret;

	if (ctx->user) {
		ret = __io_account_mem(ctx->user, nr_pages);
		if (ret)
			return ret;
	}

	if (ctx->mm_account)
		atomic64_add(nr_pages, &ctx->mm_account->pinned_vm);

	return 0;
}

static int io_buffer_validate(struct iovec *iov)
{
	unsigned long tmp, acct_len = iov->iov_len + (PAGE_SIZE - 1);

	/*
	 * Don't impose further limits on the size and buffer
	 * constraints here, we'll -EINVAL later when IO is
	 * submitted if they are wrong.
	 */
	if (!iov->iov_base)
		return iov->iov_len ? -EFAULT : 0;
	if (!iov->iov_len)
		return -EFAULT;

	/* arbitrary limit, but we need something */
	if (iov->iov_len > SZ_1G)
		return -EFAULT;

	if (check_add_overflow((unsigned long)iov->iov_base, acct_len, &tmp))
		return -EOVERFLOW;

	return 0;
}

static void io_buffer_unmap(struct io_ring_ctx *ctx, struct io_mapped_ubuf **slot)
{
	struct io_mapped_ubuf *imu = *slot;
	unsigned int i;

	*slot = NULL;
	if (imu != &dummy_ubuf) {
		if (!refcount_dec_and_test(&imu->refs))
			return;
		for (i = 0; i < imu->nr_bvecs; i++) {
			struct folio *folio = page_folio(imu->bvec[i].bv_page);

			unpin_user_folio(folio, 1);
		}
		if (imu->acct_pages)
			io_unaccount_mem(ctx, imu->acct_pages);
		kvfree(imu);
	}
}

static void io_rsrc_put_work(struct io_rsrc_node *node)
{
	struct io_rsrc_put *prsrc = &node->item;

	if (prsrc->tag)
		io_post_aux_cqe(node->ctx, prsrc->tag, 0, 0);

	switch (node->type) {
	case IORING_RSRC_FILE:
		fput(prsrc->file);
		break;
	case IORING_RSRC_BUFFER:
		io_rsrc_buf_put(node->ctx, prsrc);
		break;
	default:
		WARN_ON_ONCE(1);
		break;
	}
}

void io_rsrc_node_destroy(struct io_ring_ctx *ctx, struct io_rsrc_node *node)
{
	if (!io_alloc_cache_put(&ctx->rsrc_node_cache, node))
		kfree(node);
}

void io_rsrc_node_ref_zero(struct io_rsrc_node *node)
	__must_hold(&node->ctx->uring_lock)
{
	struct io_ring_ctx *ctx = node->ctx;

	while (!list_empty(&ctx->rsrc_ref_list)) {
		node = list_first_entry(&ctx->rsrc_ref_list,
					    struct io_rsrc_node, node);
		/* recycle ref nodes in order */
		if (node->refs)
			break;
		list_del(&node->node);

		if (likely(!node->empty))
			io_rsrc_put_work(node);
		io_rsrc_node_destroy(ctx, node);
	}
	if (list_empty(&ctx->rsrc_ref_list) && unlikely(ctx->rsrc_quiesce))
		wake_up_all(&ctx->rsrc_quiesce_wq);
}

struct io_rsrc_node *io_rsrc_node_alloc(struct io_ring_ctx *ctx)
{
	struct io_rsrc_node *ref_node;

	ref_node = io_alloc_cache_get(&ctx->rsrc_node_cache);
	if (!ref_node) {
		ref_node = kzalloc(sizeof(*ref_node), GFP_KERNEL);
		if (!ref_node)
			return NULL;
	}

	ref_node->ctx = ctx;
	ref_node->empty = 0;
	ref_node->refs = 1;
	return ref_node;
}

__cold static int io_rsrc_ref_quiesce(struct io_rsrc_data *data,
				      struct io_ring_ctx *ctx)
{
	struct io_rsrc_node *backup;
	DEFINE_WAIT(we);
	int ret;

	/* As We may drop ->uring_lock, other task may have started quiesce */
	if (data->quiesce)
		return -ENXIO;

	backup = io_rsrc_node_alloc(ctx);
	if (!backup)
		return -ENOMEM;
	ctx->rsrc_node->empty = true;
	ctx->rsrc_node->type = -1;
	list_add_tail(&ctx->rsrc_node->node, &ctx->rsrc_ref_list);
	io_put_rsrc_node(ctx, ctx->rsrc_node);
	ctx->rsrc_node = backup;

	if (list_empty(&ctx->rsrc_ref_list))
		return 0;

	if (ctx->flags & IORING_SETUP_DEFER_TASKRUN) {
		atomic_set(&ctx->cq_wait_nr, 1);
		smp_mb();
	}

	ctx->rsrc_quiesce++;
	data->quiesce = true;
	do {
		prepare_to_wait(&ctx->rsrc_quiesce_wq, &we, TASK_INTERRUPTIBLE);
		mutex_unlock(&ctx->uring_lock);

		ret = io_run_task_work_sig(ctx);
		if (ret < 0) {
			finish_wait(&ctx->rsrc_quiesce_wq, &we);
			mutex_lock(&ctx->uring_lock);
			if (list_empty(&ctx->rsrc_ref_list))
				ret = 0;
			break;
		}

		schedule();
		mutex_lock(&ctx->uring_lock);
		ret = 0;
	} while (!list_empty(&ctx->rsrc_ref_list));

	finish_wait(&ctx->rsrc_quiesce_wq, &we);
	data->quiesce = false;
	ctx->rsrc_quiesce--;

	if (ctx->flags & IORING_SETUP_DEFER_TASKRUN) {
		atomic_set(&ctx->cq_wait_nr, 0);
		smp_mb();
	}
	return ret;
}

static void io_free_page_table(void **table, size_t size)
{
	unsigned i, nr_tables = DIV_ROUND_UP(size, PAGE_SIZE);

	for (i = 0; i < nr_tables; i++)
		kfree(table[i]);
	kfree(table);
}

static void io_rsrc_data_free(struct io_rsrc_data *data)
{
	size_t size = data->nr * sizeof(data->tags[0][0]);

	if (data->tags)
		io_free_page_table((void **)data->tags, size);
	kfree(data);
}

static __cold void **io_alloc_page_table(size_t size)
{
	unsigned i, nr_tables = DIV_ROUND_UP(size, PAGE_SIZE);
	size_t init_size = size;
	void **table;

	table = kcalloc(nr_tables, sizeof(*table), GFP_KERNEL_ACCOUNT);
	if (!table)
		return NULL;

	for (i = 0; i < nr_tables; i++) {
		unsigned int this_size = min_t(size_t, size, PAGE_SIZE);

		table[i] = kzalloc(this_size, GFP_KERNEL_ACCOUNT);
		if (!table[i]) {
			io_free_page_table(table, init_size);
			return NULL;
		}
		size -= this_size;
	}
	return table;
}

__cold static int io_rsrc_data_alloc(struct io_ring_ctx *ctx, int type,
				     u64 __user *utags,
				     unsigned nr, struct io_rsrc_data **pdata)
{
	struct io_rsrc_data *data;
	int ret = 0;
	unsigned i;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;
	data->tags = (u64 **)io_alloc_page_table(nr * sizeof(data->tags[0][0]));
	if (!data->tags) {
		kfree(data);
		return -ENOMEM;
	}

	data->nr = nr;
	data->ctx = ctx;
	data->rsrc_type = type;
	if (utags) {
		ret = -EFAULT;
		for (i = 0; i < nr; i++) {
			u64 *tag_slot = io_get_tag_slot(data, i);

			if (copy_from_user(tag_slot, &utags[i],
					   sizeof(*tag_slot)))
				goto fail;
		}
	}
	*pdata = data;
	return 0;
fail:
	io_rsrc_data_free(data);
	return ret;
}

static int __io_sqe_files_update(struct io_ring_ctx *ctx,
				 struct io_uring_rsrc_update2 *up,
				 unsigned nr_args)
{
	u64 __user *tags = u64_to_user_ptr(up->tags);
	__s32 __user *fds = u64_to_user_ptr(up->data);
	struct io_rsrc_data *data = ctx->file_data;
	struct io_fixed_file *file_slot;
	int fd, i, err = 0;
	unsigned int done;

	if (!ctx->file_data)
		return -ENXIO;
	if (up->offset + nr_args > ctx->nr_user_files)
		return -EINVAL;

	for (done = 0; done < nr_args; done++) {
		u64 tag = 0;

		if ((tags && copy_from_user(&tag, &tags[done], sizeof(tag))) ||
		    copy_from_user(&fd, &fds[done], sizeof(fd))) {
			err = -EFAULT;
			break;
		}
		if ((fd == IORING_REGISTER_FILES_SKIP || fd == -1) && tag) {
			err = -EINVAL;
			break;
		}
		if (fd == IORING_REGISTER_FILES_SKIP)
			continue;

		i = array_index_nospec(up->offset + done, ctx->nr_user_files);
		file_slot = io_fixed_file_slot(&ctx->file_table, i);

		if (file_slot->file_ptr) {
			err = io_queue_rsrc_removal(data, i,
						    io_slot_file(file_slot));
			if (err)
				break;
			file_slot->file_ptr = 0;
			io_file_bitmap_clear(&ctx->file_table, i);
		}
		if (fd != -1) {
			struct file *file = fget(fd);

			if (!file) {
				err = -EBADF;
				break;
			}
			/*
			 * Don't allow io_uring instances to be registered.
			 */
			if (io_is_uring_fops(file)) {
				fput(file);
				err = -EBADF;
				break;
			}
			*io_get_tag_slot(data, i) = tag;
			io_fixed_file_set(file_slot, file);
			io_file_bitmap_set(&ctx->file_table, i);
		}
	}
	return done ? done : err;
}

static int __io_sqe_buffers_update(struct io_ring_ctx *ctx,
				   struct io_uring_rsrc_update2 *up,
				   unsigned int nr_args)
{
	u64 __user *tags = u64_to_user_ptr(up->tags);
	struct iovec fast_iov, *iov;
	struct page *last_hpage = NULL;
	struct iovec __user *uvec;
	u64 user_data = up->data;
	__u32 done;
	int i, err;

	if (!ctx->buf_data)
		return -ENXIO;
	if (up->offset + nr_args > ctx->nr_user_bufs)
		return -EINVAL;

	for (done = 0; done < nr_args; done++) {
		struct io_mapped_ubuf *imu;
		u64 tag = 0;

		uvec = u64_to_user_ptr(user_data);
		iov = iovec_from_user(uvec, 1, 1, &fast_iov, ctx->compat);
		if (IS_ERR(iov)) {
			err = PTR_ERR(iov);
			break;
		}
		if (tags && copy_from_user(&tag, &tags[done], sizeof(tag))) {
			err = -EFAULT;
			break;
		}
		err = io_buffer_validate(iov);
		if (err)
			break;
		if (!iov->iov_base && tag) {
			err = -EINVAL;
			break;
		}
		err = io_sqe_buffer_register(ctx, iov, &imu, &last_hpage);
		if (err)
			break;

		i = array_index_nospec(up->offset + done, ctx->nr_user_bufs);
		if (ctx->user_bufs[i] != &dummy_ubuf) {
			err = io_queue_rsrc_removal(ctx->buf_data, i,
						    ctx->user_bufs[i]);
			if (unlikely(err)) {
				io_buffer_unmap(ctx, &imu);
				break;
			}
			ctx->user_bufs[i] = (struct io_mapped_ubuf *)&dummy_ubuf;
		}

		ctx->user_bufs[i] = imu;
		*io_get_tag_slot(ctx->buf_data, i) = tag;
		if (ctx->compat)
			user_data += sizeof(struct compat_iovec);
		else
			user_data += sizeof(struct iovec);
	}
	return done ? done : err;
}

static int __io_register_rsrc_update(struct io_ring_ctx *ctx, unsigned type,
				     struct io_uring_rsrc_update2 *up,
				     unsigned nr_args)
{
	__u32 tmp;

	lockdep_assert_held(&ctx->uring_lock);

	if (check_add_overflow(up->offset, nr_args, &tmp))
		return -EOVERFLOW;

	switch (type) {
	case IORING_RSRC_FILE:
		return __io_sqe_files_update(ctx, up, nr_args);
	case IORING_RSRC_BUFFER:
		return __io_sqe_buffers_update(ctx, up, nr_args);
	}
	return -EINVAL;
}

int io_register_files_update(struct io_ring_ctx *ctx, void __user *arg,
			     unsigned nr_args)
{
	struct io_uring_rsrc_update2 up;

	if (!nr_args)
		return -EINVAL;
	memset(&up, 0, sizeof(up));
	if (copy_from_user(&up, arg, sizeof(struct io_uring_rsrc_update)))
		return -EFAULT;
	if (up.resv || up.resv2)
		return -EINVAL;
	return __io_register_rsrc_update(ctx, IORING_RSRC_FILE, &up, nr_args);
}

int io_register_rsrc_update(struct io_ring_ctx *ctx, void __user *arg,
			    unsigned size, unsigned type)
{
	struct io_uring_rsrc_update2 up;

	if (size != sizeof(up))
		return -EINVAL;
	if (copy_from_user(&up, arg, sizeof(up)))
		return -EFAULT;
	if (!up.nr || up.resv || up.resv2)
		return -EINVAL;
	return __io_register_rsrc_update(ctx, type, &up, up.nr);
}

__cold int io_register_rsrc(struct io_ring_ctx *ctx, void __user *arg,
			    unsigned int size, unsigned int type)
{
	struct io_uring_rsrc_register rr;

	/* keep it extendible */
	if (size != sizeof(rr))
		return -EINVAL;

	memset(&rr, 0, sizeof(rr));
	if (copy_from_user(&rr, arg, size))
		return -EFAULT;
	if (!rr.nr || rr.resv2)
		return -EINVAL;
	if (rr.flags & ~IORING_RSRC_REGISTER_SPARSE)
		return -EINVAL;

	switch (type) {
	case IORING_RSRC_FILE:
		if (rr.flags & IORING_RSRC_REGISTER_SPARSE && rr.data)
			break;
		return io_sqe_files_register(ctx, u64_to_user_ptr(rr.data),
					     rr.nr, u64_to_user_ptr(rr.tags));
	case IORING_RSRC_BUFFER:
		if (rr.flags & IORING_RSRC_REGISTER_SPARSE && rr.data)
			break;
		return io_sqe_buffers_register(ctx, u64_to_user_ptr(rr.data),
					       rr.nr, u64_to_user_ptr(rr.tags));
	}
	return -EINVAL;
}

int io_files_update_prep(struct io_kiocb *req, const struct io_uring_sqe *sqe)
{
	struct io_rsrc_update *up = io_kiocb_to_cmd(req, struct io_rsrc_update);

	if (unlikely(req->flags & (REQ_F_FIXED_FILE | REQ_F_BUFFER_SELECT)))
		return -EINVAL;
	if (sqe->rw_flags || sqe->splice_fd_in)
		return -EINVAL;

	up->offset = READ_ONCE(sqe->off);
	up->nr_args = READ_ONCE(sqe->len);
	if (!up->nr_args)
		return -EINVAL;
	up->arg = READ_ONCE(sqe->addr);
	return 0;
}

static int io_files_update_with_index_alloc(struct io_kiocb *req,
					    unsigned int issue_flags)
{
	struct io_rsrc_update *up = io_kiocb_to_cmd(req, struct io_rsrc_update);
	__s32 __user *fds = u64_to_user_ptr(up->arg);
	unsigned int done;
	struct file *file;
	int ret, fd;

	if (!req->ctx->file_data)
		return -ENXIO;

	for (done = 0; done < up->nr_args; done++) {
		if (copy_from_user(&fd, &fds[done], sizeof(fd))) {
			ret = -EFAULT;
			break;
		}

		file = fget(fd);
		if (!file) {
			ret = -EBADF;
			break;
		}
		ret = io_fixed_fd_install(req, issue_flags, file,
					  IORING_FILE_INDEX_ALLOC);
		if (ret < 0)
			break;
		if (copy_to_user(&fds[done], &ret, sizeof(ret))) {
			__io_close_fixed(req->ctx, issue_flags, ret);
			ret = -EFAULT;
			break;
		}
	}

	if (done)
		return done;
	return ret;
}

int io_files_update(struct io_kiocb *req, unsigned int issue_flags)
{
	struct io_rsrc_update *up = io_kiocb_to_cmd(req, struct io_rsrc_update);
	struct io_ring_ctx *ctx = req->ctx;
	struct io_uring_rsrc_update2 up2;
	int ret;

	up2.offset = up->offset;
	up2.data = up->arg;
	up2.nr = 0;
	up2.tags = 0;
	up2.resv = 0;
	up2.resv2 = 0;

	if (up->offset == IORING_FILE_INDEX_ALLOC) {
		ret = io_files_update_with_index_alloc(req, issue_flags);
	} else {
		io_ring_submit_lock(ctx, issue_flags);
		ret = __io_register_rsrc_update(ctx, IORING_RSRC_FILE,
						&up2, up->nr_args);
		io_ring_submit_unlock(ctx, issue_flags);
	}

	if (ret < 0)
		req_set_fail(req);
	io_req_set_res(req, ret, 0);
	return IOU_OK;
}

int io_queue_rsrc_removal(struct io_rsrc_data *data, unsigned idx, void *rsrc)
{
	struct io_ring_ctx *ctx = data->ctx;
	struct io_rsrc_node *node = ctx->rsrc_node;
	u64 *tag_slot = io_get_tag_slot(data, idx);

	ctx->rsrc_node = io_rsrc_node_alloc(ctx);
	if (unlikely(!ctx->rsrc_node)) {
		ctx->rsrc_node = node;
		return -ENOMEM;
	}

	node->item.rsrc = rsrc;
	node->type = data->rsrc_type;
	node->item.tag = *tag_slot;
	*tag_slot = 0;
	list_add_tail(&node->node, &ctx->rsrc_ref_list);
	io_put_rsrc_node(ctx, node);
	return 0;
}

void __io_sqe_files_unregister(struct io_ring_ctx *ctx)
{
	int i;

	for (i = 0; i < ctx->nr_user_files; i++) {
		struct file *file = io_file_from_index(&ctx->file_table, i);

		if (!file)
			continue;
		io_file_bitmap_clear(&ctx->file_table, i);
		fput(file);
	}

	io_free_file_tables(&ctx->file_table);
	io_file_table_set_alloc_range(ctx, 0, 0);
	io_rsrc_data_free(ctx->file_data);
	ctx->file_data = NULL;
	ctx->nr_user_files = 0;
}

int io_sqe_files_unregister(struct io_ring_ctx *ctx)
{
	unsigned nr = ctx->nr_user_files;
	int ret;

	if (!ctx->file_data)
		return -ENXIO;

	/*
	 * Quiesce may unlock ->uring_lock, and while it's not held
	 * prevent new requests using the table.
	 */
	ctx->nr_user_files = 0;
	ret = io_rsrc_ref_quiesce(ctx->file_data, ctx);
	ctx->nr_user_files = nr;
	if (!ret)
		__io_sqe_files_unregister(ctx);
	return ret;
}

int io_sqe_files_register(struct io_ring_ctx *ctx, void __user *arg,
			  unsigned nr_args, u64 __user *tags)
{
	__s32 __user *fds = (__s32 __user *) arg;
	struct file *file;
	int fd, ret;
	unsigned i;

	if (ctx->file_data)
		return -EBUSY;
	if (!nr_args)
		return -EINVAL;
	if (nr_args > IORING_MAX_FIXED_FILES)
		return -EMFILE;
	if (nr_args > rlimit(RLIMIT_NOFILE))
		return -EMFILE;
	ret = io_rsrc_data_alloc(ctx, IORING_RSRC_FILE, tags, nr_args,
				 &ctx->file_data);
	if (ret)
		return ret;

	if (!io_alloc_file_tables(&ctx->file_table, nr_args)) {
		io_rsrc_data_free(ctx->file_data);
		ctx->file_data = NULL;
		return -ENOMEM;
	}

	for (i = 0; i < nr_args; i++, ctx->nr_user_files++) {
		struct io_fixed_file *file_slot;

		if (fds && copy_from_user(&fd, &fds[i], sizeof(fd))) {
			ret = -EFAULT;
			goto fail;
		}
		/* allow sparse sets */
		if (!fds || fd == -1) {
			ret = -EINVAL;
			if (unlikely(*io_get_tag_slot(ctx->file_data, i)))
				goto fail;
			continue;
		}

		file = fget(fd);
		ret = -EBADF;
		if (unlikely(!file))
			goto fail;

		/*
		 * Don't allow io_uring instances to be registered.
		 */
		if (io_is_uring_fops(file)) {
			fput(file);
			goto fail;
		}
		file_slot = io_fixed_file_slot(&ctx->file_table, i);
		io_fixed_file_set(file_slot, file);
		io_file_bitmap_set(&ctx->file_table, i);
	}

	/* default it to the whole table */
	io_file_table_set_alloc_range(ctx, 0, ctx->nr_user_files);
	return 0;
fail:
	__io_sqe_files_unregister(ctx);
	return ret;
}

static void io_rsrc_buf_put(struct io_ring_ctx *ctx, struct io_rsrc_put *prsrc)
{
	io_buffer_unmap(ctx, &prsrc->buf);
	prsrc->buf = NULL;
}

void __io_sqe_buffers_unregister(struct io_ring_ctx *ctx)
{
	unsigned int i;

	for (i = 0; i < ctx->nr_user_bufs; i++)
		io_buffer_unmap(ctx, &ctx->user_bufs[i]);
	kfree(ctx->user_bufs);
	io_rsrc_data_free(ctx->buf_data);
	ctx->user_bufs = NULL;
	ctx->buf_data = NULL;
	ctx->nr_user_bufs = 0;
}

int io_sqe_buffers_unregister(struct io_ring_ctx *ctx)
{
	unsigned nr = ctx->nr_user_bufs;
	int ret;

	if (!ctx->buf_data)
		return -ENXIO;

	/*
	 * Quiesce may unlock ->uring_lock, and while it's not held
	 * prevent new requests using the table.
	 */
	ctx->nr_user_bufs = 0;
	ret = io_rsrc_ref_quiesce(ctx->buf_data, ctx);
	ctx->nr_user_bufs = nr;
	if (!ret)
		__io_sqe_buffers_unregister(ctx);
	return ret;
}

/*
 * Not super efficient, but this is just a registration time. And we do cache
 * the last compound head, so generally we'll only do a full search if we don't
 * match that one.
 *
 * We check if the given compound head page has already been accounted, to
 * avoid double accounting it. This allows us to account the full size of the
 * page, not just the constituent pages of a huge page.
 */
static bool headpage_already_acct(struct io_ring_ctx *ctx, struct page **pages,
				  int nr_pages, struct page *hpage)
{
	int i, j;

	/* check current page array */
	for (i = 0; i < nr_pages; i++) {
		if (!PageCompound(pages[i]))
			continue;
		if (compound_head(pages[i]) == hpage)
			return true;
	}

	/* check previously registered pages */
	for (i = 0; i < ctx->nr_user_bufs; i++) {
		struct io_mapped_ubuf *imu = ctx->user_bufs[i];

		for (j = 0; j < imu->nr_bvecs; j++) {
			if (!PageCompound(imu->bvec[j].bv_page))
				continue;
			if (compound_head(imu->bvec[j].bv_page) == hpage)
				return true;
		}
	}

	return false;
}

static int io_buffer_account_pin(struct io_ring_ctx *ctx, struct page **pages,
				 int nr_pages, struct io_mapped_ubuf *imu,
				 struct page **last_hpage)
{
	int i, ret;

	imu->acct_pages = 0;
	for (i = 0; i < nr_pages; i++) {
		if (!PageCompound(pages[i])) {
			imu->acct_pages++;
		} else {
			struct page *hpage;

			hpage = compound_head(pages[i]);
			if (hpage == *last_hpage)
				continue;
			*last_hpage = hpage;
			if (headpage_already_acct(ctx, pages, i, hpage))
				continue;
			imu->acct_pages += page_size(hpage) >> PAGE_SHIFT;
		}
	}

	if (!imu->acct_pages)
		return 0;

	ret = io_account_mem(ctx, imu->acct_pages);
	if (ret)
		imu->acct_pages = 0;
	return ret;
}

static bool io_do_coalesce_buffer(struct page ***pages, int *nr_pages,
				struct io_imu_folio_data *data, int nr_folios)
{
	struct page **page_array = *pages, **new_array = NULL;
	int nr_pages_left = *nr_pages, i, j;

	/* Store head pages only*/
	new_array = kvmalloc_array(nr_folios, sizeof(struct page *),
					GFP_KERNEL);
	if (!new_array)
		return false;

	new_array[0] = compound_head(page_array[0]);
	/*
	 * The pages are bound to the folio, it doesn't
	 * actually unpin them but drops all but one reference,
	 * which is usually put down by io_buffer_unmap().
	 * Note, needs a better helper.
	 */
	if (data->nr_pages_head > 1)
		unpin_user_pages(&page_array[1], data->nr_pages_head - 1);

	j = data->nr_pages_head;
	nr_pages_left -= data->nr_pages_head;
	for (i = 1; i < nr_folios; i++) {
		unsigned int nr_unpin;

		new_array[i] = page_array[j];
		nr_unpin = min_t(unsigned int, nr_pages_left - 1,
					data->nr_pages_mid - 1);
		if (nr_unpin)
			unpin_user_pages(&page_array[j+1], nr_unpin);
		j += data->nr_pages_mid;
		nr_pages_left -= data->nr_pages_mid;
	}
	kvfree(page_array);
	*pages = new_array;
	*nr_pages = nr_folios;
	return true;
}

static bool io_try_coalesce_buffer(struct page ***pages, int *nr_pages,
					 struct io_imu_folio_data *data)
{
	struct page **page_array = *pages;
	struct folio *folio = page_folio(page_array[0]);
	unsigned int count = 1, nr_folios = 1;
	int i;

	if (*nr_pages <= 1)
		return false;

	data->nr_pages_mid = folio_nr_pages(folio);
	if (data->nr_pages_mid == 1)
		return false;

	data->folio_shift = folio_shift(folio);
	data->first_folio_page_idx = folio_page_idx(folio, page_array[0]);
	/*
	 * Check if pages are contiguous inside a folio, and all folios have
	 * the same page count except for the head and tail.
	 */
	for (i = 1; i < *nr_pages; i++) {
		if (page_folio(page_array[i]) == folio &&
			page_array[i] == page_array[i-1] + 1) {
			count++;
			continue;
		}

		if (nr_folios == 1) {
			if (folio_page_idx(folio, page_array[i-1]) !=
				data->nr_pages_mid - 1)
				return false;

			data->nr_pages_head = count;
		} else if (count != data->nr_pages_mid) {
			return false;
		}

		folio = page_folio(page_array[i]);
		if (folio_size(folio) != (1UL << data->folio_shift) ||
			folio_page_idx(folio, page_array[i]) != 0)
			return false;

		count = 1;
		nr_folios++;
	}
	if (nr_folios == 1)
		data->nr_pages_head = count;

	return io_do_coalesce_buffer(pages, nr_pages, data, nr_folios);
}

static int io_sqe_buffer_register(struct io_ring_ctx *ctx, struct iovec *iov,
				  struct io_mapped_ubuf **pimu,
				  struct page **last_hpage)
{
	struct io_mapped_ubuf *imu = NULL;
	struct page **pages = NULL;
	unsigned long off;
	size_t size;
	int ret, nr_pages, i;
	struct io_imu_folio_data data;
	bool coalesced;

	*pimu = (struct io_mapped_ubuf *)&dummy_ubuf;
	if (!iov->iov_base)
		return 0;

	ret = -ENOMEM;
	pages = io_pin_pages((unsigned long) iov->iov_base, iov->iov_len,
				&nr_pages);
	if (IS_ERR(pages)) {
		ret = PTR_ERR(pages);
		pages = NULL;
		goto done;
	}

	/* If it's huge page(s), try to coalesce them into fewer bvec entries */
	coalesced = io_try_coalesce_buffer(&pages, &nr_pages, &data);

	imu = kvmalloc(struct_size(imu, bvec, nr_pages), GFP_KERNEL);
	if (!imu)
		goto done;

	ret = io_buffer_account_pin(ctx, pages, nr_pages, imu, last_hpage);
	if (ret)
		goto done;

	size = iov->iov_len;
	/* store original address for later verification */
	imu->ubuf = (unsigned long) iov->iov_base;
	imu->len = iov->iov_len;
	imu->nr_bvecs = nr_pages;
	imu->folio_shift = PAGE_SHIFT;
	if (coalesced)
		imu->folio_shift = data.folio_shift;
	refcount_set(&imu->refs, 1);
	off = (unsigned long)iov->iov_base & ~PAGE_MASK;
	if (coalesced)
		off += data.first_folio_page_idx << PAGE_SHIFT;
	*pimu = imu;
	ret = 0;

	for (i = 0; i < nr_pages; i++) {
		size_t vec_len;

		vec_len = min_t(size_t, size, (1UL << imu->folio_shift) - off);
		bvec_set_page(&imu->bvec[i], pages[i], vec_len, off);
		off = 0;
		size -= vec_len;
	}
done:
	if (ret) {
		kvfree(imu);
		if (pages) {
			for (i = 0; i < nr_pages; i++)
				unpin_user_folio(page_folio(pages[i]), 1);
		}
	}
	kvfree(pages);
	return ret;
}

static int io_buffers_map_alloc(struct io_ring_ctx *ctx, unsigned int nr_args)
{
	ctx->user_bufs = kcalloc(nr_args, sizeof(*ctx->user_bufs), GFP_KERNEL);
	return ctx->user_bufs ? 0 : -ENOMEM;
}

int io_sqe_buffers_register(struct io_ring_ctx *ctx, void __user *arg,
			    unsigned int nr_args, u64 __user *tags)
{
	struct page *last_hpage = NULL;
	struct io_rsrc_data *data;
	struct iovec fast_iov, *iov = &fast_iov;
	const struct iovec __user *uvec;
	int i, ret;

	BUILD_BUG_ON(IORING_MAX_REG_BUFFERS >= (1u << 16));

	if (ctx->user_bufs)
		return -EBUSY;
	if (!nr_args || nr_args > IORING_MAX_REG_BUFFERS)
		return -EINVAL;
	ret = io_rsrc_data_alloc(ctx, IORING_RSRC_BUFFER, tags, nr_args, &data);
	if (ret)
		return ret;
	ret = io_buffers_map_alloc(ctx, nr_args);
	if (ret) {
		io_rsrc_data_free(data);
		return ret;
	}

	if (!arg)
		memset(iov, 0, sizeof(*iov));

	for (i = 0; i < nr_args; i++, ctx->nr_user_bufs++) {
		if (arg) {
			uvec = (struct iovec __user *) arg;
			iov = iovec_from_user(uvec, 1, 1, &fast_iov, ctx->compat);
			if (IS_ERR(iov)) {
				ret = PTR_ERR(iov);
				break;
			}
			ret = io_buffer_validate(iov);
			if (ret)
				break;
			if (ctx->compat)
				arg += sizeof(struct compat_iovec);
			else
				arg += sizeof(struct iovec);
		}

		if (!iov->iov_base && *io_get_tag_slot(data, i)) {
			ret = -EINVAL;
			break;
		}

		ret = io_sqe_buffer_register(ctx, iov, &ctx->user_bufs[i],
					     &last_hpage);
		if (ret)
			break;
	}

	WARN_ON_ONCE(ctx->buf_data);

	ctx->buf_data = data;
	if (ret)
		__io_sqe_buffers_unregister(ctx);
	return ret;
}

int io_import_fixed(int ddir, struct iov_iter *iter,
			   struct io_mapped_ubuf *imu,
			   u64 buf_addr, size_t len)
{
	u64 buf_end;
	size_t offset;

	if (WARN_ON_ONCE(!imu))
		return -EFAULT;
	if (unlikely(check_add_overflow(buf_addr, (u64)len, &buf_end)))
		return -EFAULT;
	/* not inside the mapped region */
	if (unlikely(buf_addr < imu->ubuf || buf_end > (imu->ubuf + imu->len)))
		return -EFAULT;

	/*
	 * Might not be a start of buffer, set size appropriately
	 * and advance us to the beginning.
	 */
	offset = buf_addr - imu->ubuf;
	iov_iter_bvec(iter, ddir, imu->bvec, imu->nr_bvecs, offset + len);

	if (offset) {
		/*
		 * Don't use iov_iter_advance() here, as it's really slow for
		 * using the latter parts of a big fixed buffer - it iterates
		 * over each segment manually. We can cheat a bit here, because
		 * we know that:
		 *
		 * 1) it's a BVEC iter, we set it up
		 * 2) all bvecs are the same in size, except potentially the
		 *    first and last bvec
		 *
		 * So just find our index, and adjust the iterator afterwards.
		 * If the offset is within the first bvec (or the whole first
		 * bvec, just use iov_iter_advance(). This makes it easier
		 * since we can just skip the first segment, which may not
		 * be folio_size aligned.
		 */
		const struct bio_vec *bvec = imu->bvec;

		if (offset < bvec->bv_len) {
			iter->bvec = bvec;
			iter->count -= offset;
			iter->iov_offset = offset;
		} else {
			unsigned long seg_skip;

			/* skip first vec */
			offset -= bvec->bv_len;
			seg_skip = 1 + (offset >> imu->folio_shift);

			iter->bvec = bvec + seg_skip;
			iter->nr_segs -= seg_skip;
			iter->count -= bvec->bv_len + offset;
			iter->iov_offset = offset & ((1UL << imu->folio_shift) - 1);
		}
	}

	return 0;
}

static int io_clone_buffers(struct io_ring_ctx *ctx, struct io_ring_ctx *src_ctx)
{
	struct io_mapped_ubuf **user_bufs;
	struct io_rsrc_data *data;
	int i, ret, nbufs;

	/*
	 * Accounting state is shared between the two rings; that only works if
	 * both rings are accounted towards the same counters.
	 */
	if (ctx->user != src_ctx->user || ctx->mm_account != src_ctx->mm_account)
		return -EINVAL;

	/*
	 * Drop our own lock here. We'll setup the data we need and reference
	 * the source buffers, then re-grab, check, and assign at the end.
	 */
	mutex_unlock(&ctx->uring_lock);

	mutex_lock(&src_ctx->uring_lock);
	ret = -ENXIO;
	nbufs = src_ctx->nr_user_bufs;
	if (!nbufs)
		goto out_unlock;
	ret = io_rsrc_data_alloc(ctx, IORING_RSRC_BUFFER, NULL, nbufs, &data);
	if (ret)
		goto out_unlock;

	ret = -ENOMEM;
	user_bufs = kcalloc(nbufs, sizeof(*ctx->user_bufs), GFP_KERNEL);
	if (!user_bufs)
		goto out_free_data;

	for (i = 0; i < nbufs; i++) {
		struct io_mapped_ubuf *src = src_ctx->user_bufs[i];

		if (src != &dummy_ubuf)
			refcount_inc(&src->refs);
		user_bufs[i] = src;
	}

	/* Have a ref on the bufs now, drop src lock and re-grab our own lock */
	mutex_unlock(&src_ctx->uring_lock);
	mutex_lock(&ctx->uring_lock);
	if (!ctx->user_bufs) {
		ctx->user_bufs = user_bufs;
		ctx->buf_data = data;
		ctx->nr_user_bufs = nbufs;
		return 0;
	}

	/* someone raced setting up buffers, dump ours */
	for (i = 0; i < nbufs; i++)
		io_buffer_unmap(ctx, &user_bufs[i]);
	io_rsrc_data_free(data);
	kfree(user_bufs);
	return -EBUSY;
out_free_data:
	io_rsrc_data_free(data);
out_unlock:
	mutex_unlock(&src_ctx->uring_lock);
	mutex_lock(&ctx->uring_lock);
	return ret;
}

/*
 * Copy the registered buffers from the source ring whose file descriptor
 * is given in the src_fd to the current ring. This is identical to registering
 * the buffers with ctx, except faster as mappings already exist.
 *
 * Since the memory is already accounted once, don't account it again.
 */
int io_register_clone_buffers(struct io_ring_ctx *ctx, void __user *arg)
{
	struct io_uring_clone_buffers buf;
	bool registered_src;
	struct file *file;
	int ret;

	if (ctx->user_bufs || ctx->nr_user_bufs)
		return -EBUSY;
	if (copy_from_user(&buf, arg, sizeof(buf)))
		return -EFAULT;
	if (buf.flags & ~IORING_REGISTER_SRC_REGISTERED)
		return -EINVAL;
	if (memchr_inv(buf.pad, 0, sizeof(buf.pad)))
		return -EINVAL;

	registered_src = (buf.flags & IORING_REGISTER_SRC_REGISTERED) != 0;
	file = io_uring_register_get_file(buf.src_fd, registered_src);
	if (IS_ERR(file))
		return PTR_ERR(file);
	ret = io_clone_buffers(ctx, file->private_data);
	if (!registered_src)
		fput(file);
	return ret;
}
