/*
 * ProFTPD: mod_exec -- a module for executing external scripts
 * Copyright (c) 2002-2025 TJ Saunders
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, TJ Saunders gives permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 *
 * This is mod_exec, contrib software for proftpd 1.3.x and above.
 * For more information contact TJ Saunders <tj@castaglia.org>.
 */

#include "conf.h"
#include "privs.h"
#include "logfmt.h"
#include "jot.h"

#if defined(HAVE_SYS_RESOURCE_H)
# include <sys/resource.h>
#endif

#define MOD_EXEC_VERSION	"mod_exec/1.0"

/* Make sure the version of proftpd is as necessary. */
#if PROFTPD_VERSION_NUMBER < 0x0001030701
# error "ProFTPD 1.3.7rc1 or later required"
#endif

module exec_module;

static pool *exec_pool = NULL;
static int exec_engine = FALSE;
static unsigned int exec_nexecs = 0;

static int exec_logfd = -1;
static char *exec_logname = NULL;

static int exec_stdin_pipe[2];
static int exec_stdout_pipe[2];
static int exec_stderr_pipe[2];

static unsigned int exec_opts = 0U;
#define EXEC_OPT_LOG_STDOUT	0x0001
#define EXEC_OPT_LOG_STDERR	0x0002
#define EXEC_OPT_SEND_STDOUT	0x0004
#define EXEC_OPT_USE_STDIN	0x0008

static int exec_timeout = 0;

/* Flags for exec_ssystem() */
#define EXEC_FL_CLEAR_GROUPS	0x0010	/* Clear supplemental groups */
#define EXEC_FL_NO_SEND		0x0020	/* Do not send output via response */
#define EXEC_FL_USE_SEND	0x0040	/* Use pr_response_send() instead of
					 * pr_response_add()
					 */
#define EXEC_FL_RUN_AS_ROOT	0x0080  /* Use root privs when executing
                                         * the command.  USE CAREFULLY!
                                         */
#define EXEC_FL_RUN_AS_USER	0x0100  /* Use user privs when executing
                                         * the command.  Useful for pre-login
                                         * events.
                                         */

/* config_rec index for various stashed info */
#define EXEC_IDX_TRIGGER_CMDS		1
#define EXEC_IDX_LOGFMTS		2

struct exec_jot_buffer {
  char *ptr, *buf;
  size_t bufsz, buflen;
};

struct exec_event_data {
  unsigned int flags;
  config_rec *c;
  const char *event;
};

/* Prototypes */
static void exec_any_ev(const void *, void *);
static const char *exec_subst_var(pool *p, cmd_rec *cmd,
  const char *text, unsigned char *logfmt);
static int exec_log(const char *, ...)
#if defined(__GNUC__)
      __attribute__ ((format (printf, 1, 2)));
#else
      ;
#endif
static int exec_sess_init(void);

static const char *trace_channel = "exec";

/* Support routines
 */

static int exec_closelog(void) {
  /* sanity check */
  if (exec_logfd != -1) {
    (void) close(exec_logfd);
    exec_logfd = -1;
    exec_logname = NULL;
  }

  return 0;
}

static int exec_enabled(void) {
  config_rec *c;
  int enabled = TRUE;

  c = find_config(CURRENT_CONF, CONF_PARAM, "ExecEnable", FALSE);
  if (c != NULL) {
    enabled = *((int *) c->argv[0]);
  }

  return enabled;
}

static char *exec_get_cmd(char **list) {
  char *res = NULL, *dst = NULL;
  unsigned char quote_mode = FALSE;

  while (**list && PR_ISSPACE(**list)) {
    (*list)++;
  }

  if (!**list) {
    return NULL;
  }

  res = dst = *list;

  if (**list == '\"') {
    quote_mode = TRUE;
    (*list)++;
  }

  while (**list && **list != ',' &&
      (quote_mode ? (**list != '\"') : (!PR_ISSPACE(**list)))) {

    if (**list == '\\' && quote_mode) {
      /* Escaped char */
      if (*((*list) + 1)) {
        *dst = *(++(*list));
      }
    }

    *dst++ = **list;
    ++(*list);
  }

  if (**list) {
    (*list)++;
  }

  *dst = '\0';

  return res;
}

static int exec_log(const char *fmt, ...) {
  va_list msg;
  int res;

  if (exec_logname == NULL) {
    return 0;
  }

  va_start(msg, fmt);
  res = pr_log_vwritefile(exec_logfd, MOD_EXEC_VERSION, fmt, msg);
  va_end(msg);

  return res;
}

static int exec_match_cmd(cmd_rec *cmd, array_header *cmd_array) {
  register unsigned int i = 0;
  char **cmds = NULL;

  cmds = (char **) cmd_array->elts;

  for (i = 0; i < cmd_array->nelts && cmds[i]; i++) {
    if (strcasecmp(cmd->argv[0], cmds[i]) == 0) {
      return TRUE;
    }

    if (cmd->group != NULL &&
        strcasecmp(cmds[i], cmd->group) == 0) {
      return TRUE;
    }

    if (strcasecmp(cmds[i], "ALL") == 0) {
      return TRUE;
    }
  }

  return FALSE;
}

static int exec_openlog(void) {
  int res = 0;

  /* Sanity check */
  exec_logname = (char *) get_param_ptr(main_server->conf, "ExecLog", FALSE);
  if (exec_logname == NULL) {
    return 0;
  }

  /* Check for "none". */
  if (strcasecmp(exec_logname, "none") == 0) {
    exec_logname = NULL;
    return 0;
  }

  pr_signals_block();
  PRIVS_ROOT
  res = pr_log_openfile(exec_logname, &exec_logfd, PR_LOG_SYSTEM_MODE);
  PRIVS_RELINQUISH
  pr_signals_unblock();

  return res;
}

static void exec_parse_cmd_args(config_rec *c, cmd_rec *cmd,
    unsigned int start_idx) {
  register unsigned int i, j;
  pool *tmp_pool;
  array_header *logfmts = NULL;
  pr_jot_ctx_t *jot_ctx;
  pr_jot_parsed_t *jot_parsed;
  unsigned char parsed_buf[1024];

  logfmts = make_array(c->pool, 0, sizeof(unsigned char *));

  tmp_pool = make_sub_pool(c->pool);
  pr_pool_tag(tmp_pool, "exec cmd args pool");

  jot_parsed = pcalloc(tmp_pool, sizeof(pr_jot_parsed_t));
  jot_ctx = pcalloc(tmp_pool, sizeof(pr_jot_ctx_t));
  jot_ctx->log = jot_parsed;

  for (i = start_idx, j = 2; i < cmd->argc; i++, j++) {
    int res;
    char *text;
    unsigned char *logfmt;

    /* We make a copy of the plaintext, AND we parse it for any LogFormat
     * variables for later resolution.
     */
    text = pstrdup(c->pool, cmd->argv[i]);

    jot_parsed->bufsz = jot_parsed->buflen = sizeof(parsed_buf);
    jot_parsed->ptr = jot_parsed->buf = parsed_buf;

    res = pr_jot_parse_logfmt(tmp_pool, text, jot_ctx, pr_jot_parse_on_meta,
      pr_jot_parse_on_unknown, pr_jot_parse_on_other,
      PR_JOT_LOGFMT_PARSE_FL_UNKNOWN_AS_CUSTOM);
    if (res < 0) {
      pr_trace_msg(trace_channel, 2, "error parsing text '%s' for %s: %s",
        text, (char *) c->argv[0], strerror(errno));
      logfmt = (unsigned char *) text;

    } else {
      size_t logfmt_len;

      logfmt_len = jot_parsed->bufsz - jot_parsed->buflen;
      logfmt = palloc(c->pool, logfmt_len + 1);
      memcpy(logfmt, parsed_buf, logfmt_len);
      logfmt[logfmt_len] = '\0';
    }

    *((unsigned char **) push_array(logfmts)) = logfmt;
    c->argv[EXEC_IDX_LOGFMTS + j] = text;
  }

  /* Store the array of logfmts in the config_rec. */
  c->argv[EXEC_IDX_LOGFMTS] = logfmts;

  destroy_pool(tmp_pool);
}

static void exec_parse_trigger_cmds(config_rec *c, char *cmds) {
  char *cmd = NULL;
  array_header *cmd_array = NULL;

  cmd_array = make_array(c->pool, 0, sizeof(char *));

  /* Add each command to the array. */
  while ((cmd = exec_get_cmd(&cmds)) != NULL) {
    *((char **) push_array(cmd_array)) = pstrdup(c->pool, cmd);
  }

  /* Terminate the array with a NULL. */
  *((char **) push_array(cmd_array)) = NULL;

  /* Store the array of commands in the config_rec. */
  c->argv[EXEC_IDX_TRIGGER_CMDS] = cmd_array;
}

