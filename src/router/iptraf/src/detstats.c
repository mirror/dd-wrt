/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

detstats.c	- the interface statistics module

 ***/

#include "iptraf-ng-compat.h"

#include "tui/winops.h"

#include "counters.h"
#include "ifaces.h"
#include "fltdefs.h"
#include "packet.h"
#include "options.h"
#include "log.h"
#include "dirs.h"
#include "deskman.h"
#include "attrs.h"
#include "serv.h"
#include "timer.h"
#include "logvars.h"
#include "promisc.h"
#include "error.h"
#include "detstats.h"
#include "rate.h"

struct ifcounts {
	struct proto_counter total;
	struct pkt_counter bcast;
	struct pkt_counter bad;
	struct proto_counter ipv4;
	struct proto_counter ipv6;
	struct proto_counter nonip;

	struct proto_counter tcp;
	struct proto_counter udp;
	struct proto_counter icmp;
	struct proto_counter other;
};

/* USR1 log-rotation signal handlers */
static void rotate_dstat_log(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, current_logfile);
	signal(SIGUSR1, rotate_dstat_log);
}

static void writedstatlog(char *ifname,
		   unsigned long peakactivity, unsigned long peakpps,
		   unsigned long peakactivity_in, unsigned long peakpps_in,
		   unsigned long peakactivity_out, unsigned long peakpps_out,
		   struct ifcounts *ts, unsigned long nsecs, FILE *fd)
{
	char atime[TIME_TARGET_MAX];

	genatime(time(NULL), atime);

	fprintf(fd,
		"\n*** Detailed statistics for interface %s, generated %s\n\n",
		ifname, atime);

	fprintf(fd, "Total: \t%llu packets, %llu bytes\n",
		ts->total.proto_total.pc_packets,
		ts->total.proto_total.pc_bytes);
	fprintf(fd,
		"\t(incoming: %llu packets, %llu bytes; outgoing: %llu packets, %llu bytes)\n",
		ts->total.proto_in.pc_packets,
		ts->total.proto_in.pc_bytes,
		ts->total.proto_out.pc_packets,
		ts->total.proto_out.pc_bytes);
	fprintf(fd, "IP: \t%llu packets, %llu bytes\n",
		ts->ipv4.proto_total.pc_packets,
		ts->ipv4.proto_total.pc_bytes);
	fprintf(fd,
		"\t(incoming: %llu packets, %llu bytes; outgoing: %llu packets, %llu bytes)\n",
		ts->ipv4.proto_in.pc_packets,
		ts->ipv4.proto_in.pc_bytes,
		ts->ipv4.proto_out.pc_packets,
		ts->ipv4.proto_out.pc_bytes);
	fprintf(fd, "TCP: %llu packets, %llu bytes\n",
		ts->tcp.proto_total.pc_packets,
		ts->tcp.proto_total.pc_bytes);
	fprintf(fd,
		"\t(incoming: %llu packets, %llu bytes; outgoing: %llu packets, %llu bytes)\n",
		ts->tcp.proto_in.pc_packets,
		ts->tcp.proto_in.pc_bytes,
		ts->tcp.proto_out.pc_packets,
		ts->tcp.proto_out.pc_bytes);
	fprintf(fd, "UDP: %llu packets, %llu bytes\n",
		ts->udp.proto_total.pc_packets,
		ts->udp.proto_total.pc_bytes);
	fprintf(fd,
		"\t(incoming: %llu packets, %llu bytes; outgoing: %llu packets, %llu bytes)\n",
		ts->udp.proto_in.pc_packets,
		ts->udp.proto_in.pc_bytes,
		ts->udp.proto_out.pc_packets,
		ts->udp.proto_out.pc_bytes);
	fprintf(fd, "ICMP: %llu packets, %llu bytes\n",
		ts->icmp.proto_total.pc_packets,
		ts->icmp.proto_total.pc_bytes);
	fprintf(fd,
		"\t(incoming: %llu packets, %llu bytes; outgoing: %llu packets, %llu bytes)\n",
		ts->icmp.proto_in.pc_packets,
		ts->icmp.proto_in.pc_bytes,
		ts->icmp.proto_out.pc_packets,
		ts->icmp.proto_out.pc_bytes);
	fprintf(fd, "Other IP: %llu packets, %llu bytes\n",
		ts->other.proto_total.pc_packets,
		ts->other.proto_total.pc_bytes);
	fprintf(fd,
		"\t(incoming: %llu packets, %llu bytes; outgoing: %llu packets, %llu bytes)\n",
		ts->other.proto_in.pc_packets,
		ts->other.proto_in.pc_bytes,
		ts->other.proto_out.pc_packets,
		ts->other.proto_out.pc_bytes);
	fprintf(fd, "Non-IP: %llu packets, %llu bytes\n",
		ts->nonip.proto_total.pc_packets,
		ts->nonip.proto_total.pc_bytes);
	fprintf(fd,
		"\t(incoming: %llu packets, %llu bytes; outgoing: %llu packets, %llu bytes)\n",
		ts->nonip.proto_in.pc_packets,
		ts->nonip.proto_in.pc_bytes,
		ts->nonip.proto_out.pc_packets,
		ts->nonip.proto_out.pc_bytes);
	fprintf(fd, "Broadcast: %llu packets, %llu bytes\n",
		ts->bcast.pc_packets,
		ts->bcast.pc_bytes);

