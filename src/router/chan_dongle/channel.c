/* 
   Copyright (C) 2009 - 2010
   
   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org
   
   Dmitry Vagin <dmitry2004@yandex.ru>

   bg <bg_one@mail.ru>
*/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <asterisk.h>
#include <asterisk/dsp.h>			/* ast_dsp_digitreset() */
#include <asterisk/pbx.h>			/* pbx_builtin_setvar_helper() */
#include <asterisk/module.h>			/* ast_module_ref() ast_module_info = shit */
#include <asterisk/causes.h>			/* AST_CAUSE_INCOMPATIBLE_DESTINATION AST_CAUSE_FACILITY_NOT_IMPLEMENTED AST_CAUSE_REQUESTED_CHAN_UNAVAIL */
#include <asterisk/musiconhold.h>		/* ast_moh_start() ast_moh_stop() */
#include <asterisk/lock.h>			/* AST_MUTEX_DEFINE_STATIC */
#include <asterisk/timing.h>			/* ast_timer_fd() ast_timer_set_rate() ast_timer_ack() */
#include <asterisk/ast_version.h>		/* ASTERISK_VERSION_NUM */

#include "channel.h"
#include "chan_dongle.h"
#include "at_command.h"
#include "helpers.h"				/* get_at_clir_value()  */
#include "at_queue.h"				/* write_all() TODO: move out */
#include "manager.h"				/* manager_event_call_state_change() */

static char silence_frame[FRAME_SIZE];

#/* */
static int parse_dial_string(char * dialstr, const char** number, int * opts)
{
	char* options;
	char* dest_num;
	int lopts = 0;

	options = strchr (dialstr, '/');
	if (!options)
	{
		ast_log (LOG_WARNING, "Can't determine destination in chan_dongle\n");
		return AST_CAUSE_INCOMPATIBLE_DESTINATION;
	}
	*options++ = '\0';

	dest_num = strchr(options, ':');
	if(!dest_num)
	{
		dest_num = options;
	}
	else
	{
		*dest_num++ = '\0';

		if (!strcasecmp(options, "holdother"))
			lopts = CALL_FLAG_HOLD_OTHER;
		else if (!strcasecmp(options, "conference"))
			lopts = CALL_FLAG_HOLD_OTHER | CALL_FLAG_CONFERENCE;
		else
		{
			ast_log (LOG_WARNING, "Invalid options in chan_dongle\n");
			return AST_CAUSE_INCOMPATIBLE_DESTINATION;
		}
	}

	if (*dest_num == '\0')
	{
		ast_log (LOG_WARNING, "Empty destination in chan_dongle\n");
		return AST_CAUSE_INCOMPATIBLE_DESTINATION;
	}
	if (!is_valid_phone_number(dest_num))
	{
		ast_log (LOG_WARNING, "Invalid destination '%s' in chan_dongle, only 0123456789*#+ABC allowed\n", dest_num);
		return AST_CAUSE_INCOMPATIBLE_DESTINATION;
	}

	*number = dest_num;
	*opts = lopts;
	return 0;
}


#/* */
EXPORT_DEF int channels_loop(struct pvt * pvt, const struct ast_channel * requestor)
{
	/* not allow hold requester channel :) */
	/* FIXME: requestor may be just proxy/masquerade for real channel */
	//	use ast_bridged_channel(chan) ?
	//	use requestor->tech->get_base_channel() ?
	return (requestor
		&& ast_channel_tech(requestor) == &channel_tech
		&& ast_channel_tech_pvt(requestor)
		&& ((struct cpvt*) ast_channel_tech_pvt(requestor))->pvt == pvt)
		? 1
		: 0;
}

#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */

static struct ast_channel * channel_request (attribute_unused const char * type, struct ast_format_cap * cap, const struct ast_channel *requestor, void * data, int * cause)

#elif ASTERISK_VERSION_NUM >= 10800 /* 1.8+ */
//   TODO: simplify by move common code to functions
static struct ast_channel * channel_request (attribute_unused const char * type, format_t format, const struct ast_channel * requestor, void * data, int * cause)

#else /* 1.8- */
/* TODO: add check when request 'holdother' what requestor is not on same device for 1.6 */

static struct ast_channel * channel_request (attribute_unused const char * type, int format, void * data, int * cause)

#endif
{
#if ASTERISK_VERSION_NUM >= 10800 && ASTERISK_VERSION_NUM < 100000 /* 1.8+ .. 10- */
	format_t oldformat;
#elif ASTERISK_VERSION_NUM < 10800 /* 1.8- */
	int oldformat;
	const struct ast_channel * requestor = NULL;
#endif
	char * dest_dev;
	const char * dest_num;
	struct ast_channel * channel = NULL;
	struct pvt * pvt;
	int opts = CALL_FLAG_NONE;
	int exists;

	if (!data)
	{
		ast_log (LOG_WARNING, "Channel requested with no data\n");
		*cause = AST_CAUSE_INCOMPATIBLE_DESTINATION;
		return NULL;
	}

#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */
	if (!ast_format_cap_iscompatible(cap, &chan_dongle_format))
#else
	oldformat = format;
	format &= AST_FORMAT_SLINEAR;
	if (!format)
#endif
	{
#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */
		char buf[255];
		ast_log (LOG_WARNING, "Asked to get a channel of unsupported format '%s'\n", ast_getformatname_multiple (buf, 255, cap));
#elif ASTERISK_VERSION_NUM >= 10800 /* 1.8+ */
		ast_log (LOG_WARNING, "Asked to get a channel of unsupported format '%s'\n", ast_getformatname (oldformat));
#else
		ast_log (LOG_WARNING, "Asked to get a channel of unsupported format '%d'\n", oldformat);
#endif
		*cause = AST_CAUSE_FACILITY_NOT_IMPLEMENTED;
		return NULL;
	}

	dest_dev = ast_strdupa (data);

	*cause = parse_dial_string(dest_dev, &dest_num, &opts);
	if(*cause)
		return NULL;

#if ASTERISK_VERSION_NUM >= 10800
	pvt = find_device_by_resource(dest_dev, opts, requestor, &exists);
#else
	pvt = find_device_by_resource(dest_dev, opts, NULL, &exists);
#endif
	if(pvt)
	{
		channel = new_channel (pvt, AST_STATE_DOWN, NULL, pvt_get_pseudo_call_idx(pvt), CALL_DIR_OUTGOING, CALL_STATE_INIT, NULL, requestor);
		ast_mutex_unlock (&pvt->lock);
		if(!channel)
		{
			ast_log (LOG_WARNING, "Unable to allocate channel structure\n");
			*cause = AST_CAUSE_REQUESTED_CHAN_UNAVAIL;

		}
	}
	else
	{
		ast_log (LOG_WARNING, "[%s] Request to call on device %s\n", dest_dev, exists ? "which can not make call at this moment" : "not exists");
		*cause = AST_CAUSE_REQUESTED_CHAN_UNAVAIL;
	}

	return channel;
}

