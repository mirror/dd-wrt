/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2016
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

  Support for MS-SNTP authentication in Samba (ntp_signd)
  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "conf.h"
#include "logging.h"
#include "ntp_io.h"
#include "ntp_signd.h"
#include "sched.h"
#include "util.h"

/* Declarations per samba/source4/librpc/idl/ntp_signd.idl */

#define SIGND_VERSION 0

typedef enum {
  SIGN_TO_CLIENT = 0,
  ASK_SERVER_TO_SIGN = 1,
  CHECK_SERVER_SIGNATURE = 2,
  SIGNING_SUCCESS = 3,
  SIGNING_FAILURE = 4,
} SigndOp;

typedef struct {
  uint32_t length;
  uint32_t version;
  uint32_t op;
  uint16_t packet_id;
  uint16_t _pad;
  uint32_t key_id;
  NTP_Packet packet_to_sign;
} SigndRequest;

typedef struct {
  uint32_t length;
  uint32_t version;
  uint32_t op;
  uint32_t packet_id;
  NTP_Packet signed_packet;
} SigndResponse;

typedef struct {
  NTP_Remote_Address remote_addr;
  NTP_Local_Address local_addr;

  int sent;
  int received;
  int request_length;
  struct timespec request_ts;
  SigndRequest request;
  SigndResponse response;
} SignInstance;

/* As the communication with ntp_signd is asynchronous, incoming packets are
   saved in a queue in order to avoid loss when they come in bursts */

#define MAX_QUEUE_LENGTH 16U
#define NEXT_QUEUE_INDEX(index) (((index) + 1) % MAX_QUEUE_LENGTH)
#define IS_QUEUE_EMPTY() (queue_head == queue_tail)

/* Fixed-size array of SignInstance */
static ARR_Instance queue;
static unsigned int queue_head;
static unsigned int queue_tail;

#define INVALID_SOCK_FD -1

/* Unix domain socket connected to ntp_signd */
static int sock_fd;

#define MIN_AUTH_DELAY 1.0e-5
#define MAX_AUTH_DELAY 1.0e-2

/* Average time needed for signing one packet.  This is used to adjust the
   transmit timestamp in NTP packets.  The timestamp won't be very accurate as
   the delay is variable, but it should be good enough for MS-SNTP clients. */
static double auth_delay;

/* Flag indicating if the MS-SNTP authentication is enabled */
static int enabled;

/* ================================================== */

static void read_write_socket(int sock_fd, int event, void *anything);

/* ================================================== */

static void
close_socket(void)
{
  SCH_RemoveFileHandler(sock_fd);
  close(sock_fd);
  sock_fd = INVALID_SOCK_FD;

  /* Empty the queue */
  queue_head = queue_tail = 0;
}

/* ================================================== */

static int
open_socket(void)
{
  struct sockaddr_un s;

  if (sock_fd >= 0)
    return 1;

  sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    DEBUG_LOG("Could not open signd socket : %s", strerror(errno));
    return 0;
  }

  UTI_FdSetCloexec(sock_fd);
  SCH_AddFileHandler(sock_fd, SCH_FILE_INPUT, read_write_socket, NULL);

  s.sun_family = AF_UNIX;
  if (snprintf(s.sun_path, sizeof (s.sun_path), "%s/socket",
               CNF_GetNtpSigndSocket()) >= sizeof (s.sun_path)) {
    DEBUG_LOG("signd socket path too long");
    close_socket();
    return 0;
  }

  if (connect(sock_fd, (struct sockaddr *)&s, sizeof (s)) < 0) {
    DEBUG_LOG("Could not connect to signd : %s", strerror(errno));
    close_socket();
    return 0;
  }

  DEBUG_LOG("Connected to signd");

  return 1;
}

/* ================================================== */

static void
process_response(SignInstance *inst)
{
  struct timespec ts;
  double delay;

  if (ntohs(inst->request.packet_id) != ntohl(inst->response.packet_id)) {
    DEBUG_LOG("Invalid response ID");
    return;
  }

  if (ntohl(inst->response.op) != SIGNING_SUCCESS) {
    DEBUG_LOG("Signing failed");
    return;
  }

  /* Check if the file descriptor is still valid */
  if (!NIO_IsServerSocket(inst->local_addr.sock_fd)) {
    DEBUG_LOG("Invalid NTP socket");
    return;
  }

  SCH_GetLastEventTime(NULL, NULL, &ts);
  delay = UTI_DiffTimespecsToDouble(&ts, &inst->request_ts);

  DEBUG_LOG("Signing succeeded (delay %f)", delay);

  /* Send the signed NTP packet */
  NIO_SendPacket(&inst->response.signed_packet, &inst->remote_addr, &inst->local_addr,
                 ntohl(inst->response.length) + sizeof (inst->response.length) -
                 offsetof(SigndResponse, signed_packet), 0);

  /* Update exponential moving average of the authentication delay */
  delay = CLAMP(MIN_AUTH_DELAY, delay, MAX_AUTH_DELAY);
  auth_delay += 0.1 * (delay - auth_delay);
}

/* ================================================== */

