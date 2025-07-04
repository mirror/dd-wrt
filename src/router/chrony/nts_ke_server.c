/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2020, 2022, 2024
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  NTS-KE server
  */

#include "config.h"

#include "sysincl.h"

#include "nts_ke_server.h"

#include "array.h"
#include "conf.h"
#include "clientlog.h"
#include "local.h"
#include "logging.h"
#include "memory.h"
#include "ntp_core.h"
#include "nts_ke_session.h"
#include "privops.h"
#include "siv.h"
#include "socket.h"
#include "sched.h"
#include "sys.h"
#include "util.h"

#define SERVER_TIMEOUT 2.0

#define MAX_COOKIE_NONCE_LENGTH 16

#define KEY_ID_INDEX_BITS 2
#define MAX_SERVER_KEYS (1U << KEY_ID_INDEX_BITS)
#define FUTURE_KEYS 1

#define DUMP_FILENAME "ntskeys"
#define DUMP_IDENTIFIER "NKS1\n"
#define OLD_DUMP_IDENTIFIER "NKS0\n"

#define INVALID_SOCK_FD (-7)

typedef struct {
  uint32_t key_id;
} ServerCookieHeader;

typedef struct {
  uint32_t id;
  unsigned char key[SIV_MAX_KEY_LENGTH];
  SIV_Algorithm siv_algorithm;
  SIV_Instance siv;
  int nonce_length;
} ServerKey;

typedef struct {
  uint32_t key_id;
  uint32_t siv_algorithm;
  unsigned char key[SIV_MAX_KEY_LENGTH];
  IPAddr client_addr;
  uint16_t client_port;
  uint16_t _pad;
} HelperRequest;

/* ================================================== */

static ServerKey server_keys[MAX_SERVER_KEYS];
static int current_server_key;
static double last_server_key_ts;
static int key_rotation_interval;

static int server_sock_fd4;
static int server_sock_fd6;

static int helper_sock_fd;
static int is_helper;

static int initialised = 0;

/* Array of NKSN instances */
static ARR_Instance sessions;
static NKSN_Credentials server_credentials;

/* ================================================== */

static int handle_message(void *arg);

/* ================================================== */

static int
handle_client(int sock_fd, IPSockAddr *addr)
{
  NKSN_Instance inst, *instp;
  int i;

  /* Leave at least half of the descriptors which can handled by select()
     to other use */
  if (sock_fd > FD_SETSIZE / 2) {
    DEBUG_LOG("Rejected connection from %s (%s)",
              UTI_IPSockAddrToString(addr), "too many descriptors");
    return 0;
  }

  /* Find an unused server slot or one with an already stopped session */
  for (i = 0, inst = NULL; i < ARR_GetSize(sessions); i++) {
    instp = ARR_GetElement(sessions, i);
    if (!*instp) {
      /* NULL handler arg will be replaced with the session instance */
      inst = NKSN_CreateInstance(1, NULL, handle_message, NULL);
      *instp = inst;
      break;
    } else if (NKSN_IsStopped(*instp)) {
      inst = *instp;
      break;
    }
  }

  if (!inst) {
    DEBUG_LOG("Rejected connection from %s (%s)",
              UTI_IPSockAddrToString(addr), "too many connections");
    return 0;
  }

  assert(server_credentials);

  if (!NKSN_StartSession(inst, sock_fd, UTI_IPSockAddrToString(addr),
                         server_credentials, SERVER_TIMEOUT))
    return 0;

  return 1;
}

/* ================================================== */

static void
update_key_siv(ServerKey *key, SIV_Algorithm algorithm)
{
  if (!key->siv || key->siv_algorithm != algorithm) {
    if (key->siv)
      SIV_DestroyInstance(key->siv);
    key->siv_algorithm = algorithm;
    key->siv = SIV_CreateInstance(algorithm);
    key->nonce_length = MIN(SIV_GetMaxNonceLength(key->siv), MAX_COOKIE_NONCE_LENGTH);
  }

  if (!key->siv || !SIV_SetKey(key->siv, key->key, SIV_GetKeyLength(key->siv_algorithm)))
    LOG_FATAL("Could not set SIV key");
}

/* ================================================== */

