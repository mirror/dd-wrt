// Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: (GPL-2.0)

#include <stdio.h>
#include <errno.h>

#include "state_machine.h"

static bool mrp_mrc_ring_open(struct mrp *mrp)
{
	if (mrp->ring_role == BR_MRP_RING_ROLE_MRM)
		return false;

	mrp_set_mrm_init(mrp);

	switch (mrp->mrc_state) {
	case MRP_MRC_STATE_DE_IDLE:
		mrp_set_mrm_state(mrp, MRP_MRM_STATE_PRM_UP);
		mrp_offload_set_ring_role(mrp, BR_MRP_RING_ROLE_MRM);
		break;
	case MRP_MRC_STATE_PT:
		mrp_set_mrm_state(mrp, MRP_MRM_STATE_CHK_RC);
		mrp_offload_set_ring_role(mrp, BR_MRP_RING_ROLE_MRM);
		break;
	case MRP_MRC_STATE_DE:
		mrp_set_mrm_state(mrp, MRP_MRM_STATE_PRM_UP);
		mrp_offload_set_ring_role(mrp, BR_MRP_RING_ROLE_MRM);
		break;
	case MRP_MRC_STATE_PT_IDLE:
		mrp_set_mrm_state(mrp, MRP_MRM_STATE_CHK_RO);
		mrp_offload_set_ring_role(mrp, BR_MRP_RING_ROLE_MRM);
	default:
		break;
	}

	mrp->test_monitor = false;
	mrp_ring_test_req(mrp, mrp->ring_test_conf_short);

	return true;
}

void mrp_ring_open(struct mrp *mrp)
{
	if (mrp->mra_support)
		if (mrp_mrc_ring_open(mrp))
			return;

	if (mrp->mrm_state != MRP_MRM_STATE_CHK_RC) {
		mrp->add_test = false;
		mrp_ring_test_req(mrp, mrp->ring_test_conf_interval);
		return;
	}

	mrp_port_offload_set_state(mrp->s_port, BR_MRP_PORT_STATE_FORWARDING);

	mrp->ring_test_curr_max = mrp->ring_test_conf_max - 1;
	mrp->ring_test_curr = 0;

	mrp->add_test = false;

	if (!mrp->no_tc)
		mrp_ring_topo_req(mrp, mrp->ring_topo_conf_interval);

	mrp_ring_test_req(mrp, mrp->ring_test_conf_interval);

	mrp->ring_transitions++;
	mrp_set_mrm_state(mrp, MRP_MRM_STATE_CHK_RO);
}

void mrp_in_open(struct mrp *mrp)
{
	mrp_port_offload_set_state(mrp->i_port, BR_MRP_PORT_STATE_FORWARDING);

	mrp->in_test_curr_max = mrp->in_test_conf_max - 1;
	mrp->in_test_curr = 0;

	mrp_in_topo_req(mrp, mrp->in_topo_conf_interval);
	mrp_in_test_req(mrp, mrp->in_test_conf_interval);

	mrp->in_transitions++;
	mrp_set_mim_state(mrp, MRP_MIM_STATE_CHK_IO);
}

static void mrp_clear_fdb_expired(struct ev_loop *loop,
				  ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, clear_fdb_work);

	mrp_offload_flush(mrp);

	mrp_clear_fdb_stop(mrp);
}

static void mrp_ring_test_expired(struct ev_loop *loop,
				  ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, ring_test_work);

	pthread_mutex_lock(&mrp->lock);

	if (mrp->mrm_state == MRP_MRM_STATE_AC_STAT1)
		goto out;

	mrp->add_test = false;

