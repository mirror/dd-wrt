/*
 * out_audio.c		Save events
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
 * 			   Claudio Mettler <claudio@fnord.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <bmon/bmon.h>
#include <bmon/node.h>
#include <bmon/output.h>
#include <bmon/graph.h>
#include <bmon/input.h>
#include <bmon/utils.h>
#include <inttypes.h>

#include <alsa/asoundlib.h>

static int c_max = 1000;
static int divisor;

static snd_seq_t *seq_handle;
static snd_seq_event_t ev;

#define CELL_MAX 127.0f

void audio_draw(void)
{
	rate_cnt_t rx, tx;
	stat_attr_t *a;
	item_t *it = get_current_item();

	if (it == NULL)
		return;

	a = current_attr(it->i_graph_sel);
	if (a == NULL)
		return;

	rx = attr_get_rx_rate(a);
	tx = attr_get_tx_rate(a);

	if (rx > c_max)
		rx = c_max;

	if (tx > c_max)
		tx = c_max;

	rx /= divisor;
	tx /= divisor;

	snd_seq_ev_set_noteon(&ev, 0, rx, 127);
	snd_seq_event_output_direct(seq_handle, &ev);
	snd_seq_ev_set_noteoff(&ev, 0, rx, 127);
	snd_seq_event_output_direct(seq_handle, &ev);

	snd_seq_ev_set_noteon(&ev, 1, tx, 127);
	snd_seq_event_output_direct(seq_handle, &ev);
	snd_seq_ev_set_noteoff(&ev, 1, tx, 127);
	snd_seq_event_output_direct(seq_handle, &ev);
}

static void print_module_help(void)
{
	printf(
	"Audio - Audio Output\n" \
	"\n" \
	"  Outputs the currently selected attribute rate as MIDI\n" \
	"  sequence. OUT ::= (max(RATE,MAX)) / (MAX/127)\n" \
	"\n" \
	"  Authors: Claudio Mettler <claudio@fnord.ch>\n" \
	"           Thomas Graf <tgraf@suug.ch>\n" \
	"\n" \
	"  Options:\n" \
	"    max=NUM          Upper limit\n");
}

static void audio_set_opts(tv_t *attrs)
{
	while (attrs) {
		if (!strcasecmp(attrs->type, "max") && attrs->value)
			c_max = strtol(attrs->value, NULL, 0);
		else if (!strcasecmp(attrs->type, "help")) {
			print_module_help();
			exit(0);
		}
		
		attrs = attrs->next;
	}
}

static void audio_shutdown(void)
{
}

static int audio_probe(void)
{
	snd_seq_addr_t src;
	
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
		quit("Failed to access the ALSA sequencer.");
		return 0;
	}

	snd_seq_set_client_name(seq_handle, "bmonmidibridge");
	src.client = snd_seq_client_id(seq_handle);
	src.port = snd_seq_create_simple_port(seq_handle, "Output",
		SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
		SND_SEQ_PORT_TYPE_APPLICATION);

	
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, src.port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);

	divisor = ceil(((double) c_max / CELL_MAX));

	return 1;
}

static struct output_module audio_ops = {
	.om_name = "audio",
	.om_draw = audio_draw,
	.om_set_opts = audio_set_opts,
	.om_probe = audio_probe,
	.om_shutdown audio_shutdown,
};

static void __init audio_init(void)
{
	register_secondary_output_module(&audio_ops);
}
