/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#include "lib/misc/lib.h"
#include "lib/commands/toolcontext.h"
#include "lib/metadata/metadata.h"
#include "lib/metadata/segtype.h"
#include "lib/activate/activate.h"
#include "lib/locking/lvmlockd.h"
#include "lib/cache/lvmcache.h"
#include "daemons/lvmlockd/lvmlockd-client.h"

static daemon_handle _lvmlockd;
static const char *_lvmlockd_socket = NULL;
static int _use_lvmlockd = 0;         /* is 1 if command is configured to use lvmlockd */
static int _lvmlockd_connected = 0;   /* is 1 if command is connected to lvmlockd */
static int _lvmlockd_init_failed = 0; /* used to suppress further warnings */

void lvmlockd_set_socket(const char *sock)
{
	_lvmlockd_socket = sock;
}

/*
 * Set directly from global/use_lvmlockd
 */
void lvmlockd_set_use(int use)
{
	_use_lvmlockd = use;
}

/*
 * Returns the value of global/use_lvmlockd being used by the command.
 */
int lvmlockd_use(void)
{
	return _use_lvmlockd;
}

/*
 * The command continues even if init and/or connect fail,
 * because the command is allowed to use local VGs without lvmlockd,
 * and is allowed to read lockd VGs without locks from lvmlockd.
 */
void lvmlockd_init(struct cmd_context *cmd)
{
	if (!_use_lvmlockd) {
		/* Should never happen, don't call init when not using lvmlockd. */
		log_error("Should not initialize lvmlockd with use_lvmlockd=0.");
	}

	if (!_lvmlockd_socket) {
		log_warn("WARNING: lvmlockd socket location is not configured.");
		_lvmlockd_init_failed = 1;
	}

	if (!!access(LVMLOCKD_PIDFILE, F_OK)) {
		log_warn("WARNING: lvmlockd process is not running.");
		_lvmlockd_init_failed = 1;
	} else {
		_lvmlockd_init_failed = 0;
	}
}

void lvmlockd_connect(void)
{
	if (!_use_lvmlockd) {
		/* Should never happen, don't call connect when not using lvmlockd. */
		log_error("Should not connect to lvmlockd with use_lvmlockd=0.");
	}

	if (_lvmlockd_connected) {
		/* Should never happen, only call connect once. */
		log_error("lvmlockd is already connected.");
	}

	if (_lvmlockd_init_failed)
		return;

	_lvmlockd = lvmlockd_open(_lvmlockd_socket);

	if (_lvmlockd.socket_fd >= 0 && !_lvmlockd.error) {
		log_debug("Successfully connected to lvmlockd on fd %d.", _lvmlockd.socket_fd);
		_lvmlockd_connected = 1;
	} else {
		log_warn("WARNING: lvmlockd connect failed.");
	}
}

void lvmlockd_disconnect(void)
{
	if (_lvmlockd_connected)
		daemon_close(_lvmlockd);
	_lvmlockd_connected = 0;
}

/* Translate the result strings from lvmlockd to bit flags. */
static void _flags_str_to_lockd_flags(const char *flags_str, uint32_t *lockd_flags)
{
	if (strstr(flags_str, "NO_LOCKSPACES"))
		*lockd_flags |= LD_RF_NO_LOCKSPACES;

	if (strstr(flags_str, "NO_GL_LS"))
		*lockd_flags |= LD_RF_NO_GL_LS;

	if (strstr(flags_str, "NO_LM"))
		*lockd_flags |= LD_RF_NO_LM;

	if (strstr(flags_str, "DUP_GL_LS"))
		*lockd_flags |= LD_RF_DUP_GL_LS;

	if (strstr(flags_str, "WARN_GL_REMOVED"))
		*lockd_flags |= LD_RF_WARN_GL_REMOVED;
}

/*
 * evaluate the reply from lvmlockd, check for errors, extract
 * the result and lockd_flags returned by lvmlockd.
 * 0 failure (no result/lockd_flags set)
 * 1 success (result/lockd_flags set)
 */

/*
 * This is an arbitrary number that we know lvmlockd
 * will not return.  daemon_reply_int reverts to this
 * value if it finds no result value.
 */
#define NO_LOCKD_RESULT (-1000)

static int _lockd_result(daemon_reply reply, int *result, uint32_t *lockd_flags)
{
	int reply_result;
	const char *flags_str = NULL;
	const char *lock_type = NULL;

	*result = -1;

	if (reply.error) {
		log_error("lockd_result reply error %d", reply.error);
		return 0;
	}

	if (strcmp(daemon_reply_str(reply, "response", ""), "OK")) {
		log_error("lockd_result bad response");
		return 0;
	}

	reply_result = daemon_reply_int(reply, "op_result", NO_LOCKD_RESULT);
	if (reply_result == NO_LOCKD_RESULT) {
		log_error("lockd_result no op_result");
		return 0;
	}

	/* The lock_type that lvmlockd used for locking. */
	lock_type = daemon_reply_str(reply, "lock_type", "none");

	*result = reply_result;

	if (lockd_flags) {
		if ((flags_str = daemon_reply_str(reply, "result_flags", NULL)))
			_flags_str_to_lockd_flags(flags_str, lockd_flags);
	}

	log_debug("lockd_result %d flags %s lm %s", reply_result,
		  flags_str ? flags_str : "none", lock_type);
	return 1;
}

static daemon_reply _lockd_send(const char *req_name, ...)
{
	va_list ap;
	daemon_reply repl;
	daemon_request req;

	req = daemon_request_make(req_name);

	va_start(ap, req_name);
	daemon_request_extend_v(req, ap);
	va_end(ap);

	repl = daemon_send(_lvmlockd, req);

	daemon_request_destroy(req);

	return repl;
}

/*
 * result/lockd_flags are values returned from lvmlockd.
 *
 * return 0 (failure)
 * return 1 (result/lockd_flags indicate success/failure)
 *
 * return 1 result 0   (success)
 * return 1 result < 0 (failure)
 *
 * caller may ignore result < 0 failure depending on
 * lockd_flags and the specific command/mode.
 *
 * When this function returns 0 (failure), no result/lockd_flags
 * were obtained from lvmlockd.
 *
 * When this function returns 1 (success), result/lockd_flags may
 * have been obtained from lvmlockd.  This lvmlockd result may
 * indicate a locking failure.
 */

static int _lockd_request(struct cmd_context *cmd,
		          const char *req_name,
		          const char *vg_name,
		          const char *vg_lock_type,
		          const char *vg_lock_args,
		          const char *lv_name,
		          const char *lv_uuid,
		          const char *lv_lock_args,
		          const char *mode,
		          const char *opts,
		          int *result,
		          uint32_t *lockd_flags)
{
	const char *cmd_name = get_cmd_name();
	daemon_reply reply;
	int pid = getpid();

	*result = 0;
	*lockd_flags = 0;

	if (!strcmp(mode, "na"))
		return 1;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	/* cmd and pid are passed for informational and debugging purposes */

	if (!cmd_name || !cmd_name[0])
		cmd_name = "none";

	if (vg_name && lv_name) {
		reply = _lockd_send(req_name,
					"cmd = %s", cmd_name,
					"pid = " FMTd64, (int64_t) pid,
					"mode = %s", mode,
					"opts = %s", opts ?: "none",
					"vg_name = %s", vg_name,
					"lv_name = %s", lv_name,
					"lv_uuid = %s", lv_uuid,
					"vg_lock_type = %s", vg_lock_type ?: "none",
					"vg_lock_args = %s", vg_lock_args ?: "none",
					"lv_lock_args = %s", lv_lock_args ?: "none",
					NULL);

		if (!_lockd_result(reply, result, lockd_flags))
			goto fail;

		log_debug("lvmlockd %s %s vg %s lv %s result %d %x",
			  req_name, mode, vg_name, lv_name, *result, *lockd_flags);

	} else if (vg_name) {
		reply = _lockd_send(req_name,
					"cmd = %s", cmd_name,
					"pid = " FMTd64, (int64_t) pid,
					"mode = %s", mode,
					"opts = %s", opts ?: "none",
					"vg_name = %s", vg_name,
					"vg_lock_type = %s", vg_lock_type ?: "none",
					"vg_lock_args = %s", vg_lock_args ?: "none",
					NULL);

		if (!_lockd_result(reply, result, lockd_flags))
			goto fail;

		log_debug("lvmlockd %s %s vg %s result %d %x",
			  req_name, mode, vg_name, *result, *lockd_flags);

	} else {
		reply = _lockd_send(req_name,
					"cmd = %s", cmd_name,
					"pid = " FMTd64, (int64_t) pid,
					"mode = %s", mode,
					"opts = %s", opts ?: "none",
					"vg_lock_type = %s", vg_lock_type ?: "none",
					NULL);

		if (!_lockd_result(reply, result, lockd_flags))
			goto fail;

		log_debug("lvmlockd %s %s result %d %x",
			  req_name, mode, *result, *lockd_flags);
	}

	daemon_reply_destroy(reply);

	/* result/lockd_flags have lvmlockd result */
	return 1;

 fail:
	/* no result was obtained from lvmlockd */

	log_error("lvmlockd %s %s failed no result", req_name, mode);

	daemon_reply_destroy(reply);
	return 0;
}

/*
 * Eventually add an option to specify which pv the lvmlock lv should be placed on.
 */

#define ONE_MB_IN_BYTES 1048576

static int _create_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg,
			      const char *lock_lv_name, int num_mb)
{
	uint32_t lv_size_bytes;
	uint32_t extent_bytes;
	uint32_t total_extents;
	struct logical_volume *lv;
	struct lvcreate_params lp = {
		.activate = CHANGE_ALY,
		.alloc = ALLOC_INHERIT,
		.major = -1,
		.minor = -1,
		.permission = LVM_READ | LVM_WRITE,
		.pvh = &vg->pvs,
		.read_ahead = DM_READ_AHEAD_NONE,
		.stripes = 1,
		.vg_name = vg->name,
		.lv_name = dm_pool_strdup(cmd->mem, lock_lv_name),
		.zero = 1,
	};

	lv_size_bytes = num_mb * ONE_MB_IN_BYTES;  /* size of sanlock LV in bytes */
	extent_bytes = vg->extent_size * SECTOR_SIZE; /* size of one extent in bytes */
	total_extents = lv_size_bytes / extent_bytes; /* number of extents in sanlock LV */
	lp.extents = total_extents;

	log_debug("Creating lvmlock LV for sanlock with size %um %ub %u extents", num_mb, lv_size_bytes, lp.extents);

	dm_list_init(&lp.tags);

	if (!(lp.segtype = get_segtype_from_string(vg->cmd, SEG_TYPE_NAME_STRIPED)))
		return_0;

	lv = lv_create_single(vg, &lp);
	if (!lv) {
		log_error("Failed to create sanlock lv %s in vg %s", lock_lv_name, vg->name);
		return 0;
	}

	vg->sanlock_lv = lv;

	return 1;
}

