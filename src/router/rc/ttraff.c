/* 
 * ttraff.c by Eko: 12.feb.2008
 * 
 * used for collecting and storing WAN traffic info to nvram
 * 
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <cy_conf.h>
#include <rc.h>
#include <shutils.h>
#include <syslog.h>
#include <utils.h>

void remove_oldest_entry(int cur_month, int cur_year)
{
	char old[32];
	int month, year;
	int len = 0;

	for (year = 2008; year <= cur_year; year++) {
		for (month = 1; month <= 12; month++) {
			if (month == cur_month && year == cur_year) {
				return;
			}
			snprintf(old, sizeof(old), "traff-%02u-%u", month, year);
			len = strlen(nvram_safe_get(old));
			if (len > 0) {
				dd_syslog(LOG_DEBUG, "ttraff: old data for %d-%d removed, freeing %d bytes of nvram\n", month, year,
					  len + 15);
				nvram_unset(old);
				return;
			}
		}
	}

	return;
}

static int checkbuffer(char **buffer, char *var, int buffersize)
{
	if (buffersize >= ((buffersize - 1) - (strlen(var) + 1))) {
		buffersize += 2048;
		*buffer = (char *)realloc(*buffer, buffersize);
	}
	return buffersize;
}

void write_to_nvram(int day, int month, int year, unsigned long long rcvd, unsigned long long sent)
{
	char *next;
	char var[80];
	char tq[32];
	char temp[64] = "";
	char sbuff[256] = "";
	char *buffer;
	int i = 1, d = 1;
	unsigned int days = daysformonth(month, year);
	//      fprintf(stderr,"days %d, month %d, year %d\n",days,month,year);
	unsigned long long old_rcvd;
	unsigned long long old_sent;
	char *tdata;
	int buffersize = 2048;
	buffer = (char *)malloc(buffersize);
	memset(buffer, 0, buffersize);
	snprintf(tq, sizeof(tq), "traff-%02u-%u", month, year);

	/* keep some nvram free by removing oldest traf data */
	int space = 0;
	int used = nvram_used(&space);
	if ((space - used) < 2048) { /* 2048 bytes to be on a safe side */
		remove_oldest_entry(month, year);
	}

	tdata = nvram_safe_get(tq);

	if (!*tdata) {
		for (d = 1; d <= days; d++) {
			strcat(sbuff, "0:0 ");
		}
		strcat(sbuff, "[0:0]");
		nvram_set(tq, sbuff);
		nvram_async_commit(); // invalidate them
		tdata = nvram_safe_get(tq);
	}

	foreach(var, tdata, next)
	{
		if (i == day) {
			if (strstr(var,
				   "[")) { //check and correct faulty entries
				snprintf(temp, sizeof(temp), "%llu:%llu ", rcvd, sent);
			} else { //value OK
				sscanf(var, "%llu:%llu", &old_rcvd, &old_sent);
				snprintf(temp, sizeof(temp), "%llu:%llu ", old_rcvd + rcvd, old_sent + sent);
			}
			buffersize = checkbuffer(&buffer, temp, buffersize);
			snprintf(buffer, buffersize, "%s%s", buffer, temp);
		} else if (i == (days + 1)) //make new monthly total
		{
			sscanf(var, "[%llu:%llu]", &old_rcvd, &old_sent);
			snprintf(temp, sizeof(temp), "[%llu:%llu] ", old_rcvd + rcvd, old_sent + sent);
			buffersize = checkbuffer(&buffer, temp, buffersize);
			snprintf(buffer, buffersize, "%s%s", buffer, temp);
			i++;
			break;
		} else {
			if (strchr(var, ':') != NULL) {
				buffersize = checkbuffer(&buffer, var, buffersize);
				snprintf(buffer, buffersize, "%s%s", buffer, var);
				buffersize = checkbuffer(&buffer, " ", buffersize);
				snprintf(buffer, buffersize, "%s ", buffer);
			}
		}
		i++;
	}
	int a;
	/* correct entries if something strange happend */
	if (i < (days + 2)) {
		for (a = i; a <= days; a++) {
			buffersize = checkbuffer(&buffer, "0:0 ", buffersize);
			snprintf(buffer, buffersize, "%s0:0 ", buffer);
		}
		buffersize = checkbuffer(&buffer, "[0:0] ", buffersize);
		snprintf(buffer, buffersize, "%s[0:0] ", buffer);
	}
	strtrim_right(buffer, ' ');
	nvram_set(tq, buffer);
	free(buffer);
	return;
}