out:
	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_ring_watcher_expired(struct ev_loop *loop,
				     ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, ring_watcher_work);

	pthread_mutex_lock(&mrp->lock);

	mrp_offload_send_ring_test(mrp, mrp->ring_test_conf_interval,
				   mrp->ring_test_conf_max,
				   mrp->ring_test_conf_period);

	mrp->ring_watcher_work.repeat = (ev_tstamp)mrp->ring_test_conf_period/ 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->ring_watcher_work);

	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_ring_topo_expired(struct ev_loop *loop,
				  ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, ring_topo_work);

	printf("ring topo expired: ring_topo_curr_max: %d\n",
	       mrp->ring_topo_curr_max);

	pthread_mutex_lock(&mrp->lock);

	if (mrp->ring_topo_curr_max > 0) {
		mrp_ring_topo_send(mrp, mrp->ring_topo_curr_max *
					mrp->ring_topo_conf_interval);

		mrp->ring_topo_curr_max--;
	} else {
		mrp->ring_topo_curr_max = mrp->ring_topo_conf_max - 1;

		mrp_offload_flush(mrp);
		mrp_ring_topo_send(mrp, 0);

		mrp_ring_topo_stop(mrp);
	}

	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_ring_link_up_expired(struct ev_loop *loop,
				     ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, ring_link_up_work);
	uint32_t interval;
	uint32_t delay;

	printf("ring link up expired: ring_link_curr_max: %d\n",
	       mrp->ring_link_curr_max);

	pthread_mutex_lock(&mrp->lock);

	delay = mrp->ring_link_conf_interval;

	if (mrp->ring_link_curr_max > 0) {
		mrp->ring_link_curr_max--;

		mrp_ring_link_up_start(mrp, delay);

		interval = mrp->ring_link_curr_max * delay;

		mrp_ring_link_req(mrp->p_port, true, interval);
	} else {
		mrp->ring_link_curr_max = mrp->ring_link_conf_max;
		mrp_port_offload_set_state(mrp->s_port,
					   BR_MRP_PORT_STATE_FORWARDING);
		mrp_set_mrc_state(mrp, MRP_MRC_STATE_PT_IDLE);

		mrp_ring_link_up_stop(mrp);
	}

	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_ring_link_down_expired(struct ev_loop *loop,
				       ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, ring_link_down_work);
	uint32_t interval;
	uint32_t delay;

	printf("ring link down expired: ring_link_curr_max: %d\n",
	       mrp->ring_link_curr_max);

	pthread_mutex_lock(&mrp->lock);

	delay = mrp->ring_link_conf_interval;

	if (mrp->ring_link_curr_max > 0) {
		mrp->ring_link_curr_max--;

		mrp_ring_link_down_start(mrp, delay);

		interval = mrp->ring_link_curr_max * delay;

		mrp_ring_link_req(mrp->p_port, false, interval);
	} else {
		mrp->ring_link_curr_max = mrp->ring_link_conf_max;

		mrp_set_mrc_state(mrp, MRP_MRC_STATE_DE_IDLE);

		mrp_ring_link_down_stop(mrp);
	}

	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_in_test_expired(struct ev_loop *loop,
				ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, in_test_work);

	pthread_mutex_lock(&mrp->lock);

	if (mrp->mrm_state == MRP_MRM_STATE_AC_STAT1)
		goto out;

	mrp->add_test = false;

out:
	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_in_watcher_expired(struct ev_loop *loop,
				   ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, in_watcher_work);

	pthread_mutex_lock(&mrp->lock);

	mrp_offload_send_in_test(mrp, mrp->in_test_conf_interval,
				 mrp->in_test_conf_max,
				 mrp->in_test_conf_period);

	mrp->in_watcher_work.repeat = (ev_tstamp)mrp->in_test_conf_period/ 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->in_watcher_work);

	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_in_topo_expired(struct ev_loop *loop,
				ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, in_topo_work);

	printf("int topo expired: in_topo_curr_max: %d\n",
	       mrp->in_topo_curr_max);

	pthread_mutex_lock(&mrp->lock);

	if (mrp->in_topo_curr_max > 0) {
		mrp_in_topo_send(mrp, mrp->in_topo_curr_max *
				 mrp->in_topo_conf_interval);

		mrp->in_topo_curr_max--;
	} else {
		mrp->in_topo_curr_max = mrp->in_topo_conf_max - 1;

		mrp_offload_flush(mrp);
		mrp_in_topo_send(mrp, 0);

		mrp_in_topo_stop(mrp);
	}

	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_in_link_up_expired(struct ev_loop *loop,
				   ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, in_link_up_work);
	uint32_t interval;
	uint32_t delay;

	printf("int link up expired: in_link_curr_max: %d\n",
	       mrp->in_link_curr_max);

	pthread_mutex_lock(&mrp->lock);

	delay = mrp->in_link_conf_interval;

	if (mrp->in_link_curr_max > 0) {
		mrp->in_link_curr_max--;

		mrp_in_link_up_start(mrp, delay);

		interval = mrp->in_link_curr_max * delay;

		mrp_in_link_req(mrp, true, interval);
	} else {
		mrp->in_link_curr_max = mrp->in_link_conf_max;
		mrp_port_offload_set_state(mrp->i_port,
					   BR_MRP_PORT_STATE_FORWARDING);
		mrp_set_mic_state(mrp, MRP_MIC_STATE_IP_IDLE);

		mrp_in_link_up_stop(mrp);
	}

	pthread_mutex_unlock(&mrp->lock);
}

