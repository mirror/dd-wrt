/*
 * ProFTPD - mod_sftp key mgmt (keys)
 * Copyright (c) 2008-2022 TJ Saunders
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
 * As a special exemption, TJ Saunders and other respective copyright holders
 * give permission to link this program with OpenSSL, and distribute the
 * resulting executable, without including the source code for OpenSSL in the
 * source distribution.
 */

#include "mod_sftp.h"

#include "ssh2.h"
#include "msg.h"
#include "packet.h"
#include "crypto.h"
#include "kex.h"
#include "keys.h"
#include "agent.h"
#include "interop.h"
#include "session.h"
#include "bcrypt.h"
#if defined(PR_USE_SODIUM)
# include <sodium.h>
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL) && defined(HAVE_SHA512_OPENSSL)
# define CURVE448_SIZE          56
#endif /* HAVE_X448_OPENSSL and HAVE_SHA512_OPENSSL */

extern xaset_t *server_list;
extern module sftp_module;

/* Note: Should this size be made bigger, in light of larger hostkeys? */
#define SFTP_DEFAULT_HOSTKEY_SZ		4096
#define SFTP_MAX_SIG_SZ			4096

struct sftp_hostkey {
  enum sftp_key_type_e key_type;
  EVP_PKEY *pkey;

  /* Non-OpenSSL keys */
  unsigned char *ed25519_public_key;
  unsigned long long ed25519_public_keylen;
  unsigned char *ed25519_secret_key;
  unsigned long long ed25519_secret_keylen;

  unsigned char *ed448_public_key;
  unsigned long long ed448_public_keylen;
  unsigned char *ed448_secret_key;
  unsigned long long ed448_secret_keylen;

  const unsigned char *key_data;
  uint32_t key_datalen;

  /* This will usually not be null; if the key was obtained from a local
   * file, this will point to that file.
   */
  const char *file_path;

  /* This will usually be null; if the key was obtained from an agent,
   * this point will point to the Unix domain socket to use for talking
   * to that agent, e.g. for data signing requests.
   */
  const char *agent_path;
};

static struct sftp_hostkey *sftp_dsa_hostkey = NULL;
static struct sftp_hostkey *sftp_ed25519_hostkey = NULL;
static struct sftp_hostkey *sftp_ed448_hostkey = NULL;
static struct sftp_hostkey *sftp_rsa_hostkey = NULL;

#if defined(PR_USE_OPENSSL_ECC)
static struct sftp_hostkey *sftp_ecdsa256_hostkey = NULL;
static struct sftp_hostkey *sftp_ecdsa384_hostkey = NULL;
static struct sftp_hostkey *sftp_ecdsa521_hostkey = NULL;
#endif /* PR_USE_OPENSSL_ECC */

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

/* Default minimum key sizes, in BITS.  The RSA minimum of 768 bits comes from
 * the OpenSSH-7.2 implementation.  And the others follow from that, based on
 * the assumptions described here:
 *   https://en.wikipedia.org/wiki/Key_size#Asymmetric_algorithm_key_lengths
 *   http://www.emc.com/emc-plus/rsa-labs/standards-initiatives/key-size.htm
 *
 * Note that the RSA size refers to the size of the modulus.  The DSA size
 * refers to the size of the modulus.  The EC size refers to the minimum
 * order of the base point on the elliptic curve.
 */
static int keys_rsa_min_nbits = 768;
static int keys_dsa_min_nbits = 384;
static int keys_ec_min_nbits = 160;

/* Public key files start with "BEGIN ... PUBLIC KEY" and "END ... PUBLIC KEY"
 * lines.  Note that the "..." can be multiple different values ("RSA", "SSH2",
 * etc).
 */
#define SFTP_PUBLICKEY_BEGIN		"BEGIN PUBLIC KEY"
#define SFTP_PUBLICKEY_BEGIN_LEN	(sizeof(SFTP_PUBLICKEY_BEGIN) - 1)
#define SFTP_PUBLICKEY_END		"END PUBLIC KEY"
#define SFTP_PUBLICKEY_END_LEN		(sizeof(SFTP_PUBLICKEY_BEGIN) - 1)

/* OpenSSH's homegrown private key file format.
 *
 * See the PROTOCOL.key file in the OpenSSH source distribution for details
 * on their homegrown private key format.  See also the implementations in
 * sskey.c#sshkey_private_to_blob2 (for writing private keys) and
 * sshkey.c#sshkey_parse_private2 (for reading private keys).  The values
 * for different encryption ciphers are in the `ciphers[]` table in cipher.c.
 */

#define SFTP_OPENSSH_BEGIN		"-----BEGIN OPENSSH PRIVATE KEY-----\n"
#define SFTP_OPENSSH_END		"-----END OPENSSH PRIVATE KEY-----\n"
#define SFTP_OPENSSH_BEGIN_LEN		(sizeof(SFTP_OPENSSH_BEGIN) - 1)
#define SFTP_OPENSSH_END_LEN		(sizeof(SFTP_OPENSSH_END) - 1)
#define SFTP_OPENSSH_KDFNAME		"bcrypt"
#define SFTP_OPENSSH_MAGIC		"openssh-key-v1"

/* Encryption cipher info. */
struct openssh_cipher {
  const char *algo;
  uint32_t blocksz;
  uint32_t key_len;
  uint32_t iv_len;
  uint32_t auth_len;

  const EVP_CIPHER *cipher;
  const EVP_CIPHER *(*get_cipher)(void);
};

static struct openssh_cipher ciphers[] = {
  { "none",        8,  0, 0, 0, NULL, EVP_enc_null },
  { "aes256-cbc", 16, 32, 16, 0, NULL, EVP_aes_256_cbc },
#if defined(HAVE_EVP_AES_256_CTR_OPENSSL)
  { "aes256-ctr", 16, 32, 16, 0, NULL, EVP_aes_256_ctr },
#else
  { "aes256-ctr", 16, 32, 16, 0, NULL, NULL },
#endif /* HAVE_EVP_AES_256_CTR_OPENSSL */

  { NULL,          0,  0, 0, 0, NULL, NULL }
};

static int handle_ed448_hostkey(pool *p, const unsigned char *key_data,
    uint32_t key_datalen, const char *file_path);
static int read_openssh_private_key(pool *p, const char *path, int fd,
    const char *passphrase, enum sftp_key_type_e *key_type, EVP_PKEY **pkey,
    unsigned char **key, uint32_t *keylen);

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
    /* Ignore ENOSYS (and EPERM, since some libc's use this as ENOSYS). */
    if (errno != ENOSYS &&
        errno != EPERM) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION ": getrlimit error: %s",
        strerror(errno));
    }

    /* Pick some arbitrary high number. */
    nfiles = 255;

  } else {
    nfiles = (unsigned long) rlim.rlim_max;
  }

#else /* no RLIMIT_NOFILE or RLIMIT_OFILE */
   nfiles = 255;
#endif

  /* Appears that on some platforms (e.g. Solaris, Mac OSX), having too
   * high of an fd value can lead to undesirable behavior for some reason.
   * Need to track down why; the behavior I saw was the inability of
   * select() to work properly on the stdout/stderr fds attached to the
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

  if (sigaction(SIGINT, &sa_ignore, &sa_intr) < 0) {
    return -1;
  }

  if (sigaction(SIGQUIT, &sa_ignore, &sa_quit) < 0) {
    return -1;
  }

  sigemptyset(&set_chldmask);
  sigaddset(&set_chldmask, SIGCHLD);

  if (sigprocmask(SIG_BLOCK, &set_chldmask, &set_save) < 0) {
    return -1;
  }

  prepare_provider_pipes(stdout_pipe, stderr_pipe);

  pid = fork();
  if (pid < 0) {
    int xerrno = errno;

    pr_log_pri(PR_LOG_ALERT,
      MOD_SFTP_VERSION ": error: unable to fork: %s", strerror(xerrno));

    errno = xerrno;
    status = -1;

  } else if (pid == 0) {
    char nbuf[32];
    pool *tmp_pool;
    char *stdin_argv[4];

    /* Child process */
    session.pid = getpid();

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
    pr_snprintf(nbuf, sizeof(nbuf)-1, "%u", (unsigned int) s->ServerPort);
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
            buf[buflen-1] = '\0';

            while (res &&
                   (buf[res-1] == '\r' ||
                    buf[res-1] == '\n')) {
              pr_signals_handle();
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
          long stderrlen, stderrsz;
          char *stderrbuf;
          pool *tmp_pool = make_sub_pool(s->pool);

          stderrbuf = pr_fsio_getpipebuf(tmp_pool, stderr_pipe[0], &stderrsz);
          memset(stderrbuf, '\0', stderrsz);

          stderrlen = read(stderr_pipe[0], stderrbuf, stderrsz-1);
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

          destroy_pool(tmp_pool);
          tmp_pool = NULL;
        }
      }

      res = waitpid(pid, &status, WNOHANG);
    }
  }

  /* Restore the previous signal actions. */
  if (sigaction(SIGINT, &sa_intr, NULL) < 0) {
    return -1;
  }

  if (sigaction(SIGQUIT, &sa_quit, NULL) < 0) {
    return -1;
  }

  if (sigprocmask(SIG_SETMASK, &set_save, NULL) < 0) {
    return -1;
  }

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

  d = calloc(1, sz + (pagesz-1));
  if (d == NULL) {
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
    exit(1);
  }

  *ptr = d;

  p = ((long) d + (pagesz-1)) &~ (pagesz-1);

  return ((char *) p);
}

static unsigned char *decode_base64(pool *p, unsigned char *text,
    size_t text_len, size_t *data_len) {
  unsigned char *data = NULL;
  int have_padding = FALSE, res;

  /* Due to Base64's padding, we need to detect if the last block was padded
   * with zeros; we do this by looking for '=' characters at the end of the
   * text being decoded.  If we see these characters, then we will "trim" off
   * any trailing zero values in the decoded data, on the ASSUMPTION that they
   * are the auto-added padding bytes.
   */
  if (text[text_len-1] == '=') {
    have_padding = TRUE;
  }

  data = pcalloc(p, text_len);
  res = EVP_DecodeBlock((unsigned char *) data, (unsigned char *) text,
    (int) text_len);
  if (res <= 0) {
    /* Base64-decoding error. */
    errno = EINVAL;
    return NULL;
  }

  if (have_padding == TRUE) {
    /* Assume that only one or two zero bytes of padding were added. */
    if (data[res-1] == '\0') {
      res -= 1;

      if (data[res-1] == '\0') {
        res -= 1;
      }
    }
  }

  *data_len = (size_t) res;
  return data;
}

static int is_public_key(int fd) {
  struct stat st;
  char begin_buf[SFTP_PUBLICKEY_BEGIN_LEN+20];
  ssize_t len;
  off_t minsz;

  if (fstat(fd, &st) < 0) {
    return -1;
  }

  minsz = SFTP_PUBLICKEY_BEGIN_LEN + SFTP_PUBLICKEY_END_LEN;
  if (st.st_size < minsz) {
    return FALSE;
  }

  len = pread(fd, begin_buf, sizeof(begin_buf), 0);
  if (len != sizeof(begin_buf)) {
    return FALSE;
  }

  begin_buf[len-1] = '\0';

  if (strstr(begin_buf, "PUBLIC KEY") == NULL) {
    return FALSE;
  }

  if (strstr(begin_buf, "BEGIN") == NULL) {
    return FALSE;
  }

  return TRUE;
}

static int is_openssh_private_key(int fd) {
  struct stat st;
  char begin_buf[SFTP_OPENSSH_BEGIN_LEN], end_buf[SFTP_OPENSSH_END_LEN];
  ssize_t len;
  off_t minsz;

  if (fstat(fd, &st) < 0) {
    return -1;
  }

  minsz = SFTP_OPENSSH_BEGIN_LEN + SFTP_OPENSSH_END_LEN;
  if (st.st_size < minsz) {
    return FALSE;
  }

  len = pread(fd, begin_buf, sizeof(begin_buf), 0);
  if (len != sizeof(begin_buf)) {
    return FALSE;
  }

  if (memcmp(begin_buf, SFTP_OPENSSH_BEGIN, SFTP_OPENSSH_BEGIN_LEN) != 0) {
    return FALSE;
  }

  len = pread(fd, end_buf, sizeof(end_buf), st.st_size - SFTP_OPENSSH_END_LEN);
  if (len != sizeof(end_buf)) {
    return FALSE;
  }

  if (memcmp(end_buf, SFTP_OPENSSH_END, SFTP_OPENSSH_END_LEN) != 0) {
    return FALSE;
  }

  return TRUE;
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

      /* Ensure that the buffer is NUL-terminated. */
      buf[buflen-1] = '\0';
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
      size_t pwlen;
      /* Ensure that the buffer is NUL-terminated. */
      buf[buflen-1] = '\0';

      pwlen = strlen(buf);

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
  pool *tmp_pool;
  char prompt[256];
  FILE *fp = NULL;
  EVP_PKEY *pkey = NULL;
  unsigned char *key_data = NULL;
  uint32_t key_datalen = 0;
  int fd, prompt_fd = -1, res, xerrno, openssh_format = FALSE,
    public_key_format = FALSE;
  struct sftp_pkey_data pdata;
  register unsigned int attempt;

  memset(prompt, '\0', sizeof(prompt));
  res = pr_snprintf(prompt, sizeof(prompt)-1,
    "Host key for the %s#%d (%s) server: ",
    pr_netaddr_get_ipstr(k->server->addr), k->server->ServerPort,
    k->server->ServerName);
  prompt[res] = '\0';
  prompt[sizeof(prompt)-1] = '\0';

  PRIVS_ROOT
  fd = open(path, O_RDONLY);
  xerrno = errno;
  PRIVS_RELINQUISH

  if (fd < 0) {
    SYSerr(SYS_F_FOPEN, xerrno);
    errno = xerrno;
    return -1;
  }

  /* Make sure the fd isn't one of the big three. */
  if (fd <= STDERR_FILENO) {
    res = pr_fs_get_usable_fd(fd);
    if (res >= 0) {
      (void) close(fd);
      fd = res;
    }
  }

  public_key_format = is_public_key(fd);
  if (public_key_format == TRUE) {
    pr_trace_msg(trace_channel, 3, "hostkey file '%s' uses a public key format",
      path);
    (void) pr_log_pri(PR_LOG_WARNING, MOD_SFTP_VERSION
      ": unable to use public key '%s' for SFTPHostKey", path);
    (void) close(fd);
    errno = EINVAL;
    return -1;
  }

  openssh_format = is_openssh_private_key(fd);
  if (openssh_format != TRUE) {
    fp = fdopen(fd, "r");
    if (fp == NULL) {
      xerrno = errno;

      (void) close(fd);
      SYSerr(SYS_F_FOPEN, xerrno);

      errno = xerrno;
      return -1;
    }

    /* As the file contains sensitive data, we do not want it lingering
     * around in stdio buffers.
     */
    (void) setvbuf(fp, NULL, _IONBF, 0);

  } else {
    pr_trace_msg(trace_channel, 9,
      "handling host key '%s' as an OpenSSH-formatted private key", path);
  }

  k->host_pkey = get_page(PEM_BUFSIZE, &k->host_pkey_ptr);
  if (k->host_pkey == NULL) {
    pr_log_pri(PR_LOG_ALERT, MOD_SFTP_VERSION ": Out of memory!");
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

  tmp_pool = make_sub_pool(sftp_pool);
  pr_pool_tag(tmp_pool, "SFTP Passphrase pool");

  /* The user gets three tries to enter the correct passphrase. */
  for (attempt = 0; attempt < 3; attempt++) {

    /* Always handle signals in a loop. */
    pr_signals_handle();

    if (openssh_format == FALSE) {
      pkey = PEM_read_PrivateKey(fp, NULL, get_passphrase_cb, &pdata);
      if (pkey != NULL) {
        break;
      }

      if (fseek(fp, 0, SEEK_SET) < 0) {
        pr_trace_msg(trace_channel, 3,
          "error rewinding file handle for '%s': %s", path, strerror(errno));
      }

    } else {
      char buf[PEM_BUFSIZE];
      const char *passphrase;
      enum sftp_key_type_e key_type = SFTP_KEY_UNKNOWN;

      /* First we try with no passphrase.  Failing that, we have to invoke the
       * get_passphase_cb() callback ourselves for OpenSSH keys.
       */
      if (attempt == 0) {
        passphrase = pstrdup(tmp_pool, "");
        res = read_openssh_private_key(tmp_pool, path, fd, passphrase,
          &key_type, &pkey, &key_data, &key_datalen);

        if (lseek(fd, 0, SEEK_SET) < 0) {
          pr_trace_msg(trace_channel, 3, "error rewinding fd %d for '%s': %s",
            fd, path, strerror(errno));
        }
        if (res == 0) {
          break;
        }
      }

      res = get_passphrase_cb(buf, PEM_BUFSIZE, 0, &pdata);
      if (res > 0) {
        passphrase = pdata.buf;

        res = read_openssh_private_key(tmp_pool, path, fd, passphrase,
          &key_type, &pkey, &key_data, &key_datalen);
        if (res == 0) {
          break;
        }

        if (lseek(fd, 0, SEEK_SET) < 0) {
          pr_trace_msg(trace_channel, 3, "error rewinding fd %d for '%s': %s",
            fd, path, strerror(errno));
        }

      } else {
        pr_trace_msg(trace_channel, 2,
          "error reading passphrase for OpenSSH key: %s",
          sftp_crypto_get_errors());
      }
    }

    ERR_clear_error();
    fprintf(stderr, "\nWrong passphrase for this key.  Please try again.\n");
  }

  if (fp != NULL) {
    fclose(fp);
  }

  /* Restore the normal stderr logging. */
  (void) dup2(prompt_fd, STDERR_FILENO);
  (void) close(prompt_fd);

  if (pkey == NULL &&
      key_data == NULL) {
    return -1;
  }

  if (pkey != NULL) {
    EVP_PKEY_free(pkey);
  }

  if (key_data != NULL) {
    pr_memscrub(key_data, key_datalen);
  }

  destroy_pool(tmp_pool);

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

  if (sftp_pkey_list == NULL) {
    return;
  }
 
  /* Scrub and free all passphrases in memory. */
  pr_log_debug(DEBUG5, MOD_SFTP_VERSION
    ": scrubbing %u %s from memory",
    sftp_npkeys, sftp_npkeys != 1 ? "passphrases" : "passphrase");
 
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
    sstrncpy(buf, k->host_pkey, buflen);
    buf[buflen - 1] = '\0';
    return strlen(buf);
  }

  return 0;
}

static int has_req_perms(int fd, const char *path) {
  struct stat st;

  if (fstat(fd, &st) < 0) {
    return -1;
  }

  if (st.st_mode & (S_IRWXG|S_IRWXO)) {
    if (!(sftp_opts & SFTP_OPT_INSECURE_HOSTKEY_PERMS)) {
      errno = EACCES;
      return -1;
    }

    pr_log_pri(PR_LOG_INFO, MOD_SFTP_VERSION
      "notice: the permissions on SFTPHostKey '%s' (%04o) allow "
      "group-readable and/or world-readable access, increasing chances of "
      "system users reading the private key", path, st.st_mode);
  }

  return 0;
}

static uint32_t read_pkey_from_data(pool *p, unsigned char *pkey_data,
    uint32_t pkey_datalen, EVP_PKEY **pkey, enum sftp_key_type_e *key_type,
    int openssh_format) {
  char *pkey_type = NULL;
  uint32_t res, len = 0;

  res = sftp_msg_read_string2(p, &pkey_data, &pkey_datalen, &pkey_type);
  if (res == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading key: invalid/unsupported key format");
    return 0;
  }
  len += res;

  if (strcmp(pkey_type, "ssh-rsa") == 0) {
    RSA *rsa;
    const BIGNUM *rsa_e = NULL, *rsa_n = NULL, *rsa_d = NULL;

    *pkey = EVP_PKEY_new();
    if (*pkey == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating EVP_PKEY: %s", sftp_crypto_get_errors());
      return 0;
    }

    rsa = RSA_new();
    if (rsa == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating RSA: %s", sftp_crypto_get_errors());
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }

    res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &rsa_e);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      RSA_free(rsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }
    len += res;

    res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &rsa_n);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      RSA_free(rsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }
    len += res;

    if (openssh_format == TRUE) {
      const BIGNUM *rsa_p, *rsa_q, *rsa_iqmp;

      /* The OpenSSH private key format encodes more factors. */

      res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &rsa_d);
      if (res == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error reading key: invalid/unsupported key format");
        RSA_free(rsa);
        EVP_PKEY_free(*pkey);
        *pkey = NULL;
        return 0;
      }
      len += res;

      /* RSA_get0_crt_params */
      res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &rsa_iqmp);
      if (res == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error reading key: invalid/unsupported key format");
        RSA_free(rsa);
        EVP_PKEY_free(*pkey);
        *pkey = NULL;
        return 0;
      }
      len += res;

      /* RSA_get0_factors */
      res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &rsa_p);
      if (res == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error reading key: invalid/unsupported key format");
        RSA_free(rsa);
        EVP_PKEY_free(*pkey);
        *pkey = NULL;
        return 0;
      }
      len += res;

      res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &rsa_q);
      if (res == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error reading key: invalid/unsupported key format");
        RSA_free(rsa);
        EVP_PKEY_free(*pkey);
        *pkey = NULL;
        return 0;
      }
      len += res;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
      RSA_set0_crt_params(rsa, NULL, NULL, (BIGNUM *) rsa_iqmp);
      RSA_set0_factors(rsa, (BIGNUM *) rsa_p, (BIGNUM *) rsa_q);
