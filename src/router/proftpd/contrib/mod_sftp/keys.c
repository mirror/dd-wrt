/*
 * ProFTPD - mod_sftp key mgmt (keys)
 * Copyright (c) 2008-2010 TJ Saunders
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 *
 * $Id: keys.c,v 1.10 2010/02/10 23:29:22 castaglia Exp $
 */

#include "mod_sftp.h"

#include "msg.h"
#include "packet.h"
#include "crypto.h"
#include "keys.h"
#include "interop.h"

extern xaset_t *server_list;

#define SFTP_DEFAULT_HOSTKEY_SZ		4096
#define SFTP_MAX_SIG_SZ			4096

static EVP_PKEY *sftp_dsa_hostkey = NULL;
static EVP_PKEY *sftp_rsa_hostkey = NULL;

static const char *passphrase_provider = NULL;

struct sftp_pkey {
  struct sftp_pkey *next;
  size_t pkeysz;

  char *host_pkey;
  void *host_pkey_ptr;
  server_rec *server;
};

#define SFTP_PASSPHRASE_TIMEOUT		10

static struct sftp_pkey *sftp_pkey_list = NULL;
static unsigned int sftp_npkeys = 0;
static struct sftp_pkey *server_pkey = NULL;

struct sftp_pkey_data {
  server_rec *s;
  const char *path;
  char *buf;
  size_t buflen, bufsz;
  const char *prompt;
};

static const char *trace_channel = "ssh2";

static void prepare_provider_fds(int stdout_fd, int stderr_fd) {
  long nfiles = 0;
  register unsigned int i = 0;
  struct rlimit rlim;

  if (stdout_fd != STDOUT_FILENO) {
    if (dup2(stdout_fd, STDOUT_FILENO) < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION
        ": error duping fd %d to stdout: %s", stdout_fd, strerror(errno));
    }

    (void) close(stdout_fd);
  }

  if (stderr_fd != STDERR_FILENO) {
    if (dup2(stderr_fd, STDERR_FILENO) < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION
        ": error duping fd %d to stderr: %s", stderr_fd, strerror(errno));
    }

    (void) close(stderr_fd);
  }

  /* Make sure not to pass on open file descriptors. For stdout and stderr,
   * we dup some pipes, so that we can capture what the command may write
   * to stdout or stderr.  The stderr output will be logged to the SFTPLog.
   *
   * First, use getrlimit() to obtain the maximum number of open files
   * for this process -- then close that number.
   */
#if defined(RLIMIT_NOFILE) || defined(RLIMIT_OFILE)
# if defined(RLIMIT_NOFILE)
  if (getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
# elif defined(RLIMIT_OFILE)
  if (getrlimit(RLIMIT_OFILE, &rlim) < 0) {
# endif
    pr_log_debug(DEBUG0, MOD_SFTP_VERSION ": getrlimit error: %s",
      strerror(errno));

    /* Pick some arbitrary high number. */
    nfiles = 255;

  } else {
    nfiles = rlim.rlim_max;
  }

#else /* no RLIMIT_NOFILE or RLIMIT_OFILE */
   nfiles = 255;
#endif

  /* Appears that on some platforms (e.g. Solaris, Mac OSX), having too
   * high of an fd value can lead to undesirable behavior for some reason.
   * Need to track down why; the behavior I saw was the inability of
   * select() to work property on the stdout/stderr fds attached to the
   * exec'd script.
   */
  if (nfiles > 255) {
    nfiles = 255;
  }

  if (nfiles < 0) {
    /* Yes, using a long for the nfiles variable is not quite kosher; it should
     * be an unsigned type, otherwise a large limit (say, RLIMIT_INFINITY)
     * might overflow the data type.  In that case, though, we want to know
     * about it -- and using a signed type, we will know if the overflowed
     * value is a negative number.  Chances are we do NOT want to be closing
     * fds whose value is as high as they can possibly get; that's too many
     * fds to iterate over.  Long story short, using a long int is just fine.
     */
    nfiles = 255;
  }
 
  /* Close the "non-standard" file descriptors. */
  for (i = 3; i < nfiles; i++) {
    pr_signals_handle();
    (void) close(i);
  }

  return;
}

static void prepare_provider_pipes(int *stdout_pipe, int *stderr_pipe) {
  if (pipe(stdout_pipe) < 0) {
    pr_log_debug(DEBUG0, MOD_SFTP_VERSION ": error opening stdout pipe: %s",
      strerror(errno));
    stdout_pipe[0] = -1;
    stdout_pipe[1] = STDOUT_FILENO;

  } else {
    if (fcntl(stdout_pipe[0], F_SETFD, FD_CLOEXEC) < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION
        ": error setting close-on-exec flag on stdout pipe read fd: %s",
        strerror(errno));
    }

    if (fcntl(stdout_pipe[1], F_SETFD, 0) < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION
        ": error setting close-on-exec flag on stdout pipe write fd: %s",
        strerror(errno));
    }
  }

  if (pipe(stderr_pipe) < 0) {
    pr_log_debug(DEBUG0, MOD_SFTP_VERSION ": error opening stderr pipe: %s",
      strerror(errno));
    stderr_pipe[0] = -1;
    stderr_pipe[1] = STDERR_FILENO;

  } else {
    if (fcntl(stderr_pipe[0], F_SETFD, FD_CLOEXEC) < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION
        ": error setting close-on-exec flag on stderr pipe read fd: %s",
        strerror(errno));
    }

    if (fcntl(stderr_pipe[1], F_SETFD, 0) < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION
        ": error setting close-on-exec flag on stderr pipe write fd: %s",
        strerror(errno));
    }
  }
}