static void
handle_helper_request(int fd, int event, void *arg)
{
  SCK_Message *message;
  HelperRequest *req;
  IPSockAddr client_addr;
  ServerKey *key;
  int sock_fd;

  /* Receive the helper request with the NTS-KE session socket.
     With multiple helpers EAGAIN errors are expected here. */
  message = SCK_ReceiveMessage(fd, SCK_FLAG_MSG_DESCRIPTOR);
  if (!message)
    return;

  sock_fd = message->descriptor;
  if (sock_fd < 0) {
    /* Message with no descriptor is a shutdown command */
    SCH_QuitProgram();
    return;
  }

  if (!initialised) {
    DEBUG_LOG("Uninitialised helper");
    SCK_CloseSocket(sock_fd);
    return;
  }

  if (message->length != sizeof (HelperRequest))
    LOG_FATAL("Invalid helper request");

  req = message->data;

  /* Extract the current server key and client address from the request */
  key = &server_keys[current_server_key];
  key->id = ntohl(req->key_id);
  assert(sizeof (key->key) == sizeof (req->key));
  memcpy(key->key, req->key, sizeof (key->key));
  UTI_IPNetworkToHost(&req->client_addr, &client_addr.ip_addr);
  client_addr.port = ntohs(req->client_port);

  update_key_siv(key, ntohl(req->siv_algorithm));

  if (!handle_client(sock_fd, &client_addr)) {
    SCK_CloseSocket(sock_fd);
    return;
  }

  DEBUG_LOG("Accepted helper request fd=%d", sock_fd);
}

/* ================================================== */

static void
accept_connection(int listening_fd, int event, void *arg)
{
  SCK_Message message;
  IPSockAddr addr;
  int log_index, sock_fd;
  struct timespec now;

  sock_fd = SCK_AcceptConnection(listening_fd, &addr);
  if (sock_fd < 0)
    return;

  if (!NCR_CheckAccessRestriction(&addr.ip_addr)) {
    DEBUG_LOG("Rejected connection from %s (%s)",
              UTI_IPSockAddrToString(&addr), "access denied");
    SCK_CloseSocket(sock_fd);
    return;
  }

  SCH_GetLastEventTime(&now, NULL, NULL);

  log_index = CLG_LogServiceAccess(CLG_NTSKE, &addr.ip_addr, &now);
  if (log_index >= 0 && CLG_LimitServiceRate(CLG_NTSKE, log_index) != CLG_PASS) {
    DEBUG_LOG("Rejected connection from %s (%s)",
              UTI_IPSockAddrToString(&addr), "rate limit");
    SCK_CloseSocket(sock_fd);
    return;
  }

  /* Pass the socket to a helper process if enabled.  Otherwise, handle the
     client in the main process. */
  if (helper_sock_fd != INVALID_SOCK_FD) {
    HelperRequest req;

    memset(&req, 0, sizeof (req));

    /* Include the current server key and client address in the request */
    req.key_id = htonl(server_keys[current_server_key].id);
    req.siv_algorithm = htonl(server_keys[current_server_key].siv_algorithm);
    assert(sizeof (req.key) == sizeof (server_keys[current_server_key].key));
    memcpy(req.key, server_keys[current_server_key].key, sizeof (req.key));
    UTI_IPHostToNetwork(&addr.ip_addr, &req.client_addr);
    req.client_port = htons(addr.port);

    SCK_InitMessage(&message, SCK_ADDR_UNSPEC);
    message.data = &req;
    message.length = sizeof (req);
    message.descriptor = sock_fd;

    errno = 0;
    if (!SCK_SendMessage(helper_sock_fd, &message, SCK_FLAG_MSG_DESCRIPTOR)) {
      /* If sending failed with EPIPE, it means all helpers closed their end of
         the socket (e.g. due to a fatal error) */
      if (errno == EPIPE)
        LOG_FATAL("NTS-KE helpers failed");
      SCK_CloseSocket(sock_fd);
      return;
    }

    SCK_CloseSocket(sock_fd);
  } else {
    if (!handle_client(sock_fd, &addr)) {
      SCK_CloseSocket(sock_fd);
      return;
    }
  }

  DEBUG_LOG("Accepted connection from %s fd=%d", UTI_IPSockAddrToString(&addr), sock_fd);
}

/* ================================================== */