#else
      rsa->iqmp = rsa_iqmp;
      rsa->p = rsa_p;
      rsa->q = rsa_q;
#endif /* prior to OpenSSL-1.1.0 */

      /* Turns out that for OpenSSH formatted RSA keys, the 'e' and 'n' values
       * are in the opposite order than the normal PEM format.  Typical.
       */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
      RSA_set0_key(rsa, (BIGNUM *) rsa_e, (BIGNUM *) rsa_n, (BIGNUM *) rsa_d);
#else
      rsa->e = rsa_n;
      rsa->n = rsa_e;
      rsa->d = rsa_d;
#endif /* prior to OpenSSL-1.1.0 */
    } else {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
      RSA_set0_key(rsa, (BIGNUM *) rsa_n, (BIGNUM *) rsa_e, (BIGNUM *) rsa_d);
#else
      rsa->e = rsa_e;
      rsa->n = rsa_n;
      rsa->d = rsa_d;
#endif /* prior to OpenSSL-1.1.0 */
    }

    if (EVP_PKEY_assign_RSA(*pkey, rsa) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error assigning RSA to EVP_PKEY: %s", sftp_crypto_get_errors());
      RSA_free(rsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }

    if (key_type != NULL) {
      *key_type = SFTP_KEY_RSA;
    }

  } else if (strcmp(pkey_type, "ssh-dss") == 0) {
#if !defined(OPENSSL_NO_DSA)
    DSA *dsa;
    const BIGNUM *dsa_p, *dsa_q, *dsa_g, *dsa_pub_key, *dsa_priv_key = NULL;

    *pkey = EVP_PKEY_new();
    if (*pkey == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating EVP_PKEY: %s", sftp_crypto_get_errors());
      return 0;
    }

    dsa = DSA_new();
    if (dsa == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating DSA: %s", sftp_crypto_get_errors());
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }

    res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &dsa_p);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      DSA_free(dsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }
    len += res;

    res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &dsa_q);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      DSA_free(dsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }
    len += res;

    res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &dsa_g);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      DSA_free(dsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }
    len += res;

    res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &dsa_pub_key);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      DSA_free(dsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }
    len += res;

    if (openssh_format == TRUE) {
      res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &dsa_priv_key);
      if (res == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error reading key: invalid/unsupported key format");
        DSA_free(dsa);
        EVP_PKEY_free(*pkey);
        *pkey = NULL;
        return 0;
      }
      len += res;
    }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
    DSA_set0_pqg(dsa, (BIGNUM *) dsa_p, (BIGNUM *) dsa_q, (BIGNUM *) dsa_g);
    DSA_set0_key(dsa, (BIGNUM *) dsa_pub_key, (BIGNUM *) dsa_priv_key);
#else
    dsa->p = dsa_p;
    dsa->q = dsa_q;
    dsa->g = dsa_g;
    dsa->pub_key = dsa_pub_key;
    dsa->priv_key = dsa_priv_key;
#endif /* prior to OpenSSL-1.1.0 */

    if (EVP_PKEY_assign_DSA(*pkey, dsa) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error assigning RSA to EVP_PKEY: %s", sftp_crypto_get_errors());
      DSA_free(dsa);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }

    if (key_type != NULL) {
      *key_type = SFTP_KEY_DSA;
    }
#else
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unsupported public key algorithm '%s'", pkey_type);
    errno = EINVAL;
    return 0;
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
  } else if (strcmp(pkey_type, "ecdsa-sha2-nistp256") == 0 ||
             strcmp(pkey_type, "ecdsa-sha2-nistp384") == 0 ||
             strcmp(pkey_type, "ecdsa-sha2-nistp521") == 0) {
    EC_KEY *ec;
    const char *curve_name;
    const EC_GROUP *curve;
    EC_POINT *point;
    int ec_nid;
    char *ptr = NULL;

    res = sftp_msg_read_string2(p, &pkey_data, &pkey_datalen, &ptr);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      return 0;
    }
    len += res;

    curve_name = (const char *) ptr;

    /* If the curve name does not match the last 8 characters of the
     * public key type (which, in the case of ECDSA keys, contains the
     * curve name), then it's definitely a mismatch.
     */
    if (strncmp(pkey_type + 11, curve_name, 9) != 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "EC public key curve name '%s' does not match public key "
        "algorithm '%s'", curve_name, pkey_type);
      return 0;
    }

    if (strncmp(curve_name, "nistp256", 9) == 0) {
      ec_nid = NID_X9_62_prime256v1;

      if (key_type != NULL) {
        *key_type = SFTP_KEY_ECDSA_256;
      }

    } else if (strncmp(curve_name, "nistp384", 9) == 0) {
      ec_nid = NID_secp384r1;

      if (key_type != NULL) {
        *key_type = SFTP_KEY_ECDSA_384;
      }

    } else if (strncmp(curve_name, "nistp521", 9) == 0) {
      ec_nid = NID_secp521r1;

      if (key_type != NULL) {
        *key_type = SFTP_KEY_ECDSA_521;
      }

    } else {
      ec_nid = -1;
    }

    ec = EC_KEY_new_by_curve_name(ec_nid);
    if (ec == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating EC_KEY for %s: %s", pkey_type,
        sftp_crypto_get_errors());
      return 0;
    }

    curve = EC_KEY_get0_group(ec);

    point = EC_POINT_new(curve);
    if (point == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating EC_POINT for %s: %s", pkey_type,
        sftp_crypto_get_errors());
      EC_KEY_free(ec);
      return 0;
    }

    res = sftp_msg_read_ecpoint2(p, &pkey_data, &pkey_datalen, curve, &point);
    if (res == 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading key: invalid/unsupported key format");
      EC_KEY_free(ec);
      return 0;
    }
    len += res;

    if (point == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error reading EC_POINT from public key data: %s", strerror(errno));
      EC_POINT_free(point);
      EC_KEY_free(ec);
      return 0;
    }

    if (sftp_keys_validate_ecdsa_params(curve, point) < 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error validating EC public key: %s", strerror(errno));
      EC_POINT_free(point);
      EC_KEY_free(ec);
      return 0;
    }

    if (EC_KEY_set_public_key(ec, point) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting public key on EC_KEY: %s", sftp_crypto_get_errors());
      EC_POINT_free(point);
      EC_KEY_free(ec);
      return 0;
    }

    if (openssh_format == TRUE) {
      const BIGNUM *ec_priv_key = NULL;

      res = sftp_msg_read_mpint2(p, &pkey_data, &pkey_datalen, &ec_priv_key);
      if (res == 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error reading key: invalid/unsupported key format");
        EC_POINT_free(point);
        EC_KEY_free(ec);
        *pkey = NULL;
        return 0;
      }
      len += res;

      if (EC_KEY_set_private_key(ec, ec_priv_key) != 1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error setting private key on EC_KEY: %s", sftp_crypto_get_errors());
        EC_POINT_free(point);
        EC_KEY_free(ec);
        return 0;
      }
    }

    *pkey = EVP_PKEY_new();
    if (*pkey == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error allocating EVP_PKEY: %s", sftp_crypto_get_errors());
      EC_POINT_free(point);
      EC_KEY_free(ec);
      return 0;
    }

    if (EVP_PKEY_assign_EC_KEY(*pkey, ec) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error assigning ECDSA-256 to EVP_PKEY: %s", sftp_crypto_get_errors());
      EC_POINT_free(point);
      EC_KEY_free(ec);
      EVP_PKEY_free(*pkey);
      *pkey = NULL;
      return 0;
    }
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
  } else if (strcmp(pkey_type, "ssh-ed25519") == 0) {
    if (key_type != NULL) {
      *key_type = SFTP_KEY_ED25519;
    }
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
  } else if (strcmp(pkey_type, "ssh-ed448") == 0) {
    if (key_type != NULL) {
      *key_type = SFTP_KEY_ED448;
    }
#endif /* HAVE_X448_OPENSSL */

  } else {
    pr_trace_msg(trace_channel, 3, "unsupported public key algorithm '%s'",
      pkey_type);
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unsupported public key algorithm '%s'", pkey_type);
    errno = EINVAL;
    return 0;
  }

  return len;
}

static const char *get_pkey_type_desc(int pkey_type) {
  const char *key_desc = NULL;

  switch (pkey_type) {
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
      key_desc = "ECC";
      break;
#endif

    default:
      key_desc = "unknown";
  }

  return key_desc;
}

static const char *get_key_type_desc(enum sftp_key_type_e key_type) {
  const char *key_desc = NULL;

  switch (key_type) {
    case SFTP_KEY_UNKNOWN:
      key_desc = "unknown";
      break;

    case SFTP_KEY_DSA:
      key_desc = "DSA";
      break;

    case SFTP_KEY_RSA:
      key_desc = "RSA";
      break;

    case SFTP_KEY_ECDSA_256:
      key_desc = "ECDSA256";
      break;

    case SFTP_KEY_ECDSA_384:
      key_desc = "ECDSA384";
      break;

    case SFTP_KEY_ECDSA_521:
      key_desc = "ECDSA521";
      break;

    case SFTP_KEY_ED25519:
      key_desc = "ED25519";
      break;

    case SFTP_KEY_ED448:
      key_desc = "ED448";
      break;

    default:
      key_desc = "undefined";
      break;
  }

  return key_desc;
}

#if defined(PR_USE_OPENSSL_ECC)
/* Make sure the given ECDSA private key is suitable for use. */
static int validate_ecdsa_private_key(const EC_KEY *ec) {
  BN_CTX *bn_ctx;
  BIGNUM *ec_order, *bn_tmp;
  int ec_order_nbits, priv_key_nbits;

  /* A BN_CTX is like our pools; we allocate one, use it to get any
   * number of BIGNUM variables, and only have free up the BN_CTX when
   * we're done, rather than all of the individual BIGNUMs.
   */

  bn_ctx = BN_CTX_new();
  if (bn_ctx == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocating BN_CTX: %s", sftp_crypto_get_errors());
    return -1;
  }

  BN_CTX_start(bn_ctx);

  ec_order = BN_CTX_get(bn_ctx);
  if (ec_order == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting new BIGNUM from BN_CTX: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  bn_tmp = BN_CTX_get(bn_ctx);
  if (bn_tmp == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting new BIGNUM from BN_CTX: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  /* Make sure that log2(private key) is greater than log2(EC order)/2. */

  if (EC_GROUP_get_order(EC_KEY_get0_group(ec), ec_order, bn_ctx) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting the EC group order: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1; 
  }

  priv_key_nbits = BN_num_bits(EC_KEY_get0_private_key(ec));
  ec_order_nbits = BN_num_bits(ec_order);

  if (priv_key_nbits <= (ec_order_nbits / 2)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "ECDSA private key (%d bits) is too small, must be at "
      "least %d bits", priv_key_nbits, ec_order_nbits);
    BN_CTX_free(bn_ctx);
    errno = EACCES;
    return -1; 
  }

  /* Ensure that the private key < (EC order - 1). */

  if (BN_sub(bn_tmp, ec_order, BN_value_one()) == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error subtracting one from EC group order: %s",
      sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1; 
  }

  if (BN_cmp(EC_KEY_get0_private_key(ec), bn_tmp) >= 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "ECDSA private key is greater than or equal to EC group order, "
      "rejecting");
    BN_CTX_free(bn_ctx);
    errno = EACCES;
    return -1; 
  }

  BN_CTX_free(bn_ctx);
  return 0;
}

/* This is used to validate the ECDSA parameters we might receive e.g. from
 * a client.  These checks come from Section 3.2.2.1 of 'Standards for
 * Efficient Cryptography Group, "Elliptic Curve Cryptography", SEC 1,
 * May 2009:
 *
 *  http://www.secg.org/download/aid-780/sec1-v2.pdf
 *
 * as per RFC 5656 recommendation.
 */
int sftp_keys_validate_ecdsa_params(const EC_GROUP *group,
    const EC_POINT *point) {
  BN_CTX *bn_ctx;
  BIGNUM *ec_order, *x_coord, *y_coord, *bn_tmp;
  int coord_nbits, ec_order_nbits;
  EC_POINT *subgroup_order = NULL;

  if (EC_METHOD_get_field_type(EC_GROUP_method_of(group)) != NID_X9_62_prime_field) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "ECDSA group is not a prime field, rejecting");
    errno = EACCES;
    return -1;
  }

  /* A Q of infinity is unacceptable. */
  if (EC_POINT_is_at_infinity(group, point) != 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "ECDSA EC point has infinite value, rejecting");
    errno = EACCES;
    return -1;
  }
 
  /* A BN_CTX is like our pools; we allocate one, use it to get any
   * number of BIGNUM variables, and only have free up the BN_CTX when
   * we're done, rather than all of the individual BIGNUMs.
   */

  bn_ctx = BN_CTX_new();
  if (bn_ctx == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocating BN_CTX: %s", sftp_crypto_get_errors());
    return -1;
  }

  BN_CTX_start(bn_ctx);

  ec_order = BN_CTX_get(bn_ctx);
  if (ec_order == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting new BIGNUM from BN_CTX: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  if (EC_GROUP_get_order(group, ec_order, bn_ctx) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting EC group order: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  x_coord = BN_CTX_get(bn_ctx);
  if (x_coord == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting new BIGNUM from BN_CTX: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  y_coord = BN_CTX_get(bn_ctx);
  if (y_coord == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting new BIGNUM from BN_CTX: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  if (EC_POINT_get_affine_coordinates_GFp(group, point, x_coord, y_coord,
      bn_ctx) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting EC point affine coordinates: %s",
      sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  /* Ensure that the following are both true:
   *
   *  log2(X coord) > log2(EC order)/2 
   *  log2(Y coord) > log2(EC order)/2
   */

  coord_nbits = BN_num_bits(x_coord);
  ec_order_nbits = BN_num_bits(ec_order);
  if (coord_nbits <= (ec_order_nbits / 2)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "EC public key X coordinate (%d bits) too small (<= %d bits), rejecting",
      coord_nbits, (ec_order_nbits / 2));
    BN_CTX_free(bn_ctx);
    errno = EACCES;
    return -1;
  }

  coord_nbits = BN_num_bits(y_coord);
  if (coord_nbits <= (ec_order_nbits / 2)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "EC public key Y coordinate (%d bits) too small (<= %d bits), rejecting",
      coord_nbits, (ec_order_nbits / 2));
    BN_CTX_free(bn_ctx);
    errno = EACCES;
    return -1;
  }

  /* Ensure that the following is true:
   *
   *  subgroup order == infinity
   */

  subgroup_order = EC_POINT_new(group);
  if (subgroup_order == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocating new EC_POINT: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  if (EC_POINT_mul(group, subgroup_order, NULL, point, ec_order, bn_ctx) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error doing EC point multiplication: %s", sftp_crypto_get_errors());
    EC_POINT_free(subgroup_order);
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  if (EC_POINT_is_at_infinity(group, subgroup_order) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "EC public key has finite subgroup order, rejecting");
    EC_POINT_free(subgroup_order);
    BN_CTX_free(bn_ctx);
    errno = EACCES;
    return -1;
  }

  EC_POINT_free(subgroup_order);

  /*  Ensure that the following are both true:
   *
   *  X < order - 1
   *  Y < order - 1
   */ 

  bn_tmp = BN_CTX_get(bn_ctx);
  if (bn_tmp == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error getting new BIGNUM from BN_CTX: %s", sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  if (BN_sub(bn_tmp, ec_order, BN_value_one()) == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error subtracting one from EC group order: %s",
      sftp_crypto_get_errors());
    BN_CTX_free(bn_ctx);
    errno = EPERM;
    return -1;
  }

  if (BN_cmp(x_coord, bn_tmp) >= 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "EC public key X coordinate too large (>= EC group order - 1), "
      "rejecting");
    BN_CTX_free(bn_ctx);
    errno = EACCES;
    return -1;
  }

  if (BN_cmp(y_coord, bn_tmp) >= 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "EC public key Y coordinate too large (>= EC group order - 1), "
      "rejecting");
    BN_CTX_free(bn_ctx);
    errno = EACCES;
    return -1;
  }

  BN_CTX_free(bn_ctx);
  return 0;
}
#endif /* PR_USE_OPENSSL_ECC */

#ifdef SFTP_DEBUG_KEYS
static void debug_rsa_key(pool *p, const char *label, RSA *rsa) {
  BIO *bio = NULL;
  char *data;
  long datalen;

  bio = BIO_new(BIO_s_mem());
  RSA_print(bio, rsa, 0);
  BIO_flush(bio);
  datalen = BIO_get_mem_data(bio, &data);
  if (data != NULL &&
      datalen > 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "%s",label);
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION, "%.*s",
      (int) datalen, data);
  }

  BIO_free(bio);
}
#endif

static int get_pkey_type(EVP_PKEY *pkey) {
  int pkey_type;

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESS)
  pkey_type = EVP_PKEY_base_id(pkey);
#else
  pkey_type = EVP_PKEY_type(pkey->type);
#endif /* OpenSSL 1.1.x and later */

  return pkey_type;
}

static int rsa_compare_keys(pool *p, EVP_PKEY *remote_pkey,
    EVP_PKEY *local_pkey) {
  RSA *remote_rsa = NULL, *local_rsa = NULL;
  const BIGNUM *remote_rsa_e = NULL, *local_rsa_e = NULL;
  const BIGNUM *remote_rsa_n = NULL, *local_rsa_n = NULL;
  int res = 0;

  local_rsa = EVP_PKEY_get1_RSA(local_pkey);
  if (keys_rsa_min_nbits > 0) {
    int rsa_nbits;

    rsa_nbits = RSA_size(local_rsa) * 8;
    if (rsa_nbits < keys_rsa_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "local RSA key size (%d bits) less than required minimum (%d bits)",
        rsa_nbits, keys_rsa_min_nbits);
      RSA_free(local_rsa);
      errno = EPERM;
      return -1;
    }

    pr_trace_msg(trace_channel, 19,
      "comparing RSA keys using local RSA key (%d bits, min %d)", rsa_nbits,
      keys_rsa_min_nbits);
  }

  remote_rsa = EVP_PKEY_get1_RSA(remote_pkey);

#ifdef SFTP_DEBUG_KEYS
  debug_rsa_key(p, "remote RSA key:", remote_rsa);
  debug_rsa_key(p, "local RSA key:", local_rsa);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  RSA_get0_key(remote_rsa, &remote_rsa_n, &remote_rsa_e, NULL);
  RSA_get0_key(local_rsa, &local_rsa_n, &local_rsa_e, NULL);
#else
  remote_rsa_e = remote_rsa->e;
  local_rsa_e = local_rsa->e;
  remote_rsa_n = remote_rsa->n;
  local_rsa_n = local_rsa->n;
#endif /* prior to OpenSSL-1.1.0 */

  if (BN_cmp(remote_rsa_e, local_rsa_e) != 0) {
    pr_trace_msg(trace_channel, 17, "%s",
      "RSA key mismatch: client-sent RSA key component 'e' does not match "
      "local RSA key component 'e'");
    res = -1;
  }

  if (res == 0) {
    if (BN_cmp(remote_rsa_n, local_rsa_n) != 0) {
      pr_trace_msg(trace_channel, 17, "%s",
        "RSA key mismatch: client-sent RSA key component 'n' does not match "
        "local RSA key component 'n'");
      res = -1;
    }
  }

  RSA_free(remote_rsa);
  RSA_free(local_rsa);
  return res;
}

#if !defined(OPENSSL_NO_DSA)
static int dsa_compare_keys(pool *p, EVP_PKEY *remote_pkey,
    EVP_PKEY *local_pkey) {
  DSA *remote_dsa = NULL, *local_dsa = NULL;
  const BIGNUM *remote_dsa_p, *remote_dsa_q, *remote_dsa_g;
  const BIGNUM *local_dsa_p, *local_dsa_q, *local_dsa_g;
  const BIGNUM *remote_dsa_pub_key, *local_dsa_pub_key;
  int res = 0;

  local_dsa = EVP_PKEY_get1_DSA(local_pkey);
  if (keys_dsa_min_nbits > 0) {
    int dsa_nbits;

    dsa_nbits = DSA_size(local_dsa) * 8;
    if (dsa_nbits < keys_dsa_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "local DSA key size (%d bits) less than required minimum (%d bits)",
        dsa_nbits, keys_dsa_min_nbits);
      DSA_free(local_dsa);
      errno = EPERM;
      return -1;
    }

    pr_trace_msg(trace_channel, 19,
      "comparing DSA keys using local DSA key (%d bits)", dsa_nbits);
  }

  remote_dsa = EVP_PKEY_get1_DSA(remote_pkey);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  DSA_get0_pqg(remote_dsa, &remote_dsa_p, &remote_dsa_q, &remote_dsa_g);
  DSA_get0_pqg(local_dsa, &local_dsa_p, &local_dsa_q, &local_dsa_g);
  DSA_get0_key(remote_dsa, &remote_dsa_pub_key, NULL);
  DSA_get0_key(local_dsa, &local_dsa_pub_key, NULL);
#else
  remote_dsa_p = remote_dsa->p;
  remote_dsa_q = remote_dsa->q;
  remote_dsa_g = remote_dsa->g;
  remote_dsa_pub_key = remote_dsa->pub_key;
  local_dsa_p = local_dsa->p;
  local_dsa_q = local_dsa->q;
  local_dsa_g = local_dsa->g;
  local_dsa_pub_key = local_dsa->pub_key;