static char **exec_prepare_environ(pool *env_pool, cmd_rec *cmd) {
  config_rec *c = NULL;
  array_header *env = make_array(env_pool, 0, sizeof(char *));

  c = find_config(main_server->conf, CONF_PARAM, "ExecEnviron", FALSE);
  while (c != NULL) {
    const char *key, *val = NULL, *text;
    unsigned char *logfmt;

    pr_signals_handle();

    key = c->argv[0];
    text = c->argv[1];
    logfmt = c->argv[2];

    if (strcmp("-", text) == 0) {
      val = getenv(key);

    } else {
      val = exec_subst_var(env_pool, cmd, text, logfmt);
    }

    *((char **) push_array(env)) = pstrcat(env_pool, key, "=",
      val != NULL ? val : "", NULL);

    c = find_config_next(c, c->next, CONF_PARAM, "ExecEnviron", FALSE);
  }

  /* Make sure the environment is NULL-terminated. */
  *((char **) push_array(env)) = NULL;

  return (char **) env->elts;
}

static void exec_prepare_fds(int stdin_fd, int stdout_fd, int stderr_fd) {
  if (stdin_fd < 0) {
    stdin_fd = open("/dev/null", O_RDONLY);
    if (stdin_fd < 0) {
      exec_log("error: unable to open /dev/null for stdin: %s",
        strerror(errno));

    } else {
      if (dup2(stdin_fd, STDIN_FILENO) < 0)
        exec_log("error: unable to dup fd %d to stdin: %s", stdin_fd,
          strerror(errno));

      (void) close(stdin_fd);
    }

  } else {
    if (stdin_fd != STDIN_FILENO) {
      if (dup2(stdin_fd, STDIN_FILENO) < 0) {
        exec_log("error: unable to dup fd %d to stdin: %s", stdin_fd,
          strerror(errno));
      }

      (void) close(stdin_fd);
    }
  }

  if (stdout_fd != STDOUT_FILENO) {
    if (dup2(stdout_fd, STDOUT_FILENO) < 0) {
      exec_log("error: unable to dup fd %d to stdout: %s", stdout_fd,
        strerror(errno));
    }

    (void) close(stdout_fd);
  }

  if (stderr_fd != STDERR_FILENO) {
    if (dup2(stderr_fd, STDERR_FILENO) < 0) {
      exec_log("error: unable to dup fd %d to stderr: %s", stderr_fd,
        strerror(errno));
    }

    (void) close(stderr_fd);
  }

  /* Make sure not to pass on open file descriptors.  For stdin, we
   * dup /dev/null.  For stdout and stderr, we dup some pipes, so that
   * we can capture what the command may write to stdout or stderr.  The
   * stderr output will be logged to the ExecLog.
   */
  pr_fs_close_extra_fds();
}

static void exec_prepare_pipes(void) {

  /* Open pipes for stdin, stdout and stderr. */
  if (exec_opts & EXEC_OPT_USE_STDIN) {
    if (pipe(exec_stdin_pipe) < 0) {
      exec_log("error: unable to open stdin pipe: %s", strerror(errno));
      exec_stdin_pipe[0] = STDIN_FILENO;
      exec_stdin_pipe[1] = -1;

    } else {
      if (fcntl(exec_stdin_pipe[0], F_SETFD, 0) < 0) {
        exec_log("error: unable to set cloexec flag on stdin pipe read fd: %s",
          strerror(errno));
      }

      if (fcntl(exec_stdin_pipe[1], F_SETFD, FD_CLOEXEC) < 0) {
        exec_log("error: unable to set cloexec flag on stdin pipe write fd: %s",
          strerror(errno));
      }
    }
  }

  if (pipe(exec_stdout_pipe) < 0) {
    exec_log("error: unable to open stdout pipe: %s", strerror(errno));
    exec_stdout_pipe[0] = -1;
    exec_stdout_pipe[1] = STDOUT_FILENO;

  } else {
    if (fcntl(exec_stdout_pipe[0], F_SETFD, FD_CLOEXEC) < 0) {
      exec_log("error: unable to set cloexec flag on stdout pipe read fd: %s",
        strerror(errno));
    }

    if (fcntl(exec_stdout_pipe[1], F_SETFD, 0) < 0) {
      exec_log("error: unable to set cloexec flag on stdout pipe write fd: %s",
        strerror(errno));
    }
  }

  if (pipe(exec_stderr_pipe) < 0) {
    exec_log("error: unable to open stderr pipe: %s", strerror(errno));
    exec_stderr_pipe[0] = -1;
    exec_stderr_pipe[1] = STDERR_FILENO;

  } else {
    if (fcntl(exec_stderr_pipe[0], F_SETFD, FD_CLOEXEC) < 0) {
      exec_log("error: unable to set cloexec flag on stderr pipe read fd: %s",
        strerror(errno));
    }

    if (fcntl(exec_stderr_pipe[1], F_SETFD, 0) < 0) {
      exec_log("error: unable to set cloexec flag on stderr pipe write fd: %s",
        strerror(errno));
    }
  }
}

/* Provides a "safe" version of the system(2) call by dropping all special
 * privileges, currently retained by the daemon, before exec()'ing the
 * given command.
 */