static int
open_socket(int family)
{
  IPSockAddr local_addr;
  int backlog, sock_fd;
  char *iface;

  if (!SCK_IsIpFamilyEnabled(family))
    return INVALID_SOCK_FD;

  CNF_GetBindAddress(family, &local_addr.ip_addr);
  local_addr.port = CNF_GetNtsServerPort();
  iface = CNF_GetBindNtpInterface();

  sock_fd = SCK_OpenTcpSocket(NULL, &local_addr, iface, 0);
  if (sock_fd < 0) {
    LOG(LOGS_ERR, "Could not open NTS-KE socket on %s", UTI_IPSockAddrToString(&local_addr));
    return INVALID_SOCK_FD;
  }

  /* Set the maximum number of waiting connections on the socket to the maximum
     number of concurrent sessions */
  backlog = MAX(CNF_GetNtsServerProcesses(), 1) * CNF_GetNtsServerConnections();

  if (!SCK_ListenOnSocket(sock_fd, backlog)) {
    SCK_CloseSocket(sock_fd);
    return INVALID_SOCK_FD;
  }

  SCH_AddFileHandler(sock_fd, SCH_FILE_INPUT, accept_connection, NULL);

  return sock_fd;
}

/* ================================================== */

static void
helper_signal(int x)
{
  SCH_QuitProgram();
}

/* ================================================== */

static int
prepare_response(NKSN_Instance session, int error, int next_protocol, int aead_algorithm,
                 int compliant_128gcm)
{
  SIV_Algorithm exporter_algorithm;
  NKE_Context context;
  NKE_Cookie cookie;
  char *ntp_server;
  uint16_t datum;
  int i;

  DEBUG_LOG("NTS KE response: error=%d next=%d aead=%d", error, next_protocol, aead_algorithm);

  NKSN_BeginMessage(session);

  if (error >= 0) {
    datum = htons(error);
    if (!NKSN_AddRecord(session, 1, NKE_RECORD_ERROR, &datum, sizeof (datum)))
      return 0;
  } else if (next_protocol < 0) {
    if (!NKSN_AddRecord(session, 1, NKE_RECORD_NEXT_PROTOCOL, NULL, 0))
      return 0;
  } else if (aead_algorithm < 0) {
    datum = htons(next_protocol);
    if (!NKSN_AddRecord(session, 1, NKE_RECORD_NEXT_PROTOCOL, &datum, sizeof (datum)))
      return 0;
    if (!NKSN_AddRecord(session, 1, NKE_RECORD_AEAD_ALGORITHM, NULL, 0))
      return 0;
  } else {
    datum = htons(next_protocol);
    if (!NKSN_AddRecord(session, 1, NKE_RECORD_NEXT_PROTOCOL, &datum, sizeof (datum)))
      return 0;

    datum = htons(aead_algorithm);
    if (!NKSN_AddRecord(session, 1, NKE_RECORD_AEAD_ALGORITHM, &datum, sizeof (datum)))
      return 0;

    if (aead_algorithm == AEAD_AES_128_GCM_SIV && compliant_128gcm) {
      if (!NKSN_AddRecord(session, 0, NKE_RECORD_COMPLIANT_128GCM_EXPORT, NULL, 0))
        return 0;
    }

    if (CNF_GetNTPPort() != NTP_PORT) {
      datum = htons(CNF_GetNTPPort());
      if (!NKSN_AddRecord(session, 1, NKE_RECORD_NTPV4_PORT_NEGOTIATION, &datum, sizeof (datum)))
        return 0;
    }

    ntp_server = CNF_GetNtsNtpServer();
    if (ntp_server) {
      if (!NKSN_AddRecord(session, 1, NKE_RECORD_NTPV4_SERVER_NEGOTIATION,
                          ntp_server, strlen(ntp_server)))
        return 0;
    }

    context.algorithm = aead_algorithm;
    exporter_algorithm = aead_algorithm;

    /* With AES-128-GCM-SIV, set the algorithm ID in the RFC5705 key exporter
       context incorrectly for compatibility with older chrony clients unless
       the client requested the compliant context */
    if (exporter_algorithm == AEAD_AES_128_GCM_SIV && !compliant_128gcm)
      exporter_algorithm = AEAD_AES_SIV_CMAC_256;

    if (!NKSN_GetKeys(session, aead_algorithm, exporter_algorithm,
                      NKE_NEXT_PROTOCOL_NTPV4, &context.c2s, &context.s2c))
      return 0;

    for (i = 0; i < NKE_MAX_COOKIES; i++) {
      if (!NKS_GenerateCookie(&context, &cookie))
        return 0;
      if (!NKSN_AddRecord(session, 0, NKE_RECORD_COOKIE, cookie.cookie, cookie.length))
        return 0;
    }
  }

  if (!NKSN_EndMessage(session))
    return 0;

  return 1;
}