static int exec_passphrase_provider(server_rec *s, char *buf, int buflen,
    const char *path) {
  pid_t pid;
  int status;
  int stdout_pipe[2], stderr_pipe[2];

  struct sigaction sa_ignore, sa_intr, sa_quit;
  sigset_t set_chldmask, set_save;

  /* Prepare signal dispositions. */
  sa_ignore.sa_handler = SIG_IGN;
  sigemptyset(&sa_ignore.sa_mask);
  sa_ignore.sa_flags = 0;

  if (sigaction(SIGINT, &sa_ignore, &sa_intr) < 0)
    return -1;

  if (sigaction(SIGQUIT, &sa_ignore, &sa_quit) < 0)
    return -1;

  sigemptyset(&set_chldmask);
  sigaddset(&set_chldmask, SIGCHLD);

  if (sigprocmask(SIG_BLOCK, &set_chldmask, &set_save) < 0)
    return -1;

  prepare_provider_pipes(stdout_pipe, stderr_pipe);

  pid = fork();
  if (pid < 0) {
    pr_log_pri(PR_LOG_ERR, MOD_SFTP_VERSION ": error: unable to fork: %s",
      strerror(errno));
    status = -1;

  } else if (pid == 0) {
    char nbuf[32];
    pool *tmp_pool;
    char *stdin_argv[4];

    /* Child process */

    /* Note: there is no need to clean up this temporary pool, as we've
     * forked.  If the exec call succeeds, this child process will exit
     * normally, and its process space recovered by the OS.  If the exec
     * call fails, we still exit, and the process space is recovered by
     * the OS.  Either way, the memory will be cleaned up without need for
     * us to do it explicitly (unless one wanted to be pedantic about it,
     * of course).
     */
    tmp_pool = make_sub_pool(s->pool);

    /* Restore previous signal actions. */
    sigaction(SIGINT, &sa_intr, NULL);
    sigaction(SIGQUIT, &sa_quit, NULL);
    sigprocmask(SIG_SETMASK, &set_save, NULL);

    stdin_argv[0] = pstrdup(tmp_pool, passphrase_provider);

    memset(nbuf, '\0', sizeof(nbuf));
    snprintf(nbuf, sizeof(nbuf)-1, "%u", (unsigned int) s->ServerPort);
    nbuf[sizeof(nbuf)-1] = '\0';
    stdin_argv[1] = pstrcat(tmp_pool, s->ServerName, ":", nbuf, NULL);
    stdin_argv[2] = pstrdup(tmp_pool, path);
    stdin_argv[3] = NULL;

    PRIVS_ROOT

    pr_log_debug(DEBUG6, MOD_SFTP_VERSION
      ": executing '%s' with uid %lu (euid %lu), gid %lu (egid %lu)",
      passphrase_provider,
      (unsigned long) getuid(), (unsigned long) geteuid(),
      (unsigned long) getgid(), (unsigned long) getegid());

    /* Prepare the file descriptors that the process will inherit. */
    prepare_provider_fds(stdout_pipe[1], stderr_pipe[1]);

    errno = 0;
    execv(passphrase_provider, stdin_argv);

    /* Since all previous file descriptors (including those for log files)
     * have been closed, and root privs have been revoked, there's little
     * chance of directing a message of execv() failure to proftpd's log
     * files.  execv() only returns if there's an error; the only way we
     * can signal this to the waiting parent process is to exit with a
     * non-zero value (the value of errno will do nicely).
     */

    exit(errno);

  } else {
    int res;
    int maxfd = -1, fds, send_sigterm = 1;
    fd_set readfds;
    time_t start_time = time(NULL);
    struct timeval tv;

    /* Parent process */

    close(stdout_pipe[1]);
    stdout_pipe[1] = -1;

    close(stderr_pipe[1]);
    stderr_pipe[1] = -1;

    if (stdout_pipe[0] > maxfd) {
      maxfd = stdout_pipe[0];
    }

    if (stderr_pipe[0] > maxfd) {
      maxfd = stderr_pipe[0];
    }

    res = waitpid(pid, &status, WNOHANG);
    while (res <= 0) {
      if (res < 0) {
        if (errno != EINTR) {
          pr_log_debug(DEBUG2, MOD_SFTP_VERSION
            ": passphrase provider error: unable to wait for pid %u: %s",
            (unsigned int) pid, strerror(errno));
          status = -1;
          break;

        } else {
          pr_signals_handle();
          continue;
        }
      }

      /* Check the time elapsed since we started. */
      if ((time(NULL) - start_time) > SFTP_PASSPHRASE_TIMEOUT) {

        /* Send TERM, the first time, to be polite. */
        if (send_sigterm) {
          send_sigterm = 0;
          pr_log_debug(DEBUG6, MOD_SFTP_VERSION
            ": '%s' has exceeded the timeout (%lu seconds), sending "
            "SIGTERM (signal %d)", passphrase_provider,
            (unsigned long) SFTP_PASSPHRASE_TIMEOUT, SIGTERM);
          kill(pid, SIGTERM);

        } else {
          /* The child is still around?  Terminate with extreme prejudice. */
          pr_log_debug(DEBUG6, MOD_SFTP_VERSION
            ": '%s' has exceeded the timeout (%lu seconds), sending "
            "SIGKILL (signal %d)", passphrase_provider,
            (unsigned long) SFTP_PASSPHRASE_TIMEOUT, SIGKILL);
          kill(pid, SIGKILL);
        }
      }

      /* Select on the pipe read fds, to see if the child has anything
       * to tell us.
       */
      FD_ZERO(&readfds);

      FD_SET(stdout_pipe[0], &readfds);
      FD_SET(stderr_pipe[0], &readfds);

      /* Note: this delay should be configurable somehow. */
      tv.tv_sec = 2L;
      tv.tv_usec = 0L;

      fds = select(maxfd + 1, &readfds, NULL, NULL, &tv);

      if (fds == -1 &&
          errno == EINTR) {
        pr_signals_handle();
      }

      if (fds > 0) {
        /* The child sent us something.  How thoughtful. */

        if (FD_ISSET(stdout_pipe[0], &readfds)) {
          res = read(stdout_pipe[0], buf, buflen);
          if (res > 0) {
              while (res &&
                     (buf[res-1] == '\r' ||
                      buf[res-1] == '\n')) {
                res--;
              }
              buf[res] = '\0';

          } else if (res < 0) {
            pr_log_debug(DEBUG2, MOD_SFTP_VERSION
              ": error reading stdout from '%s': %s",
              passphrase_provider, strerror(errno));
          }
        }

        if (FD_ISSET(stderr_pipe[0], &readfds)) {
          int stderrlen;
          char stderrbuf[PIPE_BUF];

          memset(stderrbuf, '\0', sizeof(stderrbuf));
          stderrlen = read(stderr_pipe[0], stderrbuf, sizeof(stderrbuf)-1);
          if (stderrlen > 0) {
            while (stderrlen &&
                   (stderrbuf[stderrlen-1] == '\r' ||
                    stderrbuf[stderrlen-1] == '\n')) {
              stderrlen--;
            }
            stderrbuf[stderrlen] = '\0';

            pr_log_debug(DEBUG5, MOD_SFTP_VERSION
              ": stderr from '%s': %s", passphrase_provider, stderrbuf);

          } else if (res < 0) {
            pr_log_debug(DEBUG2, MOD_SFTP_VERSION
              ": error reading stderr from '%s': %s",
              passphrase_provider, strerror(errno));
          }
        }
      }

      res = waitpid(pid, &status, WNOHANG);
    }
  }

  /* Restore the previous signal actions. */
  if (sigaction(SIGINT, &sa_intr, NULL) < 0)
    return -1;

  if (sigaction(SIGQUIT, &sa_quit, NULL) < 0)
    return -1;

  if (sigprocmask(SIG_SETMASK, &set_save, NULL) < 0)
    return -1;

  if (WIFSIGNALED(status)) {
    pr_log_debug(DEBUG2, MOD_SFTP_VERSION ": '%s' died from signal %d",
      passphrase_provider, WTERMSIG(status));
    errno = EPERM;
    return -1;
  }

  return 0;
}