static int exec_ssystem(cmd_rec *cmd, config_rec *c, int flags) {
  pid_t pid;
  int status;
  unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
  const char *path;

  struct sigaction sa_ignore, sa_intr, sa_quit;
  sigset_t set_chldmask, set_save;

  /* Prepare signal dispositions. */
  sa_ignore.sa_handler = SIG_IGN;
  sigemptyset(&sa_ignore.sa_mask);
  sa_ignore.sa_flags = 0;

  if (sigaction(SIGINT, &sa_ignore, &sa_intr) < 0) {
    return errno;
  }

  if (sigaction(SIGQUIT, &sa_ignore, &sa_quit) < 0) {
    return errno;
  }

  sigemptyset(&set_chldmask);
  sigaddset(&set_chldmask, SIGCHLD);

  if (sigprocmask(SIG_BLOCK, &set_chldmask, &set_save) < 0) {
    exec_log("sigprocmask error: %s", strerror(errno));
    return errno;
  }

  exec_prepare_pipes();
  path = c->argv[path_idx];

  pid = fork();
  if (pid < 0) {
    int xerrno = errno;

    pr_log_pri(PR_LOG_ALERT,
      MOD_EXEC_VERSION ": error: unable to fork: %s", strerror(xerrno));
    exec_log("error: unable to fork: %s", strerror(xerrno));

    errno = xerrno;
    status = -1;

  } else if (pid == 0) {
    register unsigned int i = 0;
    char **env = NULL, *ptr = NULL;
    pool *tmp_pool;

    /* Child process */

    /* Note: there is no need to clean up this temporary pool, as we've
     * forked.  If the exec call succeeds, this child process will exit
     * normally, and its process space recovered by the OS.  If the exec
     * call fails, we still exit, and the process space is recovered by
     * the OS.  Either way, the memory will be cleaned up without need for
     * us to do it explicitly (unless one wanted to be pedantic about it,
     * of course).
     */
    tmp_pool = cmd ? cmd->tmp_pool : make_sub_pool(session.pool);

    /* Don't forget to update the PID. */
    session.pid = getpid();

    if (!(exec_opts & EXEC_OPT_USE_STDIN)) {
      register unsigned int j;
      array_header *logfmts;

      /* Prepare the environment. */
      env = exec_prepare_environ(tmp_pool, cmd);

      logfmts = c->argv[EXEC_IDX_LOGFMTS];

      /* Perform any required substitution on the command arguments. */
      for (i = path_idx+1, j = 0; i < c->argc; i++, j++) {
        unsigned char *logfmt;

        pr_signals_handle();

        logfmt = ((unsigned char **) logfmts->elts)[j];
        c->argv[i] = (void *) exec_subst_var(tmp_pool, cmd, c->argv[i], logfmt);
      }

    } else {
      /* Make sure that env is at least a NULL-terminated array. */
      env = pcalloc(tmp_pool, sizeof(char **));
    }

    /* Restore previous signal actions. */
    sigaction(SIGINT, &sa_intr, NULL);
    sigaction(SIGQUIT, &sa_quit, NULL);
    sigprocmask(SIG_SETMASK, &set_save, NULL);

    /* Per Bug#4049, when running the command as the user, do NOT clear
     * the supplemental groups.
     */
    if (flags & EXEC_FL_RUN_AS_USER) {
      flags &= ~EXEC_FL_CLEAR_GROUPS;
    }

    /* If requested, clear the supplemental group membership of the process. */
    if (flags & EXEC_FL_CLEAR_GROUPS) {
      PRIVS_ROOT
      setgroups(0, NULL);
      PRIVS_RELINQUISH
    }

    if (flags & EXEC_FL_RUN_AS_ROOT) {
      /* We were asked to run using root privs.  Yuck. */
      PRIVS_ROOT

    } else if (flags & EXEC_FL_RUN_AS_USER) {
      /* We were asked to run using user privs.  Sigh. */
      if (geteuid() != session.login_uid) {
        PRIVS_SETUP(session.login_uid, session.login_gid)
      }

      PRIVS_REVOKE

    } else {
      /* Drop all special privileges before exec()'ing the command.  This
       * allows for the user to specify arbitrary input via the given
       * filename without the admin worrying that some arbitrary command
       * is being executed that could take advantage of proftpd's retention
       * of root real user ID.
       */
      PRIVS_REVOKE
    }

    exec_log("preparing to execute '%s' with uid %s (euid %s), "
      "gid %s (egid %s)", path,
      pr_uid2str(tmp_pool, getuid()), pr_uid2str(tmp_pool, geteuid()),
      pr_gid2str(tmp_pool, getgid()), pr_gid2str(tmp_pool, getegid()));

    /* Trim the given path to the command to execute to just the last
     * component; this name will be the first argument to the executed
     * command, as per execve(2) convention.
     */
    ptr = strrchr(c->argv[path_idx], '/');
    c->argv[path_idx] = ptr + 1;

    for (i = path_idx+1; i < c->argc; i++) {
      if (c->argv[i] != NULL) {
        exec_log(" + '%s': argv[%u] = %s", (const char *) path,
          i - path_idx, (const char *) c->argv[i]);
      }
    }

    /* Prepare the file descriptors that the process will inherit. */
    if (exec_opts & EXEC_OPT_USE_STDIN) {
      exec_prepare_fds(exec_stdin_pipe[0], exec_stdout_pipe[1],
        exec_stderr_pipe[1]);

    } else {
      exec_prepare_fds(-1, exec_stdout_pipe[1], exec_stderr_pipe[1]);
    }

    errno = 0;

    /* If we are using stdin to convey all of the arguments, then we need
     * not provide any sort of environment variables.
     */
    if (exec_opts & EXEC_OPT_USE_STDIN) {
      char *args[] = { ptr + 1, NULL };

      execve(path, args, env);

    } else {
      execve(path, (char **) (c->argv + path_idx), env);
    }

    /* Since all previous file descriptors (including those for log files)
     * have been closed, and root privs have been revoked, there's little
     * chance of directing a message of execve() failure to proftpd's log
     * files.  execve() only returns if there's an error; the only way we
     * can signal this to the waiting parent process is to exit with a
     * non-zero value (the value of errno will do nicely).
     */
    exit(errno);

  } else {
    int res;

    /* Parent process */

    /* If we are using stdin, then we need to write all of the arguments to
     * stdin before closing that pipe.
     */
    if (exec_opts & EXEC_OPT_USE_STDIN) {
      register unsigned int i, j;
      int maxfd = -1, fds;
      fd_set writefds;
      struct timeval tv;
      array_header *logfmts;
      pool *tmp_pool;

      tmp_pool = cmd ? cmd->tmp_pool : make_sub_pool(session.pool);

      /* Wait for stdin to be available for writing. */

      FD_ZERO(&writefds);
      FD_SET(exec_stdin_pipe[1], &writefds);
      if (exec_stdin_pipe[1] > maxfd) {
        maxfd = exec_stdin_pipe[1];
      }

      /* Note: this delay should be configurable somehow. */
      tv.tv_sec = 2L;
      tv.tv_usec = 0L;

      fds = select(maxfd + 1, &writefds, NULL, NULL, &tv);

      if (fds == -1 &&
          errno == EINTR) {
        pr_signals_handle();
      }

      logfmts = c->argv[EXEC_IDX_LOGFMTS];

      /* Perform any required substitution on the command arguments. */
      for (i = path_idx+1, j = 0; i < c->argc; i++, j++) {
        unsigned char *logfmt;

        pr_signals_handle();

        logfmt = ((unsigned char **) logfmts->elts)[j];

        /* Handle the NULL-terminated argv lists here, since we are processing
         * it manually for sending via stdin.
         */
        if (c->argv[i] == NULL ||
            logfmt == NULL) {
          break;
        }

        c->argv[i] = (void *) exec_subst_var(tmp_pool, cmd, c->argv[i], logfmt);

        /* Write the argument to stdin, terminated by a newline. */
        if (write(exec_stdin_pipe[1], c->argv[i], strlen(c->argv[i])) < 0) {
          exec_log("error writing argument to stdin: %s", strerror(errno));

        } else {
          exec_log("wrote argument %u (%s) to stdin (%d)", i - path_idx,
            (char *) c->argv[i], exec_stdin_pipe[1]);
        }

        if (write(exec_stdin_pipe[1], "\n", 1) < 0) {
          exec_log("error writing newline to stdin: %s", strerror(errno));
        }
      }

      /* Signal the end of stdin arguments using a period followed by a newline.
       * (This is the end-of-arguments indicated used by other protocols such
       * as SMTP.)
       */
      if (write(exec_stdin_pipe[1], ".\n", 2) < 0) {
        exec_log("error writing end-of-argument indicator: %s",
          strerror(errno));
      }

      if (cmd == NULL) {
        destroy_pool(tmp_pool);
      }
    }

    if (exec_opts & EXEC_OPT_USE_STDIN) {
      (void) close(exec_stdin_pipe[0]);
      exec_stdin_pipe[0] = -1;
    }

    (void) close(exec_stdout_pipe[1]);
    exec_stdout_pipe[1] = -1;

    (void) close(exec_stderr_pipe[1]);
    exec_stderr_pipe[1] = -1;

    if ((exec_opts & EXEC_OPT_LOG_STDOUT) ||
        (exec_opts & EXEC_OPT_LOG_STDERR) ||
        (exec_opts & EXEC_OPT_SEND_STDOUT) ||
        exec_timeout > 0) {
      int maxfd = -1, fds, send_sigterm = 1;
      fd_set readfds;
      struct timeval tv;
      time_t start_time = time(NULL);
      pool *tmp_pool;

      tmp_pool = cmd ? cmd->tmp_pool : make_sub_pool(session.pool);

      /* We set the result value to zero initially, so that at least one
       * pass through the stdout/stderr reading code happens.
       */
      res = 0;
      while (res <= 0) {
        if (res < 0) {
          if (errno != EINTR) {
            exec_log("error: unable to wait for pid %d: %s", pid,
              strerror(errno));
            status = -1;
            break;

          } else {
            pr_signals_handle();
            continue;
          }
        }

        if (exec_timeout > 0) {
          /* Check the time elapsed since we started. */
          if ((time(NULL) - start_time) > exec_timeout) {

            /* Send TERM, the first time, to be polite. */
            if (send_sigterm) {
              send_sigterm = 0;
              exec_log("'%s' has exceeded ExecTimeout (%lu seconds), sending "
                "SIGTERM (signal %d)", path, (unsigned long) exec_timeout,
                SIGTERM);
              kill(pid, SIGTERM);

            } else {
              /* The child is still around?  Terminate with extreme
               * prejudice.
               */
              exec_log("'%s' has exceeded ExecTimeout (%lu seconds), sending "
                "SIGKILL (signal %d)", path, (unsigned long) exec_timeout,
                SIGKILL);
              kill(pid, SIGKILL);
            }
          }
        }

        /* Select on the pipe read fds, to see if the child has anything
         * to tell us.
         */
        FD_ZERO(&readfds);

        if ((exec_opts & EXEC_OPT_LOG_STDOUT) ||
            (exec_opts & EXEC_OPT_SEND_STDOUT)) {
          FD_SET(exec_stdout_pipe[0], &readfds);

          if (exec_stdout_pipe[0] > maxfd) {
            maxfd = exec_stdout_pipe[0];
          }
        }

        if (exec_opts & EXEC_OPT_LOG_STDERR) {
          FD_SET(exec_stderr_pipe[0], &readfds);

          if (exec_stderr_pipe[0] > maxfd) {
            maxfd = exec_stderr_pipe[0];
          }
        }

        /* Note: this delay should be configurable somehow. */
        tv.tv_sec = 2L;
        tv.tv_usec = 0L;

        fds = select(maxfd + 1, &readfds, NULL, NULL, &tv);
        if (fds == -1 &&
            errno == EINTR) {
          pr_signals_handle();
        }

        if (fds >= 0) {
          long buflen, bufsz;
          char *buf;

          buf = pr_fsio_getpipebuf(tmp_pool, exec_stdout_pipe[0], &bufsz);

          /* The child sent us something.  How thoughtful. */

          if (FD_ISSET(exec_stdout_pipe[0], &readfds)) {
            memset(buf, '\0', bufsz);

            buflen = read(exec_stdout_pipe[0], buf, bufsz-1);
            if (buflen > 0) {
              if (exec_opts & EXEC_OPT_SEND_STDOUT) {

                if (!(flags & EXEC_FL_NO_SEND)) {
                  if (flags & EXEC_FL_USE_SEND) {
                    pr_response_send(R_220, "%s", buf);

                  } else {
                    pr_response_add(R_DUP, "%s", buf);
                  }

                } else {
                  exec_log("not appropriate to send stdout to client at "
                    "this time");
                }
              }

              /* Trim trailing CRs and LFs. */
              while (buflen > 0 &&
                     (buf[buflen-1] == '\r' || buf[buflen-1] == '\n')) {
                pr_signals_handle();
                buf[buflen-1] = '\0';
                buflen--;
              }

              /* We told read(2) that the size of buf is one less than its
               * actual size.  Which means that the buflen value returned
               * by read(2) can, at most, be one less than the size of buf.
               * Thus it should be OK to do the following.
               */
              buf[buflen] = '\0';

              if (exec_opts & EXEC_OPT_LOG_STDOUT) {
                exec_log("stdout from '%s': '%s'", path, buf);
              }

            } else if (buflen < 0) {
              if (errno != 0) {
                exec_log("error reading stdout from '%s': %s", path,
                  strerror(errno));
              }
            }
          }

          if (FD_ISSET(exec_stderr_pipe[0], &readfds)) {
            memset(buf, '\0', bufsz);

            buflen = read(exec_stderr_pipe[0], buf, bufsz-1);
            if (buflen > 0) {

              /* Trim trailing CRs and LFs. */
              while (buflen > 0 &&
                     (buf[buflen-1] == '\r' || buf[buflen-1] == '\n')) {
                pr_signals_handle();
                buf[buflen-1] = '\0';
                buflen--;
              }

              /* We told read(2) that the size of buf is one less than its
               * actual size.  Which means that the buflen value returned
               * by read(2) can, at most, be one less than the size of buf.
               * Thus it should be OK to do the following.
               */
              buf[buflen] = '\0';

              if (exec_opts & EXEC_OPT_LOG_STDERR) {
                exec_log("stderr from '%s': '%s'", path, buf);
              }

            } else if (buflen < 0) {
              if (errno != 0) {
                exec_log("error reading stderr from '%s': %s", path,
                  strerror(errno));
              }
            }
          }
        }

        res = waitpid(pid, &status, WNOHANG);
      }

      if (cmd == NULL) {
        destroy_pool(tmp_pool);
      }

    } else {
      res = waitpid(pid, &status, 0);
      while (res <= 0) {
        if (res < 0) {
          if (errno != EINTR) {
            exec_log("error: unable to wait for pid %d: %s", pid,
              strerror(errno));
            status = -1;
            break;

          } else {
            pr_signals_handle();
            continue;
          }
        }

        res = waitpid(pid, &status, 0);
      }
    }
  }

  close(exec_stdout_pipe[0]);
  close(exec_stderr_pipe[0]);

  /* Restore the previous signal actions. */
  if (sigaction(SIGINT, &sa_intr, NULL) < 0) {
    exec_log("sigaction() error: %s", strerror(errno));
    return errno;
  }

  if (sigaction(SIGQUIT, &sa_quit, NULL) < 0) {
    exec_log("sigaction() error: %s", strerror(errno));
    return errno;
  }

  if (sigprocmask(SIG_SETMASK, &set_save, NULL) < 0) {
    exec_log("sigprocmask() error: %s", strerror(errno));
    return errno;
  }

  if (WIFEXITED(status)) {
    int exit_status;

    exit_status = WEXITSTATUS(status);
    exec_log("'%s' terminated normally, with exit status %d", path,
      exit_status);
    return exit_status;
  }

  if (WIFSIGNALED(status)) {
    exec_log("'%s' died from signal %d", path, WTERMSIG(status));

    if (WCOREDUMP(status)) {
      exec_log("'%s' created a coredump", path);
    }

    return EPERM;
  }

  return status;
}

