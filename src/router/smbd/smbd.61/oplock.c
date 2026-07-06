// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@kernel.org>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/moduleparam.h>
#include <linux/err.h>

#include "glob.h"
#include "oplock.h"

#include "smb_common.h"
#ifdef CONFIG_SMB_INSECURE_SERVER
#include "smb1pdu.h"
#endif
#include "smbstatus.h"
#include "connection.h"
#include "mgmt/user_session.h"
#include "mgmt/share_config.h"
#include "mgmt/tree_connect.h"

static LIST_HEAD(lease_table_list);
static DEFINE_RWLOCK(lease_list_lock);

#define SMB2_LEASE_STATE_MASK_LE	(SMB2_LEASE_READ_CACHING_LE | \
					 SMB2_LEASE_HANDLE_CACHING_LE | \
					 SMB2_LEASE_WRITE_CACHING_LE)

static bool lease_state_valid(__le32 state)
{
	return !(state & ~SMB2_LEASE_STATE_MASK_LE);
}

static bool lease_v2_flags_valid(__le32 flags)
{
	return !(flags & ~SMB2_LEASE_FLAG_PARENT_LEASE_KEY_SET_LE);
}

static bool lease_has_parent_key(struct lease *lease)
{
	return lease->flags & SMB2_LEASE_FLAG_PARENT_LEASE_KEY_SET_LE;
}

static bool lease_break_in_progress(struct lease *lease)
{
	struct oplock_info *opinfo;
	bool ret = false;

	spin_lock(&lease->lock);
	list_for_each_entry(opinfo, &lease->open_list, lease_entry) {
		if (opinfo->op_state == OPLOCK_ACK_WAIT) {
			ret = true;
			break;
		}
	}
	spin_unlock(&lease->lock);

	return ret;
}

/**
 * alloc_opinfo() - allocate a new opinfo object for oplock info
 * @work:	smb work
 * @id:		fid of open file
 * @Tid:	tree id of connection
 *
 * Return:      allocated opinfo object on success, otherwise NULL
 */
static struct oplock_info *alloc_opinfo(struct ksmbd_work *work,
					u64 id, __u16 Tid)
{
	struct ksmbd_session *sess = work->sess;
	struct oplock_info *opinfo;

	opinfo = kzalloc(sizeof(struct oplock_info), KSMBD_DEFAULT_GFP);
	if (!opinfo)
		return NULL;

	opinfo->sess = sess;
	opinfo->conn = ksmbd_conn_get(work->conn);
	opinfo->level = SMB2_OPLOCK_LEVEL_NONE;
	opinfo->op_state = OPLOCK_STATE_NONE;
	opinfo->pending_break = 0;
	opinfo->fid = id;
	opinfo->Tid = Tid;
#ifdef CONFIG_SMB_INSECURE_SERVER
	opinfo->is_smb2 = IS_SMB2(opinfo->conn);
#endif
	INIT_LIST_HEAD(&opinfo->op_entry);
	INIT_LIST_HEAD(&opinfo->lease_entry);
	init_waitqueue_head(&opinfo->oplock_q);
	init_waitqueue_head(&opinfo->oplock_brk);
	atomic_set(&opinfo->refcount, 1);
	atomic_set(&opinfo->breaking_cnt, 0);

	return opinfo;
}

static void lease_get(struct lease *lease)
{
	atomic_inc(&lease->refcount);
}

static void lease_put(struct lease *lease)
{
	if (lease && atomic_dec_and_test(&lease->refcount))
		kfree(lease);
}

static void lease_add_table(struct lease *lease, struct lease_table *lb)
{
	lease_get(lease);
	lease->l_lb = lb;
	spin_lock(&lb->lb_lock);
	list_add_rcu(&lease->l_entry, &lb->lease_list);
	spin_unlock(&lb->lb_lock);
}

static void lease_del_table(struct lease *lease)
{
	struct lease_table *lb = lease->l_lb;

	if (!lb)
		return;

	spin_lock(&lb->lb_lock);
	if (list_empty(&lease->l_entry)) {
		spin_unlock(&lb->lb_lock);
		return;
	}

	list_del_init(&lease->l_entry);
	lease->l_lb = NULL;
	spin_unlock(&lb->lb_lock);

	lease_put(lease);
}

static struct lease_table *alloc_lease_table(struct oplock_info *opinfo)
{
	struct lease_table *lb;

	lb = kmalloc(sizeof(struct lease_table), KSMBD_DEFAULT_GFP);
	if (!lb)
		return NULL;

	memcpy(lb->client_guid, opinfo->conn->ClientGUID,
	       SMB2_CLIENT_GUID_SIZE);
	lb->conn = ksmbd_conn_get(opinfo->conn);
	INIT_LIST_HEAD(&lb->lease_list);
	spin_lock_init(&lb->lb_lock);
	return lb;
}

static void free_lease_table(struct lease_table *lb)
{
	if (!lb)
		return;

	ksmbd_conn_put(lb->conn);
	kfree(lb);
}

static struct lease *alloc_lease(struct lease_ctx_info *lctx,
				struct ksmbd_inode *ci)
{
	struct lease *lease;

	lease = kmalloc(sizeof(struct lease), KSMBD_DEFAULT_GFP);
	if (!lease)
		return NULL;

	memcpy(lease->lease_key, lctx->lease_key, SMB2_LEASE_KEY_SIZE);
	lease->state = lctx->req_state;
	lease->new_state = 0;
	lease->flags = lctx->flags;
	lease->duration = lctx->duration;
	lease->is_dir = lctx->is_dir;
	memcpy(lease->parent_lease_key, lctx->parent_lease_key, SMB2_LEASE_KEY_SIZE);
	lease->version = lctx->version;
	lease->epoch = lctx->version == 2 ? le16_to_cpu(lctx->epoch) + 1 : 0;
	lease->ci = ci;
	lease->reuse_epoch = false;
	lease->l_lb = NULL;
	INIT_LIST_HEAD(&lease->l_entry);
	INIT_LIST_HEAD(&lease->open_list);
	spin_lock_init(&lease->lock);
	atomic_set(&lease->refcount, 1);

	return lease;
}

static void lease_add_open(struct lease *lease, struct oplock_info *opinfo)
{
	spin_lock(&lease->lock);
	list_add(&opinfo->lease_entry, &lease->open_list);
	spin_unlock(&lease->lock);
}

static void lease_del_open(struct oplock_info *opinfo)
{
	struct lease *lease = opinfo->o_lease;
	bool remove_table = false;

	if (!lease)
		return;

	spin_lock(&lease->lock);
	if (!list_empty(&opinfo->lease_entry)) {
		list_del_init(&opinfo->lease_entry);
		remove_table = list_empty(&lease->open_list);
	}
	spin_unlock(&lease->lock);

	if (remove_table) {
		write_lock(&lease_list_lock);
		lease_del_table(lease);
		write_unlock(&lease_list_lock);
	}
}

static void free_lease(struct oplock_info *opinfo)
{
	lease_put(opinfo->o_lease);
}

static void __free_opinfo(struct oplock_info *opinfo)
{
	if (opinfo->is_lease)
		free_lease(opinfo);
	ksmbd_conn_put(opinfo->conn);
	kfree(opinfo);
}

static void free_opinfo_rcu(struct rcu_head *rcu)
{
	struct oplock_info *opinfo = container_of(rcu, struct oplock_info, rcu);

	__free_opinfo(opinfo);
}

static void free_opinfo(struct oplock_info *opinfo)
{
	call_rcu(&opinfo->rcu, free_opinfo_rcu);
}

void lease_update_oplock_levels(struct lease *lease)
{
	struct oplock_info *opinfo;
	__u8 level;

	if (!lease)
		return;

	level = smb2_map_lease_to_oplock(lease->state);
	spin_lock(&lease->lock);
	list_for_each_entry(opinfo, &lease->open_list, lease_entry)
		opinfo->level = level;
	spin_unlock(&lease->lock);
}

struct oplock_info *opinfo_get(struct ksmbd_file *fp)
{
	struct oplock_info *opinfo;

	rcu_read_lock();
	opinfo = rcu_dereference(fp->f_opinfo);
	if (opinfo && !atomic_inc_not_zero(&opinfo->refcount))
		opinfo = NULL;
	rcu_read_unlock();

	return opinfo;
}

static struct oplock_info *opinfo_get_list(struct ksmbd_inode *ci)
{
	struct oplock_info *opinfo;

	down_read(&ci->m_lock);
	opinfo = list_first_entry_or_null(&ci->m_op_list, struct oplock_info,
					  op_entry);
	if (opinfo) {
		if (opinfo->conn == NULL ||
		    !atomic_inc_not_zero(&opinfo->refcount))
			opinfo = NULL;
		else {
			if (ksmbd_conn_releasing(opinfo->conn)) {
				atomic_dec(&opinfo->refcount);
				opinfo = NULL;
			}
		}
	}
	up_read(&ci->m_lock);

	return opinfo;
}

void opinfo_put(struct oplock_info *opinfo)
{
	if (!opinfo)
		return;

	if (!atomic_dec_and_test(&opinfo->refcount))
		return;

	free_opinfo(opinfo);
}

static void opinfo_add(struct oplock_info *opinfo, struct ksmbd_file *fp)
{
	struct ksmbd_inode *ci = fp->f_ci;

	down_write(&ci->m_lock);
	list_add(&opinfo->op_entry, &ci->m_op_list);
	up_write(&ci->m_lock);
}

static void opinfo_del(struct oplock_info *opinfo)
{
	struct ksmbd_inode *ci = opinfo->o_fp->f_ci;

	if (opinfo->is_lease)
		lease_del_open(opinfo);

	down_write(&ci->m_lock);
	list_del(&opinfo->op_entry);
	up_write(&ci->m_lock);
}

static unsigned long opinfo_count(struct ksmbd_file *fp)
{
	if (ksmbd_stream_fd(fp))
		return atomic_read(&fp->f_ci->sop_count);
	else
		return atomic_read(&fp->f_ci->op_count);
}

static void opinfo_count_inc(struct ksmbd_file *fp)
{
	if (ksmbd_stream_fd(fp))
		return atomic_inc(&fp->f_ci->sop_count);
	else
		return atomic_inc(&fp->f_ci->op_count);
}

static void opinfo_count_dec(struct ksmbd_file *fp)
{
	if (ksmbd_stream_fd(fp))
		return atomic_dec(&fp->f_ci->sop_count);
	else
		return atomic_dec(&fp->f_ci->op_count);
}

