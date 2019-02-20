/*
 * Copyright (C) 2014-2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "lvmpolld-common.h"

#include "libdaemon/client/config-util.h"

#include <fcntl.h>
#include <signal.h>

static const char LVM_SYSTEM_DIR[] = "LVM_SYSTEM_DIR=";

static char *_construct_full_lvname(const char *vgname, const char *lvname)
{
	char *name;
	size_t l;

	l = strlen(vgname) + strlen(lvname) + 2; /* vg/lv and \0 */
	name = (char *) malloc(l * sizeof(char));
	if (!name)
		return NULL;

	if (dm_snprintf(name, l, "%s/%s", vgname, lvname) < 0) {
		free(name);
		name = NULL;
	}

	return name;
}

static char *_construct_lvm_system_dir_env(const char *sysdir)
{
	/*
	 *  Store either "LVM_SYSTEM_DIR=/path/to..."
	 *		    - or -
	 *  just single char to store NULL byte
	 */
	size_t l = sysdir ? strlen(sysdir) + 16 : 1;
	char *env = (char *) malloc(l * sizeof(char));

	if (!env)
		return NULL;

	*env = '\0';

	if (sysdir && dm_snprintf(env, l, "%s%s", LVM_SYSTEM_DIR, sysdir) < 0) {
		free(env);
		env = NULL;
	}

	return env;
}

static const char *_get_lvid(const char *lvmpolld_id, const char *sysdir)
{
	return lvmpolld_id ? (lvmpolld_id + (sysdir ? strlen(sysdir) : 0)) : NULL;
}

char *construct_id(const char *sysdir, const char *uuid)
{
	char *id;
	int r;
	size_t l;

	l = strlen(uuid) + (sysdir ? strlen(sysdir) : 0) + 1;
	id = (char *) malloc(l * sizeof(char));
	if (!id)
		return NULL;

	r = sysdir ? dm_snprintf(id, l, "%s%s", sysdir, uuid) :
		     dm_snprintf(id, l, "%s", uuid);

	if (r < 0) {
		free(id);
		id = NULL;
	}

	return id;
}

struct lvmpolld_lv *pdlv_create(struct lvmpolld_state *ls, const char *id,
			   const char *vgname, const char *lvname,
			   const char *sysdir, enum poll_type type,
			   const char *sinterval, unsigned pdtimeout,
			   struct lvmpolld_store *pdst)
{
	char *lvmpolld_id = strdup(id), /* copy */
	     *full_lvname = _construct_full_lvname(vgname, lvname), /* copy */
	     *lvm_system_dir_env = _construct_lvm_system_dir_env(sysdir); /* copy */

	struct lvmpolld_lv tmp = {
		.ls = ls,
		.type = type,
		.lvmpolld_id = lvmpolld_id,
		.lvid = _get_lvid(lvmpolld_id, sysdir),
		.lvname = full_lvname,
		.lvm_system_dir_env = lvm_system_dir_env,
		.sinterval = strdup(sinterval), /* copy */
		.pdtimeout = pdtimeout < MIN_POLLING_TIMEOUT ? MIN_POLLING_TIMEOUT : pdtimeout,
		.cmd_state = { .retcode = -1, .signal = 0 },
		.pdst = pdst,
		.init_rq_count = 1
	}, *pdlv = (struct lvmpolld_lv *) malloc(sizeof(struct lvmpolld_lv));

	if (!pdlv || !tmp.lvid || !tmp.lvname || !tmp.lvm_system_dir_env || !tmp.sinterval)
		goto err;

	memcpy(pdlv, &tmp, sizeof(*pdlv));

	if (pthread_mutex_init(&pdlv->lock, NULL))
		goto err;

	return pdlv;

err:
	free((void *)full_lvname);
	free((void *)lvmpolld_id);
	free((void *)lvm_system_dir_env);
	free((void *)tmp.sinterval);
	free((void *)pdlv);

	return NULL;
}

void pdlv_destroy(struct lvmpolld_lv *pdlv)
{
	free((void *)pdlv->lvmpolld_id);
	free((void *)pdlv->lvname);
	free((void *)pdlv->sinterval);
	free((void *)pdlv->lvm_system_dir_env);
	free((void *)pdlv->cmdargv);
	free((void *)pdlv->cmdenvp);

	pthread_mutex_destroy(&pdlv->lock);

	free((void *)pdlv);
}