#/* */
static int channel_call (struct ast_channel* channel, char* dest, attribute_unused int timeout)
{
	struct cpvt* cpvt = ast_channel_tech_pvt(channel);
	struct pvt* pvt;
	char* dest_dev;
	const char* dest_num;
	int clir = 0;
	int opts;

	if(!cpvt || cpvt->channel != channel || !cpvt->pvt)
	{
		ast_log (LOG_WARNING, "call on unreferenced %s\n", ast_channel_name(channel));
		return -1;
	}
	pvt = cpvt->pvt;

	dest_dev = ast_strdupa (dest);

	if(parse_dial_string(dest_dev, &dest_num, &opts))
		return -1;

	if ((ast_channel_state(channel) != AST_STATE_DOWN) && (ast_channel_state(channel) != AST_STATE_RESERVED))
	{
		ast_log (LOG_WARNING, "channel_call called on %s, neither down nor reserved\n", ast_channel_name(channel));
		return -1;
	}

	ast_mutex_lock (&pvt->lock);

// FIXME: check if bridged on same device with CALL_FLAG_HOLD_OTHER
	if (!ready4voice_call(pvt, cpvt, opts))
	{
		ast_mutex_unlock (&pvt->lock);
		ast_log (LOG_ERROR, "[%s] Error device already in use or uninitialized\n", PVT_ID(pvt));
		return -1;
	}
	CPVT_SET_FLAGS(cpvt, opts);

	ast_debug (1, "[%s] Calling %s on %s\n", PVT_ID(pvt), dest, ast_channel_name(channel));

	if (CONF_SHARED(pvt, usecallingpres))
	{
		if (CONF_SHARED(pvt, callingpres) < 0)
		{
			clir = ast_channel_connected(channel)->id.number.presentation;
		}
		else
		{
			clir = CONF_SHARED(pvt, callingpres);
		}

		clir = get_at_clir_value (pvt, clir);
	}
	else
	{
		clir = -1;
	}

	PVT_STAT(pvt, out_calls) ++;
	if (at_enque_dial (cpvt, dest_num, clir))
	{
		ast_mutex_unlock (&pvt->lock);
		ast_log (LOG_ERROR, "[%s] Error sending ATD command\n", PVT_ID(pvt));
		return -1;
	}

	ast_mutex_unlock (&pvt->lock);

	return 0;
}

#/* ARCH: move to cpvt level */
static void disactivate_call(struct cpvt* cpvt)
{
	if(cpvt->channel && CPVT_TEST_FLAG(cpvt, CALL_FLAG_ACTIVATED))
	{
		mixb_detach(&cpvt->pvt->a_write_mixb, &cpvt->mixstream);
		ast_channel_set_fd (cpvt->channel, 1, -1);
		ast_channel_set_fd (cpvt->channel, 0, -1);
		CPVT_RESET_FLAGS(cpvt, CALL_FLAG_ACTIVATED | CALL_FLAG_MASTER);

		ast_debug (6, "[%s] call idx %d disactivated\n", PVT_ID(cpvt->pvt), cpvt->call_idx);
	}
}

#/* ARCH: move to cpvt level */
static void activate_call(struct cpvt* cpvt)
{
	struct cpvt* cpvt2;
	struct pvt* pvt;

	/* nothing todo, already main */
	if(CPVT_TEST_FLAG(cpvt, CALL_FLAG_MASTER))
		return;

	/* drop any other from MASTER, any set pipe for actives */
	pvt = cpvt->pvt;
	AST_LIST_TRAVERSE(&pvt->chans, cpvt2, entry)
	{
		if(cpvt2 != cpvt)
		{
			if(CPVT_TEST_FLAG(cpvt, CALL_FLAG_MASTER))
			{
				ast_debug (6, "[%s] call idx %d gave master\n", PVT_ID(pvt), cpvt2->call_idx);
			}

			CPVT_RESET_FLAGS(cpvt2, CALL_FLAG_MASTER);
			if(cpvt2->channel)
			{
				ast_channel_set_fd (cpvt2->channel, 1, -1);
				if(CPVT_TEST_FLAG(cpvt, CALL_FLAG_ACTIVATED))
				{
					ast_channel_set_fd (cpvt2->channel, 0, cpvt2->rd_pipe[PIPE_READ]);
					ast_debug (6, "[%s] call idx %d still active fd %d\n", PVT_ID(pvt), cpvt2->call_idx, cpvt2->rd_pipe[PIPE_READ]);
				}
			}
		}
	}

	/* setup call local write possition */
	if(!CPVT_TEST_FLAG(cpvt, CALL_FLAG_ACTIVATED))
	{
		// FIXME: reset possition?
		mixb_attach(&pvt->a_write_mixb, &cpvt->mixstream);
//		rb_init (&cpvt->a_write_rb, cpvt->a_write_buf, sizeof (cpvt->a_write_buf));
//		cpvt->write = pvt->a_write_rb.write;
//		cpvt->used = pvt->a_write_rb.used;
	}

	if (pvt->audio_fd >= 0)
	{
		CPVT_SET_FLAGS(cpvt, CALL_FLAG_ACTIVATED | CALL_FLAG_MASTER);
		if(cpvt->channel)
		{
			ast_channel_set_fd (cpvt->channel, 0, pvt->audio_fd);
			if (pvt->a_timer)
			{
				ast_channel_set_fd (cpvt->channel, 1, ast_timer_fd (pvt->a_timer));
				ast_timer_set_rate (pvt->a_timer, 50);
/*				ast_debug (3, "[%s] Timer set\n", PVT_ID(pvt));
*/
			}
		}
		if(pvt->dsp)
			ast_dsp_digitreset(pvt->dsp);
		pvt->dtmf_digit = 0;
		ast_debug (6, "[%s] call idx %d was master\n", PVT_ID(pvt), cpvt->call_idx);
	}
}