static void exec_jot_append_text(struct exec_jot_buffer *log, const char *text,
    size_t text_len) {
  if (text == NULL ||
      text_len == 0) {
    return;
  }

  if (text_len > log->buflen) {
    text_len = log->buflen;
  }

  pr_trace_msg(trace_channel, 19, "appending text '%.*s' (%lu) to buffer",
    (int) text_len, text, (unsigned long) text_len);
  memcpy(log->buf, text, text_len);
  log->buf += text_len;
  log->buflen -= text_len;
}

static int resolve_on_meta(pool *p, pr_jot_ctx_t *jot_ctx,
    unsigned char logfmt_id, const char *jot_hint, const void *val) {
  struct exec_jot_buffer *log;

  log = jot_ctx->log;
  if (log->buflen > 0) {
    const char *text = NULL;
    size_t text_len = 0;
    char buf[1024];

    switch (logfmt_id) {
      case LOGFMT_META_CUSTOM: {
        const char *key;

        /* The Var API is particular about the format of the keys. */
        key = pstrcat(p, "%{", val, "}", NULL);
        text = pr_var_get(key);
        break;
      }

      case LOGFMT_META_MICROSECS: {
        unsigned long num;

        num = *((double *) val);
        text_len = pr_snprintf(buf, sizeof(buf)-1, "%06lu", num);
        buf[text_len] = '\0';
        text = buf;
        break;
      }

      case LOGFMT_META_MILLISECS: {
        unsigned long num;

        num = *((double *) val);
        text_len = pr_snprintf(buf, sizeof(buf)-1, "%03lu", num);
        buf[text_len] = '\0';
        text = buf;
        break;
      }

      case LOGFMT_META_LOCAL_PORT:
      case LOGFMT_META_REMOTE_PORT:
      case LOGFMT_META_RESPONSE_CODE:
      case LOGFMT_META_XFER_PORT: {
        int num;

        num = *((double *) val);
        text_len = pr_snprintf(buf, sizeof(buf)-1, "%d", num);
        buf[text_len] = '\0';
        text = buf;
        break;
      }

      case LOGFMT_META_UID: {
        uid_t uid;

        uid = *((double *) val);
        text = pr_uid2str(p, uid);
        break;
      }

      case LOGFMT_META_GID: {
        gid_t gid;

        gid = *((double *) val);
        text = pr_gid2str(p, gid);
        break;
      }

      case LOGFMT_META_BYTES_SENT:
      case LOGFMT_META_FILE_OFFSET:
      case LOGFMT_META_FILE_SIZE:
      case LOGFMT_META_RAW_BYTES_IN:
      case LOGFMT_META_RAW_BYTES_OUT:
      case LOGFMT_META_RESPONSE_MS:
      case LOGFMT_META_XFER_MS: {
        off_t num;

        num = *((double *) val);
        text_len = pr_snprintf(buf, sizeof(buf)-1, "%" PR_LU, (pr_off_t) num);
        buf[text_len] = '\0';
        text = buf;
        break;
      }

      case LOGFMT_META_EPOCH:
      case LOGFMT_META_PID: {
        unsigned long num;

        num = *((double *) val);
        text_len = pr_snprintf(buf, sizeof(buf)-1, "%lu", num);
        buf[text_len] = '\0';
        text = buf;
        break;
      }

      case LOGFMT_META_FILE_MODIFIED: {
        int truth;

        truth = *((int *) val);
        text = truth ? "true" : "false";
        break;
      }

      case LOGFMT_META_SECONDS: {
        float num;

        num = *((double *) val);
        text_len = pr_snprintf(buf, sizeof(buf)-1, "%0.3f", num);
        buf[text_len] = '\0';
        text = buf;
        break;
      }

      case LOGFMT_META_ANON_PASS:
      case LOGFMT_META_BASENAME:
      case LOGFMT_META_CLASS:
      case LOGFMT_META_CMD_PARAMS:
      case LOGFMT_META_COMMAND:
      case LOGFMT_META_DIR_NAME:
      case LOGFMT_META_DIR_PATH:
      case LOGFMT_META_ENV_VAR:
      case LOGFMT_META_EOS_REASON:
      case LOGFMT_META_FILENAME:
      case LOGFMT_META_GROUP:
      case LOGFMT_META_IDENT_USER:
      case LOGFMT_META_ISO8601:
      case LOGFMT_META_LOCAL_FQDN:
      case LOGFMT_META_LOCAL_IP:
      case LOGFMT_META_LOCAL_NAME:
      case LOGFMT_META_METHOD:
      case LOGFMT_META_NOTE_VAR:
      case LOGFMT_META_ORIGINAL_USER:
      case LOGFMT_META_PROTOCOL:
      case LOGFMT_META_REMOTE_HOST:
      case LOGFMT_META_REMOTE_IP:
      case LOGFMT_META_RENAME_FROM:
      case LOGFMT_META_RESPONSE_STR:
      case LOGFMT_META_TIME:
      case LOGFMT_META_USER:
      case LOGFMT_META_VERSION:
      case LOGFMT_META_VHOST_IP:
      case LOGFMT_META_XFER_FAILURE:
      case LOGFMT_META_XFER_PATH:
      case LOGFMT_META_XFER_SPEED:
      case LOGFMT_META_XFER_STATUS:
      case LOGFMT_META_XFER_TYPE:
      default:
        text = val;
    }

    if (text != NULL &&
        text_len == 0) {
      text_len = strlen(text);
    }

    exec_jot_append_text(log, text, text_len);
  }

  return 0;
}