/* Return the size of a page on this architecture. */
static size_t get_pagesz(void) {
  long pagesz;

#if defined(_SC_PAGESIZE)
  pagesz = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
  pagesz = sysconf(_SC_PAGE_SIZE);
#else
  /* Default to using OpenSSL's defined buffer size for PEM files. */
  pagesz = PEM_BUFSIZE;
#endif /* !_SC_PAGESIZE and !_SC_PAGE_SIZE */

  return pagesz;
}

/* Return a page-aligned pointer to memory of at least the given size. */
static char *get_page(size_t sz, void **ptr) {
  void *d;
  long pagesz = get_pagesz(), p;

  d = malloc(sz + (pagesz-1));
  if (d == NULL) {
    pr_log_pri(PR_LOG_ERR, "Out of memory!");
    exit(1);
  }

  *ptr = d;

  p = ((long) d + (pagesz-1)) &~ (pagesz-1);

  return ((char *) p);
}

static int get_passphrase_cb(char *buf, int buflen, int rwflag, void *d) {
  static int need_banner = TRUE;
  struct sftp_pkey_data *pdata = d;

  if (passphrase_provider == NULL) {
    register unsigned int attempt;
    size_t pwlen = 0;

    pr_log_debug(DEBUG0, MOD_SFTP_VERSION ": requesting passphrase from admin");

    if (need_banner) {
      fprintf(stderr, "\nPlease provide passphrase for the encrypted host key:\n");
      need_banner = FALSE;
    }

    /* You get three attempts at entering the passphrase correctly. */
    for (attempt = 0; attempt < 3; attempt++) {
      int res;

      /* Always handle signals in a loop. */
      pr_signals_handle();

      res = EVP_read_pw_string(buf, buflen, pdata->prompt, TRUE);

      /* A return value of zero from EVP_read_pw_string() means success; -1
       * means a system error occurred, and 1 means user interaction problems.
       */
      if (res != 0) {
         fprintf(stderr, "\nPassphrases do not match.  Please try again.\n");
         continue;
      }

      pwlen = strlen(buf);
      if (pwlen < 1) {
        fprintf(stderr, "Error: passphrase must be at least one character\n");

      } else {
        sstrncpy(pdata->buf, buf, pdata->bufsz);
        pdata->buflen = pwlen;

        return pwlen;
      }
    }

  } else {
    pr_log_debug(DEBUG0, MOD_SFTP_VERSION ": requesting passphrase from '%s'",
      passphrase_provider);

    if (exec_passphrase_provider(pdata->s, buf, buflen, pdata->path) < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION
        ": error obtaining passphrase from '%s': %s",
        passphrase_provider, strerror(errno));

    } else {
      size_t pwlen = strlen(buf);

      sstrncpy(pdata->buf, buf, pdata->bufsz);
      pdata->buflen = pwlen;

      return pwlen;
    }
  }

#if OPENSSL_VERSION_NUMBER < 0x00908001
  PEMerr(PEM_F_DEF_CALLBACK, PEM_R_PROBLEMS_GETTING_PASSWORD);
#else
  PEMerr(PEM_F_PEM_DEF_CALLBACK, PEM_R_PROBLEMS_GETTING_PASSWORD);
#endif

  pr_memscrub(buf, buflen);
  return -1;
}