/**
 * opinfo_write_to_read() - convert a write oplock to read oplock
 * @opinfo:		current oplock info
 *
 * Return:      0 on success, otherwise -EINVAL
 */
int opinfo_write_to_read(struct oplock_info *opinfo)
{
	struct lease *lease = opinfo->o_lease;

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (opinfo->is_smb2) {
		if (!(opinfo->level == SMB2_OPLOCK_LEVEL_BATCH ||
		      opinfo->level == SMB2_OPLOCK_LEVEL_EXCLUSIVE)) {
			pr_err("bad oplock(0x%x)\n", opinfo->level);
			if (opinfo->is_lease)
				pr_err("lease state(0x%x)\n", lease->state);
			return -EINVAL;
		}
		opinfo->level = SMB2_OPLOCK_LEVEL_II;

		if (opinfo->is_lease)
			lease->state = lease->new_state;
	} else {
		if (!(opinfo->level == OPLOCK_EXCLUSIVE ||
		      opinfo->level == OPLOCK_BATCH)) {
			pr_err("bad oplock(0x%x)\n", opinfo->level);
			return -EINVAL;
		}
		opinfo->level = OPLOCK_READ;
	}
#else
	if (!(opinfo->level == SMB2_OPLOCK_LEVEL_BATCH ||
	      opinfo->level == SMB2_OPLOCK_LEVEL_EXCLUSIVE)) {
		pr_err("bad oplock(0x%x)\n", opinfo->level);
		if (opinfo->is_lease)
			pr_err("lease state(0x%x)\n", lease->state);
		return -EINVAL;
	}
	opinfo->level = SMB2_OPLOCK_LEVEL_II;

	if (opinfo->is_lease) {
		lease->state = lease->new_state;
		lease_update_oplock_levels(lease);
	}
#endif
	return 0;
}

/**
 * opinfo_read_handle_to_read() - convert a read/handle oplock to read oplock
 * @opinfo:		current oplock info
 *
 * Return:      0 on success, otherwise -EINVAL
 */
int opinfo_read_handle_to_read(struct oplock_info *opinfo)
{
	struct lease *lease = opinfo->o_lease;

	lease->state = lease->new_state;
	lease_update_oplock_levels(lease);
	return 0;
}

/**
 * opinfo_write_to_none() - convert a write oplock to none
 * @opinfo:	current oplock info
 *
 * Return:      0 on success, otherwise -EINVAL
 */
int opinfo_write_to_none(struct oplock_info *opinfo)
{
	struct lease *lease = opinfo->o_lease;

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (opinfo->is_smb2) {
		if (!(opinfo->level == SMB2_OPLOCK_LEVEL_BATCH ||
		      opinfo->level == SMB2_OPLOCK_LEVEL_EXCLUSIVE)) {
			pr_err("bad oplock(0x%x)\n", opinfo->level);
			if (opinfo->is_lease)
				pr_err("lease state(0x%x)\n", lease->state);
			return -EINVAL;
		}
		opinfo->level = SMB2_OPLOCK_LEVEL_NONE;
		if (opinfo->is_lease)
			lease->state = lease->new_state;
	} else {
		if (!(opinfo->level == OPLOCK_EXCLUSIVE ||
		      opinfo->level == OPLOCK_BATCH)) {
			pr_err("bad oplock(0x%x)\n", opinfo->level);
			return -EINVAL;
		}
		opinfo->level = OPLOCK_NONE;
	}
#else
	if (!(opinfo->level == SMB2_OPLOCK_LEVEL_BATCH ||
	      opinfo->level == SMB2_OPLOCK_LEVEL_EXCLUSIVE)) {
		pr_err("bad oplock(0x%x)\n", opinfo->level);
		if (opinfo->is_lease)
			pr_err("lease state(0x%x)\n", lease->state);
		return -EINVAL;
	}
	opinfo->level = SMB2_OPLOCK_LEVEL_NONE;
	if (opinfo->is_lease) {
		lease->state = lease->new_state;
		lease_update_oplock_levels(lease);
	}
#endif
	return 0;
}

/**
 * opinfo_read_to_none() - convert a write read to none
 * @opinfo:	current oplock info
 *
 * Return:      0 on success, otherwise -EINVAL
 */
int opinfo_read_to_none(struct oplock_info *opinfo)
{
	struct lease *lease = opinfo->o_lease;

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (opinfo->is_smb2) {
		if (opinfo->level != SMB2_OPLOCK_LEVEL_II) {
			pr_err("bad oplock(0x%x)\n", opinfo->level);
			if (opinfo->is_lease)
				pr_err("lease state(0x%x)\n", lease->state);
			return -EINVAL;
		}
		opinfo->level = SMB2_OPLOCK_LEVEL_NONE;
		if (opinfo->is_lease)
			lease->state = lease->new_state;
	} else {
		if (opinfo->level != OPLOCK_READ) {
			pr_err("bad oplock(0x%x)\n", opinfo->level);
			return -EINVAL;
		}
		opinfo->level = OPLOCK_NONE;
	}
#else
	if (opinfo->level != SMB2_OPLOCK_LEVEL_II) {
		pr_err("bad oplock(0x%x)\n", opinfo->level);
		if (opinfo->is_lease)
			pr_err("lease state(0x%x)\n", lease->state);
		return -EINVAL;
	}
	opinfo->level = SMB2_OPLOCK_LEVEL_NONE;
	if (opinfo->is_lease) {
		lease->state = lease->new_state;
		lease_update_oplock_levels(lease);
	}
#endif
	return 0;
}

/**
 * lease_read_to_write() - upgrade lease state from read to write
 * @opinfo:	current lease info
 *
 * Return:      0 on success, otherwise -EINVAL
 */
int lease_read_to_write(struct oplock_info *opinfo)
{
	struct lease *lease = opinfo->o_lease;

	if (!(lease->state & SMB2_LEASE_READ_CACHING_LE)) {
		ksmbd_debug(OPLOCK, "bad lease state(0x%x)\n", lease->state);
		return -EINVAL;
	}

	lease->new_state = SMB2_LEASE_NONE_LE;
	lease->state |= SMB2_LEASE_WRITE_CACHING_LE;
	lease_update_oplock_levels(lease);
	return 0;
}

/**
 * lease_none_upgrade() - upgrade lease state from none
 * @opinfo:	current lease info
 * @new_state:	new lease state
 *
 * Return:	0 on success, otherwise -EINVAL
 */
static int lease_none_upgrade(struct oplock_info *opinfo, __le32 new_state)
{
	struct lease *lease = opinfo->o_lease;

	if (!(lease->state == SMB2_LEASE_NONE_LE)) {
		ksmbd_debug(OPLOCK, "bad lease state(0x%x)\n", lease->state);
		return -EINVAL;
	}

	lease->new_state = SMB2_LEASE_NONE_LE;
	lease->state = new_state;
	lease_update_oplock_levels(lease);

	return 0;
}

/**
 * close_id_del_oplock() - release oplock object at file close time
 * @fp:		ksmbd file pointer
 */
void close_id_del_oplock(struct ksmbd_file *fp)
{
	struct oplock_info *opinfo;

	if (fp->reserve_lease_break)
		smb_lazy_parent_lease_break_close(fp);

	opinfo = opinfo_get(fp);
	if (!opinfo)
		return;

	opinfo_del(opinfo);

	rcu_assign_pointer(fp->f_opinfo, NULL);
	if (opinfo->op_state == OPLOCK_ACK_WAIT) {
		opinfo->op_state = OPLOCK_CLOSING;
		wake_up_interruptible_all(&opinfo->oplock_q);
		if (opinfo->is_lease) {
			atomic_set(&opinfo->breaking_cnt, 0);
			wake_up_interruptible_all(&opinfo->oplock_brk);
		}
	}

	opinfo_count_dec(fp);
	atomic_dec(&opinfo->refcount);
	opinfo_put(opinfo);
}

/**
 * grant_write_oplock() - grant exclusive/batch oplock or write lease
 * @opinfo_new:	new oplock info object
 * @req_oplock: request oplock
 * @lctx:	lease context information
 *
 * Return:      0
 */
static void grant_write_oplock(struct oplock_info *opinfo_new, int req_oplock,
			       struct lease_ctx_info *lctx)
{
	struct lease *lease = opinfo_new->o_lease;

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (opinfo_new->is_smb2) {
		if (req_oplock == SMB2_OPLOCK_LEVEL_BATCH)
			opinfo_new->level = SMB2_OPLOCK_LEVEL_BATCH;
		else
			opinfo_new->level = SMB2_OPLOCK_LEVEL_EXCLUSIVE;
	} else {
		if (req_oplock == REQ_BATCHOPLOCK)
			opinfo_new->level = OPLOCK_BATCH;
		else
			opinfo_new->level = OPLOCK_EXCLUSIVE;
	}
#else
	if (req_oplock == SMB2_OPLOCK_LEVEL_BATCH)
		opinfo_new->level = SMB2_OPLOCK_LEVEL_BATCH;
	else
		opinfo_new->level = SMB2_OPLOCK_LEVEL_EXCLUSIVE;
#endif

	if (lctx) {
		lease->state = lctx->req_state;
		memcpy(lease->lease_key, lctx->lease_key, SMB2_LEASE_KEY_SIZE);
	}
}

/**
 * grant_read_oplock() - grant level2 oplock or read lease
 * @opinfo_new:	new oplock info object
 * @lctx:	lease context information
 *
 * Return:      0
 */
static void grant_read_oplock(struct oplock_info *opinfo_new,
			      struct lease_ctx_info *lctx)
{
	struct lease *lease = opinfo_new->o_lease;

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (opinfo_new->is_smb2)
		opinfo_new->level = SMB2_OPLOCK_LEVEL_II;
	else
		opinfo_new->level = OPLOCK_READ;
#else
	opinfo_new->level = SMB2_OPLOCK_LEVEL_II;
#endif

	if (lctx) {
		lease->state = SMB2_LEASE_READ_CACHING_LE;
		if (lctx->req_state & SMB2_LEASE_HANDLE_CACHING_LE)
			lease->state |= SMB2_LEASE_HANDLE_CACHING_LE;
		memcpy(lease->lease_key, lctx->lease_key, SMB2_LEASE_KEY_SIZE);
	}
}

/**
 * grant_none_oplock() - grant none oplock or none lease
 * @opinfo_new:	new oplock info object
 * @lctx:	lease context information
 *
 * Return:      0
 */