#/* we has 2 case of call this function, when local side want terminate call and when called for cleanup after remote side alreay terminate call, CEND received and cpvt destroyed */
static int channel_hangup (struct ast_channel* channel)
{
	struct cpvt* cpvt = ast_channel_tech_pvt(channel);
	struct pvt* pvt;

	/* its possible call with channel w/o tech_pvt */
	if(cpvt && cpvt->channel == channel && cpvt->pvt)
	{
		pvt = cpvt->pvt;

		ast_mutex_lock (&pvt->lock);

		ast_debug (1, "[%s] Hanging up call idx %d need hangup %d\n", PVT_ID(pvt), cpvt->call_idx, CPVT_TEST_FLAG(cpvt, CALL_FLAG_NEED_HANGUP) ? 1 : 0);

		if (CPVT_TEST_FLAG(cpvt, CALL_FLAG_NEED_HANGUP))
		{
			if (at_enque_hangup (cpvt, cpvt->call_idx))
				ast_log (LOG_ERROR, "[%s] Error adding AT+CHUP command to queue, call not terminated!\n", PVT_ID(pvt));
			else
				CPVT_RESET_FLAGS(cpvt, CALL_FLAG_NEED_HANGUP);

		}

		disactivate_call (cpvt);

		/* drop cpvt->channel reference */
		cpvt->channel = NULL;
		ast_mutex_unlock (&pvt->lock);
	}

	/* drop channel -> cpvt reference */
	ast_channel_tech_pvt_set(channel, NULL);

	ast_module_unref (self_module());
	ast_setstate (channel, AST_STATE_DOWN);

	return 0;
}

#/* */
static int channel_answer (struct ast_channel* channel)
{
	struct cpvt* cpvt = ast_channel_tech_pvt(channel);
	struct pvt* pvt;

	if(!cpvt || cpvt->channel != channel || !cpvt->pvt)
	{
		ast_log (LOG_WARNING, "call on unreferenced %s\n", ast_channel_name(channel));
		return 0;
	}
	pvt = cpvt->pvt;

	ast_mutex_lock (&pvt->lock);

	if (cpvt->dir == CALL_DIR_INCOMING)
	{
		if (at_enque_answer (cpvt))
		{
			ast_log (LOG_ERROR, "[%s] Error sending answer commands\n", PVT_ID(pvt));
		}
	}

	ast_mutex_unlock (&pvt->lock);

	return 0;

}

#/* */
static int channel_digit_begin (struct ast_channel* channel, char digit)
{
	struct cpvt* cpvt = ast_channel_tech_pvt(channel);
	struct pvt* pvt;
	int rv;

	if(!cpvt || cpvt->channel != channel || !cpvt->pvt)
	{
		ast_log (LOG_WARNING, "call on unreferenced %s\n", ast_channel_name(channel));
		return -1;
	}
	pvt = cpvt->pvt;

	ast_mutex_lock (&pvt->lock);

	rv = at_enque_dtmf (cpvt, digit);
	if (rv)
	{
		ast_mutex_unlock (&pvt->lock);
		if(rv == -1974)
			ast_log (LOG_WARNING, "[%s] Sending DTMF %c not supported by dongle. Tell Asterisk to generate inband\n", PVT_ID(pvt), digit);
		else
			ast_log (LOG_ERROR, "[%s] Error adding DTMF %c command to queue\n", PVT_ID(pvt), digit);
		return -1;
	}

	ast_mutex_unlock (&pvt->lock);

	ast_debug (3, "[%s] Send DTMF %c\n", PVT_ID(pvt), digit);

	return 0;
}


#/* */
static int channel_digit_end (attribute_unused struct ast_channel* channel, attribute_unused char digit, attribute_unused unsigned int duration)
{
	return 0;
}

#/* ARCH: move to cpvt level */
static void iov_write(struct pvt* pvt, int fd, struct iovec * iov, int iovcnt)
{
	ssize_t written;
	ssize_t done = 0;
	int count = 10;

	while(iovcnt)
	{
again:
		written = writev (fd, iov, iovcnt);
		if(written < 0)
		{
			if((errno == EINTR || errno == EAGAIN))
			{
				--count;
				if(count != 0)
				    goto again;
				ast_debug (1, "[%s] Deadlock avoided for write!\n", PVT_ID(pvt));
			}
			break;
		}
		else
		{
			done += written;
			count = 10;
			do
			{
				if((size_t)written >= iov->iov_len)
				{
					written -= iov->iov_len;
					iovcnt--;
					iov++;
				}
				else
				{
					iov->iov_len -= written;
					goto again;
				}
			} while(written > 0);
		}
	}
	PVT_STAT(pvt, a_write_bytes) += done;

	if (done != FRAME_SIZE)
	{
		ast_debug (1, "[%s] Write error!\n", PVT_ID(pvt));
	}
}

#/* */
static void timing_write (struct pvt* pvt)
{
	size_t			used;
	int			iovcnt;
	struct iovec		iov[3];
	const char*		msg = NULL;
//	char			buffer[FRAME_SIZE];
//	struct cpvt*		cpvt;

//	ast_debug (6, "[%s] tm write |\n", PVT_ID(pvt));

//	memset(buffer, 0, sizeof(buffer));

//	AST_LIST_TRAVERSE(&pvt->chans, cpvt, entry) {

//		if(!CPVT_IS_ACTIVE(cpvt))
//			continue;

		used = mixb_used (&pvt->a_write_mixb);
//		used = rb_used (&cpvt->a_write_rb);

		if (used >= FRAME_SIZE)
		{
			iovcnt = mixb_read_n_iov (&pvt->a_write_mixb, iov, FRAME_SIZE);
			mixb_read_n_iov (&pvt->a_write_mixb, iov, FRAME_SIZE);
			mixb_read_upd (&pvt->a_write_mixb, FRAME_SIZE);
		}
		else if (used > 0)
		{
			PVT_STAT(pvt, write_tframes) ++;
			msg = "[%s] write truncated frame\n";

			iovcnt = mixb_read_all_iov (&pvt->a_write_mixb, iov);
			mixb_read_all_iov (&pvt->a_write_mixb, iov);
			mixb_read_upd (&pvt->a_write_mixb, used);

			iov[iovcnt].iov_base	= silence_frame;
			iov[iovcnt].iov_len	= FRAME_SIZE - used;
			iovcnt++;
		}
		else
		{
			PVT_STAT(pvt, write_sframes) ++;
			msg = "[%s] write silence\n";

			iov[0].iov_base		= silence_frame;
			iov[0].iov_len		= FRAME_SIZE;
			iovcnt			= 1;
//			continue;
		}

//		iov_add(buffer, sizeof(buffer), iov);
		if(msg)
			ast_debug (7, msg, PVT_ID(pvt));

//	}


	PVT_STAT(pvt, write_frames) ++;
	iov_write(pvt, pvt->audio_fd, iov, iovcnt);
//	if(write_all(pvt->audio_fd, buffer, sizeof(buffer)) != sizeof(buffer))
//		ast_debug (1, "[%s] Write error!\n", PVT_ID(pvt));

}

