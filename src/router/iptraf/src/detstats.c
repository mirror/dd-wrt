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
#include "error.h"
#include "detstats.h"
#include "rate.h"
#include "capt.h"

struct ifcounts {
	struct proto_counter total;
	struct pkt_counter bad;
	struct proto_counter ipv4;
	struct proto_counter ipv6;
	struct proto_counter nonip;
	struct proto_counter bcast;

	struct proto_counter tcp;
	struct proto_counter udp;
	struct proto_counter icmp;
	struct proto_counter other;

	struct proto_counter span;
	struct pkt_counter span_bcast;
};

struct ifrates {
	struct rate rate;
	struct rate rate_in;
	struct rate rate_out;
	struct rate rate_bcast;
	unsigned long activity;
	unsigned long peakactivity;
	unsigned long activity_in;
	unsigned long peakactivity_in;
	unsigned long activity_out;
	unsigned long peakactivity_out;
	unsigned long activity_bcast;

	struct rate pps_rate;
	struct rate pps_rate_in;
	struct rate pps_rate_out;
	struct rate pps_rate_bcast;
	unsigned long pps;
	unsigned long peakpps;
	unsigned long pps_in;
	unsigned long peakpps_in;
	unsigned long pps_out;
	unsigned long peakpps_out;
	unsigned long pps_bcast;
};

/* USR1 log-rotation signal handlers */
static void rotate_dstat_log(int s __unused)
{
	rotate_flag = 1;
	strcpy(target_logname, current_logfile);
	signal(SIGUSR1, rotate_dstat_log);
}

static void writedstatlog(char *ifname, struct ifcounts *ts,
			  struct ifrates *ifrates, unsigned long nsecs, FILE *fd)
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
		ts->bcast.proto_total.pc_packets,
		ts->bcast.proto_total.pc_bytes);

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
		rate_print(ifrates->peakactivity, bps_string, sizeof(bps_string));
		rate_print_pps(ifrates->peakpps, pps_string, sizeof(pps_string));
		fprintf(fd, "\nPeak total activity: %s, %s\n", bps_string, pps_string);
		rate_print(ifrates->peakactivity_in, bps_string, sizeof(bps_string));
		rate_print_pps(ifrates->peakpps_in, pps_string, sizeof(pps_string));
		fprintf(fd, "Peak incoming rate: %s, %s\n", bps_string, pps_string);
		rate_print(ifrates->peakactivity_out, bps_string, sizeof(bps_string));
		rate_print_pps(ifrates->peakpps_out, pps_string, sizeof(pps_string));
		fprintf(fd, "Peak outgoing rate: %s, %s\n\n", bps_string, pps_string);
	}
	fprintf(fd, "IP checksum errors: %llu\n\n", ts->bad.pc_packets);
	fprintf(fd, "Running time: %lu seconds\n", nsecs);
	fflush(fd);
}

static void ifcounts_init(struct ifcounts *ifcounts)
{
	if (ifcounts == NULL)
		return;

	proto_counter_reset(&ifcounts->total);
	pkt_counter_reset(&ifcounts->bad);
	proto_counter_reset(&ifcounts->ipv4);
	proto_counter_reset(&ifcounts->ipv6);
	proto_counter_reset(&ifcounts->nonip);
	proto_counter_reset(&ifcounts->bcast);

	proto_counter_reset(&ifcounts->tcp);
	proto_counter_reset(&ifcounts->udp);
	proto_counter_reset(&ifcounts->icmp);
	proto_counter_reset(&ifcounts->other);

	proto_counter_reset(&ifcounts->span);
	pkt_counter_reset(&ifcounts->span_bcast);
}

static void ifcounts_destroy(struct ifcounts *ifcounts __unused)
{
	/* do nothing for now */
}