static void grant_none_oplock(struct oplock_info *opinfo_new,
			      struct lease_ctx_info *lctx)
{
	struct lease *lease = opinfo_new->o_lease;

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (opinfo_new->is_smb2)
		opinfo_new->level = SMB2_OPLOCK_LEVEL_NONE;
	else
		opinfo_new->level = OPLOCK_NONE;
#else
	opinfo_new->level = SMB2_OPLOCK_LEVEL_NONE;
#endif

	if (lctx) {
		lease->state = 0;
		memcpy(lease->lease_key, lctx->lease_key, SMB2_LEASE_KEY_SIZE);
	}
}

static inline int compare_guid_key(struct oplock_info *opinfo,
				   const char *guid1, const char *key1)
{
	const char *guid2, *key2;

	guid2 = opinfo->conn->ClientGUID;
	key2 = opinfo->o_lease->lease_key;
	if (!memcmp(guid1, guid2, SMB2_CLIENT_GUID_SIZE) &&
	    !memcmp(key1, key2, SMB2_LEASE_KEY_SIZE))
		return 1;

	return 0;
}

static bool lease_break_needed(struct oplock_info *opinfo, int req_op_level,
			       bool open_trunc)
{
	struct lease *lease = opinfo->o_lease;

	if (open_trunc)
		return lease->state != SMB2_LEASE_NONE_LE;

	return opinfo->level > req_op_level;
}

/**
 * same_client_has_lease() - check whether current lease request is
 *		from lease owner of file
 * @ci:		master file pointer
 * @client_guid:	Client GUID
 * @lctx:		lease context information
 *
 * Return:      oplock(lease) object on success, otherwise NULL
 */
static struct oplock_info *same_client_has_lease(struct ksmbd_inode *ci,
						 const char *client_guid,
						 struct lease_ctx_info *lctx)
{
	int ret;
	struct lease *lease;
	struct oplock_info *opinfo;
	struct oplock_info *m_opinfo = NULL;

	if (!lctx)
		return NULL;

	/*
	 * Compare lease key and client_guid to know request from same owner
	 * of same client
	 */
	down_read(&ci->m_lock);
	list_for_each_entry(opinfo, &ci->m_op_list, op_entry) {
		if (!opinfo->is_lease || !opinfo->conn)
			continue;
		lease = opinfo->o_lease;

		ret = compare_guid_key(opinfo, client_guid, lctx->lease_key);
		if (ret) {
			if (!atomic_inc_not_zero(&opinfo->refcount))
				continue;
			if (m_opinfo)
				opinfo_put(m_opinfo);
			m_opinfo = opinfo;

			/* skip upgrading lease about breaking lease */
			if (atomic_read(&opinfo->breaking_cnt))
				continue;

			/* upgrading lease */
			if ((atomic_read(&ci->op_count) +
			     atomic_read(&ci->sop_count)) == 1) {
				if (lease->state != SMB2_LEASE_NONE_LE &&
				    lease->state == (lctx->req_state & lease->state)) {
					lease->epoch++;
					lease->state |= lctx->req_state;
					if (lctx->req_state &
						SMB2_LEASE_WRITE_CACHING_LE)
						lease_read_to_write(opinfo);

				}
			} else if ((atomic_read(&ci->op_count) +
				    atomic_read(&ci->sop_count)) > 1) {
				if (lctx->req_state ==
				    (SMB2_LEASE_READ_CACHING_LE |
				     SMB2_LEASE_HANDLE_CACHING_LE)) {
					if (lease->state != lctx->req_state) {
						lease->epoch++;
						lease->state = lctx->req_state;
						lease_update_oplock_levels(lease);
					}
				}
			}

			if (lctx->req_state && lease->state ==
			    SMB2_LEASE_NONE_LE) {
				lease->epoch++;
				lease_none_upgrade(opinfo, lctx->req_state);
			}
		}
	}
	up_read(&ci->m_lock);

	return m_opinfo;
}

static void wait_for_break_ack(struct oplock_info *opinfo)
{
	int rc = 0;

	rc = wait_event_interruptible_timeout(opinfo->oplock_q,
					      opinfo->op_state == OPLOCK_STATE_NONE ||
					      opinfo->op_state == OPLOCK_CLOSING,
					      OPLOCK_WAIT_TIME);

	/* is this a timeout ? */
	if (!rc) {
		if (opinfo->is_lease) {
			opinfo->o_lease->state = SMB2_LEASE_NONE_LE;
			lease_update_oplock_levels(opinfo->o_lease);
		}
		opinfo->level = SMB2_OPLOCK_LEVEL_NONE;
		opinfo->op_state = OPLOCK_STATE_NONE;
	}
}

static void wake_up_oplock_break(struct oplock_info *opinfo)
{
	clear_bit_unlock(0, &opinfo->pending_break);
	/* memory barrier is needed for wake_up_bit() */
	smp_mb__after_atomic();
	wake_up_bit(&opinfo->pending_break, 0);
}

static int oplock_break_pending(struct oplock_info *opinfo, int req_op_level)
{
	while (test_and_set_bit(0, &opinfo->pending_break)) {
		if (opinfo->is_lease)
			opinfo->o_lease->reuse_epoch = true;

		wait_on_bit(&opinfo->pending_break, 0, TASK_UNINTERRUPTIBLE);

		/* Not immediately break to none. */
		opinfo->open_trunc = 0;

		if (opinfo->op_state == OPLOCK_CLOSING)
			return -ENOENT;
		else if (opinfo->level <= req_op_level) {
			if (opinfo->is_lease == false)
				return 1;

			if (opinfo->o_lease->state !=
			    (SMB2_LEASE_HANDLE_CACHING_LE |
			     SMB2_LEASE_READ_CACHING_LE))
				return 1;
		}
	}

	if (opinfo->level <= req_op_level) {
		if (opinfo->is_lease == false) {
			wake_up_oplock_break(opinfo);
			return 1;
		}
		if (opinfo->o_lease->state !=
		    (SMB2_LEASE_HANDLE_CACHING_LE |
		     SMB2_LEASE_READ_CACHING_LE)) {
			wake_up_oplock_break(opinfo);
			return 1;
		}
	}
	return 0;
}

#ifdef CONFIG_SMB_INSECURE_SERVER
/**
 * smb1_oplock_break_noti() - send smb1 oplock break cmd from conn
 * to client
 * @work:     smb work object
 *
 * There are two ways this function can be called. 1- while file open we break
 * from exclusive/batch lock to levelII oplock and 2- while file write/truncate
 * we break from levelII oplock no oplock.
 * work->request_buf contains oplock_info.
 */
static void __smb1_oplock_break_noti(struct work_struct *wk)
{
	struct ksmbd_work *work = container_of(wk, struct ksmbd_work, work);
	struct ksmbd_conn *conn = work->conn;
	struct smb_hdr *rsp_hdr;
	struct smb_com_lock_req *req;
	struct oplock_info *opinfo = work->request_buf;

	if (allocate_interim_rsp_buf(work)) {
		pr_err("smb_allocate_rsp_buf failed! ");
		goto out;
	}

	/* Init response header */
	rsp_hdr = work->response_buf;
	/* wct is 8 for locking andx(18) */
	memset(rsp_hdr, 0, sizeof(struct smb_hdr) + 18);
	rsp_hdr->smb_buf_length =
		cpu_to_be32(conn->vals->header_size - 4 + 18);
	rsp_hdr->Protocol[0] = 0xFF;
	rsp_hdr->Protocol[1] = 'S';
	rsp_hdr->Protocol[2] = 'M';
	rsp_hdr->Protocol[3] = 'B';

	rsp_hdr->Command = SMB_COM_LOCKING_ANDX;
	/* we know unicode, long file name and use nt error codes */
	rsp_hdr->Flags2 = SMBFLG2_UNICODE | SMBFLG2_KNOWS_LONG_NAMES |
		SMBFLG2_ERR_STATUS;
	rsp_hdr->Uid = cpu_to_le16(work->sess->id);
	rsp_hdr->Pid = cpu_to_le16(0xFFFF);
	rsp_hdr->Mid = cpu_to_le16(0xFFFF);
	rsp_hdr->Tid = cpu_to_le16(opinfo->Tid);
	rsp_hdr->WordCount = 8;

	/* Init locking request */
	req = work->response_buf;

	req->AndXCommand = 0xFF;
	req->AndXReserved = 0;
	req->AndXOffset = 0;
	req->Fid = opinfo->fid;
	req->LockType = LOCKING_ANDX_OPLOCK_RELEASE;
	if (!opinfo->open_trunc &&
	    (opinfo->level == OPLOCK_BATCH ||
	     opinfo->level == OPLOCK_EXCLUSIVE))
		req->OplockLevel = 1;
	else
		req->OplockLevel = 0;
	req->Timeout = 0;
	req->NumberOfUnlocks = 0;
	req->ByteCount = 0;
	ksmbd_debug(OPLOCK, "sending oplock break for fid %d lock level = %d\n",
		    req->Fid, req->OplockLevel);

	ksmbd_conn_write(work);
out:
	ksmbd_free_work_struct(work);
	ksmbd_conn_r_count_dec(conn);
}

/**
 * smb1_oplock_break() - send smb1 exclusive/batch to level2 oplock
 *		break command from server to client
 * @opinfo:		oplock info object
 * @ack_required	if requiring ack
 *
 * Return:      0 on success, otherwise error
 */
static int smb1_oplock_break_noti(struct oplock_info *opinfo)
{
	struct ksmbd_conn *conn = opinfo->conn;
	struct ksmbd_work *work = ksmbd_alloc_work_struct();

	if (!work)
		return -ENOMEM;

	work->request_buf = (char *)opinfo;
	work->conn = conn;

	ksmbd_conn_r_count_inc(conn);
	if (opinfo->op_state == OPLOCK_ACK_WAIT) {
		INIT_WORK(&work->work, __smb1_oplock_break_noti);
		ksmbd_queue_work(work);

		wait_for_break_ack(opinfo);
	} else {
		__smb1_oplock_break_noti(&work->work);
		if (opinfo->level == OPLOCK_READ)
			opinfo->level = OPLOCK_NONE;
	}
	return 0;
}
#endif

/**
 * __smb2_oplock_break_noti() - send smb2 oplock break cmd from conn
 * to client
 * @wk:     smb work object
 *
 * There are two ways this function can be called. 1- while file open we break
 * from exclusive/batch lock to levelII oplock and 2- while file write/truncate
 * we break from levelII oplock no oplock.
 * work->request_buf contains oplock_info.
 */
