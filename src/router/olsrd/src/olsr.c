/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

/**
 * All these functions are global
 */

#include "defs.h"
#include "builddata.h"
#include "olsr.h"
#include "link_set.h"
#include "two_hop_neighbor_table.h"
#include "tc_set.h"
#include "duplicate_set.h"
#include "mpr_selector_set.h"
#include "mid_set.h"
#include "mpr.h"
#include "lq_mpr.h"
#include "olsr_spf.h"
#include "scheduler.h"
#include "apm.h"
#include "misc.h"
#include "neighbor_table.h"
#include "log.h"
#include "lq_packet.h"
#include "common/avl.h"
#include "net_olsr.h"
#include "lq_plugin.h"
#include "gateway.h"
#include "duplicate_handler.h"
#include "olsr_random.h"

#include <stdarg.h>
#include <signal.h>
#include <unistd.h>

bool changes_topology;
bool changes_neighborhood;
bool changes_hna;
bool changes_force;

/*COLLECT startup sleeps caused by warnings*/

#ifdef OLSR_COLLECT_STARTUP_SLEEP
static int max_startup_sleep = 0;
#endif /* OLSR_COLLECT_STARTUP_SLEEP */
static int sum_startup_sleep = 0;

void olsr_startup_sleep(int s)
{
  sum_startup_sleep += s;
#ifdef OLSR_COLLECT_STARTUP_SLEEP
  if (s > max_startup_sleep) max_startup_sleep=s;
#else /* OLSR_COLLECT_STARTUP_SLEEP */
  sleep(s);
#endif /* OLSR_COLLECT_STARTUP_SLEEP */
}

void olsr_do_startup_sleep(void)
{
#ifdef OLSR_COLLECT_STARTUP_SLEEP
  if (sum_startup_sleep > max_startup_sleep)
    printf("OLSR encountered multiple problems on startup, which should delay startup by %i seconds.\nAs this is quite much time, OLSR will sleep only %i seconds.\nBUT YOU SHOULD FIX ABOVE PROBLEMS!\n",
           sum_startup_sleep,max_startup_sleep);
  sleep(max_startup_sleep);
#else /* OLSR_COLLECT_STARTUP_SLEEP */
  if (sum_startup_sleep > 0) 
    printf("olsrd startup was delayed %i seconds due to various nasty error messages.\nYOU SHOULD REALLY FIX ABOVE PROBLEMS!\n",
           sum_startup_sleep);
#endif /* OLSR_COLLECT_STARTUP_SLEEP */
}

/**
 * Process changes functions
 */

struct pcf {
  int (*function) (int, int, int);
  struct pcf *next;
};

static struct pcf *pcf_list;

static uint16_t message_seqno;
union olsr_ip_addr all_zero;

/**
 *Initialize the message sequence number as a random value
 */
void
init_msg_seqno(void)
{
  message_seqno = olsr_random() & 0xFFFF;
}

/**
 * Get and increment the message sequence number
 *
 *@return the seqno
 */
uint16_t
get_msg_seqno(void)
{
  return message_seqno++;
}

bool
olsr_is_bad_duplicate_msg_seqno(uint16_t seqno) {
  int32_t diff = (int32_t) seqno - (int32_t) message_seqno;

  if (diff < -32768) {
    diff += 65536;
  }
  else if (diff > 32767) {
    diff -= 65536;
  }
  return diff > 0;
}

void
register_pcf(int (*f) (int, int, int))
{
  struct pcf *new_pcf;

  OLSR_PRINTF(1, "Registering pcf function\n");

  new_pcf = olsr_malloc(sizeof(struct pcf), "New PCF");

  new_pcf->function = f;
  new_pcf->next = pcf_list;
  pcf_list = new_pcf;

}

/**
 *Process changes in neighborhood or/and topology.
 *Re-calculates the neighborhood/topology if there
 *are any updates - then calls the right functions to
 *update the routing table.
 *@return 0
 */