static void ifrates_init(struct ifrates *ifrates)
{
	if (ifrates == NULL)
		return;

	rate_alloc(&ifrates->rate, 5);
	rate_alloc(&ifrates->rate_in, 5);
	rate_alloc(&ifrates->rate_out, 5);
	rate_alloc(&ifrates->rate_bcast, 5);

	ifrates->activity = 0UL;
	ifrates->peakactivity = 0UL;
	ifrates->activity_in = 0UL;
	ifrates->peakactivity_in = 0UL;
	ifrates->activity_out = 0UL;
	ifrates->peakactivity_out = 0UL;
	ifrates->activity_bcast = 0UL;

	rate_alloc(&ifrates->pps_rate, 5);
	rate_alloc(&ifrates->pps_rate_in, 5);
	rate_alloc(&ifrates->pps_rate_out, 5);
	rate_alloc(&ifrates->pps_rate_bcast, 5);

	ifrates->pps = 0UL;
	ifrates->peakpps = 0UL;
	ifrates->pps_in = 0UL;
	ifrates->peakpps_in = 0UL;
	ifrates->pps_out = 0UL;
	ifrates->peakpps_out = 0UL;
	ifrates->pps_bcast = 0UL;
}

static void ifrates_destroy(struct ifrates *ifrates)
{
	if (ifrates == NULL)
		return;

	rate_destroy(&ifrates->pps_rate_bcast);
	rate_destroy(&ifrates->pps_rate_out);
	rate_destroy(&ifrates->pps_rate_in);
	rate_destroy(&ifrates->pps_rate);

	rate_destroy(&ifrates->rate_bcast);
	rate_destroy(&ifrates->rate_out);
	rate_destroy(&ifrates->rate_in);
	rate_destroy(&ifrates->rate);
}

static void ifrates_update(struct ifrates *ifrates, struct ifcounts *ifcounts,
			   unsigned long msecs)
{
	rate_add_rate(&ifrates->rate, ifcounts->span.proto_total.pc_bytes, msecs);
	ifrates->activity = rate_get_average(&ifrates->rate);
	rate_add_rate(&ifrates->rate_in, ifcounts->span.proto_in.pc_bytes, msecs);
	ifrates->activity_in = rate_get_average(&ifrates->rate_in);
	rate_add_rate(&ifrates->rate_out, ifcounts->span.proto_out.pc_bytes, msecs);
	ifrates->activity_out = rate_get_average(&ifrates->rate_out);
	rate_add_rate(&ifrates->rate_bcast, ifcounts->span_bcast.pc_bytes, msecs);
	ifrates->activity_bcast = rate_get_average(&ifrates->rate_bcast);

	rate_add_rate(&ifrates->pps_rate, ifcounts->span.proto_total.pc_packets, msecs);
	ifrates->pps = rate_get_average(&ifrates->pps_rate);
	rate_add_rate(&ifrates->pps_rate_in, ifcounts->span.proto_in.pc_packets, msecs);
	ifrates->pps_in = rate_get_average(&ifrates->pps_rate_in);
	rate_add_rate(&ifrates->pps_rate_out, ifcounts->span.proto_out.pc_packets, msecs);
	ifrates->pps_out = rate_get_average(&ifrates->pps_rate_out);
	rate_add_rate(&ifrates->pps_rate_bcast, ifcounts->span_bcast.pc_packets, msecs);
	ifrates->pps_bcast = rate_get_average(&ifrates->pps_rate_bcast);

	proto_counter_reset(&ifcounts->span);
	pkt_counter_reset(&ifcounts->span_bcast);

	if (ifrates->activity > ifrates->peakactivity)
		ifrates->peakactivity = ifrates->activity;

	if (ifrates->activity_in > ifrates->peakactivity_in)
		ifrates->peakactivity_in = ifrates->activity_in;

	if (ifrates->activity_out > ifrates->peakactivity_out)
		ifrates->peakactivity_out = ifrates->activity_out;

	if (ifrates->pps > ifrates->peakpps)
		ifrates->peakpps = ifrates->pps;

	if (ifrates->pps_in > ifrates->peakpps_in)
		ifrates->peakpps_in = ifrates->pps_in;

	if (ifrates->pps_out > ifrates->peakpps_out)
		ifrates->peakpps_out = ifrates->pps_out;
}