static void __smb2_oplock_break_noti(struct work_struct *wk)
{
	struct smb2_oplock_break *rsp = NULL;
	struct ksmbd_work *work = container_of(wk, struct ksmbd_work, work);
	struct ksmbd_conn *conn = work->conn;
	struct oplock_break_info *br_info = work->request_buf;
	struct smb2_hdr *rsp_hdr;
	struct ksmbd_file *fp;

	fp = ksmbd_lookup_global_fd(br_info->fid);
	if (!fp)
		goto out;

	if (allocate_interim_rsp_buf(work)) {
		pr_err("smb2_allocate_rsp_buf failed! ");
		ksmbd_fd_put(work, fp);
		goto out;
	}

	rsp_hdr = smb2_get_msg(work->response_buf);
	memset(rsp_hdr, 0, sizeof(struct smb2_hdr) + 2);
	rsp_hdr->ProtocolId = SMB2_PROTO_NUMBER;
	rsp_hdr->StructureSize = SMB2_HEADER_STRUCTURE_SIZE;
	rsp_hdr->CreditRequest = cpu_to_le16(0);
	rsp_hdr->Command = SMB2_OPLOCK_BREAK;
	rsp_hdr->Flags = (SMB2_FLAGS_SERVER_TO_REDIR);
	rsp_hdr->NextCommand = 0;
	rsp_hdr->MessageId = cpu_to_le64(-1);
	rsp_hdr->Id.SyncId.ProcessId = 0;
	rsp_hdr->Id.SyncId.TreeId = 0;
	rsp_hdr->SessionId = 0;
	memset(rsp_hdr->Signature, 0, 16);

	rsp = smb2_get_msg(work->response_buf);

	rsp->StructureSize = cpu_to_le16(24);
	if (!br_info->open_trunc &&
	    (br_info->level == SMB2_OPLOCK_LEVEL_BATCH ||
	     br_info->level == SMB2_OPLOCK_LEVEL_EXCLUSIVE))
		rsp->OplockLevel = SMB2_OPLOCK_LEVEL_II;
	else
		rsp->OplockLevel = SMB2_OPLOCK_LEVEL_NONE;
	rsp->Reserved = 0;
	rsp->Reserved2 = 0;
	rsp->PersistentFid = fp->persistent_id;
	rsp->VolatileFid = fp->volatile_id;

	ksmbd_fd_put(work, fp);
	if (ksmbd_iov_pin_rsp(work, (void *)rsp,
			      sizeof(struct smb2_oplock_break)))
		goto out;

	ksmbd_debug(OPLOCK,
		    "sending oplock break v_id %llu p_id = %llu lock level = %d\n",
		    rsp->VolatileFid, rsp->PersistentFid, rsp->OplockLevel);

	ksmbd_conn_write(work);

out:
	ksmbd_free_work_struct(work);
	ksmbd_conn_r_count_dec(conn);
}

/**
 * smb2_oplock_break_noti() - send smb2 exclusive/batch to level2 oplock
 *		break command from server to client
 * @opinfo:		oplock info object
 *
 * Return:      0 on success, otherwise error
 */
static int smb2_oplock_break_noti(struct oplock_info *opinfo)
{
	struct ksmbd_conn *conn;
	struct oplock_break_info *br_info;
	int ret = 0;
	struct ksmbd_work *work;

	conn = READ_ONCE(opinfo->conn);
	if (!conn)
		return 0;

	work = ksmbd_alloc_work_struct();
	if (!work)
		return -ENOMEM;

	br_info = kmalloc(sizeof(struct oplock_break_info), KSMBD_DEFAULT_GFP);
	if (!br_info) {
		ksmbd_free_work_struct(work);
		return -ENOMEM;
	}

	br_info->level = opinfo->level;
	br_info->fid = opinfo->fid;
	br_info->open_trunc = opinfo->open_trunc;

	work->request_buf = (char *)br_info;
	work->conn = conn;
	work->sess = opinfo->sess;

	ksmbd_conn_r_count_inc(conn);
	if (opinfo->op_state == OPLOCK_ACK_WAIT) {
		INIT_WORK(&work->work, __smb2_oplock_break_noti);
		ksmbd_queue_work(work);

		wait_for_break_ack(opinfo);
	} else {
		__smb2_oplock_break_noti(&work->work);
		if (opinfo->level == SMB2_OPLOCK_LEVEL_II)
			opinfo->level = SMB2_OPLOCK_LEVEL_NONE;
	}
	return ret;
}

/**
 * __smb2_lease_break_noti() - send lease break command from server
 * to client
 * @wk:     smb work object
 */
static void __smb2_lease_break_noti(struct work_struct *wk)
{
	struct smb2_lease_break *rsp = NULL;
	struct ksmbd_work *work = container_of(wk, struct ksmbd_work, work);
	struct ksmbd_conn *conn = work->conn;
	struct lease_break_info *br_info = work->request_buf;
	struct smb2_hdr *rsp_hdr;

	if (allocate_interim_rsp_buf(work)) {
		ksmbd_debug(OPLOCK, "smb2_allocate_rsp_buf failed! ");
		goto out;
	}

	rsp_hdr = smb2_get_msg(work->response_buf);
	memset(rsp_hdr, 0, sizeof(struct smb2_hdr) + 2);
	rsp_hdr->ProtocolId = SMB2_PROTO_NUMBER;
	rsp_hdr->StructureSize = SMB2_HEADER_STRUCTURE_SIZE;
	rsp_hdr->CreditRequest = cpu_to_le16(0);
	rsp_hdr->Command = SMB2_OPLOCK_BREAK;
	rsp_hdr->Flags = (SMB2_FLAGS_SERVER_TO_REDIR);
	rsp_hdr->NextCommand = 0;
	rsp_hdr->MessageId = cpu_to_le64(-1);
	rsp_hdr->Id.SyncId.ProcessId = 0;
	rsp_hdr->Id.SyncId.TreeId = 0;
	rsp_hdr->SessionId = 0;
	memset(rsp_hdr->Signature, 0, 16);

	rsp = smb2_get_msg(work->response_buf);
	rsp->StructureSize = cpu_to_le16(44);
	rsp->Epoch = br_info->epoch;
	rsp->Flags = 0;

	if (br_info->curr_state & (SMB2_LEASE_WRITE_CACHING_LE |
			SMB2_LEASE_HANDLE_CACHING_LE))
		rsp->Flags = SMB2_NOTIFY_BREAK_LEASE_FLAG_ACK_REQUIRED;

	memcpy(rsp->LeaseKey, br_info->lease_key, SMB2_LEASE_KEY_SIZE);
	rsp->CurrentLeaseState = br_info->curr_state;
	rsp->NewLeaseState = br_info->new_state;
	rsp->BreakReason = 0;
	rsp->AccessMaskHint = 0;
	rsp->ShareMaskHint = 0;

	if (ksmbd_iov_pin_rsp(work, (void *)rsp,
			      sizeof(struct smb2_lease_break)))
		goto out;

	ksmbd_conn_write(work);

out:
	ksmbd_free_work_struct(work);
	ksmbd_conn_r_count_dec(conn);
}

/**
 * smb2_lease_break_noti() - break lease when a new client request
 *			write lease
 * @opinfo:		contains lease state information
 *
 * Return:	0 on success, otherwise error
 */
static int smb2_lease_break_noti(struct oplock_info *opinfo, bool wait_ack,
				 bool inc_epoch)
{
	struct ksmbd_conn *conn;
	struct ksmbd_work *work;
	struct lease_break_info *br_info;
	struct lease *lease = opinfo->o_lease;

	conn = READ_ONCE(opinfo->conn);
	if (lease->version == 2 && lease->l_lb && lease->l_lb->conn &&
	    !ksmbd_conn_releasing(lease->l_lb->conn))
		conn = lease->l_lb->conn;
	if (!conn)
		return 0;

	work = ksmbd_alloc_work_struct();
	if (!work)
		return -ENOMEM;

	br_info = kmalloc(sizeof(struct lease_break_info), KSMBD_DEFAULT_GFP);
	if (!br_info) {
		ksmbd_free_work_struct(work);
		return -ENOMEM;
	}

	br_info->curr_state = lease->state;
	br_info->new_state = lease->new_state;
	if (lease->version == 2) {
		if (inc_epoch)
			lease->epoch++;
		br_info->epoch = cpu_to_le16(lease->epoch);
	} else {
		br_info->epoch = 0;
	}
	memcpy(br_info->lease_key, lease->lease_key, SMB2_LEASE_KEY_SIZE);

	work->request_buf = (char *)br_info;
	work->conn = conn;
	work->sess = opinfo->sess;

	ksmbd_conn_r_count_inc(conn);
	if (opinfo->op_state == OPLOCK_ACK_WAIT) {
		INIT_WORK(&work->work, __smb2_lease_break_noti);
		ksmbd_queue_work(work);
		if (wait_ack)
			wait_for_break_ack(opinfo);
	} else {
		__smb2_lease_break_noti(&work->work);
		if (opinfo->o_lease->new_state == SMB2_LEASE_NONE_LE) {
			opinfo->o_lease->state = SMB2_LEASE_NONE_LE;
			lease_update_oplock_levels(opinfo->o_lease);
		}
	}
	return 0;
}

static void wait_lease_breaking(struct oplock_info *opinfo)
{
	if (!opinfo->is_lease)
		return;

	wake_up_interruptible_all(&opinfo->oplock_brk);
	if (atomic_read(&opinfo->breaking_cnt)) {
		int ret = 0;

		ret = wait_event_interruptible_timeout(opinfo->oplock_brk,
						       atomic_read(&opinfo->breaking_cnt) == 0,
						       HZ);
		if (!ret)
			atomic_set(&opinfo->breaking_cnt, 0);
	}
}

static int oplock_break(struct oplock_info *brk_opinfo, int req_op_level,
			struct ksmbd_work *in_work)
{
	int err = 0;
	bool sent_interim = false;

	/* Need to break exclusive/batch oplock, write lease or overwrite_if */
	ksmbd_debug(OPLOCK,
		    "request to send oplock(level : 0x%x) break notification\n",
		    brk_opinfo->level);