static int resolve_on_other(pool *p, pr_jot_ctx_t *jot_ctx, unsigned char *text,
    size_t text_len) {
  struct exec_jot_buffer *log;

  log = jot_ctx->log;
  exec_jot_append_text(log, (const char *) text, text_len);
  return 0;
}

/* Perform any substitution of "magic cookie" values. */
static const char *exec_subst_var(pool *p, cmd_rec *cmd,
    const char *text, unsigned char *logfmt) {
  int res;
  pool *tmp_pool;
  pr_jot_ctx_t *jot_ctx;
  struct exec_jot_buffer *ejb;
  char resolved_buf[2048];
  const char *resolved_text = NULL;

  if (text == NULL ||
      logfmt == NULL) {
    return NULL;
  }

  tmp_pool = make_sub_pool(p);
  pr_pool_tag(tmp_pool, "exec jot pool");

  ejb = pcalloc(tmp_pool, sizeof(struct exec_jot_buffer));
  ejb->bufsz = ejb->buflen = sizeof(resolved_buf)-1;
  ejb->ptr = ejb->buf = resolved_buf;

  jot_ctx = pcalloc(tmp_pool, sizeof(pr_jot_ctx_t));
  jot_ctx->log = ejb;

  res = pr_jot_resolve_logfmt(tmp_pool, cmd, NULL, logfmt, jot_ctx,
    resolve_on_meta, NULL, resolve_on_other);
  if (res == 0) {
    size_t resolved_buflen;

    resolved_buflen = ejb->bufsz - ejb->buflen;
    resolved_text = pstrndup(p, resolved_buf, resolved_buflen);

  } else {
    pr_trace_msg(trace_channel, 3, "error resolving '%s' text: %s", text,
      strerror(errno));
  }

  destroy_pool(tmp_pool);
  return resolved_text;
}

/* Command handlers
 */

MODRET exec_log_exit(cmd_rec *cmd) {
  config_rec *c;

  if (exec_engine == FALSE) {
    return PR_DECLINED(cmd);
  }

  c = find_config(main_server->conf, CONF_PARAM, "ExecOnExit", FALSE);
  while (c != NULL) {
    int res;
    unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
    const char *path;

    pr_signals_handle();

    path = c->argv[path_idx];
    res = exec_ssystem(cmd, c, EXEC_FL_CLEAR_GROUPS|EXEC_FL_NO_SEND);
    if (res != 0) {
      exec_log("ExecOnExit '%s' failed: %s", path, strerror(res));

    } else {
      exec_log("ExecOnExit '%s' succeeded", path);
    }

    c = find_config_next(c, c->next, CONF_PARAM, "ExecOnExit", FALSE);
  }

  return PR_DECLINED(cmd);
}