/* ================================================== */

static int
process_request(NKSN_Instance session)
{
  int next_protocol_records = 0, aead_algorithm_records = 0;
  int next_protocol_values = 0, aead_algorithm_values = 0;
  int next_protocol = -1, aead_algorithm = -1, error = -1;
  int i, j, critical, type, length;
  int compliant_128gcm = 0;
  uint16_t data[NKE_MAX_RECORD_BODY_LENGTH / sizeof (uint16_t)];

  assert(NKE_MAX_RECORD_BODY_LENGTH % sizeof (uint16_t) == 0);
  assert(sizeof (uint16_t) == 2);

  while (error < 0) {
    if (!NKSN_GetRecord(session, &critical, &type, &length, &data, sizeof (data)))
      break;

    switch (type) {
      case NKE_RECORD_NEXT_PROTOCOL:
        if (!critical || length < 2 || length % 2 != 0) {
          error = NKE_ERROR_BAD_REQUEST;
          break;
        }

        next_protocol_records++;

        for (i = 0; i < MIN(length, sizeof (data)) / 2; i++) {
          next_protocol_values++;
          if (ntohs(data[i]) == NKE_NEXT_PROTOCOL_NTPV4)
            next_protocol = NKE_NEXT_PROTOCOL_NTPV4;
        }
        break;
      case NKE_RECORD_AEAD_ALGORITHM:
        if (length < 2 || length % 2 != 0) {
          error = NKE_ERROR_BAD_REQUEST;
          break;
        }

        aead_algorithm_records++;

        for (i = 0; i < MIN(length, sizeof (data)) / 2; i++) {
          aead_algorithm_values++;
          /* Use the first enabled and supported algorithm */
          for (j = 0; j < ARR_GetSize(CNF_GetNtsAeads()); j++) {
            if (ntohs(data[i]) == *(int *)ARR_GetElement(CNF_GetNtsAeads(), j) &&
                aead_algorithm < 0 && SIV_GetKeyLength(ntohs(data[i])) > 0)
              aead_algorithm = ntohs(data[i]);
          }
        }
        break;
      case NKE_RECORD_COMPLIANT_128GCM_EXPORT:
        if (length != 0) {
          error = NKE_ERROR_BAD_REQUEST;
          break;
        }
        compliant_128gcm = 1;
        break;
      case NKE_RECORD_ERROR:
      case NKE_RECORD_WARNING:
      case NKE_RECORD_COOKIE:
        error = NKE_ERROR_BAD_REQUEST;
        break;
      default:
        if (critical)
          error = NKE_ERROR_UNRECOGNIZED_CRITICAL_RECORD;
    }
  }

  if (error < 0) {
    if (next_protocol_records != 1 || next_protocol_values < 1 ||
        (next_protocol == NKE_NEXT_PROTOCOL_NTPV4 &&
         (aead_algorithm_records != 1 || aead_algorithm_values < 1)))
      error = NKE_ERROR_BAD_REQUEST;
  }

  if (!prepare_response(session, error, next_protocol, aead_algorithm, compliant_128gcm))
    return 0;

  return 1;
}

/* ================================================== */

static int
handle_message(void *arg)
{
  NKSN_Instance session = arg;

  return process_request(session);
}

/* ================================================== */

static void
generate_key(int index)
{
  SIV_Algorithm algorithm;
  ServerKey *key;
  int key_length;

  BRIEF_ASSERT(index >= 0 && index < MAX_SERVER_KEYS);

  /* Prefer AES-128-GCM-SIV if available.  Note that if older keys loaded
     from ntsdumpdir use a different algorithm, responding to NTP requests
     with cookies encrypted with those keys will not work if the new algorithm
     produces longer cookies (i.e. response would be longer than request).
     Switching from AES-SIV-CMAC-256 to AES-128-GCM-SIV is ok. */
  algorithm = SIV_GetKeyLength(AEAD_AES_128_GCM_SIV) > 0 ?
              AEAD_AES_128_GCM_SIV : AEAD_AES_SIV_CMAC_256;

  key = &server_keys[index];

  key_length = SIV_GetKeyLength(algorithm);
  BRIEF_ASSERT(key_length <= sizeof (key->key));

  UTI_GetRandomBytesUrandom(key->key, key_length);
  if (key_length < sizeof (key->key))
    memset(key->key + key_length, 0, sizeof (key->key) - key_length);
  UTI_GetRandomBytes(&key->id, sizeof (key->id));

  /* Encode the index in the lowest bits of the ID */
  key->id &= -1U << KEY_ID_INDEX_BITS;
  key->id |= index;

  update_key_siv(key, algorithm);

  DEBUG_LOG("Generated key %08"PRIX32" (%d)", key->id, (int)key->siv_algorithm);

  last_server_key_ts = SCH_GetLastEventMonoTime();
}