	if (brk_opinfo->is_lease) {
		struct lease *lease = brk_opinfo->o_lease;
		bool open_trunc = brk_opinfo->open_trunc;
		bool was_pending = test_bit(0, &brk_opinfo->pending_break);
		bool wait_ack;
		bool inc_epoch = true;

		if (in_work && was_pending) {
			setup_async_work(in_work, NULL, NULL);
			smb2_send_interim_resp(in_work, STATUS_PENDING);
			release_async_work(in_work);
			sent_interim = true;
		}

		err = oplock_break_pending(brk_opinfo, req_op_level);
		if (err)
			return err < 0 ? err : 0;
		if (was_pending)
			open_trunc = brk_opinfo->open_trunc;

again:
		atomic_inc(&brk_opinfo->breaking_cnt);
		if (open_trunc) {
			/*
			 * Create overwrite break trigger the lease break to
			 * none.
			 */
			lease->new_state = SMB2_LEASE_NONE_LE;
		} else {
			if (lease->state & SMB2_LEASE_WRITE_CACHING_LE) {
				if (lease->state & SMB2_LEASE_HANDLE_CACHING_LE)
					lease->new_state =
						SMB2_LEASE_READ_CACHING_LE |
						SMB2_LEASE_HANDLE_CACHING_LE;
				else
					lease->new_state =
						SMB2_LEASE_READ_CACHING_LE;
			} else {
				if (lease->state & SMB2_LEASE_HANDLE_CACHING_LE &&
						!lease->is_dir)
					lease->new_state =
						SMB2_LEASE_READ_CACHING_LE;
				else
					lease->new_state = SMB2_LEASE_NONE_LE;
			}
		}

		if (in_work && !sent_interim) {
			setup_async_work(in_work, NULL, NULL);
			smb2_send_interim_resp(in_work, STATUS_PENDING);
			release_async_work(in_work);
			sent_interim = true;
		}

		if (lease->state & (SMB2_LEASE_WRITE_CACHING_LE |
				SMB2_LEASE_HANDLE_CACHING_LE)) {
			brk_opinfo->op_state = OPLOCK_ACK_WAIT;
		} else
			atomic_dec(&brk_opinfo->breaking_cnt);

		wait_ack = !(open_trunc &&
			     lease->state == (SMB2_LEASE_READ_CACHING_LE |
					      SMB2_LEASE_HANDLE_CACHING_LE));
		if (lease->reuse_epoch) {
			inc_epoch = false;
			lease->reuse_epoch = false;
		}
		err = smb2_lease_break_noti(brk_opinfo, wait_ack, inc_epoch);
		inc_epoch = false;

		ksmbd_debug(OPLOCK, "oplock granted = %d\n", brk_opinfo->level);
		if (brk_opinfo->op_state == OPLOCK_CLOSING)
			err = -ENOENT;

		if (wait_ack)
			wait_lease_breaking(brk_opinfo);
		if (wait_ack && !err &&
		    lease_break_needed(brk_opinfo, req_op_level, open_trunc))
			goto again;

		wake_up_oplock_break(brk_opinfo);
		return err;
	} else {
		err = oplock_break_pending(brk_opinfo, req_op_level);
		if (err)
			return err < 0 ? err : 0;

		if (brk_opinfo->level == SMB2_OPLOCK_LEVEL_BATCH ||
		    brk_opinfo->level == SMB2_OPLOCK_LEVEL_EXCLUSIVE)
			brk_opinfo->op_state = OPLOCK_ACK_WAIT;
	}

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (brk_opinfo->is_smb2)
		if (brk_opinfo->is_lease)
			err = smb2_lease_break_noti(brk_opinfo);
		else
			err = smb2_oplock_break_noti(brk_opinfo);
	else
		err = smb1_oplock_break_noti(brk_opinfo);
#else
	err = smb2_oplock_break_noti(brk_opinfo);
#endif

	ksmbd_debug(OPLOCK, "oplock granted = %d\n", brk_opinfo->level);
	if (brk_opinfo->op_state == OPLOCK_CLOSING)
		err = -ENOENT;
	wake_up_oplock_break(brk_opinfo);

	return err;
}

void destroy_lease_table(struct ksmbd_conn *conn)
{
	struct lease_table *lb, *lbtmp;
	struct lease *lease, *ltmp;

	write_lock(&lease_list_lock);
	if (list_empty(&lease_table_list)) {
		write_unlock(&lease_list_lock);
		return;
	}

	list_for_each_entry_safe(lb, lbtmp, &lease_table_list, l_entry) {
		if (conn && memcmp(lb->client_guid, conn->ClientGUID,
				   SMB2_CLIENT_GUID_SIZE))
			continue;
		list_for_each_entry_safe(lease, ltmp, &lb->lease_list, l_entry)
			lease_del_table(lease);
		list_del(&lb->l_entry);
		free_lease_table(lb);
	}
	write_unlock(&lease_list_lock);
}

int find_same_lease_key(struct ksmbd_conn *conn, struct ksmbd_inode *ci,
			struct lease_ctx_info *lctx)
{
	struct lease *lease;
	int err = 0;
	struct lease_table *lb;

	if (!lctx)
		return err;

	read_lock(&lease_list_lock);
	if (list_empty(&lease_table_list)) {
		read_unlock(&lease_list_lock);
		return 0;
	}

	list_for_each_entry(lb, &lease_table_list, l_entry) {
		if (!memcmp(lb->client_guid, conn->ClientGUID,
			    SMB2_CLIENT_GUID_SIZE))
			goto found;
	}
	read_unlock(&lease_list_lock);

	return 0;

found:
	list_for_each_entry(lease, &lb->lease_list, l_entry) {
		if (lease->ci == ci)
			continue;
		if (!memcmp(lease->lease_key, lctx->lease_key,
			    SMB2_LEASE_KEY_SIZE)) {
			err = -EINVAL;
			ksmbd_debug(OPLOCK,
				    "found same lease key is already used in other files\n");
			goto out;
		}
	}

out:
	read_unlock(&lease_list_lock);
	return err;
}

static void add_lease_global_list(struct lease *lease, struct ksmbd_conn *conn,
				 struct lease_table *new_lb)
{
	struct lease_table *lb;

	write_lock(&lease_list_lock);
	list_for_each_entry(lb, &lease_table_list, l_entry) {
		if (!memcmp(lb->client_guid, conn->ClientGUID,
			    SMB2_CLIENT_GUID_SIZE)) {
			lease_add_table(lease, lb);
			write_unlock(&lease_list_lock);
			free_lease_table(new_lb);
			return;
		}
	}

	lease_add_table(lease, new_lb);
	list_add(&new_lb->l_entry, &lease_table_list);
	write_unlock(&lease_list_lock);
}

static void set_oplock_level(struct oplock_info *opinfo, int level,
			     struct lease_ctx_info *lctx)
{
	switch (level) {
#ifdef CONFIG_SMB_INSECURE_SERVER
	case REQ_OPLOCK:
	case REQ_BATCHOPLOCK:
#endif
	case SMB2_OPLOCK_LEVEL_BATCH:
	case SMB2_OPLOCK_LEVEL_EXCLUSIVE:
		grant_write_oplock(opinfo, level, lctx);
		break;
	case SMB2_OPLOCK_LEVEL_II:
		grant_read_oplock(opinfo, lctx);
		break;
	default:
		grant_none_oplock(opinfo, lctx);
		break;
	}
}

void smb_send_parent_lease_break_noti(struct ksmbd_file *fp,
				      struct lease_ctx_info *lctx)
{
	struct oplock_info *opinfo;
	struct ksmbd_inode *p_ci = NULL;

	if (lctx->version != 2)
		return;

	p_ci = ksmbd_inode_lookup_lock(fp->filp->f_path.dentry->d_parent);
	if (!p_ci)
		return;

	down_read(&p_ci->m_lock);
	list_for_each_entry(opinfo, &p_ci->m_op_list, op_entry) {
		if (opinfo->conn == NULL || !opinfo->is_lease)
			continue;

		if (opinfo->o_lease->state != SMB2_OPLOCK_LEVEL_NONE &&
		    (!(lctx->flags & SMB2_LEASE_FLAG_PARENT_LEASE_KEY_SET_LE) ||
		     !compare_guid_key(opinfo, fp->conn->ClientGUID,
				      lctx->parent_lease_key))) {
			if (!atomic_inc_not_zero(&opinfo->refcount))
				continue;

			if (ksmbd_conn_releasing(opinfo->conn)) {
				opinfo_put(opinfo);
				continue;
			}

			oplock_break(opinfo, SMB2_OPLOCK_LEVEL_NONE, NULL);
			opinfo_put(opinfo);
		}
	}
	up_read(&p_ci->m_lock);

	ksmbd_inode_put(p_ci);
}

void smb_lazy_parent_lease_break_close(struct ksmbd_file *fp)
{
	struct oplock_info *opinfo;
	struct ksmbd_inode *p_ci = NULL;

	rcu_read_lock();
	opinfo = rcu_dereference(fp->f_opinfo);

	if (!opinfo || !opinfo->is_lease || opinfo->o_lease->version != 2) {
		rcu_read_unlock();
		return;
	}
	rcu_read_unlock();

	p_ci = ksmbd_inode_lookup_lock(fp->filp->f_path.dentry->d_parent);
	if (!p_ci)
		return;

	down_read(&p_ci->m_lock);
	list_for_each_entry(opinfo, &p_ci->m_op_list, op_entry) {
		if (opinfo->conn == NULL || !opinfo->is_lease)
			continue;

		if (opinfo->o_lease->state != SMB2_OPLOCK_LEVEL_NONE) {
			if (!atomic_inc_not_zero(&opinfo->refcount))
				continue;

			if (ksmbd_conn_releasing(opinfo->conn)) {
				opinfo_put(opinfo);
				continue;
			}

			oplock_break(opinfo, SMB2_OPLOCK_LEVEL_NONE, NULL);
			opinfo_put(opinfo);
		}
	}
	up_read(&p_ci->m_lock);

	ksmbd_inode_put(p_ci);
}

/**
 * smb_grant_oplock() - handle oplock/lease request on file open
 * @work:		smb work
 * @req_op_level:	oplock level
 * @pid:		id of open file
 * @fp:			ksmbd file pointer
 * @tid:		Tree id of connection
 * @lctx:		lease context information on file open
 * @share_ret:		share mode
 *
 * Return:      0 on success, otherwise error
 */