#/* copy voice data from device to each channel in conference */
static void write_conference(struct pvt * pvt, const char * buffer, size_t length)
{
	struct cpvt* cpvt;
	size_t wr;

	AST_LIST_TRAVERSE(&pvt->chans, cpvt, entry) {
		if(CPVT_IS_ACTIVE(cpvt) && !CPVT_IS_MASTER(cpvt) && CPVT_TEST_FLAG(cpvt, CALL_FLAG_MULTIPARTY) && cpvt->rd_pipe[PIPE_WRITE] >= 0)
		{
			wr = write_all(cpvt->rd_pipe[PIPE_WRITE], buffer, length);
//			ast_debug (6, "[%s] write2 | call idx %d pipe fd %d wrote %d bytes\n", PVT_ID(pvt), cpvt->call_idx, cpvt->rd_pipe[PIPE_WRITE], wr);
			if(wr != length)
			{
				ast_debug (1, "[%s] Pipe write error %d\n", PVT_ID(pvt), errno);
			}
		}
	}

}

#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */
#define subclass_integer	subclass.integer
#elif ASTERISK_VERSION_NUM >= 10800 /* 1.8+ */
#define subclass_codec		subclass.codec
#define subclass_integer	subclass.integer
#else /* 1.8- */
#define subclass_codec		subclass
#define subclass_integer	subclass
#endif

#/* */
static struct ast_frame* channel_read (struct ast_channel* channel)
{
	struct cpvt*		cpvt = ast_channel_tech_pvt(channel);
	struct pvt*		pvt;
	struct ast_frame*	f = &ast_null_frame;
	ssize_t			res;

	if(!cpvt || cpvt->channel != channel || !cpvt->pvt)
	{
		return f;
	}
	pvt = cpvt->pvt;

	while (ast_mutex_trylock (&pvt->lock))
	{
		CHANNEL_DEADLOCK_AVOIDANCE (channel);
	}

	ast_debug (7, "[%s] read call idx %d state %d audio_fd %d\n", PVT_ID(pvt), cpvt->call_idx, cpvt->state, pvt->audio_fd);

	/* FIXME: move down for enable timing_write() to device ? */
	if (!CPVT_IS_SOUND_SOURCE(cpvt) || pvt->audio_fd < 0)
	{
		goto e_return;
	}

	if (pvt->a_timer && ast_channel_fdno(channel) == 1)
	{
		ast_timer_ack (pvt->a_timer, 1);
		timing_write (pvt);
		ast_debug (7, "[%s] *** timing ***\n", PVT_ID(pvt));
	}

	else
	{
		memset (&cpvt->a_read_frame, 0, sizeof (cpvt->a_read_frame));

		cpvt->a_read_frame.frametype = AST_FRAME_VOICE;
#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */
		ast_format_copy(&cpvt->a_read_frame.subclass.format, &chan_dongle_format);
#else
		cpvt->a_read_frame.subclass_codec = AST_FORMAT_SLINEAR;
#endif
		cpvt->a_read_frame.data.ptr = cpvt->a_read_buf + AST_FRIENDLY_OFFSET;
		cpvt->a_read_frame.offset = AST_FRIENDLY_OFFSET;
		cpvt->a_read_frame.src = AST_MODULE;

		res = read (CPVT_IS_MASTER(cpvt) ? pvt->audio_fd : cpvt->rd_pipe[PIPE_READ], cpvt->a_read_frame.data.ptr, FRAME_SIZE);
		if (res <= 0)
		{
			if (errno != EAGAIN && errno != EINTR)
			{
				ast_debug (1, "[%s] Read error %d, going to wait for new connection\n", PVT_ID(pvt), errno);
			}

			goto e_return;
		}

/*		ast_debug (7, "[%s] call idx %d read %u\n", PVT_ID(pvt), cpvt->call_idx, (unsigned)res);
		ast_debug (6, "[%s] read | call idx %d fd %d readed %d bytes\n", PVT_ID(pvt), cpvt->call_idx, pvt->audio_fd, res);
*/

		if(CPVT_IS_MASTER(cpvt))
		{
			if(CPVT_TEST_FLAG(cpvt, CALL_FLAG_MULTIPARTY))
				write_conference(pvt, cpvt->a_read_frame.data.ptr, res);

			PVT_STAT(pvt, a_read_bytes) += res;
			PVT_STAT(pvt, read_frames) ++;
			if(res < FRAME_SIZE)
				PVT_STAT(pvt, read_sframes) ++;
		}

		cpvt->a_read_frame.samples	= res / 2;
		cpvt->a_read_frame.datalen	= res;
		ast_frame_byteswap_le (&cpvt->a_read_frame);
/*
		cpvt->a_read_frame.ts;
		cpvt->a_read_frame.len;
		cpvt->a_read_frame.seqno;
*/
		f = &cpvt->a_read_frame;
		if (pvt->dsp)
		{
			f = ast_dsp_process (channel, pvt->dsp, f);
			if ((f->frametype == AST_FRAME_DTMF_END) || (f->frametype == AST_FRAME_DTMF_BEGIN))
			{
				if ((f->subclass_integer == 'm') || (f->subclass_integer == 'u'))
				{
					f->frametype = AST_FRAME_NULL;
					f->subclass_integer = 0;
					goto e_return;
				}
				if(f->frametype == AST_FRAME_DTMF_BEGIN)
				{
					pvt->dtmf_begin_time = ast_tvnow();
				}
				else if (f->frametype == AST_FRAME_DTMF_END)
				{
					if(!ast_tvzero(pvt->dtmf_begin_time) && ast_tvdiff_ms(ast_tvnow(), pvt->dtmf_begin_time) < CONF_SHARED(pvt, mindtmfgap))
					{
						ast_debug(1, "[%s] DTMF char %c ignored min gap %d > %ld\n", PVT_ID(pvt), f->subclass_integer, CONF_SHARED(pvt, mindtmfgap), (long)ast_tvdiff_ms(ast_tvnow(), pvt->dtmf_begin_time));
						f->frametype = AST_FRAME_NULL;
						f->subclass_integer = 0;
					}
					else if(f->len < CONF_SHARED(pvt, mindtmfduration))
					{
						ast_debug(1, "[%s] DTMF char %c ignored min duration %d > %ld\n", PVT_ID(pvt), f->subclass_integer, CONF_SHARED(pvt, mindtmfduration), f->len);
						f->frametype = AST_FRAME_NULL;
						f->subclass_integer = 0;
					}
					else if(f->subclass_integer == pvt->dtmf_digit
							&& 
						!ast_tvzero(pvt->dtmf_end_time)
							&& 
						ast_tvdiff_ms(ast_tvnow(), pvt->dtmf_end_time) < CONF_SHARED(pvt, mindtmfinterval))
					{
						ast_debug(1, "[%s] DTMF char %c ignored min interval %d > %ld\n", PVT_ID(pvt), f->subclass_integer, CONF_SHARED(pvt, mindtmfinterval), (long)ast_tvdiff_ms(ast_tvnow(), pvt->dtmf_end_time));
						f->frametype = AST_FRAME_NULL;
						f->subclass_integer = 0;
					}
					else
					{
						ast_debug(1, "[%s] Got DTMF char %c\n",PVT_ID(pvt), f->subclass_integer);
						pvt->dtmf_digit = f->subclass_integer;
						pvt->dtmf_end_time = ast_tvnow();
					}

				}
				goto e_return;
			}
		}

		if (CONF_SHARED(pvt, rxgain) && f->frametype == AST_FRAME_VOICE)
		{
			if (ast_frame_adjust_volume (f, CONF_SHARED(pvt, rxgain)) == -1)
			{
				ast_debug (1, "[%s] Volume could not be adjusted!\n", PVT_ID(pvt));
			}
		}
	}

e_return:
	ast_mutex_unlock (&pvt->lock);