/* ================================================== */

static void
save_keys(void)
{
  char buf[SIV_MAX_KEY_LENGTH * 2 + 1], *dump_dir;
  int i, index, key_length;
  double last_key_age;
  FILE *f;

  /* Don't save the keys if rotation is disabled to enable an external
     management of the keys (e.g. share them with another server) */
  if (key_rotation_interval == 0)
    return;

  dump_dir = CNF_GetNtsDumpDir();
  if (!dump_dir)
    return;

  f = UTI_OpenFile(dump_dir, DUMP_FILENAME, ".tmp", 'w', 0600);
  if (!f)
    return;

  last_key_age = SCH_GetLastEventMonoTime() - last_server_key_ts;

  if (fprintf(f, "%s%.1f\n", DUMP_IDENTIFIER, last_key_age) < 0)
    goto error;

  for (i = 0; i < MAX_SERVER_KEYS; i++) {
    index = (current_server_key + i + 1 + FUTURE_KEYS) % MAX_SERVER_KEYS;
    key_length = SIV_GetKeyLength(server_keys[index].siv_algorithm);

    if (key_length > sizeof (server_keys[index].key) ||
        !UTI_BytesToHex(server_keys[index].key, key_length, buf, sizeof (buf)) ||
        fprintf(f, "%08"PRIX32" %s %d\n", server_keys[index].id, buf,
                (int)server_keys[index].siv_algorithm) < 0)
      goto error;
  }

  fclose(f);

  /* Rename the temporary file, or remove it if that fails */
  if (!UTI_RenameTempFile(dump_dir, DUMP_FILENAME, ".tmp", NULL)) {
    if (!UTI_RemoveFile(dump_dir, DUMP_FILENAME, ".tmp"))
      ;
  }

  return;

error:
  LOG(LOGS_ERR, "Could not %s %s", "save", "server NTS keys");
  fclose(f);

  if (!UTI_RemoveFile(dump_dir, DUMP_FILENAME, NULL))
    ;
}

/* ================================================== */

#define MAX_WORDS 3

static int
load_keys(void)
{
  int i, index, key_length, algorithm = 0, old_ver;
  char *dump_dir, line[1024], *words[MAX_WORDS];
  ServerKey new_keys[MAX_SERVER_KEYS];
  double key_age;
  FILE *f;

  dump_dir = CNF_GetNtsDumpDir();
  if (!dump_dir)
    return 0;

  f = UTI_OpenFile(dump_dir, DUMP_FILENAME, NULL, 'r', 0);
  if (!f)
    return 0;

  if (!fgets(line, sizeof (line), f) ||
      (strcmp(line, DUMP_IDENTIFIER) != 0 && strcmp(line, OLD_DUMP_IDENTIFIER) != 0))
    goto error;

  old_ver = strcmp(line, DUMP_IDENTIFIER) != 0;

  if (!fgets(line, sizeof (line), f) ||
      UTI_SplitString(line, words, MAX_WORDS) != (old_ver ? 2 : 1) ||
      (old_ver && sscanf(words[0], "%d", &algorithm) != 1) ||
      sscanf(words[old_ver ? 1 : 0], "%lf", &key_age) != 1)
    goto error;

  for (i = 0; i < MAX_SERVER_KEYS && fgets(line, sizeof (line), f); i++) {
    if (UTI_SplitString(line, words, MAX_WORDS) != (old_ver ? 2 : 3) ||
        sscanf(words[0], "%"PRIX32, &new_keys[i].id) != 1 ||
        (!old_ver && sscanf(words[2], "%d", &algorithm) != 1))
      goto error;

    new_keys[i].siv_algorithm = algorithm;
    key_length = SIV_GetKeyLength(algorithm);

    if ((i > 0 && (new_keys[i].id - new_keys[i - 1].id) % MAX_SERVER_KEYS != 1) ||
        key_length <= 0 ||
        UTI_HexToBytes(words[1], new_keys[i].key, sizeof (new_keys[i].key)) != key_length)
      goto error;
    memset(new_keys[i].key + key_length, 0, sizeof (new_keys[i].key) - key_length);
  }

  if (i < MAX_SERVER_KEYS)
    goto error;

  for (i = 0; i < MAX_SERVER_KEYS; i++) {
    index = new_keys[i].id % MAX_SERVER_KEYS;
    server_keys[index].id = new_keys[i].id;
    memcpy(server_keys[index].key, new_keys[i].key, sizeof (server_keys[index].key));

    update_key_siv(&server_keys[index], new_keys[i].siv_algorithm);

    DEBUG_LOG("Loaded key %08"PRIX32" (%d)",
              server_keys[index].id, (int)server_keys[index].siv_algorithm);
  }

  current_server_key = (index + MAX_SERVER_KEYS - FUTURE_KEYS) % MAX_SERVER_KEYS;
  last_server_key_ts = SCH_GetLastEventMonoTime() - MAX(key_age, 0.0);

  fclose(f);

  LOG(LOGS_INFO, "Loaded %s", "server NTS keys");
  return 1;

error:
  LOG(LOGS_ERR, "Could not %s %s", "load", "server NTS keys");
  fclose(f);

  return 0;
}