	if (nsecs > 5) {
		char bps_string[64];
		char pps_string[64];

		fprintf(fd, "\nAverage rates:\n");

		rate_print(ts->total.proto_total.pc_bytes / nsecs, bps_string, sizeof(bps_string));
		rate_print_pps(ts->total.proto_total.pc_packets / nsecs, pps_string, sizeof(pps_string));
		fprintf(fd, "  Total:\t%s, %s\n", bps_string, pps_string);
		rate_print(ts->total.proto_in.pc_bytes / nsecs, bps_string, sizeof(bps_string));
		rate_print_pps(ts->total.proto_in.pc_packets / nsecs, pps_string, sizeof(pps_string));
		fprintf(fd, "  Incoming:\t%s, %s\n", bps_string, pps_string);
		rate_print(ts->total.proto_out.pc_bytes / nsecs, bps_string, sizeof(bps_string));
		rate_print_pps(ts->total.proto_out.pc_packets / nsecs, pps_string, sizeof(pps_string));
		fprintf(fd, "  Outgoing:\t%s, %s\n", bps_string, pps_string);
		rate_print(peakactivity, bps_string, sizeof(bps_string));
		rate_print_pps(peakpps, pps_string, sizeof(pps_string));
		fprintf(fd, "\nPeak total activity: %s, %s\n", bps_string, pps_string);
		rate_print(peakactivity_in, bps_string, sizeof(bps_string));
		rate_print_pps(peakpps_in, pps_string, sizeof(pps_string));
		fprintf(fd, "Peak incoming rate: %s, %s\n", bps_string, pps_string);
		rate_print(peakactivity_out, bps_string, sizeof(bps_string));
		rate_print_pps(peakpps_out, pps_string, sizeof(pps_string));
		fprintf(fd, "Peak outgoing rate: %s, %s\n\n", bps_string, pps_string);
	}
	fprintf(fd, "IP checksum errors: %llu\n\n", ts->bad.pc_packets);
	fprintf(fd, "Running time: %lu seconds\n", nsecs);
	fflush(fd);
}

static void printdetlabels(WINDOW * win)
{
	wattrset(win, BOXATTR);
	mvwprintw(win, 2, 14,
		  "  Total      Total    Incoming   Incoming    Outgoing   Outgoing");
	mvwprintw(win, 3, 14,
		  "Packets      Bytes     Packets      Bytes     Packets      Bytes");
	wattrset(win, STDATTR);
	mvwprintw(win, 4, 2, "Total:");
	mvwprintw(win, 5, 2, "IPv4:");
	mvwprintw(win, 6, 2, "IPv6:");
	mvwprintw(win, 7, 2, "TCP:");
	mvwprintw(win, 8, 2, "UDP:");
	mvwprintw(win, 9, 2, "ICMP:");
	mvwprintw(win, 10, 2, "Other IP:");
	mvwprintw(win, 11, 2, "Non-IP:");
	mvwprintw(win, 14, 2, "Total rates:");
	mvwprintw(win, 17, 2, "Incoming rates:");
	mvwprintw(win, 20, 2, "Outgoing rates:");

	mvwprintw(win, 14, 45, "Broadcast packets:");
	mvwprintw(win, 15, 45, "Broadcast bytes:");
	mvwprintw(win, 19, 45, "IP checksum errors:");

	update_panels();
	doupdate();
}