	return f;
}

#/* */
static int channel_write (struct ast_channel* channel, struct ast_frame* f)
{
	struct cpvt* cpvt = ast_channel_tech_pvt(channel);
	struct pvt* pvt;
	size_t count;
	int gains[2];

#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */
	if (f->frametype != AST_FRAME_VOICE || f->subclass.format.id != AST_FORMAT_SLINEAR)
#else /* 10- */
	if (f->frametype != AST_FRAME_VOICE || f->subclass_codec != AST_FORMAT_SLINEAR)
#endif
	{
		return 0;
	}

	if(!cpvt || cpvt->channel != channel || !cpvt->pvt)
	{
		ast_log (LOG_WARNING, "call on unreferenced %s\n", ast_channel_name(channel));
		return 0;
	}

	/* TODO: write silence better ? */
	/* TODO: check end of bridge loop condition */
	/* never write to same device from other channel its possible for call hold or conference */
	if(CPVT_TEST_FLAG(cpvt, CALL_FLAG_BRIDGE_LOOP))
		return 0;

	pvt = cpvt->pvt;

	ast_debug (7, "[%s] write call idx %d state %d\n", PVT_ID(pvt), cpvt->call_idx, cpvt->state);

	while (ast_mutex_trylock (&pvt->lock))
	{
		CHANNEL_DEADLOCK_AVOIDANCE (channel);
	}

	if(!CPVT_IS_ACTIVE(cpvt))
		goto e_return;

	if(CPVT_TEST_FLAG(cpvt, CALL_FLAG_MULTIPARTY) && !CPVT_TEST_FLAG(cpvt, CALL_FLAG_BRIDGE_CHECK))
	{
		struct ast_channel* bridged = ast_bridged_channel(channel);

		CPVT_SET_FLAGS(cpvt, CALL_FLAG_BRIDGE_CHECK);

		if(bridged && ast_channel_tech(bridged) == &channel_tech && ast_channel_tech_pvt(bridged) && ((struct cpvt*) ast_channel_tech_pvt(bridged))->pvt == pvt)
		{
			CPVT_SET_FLAGS(cpvt, CALL_FLAG_BRIDGE_LOOP);
			CPVT_SET_FLAGS((struct cpvt*) ast_channel_tech_pvt(bridged), CALL_FLAG_BRIDGE_LOOP);
			ast_log (LOG_WARNING, "[%s] Bridged channels %s and %s working on same device, discard writes to avoid voice loop\n", PVT_ID(pvt), ast_channel_name(channel), ast_channel_name(bridged));
			goto e_return;
		}
	}

	if (pvt->audio_fd < 0)
	{
		ast_debug (1, "[%s] audio_fd not ready\n", PVT_ID(pvt));
	}
	else
	{
		if(f->datalen)
		{
			/** try to minimize of ast_frame_adjust_volume() calls: 
			 *  one hand we must obey txgain but with other divide gain to 
			 *  number of mixed channels. In some cases one call of ast_frame_adjust_volume() enough
			*/

			gains[1] = mixb_streams(&pvt->a_write_mixb);
			if(gains[1] < 1 || pvt->a_timer == NULL)
				gains[1] = 1;

			gains[0] = CONF_SHARED(pvt, txgain);
			if(gains[0] <= -2)
			{
				gains[0] *= gains[1];
				gains[1] = 0;
			}
			else if(gains[0] <= 1)
			{
				gains[0] = - gains[1];
				gains[1] = 0;
			}
			else if(gains[0] % gains[1] == 0)
			{
				gains[0] /= gains[1];
				gains[1] = 0;
			}

			for(count = 0; count < ITEMS_OF(gains); ++count)
			{
				if(gains[count] > 1 || gains[count] < -1)
					if(ast_frame_adjust_volume (f, gains[count]) == -1)
					{
						ast_debug (1, "[%s] Volume could not be adjusted!\n", PVT_ID(pvt));
					}
			}
		}


		if (pvt->a_timer)
		{
			count = mixb_free (&pvt->a_write_mixb, &cpvt->mixstream);

			if (count < (size_t) f->datalen)
			{
				mixb_read_upd (&pvt->a_write_mixb, f->datalen - count);

				PVT_STAT(pvt, write_rb_overflow_bytes) += f->datalen - count;
				PVT_STAT(pvt, write_rb_overflow) ++;
			}

			mixb_write (&pvt->a_write_mixb, &cpvt->mixstream, f->data.ptr, f->datalen);

/*
			ast_debug (6, "[%s] write | call idx %d, %d bytes lwrite %d lused %d write %d used %d\n", PVT_ID(pvt), cpvt->call_idx, f->datalen, cpvt->write, cpvt->used, pvt->a_write_rb.write, pvt->a_write_rb.used);
			rb_tetris(&pvt->a_write_rb, f->data.ptr, f->datalen, &cpvt->write, &cpvt->used);
			ast_debug (6, "[%s] write | lwrite %d lused %d write %d used %d\n", PVT_ID(pvt), cpvt->write, cpvt->used, pvt->a_write_rb.write, pvt->a_write_rb.used);
*/
		}

		else
		{
			if(mixb_streams(&pvt->a_write_mixb) != 1)
			{
				ast_log (LOG_ERROR, "[%s] write conference without timer\n", PVT_ID(pvt));
				goto e_return;
			}

			{
			int iovcnt;
			struct iovec iov[2];

			ast_frame_byteswap_le (f);
			iov[0].iov_base = f->data.ptr;
			iov[0].iov_len = FRAME_SIZE;

			if (f->datalen < FRAME_SIZE)
			{
				iov[0].iov_len = f->datalen;
				iov[1].iov_base = silence_frame;
				iov[1].iov_len = FRAME_SIZE - f->datalen;
				iovcnt = 2;
				PVT_STAT(pvt, write_tframes) ++;
			}
			else
			{
				iovcnt = 1;
			}

			iov_write(pvt, pvt->audio_fd, iov, iovcnt);
			PVT_STAT(pvt, write_frames) ++;
			}
		}

/*		if (f->datalen != 320)
*/
		{
			ast_debug (7, "[%s] Write frame: samples = %d, data lenght = %d byte\n", PVT_ID(pvt), f->samples, f->datalen);
		}
	}

e_return:
	ast_mutex_unlock (&pvt->lock);

	return 0;
}
#undef subclass_integer
#undef subclass_codec

