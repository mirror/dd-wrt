/*
 * src/bmon.c		   Bandwidth Monitor
 *
 * Copyright (c) 2001-2005 Thomas Graf <tgraf@suug.ch>
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
#include <bmon/conf.h>
#include <bmon/item.h>
#include <bmon/itemtab.h>
#include <bmon/input.h>
#include <bmon/output.h>
#include <bmon/signal.h>
#include <bmon/utils.h>

int start_time;
int do_quit = 0;

struct reader_timing rtiming;
static char *usage_text =
"Usage: bmon [OPTION]...\n" \
"\n" \
"Options:\n" \
"   -i <modparm>    Primary input module\n" \
"   -o <modparm>    Primary ouptut module\n" \
"   -I <modparm>    Secondary input modules\n" \
"   -O <modparm>    Secondary output modules\n" \
"   -f <path>       Alternative path to configuration file\n" \
"   -p <policy>     Interface acceptance policy\n" \
"   -a              Accept interfaces even if they are down\n" \
"   -A <attrs>      Attributes history configuration\n" \
"   -r <float>      Read interval in seconds\n" \
"   -R <float>      Rate interval in seconds\n" \
"   -H <hbeat>      Heartbeat factor (0..1)\n" \
"   -L <lifetime>   Lifetime of a item in seconds\n" \
"   -c              Use SI units\n" \
"   -t <path>       Alternative path to itemtab file\n" \
"   -N <num>        Number of graphs to draw\n" \
"   -s <float>      Sleep time in seconds\n" \
"   -w              Signal driven output intervals\n" \
"   -S <pid>        Send SIGUSR1 to a running bmon instance\n" \
"   -u <uid>        Drop privileges and change UID\n" \
"   -g <gid>        Drop privileges and change GID\n" \
"   -h              show this help text\n" \
"   -V              show version\n" \
"\n" \
"Module configuration:\n" \
"   modparm := MODULE:optlist,MODULE:optlist,...\n" \
"   optlist := option;option;...\n" \
"   option  := TYPE[=VALUE]\n" \
"\n" \
"   Examples:\n" \
"       -O html:path=/var/www/html,distribution:port=2444;debug\n" \
"       -O list          # Shows a list of available modules\n" \
"       -O html:help     # Shows a help text for html module\n" \
"\n" \
"Interface selection:\n" \
"   policy  := [!]simple_regexp,[!]simple_regexp,...\n" \
"\n" \
"   Example: -p 'eth*,lo*,!eth1'\n" \
"\n" \
"Attributes selection:\n" \
"   attrs   := [!]name,[!]name,...\n" \
"\n" \
"   Example: -A !hbeat_err\n" \
"\n" \
"Please see the bmon(1) man pages for full documentation.\n";

static char *uid, *gid;

static void do_shutdown(void)
{
	static int done;
	
	if (!done) {
		done = 1;
		input_shutdown();
		output_shutdown();
	}
}

RETSIGTYPE sig_int(int unused)
{
	if (do_quit)
		exit(-1);
	do_quit = 1;
}

void sig_exit(void)
{
	do_shutdown();
}

void quit(const char *fmt, ...)
{
	static int done;
	va_list args;

	if (!done) {
		done = 1;
		do_shutdown();
	}

	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	exit(1);
}

static void print_version(void)
{
	printf("bmon %s\n", PACKAGE_VERSION);
	printf("Copyright (C) 2001-2005 by Thomas Graf <tgraf@suug.ch>\n");
	printf("bmon comes with ABSOLUTELY NO WARRANTY. This is free software, " \
		"and you\nare welcome to redistribute it under certain conditions. " \
		"See the source\ncode for details.\n");
}

static void parse_args(int argc, char *argv[])
{
	for (;;)
	{
		int c = getopt(argc, argv, "hVf:i:I:o:O:p:r:s:S:waA:cN" \
		    ":u:g:H:R:L:t:");

		if (c == -1)
			break;
		
		switch (c)
		{
			case 'i':
				set_input(optarg);
				break;

			case 'I':
				set_sec_input(optarg);
				break;

			case 'o':
				set_output(optarg);
				break;

			case 'O':
				set_sec_output(optarg);
				break;
				
			case 'f':
				set_configfile(optarg);
				break;

			case 'p':
				item_parse_policy(optarg);
				break;

			case 'A':
				parse_attr_policy(optarg);
				break;

			case 'r':
				set_read_interval(optarg);
				break;

			case 'R':
				set_rate_interval(optarg);
				break;

			case 'L':
				set_lifetime(optarg);
				break;

			case 's':
				set_sleep_time(optarg);
				break;

			case 'S':
				send_signal(optarg);
				exit(0);

			case 'w':
				set_signal_output(1);
				break;

			case 'a':
				set_show_only_running(0);
				break;

			case 'h':
				print_version();
				printf("\n%s", usage_text);
				exit(1);

			case 'N':
				set_ngraphs(strtol(optarg, NULL, 0));
				break;

			case 'c':
				set_use_si();
				break;
			
			case 'u':
				uid = strdup(optarg);
				break;

			case 'g':
				gid = strdup(optarg);
				break;

			case 'H':
				set_hb_factor(optarg);
				break;

			case 't':
				set_itemtab(optarg);
				break;

			case 'V':
			case 'v':
				print_version();
				exit(0);

			default:
				quit("Aborting...\n");
		}
	}
}

static void calc_variance(timestamp_t *c, timestamp_t *ri)
{
	float v = (ts_to_float(c) / ts_to_float(ri)) * 100.0f;

	rtiming.rt_variance.v_error = v;
	rtiming.rt_variance.v_total += v;

	if (v > rtiming.rt_variance.v_max)
		rtiming.rt_variance.v_max = v;

	if (v < rtiming.rt_variance.v_min)
		rtiming.rt_variance.v_min = v;
}

static void drop_privs(void)
{
	struct passwd *uentry = NULL;
	struct group *gentry = NULL;

	if (gid)
		gentry = getgrnam(gid);

	if (uid)
		uentry = getpwnam(uid);

	if (gentry)
		if (setgid(gentry->gr_gid) < 0)
			quit("Unable to set group id %d: %s\n",
			    gentry->gr_gid, strerror(errno));

	if (uentry)
		if (setuid(uentry->pw_uid) < 0)
			quit("Unable to set user id %d: %s\n",
			    uentry->pw_uid, strerror(errno));
}

int main(int argc, char *argv[])
{
	unsigned long c_sleep_time;
	float read_interval;

	start_time = time(0);

	parse_args(argc, argv);
	read_configfile();
	read_itemtab();

	c_sleep_time = get_sleep_time();
	read_interval = get_read_interval();

	if (((double) c_sleep_time/1000000.0f) > read_interval)
		c_sleep_time = (unsigned long) (read_interval * 1000000.0f);

	input_init();
	output_init();

	drop_privs();

	{
		/*
		 * E  := Elapsed time
		 * NR := Next Read
		 * LR := Last Read
		 * RI := Read Interval
		 * ST := Sleep Time
		 * C  := Correction
		 */
		timestamp_t e, ri, tmp;
		unsigned long st;

		get_read_interval_as_ts(&ri);

		/*
		 * E := NOW()
		 */
		update_ts(&e);
		
		/*
		 * NR := E
		 */
		COPY_TS(&rtiming.rt_next_read, &e);
		
		for (;;) {
			output_pre();

			/*
			 * E := NOW()
			 */
			update_ts(&e);

			/*
			 * IF NR <= E THEN
			 */
			if (ts_le(&rtiming.rt_next_read, &e)) {
				timestamp_t c;

				/*
				 * C :=  (NR - E)
				 */
				ts_sub(&c, &rtiming.rt_next_read, &e);

				calc_variance(&c, &ri);

				/*
				 * LR := E
				 */
				COPY_TS(&rtiming.rt_last_read, &e);

				/*
				 * NR := E + RI + C
				 */
				ts_add(&rtiming.rt_next_read, &e, &ri);
				ts_add(&rtiming.rt_next_read, &rtiming.rt_next_read, &c);

				input_read();
				output_draw();

				output_post();
			}

			if (do_quit)
				exit(0);

			if (got_resized())
				output_resize();

			/*
			 * ST := Configured ST
			 */
			st = c_sleep_time;

			/*
			 * IF (NR - E) < ST THEN
			 */
			ts_sub(&tmp, &rtiming.rt_next_read, &e);

			if (tmp.tv_sec < 0)
				continue;

			if (tmp.tv_sec == 0 && tmp.tv_usec < st) {
				if (tmp.tv_usec < 0)
					continue;
				/*
				 * ST := (NR - E)
				 */
				st = tmp.tv_usec;
			}
			
			/*
			 * SLEEP(ST)
			 */
			usleep(st);
		}
	}
	
	return 0; /* buddha says i'll never be reached */
}

static void __init bmon_init(void)
{
	memset(&rtiming, 0, sizeof(rtiming));
	rtiming.rt_variance.v_min = 10000000.0f;
	atexit(&sig_exit);
	signal(SIGINT, &sig_int);
}