static void ifrates_show(struct ifrates *ifrates, WINDOW *statwin)
{
	char buf[64];

	wattrset(statwin, HIGHATTR);
	rate_print(ifrates->activity, buf, sizeof(buf));
	mvwprintw(statwin, 14, 19, "%s", buf);
	rate_print_pps(ifrates->pps, buf, sizeof(buf));
	mvwprintw(statwin, 15, 19, "%s", buf);
	rate_print(ifrates->activity_in, buf, sizeof(buf));
	mvwprintw(statwin, 17, 19, "%s", buf);
	rate_print_pps(ifrates->pps_in, buf, sizeof(buf));
	mvwprintw(statwin, 18, 19, "%s", buf);
	rate_print(ifrates->activity_out, buf, sizeof(buf));
	mvwprintw(statwin, 20, 19, "%s", buf);
	rate_print_pps(ifrates->pps_out, buf, sizeof(buf));
	mvwprintw(statwin, 21, 19, "%s", buf);
	rate_print(ifrates->activity_bcast, buf, sizeof(buf));
	mvwprintw(statwin, 14, 64, "%s", buf);
	rate_print_pps(ifrates->pps_bcast, buf, sizeof(buf));
	mvwprintw(statwin, 15, 64, "%s", buf);
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
	mvwprintw(win, 12, 2, "Broadcast:");
	mvwprintw(win, 14, 2, "Total rates:");
	mvwprintw(win, 17, 2, "Incoming rates:");
	mvwprintw(win, 20, 2, "Outgoing rates:");

	mvwprintw(win, 14, 45, "Broadcast rates:");
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

	/* Print broadcast totals */
	printstatrow_proto(win, 12, &ifcounts->bcast);


	/* Bad packet count */

	mvwprintw(win, 19, 65, "%8llu", ifcounts->bad.pc_packets);
}

static void detstats_process_key(int ch)
{
	switch (ch) {
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
	case ERR:
		/* no key ready, do nothing */
		/* fall through */
	default:
		/* do nothing */
		break;
	}
}

static void detstats_process_packet(struct ifcounts *ifcounts, struct pkt_hdr *pkt)
{
	int pkt_result = packet_process(pkt, NULL, NULL, NULL,
					MATCH_OPPOSITE_USECONFIG,
					options.v6inv4asv6);

	switch (pkt_result) {
	case PACKET_OK:			/* we only handle these */
	case MORE_FRAGMENTS:
	case CHECKSUM_ERROR:
		break;
	default:			/* drop others */
		return;
	}

	int outgoing = (pkt->from->sll_pkttype == PACKET_OUTGOING);
	proto_counter_update(&ifcounts->total, outgoing, pkt->pkt_len);
	if (pkt->from->sll_pkttype == PACKET_BROADCAST) {
		proto_counter_update(&ifcounts->bcast, outgoing, pkt->pkt_len);
		pkt_counter_update(&ifcounts->span_bcast, pkt->pkt_len);
	}

	proto_counter_update(&ifcounts->span, outgoing, pkt->pkt_len);

	unsigned int iplen = 0;

	/* account network layer protocol */
	switch(pkt->pkt_protocol) {
	case ETH_P_IP:
		if (pkt_result == CHECKSUM_ERROR) {
			pkt_counter_update(&ifcounts->bad, pkt->pkt_len);
			return;
		}

		iplen = ntohs(pkt->iphdr->tot_len);

		proto_counter_update(&ifcounts->ipv4, outgoing, iplen);
		break;
	case ETH_P_IPV6:
		iplen = ntohs(pkt->ip6_hdr->ip6_plen) + 40;

		proto_counter_update(&ifcounts->ipv6, outgoing, iplen);
		break;
	default:
		proto_counter_update(&ifcounts->nonip, outgoing, pkt->pkt_len);
		return;
	}

	__u8 ip_protocol = pkt_ip_protocol(pkt);

	/* account transport layer protocol */
	switch (ip_protocol) {
	case IPPROTO_TCP:
		proto_counter_update(&ifcounts->tcp, outgoing, iplen);
		break;
	case IPPROTO_UDP:
		proto_counter_update(&ifcounts->udp, outgoing, iplen);
		break;
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		proto_counter_update(&ifcounts->icmp, outgoing, iplen);
		break;
	default:
		proto_counter_update(&ifcounts->other, outgoing, iplen);
		break;
	}
}