#endif /* prior to OpenSSL-1.1.0 */

  if (BN_cmp(remote_dsa_p, local_dsa_p) != 0) {
    pr_trace_msg(trace_channel, 17, "%s",
      "DSA key mismatch: client-sent DSA key parameter 'p' does not match "
      "local DSA key parameter 'p'");
    res = -1;
  }

  if (res == 0) {
    if (BN_cmp(remote_dsa_q, local_dsa_q) != 0) {
      pr_trace_msg(trace_channel, 17, "%s",
        "DSA key mismatch: client-sent DSA key parameter 'q' does not match "
        "local DSA key parameter 'q'");
      res = -1;
    }
  }

  if (res == 0) {
    if (BN_cmp(remote_dsa_g, local_dsa_g) != 0) {
      pr_trace_msg(trace_channel, 17, "%s",
        "DSA key mismatch: client-sent DSA key parameter 'g' does not match "
        "local DSA key parameter 'g'");
      res = -1;
    }
  }

  if (res == 0) {
    if (BN_cmp(remote_dsa_pub_key, local_dsa_pub_key) != 0) {
      pr_trace_msg(trace_channel, 17, "%s",
        "DSA key mismatch: client-sent DSA key parameter 'pub_key' does not "
        "match local DSA key parameter 'pub_key'");
      res = -1;
    }
  }

  DSA_free(remote_dsa);
  DSA_free(local_dsa);
  return res;
}
#endif /* OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
static int ecdsa_compare_keys(pool *p, EVP_PKEY *remote_pkey,
    EVP_PKEY *local_pkey) {
  EC_KEY *remote_ec, *local_ec;
  int res = 0;

  local_ec = EVP_PKEY_get1_EC_KEY(local_pkey);
  if (keys_ec_min_nbits > 0) {
    int ec_nbits;

    ec_nbits = EVP_PKEY_bits(local_pkey) * 8;
    if (ec_nbits < keys_ec_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "local EC key size (%d bits) less than required minimum (%d bits)",
        ec_nbits, keys_ec_min_nbits);
      EC_KEY_free(local_ec);
      errno = EPERM;
      return -1;
    }

    pr_trace_msg(trace_channel, 19,
      "comparing EC keys using local EC key (%d bits)", ec_nbits);
  }

  remote_ec = EVP_PKEY_get1_EC_KEY(remote_pkey);

  if (EC_GROUP_cmp(EC_KEY_get0_group(local_ec),
      EC_KEY_get0_group(remote_ec), NULL) != 0) {
    pr_trace_msg(trace_channel, 17, "%s",
      "ECC key mismatch: client-sent curve does not match local ECC curve");
    res = -1;
  }

  if (res == 0) {
    if (EC_POINT_cmp(EC_KEY_get0_group(local_ec),
        EC_KEY_get0_public_key(local_ec),
        EC_KEY_get0_public_key(remote_ec), NULL) != 0) {
      pr_trace_msg(trace_channel, 17, "%s",
        "ECC key mismatch: client-sent public key 'Q' does not match "
        "local ECC public key 'Q'");
      res = -1;
    }
  }

  EC_KEY_free(remote_ec);
  EC_KEY_free(local_ec);
  return res;
}
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
static int ed25519_compare_keys(pool *p,
    unsigned char *remote_pubkey_data, uint32_t remote_pubkey_datalen,
    unsigned char *local_pubkey_data, uint32_t local_pubkey_datalen) {
  int res = 0;

  if (remote_pubkey_datalen != local_pubkey_datalen) {
    return -1;
  }

  if (memcmp(remote_pubkey_data, local_pubkey_data, remote_pubkey_datalen) != 0) {
    res = -1;
  }

  return res;
}
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
static int ed448_compare_keys(pool *p,
    unsigned char *remote_pubkey_data, uint32_t remote_pubkey_datalen,
    unsigned char *local_pubkey_data, uint32_t local_pubkey_datalen) {
  int res = 0;

  if (remote_pubkey_datalen != local_pubkey_datalen) {
    return -1;
  }

  if (memcmp(remote_pubkey_data, local_pubkey_data, remote_pubkey_datalen) != 0) {
    res = -1;
  }

  return res;
}
#endif /* HAVE_X448_OPENSSL */

/* Compare a "blob" of pubkey data sent by the client for authentication
 * with a local file pubkey (from an RFC4716 formatted file).  Returns -1 if
 * there was an error, TRUE if the keys are equals, and FALSE if not.
 */
int sftp_keys_compare_keys(pool *p,
    unsigned char *remote_pubkey_data, uint32_t remote_pubkey_datalen,
    unsigned char *local_pubkey_data, uint32_t local_pubkey_datalen) {
  enum sftp_key_type_e remote_key_type, local_key_type;
  EVP_PKEY *remote_pkey = NULL, *local_pkey = NULL;
  int res = -1;
  uint32_t len = 0;

  if (remote_pubkey_data == NULL ||
      local_pubkey_data == NULL) {
    errno = EINVAL;
    return -1;
  }

  remote_key_type = local_key_type = SFTP_KEY_UNKNOWN;

  len = read_pkey_from_data(p, remote_pubkey_data, remote_pubkey_datalen,
    &remote_pkey, &remote_key_type, FALSE);
  if (len == 0) {
    return -1;
  }

  len = read_pkey_from_data(p, local_pubkey_data, local_pubkey_datalen,
    &local_pkey, &local_key_type, FALSE);
  if (len == 0) {
    int xerrno = errno;

    if (remote_pkey != NULL) {
      EVP_PKEY_free(remote_pkey);
    }

    errno = xerrno;
    return -1;
  }

  if (remote_pkey != NULL &&
      local_pkey != NULL &&
      remote_key_type == local_key_type) {
    switch (get_pkey_type(remote_pkey)) {
      case EVP_PKEY_RSA: {
        if (rsa_compare_keys(p, remote_pkey, local_pkey) == 0) {
          res = TRUE;

        } else {
          res = FALSE;
        }

        break;
      }

#if !defined(OPENSSL_NO_DSA)
      case EVP_PKEY_DSA: {
        if (dsa_compare_keys(p, remote_pkey, local_pkey) == 0) {
          res = TRUE;

        } else {
          res = FALSE;
        }

        break;
      }
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
      case EVP_PKEY_EC: {
        if (ecdsa_compare_keys(p, remote_pkey, local_pkey) == 0) {
          res = TRUE;

        } else {
          res = FALSE;
        }

        break;
      }
#endif /* PR_USE_OPENSSL_ECC */

      default:
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "unable to compare %s keys: unsupported key type",
          get_pkey_type_desc(get_pkey_type(remote_pkey)));
        errno = ENOSYS;
        break;
    }

  } else if (remote_key_type == SFTP_KEY_ED25519 &&
             remote_key_type == local_key_type) {
#if defined(PR_USE_SODIUM)
    if (ed25519_compare_keys(p, remote_pubkey_data, remote_pubkey_datalen,
        local_pubkey_data, local_pubkey_datalen) == 0) {
      res = TRUE;

    } else {
      res = FALSE;
    }
#endif /* PR_USE_SODIUM */

  } else if (remote_key_type == SFTP_KEY_ED448 &&
             remote_key_type == local_key_type) {
#if defined(HAVE_X448_OPENSSL)
    if (ed448_compare_keys(p, remote_pubkey_data, remote_pubkey_datalen,
        local_pubkey_data, local_pubkey_datalen) == 0) {
      res = TRUE;

    } else {
      res = FALSE;
    }
#endif /* HAVE_X448_OPENSSL */

  } else {
    if (pr_trace_get_level(trace_channel) >= 17) {
      const char *remote_key_desc, *local_key_desc;

      remote_key_desc = get_key_type_desc(remote_key_type);
      local_key_desc = get_key_type_desc(local_key_type);

      pr_trace_msg(trace_channel, 17, "key mismatch: cannot compare %s key "
        "(client-sent) with %s key (local)", remote_key_desc, local_key_desc);
    }

    res = FALSE;
  }

  if (remote_pkey != NULL) {
    EVP_PKEY_free(remote_pkey);
  }

  if (local_pkey != NULL) {
    EVP_PKEY_free(local_pkey);
  }

  return res;
}

const char *sftp_keys_get_fingerprint(pool *p, unsigned char *key_data,
    uint32_t key_datalen, int digest_algo) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_MD_CTX ctx;
#endif /* prior to OpenSSL-1.1.0 */
  EVP_MD_CTX *pctx;
  const EVP_MD *digest;
  char *digest_name = "none", *fp;
  unsigned char *fp_data;
  unsigned int fp_datalen = 0;
  register unsigned int i;

  switch (digest_algo) {
    case SFTP_KEYS_FP_DIGEST_MD5:
      digest = EVP_md5();
      digest_name = "md5";
      break;

    case SFTP_KEYS_FP_DIGEST_SHA1:
      digest = EVP_sha1();
      digest_name = "sha1";
      break;

#if defined(HAVE_SHA256_OPENSSL)
    case SFTP_KEYS_FP_DIGEST_SHA256:
      digest = EVP_sha256();
      digest_name = "sha256";
      break;
#endif /* HAVE_SHA256_OPENSSL */

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unsupported key fingerprint digest algorithm (%d)", digest_algo);
      errno = EACCES;
      return NULL;
  }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  pctx = EVP_MD_CTX_new();
#else
  pctx = &ctx;
#endif /* prior to OpenSSL-1.1.0 */

  /* In OpenSSL 0.9.6, many of the EVP_Digest* functions returned void, not
   * int.  Without these ugly OpenSSL version preprocessor checks, the
   * compiler will error out with "void value not ignored as it ought to be".
   */

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestInit(pctx, digest) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing %s digest: %s", digest_name,
      sftp_crypto_get_errors());
# if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
     !defined(HAVE_LIBRESSL)
    EVP_MD_CTX_free(pctx);
# endif /* OpenSSL-1.1.0 and later */
    errno = EPERM;
    return NULL;
  }
#else
  EVP_DigestInit(pctx, digest);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestUpdate(pctx, key_data, key_datalen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error updating %s digest: %s", digest_name, sftp_crypto_get_errors());
# if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
     !defined(HAVE_LIBRESSL)
    EVP_MD_CTX_free(pctx);
# endif /* OpenSSL-1.1.0 and later */
    errno = EPERM;
    return NULL;
  }
#else
  EVP_DigestUpdate(pctx, key_data, key_datalen);
#endif

  fp_data = palloc(p, EVP_MAX_MD_SIZE);

#if OPENSSL_VERSION_NUMBER >= 0x000907000L
  if (EVP_DigestFinal(pctx, fp_data, &fp_datalen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error finishing %s digest: %s", digest_name, sftp_crypto_get_errors());
# if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
     !defined(HAVE_LIBRESSL)
    EVP_MD_CTX_free(pctx);
# endif /* OpenSSL-1.1.0 and later */
    errno = EPERM;
    return NULL;
  }
#else
  EVP_DigestFinal(pctx, fp_data, &fp_datalen);
#endif

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  EVP_MD_CTX_free(pctx);
#endif /* OpenSSL-1.1.0 and later */

  /* Now encode that digest in fp_data as hex characters. */
  fp = "";

  for (i = 0; i < fp_datalen; i++) {
    char c[4];

    memset(c, '\0', sizeof(c));
    pr_snprintf(c, sizeof(c), "%02x:", fp_data[i]);
    fp = pstrcat(p, fp, &c, NULL);
  }
  fp[strlen(fp)-1] = '\0';

  return fp;
}

#if defined(PR_USE_OPENSSL_ECC)
/* Returns the NID for the configured EVP_PKEY_EC key. */
static int get_ecdsa_nid(EC_KEY *ec) {
  register unsigned int i;
  const EC_GROUP *key_group;
  EC_GROUP *new_group = NULL;
  BN_CTX *bn_ctx = NULL;
  int supported_ecdsa_nids[] = {
    NID_X9_62_prime256v1,
    NID_secp384r1,
    NID_secp521r1,
    -1
  };
  int nid;

  if (ec == NULL) {
    errno = EINVAL;
    return -1;
  }

  /* Since the EC group might be encoded in different ways, we need to try
   * different lookups to find the NID.
   *
   * First, we see if the EC group is encoded as a "named group" in the
   * private key.
   */
  key_group = EC_KEY_get0_group(ec);
  nid = EC_GROUP_get_curve_name(key_group);
  if (nid > 0) {
    return nid;
  }

  /* Otherwise, we check to see if the group is encoded via explicit group
   * parameters in the private key.
   */

  bn_ctx = BN_CTX_new();
  if (bn_ctx == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocated BN_CTX: %s", sftp_crypto_get_errors());
    return -1;
  }

  for (i = 0; supported_ecdsa_nids[i] != -1; i++) {
    new_group = EC_GROUP_new_by_curve_name(supported_ecdsa_nids[i]);
    if (new_group == NULL) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error creating new EC_GROUP by curve name %d: %s",
        supported_ecdsa_nids[i], sftp_crypto_get_errors());
      BN_CTX_free(bn_ctx);
      return -1;
    }

    if (EC_GROUP_cmp(key_group, new_group, bn_ctx) == 0) {
      /* We have a match. */
      break;
    }

    EC_GROUP_free(new_group);
    new_group = NULL;
  }

  BN_CTX_free(bn_ctx);

  if (supported_ecdsa_nids[i] != -1) {
    EC_GROUP_set_asn1_flag(new_group, OPENSSL_EC_NAMED_CURVE);
    if (EC_KEY_set_group(ec, new_group) != 1) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error setting EC group on key: %s", sftp_crypto_get_errors());
      EC_GROUP_free(new_group);
      return -1;
    }

    EC_GROUP_free(new_group);
  }

  return supported_ecdsa_nids[i];
}
#endif /* PR_USE_OPENSSL_ECC */

static int handle_hostkey(pool *p, EVP_PKEY *pkey,
    const unsigned char *key_data, uint32_t key_datalen,
    const char *file_path, const char *agent_path) {

  switch (get_pkey_type(pkey)) {
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
            "error enabling RSA blinding for key '%s': %s",
            file_path ? file_path : agent_path,
            sftp_crypto_get_errors());

        } else {
          (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
            "RSA blinding enabled for key '%s'",
            file_path ? file_path : agent_path);
        }

        RSA_free(rsa);
      }
#endif

      if (sftp_rsa_hostkey != NULL) {
        /* If we have an existing RSA hostkey, free it up. */
        EVP_PKEY_free(sftp_rsa_hostkey->pkey);
        sftp_rsa_hostkey->pkey = NULL;
        sftp_rsa_hostkey->key_data = NULL;
        sftp_rsa_hostkey->key_datalen = 0;
        sftp_rsa_hostkey->file_path = NULL;
        sftp_rsa_hostkey->agent_path = NULL;

      } else {
        sftp_rsa_hostkey = pcalloc(p, sizeof(struct sftp_hostkey));
      }

      sftp_rsa_hostkey->key_type = SFTP_KEY_RSA;
      sftp_rsa_hostkey->pkey = pkey;
      sftp_rsa_hostkey->key_data = key_data;
      sftp_rsa_hostkey->key_datalen = key_datalen;
      sftp_rsa_hostkey->file_path = file_path;
      sftp_rsa_hostkey->agent_path = agent_path;

      if (file_path != NULL) {
        pr_trace_msg(trace_channel, 4, "using '%s' as RSA hostkey (%d bits)",
          file_path, EVP_PKEY_bits(pkey));

      } else if (agent_path != NULL) {
        pr_trace_msg(trace_channel, 4,
          "using RSA hostkey (%d bits) from SSH agent at '%s'",
          EVP_PKEY_bits(pkey), agent_path);
      }

      break;
    }

    case EVP_PKEY_DSA: {
      if (sftp_dsa_hostkey != NULL) {
        /* If we have an existing DSA hostkey, free it up. */
        EVP_PKEY_free(sftp_dsa_hostkey->pkey);
        sftp_dsa_hostkey->pkey = NULL;
        sftp_dsa_hostkey->key_data = NULL;
        sftp_dsa_hostkey->key_datalen = 0;
        sftp_dsa_hostkey->file_path = NULL;
        sftp_dsa_hostkey->agent_path = NULL;

      } else {
        sftp_dsa_hostkey = pcalloc(p, sizeof(struct sftp_hostkey));
      }

      sftp_dsa_hostkey->key_type = SFTP_KEY_DSA;
      sftp_dsa_hostkey->pkey = pkey;
      sftp_dsa_hostkey->key_data = key_data;
      sftp_dsa_hostkey->key_datalen = key_datalen;
      sftp_dsa_hostkey->file_path = file_path;
      sftp_dsa_hostkey->agent_path = agent_path;

      if (file_path != NULL) {
        pr_trace_msg(trace_channel, 4, "using '%s' as DSA hostkey (%d bits)",
          file_path, EVP_PKEY_bits(pkey));

      } else if (agent_path != NULL) {
        pr_trace_msg(trace_channel, 4,
          "using DSA hostkey (%d bits) from SSH agent at '%s'",
          EVP_PKEY_bits(pkey), agent_path);
      }

      break;
    }

#if defined(PR_USE_OPENSSL_ECC)
    case EVP_PKEY_EC: {
      EC_KEY *ec;
      int ec_nid;

      ec = EVP_PKEY_get1_EC_KEY(pkey);
      ec_nid = get_ecdsa_nid(ec);
      if (ec_nid < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "unsupported NID in EC key, ignoring");
        EC_KEY_free(ec);
        EVP_PKEY_free(pkey);
        return -1;
      }

      if (sftp_keys_validate_ecdsa_params(EC_KEY_get0_group(ec),
          EC_KEY_get0_public_key(ec)) < 0) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error validating EC public key: %s", strerror(errno));
        EC_KEY_free(ec);
        EVP_PKEY_free(pkey);
        return -1;
      }

      if (validate_ecdsa_private_key(ec)) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error validating EC private key: %s", strerror(errno));
        EC_KEY_free(ec);
        EVP_PKEY_free(pkey);
        return -1;
      }

      EC_KEY_free(ec);

      switch (ec_nid) {
        case NID_X9_62_prime256v1:
          if (sftp_ecdsa256_hostkey != NULL) {
            /* If we have an existing 256-bit ECDSA hostkey, free it up. */
            EVP_PKEY_free(sftp_ecdsa256_hostkey->pkey);
            sftp_ecdsa256_hostkey->pkey = NULL;
            sftp_ecdsa256_hostkey->key_data = NULL;
            sftp_ecdsa256_hostkey->key_datalen = 0;
            sftp_ecdsa256_hostkey->file_path = NULL;
            sftp_ecdsa256_hostkey->agent_path = NULL;

          } else {
            sftp_ecdsa256_hostkey = pcalloc(p, sizeof(struct sftp_hostkey));
          }

          sftp_ecdsa256_hostkey->key_type = SFTP_KEY_ECDSA_256;
          sftp_ecdsa256_hostkey->pkey = pkey;
          sftp_ecdsa256_hostkey->key_data = key_data;
          sftp_ecdsa256_hostkey->key_datalen = key_datalen;
          sftp_ecdsa256_hostkey->file_path = file_path;
          sftp_ecdsa256_hostkey->agent_path = agent_path;

          if (file_path != NULL) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "using '%s' as 256-bit ECDSA hostkey (%d bits)", file_path,
              EVP_PKEY_bits(pkey));

          } else if (agent_path != NULL) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "using 256-bit ECDSA hostkey (%d bits) from SSH agent at '%s'",
              EVP_PKEY_bits(pkey), agent_path);
          }

          break;

        case NID_secp384r1:
          if (sftp_ecdsa384_hostkey != NULL) {
            /* If we have an existing 384-bit ECDSA hostkey, free it up. */
            EVP_PKEY_free(sftp_ecdsa384_hostkey->pkey);
            sftp_ecdsa384_hostkey->pkey = NULL;
            sftp_ecdsa384_hostkey->key_data = NULL;
            sftp_ecdsa384_hostkey->key_datalen = 0;
            sftp_ecdsa384_hostkey->file_path = NULL;
            sftp_ecdsa384_hostkey->agent_path = NULL;
          
          } else {
            sftp_ecdsa384_hostkey = pcalloc(p, sizeof(struct sftp_hostkey));
          } 
          
          sftp_ecdsa384_hostkey->key_type = SFTP_KEY_ECDSA_384;
          sftp_ecdsa384_hostkey->pkey = pkey;
          sftp_ecdsa384_hostkey->key_data = key_data;
          sftp_ecdsa384_hostkey->key_datalen = key_datalen;
          sftp_ecdsa384_hostkey->file_path = file_path;
          sftp_ecdsa384_hostkey->agent_path = agent_path;

          if (file_path != NULL) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "using '%s' as 384-bit ECDSA hostkey (%d bits)", file_path,
              EVP_PKEY_bits(pkey));

          } else if (agent_path != NULL) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "using 384-bit ECDSA hostkey (%d bits) from SSH agent at '%s'",
              EVP_PKEY_bits(pkey), agent_path);
          }

          break;

        case NID_secp521r1:
          if (sftp_ecdsa521_hostkey != NULL) {
            /* If we have an existing 521-bit ECDSA hostkey, free it up. */
            EVP_PKEY_free(sftp_ecdsa521_hostkey->pkey);
            sftp_ecdsa521_hostkey->pkey = NULL;
            sftp_ecdsa521_hostkey->key_data = NULL;
            sftp_ecdsa521_hostkey->key_datalen = 0;
            sftp_ecdsa521_hostkey->file_path = NULL;
            sftp_ecdsa521_hostkey->agent_path = NULL;
          
          } else {
            sftp_ecdsa521_hostkey = pcalloc(p, sizeof(struct sftp_hostkey));
          } 
          
          sftp_ecdsa521_hostkey->key_type = SFTP_KEY_ECDSA_521;
          sftp_ecdsa521_hostkey->pkey = pkey;
          sftp_ecdsa521_hostkey->key_data = key_data;
          sftp_ecdsa521_hostkey->key_datalen = key_datalen;
          sftp_ecdsa521_hostkey->file_path = file_path;
          sftp_ecdsa521_hostkey->agent_path = agent_path;

          if (file_path != NULL) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "using '%s' as 521-bit ECDSA hostkey (%d bits)", file_path,
              EVP_PKEY_bits(pkey));

          } else if (agent_path != NULL) {
            (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
              "using 521-bit hostkey (%d bits) from SSH agent at '%s'",
              EVP_PKEY_bits(pkey), agent_path);
          }

          break;
      }

      break;
    }