static void printstatrow(WINDOW * win, int row, unsigned long long total,
		  unsigned long long btotal, unsigned long long total_in,
		  unsigned long long btotal_in, unsigned long long total_out,
		  unsigned long long btotal_out)
{
	wmove(win, row, 12);
	printlargenum(total, win);
	wmove(win, row, 23);
	printlargenum(btotal, win);
	wmove(win, row, 35);
	printlargenum(total_in, win);
	wmove(win, row, 46);
	printlargenum(btotal_in, win);
	wmove(win, row, 58);
	printlargenum(total_out, win);
	wmove(win, row, 69);
	printlargenum(btotal_out, win);
}

static void printstatrow_proto(WINDOW *win, int row, struct proto_counter *proto_counter)
{
	printstatrow(win, row,
		     proto_counter->proto_total.pc_packets,
		     proto_counter->proto_total.pc_bytes,
		     proto_counter->proto_in.pc_packets,
		     proto_counter->proto_in.pc_bytes,
		     proto_counter->proto_out.pc_packets,
		     proto_counter->proto_out.pc_bytes);
}

static void printdetails(struct ifcounts *ifcounts, WINDOW * win)
{
	wattrset(win, HIGHATTR);
	/* Print totals on the IP protocols */
	printstatrow_proto(win, 4, &ifcounts->total);
	printstatrow_proto(win, 5, &ifcounts->ipv4);
	printstatrow_proto(win, 6, &ifcounts->ipv6);
	printstatrow_proto(win, 7, &ifcounts->tcp);
	printstatrow_proto(win, 8, &ifcounts->udp);
	printstatrow_proto(win, 9, &ifcounts->icmp);
	printstatrow_proto(win, 10, &ifcounts->other);

	/* Print non-IP totals */

	printstatrow_proto(win, 11, &ifcounts->nonip);

	/* Broadcast totals */
	wmove(win, 14, 67);
	printlargenum(ifcounts->bcast.pc_packets, win);
	wmove(win, 15, 67);
	printlargenum(ifcounts->bcast.pc_bytes, win);

	/* Bad packet count */

	mvwprintw(win, 19, 68, "%8lu", ifcounts->bad.pc_packets);
}

/*
 * The detailed interface statistics function
 */