static int get_passphrase(struct sftp_pkey *k, const char *path) {
  char prompt[256];
  FILE *fp;
  EVP_PKEY *pkey = NULL;
  int fd, prompt_fd = -1, res;
  struct sftp_pkey_data pdata;
  register unsigned int attempt;

  memset(prompt, '\0', sizeof(prompt));
  snprintf(prompt, sizeof(prompt)-1, "Host key for the %s#%d (%s) server: ",
    pr_netaddr_get_ipstr(k->server->addr), k->server->ServerPort,
    k->server->ServerName);
  prompt[sizeof(prompt)-1] = '\0';

  PRIVS_ROOT
  fd = open(path, O_RDONLY);
  PRIVS_RELINQUISH

  if (fd < 0) {
    SYSerr(SYS_F_FOPEN, errno);
    return -1;
  }

  /* Make sure the fd isn't one of the big three. */
  res = pr_fs_get_usable_fd(fd);
  if (res >= 0) {
    fd = res;
  }

  fp = fdopen(fd, "r");
  if (fp == NULL) {
    SYSerr(SYS_F_FOPEN, errno);
    return -1;
  }

  k->host_pkey = get_page(PEM_BUFSIZE, &k->host_pkey_ptr);
  if (k->host_pkey == NULL) {
    pr_log_pri(PR_LOG_ERR, "Out of memory!");
    exit(1);
  }

  pdata.s = k->server;
  pdata.buf = k->host_pkey;
  pdata.buflen = 0;
  pdata.bufsz = k->pkeysz;
  pdata.path = path;
  pdata.prompt = prompt;

  /* Reconnect stderr to the term because proftpd connects stderr, earlier,
   * to the general stderr logfile.
   */
  prompt_fd = open("/dev/null", O_WRONLY);
  if (prompt_fd == -1) {
    /* This is an arbitrary, meaningless placeholder number. */
    prompt_fd = 76;
  }

  dup2(STDERR_FILENO, prompt_fd);
  dup2(STDOUT_FILENO, STDERR_FILENO);

  /* The user gets three tries to enter the correct passphrase. */
  for (attempt = 0; attempt < 3; attempt++) {

    /* Always handle signals in a loop. */
    pr_signals_handle();

    pkey = PEM_read_PrivateKey(fp, NULL, get_passphrase_cb, &pdata);
    if (pkey)
      break;

    fseek(fp, 0, SEEK_SET);
    ERR_clear_error();
    fprintf(stderr, "\nWrong passphrase for this key.  Please try again.\n");
  }

  fclose(fp);

  /* Restore the normal stderr logging. */
  dup2(prompt_fd, STDERR_FILENO);
  close(prompt_fd);

  if (pkey == NULL)
    return -1;

  if (pdata.buflen > 0) {
#if OPENSSL_VERSION_NUMBER >= 0x000905000L
    /* Use the obtained passphrase as additional entropy, ostensibly
     * unknown to attackers who may be watching the network, for
     * OpenSSL's PRNG.
     *
     * Human language gives about 2-3 bits of entropy per byte (RFC1750).
     */
    RAND_add(pdata.buf, pdata.buflen, pdata.buflen * 0.25);
#endif

#ifdef HAVE_MLOCK
    PRIVS_ROOT
    if (mlock(k->host_pkey, k->pkeysz) < 0) {
      pr_log_debug(DEBUG1, MOD_SFTP_VERSION
        ": error locking passphrase into memory: %s", strerror(errno));

    } else {
      pr_log_debug(DEBUG1, MOD_SFTP_VERSION ": passphrase locked into memory");
    }
    PRIVS_RELINQUISH
#endif
  }

  EVP_PKEY_free(pkey);
  return 0;
}

static struct sftp_pkey *lookup_pkey(void) {
  struct sftp_pkey *k, *pkey = NULL;

  for (k = sftp_pkey_list; k; k = k->next) {

    /* If this pkey matches the current server_rec, mark it and move on. */
    if (k->server == main_server) {

#ifdef HAVE_MLOCK
      /* mlock() the passphrase memory areas again; page locks are not
       * inherited across forks.
       */
      PRIVS_ROOT
      if (k->host_pkey) {
        if (mlock(k->host_pkey, k->pkeysz) < 0) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error locking passphrase into memory: %s", strerror(errno));
        }
      }
      PRIVS_RELINQUISH
#endif /* HAVE_MLOCK */

      pkey = k;
      continue;
    }

    /* Otherwise, scrub the passphrase's memory areas. */
    if (k->host_pkey) {
      pr_memscrub(k->host_pkey, k->pkeysz);
      free(k->host_pkey_ptr);
      k->host_pkey = k->host_pkey_ptr = NULL;
    }
  }

  return pkey;
}

static void scrub_pkeys(void) {
  struct sftp_pkey *k;
 
  /* Scrub and free all passphrases in memory. */
  if (sftp_pkey_list) {
    pr_log_debug(DEBUG5, MOD_SFTP_VERSION
      ": scrubbing %u %s from memory",
      sftp_npkeys, sftp_npkeys != 1 ? "passphrases" : "passphrase");
 
  } else
    return;
 
  for (k = sftp_pkey_list; k; k = k->next) {
    if (k->host_pkey) {
      pr_memscrub(k->host_pkey, k->pkeysz);
      free(k->host_pkey_ptr);
      k->host_pkey = k->host_pkey_ptr = NULL;
    }
  }

  sftp_pkey_list = NULL;
  sftp_npkeys = 0;
}

static int pkey_cb(char *buf, int buflen, int rwflag, void *d) {
  struct sftp_pkey *k;

  if (d == NULL)
    return 0;

  k = (struct sftp_pkey *) d;

  if (k->host_pkey) {
    strncpy(buf, k->host_pkey, buflen);
    buf[buflen - 1] = '\0';
    return strlen(buf);
  }

  return 0;
}

static int has_req_perms(int fd) {
  struct stat st;

  if (fstat(fd, &st) < 0)
    return -1;

  if (st.st_mode & (S_IRWXG|S_IRWXO)) {
    errno = EACCES;
    return -1;
  }

  return 0;
}

static EVP_PKEY *get_pkey_from_data(pool *p, char *pkey_data,
    uint32_t pkey_datalen) {
  EVP_PKEY *pkey = NULL;
  char *pkey_type;

  pkey_type = sftp_msg_read_string(p, &pkey_data, &pkey_datalen);

  if (strcmp(pkey_type, "ssh-rsa") == 0) {
    RSA *rsa;

    pkey = EVP_PKEY_new();
    if (pkey == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating EVP_PKEY: %s", sftp_crypto_get_errors());
      return NULL;
    }

    rsa = RSA_new();
    if (rsa == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating RSA: %s", sftp_crypto_get_errors());
      EVP_PKEY_free(pkey);
      return NULL;
    }

    rsa->e = sftp_msg_read_mpint(p, &pkey_data, &pkey_datalen);
    rsa->n = sftp_msg_read_mpint(p, &pkey_data, &pkey_datalen);

    if (EVP_PKEY_assign_RSA(pkey, rsa) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error assigning RSA to EVP_PKEY: %s", sftp_crypto_get_errors());
      RSA_free(rsa);
      EVP_PKEY_free(pkey);
      return NULL;
    }

  } else if (strcmp(pkey_type, "ssh-dss") == 0) {
    DSA *dsa;

    pkey = EVP_PKEY_new();
    if (pkey == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating EVP_PKEY: %s", sftp_crypto_get_errors());
      return NULL;
    }

    dsa = DSA_new();
    if (dsa == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating DSA: %s", sftp_crypto_get_errors());
      EVP_PKEY_free(pkey);
      return NULL;
    }

    dsa->p = sftp_msg_read_mpint(p, &pkey_data, &pkey_datalen);
    dsa->q = sftp_msg_read_mpint(p, &pkey_data, &pkey_datalen);
    dsa->g = sftp_msg_read_mpint(p, &pkey_data, &pkey_datalen);
    dsa->pub_key = sftp_msg_read_mpint(p, &pkey_data, &pkey_datalen);

    if (EVP_PKEY_assign_DSA(pkey, dsa) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error assigning RSA to EVP_PKEY: %s", sftp_crypto_get_errors());
      DSA_free(dsa);
      EVP_PKEY_free(pkey);
      return NULL;
    }

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unsupported public key algorithm '%s'", pkey_type);
    return NULL;
  }

  return pkey;
}