int smb_grant_oplock(struct ksmbd_work *work, int req_op_level, u64 pid,
		     struct ksmbd_file *fp, __u16 tid,
		     struct lease_ctx_info *lctx, int share_ret)
{
	int err = 0;
	int break_level = SMB2_OPLOCK_LEVEL_II;
	struct oplock_info *opinfo = NULL, *prev_opinfo = NULL;
	struct ksmbd_inode *ci = fp->f_ci;
	struct lease_table *new_lb = NULL;
	bool prev_op_has_lease;
	bool new_lease = false;
	__le32 prev_op_state = 0;

	/* Only v2 leases handle the directory */
	if (S_ISDIR(file_inode(fp->filp)->i_mode)) {
		if (!lctx || lctx->version != 2)
			return 0;
	}

	opinfo = alloc_opinfo(work, pid, tid);
	if (!opinfo)
		return -ENOMEM;

	if (lctx) {
		opinfo->o_lease = alloc_lease(lctx, ci);
		if (!opinfo->o_lease) {
			err = -ENOMEM;
			goto err_out;
		}
		opinfo->is_lease = 1;
		new_lease = true;
	}

	/* ci does not have any oplock */
	if (!opinfo_count(fp))
		goto set_lev;

	/* grant none-oplock if second open is trunc */
	if (fp->attrib_only && fp->cdoption != FILE_OVERWRITE_IF_LE &&
	    fp->cdoption != FILE_OVERWRITE_LE &&
	    fp->cdoption != FILE_SUPERSEDE_LE) {
		req_op_level = SMB2_OPLOCK_LEVEL_NONE;
		goto set_lev;
	}

	if (lctx) {
		struct oplock_info *m_opinfo;

		/* is lease already granted ? */
		m_opinfo = same_client_has_lease(ci, work->conn->ClientGUID,
						 lctx);
		if (m_opinfo) {
			lease_put(opinfo->o_lease);
			lease_get(m_opinfo->o_lease);
				opinfo->o_lease = m_opinfo->o_lease;
				opinfo->level = m_opinfo->level;
				new_lease = false;
				opinfo_put(m_opinfo);
				goto out;
			}
	}
	prev_opinfo = opinfo_get_list(ci);
	if (!prev_opinfo ||
	    (prev_opinfo->level == SMB2_OPLOCK_LEVEL_NONE && lctx)) {
		opinfo_put(prev_opinfo);
		goto set_lev;
	}
	prev_op_has_lease = prev_opinfo->is_lease;
	if (prev_op_has_lease)
		prev_op_state = prev_opinfo->o_lease->state;
	if (prev_op_has_lease && !lctx &&
	    prev_op_state & SMB2_LEASE_HANDLE_CACHING_LE)
		break_level = SMB2_OPLOCK_LEVEL_NONE;

	if (share_ret < 0 &&
	    prev_opinfo->level == SMB2_OPLOCK_LEVEL_EXCLUSIVE) {
		err = share_ret;
		opinfo_put(prev_opinfo);
		goto err_out;
	}

	if (prev_opinfo->level != SMB2_OPLOCK_LEVEL_BATCH &&
	    prev_opinfo->level != SMB2_OPLOCK_LEVEL_EXCLUSIVE) {
		opinfo_put(prev_opinfo);
		goto op_break_not_needed;
	}

	err = oplock_break(prev_opinfo, break_level, work);
	opinfo_put(prev_opinfo);
	if (err == -ENOENT)
		goto set_lev;
	/* Check all oplock was freed by close */
	else if (err < 0)
		goto err_out;

op_break_not_needed:
	if (share_ret < 0) {
		err = share_ret;
		goto err_out;
	}

	if (req_op_level != SMB2_OPLOCK_LEVEL_NONE)
		req_op_level = SMB2_OPLOCK_LEVEL_II;

	/* grant fixed oplock on stacked locking between lease and oplock */
	if (prev_op_has_lease && !lctx)
		if (prev_op_state & SMB2_LEASE_HANDLE_CACHING_LE)
			req_op_level = SMB2_OPLOCK_LEVEL_NONE;

	if (!prev_op_has_lease && lctx) {
		req_op_level = SMB2_OPLOCK_LEVEL_II;
		lctx->req_state = SMB2_LEASE_READ_CACHING_LE;
	}

set_lev:
	set_oplock_level(opinfo, req_op_level, lctx);

out:
	/*
	 * Keep the original publication order so concurrent opens can
	 * still observe the in-flight grant via ci->m_op_list, but make
	 * everything after opinfo_add() no-fail by preallocating any new
	 * lease_table first.
	 */
	opinfo->o_fp = fp;
	if (new_lease) {
		new_lb = alloc_lease_table(opinfo);
		if (!new_lb) {
			err = -ENOMEM;
			goto err_out;
		}
	}

	opinfo_count_inc(fp);
	opinfo_add(opinfo, fp);

	if (new_lease)
		add_lease_global_list(opinfo->o_lease, opinfo->conn, new_lb);
	if (opinfo->is_lease)
		lease_add_open(opinfo->o_lease, opinfo);

	rcu_assign_pointer(fp->f_opinfo, opinfo);

	return 0;
err_out:
	kfree(new_lb);
	opinfo_put(opinfo);
	return err;
}

/**
 * smb_break_all_write_oplock() - break batch/exclusive oplock to level2
 * @work:	smb work
 * @fp:		ksmbd file pointer
 * @is_trunc:	truncate on open
 */
static bool smb_break_all_write_oplock(struct ksmbd_work *work,
				       struct ksmbd_file *fp, int is_trunc)
{
	struct oplock_info *brk_opinfo;
	bool sent_break = false;

	brk_opinfo = opinfo_get_list(fp->f_ci);
	if (!brk_opinfo)
		return false;
	if (brk_opinfo->level != SMB2_OPLOCK_LEVEL_BATCH &&
	    brk_opinfo->level != SMB2_OPLOCK_LEVEL_EXCLUSIVE) {
		opinfo_put(brk_opinfo);
		return false;
	}

	brk_opinfo->open_trunc = is_trunc;
	oplock_break(brk_opinfo, SMB2_OPLOCK_LEVEL_II, work);
	sent_break = true;
	opinfo_put(brk_opinfo);

	return sent_break;
}

/**
 * smb_break_all_levII_oplock() - send level2 oplock or read lease break command
 *	from server to client
 * @work:	smb work
 * @fp:		ksmbd file pointer
 * @is_trunc:	truncate on open
 */
static void __smb_break_all_levII_oplock(struct ksmbd_work *work,
					 struct ksmbd_file *fp, int is_trunc,
					 bool send_interim)
{
	struct oplock_info *op, *brk_op;
	struct ksmbd_inode *ci;
	struct ksmbd_conn *conn = work->conn;
	bool sent_interim = false;

	if (!test_share_config_flag(work->tcon->share_conf,
				    KSMBD_SHARE_FLAG_OPLOCKS))
		return;

	ci = fp->f_ci;
	op = opinfo_get(fp);

	down_read(&ci->m_lock);
	list_for_each_entry(brk_op, &ci->m_op_list, op_entry) {
		if (brk_op->conn == NULL)
			continue;

		if (!atomic_inc_not_zero(&brk_op->refcount))
			continue;

		if (ksmbd_conn_releasing(brk_op->conn)) {
			opinfo_put(brk_op);
			continue;
		}

#ifdef CONFIG_SMB_INSECURE_SERVER
		if (brk_op->is_smb2) {
			if (brk_op->is_lease && (brk_op->o_lease->state &
					(~(SMB2_LEASE_READ_CACHING_LE |
					   SMB2_LEASE_HANDLE_CACHING_LE)))) {
				ksmbd_debug(OPLOCK,
					    "unexpected lease state(0x%x)\n",
					    brk_op->o_lease->state);
				goto next;
			} else if (brk_op->level !=
					SMB2_OPLOCK_LEVEL_II) {
				ksmbd_debug(OPLOCK, "unexpected oplock(0x%x)\n",
					    brk_op->level);
				goto next;
			}

			/* Skip oplock being break to none */
			if (brk_op->is_lease &&
			    brk_op->o_lease->new_state == SMB2_LEASE_NONE_LE &&
			    atomic_read(&brk_op->breaking_cnt))
				goto next;
		} else {
			if (brk_op->level != OPLOCK_READ) {
				ksmbd_debug(OPLOCK, "unexpected oplock(0x%x)\n",
					    brk_op->level);
				goto next;
			}
		}
#else
		if (!brk_op->is_lease &&
		    brk_op->level != SMB2_OPLOCK_LEVEL_II) {
			ksmbd_debug(OPLOCK, "unexpected oplock(0x%x)\n",
				    brk_op->level);
			goto next;
		}

		/* Skip oplock being break to none */
		if (brk_op->is_lease &&
		    brk_op->o_lease->new_state == SMB2_LEASE_NONE_LE &&
		    atomic_read(&brk_op->breaking_cnt))
			goto next;
#endif

		if (op && op->is_lease && brk_op->is_lease &&
		    !memcmp(conn->ClientGUID, brk_op->conn->ClientGUID,
			    SMB2_CLIENT_GUID_SIZE) &&
		    !memcmp(op->o_lease->lease_key, brk_op->o_lease->lease_key,
			    SMB2_LEASE_KEY_SIZE))
			goto next;
		brk_op->open_trunc = is_trunc;
		oplock_break(brk_op,
			     brk_op->is_lease && !is_trunc ?
			     SMB2_OPLOCK_LEVEL_II : SMB2_OPLOCK_LEVEL_NONE,
			     send_interim && !sent_interim ? work : NULL);
		sent_interim = true;
next:
		opinfo_put(brk_op);
	}
	up_read(&ci->m_lock);

	if (op)
		opinfo_put(op);
}

void smb_break_all_levII_oplock(struct ksmbd_work *work, struct ksmbd_file *fp,
				int is_trunc)
{
	__smb_break_all_levII_oplock(work, fp, is_trunc, true);
}

/**
 * smb_break_all_oplock() - break both batch/exclusive and level2 oplock
 * @work:	smb work
 * @fp:		ksmbd file pointer
 */
void smb_break_all_oplock(struct ksmbd_work *work, struct ksmbd_file *fp)
{
	bool sent_break;

	if (!test_share_config_flag(work->tcon->share_conf,
				    KSMBD_SHARE_FLAG_OPLOCKS))
		return;

	sent_break = smb_break_all_write_oplock(work, fp, 1);
	__smb_break_all_levII_oplock(work, fp, 1, !sent_break);
}