/* detailed interface statistics function */
void detstats(char *iface, time_t facilitytime)
{
	int logging = options.logging;

	WINDOW *statwin;
	PANEL *statpanel;

	FILE *logfile = NULL;

	struct ifcounts ifcounts;
	struct ifrates ifrates;

	int ch;

	struct capt capt;

	struct pkt_hdr pkt;

	if (!dev_up(iface)) {
		err_iface_down();
		return;
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
	leaveok(statwin, TRUE);

	update_panels();
	doupdate();

	if (capt_init(&capt, iface) == -1) {
		write_error("Unable to initialize packet capture interface");
		goto err;
	}

	ifcounts_init(&ifcounts);
	ifrates_init(&ifrates);

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

	packet_init(&pkt);

	exitloop = 0;

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	struct timespec last_time = now;
	struct timespec next_screen_update = { 0 };

	time_t starttime = now.tv_sec;
	time_t endtime = INT_MAX;
	if (facilitytime != 0)
		endtime = now.tv_sec + facilitytime * 60;

	time_t log_next = INT_MAX;
	if (logging)
		log_next = now.tv_sec + options.logspan;

	/* data-gathering loop */
	while (!exitloop) {
		clock_gettime(CLOCK_MONOTONIC, &now);

		if (now.tv_sec > last_time.tv_sec) {
			unsigned long msecs = timespec_diff_msec(&now, &last_time);
			ifrates_update(&ifrates, &ifcounts, msecs);
			ifrates_show(&ifrates, statwin);

			wattrset(statwin, BOXATTR);
			printelapsedtime(now.tv_sec - starttime, 1, statwin);

			print_packet_drops(capt_get_dropped(&capt), statwin, 49);

			if (now.tv_sec > endtime)
				exitloop = 1;

			if (logging && (now.tv_sec > log_next)) {
				check_rotate_flag(&logfile);
				writedstatlog(iface, &ifcounts, &ifrates,
					      now.tv_sec - starttime,
					      logfile);
				log_next = now.tv_sec + options.logspan;
			}

			last_time = now;
		}
		if (time_after(&now, &next_screen_update)) {
			printdetails(&ifcounts, statwin);
			update_panels();
			doupdate();

			set_next_screen_update(&next_screen_update, &now);
		}

		if (capt_get_packet(&capt, &pkt, &ch, statwin) == -1) {
			write_error("Packet receive failed");
			exitloop = 1;
			break;
		}

		if (ch != ERR)
			detstats_process_key(ch);

		if (pkt.pkt_len > 0) {
			detstats_process_packet(&ifcounts, &pkt);
			capt_put_packet(&capt, &pkt);
		}

	}
	packet_destroy(&pkt);

	if (logging) {
		signal(SIGUSR1, SIG_DFL);
		writedstatlog(iface, &ifcounts, &ifrates,
			      time(NULL) - starttime, logfile);
		writelog(logging, logfile,
			 "******** Detailed interface statistics stopped ********");
		fclose(logfile);
	}
	strcpy(current_logfile, "");

	ifrates_destroy(&ifrates);
	ifcounts_destroy(&ifcounts);
	capt_destroy(&capt);
err:
	del_panel(statpanel);
	delwin(statwin);
	update_panels();
	doupdate();
}