static int _remove_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg)
{
	if (!lv_remove(vg->sanlock_lv)) {
		log_error("Failed to remove sanlock LV %s/%s", vg->name, vg->sanlock_lv->name);
		return 0;
	}

	return 1;
}

static int _extend_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg, int extend_mb)
{
	struct device *dev;
	char path[PATH_MAX];
	uint64_t old_size_bytes;
	uint64_t new_size_bytes;
	uint32_t extend_bytes;
	uint32_t extend_sectors;
	uint32_t new_size_sectors;
	struct logical_volume *lv = vg->sanlock_lv;
	struct lvresize_params lp = {
		.sign = SIGN_NONE,
		.size = 0,
		.percent = PERCENT_NONE,
		.resize = LV_EXTEND,
		.force = 1,
	};
	uint64_t i;

	extend_bytes = extend_mb * ONE_MB_IN_BYTES;
	extend_sectors = extend_bytes / SECTOR_SIZE;
	new_size_sectors = lv->size + extend_sectors;
	old_size_bytes = lv->size * SECTOR_SIZE;

	log_debug("Extend sanlock LV from %llus (%llu bytes) to %us (%u bytes)",
		  (unsigned long long)lv->size,
		  (unsigned long long)old_size_bytes,
		  (uint32_t)new_size_sectors,
		  (uint32_t)(new_size_sectors * SECTOR_SIZE));

	lp.size = new_size_sectors;

	if (!lv_resize(lv, &lp, &vg->pvs)) {
		log_error("Extend sanlock LV %s to size %s failed.",
			  display_lvname(lv), display_size(cmd, lp.size));
		return 0;
	}

	if (!lv_refresh_suspend_resume(lv)) {
		log_error("Failed to refresh sanlock LV %s after extend.", display_lvname(lv));
		return 0;
	}

	new_size_bytes = lv->size * SECTOR_SIZE;

	if (dm_snprintf(path, sizeof(path), "%s/mapper/%s-%s", lv->vg->cmd->dev_dir,
			lv->vg->name, lv->name) < 0) {
		log_error("Extend sanlock LV %s name too long - extended size not zeroed.",
			  display_lvname(lv));
		return 0;
	}

	log_debug("Extend sanlock LV zeroing %u bytes from offset %llu to %llu",
		  (uint32_t)(new_size_bytes - old_size_bytes),
		  (unsigned long long)old_size_bytes,
		  (unsigned long long)new_size_bytes);

	log_print("Zeroing %u MiB on extended internal lvmlock LV...", extend_mb);

	if (!(dev = dev_cache_get(cmd, path, NULL))) {
		log_error("Extend sanlock LV %s cannot find device.", display_lvname(lv));
		return 0;
	}

	if (!label_scan_open(dev)) {
		log_error("Extend sanlock LV %s cannot open device.", display_lvname(lv));
		return 0;
	}

	for (i = 0; i < extend_mb; i++) {
		if (!dev_write_zeros(dev, old_size_bytes + (i * ONE_MB_IN_BYTES), ONE_MB_IN_BYTES)) {
			log_error("Extend sanlock LV %s cannot zero device at " FMTu64 ".",
				  display_lvname(lv), (old_size_bytes + i * ONE_MB_IN_BYTES));
			label_scan_invalidate(dev);
			return 0;
		}
	}

	label_scan_invalidate(dev);
	return 1;
}

/* When one host does _extend_sanlock_lv, the others need to refresh the size. */

static int _refresh_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg)
{
	if (!lv_refresh_suspend_resume(vg->sanlock_lv)) {
		log_error("Failed to refresh %s.", vg->sanlock_lv->name);
		return 0;
	}

	return 1;
}

/*
 * Called at the beginning of lvcreate in a sanlock VG to ensure
 * that there is space in the sanlock LV for a new lock.  If it's
 * full, then this extends it.
 */

int handle_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg)
{
	daemon_reply reply;
	int extend_mb;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 1;
	if (!_lvmlockd_connected)
		return 0;

	extend_mb = find_config_tree_int(cmd, global_sanlock_lv_extend_CFG, NULL);

	/*
	 * User can choose to not automatically extend the lvmlock LV
	 * so they can manually extend it.
	 */
	if (!extend_mb)
		return 1;

	/*
	 * Another host may have extended the lvmlock LV already.
	 * Refresh so that we'll find the new space they added
	 * when we search for new space.
	 */
	if (!_refresh_sanlock_lv(cmd, vg))
		return 0;

	/*
	 * Ask lvmlockd/sanlock to look for an unused lock.
	 */
	reply = _lockd_send("find_free_lock",
			"pid = " FMTd64, (int64_t) getpid(),
			"vg_name = %s", vg->name,
			NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	/* No space on the lvmlock lv for a new lease. */
	if (result == -EMSGSIZE)
		ret = _extend_sanlock_lv(cmd, vg, extend_mb);

	daemon_reply_destroy(reply);

	return ret;
}

static int _activate_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg)
{
	if (!activate_lv(cmd, vg->sanlock_lv)) {
		log_error("Failed to activate sanlock lv %s/%s", vg->name, vg->sanlock_lv->name);
		return 0;
	}

	return 1;
}

static int _deactivate_sanlock_lv(struct cmd_context *cmd, struct volume_group *vg)
{
	if (!deactivate_lv(cmd, vg->sanlock_lv)) {
		log_error("Failed to deactivate sanlock lv %s/%s", vg->name, vg->sanlock_lv->name);
		return 0;
	}

	return 1;
}