static const char *get_key_type_desc(int key_type) {
  const char *key_desc;

  switch (key_type) {
#ifdef EVP_PKEY_NONE
    case EVP_PKEY_NONE:
      key_desc = "undefined";
      break;
#endif

#ifdef EVP_PKEY_RSA
    case EVP_PKEY_RSA:
      key_desc = "RSA";
      break;
#endif

#ifdef EVP_PKEY_DSA
    case EVP_PKEY_DSA:
      key_desc = "DSA";
      break;
#endif

#ifdef EVP_PKEY_DH
    case EVP_PKEY_DH:
      key_desc = "DH";
      break;
#endif

#ifdef EVP_PKEY_EC
    case EVP_PKEY_EC:
      key_desc = "EC";
      break;
#endif

    default:
      key_desc = "unknown";
  }

  return key_desc;
}

/* Compare a "blob" of pubkey data sent by the client for authentication
 * with a file pubkey (from an RFC4716 formatted file).  Returns -1 if
 * there was an error, TRUE if the keys are equals, and FALSE if not.
 */
int sftp_keys_compare_keys(pool *p, char *client_pubkey_data,
    uint32_t client_pubkey_datalen, char *file_pubkey_data,
    uint32_t file_pubkey_datalen) {
  EVP_PKEY *client_pkey, *file_pkey;
  int res = -1;

  if (client_pubkey_data == NULL ||
      file_pubkey_data == NULL) {
    errno = EINVAL;
    return -1;
  }

  client_pkey = get_pkey_from_data(p, client_pubkey_data,
    client_pubkey_datalen);
  if (client_pkey == NULL) {
    return -1;
  }

  file_pkey = get_pkey_from_data(p, file_pubkey_data, file_pubkey_datalen);
  if (file_pkey == NULL) {
    return -1;
  }

  if (EVP_PKEY_type(client_pkey->type) == EVP_PKEY_type(file_pkey->type)) {
    switch (EVP_PKEY_type(client_pkey->type)) {
      case EVP_PKEY_RSA: {
        RSA *client_rsa, *file_rsa;

        client_rsa = EVP_PKEY_get1_RSA(client_pkey);
        file_rsa = EVP_PKEY_get1_RSA(file_pkey);

        if (BN_cmp(client_rsa->e, file_rsa->e) != 0) {
          pr_trace_msg(trace_channel, 17, "%s",
            "RSA key mismatch: client-sent RSA key component 'e' does not "
            "match local RSA key component 'e'");
          res = FALSE;

        } else {
          if (BN_cmp(client_rsa->n, file_rsa->n) != 0) {
            pr_trace_msg(trace_channel, 17, "%s",
              "RSA key mismatch: client-sent RSA key component 'n' does not "
              "match local RSA key component 'n'");
            res = FALSE;

          } else {
            res = TRUE;
          }
        } 

        RSA_free(client_rsa);
        RSA_free(file_rsa);
        break;
      }

      case EVP_PKEY_DSA: {
        DSA *client_dsa, *file_dsa;

        client_dsa = EVP_PKEY_get1_DSA(client_pkey);
        file_dsa = EVP_PKEY_get1_DSA(file_pkey);

        if (BN_cmp(client_dsa->p, file_dsa->p) != 0) {
          pr_trace_msg(trace_channel, 17, "%s",
            "DSA key mismatch: client-sent DSA key parameter 'p' does not "
            "match local DSA key parameter 'p'");
          res = FALSE;

        } else {
          if (BN_cmp(client_dsa->q, file_dsa->q) != 0) {
            pr_trace_msg(trace_channel, 17, "%s",
              "DSA key mismatch: client-sent DSA key parameter 'q' does not "
              "match local DSA key parameter 'q'");
            res = FALSE;

          } else {
            if (BN_cmp(client_dsa->g, file_dsa->g) != 0) {
              pr_trace_msg(trace_channel, 17, "%s",
                "DSA key mismatch: client-sent DSA key parameter 'g' does not "
                "match local DSA key parameter 'g'");
              res = FALSE;

            } else {
              if (BN_cmp(client_dsa->pub_key, file_dsa->pub_key) != 0) {
                pr_trace_msg(trace_channel, 17, "%s",
                  "DSA key mismatch: client-sent DSA key parameter 'pub_key' "
                  "does not match local DSA key parameter 'pub_key'");
                res = FALSE;

              } else {
                res = TRUE;
              }
            }
          }
        }

        DSA_free(client_dsa);
        DSA_free(file_dsa);

        break;
      }
    }

  } else {
    if (pr_trace_get_level(trace_channel) >= 17) {
      const char *client_key_desc, *file_key_desc;

      client_key_desc = get_key_type_desc(EVP_PKEY_type(client_pkey->type));
      file_key_desc = get_key_type_desc(EVP_PKEY_type(file_pkey->type));

      pr_trace_msg(trace_channel, 17, "key mismatch: cannot compare %s key "
        "(client-sent) with %s key (local)", client_key_desc, file_key_desc);
    }

    res = FALSE;
  }

  EVP_PKEY_free(client_pkey);
  EVP_PKEY_free(file_pkey);

  return res;
}