MODRET exec_pre_cmd(cmd_rec *cmd) {
  config_rec *c = NULL;
  array_header *seen_execs = NULL;

  if (exec_engine == FALSE) {
    return PR_DECLINED(cmd);
  }

  if (exec_enabled() != TRUE) {
    return PR_DECLINED(cmd);
  }

  /* Create an array that will contain the IDs of the Execs we've
   * already processed.
   */
  seen_execs = make_array(cmd->tmp_pool, 0, sizeof(unsigned int));

  c = find_config(CURRENT_CONF, CONF_PARAM, "ExecBeforeCommand", FALSE);
  while (c != NULL) {
    pr_signals_handle();

    /* If we've already seen this Exec, skip on to the next Exec. */
    if (seen_execs->nelts > 0) {
      register unsigned int i = 0;
      unsigned char saw_exec = FALSE;
      unsigned int id = *((unsigned int *) c->argv[0]), *ids = seen_execs->elts;

      for (i = 0; i < seen_execs->nelts; i++) {
        if (ids[i] == id) {
          saw_exec = TRUE;
          break;
        }
      }

      if (saw_exec == TRUE) {
        exec_log("already saw this Exec, skipping");
        c = find_config_next(c, c->next, CONF_PARAM, "ExecBeforeCommand",
          FALSE);
        continue;
      }
    }

    /* Add this Exec's ID to the list of seen Execs. */
    *((unsigned int *) push_array(seen_execs)) = *((unsigned int *) c->argv[0]);

    /* Check the command list for this program against the current command. */
    if (exec_match_cmd(cmd, c->argv[EXEC_IDX_TRIGGER_CMDS]) == TRUE) {
      int res;
      unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
      const char *path;

      path = c->argv[path_idx];
      res = exec_ssystem(cmd, c, EXEC_FL_NO_SEND);
      if (res != 0) {
        exec_log("%s ExecBeforeCommand '%s' failed: %s", (char *) cmd->argv[0],
          path, strerror(res));

      } else {
        exec_log("%s ExecBeforeCommand '%s' succeeded", (char *) cmd->argv[0],
          path);
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, "ExecBeforeCommand", FALSE);
  }

  return PR_DECLINED(cmd);
}

MODRET exec_post_cmd(cmd_rec *cmd) {
  config_rec *c = NULL;
  array_header *seen_execs = NULL;

  if (exec_engine == FALSE) {
    return PR_DECLINED(cmd);
  }

  if (exec_enabled() != TRUE) {
    return PR_DECLINED(cmd);
  }

  /* Create an array that will contain the IDs of the Execs we've
   * already processed.
   */
  seen_execs = make_array(cmd->tmp_pool, 0, sizeof(unsigned int));

  c = find_config(CURRENT_CONF, CONF_PARAM, "ExecOnCommand", FALSE);
  while (c != NULL) {
    pr_signals_handle();

    /* If we've already seen this Exec, skip on to the next Exec. */
    if (seen_execs->nelts > 0) {
      register unsigned int i = 0;
      unsigned char saw_exec = FALSE;
      unsigned int id = *((unsigned int *) c->argv[0]), *ids = seen_execs->elts;

      for (i = 0; i < seen_execs->nelts; i++) {
        if (ids[i] == id) {
          saw_exec = TRUE;
          break;
        }
      }

      if (saw_exec == TRUE) {
        exec_log("already saw this Exec, skipping");
        c = find_config_next(c, c->next, CONF_PARAM, "ExecOnCommand", FALSE);
        continue;
      }
    }

    /* Add this Exec's ID to the list of seen Execs. */
    *((unsigned int *) push_array(seen_execs)) = *((unsigned int *) c->argv[0]);

    /* Check the command list for this program against the command. */
    if (exec_match_cmd(cmd, c->argv[EXEC_IDX_TRIGGER_CMDS]) == TRUE) {
      int res;
      unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
      const char *path;

      path = c->argv[path_idx];
      res = exec_ssystem(cmd, c, 0);
      if (res != 0) {
        exec_log("%s ExecOnCommand '%s' failed: %s", (char *) cmd->argv[0],
          path, strerror(res));

      } else {
        exec_log("%s ExecOnCommand '%s' succeeded", (char *) cmd->argv[0],
          path);
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, "ExecOnCommand", FALSE);
  }

  return PR_DECLINED(cmd);
}

MODRET exec_post_cmd_err(cmd_rec *cmd) {
  config_rec *c = NULL;
  array_header *seen_execs = NULL;

  if (exec_engine == FALSE) {
    return PR_DECLINED(cmd);
  }

  if (exec_enabled() != TRUE) {
    return PR_DECLINED(cmd);
  }

  /* Create an array that will contain the IDs of the Execs we've
   * already processed.
   */
  seen_execs = make_array(cmd->tmp_pool, 0, sizeof(unsigned int));

  c = find_config(CURRENT_CONF, CONF_PARAM, "ExecOnError", FALSE);
  while (c != NULL) {
    pr_signals_handle();

    /* If we've already seen this Exec, skip on to the next Exec. */
    if (seen_execs->nelts > 0) {
      register unsigned int i = 0;
      unsigned char saw_exec = FALSE;
      unsigned int id = *((unsigned int *) c->argv[0]), *ids = seen_execs->elts;

      for (i = 0; i < seen_execs->nelts; i++) {
        if (ids[i] == id) {
          saw_exec = TRUE;
          break;
        }
      }

      if (saw_exec == TRUE) {
        exec_log("already saw this Exec, skipping");
        c = find_config_next(c, c->next, CONF_PARAM, "ExecOnError", FALSE);
        continue;
      }
    }

    /* Add this Exec's ID to the list of seen Execs. */
    *((unsigned int *) push_array(seen_execs)) = *((unsigned int *) c->argv[0]);

    /* Check the command list for this program against the errored command. */
    if (exec_match_cmd(cmd, c->argv[EXEC_IDX_TRIGGER_CMDS]) == TRUE) {
      int res;
      unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
      const char *path;

      path = c->argv[path_idx];
      res = exec_ssystem(cmd, c, 0);
      if (res != 0) {
        exec_log("%s ExecOnError '%s' failed: %s", (char *) cmd->argv[0],
          path, strerror(res));

      } else {
        exec_log("%s ExecOnError '%s' succeeded", (char *) cmd->argv[0],
          path);
      }
    }

    c = find_config_next(c, c->next, CONF_PARAM, "ExecOnError", FALSE);
  }

  return PR_DECLINED(cmd);
}

/* Configuration directive handlers
 */

/* usage: ExecBeforeCommand cmds path [args] */
MODRET set_execbeforecommand(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned int path_idx = 2;
  char *path;

  if (cmd->argc-1 < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR);

  path = cmd->argv[path_idx];
  if (*path != '/') {
    CONF_ERROR(cmd, "path to program must be a full path");
  }

  c = add_config_param(cmd->argv[0], 0);
  c->argc = cmd->argc + 2;
  c->argv = pcalloc(c->pool, sizeof(void *) * (c->argc + 2));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = exec_nexecs++;

  exec_parse_trigger_cmds(c, cmd->argv[1]);

  /* Store the executable path. */
  c->argv[EXEC_IDX_LOGFMTS+1] = pstrdup(c->pool, path);

  exec_parse_cmd_args(c, cmd, path_idx+1);

  c->flags |= CF_MERGEDOWN_MULTI;
  return PR_HANDLED(cmd);
}

/* usage: ExecEnable on|off */
MODRET set_execenable(cmd_rec *cmd) {
  int enable = -1;
  config_rec *c;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ANON|CONF_DIR|CONF_DYNDIR);

  enable = get_boolean(cmd, 1);
  if (enable == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = palloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = enable;

  return PR_HANDLED(cmd);
}

/* usage: ExecEngine on|off */
MODRET set_execengine(cmd_rec *cmd) {
  int engine = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  engine = get_boolean(cmd, 1);
  if (engine == -1) {
    CONF_ERROR(cmd, "expected Boolean parameter");
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = engine;

  /* Also set this here, for the daemon process. */
  exec_engine = engine;

  return PR_HANDLED(cmd);
}

/* usage: ExecEnviron variable value */
MODRET set_execenviron(cmd_rec *cmd) {
  register unsigned int i = 0;
  config_rec *c = NULL;
  int res;
  char *key, *text;
  pr_jot_ctx_t *jot_ctx;
  pr_jot_parsed_t *jot_parsed;
  unsigned char parsed_buf[1024], *logfmt = NULL;
  size_t logfmt_len = 0;

  CHECK_ARGS(cmd, 2);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param_str(cmd->argv[0], 3, NULL, cmd->argv[2], NULL);

  /* Make sure the given environment variable name is uppercased.
   * NOTE: Are there cases where this SHOULD NOT happen?  Why should
   * environment variable names always be uppercased?
   */
  key = cmd->argv[1];

  for (i = 0; i < strlen(key); i++) {
    key[i] = toupper(key[i]);
  }

  c->argv[0] = pstrdup(c->pool, key);

  text = cmd->argv[2];

  jot_parsed = pcalloc(cmd->tmp_pool, sizeof(pr_jot_parsed_t));
  jot_parsed->bufsz = jot_parsed->buflen = sizeof(parsed_buf);
  jot_parsed->ptr = jot_parsed->buf = parsed_buf;

  jot_ctx = pcalloc(cmd->tmp_pool, sizeof(pr_jot_ctx_t));
  jot_ctx->log = jot_parsed;

  res = pr_jot_parse_logfmt(cmd->tmp_pool, text, jot_ctx, pr_jot_parse_on_meta,
    pr_jot_parse_on_unknown, pr_jot_parse_on_other, 0);
  if (res < 0) {
    pr_log_pri(PR_LOG_INFO, MOD_EXEC_VERSION ": error parsing '%s': %s",
      text, strerror(errno));
    logfmt = (unsigned char *) text;
    logfmt_len = strlen(text);

  } else {
    logfmt_len = jot_parsed->bufsz - jot_parsed->buflen;
    logfmt = palloc(cmd->tmp_pool, logfmt_len + 1);
    memcpy(logfmt, parsed_buf, logfmt_len);
    logfmt[logfmt_len] = '\0';
  }

  c->argv[1] = pstrdup(c->pool, text);
  c->argv[2] = pstrndup(c->pool, (char *) logfmt, logfmt_len);

  if (pr_module_exists("mod_ifsession.c")) {
    /* These are needed in case this directive is used with mod_ifsession
     * configuration.
     */
    c->flags |= CF_MULTI;
  }

  return PR_HANDLED(cmd);
}

/* usage: ExecLog path|"none" */
MODRET set_execlog(cmd_rec *cmd) {
  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  add_config_param_str(cmd->argv[0], 1, cmd->argv[1]);
  return PR_HANDLED(cmd);
}

/* usage: ExecOnCommand cmds path [args] */
MODRET set_execoncommand(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned int path_idx = 2;
  char *path;

  if (cmd->argc-1 < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR);

  path = cmd->argv[path_idx];
  if (*path != '/') {
    CONF_ERROR(cmd, "path to program must be a full path");
  }

  c = add_config_param(cmd->argv[0], 0);
  c->argc = cmd->argc + 2;
  c->argv = pcalloc(c->pool, sizeof(void *) * (c->argc + 2));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = exec_nexecs++;

  exec_parse_trigger_cmds(c, cmd->argv[1]);

  /* Store the executable path. */
  c->argv[EXEC_IDX_LOGFMTS+1] = pstrdup(c->pool, path);

  exec_parse_cmd_args(c, cmd, path_idx+1);

  c->flags |= CF_MERGEDOWN_MULTI;
  return PR_HANDLED(cmd);
}

/* usage: ExecOnConnect path [args] */
MODRET set_execonconnect(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned int path_idx = 1;
  char *path;

  if (cmd->argc-1 < 1) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  path = cmd->argv[path_idx];
  if (*path != '/') {
    CONF_ERROR(cmd, "path to program must be a full path");
  }

  c = add_config_param(cmd->argv[0], 0);
  c->argc = cmd->argc + 2;
  c->argv = pcalloc(c->pool, sizeof(void *) * (c->argc + 2));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = exec_nexecs++;

  if (pr_module_exists("mod_ifsession.c")) {
    /* These are needed in case this directive is used with mod_ifsession
     * configuration.
     */
    c->flags |= CF_MULTI;
  }

  /* Store the executable path. */
  c->argv[EXEC_IDX_LOGFMTS+1] = pstrdup(c->pool, path);

  exec_parse_cmd_args(c, cmd, path_idx+1);
  return PR_HANDLED(cmd);
}

/* usage: ExecOnError cmds path [args] */
MODRET set_execonerror(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned int path_idx = 2;
  char *path;

  if (cmd->argc-1 < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL|CONF_ANON|
    CONF_DIR);

  path = cmd->argv[path_idx];
  if (*path != '/') {
    CONF_ERROR(cmd, "path to program must be a full path");
  }

  c = add_config_param(cmd->argv[0], 0);
  c->argc = cmd->argc + 2;
  c->argv = pcalloc(c->pool, sizeof(void *) * (c->argc + 2));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = exec_nexecs++;

  exec_parse_trigger_cmds(c, cmd->argv[1]);

  /* Store the executable path. */
  c->argv[EXEC_IDX_LOGFMTS+1] = pstrdup(c->pool, path);

  exec_parse_cmd_args(c, cmd, path_idx+1);

  c->flags |= CF_MERGEDOWN_MULTI;
  return PR_HANDLED(cmd);
}

/* usage: ExecOnEvent event path [args] */
MODRET set_execonevent(cmd_rec *cmd) {
  unsigned int flags = EXEC_FL_CLEAR_GROUPS|EXEC_FL_NO_SEND;
  unsigned int path_idx = 2;
  char *event_name, *path;
  size_t event_namelen;
  config_rec *c;
  struct exec_event_data *eed;

  if (cmd->argc-1 < 2) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  event_name = cmd->argv[1];
  event_namelen = strlen(event_name);

  if (event_name[event_namelen-1] == '*') {
    flags |= EXEC_FL_RUN_AS_ROOT;
    event_name[event_namelen-1] = '\0';
    event_namelen--;

  } else if (event_name[event_namelen-1] == '~') {
    flags |= EXEC_FL_RUN_AS_USER;
    event_name[event_namelen-1] = '\0';
    event_namelen--;
  }

  path = cmd->argv[path_idx];
  if (*path != '/') {
    CONF_ERROR(cmd, "path to program must be a full path");
  }

  c = pcalloc(cmd->server->pool, sizeof(config_rec));
  c->pool = make_sub_pool(cmd->server->pool);
  pr_pool_tag(c->pool, cmd->argv[0]);
  c->argc = cmd->argc + 2;
  c->argv = pcalloc(c->pool, sizeof(void *) * (c->argc + 2));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  c->argv[1] = NULL;

  if (pr_module_exists("mod_ifsession.c")) {
    /* These are needed in case this directive is used with mod_ifsession
     * configuration.
     */
    c->flags |= CF_MULTI;
  }

  /* Store the executable path. */
  c->argv[EXEC_IDX_LOGFMTS+1] = pstrdup(c->pool, path);

  exec_parse_cmd_args(c, cmd, path_idx+1);

  eed = pcalloc(c->pool, sizeof(struct exec_event_data));
  eed->flags = flags;
  eed->event = pstrdup(c->pool, event_name);
  eed->c = c;

  if (strcasecmp(eed->event, "MaxConnectionRate") == 0) {
    pr_event_register(&exec_module, "core.max-connection-rate", exec_any_ev,
      eed);

  } else if (strcasecmp(eed->event, "MaxInstances") == 0) {
     pr_event_register(&exec_module, "core.max-instances", exec_any_ev, eed);

  } else {
    /* Assume that the sysadmin knows the name of the event to use. */
    pr_event_register(&exec_module, eed->event, exec_any_ev, eed);
  }

  return PR_HANDLED(cmd);
}

/* usage: ExecOnExit path [args] */
MODRET set_execonexit(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned int path_idx = 1;
  char *path;

  if (cmd->argc-1 < 1) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  path = cmd->argv[path_idx];
  if (*path != '/') {
    CONF_ERROR(cmd, "path to program must be a full path");
  }

  c = add_config_param(cmd->argv[0], 0);
  c->argc = cmd->argc + 2;
  c->argv = pcalloc(c->pool, sizeof(void *) * (c->argc + 2));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = exec_nexecs++;

  if (pr_module_exists("mod_ifsession.c")) {
    /* These are needed in case this directive is used with mod_ifsession
     * configuration.
     */
    c->flags |= CF_MULTI;
  }

  /* Store the executable path. */
  c->argv[EXEC_IDX_LOGFMTS+1] = pstrdup(c->pool, path);

  exec_parse_cmd_args(c, cmd, path_idx+1);
  return PR_HANDLED(cmd);
}

/* usage: ExecOnRestart path [args] */
MODRET set_execonrestart(cmd_rec *cmd) {
  config_rec *c = NULL;
  unsigned int path_idx = 1;
  char *path;

  if (cmd->argc-1 < 1) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  path = cmd->argv[path_idx];
  if (*path != '/') {
    CONF_ERROR(cmd, "path to program must be a full path");
  }

  c = add_config_param(cmd->argv[0], 0);
  c->argc = cmd->argc + 2;
  c->argv = pcalloc(c->pool, sizeof(void *) * (c->argc + 2));

  c->argv[0] = pcalloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = exec_nexecs++;

  if (pr_module_exists("mod_ifsession.c")) {
    /* These are needed in case this directive is used with mod_ifsession
     * configuration.
     */
    c->flags |= CF_MULTI;
  }

  /* Store the executable path. */
  c->argv[EXEC_IDX_LOGFMTS+1] = pstrdup(c->pool, path);

  exec_parse_cmd_args(c, cmd, path_idx+1);
  return PR_HANDLED(cmd);
}

/* usage: ExecOptions opt1 opt2 ... optN */
MODRET set_execoptions(cmd_rec *cmd) {
  config_rec *c = NULL;
  register unsigned int i;
  unsigned int opts = 0U;

  if (cmd->argc-1 == 0) {
    CONF_ERROR(cmd, "wrong number of parameters");
  }

  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  c = add_config_param(cmd->argv[0], 1, NULL);

  for (i = 1; i < cmd->argc; i++) {
    if (strcmp(cmd->argv[i], "logStdout") == 0) {
      opts |= EXEC_OPT_LOG_STDOUT;

    } else if (strcmp(cmd->argv[i], "logStderr") == 0) {
      opts |= EXEC_OPT_LOG_STDERR;

    } else if (strcmp(cmd->argv[i], "sendStdout") == 0) {
      opts |= EXEC_OPT_SEND_STDOUT;

    } else if (strcmp(cmd->argv[i], "useStdin") == 0) {
      opts |= EXEC_OPT_USE_STDIN;

    } else {
      CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, ": unknown ExecOption: '",
        (char *) cmd->argv[i], "'", NULL));
    }
  }

  c->argv[0] = palloc(c->pool, sizeof(unsigned int));
  *((unsigned int *) c->argv[0]) = opts;

  if (pr_module_exists("mod_ifsession.c")) {
    /* These are needed in case this directive is used with mod_ifsession
     * configuration.
     */
    c->flags |= CF_MULTI;
  }

  return PR_HANDLED(cmd);
}

/* usage: ExecTimeout <seconds> */
MODRET set_exectimeout(cmd_rec *cmd) {
  int timeout = -1;
  config_rec *c = NULL;

  CHECK_ARGS(cmd, 1);
  CHECK_CONF(cmd, CONF_ROOT|CONF_VIRTUAL|CONF_GLOBAL);

  if (pr_str_get_duration(cmd->argv[1], &timeout) < 0) {
    CONF_ERROR(cmd, pstrcat(cmd->tmp_pool, "error parsing timeout value '",
      cmd->argv[1], "': ", strerror(errno), NULL));
  }

  c = add_config_param(cmd->argv[0], 1, NULL);
  c->argv[0] = pcalloc(c->pool, sizeof(int));
  *((int *) c->argv[0]) = timeout;

  return PR_HANDLED(cmd);
}

/* Event handlers
 */

static void exec_any_ev(const void *event_data, void *user_data) {
  struct exec_event_data *eed = user_data;
  int res;
  unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
  const char *path;

  if (exec_engine == FALSE) {
    return;
  }

  path = eed->c->argv[path_idx];
  res = exec_ssystem(NULL, eed->c, eed->flags);
  if (res != 0) {
    exec_log("ExecOnEvent '%s' for %s failed: %s", eed->event,
      path, strerror(res));

  } else {
    exec_log("ExecOnEvent '%s' for %s succeeded", eed->event, path);
  }
}

#if defined(PR_SHARED_MODULE)
static void exec_mod_unload_ev(const void *event_data, void *user_data) {
  if (strcmp("mod_exec.c", (const char *) event_data) != 0) {
    return;
  }

  pr_event_unregister(&exec_module, NULL, NULL);

  if (exec_pool != NULL) {
    destroy_pool(exec_pool);
    exec_pool = NULL;
  }

  (void) close(exec_logfd);
  exec_logfd = -1;
}
#endif /* PR_SHARED_MODULE */

static void exec_postparse_ev(const void *event_data, void *user_data) {
  exec_openlog();
}

static void exec_restart_ev(const void *event_data, void *user_data) {
  if (exec_pool != NULL) {
    destroy_pool(exec_pool);
    exec_pool = NULL;
  }

  if (exec_engine == TRUE) {
    config_rec *c = NULL;
    cmd_rec *cmd = NULL;
    pool *tmp_pool;

    exec_pool = make_sub_pool(permanent_pool);
    pr_pool_tag(exec_pool, MOD_EXEC_VERSION);

    /* Make sure the User/Group IDs are set, so the the PRIVS_REVOKE call
     * later succeeds properly.
     */
    {
      uid_t *uid = (uid_t *) get_param_ptr(main_server->conf, "UserID", FALSE);
      gid_t *gid = (gid_t *) get_param_ptr(main_server->conf, "GroupID", FALSE);

      session.uid = uid ? *uid : geteuid();
      session.gid = gid ? *gid : getegid();
    }

    tmp_pool = make_sub_pool(exec_pool);
    cmd = pr_cmd_alloc(tmp_pool, 1, pstrdup(tmp_pool, "RESTART"));

    c = find_config(main_server->conf, CONF_PARAM, "ExecOnRestart", FALSE);
    while (c != NULL) {
      int res;
      unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
      const char *path;

      pr_signals_handle();

      path = c->argv[path_idx];
      res = exec_ssystem(cmd, c, EXEC_FL_CLEAR_GROUPS|EXEC_FL_NO_SEND);
      if (res != 0) {
        exec_log("ExecOnRestart '%s' failed: %s", path, strerror(res));

      } else {
        exec_log("ExecOnRestart '%s' succeeded", path);
      }

      c = find_config_next(c, c->next, CONF_PARAM, "ExecOnRestart", FALSE);
    }

    destroy_pool(tmp_pool);
  }

  pr_event_unregister(&exec_module, "core.max-connection-rate", NULL);
  pr_event_unregister(&exec_module, "core.max-instances", NULL);

  /* Bounce the log file descriptor. */
  exec_closelog();
  exec_openlog();
}

static void exec_sess_reinit_ev(const void *event_data, void *user_data) {
  int res;

  /* A HOST command changed the main_server pointer, reinitialize ourselves. */

  pr_event_unregister(&exec_module, "core.session-reinit", exec_sess_reinit_ev);

  exec_engine = FALSE;
  exec_opts = 0U;
  exec_timeout = 0;

  (void) close(exec_logfd);
  exec_logfd = -1;
  exec_logname = NULL;

  res = exec_sess_init();
  if (res < 0) {
    pr_session_disconnect(&exec_module,
      PR_SESS_DISCONNECT_SESSION_INIT_FAILED, NULL);
  }
}

/* Initialization routines
 */

static int exec_sess_init(void) {
  int *use_exec = NULL;
  config_rec *c = NULL;
  const char *proto;
  pool *tmp_pool = NULL;
  cmd_rec *cmd = NULL;

  pr_event_register(&exec_module, "core.session-reinit", exec_sess_reinit_ev,
    NULL);

  use_exec = get_param_ptr(main_server->conf, "ExecEngine", FALSE);
  if (use_exec != NULL &&
      *use_exec == TRUE) {
    exec_engine = TRUE;

  } else {
    exec_engine = FALSE;
    return 0;
  }

  c = find_config(main_server->conf, CONF_PARAM, "ExecOptions", FALSE);
  while (c != NULL) {
    unsigned long opts;

    pr_signals_handle();

    opts = *((unsigned int *) c->argv[0]);
    exec_opts |= opts;

    c = find_config_next(c, c->next, CONF_PARAM, "ExecOptions", FALSE);
  }

  /* If we are handling an SSH2 session, then disable the sendStdout
   * ExecOption, if present.
   *
   * Attempting to send the stdout of commands to connecting SSH2 clients
   * can confuse them and lead to connection problems.
   */
  proto = pr_session_get_protocol(0);
  if (strcmp(proto, "ssh2") == 0) {
    exec_opts &= ~EXEC_OPT_SEND_STDOUT;
  }

  c = find_config(main_server->conf, CONF_PARAM, "ExecTimeout", FALSE);
  if (c != NULL) {
    exec_timeout = *((int *) c->argv[0]);
  }

  exec_closelog();
  exec_openlog();

  /* Make sure the User/Group IDs are set, so the the PRIVS_REVOKE call
   * later succeeds properly.
   */
  {
    uid_t *uid = (uid_t *) get_param_ptr(main_server->conf, "UserID", FALSE);
    gid_t *gid = (gid_t *) get_param_ptr(main_server->conf, "GroupID", FALSE);

    session.uid = uid ? *uid : geteuid();
    session.gid = gid ? *gid : getegid();
  }

  /* Create fake "CONNECT" command for any ExecOnConnect directives. */
  tmp_pool = make_sub_pool(exec_pool);
  pr_pool_tag(tmp_pool, "exec sess init pool");

  cmd = pr_cmd_alloc(tmp_pool, 1, pstrdup(tmp_pool, "CONNECT"));
  cmd->cmd_class |= CL_CONNECT;

  c = find_config(main_server->conf, CONF_PARAM, "ExecOnConnect", FALSE);
  while (c != NULL) {
    int res;
    unsigned int path_idx = EXEC_IDX_LOGFMTS+1;
    const char *path;

    pr_signals_handle();

    path = c->argv[path_idx];
    res = exec_ssystem(cmd, c, EXEC_FL_CLEAR_GROUPS|EXEC_FL_USE_SEND);
    if (res != 0) {
      exec_log("ExecOnConnect '%s' failed: %s", path, strerror(res));

    } else {
      exec_log("ExecOnConnect '%s' succeeded", path);
    }

    c = find_config_next(c, c->next, CONF_PARAM, "ExecOnConnect", FALSE);
  }

  destroy_pool(tmp_pool);
  return 0;
}

static int exec_init(void) {
  exec_pool = make_sub_pool(permanent_pool);
  pr_pool_tag(exec_pool, MOD_EXEC_VERSION);

  /* Register event handlers. */
#if defined(PR_SHARED_MODULE)
  pr_event_register(&exec_module, "core.module-unload", exec_mod_unload_ev,
    NULL);
#endif /* PR_SHARED_MODULE */
  pr_event_register(&exec_module, "core.postparse", exec_postparse_ev, NULL);
  pr_event_register(&exec_module, "core.restart", exec_restart_ev, NULL);

  return 0;
}

/* Module API tables
 */

static conftable exec_conftab[] = {
  { "ExecBeforeCommand",set_execbeforecommand,	NULL },
  { "ExecEnable",	set_execenable,		NULL },
  { "ExecEngine",	set_execengine,		NULL },
  { "ExecEnviron",	set_execenviron,	NULL },
  { "ExecLog",		set_execlog,		NULL },
  { "ExecOnCommand",	set_execoncommand,	NULL },
  { "ExecOnConnect",	set_execonconnect,	NULL },
  { "ExecOnError",	set_execonerror,	NULL },
  { "ExecOnEvent",	set_execonevent,	NULL },
  { "ExecOnExit",	set_execonexit,		NULL },
  { "ExecOnRestart",	set_execonrestart,	NULL },
  { "ExecOptions",	set_execoptions,	NULL },
  { "ExecTimeout",	set_exectimeout,	NULL },
  { NULL }
};

static cmdtable exec_cmdtab[] = {
  { LOG_CMD,		"EXIT",	G_NONE, exec_log_exit,		FALSE,	FALSE },
  { PRE_CMD,		C_ANY,	G_NONE,	exec_pre_cmd,		FALSE,	FALSE },
  { POST_CMD,		C_ANY,	G_NONE, exec_post_cmd,		FALSE,	FALSE },
  { POST_CMD_ERR,	C_ANY,	G_NONE,	exec_post_cmd_err,	FALSE,	FALSE },
  { 0,	NULL }
};

module exec_module = {

  /* Always NULL */
  NULL, NULL,

  /* Module API version */
  0x20,

  /* Module name */
  "exec",

  /* Configuration handler table */
  exec_conftab,

  /* Command handler table */
  exec_cmdtab,

  /* Authentication handler table */
  NULL,

  /* Module initialization */
  exec_init,

  /* Session initialization */
  exec_sess_init,

  /* Module version */
  MOD_EXEC_VERSION
};