/**
 * smb2_map_lease_to_oplock() - map lease state to corresponding oplock type
 * @lease_state:     lease type
 *
 * Return:      0 if no mapping, otherwise corresponding oplock type
 */
__u8 smb2_map_lease_to_oplock(__le32 lease_state)
{
	if ((lease_state & SMB2_LEASE_WRITE_CACHING_LE) &&
	    (lease_state & SMB2_LEASE_HANDLE_CACHING_LE)) {
		return SMB2_OPLOCK_LEVEL_BATCH;
	} else if (lease_state & SMB2_LEASE_WRITE_CACHING_LE) {
		return SMB2_OPLOCK_LEVEL_EXCLUSIVE;
	} else if (lease_state & (SMB2_LEASE_READ_CACHING_LE |
				  SMB2_LEASE_HANDLE_CACHING_LE)) {
		return SMB2_OPLOCK_LEVEL_II;
	}
	return 0;
}

/**
 * create_lease_buf() - create lease context for open cmd response
 * @rbuf:	buffer to create lease context response
 * @lease:	buffer to stored parsed lease state information
 */
void create_lease_buf(u8 *rbuf, struct lease *lease)
{
	if (lease->version == 2) {
		struct create_lease_v2 *buf = (struct create_lease_v2 *)rbuf;
		__le32 flags = 0;

		memset(buf, 0, sizeof(struct create_lease_v2));
		memcpy(buf->lcontext.LeaseKey, lease->lease_key,
		       SMB2_LEASE_KEY_SIZE);
		if (lease_has_parent_key(lease))
			flags |= SMB2_LEASE_FLAG_PARENT_LEASE_KEY_SET_LE;
		if (lease_break_in_progress(lease))
			flags |= SMB2_LEASE_FLAG_BREAK_IN_PROGRESS_LE;
		buf->lcontext.LeaseFlags = flags;
		buf->lcontext.Epoch = cpu_to_le16(lease->epoch);
		buf->lcontext.LeaseState = lease->state;
		if (lease_has_parent_key(lease))
			memcpy(buf->lcontext.ParentLeaseKey, lease->parent_lease_key,
			       SMB2_LEASE_KEY_SIZE);
		buf->ccontext.DataOffset = cpu_to_le16(offsetof
				(struct create_lease_v2, lcontext));
		buf->ccontext.DataLength = cpu_to_le32(sizeof(struct lease_context_v2));
		buf->ccontext.NameOffset = cpu_to_le16(offsetof
				(struct create_lease_v2, Name));
		buf->ccontext.NameLength = cpu_to_le16(4);
		buf->Name[0] = 'R';
		buf->Name[1] = 'q';
		buf->Name[2] = 'L';
		buf->Name[3] = 's';
	} else {
		struct create_lease *buf = (struct create_lease *)rbuf;

		memset(buf, 0, sizeof(struct create_lease));
		memcpy(buf->lcontext.LeaseKey, lease->lease_key, SMB2_LEASE_KEY_SIZE);
		if (lease_break_in_progress(lease))
			buf->lcontext.LeaseFlags =
				SMB2_LEASE_FLAG_BREAK_IN_PROGRESS_LE;
		buf->lcontext.LeaseState = lease->state;
		buf->ccontext.DataOffset = cpu_to_le16(offsetof
				(struct create_lease, lcontext));
		buf->ccontext.DataLength = cpu_to_le32(sizeof(struct lease_context));
		buf->ccontext.NameOffset = cpu_to_le16(offsetof
				(struct create_lease, Name));
		buf->ccontext.NameLength = cpu_to_le16(4);
		buf->Name[0] = 'R';
		buf->Name[1] = 'q';
		buf->Name[2] = 'L';
		buf->Name[3] = 's';
	}
}

/**
 * parse_lease_state() - parse lease context contained in file open request
 * @open_req:	buffer containing smb2 file open(create) request
 * @is_dir:	whether leasing file is directory
 *
 * Return: allocated lease context object on success, otherwise NULL
 */
struct lease_ctx_info *parse_lease_state(void *open_req)
{
	struct create_context *cc;
	struct smb2_create_req *req = (struct smb2_create_req *)open_req;
	struct lease_ctx_info *lreq;

	cc = smb2_find_context_vals(req, SMB2_CREATE_REQUEST_LEASE, 4);
	if (IS_ERR(cc))
		return ERR_CAST(cc);
	if (!cc)
		return NULL;

	lreq = kzalloc(sizeof(struct lease_ctx_info), KSMBD_DEFAULT_GFP);
	if (!lreq)
		return ERR_PTR(-ENOMEM);

	if (sizeof(struct lease_context_v2) == le32_to_cpu(cc->DataLength)) {
		struct create_lease_v2 *lc = (struct create_lease_v2 *)cc;

		if (le16_to_cpu(cc->DataOffset) + le32_to_cpu(cc->DataLength) <
		    sizeof(struct create_lease_v2) - 4)
			goto err_out;

		memcpy(lreq->lease_key, lc->lcontext.LeaseKey, SMB2_LEASE_KEY_SIZE);
		lreq->req_state = lc->lcontext.LeaseState;
		lreq->flags = lc->lcontext.LeaseFlags;
		lreq->epoch = lc->lcontext.Epoch;
		lreq->duration = lc->lcontext.LeaseDuration;
		if (!lease_state_valid(lreq->req_state) ||
		    !lease_v2_flags_valid(lreq->flags))
			goto err_out;
		if (lreq->flags == SMB2_LEASE_FLAG_PARENT_LEASE_KEY_SET_LE)
			memcpy(lreq->parent_lease_key, lc->lcontext.ParentLeaseKey,
			       SMB2_LEASE_KEY_SIZE);
		lreq->version = 2;
	} else if (sizeof(struct lease_context) == le32_to_cpu(cc->DataLength)) {
		struct create_lease *lc = (struct create_lease *)cc;

		if (le16_to_cpu(cc->DataOffset) + le32_to_cpu(cc->DataLength) <
		    sizeof(struct create_lease))
			goto err_out;

		memcpy(lreq->lease_key, lc->lcontext.LeaseKey, SMB2_LEASE_KEY_SIZE);
		lreq->req_state = lc->lcontext.LeaseState;
		lreq->flags = 0;
		lreq->duration = lc->lcontext.LeaseDuration;
		if (!lease_state_valid(lreq->req_state))
			goto err_out;
		lreq->version = 1;
	} else
		goto err_out;
	return lreq;
err_out:
	kfree(lreq);
	return ERR_PTR(-EINVAL);
}

/**
 * smb2_find_context_vals() - find a particular context info in open request
 * @open_req:	buffer containing smb2 file open(create) request
 * @tag:	context name to search for
 * @tag_len:	the length of tag
 *
 * Return:	pointer to requested context, NULL if @str context not found
 *		or error pointer if name length is invalid.
 */
struct create_context *smb2_find_context_vals(void *open_req, const char *tag, int tag_len)
{
	struct create_context *cc;
	unsigned int next = 0;
	char *name;
	struct smb2_create_req *req = (struct smb2_create_req *)open_req;
	unsigned int remain_len, name_off, name_len, value_off, value_len,
		     cc_len;

	/*
	 * CreateContextsOffset and CreateContextsLength are guaranteed to
	 * be valid because of ksmbd_smb2_check_message().
	 */
	cc = (struct create_context *)((char *)req +
				       le32_to_cpu(req->CreateContextsOffset));
	remain_len = le32_to_cpu(req->CreateContextsLength);
	do {
		cc = (struct create_context *)((char *)cc + next);
		if (remain_len < offsetof(struct create_context, Buffer))
			return ERR_PTR(-EINVAL);

		next = le32_to_cpu(cc->Next);
		name_off = le16_to_cpu(cc->NameOffset);
		name_len = le16_to_cpu(cc->NameLength);
		value_off = le16_to_cpu(cc->DataOffset);
		value_len = le32_to_cpu(cc->DataLength);
		cc_len = next ? next : remain_len;

		if ((next & 0x7) != 0 ||
		    next > remain_len ||
		    name_off != offsetof(struct create_context, Buffer) ||
		    name_len < 4 ||
		    name_off + name_len > cc_len ||
		    (value_off & 0x7) != 0 ||
		    (value_len && value_off < name_off + (name_len < 8 ? 8 : name_len)) ||
		    ((u64)value_off + value_len > cc_len))
			return ERR_PTR(-EINVAL);

		name = (char *)cc + name_off;
		if (name_len == tag_len && !memcmp(name, tag, name_len))
			return cc;

		remain_len -= next;
	} while (next != 0);

	return NULL;
}

/**
 * create_durable_rsp_buf() - create durable handle context
 * @cc:	buffer to create durable context response
 */
void create_durable_rsp_buf(char *cc)
{
	struct create_durable_rsp *buf;

	buf = (struct create_durable_rsp *)cc;
	memset(buf, 0, sizeof(struct create_durable_rsp));
	buf->ccontext.DataOffset = cpu_to_le16(offsetof
			(struct create_durable_rsp, Data));
	buf->ccontext.DataLength = cpu_to_le32(8);
	buf->ccontext.NameOffset = cpu_to_le16(offsetof
			(struct create_durable_rsp, Name));
	buf->ccontext.NameLength = cpu_to_le16(4);
	/* SMB2_CREATE_DURABLE_HANDLE_RESPONSE is "DHnQ" */
	buf->Name[0] = 'D';
	buf->Name[1] = 'H';
	buf->Name[2] = 'n';
	buf->Name[3] = 'Q';
}

/**
 * create_durable_v2_rsp_buf() - create durable handle v2 context
 * @cc:	buffer to create durable context response
 * @fp: ksmbd file pointer
 */
void create_durable_v2_rsp_buf(char *cc, struct ksmbd_file *fp)
{
	struct create_durable_v2_rsp *buf;

	buf = (struct create_durable_v2_rsp *)cc;
	memset(buf, 0, sizeof(struct create_durable_rsp));
	buf->ccontext.DataOffset = cpu_to_le16(offsetof
			(struct create_durable_rsp, Data));
	buf->ccontext.DataLength = cpu_to_le32(8);
	buf->ccontext.NameOffset = cpu_to_le16(offsetof
			(struct create_durable_rsp, Name));
	buf->ccontext.NameLength = cpu_to_le16(4);
	/* SMB2_CREATE_DURABLE_HANDLE_RESPONSE_V2 is "DH2Q" */
	buf->Name[0] = 'D';
	buf->Name[1] = 'H';
	buf->Name[2] = '2';
	buf->Name[3] = 'Q';

	buf->Timeout = cpu_to_le32(fp->durable_timeout);
	if (fp->is_persistent)
		buf->Flags = SMB2_DHANDLE_FLAG_PERSISTENT;
}

