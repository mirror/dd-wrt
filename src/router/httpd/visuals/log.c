#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/klog.h>
#include <netdb.h>

#include <broadcom.h>
#include <support.h>

#define LOG_BUF	16384		// max buf, total have 64 entries

/*
 * Dump firewall log 
 */
void ej_dumplog(webs_t wp, int argc, char_t ** argv)
{
	char buf[LOG_BUF], *line, *next, *s;
	int len;
	char *type;

	time_t tm;
	char *verdict, *src, *dst, *proto, *spt, *dpt, *in, *out;
	char src_old[10] = "", dpt_old[10] = "", dst_old[10] =
	    "", proto_old[10] = "";

	int _dport, _sport;
	char *_proto = NULL;
	struct servent *servp;

	// struct servent *d_servp;

	char *wan_if = get_wan_face();
	char *lan_if = nvram_safe_get("lan_ifname");

	int count = 0;

	if (ejArgs(argc, argv, "%s", &type) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
	// if (klogctl(3, buf, 4096) < 0) {
	if (klogctl(3, buf, LOG_BUF) < 0) {
		websError(wp, 400, "Insufficient memory\n");
		return;
	}
	cprintf("log: %s\n", buf);
	for (next = buf; (line = strsep(&next, "\n"));) {
		if (!strncmp(line, "<4>DROP", 7))
			verdict = "Dropped";
		else if (!strncmp(line, "<4>ACCEPT", 9))
			verdict = "Accepted";
		else if (!strncmp(line, "<4>REJECT", 9))
			verdict = "Rejected";
		else
			continue;

		/*
		 * Parse into tokens 
		 */
		s = line;
		len = strlen(s);
		while (strsep(&s, " ")) ;

		/*
		 * Initialize token values 
		 */
		time(&tm);
		in = out = src = dst = proto = spt = dpt = "n/a";

		/*
		 * Set token values 
		 */
		for (s = line; s < &line[len] && *s; s += strlen(s) + 1) {
			if (!strncmp(s, "TIME=", 5))
				tm = strtoul(&s[5], NULL, 10);
			else if (!strncmp(s, "IN=", 3))
				in = &s[3];
			else if (!strncmp(s, "OUT=", 4))
				out = &s[4];
			else if (!strncmp(s, "SRC=", 4))
				src = &s[4];
			else if (!strncmp(s, "DST=", 4))
				dst = &s[4];
			else if (!strncmp(s, "PROTO=", 6))
				proto = &s[6];
			else if (!strncmp(s, "SPT=", 4))
				spt = &s[4];
			else if (!strncmp(s, "DPT=", 4))
				dpt = &s[4];
		}

		if (!strncmp(dpt, "n/a", 3))	// example: ping
			continue;

		_dport = atoi(dpt);
		_sport = atoi(spt);

		if (!strncmp(proto, "TCP", 3))
			_proto = "tcp";
		else if (!strncmp(proto, "UDP", 3))
			_proto = "udp";

		servp = my_getservbyport(htons(_dport), _proto);

		if (!strcmp(type, "incoming")) {
			if ((!strncmp(in, "ppp", 3) && !strncmp(in, wan_if, 3))
			    || (!strcmp(in, wan_if))) {
				if (!strcmp(src, src_old)
				    && !strcmp(dpt, dpt_old)
				    && !strcmp(proto, proto_old)) {
					continue;	// skip same record
				} else {
					strcpy(src_old, src);
					strcpy(dpt_old, dpt);
					strcpy(proto_old, proto);
				}

				websWrite(wp, "<tr height=\"1\">\n");
				websWrite(wp, "<td>%s</td>\n", src);
				websWrite(wp, "<td align=\"center\">%s</td>\n",
					  proto);
				websWrite(wp, "<td align=\"center\">%s</td>\n",
					  servp ? servp->s_name : dpt);
				websWrite(wp, "<td align=\"center\">%s</td>\n",
					  verdict);
				websWrite(wp, "</tr>\n");
			}
		} else if (!strcmp(type, "outgoing")) {
			if (!strncmp(in, lan_if, 3) &&
			    ((!strncmp(out, "ppp", 3)
			      && !strncmp(out, wan_if, 3))
			     || (!strcmp(out, wan_if)))) {
				if (_dport == 53) {
					continue;	// skip DNS
				}

				if (!strcmp(src, src_old)
				    && !strcmp(dst, dst_old)
				    && !strcmp(proto, proto_old)
				    && !strcmp(dpt, dpt_old)) {
					continue;	// skip same record
				} else {
					strcpy(src_old, src);
					strcpy(dst_old, dst);
					strcpy(proto_old, proto);
					strcpy(dpt_old, dpt);
				}

				websWrite(wp, "<tr height=\"1\">\n");
				websWrite(wp, "<td>%s</td>\n", src);
				websWrite(wp, "<td>%s</td>\n", dst);
				websWrite(wp, "<td align=\"center\">%s</td>\n",
					  proto);
				websWrite(wp, "<td>%s</td>\n",
					  servp ? servp->s_name : dpt);
				websWrite(wp, "<td align=\"center\">%s</td>\n",
					  verdict);
				websWrite(wp, "</tr>\n");
			}
		} else if (!strcmp(type, "all")) {
			int dir = 0;

			if ((!strncmp(out, "ppp", 3) && !strncmp(out, wan_if, 3)) || (!strcmp(out, wan_if)))	// incoming
				dir = 1;
			else if (!strncmp(in, lan_if, 3) && ((!strncmp(out, "ppp", 3) && !strncmp(out, wan_if, 3)) || (!strcmp(out, wan_if))))	// outgoing
				dir = 2;
			else
				continue;

			if (_dport == 53) {
				continue;	// skip DNS
			}

			if (!strcmp(src, src_old) && !strcmp(dpt, dpt_old)
			    && !strcmp(dst, dst_old)) {
				continue;	// skip same record
			} else {
				strcpy(src_old, src);
				strcpy(dpt_old, dpt);
				strcpy(dst_old, dst);
			}

			websWrite(wp, "%c'%s','%s','%s','%s','%d'\n",
				  count ? ',' : ' ', proto, src, dst,
				  servp ? servp->s_name : dpt, dir);
			count++;
		}
		// if(s_service) free(s_service);
		// if(d_service) free(d_service);
	}

	return;

}