unsigned pdlv_get_polling_finished(struct lvmpolld_lv *pdlv)
{
	unsigned ret;

	pdlv_lock(pdlv);
	ret = pdlv->polling_finished;
	pdlv_unlock(pdlv);

	return ret;
}

struct lvmpolld_lv_state pdlv_get_status(struct lvmpolld_lv *pdlv)
{
	struct lvmpolld_lv_state r;

	pdlv_lock(pdlv);
	r.error = pdlv_locked_error(pdlv);
	r.polling_finished = pdlv_locked_polling_finished(pdlv);
	r.cmd_state = pdlv_locked_cmd_state(pdlv);
	pdlv_unlock(pdlv);

	return r;
}

void pdlv_set_cmd_state(struct lvmpolld_lv *pdlv, const struct lvmpolld_cmd_stat *cmd_state)
{
	pdlv_lock(pdlv);
	pdlv->cmd_state = *cmd_state;
	pdlv_unlock(pdlv);
}

void pdlv_set_error(struct lvmpolld_lv *pdlv, unsigned error)
{
	pdlv_lock(pdlv);
	pdlv->error = error;
	pdlv_unlock(pdlv);
}

void pdlv_set_polling_finished(struct lvmpolld_lv *pdlv, unsigned finished)
{
	pdlv_lock(pdlv);
	pdlv->polling_finished = finished;
	pdlv_unlock(pdlv);
}

struct lvmpolld_store *pdst_init(const char *name)
{
	struct lvmpolld_store *pdst = (struct lvmpolld_store *) malloc(sizeof(struct lvmpolld_store));
	if (!pdst)
		return NULL;

	pdst->store = dm_hash_create(32);
	if (!pdst->store)
		goto err_hash;
	if (pthread_mutex_init(&pdst->lock, NULL))
		goto err_mutex;

	pdst->name = name;
	pdst->active_polling_count = 0;

	return pdst;

err_mutex:
	dm_hash_destroy(pdst->store);
err_hash:
	free(pdst);
	return NULL;
}

void pdst_destroy(struct lvmpolld_store *pdst)
{
	if (!pdst)
		return;

	dm_hash_destroy(pdst->store);
	pthread_mutex_destroy(&pdst->lock);
	free(pdst);
}

void pdst_locked_lock_all_pdlvs(const struct lvmpolld_store *pdst)
{
	struct dm_hash_node *n;

	dm_hash_iterate(n, pdst->store)
		pdlv_lock(dm_hash_get_data(pdst->store, n));
}

void pdst_locked_unlock_all_pdlvs(const struct lvmpolld_store *pdst)
{
	struct dm_hash_node *n;

	dm_hash_iterate(n, pdst->store)
		pdlv_unlock(dm_hash_get_data(pdst->store, n));
}

static void _pdlv_locked_dump(struct buffer *buff, const struct lvmpolld_lv *pdlv)
{
	char tmp[1024];
	const struct lvmpolld_cmd_stat *cmd_state = &pdlv->cmd_state;

	/* pdlv-section { */
	if (dm_snprintf(tmp, sizeof(tmp), "\t%s {\n", pdlv->lvmpolld_id) > 0)
		buffer_append(buff, tmp);

	if (dm_snprintf(tmp, sizeof(tmp), "\t\tlvid=\"%s\"\n", pdlv->lvid) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\ttype=\"%s\"\n", polling_op(pdlv->type)) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\tlvname=\"%s\"\n", pdlv->lvname) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\tlvmpolld_internal_timeout=%d\n", pdlv->pdtimeout) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\tlvm_command_interval=\"%s\"\n", pdlv->sinterval ?: "<undefined>") > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\t%s\"%s\"\n", LVM_SYSTEM_DIR,
			(*pdlv->lvm_system_dir_env ? (pdlv->lvm_system_dir_env + (sizeof(LVM_SYSTEM_DIR) - 1)) : "<undefined>")) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\tlvm_command_pid=%d\n", pdlv->cmd_pid) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\tpolling_finished=%d\n", pdlv->polling_finished) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\terror_occured=%d\n", pdlv->error) > 0)
		buffer_append(buff, tmp);
	if (dm_snprintf(tmp, sizeof(tmp), "\t\tinit_requests_count=%d\n", pdlv->init_rq_count) > 0)
		buffer_append(buff, tmp);

	/* lvm_commmand-section { */
	buffer_append(buff, "\t\tlvm_command {\n");
	if (cmd_state->retcode == -1 && !cmd_state->signal)
		buffer_append(buff, "\t\t\tstate=\"" LVMPD_RESP_IN_PROGRESS "\"\n");
	else {
		buffer_append(buff, "\t\t\tstate=\"" LVMPD_RESP_FINISHED "\"\n");
		if (dm_snprintf(tmp, sizeof(tmp), "\t\t\treason=\"%s\"\n\t\t\tvalue=%d\n",
				(cmd_state->signal ? LVMPD_REAS_SIGNAL : LVMPD_REAS_RETCODE),
				(cmd_state->signal ?: cmd_state->retcode)) > 0)
			buffer_append(buff, tmp);
	}
	buffer_append(buff, "\t\t}\n");
	/* } lvm_commmand-section */

	buffer_append(buff, "\t}\n");
	/* } pdlv-section */
}