#endif /* PR_USE_OPENSSL_ECC */

#if defined(HAVE_X448_OPENSSL)
    case EVP_PKEY_ED448: {
      unsigned char *privkey_data;
      size_t privkey_datalen;

      privkey_datalen = (CURVE448_SIZE * 2);
      privkey_data = palloc(p, privkey_datalen);
      if (EVP_PKEY_get_raw_private_key(pkey, privkey_data,
          &privkey_datalen) != 1) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error reading ED448 private key from '%s': %s", file_path,
          sftp_crypto_get_errors());
        EVP_PKEY_free(pkey);
        return -1;
      }

      if (handle_ed448_hostkey(p, privkey_data, privkey_datalen,
          file_path) < 0) {
        EVP_PKEY_free(pkey);
        return -1;
      }
      break;
    }
#endif /* HAVE_X448_OPENSSL */

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unknown private key type (%d), ignoring", get_pkey_type(pkey));
      EVP_PKEY_free(pkey);
      return -1;
  }

  return 0;
}

static int load_agent_hostkeys(pool *p, const char *path) {
  register unsigned int i;
  int accepted_nkeys = 0, res;
  array_header *key_list;

  key_list = make_array(p, 0, sizeof(struct agent_key *));  

  res = sftp_agent_get_keys(p, path, key_list);
  if (res < 0) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error loading hostkeys from SSH agent at '%s': %s", path,
      strerror(xerrno));

    errno = xerrno;
    return -1;
  }

  if (key_list->nelts == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SSH agent at '%s' returned no keys", path);
    errno = ENOENT;
    return -1;
  }

  pr_trace_msg(trace_channel, 9, "processing %d keys from SSH agent at '%s'",
    key_list->nelts, path);

  for (i = 0; i < key_list->nelts; i++) {
    EVP_PKEY *pkey;
    uint32_t len;
    struct agent_key *agent_key;

    agent_key = ((struct agent_key **) key_list->elts)[i];

    len = read_pkey_from_data(p, agent_key->key_data, agent_key->key_datalen,
      &pkey, NULL, FALSE);
    if (len == 0) {
      continue;
    }

    if (handle_hostkey(p, pkey, agent_key->key_data, agent_key->key_datalen,
        NULL, path) == 0) {
      accepted_nkeys++;
    }
  }

  if (accepted_nkeys == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "none of the keys provided by the SSH agent at '%s' were acceptable",
      path);
    errno = EINVAL;
    return -1;
  }

  pr_trace_msg(trace_channel, 9, "loaded %d keys from SSH agent at '%s'",
    accepted_nkeys, path);

  /* Return the number of keys we successfully accept from the agent. */
  return accepted_nkeys;
}

static struct openssh_cipher *get_openssh_cipher(const char *name) {
  register unsigned int i;
  struct openssh_cipher *cipher = NULL;

  for (i = 0; ciphers[i].algo != NULL; i++) {
    if (strcmp(ciphers[i].algo, name) == 0) {
      cipher = &ciphers[i];
      break;
    }
  }

  if (cipher == NULL) {
    errno = ENOENT;
    return NULL;
  }

  if (cipher->get_cipher != NULL) {
    cipher->cipher = (cipher->get_cipher)();
    if (cipher->cipher != NULL) {
      return cipher;
    }
  }

  /* The CTR algorithms may require our own implementation, not the OpenSSL
   * implementation.
   */

  cipher->cipher = sftp_crypto_get_cipher(name, NULL, NULL, NULL);
  if (cipher->cipher == NULL) {
    errno = ENOSYS;
    return NULL;
  }

  return cipher;
}

static int decrypt_openssh_data(pool *p, const char *path,
    unsigned char *encrypted_data, uint32_t encrypted_len,
    const char *passphrase, struct openssh_cipher *cipher,
    const char *kdf_name, unsigned char *kdf_data, uint32_t kdf_len,
    unsigned char **decrypted_data, uint32_t *decrypted_len) {
  EVP_CIPHER_CTX *cipher_ctx = NULL;
  unsigned char *buf, *key, *iv, *salt_data;
  uint32_t buflen, key_len, rounds, salt_len;
  size_t passphrase_len;

  if (strcmp(kdf_name, "none") == 0) {
    *decrypted_data = encrypted_data;
    *decrypted_len = encrypted_len;

    return 0;
  }

  if (strcmp(kdf_name, "bcrypt") != 0) {
    pr_trace_msg(trace_channel, 3,
      "'%s' key uses unsupported %s KDF", path, kdf_name);
    errno = ENOSYS;
    return -1;
  }

  salt_len = sftp_msg_read_int(p, &kdf_data, &kdf_len);
  salt_data = sftp_msg_read_data(p, &kdf_data, &kdf_len, salt_len);
  rounds = sftp_msg_read_int(p, &kdf_data, &kdf_len);

  pr_trace_msg(trace_channel, 9,
    "'%s' key %s KDF using %lu bytes of salt, %lu rounds", path,
    kdf_name, (unsigned long) salt_len, (unsigned long) rounds);

  /* Compute the decryption key using the KDF and the passphrase.  Note that
   * we derive the key AND the IV using this approach at the same time.
   */
  passphrase_len = strlen(passphrase);
  key_len = cipher->key_len + cipher->iv_len;

  pr_trace_msg(trace_channel, 13,
    "generating %s decryption key using %s KDF (key len = %lu, IV len = %lu)",
    cipher->algo, kdf_name, (unsigned long) cipher->key_len,
    (unsigned long) cipher->iv_len);
  key = pcalloc(p, key_len);
  if (sftp_bcrypt_pbkdf2(p, passphrase, passphrase_len, salt_data, salt_len,
      rounds, key, key_len) < 0) {
    pr_trace_msg(trace_channel, 3,
      "error computing key using %s KDF: %s", kdf_name, strerror(errno));
    errno = EPERM;
    return -1;
  }

  if (cipher->iv_len > 0) {
    iv = key + cipher->key_len;

  } else {
    iv = NULL;
  }

  cipher_ctx = EVP_CIPHER_CTX_new();
  if (cipher_ctx == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocating cipher context: %s", sftp_crypto_get_errors());
    errno = EPERM;
    return -1;
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_CIPHER_CTX_init(cipher_ctx);
#endif

#if defined(PR_USE_OPENSSL_EVP_CIPHERINIT_EX)
  if (EVP_CipherInit_ex(cipher_ctx, cipher->cipher, NULL, key, iv, 0) != 1) {
#else
  if (EVP_CipherInit(cipher_ctx, cipher->cipher, key, iv, 0) != 1) {
#endif /* PR_USE_OPENSSL_EVP_CIPHERINIT_EX */
    pr_trace_msg(trace_channel, 3,
      "error initializing %s cipher for decryption: %s", cipher->algo,
      sftp_crypto_get_errors());
    EVP_CIPHER_CTX_free(cipher_ctx);
    pr_memscrub(key, key_len);
    errno = EPERM;
    return -1;
  }

  if (cipher->key_len > 0) {
    if (EVP_CIPHER_CTX_set_key_length(cipher_ctx, cipher->key_len) != 1) {
      pr_trace_msg(trace_channel, 3,
        "error setting key length (%lu bytes) for %s cipher for decryption: %s",
        (unsigned long) cipher->key_len, cipher->algo,
        sftp_crypto_get_errors());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
      EVP_CIPHER_CTX_cleanup(cipher_ctx);
#endif /* prior to OpenSSL-1.1.0 */
      EVP_CIPHER_CTX_free(cipher_ctx);
      pr_memscrub(key, key_len);
      errno = EPERM;
      return -1;
    }
  }

  buflen = encrypted_len;
  buf = pcalloc(p, buflen);

  /* TODO: this currently works because our data does NOT contain any extra
   * trailing AEAD bytes.  Need to fix that in the future.
   */

  if (EVP_Cipher(cipher_ctx, buf, encrypted_data, encrypted_len) < 0) {
    /* This might happen due to a wrong/bad passphrase. */
    pr_trace_msg(trace_channel, 3,
      "error decrypting %s data for key: %s", cipher->algo,
      sftp_crypto_get_errors());
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX_cleanup(cipher_ctx);
#endif /* prior to OpenSSL-1.1.0 */
    EVP_CIPHER_CTX_free(cipher_ctx);
    pr_memscrub(key, key_len);
    pr_memscrub(buf, buflen);
    errno = EPERM;
    return -1;
  }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  EVP_CIPHER_CTX_cleanup(cipher_ctx);
#endif /* prior to OpenSSL-1.1.0 */
  EVP_CIPHER_CTX_free(cipher_ctx);
  pr_memscrub(key, key_len);

  *decrypted_data = buf;
  *decrypted_len = buflen;
  return 0;
}

/* See openssh-7.9p1/sshkey.c#sshkey_from_blob_internal(). */
static int deserialize_openssh_private_key(pool *p, const char *path,
    unsigned char **data, uint32_t *data_len, enum sftp_key_type_e *key_type,
    EVP_PKEY **pkey, unsigned char **key, uint32_t *keylen) {
  uint32_t len = 0;
  const char *pkey_type;
  uint32_t public_keylen = 0, secret_keylen = 0;
  unsigned char *public_key = NULL, *secret_key = NULL;
  int have_extra_public_key = FALSE;

  len = read_pkey_from_data(p, *data, *data_len, pkey, key_type, TRUE);
  if (len == 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unsupported key type %d found in '%s'", *key_type, path);
    errno = EPERM;
    return -1;
  }

  /* Advance our pointers for all of the data read from them. */
  (*data) += len;
  (*data_len) -= len;

  switch (*key_type) {
    case SFTP_KEY_DSA:
    case SFTP_KEY_RSA:
    case SFTP_KEY_ECDSA_256:
    case SFTP_KEY_ECDSA_384:
    case SFTP_KEY_ECDSA_521:
    case SFTP_KEY_RSA_SHA256:
    case SFTP_KEY_RSA_SHA512:
      return 0;

    case SFTP_KEY_ED25519:
      pkey_type = "ssh-ed25519";
      break;

    case SFTP_KEY_ED448:
      pkey_type = "ssh-ed448";
      break;

    case SFTP_KEY_UNKNOWN:
    default:
      *key = NULL;
      *keylen = 0;
      return 0;
  }

  public_keylen = sftp_msg_read_int(p, data, data_len);
  public_key = sftp_msg_read_data(p, data, data_len, public_keylen);
  if (public_key == NULL) {
    pr_trace_msg(trace_channel, 2,
      "error reading %s key: invalid/supported key format", pkey_type);
    errno = EINVAL;
    return -1;
  }

  secret_keylen = sftp_msg_read_int(p, data, data_len);

  /* NOTE: PuTTY's puttygen adds the public key _again_, in the second half
   * of the secret key data, per commments in its
   * `sshecc.c#eddsa_new_priv_openssh` function.  Thus if this secret key
   * length is larger than expected for Ed448 keys, only use the first half of
   * it.  Ugh.  This "divide in half" hack only works for these keys where the
   * private and public key sizes are the same.
   */
  switch (*key_type) {
    case SFTP_KEY_ED448:
#if defined(HAVE_X448_OPENSSL) && defined(HAVE_SHA512_OPENSSL)
      if (secret_keylen > (CURVE448_SIZE + 1)) {
        have_extra_public_key = TRUE;
        secret_keylen /= 2;
      }
#endif /* HAVE_X448_OPENSSL and HAVE_SHA512_OPENSSL */
      break;

    default:
      break;
  }

  secret_key = sftp_msg_read_data(p, data, data_len, secret_keylen);
  if (secret_key == NULL) {
    pr_trace_msg(trace_channel, 2,
      "error reading %s key: invalid/supported key format", pkey_type);
    errno = EINVAL;
    return -1;
  }

  if (have_extra_public_key == TRUE) {
    /* Read (and ignore) the rest of the secret data */
    (void) sftp_msg_read_data(p, data, data_len, secret_keylen);
  }

  /* The secret key is what we need to extract. */
  *key = secret_key;
  *keylen = secret_keylen;

  return 0;
}

static int decrypt_openssh_private_key(pool *p, const char *path,
    unsigned char *encrypted_data, uint32_t encrypted_len,
    const char *passphrase, struct openssh_cipher *cipher,
    const char *kdf_name, unsigned char *kdf_data, uint32_t kdf_len,
    enum sftp_key_type_e *key_type, EVP_PKEY **pkey, unsigned char **key,
    uint32_t *keylen) {
  unsigned char *decrypted_data = NULL, *decrypted_ptr = NULL;
  uint32_t check_bytes[2], decrypted_len = 0, decrypted_sz = 0;
  char *comment = NULL;
  int res;
  unsigned int i = 0;

  res = decrypt_openssh_data(p, path, encrypted_data, encrypted_len, passphrase,
    cipher, kdf_name, kdf_data, kdf_len, &decrypted_data, &decrypted_len);
  if (res < 0) {
    pr_trace_msg(trace_channel, 6,
      "failed to decrypt '%s' using %s cipher: %s", path, cipher->algo,
      strerror(errno));
    errno = EINVAL;
    return -1;
  }

  pr_trace_msg(trace_channel, 14,
    "decrypted %lu bytes into %lu bytes", (unsigned long) encrypted_len,
    (unsigned long) decrypted_len);

  decrypted_ptr = decrypted_data;
  decrypted_sz = decrypted_len;

  check_bytes[0] = sftp_msg_read_int(p, &decrypted_data, &decrypted_len);
  check_bytes[1] = sftp_msg_read_int(p, &decrypted_data, &decrypted_len);

  if (check_bytes[0] != check_bytes[1]) {
    pr_trace_msg(trace_channel, 6,
      "'%s' has mismatched check bytes (%lu != %lu); wrong passphrase", path,
      (unsigned long) check_bytes[0], (unsigned long) check_bytes[1]);
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to read hostkey '%s': wrong passphrase", path);

    pr_memscrub(decrypted_ptr, decrypted_sz);
    errno = EINVAL;
    return -1;
  }

  res = deserialize_openssh_private_key(p, path, &decrypted_data,
    &decrypted_len, key_type, pkey, key, keylen);
  if (res < 0) {
    int xerrno = errno;

    pr_memscrub(decrypted_ptr, decrypted_sz);
    errno = xerrno;
    return -1;
  }

  comment = sftp_msg_read_string(p, &decrypted_data, &decrypted_len);
  if (comment != NULL) {
    pr_trace_msg(trace_channel, 9,
      "'%s' comment: '%s'", path, comment);
  }

  /* Verify the expected remaining padding. */
  for (i = 1; decrypted_len > 0; i++) {
    char padding;

    pr_signals_handle();

    padding = sftp_msg_read_byte(p, &decrypted_data, &decrypted_len);
    if (((unsigned int) padding) != (i & 0xFF)) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "'%s' key has invalid padding", path);
      pr_memscrub(decrypted_ptr, decrypted_sz);
      errno = EINVAL;
      return -1;
    }
  }

  pr_memscrub(decrypted_ptr, decrypted_sz);
  return 0;
}

static int unwrap_openssh_private_key(pool *p, const char *path,
    unsigned char *text, size_t text_len, const char *passphrase,
    enum sftp_key_type_e *key_type, EVP_PKEY **pkey, unsigned char **key,
    uint32_t *keylen) {
  char *cipher_name, *kdf_name;
  unsigned char *buf, *data = NULL, *kdf_data, *encrypted_data;
  size_t data_len = 0, magicsz;
  uint32_t buflen, kdf_len = 0, key_count = 0, encrypted_len = 0;
  struct openssh_cipher *cipher = NULL;
  int xerrno = 0;

  data = decode_base64(p, text, text_len, &data_len);
  xerrno = errno;

  if (data == NULL) {
    pr_trace_msg(trace_channel, 6,
      "error base64-decoding key '%s': %s", path, strerror(xerrno));
    errno = xerrno;
    return -1;
  }

  magicsz = sizeof(SFTP_OPENSSH_MAGIC);
  if (data_len < magicsz) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key base64-decoded data too short (%lu bytes < %lu minimum "
      "required)", path, (unsigned long) data_len, (unsigned long) magicsz);
    errno = EINVAL;
    return -1;
  }

  if (memcmp(data, SFTP_OPENSSH_MAGIC, magicsz) != 0) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key base64-decoded contains invalid magic value", path);
    errno = EINVAL;
    return -1;
  }

  data += magicsz;
  data_len -= magicsz;

  buf = data;
  buflen = data_len;

  cipher_name = sftp_msg_read_string(p, &buf, &buflen);
  kdf_name = sftp_msg_read_string(p, &buf, &buflen);

  kdf_len = sftp_msg_read_int(p, &buf, &buflen);
  kdf_data = sftp_msg_read_data(p, &buf, &buflen, kdf_len);
  key_count = sftp_msg_read_int(p, &buf, &buflen);

  /* Ignore the public key */
  (void) sftp_msg_read_string(p, &buf, &buflen);

  encrypted_len = sftp_msg_read_int(p, &buf, &buflen);

  pr_trace_msg(trace_channel, 9,
    "'%s' key cipher = '%s', KDF = '%s' (%lu bytes KDF data), "
     "key count = %lu, (%lu bytes encrypted data)", path, cipher_name, kdf_name,
     (unsigned long) kdf_len, (unsigned long) key_count,
     (unsigned long) encrypted_len);

  cipher = get_openssh_cipher(cipher_name);
  if (cipher == NULL) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key uses unexpected/unsupported cipher (%s)", path, cipher_name);
    errno = EPERM;
    return -1;
  }

  if ((passphrase == NULL ||
       strlen(passphrase) == 0) &&
      strcmp(cipher_name, "none") != 0) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key requires passphrase for cipher (%s)", path, cipher_name);
    errno = EPERM;
    return -1;
  }

  /* We only support the "none" and "bcrypt" KDFs at present. */
  if (strcmp(kdf_name, "bcrypt") != 0 &&
      strcmp(kdf_name, "none") != 0) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key encrypted using unsupported KDF '%s'", path, kdf_name);
    errno = EPERM;
    return -1;
  }

  /* If our KDF is "none" and our cipher is NOT "none", we have a problem. */
  if (strcmp(kdf_name, "none") == 0 &&
      strcmp(cipher_name, "none") != 0) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key encrypted using mismatched KDF and cipher algorithms: "
      "KDF '%s', cipher '%s'", path, kdf_name, cipher_name);
    errno = EPERM;
    return -1;
  }

  /* OpenSSH only supports one key at present.  Huh. */
  if (key_count != 1) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key includes unexpected/unsupported key count (%lu)",
      path, (unsigned long) key_count);
    errno = EPERM;
    return -1;
  }

  /* XXX Should we enforce that the KDF data be empty for the "none" KDF? */
  if (strcmp(kdf_name, "none") == 0 &&
      kdf_len > 0) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key uses KDF 'none', but contains unexpected %lu bytes "
      "of KDF options", path, (unsigned long) kdf_len);
  }

  if (buflen < encrypted_len) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key declares %lu bytes of encrypted data, but has "
      "only %lu bytes remaining", path, (unsigned long) encrypted_len,
      (unsigned long) buflen);
    errno = EPERM;
    return -1;
  }

  if (encrypted_len < cipher->blocksz ||
      (encrypted_len % cipher->blocksz) != 0) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key declares %lu bytes of encrypted data, which is invalid for "
      "the %s cipher block size (%lu bytes)", path,
      (unsigned long) encrypted_len, cipher_name,
      (unsigned long) cipher->blocksz);
    errno = EPERM;
    return -1;
  }

  if (buflen < (encrypted_len + cipher->auth_len)) {
    pr_trace_msg(trace_channel, 6,
      "'%s' key declares %lu bytes of encrypted data and %lu bytes of auth "
      "data, but has only %lu bytes remaining", path,
      (unsigned long) encrypted_len, (unsigned long) cipher->auth_len,
      (unsigned long) buflen);
    errno = EPERM;
    return -1;
  }

  encrypted_data = sftp_msg_read_data(p, &buf, &buflen, encrypted_len);

#if 0
  if (cipher->auth_len > 0) {
    /* Read (and ignore) any auth data (for AEAD ciphers). */
    (void) sftp_msg_read_data(p, &encrypted_data, &encrypted_len,
      cipher->auth_len);
  }

  /* We should have used all of the encrypted data, with none left over. */
  if (encrypted_len != 0) {
    pr_trace_msg(trace_channel, 3,
      "'%s' key provided too much data (%lu bytes remaining unexpectedly)",
      path, (unsigned long) encrypted_len);
    pr_memscrub(buf, buflen);
    errno = EINVAL;
    return -1;
  }
#endif

  return decrypt_openssh_private_key(p, path, encrypted_data, encrypted_len,
    passphrase, cipher, kdf_name, kdf_data, kdf_len, key_type, pkey, key,
    keylen);
}