static void mrp_in_link_down_expired(struct ev_loop *loop,
				     ev_timer *w, int revents)
{
	struct mrp *mrp = container_of(w, struct mrp, in_link_down_work);
	uint32_t interval;
	uint32_t delay;

	printf("int link down expired: in_link_curr_max: %d\n",
	       mrp->in_link_curr_max);

	pthread_mutex_lock(&mrp->lock);

	delay = mrp->in_link_conf_interval;

	if (mrp->in_link_curr_max > 0) {
		mrp->in_link_curr_max--;

		mrp_in_link_down_start(mrp, delay);

		interval = mrp->in_link_curr_max * delay;

		mrp_in_link_req(mrp, false, interval);
	} else {
		mrp->in_link_curr_max = mrp->in_link_conf_max;

		mrp_in_link_down_stop(mrp);
	}

	pthread_mutex_unlock(&mrp->lock);
}

int mrp_ring_test_start(struct mrp *mrp, uint32_t interval)
{
	int err;

	if (interval == mrp->ring_test_hw_interval)
		goto update_only_sw;

	mrp->ring_watcher_work.repeat = (ev_tstamp)mrp->ring_test_conf_period/ 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->ring_watcher_work);

	mrp->ring_test_hw_interval = interval;
	err = mrp_offload_send_ring_test(mrp, interval,
					 mrp->ring_test_conf_max,
					 mrp->ring_test_conf_period);
	if (err)
		return err;

update_only_sw:
	mrp->ring_test_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->ring_test_work);
	return 0;
}

void mrp_ring_test_stop(struct mrp *mrp)
{
	mrp_offload_send_ring_test(mrp, 0, 0, 0);
	/* Make sure that at the next start the HW is updated */
	mrp->ring_test_hw_interval = -1;
	ev_timer_stop(EV_DEFAULT, &mrp->ring_test_work);
	ev_timer_stop(EV_DEFAULT, &mrp->ring_watcher_work);
}

void mrp_ring_topo_start(struct mrp *mrp, uint32_t interval)
{
	mrp->ring_topo_running = true;
	mrp->ring_topo_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->ring_topo_work);
}

void mrp_ring_topo_stop(struct mrp *mrp)
{
	mrp->ring_topo_running = false;
	ev_timer_stop(EV_DEFAULT, &mrp->ring_topo_work);
}

void mrp_ring_link_up_start(struct mrp *mrp, uint32_t interval)
{
	mrp->ring_link_up_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->ring_link_up_work);
}

void mrp_ring_link_up_stop(struct mrp *mrp)
{
	ev_timer_stop(EV_DEFAULT, &mrp->ring_link_up_work);
}

void mrp_ring_link_down_start(struct mrp *mrp, uint32_t interval)
{
	mrp->ring_link_down_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->ring_link_down_work);
}

void mrp_ring_link_down_stop(struct mrp *mrp)
{
	ev_timer_stop(EV_DEFAULT, &mrp->ring_link_down_work);
}