void pdst_locked_dump(const struct lvmpolld_store *pdst, struct buffer *buff)
{
	struct dm_hash_node *n;

	dm_hash_iterate(n, pdst->store)
		_pdlv_locked_dump(buff, dm_hash_get_data(pdst->store, n));
}

void pdst_locked_send_cancel(const struct lvmpolld_store *pdst)
{
	struct lvmpolld_lv *pdlv;
	struct dm_hash_node *n;

	dm_hash_iterate(n, pdst->store) {
		pdlv = dm_hash_get_data(pdst->store, n);
		if (!pdlv_locked_polling_finished(pdlv))
			pthread_cancel(pdlv->tid);
	}
}

void pdst_locked_destroy_all_pdlvs(const struct lvmpolld_store *pdst)
{
	struct dm_hash_node *n;

	dm_hash_iterate(n, pdst->store)
		pdlv_destroy(dm_hash_get_data(pdst->store, n));
}

struct lvmpolld_thread_data *lvmpolld_thread_data_constructor(struct lvmpolld_lv *pdlv)
{
	struct lvmpolld_thread_data *data = (struct lvmpolld_thread_data *) malloc(sizeof(struct lvmpolld_thread_data));
	if (!data)
		return NULL;

	data->pdlv = NULL;
	data->line = NULL;
	data->line_size = 0;
	data->fout = data->ferr = NULL;
	data->outpipe[0] = data->outpipe[1] = data->errpipe[0] = data->errpipe[1] = -1;

	if (pipe(data->outpipe) || pipe(data->errpipe)) {
		lvmpolld_thread_data_destroy(data);
		return NULL;
	}

	if (fcntl(data->outpipe[0], F_SETFD, FD_CLOEXEC) ||
	    fcntl(data->outpipe[1], F_SETFD, FD_CLOEXEC) ||
	    fcntl(data->errpipe[0], F_SETFD, FD_CLOEXEC) ||
	    fcntl(data->errpipe[1], F_SETFD, FD_CLOEXEC)) {
		lvmpolld_thread_data_destroy(data);
		return NULL;
	}

	data->pdlv = pdlv;

	return data;
}

void lvmpolld_thread_data_destroy(void *thread_private)
{
	struct lvmpolld_thread_data *data = (struct lvmpolld_thread_data *) thread_private;
	if (!data)
		return;

	if (data->pdlv) {
		pdst_lock(data->pdlv->pdst);
		/*
		 * FIXME: skip this step if lvmpolld is activated
		 * 	  by systemd.
		 */
		if (!pdlv_get_polling_finished(data->pdlv))
			kill(data->pdlv->cmd_pid, SIGTERM);
		pdlv_set_polling_finished(data->pdlv, 1);
		pdst_locked_dec(data->pdlv->pdst);
		pdst_unlock(data->pdlv->pdst);
	}

	/* may get reallocated in getline(). free must not be used */
	free(data->line);

	if (data->fout && !fclose(data->fout))
		data->outpipe[0] = -1;

	if (data->ferr && !fclose(data->ferr))
		data->errpipe[0] = -1;

	if (data->outpipe[0] >= 0)
		(void) close(data->outpipe[0]);

	if (data->outpipe[1] >= 0)
		(void) close(data->outpipe[1]);

	if (data->errpipe[0] >= 0)
		(void) close(data->errpipe[0]);

	if (data->errpipe[1] >= 0)
		(void) close(data->errpipe[1]);

	free(data);
}