const char *sftp_keys_get_fingerprint(pool *p, char *key_data,
    uint32_t key_datalen, int digest_algo) {
  const EVP_MD *digest;
  EVP_MD_CTX ctx;
  char *fp;
  unsigned char *fp_data;
  unsigned int fp_datalen = 0;
  register unsigned int i;

  switch (digest_algo) {
    case SFTP_KEYS_FP_DIGEST_MD5:
      digest = EVP_md5();
      break;

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unsupported key fingerprint digest algorithm (%d)", digest_algo);
      errno = EACCES;
      return NULL;
  }

  fp_data = palloc(p, EVP_MAX_MD_SIZE);
  EVP_DigestInit(&ctx, digest);
  EVP_DigestUpdate(&ctx, key_data, key_datalen);
  EVP_DigestFinal(&ctx, fp_data, &fp_datalen);

  /* Now encode that digest in fp_data as hex characters. */
  fp = "";

  for (i = 0; i < fp_datalen; i++) {
    char c[4];

    memset(c, '\0', sizeof(c));
    snprintf(c, sizeof(c), "%02x:", fp_data[i]);
    fp = pstrcat(p, fp, &c, NULL);
  }
  fp[strlen(fp)-1] = '\0';
  
  return fp;
}

int sftp_keys_get_hostkey(const char *path) {
  int fd;
  FILE *fp;
  EVP_PKEY *pkey;

  pr_signals_block();
  PRIVS_ROOT

  /* XXX Would we ever want to allow host keys to be read from FIFOs?  If
   * so, we would need to include the O_NONBLOCK flag here.
   */
  fd = open(path, O_RDONLY, 0);
  PRIVS_RELINQUISH
  pr_signals_unblock();

  if (fd < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading '%s': %s", path, strerror(errno));
    return -1;
  }

  if (has_req_perms(fd) < 0) {
    if (errno == EACCES) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "'%s' is accessible by group or world, which is not allowed", path);

    } else {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error checking '%s' perms: %s", path, strerror(errno));
    }

    close(fd);
    return -1;
  }

  /* OpenSSL's APIs prefer stdio file handles. */
  fp = fdopen(fd, "r");
  if (fp == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error opening stdio fp on fd %d: %s", fd, strerror(errno));
    close(fd);
    return -1;
  }

  if (server_pkey == NULL) {
    server_pkey = lookup_pkey();
  }

  if (server_pkey) {
    pkey = PEM_read_PrivateKey(fp, NULL, pkey_cb, (void *) &server_pkey);

  } else {
    /* Assume that the key is not passphrase-protected. */
    pkey = PEM_read_PrivateKey(fp, NULL, NULL, "");
  }

  fclose(fp);

  if (pkey == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading private key from '%s': %s", path,
      sftp_crypto_get_errors());
    return -1;
  }

  switch (pkey->type) {
    case EVP_PKEY_RSA: {
#if OPENSSL_VERSION_NUMBER < 0x0090702fL
      /* In OpenSSL-0.9.7a and later, RSA blinding is turned on by default.
       * Thus if our OpenSSL is older than that, manually enable RSA
       * blinding.
       */
      RSA *rsa;

      rsa = EVP_PKEY_get1_RSA(pkey);
      if (rsa) {
        if (RSA_blinding_on(rsa, NULL) != 1) {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "error enabling RSA blinding for key '%s': %s", path,
            sftp_crypto_get_errors());

        } else {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "RSA blinding enabled for key '%s'", path);
        }

        RSA_free(rsa);
      }
#endif

      if (sftp_rsa_hostkey) {
        /* If we have an existing RSA hostkey, free it up. */
        EVP_PKEY_free(sftp_rsa_hostkey);
        sftp_rsa_hostkey = NULL;
      }

      sftp_rsa_hostkey = pkey;
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "using '%s' as RSA hostkey", path);
      break;
    }

    case EVP_PKEY_DSA: {
      if (sftp_dsa_hostkey) {
        /* If we have an existing DSA hostkey, free it up. */
        EVP_PKEY_free(sftp_dsa_hostkey);
        sftp_dsa_hostkey = NULL;
      }

      sftp_dsa_hostkey = pkey;
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "using '%s' as DSA hostkey", path);
      break;
    }

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unknown private key type (%d), ignoring", pkey->type);
      EVP_PKEY_free(pkey);
      return -1;
  }

  return 0;
}

const char *sftp_keys_get_hostkey_data(pool *p, int key_type,
    size_t *datalen) {
  char *buf = NULL, *ptr = NULL;
  uint32_t buflen = SFTP_DEFAULT_HOSTKEY_SZ;

  switch (key_type) {
    case EVP_PKEY_RSA: {
      RSA *rsa;

      rsa = EVP_PKEY_get1_RSA(sftp_rsa_hostkey);
      if (rsa == NULL) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error using RSA hostkey: %s", sftp_crypto_get_errors());
        return NULL;
      }

      /* XXX Is this buffer large enough?  Too large? */
      ptr = buf = sftp_msg_getbuf(p, buflen);
      sftp_msg_write_string(&buf, &buflen, "ssh-rsa");
      sftp_msg_write_mpint(&buf, &buflen, rsa->e);
      sftp_msg_write_mpint(&buf, &buflen, rsa->n);

      RSA_free(rsa);
      break;
    }

    case EVP_PKEY_DSA: {
      DSA *dsa;

      dsa = EVP_PKEY_get1_DSA(sftp_dsa_hostkey);
      if (dsa == NULL) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error using DSA hostkey: %s", sftp_crypto_get_errors());
        return NULL;
      }

      /* XXX Is this buffer large enough?  Too large? */
      ptr = buf = sftp_msg_getbuf(p, buflen);
      sftp_msg_write_string(&buf, &buflen, "ssh-dss");
      sftp_msg_write_mpint(&buf, &buflen, dsa->p);
      sftp_msg_write_mpint(&buf, &buflen, dsa->q);
      sftp_msg_write_mpint(&buf, &buflen, dsa->g);
      sftp_msg_write_mpint(&buf, &buflen, dsa->pub_key);

      DSA_free(dsa);
      break;
    }

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unknown key type (%d) requested, ignoring", key_type);
      return NULL;
  }

  *datalen = SFTP_DEFAULT_HOSTKEY_SZ - buflen;

  /* If the caller provided a pool, make a copy of the data from the
   * given pool, and return the copy.  Make sure the scrub the original
   * after making the copy.
   */
  if (p) {
    buf = palloc(p, *datalen);
    memcpy(buf, ptr, *datalen);

    pr_memscrub(ptr, *datalen);
    return buf;
  }

  return ptr;
}

