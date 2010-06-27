/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 1997, 1998 Public Flood Software
 * Copyright (c) 1999, 2000 MacGyver aka Habeeb J. Dihu <macgyver@tos.net>
 * Copyright (c) 2001-2009 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

/* Shows a count of "who" is online via proftpd.  Uses the scoreboard file.
 *
 * $Id: ftpwho.c,v 1.27 2009/09/04 17:13:10 castaglia Exp $
 */

#include "utils.h"

#define MAX_CLASSES 100
struct scoreboard_class {
   char *score_class;
   unsigned long score_count;
};

#define OF_COMPAT		0x001
#define OF_ONELINE		0x002

static const char *config_filename = PR_CONFIG_FILE_PATH;

static char *percent_complete(off_t size, off_t done) {
  static char sbuf[32];

  memset(sbuf, '\0', sizeof(sbuf));

  if (done == 0) {
    util_sstrncpy(sbuf, "0", sizeof(sbuf));

  } else if (size == 0) {
    util_sstrncpy(sbuf, "Inf", sizeof(sbuf));

  } else if (done >= size) {
    util_sstrncpy(sbuf, "100", sizeof(sbuf));

  } else {
    snprintf(sbuf, sizeof(sbuf), "%.0f",
	     ((double) done / (double) size) * 100.0);
    sbuf[sizeof(sbuf)-1] = '\0';
  }

  return sbuf;
}

static const char *show_time(time_t *i) {
  time_t now = time(NULL);
  unsigned long l;
  static char sbuf[7];

  if (!i || !*i)
    return "-";

  memset(sbuf, '\0', sizeof(sbuf));
  l = now - *i;

  if (l < 3600)
    snprintf(sbuf, sizeof(sbuf), "%lum%lus",(l / 60),(l % 60));
  else
    snprintf(sbuf, sizeof(sbuf), "%luh%lum",(l / 3600),
    ((l - (l / 3600) * 3600) / 60));

  return sbuf;
}

static int check_scoreboard_file(void) {
  struct stat st;

  if (stat(util_get_scoreboard(), &st) < 0)
    return -1;

  return 0;
}

static const char *show_uptime(time_t uptime_since) {
  static char buf[128] = {'\0'};
  time_t uptime_secs = time(NULL) - uptime_since;
  int upminutes, uphours, updays;
  int pos = 0;

  memset(buf, '\0', sizeof(buf));

  updays = (int) uptime_secs / (60 * 60 * 24);

  if (updays)
    pos += sprintf(buf + pos, "%d day%s, ", updays, (updays != 1) ? "s" : "");

  upminutes = (int) uptime_secs / 60;

  uphours = upminutes / 60;
  uphours = uphours % 24;

  upminutes = upminutes % 60;

  if (uphours)
    pos += sprintf(buf + pos, "%2d hr%s %02d min", uphours,
      (uphours != 1) ? "s" : "", upminutes);
  else
    pos += sprintf(buf + pos, "%d min", upminutes);

  return buf;
}

static struct option_help {
  const char *long_opt,*short_opt,*desc;
} opts_help[] = {
  { "--config",	"-c",	"specify full path to proftpd configuration file" },
  { "--file",	"-f",	"specify full path to scoreboard file" },
  { "--help",	"-h",	NULL },
  { "--outform","-o",	"specify an output format" },
  { "--verbose","-v",	"display additional information for each connection" },
  { "--server",	"-S",	"show users only for specified ServerName" },
  { NULL }
};

#ifdef HAVE_GETOPT_LONG
static struct option opts[] = {
  { "config",  1, NULL, 'c' },
  { "file",    1, NULL, 'f' },
  { "help",    0, NULL, 'h' },
  { "outform", 1, NULL, 'o' },
  { "verbose", 0, NULL, 'v' },
  { "server",  1, NULL, 'S' },
  { NULL,      0, NULL, 0   }
};
#endif /* HAVE_GETOPT_LONG */

static void show_usage(const char *progname, int exit_code) {
  struct option_help *h = NULL;

  printf("usage: %s [options]\n", progname);
  for (h = opts_help; h->long_opt; h++) {
#ifdef HAVE_GETOPT_LONG
    printf("  %s, %s\n", h->short_opt, h->long_opt);
#else /* HAVE_GETOPT_LONG */
    printf("  %s\n", h->short_opt);
#endif
    if (!h->desc) {
      printf("    display %s usage\n", progname);

    } else {
      printf("    %s\n", h->desc);
    }
  }

  exit(exit_code);
}