#/* */
static int channel_fixup (struct ast_channel* oldchannel, struct ast_channel* newchannel)
{
	struct cpvt * cpvt = ast_channel_tech_pvt(newchannel);
	struct pvt* pvt;

	if (!cpvt || !cpvt->pvt)
	{
		ast_log (LOG_WARNING, "call on unreferenced %s\n", ast_channel_name(newchannel));
		return -1;
	}
	pvt = cpvt->pvt;

	ast_mutex_lock (&pvt->lock);
	if (cpvt->channel == oldchannel)
	{
		cpvt->channel = newchannel;
	}
	ast_mutex_unlock (&pvt->lock);

	return 0;
}

#/* FIXME: must modify in conjuction with state on call not whole device? */
static int channel_devicestate (void* data)
{
	char*	device;
	struct pvt*	pvt;
	int	res = AST_DEVICE_INVALID;

	device = ast_strdupa (data ? data : "");

	ast_debug (1, "Checking device state for device %s\n", device);

	pvt = find_device_ext (device, NULL);
	if (pvt)
	{
		if (pvt->connected)
		{
			if (is_dial_possible(pvt, CALL_FLAG_NONE))
			{
				res = AST_DEVICE_NOT_INUSE;
			}
			else
			{
				res = AST_DEVICE_INUSE;
			}
		}
		ast_mutex_unlock (&pvt->lock);
	}

	return res;
}

#/* */
static int channel_indicate (struct ast_channel* channel, int condition, const void* data, attribute_unused size_t datalen)
{
	int res = 0;

	ast_debug (1, "[%s] Requested indication %d\n", ast_channel_name(channel), condition);

	switch (condition)
	{
		case AST_CONTROL_BUSY:
		case AST_CONTROL_CONGESTION:
		case AST_CONTROL_RINGING:
		case -1:
			res = -1;
			break;
/* appears in r295843 */
#ifdef HAVE_AST_CONTROL_SRCCHANGE
		case AST_CONTROL_SRCCHANGE:
#endif
		case AST_CONTROL_PROGRESS:
		case AST_CONTROL_PROCEEDING:
		case AST_CONTROL_VIDUPDATE:
		case AST_CONTROL_SRCUPDATE:
			break;

		case AST_CONTROL_HOLD:
			ast_moh_start (channel, data, NULL);
			break;

		case AST_CONTROL_UNHOLD:
			ast_moh_stop (channel);
			break;

		default:
			ast_log (LOG_WARNING, "[%s] Don't know how to indicate condition %d\n", ast_channel_name(channel), condition);
			res = -1;
			break;
	}

	return res;
}


/* ARCH: move to cpvt */
/* FIXME: protection for cpvt->channel if exists */
#/* NOTE: called from device level with locked pvt */
EXPORT_DEF void change_channel_state(struct cpvt * cpvt, unsigned newstate, int cause)
{
	struct ast_channel * channel;
	struct pvt* pvt;
	call_state_t oldstate = cpvt->state;
	short call_idx;

	if(newstate != oldstate)
	{
		pvt = cpvt->pvt;
		channel = cpvt->channel;
		call_idx = cpvt->call_idx;

		cpvt->state = newstate;
		PVT_STATE(pvt, chan_count[oldstate])--;
		PVT_STATE(pvt, chan_count[newstate])++;

		ast_debug (1, "[%s] call idx %d mpty %d, change state from '%s' to '%s' has%s channel\n", PVT_ID(pvt), call_idx, CPVT_TEST_FLAG(cpvt, CALL_FLAG_MULTIPARTY) ? 1 : 0, call_state2str(oldstate), call_state2str(newstate), channel ? "" : "'t");

		/* update bits of devstate cache */
		switch(newstate)
		{
			case CALL_STATE_ACTIVE:
			case CALL_STATE_RELEASED:
				/* no split to incoming/outgoing because these states not intersect */
				switch(oldstate)
				{
					case CALL_STATE_INIT:
					case CALL_STATE_DIALING:
					case CALL_STATE_ALERTING:
						pvt->dialing = 0;
						break;
					case CALL_STATE_INCOMING:
						pvt->ring = 0;
						break;
					case CALL_STATE_WAITING:
						pvt->cwaiting = 0;
						break;
					default:;
				}
				break;
			default:;
		}

		/* check channel is dead */
		if(!channel)
		{
			/* channel already dead */
			if(newstate == CALL_STATE_RELEASED)
				cpvt_free(cpvt);
		}
		else
		{
			/* for live channel */
			switch(newstate)
			{
				case CALL_STATE_DIALING:
					/* from ^ORIG:idx,y */
					activate_call(cpvt);
					queue_control_channel (cpvt, AST_CONTROL_PROGRESS);
					ast_setstate (channel, AST_STATE_DIALING);
					break;

				case CALL_STATE_ALERTING:
					activate_call(cpvt);
					queue_control_channel (cpvt, AST_CONTROL_RINGING);
					ast_setstate (channel, AST_STATE_RINGING);
					break;

				case CALL_STATE_ACTIVE:
					activate_call(cpvt);
					if (oldstate == CALL_STATE_ONHOLD)
					{
						ast_debug (1, "[%s] Unhold call idx %d\n", PVT_ID(pvt), call_idx);
						queue_control_channel (cpvt, AST_CONTROL_UNHOLD);
					}
					else if (cpvt->dir == CALL_DIR_OUTGOING)
					{
						ast_debug (1, "[%s] Remote end answered on call idx %d\n", PVT_ID(pvt), call_idx);
						queue_control_channel (cpvt, AST_CONTROL_ANSWER);
					}
					else /* if (cpvt->answered) */
					{
						ast_debug (1, "[%s] Call idx %d answer\n", PVT_ID(pvt), call_idx);
						ast_setstate (channel, AST_STATE_UP);
					}
					break;

				case CALL_STATE_ONHOLD:
					disactivate_call(cpvt);
					ast_debug (1, "[%s] Hold call idx %d\n", PVT_ID(pvt), call_idx);
					queue_control_channel (cpvt, AST_CONTROL_HOLD);
					break;

				case CALL_STATE_RELEASED:
					disactivate_call(cpvt);
					/* from +CEND, restart or disconnect */


					/* drop channel -> cpvt reference */
					ast_channel_tech_pvt_set(channel, NULL);
					cpvt_free(cpvt);
					if (queue_hangup (channel, cause))
					{
						ast_log (LOG_ERROR, "[%s] Error queueing hangup...\n", PVT_ID(pvt));
					}

					break;
			}
		}
		manager_event_call_state_change(PVT_ID(pvt), call_idx, call_state2str(newstate));
	}
}

