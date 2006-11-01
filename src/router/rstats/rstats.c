/*

	rstats
	Copyright (C) 2006 Jonathan Zarate

	Licensed under GNU GPL v2 or later.
	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <stdint.h>
#include <syslog.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <shared.h>

#define _dprintf(args...)	do { } while (0)
//	#define _dprintf(args...)	cprintf(args)

#define K 1024
#define M (1024 * 1024)
#define G (1024 * 1024 * 1024)

#define SMIN	60
#define	SHOUR	(60 * 60)
#define	SDAY	(60 * 60 * 24)
#define Y2K		946684800UL

#define INTERVAL		60

#define MAX_NSPEED		((5 * SHOUR) / INTERVAL)
#define MAX_NDAILY		62
#define MAX_NMONTHLY	25
#define MAX_SPEED_IF	7
#define MAX_ROLLOVER	(225 * M)

#define MAX_COUNTER	2
#define RX 			0
#define TX 			1

#define DAILY		0
#define MONTHLY		1

#define ID_V0		0x30305352
#define ID_V1		0x31305352
#define CURRENT_ID	ID_V1


typedef struct {
	uint32_t xtime;
	uint64_t counter[MAX_COUNTER];
} data_t;

typedef struct {
	unsigned long id;
	
	data_t daily[MAX_NDAILY];
	int dailyp;

	data_t monthly[MAX_NMONTHLY];
	int monthlyp;
} history_t;

typedef struct {
	unsigned long id;
	
	data_t daily[62];
	int dailyp;

	data_t monthly[12];
	int monthlyp;
} history_v0_t;

typedef struct {
	char ifname[12];
	long utime;
	unsigned long speed[MAX_NSPEED][MAX_COUNTER];
	unsigned long last[MAX_COUNTER];
	int tail;
	char sync;
} speed_t;

history_t history;
speed_t speed[MAX_SPEED_IF];
int speed_count;
long save_utime;
char save_path[96];
volatile int gothup = 0;
volatile int gotuser = 0;
volatile int gotterm = 0;

const char history_fn[] = "/var/lib/misc/rstats-history";
const char speed_fn[] = "/var/lib/misc/rstats-speed";
const char uncomp_fn[] = "/var/tmp/rstats-uncomp";



static int get_stime(void)
{
	int t;

	t = atoi(nvram_safe_get("rstats_stime"));
	if (t < 1) t = 1;
		else if (t > 8760) t = 8760;
	return t * SHOUR;
}

static int comp(const char *path, void *buffer, int size)
{
	char s[256];

	if (f_write(path, buffer, size, 0, 0) != size) return 0;
	
	sprintf(s, "%s.gz", path);
	unlink(s);

	sprintf(s, "gzip %s", path);
	return system(s) == 0;
}

static void save(int exiting)
{
	int i;
	time_t now;
	char *bi, *bo;
	int n;
	char hgz[256];
	char tmp[128];

	_dprintf("save\n");

	f_write("/var/lib/misc/rstats-stime", &save_utime, sizeof(save_utime), 0, 0);

	comp(speed_fn, speed, sizeof(speed[0]) * speed_count);
	
	if ((now = time(0)) < Y2K) {
		_dprintf("time not set\n");
		return;
	}
	
	comp(history_fn, &history, sizeof(history));
	
	if (exiting) {
		_dprintf("exiting=1\n");
		return;
	}

	_dprintf("save ext\n");
	
	sprintf(hgz, "%s.gz", history_fn);
	
	if (strcmp(save_path, "*nvram") == 0) {
		if (!wait_action_idle(10)) {
			_dprintf("busy, not saving\n");
			return;
		}

		if ((n = f_read_alloc(hgz, &bi, 20 * 1024)) > 0) {
			if ((bo = malloc(base64_encoded_len(n) + 1)) != NULL) {
				n = base64_encode(bi, bo, n);
				bo[n] = 0;
				_dprintf("rstats_data=%s\n", bo);
				nvram_set("rstats_data", bo);
				if (!nvram_match("debug_nocommit", "1")) nvram_commit();
				free(bo);
			}
		}
		free(bi);
	}
	else if (save_path[0] != 0) {
		strcpy(tmp, save_path);
		strcat(tmp, "_tmp");

		for (i = 15; i > 0; --i) {
			if (!wait_action_idle(10)) {
				_dprintf("busy, not saving\n");
			}
			else {
				_dprintf("copy %s to %s\n", hgz, save_path);
				if (eval("cp", hgz, tmp) == 0) {
					_dprintf("copy ok\n");
					if (rename(tmp, save_path) == 0) {
						_dprintf("rename ok\n");
						break;
					}
				}
			}

			// might not be ready
			sleep(3);
		}
	}
}

static int decomp(const char *path, void *buffer, int size, int max)
{
	char s[256];
	int n;

	unlink(uncomp_fn);

	n = 0;
	sprintf(s, "gzip -dc %s.gz > %s", path, uncomp_fn);
	if (system(s) == 0) {
		n = f_read(uncomp_fn, buffer, size * max);
		_dprintf("decomp n = %d\n", n);
		if (n <= 0) n = 0;
			else n = n / size;
	}
	else {
		_dprintf("%s != 0\n", s);
	}
	unlink(uncomp_fn);
	memset((char *)buffer + (size * n), 0, (max - n) * size);	
	return n;
}

static int load_history(void)
{
	if ((decomp(history_fn, &history, sizeof(history), 1) != 1) || (history.id != CURRENT_ID)) {
		memset(&history, 0, sizeof(history));
		
		history_v0_t v0;
		
		if ((decomp(history_fn, &v0, sizeof(v0), 1) != 1) || (v0.id != ID_V0)) {
			syslog(LOG_INFO, "Unable to load history, clearing...");
			history.id = CURRENT_ID;
			return 0;
		}
		else {
			// --- temp conversion ---

			// V0 -> V1
			history.id = CURRENT_ID;
			memcpy(history.daily, v0.daily, sizeof(history.daily));
			history.dailyp = v0.dailyp;
			memcpy(history.monthly, v0.monthly, sizeof(v0.monthly));	// v0 is just shorter
			history.monthlyp = v0.monthlyp;
		}
	}
	else {
		_dprintf("history loaded d=%d m=%d\n", history.dailyp, history.monthlyp);
	}

	return 1;
}

static void load(long uptime)
{
	int i;
	long t;
	char *bi, *bo;
	int n;
	char hgz[256];
	unsigned char mac[6];

	strlcpy(save_path, nvram_safe_get("rstats_path"), sizeof(save_path) - 32);
	if (((n = strlen(save_path)) > 0) && (save_path[n - 1] == '/')) {
		ether_atoe(nvram_safe_get("et0macaddr"), mac);
		sprintf(save_path + n, "ddwrt_rstats_%02x%02x%02x%02x%02x%02x.gz",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}

	if (f_read("/var/lib/misc/rstats-stime", &save_utime, sizeof(save_utime)) != sizeof(save_utime)) {
		save_utime = 0;
	}
	t = uptime + get_stime();
	if ((save_utime < uptime) || (save_utime > t)) save_utime = t;
	_dprintf("uptime = %dm, save_utime = %dm\n", uptime / 60, save_utime / 60);

	speed_count = decomp(speed_fn, speed, sizeof(speed[0]), MAX_SPEED_IF);
	_dprintf("load speed_count = %d\n", speed_count);
	
	if ((load_history()) || (save_path[0] == 0)) return;

	sprintf(hgz, "%s.gz", history_fn);

	if (strcmp(save_path, "*nvram") == 0) {
		if (!wait_action_idle(60)) exit(0);
		
		bi = nvram_safe_get("rstats_data");
		if ((n = strlen(bi)) > 0) {
			if ((bo = malloc(base64_decoded_len(n))) != NULL) {
				n = base64_decode(bi, bo, n);
				f_write(hgz, bo, n, 0, 0);
				free(bo);
				_dprintf("from nvram = %d\n", n);
			}
		}
	}
	else {
		for (i = 30; i > 0; --i) {
			if (!wait_action_idle(60)) exit(0);

			_dprintf("copy %s to %s\n", save_path, hgz);
			if (eval("cp", save_path, hgz) == 0) break;

			// not ready...
			sleep(3);
		}
		if (i == 0) {
			save_utime = uptime + 60;
			_dprintf("forcing early save on next\n");
		}
	}

	load_history();
}

static void save_speedjs(void)
{
	int i, j, k;
	speed_t *sp;
	int p;
	FILE *f;
	uint64_t tavg;
	uint64_t tmax;
	unsigned long n;

	if ((f = fopen("/var/tmp/rstats-speed.js", "w")) == NULL) return;

	_dprintf("speed_count = %d\n", speed_count);
	
	fprintf(f, "\nspeed_history = {\n");
	for (i = 0; i < speed_count; ++i) {
		sp = &speed[i];
		fprintf(f, "%s'%s': {\n", i ? " },\n" : "", sp->ifname);
		for (j = 0; j < MAX_COUNTER; ++j) {
			tavg = tmax = 0;
			fprintf(f, "%sx: [", j ? ",\n t" : " r");
			p = sp->tail;
			for (k = 0; k < MAX_NSPEED; ++k) {
				p = (p + 1) % MAX_NSPEED;
				n = sp->speed[p][j];
				fprintf(f, "%s%lu", k ? "," : "", n);
				tavg += n;
				if (n > tmax) tmax = n;
			}
			fprintf(f, "],\n");
			fprintf(f, " %cx_avg: %llu,\n %cx_max: %llu",
				j ? 't' : 'r', tavg / MAX_NSPEED,
				j ? 't' : 'r', tmax);
		}
	}
	fprintf(f, "%s};\n", speed_count ? "}\n" : "");
	fclose(f);

	rename("/var/tmp/rstats-speed.js", "/var/spool/rstats-speed.js");
}


static void save_datajs(FILE *f, int mode)
{
	data_t *data;
	int p;
	int max;
	int k, kn;

	fprintf(f, "\n%s_history = [\n", (mode == DAILY) ? "daily" : "monthly");

	if (mode == DAILY) {
		data = history.daily;
		p = history.dailyp;
		max = MAX_NDAILY;
	}
	else {
		data = history.monthly;
		p = history.monthlyp;
		max = MAX_NMONTHLY;
	}
	kn = 0;
	for (k = max; k > 0; --k) {
		p = (p + 1) % max;
		if (data[p].xtime == 0) continue;
		fprintf(f, "%s[0x%lx,0x%llx,0x%llx]", kn ? "," : "",
			(unsigned long)data[p].xtime, data[p].counter[0] / K, data[p].counter[1] / K);
		++kn;
	}
	fprintf(f, "];\n");
}

static void save_histjs(void)
{
	FILE *f;

	if ((f = fopen("/var/tmp/rstats-history.js", "w")) != NULL) {
		save_datajs(f, DAILY);
		save_datajs(f, MONTHLY);
		fclose(f);
		rename("/var/tmp/rstats-history.js", "/var/spool/rstats-history.js");
	}
}


static void bump(data_t *data, int *tail, int max, uint32_t xnow, unsigned long *counter)
{
	int t, i;

	t = *tail;
	if (data[t].xtime != xnow) {
		for (i = max - 1; i >= 0; --i) {
			if (data[i].xtime == xnow) {
				t = i;
				break;
			}
		}
		if (i < 0) {
			*tail = t = (t + 1) % max;
			data[t].xtime = xnow;
			memset(data[t].counter, 0, sizeof(data[0].counter));
		}
	}
	for (i = 0; i < MAX_COUNTER; ++i) {
		data[t].counter[i] += counter[i];
	}
}

static void calc(long uptime)
{
	FILE *f;
	char buf[256];
	char *ifname;
	char *p;
	unsigned long counter[MAX_COUNTER];
	speed_t *sp;
	int i;
	time_t now;
	struct tm *tms;
	uint32_t xnow;
	uint32_t c;
	uint32_t sc;
	unsigned long diff;
	long tick;
	int wan;

	now = time(0);
	tms = localtime(&now);

	if ((f = fopen("/proc/net/dev", "r")) == NULL) return;
	fgets(buf, sizeof(buf), f);	// header
	fgets(buf, sizeof(buf), f);	// "
	while (fgets(buf, sizeof(buf), f)) {
		if ((p = strchr(buf, ':')) == NULL) continue;
		*p = 0;
		if ((ifname = strrchr(buf, ' ')) == NULL) ifname = buf;
			else ++ifname;

		wan = !nvram_match("wan_proto", "disabled") && nvram_match("wan_iface", ifname);
		if ((!wan) && (!nvram_match("lan_ifname", ifname)) && (!nvram_match("wl0_ifname", ifname))) continue;

		// <rx bytes, packets, errors, dropped, fifo errors, frame errors, compressed, multicast><tx ...>
		if (sscanf(p + 1, "%lu%*u%*u%*u%*u%*u%*u%*u%lu", &counter[0], &counter[1]) != 2) continue;

		sp = speed;
		for (i = speed_count; i > 0; --i) {
			if (strcmp(sp->ifname, ifname) == 0) break;
			++sp;
		}
		if (i == 0) {
			if (speed_count >= MAX_SPEED_IF) continue;
			
			_dprintf("add %s as #%d\n", ifname, speed_count);
			sp = &speed[speed_count++];
			memset(sp, 0, sizeof(*sp));
			strcpy(sp->ifname, ifname);
			sp->sync = 1;
		}
		else {
			if (sp->utime == uptime) continue;
			if (sp->utime > uptime) sp->sync = 1;
		}

//		_dprintf("%s: ", ifname);


		tick = uptime - sp->utime;
		sp->utime = uptime;
		sp->tail = (sp->tail + 1) % MAX_NSPEED;

		if (sp->sync) {
			_dprintf("%s sync\n", ifname);

			sp->sync = 0;
			memcpy(sp->last, counter, sizeof(sp->last));
			memset(sp->speed[sp->tail], 0, sizeof(sp->speed[0]));
			memset(counter, 0, sizeof(counter));
		}
		else {
			for (i = 0; i < MAX_COUNTER; ++i) {
				c = counter[i];
				sc = sp->last[i];
				if (c < sc) {
					diff = (0xFFFFFFFF - sc) + c;
					if (diff > MAX_ROLLOVER) diff = 0;
				}
				else {
					 diff = c - sc;
				}
				sp->last[i] = c;
				counter[i] = diff;
				sp->speed[sp->tail][i] = diff / tick;

//				_dprintf("%7lu (%7luK/s) ", diff, sp->speed[sp->tail][i] / K);
			}
		}

//		_dprintf("\n");

		// todo: split, delayed
		
		if ((!wan) || (now < Y2K)) continue;
		
		xnow = (tms->tm_year << 16) | ((uint32_t)tms->tm_mon << 8) | tms->tm_mday;
//		history.time = now;
		bump(history.daily, &history.dailyp, MAX_NDAILY, xnow, counter);
		bump(history.monthly, &history.monthlyp, MAX_NMONTHLY, xnow & 0xFFFF00, counter);
	}
	fclose(f);

	// missing IFs get sync=1, really old ones get removed
	for (i = 0; i < speed_count; ++i) {
		sp = &speed[i];
		if (sp->utime == uptime) continue;
		if ((uptime - sp->utime) > SDAY) {
			--speed_count;
			memcpy(sp, sp + 1, (speed_count - i) * sizeof(speed[0]));
		}
		else {
			sp->sync = 1;
		}
	}

	// todo: total > user
	if (uptime >= save_utime) {
		save(0);
		save_utime = uptime + get_stime();
		_dprintf("uptime = %dm, save_utime = %dm\n", uptime / 60, save_utime / 60);
	}
}


static void sig_handler(int sig)
{
	switch (sig) {
	case SIGTERM:
	case SIGINT:
		gotterm = 1;
		break;
	case SIGHUP:
		gothup = 1;
		break;
	case SIGUSR1:
		gotuser = 1;
		break;
	case SIGUSR2:
		gotuser = 2;
		break;
	}
}

int main(int argc, char *argv[])
{
	struct sysinfo si;
	long z;
	struct sigaction sa;

	printf("rstats\nCopyright (C) 2006 Jonathan Zarate\n\n");
	
	if (fork() != 0) return 0;

	openlog("rstats", LOG_PID, LOG_USER);
	
	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	sysinfo(&si);
	z = si.uptime;

	load(si.uptime);
	
	while (1) {
		while (si.uptime < z) {
			sleep(z - si.uptime);
			if (gothup) {
				if (unlink("/var/tmp/rstats-reload") == 0) {
					load_history();
				}
				else {
					save(0);
				}
				gothup = 0;
			}
			if (gotterm) {
				save(1);
				exit(0);
			}
			if (gotuser == 1) {
				save_speedjs();
				gotuser = 0;
			}
			else if (gotuser == 2) {
				save_histjs();
				gotuser = 0;
			}
			sysinfo(&si);
		}
		calc(si.uptime);
		z += INTERVAL;
	}

	return 0;
}