static int read_openssh_private_key(pool *p, const char *path, int fd,
    const char *passphrase, enum sftp_key_type_e *key_type, EVP_PKEY **pkey,
    unsigned char **key, uint32_t *keylen) {
  struct stat st;
  pool *tmp_pool;
  unsigned char *decoded_buf, *decoded_ptr, *input_buf, *input_ptr;
  unsigned char *tmp_key = NULL;
  int res, xerrno = 0;
  size_t decoded_len, input_len;
  off_t input_sz;

  if (p == NULL ||
      path == NULL ||
      fd < 0 ||
      key == NULL ||
      keylen == NULL) {
    errno = EINVAL;
    return -1;
  }

  if (fstat(fd, &st) < 0) {
    return -1;
  }

  tmp_pool = make_sub_pool(p);

  /* Read the entire file into memory. */
  /* TODO: Impose maximum size limit for this treatment? */
  input_sz = st.st_size;
  input_ptr = input_buf = palloc(tmp_pool, input_sz);
  input_len = 0;

  res = read(fd, input_buf, input_sz);
  xerrno = errno;

  while (res != 0) {
    pr_signals_handle();

    if (res < 0) {
      pr_log_debug(DEBUG0, MOD_SFTP_VERSION ": error reading '%s': %s",
        path, strerror(xerrno));
      destroy_pool(tmp_pool);
      errno = xerrno;
      return -1;
    }

    input_buf += res;
    input_len += res;
    input_sz -= res;

    res = read(fd, input_buf, input_sz);
    xerrno = errno;
  }

  input_buf = input_ptr;

  /* Now we read from the input buffer into a buffer for decoding, for use
   * in the unwrapping process.
   */
  decoded_ptr = decoded_buf = palloc(tmp_pool, input_len);
  decoded_len = 0;

  /* We know (due to the OpenSSH private key check) that the first bytes
   * match the expected start, and that the last bytes match the expected end.
   * So skip past them.
   */
  input_buf += SFTP_OPENSSH_BEGIN_LEN;
  input_len -= (SFTP_OPENSSH_BEGIN_LEN + SFTP_OPENSSH_END_LEN);

  while (input_len > 0) {
    char ch;

    pr_signals_handle();

    ch = *input_buf;

    /* Skip whitespace */
    if (ch != '\r' &&
        ch != '\n') {
      *decoded_buf++ = ch;
      decoded_len++;
    }

    input_buf++;
    input_len--;
  }

  res = unwrap_openssh_private_key(tmp_pool, path, decoded_ptr, decoded_len,
    passphrase, key_type, pkey, &tmp_key, keylen);
  xerrno = errno;

  if (res < 0) {
    destroy_pool(tmp_pool);
    errno = xerrno;
    return -1;
  }

  /* At this point, our unwrapped key is allocated out of the temporary pool.
   * We need to copy it to memory out of the longer-lived pool given to us.
   */
  if (*keylen > 0) {
    *key = palloc(p, *keylen);
    memcpy(*key, tmp_key, *keylen);

    pr_memscrub(tmp_key, *keylen);
  }

  destroy_pool(tmp_pool);
  return 0;
}

#if defined(PR_USE_SODIUM)
static int handle_ed25519_hostkey(pool *p, const unsigned char *key_data,
    uint32_t key_datalen, const char *file_path) {
  unsigned char *public_key;

  if (sftp_ed25519_hostkey != NULL) {
    /* If we have an existing ED25519 hostkey, free it up. */
    pr_memscrub(sftp_ed25519_hostkey->ed25519_secret_key,
      sftp_ed25519_hostkey->ed25519_secret_keylen);
    sftp_ed25519_hostkey->ed25519_secret_key = NULL;
    sftp_ed25519_hostkey->ed25519_secret_keylen = 0;

    pr_memscrub(sftp_ed25519_hostkey->ed25519_public_key,
      sftp_ed25519_hostkey->ed25519_public_keylen);
    sftp_ed25519_hostkey->ed25519_public_key = NULL;
    sftp_ed25519_hostkey->ed25519_public_keylen = 0;

    sftp_ed25519_hostkey->file_path = NULL;
    sftp_ed25519_hostkey->agent_path = NULL;

  } else {
    sftp_ed25519_hostkey = pcalloc(p, sizeof(struct sftp_hostkey));
  }

  sftp_ed25519_hostkey->key_type = SFTP_KEY_ED25519;
  sftp_ed25519_hostkey->ed25519_secret_key = (unsigned char *) key_data;
  sftp_ed25519_hostkey->ed25519_secret_keylen = key_datalen;

  /* Use the secret key to get the public key. */
  public_key = palloc(p, crypto_sign_ed25519_PUBLICKEYBYTES);
  if (crypto_sign_ed25519_sk_to_pk(public_key, key_data) != 0) {
    return -1;
  }

  sftp_ed25519_hostkey->ed25519_public_key = public_key;
  sftp_ed25519_hostkey->ed25519_public_keylen = crypto_sign_ed25519_PUBLICKEYBYTES;

  sftp_ed25519_hostkey->file_path = file_path;
  pr_trace_msg(trace_channel, 4, "using '%s' as Ed25519 hostkey", file_path);

  return 0;
}
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
static int handle_ed448_hostkey(pool *p, const unsigned char *key_data,
    uint32_t key_datalen, const char *file_path) {
  unsigned char *public_key;
  EVP_PKEY *pkey = NULL;
  size_t public_keylen = 0;

  if (sftp_ed448_hostkey != NULL) {
    /* If we have an existing ED448 hostkey, free it up. */
    pr_memscrub(sftp_ed448_hostkey->ed448_secret_key,
      sftp_ed448_hostkey->ed448_secret_keylen);
    sftp_ed448_hostkey->ed448_secret_key = NULL;
    sftp_ed448_hostkey->ed448_secret_keylen = 0;

    pr_memscrub(sftp_ed448_hostkey->ed448_public_key,
      sftp_ed448_hostkey->ed448_public_keylen);
    sftp_ed448_hostkey->ed448_public_key = NULL;
    sftp_ed448_hostkey->ed448_public_keylen = 0;

    sftp_ed448_hostkey->file_path = NULL;
    sftp_ed448_hostkey->agent_path = NULL;

  } else {
    sftp_ed448_hostkey = pcalloc(p, sizeof(struct sftp_hostkey));
  }

  sftp_ed448_hostkey->key_type = SFTP_KEY_ED448;
  sftp_ed448_hostkey->ed448_secret_key = (unsigned char *) key_data;
  sftp_ed448_hostkey->ed448_secret_keylen = key_datalen;

  pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED448, NULL,
    sftp_ed448_hostkey->ed448_secret_key,
    sftp_ed448_hostkey->ed448_secret_keylen);
  if (pkey == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing Ed448 private key: %s", sftp_crypto_get_errors());
    return -1;
  }

  /* Use the secret key to get the public key. */
  public_keylen = (CURVE448_SIZE * 2);
  public_key = palloc(p, public_keylen);
  if (EVP_PKEY_get_raw_public_key(pkey, public_key, &public_keylen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining Ed448 public key: %s", sftp_crypto_get_errors());
    EVP_PKEY_free(pkey);
    return -1;
  }

  EVP_PKEY_free(pkey);

  sftp_ed448_hostkey->ed448_public_key = public_key;
  sftp_ed448_hostkey->ed448_public_keylen = public_keylen;

  sftp_ed448_hostkey->file_path = file_path;
  pr_trace_msg(trace_channel, 4, "using '%s' as Ed448 hostkey", file_path);

  return 0;
}
#endif /* HAVE_X448_OPENSSL */

static int load_openssh_hostkey(pool *p, const char *path, int fd) {
  const char *passphrase = NULL;
  enum sftp_key_type_e key_type = SFTP_KEY_UNKNOWN;
  EVP_PKEY *pkey = NULL;
  unsigned char *key = NULL;
  uint32_t keylen = 0;
  int res;

  if (server_pkey != NULL) {
    passphrase = server_pkey->host_pkey;
  }

  res = read_openssh_private_key(p, path, fd, passphrase, &key_type, &pkey,
    &key, &keylen);
  if (res < 0) {
    return -1;
  }

  switch (key_type) {
#if defined(PR_USE_SODIUM)
    case SFTP_KEY_ED25519:
      res = handle_ed25519_hostkey(p, key, keylen, path);
      break;
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
    case SFTP_KEY_ED448:
      res = handle_ed448_hostkey(p, key, keylen, path);
      break;
#endif /* HAVE_X448_OPENSSL */

    default:
      res = handle_hostkey(p, pkey, NULL, 0, path, NULL);
      break;
  }

  return res;
}

static int load_file_hostkey(pool *p, const char *path) {
  int fd, xerrno = 0, openssh_format = FALSE, public_key_format = FALSE;
  FILE *fp;
  EVP_PKEY *pkey;

  pr_signals_block();
  PRIVS_ROOT

  /* XXX Would we ever want to allow host keys to be read from FIFOs?  If
   * so, we would need to include the O_NONBLOCK flag here.
   */
  fd = open(path, O_RDONLY, 0);
  xerrno = errno;
  PRIVS_RELINQUISH
  pr_signals_unblock();

  if (fd < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading '%s': %s", path, strerror(xerrno));
    errno = xerrno;
    return -1;
  }

  if (has_req_perms(fd, path) < 0) {
    xerrno = errno;

    if (xerrno == EACCES) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "'%s' is accessible by group or world, which is not allowed", path);

    } else {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "error checking '%s' perms: %s", path, strerror(xerrno));
    }

    (void) close(fd);
    errno = xerrno;
    return -1;
  }

  if (server_pkey == NULL) {
    server_pkey = lookup_pkey();
  }

  /* Make sure this is not a public key inadvertently configured as a hostkey.
   */
  public_key_format = is_public_key(fd);
  if (public_key_format == TRUE) {
    pr_trace_msg(trace_channel, 3, "hostkey file '%s' uses a public key format",
      path);
    (void) pr_log_pri(PR_LOG_WARNING, MOD_SFTP_VERSION
      ": unable to use public key '%s' for SFTPHostKey", path);
    (void) close(fd);
    errno = EINVAL;
    return -1;
  }

  /* If this happens to be in the OpenSSH private key format, handle it
   * separately.
   */
  openssh_format = is_openssh_private_key(fd);
  if (openssh_format == TRUE) {
    int res;

    pr_trace_msg(trace_channel, 9, "hostkey file '%s' uses OpenSSH key format",
      path);

    res = load_openssh_hostkey(p, path, fd);
    xerrno = errno;

    (void) close(fd);
    errno = xerrno;
    return res;
  }

  /* OpenSSL's APIs prefer stdio file handles. */
  fp = fdopen(fd, "r");
  if (fp == NULL) {
    xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error opening stdio handle on fd %d: %s", fd, strerror(xerrno));
    (void) close(fd);

    errno = xerrno;
    return -1;
  }

  /* As the file contains sensitive data, we do not want it lingering
   * around in stdio buffers.
   */
  (void) setvbuf(fp, NULL, _IONBF, 0);

  if (server_pkey != NULL) {
    pkey = PEM_read_PrivateKey(fp, NULL, pkey_cb, (void *) server_pkey);

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

  return handle_hostkey(p, pkey, NULL, 0, path, NULL);
}

int sftp_keys_get_hostkey(pool *p, const char *path) {
  int res;

  /* Check whether we are to load keys from a file on disk, or from an
   * SSH agent.
   */
  if (strncmp(path, "agent:", 6) != 0) {
    pr_trace_msg(trace_channel, 9, "loading host key from file '%s'", path);
    res = load_file_hostkey(p, path);

  } else {
    const char *agent_path;

    /* Skip past the "agent:" prefix. */
    agent_path = (path + 6);

    pr_trace_msg(trace_channel, 9, "loading host keys from SSH agent at '%s'",
      agent_path);
    res = load_agent_hostkeys(p, agent_path);
  }

  return res;
}

static int get_rsa_hostkey_data(pool *p, unsigned char **buf,
    unsigned char **ptr, uint32_t *buflen) {
  RSA *rsa;
  const BIGNUM *rsa_n = NULL, *rsa_e = NULL;

  rsa = EVP_PKEY_get1_RSA(sftp_rsa_hostkey->pkey);
  if (rsa == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error using RSA hostkey: %s", sftp_crypto_get_errors());
    return -1;
  }

  /* XXX Is this buffer large enough?  Too large? */
  *ptr = *buf = palloc(p, *buflen);
  sftp_msg_write_string(buf, buflen, "ssh-rsa");

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  RSA_get0_key(rsa, &rsa_n, &rsa_e, NULL);
#else
  rsa_e = rsa->e;
  rsa_n = rsa->n;
#endif /* prior to OpenSSL-1.1.0 */
  sftp_msg_write_mpint(buf, buflen, rsa_e);
  sftp_msg_write_mpint(buf, buflen, rsa_n);

  RSA_free(rsa);
  return 0;
}

#if !defined(OPENSSL_NO_DSA)
static int get_dsa_hostkey_data(pool *p, unsigned char **buf,
    unsigned char **ptr, uint32_t *buflen) {
  DSA *dsa;
  const BIGNUM *dsa_p = NULL, *dsa_q = NULL, *dsa_g = NULL, *dsa_pub_key = NULL;

  dsa = EVP_PKEY_get1_DSA(sftp_dsa_hostkey->pkey);
  if (dsa == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error using DSA hostkey: %s", sftp_crypto_get_errors());
    return -1;
  }

  /* XXX Is this buffer large enough?  Too large? */
  *ptr = *buf = palloc(p, *buflen);
  sftp_msg_write_string(buf, buflen, "ssh-dss");

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  DSA_get0_pqg(dsa, &dsa_p, &dsa_q, &dsa_g);
  DSA_get0_key(dsa, &dsa_pub_key, NULL);
#else
  dsa_p = dsa->p;
  dsa_q = dsa->q;
  dsa_g = dsa->g;
  dsa_pub_key = dsa->pub_key;;
#endif /* prior to OpenSSL-1.1.0 */
  sftp_msg_write_mpint(buf, buflen, dsa_p);
  sftp_msg_write_mpint(buf, buflen, dsa_q);
  sftp_msg_write_mpint(buf, buflen, dsa_g);
  sftp_msg_write_mpint(buf, buflen, dsa_pub_key);

  DSA_free(dsa);
  return 0;
}
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
static int get_ecdsa_hostkey_data(pool *p, struct sftp_hostkey *hostkey,
    const char *algo, const char *curve, unsigned char **buf,
    unsigned char **ptr, uint32_t *buflen) {
  EC_KEY *ec;

  ec = EVP_PKEY_get1_EC_KEY(hostkey->pkey);
  if (ec == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error using %s hostkey: %s", algo, sftp_crypto_get_errors());
    return -1;
  }

  /* XXX Is this buffer large enough?  Too large? */
  *ptr = *buf = palloc(p, *buflen);
  sftp_msg_write_string(buf, buflen, algo);
  sftp_msg_write_string(buf, buflen, curve);
  sftp_msg_write_ecpoint(buf, buflen, EC_KEY_get0_group(ec),
    EC_KEY_get0_public_key(ec));

  EC_KEY_free(ec);
  return 0;
}
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
static int get_ed25519_hostkey_data(pool *p, unsigned char **buf,
    unsigned char **ptr, uint32_t *buflen) {

  /* XXX Is this buffer large enough?  Too large? */
  *ptr = *buf = palloc(p, *buflen);
  sftp_msg_write_string(buf, buflen, "ssh-ed25519");
  sftp_msg_write_data(buf, buflen, sftp_ed25519_hostkey->ed25519_public_key,
    sftp_ed25519_hostkey->ed25519_public_keylen, TRUE);

  return 0;
}
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
static int get_ed448_hostkey_data(pool *p, unsigned char **buf,
    unsigned char **ptr, uint32_t *buflen) {

  /* XXX Is this buffer large enough?  Too large? */
  *ptr = *buf = palloc(p, *buflen);
  sftp_msg_write_string(buf, buflen, "ssh-ed448");
  sftp_msg_write_data(buf, buflen, sftp_ed448_hostkey->ed448_public_key,
    sftp_ed448_hostkey->ed448_public_keylen, TRUE);

  return 0;
}
#endif /* HAVE_X448_OPENSSL */

const unsigned char *sftp_keys_get_hostkey_data(pool *p,
    enum sftp_key_type_e key_type, uint32_t *datalen) {
  unsigned char *buf = NULL, *ptr = NULL;
  uint32_t buflen = SFTP_DEFAULT_HOSTKEY_SZ;
  int res;

  switch (key_type) {
    case SFTP_KEY_RSA:
    case SFTP_KEY_RSA_SHA256:
    case SFTP_KEY_RSA_SHA512: {
      res = get_rsa_hostkey_data(p, &buf, &ptr, &buflen);
      if (res < 0) {
        return NULL;
      }

      break;
    }

#if !defined(OPENSSL_NO_DSA)
    case SFTP_KEY_DSA: {
      res = get_dsa_hostkey_data(p, &buf, &ptr, &buflen);
      if (res < 0) {
        return NULL;
      }

      break;
    }
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
    case SFTP_KEY_ECDSA_256: {
      res = get_ecdsa_hostkey_data(p, sftp_ecdsa256_hostkey,
        "ecdsa-sha2-nistp256", "nistp256", &buf, &ptr, &buflen);
      if (res < 0) {
        return NULL;
      }

      break;
    }

    case SFTP_KEY_ECDSA_384: {
      res = get_ecdsa_hostkey_data(p, sftp_ecdsa384_hostkey,
        "ecdsa-sha2-nistp384", "nistp384", &buf, &ptr, &buflen);
      if (res < 0) {
        return NULL;
      }

      break;
    }

    case SFTP_KEY_ECDSA_521: {
      res = get_ecdsa_hostkey_data(p, sftp_ecdsa521_hostkey,
        "ecdsa-sha2-nistp521", "nistp521", &buf, &ptr, &buflen);
      if (res < 0) {
        return NULL;
      }

      break;
    }
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
    case SFTP_KEY_ED25519: {
      res = get_ed25519_hostkey_data(p, &buf, &ptr, &buflen);
      if (res < 0) {
        return NULL;
      }

      break;
    }
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
    case SFTP_KEY_ED448: {
      res = get_ed448_hostkey_data(p, &buf, &ptr, &buflen);
      if (res < 0) {
        return NULL;
      }

      break;
    }
#endif /* HAVE_X448_OPENSSL */

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unknown/unsupported key type (%d) requested, ignoring", key_type);
      return NULL;
  }

  *datalen = SFTP_DEFAULT_HOSTKEY_SZ - buflen;

  /* If the caller provided a pool, make a copy of the data from the
   * given pool, and return the copy.  Make sure to scrub the original
   * after making the copy.
   *
   * Note that we do this copy, even though we use the given pool, since
   * we only know the actual size of the data after the fact.  And we need
   * to provide the size of the data to the caller, NOT the optimistic size
   * we allocate out of the pool for writing the data in the first place.
   * Hence the copy.
   */
  buf = palloc(p, *datalen);
  memcpy(buf, ptr, *datalen);

  pr_memscrub(ptr, *datalen);
  return buf;
}