void
olsr_process_changes(void)
{
  struct pcf *tmp_pc_list;

#ifdef DEBUG
  if (changes_neighborhood)
    OLSR_PRINTF(3, "CHANGES IN NEIGHBORHOOD\n");
  if (changes_topology)
    OLSR_PRINTF(3, "CHANGES IN TOPOLOGY\n");
  if (changes_hna)
    OLSR_PRINTF(3, "CHANGES IN HNA\n");
#endif /* DEBUG */

  if (!changes_neighborhood && !changes_topology && !changes_hna)
    return;

  if (olsr_cnf->debug_level > 0 && olsr_cnf->clear_screen && isatty(1)) {
    clear_console();
    printf("       *** %s (%s on %s) ***\n", olsrd_version, build_date, build_host);
  }

  if (changes_neighborhood) {
    if (olsr_cnf->lq_level < 1) {
      olsr_calculate_mpr();
    } else {
      olsr_calculate_lq_mpr();
    }
  }

  /* calculate the routing table */
  if (changes_neighborhood || changes_topology || changes_hna) {
    olsr_calculate_routing_table(false);
  }

  if (olsr_cnf->debug_level > 0) {
    if (olsr_cnf->debug_level > 2) {
      olsr_print_mid_set();
#ifdef __linux__
    olsr_print_gateway_entries();
#endif /* __linux__ */

      if (olsr_cnf->debug_level > 3) {
        if (olsr_cnf->debug_level > 8) {
          olsr_print_duplicate_table();
        }
        olsr_print_hna_set();
      }
    }
    olsr_print_link_set();
    olsr_print_neighbor_table();
    olsr_print_two_hop_neighbor_table();
    if (olsr_cnf->debug_level > 3) {
      olsr_print_tc_table();
    }
  }

  for (tmp_pc_list = pcf_list; tmp_pc_list != NULL; tmp_pc_list = tmp_pc_list->next) {
    tmp_pc_list->function(changes_neighborhood, changes_topology, changes_hna);
  }

  changes_neighborhood = false;
  changes_topology = false;
  changes_hna = false;
  changes_force = false;
}

/**
 *Initialize all the tables used(neighbor,
 *topology, MID,  HNA, MPR, dup).
 *Also initalizes other variables
 */
void
olsr_init_tables(void)
{
  changes_topology = false;
  changes_neighborhood = false;
  changes_hna = false;

  /* Set avl tree comparator */
  if (olsr_cnf->ipsize == 4) {
    avl_comp_default = avl_comp_ipv4;
    avl_comp_prefix_default = avl_comp_ipv4_prefix;
  } else {
    avl_comp_default = avl_comp_ipv6;
    avl_comp_prefix_default = avl_comp_ipv6_prefix;
  }

  /* Initialize lq plugin set */
  init_lq_handler_tree();

  /* Initialize link set */
  olsr_init_link_set();

  /* Initialize duplicate table */
  olsr_init_duplicate_set();

  /* Initialize neighbor table */
  olsr_init_neighbor_table();

  /* Initialize routing table */
  olsr_init_routing_table();

  /* Initialize two hop table */
  olsr_init_two_hop_table();

  /* Initialize topology */
  olsr_init_tc();

  /* Initialize mpr selector table */
  olsr_init_mprs_set();

  /* Initialize MID set */
  olsr_init_mid_set();

  /* Initialize HNA set */
  olsr_init_hna_set();

  /* Initialize duplicate handler */
#ifndef NO_DUPLICATE_DETECTION_HANDLER
  olsr_duplicate_handler_init();
#endif /* NO_DUPLICATE_DETECTION_HANDLER */
}

/**
 *Check if a message is to be forwarded and forward
 *it if necessary.
 *
 *@param m the OLSR message to be forwarded
 *@param in_if the incoming interface
 *@param from_addr neighbour we received message from
 *
 *@returns positive if forwarded
 */