#/* */
static void set_channel_vars(struct pvt* pvt, struct ast_channel* channel)
{
	unsigned idx;
	channel_var_t dev_vars[] = 
	{
		{ "DONGLENAME", PVT_ID(pvt) },
		{ "DONGLEPROVIDER", pvt->provider_name },
		{ "DONGLEIMEI", pvt->imei },
		{ "DONGLEIMSI", pvt->imsi },
		{ "DONGLENUMBER", pvt->subscriber_number },
	};

	//TODO uncomment and fix
	//ast_string_field_set (channel, language, CONF_SHARED(pvt, language));

	for(idx = 0; idx < ITEMS_OF(dev_vars); ++idx)
		pbx_builtin_setvar_helper (channel, dev_vars[idx].name, dev_vars[idx].value);

}

#/* NOTE: called from device and current levels with locked pvt */
EXPORT_DEF struct ast_channel* new_channel (struct pvt* pvt, int ast_state, const char* cid_num, int call_idx, unsigned dir, call_state_t state, const char * dnid, attribute_unused const struct ast_channel * requestor)
{
	struct ast_channel* channel;
	struct cpvt * cpvt;

	cpvt = cpvt_alloc(pvt, call_idx, dir, state);
	if (cpvt)
	{
#if ASTERISK_VERSION_NUM >= 10800
		channel = ast_channel_alloc (1, ast_state, cid_num, PVT_ID(pvt), NULL, dnid, CONF_SHARED(pvt, context), requestor ? ast_channel_linkedid(requestor): NULL, 0, "%s/%s-%02u%08lx", channel_tech.type, PVT_ID(pvt), call_idx, pvt->channel_instanse);
#else
		channel = ast_channel_alloc (1, ast_state, cid_num, PVT_ID(pvt), NULL, dnid, CONF_SHARED(pvt, context), 0, "%s/%s-%02u%08lx", channel_tech.type, PVT_ID(pvt), call_idx, pvt->channel_instanse);
#endif
		if (channel)
		{
			cpvt->channel = channel;
			pvt->channel_instanse++;

			ast_channel_tech_pvt_set(channel, cpvt);
			ast_channel_tech_set(channel, &channel_tech);
#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */
			ast_format_cap_add(ast_channel_nativeformats(channel), &chan_dongle_format);
			ast_format_copy(ast_channel_writeformat(channel), &chan_dongle_format);
			ast_format_copy(ast_channel_readformat(channel), &chan_dongle_format);
#else /* 10- */
			channel->nativeformats	= AST_FORMAT_SLINEAR;
			channel->writeformat	= AST_FORMAT_SLINEAR;
			channel->readformat	= AST_FORMAT_SLINEAR;
#endif

			if (ast_state == AST_STATE_RING)
			{
				ast_channel_rings_set(channel, 1);
			}

			set_channel_vars(pvt, channel);

			if(dnid != NULL && dnid[0] != 0)
				pbx_builtin_setvar_helper(channel, "CALLERID(dnid)", dnid);
/*
#if ASTERISK_VERSION_NUM >= 10800
				channel->dialed.number.str = ast_strdup(dnid);
#else
				channel->cid.cid_dnid = ast_strdup(dnid);
#endif
*/
			ast_jb_configure (channel, &CONF_GLOBAL(jbconf));

			ast_module_ref (self_module());

			return channel;
		}
		cpvt_free(cpvt);
	}
	return NULL;
}

/* NOTE: bg: hmm ast_queue_control() say no need channel lock, trylock got deadlock up to 30 seconds here */
/* NOTE: called from device and current levels with pvt locked */
EXPORT_DEF int queue_control_channel (struct cpvt * cpvt, enum ast_control_frame_type control)
{
/*
	for (;;)
	{
*/
		if (cpvt->channel)
		{
/*
			if (ast_channel_trylock (cpvt->channel))
			{
				DEADLOCK_AVOIDANCE (&cpvt->pvt->lock);
			}
			else
			{
*/
				ast_queue_control (cpvt->channel, control);
/*
				ast_channel_unlock (cpvt->channel);
				break;
			}
*/
		}
/*
		else
		{
			break;
		}
	}
*/
	return 0;
}

/* NOTE: bg: hmm ast_queue_hangup() say no need channel lock before call, trylock got deadlock up to 30 seconds here */
/* NOTE: bg: called from device level and change_channel_state() with pvt locked */
EXPORT_DEF int queue_hangup(struct ast_channel* channel, int hangupcause)
{
	int rv = 0;
	if(channel)
	{
		if (hangupcause != 0)
			ast_channel_hangupcause_set(channel, hangupcause);

		rv = ast_queue_hangup (channel);
	}
	return rv;
}