/**
 * create_mxac_rsp_buf() - create query maximal access context
 * @cc:			buffer to create maximal access context response
 * @maximal_access:	maximal access
 */
void create_mxac_rsp_buf(char *cc, int maximal_access)
{
	struct create_mxac_rsp *buf;

	buf = (struct create_mxac_rsp *)cc;
	memset(buf, 0, sizeof(struct create_mxac_rsp));
	buf->ccontext.DataOffset = cpu_to_le16(offsetof
			(struct create_mxac_rsp, QueryStatus));
	buf->ccontext.DataLength = cpu_to_le32(8);
	buf->ccontext.NameOffset = cpu_to_le16(offsetof
			(struct create_mxac_rsp, Name));
	buf->ccontext.NameLength = cpu_to_le16(4);
	/* SMB2_CREATE_QUERY_MAXIMAL_ACCESS_RESPONSE is "MxAc" */
	buf->Name[0] = 'M';
	buf->Name[1] = 'x';
	buf->Name[2] = 'A';
	buf->Name[3] = 'c';

	buf->QueryStatus = STATUS_SUCCESS;
	buf->MaximalAccess = cpu_to_le32(maximal_access);
}

void create_disk_id_rsp_buf(char *cc, __u64 file_id, __u64 vol_id)
{
	struct create_disk_id_rsp *buf;

	buf = (struct create_disk_id_rsp *)cc;
	memset(buf, 0, sizeof(struct create_disk_id_rsp));
	buf->ccontext.DataOffset = cpu_to_le16(offsetof
			(struct create_disk_id_rsp, DiskFileId));
	buf->ccontext.DataLength = cpu_to_le32(32);
	buf->ccontext.NameOffset = cpu_to_le16(offsetof
			(struct create_mxac_rsp, Name));
	buf->ccontext.NameLength = cpu_to_le16(4);
	/* SMB2_CREATE_QUERY_ON_DISK_ID_RESPONSE is "QFid" */
	buf->Name[0] = 'Q';
	buf->Name[1] = 'F';
	buf->Name[2] = 'i';
	buf->Name[3] = 'd';

	buf->DiskFileId = cpu_to_le64(file_id);
	buf->VolumeId = cpu_to_le64(vol_id);
}

/**
 * create_posix_rsp_buf() - create posix extension context
 * @cc:	buffer to create posix on posix response
 * @fp: ksmbd file pointer
 */
void create_posix_rsp_buf(char *cc, struct ksmbd_file *fp)
{
	struct create_posix_rsp *buf;
	struct inode *inode = file_inode(fp->filp);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)
	struct mnt_idmap *idmap = file_mnt_idmap(fp->filp);
	vfsuid_t vfsuid = i_uid_into_vfsuid(idmap, inode);
	vfsgid_t vfsgid = i_gid_into_vfsgid(idmap, inode);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	struct user_namespace *user_ns = file_mnt_user_ns(fp->filp);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	vfsuid_t vfsuid = i_uid_into_vfsuid(user_ns, inode);
	vfsgid_t vfsgid = i_gid_into_vfsgid(user_ns, inode);
#endif
#endif

	buf = (struct create_posix_rsp *)cc;
	memset(buf, 0, sizeof(struct create_posix_rsp));
	buf->ccontext.DataOffset = cpu_to_le16(offsetof
			(struct create_posix_rsp, nlink));
	/*
	 * DataLength = nlink(4) + reparse_tag(4) + mode(4) +
	 * domain sid(28) + unix group sid(16).
	 */
	buf->ccontext.DataLength = cpu_to_le32(56);
	buf->ccontext.NameOffset = cpu_to_le16(offsetof
			(struct create_posix_rsp, Name));
	buf->ccontext.NameLength = cpu_to_le16(POSIX_CTXT_DATA_LEN);
	/* SMB2_CREATE_TAG_POSIX is "0x93AD25509CB411E7B42383DE968BCD7C" */
	buf->Name[0] = 0x93;
	buf->Name[1] = 0xAD;
	buf->Name[2] = 0x25;
	buf->Name[3] = 0x50;
	buf->Name[4] = 0x9C;
	buf->Name[5] = 0xB4;
	buf->Name[6] = 0x11;
	buf->Name[7] = 0xE7;
	buf->Name[8] = 0xB4;
	buf->Name[9] = 0x23;
	buf->Name[10] = 0x83;
	buf->Name[11] = 0xDE;
	buf->Name[12] = 0x96;
	buf->Name[13] = 0x8B;
	buf->Name[14] = 0xCD;
	buf->Name[15] = 0x7C;

	buf->nlink = cpu_to_le32(inode->i_nlink);
	buf->reparse_tag = cpu_to_le32(fp->volatile_id);
	buf->mode = cpu_to_le32(inode->i_mode & 0777);
	/*
	 * SidBuffer(44) contain two sids(Domain sid(28), UNIX group sid(16)).
	 * Domain sid(28) = revision(1) + num_subauth(1) + authority(6) +
	 * 		    sub_auth(4 * 4(num_subauth)) + RID(4).
	 * UNIX group id(16) = revision(1) + num_subauth(1) + authority(6) +
	 * 		       sub_auth(4 * 1(num_subauth)) + RID(4).
	 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	id_to_sid(from_kuid_munged(&init_user_ns, vfsuid_into_kuid(vfsuid)),
#else
	id_to_sid(from_kuid_munged(&init_user_ns,
				   i_uid_into_mnt(user_ns, inode)),
#endif
#else
	id_to_sid(from_kuid_munged(&init_user_ns, inode->i_uid),
#endif
		  SIDOWNER, (struct smb_sid *)&buf->SidBuffer[0]);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
	id_to_sid(from_kgid_munged(&init_user_ns, vfsgid_into_kgid(vfsgid)),
#else
	id_to_sid(from_kgid_munged(&init_user_ns,
				   i_gid_into_mnt(user_ns, inode)),
#endif
#else
	id_to_sid(from_kgid_munged(&init_user_ns, inode->i_gid),
#endif
		  SIDUNIX_GROUP, (struct smb_sid *)&buf->SidBuffer[28]);
}

/*
 * Find lease object(opinfo) for given lease key/fid from lease
 * break/file close path.
 */
/**
 * lookup_lease_in_table() - find a matching lease info object
 * @conn:	connection instance
 * @lease_key:	lease key to be searched for
 *
 * Return:      opinfo if found matching opinfo, otherwise NULL
 */
struct oplock_info *lookup_lease_in_table(struct ksmbd_conn *conn,
					  char *lease_key)
{
	struct oplock_info *opinfo = NULL, *ret_op = NULL;
	struct lease *lease;
	struct lease_table *lt;

	read_lock(&lease_list_lock);
	list_for_each_entry(lt, &lease_table_list, l_entry) {
		if (!memcmp(lt->client_guid, conn->ClientGUID,
			    SMB2_CLIENT_GUID_SIZE))
			goto found;
	}

	read_unlock(&lease_list_lock);
	return NULL;

found:
	list_for_each_entry(lease, &lt->lease_list, l_entry) {
		if (memcmp(lease->lease_key, lease_key, SMB2_LEASE_KEY_SIZE))
			continue;
		if (!(lease->state & (SMB2_LEASE_HANDLE_CACHING_LE |
				      SMB2_LEASE_WRITE_CACHING_LE)))
			break;

		spin_lock(&lease->lock);
		list_for_each_entry(opinfo, &lease->open_list, lease_entry) {
			if (!opinfo->op_state ||
			    opinfo->op_state == OPLOCK_CLOSING)
				continue;
			if (!atomic_inc_not_zero(&opinfo->refcount))
				continue;
			ret_op = opinfo;
			break;
		}
		spin_unlock(&lease->lock);
		if (ret_op) {
			ksmbd_debug(OPLOCK, "found opinfo\n");
			goto out;
		}
		break;
	}

out:
	read_unlock(&lease_list_lock);
	return ret_op;
}

int smb2_check_durable_oplock(struct ksmbd_conn *conn,
			      struct ksmbd_share_config *share,
			      struct ksmbd_file *fp,
			      struct lease_ctx_info *lctx,
			      struct ksmbd_user *user,
			      char *name)
{
	struct oplock_info *opinfo = opinfo_get(fp);
	int ret = 0;

	if (!opinfo)
		return 0;

	if (ksmbd_vfs_compare_durable_owner(fp, user) == false) {
		ksmbd_debug(SMB, "Durable handle reconnect failed: owner mismatch\n");
		ret = -EBADF;
		goto out;
	}

	if (opinfo->is_lease == false) {
		if (lctx) {
			pr_err("create context include lease\n");
			ret = -EBADF;
			goto out;
		}

		if (opinfo->level != SMB2_OPLOCK_LEVEL_BATCH) {
			pr_err("oplock level is not equal to SMB2_OPLOCK_LEVEL_BATCH\n");
			ret = -EBADF;
		}

		goto out;
	}

	if (memcmp(conn->ClientGUID, fp->client_guid,
				SMB2_CLIENT_GUID_SIZE)) {
		ksmbd_debug(SMB, "Client guid of fp is not equal to the one of connection\n");
		ret = -EBADF;
		goto out;
	}

	if (!lctx) {
		ksmbd_debug(SMB, "create context does not include lease\n");
		ret = -EBADF;
		goto out;
	}

	if (memcmp(opinfo->o_lease->lease_key, lctx->lease_key,
				SMB2_LEASE_KEY_SIZE)) {
		ksmbd_debug(SMB,
			    "lease key of fp does not match lease key in create context\n");
		ret = -EBADF;
		goto out;
	}

	if (!(opinfo->o_lease->state & SMB2_LEASE_HANDLE_CACHING_LE)) {
		ksmbd_debug(SMB, "lease state does not contain SMB2_LEASE_HANDLE_CACHING\n");
		ret = -EBADF;
		goto out;
	}

	if (opinfo->o_lease->version != lctx->version) {
		ksmbd_debug(SMB,
			    "lease version of fp does not match the one in create context\n");
		ret = -EBADF;
		goto out;
	}

	if (!ksmbd_inode_pending_delete(fp))
		ret = ksmbd_validate_name_reconnect(share, fp, name);
out:
	opinfo_put(opinfo);
	return ret;
}