int
olsr_forward_message(union olsr_message *m, struct interface_olsr *in_if, union olsr_ip_addr *from_addr)
{
  union olsr_ip_addr *src;
  struct neighbor_entry *neighbor;
  int msgsize;
  struct interface_olsr *ifn;
  bool is_ttl_1 = false;

  /*
   * Sven-Ola: We should not flood the mesh with overdue messages. Because
   * of a bug in parser.c:parse_packet, we have a lot of messages because
   * all older olsrd's have lq_fish enabled.
   */
  if (AF_INET == olsr_cnf->ip_version) {
    if (m->v4.ttl < 2 || 255 < (int)m->v4.hopcnt + (int)m->v4.ttl)
      is_ttl_1 = true;
  } else {
    if (m->v6.ttl < 2 || 255 < (int)m->v6.hopcnt + (int)m->v6.ttl)
      is_ttl_1 = true;
  }

  /* Lookup sender address */
  src = mid_lookup_main_addr(from_addr);
  if (!src)
    src = from_addr;

  neighbor = olsr_lookup_neighbor_table(src);
  if (!neighbor)
    return 0;

  if (neighbor->status != SYM)
    return 0;

  /* Check MPR */
  if (olsr_lookup_mprs_set(src) == NULL) {
#ifdef DEBUG
    struct ipaddr_str buf;
    OLSR_PRINTF(5, "Forward - sender %s not MPR selector\n", olsr_ip_to_string(&buf, src));
#endif /* DEBUG */
    return 0;
  }

  if (olsr_message_is_duplicate(m)) {
    return 0;
  }

  /* Treat TTL hopcnt except for ethernet link */
  if (!is_ttl_1) {
    if (olsr_cnf->ip_version == AF_INET) {
      /* IPv4 */
      m->v4.hopcnt++;
      m->v4.ttl--;
    } else {
      /* IPv6 */
      m->v6.hopcnt++;
      m->v6.ttl--;
    }
  }

  /* Update packet data */
  msgsize = ntohs(m->v4.olsr_msgsize);

  /* looping trough interfaces */
  for (ifn = ifnet; ifn; ifn = ifn->int_next) {
    /* do not retransmit out through the same interface if it has mode == ether */
    if (ifn == in_if && ifn->mode == IF_MODE_ETHER) continue;

    /* do not forward TTL 1 messages to non-ether interfaces */
    if (is_ttl_1 && ifn->mode != IF_MODE_ETHER) continue;

    if (net_output_pending(ifn)) {
      /*
       * Check if message is to big to be piggybacked
       */
      if (net_outbuffer_push(ifn, m, msgsize) != msgsize) {
        /* Send */
        net_output(ifn);
        /* Buffer message */
        set_buffer_timer(ifn);

        if (net_outbuffer_push(ifn, m, msgsize) != msgsize) {
          OLSR_PRINTF(1, "Received message to big to be forwarded in %s(%d bytes)!", ifn->int_name, msgsize);
          olsr_syslog(OLSR_LOG_ERR, "Received message to big to be forwarded on %s(%d bytes)!", ifn->int_name, msgsize);
        }
      }
    } else {
      /* No forwarding pending */
      set_buffer_timer(ifn);

      if (net_outbuffer_push(ifn, m, msgsize) != msgsize) {
        OLSR_PRINTF(1, "Received message to big to be forwarded in %s(%d bytes)!", ifn->int_name, msgsize);
        olsr_syslog(OLSR_LOG_ERR, "Received message to big to be forwarded on %s(%d bytes)!", ifn->int_name, msgsize);
      }
    }
  }
  return 1;
}

void
set_buffer_timer(struct interface_olsr *ifn)
{
  /* Set timer */
  ifn->fwdtimer = GET_TIMESTAMP(olsr_random() * olsr_cnf->max_jitter * MSEC_PER_SEC / OLSR_RANDOM_MAX);
}

void
olsr_init_willingness(void)
{
  if (olsr_cnf->willingness_auto) {

    /* Run it first and then periodic. */
    olsr_update_willingness(NULL);

    olsr_start_timer((unsigned int)olsr_cnf->will_int * MSEC_PER_SEC, 5, OLSR_TIMER_PERIODIC, &olsr_update_willingness, NULL, 0);
  }
}

void
olsr_update_willingness(void *foo __attribute__ ((unused)))
{
  int tmp_will = olsr_cnf->willingness;

  /* Re-calculate willingness */
  olsr_cnf->willingness = olsr_calculate_willingness();

  if (tmp_will != olsr_cnf->willingness) {
    OLSR_PRINTF(1, "Local willingness updated: old %d new %d\n", tmp_will, olsr_cnf->willingness);
  }
}

/**
 *Calculate this nodes willingness to act as a MPR
 *based on either a fixed value or the power status
 *of the node using APM
 *
 *@return a 8bit value from 0-7 representing the willingness
 */