int sftp_keys_clear_dsa_hostkey(void) {
  if (sftp_dsa_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  if (sftp_dsa_hostkey->pkey != NULL) {
    EVP_PKEY_free(sftp_dsa_hostkey->pkey);
  }

  sftp_dsa_hostkey = NULL;
  return 0;
}

int sftp_keys_clear_ecdsa_hostkey(void) {
#if defined(PR_USE_OPENSSL_ECC)
  int count = 0;

  if (sftp_ecdsa256_hostkey != NULL) {
    if (sftp_ecdsa256_hostkey->pkey != NULL) {
      EVP_PKEY_free(sftp_ecdsa256_hostkey->pkey);
    }

    sftp_ecdsa256_hostkey = NULL;
    count++;
  }

  if (sftp_ecdsa384_hostkey != NULL) {
    if (sftp_ecdsa384_hostkey->pkey != NULL) {
      EVP_PKEY_free(sftp_ecdsa384_hostkey->pkey);
    }

    sftp_ecdsa384_hostkey = NULL;
    count++;
  }

  if (sftp_ecdsa521_hostkey != NULL) {
    if (sftp_ecdsa521_hostkey->pkey != NULL) {
      EVP_PKEY_free(sftp_ecdsa521_hostkey->pkey);
    }

    sftp_ecdsa521_hostkey = NULL;
    count++;
  }

  if (count > 0) {
    return 0;
  }

#endif /* PR_USE_OPENSSL_ECC */
  errno = ENOENT;
  return -1;
}

int sftp_keys_clear_ed25519_hostkey(void) {
  if (sftp_ed25519_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  if (sftp_ed25519_hostkey->ed25519_secret_key != NULL) {
    pr_memscrub(sftp_ed25519_hostkey->ed25519_secret_key,
      sftp_ed25519_hostkey->ed25519_secret_keylen);
    sftp_ed25519_hostkey->ed25519_secret_key = NULL;
    sftp_ed25519_hostkey->ed25519_secret_keylen = 0;
  }

  if (sftp_ed25519_hostkey->ed25519_public_key != NULL) {
    pr_memscrub(sftp_ed25519_hostkey->ed25519_public_key,
      sftp_ed25519_hostkey->ed25519_public_keylen);
    sftp_ed25519_hostkey->ed25519_public_key = NULL;
    sftp_ed25519_hostkey->ed25519_public_keylen = 0;
  }

  return 0;
}

int sftp_keys_clear_ed448_hostkey(void) {
  if (sftp_ed448_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  if (sftp_ed448_hostkey->ed448_secret_key != NULL) {
    pr_memscrub(sftp_ed448_hostkey->ed448_secret_key,
      sftp_ed448_hostkey->ed448_secret_keylen);
    sftp_ed448_hostkey->ed448_secret_key = NULL;
    sftp_ed448_hostkey->ed448_secret_keylen = 0;
  }

  if (sftp_ed448_hostkey->ed448_public_key != NULL) {
    pr_memscrub(sftp_ed448_hostkey->ed448_public_key,
      sftp_ed448_hostkey->ed448_public_keylen);
    sftp_ed448_hostkey->ed448_public_key = NULL;
    sftp_ed448_hostkey->ed448_public_keylen = 0;
  }

  return 0;
}

int sftp_keys_clear_rsa_hostkey(void) {
  if (sftp_rsa_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  if (sftp_rsa_hostkey->pkey != NULL) {
    EVP_PKEY_free(sftp_rsa_hostkey->pkey);
  }

  sftp_rsa_hostkey = NULL;
  return 0;
}

int sftp_keys_have_dsa_hostkey(void) {
  if (sftp_dsa_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  return 0;
}

/* Returns the count of returned NIDs for the configured ECDSA hostkeys,
 * if any.
 */
int sftp_keys_have_ecdsa_hostkey(pool *p, int **nids) {
#if defined(PR_USE_OPENSSL_ECC)
  int count = 0;

  if (nids != NULL) {
    *nids = palloc(p, sizeof(int) * 3);
  }

  if (sftp_ecdsa256_hostkey != NULL) {
    EC_KEY *ec;

    ec = EVP_PKEY_get1_EC_KEY(sftp_ecdsa256_hostkey->pkey);
    if (nids != NULL) {
      (*nids)[count] = get_ecdsa_nid(ec);
    }
    count++;
    EC_KEY_free(ec);

  } else if (sftp_ecdsa384_hostkey != NULL) {
    EC_KEY *ec;

    ec = EVP_PKEY_get1_EC_KEY(sftp_ecdsa384_hostkey->pkey);
    if (nids != NULL) {
      (*nids)[count] = get_ecdsa_nid(ec);
    }
    count++;
    EC_KEY_free(ec);

  } else if (sftp_ecdsa521_hostkey != NULL) {
    EC_KEY *ec;

    ec = EVP_PKEY_get1_EC_KEY(sftp_ecdsa521_hostkey->pkey);
    if (nids != NULL) {
      (*nids)[count] = get_ecdsa_nid(ec);
    }
    count++;
    EC_KEY_free(ec);
  }

  if (count > 0) {
    return count;
  }
#endif /* PR_USE_OPENSSL_ECC */

  errno = ENOENT;
  return -1;
}

int sftp_keys_have_ed25519_hostkey(void) {
  if (sftp_ed25519_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  return 0;
}

int sftp_keys_have_ed448_hostkey(void) {
  if (sftp_ed448_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  return 0;
}

int sftp_keys_have_rsa_hostkey(void) {
  if (sftp_rsa_hostkey == NULL) {
    errno = ENOENT;
    return -1;
  }

  return 0;
}

static const unsigned char *agent_sign_data(pool *p, const char *agent_path,
    const unsigned char *key_data, uint32_t key_datalen,
    const unsigned char *data, size_t datalen, size_t *siglen, int flags) {
  unsigned char *sig_data;
  uint32_t sig_datalen = 0;

  pr_trace_msg(trace_channel, 15,
    "asking SSH agent at '%s' to sign data", agent_path);

  /* Ask the agent to sign the data for this hostkey for us. */
  sig_data = (unsigned char *) sftp_agent_sign_data(p, agent_path,
    key_data, key_datalen, data, datalen, &sig_datalen, flags);

  if (sig_data == NULL) {
    int xerrno = errno;

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "SSH agent at '%s' could not sign data: %s", agent_path,
      strerror(xerrno));

    errno = xerrno;
    return NULL;
  }

  /* The SSH agent already provides the signed data in the correct
   * SSH2-style.
   */

  *siglen = sig_datalen;
  return sig_data;
}

static const unsigned char *get_rsa_signed_data(pool *p,
    const unsigned char *data, size_t datalen, size_t *siglen,
    const char *sig_name, const EVP_MD *md) {
  RSA *rsa;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_MD_CTX ctx;
#endif /* prior to OpenSSL-1.1.0 */
  EVP_MD_CTX *pctx;
  unsigned char dgst[EVP_MAX_MD_SIZE], *sig_data;
  unsigned char *buf, *ptr;
  size_t bufsz;
  uint32_t buflen, dgstlen = 0, sig_datalen = 0, sig_rsalen = 0;
  int res;

  rsa = EVP_PKEY_get1_RSA(sftp_rsa_hostkey->pkey);
  if (rsa == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error using RSA hostkey: %s", sftp_crypto_get_errors());
    return NULL;
  }

  if (keys_rsa_min_nbits > 0) {
    int rsa_nbits;

    rsa_nbits = RSA_size(rsa) * 8;
    if (rsa_nbits < keys_rsa_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "RSA hostkey size (%d bits) less than required minimum (%d bits)",
        rsa_nbits, keys_rsa_min_nbits);
      RSA_free(rsa);

      errno = EINVAL;
      return NULL;
    }
  }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  pctx = EVP_MD_CTX_new();
#else
  pctx = &ctx;
#endif /* prior to OpenSSL-1.1.0 */

  EVP_DigestInit(pctx, md);
  EVP_DigestUpdate(pctx, data, datalen);
  EVP_DigestFinal(pctx, dgst, &dgstlen);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  EVP_MD_CTX_free(pctx);
#endif /* OpenSSL-1.1.0 and later */

  sig_rsalen = RSA_size(rsa);
  sig_data = pcalloc(p, sig_rsalen);
  res = RSA_sign(EVP_MD_type(md), dgst, dgstlen, sig_data, &sig_datalen, rsa);

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
  ptr = buf = palloc(p, bufsz);

  /* Now build up the signature, SSH2-style */
  sftp_msg_write_string(&buf, &buflen, sig_name);
  sftp_msg_write_data(&buf, &buflen, sig_data, sig_datalen, TRUE);

  pr_memscrub(sig_data, sig_datalen);
  RSA_free(rsa);

  /* At this point, buflen is the amount remaining in the allocated buffer.
   * So the total length of the signed data is the buffer size, minus those
   * remaining unused bytes.
   */
  *siglen = (bufsz - buflen);
  return ptr;
}

static const unsigned char *rsa_sign_data(pool *p, const unsigned char *data,
    size_t datalen, size_t *siglen) {
  if (sftp_rsa_hostkey->agent_path != NULL) {
    return agent_sign_data(p, sftp_rsa_hostkey->agent_path,
      sftp_rsa_hostkey->key_data, sftp_rsa_hostkey->key_datalen, data, datalen,
      siglen, 0);
  }

  return get_rsa_signed_data(p, data, datalen, siglen, "ssh-rsa", EVP_sha1());
}

#if defined(HAVE_SHA256_OPENSSL)
static const unsigned char *rsa_sha256_sign_data(pool *p,
    const unsigned char *data, size_t datalen, size_t *siglen) {
  if (sftp_rsa_hostkey->agent_path != NULL) {
    return agent_sign_data(p, sftp_rsa_hostkey->agent_path,
      sftp_rsa_hostkey->key_data, sftp_rsa_hostkey->key_datalen, data, datalen,
      siglen, SFTP_AGENT_SIGN_FL_USE_RSA_SHA256);
  }

  return get_rsa_signed_data(p, data, datalen, siglen, "rsa-sha2-256",
    EVP_sha256());
}
#endif /* HAVE_SHA256_OPENSSL */

#if defined(HAVE_SHA512_OPENSSL)
static const unsigned char *rsa_sha512_sign_data(pool *p,
    const unsigned char *data, size_t datalen, size_t *siglen) {
  if (sftp_rsa_hostkey->agent_path != NULL) {
    return agent_sign_data(p, sftp_rsa_hostkey->agent_path,
      sftp_rsa_hostkey->key_data, sftp_rsa_hostkey->key_datalen, data, datalen,
      siglen, SFTP_AGENT_SIGN_FL_USE_RSA_SHA512);
  }

  return get_rsa_signed_data(p, data, datalen, siglen, "rsa-sha2-512",
    EVP_sha512());
}
#endif /* HAVE_SHA256_OPENSSL */

/* RFC 4253, Section 6.6, is quite specific about the length of a DSA
 * ("ssh-dss") signature blob.  It is comprised of two integers R and S,
 * each 160 bits (20 bytes), so that the total signature blob is 40 bytes
 * long.
 */
#define SFTP_DSA_INTEGER_LEN			20
#define SFTP_DSA_SIGNATURE_LEN			(SFTP_DSA_INTEGER_LEN * 2)

#if !defined(OPENSSL_NO_DSA)
static const unsigned char *dsa_sign_data(pool *p, const unsigned char *data,
    size_t datalen, size_t *siglen) {
  DSA *dsa;
  DSA_SIG *sig;
  const BIGNUM *sig_r = NULL, *sig_s = NULL;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_MD_CTX ctx;
#endif /* prior to OpenSSL-1.1.0 */
  EVP_MD_CTX *pctx;
  const EVP_MD *sha1 = EVP_sha1();
  unsigned char dgst[EVP_MAX_MD_SIZE], *sig_data;
  unsigned char *buf, *ptr;
  size_t bufsz;
  uint32_t buflen, dgstlen = 0;
  unsigned int rlen = 0, slen = 0;

  if (sftp_dsa_hostkey->agent_path != NULL) {
    return agent_sign_data(p, sftp_dsa_hostkey->agent_path,
      sftp_dsa_hostkey->key_data, sftp_dsa_hostkey->key_datalen, data, datalen,
      siglen, 0);
  }

  dsa = EVP_PKEY_get1_DSA(sftp_dsa_hostkey->pkey);
  if (dsa == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error using DSA hostkey: %s", sftp_crypto_get_errors());
    return NULL;
  }

  if (keys_dsa_min_nbits > 0) {
    int dsa_nbits;

    dsa_nbits = DSA_size(dsa) * 8;
    if (dsa_nbits < keys_dsa_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "DSA hostkey size (%d bits) less than required minimum (%d bits)",
        dsa_nbits, keys_dsa_min_nbits);
      DSA_free(dsa);

      errno = EINVAL;
      return NULL;
    }
  }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  pctx = EVP_MD_CTX_new();
#else
  pctx = &ctx;
#endif /* prior to OpenSSL-1.1.0 */

  EVP_DigestInit(pctx, sha1);
  EVP_DigestUpdate(pctx, data, datalen);
  EVP_DigestFinal(pctx, dgst, &dgstlen);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  EVP_MD_CTX_free(pctx);
#endif /* OpenSSL-1.1.0 and later */

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

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  DSA_SIG_get0(sig, &sig_r, &sig_s);
#else
  sig_r = sig->r;
  sig_s = sig->s;
#endif /* prior to OpenSSL-1.1.0 */

  rlen = BN_num_bytes(sig_r);
  slen = BN_num_bytes(sig_s);

  /* Make sure the values of R and S are big enough. */
  if (rlen > SFTP_DSA_INTEGER_LEN ||
      slen > SFTP_DSA_INTEGER_LEN) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "bad DSA signature size (%u, %u)", rlen, slen);
    DSA_SIG_free(sig);
    DSA_free(dsa);
    return NULL;
  }

  sig_data = pcalloc(p, SFTP_MAX_SIG_SZ);

  /* These may look strange, but the pointer arithmetic is necessary to
   * ensure the correct placement of the R and S values in the signature,
   * per RFC 4253 Section 6.6 requirements.
   */
  BN_bn2bin(sig_r,
    sig_data + SFTP_DSA_SIGNATURE_LEN - SFTP_DSA_INTEGER_LEN - rlen);
  BN_bn2bin(sig_s, sig_data + SFTP_DSA_SIGNATURE_LEN - slen);

  /* Done with the signature. */
  DSA_SIG_free(sig);
  DSA_free(dsa);

  /* XXX Is this buffer large enough?  Too large? */
  buflen = bufsz = SFTP_MAX_SIG_SZ;
  ptr = buf = palloc(p, bufsz);

  /* Now build up the signature, SSH2-style */
  sftp_msg_write_string(&buf, &buflen, "ssh-dss");
  sftp_msg_write_data(&buf, &buflen, sig_data, SFTP_DSA_SIGNATURE_LEN, TRUE);

  /* At this point, buflen is the amount remaining in the allocated buffer.
   * So the total length of the signed data is the buffer size, minus those
   * remaining unused bytes.
   */
  *siglen = (bufsz - buflen);
  return ptr;
}
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
static const unsigned char *ecdsa_sign_data(pool *p, const unsigned char *data,
    size_t datalen, size_t *siglen, int nid) {
  EVP_PKEY *pkey = NULL;
  EC_KEY *ec = NULL;
  ECDSA_SIG *sig;
  const BIGNUM *sig_r = NULL, *sig_s = NULL;
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_MD_CTX ctx;
#endif /* prior to OpenSSL-1.1.0 */
  EVP_MD_CTX *pctx;
  const EVP_MD *md;
  unsigned char dgst[EVP_MAX_MD_SIZE];
  unsigned char *buf, *ptr, *sig_buf, *sig_ptr;
  uint32_t bufsz, buflen, dgstlen = 0, sig_buflen, sig_bufsz;

  switch (nid) {
    case NID_X9_62_prime256v1:
      if (sftp_ecdsa256_hostkey->agent_path != NULL) {
        return agent_sign_data(p, sftp_ecdsa256_hostkey->agent_path,
          sftp_ecdsa256_hostkey->key_data, sftp_ecdsa256_hostkey->key_datalen,
          data, datalen, siglen, 0);
      }

      ec = EVP_PKEY_get1_EC_KEY(sftp_ecdsa256_hostkey->pkey);
      if (ec == NULL) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error using ECDSA-256 hostkey: %s", sftp_crypto_get_errors());
        return NULL;
      }

      pkey = sftp_ecdsa256_hostkey->pkey;
      md = EVP_sha256();
      break;

    case NID_secp384r1:
      if (sftp_ecdsa384_hostkey->agent_path != NULL) {
        return agent_sign_data(p, sftp_ecdsa384_hostkey->agent_path,
          sftp_ecdsa384_hostkey->key_data, sftp_ecdsa384_hostkey->key_datalen,
          data, datalen, siglen, 0);
      }

      ec = EVP_PKEY_get1_EC_KEY(sftp_ecdsa384_hostkey->pkey);
      if (ec == NULL) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error using ECDSA-384 hostkey: %s", sftp_crypto_get_errors());
        return NULL;
      }

      pkey = sftp_ecdsa384_hostkey->pkey;
      md = EVP_sha384();
      break;

    case NID_secp521r1:
      if (sftp_ecdsa521_hostkey->agent_path != NULL) {
        return agent_sign_data(p, sftp_ecdsa521_hostkey->agent_path,
          sftp_ecdsa521_hostkey->key_data, sftp_ecdsa521_hostkey->key_datalen,
          data, datalen, siglen, 0);
      }

      ec = EVP_PKEY_get1_EC_KEY(sftp_ecdsa521_hostkey->pkey);
      if (ec == NULL) {
        (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
          "error using ECDSA-521 hostkey: %s", sftp_crypto_get_errors());
        return NULL;
      }

      pkey = sftp_ecdsa521_hostkey->pkey;
      md = EVP_sha512();
      break;

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unknown/unsupported ECDSA NID (%d) requested", nid);
      return NULL;
  }

  if (keys_ec_min_nbits > 0) {
    int ec_nbits;

    ec_nbits = EVP_PKEY_bits(pkey) * 8;
    if (ec_nbits < keys_ec_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "EC hostkey size (%d bits) less than required minimum (%d bits)",
        ec_nbits, keys_ec_min_nbits);
      EC_KEY_free(ec);

      errno = EINVAL;
      return NULL;
    }
  }

  buflen = bufsz = SFTP_MAX_SIG_SZ;
  ptr = buf = palloc(p, bufsz);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  pctx = EVP_MD_CTX_new();
#else
  pctx = &ctx;
#endif /* prior to OpenSSL-1.1.0 */

  EVP_DigestInit(pctx, md);
  EVP_DigestUpdate(pctx, data, datalen);
  EVP_DigestFinal(pctx, dgst, &dgstlen);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  EVP_MD_CTX_free(pctx);
#endif /* OpenSSL-1.1.0 and later */

  sig = ECDSA_do_sign(dgst, dgstlen, ec);
  if (sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining ECDSA signature: %s", sftp_crypto_get_errors());
    pr_memscrub(dgst, dgstlen);
    EC_KEY_free(ec);
    return NULL;
  }

  /* Got the signature, no need for the digest memory. */
  pr_memscrub(dgst, dgstlen);

  /* Unlike DSA, the R and S lengths for ECDSA are dependent on the curve
   * selected, so we do no sanity checking of their lengths.
   */

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  ECDSA_SIG_get0(sig, &sig_r, &sig_s);
#else
  sig_r = sig->r;
  sig_s = sig->s;
#endif /* prior to OpenSSL-1.1.0 */

  /* XXX Is this buffer large enough?  Too large? */
  sig_buflen = sig_bufsz = 256;
  sig_ptr = sig_buf = palloc(p, sig_bufsz);

  sftp_msg_write_mpint(&sig_buf, &sig_buflen, sig_r);
  sftp_msg_write_mpint(&sig_buf, &sig_buflen, sig_s);

  /* Done with the signature. */
  ECDSA_SIG_free(sig);
  EC_KEY_free(ec);

  /* XXX Is this buffer large enough?  Too large? */
  buflen = bufsz = SFTP_MAX_SIG_SZ;
  ptr = buf = palloc(p, bufsz);

  /* Now build up the signature, SSH2-style */
  switch (nid) {
    case NID_X9_62_prime256v1:
      sftp_msg_write_string(&buf, &buflen, "ecdsa-sha2-nistp256");
      break;

    case NID_secp384r1:
      sftp_msg_write_string(&buf, &buflen, "ecdsa-sha2-nistp384");
      break;

    case NID_secp521r1:
      sftp_msg_write_string(&buf, &buflen, "ecdsa-sha2-nistp521");
      break;
  }

  sftp_msg_write_data(&buf, &buflen, sig_ptr, (sig_bufsz - sig_buflen), TRUE);
  pr_memscrub(sig_ptr, sig_bufsz);

  /* At this point, buflen is the amount remaining in the allocated buffer.
   * So the total length of the signed data is the buffer size, minus those
   * remaining unused bytes.
   */
  *siglen = (bufsz - buflen);
  return ptr;
}
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
static const unsigned char *ed25519_sign_data(pool *p,
    const unsigned char *data, size_t datalen, size_t *siglen) {
  unsigned char *buf, *ptr, *sig_buf, *sig_ptr;
  uint32_t bufsz, buflen, sig_buflen, sig_bufsz;
  unsigned long long slen;
  int res;

/* XXX TODO ED25519: Test this! */
  if (sftp_ed25519_hostkey->agent_path != NULL) {
    return agent_sign_data(p, sftp_ed25519_hostkey->agent_path,
      sftp_ed25519_hostkey->ed25519_public_key,
      sftp_ed25519_hostkey->ed25519_public_keylen,
      data, datalen, siglen, 0);
  }

  sig_buflen = sig_bufsz = slen = datalen + crypto_sign_ed25519_BYTES;
  sig_ptr = sig_buf = palloc(p, sig_bufsz);

  res = crypto_sign_ed25519(sig_buf, &slen, data, datalen,
    sftp_ed25519_hostkey->ed25519_secret_key);
  if (res != 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "failed to sign data using Ed25519 (%d)", res);
    pr_memscrub(sig_ptr, sig_bufsz);
    return NULL;
  }

  sig_buflen = slen;
  if (sig_buflen <= datalen) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "invalid Ed25519 signature (%lu bytes) generated, expected more than "
      "%lu bytes", (unsigned long) sig_buflen, (unsigned long) datalen);
    pr_memscrub(sig_ptr, sig_bufsz);
    return NULL;
  }

  /* XXX Is this buffer large enough?  Too large? */
  buflen = bufsz = SFTP_MAX_SIG_SZ;
  ptr = buf = palloc(p, bufsz);

  /* Now build up the signature, SSH2-style */
  sftp_msg_write_string(&buf, &buflen, "ssh-ed25519");
  sftp_msg_write_data(&buf, &buflen, sig_ptr, sig_buflen - datalen, TRUE);
  pr_memscrub(sig_ptr, sig_bufsz);

  /* At this point, buflen is the amount remaining in the allocated buffer.
   * So the total length of the signed data is the buffer size, minus those
   * remaining unused bytes.
   */
  *siglen = (bufsz - buflen);
  return ptr;
}
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
static const unsigned char *ed448_sign_data(pool *p,
    const unsigned char *data, size_t datalen, size_t *siglen) {
  unsigned char *buf, *ptr, *sig_buf;
  uint32_t bufsz, buflen, sig_buflen, sig_bufsz;
  EVP_MD_CTX *md_ctx;
  EVP_PKEY *pkey;

/* XXX TODO ED448: Test this! */
  if (sftp_ed448_hostkey->agent_path != NULL) {
    return agent_sign_data(p, sftp_ed448_hostkey->agent_path,
      sftp_ed448_hostkey->ed448_public_key,
      sftp_ed448_hostkey->ed448_public_keylen,
      data, datalen, siglen, 0);
  }

  sig_buflen = sig_bufsz = datalen + 256;
  sig_buf = palloc(p, sig_bufsz);

  md_ctx = EVP_MD_CTX_new();
  pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED448, NULL,
    sftp_ed448_hostkey->ed448_secret_key,
    sftp_ed448_hostkey->ed448_secret_keylen);
  if (pkey == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing Ed448 private key: %s", sftp_crypto_get_errors());
    EVP_MD_CTX_free(md_ctx);
    return NULL;
  }

  if (EVP_DigestSignInit(md_ctx, NULL, NULL, NULL, pkey) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing Ed448 signature: %s", sftp_crypto_get_errors());
    EVP_PKEY_free(pkey);
    EVP_MD_CTX_free(md_ctx);
    return NULL;
  }

  if (EVP_DigestSign(md_ctx, sig_buf, (size_t *) &sig_buflen, data,
      datalen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "failed to sign data using Ed448: %s", sftp_crypto_get_errors());
    EVP_PKEY_free(pkey);
    EVP_MD_CTX_free(md_ctx);
    return NULL;
  }

  EVP_PKEY_free(pkey);
  EVP_MD_CTX_free(md_ctx);

  /* XXX Is this buffer large enough?  Too large? */
  buflen = bufsz = SFTP_MAX_SIG_SZ;
  ptr = buf = palloc(p, bufsz);

  /* Now build up the signature, SSH2-style */
  sftp_msg_write_string(&buf, &buflen, "ssh-ed448");
  sftp_msg_write_data(&buf, &buflen, sig_buf, sig_buflen, TRUE);
  pr_memscrub(sig_buf, sig_bufsz);

  /* At this point, buflen is the amount remaining in the allocated buffer.
   * So the total length of the signed data is the buffer size, minus those
   * remaining unused bytes.
   */
  *siglen = (bufsz - buflen);
  return ptr;
}
#endif /* HAVE_X448_OPENSSL */