int sftp_keys_have_dsa_hostkey(void) {
  if (sftp_dsa_hostkey != NULL) {
    return 0;
  }

  errno = ENOENT;
  return -1;
}

int sftp_keys_have_rsa_hostkey(void) {
  if (sftp_rsa_hostkey != NULL) {
    return 0;
  }

  errno = ENOENT;
  return -1;
}

static const char *rsa_sign_data(pool *p, const unsigned char *data,
    size_t datalen, size_t *siglen) {
  RSA *rsa;
  EVP_MD_CTX ctx;
  const EVP_MD *sha1 = EVP_sha1();
  unsigned char dgst[EVP_MAX_MD_SIZE], *sig_data;
  char *buf, *ptr;
  size_t bufsz;
  uint32_t buflen, dgstlen = 0, sig_datalen = 0, sig_rsalen = 0;
  int res;

  rsa = EVP_PKEY_get1_RSA(sftp_rsa_hostkey);
  if (rsa == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error using RSA hostkey: %s", sftp_crypto_get_errors());
    return NULL;
  }

  EVP_DigestInit(&ctx, sha1);
  EVP_DigestUpdate(&ctx, data, datalen);
  EVP_DigestFinal(&ctx, dgst, &dgstlen);

  sig_rsalen = RSA_size(rsa);
  sig_data = pcalloc(p, sig_rsalen);

  res = RSA_sign(NID_sha1, dgst, dgstlen, sig_data, &sig_datalen, rsa);

  /* Regardless of whether the RSA signing succeeds or fails, we are done
   * with the digest buffer.
   */
  pr_memscrub(dgst, dgstlen);

  if (res != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error signing data using RSA: %s", sftp_crypto_get_errors());
    RSA_free(rsa);
    return NULL;
  }

  /* XXX Is this buffer large enough?  Too large? */
  buflen = bufsz = SFTP_MAX_SIG_SZ;
  ptr = buf = sftp_msg_getbuf(p, bufsz);

  /* Now build up the signature, SSH2-style */
  sftp_msg_write_string(&buf, &buflen, "ssh-rsa");
  sftp_msg_write_data(&buf, &buflen, (char *) sig_data, sig_datalen, TRUE);

  pr_memscrub(sig_data, sig_datalen);
  RSA_free(rsa);

  /* At this point, buflen is the amount remaining in the allocated buffer.
   * So the total length of the signed data is the buffer size, minus those
   * remaining unused bytes.
   */
  *siglen = (bufsz - buflen);
  return ptr;
}

static const char *dsa_sign_data(pool *p, const unsigned char *data,
    size_t datalen, size_t *siglen) {
  DSA *dsa;
  DSA_SIG *sig;
  EVP_MD_CTX ctx;
  const EVP_MD *sha1 = EVP_sha1();
  unsigned char dgst[EVP_MAX_MD_SIZE], sig_data[SFTP_MAX_SIG_SZ];
  char *buf, *ptr;
  size_t bufsz;
  uint32_t buflen, dgstlen = 0, sig_datalen = 0;

  dsa = EVP_PKEY_get1_DSA(sftp_dsa_hostkey);
  if (dsa == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error using DSA hostkey: %s", sftp_crypto_get_errors());
    return NULL;
  }

  EVP_DigestInit(&ctx, sha1);
  EVP_DigestUpdate(&ctx, data, datalen);
  EVP_DigestFinal(&ctx, dgst, &dgstlen);

  sig = DSA_do_sign(dgst, dgstlen, dsa);
  if (sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining DSA signature: %s", sftp_crypto_get_errors());
    pr_memscrub(dgst, dgstlen);
    DSA_free(dsa);
    return NULL;
  }

  /* Got the signature, no need for the digest memory. */
  pr_memscrub(dgst, dgstlen);

  memset(sig_data, 0, sizeof(sig_data));
  sig_datalen += BN_bn2bin(sig->r, sig_data);
  sig_datalen += BN_bn2bin(sig->s, sig_data + BN_num_bytes(sig->r));

  /* XXX Is this buffer large enough?  Too large? */
  buflen = bufsz = SFTP_MAX_SIG_SZ;
  ptr = buf = sftp_msg_getbuf(p, bufsz);

  /* Now build up the signature, SSH2-style */
  sftp_msg_write_string(&buf, &buflen, "ssh-dss");
  sftp_msg_write_data(&buf, &buflen, (char *) sig_data, sig_datalen, TRUE);

  /* Done with the signature. */
  DSA_SIG_free(sig);
  DSA_free(dsa);

  /* At this point, buflen is the amount remaining in the allocated buffer.
   * So the total length of the signed data is the buffer size, minus those
   * remaining unused bytes.
   */
  *siglen = (bufsz - buflen);
  return ptr;
}

const char *sftp_keys_sign_data(pool *p, int key_type,
    const unsigned char *data, size_t datalen, size_t *siglen) {
  const char *res;

  switch (key_type) {
    case EVP_PKEY_RSA:
      res = rsa_sign_data(p, data, datalen, siglen);
      break;

    case EVP_PKEY_DSA:
      res = dsa_sign_data(p, data, datalen, siglen);
      break;

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unknown key type (%d) requested for signing, ignoring", key_type);
      return NULL;
  }

  if (p) {
    char *buf = palloc(p, *siglen);
    memcpy(buf, res, *siglen);

    pr_memscrub((char *) res, *siglen);
    return buf;
  }

  return res;
}

int sftp_keys_verify_pubkey_type(pool *p, char *pubkey_data,
    uint32_t pubkey_len, int pubkey_type) {
  EVP_PKEY *pkey;
  int res;

  if (pubkey_data == NULL) {
    errno = EINVAL;
    return -1;
  }

  pkey = get_pkey_from_data(p, pubkey_data, pubkey_len);
  if (pkey == NULL) {
    return -1;
  }

  res = (EVP_PKEY_type(pkey->type) == pubkey_type);

  EVP_PKEY_free(pkey);
  return res;
}