/* ================================================== */

static void
key_timeout(void *arg)
{
  current_server_key = (current_server_key + 1) % MAX_SERVER_KEYS;
  generate_key((current_server_key + FUTURE_KEYS) % MAX_SERVER_KEYS);
  save_keys();

  SCH_AddTimeoutByDelay(key_rotation_interval, key_timeout, NULL);
}

/* ================================================== */

static void
run_helper(uid_t uid, gid_t gid, int scfilter_level, int sock_fd)
{
  LOG_Severity log_severity;

  /* Finish minimal initialisation and run using the scheduler loop
     similarly to the main process */

  DEBUG_LOG("Helper started");

  SCK_CloseReusableSockets();

  /* Suppress a log message about disabled clock control */
  log_severity = LOG_GetMinSeverity();
  LOG_SetMinSeverity(LOGS_ERR);

  SYS_Initialise(0);
  LOG_SetMinSeverity(log_severity);

  if (!geteuid() && (uid || gid))
    SYS_DropRoot(uid, gid, SYS_NTSKE_HELPER);

  NKS_Initialise();

  UTI_SetQuitSignalsHandler(helper_signal, 1);
  if (scfilter_level != 0)
    SYS_EnableSystemCallFilter(scfilter_level, SYS_NTSKE_HELPER);

  SCH_AddFileHandler(sock_fd, SCH_FILE_INPUT, handle_helper_request, NULL);

  SCH_MainLoop();

  DEBUG_LOG("Helper exiting");

  SCH_RemoveFileHandler(sock_fd);
  close(sock_fd);

  NKS_Finalise();
  SCK_Finalise();
  SYS_Finalise();
  SCH_Finalise();
  LCL_Finalise();
  PRV_Finalise();
  CNF_Finalise();
  LOG_Finalise();

  UTI_ResetGetRandomFunctions();

  exit(0);
}

/* ================================================== */

void
NKS_PreInitialise(uid_t uid, gid_t gid, int scfilter_level)
{
  int i, processes, sock_fd1, sock_fd2;
  const char **certs, **keys;
  char prefix[16];
  pid_t pid;

  helper_sock_fd = INVALID_SOCK_FD;
  is_helper = 0;

  if (CNF_GetNtsServerCertAndKeyFiles(&certs, &keys) <= 0)
    return;

  processes = CNF_GetNtsServerProcesses();
  if (processes <= 0)
    return;

  /* Start helper processes to perform (computationally expensive) NTS-KE
     sessions with clients on sockets forwarded from the main process */

  sock_fd1 = SCK_OpenUnixSocketPair(0, &sock_fd2);
  if (sock_fd1 < 0)
    LOG_FATAL("Could not open socket pair");

  for (i = 0; i < processes; i++) {
    pid = fork();

    if (pid < 0)
      LOG_FATAL("fork() failed : %s", strerror(errno));

    if (pid > 0)
      continue;

    is_helper = 1;

    UTI_ResetGetRandomFunctions();

    snprintf(prefix, sizeof (prefix), "nks#%d:", i + 1);
    LOG_SetDebugPrefix(prefix);
    LOG_CloseParentFd();

    SCK_CloseSocket(sock_fd1);

    run_helper(uid, gid, scfilter_level, sock_fd2);
  }

  SCK_CloseSocket(sock_fd2);
  helper_sock_fd = sock_fd1;
}