int main(int argc, char **argv)
{
	char wan_if_buffer[33];
	switch (fork()) {
	case -1:
		// can't fork
		exit(0);
		break;
	case 0:
		/* 
		 * child process 
		 */
		// fork ok
		(void)setsid();
		break;
	default:
		/* 
		 * parent process should just die 
		 */
		_exit(0);
	}

	struct tm *currtime;
	long int tloc;

	time(&tloc); // get time in seconds since epoch
	currtime = localtime(&tloc); // convert seconds to date structure

	while (currtime->tm_year < 100) // loop until ntp time is set (year
	// >= 2000)
	{
		sleep(15);
		time(&tloc);
		currtime = localtime(&tloc);
	}

	/* 
	 * now we have time, let's start 
	 */
	char wanface[32];
	char line[256];
	unsigned long long in_dev = 0;
	unsigned long long out_dev = 0;
	unsigned long long in_diff = 0;
	unsigned long long out_diff = 0;
	unsigned long long in_dev_last = 0;
	unsigned long long out_dev_last = 0;
	int gotbase = 0;
	unsigned long long megcounti, megcounto;
	unsigned long long megi = 0;
	unsigned long long mego = 0;
	int needcommit = 0;
	int commited = 0;
	int day, month, year;
	int ifl;
	FILE *in;

	if (nvram_match("ttraff_iface", "") || !nvram_exists("ttraff_iface"))
		strncpy(wanface, safe_get_wan_face(wan_if_buffer), sizeof(wanface));
	else
		strncpy(wanface, nvram_safe_get("ttraff_iface"), sizeof(wanface));
	strcat(wanface, ":");
	/* 
	 * now we can loop and collect data 
	 */

	dd_syslog(LOG_DEBUG, "ttraff: data collection started\n");

	do {
		time(&tloc);
		currtime = localtime(&tloc);

		day = currtime->tm_mday;
		month = currtime->tm_mon + 1; // 1 - 12
		year = currtime->tm_year + 1900;
		if ((in = fopen("/proc/net/dev", "rb")) != NULL) {
			/* eat first two lines */
			fgets(line, sizeof(line), in);
			fgets(line, sizeof(line), in);

			while (fgets(line, sizeof(line), in) != NULL) {
				ifl = 0;

				if (strstr(line, wanface)) {
					while (line[ifl] != ':')
						ifl++;
					line[ifl] = 0;

					sscanf(line + ifl + 1,
					       "%llu %*llu %*llu %*llu %*llu %*llu %*llu %*llu %llu %*llu %*llu %*llu %*llu %*llu %*llu %*llu",
					       &in_dev, &out_dev);
				}
			}

			fclose(in);
		}

		if (gotbase == 0) {
			in_dev_last = in_dev;
			out_dev_last = out_dev;
			gotbase = 1;
		}

		if (in_dev_last > in_dev) // 4GB limit was reached or couter
		// reseted
		{
			megi = (in_dev_last >> 20) + (in_dev >> 20); // to avarage
			//
			//
			//
			//
			//
			//
			//
			// loss and
			// gain here
			// to 0 over
			// long time
			in_diff = (in_dev >> 20) * 2; // to avarage loss and gain
			// here to 0 over long time
			in_dev_last = in_dev;
		} else {
			in_diff = (in_dev - in_dev_last) >> 20; // MB
			in_dev_last += (in_diff << 20);
		}

		if (out_dev_last > out_dev) // 4GB limit was reached or counter
		// reseted
		{
			mego = (out_dev_last >> 20) + (out_dev >> 20); // to avarage
			//
			//
			//
			//
			//
			//
			//
			// loss and
			// gain here
			// to 0 over
			// long time
			out_diff = (out_dev >> 20) * 2; // to avarage loss and gain
			// here to 0 over long time
			out_dev_last = out_dev;
		} else {
			out_diff = (out_dev - out_dev_last) >> 20; // MB
			out_dev_last += (out_diff << 20);
		}

		//              fprintf (stderr, "in_diff=%lu, out_diff=%lu\n", in_diff,
		// out_diff);

		if (in_diff || out_diff) {
			write_to_nvram(day, month, year, in_diff, out_diff);
		}

		if (megi || mego) // leave trace in /tmp/.megc
		{
			megcounti = 0;
			megcounto = 0;
			if ((in = fopen("/tmp/.megc", "r")) != NULL) {
				fgets(line, sizeof(line), in);
				sscanf(line, "%llu:%llu", &megcounti, &megcounto);
				fclose(in);
			}
			in = fopen("/tmp/.megc", "w");
			sprintf(line, "%llu:%llu", megcounti + megi, megcounto + mego);
			fputs(line, in);
			fclose(in);
			megi = 0;
			mego = 0;
		}

		if (currtime->tm_hour == 23 && currtime->tm_min == 59 && commited == 0) {
			needcommit = 1;
		} else {
			commited = 0;
		}

		if (needcommit) // commit only 1 time per day (at 23:59)
		{
			nvram_async_commit();
			commited = 1;
			needcommit = 0;
			dd_syslog(LOG_DEBUG, "ttraff: data for %d-%d-%d commited to nvram\n", day, month, year);
		}

		sleep(58);

	} while (1);

	return 0;
}