static int _init_vg_dlm(struct cmd_context *cmd, struct volume_group *vg)
{
	daemon_reply reply;
	const char *reply_str;
	const char *vg_lock_args = NULL;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	reply = _lockd_send("init_vg",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"vg_lock_type = %s", "dlm",
				NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
		result = -ELOCKD;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	switch (result) {
	case 0:
		break;
	case -ELOCKD:
		log_error("VG %s init failed: lvmlockd not available", vg->name);
		break;
	case -EARGS:
		log_error("VG %s init failed: invalid parameters for dlm", vg->name);
		break;
	case -EMANAGER:
		log_error("VG %s init failed: lock manager dlm is not running", vg->name);
		break;
	case -EPROTONOSUPPORT:
		log_error("VG %s init failed: lock manager dlm is not supported by lvmlockd", vg->name);
		break;
	case -EEXIST:
		log_error("VG %s init failed: a lockspace with the same name exists", vg->name);
		break;
	default:
		log_error("VG %s init failed: %d", vg->name, result);
	}

	if (!ret)
		goto out;

	if (!(reply_str = daemon_reply_str(reply, "vg_lock_args", NULL))) {
		log_error("VG %s init failed: lock_args not returned", vg->name);
		ret = 0;
		goto out;
	}

	if (!(vg_lock_args = dm_pool_strdup(cmd->mem, reply_str))) {
		log_error("VG %s init failed: lock_args alloc failed", vg->name);
		ret = 0;
		goto out;
	}

	vg->lock_type = "dlm";
	vg->lock_args = vg_lock_args;

	if (!vg_write(vg) || !vg_commit(vg)) {
		log_error("VG %s init failed: vg_write vg_commit", vg->name);
		ret = 0;
		goto out;
	}

	ret = 1;
out:
	daemon_reply_destroy(reply);
	return ret;
}

static int _init_vg_sanlock(struct cmd_context *cmd, struct volume_group *vg, int lv_lock_count)
{
	daemon_reply reply;
	const char *reply_str;
	const char *vg_lock_args = NULL;
	const char *opts = NULL;
	struct pv_list *pvl;
	struct device *sector_dev;
	uint32_t sector_size = 0;
	unsigned int phys_block_size, block_size;
	int num_mb;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	/*
	 * We need the sector size to know what size to create the LV,
	 * but we're not sure what PV the LV will be allocated from, so
	 * just get the sector size of the first PV.
	 */

	dm_list_iterate_items(pvl, &vg->pvs) {
		if (!dev_get_block_size(pvl->pv->dev, &phys_block_size, &block_size))
			continue;

		if (!sector_size) {
			sector_size = phys_block_size;
			sector_dev = pvl->pv->dev;
		} else if (sector_size != phys_block_size) {
			log_warn("Inconsistent sector sizes for %s and %s.",
				 dev_name(pvl->pv->dev), dev_name(sector_dev));
			return 1;
		}
	}

	if ((sector_size != 512) && (sector_size != 4096)) {
		log_error("Unknown sector size.");
		return 1;
	}

	log_debug("Using sector size %u for sanlock LV", sector_size);

	/* Base starting size of sanlock LV is 256MB/1GB for 512/4K sectors */
	if (sector_size == 512)
		num_mb = 256;
	else if (sector_size == 4096)
		num_mb = 1024;


	/*
	 * Creating the sanlock LV writes the VG containing the new lvmlock
	 * LV, then activates the lvmlock LV.  The lvmlock LV must be active
	 * before we ask lvmlockd to initialize the VG because sanlock needs
	 * to initialize leases on the lvmlock LV.
	 *
	 * When converting an existing VG to sanlock, the sanlock lv needs to
	 * be large enough to hold leases for all existing lvs needing locks.
	 * One sanlock lease uses 1MB/8MB for 512/4K sector size devices, so
	 * increase the initial size by 1MB/8MB for each existing lv.
	 */

	if (lv_lock_count) {
		if (sector_size == 512)
			num_mb += lv_lock_count;
		else if (sector_size == 4096)
			num_mb += 8 * lv_lock_count;
	}

	if (!_create_sanlock_lv(cmd, vg, LOCKD_SANLOCK_LV_NAME, num_mb)) {
		log_error("Failed to create internal lv.");
		return 0;
	}

	/*
	 * N.B. this passes the sanlock lv name as vg_lock_args
	 * even though it is only part of the final args string
	 * which will be returned from lvmlockd.
	 */

	reply = _lockd_send("init_vg",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"vg_lock_type = %s", "sanlock",
				"vg_lock_args = %s", vg->sanlock_lv->name,
				"opts = %s", opts ?: "none",
				NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
		result = -ELOCKD;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	switch (result) {
	case 0:
		break;
	case -ELOCKD:
		log_error("VG %s init failed: lvmlockd not available", vg->name);
		break;
	case -EARGS:
		log_error("VG %s init failed: invalid parameters for sanlock", vg->name);
		break;
	case -EDEVOPEN:
		log_error("VG %s init failed: sanlock cannot open device /dev/mapper/%s-%s", vg->name, vg->name, LOCKD_SANLOCK_LV_NAME);
		log_error("Check that sanlock has permission to access disks.");
		break;
	case -EMANAGER:
		log_error("VG %s init failed: lock manager sanlock is not running", vg->name);
		break;
	case -EPROTONOSUPPORT:
		log_error("VG %s init failed: lock manager sanlock is not supported by lvmlockd", vg->name);
		break;
	case -EMSGSIZE:
		log_error("VG %s init failed: no disk space for leases", vg->name);
		break;
	case -EEXIST:
		log_error("VG %s init failed: a lockspace with the same name exists", vg->name);
		break;
	default:
		log_error("VG %s init failed: %d", vg->name, result);
	}

	if (!ret)
		goto out;

	if (!(reply_str = daemon_reply_str(reply, "vg_lock_args", NULL))) {
		log_error("VG %s init failed: lock_args not returned", vg->name);
		ret = 0;
		goto out;
	}

	if (!(vg_lock_args = dm_pool_strdup(cmd->mem, reply_str))) {
		log_error("VG %s init failed: lock_args alloc failed", vg->name);
		ret = 0;
		goto out;
	}

	lv_set_hidden(vg->sanlock_lv);
	vg->sanlock_lv->status |= LOCKD_SANLOCK_LV;

	vg->lock_type = "sanlock";
	vg->lock_args = vg_lock_args;

	if (!vg_write(vg) || !vg_commit(vg)) {
		log_error("VG %s init failed: vg_write vg_commit", vg->name);
		ret = 0;
		goto out;
	}

	ret = 1;
out:
	if (!ret) {
		/*
		 * The usleep delay gives sanlock time to close the lock lv,
		 * and usually avoids having an annoying error printed.
		 */
		usleep(1000000);
		_deactivate_sanlock_lv(cmd, vg);
		_remove_sanlock_lv(cmd, vg);
		if (!vg_write(vg) || !vg_commit(vg))
			stack;
	}

	daemon_reply_destroy(reply);
	return ret;
}

/* called after vg_remove on disk */

static int _free_vg_dlm(struct cmd_context *cmd, struct volume_group *vg)
{
	daemon_reply reply;
	uint32_t lockd_flags = 0;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	reply = _lockd_send("free_vg",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"vg_lock_type = %s", vg->lock_type,
				"vg_lock_args = %s", vg->lock_args,
				NULL);

	if (!_lockd_result(reply, &result, &lockd_flags)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	if (!ret)
		log_error("_free_vg_dlm lvmlockd result %d", result);

	daemon_reply_destroy(reply);

	return 1;
}

/* called before vg_remove on disk */

static int _busy_vg_dlm(struct cmd_context *cmd, struct volume_group *vg)
{
	daemon_reply reply;
	uint32_t lockd_flags = 0;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	/*
	 * Check that other hosts do not have the VG lockspace started.
	 */

	reply = _lockd_send("busy_vg",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"vg_lock_type = %s", vg->lock_type,
				"vg_lock_args = %s", vg->lock_args,
				NULL);

	if (!_lockd_result(reply, &result, &lockd_flags)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	if (result == -EBUSY) {
		log_error("Lockspace for \"%s\" not stopped on other hosts", vg->name);
		goto out;
	}

	if (!ret)
		log_error("_busy_vg_dlm lvmlockd result %d", result);

 out:
	daemon_reply_destroy(reply);
	return ret;
}

/* called before vg_remove on disk */

static int _free_vg_sanlock(struct cmd_context *cmd, struct volume_group *vg)
{
	daemon_reply reply;
	uint32_t lockd_flags = 0;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	/*
	 * vgremove originally held the global lock, but lost it because the
	 * vgremove command is removing multiple VGs, and removed the VG
	 * holding the global lock before attempting to remove this VG.
	 * To avoid this situation, the user should remove the VG holding
	 * the global lock in a command by itself, or as the last arg in a
	 * vgremove command that removes multiple VGs.
	 */
	if (cmd->lockd_gl_removed) {
		log_error("Global lock failed: global lock was lost by removing a previous VG.");
		return 0;
	}

	if (!vg->lock_args || !strlen(vg->lock_args)) {
		/* Shouldn't happen in general, but maybe in some error cases? */
		log_debug("_free_vg_sanlock %s no lock_args", vg->name);
		return 1;
	}

	reply = _lockd_send("free_vg",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"vg_lock_type = %s", vg->lock_type,
				"vg_lock_args = %s", vg->lock_args,
				NULL);

	if (!_lockd_result(reply, &result, &lockd_flags)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	/*
	 * Other hosts could still be joined to the lockspace, which means they
	 * are using the internal sanlock LV, which means we cannot remove the
	 * VG.  Once other hosts stop using the VG it can be removed.
	 */
	if (result == -EBUSY) {
		log_error("Lockspace for \"%s\" not stopped on other hosts", vg->name);
		goto out;
	}

	if (!ret) {
		log_error("_free_vg_sanlock lvmlockd result %d", result);
		goto out;
	}

	/*
	 * If the global lock was been removed by removing this VG, then:
	 *
	 * Print a warning indicating that the global lock should be enabled
	 * in another remaining sanlock VG.
	 *
	 * Do not allow any more VGs to be removed by this command, e.g.
	 * if a command removes two sanlock VGs, like vgremove foo bar,
	 * and the global lock existed in foo, do not continue to remove
	 * VG bar without the global lock.  See the corresponding check above.
	 */
	if (lockd_flags & LD_RF_WARN_GL_REMOVED) {
		log_warn("VG %s held the sanlock global lock, enable global lock in another VG.", vg->name);
		cmd->lockd_gl_removed = 1;
	}

	/*
	 * The usleep delay gives sanlock time to close the lock lv,
	 * and usually avoids having an annoying error printed.
	 */
	usleep(1000000);

	_deactivate_sanlock_lv(cmd, vg);
	_remove_sanlock_lv(cmd, vg);
 out:
	daemon_reply_destroy(reply);

	return ret;
}

/* vgcreate */

int lockd_init_vg(struct cmd_context *cmd, struct volume_group *vg,
		  const char *lock_type, int lv_lock_count)
{
	switch (get_lock_type_from_string(lock_type)) {
	case LOCK_TYPE_NONE:
		return 1;
	case LOCK_TYPE_CLVM:
		return 1;
	case LOCK_TYPE_DLM:
		return _init_vg_dlm(cmd, vg);
	case LOCK_TYPE_SANLOCK:
		return _init_vg_sanlock(cmd, vg, lv_lock_count);
	default:
		log_error("Unknown lock_type.");
		return 0;
	}
}

static int _lockd_all_lvs(struct cmd_context *cmd, struct volume_group *vg)
{
	struct lv_list *lvl;

	dm_list_iterate_items(lvl, &vg->lvs) {
		if (!lockd_lv_uses_lock(lvl->lv))
			continue;

		if (!lockd_lv(cmd, lvl->lv, "ex", 0)) {
			log_error("LV %s/%s must be inactive on all hosts.",
				  vg->name, lvl->lv->name);
			return 0;
		}

		if (!lockd_lv(cmd, lvl->lv, "un", 0)) {
			log_error("Failed to unlock LV %s/%s.", vg->name, lvl->lv->name);
			return 0;
		}
	}

	return 1;
}

/* vgremove before the vg is removed */

int lockd_free_vg_before(struct cmd_context *cmd, struct volume_group *vg,
			 int changing)
{
	int lock_type_num = get_lock_type_from_string(vg->lock_type);

	/*
	 * Check that no LVs are active on other hosts.
	 * When removing (not changing), each LV is locked
	 * when it is removed, they do not need checking here.
	 */
	if (lock_type_num == LOCK_TYPE_DLM || lock_type_num == LOCK_TYPE_SANLOCK) {
		if (changing && !_lockd_all_lvs(cmd, vg)) {
			log_error("Cannot change VG %s with active LVs", vg->name);
			return 0;
		}
	}

	switch (lock_type_num) {
	case LOCK_TYPE_NONE:
	case LOCK_TYPE_CLVM:
		return 1;
	case LOCK_TYPE_DLM:
		/* returning an error will prevent vg_remove() */
		return _busy_vg_dlm(cmd, vg);
	case LOCK_TYPE_SANLOCK:
		/* returning an error will prevent vg_remove() */
		return _free_vg_sanlock(cmd, vg);
	default:
		log_error("Unknown lock_type.");
		return 0;
	}
}

/* vgremove after the vg is removed */

void lockd_free_vg_final(struct cmd_context *cmd, struct volume_group *vg)
{
	switch (get_lock_type_from_string(vg->lock_type)) {
	case LOCK_TYPE_NONE:
	case LOCK_TYPE_CLVM:
	case LOCK_TYPE_SANLOCK:
		break;
	case LOCK_TYPE_DLM:
		_free_vg_dlm(cmd, vg);
		break;
	default:
		log_error("Unknown lock_type.");
	}
}

/*
 * Starting a vg involves:
 * 1. reading the vg without a lock
 * 2. getting the lock_type/lock_args from the vg metadata
 * 3. doing start_vg in lvmlockd for the lock_type;
 *    this means joining the lockspace
 *
 * The vg read in step 1 should not be used for anything
 * other than getting the lock_type/lock_args/uuid necessary
 * for starting the lockspace.  To use the vg after starting
 * the lockspace, follow the standard method which is:
 * lock the vg, read/use/write the vg, unlock the vg.
 *
 * start_init is 1 when the VG is being started after the
 * command has done lockd_init_vg().  This tells lvmlockd
 * that the VG lockspace being started is new.
 */

int lockd_start_vg(struct cmd_context *cmd, struct volume_group *vg, int start_init)
{
	char uuid[64] __attribute__((aligned(8)));
	daemon_reply reply;
	uint32_t lockd_flags = 0;
	int host_id = 0;
	int result;
	int ret;

	memset(uuid, 0, sizeof(uuid));

	if (!vg_is_shared(vg))
		return 1;

	if (!_use_lvmlockd) {
		log_error("VG %s start failed: lvmlockd is not enabled", vg->name);
		return 0;
	}
	if (!_lvmlockd_connected) {
		log_error("VG %s start failed: lvmlockd is not running", vg->name);
		return 0;
	}

	log_debug("lockd start VG %s lock_type %s init %d",
		  vg->name, vg->lock_type ? vg->lock_type : "empty", start_init);

	if (!id_write_format(&vg->id, uuid, sizeof(uuid)))
		return_0;

	if (vg->lock_type && !strcmp(vg->lock_type, "sanlock")) {
		/*
		 * This is the big difference between starting
		 * sanlock vgs vs starting dlm vgs: the internal
		 * sanlock lv needs to be activated before lvmlockd
		 * does the start because sanlock needs to use the lv
		 * to access locks.
		 */
		if (!_activate_sanlock_lv(cmd, vg))
			return 0;

		host_id = find_config_tree_int(cmd, local_host_id_CFG, NULL);
	}

	reply = _lockd_send("start_vg",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"vg_lock_type = %s", vg->lock_type,
				"vg_lock_args = %s", vg->lock_args ?: "none",
				"vg_uuid = %s", uuid[0] ? uuid : "none",
				"version = " FMTd64, (int64_t) vg->seqno,
				"host_id = " FMTd64, (int64_t) host_id,
				"opts = %s", start_init ? "start_init" : "none",
				NULL);

	if (!_lockd_result(reply, &result, &lockd_flags)) {
		ret = 0;
		result = -ELOCKD;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	if (lockd_flags & LD_RF_WARN_GL_REMOVED)
		cmd->lockd_gl_removed = 1;

	switch (result) {
	case 0:
		log_print_unless_silent("VG %s starting %s lockspace", vg->name, vg->lock_type);
		break;
	case -ELOCKD:
		log_error("VG %s start failed: lvmlockd not available", vg->name);
		break;
	case -EEXIST:
		log_debug("VG %s start error: already started", vg->name);
		ret = 1;
		break;
	case -EARGS:
		log_error("VG %s start failed: invalid parameters for %s", vg->name, vg->lock_type);
		break;
	case -EHOSTID:
		log_error("VG %s start failed: invalid sanlock host_id, set in lvmlocal.conf", vg->name);
		break;
	case -EMANAGER:
		log_error("VG %s start failed: lock manager %s is not running", vg->name, vg->lock_type);
		break;
	case -EPROTONOSUPPORT:
		log_error("VG %s start failed: lock manager %s is not supported by lvmlockd", vg->name, vg->lock_type);
		break;
	default:
		log_error("VG %s start failed: %d", vg->name, result);
	}

	daemon_reply_destroy(reply);

	return ret;
}

int lockd_stop_vg(struct cmd_context *cmd, struct volume_group *vg)
{
	daemon_reply reply;
	int result;
	int ret;

	if (!vg_is_shared(vg))
		return 1;
	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	log_debug("lockd stop VG %s lock_type %s",
		  vg->name, vg->lock_type ? vg->lock_type : "empty");

	reply = _lockd_send("stop_vg",
			"pid = " FMTd64, (int64_t) getpid(),
			"vg_name = %s", vg->name,
			NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	if (result == -ENOLS) {
		ret = 1;
		goto out;
	}

	if (result == -EBUSY) {
		log_error("VG %s stop failed: LVs must first be deactivated", vg->name);
		goto out;
	}

	if (!ret) {
		log_error("VG %s stop failed: %d", vg->name, result);
		goto out;
	}

	if (!strcmp(vg->lock_type, "sanlock")) {
		log_debug("lockd_stop_vg deactivate sanlock lv");
		_deactivate_sanlock_lv(cmd, vg);
	}
out:
	daemon_reply_destroy(reply);

	return ret;
}

int lockd_start_wait(struct cmd_context *cmd)
{
	daemon_reply reply;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	reply = _lockd_send("start_wait",
			"pid = " FMTd64, (int64_t) getpid(),
			NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	if (!ret)
		log_error("Lock start failed");

	/*
	 * FIXME: get a list of vgs that started so we can
	 * better report what worked and what didn't?
	 */

	daemon_reply_destroy(reply);

	if (cmd->lockd_gl_removed) {
		log_error("Missing global lock: global lock was lost by removing a previous VG.");
		log_error("To enable the global lock in another VG, see lvmlockctl --gl-enable.");
	}

	return ret;
}

/*
 * lockd_gl_create() is a variation of lockd_gl() used only by vgcreate.
 * It handles the case that when using sanlock, the global lock does
 * not exist until after the first vgcreate is complete, since the global
 * lock exists on storage within an actual VG.  So, the first vgcreate
 * needs special logic to detect this bootstrap case.
 *
 * When the vgcreate is not creating the first VG, then lockd_gl_create()
 * behaves the same as lockd_gl().
 *
 * vgcreate will have a lock_type for the new VG which lockd_gl_create()
 * can provide in the lock-gl call.
 *
 * lockd_gl() and lockd_gl_create() differ in the specific cases where
 * ENOLS (no lockspace found) is overriden.  In the vgcreate case, the
 * override cases are related to sanlock bootstrap, and the lock_type of
 * the vg being created is needed.
 *
 * 1. vgcreate of the first lockd-type vg calls lockd_gl_create()
 *    to acquire the global lock.
 *
 * 2. vgcreate/lockd_gl_create passes gl lock request to lvmlockd,
 *    along with lock_type of the new vg.
 *
 * 3. lvmlockd finds no global lockspace/lock.
 *
 * 4. dlm:
 *    If the lock_type from vgcreate is dlm, lvmlockd creates the
 *    dlm global lockspace, and queues the global lock request
 *    for vgcreate.  lockd_gl_create returns sucess with the gl held.
 *
 *    sanlock:
 *    If the lock_type from vgcreate is sanlock, lvmlockd returns -ENOLS
 *    with the NO_GL_LS flag.  lvmlockd cannot create or acquire a sanlock
 *    global lock until the VG exists on disk (the locks live within the VG).
 *
 *    lockd_gl_create sees sanlock/ENOLS/NO_GL_LS (and optionally the
 *    "enable" lock-gl arg), determines that this is the sanlock
 *    bootstrap special case, and returns success without the global lock.
 *
 *    vgcreate creates the VG on disk, and calls lockd_init_vg() which
 *    initializes/enables a global lock on the new VG's internal sanlock lv.
 *    Future lockd_gl/lockd_gl_create calls will acquire the existing gl.
 */

int lockd_gl_create(struct cmd_context *cmd, const char *def_mode, const char *vg_lock_type)
{
	const char *mode = NULL;
	uint32_t lockd_flags;
	int retries = 0;
	int result;

	/*
	 * There are four variations of creating a local/lockd VG
	 * with/without use_lvmlockd set.
	 *
	 * use_lvmlockd=1, lockd VG:
	 * This function should acquire or create the global lock.
	 *
	 * use_lvmlockd=0, local VG:
	 * This function is a no-op, just returns 1.
	 *
	 * use_lvmlockd=0, lockd VG
	 * An error is returned in vgcreate_params_set_from_args (before this is called).
	 *
	 * use_lvmlockd=1, local VG
	 * This function should acquire the global lock.
	 */
	if (!_use_lvmlockd) {
		if (!is_lockd_type(vg_lock_type))
			return 1;
		log_error("Cannot create VG with lock_type %s without lvmlockd.", vg_lock_type);
		return 0;
	}

	log_debug("lockd global lock_type %s", vg_lock_type);

	if (!mode)
		mode = def_mode;
	if (!mode) {
		log_error("Unknown lock-gl mode");
		return 0;
	}

 req:
	if (!_lockd_request(cmd, "lock_gl",
			      NULL, vg_lock_type, NULL, NULL, NULL, NULL, mode, NULL,
			      &result, &lockd_flags)) {
		/* No result from lvmlockd, it is probably not running. */
		log_error("Global lock failed: check that lvmlockd is running.");
		return 0;
	}

	if (result == -EAGAIN) {
		if (retries < find_config_tree_int(cmd, global_lvmlockd_lock_retries_CFG, NULL)) {
			log_warn("Retrying %s global lock", mode);
			sleep(1);
			retries++;
			goto req;
		}
	}

	/*
	 * ENOLS: no lockspace was found with a global lock.
	 * It may not exist (perhaps this command is creating the first),
	 * or it may not be visible or started on the system yet.
	 */

	if (result == -ENOLS) {
		if (!strcmp(mode, "un"))
			return 1;

		/*
		 * This is the sanlock bootstrap condition for proceding
		 * without the global lock: a chicken/egg case for the first
		 * sanlock VG that is created.  When creating the first
		 * sanlock VG, there is no global lock to acquire because
		 * the gl will exist in the VG being created.  So, we
		 * skip acquiring the global lock when creating this initial
		 * VG, and enable the global lock in this VG.
		 *
		 * This initial bootstrap condition is identified based on
		 * two things:
		 *
		 * 1. No sanlock VGs have been started in lvmlockd, causing
		 *    lvmlockd to return NO_GL_LS/NO_LOCKSPACES.
		 *
		 * 2. No sanlock VGs are seen in lvmcache after the disk
		 *    scan performed.
		 *
		 * If both of those are true, we go ahead and create this new
		 * VG which will have the global lock enabled.  However, this
		 * has a shortcoming: another sanlock VG may exist that hasn't
		 * appeared to the system yet.  If that VG has its global lock
		 * enabled, then when it appears later, duplicate global locks
		 * will be seen, and a warning will indicate that one of them
		 * should be disabled.
		 *
		 * The two bootstrap conditions have another shortcoming to the
		 * opposite effect:  other sanlock VGs may be visible to the
		 * system, but none of them have a global lock enabled.
		 * In that case, it would make sense to create this new VG with
		 * an enabled global lock.  (FIXME: we could detect that none
		 * of the existing sanlock VGs have a gl enabled and allow this
		 * vgcreate to go ahead.)  Enabling the global lock in one of
		 * the existing sanlock VGs is currently the simplest solution.
		 */

		if ((lockd_flags & LD_RF_NO_GL_LS) &&
		    (lockd_flags & LD_RF_NO_LOCKSPACES) &&
		    !strcmp(vg_lock_type, "sanlock")) {
			if (lvmcache_contains_lock_type_sanlock(cmd)) {
				/* FIXME: we could check that all are started, and then check that none have gl enabled. */
				log_error("Global lock failed: start existing sanlock VGs to access global lock.");
				log_error("(If all sanlock VGs are started, enable global lock with lvmlockctl.)");
				return 0;
			}
			log_print_unless_silent("Enabling sanlock global lock");
			return 1;
		}

		if (!strcmp(vg_lock_type, "sanlock"))
			log_error("Global lock failed: check that VG holding global lock exists and is started.");
		else
			log_error("Global lock failed: check that global lockspace is started.");

		if (lockd_flags & LD_RF_NO_LM)
			log_error("Start a lock manager, lvmlockd did not find one running.");
		return 0;
	}

	/*
	 * Check for each specific error that can be returned so a helpful
	 * message can be printed for it.
	 */
	if (result < 0) {
		if (result == -ESTARTING)
			log_error("Global lock failed: lockspace is starting.");
		else if (result == -EAGAIN)
			log_error("Global lock failed: held by other host.");
		else if (result == -EPROTONOSUPPORT)
			log_error("VG create failed: lock manager %s is not supported by lvmlockd.", vg_lock_type);
		else
			log_error("Global lock failed: error %d", result);
		return 0;
	}

	/* --shared with vgcreate does not mean include_shared_vgs */
	cmd->include_shared_vgs = 0;

	return 1;
}

/*
 * The global lock protects:
 *
 * - The global VG namespace.  Two VGs cannot have the same name.
 *   Used by any command that creates or removes a VG name,
 *   e.g. vgcreate, vgremove, vgrename, vgsplit, vgmerge.
 *
 * - The set of orphan PVs.
 *   Used by any command that changes a non-PV device into an orphan PV,
 *   an orphan PV into a device, a non-orphan PV (in a VG) into an orphan PV
 *   (not in a VG), or an orphan PV into a non-orphan PV,
 *   e.g. pvcreate, pvremove, vgcreate, vgremove, vgextend, vgreduce.
 *
 * - The properties of orphan PVs.  It is possible to make changes to the
 *   properties of an orphan PV, e.g. pvresize, pvchange.
 *
 * These are things that cannot be protected by a VG lock alone, since
 * orphan PVs do not belong to a real VG (an artificial VG does not
 * apply since a sanlock lock only exists on real storage.)
 *
 * If a command will change any of the things above, it must first acquire
 * the global lock in exclusive mode.
 *
 * If command is reading any of the things above, it must acquire the global
 * lock in shared mode.  A number of commands read the things above, including:
 *
 * - Reporting/display commands which show all VGs.  Any command that
 *   will iterate through the entire VG namespace must first acquire the
 *   global lock shared so that it has an accurate view of the namespace.
 *
 * - A command where a tag name is used to identify what to process.
 *   A tag requires reading all VGs to check if they match the tag.
 *
 * In these cases, the global lock must be acquired before the list of
 * all VGs is created.
 *
 * The global lock is not generally unlocked explicitly in the code.
 * When the command disconnects from lvmlockd, lvmlockd automatically
 * releases the locks held by the command.  The exception is if a command
 * will continue running for a long time while not needing the global lock,
 * e.g. commands that poll to report progress.
 *
 * There are two cases where the global lock can be taken in shared mode,
 * and then later converted to ex.  pvchange and pvresize use process_each_pv
 * which does lockd_gl("sh") to get the list of VGs.  Later, in the "_single"
 * function called within process_each_pv, the PV may be an orphan, in which
 * case the ex global lock is needed, so it's converted to ex at that point.
 *
 * Effects of misconfiguring use_lvmlockd.
 *
 * - Setting use_lvmlockd=1 tells lvm commands to use the global lock.
 * This should not be set unless a lock manager and lockd VGs will
 * be used.  Setting use_lvmlockd=1 without setting up a lock manager
 * or using lockd VGs will cause lvm commands to fail when they attempt
 * to change any global state (requiring the ex global lock), and will
 * cause warnings when the commands read global state (requiring the sh
 * global lock).  In this condition, lvm is nominally useful, and existing
 * local VGs can continue to be used mostly as usual.  But, the
 * warnings/errors should lead a user to either set up a lock manager
 * and lockd VGs, or set use_lvmlockd to 0.
 *
 * - Setting use_lvmlockd=0 tells lvm commands to not use the global lock.
 * If use_lvmlockd=0 when lockd VGs exist which require lvmlockd, the
 * lockd_gl() calls become no-ops, but the lockd_vg() calls for the lockd
 * VGs will fail.  The warnings/errors from accessing the lockd VGs
 * should lead the user to set use_lvmlockd to 1 and run the necessary
 * lock manager.  In this condition, lvm reverts to the behavior of
 * the following case, in which system ID largely protects shared
 * devices, but has limitations.
 *
 * - Setting use_lvmlockd=0 with shared devices, no lockd VGs and
 * no lock manager is a recognized mode of operation that is
 * described in the lvmsystemid man page.  Using lvm on shared
 * devices this way is made safe by using system IDs to assign
 * ownership of VGs to single hosts.  The main limitation of this
 * mode (among others outlined in the man page), is that orphan PVs
 * are unprotected.
 */

int lockd_gl(struct cmd_context *cmd, const char *def_mode, uint32_t flags)
{
	const char *mode = NULL;
	const char *opts = NULL;
	uint32_t lockd_flags;
	int retries = 0;
	int result;

	if (!_use_lvmlockd)
		return 1;

	/*
	 * Verify that when --readonly is used, no ex locks should be used.
	 */
	if (cmd->metadata_read_only && def_mode && !strcmp(def_mode, "ex")) {
		log_error("Exclusive locks are not allowed with readonly option.");
		return 0;
	}

	if (cmd->lockd_gl_disable)
		return 1;

	if (def_mode && !strcmp(def_mode, "un")) {
		mode = "un";
		goto req;
	}

	if (!mode)
		mode = def_mode;
	if (!mode) {
		log_error("Unknown lock-gl mode");
		return 0;
	}

 req:
	log_debug("lockd global mode %s", mode);

	if (!_lockd_request(cmd, "lock_gl",
			    NULL, NULL, NULL, NULL, NULL, NULL, mode, opts,
			    &result, &lockd_flags)) {
		/* No result from lvmlockd, it is probably not running. */

		/* We don't care if an unlock fails. */
		if (!strcmp(mode, "un"))
			return 1;

		/* We can continue reading if a shared lock fails. */
		if (!strcmp(mode, "sh")) {
			log_warn("Reading without shared global lock.");
			goto allow;
		}

		log_error("Global lock failed: check that lvmlockd is running.");
		return 0;
	}

	if (result == -EAGAIN) {
		if (retries < find_config_tree_int(cmd, global_lvmlockd_lock_retries_CFG, NULL)) {
			log_warn("Retrying %s global lock", mode);
			sleep(1);
			retries++;
			goto req;
		}
	}

	if (result == -EALREADY) {
		/*
		 * This should generally not happen because commands should be coded
		 * to avoid reacquiring the global lock.  If there is a case that's
		 * missed which causes the command to request the gl when it's already
		 * held, it's not a problem, so let it go.
		 */
		log_debug("lockd global mode %s already held.", mode);
		return 1;
	}

	if (!strcmp(mode, "un"))
		return 1;

	/*
	 * ENOLS: no lockspace was found with a global lock.
	 * The VG with the global lock may not be visible or started yet,
	 * this should be a temporary condition.
	 *
	 * ESTARTING: the lockspace with the gl is starting.
	 * The VG with the global lock is starting and should finish shortly.
	 *
	 * ELOCKIO: sanlock gets i/o errors when trying to read/write leases
	 * (This can progress to EVGKILLED.)
	 *
	 * EVGKILLED: the sanlock lockspace is being killed after losing
	 * access to lease storage.
	 */

	if (result == -ENOLS && (lockd_flags & LD_RF_NO_LM))
		log_error("Start a lock manager, lvmlockd did not find one running.");

	if (result == -ENOLS ||
	    result == -ESTARTING ||
	    result == -EVGKILLED ||
	    result == -ELOCKIO) {
		/*
		 * If an ex global lock fails, then the command fails.
		 */
		if (strcmp(mode, "sh")) {
			if (result == -ESTARTING)
				log_error("Global lock failed: lockspace is starting");
			else if (result == -ENOLS)
				log_error("Global lock failed: check that global lockspace is started");
			else if (result == -ELOCKIO)
				log_error("Global lock failed: storage errors for sanlock leases");
			else if (result == -EVGKILLED)
				log_error("Global lock failed: storage failed for sanlock leases");
			else
				log_error("Global lock failed: error %d", result);

			return 0;
		}

		/*
		 * If a sh global lock fails, then the command can continue
		 * reading without it, but force a global cache validation,
		 * and print a warning.
		 */

		if (result == -ESTARTING) {
			log_warn("Skipping global lock: lockspace is starting");
			goto allow;
		}

		if (result == -ELOCKIO || result == -EVGKILLED) {
			log_warn("Skipping global lock: storage %s for sanlock leases",
				  result == -ELOCKIO ? "errors" : "failed");
			goto allow;
		}

		if ((lockd_flags & LD_RF_NO_GL_LS) && (lockd_flags & LD_RF_WARN_GL_REMOVED)) {
			log_warn("Skipping global lock: VG with global lock was removed");
			goto allow;
		}

		if ((lockd_flags & LD_RF_NO_GL_LS) || (lockd_flags & LD_RF_NO_LOCKSPACES)) {
			log_warn("Skipping global lock: lockspace not found or started");
			goto allow;
		}

		/*
		 * This is for completeness.  If we reach here, then
		 * a specific check for the error should be added above
		 * with a more helpful message.
		 */
		log_error("Global lock failed: error %d", result);
		return 0;
	}

	if ((lockd_flags & LD_RF_DUP_GL_LS) && strcmp(mode, "un"))
		log_warn("Duplicate sanlock global locks should be corrected");

	if (result < 0) {
		if (result == -EAGAIN) {
			/*
			 * Most of the time, retries should avoid this case.
			 */
			log_error("Global lock failed: held by other host.");
			return 0;
		} else {
			/*
			 * We don't intend to reach this.  We should check
			 * any known/possible error specifically and print
			 * a more helpful message.  This is for completeness.
			 */
			log_error("Global lock failed: error %d.", result);
			return 0;
		}
	}

 allow:
	return 1;
}

/*
 * VG lock
 *
 * Return 1: continue, lockd_state may still indicate an error
 * Return 0: failure, do not continue
 *
 * lvmlockd could also return the lock_type that it used for the VG,
 * and we could encode that in lockd_state, and verify later that it
 * matches vg->lock_type.
 *
 * The result of the VG lock operation needs to be saved in lockd_state
 * because the result needs to be passed into vg_read so it can be
 * assessed in combination with vg->lock_type.
 *
 * The VG lock protects the VG metadata on disk from concurrent access
 * among hosts.
 *
 * The VG lock must be acquired before the VG is read, i.e. before vg_read().
 * The result from lockd_vg() is saved in the "lockd_state" variable, and
 * this result is passed into vg_read().  After vg_read() reads the VG,
 * it checks if the VG lock_type (sanlock or dlm) requires a lock to be
 * held, and if so, it verifies that the lock was correctly acquired by
 * looking at lockd_state.  If vg_read() sees that the VG is a local VG,
 * i.e. lock_type is not sanlock or dlm, then no lock is required, and it
 * ignores lockd_state (which would indicate no lock was found.)
 */

int lockd_vg(struct cmd_context *cmd, const char *vg_name, const char *def_mode,
	     uint32_t flags, uint32_t *lockd_state)
{
	const char *mode = NULL;
	uint32_t lockd_flags;
	uint32_t prev_state = *lockd_state;
	int retries = 0;
	int result;
	int ret;

	/*
	 * The result of the VG lock request is saved in lockd_state to be
	 * passed into vg_read where the lock result is needed once we
	 * know if this is a local VG or lockd VG.
	 */
	*lockd_state = 0;

	if (!is_real_vg(vg_name))
		return 1;

	/*
	 * Verify that when --readonly is used, no ex locks should be used.
	 */
	if (cmd->metadata_read_only &&
	    ((def_mode && !strcmp(def_mode, "ex")) ||
	     (!def_mode && !cmd->lockd_vg_default_sh))) {
		log_error("Exclusive locks are not allowed with readonly option.");
		return 0;
	}

	/*
	 * Some special cases need to disable the vg lock.
	 */
	if (cmd->lockd_vg_disable)
		return 1;

	/*
	 * An unlock is simply sent or skipped without any need
	 * for the mode checking for sh/ex.
	 *
	 * Look at lockd_state from the sh/ex lock, and if it failed,
	 * don't bother sending the unlock to lvmlockd.  The main
	 * purpose of this is to avoid sending an unnecessary unlock
	 * for local VGs (the lockd_state from sh/ex on the local VG
	 * will be failed.)  This implies that the lockd_state value
	 * should be preserved from the sh/ex lockd_vg() call and
	 * passed back to lockd_vg() for the corresponding unlock.
	 */
	if (def_mode && !strcmp(def_mode, "un")) {
		if (prev_state & LDST_FAIL)
			return 1;

		mode = "un";
		goto req;
	}

	/*
	 * The default mode may not have been provided in the
	 * function args.  This happens when lockd_vg is called
	 * from a process_each function that handles different
	 * commands.  Commands that only read/check/report/display
	 * the vg have LOCKD_VG_SH set in commands.h, which is
	 * copied to lockd_vg_default_sh.  Commands without this
	 * set modify the vg and need ex.
	 */
	if (!mode)
		mode = def_mode;
	if (!mode)
		mode = cmd->lockd_vg_default_sh ? "sh" : "ex";

	if (!strcmp(mode, "ex"))
		*lockd_state |= LDST_EX;

 req:
	/*
	 * This check is not at the top of the function so that
	 * we can first set LDST_EX which will be used later to
	 * decide whether a failure can be ignored or not.
	 *
	 * We do not know if this is a local VG or lockd VG yet,
	 * so we must return success, go ahead and read the VG,
	 * then check if the lock_type required lvmlockd or not.
	 */
	if (!_use_lvmlockd) {
		*lockd_state |= LDST_FAIL_REQUEST;
		return 1;
	}

	log_debug("lockd VG %s mode %s", vg_name, mode);

	if (!_lockd_request(cmd, "lock_vg",
			      vg_name, NULL, NULL, NULL, NULL, NULL, mode, NULL,
			      &result, &lockd_flags)) {
		/*
		 * No result from lvmlockd, it is probably not running.
		 * Decide if it is ok to continue without a lock in
		 * access_vg_lock_type() after the VG has been read and
		 * the lock_type can be checked.  We don't care about
		 * this error for local VGs, but we do care for lockd VGs.
		 */
		*lockd_state |= LDST_FAIL_REQUEST;
		return 1;
	}

	if (result == -EAGAIN) {
		if (retries < find_config_tree_int(cmd, global_lvmlockd_lock_retries_CFG, NULL)) {
			log_warn("Retrying %s lock on VG %s", mode, vg_name);
			sleep(1);
			retries++;
			goto req;
		}
	}

	switch (result) {
	case 0:
		/* success */
		break;
	case -ENOLS:
		*lockd_state |= LDST_FAIL_NOLS;
		break;
	case -ESTARTING:
		*lockd_state |= LDST_FAIL_STARTING;
		break;
	default:
		*lockd_state |= LDST_FAIL_OTHER;
	}

	/*
	 * Normal success.
	 */
	if (!result) {
		ret = 1;
		goto out;
	}

	/*
	 * The VG has been removed.  This will only happen with a dlm VG
	 * since a sanlock VG must be stopped everywhere before it's removed.
	 */
	if (result == -EREMOVED) {
		log_error("VG %s lock failed: removed", vg_name);
		ret = 1;
		goto out;
	}

	/*
	 * The lockspace for the VG is starting (the VG must not
	 * be local), and is not yet ready to do locking.  Allow
	 * reading without a sh lock during this period.
	 */
	if (result == -ESTARTING) {
		if (!strcmp(mode, "un")) {
			ret = 1;
			goto out;
		} else if (!strcmp(mode, "sh")) {
			log_warn("VG %s lock skipped: lock start in progress", vg_name);
			ret = 1;
			goto out;
		} else {
			log_error("VG %s lock failed: lock start in progress", vg_name);
			ret = 0;
			goto out;
		}
	}

	/*
	 * sanlock is getting i/o errors while reading/writing leases, or the
	 * lockspace/VG is being killed after failing to renew its lease for
	 * too long.
	 */
	if (result == -EVGKILLED || result == -ELOCKIO) {
		const char *problem = (result == -ELOCKIO ? "errors" : "failed");

		if (!strcmp(mode, "un")) {
			ret = 1;
			goto out;
		} else if (!strcmp(mode, "sh")) {
			log_warn("VG %s lock skipped: storage %s for sanlock leases", vg_name, problem);
			ret = 1;
			goto out;
		} else {
			log_error("VG %s lock failed: storage %s for sanlock leases", vg_name, problem);
			ret = 0;
			goto out;
		}
	}

	/*
	 * The lock is held by another host, and retries have been unsuccessful.
	 */
	if (result == -EAGAIN) {
		if (!strcmp(mode, "un")) {
			ret = 1;
			goto out;
		} else if (!strcmp(mode, "sh")) {
			log_warn("VG %s lock skipped: held by other host.", vg_name);
			ret = 1;
			goto out;
		} else {
			log_error("VG %s lock failed: held by other host.", vg_name);
			ret = 0;
			goto out;
		}
	}
	/*
	 * No lockspace for the VG was found.  It may be a local
	 * VG that lvmlockd doesn't keep track of, or it may be
	 * a lockd VG that lvmlockd doesn't yet know about (it hasn't
	 * been started yet.)  Decide what to do after the VG is
	 * read and we can see the lock_type.
	 */
	if (result == -ENOLS) {
		ret = 1;
		goto out;
	}

	/*
	 * Another error.  We don't intend to reach here, but
	 * want to check for each specific error above so that
	 * a helpful message can be printed.
	 */
	if (result) {
		if (!strcmp(mode, "un")) {
			ret = 1;
			goto out;
		} else if (!strcmp(mode, "sh")) {
			log_warn("VG %s lock skipped: error %d", vg_name, result);
			ret = 1;
			goto out;
		} else {
			log_error("VG %s lock failed: error %d", vg_name, result);
			ret = 0;
			goto out;
		}
	}

out:
	/*
	 * A notice from lvmlockd that duplicate gl locks have been found.
	 * It would be good for the user to disable one of them.
	 */
	if ((lockd_flags & LD_RF_DUP_GL_LS) && strcmp(mode, "un"))
		log_warn("Duplicate sanlock global lock in VG %s", vg_name);
 
	return ret;
}

/*
 * This must be called before a new version of the VG metadata is
 * written to disk.  For local VGs, this is a no-op, but for lockd
 * VGs, this notifies lvmlockd of the new VG seqno.  lvmlockd must
 * know the latest VG seqno so that it can save it within the lock's
 * LVB.  The VG seqno in the VG lock's LVB is used by other hosts to
 * detect when their cached copy of the VG metadata is stale, i.e.
 * the cached VG metadata has a lower seqno than the seqno seen in
 * the VG lock.
 */

int lockd_vg_update(struct volume_group *vg)
{
	daemon_reply reply;
	int result;
	int ret;

	if (!vg_is_shared(vg))
		return 1;
	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	reply = _lockd_send("vg_update",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"version = " FMTd64, (int64_t) vg->seqno,
				NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	daemon_reply_destroy(reply);
	return ret;
}

/*
 * When this is called directly (as opposed to being called from
 * lockd_lv), the caller knows that the LV has a lock.
 */

int lockd_lv_name(struct cmd_context *cmd, struct volume_group *vg,
		  const char *lv_name, struct id *lv_id,
		  const char *lock_args, const char *def_mode, uint32_t flags)
{
	char lv_uuid[64] __attribute__((aligned(8)));
	const char *mode = NULL;
	const char *opts = NULL;
	uint32_t lockd_flags;
	int refreshed = 0;
	int result;

	/*
	 * Verify that when --readonly is used, no LVs should be activated or used.
	 */
	if (cmd->metadata_read_only) {
		log_error("LV locks are not allowed with readonly option.");
		return 0;
	}

	if (cmd->lockd_lv_disable)
		return 1;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	if (!id_write_format(lv_id, lv_uuid, sizeof(lv_uuid)))
		return_0;

	/*
	 * For lvchange/vgchange activation, def_mode is "sh" or "ex"
	 * according to the specific -a{e,s}y mode designation.
	 * No e,s designation gives NULL def_mode.
	 */

	if (def_mode)
		mode = def_mode;

	if (mode && !strcmp(mode, "sh") && (flags & LDLV_MODE_NO_SH)) {
		struct logical_volume *lv = find_lv(vg, lv_name);
		log_error("Shared activation not compatible with LV type %s of %s/%s",
			  lv ? lvseg_name(first_seg(lv)) : "", vg->name, lv_name);
		return 0;
	}

	/*
	 * This is a hack for mirror LVs which need to know at a very low level
	 * which lock mode the LV is being activated with so that it can pick
	 * a mirror log type during activation.  Do not use this for anything
	 * else.
	 */
	if (mode && !strcmp(mode, "sh"))
		cmd->lockd_lv_sh = 1;

	if (!mode)
		mode = "ex";

	if (flags & LDLV_PERSISTENT)
		opts = "persistent";

 retry:
	log_debug("lockd LV %s/%s mode %s uuid %s", vg->name, lv_name, mode, lv_uuid);

	if (!_lockd_request(cmd, "lock_lv",
			       vg->name, vg->lock_type, vg->lock_args,
			       lv_name, lv_uuid, lock_args, mode, opts,
			       &result, &lockd_flags)) {
		/* No result from lvmlockd, it is probably not running. */
		log_error("Locking failed for LV %s/%s", vg->name, lv_name);
		return 0;
	}

	/* The lv was not active/locked. */
	if (result == -ENOENT && !strcmp(mode, "un"))
		return 1;

	if (result == -EALREADY)
		return 1;

	if (result == -EAGAIN) {
		log_error("LV locked by other host: %s/%s", vg->name, lv_name);
		return 0;
	}

	if (result == -EEXIST) {
		/*
		 * This happens if a command like lvchange tries to modify the
		 * LV with an ex LV lock when the LV is already active with a
		 * sh LV lock.
		 */
		log_error("LV is already locked with incompatible mode: %s/%s", vg->name, lv_name);
		return 0;
	}

	if (result == -EMSGSIZE) {
		/* Another host probably extended lvmlock. */
		if (!refreshed++) {
			log_debug("Refresh lvmlock");
		       	_refresh_sanlock_lv(cmd, vg);
			goto retry;
		}
	}

	if (result == -ENOLS) {
		log_error("LV %s/%s lock failed: lockspace is inactive", vg->name, lv_name);
		return 0;
	}

	if (result == -EVGKILLED || result == -ELOCKIO) {
		const char *problem = (result == -ELOCKIO ? "errors" : "failed");
		log_error("LV %s/%s lock failed: storage %s for sanlock leases", vg->name, lv_name, problem);
		return 0;
	}

	if (result < 0) {
		log_error("LV %s/%s lock failed: error %d", vg->name, lv_name, result);
		return 0;
	}

	return 1;
}

/*
 * Direct the lock request to the pool LV.
 * For a thin pool and all its thin volumes, one ex lock is used.
 * It is the one specified in metadata of the pool data lv.
 */

static int _lockd_lv_thin(struct cmd_context *cmd, struct logical_volume *lv,
			  const char *def_mode, uint32_t flags)
{
	struct logical_volume *pool_lv = NULL;

	if (lv_is_thin_volume(lv)) {
		struct lv_segment *pool_seg = first_seg(lv);
		pool_lv = pool_seg ? pool_seg->pool_lv : NULL;

	} else if (lv_is_thin_pool(lv)) {
		pool_lv = lv;

	} else if (lv_is_thin_pool_data(lv)) {
		/* FIXME: there should be a function to get pool lv from data lv. */
		pool_lv = lv_parent(lv);

	} else if (lv_is_thin_pool_metadata(lv)) {
		struct lv_segment *pool_seg = get_only_segment_using_this_lv(lv);
		if (pool_seg)
			pool_lv = pool_seg->lv;

	} else {
		/* This should not happen AFAIK. */
		log_error("Lock on incorrect thin lv type %s/%s",
			  lv->vg->name, lv->name);
		return 0;
	}

	if (!pool_lv) {
		/* This should not happen. */
		log_error("Cannot find thin pool for %s/%s",
			  lv->vg->name, lv->name);
		return 0;
	}

	/*
	 * Locking a locked lv (pool in this case) is a no-op.
	 * Unlock when the pool is no longer active.
	 */

	if (def_mode && !strcmp(def_mode, "un") && pool_is_active(pool_lv))
		return 1;

	flags |= LDLV_MODE_NO_SH;

	return lockd_lv_name(cmd, pool_lv->vg, pool_lv->name, &pool_lv->lvid.id[1],
			     pool_lv->lock_args, def_mode, flags);
}

/*
 * Only the combination of dlm + corosync + cmirrord allows
 * mirror LVs to be activated in shared mode on multiple nodes.
 */
static int _lockd_lv_mirror(struct cmd_context *cmd, struct logical_volume *lv,
			    const char *def_mode, uint32_t flags)
{
	if (!strcmp(lv->vg->lock_type, "sanlock"))
		flags |= LDLV_MODE_NO_SH;

	else if (!strcmp(lv->vg->lock_type, "dlm") && def_mode && !strcmp(def_mode, "sh")) {
#ifdef CMIRRORD_PIDFILE
		if (!cmirrord_is_running()) {
			log_error("cmirrord must be running to activate an LV in shared mode.");
			return 0;
		}
#else
		flags |= LDLV_MODE_NO_SH;
#endif
	}

	return lockd_lv_name(cmd, lv->vg, lv->name, &lv->lvid.id[1],
			     lv->lock_args, def_mode, flags);
}

/*
 * If the VG has no lock_type, then this function can return immediately.
 * The LV itself may have no lock (NULL lv->lock_args), but the lock request
 * may be directed to another lock, e.g. the pool LV lock in _lockd_lv_thin.
 * If the lock request is not directed to another LV, and the LV has no
 * lock_type set, it means that the LV has no lock, and no locking is done
 * for it.
 *
 * An LV lock is acquired before the LV is activated, and released
 * after the LV is deactivated.  If the LV lock cannot be acquired,
 * it means that the LV is active on another host and the activation
 * fails.  Commands that modify an inactive LV also acquire the LV lock.
 *
 * In non-lockd VGs, this is a no-op.
 *
 * In lockd VGs, normal LVs each have their own lock, but other
 * LVs do not have their own lock, e.g. the lock for a thin LV is
 * acquired on the thin pool LV, and a thin LV does not have a lock
 * of its own.  A cache pool LV does not have a lock of its own.
 * When the cache pool LV is linked to an origin LV, the lock of
 * the orgin LV protects the combined origin + cache pool.
 */

int lockd_lv(struct cmd_context *cmd, struct logical_volume *lv,
	     const char *def_mode, uint32_t flags)
{
	if (!vg_is_shared(lv->vg))
		return 1;

	if (!_use_lvmlockd) {
		log_error("LV in VG %s with lock_type %s requires lvmlockd.",
			  lv->vg->name, lv->vg->lock_type);
		return 0;
	}

	if (!_lvmlockd_connected)
		return 0;

	if (lv_is_thin_type(lv))
		return _lockd_lv_thin(cmd, lv, def_mode, flags);

	/*
	 * An LV with NULL lock_args does not have a lock of its own.
	 */
	if (!lv->lock_args)
		return 1;

	/*
	 * LV type cannot be active concurrently on multiple hosts,
	 * so shared mode activation is not allowed.
	 */
	if (lv_is_external_origin(lv) ||
	    lv_is_thin_type(lv) ||
	    lv_is_raid_type(lv) ||
	    lv_is_cache_type(lv)) {
		flags |= LDLV_MODE_NO_SH;
	}

	if (lv_is_mirror_type(lv))
		return _lockd_lv_mirror(cmd, lv, def_mode, flags);
	       
	return lockd_lv_name(cmd, lv->vg, lv->name, &lv->lvid.id[1],
			     lv->lock_args, def_mode, flags);
}

static int _init_lv_sanlock(struct cmd_context *cmd, struct volume_group *vg,
			    const char *lv_name, struct id *lv_id,
			    const char **lock_args_ret)
{
	char lv_uuid[64] __attribute__((aligned(8)));
	daemon_reply reply;
	const char *reply_str;
	const char *lv_lock_args = NULL;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	if (!id_write_format(lv_id, lv_uuid, sizeof(lv_uuid)))
		return_0;

	reply = _lockd_send("init_lv",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"lv_name = %s", lv_name,
				"lv_uuid = %s", lv_uuid,
				"vg_lock_type = %s", "sanlock",
				"vg_lock_args = %s", vg->lock_args,
				NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	if (result == -EEXIST) {
		log_error("Lock already exists for LV %s/%s", vg->name, lv_name);
		goto out;
	}

	if (result == -EMSGSIZE) {
		/*
		 * No space on the lvmlock lv for a new lease, this should be
		 * detected by handle_sanlock_lv() called before.
		 */
		log_error("No sanlock space for lock for LV %s/%s", vg->name, lv_name);
		goto out;
	}

	if (!ret) {
		log_error("_init_lv_sanlock lvmlockd result %d", result);
		goto out;
	}

	if (!(reply_str = daemon_reply_str(reply, "lv_lock_args", NULL))) {
		log_error("lv_lock_args not returned");
		ret = 0;
		goto out;
	}

	if (!(lv_lock_args = dm_pool_strdup(cmd->mem, reply_str))) {
		log_error("lv_lock_args allocation failed");
		ret = 0;
	}
out:
	daemon_reply_destroy(reply);

	*lock_args_ret = lv_lock_args;
	return ret;
}

static int _free_lv(struct cmd_context *cmd, struct volume_group *vg,
		    const char *lv_name, struct id *lv_id, const char *lock_args)
{
	char lv_uuid[64] __attribute__((aligned(8)));
	daemon_reply reply;
	int result;
	int ret;

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	if (!id_write_format(lv_id, lv_uuid, sizeof(lv_uuid)))
		return_0;

	reply = _lockd_send("free_lv",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"lv_name = %s", lv_name,
				"lv_uuid = %s", lv_uuid,
				"vg_lock_type = %s", vg->lock_type,
				"vg_lock_args = %s", vg->lock_args,
				"lv_lock_args = %s", lock_args ?: "none",
				NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	if (!ret)
		log_error("_free_lv lvmlockd result %d", result);

	daemon_reply_destroy(reply);

	return ret;
}

int lockd_init_lv_args(struct cmd_context *cmd, struct volume_group *vg,
		       struct logical_volume *lv,
		       const char *lock_type, const char **lock_args)
{
	/* sanlock is the only lock type that sets per-LV lock_args. */
	if (!strcmp(lock_type, "sanlock"))
		return _init_lv_sanlock(cmd, vg, lv->name, &lv->lvid.id[1], lock_args);
	return 1;
}

/*
 * lvcreate
 *
 * An LV created in a lockd VG inherits the lock_type of the VG.  In some
 * cases, e.g. thin LVs, this function may decide that the LV should not be
 * given a lock, in which case it sets lp lock_args to NULL, which will cause
 * the LV to not have lock_args set in its metadata.  A lockd_lv() request on
 * an LV with no lock_args will do nothing (unless the LV type causes the lock
 * request to be directed to another LV with a lock, e.g. to the thin pool LV
 * for thin LVs.)
 */

int lockd_init_lv(struct cmd_context *cmd, struct volume_group *vg, struct logical_volume *lv,
		  struct lvcreate_params *lp)
{
	int lock_type_num = get_lock_type_from_string(vg->lock_type);

	switch (lock_type_num) {
	case LOCK_TYPE_NONE:
	case LOCK_TYPE_CLVM:
		return 1;
	case LOCK_TYPE_SANLOCK:
	case LOCK_TYPE_DLM:
		break;
	default:
		log_error("lockd_init_lv: unknown lock_type.");
		return 0;
	}

	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	if (!lp->needs_lockd_init) {
		/* needs_lock_init is set for LVs that need a lockd lock. */
		return 1;

	} else if (seg_is_cache_pool(lp)) {
		/*
		 * A cache pool does not use a lockd lock because it cannot be
		 * used by itself.  When a cache pool is attached to an actual
		 * LV, the lockd lock for that LV covers the LV and the cache
		 * pool attached to it.
		 */
		lv->lock_args = NULL;
		return 1;

	} else if (!seg_is_thin_volume(lp) && lp->snapshot) {
		struct logical_volume *origin_lv;

		/*
		 * COW snapshots are associated with their origin LV,
		 * and only the origin LV needs its own lock, which
		 * represents itself and all associated cow snapshots.
		 */

		if (!lp->origin_name) {
			/* Sparse LV case. We require a lock from the origin LV. */
			log_error("Cannot create snapshot without origin LV in shared VG.");
			return 0;
		}

		if (!(origin_lv = find_lv(vg, lp->origin_name))) {
			log_error("Failed to find origin LV %s/%s", vg->name, lp->origin_name);
			return 0;
		}
		if (!lockd_lv(cmd, origin_lv, "ex", LDLV_PERSISTENT)) {
			log_error("Failed to lock origin LV %s/%s", vg->name, lp->origin_name);
			return 0;
		}
		lv->lock_args = NULL;
		return 1;

	} else if (seg_is_thin(lp)) {
		if ((seg_is_thin_volume(lp) && !lp->create_pool) ||
		    (!seg_is_thin_volume(lp) && lp->snapshot)) {
			struct lv_list *lvl;

			/*
			 * Creating a new thin lv or snapshot.  These lvs do not get
			 * their own lock but use the pool lock.  If an lv does not
			 * use its own lock, its lock_args is set to NULL.
			 */

			if (!(lvl = find_lv_in_vg(vg, lp->pool_name))) {
				log_error("Failed to find thin pool %s/%s", vg->name, lp->pool_name);
				return 0;
			}
			if (!lockd_lv(cmd, lvl->lv, "ex", LDLV_PERSISTENT)) {
				log_error("Failed to lock thin pool %s/%s", vg->name, lp->pool_name);
				return 0;
			}
			lv->lock_args = NULL;
			return 1;

		} else if (seg_is_thin_volume(lp) && lp->create_pool) {
			/*
			 * Creating a thin pool and a thin lv in it.  We could
			 * probably make this work.
			 *
			 * This should not happen because the command defs are
			 * checked and excluded for shared VGs early in lvcreate.
			 */
			log_error("Create thin pool and thin LV separately with lock type %s",
				  vg->lock_type);
			return 0;

		} else if (!seg_is_thin_volume(lp) && lp->create_pool) {
			/* Creating a thin pool only. */
			/* lv_name_lock = lp->pool_name; */

		} else {
			log_error("Unknown thin options for lock init.");
			return 0;
		}

	} else {
		/* Creating a normal lv. */
		/* lv_name_lock = lv_name; */
	}

	/*
	 * The LV gets its own lock, so set lock_args to non-NULL.
	 *
	 * lockd_init_lv_args() will be called during vg_write()
	 * to complete the sanlock LV lock initialization, where
	 * actual space on disk is allocated.  Waiting to do this
	 * last step until vg_write() avoids the need to revert
	 * the sanlock allocation if the lvcreate function isn't
	 * completed.
	 *
	 * This works, but would leave the sanlock lease allocated
	 * unless the lease was freed on each early exit path from
	 * lvcreate:
	 *
	 * return lockd_init_lv_args(cmd, vg, lv_name_lock, lv_id,
	 * 			     vg->lock_type, &lv->lock_args);
	 */

	if (!strcmp(vg->lock_type, "sanlock"))
		lv->lock_args = "pending";
	else if (!strcmp(vg->lock_type, "dlm"))
		lv->lock_args = "dlm";

	return 1;
}

/* lvremove */

int lockd_free_lv(struct cmd_context *cmd, struct volume_group *vg,
		  const char *lv_name, struct id *lv_id, const char *lock_args)
{
	switch (get_lock_type_from_string(vg->lock_type)) {
	case LOCK_TYPE_NONE:
	case LOCK_TYPE_CLVM:
		return 1;
	case LOCK_TYPE_DLM:
	case LOCK_TYPE_SANLOCK:
		if (!lock_args)
			return 1;
		return _free_lv(cmd, vg, lv_name, lv_id, lock_args);
	default:
		log_error("lockd_free_lv: unknown lock_type.");
		return 0;
	}
}

int lockd_rename_vg_before(struct cmd_context *cmd, struct volume_group *vg)
{
	daemon_reply reply;
	int result;
	int ret;

	if (!vg_is_shared(vg))
		return 1;
	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	if (lvs_in_vg_activated(vg)) {
		log_error("LVs must be inactive before vgrename.");
		return 0;
	}

	/* Check that no LVs are active on other hosts. */
	if (!_lockd_all_lvs(cmd, vg)) {
		log_error("Cannot rename VG %s with active LVs", vg->name);
		return 0;
	}

	/*
	 * lvmlockd:
	 * checks for other hosts in lockspace
	 * leaves the lockspace
	 */

	reply = _lockd_send("rename_vg_before",
			"pid = " FMTd64, (int64_t) getpid(),
			"vg_name = %s", vg->name,
			"vg_lock_type = %s", vg->lock_type,
			"vg_lock_args = %s", vg->lock_args,
			NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		ret = 0;
	} else {
		ret = (result < 0) ? 0 : 1;
	}

	daemon_reply_destroy(reply);

	/* Other hosts have not stopped the lockspace. */
	if (result == -EBUSY) {
		log_error("Lockspace for \"%s\" not stopped on other hosts", vg->name);
		return 0;
	}
	
	if (!ret) {
		log_error("lockd_rename_vg_before lvmlockd result %d", result);
		return 0;
	}

	if (!strcmp(vg->lock_type, "sanlock")) {
		log_debug("lockd_rename_vg_before deactivate sanlock lv");
		_deactivate_sanlock_lv(cmd, vg);
	}

	return 1;
}

int lockd_rename_vg_final(struct cmd_context *cmd, struct volume_group *vg, int success)
{
	daemon_reply reply;
	int result;
	int ret;

	if (!vg_is_shared(vg))
		return 1;
	if (!_use_lvmlockd)
		return 0;
	if (!_lvmlockd_connected)
		return 0;

	if (!success) {
		/*
		 * Depending on the problem that caused the rename to
		 * fail, it may make sense to not restart the VG here.
		 */
		if (!lockd_start_vg(cmd, vg, 0))
			log_error("Failed to restart VG %s lockspace.", vg->name);
		return 1;
	}

	if (!strcmp(vg->lock_type, "sanlock")) {
		if (!_activate_sanlock_lv(cmd, vg))
			return 0;

		/*
		 * lvmlockd needs to rewrite the leases on disk
		 * with the new VG (lockspace) name.
		 */
		reply = _lockd_send("rename_vg_final",
				"pid = " FMTd64, (int64_t) getpid(),
				"vg_name = %s", vg->name,
				"vg_lock_type = %s", vg->lock_type,
				"vg_lock_args = %s", vg->lock_args,
				NULL);

		if (!_lockd_result(reply, &result, NULL)) {
			ret = 0;
		} else {
			ret = (result < 0) ? 0 : 1;
		}
	
		daemon_reply_destroy(reply);

		if (!ret) {
			/*
			 * The VG has been renamed on disk, but renaming the
			 * sanlock leases failed.  Cleaning this up can
			 * probably be done by converting the VG to lock_type
			 * none, then converting back to sanlock.
			 */
			log_error("lockd_rename_vg_final lvmlockd result %d", result);
			return 0;
		}
	}

	if (!lockd_start_vg(cmd, vg, 1))
		log_error("Failed to start VG %s lockspace.", vg->name);

	return 1;
}

const char *lockd_running_lock_type(struct cmd_context *cmd, int *found_multiple)
{
	daemon_reply reply;
	const char *lock_type = NULL;
	int result;

	if (!_use_lvmlockd)
		return NULL;
	if (!_lvmlockd_connected)
		return NULL;

	reply = _lockd_send("running_lm",
			"pid = " FMTd64, (int64_t) getpid(),
			NULL);

	if (!_lockd_result(reply, &result, NULL)) {
		log_error("Failed to get result from lvmlockd");
		goto out;
	}

	switch (result) {
	case -EXFULL:
		*found_multiple = 1;
		break;
	case -ENOLCK:
		break;
	case LOCK_TYPE_SANLOCK:
		log_debug("lvmlockd found sanlock");
		lock_type = "sanlock";
		break;
	case LOCK_TYPE_DLM:
		log_debug("lvmlockd found dlm");
		lock_type = "dlm";
		break;
	default:
		log_error("Failed to find a running lock manager.");
		break;
	}
out:
	daemon_reply_destroy(reply);

	return lock_type;
}

/* Some LV types have no lock. */

int lockd_lv_uses_lock(struct logical_volume *lv)
{
	if (lv_is_thin_volume(lv))
		return 0;

	if (lv_is_thin_pool_data(lv))
		return 0;

	if (lv_is_thin_pool_metadata(lv))
		return 0;

	if (lv_is_pool_metadata_spare(lv))
		return 0;

	if (lv_is_cache_pool(lv))
		return 0;

	if (lv_is_cache_pool_data(lv))
		return 0;

	if (lv_is_cache_pool_metadata(lv))
		return 0;

	if (lv_is_cow(lv))
		return 0;

	if (lv_is_snapshot(lv))
		return 0;

	/* FIXME: lv_is_virtual_origin ? */

	if (lv_is_lockd_sanlock_lv(lv))
		return 0;

	if (lv_is_mirror_image(lv))
		return 0;

	if (lv_is_mirror_log(lv))
		return 0;

	if (lv_is_raid_image(lv))
		return 0;

	if (lv_is_raid_metadata(lv))
		return 0;

	if (!lv_is_visible(lv))
		return 0;

	return 1;
}