int sftp_keys_verify_signed_data(pool *p, const char *pubkey_algo,
    char *pubkey_data, uint32_t pubkey_datalen, char *signature,
    uint32_t signaturelen, unsigned char *sig_data, size_t sig_datalen) {
  EVP_PKEY *pkey;
  EVP_MD_CTX ctx;
  unsigned char *sig;
  uint32_t sig_len;
  unsigned char digest[EVP_MAX_MD_SIZE];
  char *sig_type;
  unsigned int digestlen;
  int res;

  if (pubkey_algo == NULL ||
      pubkey_data == NULL ||
      signature == NULL ||
      sig_data == NULL ||
      sig_datalen == 0) {
    errno = EINVAL;
    return -1;
  }

  pkey = get_pkey_from_data(p, pubkey_data, pubkey_datalen);
  if (pkey == NULL) {
    return -1;
  }

  if (strcmp(pubkey_algo, "ssh-dss") == 0) {
    if (sftp_interop_supports_feature(SFTP_SSH2_FEAT_HAVE_PUBKEY_ALGO_IN_DSA_SIG)) {
      sig_type = sftp_msg_read_string(p, &signature, &signaturelen);

    } else {
      /* The client did not prepend the public key algorithm name to their
       * signature data, so there is no need to extract that string.
       * We will ASSUME that the public key algorithm provided elsewhere
       * in the 'publickey' USERAUTH_REQUEST is accurate.
       */
      pr_trace_msg(trace_channel, 9, "assuming client did not prepend public "
        "key algorithm name to DSA signature");
      sig_type = "ssh-dss";
    }

  } else {
    sig_type = sftp_msg_read_string(p, &signature, &signaturelen);
  }

  EVP_DigestInit(&ctx, EVP_sha1());
  EVP_DigestUpdate(&ctx, sig_data, sig_datalen);
  EVP_DigestFinal(&ctx, digest, &digestlen);

  if (strcmp(sig_type, "ssh-rsa") == 0) {
    RSA *rsa;
    int ok;

    rsa = EVP_PKEY_get1_RSA(pkey);

    sig_len = sftp_msg_read_int(p, &signature, &signaturelen);
    sig = (unsigned char *) sftp_msg_read_data(p, &signature, &signaturelen,
      sig_len);

    ok = RSA_verify(NID_sha1, digest, digestlen, sig, sig_len, rsa);
    if (ok == 1) {
      res = 0;

    } else {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error verifying RSA signature: %s", sftp_crypto_get_errors());
      res = -1;
    }

    RSA_free(rsa);

  } else if (strcmp(sig_type, "ssh-dss") == 0) {
    DSA *dsa;
    DSA_SIG *dsa_sig;
    int ok;

    dsa = EVP_PKEY_get1_DSA(pkey);

    sig_len = sftp_msg_read_int(p, &signature, &signaturelen);

    /* A DSA signature string is composed of 2 20 character parts. */

    if (sig_len != 40) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "bad DSA signature len (%lu)", (unsigned long) sig_len);
    }

    sig = (unsigned char *) sftp_msg_read_data(p, &signature, &signaturelen,
      sig_len);

    dsa_sig = DSA_SIG_new();
    dsa_sig->r = BN_new();
    dsa_sig->s = BN_new();

    if (BN_bin2bn(sig, 20, dsa_sig->r) == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error obtaining 'r' DSA signature component: %s",
        sftp_crypto_get_errors());
      res = -1;
    }

    if (BN_bin2bn(sig + 20, 20, dsa_sig->s) == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error obtaining 's' DSA signature component: %s",
        sftp_crypto_get_errors());
      res = -1;
    }

    ok = DSA_do_verify(digest, digestlen, dsa_sig, dsa);
    if (ok == 1) {
      res = 0;

    } else {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error verifying DSA signature: %s", sftp_crypto_get_errors());
      res = -1;
    }

    DSA_free(dsa);
    DSA_SIG_free(dsa_sig);

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to verify signed data: unsupported signature algorithm '%s'",
      sig_type);
    return -1;
  }

  pr_memscrub(digest, digestlen);
  EVP_PKEY_free(pkey);
  return res;
}

int sftp_keys_set_passphrase_provider(const char *provider) {
  if (provider == NULL) {
    errno = EINVAL;
    return -1;
  }

  passphrase_provider = provider;
  return 0;
}

void sftp_keys_get_passphrases(void) {
  server_rec *s = NULL;

  for (s = (server_rec *) server_list->xas_list; s; s = s->next) {
    config_rec *c;
    struct sftp_pkey *k;

    c = find_config(s->conf, CONF_PARAM, "SFTPHostKey", FALSE);
    while (c) {
      pr_signals_handle();

      k = pcalloc(s->pool, sizeof(struct sftp_pkey));      
      k->pkeysz = PEM_BUFSIZE;
      k->server = s;

      if (get_passphrase(k, c->argv[0]) < 0) {
        const char *errstr;

        errstr = sftp_crypto_get_errors();
        pr_log_pri(PR_LOG_NOTICE, MOD_SFTP_VERSION
          ": error reading passphrase for SFTPHostKey '%s': %s",
          (const char *) c->argv[0], errstr ? errstr : strerror(errno));

        pr_log_pri(PR_LOG_ERR, MOD_SFTP_VERSION
          ": unable to use key in SFTPHostKey '%s', exiting",
          (const char *) c->argv[0]);
        end_login(1);
      }

      k->next = sftp_pkey_list;
      sftp_pkey_list = k;
      sftp_npkeys++;

      c = find_config_next(c, c->next, CONF_PARAM, "SFTPHostKey", FALSE);
    }
  }
}

/* Make sure that no valuable information can be inadvertently written
 * out to swap.
 */
void sftp_keys_free(void) {
  scrub_pkeys();
 
  if (sftp_dsa_hostkey) {
    EVP_PKEY_free(sftp_dsa_hostkey);
    sftp_dsa_hostkey = NULL;
  }

  if (sftp_rsa_hostkey) {
    EVP_PKEY_free(sftp_rsa_hostkey);
    sftp_rsa_hostkey = NULL;
  }
}