#/* NOTE: bg: called from device level with pvt locked */
EXPORT_DEF void start_local_channel (struct pvt* pvt, const char* exten, const char* number, channel_var_t* vars)
{
	struct ast_channel*	channel;
	int			cause = 0;
	char			channel_name[1024];

	snprintf (channel_name, sizeof (channel_name), "%s@%s", exten, CONF_SHARED(pvt, context));

#if ASTERISK_VERSION_NUM >= 100000 /* 10+ */
	channel = ast_request ("Local", chan_dongle_format_cap, NULL, channel_name, &cause);
#elif ASTERISK_VERSION_NUM >= 10800 /* 1.8+ */
	channel = ast_request ("Local", AST_FORMAT_AUDIO_MASK, NULL, channel_name, &cause);
#else /* 1.8- */
	channel = ast_request ("Local", AST_FORMAT_AUDIO_MASK, channel_name, &cause);
#endif
	if (channel)
	{
		set_channel_vars(pvt, channel);
		ast_set_callerid (channel, number, PVT_ID(pvt), number);

		for(; vars->name; ++vars)
			pbx_builtin_setvar_helper (channel, vars->name, vars->value);

		cause = ast_pbx_start (channel);
		if (cause)
		{
			ast_hangup (channel);
			ast_log (LOG_ERROR, "[%s] Unable to start pbx on channel Local/%s\n", PVT_ID(pvt), channel_name);
		}
	}
	else
	{
		ast_log (LOG_ERROR, "[%s] Unable to request channel Local/%s\n", PVT_ID(pvt), channel_name);
	}
}

#/* */
static int channel_func_read(struct ast_channel* channel, attribute_unused const char* function, char* data, char* buf, size_t len)
{
	struct cpvt* cpvt = ast_channel_tech_pvt(channel);
	struct pvt* pvt;
	int ret = 0;

	if(!cpvt || !cpvt->pvt)
	{
		ast_log (LOG_WARNING, "call on unreferenced %s\n", ast_channel_name(channel));
		return -1;
	}
	pvt = cpvt->pvt;

	if (!strcasecmp(data, "callstate"))
	{
		while (ast_mutex_trylock (&pvt->lock))
		{
			CHANNEL_DEADLOCK_AVOIDANCE (channel);
		}
		call_state_t state = cpvt->state;
		ast_mutex_unlock(&pvt->lock);

		ast_copy_string(buf, call_state2str(state), len);
	}
/*
	else if (!strcasecmp(data, "calls"))
	{
		char buffer[20];
		while (ast_mutex_trylock (&pvt->lock))
		{
			CHANNEL_DEADLOCK_AVOIDANCE (channel);
		}
		unsigned calls = pvt->chansno;
		ast_mutex_unlock(&pvt->lock);

		snprintf(buffer, sizeof(buffer), "%u", calls);
		ast_copy_string(buf, buffer, len);
	}
*/
	else if (!strcasecmp(data, "dtmf"))
	{
		while (ast_mutex_trylock (&pvt->lock))
		{
			CHANNEL_DEADLOCK_AVOIDANCE (channel);
		}
		const char * dtmf = dc_dtmf_setting2str(pvt->real_dtmf);
		ast_mutex_unlock(&pvt->lock);

		ast_copy_string(buf, dtmf, len);
	}
	else
		ret = -1;

	return ret;
}

#/* */
static int channel_func_write(struct ast_channel* channel, const char* function, char* data, const char* value)
{
	struct cpvt* cpvt = ast_channel_tech_pvt(channel);
	struct pvt* pvt = cpvt->pvt;
	call_state_t newstate, oldstate;
	int ret = 0;

	if(!cpvt || !cpvt->pvt)
	{
		ast_log (LOG_WARNING, "call on unreferenced %s\n", ast_channel_name(channel));
		return -1;
	}

	if (!strcasecmp(data, "callstate"))
	{
		if (!strcasecmp(value, "active"))
		{
			newstate = CALL_STATE_ACTIVE;
		}
		else
		{
			ast_log(LOG_WARNING, "Invalid value for %s(callstate).", function);
			return -1;
		}

		while (ast_mutex_trylock (&cpvt->pvt->lock))
		{
			CHANNEL_DEADLOCK_AVOIDANCE (channel);
		}
		oldstate = cpvt->state;

		if (oldstate == newstate)
			;
		else if (oldstate == CALL_STATE_ONHOLD)
		{
			if(at_enque_activate(cpvt))
			{
				/* TODO: handle error */
				ast_log(LOG_ERROR, "Error state to active for call idx %d in %s(callstate).", cpvt->call_idx, function);
			}
		}
		else
		{
			ast_log(LOG_WARNING, "allow change state to 'active' only from 'held' in %s(callstate).", function);
			ret = -1;
		}
		ast_mutex_unlock(&cpvt->pvt->lock);
	}
	else if (!strcasecmp(data, "dtmf"))
	{
		int val = dc_dtmf_str2setting(value);

		if(val >= 0)
		{
			while (ast_mutex_trylock (&cpvt->pvt->lock))
			{
				CHANNEL_DEADLOCK_AVOIDANCE (channel);
			}

			if((dc_dtmf_setting_t)val != pvt->real_dtmf)
			{
				pvt_dsp_setup(pvt, PVT_ID(pvt), val);
			}
			
			ast_mutex_unlock(&cpvt->pvt->lock);
		}
		else
		{
			ast_log(LOG_WARNING, "Invalid value for %s(dtmf).", function);
			return -1;
		}
	}
	else
		ret = -1;

	return ret;
}

EXPORT_DEF struct ast_channel_tech channel_tech =
{
	.type			= "Dongle",
	.description		= MODULE_DESCRIPTION,
#if ASTERISK_VERSION_NUM < 100000 /* 10- */
	.capabilities		= AST_FORMAT_SLINEAR,
#endif
	.requester		= channel_request,
	.call			= channel_call,
	.hangup			= channel_hangup,
	.answer			= channel_answer,
	.send_digit_begin	= channel_digit_begin,
	.send_digit_end		= channel_digit_end,
	.read			= channel_read,
	.write			= channel_write,
	.exception		= channel_read,
	.fixup			= channel_fixup,
	.devicestate		= channel_devicestate,
	.indicate		= channel_indicate,
	.func_channel_read	= channel_func_read,
	.func_channel_write	= channel_func_write
};