const unsigned char *sftp_keys_sign_data(pool *p,
    enum sftp_key_type_e key_type, const unsigned char *data,
    size_t datalen, size_t *siglen) {
  const unsigned char *res;

  switch (key_type) {
    case SFTP_KEY_RSA:
      res = rsa_sign_data(p, data, datalen, siglen);
      break;

#if defined(HAVE_SHA256_OPENSSL)
    case SFTP_KEY_RSA_SHA256:
      res = rsa_sha256_sign_data(p, data, datalen, siglen);
      break;
#endif /* HAVE_SHA256_OPENSSL */

#if defined(HAVE_SHA512_OPENSSL)
    case SFTP_KEY_RSA_SHA512:
      res = rsa_sha512_sign_data(p, data, datalen, siglen);
      break;
#endif /* HAVE_SHA512_OPENSSL */

#if !defined(OPENSSL_NO_DSA)
    case SFTP_KEY_DSA:
      res = dsa_sign_data(p, data, datalen, siglen);
      break;
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
    case SFTP_KEY_ECDSA_256:
      res = ecdsa_sign_data(p, data, datalen, siglen, NID_X9_62_prime256v1);
      break;

    case SFTP_KEY_ECDSA_384:
      res = ecdsa_sign_data(p, data, datalen, siglen, NID_secp384r1);
      break;

    case SFTP_KEY_ECDSA_521:
      res = ecdsa_sign_data(p, data, datalen, siglen, NID_secp521r1);
      break;
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
    case SFTP_KEY_ED25519:
      res = ed25519_sign_data(p, data, datalen, siglen);
      break;
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
    case SFTP_KEY_ED448:
      res = ed448_sign_data(p, data, datalen, siglen);
      break;
#endif /* HAVE_X448_OPENSSL */

    default:
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unknown key type (%d) requested for signing, ignoring", key_type);
      return NULL;
  }

  if (res != NULL &&
      p != NULL) {
    unsigned char *buf;

    buf = palloc(p, *siglen);
    memcpy(buf, res, *siglen);

    pr_memscrub((char *) res, *siglen);
    return buf;
  }

  return res;
}

int sftp_keys_verify_pubkey_type(pool *p, unsigned char *pubkey_data,
    uint32_t pubkey_len, enum sftp_key_type_e pubkey_type) {
  EVP_PKEY *pkey = NULL;
  int res = FALSE;
  uint32_t len;

  if (pubkey_data == NULL ||
      pubkey_len == 0) {
    errno = EINVAL;
    return -1;
  }

  len = read_pkey_from_data(p, pubkey_data, pubkey_len, &pkey, NULL, FALSE);
  if (len == 0) {
    return -1;
  }

  switch (pubkey_type) {
    case SFTP_KEY_RSA:
    case SFTP_KEY_RSA_SHA256:
    case SFTP_KEY_RSA_SHA512:
      res = (get_pkey_type(pkey) == EVP_PKEY_RSA);
      break;

    case SFTP_KEY_DSA:
      res = (get_pkey_type(pkey) == EVP_PKEY_DSA);
      break;

#if defined(PR_USE_OPENSSL_ECC)
    case SFTP_KEY_ECDSA_256:
    case SFTP_KEY_ECDSA_384:
    case SFTP_KEY_ECDSA_521:
      if (get_pkey_type(pkey) == EVP_PKEY_EC) {
        EC_KEY *ec;
        int ec_nid;

        ec = EVP_PKEY_get1_EC_KEY(pkey);
        ec_nid = get_ecdsa_nid(ec);
        EC_KEY_free(ec);

        switch (ec_nid) {
          case NID_X9_62_prime256v1:
            res = (pubkey_type == SFTP_KEY_ECDSA_256);
            break;

          case NID_secp384r1:
            res = (pubkey_type == SFTP_KEY_ECDSA_384);
            break;

          case NID_secp521r1:
            res = (pubkey_type == SFTP_KEY_ECDSA_521);
            break;
        }
      }
      break;
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
    case SFTP_KEY_ED25519: {
      char *pkey_type;

      pkey_type = sftp_msg_read_string(p, &pubkey_data, &pubkey_len);
      if (strcmp(pkey_type, "ssh-ed25519") != 0) {
        pr_trace_msg(trace_channel, 8,
         "invalid public key type '%s' for Ed25519 key", pkey_type);
        res = FALSE;

      } else {
        uint32_t pklen;

        pklen = sftp_msg_read_int(p, &pubkey_data, &pubkey_len);

        res = (pklen == (uint32_t) crypto_sign_ed25519_PUBLICKEYBYTES);
        if (res == FALSE) {
          pr_trace_msg(trace_channel, 8,
           "Ed25519 public key length (%lu bytes) does not match expected "
           "length (%lu bytes)", (unsigned long) pklen,
           (unsigned long) crypto_sign_ed25519_PUBLICKEYBYTES);
        }
      }
      break;
    }
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
    case SFTP_KEY_ED448: {
      char *pkey_type;

      pkey_type = sftp_msg_read_string(p, &pubkey_data, &pubkey_len);
      if (strcmp(pkey_type, "ssh-ed448") != 0) {
        pr_trace_msg(trace_channel, 8,
         "invalid public key type '%s' for Ed448 key", pkey_type);
        res = FALSE;

      } else {
        uint32_t pklen;

        pklen = sftp_msg_read_int(p, &pubkey_data, &pubkey_len);

        res = FALSE;
        if (pklen == (uint32_t) CURVE448_SIZE ||
            pklen == ((uint32_t) CURVE448_SIZE + 1)) {
          res = TRUE;
        }

        if (res == FALSE) {
          pr_trace_msg(trace_channel, 8,
           "Ed448 public key length (%lu bytes) does not match expected "
           "length (%lu bytes)", (unsigned long) pklen,
           (unsigned long) CURVE448_SIZE);
        }
      }
      break;
    }
#endif /* HAVE_X448_OPENSSL */

    default:
      /* No matching public key type/algorithm. */
      errno = ENOENT;
      res = FALSE;
      break;
  }

  if (pkey != NULL) {
    EVP_PKEY_free(pkey);
  }

  return res;
}

static int verify_rsa_signed_data(pool *p, EVP_PKEY *pkey,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen, const EVP_MD *md) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_MD_CTX ctx;
#endif /* prior to OpenSSL-1.1.0 */
  EVP_MD_CTX *pctx;
  RSA *rsa;
  uint32_t len, sig_len;
  unsigned char digest[EVP_MAX_MD_SIZE], *sig;
  unsigned int digest_len = 0, modulus_len = 0;
  int ok = FALSE, res = 0;

  len = sftp_msg_read_int2(p, &signature, &signature_len, &sig_len);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_data2(p, &signature, &signature_len, sig_len, &sig);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying RSA signature: missing signature data");
    errno = EINVAL;
    return -1;
  }

  rsa = EVP_PKEY_get1_RSA(pkey);

  if (keys_rsa_min_nbits > 0) {
    int rsa_nbits;

    rsa_nbits = RSA_size(rsa) * 8;
    if (rsa_nbits < keys_rsa_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "RSA key size (%d bits) less than required minimum (%d bits)",
        rsa_nbits, keys_rsa_min_nbits);
      RSA_free(rsa);

      errno = EINVAL;
      return -1;
    }
  }

  modulus_len = RSA_size(rsa);

  /* If the signature provided by the client is more than the expected
   * key length, the verification will fail.
   */
  if (sig_len > modulus_len) {
    RSA_free(rsa);

    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying RSA signature: signature len (%lu) > RSA modulus "
      "len (%u)", (unsigned long) sig_len, modulus_len);
    errno = EINVAL;
    return -1;
  }

  /* If the signature provided by the client is less than the expected
   * key length, the verification will fail.  In such cases, we need to
   * pad the provided signature with leading zeros (Bug#3992).
   */
  if (sig_len < modulus_len) {
    unsigned int padding_len;
    unsigned char *padded_sig;

    padding_len = modulus_len - sig_len;
    padded_sig = pcalloc(p, modulus_len);

    pr_trace_msg(trace_channel, 12, "padding client-sent RSA signature "
      "(%lu) bytes with %u bytes of zeroed data", (unsigned long) sig_len,
      padding_len);
    memmove(padded_sig + padding_len, sig, sig_len);

    sig = padded_sig;
    sig_len = (uint32_t) modulus_len;
  }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  pctx = EVP_MD_CTX_new();
#else
  pctx = &ctx;
#endif /* prior to OpenSSL-1.1.0 */

  EVP_DigestInit(pctx, md);
  EVP_DigestUpdate(pctx, sig_data, sig_datalen);
  EVP_DigestFinal(pctx, digest, &digest_len);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  EVP_MD_CTX_free(pctx);
#endif /* OpenSSL-1.1.0 and later */

  ok = RSA_verify(EVP_MD_type(md), digest, digest_len, sig, sig_len, rsa);
  if (ok == 1) {
    res = 0;

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying RSA signature: %s", sftp_crypto_get_errors());
    res = -1;
  }

  pr_memscrub(digest, digest_len);
  RSA_free(rsa);
  return res;
}

static int rsa_verify_signed_data(pool *p, EVP_PKEY *pkey,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen) {
  return verify_rsa_signed_data(p, pkey, signature, signature_len,
    sig_data, sig_datalen, EVP_sha1());
}

#if defined(HAVE_SHA256_OPENSSL)
static int rsa_sha256_verify_signed_data(pool *p, EVP_PKEY *pkey,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen) {
  return verify_rsa_signed_data(p, pkey, signature, signature_len,
    sig_data, sig_datalen, EVP_sha256());
}
#endif /* HAVE_SHA256_OPENSSL */

#if defined(HAVE_SHA512_OPENSSL)
static int rsa_sha512_verify_signed_data(pool *p, EVP_PKEY *pkey,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen) {
  return verify_rsa_signed_data(p, pkey, signature, signature_len,
    sig_data, sig_datalen, EVP_sha512());
}
#endif /* HAVE_SHA512_OPENSSL */

#if !defined(OPENSSL_NO_DSA)
static int dsa_verify_signed_data(pool *p, EVP_PKEY *pkey,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_MD_CTX ctx;
#endif /* prior to OpenSSL-1.1.0 */
  EVP_MD_CTX *pctx;
  DSA *dsa;
  DSA_SIG *dsa_sig;
  const BIGNUM *sig_r, *sig_s;
  uint32_t len, sig_len;
  unsigned char digest[EVP_MAX_MD_SIZE], *sig;
  unsigned int digest_len = 0;
  int ok = FALSE, res = 0;

  len = sftp_msg_read_int2(p, &signature, &signature_len, &sig_len);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  /* A DSA signature string is composed of 2 20 character parts. */
  if (sig_len != 40) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "bad DSA signature len (%lu)", (unsigned long) sig_len);
  }

  len = sftp_msg_read_data2(p, &signature, &signature_len, sig_len, &sig);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying DSA signature: missing signature data");
    errno = EINVAL;
    return -1;
  }

  dsa = EVP_PKEY_get1_DSA(pkey);

  if (keys_dsa_min_nbits > 0) {
    int dsa_nbits;

    dsa_nbits = DSA_size(dsa) * 8;
    if (dsa_nbits < keys_dsa_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "DSA key size (%d bits) less than required minimum (%d bits)",
        dsa_nbits, keys_dsa_min_nbits);
      DSA_free(dsa);

      errno = EINVAL;
      return -1;
    }
  }

  dsa_sig = DSA_SIG_new();
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  DSA_SIG_get0(dsa_sig, &sig_r, &sig_s);
#else
  sig_r = dsa_sig->r;
  sig_s = dsa_sig->s;
#endif /* prior to OpenSSL-1.1.0 */

  sig_r = BN_bin2bn(sig, 20, (BIGNUM *) sig_r);
  if (sig_r == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining 'r' DSA signature component: %s",
      sftp_crypto_get_errors());
    DSA_free(dsa);
    DSA_SIG_free(dsa_sig);
    return -1;
  }

  sig_s = BN_bin2bn(sig + 20, 20, (BIGNUM *) sig_s);
  if (sig_s == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error obtaining 's' DSA signature component: %s",
      sftp_crypto_get_errors());
    BN_clear_free((BIGNUM *) sig_r);
    DSA_free(dsa);
    DSA_SIG_free(dsa_sig);
    return -1;
  }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  pctx = EVP_MD_CTX_new();
#else
  pctx = &ctx;
#endif /* prior to OpenSSL-1.1.0 */

  EVP_DigestInit(pctx, EVP_sha1());
  EVP_DigestUpdate(pctx, sig_data, sig_datalen);
  EVP_DigestFinal(pctx, digest, &digest_len);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  EVP_MD_CTX_free(pctx);
#endif /* OpenSSL-1.1.0 and later */

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
# if OPENSSL_VERSION_NUMBER >= 0x10100006L
  DSA_SIG_set0(dsa_sig, (BIGNUM *) sig_r, (BIGNUM *) sig_s);
# else
  /* XXX What to do here? */
# endif /* prior to OpenSSL-1.1.0-pre6 */
#else
  dsa_sig->r = sig_r;
  dsa_sig->s = sig_s;
#endif /* prior to OpenSSL-1.1.0 */

  ok = DSA_do_verify(digest, digest_len, dsa_sig, dsa);
  if (ok == 1) {
    res = 0;

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying DSA signature: %s", sftp_crypto_get_errors());
    res = -1;
  }

  pr_memscrub(digest, digest_len);
  DSA_free(dsa);
  DSA_SIG_free(dsa_sig);
  return res;
}
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
static int ecdsa_verify_signed_data(pool *p, EVP_PKEY *pkey,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen, char *sig_type) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    defined(HAVE_LIBRESSL)
  EVP_MD_CTX ctx;
#endif /* prior to OpenSSL-1.1.0 */
  EVP_MD_CTX *pctx;
  const EVP_MD *md = NULL;
  EC_KEY *ec;
  ECDSA_SIG *ecdsa_sig;
  const BIGNUM *sig_r, *sig_s;
  uint32_t len, sig_len;
  unsigned char digest[EVP_MAX_MD_SIZE], *sig;
  unsigned int digest_len = 0;
  int ok = FALSE, res = 0;

  if (keys_ec_min_nbits > 0) {
    int ec_nbits;

    ec_nbits = EVP_PKEY_bits(pkey) * 8;
    if (ec_nbits < keys_ec_min_nbits) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "EC key size (%d bits) less than required minimum (%d bits)",
        ec_nbits, keys_ec_min_nbits);
      errno = EINVAL;
      return -1;
    }
  }

  len = sftp_msg_read_int2(p, &signature, &signature_len, &sig_len);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_data2(p, &signature, &signature_len, sig_len, &sig);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying ECDSA signature: missing signature data");
    errno = EINVAL;
    return -1;
  }

  ecdsa_sig = ECDSA_SIG_new();
  if (ecdsa_sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error allocating new ECDSA_SIG: %s", sftp_crypto_get_errors());
    return -1;
  }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  ECDSA_SIG_get0(ecdsa_sig, &sig_r, &sig_s);
#else
  sig_r = ecdsa_sig->r;
  sig_s = ecdsa_sig->s;
#endif /* prior to OpenSSL-1.1.0 */

  len = sftp_msg_read_mpint2(p, &sig, &sig_len, &sig_r);
  if (len == 0) {
    ECDSA_SIG_free(ecdsa_sig);
    errno = EINVAL;
    return -1;
  }

  if (sig_r == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading 'r' ECDSA signature component: %s",
      sftp_crypto_get_errors());
    ECDSA_SIG_free(ecdsa_sig);
    return -1;
  }

  len = sftp_msg_read_mpint2(p, &sig, &sig_len, &sig_s);
  if (len == 0) {
    ECDSA_SIG_free(ecdsa_sig);
    errno = EINVAL;
    return -1;
  }

  if (sig_s == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error reading 's' ECDSA signature component: %s",
      sftp_crypto_get_errors());
    ECDSA_SIG_free(ecdsa_sig);
    return -1;
  }

  /* Skip past the common leading prefix "ecdsa-sha2-" to compare just
   * last 9 characters.
   */

  if (strncmp(sig_type + 11, "nistp256", 9) == 0) {
    md = EVP_sha256();

  } else if (strncmp(sig_type + 11, "nistp384", 9) == 0) {
    md = EVP_sha384();

  } else if (strncmp(sig_type + 11, "nistp521", 9) == 0) {
    md = EVP_sha512();
  }

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  pctx = EVP_MD_CTX_new();
#else
  pctx = &ctx;
#endif /* prior to OpenSSL-1.1.0 */

  EVP_DigestInit(pctx, md);
  EVP_DigestUpdate(pctx, sig_data, sig_datalen);
  EVP_DigestFinal(pctx, digest, &digest_len);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
  EVP_MD_CTX_free(pctx);
#endif /* OpenSSL-1.1.0 and later */

  ec = EVP_PKEY_get1_EC_KEY(pkey);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && \
    !defined(HAVE_LIBRESSL)
# if OPENSSL_VERSION_NUMBER >= 0x10100006L
  ECDSA_SIG_set0(ecdsa_sig, (BIGNUM *) sig_r, (BIGNUM *) sig_s);
# else
  /* XXX What to do here? */
# endif /* prior to OpenSSL-1.1.0-pre6 */
#else
  ecdsa_sig->r = sig_r;
  ecdsa_sig->s = sig_s;
#endif /* prior to OpenSSL-1.1.0 */

  ok = ECDSA_do_verify(digest, digest_len, ecdsa_sig, ec);
  if (ok == 1) {
    res = 0;

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying ECDSA signature: %s", sftp_crypto_get_errors());
    res = -1;
  }

  pr_memscrub(digest, digest_len);
  EC_KEY_free(ec);
  ECDSA_SIG_free(ecdsa_sig);
  return res;
}
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
static int ed25519_verify_signed_data(pool *p,
    unsigned char *pubkey_data, uint32_t pubkey_datalen,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen) {
  char *pkey_type;
  uint32_t len, public_keylen, sig_len;
  unsigned char *msg, *public_key, *signed_msg, *sig;
  unsigned long long msg_len, signed_msglen;
  int res;

  len = sftp_msg_read_string2(p, &pubkey_data, &pubkey_datalen, &pkey_type);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (strcmp(pkey_type, "ssh-ed25519") != 0) {
    pr_trace_msg(trace_channel, 17,
      "public key type '%s' does not match expected key type 'ssh-ed25519'",
      pkey_type);
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_int2(p, &pubkey_data, &pubkey_datalen, &public_keylen);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (public_keylen != crypto_sign_ed25519_PUBLICKEYBYTES) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "invalid Ed25519 public key length (%lu bytes), expected %lu bytes",
      (unsigned long) public_keylen,
      (unsigned long) crypto_sign_ed25519_PUBLICKEYBYTES);
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_data2(p, &pubkey_data, &pubkey_datalen, public_keylen,
    &public_key);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_int2(p, &signature, &signature_len, &sig_len);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_data2(p, &signature, &signature_len, sig_len, &sig);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying Ed25519 signature: missing signature data");
    errno = EINVAL;
    return -1;
  }

  if (sig_len > crypto_sign_ed25519_BYTES) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "Ed25519 signature length (%lu bytes) exceeds valid length (%lu bytes)",
      (unsigned long) sig_len, (unsigned long) crypto_sign_ed25519_BYTES);
    errno = EINVAL;
    return -1;
  }

  signed_msglen = sig_len + sig_datalen;
  signed_msg = palloc(p, signed_msglen);
  memcpy(signed_msg, sig, sig_len);
  memcpy(signed_msg + sig_len, sig_data, sig_datalen);

  msg_len = signed_msglen;
  msg = palloc(p, msg_len);

  res = crypto_sign_ed25519_open(msg, &msg_len, signed_msg, signed_msglen,
    public_key);
  if (res != 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "failed Ed25519 signature verification (%d)", res);
    res = -1;
  }

  if (res == 0) {
    if (msg_len != sig_datalen) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "invalid Ed25519 signature length (%lu bytes), expected %lu bytes",
        (unsigned long) sig_datalen, (unsigned long) msg_len);
      errno = EINVAL;
      res = -1;
    }
  }

  if (res == 0) {
    if (sodium_memcmp(msg, sig_data, msg_len) != 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "invalid Ed25519 signature (mismatched data)");
      errno = EINVAL;
      res = -1;
    }
  }

  pr_memscrub(signed_msg, signed_msglen);
  pr_memscrub(msg, msg_len);
  return res;
}
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
static int ed448_verify_signed_data(pool *p,
    unsigned char *pubkey_data, uint32_t pubkey_datalen,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen) {
  char *pkey_type;
  uint32_t len, public_keylen, sig_len;
  unsigned char *public_key, *sig;
  EVP_MD_CTX *md_ctx = NULL;
  EVP_PKEY *pkey = NULL;

  len = sftp_msg_read_string2(p, &pubkey_data, &pubkey_datalen, &pkey_type);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (strcmp(pkey_type, "ssh-ed448") != 0) {
    pr_trace_msg(trace_channel, 17,
      "public key type '%s' does not match expected key type 'ssh-ed448'",
      pkey_type);
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_int2(p, &pubkey_data, &pubkey_datalen, &public_keylen);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (public_keylen != CURVE448_SIZE &&
      public_keylen != (CURVE448_SIZE + 1)) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "invalid Ed448 public key length (%lu bytes), expected %lu bytes",
      (unsigned long) public_keylen, (unsigned long) CURVE448_SIZE);
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_data2(p, &pubkey_data, &pubkey_datalen, public_keylen,
    &public_key);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_int2(p, &signature, &signature_len, &sig_len);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  len = sftp_msg_read_data2(p, &signature, &signature_len, sig_len, &sig);
  if (len == 0) {
    errno = EINVAL;
    return -1;
  }

  if (sig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error verifying Ed448 signature: missing signature data");
    errno = EINVAL;
    return -1;
  }

  pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED448, NULL, public_key,
    public_keylen);
  if (pkey == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing Ed448 public key: %s", sftp_crypto_get_errors());
    return -1;
  }

  md_ctx = EVP_MD_CTX_new();
  if (EVP_DigestVerifyInit(md_ctx, NULL, NULL, NULL, pkey) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error initializing Ed448 verification: %s", sftp_crypto_get_errors());
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(pkey);
    return -1;
  }

  if (EVP_DigestVerify(md_ctx, sig, sig_len, sig_data, sig_datalen) != 1) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "invalid Ed448 signature (mismatched data)");
    EVP_MD_CTX_free(md_ctx);
    EVP_PKEY_free(pkey);
    errno = EINVAL;
    return -1;
  }

  return 0;
}
#endif /* HAVE_X448_OPENSSL */