int main(int argc, char **argv) {
  pr_scoreboard_entry_t *score = NULL;
  pid_t mpid = 0;
  time_t uptime = 0;
  unsigned int count = 0, total = 0;
  int c = 0, res = 0;
  char *server_name = NULL;
  struct scoreboard_class classes[MAX_CLASSES];
  char *cp, *progname = *argv;
  const char *cmdopts = "S:c:f:ho:v";
  unsigned char verbose = FALSE;
  unsigned long outform = 0;

  memset(classes, 0, MAX_CLASSES * sizeof(struct scoreboard_class));

  cp = strrchr(progname, '/');
  if (cp != NULL)
    progname = cp+1;

  opterr = 0;
  while ((c =
#ifdef HAVE_GETOPT_LONG
	 getopt_long(argc, argv, cmdopts, opts, NULL)
#else /* HAVE_GETOPT_LONG */
	 getopt(argc, argv, cmdopts)
#endif /* HAVE_GETOPT_LONG */
	 ) != -1) {
    switch (c) {
      case 'h':
        show_usage(progname, 0);

      case 'v':
        verbose = TRUE;
        break;

      case 'f':
        util_set_scoreboard(optarg);
        break;

      case 'c':
        config_filename = strdup(optarg);
        break;

      case 'o':
        /* Check the given outform parameter. */
        if (strcmp(optarg, "compat") == 0) {
          outform |= OF_COMPAT;
          break;

        } else if (strcmp(optarg, "oneline") == 0) {
          outform |= OF_ONELINE;
          break;
        }

        fprintf(stderr, "unknown outform value: '%s'\n", optarg);
        return 1;

      case 'S':
        server_name = strdup(optarg);
        break;

      case '?':
        fprintf(stderr, "unknown option: %c\n", (char) optopt);
        show_usage(progname, 1);
    }
  }

  /* First attempt to check the supplied/default scoreboard path.  If this is
   * incorrect, try the config file kludge.
   */
  if (check_scoreboard_file() < 0) {
    const char *file = util_scan_config(config_filename, "ScoreboardFile");
    if (file)
      util_set_scoreboard(file);

    if (check_scoreboard_file() < 0) {
      fprintf(stderr, "%s: %s\n", util_get_scoreboard(), strerror(errno));
      fprintf(stderr, "(Perhaps you need to specify the ScoreboardFile with -f, or change\n");
      fprintf(stderr," the compile-time default directory?)\n");
      exit(1);
    }
  }

  count = 0;
  if ((res = util_open_scoreboard(O_RDONLY)) < 0) {
    switch (res) {
      case -1:
        fprintf(stderr, "unable to open scoreboard: %s\n", strerror(errno));
        return 1;

      case UTIL_SCORE_ERR_BAD_MAGIC:
        fprintf(stderr, "scoreboard is corrupted or old\n");
        return 1;

      case UTIL_SCORE_ERR_OLDER_VERSION:
        fprintf(stderr, "scoreboard version is too old\n");
        return 1;

      case UTIL_SCORE_ERR_NEWER_VERSION:
        fprintf(stderr, "scoreboard version is too new\n");
        return 1;
    }
  }

  mpid = util_scoreboard_get_daemon_pid();
  uptime = util_scoreboard_get_daemon_uptime();

  if (!mpid) {
    printf("inetd FTP daemon:\n");

  } else {
    printf("standalone FTP daemon [%u], up for %s\n", (unsigned int) mpid,
      show_uptime(uptime));
  }

  if (server_name)
    printf("ProFTPD Server '%s'\n", server_name);

  while ((score = util_scoreboard_entry_read()) != NULL) {
    int downloading = FALSE, uploading = FALSE;
    register unsigned int i = 0;

    /* If a ServerName was given, skip unless the scoreboard entry matches. */
    if (server_name &&
        strcmp(server_name, score->sce_server_label) != 0)
      continue;

    if (!count++) {
      if (total)
        printf("   -  %d user%s\n\n", total, total > 1 ? "s" : "");

      total = 0;
    }

    /* Tally up per-Class counters. */
    for (i = 0; i != MAX_CLASSES; i++) {
      if (classes[i].score_class == 0) {
        classes[i].score_class = strdup(score->sce_class);
        classes[i].score_count++;
        break;
      }

      if (strcasecmp(classes[i].score_class, score->sce_class) == 0) {
        classes[i].score_count++;
        break;
      }
    }

    total++;

    if (strcmp(score->sce_cmd, "RETR") == 0 ||
        strcmp(score->sce_cmd, "READ") == 0 ||
        strcmp(score->sce_cmd, "scp download") == 0) {
      downloading = TRUE;

    } else {
      if (strcmp(score->sce_cmd, "STOR") == 0 ||
          strcmp(score->sce_cmd, "STOU") == 0 ||
          strcmp(score->sce_cmd, "APPE") == 0 ||
          strcmp(score->sce_cmd, "WRITE") == 0 ||
          strcmp(score->sce_cmd, "scp upload") == 0) {
        uploading = TRUE;
      }
    }

    if (outform & OF_COMPAT) {
      if ((downloading || uploading) &&
          score->sce_xfer_size > 0) {
        if (downloading) {
          printf("%5d %-6s (%s%%) %s %s\n", (int) score->sce_pid,
            show_time(&score->sce_begin_idle),
            percent_complete(score->sce_xfer_size, score->sce_xfer_done),
            score->sce_cmd, score->sce_cmd_arg);

        } else {
          printf("%5d %-6s (n/a) %s %s\n", (int) score->sce_pid,
            show_time(&score->sce_begin_idle), score->sce_cmd,
            score->sce_cmd_arg);
        }

      } else {
        printf("%5d %-6s %s %s\n", (int) score->sce_pid,
          show_time(&score->sce_begin_idle), score->sce_cmd,
          score->sce_cmd_arg);
      }

      if (verbose) {
        if (score->sce_client_addr[0]) {
          printf("             (host: %s [%s])\n", score->sce_client_name,
            score->sce_client_addr);
        }

        if (score->sce_protocol[0]) {
          printf("              (protocol: %s)\n", score->sce_protocol);
        }

        if (score->sce_cwd[0]) {
          printf("              (cwd: %s)\n", score->sce_cwd);
        }

        if (score->sce_class[0]) {
          printf("              (class: %s)\n", score->sce_class);
        }
      }

      continue;
    }

    /* Has the client authenticated yet, or not? */
    if (strcmp(score->sce_user, "(none)")) {

      /* Is the client idle? */
      if (strcmp(score->sce_cmd, "idle") == 0) {

        /* These printf() calls needs to be split up, as show_time() returns
         * a pointer to a static buffer, and pushing two invocations onto
         * the stack means that the times thus formatted will be incorrect.
         */
        printf("%5d %-8s [%6s] ", (int) score->sce_pid,
          score->sce_user, show_time(&score->sce_begin_session));
        printf("%6s %s", show_time(&score->sce_begin_idle), score->sce_cmd);

        if (verbose && !(outform & OF_ONELINE))
          printf("\n");

      } else {
        if (downloading) {
          printf("%5d %-8s [%6s] (%3s%%) %s %s", (int) score->sce_pid,
            score->sce_user, show_time(&score->sce_begin_session),
            percent_complete(score->sce_xfer_size, score->sce_xfer_done),
            score->sce_cmd, score->sce_cmd_arg);

        } else {
          printf("%5d %-8s [%6s] (n/a) %s %s", (int) score->sce_pid,
            score->sce_user, show_time(&score->sce_begin_session),
            score->sce_cmd, score->sce_cmd_arg);
        }

        if (verbose) {
          printf("%sKB/s: %3.2f%s",
            (outform & OF_ONELINE) ? " " : "\n\t",
            (score->sce_xfer_len / 1024.0) /
              (score->sce_xfer_elapsed / 1000),
            (outform & OF_ONELINE) ? "" : "\n");
        }
      }

      /* Display additional information, if requested. */
      if (verbose) {
        if (score->sce_client_addr[0]) {
          printf("%sclient: %s [%s]%s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_client_name, score->sce_client_addr,
            (outform & OF_ONELINE) ? "" : "\n");
        }

        if (score->sce_server_addr[0]) {
          printf("%sserver: %s (%s)%s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_server_addr, score->sce_server_label,
            (outform & OF_ONELINE) ? "" : "\n");
        }

        if (score->sce_protocol[0]) {
          printf("%sprotocol: %s%s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_protocol,
            (outform & OF_ONELINE) ? "" : "\n");
        }

        if (score->sce_cwd[0]) {
          printf("%slocation: %s%s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_cwd,
            (outform & OF_ONELINE) ? "" : "\n");
        }

        if (score->sce_class[0]) {
          printf("%sclass: %s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_class);
        }

        printf("%s", "\n");

      } else {
        printf("%s", "\n");
      }

    } else {

      printf("%5d %-8s [%6s] (authenticating)", (int) score->sce_pid,
        score->sce_user, show_time(&score->sce_begin_session));

      /* Display additional information, if requested. */
      if (verbose) {
        if (score->sce_client_addr[0]) {
          printf("%sclient: %s [%s]%s",
            (outform & OF_ONELINE) ? " " : "\n\t",
            score->sce_client_name, score->sce_client_addr,
            (outform & OF_ONELINE) ? "" : "\n");
        }

        if (score->sce_server_addr[0]) {
          printf("%sserver: %s (%s)%s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_server_addr, score->sce_server_label,
            (outform & OF_ONELINE) ? "" : "\n");
        }

        if (score->sce_protocol[0]) {
          printf("%sprotocol: %s%s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_protocol,
            (outform & OF_ONELINE) ? "" : "\n");
        }

        if (score->sce_class[0]) {
          printf("%sclass: %s",
            (outform & OF_ONELINE) ? " " : "\t",
            score->sce_class);
        }
      }

      printf("%s", "\n");
    }
  }
  util_close_scoreboard();

  if (total) {
    register unsigned int i = 0;

    for (i = 0; i != MAX_CLASSES; i++) {
      if (classes[i].score_class == 0)
         break;

       printf("Service class %-20s - %3lu user%s\n", classes[i].score_class,
         classes[i].score_count, classes[i].score_count > 1 ? "s" : "");
    }

  } else {
    printf("no users connected\n");
  }

  return 0;
}