void detstats(char *iface, time_t facilitytime)
{
	int logging = options.logging;

	WINDOW *statwin;
	PANEL *statpanel;

	int pkt_result = 0;

	FILE *logfile = NULL;

	unsigned int iplen = 0;

	struct ifcounts ifcounts;

	int ch;

	struct timeval tv;
	struct timeval start_tv;
	struct timeval updtime;
	time_t starttime;
	time_t now;
	time_t statbegin;
	time_t startlog;

	struct proto_counter span;

	struct rate rate;
	struct rate rate_in;
	struct rate rate_out;
	unsigned long peakactivity = 0;
	unsigned long peakactivity_in = 0;
	unsigned long peakactivity_out = 0;

	struct rate pps_rate;
	struct rate pps_rate_in;
	struct rate pps_rate_out;
	unsigned long peakpps = 0;
	unsigned long peakpps_in = 0;
	unsigned long peakpps_out = 0;

	int fd;

	if (!dev_up(iface)) {
		err_iface_down();
		return;
	}

	LIST_HEAD(promisc);
	if (options.promisc) {
		promisc_init(&promisc, iface);
		promisc_set_list(&promisc);
	}

	move(LINES - 1, 1);
	stdexitkeyhelp();
	statwin = newwin(LINES - 2, COLS, 1, 0);
	statpanel = new_panel(statwin);
	tx_stdwinset(statwin);
	wtimeout(statwin, -1);
	wattrset(statwin, BOXATTR);
	tx_colorwin(statwin);
	tx_box(statwin, ACS_VLINE, ACS_HLINE);
	wmove(statwin, 0, 1);
	wprintw(statwin, " Statistics for %s ", iface);
	wattrset(statwin, STDATTR);
	update_panels();
	doupdate();

	memset(&ifcounts, 0, sizeof(struct ifcounts));

	if (logging) {
		if (strcmp(current_logfile, "") == 0) {
			snprintf(current_logfile, 64, "%s-%s.log", DSTATLOG,
				 iface);

			if (!daemonized)
				input_logfile(current_logfile, &logging);
		}
	}

	if (logging) {
		opentlog(&logfile, current_logfile);

		if (logfile == NULL)
			logging = 0;
	}
	if (logging) {
		signal(SIGUSR1, rotate_dstat_log);

		rotate_flag = 0;
		writelog(logging, logfile,
			 "******** Detailed interface statistics started ********");
	}

	printdetlabels(statwin);
	printdetails(&ifcounts, statwin);
	update_panels();
	doupdate();

	memset(&span, 0, sizeof(span));
	rate_alloc(&rate, 5);
	rate_alloc(&rate_in, 5);
	rate_alloc(&rate_out, 5);

	rate_alloc(&pps_rate, 5);
	rate_alloc(&pps_rate_in, 5);
	rate_alloc(&pps_rate_out, 5);

	gettimeofday(&tv, NULL);
	start_tv = tv;
	updtime = tv;
	starttime = startlog = statbegin = tv.tv_sec;

	leaveok(statwin, TRUE);

	fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if(fd == -1) {
		write_error("Unable to obtain monitoring socket");
		goto err;
	}
	if(dev_bind_ifname(fd, iface) == -1) {
		write_error("Unable to bind interface on the socket");
		goto err_close;
	}

	exitloop = 0;

	PACKET_INIT(pkt);

	/*
	 * Data-gathering loop
	 */

	while (!exitloop) {
		gettimeofday(&tv, NULL);
		now = tv.tv_sec;

		if ((now - starttime) >= 1) {
			char buf[64];
			unsigned long activity, activity_in, activity_out;
			unsigned long pps, pps_in, pps_out;
			unsigned long msecs;

			wattrset(statwin, BOXATTR);
			printelapsedtime(statbegin, now, LINES - 3, 1, statwin);

			msecs = timeval_diff_msec(&tv, &start_tv);

			rate_add_rate(&rate, span.proto_total.pc_bytes, msecs);
			activity = rate_get_average(&rate);
			rate_add_rate(&rate_in, span.proto_in.pc_bytes, msecs);
			activity_in = rate_get_average(&rate_in);
			rate_add_rate(&rate_out, span.proto_out.pc_bytes, msecs);
			activity_out = rate_get_average(&rate_out);

			rate_add_rate(&pps_rate, span.proto_total.pc_packets, msecs);
			pps = rate_get_average(&pps_rate);
			rate_add_rate(&pps_rate_in, span.proto_in.pc_packets, msecs);
			pps_in = rate_get_average(&pps_rate_in);
			rate_add_rate(&pps_rate_out, span.proto_out.pc_packets, msecs);
			pps_out = rate_get_average(&pps_rate_out);

			memset(&span, 0, sizeof(span));
			starttime = now;
			start_tv = tv;

			wattrset(statwin, HIGHATTR);
			rate_print(activity, buf, sizeof(buf));
			mvwprintw(statwin, 14, 19, "%s", buf);
			rate_print_pps(pps, buf, sizeof(buf));
			mvwprintw(statwin, 15, 19, "%s", buf);
			rate_print(activity_in, buf, sizeof(buf));
			mvwprintw(statwin, 17, 19, "%s", buf);
			rate_print_pps(pps_in, buf, sizeof(buf));
			mvwprintw(statwin, 18, 19, "%s", buf);
			rate_print(activity_out, buf, sizeof(buf));
			mvwprintw(statwin, 20, 19, "%s", buf);
			rate_print_pps(pps_out, buf, sizeof(buf));
			mvwprintw(statwin, 21, 19, "%s", buf);

			if (activity > peakactivity)
				peakactivity = activity;

			if (activity_in > peakactivity_in)
				peakactivity_in = activity_in;

			if (activity_out > peakactivity_out)
				peakactivity_out = activity_out;

			if (pps > peakpps)
				peakpps = pps;

			if (pps_in > peakpps_in)
				peakpps_in = pps_in;

			if (pps_out > peakpps_out)
				peakpps_out = pps_out;
		}
		if (logging) {
			check_rotate_flag(&logfile);
			if ((now - startlog) >= options.logspan) {
				writedstatlog(iface,
					      peakactivity, peakpps,
					      peakactivity_in, peakpps_in,
					      peakactivity_out, peakpps_out,
					      &ifcounts, time(NULL) - statbegin,
					      logfile);

				startlog = now;
			}
		}

		if (screen_update_needed(&tv, &updtime)) {
			printdetails(&ifcounts, statwin);
			update_panels();
			doupdate();

			updtime = tv;
		}

		if ((facilitytime != 0)
		    && (((now - statbegin) / 60) >= facilitytime))
			exitloop = 1;

		if (packet_get(fd, &pkt, &ch, statwin) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		switch (ch) {
		case ERR:
			/* no key ready, do nothing */
			break;
		case 12:
		case 'l':
		case 'L':
			tx_refresh_screen();
			break;

		case 'Q':
		case 'q':
		case 'X':
		case 'x':
		case 24:
		case 27:
			exitloop = 1;
			break;
		}
		if (pkt.pkt_len <= 0)
			continue;

		int outgoing;

		pkt_result =
			packet_process(&pkt, NULL, NULL, NULL,
				       MATCH_OPPOSITE_USECONFIG,
				       options.v6inv4asv6);

		if (pkt_result != PACKET_OK
		    && pkt_result != MORE_FRAGMENTS)
			continue;

		outgoing = (pkt.pkt_pkttype == PACKET_OUTGOING);
		update_proto_counter(&ifcounts.total, outgoing, pkt.pkt_len);
		if (pkt.pkt_pkttype == PACKET_BROADCAST) {
			update_pkt_counter(&ifcounts.bcast, pkt.pkt_len);
		}

		update_proto_counter(&span, outgoing, pkt.pkt_len);

		/* account network layer protocol */
		switch(pkt.pkt_protocol) {
		case ETH_P_IP:
			if (pkt_result == CHECKSUM_ERROR) {
				update_pkt_counter(&ifcounts.bad, pkt.pkt_len);
				continue;
			}

			iplen = ntohs(pkt.iphdr->tot_len);

			update_proto_counter(&ifcounts.ipv4, outgoing, iplen);
			break;
		case ETH_P_IPV6:
			iplen = ntohs(pkt.ip6_hdr->ip6_plen) + 40;

			update_proto_counter(&ifcounts.ipv6, outgoing, iplen);
			break;
		default:
			update_proto_counter(&ifcounts.nonip, outgoing, iplen);
			continue;
		}

		__u8 ip_protocol = pkt_ip_protocol(&pkt);

		/* account transport layer protocol */
		switch (ip_protocol) {
		case IPPROTO_TCP:
			update_proto_counter(&ifcounts.tcp, outgoing, iplen);
			break;
		case IPPROTO_UDP:
			update_proto_counter(&ifcounts.udp, outgoing, iplen);
			break;
		case IPPROTO_ICMP:
		case IPPROTO_ICMPV6:
			update_proto_counter(&ifcounts.icmp, outgoing, iplen);
			break;
		default:
			update_proto_counter(&ifcounts.other, outgoing, iplen);
			break;
		}
	}

err_close:
	close(fd);

err:
	rate_destroy(&pps_rate_out);
	rate_destroy(&pps_rate_in);
	rate_destroy(&pps_rate);

	rate_destroy(&rate_out);
	rate_destroy(&rate_in);
	rate_destroy(&rate);

	if (options.promisc) {
		promisc_restore_list(&promisc);
		promisc_destroy(&promisc);
	}

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writedstatlog(iface,
			      peakactivity, peakpps, peakactivity_in,
			      peakpps_in, peakactivity_out, peakpps_out,
			      &ifcounts, time(NULL) - statbegin,
			      logfile);
		writelog(logging, logfile,
			 "******** Detailed interface statistics stopped ********");
		fclose(logfile);
	}

	del_panel(statpanel);
	delwin(statwin);
	strcpy(current_logfile, "");
	pkt_cleanup();
	update_panels();
	doupdate();
}