int sftp_keys_verify_signed_data(pool *p, const char *pubkey_algo,
    unsigned char *pubkey_data, uint32_t pubkey_datalen,
    unsigned char *signature, uint32_t signature_len,
    unsigned char *sig_data, size_t sig_datalen) {
  EVP_PKEY *pkey = NULL;
  char *sig_type;
  uint32_t len;
  int res = 0;

  if (pubkey_algo == NULL ||
      pubkey_data == NULL ||
      signature == NULL ||
      sig_data == NULL ||
      sig_datalen == 0) {
    errno = EINVAL;
    return -1;
  }

  len = read_pkey_from_data(p, pubkey_data, pubkey_datalen, &pkey, NULL, FALSE);
  if (len == 0) {
    return -1;
  }

  if (strcmp(pubkey_algo, "ssh-dss") == 0) {
    if (sftp_interop_supports_feature(SFTP_SSH2_FEAT_HAVE_PUBKEY_ALGO_IN_DSA_SIG)) {
      len = sftp_msg_read_string2(p, &signature, &signature_len, &sig_type);
      if (len == 0) {
        errno = EINVAL;
        return -1;
      }

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
    len = sftp_msg_read_string2(p, &signature, &signature_len, &sig_type);
    if (len == 0) {
      errno = EINVAL;
      return -1;
    }
  }

  if (strcmp(sig_type, "ssh-rsa") == 0) {
    res = rsa_verify_signed_data(p, pkey, signature, signature_len, sig_data,
      sig_datalen);

#if defined(HAVE_SHA256_OPENSSL)
  } else if (strcmp(sig_type, "rsa-sha2-256") == 0) {
    if (strcmp(pubkey_algo, sig_type) != 0 &&
        strcmp(pubkey_algo, "ssh-rsa") != 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unable to verify signed data: signature type '%s' does not match "
        "publickey algorithm '%s'", sig_type, pubkey_algo);
      errno = EINVAL;
      return -1;
    }

    res = rsa_sha256_verify_signed_data(p, pkey, signature, signature_len,
      sig_data, sig_datalen);
#endif /* HAVE_SHA256_OPENSSL */

#if defined(HAVE_SHA512_OPENSSL)
  } else if (strcmp(sig_type, "rsa-sha2-512") == 0) {
    if (strcmp(pubkey_algo, sig_type) != 0 &&
        strcmp(pubkey_algo, "ssh-rsa") != 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unable to verify signed data: signature type '%s' does not match "
        "publickey algorithm '%s'", sig_type, pubkey_algo);
      errno = EINVAL;
      return -1;
    }

    res = rsa_sha512_verify_signed_data(p, pkey, signature, signature_len,
      sig_data, sig_datalen);
#endif /* HAVE_SHA512_OPENSSL */

#if !defined(OPENSSL_NO_DSA)
  } else if (strcmp(sig_type, "ssh-dss") == 0) {
    res = dsa_verify_signed_data(p, pkey, signature, signature_len, sig_data,
      sig_datalen);
#endif /* !OPENSSL_NO_DSA */

#if defined(PR_USE_OPENSSL_ECC)
  } else if (strcmp(sig_type, "ecdsa-sha2-nistp256") == 0 ||
             strcmp(sig_type, "ecdsa-sha2-nistp384") == 0 ||
             strcmp(sig_type, "ecdsa-sha2-nistp521") == 0) {

    if (strcmp(pubkey_algo, sig_type) != 0) {
      (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
        "unable to verify signed data: public key algorithm '%s' does not "
        "match signature algorithm '%s'", pubkey_algo, sig_type);
      return -1;
    }

    res = ecdsa_verify_signed_data(p, pkey, signature, signature_len, sig_data,
      sig_datalen, sig_type);
#endif /* PR_USE_OPENSSL_ECC */

#if defined(PR_USE_SODIUM)
  } else if (strcmp(sig_type, "ssh-ed25519") == 0) {
    res = ed25519_verify_signed_data(p, pubkey_data, pubkey_datalen, signature,
      signature_len, sig_data, sig_datalen);
#endif /* PR_USE_SODIUM */

#if defined(HAVE_X448_OPENSSL)
  } else if (strcmp(sig_type, "ssh-ed448") == 0) {
    res = ed448_verify_signed_data(p, pubkey_data, pubkey_datalen, signature,
      signature_len, sig_data, sig_datalen);
#endif /* HAVE_X448_OPENSSL */

  } else {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "unable to verify signed data: unsupported signature algorithm '%s'",
      sig_type);
    errno = EINVAL;
    return -1;
  }

  if (pkey != NULL) {
    EVP_PKEY_free(pkey);
  }

  return res;
}

int sftp_keys_set_key_limits(int rsa_min, int dsa_min, int ec_min) {
  /* Ignore any negative values. */

  if (rsa_min >= 0) {
    keys_rsa_min_nbits = (unsigned int) rsa_min;
  }

  if (dsa_min >= 0) {
    keys_dsa_min_nbits = (unsigned int) dsa_min;
  }

  if (ec_min >= 0) {
    keys_ec_min_nbits = (unsigned int) ec_min;
  }

  return 0;
}

/* Send "hostkeys-00@openssh.com" GLOBAL_REQUEST with our hostkeys. */
int sftp_keys_send_hostkeys(pool *p) {
  int res, *nids = NULL;
  pool *tmp_pool;
  struct ssh2_packet *pkt;
  unsigned char *buf, *ptr;
  const unsigned char *hostkey_data = NULL;
  uint32_t buflen, bufsz, hostkey_datalen = 0;

  if (!sftp_interop_supports_feature(SFTP_SSH2_FEAT_HOSTKEYS)) {
    return 0;
  }

  if (sftp_opts & SFTP_OPT_NO_HOSTKEY_ROTATION) {
    return 0;
  }

  tmp_pool = make_sub_pool(p);
  pr_pool_tag(tmp_pool, "hostkeys-00@openssh.com pool");
  pkt = sftp_ssh2_packet_create(tmp_pool);

  /* Allocate 32K, for all of the possible hostkeys. */
  buflen = bufsz = 32768;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_GLOBAL_REQUEST);
  sftp_msg_write_string(&buf, &buflen, "hostkeys-00@openssh.com");
  sftp_msg_write_bool(&buf, &buflen, FALSE);

  res = sftp_keys_have_rsa_hostkey();
  if (res == 0) {
    hostkey_data = sftp_keys_get_hostkey_data(tmp_pool, SFTP_KEY_RSA,
      &hostkey_datalen);
    if (hostkey_data != NULL) {
      pr_trace_msg(trace_channel, 17, "adding RSA hostkey to hostkeys message");
      sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);
    }
  }

  res = sftp_keys_have_dsa_hostkey();
  if (res == 0) {
    hostkey_data = sftp_keys_get_hostkey_data(tmp_pool, SFTP_KEY_DSA,
      &hostkey_datalen);
    if (hostkey_data != NULL) {
      pr_trace_msg(trace_channel, 17, "adding DSA hostkey to hostkeys message");
      sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);
    }
  }

  res = sftp_keys_have_ed25519_hostkey();
  if (res == 0) {
    hostkey_data = sftp_keys_get_hostkey_data(tmp_pool, SFTP_KEY_ED25519,
      &hostkey_datalen);
    if (hostkey_data != NULL) {
      pr_trace_msg(trace_channel, 17,
        "adding Ed25519 hostkey to hostkeys message");
      sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);
    }
  }

  res = sftp_keys_have_ed448_hostkey();
  if (res == 0) {
    hostkey_data = sftp_keys_get_hostkey_data(tmp_pool, SFTP_KEY_ED448,
      &hostkey_datalen);
    if (hostkey_data != NULL) {
      pr_trace_msg(trace_channel, 17,
        "adding Ed448 hostkey to hostkeys message");
      sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);
    }
  }

  res = sftp_keys_have_ecdsa_hostkey(tmp_pool, &nids);
#if defined(PR_USE_OPENSSL_ECC)
  if (res > 0) {
    register int i;

    for (i = 0; i < res; i++) {
      enum sftp_key_type_e key_type;
      const char *key_desc;

      switch (nids[i]) {
        case NID_X9_62_prime256v1:
          key_type = SFTP_KEY_ECDSA_256;
          key_desc = "ECDSA256";
          break;

        case NID_secp384r1:
          key_type = SFTP_KEY_ECDSA_384;
          key_desc = "ECDSA384";
          break;

        case NID_secp521r1:
          key_type = SFTP_KEY_ECDSA_521;
          key_desc = "ECDSA521";
          break;

        default:
          continue;
      }

      hostkey_data = sftp_keys_get_hostkey_data(tmp_pool, key_type,
        &hostkey_datalen);
      if (hostkey_data != NULL) {
        pr_trace_msg(trace_channel, 17,
          "adding %s hostkey to hostkeys message", key_desc);
        sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);
      }
    }
  }
#endif /* PR_USE_OPENSSL_ECC */

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  pr_trace_msg(trace_channel, 17,
    "sending 'hostkeys-00@openssh.com' GLOBAL_REQUEST (%lu bytes)",
    (unsigned long) pkt->payload_len);
  (void) sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
  destroy_pool(tmp_pool);

  return 0;
}

static enum sftp_key_type_e get_hostkey_type(pool *p,
    const unsigned char *hostkey_data, uint32_t hostkey_datalen) {
  unsigned char *buf;
  uint32_t buflen;
  char *key_desc;
  enum sftp_key_type_e key_type = SFTP_KEY_UNKNOWN;

  buf = (unsigned char *) hostkey_data;
  buflen = hostkey_datalen;
  key_desc = sftp_msg_read_string(p, &buf, &buflen);

  if (strcmp(key_desc, "ssh-rsa") == 0) {
    key_type = SFTP_KEY_RSA;

  } else if (strcmp(key_desc, "ssh-dss") == 0) {
    key_type = SFTP_KEY_DSA;

  } else if (strcmp(key_desc, "ssh-ed25519") == 0) {
    key_type = SFTP_KEY_ED25519;

  } else if (strcmp(key_desc, "ssh-ed448") == 0) {
    key_type = SFTP_KEY_ED448;

  } else if (strcmp(key_desc, "ecdsa-sha2-nistp256") == 0) {
    key_type = SFTP_KEY_ECDSA_256;

  } else if (strcmp(key_desc, "ecdsa-sha2-nistp384") == 0) {
    key_type = SFTP_KEY_ECDSA_384;

  } else if (strcmp(key_desc, "ecdsa-sha2-nistp521") == 0) {
    key_type = SFTP_KEY_ECDSA_521;
  }

  return key_type;
}

static int have_hostkey(pool *p, enum sftp_key_type_e hostkey_type,
    const unsigned char *hostkey_data, uint32_t hostkey_datalen) {
  const unsigned char *local_hostkey_data = NULL;
  uint32_t local_hostkey_datalen = 0;

  switch (hostkey_type) {
    case SFTP_KEY_RSA:
    case SFTP_KEY_RSA_SHA256:
    case SFTP_KEY_RSA_SHA512:
      if (sftp_keys_have_rsa_hostkey() == 0) {
        local_hostkey_data = sftp_keys_get_hostkey_data(p, hostkey_type,
          &local_hostkey_datalen);
      }
      break;

    case SFTP_KEY_DSA:
      if (sftp_keys_have_dsa_hostkey() == 0) {
        local_hostkey_data = sftp_keys_get_hostkey_data(p, hostkey_type,
          &local_hostkey_datalen);
      }
      break;

    case SFTP_KEY_ED25519:
      if (sftp_keys_have_ed25519_hostkey() == 0) {
        local_hostkey_data = sftp_keys_get_hostkey_data(p, hostkey_type,
          &local_hostkey_datalen);
      }
      break;

    case SFTP_KEY_ED448:
      if (sftp_keys_have_ed448_hostkey() == 0) {
        local_hostkey_data = sftp_keys_get_hostkey_data(p, hostkey_type,
          &local_hostkey_datalen);
      }
      break;

    case SFTP_KEY_ECDSA_256:
    case SFTP_KEY_ECDSA_384:
    case SFTP_KEY_ECDSA_521:
      if (sftp_keys_have_ecdsa_hostkey(p, NULL) > 0) {
        local_hostkey_data = sftp_keys_get_hostkey_data(p, hostkey_type,
          &local_hostkey_datalen);
      }
      break;

    default:
      /* Unknown. */
      break;
  }

  if (local_hostkey_data == NULL) {
    return -1;
  }

  if (hostkey_datalen != local_hostkey_datalen) {
    return -1;
  }

  if (memcmp(hostkey_data, local_hostkey_data, hostkey_datalen) != 0) {
    return -1;
  }

  return 0;
}

static const unsigned char *prove_hostkey(pool *p,
    const unsigned char *hostkey_data, uint32_t hostkey_datalen,
    size_t *hsiglen) {
  pool *tmp_pool;
  enum sftp_key_type_e hostkey_type;
  const unsigned char *session_id, *hsig;
  unsigned char *buf, *ptr;
  uint32_t buflen, bufsz, session_idsz;

  tmp_pool = make_sub_pool(p);
  pr_pool_tag(tmp_pool, "hostkeys-prove-00@openssh.com pool");

  buflen = bufsz = 8192;
  ptr = buf = palloc(tmp_pool, bufsz);

  sftp_msg_write_string(&buf, &buflen, "hostkeys-prove-00@openssh.com");
  session_idsz = sftp_session_get_id(&session_id);
  sftp_msg_write_data(&buf, &buflen, session_id, session_idsz, TRUE);
  sftp_msg_write_data(&buf, &buflen, hostkey_data, hostkey_datalen, TRUE);

  hostkey_type = get_hostkey_type(tmp_pool, hostkey_data, hostkey_datalen);

  pr_trace_msg(trace_channel, 17, "proving possession of %s hostkey",
    get_key_type_desc(hostkey_type));

  /* Is this even one of our hostkeys? Use the detected hostkey type to
   * make for a faster search.
   */
  if (have_hostkey(tmp_pool, hostkey_type, hostkey_data, hostkey_datalen) < 0) {
    pr_trace_msg(trace_channel, 7, "unknown %s hostkey proof requested",
      get_key_type_desc(hostkey_type));
    destroy_pool(tmp_pool);
    return NULL;
  }

  /* Note that the hostkey type used for signing, for RSA keys, should take
   * into account the KEX hostkey sig type, such as for SHA256/SHA512 signatures
   * for RSA keys.
   */
  if (hostkey_type == SFTP_KEY_RSA) {
    enum sftp_key_type_e kex_hostkey_type;

    kex_hostkey_type = sftp_kex_get_hostkey_type();
    switch (kex_hostkey_type) {
      case SFTP_KEY_RSA:
      case SFTP_KEY_RSA_SHA256:
      case SFTP_KEY_RSA_SHA512:
        hostkey_type = kex_hostkey_type;
        break;

      default:
        /* Ignore */
        break;
    }
  }

  hsig = sftp_keys_sign_data(p, hostkey_type, ptr, (bufsz - buflen), hsiglen);
  if (hsig == NULL) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error proving hostkey");
  }

  destroy_pool(tmp_pool);
  return hsig;
}

static int prove_hostkeys_failed(pool *p) {
  unsigned char *buf, *ptr;
  uint32_t buflen, bufsz;
  struct ssh2_packet *pkt;

  pkt = sftp_ssh2_packet_create(p);
  buflen = bufsz = 256;
  ptr = buf = palloc(pkt->pool, bufsz);

  sftp_msg_write_byte(&buf, &buflen, SFTP_SSH2_MSG_REQUEST_FAILURE);

  pkt->payload = ptr;
  pkt->payload_len = (bufsz - buflen);

  return sftp_ssh2_packet_write(sftp_conn->wfd, pkt);
}

/* Handle "hostkeys-prove-00@openssh.com" GLOBAL_REQUEST from client. */
int sftp_keys_prove_hostkeys(pool *p, int want_reply, unsigned char *buf,
    uint32_t buflen) {
  int res;
  unsigned char *buf2, *ptr2;
  uint32_t buflen2, bufsz2;
  struct ssh2_packet *pkt2;

  /* No point in doing the work, if the client does not care about a reply. */
  if (want_reply == FALSE) {
    return 0;
  }

  if (sftp_opts & SFTP_OPT_NO_HOSTKEY_ROTATION) {
    return prove_hostkeys_failed(p);
  }

  pr_trace_msg(trace_channel, 16,
    "handling 'hostkeys-prove-00@openssh.sh' request (%lu bytes)",
    (unsigned long) buflen);

  pkt2 = sftp_ssh2_packet_create(p);
  buflen2 = bufsz2 = 32768;
  ptr2 = buf2 = palloc(pkt2->pool, bufsz2);

  sftp_msg_write_byte(&buf2, &buflen2, SFTP_SSH2_MSG_REQUEST_SUCCESS);

  while (buflen != 0) {
    const unsigned char *hsig = NULL;
    unsigned char *hostkey_data = NULL;
    uint32_t hostkey_datalen = 0;
    size_t hsiglen = 0;

    pr_signals_handle();

    hostkey_datalen = sftp_msg_read_int(p, &buf, &buflen);
    hostkey_data = sftp_msg_read_data(p, &buf, &buflen, hostkey_datalen);

    hsig = prove_hostkey(p, hostkey_data, hostkey_datalen, &hsiglen);
    if (hsig == NULL) {
      return prove_hostkeys_failed(p);
    }

    sftp_msg_write_data(&buf2, &buflen2, hsig, hsiglen, TRUE);
  }

  pkt2->payload = ptr2;
  pkt2->payload_len = (bufsz2 - buflen2);

  res = sftp_ssh2_packet_write(sftp_conn->wfd, pkt2);
  if (res < 0) {
    (void) pr_log_writefile(sftp_logfd, MOD_SFTP_VERSION,
      "error writing 'hostkeys-prove-00@openssh.com' response: %s",
      strerror(errno));
  }

  return 0;
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
    while (c != NULL) {
      int flags;

      pr_signals_handle();

      flags = *((int *) c->argv[1]);

      /* Skip any agent-provided SFTPHostKey directives, as well as any
       * "disabling key" directives.
       */
      if (flags != 0 ||
          strncmp(c->argv[0], "agent:", 6) == 0) {
        c = find_config_next(c, c->next, CONF_PARAM, "SFTPHostKey", FALSE);
        continue;
      }

      k = pcalloc(s->pool, sizeof(struct sftp_pkey));      
      k->pkeysz = PEM_BUFSIZE-1;
      k->server = s;

      if (get_passphrase(k, c->argv[0]) < 0) {
        int xerrno = errno;
        const char *errstr;

        errstr = sftp_crypto_get_errors();

        pr_log_pri(PR_LOG_WARNING, MOD_SFTP_VERSION
          ": error reading passphrase for SFTPHostKey '%s': %s",
          (const char *) c->argv[0], errstr ? errstr : strerror(xerrno));

        pr_log_pri(PR_LOG_ERR, MOD_SFTP_VERSION
          ": unable to use key in SFTPHostKey '%s', exiting",
          (const char *) c->argv[0]);
        pr_session_disconnect(&sftp_module, PR_SESS_DISCONNECT_BAD_CONFIG,
          NULL);
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

  sftp_keys_clear_dsa_hostkey();
  sftp_keys_clear_ecdsa_hostkey();
  sftp_keys_clear_ed25519_hostkey();
  sftp_keys_clear_ed448_hostkey();
  sftp_keys_clear_rsa_hostkey();
}