static void
read_write_socket(int sock_fd, int event, void *anything)
{
  SignInstance *inst;
  uint32_t response_length;
  int s;

  inst = ARR_GetElement(queue, queue_head);

  if (event == SCH_FILE_OUTPUT) {
    assert(!IS_QUEUE_EMPTY());
    assert(inst->sent < inst->request_length);

    if (!inst->sent)
      SCH_GetLastEventTime(NULL, NULL, &inst->request_ts);

    s = send(sock_fd, (char *)&inst->request + inst->sent,
             inst->request_length - inst->sent, 0);

    if (s < 0) {
      DEBUG_LOG("signd socket error: %s", strerror(errno));
      close_socket();
      return;
    }

    DEBUG_LOG("Sent %d bytes to signd", s);
    inst->sent += s;

    /* Try again later if the request is not complete yet */
    if (inst->sent < inst->request_length)
      return;

    /* Disable output and wait for a response */
    SCH_SetFileHandlerEvent(sock_fd, SCH_FILE_OUTPUT, 0);
  }

  if (event == SCH_FILE_INPUT) {
    if (IS_QUEUE_EMPTY()) {
        DEBUG_LOG("Unexpected signd response");
        close_socket();
        return;
    }

    assert(inst->received < sizeof (inst->response));
    s = recv(sock_fd, (char *)&inst->response + inst->received,
             sizeof (inst->response) - inst->received, 0);

    if (s <= 0) {
      if (s < 0)
        DEBUG_LOG("signd socket error: %s", strerror(errno));
      else
        DEBUG_LOG("signd socket closed");

      close_socket();
      return;
    }

    DEBUG_LOG("Received %d bytes from signd", s);
    inst->received += s;

    if (inst->received < sizeof (inst->response.length))
      return;

    response_length = ntohl(inst->response.length) + sizeof (inst->response.length);

    if (response_length < offsetof(SigndResponse, signed_packet) ||
        response_length > sizeof (SigndResponse)) {
      DEBUG_LOG("Invalid response length");
      close_socket();
      return;
    }

    /* Wait for more data if not complete yet */
    if (inst->received < response_length)
      return;

    process_response(inst);

    /* Move the head and enable output for the next packet */
    queue_head = NEXT_QUEUE_INDEX(queue_head);
    if (!IS_QUEUE_EMPTY())
      SCH_SetFileHandlerEvent(sock_fd, SCH_FILE_OUTPUT, 1);
  }
}

/* ================================================== */

void
NSD_Initialise()
{
  sock_fd = INVALID_SOCK_FD;
  auth_delay = MIN_AUTH_DELAY;
  enabled = CNF_GetNtpSigndSocket() && CNF_GetNtpSigndSocket()[0];

  if (!enabled)
    return;

  queue = ARR_CreateInstance(sizeof (SignInstance));
  ARR_SetSize(queue, MAX_QUEUE_LENGTH);
  queue_head = queue_tail = 0;

  LOG(LOGS_INFO, "MS-SNTP authentication enabled");
}

/* ================================================== */

void
NSD_Finalise()
{
  if (!enabled)
    return;
  if (sock_fd != INVALID_SOCK_FD)
    close_socket();
  ARR_DestroyInstance(queue);
}

/* ================================================== */

extern int NSD_GetAuthDelay(uint32_t key_id)
{
  return 1.0e9 * auth_delay;
}

/* ================================================== */

int
NSD_SignAndSendPacket(uint32_t key_id, NTP_Packet *packet, NTP_Remote_Address *remote_addr, NTP_Local_Address *local_addr, int length)
{
  SignInstance *inst;

  if (!enabled) {
    DEBUG_LOG("signd disabled");
    return 0;
  }

  if (queue_head == NEXT_QUEUE_INDEX(queue_tail)) {
    DEBUG_LOG("signd queue full");
    return 0;
  }

  if (length != NTP_NORMAL_PACKET_LENGTH) {
    DEBUG_LOG("Invalid packet length");
    return 0;
  }

  if (!open_socket())
    return 0;

  inst = ARR_GetElement(queue, queue_tail);
  inst->remote_addr = *remote_addr;
  inst->local_addr = *local_addr;
  inst->sent = 0;
  inst->received = 0;
  inst->request_length = offsetof(SigndRequest, packet_to_sign) + length;

  /* The length field doesn't include itself */
  inst->request.length = htonl(inst->request_length - sizeof (inst->request.length));
  inst->request.version = htonl(SIGND_VERSION);
  inst->request.op = htonl(SIGN_TO_CLIENT);
  inst->request.packet_id = htons(queue_tail);
  inst->request._pad = 0;
  inst->request.key_id = htonl(key_id);

  memcpy(&inst->request.packet_to_sign, packet, length);

  /* Enable output if there was no pending request */
  if (IS_QUEUE_EMPTY())
    SCH_SetFileHandlerEvent(sock_fd, SCH_FILE_OUTPUT, 1);

  queue_tail = NEXT_QUEUE_INDEX(queue_tail);

  DEBUG_LOG("Packet added to signd queue (%u:%u)", queue_head, queue_tail);

  return 1;
}