void mrp_clear_fdb_start(struct mrp *mrp, uint32_t interval)
{
	mrp->clear_fdb_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->clear_fdb_work);
	if (interval == 0)
		mrp_offload_flush(mrp);
}

int mrp_in_test_start(struct mrp *mrp, uint32_t interval)
{
	int err;

	if (interval == mrp->in_test_hw_interval)
		goto update_only_sw;

	mrp->in_watcher_work.repeat = (ev_tstamp)mrp->in_test_conf_period/ 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->in_watcher_work);

	mrp->in_test_hw_interval = interval;
	err = mrp_offload_send_in_test(mrp, interval, mrp->in_test_conf_max,
				       mrp->in_test_conf_period);
	if (err)
		return err;

update_only_sw:
	mrp->in_test_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->in_test_work);
	return 0;
}

void mrp_in_test_stop(struct mrp *mrp)
{
	mrp_offload_send_in_test(mrp, 0, 0, 0);
	/* Make sure that at the next start the HW is updated */
	mrp->in_test_hw_interval = -1;
	ev_timer_stop(EV_DEFAULT, &mrp->in_test_work);
	ev_timer_stop(EV_DEFAULT, &mrp->in_watcher_work);
}

void mrp_in_topo_start(struct mrp *mrp, uint32_t interval)
{
	mrp->in_topo_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->in_topo_work);
}

void mrp_in_topo_stop(struct mrp *mrp)
{
	ev_timer_stop(EV_DEFAULT, &mrp->in_topo_work);
}

void mrp_in_link_up_start(struct mrp *mrp, uint32_t interval)
{
	mrp->in_link_up_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->in_link_up_work);
}

void mrp_in_link_up_stop(struct mrp *mrp)
{
	ev_timer_stop(EV_DEFAULT, &mrp->in_link_up_work);
}

void mrp_in_link_down_start(struct mrp *mrp, uint32_t interval)
{
	mrp->in_link_down_work.repeat = (ev_tstamp)interval / 1000000;
	ev_timer_again(EV_DEFAULT, &mrp->in_link_down_work);
}

void mrp_in_link_down_stop(struct mrp *mrp)
{
	ev_timer_stop(EV_DEFAULT, &mrp->in_link_down_work);
}

void mrp_clear_fdb_stop(struct mrp *mrp)
{
	ev_timer_stop(EV_DEFAULT, &mrp->clear_fdb_work);
}

/* Stops all the timers */
void mrp_timer_stop(struct mrp *mrp)
{
	mrp_clear_fdb_stop(mrp);
	mrp_ring_topo_stop(mrp);
	mrp_ring_link_up_stop(mrp);
	mrp_ring_link_down_stop(mrp);
	mrp_ring_test_stop(mrp);

	if (mrp->in_role != BR_MRP_IN_ROLE_DISABLED) {
		mrp_in_topo_stop(mrp);
		mrp_in_link_up_stop(mrp);
		mrp_in_link_down_stop(mrp);
		mrp_in_test_stop(mrp);
	}
}

void mrp_timer_init(struct mrp *mrp)
{
	ev_init(&mrp->clear_fdb_work, mrp_clear_fdb_expired);
	ev_init(&mrp->ring_topo_work, mrp_ring_topo_expired);
	ev_init(&mrp->ring_test_work, mrp_ring_test_expired);
	ev_init(&mrp->ring_watcher_work, mrp_ring_watcher_expired);
	ev_init(&mrp->ring_link_up_work, mrp_ring_link_up_expired);
	ev_init(&mrp->ring_link_down_work, mrp_ring_link_down_expired);
	ev_init(&mrp->in_watcher_work, mrp_in_watcher_expired);
	ev_init(&mrp->in_test_work, mrp_in_test_expired);
	ev_init(&mrp->in_topo_work, mrp_in_topo_expired);
	ev_init(&mrp->in_link_up_work, mrp_in_link_up_expired);
	ev_init(&mrp->in_link_down_work, mrp_in_link_down_expired);
}