uint8_t
olsr_calculate_willingness(void)
{
  struct olsr_apm_info ainfo;

  /* If fixed willingness */
  if (!olsr_cnf->willingness_auto)
    return olsr_cnf->willingness;

  if (apm_read(&ainfo) < 1)
    return WILL_DEFAULT;

  apm_printinfo(&ainfo);

  /* If AC powered */
  if (ainfo.ac_line_status == OLSR_AC_POWERED)
    return 6;

  /* If battery powered
   *
   * juice > 78% will: 3
   * 78% > juice > 26% will: 2
   * 26% > juice will: 1
   */
  return (ainfo.battery_percentage / 26);
}

const char *
olsr_msgtype_to_string(uint8_t msgtype)
{
  static char type[20];

  switch (msgtype) {
  case (HELLO_MESSAGE):
    return "HELLO";
  case (TC_MESSAGE):
    return "TC";
  case (MID_MESSAGE):
    return "MID";
  case (HNA_MESSAGE):
    return "HNA";
  case (LQ_HELLO_MESSAGE):
    return ("LQ-HELLO");
  case (LQ_TC_MESSAGE):
    return ("LQ-TC");
  default:
    break;
  }

  snprintf(type, sizeof(type), "UNKNOWN(%d)", msgtype);
  return type;
}

const char *
olsr_link_to_string(uint8_t linktype)
{
  static char type[20];

  switch (linktype) {
  case (UNSPEC_LINK):
    return "UNSPEC";
  case (ASYM_LINK):
    return "ASYM";
  case (SYM_LINK):
    return "SYM";
  case (LOST_LINK):
    return "LOST";
  case (HIDE_LINK):
    return "HIDE";
  default:
    break;
  }

  snprintf(type, sizeof(type), "UNKNOWN(%d)", linktype);
  return type;
}

const char *
olsr_status_to_string(uint8_t status)
{
  static char type[20];

  switch (status) {
  case (NOT_NEIGH):
    return "NOT NEIGH";
  case (SYM_NEIGH):
    return "NEIGHBOR";
  case (MPR_NEIGH):
    return "MPR";
  default:
    break;
  }

  snprintf(type, sizeof(type), "UNKNOWN(%d)", status);
  return type;
}

/**
 *Termination function to be called whenever a error occures
 *that requires the daemon to terminate
 *
 *@param msg the message to write to the syslog and possibly stdout
 *@param val the exit code
 */

void
olsr_exit(const char *msg, int val)
{
  if (msg) {
    OLSR_PRINTF(1, "OLSR EXIT: %s\n", msg);
    olsr_syslog(OLSR_LOG_ERR, "olsrd exit: %s\n", msg);
  }
  fflush(stdout);
  fflush(stderr);

  if (olsr_cnf) {
    olsr_cnf->exit_value = val;
  }

  raise(SIGTERM);

  /* in case the signal handler was not setup yet */
  exit(val);
}

/**
 * Wrapper for malloc(3) that does error-checking
 *
 * @param size the number of bytes to allocate
 * @param id a string identifying the caller for
 * use in error messaging
 *
 * @return a void pointer to the memory allocated
 */
void *
olsr_malloc(size_t size, const char *id)
{
  void *ptr;

  /*
   * Not all the callers do a proper cleaning of memory.
   * Clean it on behalf of those.
   */
  ptr = calloc(1, size);

  if (!ptr) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s: out of memory!: %s\n", id, strerror(errno));
    olsr_exit(buf, EXIT_FAILURE);
  }

  return ptr;
}

/**
 * Wrapper for realloc(3) that does error-checking
 *
 * @param ptr pointer to the buffer
 * @param size the number of bytes to (re)allocate
 * @param id a string identifying the caller for
 * use in error messaging
 *
 * @return a void pointer to the memory allocated
 */
void *
olsr_realloc(void * ptr, size_t size, const char *id)
{
  ptr = realloc(ptr, size);
  if (!ptr) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s: out of memory!: %s\n", id, strerror(errno));
    olsr_exit(buf, EXIT_FAILURE);
  }

  return ptr;
}

/**
 *Wrapper for printf that prints to a specific
 *debuglevel upper limit
 *
 */

int
olsr_printf(int loglevel, const char *format, ...)
{
  if ((loglevel <= olsr_cnf->debug_level) && debug_handle) {
    va_list arglist;
    va_start(arglist, format);
    vfprintf(debug_handle, format, arglist);
    va_end(arglist);
  }
  return 0;
}

/*
 * Local Variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