/* ================================================== */

void
NKS_Initialise(void)
{
  const char **certs, **keys;
  int i, n_certs_keys;
  double key_delay;

  server_sock_fd4 = INVALID_SOCK_FD;
  server_sock_fd6 = INVALID_SOCK_FD;

  n_certs_keys = CNF_GetNtsServerCertAndKeyFiles(&certs, &keys);
  if (n_certs_keys <= 0)
    return;

  if (helper_sock_fd == INVALID_SOCK_FD) {
    server_credentials = NKSN_CreateServerCertCredentials(certs, keys, n_certs_keys);
    if (!server_credentials)
      return;
  } else {
    server_credentials = NULL;
  }

  sessions = ARR_CreateInstance(sizeof (NKSN_Instance));
  for (i = 0; i < CNF_GetNtsServerConnections(); i++)
    *(NKSN_Instance *)ARR_GetNewElement(sessions) = NULL;

  /* Generate random keys, even if they will be replaced by reloaded keys,
     or unused (in the helper) */
  for (i = 0; i < MAX_SERVER_KEYS; i++) {
    server_keys[i].siv = NULL;
    generate_key(i);
  }

  current_server_key = MAX_SERVER_KEYS - 1;

  if (!is_helper) {
    server_sock_fd4 = open_socket(IPADDR_INET4);
    server_sock_fd6 = open_socket(IPADDR_INET6);

    key_rotation_interval = MAX(CNF_GetNtsRotate(), 0);

    /* Reload saved keys, or save the new keys */
    if (!load_keys())
      save_keys();

    if (key_rotation_interval > 0) {
      key_delay = key_rotation_interval - (SCH_GetLastEventMonoTime() - last_server_key_ts);
      SCH_AddTimeoutByDelay(MAX(key_delay, 0.0), key_timeout, NULL);
    }

    /* Warn if keys are not saved, which can cause a flood of requests
       after server restart */
    if (!CNF_GetNtsDumpDir())
      LOG(LOGS_WARN, "No ntsdumpdir to save server keys");
  }

  initialised = 1;
}

/* ================================================== */

void
NKS_Finalise(void)
{
  int i;

  if (!initialised)
    return;

  if (helper_sock_fd != INVALID_SOCK_FD) {
    /* Send the helpers a request to exit */
    for (i = 0; i < CNF_GetNtsServerProcesses(); i++) {
      if (!SCK_Send(helper_sock_fd, "", 1, 0))
        ;
    }
    SCK_CloseSocket(helper_sock_fd);
  }
  if (server_sock_fd4 != INVALID_SOCK_FD)
    SCK_CloseSocket(server_sock_fd4);
  if (server_sock_fd6 != INVALID_SOCK_FD)
    SCK_CloseSocket(server_sock_fd6);

  if (!is_helper)
    save_keys();

  for (i = 0; i < MAX_SERVER_KEYS; i++)
    SIV_DestroyInstance(server_keys[i].siv);

  for (i = 0; i < ARR_GetSize(sessions); i++) {
    NKSN_Instance session = *(NKSN_Instance *)ARR_GetElement(sessions, i);
    if (session)
      NKSN_DestroyInstance(session);
  }
  ARR_DestroyInstance(sessions);

  if (server_credentials)
    NKSN_DestroyCertCredentials(server_credentials);
}

/* ================================================== */

void
NKS_DumpKeys(void)
{
  save_keys();
}

/* ================================================== */

void
NKS_ReloadKeys(void)
{
  /* Don't load the keys if they are expected to be generated by this server
     instance (i.e. they are already loaded) to not delay the next rotation */
  if (key_rotation_interval > 0)
    return;

  load_keys();
}

/* ================================================== */

/* A server cookie consists of key ID, nonce, and encrypted C2S+S2C keys */

int
NKS_GenerateCookie(NKE_Context *context, NKE_Cookie *cookie)
{
  unsigned char *nonce, plaintext[2 * NKE_MAX_KEY_LENGTH], *ciphertext;
  int plaintext_length, tag_length;
  ServerCookieHeader *header;
  ServerKey *key;

  if (!initialised) {
    DEBUG_LOG("NTS server disabled");
    return 0;
  }

  /* The AEAD ID is not encoded in the cookie.  It is implied from the key
     length (as long as only algorithms with different key lengths are
     supported). */

  if (context->c2s.length < 0 || context->c2s.length > NKE_MAX_KEY_LENGTH ||
      context->s2c.length != context->c2s.length) {
    DEBUG_LOG("Invalid key length");
    return 0;
  }

  key = &server_keys[current_server_key];

  header = (ServerCookieHeader *)cookie->cookie;

  header->key_id = htonl(key->id);

  nonce = cookie->cookie + sizeof (*header);
  BRIEF_ASSERT(key->nonce_length <= sizeof (cookie->cookie) - sizeof (*header));
  UTI_GetRandomBytes(nonce, key->nonce_length);

  plaintext_length = context->c2s.length + context->s2c.length;
  assert(plaintext_length <= sizeof (plaintext));
  memcpy(plaintext, context->c2s.key, context->c2s.length);
  memcpy(plaintext + context->c2s.length, context->s2c.key, context->s2c.length);

  tag_length = SIV_GetTagLength(key->siv);
  cookie->length = sizeof (*header) + key->nonce_length + plaintext_length + tag_length;
  assert(cookie->length <= sizeof (cookie->cookie));
  ciphertext = cookie->cookie + sizeof (*header) + key->nonce_length;

  if (!SIV_Encrypt(key->siv, nonce, key->nonce_length,
                   "", 0,
                   plaintext, plaintext_length,
                   ciphertext, plaintext_length + tag_length)) {
    DEBUG_LOG("Could not encrypt cookie");
    return 0;
  }

  return 1;
}

/* ================================================== */

int
NKS_DecodeCookie(NKE_Cookie *cookie, NKE_Context *context)
{
  unsigned char *nonce, plaintext[2 * NKE_MAX_KEY_LENGTH], *ciphertext;
  int ciphertext_length, plaintext_length, tag_length;
  ServerCookieHeader *header;
  ServerKey *key;
  uint32_t key_id;

  if (!initialised) {
    DEBUG_LOG("NTS server disabled");
    return 0;
  }

  if (cookie->length <= (int)sizeof (*header)) {
    DEBUG_LOG("Invalid cookie length");
    return 0;
  }

  header = (ServerCookieHeader *)cookie->cookie;

  key_id = ntohl(header->key_id);
  key = &server_keys[key_id % MAX_SERVER_KEYS];
  if (key_id != key->id) {
    DEBUG_LOG("Unknown key %"PRIX32, key_id);
    return 0;
  }

  tag_length = SIV_GetTagLength(key->siv);

  if (cookie->length <= (int)sizeof (*header) + key->nonce_length + tag_length) {
    DEBUG_LOG("Invalid cookie length");
    return 0;
  }

  nonce = cookie->cookie + sizeof (*header);
  ciphertext = cookie->cookie + sizeof (*header) + key->nonce_length;
  ciphertext_length = cookie->length - sizeof (*header) - key->nonce_length;
  plaintext_length = ciphertext_length - tag_length;

  if (plaintext_length > sizeof (plaintext) || plaintext_length % 2 != 0) {
    DEBUG_LOG("Invalid cookie length");
    return 0;
  }

  if (!SIV_Decrypt(key->siv, nonce, key->nonce_length,
                   "", 0,
                   ciphertext, ciphertext_length,
                   plaintext, plaintext_length)) {
    DEBUG_LOG("Could not decrypt cookie");
    return 0;
  }

  /* Select a supported algorithm corresponding to the key length, avoiding
     potentially slow SIV_GetKeyLength() */
  switch (plaintext_length / 2) {
    case 16:
      context->algorithm = AEAD_AES_128_GCM_SIV;
      break;
    case 32:
      context->algorithm = AEAD_AES_SIV_CMAC_256;
      break;
    default:
      DEBUG_LOG("Unknown key length");
      return 0;
  }

  context->c2s.length = plaintext_length / 2;
  context->s2c.length = plaintext_length / 2;
  assert(context->c2s.length <= sizeof (context->c2s.key));

  memcpy(context->c2s.key, plaintext, context->c2s.length);
  memcpy(context->s2c.key, plaintext + context->c2s.length, context->s2c.length);

  return 1;
}
