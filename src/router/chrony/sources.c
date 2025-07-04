/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2003
 * Copyright (C) Miroslav Lichvar  2011-2016, 2018, 2020-2025
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

  The routines in this file manage the complete pool of sources that
  we might be synchronizing to.  This includes NTP sources and others
  (e.g. local reference clocks, eyeball + wristwatch etc).

  */

#include "config.h"

#include "sysincl.h"

#include "sources.h"
#include "sourcestats.h"
#include "memory.h"
#include "ntp.h" /* For NTP_Leap */
#include "ntp_sources.h"
#include "local.h"
#include "reference.h"
#include "util.h"
#include "conf.h"
#include "logging.h"
#include "reports.h"
#include "nameserv.h"
#include "sched.h"
#include "regress.h"

/* ================================================== */
/* Flag indicating that we are initialised */
static int initialised = 0;

/* ================================================== */
/* Structure used to hold info for selecting between sources */
struct SelectInfo {
  int select_ok;
  double std_dev;
  double root_distance;
  double lo_limit;
  double hi_limit;
  double last_sample_ago;
};

/* ================================================== */
/* This enum contains the flag values that are used to label
   each source */
typedef enum {
  SRC_OK = 0,           /* OK so far, not a final status! */
  SRC_UNSELECTABLE,     /* Has noselect option set */
  SRC_BAD_STATS,        /* Doesn't have valid stats data */
  SRC_UNSYNCHRONISED,   /* Provides samples, but not synchronised */
  SRC_BAD_DISTANCE,     /* Has root distance longer than allowed maximum */
  SRC_JITTERY,          /* Had std dev larger than allowed maximum */
  SRC_WAITS_STATS,      /* Others have bad stats, selection postponed */
  SRC_STALE,            /* Has older samples than others */
  SRC_ORPHAN,           /* Has stratum equal or larger than orphan stratum */
  SRC_UNTRUSTED,        /* Overlaps trusted sources */
  SRC_FALSETICKER,      /* Doesn't agree with others */
  SRC_WAITS_SOURCES,    /* Not enough sources, selection postponed */
  SRC_NONPREFERRED,     /* Others have prefer option */
  SRC_WAITS_UPDATE,     /* No updates, selection postponed */
  SRC_DISTANT,          /* Others have shorter root distance */
  SRC_OUTLIER,          /* Outlier in clustering (not used yet) */
  SRC_UNSELECTED,       /* Used for synchronisation, not system peer */
  SRC_SELECTED,         /* Used for synchronisation, selected as system peer */
} SRC_Status;

/* ================================================== */
/* Define the instance structure used to hold information about each
   source */
struct SRC_Instance_Record {
  SST_Stats stats;
  int index;                    /* Index back into the array of source */
  uint32_t ref_id;              /* The reference ID of this source
                                   (i.e. from its IP address, NOT the
                                   reference _it_ is sync'd to) */
  IPAddr *ip_addr;              /* Its IP address if NTP source */

  /* Flag indicating that the source is updating reachability */
  int active;

  /* Reachability register */
  int reachability;

  /* Number of set bits in the reachability register */
  int reachability_size;

  /* Updates since last reference update */
  int updates;

  /* Updates left before allowing combining */
  int distant;

  /* Updates with a status requiring source replacement */
  int bad;

  /* Flag indicating the status of the source */
  SRC_Status status;

  /* Type of the source */
  SRC_Type type;

  /* Flag indicating that the source is authenticated */
  int authenticated;

  /* Configured selection options */
  int conf_sel_options;

  /* Effective selection options */
  int sel_options;

  /* Score against currently selected source */
  double sel_score;

  struct SelectInfo sel_info;

  /* Current stratum */
  int stratum;

  /* Current leap status */
  NTP_Leap leap;

  /* Flag indicating the source has a leap second vote */
  int leap_vote;

  /* Flags indicating which status was already reported for
     the source since the last change of the system peer */
  char reported_status[SRC_SELECTED + 1];
};

/* ================================================== */
/* Structure used to build the sort list for finding falsetickers */
struct Sort_Element {
  int index;
  double offset;
  enum {
    LOW = -1,
    HIGH = 1
  } tag;
};

/* ================================================== */
/* Table of sources */
static struct SRC_Instance_Record **sources;
static struct Sort_Element *sort_list;
static int *sel_sources;
static int n_sources; /* Number of sources currently in the table */
static int max_n_sources; /* Capacity of the table */

#define INVALID_SOURCE (-1)
static int selected_source_index; /* Which source index is currently
                                     selected (set to INVALID_SOURCE
                                     if no current valid reference) */
static int reported_no_majority;  /* Flag to avoid repeated log message
                                     about no majority */
static int report_selection_loss; /* Flag to force logging a message if
                                     selection is lost in a transient state
                                     (SRC_WAITS_STATS, SRC_WAITS_UPDATE) */
static int forced_first_report;   /* Flag to allow one failed selection to be
                                     logged before a successful selection */

/* Score needed to replace the currently selected source */
#define SCORE_LIMIT 10.0

/* Number of updates needed to reset the distant status */
#define DISTANT_PENALTY 32

/* Number of updates needed to trigger handling of bad sources */
#define BAD_HANDLE_THRESHOLD 4

static double max_distance;
static double max_jitter;
static double reselect_distance;
static double stratum_weight;
static double combine_limit;

static SRC_Instance last_updated_inst;

static LOG_FileID logfileid;

/* Identifier of the dump file */
#define DUMP_IDENTIFIER "SRC0\n"

/* ================================================== */
/* Forward prototype */

static void update_sel_options(void);
static void unselect_selected_source(LOG_Severity severity, const char *format, ...);
static void slew_sources(struct timespec *raw, struct timespec *cooked, double dfreq,
                         double doffset, LCL_ChangeType change_type, void *anything);
static void add_dispersion(double dispersion, void *anything);
static char *source_to_string(SRC_Instance inst);
static char get_status_char(SRC_Status status);

/* ================================================== */
/* Initialisation function */
void SRC_Initialise(void) {
  sources = NULL;
  sort_list = NULL;
  sel_sources = NULL;
  n_sources = 0;
  max_n_sources = 0;
  selected_source_index = INVALID_SOURCE;
  max_distance = CNF_GetMaxDistance();
  max_jitter = CNF_GetMaxJitter();
  reselect_distance = CNF_GetReselectDistance();
  stratum_weight = CNF_GetStratumWeight();
  combine_limit = CNF_GetCombineLimit();
  initialised = 1;

  LCL_AddParameterChangeHandler(slew_sources, NULL);
  LCL_AddDispersionNotifyHandler(add_dispersion, NULL);

  last_updated_inst = NULL;

  logfileid = CNF_GetLogSelection() ? LOG_FileOpen("selection",
              "   Date (UTC) Time     IP Address   S EOpts Reach Score Last sample  Low end   High end")
              : -1;
}

/* ================================================== */
/* Finalisation function */
void SRC_Finalise(void)
{
  LCL_RemoveParameterChangeHandler(slew_sources, NULL);
  LCL_RemoveDispersionNotifyHandler(add_dispersion, NULL);

  Free(sources);
  Free(sort_list);
  Free(sel_sources);

  initialised = 0;
}

/* ================================================== */
/* Function to create a new instance.  This would be called by one of
   the individual source-type instance creation routines. */

SRC_Instance SRC_CreateNewInstance(uint32_t ref_id, SRC_Type type, int authenticated,
                                   int sel_options, IPAddr *addr, int min_samples,
                                   int max_samples, double min_delay, double asymmetry)
{
  SRC_Instance result;

  assert(initialised);

  if (min_samples == SRC_DEFAULT_MINSAMPLES)
    min_samples = CNF_GetMinSamples();
  if (max_samples == SRC_DEFAULT_MAXSAMPLES)
    max_samples = CNF_GetMaxSamples();

  result = MallocNew(struct SRC_Instance_Record);
  result->stats = SST_CreateInstance(ref_id, addr, min_samples, max_samples,
                                     min_delay, asymmetry);

  if (n_sources == max_n_sources) {
    /* Reallocate memory */
    max_n_sources = max_n_sources > 0 ? 2 * max_n_sources : 4;
    if (sources) {
      sources = ReallocArray(struct SRC_Instance_Record *, max_n_sources, sources);
      sort_list = ReallocArray(struct Sort_Element, 3*max_n_sources, sort_list);
      sel_sources = ReallocArray(int, max_n_sources, sel_sources);
    } else {
      sources = MallocArray(struct SRC_Instance_Record *, max_n_sources);
      sort_list = MallocArray(struct Sort_Element, 3*max_n_sources);
      sel_sources = MallocArray(int, max_n_sources);
    }
  }

  sources[n_sources] = result;

  result->index = n_sources;
  result->type = type;
  result->authenticated = authenticated;
  result->conf_sel_options = sel_options;
  result->sel_options = sel_options;
  result->active = 0;

  SRC_SetRefid(result, ref_id, addr);
  SRC_ResetInstance(result);

  n_sources++;

  update_sel_options();

  return result;
}

/* ================================================== */
/* Function to get rid of a source when it is being unconfigured.
   This may cause the current reference source to be reselected, if this
   was the reference source or contributed significantly to a
   falseticker decision. */

void SRC_DestroyInstance(SRC_Instance instance)
{
  int dead_index, i;

  if (last_updated_inst == instance)
    last_updated_inst = NULL;

  assert(initialised);
  BRIEF_ASSERT(instance->index >= 0 && instance->index < n_sources &&
               instance == sources[instance->index]);

  SST_DeleteInstance(instance->stats);
  dead_index = instance->index;
  for (i=dead_index; i<n_sources-1; i++) {
    sources[i] = sources[i+1];
    sources[i]->index = i;
  }
  --n_sources;
  Free(instance);

  update_sel_options();

  if (selected_source_index > dead_index)
    --selected_source_index;
  else if (selected_source_index == dead_index)
    unselect_selected_source(LOGS_INFO, NULL);

  SRC_SelectSource(NULL);
}

/* ================================================== */

void
SRC_ResetInstance(SRC_Instance instance)
{
  instance->updates = 0;
  instance->reachability = 0;
  instance->reachability_size = 0;
  instance->distant = 0;
  instance->bad = 0;
  instance->status = SRC_BAD_STATS;
  instance->sel_score = 1.0;
  instance->stratum = 0;
  instance->leap = LEAP_Unsynchronised;
  instance->leap_vote = 0;

  memset(instance->reported_status, 0, sizeof (instance->reported_status));
  memset(&instance->sel_info, 0, sizeof (instance->sel_info));

  SST_ResetInstance(instance->stats);

  if (selected_source_index == instance->index)
    SRC_SelectSource(NULL);
}

/* ================================================== */

void
SRC_SetRefid(SRC_Instance instance, uint32_t ref_id, IPAddr *addr)
{
  instance->ref_id = ref_id;
  instance->ip_addr = addr;
  SST_SetRefid(instance->stats, ref_id, addr);
}

/* ================================================== */

SST_Stats
SRC_GetSourcestats(SRC_Instance instance)
{
  assert(initialised);
  return instance->stats;
}

/* ================================================== */

static NTP_Leap
get_leap_status(void)
{
  int i, leap_votes, leap_ins, leap_del;

  /* Accept a leap second if more than half of the sources with a vote agree */

  for (i = leap_ins = leap_del = leap_votes = 0; i < n_sources; i++) {
    if (!sources[i]->leap_vote)
      continue;

    leap_votes++;
    if (sources[i]->leap == LEAP_InsertSecond)
      leap_ins++;
    else if (sources[i]->leap == LEAP_DeleteSecond)
      leap_del++;
  }

  if (leap_ins > leap_votes / 2)
    return LEAP_InsertSecond;
  else if (leap_del > leap_votes / 2)
    return LEAP_DeleteSecond;
  else
    return LEAP_Normal;
}

/* ================================================== */

void
SRC_UpdateStatus(SRC_Instance inst, int stratum, NTP_Leap leap)
{
  inst->stratum = stratum;

  if (REF_IsLeapSecondClose(NULL, 0.0))
    return;

  inst->leap = leap;

  if (inst->leap_vote)
    REF_UpdateLeapStatus(get_leap_status());
}

/* ================================================== */

/* This function is called by one of the source drivers when it has
   a new sample that is to be accumulated.

   This function causes the frequency estimation to be re-run for the
   designated source, and the clock selection procedure to be re-run
   afterwards.
   */

void
SRC_AccumulateSample(SRC_Instance inst, NTP_Sample *sample)
{

  assert(initialised);

  DEBUG_LOG("src=%s ts=%s offset=%e delay=%e disp=%e",
            source_to_string(inst), UTI_TimespecToString(&sample->time), -sample->offset,
            sample->root_delay, sample->root_dispersion);

  if (REF_IsLeapSecondClose(&sample->time, sample->offset)) {
    LOG(LOGS_INFO, "Dropping sample around leap second");
    return;
  }

  SST_AccumulateSample(inst->stats, sample);
  SST_DoNewRegression(inst->stats);
}

/* ================================================== */

void
SRC_SetActive(SRC_Instance inst)
{
  inst->active = 1;
}

/* ================================================== */

void
SRC_UnsetActive(SRC_Instance inst)
{
  inst->active = 0;
}

/* ================================================== */

static int
special_mode_end(void)
{
    int i;

    for (i = 0; i < n_sources; i++) {
      /* No updates from inactive sources */
      if (!sources[i]->active)
        continue;

      /* Don't expect more updates than the initial burst of an NTP source */
      if (sources[i]->reachability_size >= SOURCE_REACH_BITS - 1)
        continue;

      /* Check if the source could still have enough samples to be selectable */
      if (SOURCE_REACH_BITS - 1 - sources[i]->reachability_size +
            SST_Samples(sources[i]->stats) >= MIN_SAMPLES_FOR_REGRESS)
        return 0;
    }

    return 1;
}

/* ================================================== */

static void
handle_bad_source(SRC_Instance inst)
{
  if (inst->type == SRC_NTP) {
    DEBUG_LOG("Bad source status=%c", get_status_char(inst->status));
    NSR_HandleBadSource(inst->ip_addr);
  }
}

/* ================================================== */

void
SRC_UpdateReachability(SRC_Instance inst, int reachable)
{
  inst->reachability <<= 1;
  inst->reachability |= !!reachable;
  inst->reachability %= 1U << SOURCE_REACH_BITS;

  if (inst->reachability_size < SOURCE_REACH_BITS)
      inst->reachability_size++;

  /* Check if special reference update mode failed */
  if (REF_GetMode() != REF_ModeNormal && special_mode_end()) {
    REF_SetUnsynchronised();
  }

  /* Try to replace unreachable NTP sources */
  if (inst->reachability == 0 && inst->reachability_size == SOURCE_REACH_BITS)
    handle_bad_source(inst);

  /* Source selection can change with unreachable sources */
  if (inst->reachability == 0)
    SRC_SelectSource(NULL);
}

/* ================================================== */

void
SRC_ResetReachability(SRC_Instance inst)
{
  inst->reachability = 0;
  inst->reachability_size = 0;
  SRC_UpdateReachability(inst, 0);
}

/* ================================================== */

static void
update_sel_options(void)
{
  int options, auth_ntp_options, unauth_ntp_options, refclk_options;
  int i, auth_ntp_sources, unauth_ntp_sources;

  auth_ntp_sources = unauth_ntp_sources = 0;

  for (i = 0; i < n_sources; i++) {
    if (sources[i]->conf_sel_options & SRC_SELECT_NOSELECT)
      continue;
    if (sources[i]->type != SRC_NTP)
      continue;
    if (sources[i]->authenticated)
      auth_ntp_sources++;
    else
      unauth_ntp_sources++;
  }

  auth_ntp_options = unauth_ntp_options = refclk_options = 0;

  /* Determine which selection options need to be added to authenticated NTP
     sources, unauthenticated NTP sources, and refclocks, to follow the
     configured selection mode */
  switch (CNF_GetAuthSelectMode()) {
    case SRC_AUTHSELECT_IGNORE:
      break;
    case SRC_AUTHSELECT_MIX:
      if (auth_ntp_sources > 0 && unauth_ntp_sources > 0)
        auth_ntp_options = refclk_options = SRC_SELECT_REQUIRE | SRC_SELECT_TRUST;
      break;
    case SRC_AUTHSELECT_PREFER:
      if (auth_ntp_sources > 0)
        unauth_ntp_options = SRC_SELECT_NOSELECT;
      break;
    case SRC_AUTHSELECT_REQUIRE:
      unauth_ntp_options = SRC_SELECT_NOSELECT;
      break;
    default:
      assert(0);
  }

  for (i = 0; i < n_sources; i++) {
    options = sources[i]->conf_sel_options;

    if (!(options & SRC_SELECT_NOSELECT)) {
      switch (sources[i]->type) {
        case SRC_NTP:
          options |= sources[i]->authenticated ? auth_ntp_options : unauth_ntp_options;
          break;
        case SRC_REFCLOCK:
          options |= refclk_options;
          break;
        default:
          assert(0);
      }
    }

    if (sources[i]->sel_options != options) {
      DEBUG_LOG("changing %s from %x to %x", source_to_string(sources[i]),
                (unsigned int)sources[i]->sel_options, (unsigned int)options);
      sources[i]->sel_options = options;
    }
  }
}

/* ================================================== */

FORMAT_ATTRIBUTE_PRINTF(2, 3)
static void
log_selection_message(LOG_Severity severity, const char *format, ...)
{
  char buf[256];
  va_list ap;

  if (REF_GetMode() != REF_ModeNormal)
    return;

  va_start(ap, format);
  vsnprintf(buf, sizeof (buf), format, ap);
  va_end(ap);

  LOG(severity, "%s", buf);
}

/* ================================================== */

FORMAT_ATTRIBUTE_PRINTF(3, 4)
static void
log_selection_source(LOG_Severity severity, SRC_Instance inst, const char *format, ...)
{
  char buf[320], buf2[256], *name, *ntp_name, *s;
  va_list ap;

  if (REF_GetMode() != REF_ModeNormal)
    return;

  va_start(ap, format);
  vsnprintf(buf2, sizeof (buf2), format, ap);
  va_end(ap);

  /* Replace ## with %s in the formatted string to be the source name */
  s = strstr(buf2, "##");
  if (!s || strchr(buf2, '%'))
    return;
  s[0] = '%';
  s[1] = 's';

  name = source_to_string(inst);
  ntp_name = inst->type == SRC_NTP ? NSR_GetName(inst->ip_addr) : NULL;

  if (ntp_name && strcmp(name, ntp_name) != 0)
    snprintf(buf, sizeof (buf), "%s (%s)", name, ntp_name);
  else
    snprintf(buf, sizeof (buf), "%s", name);

  LOG(severity, buf2, buf);

  inst->reported_status[inst->status] = 1;
}

/* ================================================== */

static int
compare_sort_elements(const void *a, const void *b)
{
  const struct Sort_Element *u = (const struct Sort_Element *) a;
  const struct Sort_Element *v = (const struct Sort_Element *) b;

  if (u->offset < v->offset) {
    return -1;
  } else if (u->offset > v->offset) {
    return +1;
  } else if (u->tag < v->tag) {
    return -1;
  } else if (u->tag > v->tag) {
    return +1;
  } else {
    return 0;
  }
}

/* ================================================== */

static char *
source_to_string(SRC_Instance inst)
{
  switch (inst->type) {
    case SRC_NTP:
      return UTI_IPToString(inst->ip_addr);
    case SRC_REFCLOCK:
      return UTI_RefidToString(inst->ref_id);
    default:
      assert(0);
  }
  return NULL;
}

/* ================================================== */

static void
set_source_status(SRC_Instance inst, SRC_Status status)
{
  struct timespec now;

  inst->status = status;

  /* Try to replace NTP sources that are falsetickers, or have a root
     distance or jitter larger than the allowed maximums */
  if (inst == last_updated_inst) {
    if (inst->bad < INT_MAX &&
        (status == SRC_FALSETICKER || status == SRC_BAD_DISTANCE || status == SRC_JITTERY))
      inst->bad++;
    else
      inst->bad = 0;
    if (inst->bad >= BAD_HANDLE_THRESHOLD)
      handle_bad_source(inst);
  }

  DEBUG_LOG("%s status=%c options=%x reach=%o/%d updates=%d distant=%d bad=%d leap=%d vote=%d lo=%f hi=%f",
            source_to_string(inst), get_status_char(inst->status),
            (unsigned int)inst->sel_options, (unsigned int)inst->reachability,
            inst->reachability_size, inst->updates,
            inst->distant, inst->bad, (int)inst->leap, inst->leap_vote,
            inst->sel_info.lo_limit, inst->sel_info.hi_limit);

  if (logfileid == -1)
    return;

  SCH_GetLastEventTime(&now, NULL, NULL);

  LOG_FileWrite(logfileid,
                "%s %-15s %c -%c%c%c%c %4o %5.2f %10.3e %10.3e %10.3e",
                UTI_TimeToLogForm(now.tv_sec), source_to_string(inst),
                get_status_char(inst->status),
                inst->sel_options & SRC_SELECT_NOSELECT ? 'N' : '-',
                inst->sel_options & SRC_SELECT_PREFER ? 'P' : '-',
                inst->sel_options & SRC_SELECT_TRUST ? 'T' : '-',
                inst->sel_options & SRC_SELECT_REQUIRE ? 'R' : '-',
                (unsigned int)inst->reachability, inst->sel_score,
                inst->sel_info.last_sample_ago,
                inst->sel_info.lo_limit, inst->sel_info.hi_limit);
}

/* ================================================== */

static void
mark_source(SRC_Instance inst, SRC_Status status)
{
  set_source_status(inst, status);

  BRIEF_ASSERT(status >= SRC_OK && status < sizeof (inst->reported_status));

  if (!inst->reported_status[status]) {
    switch (status) {
      case SRC_BAD_DISTANCE:
        if (inst->bad < BAD_HANDLE_THRESHOLD)
          break;
        log_selection_source(LOGS_WARN, inst,
                             "Root distance of ## exceeds maxdistance of %.3f seconds",
                             max_distance);
        break;
      case SRC_JITTERY:
        if (inst->bad < BAD_HANDLE_THRESHOLD)
          break;
        log_selection_source(LOGS_WARN, inst,
                             "Jitter of ## exceeds maxjitter of %.3f seconds",
                             max_jitter);
        break;
      case SRC_FALSETICKER:
        log_selection_source(LOGS_WARN, inst, "Detected falseticker ##");
        break;
      case SRC_SELECTED:
        log_selection_source(LOGS_INFO, inst, "Selected source ##");
        break;
      default:
        break;
    }
  }
}

/* ================================================== */

static void
mark_ok_sources(SRC_Status status)
{
  int i;

  for (i = 0; i < n_sources; i++) {
    if (sources[i]->status != SRC_OK)
      continue;
    /* Don't log the status in this case (multiple sources at once) */
    set_source_status(sources[i], status);
  }
}

/* ================================================== */
/* Reset the index of selected source and report the selection loss.  If no
   message is provided, assume it is a transient state and wait for another
   call providing a message or selection of another source, which resets the
   report_selection_loss flag. */

FORMAT_ATTRIBUTE_PRINTF(2, 3)
static void
unselect_selected_source(LOG_Severity severity, const char *format, ...)
{
  char buf[256];
  va_list ap;

  if (selected_source_index != INVALID_SOURCE) {
    selected_source_index = INVALID_SOURCE;
    report_selection_loss = 1;
  }

  if (report_selection_loss && format) {
    va_start(ap, format);
    vsnprintf(buf, sizeof (buf), format, ap);
    va_end(ap);

    log_selection_message(severity, "%s", buf);
    report_selection_loss = 0;
  }
}

/* ================================================== */

static int
combine_sources(int n_sel_sources, struct timespec *ref_time, double *offset,
                double *offset_sd, double *frequency, double *frequency_sd, double *skew)
{
  struct timespec src_ref_time;
  double src_offset, src_offset_sd, src_frequency, src_frequency_sd, src_skew;
  double src_root_delay, src_root_dispersion, sel_src_distance, elapsed;
  double offset_weight, sum_offset_weight, sum_offset, sum2_offset_sd;
  double frequency_weight, sum_frequency_weight, sum_frequency;
  double inv_sum2_frequency_sd, inv_sum2_skew;
  int i, index, combined;

  if (n_sel_sources == 1)
    return 1;

  sum_offset_weight = sum_offset = sum2_offset_sd = 0.0;
  sum_frequency_weight = sum_frequency = inv_sum2_frequency_sd = inv_sum2_skew = 0.0;

  sel_src_distance = sources[selected_source_index]->sel_info.root_distance;
  if (sources[selected_source_index]->type == SRC_NTP)
    sel_src_distance += reselect_distance;

  for (i = combined = 0; i < n_sel_sources; i++) {
    index = sel_sources[i];
    SST_GetTrackingData(sources[index]->stats, &src_ref_time,
                        &src_offset, &src_offset_sd,
                        &src_frequency, &src_frequency_sd, &src_skew,
                        &src_root_delay, &src_root_dispersion);

    /* Don't include this source if its distance is longer than the distance of
       the selected source multiplied by the limit, their estimated frequencies
       are not close, or it was recently marked as distant */

    if (index != selected_source_index &&
        (sources[index]->sel_info.root_distance > combine_limit * sel_src_distance ||
         fabs(*frequency - src_frequency) >
           combine_limit * (*skew + src_skew + LCL_GetMaxClockError()))) {
      /* Use a smaller penalty in first few updates */
      sources[index]->distant = sources[index]->reachability_size >= SOURCE_REACH_BITS ?
                                DISTANT_PENALTY : 1;
    } else if (sources[index]->distant) {
      sources[index]->distant--;
    }

    if (sources[index]->distant) {
      mark_source(sources[index], SRC_DISTANT);
      continue;
    }

    if (sources[index]->status == SRC_OK)
      mark_source(sources[index], SRC_UNSELECTED);

    elapsed = UTI_DiffTimespecsToDouble(ref_time, &src_ref_time);
    src_offset += elapsed * src_frequency;
    src_offset_sd += elapsed * src_frequency_sd;
    offset_weight = 1.0 / sources[index]->sel_info.root_distance;
    frequency_weight = 1.0 / SQUARE(src_frequency_sd);

    DEBUG_LOG("combining %s oweight=%e offset=%e osd=%e fweight=%e freq=%e fsd=%e skew=%e",
              source_to_string(sources[index]), offset_weight, src_offset, src_offset_sd,
              frequency_weight, src_frequency, src_frequency_sd, src_skew);

    sum_offset_weight += offset_weight;
    sum_offset += offset_weight * src_offset;
    sum2_offset_sd += offset_weight * (SQUARE(src_offset_sd) +
                                       SQUARE(src_offset - *offset));

    sum_frequency_weight += frequency_weight;
    sum_frequency += frequency_weight * src_frequency;
    inv_sum2_frequency_sd += 1.0 / SQUARE(src_frequency_sd);
    inv_sum2_skew += 1.0 / SQUARE(src_skew);

    combined++;
  }

  assert(combined);
  *offset = sum_offset / sum_offset_weight;
  *offset_sd = sqrt(sum2_offset_sd / sum_offset_weight);
  *frequency = sum_frequency / sum_frequency_weight;
  *frequency_sd = 1.0 / sqrt(inv_sum2_frequency_sd);
  *skew = 1.0 / sqrt(inv_sum2_skew);

  DEBUG_LOG("combined result offset=%e osd=%e freq=%e fsd=%e skew=%e",
            *offset, *offset_sd, *frequency, *frequency_sd, *skew);

  return combined;
}

/* ================================================== */
/* This function selects the current reference from amongst the pool
   of sources we are holding and updates the local reference */

void
SRC_SelectSource(SRC_Instance updated_inst)
{
  struct SelectInfo *si;
  struct timespec now, ref_time;
  int i, j, j1, j2, index, sel_prefer, n_endpoints, n_sel_sources, sel_req_source;
  int max_badstat_reach, max_badstat_reach_size, n_badstats_sources;
  int max_sel_reach, max_sel_reach_size, n_unreach_sources;
  int depth, best_depth, trust_depth, best_trust_depth, n_sel_trust_sources;
  int combined, stratum, min_stratum, max_score_index;
  int orphan_stratum, orphan_source;
  double src_offset, src_offset_sd, src_frequency, src_frequency_sd, src_skew;
  double src_root_delay, src_root_dispersion;
  double best_lo, best_hi, distance, sel_src_distance, max_score;
  double best_trust_lo, best_trust_hi;
  double first_sample_ago, max_reach_sample_ago;
  NTP_Leap leap_status;

  if (updated_inst) {
    updated_inst->updates++;
    last_updated_inst = updated_inst;
  }

  if (n_sources == 0) {
    unselect_selected_source(LOGS_WARN, "Can't synchronise: no sources");
    return;
  }

  /* This is accurate enough and cheaper than calling LCL_ReadCookedTime */
  SCH_GetLastEventTime(&now, NULL, NULL);

  /* Step 1 - build intervals about each source */

  n_endpoints = 0;
  n_sel_sources = n_sel_trust_sources = 0;
  n_badstats_sources = 0;
  n_unreach_sources = 0;
  sel_req_source = 0;
  max_sel_reach = max_badstat_reach = 0;
  max_sel_reach_size = max_badstat_reach_size = 0;
  max_reach_sample_ago = 0.0;

  for (i = 0; i < n_sources; i++) {
    assert(sources[i]->status != SRC_OK);

    /* Don't allow the source to vote on leap seconds unless it's selectable */
    sources[i]->leap_vote = 0;

    /* If some sources are specified with the require option, at least one
       of them will have to be selectable in order to update the clock */
    if (sources[i]->sel_options & SRC_SELECT_REQUIRE)
      sel_req_source = 1;

    /* Ignore sources which were added with the noselect option */
    if (sources[i]->sel_options & SRC_SELECT_NOSELECT) {
      mark_source(sources[i], SRC_UNSELECTABLE);
      continue;
    }

    /* Count unreachable sources for a warning message */
    if (sources[i]->reachability == 0)
      n_unreach_sources++;

    si = &sources[i]->sel_info;
    SST_GetSelectionData(sources[i]->stats, &now,
                         &si->lo_limit, &si->hi_limit, &si->root_distance,
                         &si->std_dev, &first_sample_ago,
                         &si->last_sample_ago, &si->select_ok);

    if (!si->select_ok) {
      ++n_badstats_sources;
      mark_source(sources[i], SRC_BAD_STATS);
      if (max_badstat_reach < sources[i]->reachability)
        max_badstat_reach = sources[i]->reachability;
      if (max_badstat_reach_size < sources[i]->reachability_size)
        max_badstat_reach_size = sources[i]->reachability_size;
      continue;
    }

    /* Ignore sources which are not synchronised */
    if (sources[i]->leap == LEAP_Unsynchronised) {
      mark_source(sources[i], SRC_UNSYNCHRONISED);
      continue;
    }

    /* Include extra dispersion in the root distance of sources that don't
       have new samples (the last sample is older than span of all samples) */
    if (first_sample_ago < 2.0 * si->last_sample_ago) {
      double extra_disp = LCL_GetMaxClockError() *
                          (2.0 * si->last_sample_ago - first_sample_ago);
      si->root_distance += extra_disp;
      si->lo_limit -= extra_disp;
      si->hi_limit += extra_disp;
    }

    /* Require the root distance to be below the allowed maximum and the
       endpoints to be in the right order (i.e. a non-negative distance) */
    if (!(si->root_distance <= max_distance && si->lo_limit <= si->hi_limit)) {
      mark_source(sources[i], SRC_BAD_DISTANCE);
      continue;
    }

    /* And the same applies for the estimated standard deviation */
    if (si->std_dev > max_jitter) {
      mark_source(sources[i], SRC_JITTERY);
      continue;
    }

    sources[i]->status = SRC_OK; /* For now */

    if (sources[i]->reachability && max_reach_sample_ago < first_sample_ago)
      max_reach_sample_ago = first_sample_ago;

    if (max_sel_reach < sources[i]->reachability)
      max_sel_reach = sources[i]->reachability;

    if (max_sel_reach_size < sources[i]->reachability_size)
      max_sel_reach_size = sources[i]->reachability_size;
  }

  orphan_stratum = REF_GetOrphanStratum();
  orphan_source = INVALID_SOURCE;

  for (i = 0; i < n_sources; i++) {
    if (sources[i]->status != SRC_OK)
      continue;

    si = &sources[i]->sel_info;

    /* Reachability is not a requirement for selection.  An unreachable source
       can still be selected if its newest sample is not older than the oldest
       sample from reachable sources. */
    if (!sources[i]->reachability && max_reach_sample_ago < si->last_sample_ago) {
      mark_source(sources[i], SRC_STALE);
      continue;
    }

    /* When the local reference is configured with the orphan option, NTP
       sources that have stratum equal to the configured local stratum are
       considered to be orphans (i.e. serving local time while not being
       synchronised with real time) and are excluded from the normal source
       selection.  Sources with stratum larger than the local stratum are
       considered to be directly on indirectly synchronised to an orphan and
       are always ignored.

       If no selectable source is available and all orphan sources have
       reference IDs larger than the local ID, no source will be selected and
       the local reference mode will be activated at some point, i.e. this host
       will become an orphan.  Otherwise, the orphan source with the smallest
       reference ID will be selected.  This ensures a group of servers polling
       each other (with the same orphan configuration) which have no external
       source can settle down to a state where only one server is serving its
       local unsychronised time and others are synchronised to it. */

    if (sources[i]->stratum >= orphan_stratum && sources[i]->type == SRC_NTP) {
      mark_source(sources[i], SRC_ORPHAN);

      if (sources[i]->stratum == orphan_stratum && sources[i]->reachability &&
          (orphan_source == INVALID_SOURCE ||
           sources[i]->ref_id < sources[orphan_source]->ref_id))
        orphan_source = i;

      continue;
    }

    ++n_sel_sources;
  }

  /* If no selectable source is available, consider the orphan source */
  if (!n_sel_sources && orphan_source != INVALID_SOURCE) {
    uint32_t local_ref_id = NSR_GetLocalRefid(sources[orphan_source]->ip_addr);

    if (!local_ref_id) {
      LOG(LOGS_ERR, "Unknown local refid in orphan mode");
    } else if (sources[orphan_source]->ref_id < local_ref_id) {
      sources[orphan_source]->status = SRC_OK;
      n_sel_sources = 1;
      DEBUG_LOG("selecting orphan refid=%"PRIx32, sources[orphan_source]->ref_id);
    }
  }

  for (i = 0; i < n_sources; i++) {
    if (sources[i]->status != SRC_OK)
      continue;

    if (sources[i]->sel_options & SRC_SELECT_TRUST)
      n_sel_trust_sources++;

    si = &sources[i]->sel_info;

    j1 = n_endpoints;
    j2 = j1 + 1;

    sort_list[j1].index = i;
    sort_list[j1].offset = si->lo_limit;
    sort_list[j1].tag = LOW;

    sort_list[j2].index = i;
    sort_list[j2].offset = si->hi_limit;
    sort_list[j2].tag = HIGH;

    n_endpoints += 2;
  }

  DEBUG_LOG("badstat=%d sel=%d badstat_reach=%o sel_reach=%o size=%d max_reach_ago=%f",
            n_badstats_sources, n_sel_sources, (unsigned int)max_badstat_reach,
            (unsigned int)max_sel_reach, max_sel_reach_size, max_reach_sample_ago);

  /* Wait for the next call if we have no source selected and there is
     a source with bad stats (has less than 3 samples) with reachability
     equal to shifted maximum reachability of sources with valid stats.
     This delays selecting source on start with servers using the same
     polling interval until they all have valid stats. */
  if (n_badstats_sources && n_sel_sources && selected_source_index == INVALID_SOURCE &&
      max_sel_reach_size < SOURCE_REACH_BITS && max_sel_reach >> 1 == max_badstat_reach) {
    mark_ok_sources(SRC_WAITS_STATS);
    unselect_selected_source(LOGS_INFO, NULL);
    return;
  }

  /* Wait for a source to have full reachability register to allow one
     failed selection to be logged before first successful selection */
  if (!forced_first_report &&
      MAX(max_sel_reach_size, max_badstat_reach_size) == SOURCE_REACH_BITS) {
    report_selection_loss = 1;
    forced_first_report = 1;
  }

  if (n_endpoints == 0) {
    /* No sources provided valid endpoints */
    unselect_selected_source(LOGS_WARN, "Can't synchronise: no selectable sources"
                                        " (%d unreachable sources)", n_unreach_sources);
    return;
  }

  /* Now sort the endpoint list */
  qsort((void *) sort_list, n_endpoints, sizeof(struct Sort_Element), compare_sort_elements);

  /* Now search for the interval which is contained in the most
     individual source intervals.  Any source which overlaps this
     will be a candidate.

     If we get a case like

     <----------------------->
         <-->
                  <-->
         <===========>

     we will build the interval as shown with '=', whereas with an extra source we get

     <----------------------->
        <------->
         <-->
                  <-->
         <==>

     The first case is just bad luck - we need extra sources to
     detect the falseticker, so just make an arbitrary choice based
     on stratum & stability etc.

     Intervals from sources specified with the trust option have higher
     priority in the search.
     */

  trust_depth = best_trust_depth = 0;
  depth = best_depth = 0;
  best_lo = best_hi = best_trust_lo = best_trust_hi = 0.0;

  for (i = 0; i < n_endpoints; i++) {
    switch (sort_list[i].tag) {
      case LOW:
        depth++;
        if (sources[sort_list[i].index]->sel_options & SRC_SELECT_TRUST)
          trust_depth++;
        if (trust_depth > best_trust_depth ||
            (trust_depth == best_trust_depth && depth > best_depth)) {
          if (trust_depth > best_trust_depth) {
            best_trust_depth = trust_depth;
            best_trust_lo = sort_list[i].offset;
          }
          best_depth = depth;
          best_lo = sort_list[i].offset;
        }
        break;
      case HIGH:
        if (trust_depth == best_trust_depth) {
          if (depth == best_depth)
            best_hi = sort_list[i].offset;
          best_trust_hi = sort_list[i].offset;
        }
        if (sources[sort_list[i].index]->sel_options & SRC_SELECT_TRUST)
          trust_depth--;
        depth--;
        break;
      default:
        assert(0);
    }
    assert(trust_depth <= depth);
    assert(trust_depth >= 0);
  }

  assert(depth == 0 && trust_depth == 0);
  assert(2 * n_sel_sources == n_endpoints);

  if (best_trust_depth > 0) {
    best_depth = best_trust_depth;
    n_sel_sources = n_sel_trust_sources;
  }

  if (best_depth <= n_sel_sources / 2) {
    /* Could not even get half the reachable (trusted) sources to agree */

    if (!reported_no_majority) {
      if (best_depth < 2)
        log_selection_message(LOGS_WARN, "%s (no agreement among %d %ssources)",
                              "Can't synchronise: no majority", n_sel_sources,
                              best_trust_depth > 0 ? "trusted " : "");
      else
        log_selection_message(LOGS_WARN, "%s (only %d of %d %ssources agree)",
                              "Can't synchronise: no majority", best_depth,
                              n_sel_sources, best_trust_depth > 0 ? "trusted " : "");
      reported_no_majority = 1;
      report_selection_loss = 0;
    }

    if (selected_source_index != INVALID_SOURCE) {
      REF_SetUnsynchronised();
      selected_source_index = INVALID_SOURCE;
    }

    /* .. and mark all sources as falsetickers (so they appear thus
       on the outputs from the command client) */
    mark_ok_sources(SRC_FALSETICKER);

    return;
  }

  /* We have our interval, now work out which source are in it,
     i.e. build list of admissible sources. */

  n_sel_sources = 0;

  for (i = 0; i < n_sources; i++) {
    /* This should be the same condition to get into the endpoint
       list */
    if (sources[i]->status != SRC_OK)
      continue;

    /* Check if source's interval contains the best interval, or is wholly
       contained within it.  If there are any trusted sources, other sources
       are required to be wholly contained within the best interval of the
       trusted sources to not allow non-trusted sources to move the final
       offset outside the trusted interval. */
    if ((sources[i]->sel_info.lo_limit <= best_lo &&
         sources[i]->sel_info.hi_limit >= best_hi) ||
        (sources[i]->sel_info.lo_limit >= best_lo &&
         sources[i]->sel_info.hi_limit <= best_hi)) {

      if (!(best_trust_depth == 0 || (sources[i]->sel_options & SRC_SELECT_TRUST) ||
            (sources[i]->sel_info.lo_limit >= best_trust_lo &&
             sources[i]->sel_info.hi_limit <= best_trust_hi))) {
        mark_source(sources[i], SRC_UNTRUSTED);
        continue;
      }

      sel_sources[n_sel_sources++] = i;

      if (sources[i]->sel_options & SRC_SELECT_REQUIRE)
        sel_req_source = 0;
    } else {
      mark_source(sources[i], SRC_FALSETICKER);
    }
  }

  if (!n_sel_sources || sel_req_source || n_sel_sources < CNF_GetMinSources()) {
    unselect_selected_source(LOGS_WARN, "Can't synchronise: %s selectable sources",
                             !n_sel_sources ? "no" :
                             sel_req_source ? "no required source in" : "not enough");
    mark_ok_sources(SRC_WAITS_SOURCES);
    return;
  }

  /* Enable the selectable sources (and trusted if there are any) to
     vote on leap seconds */
  for (i = 0; i < n_sel_sources; i++) {
    index = sel_sources[i];
    if (best_trust_depth && !(sources[index]->sel_options & SRC_SELECT_TRUST))
      continue;
    sources[index]->leap_vote = 1;
  }

  /* If there are any sources with prefer option, reduce the list again
     only to the preferred sources */
  for (i = 0; i < n_sel_sources; i++) {
    if (sources[sel_sources[i]]->sel_options & SRC_SELECT_PREFER)
      break;
  }
  if (i < n_sel_sources) {
    for (i = j = 0; i < n_sel_sources; i++) {
      if (!(sources[sel_sources[i]]->sel_options & SRC_SELECT_PREFER))
        mark_source(sources[sel_sources[i]], SRC_NONPREFERRED);
      else
        sel_sources[j++] = sel_sources[i];
    }
    assert(j > 0);
    n_sel_sources = j;
    sel_prefer = 1;
  } else {
    sel_prefer = 0;
  }

  /* Find minimum stratum */

  index = sel_sources[0];
  min_stratum = sources[index]->stratum;
  for (i = 1; i < n_sel_sources; i++) {
    index = sel_sources[i];
    stratum = sources[index]->stratum;
    if (stratum < min_stratum)
      min_stratum = stratum;
  }

  /* Update scores and find the source with maximum score */

  max_score_index = INVALID_SOURCE;
  max_score = 0.0;
  sel_src_distance = 0.0;

  if (selected_source_index != INVALID_SOURCE)
    sel_src_distance = sources[selected_source_index]->sel_info.root_distance +
      (sources[selected_source_index]->stratum - min_stratum) * stratum_weight;

  for (i = 0; i < n_sources; i++) {
    /* Reset score for non-selectable sources */
    if (sources[i]->status != SRC_OK ||
        (sel_prefer && !(sources[i]->sel_options & SRC_SELECT_PREFER))) {
      sources[i]->sel_score = 1.0;
      sources[i]->distant = DISTANT_PENALTY;
      continue;
    }

    distance = sources[i]->sel_info.root_distance +
      (sources[i]->stratum - min_stratum) * stratum_weight;
    if (sources[i]->type == SRC_NTP)
      distance += reselect_distance;

    if (selected_source_index != INVALID_SOURCE) {
      /* Update score, but only for source pairs where one source
         has a new sample */
      if (sources[i] == updated_inst ||
          sources[selected_source_index] == updated_inst) {

        sources[i]->sel_score *= sel_src_distance / distance;

        if (sources[i]->sel_score < 1.0)
          sources[i]->sel_score = 1.0;
      }
    } else {
      /* When there is no selected source yet, assign scores so that the
         source with minimum distance will have maximum score.  The scores
         will be reset when the source is selected later in this function. */
      sources[i]->sel_score = 1.0 / distance;
    }

    DEBUG_LOG("%s score=%f dist=%f",
              source_to_string(sources[i]), sources[i]->sel_score, distance);

    if (max_score < sources[i]->sel_score) {
      max_score = sources[i]->sel_score;
      max_score_index = i;
    }
  }

  assert(max_score_index != INVALID_SOURCE);

  /* Is the current source still a survivor and no other source has reached
     the score limit? */
  if (selected_source_index == INVALID_SOURCE ||
      sources[selected_source_index]->status != SRC_OK ||
      (max_score_index != selected_source_index && max_score > SCORE_LIMIT)) {

    /* Before selecting the new synchronisation source wait until the reference
       can be updated */
    if (sources[max_score_index]->updates == 0) {
      unselect_selected_source(LOGS_INFO, NULL);
      mark_ok_sources(SRC_WAITS_UPDATE);
      return;
    }

    selected_source_index = max_score_index;

    /* New source has been selected, reset all scores */
    for (i = 0; i < n_sources; i++) {
      sources[i]->sel_score = 1.0;
      sources[i]->distant = 0;
      memset(sources[i]->reported_status, 0, sizeof (sources[i]->reported_status));
    }

    reported_no_majority = 0;
    report_selection_loss = 0;
    forced_first_report = 1;
  }

  mark_source(sources[selected_source_index], SRC_SELECTED);

  /* Don't update reference when the selected source has no new samples */

  if (sources[selected_source_index]->updates == 0) {
    /* Mark the remaining sources as last combine_sources() call */

    for (i = 0; i < n_sel_sources; i++) {
      index = sel_sources[i];
      if (sources[index]->status == SRC_OK)
        mark_source(sources[index], sources[index]->distant ? SRC_DISTANT : SRC_UNSELECTED);
    }
    return;
  }

  for (i = 0; i < n_sources; i++)
    sources[i]->updates = 0;

  leap_status = get_leap_status();

  /* Now just use the statistics of the selected source combined with
     the other selectable sources for trimming the local clock */

  SST_GetTrackingData(sources[selected_source_index]->stats, &ref_time,
                      &src_offset, &src_offset_sd,
                      &src_frequency, &src_frequency_sd, &src_skew,
                      &src_root_delay, &src_root_dispersion);

  combined = combine_sources(n_sel_sources, &ref_time, &src_offset, &src_offset_sd,
                             &src_frequency, &src_frequency_sd, &src_skew);

  REF_SetReference(sources[selected_source_index]->stratum,
                   leap_status, combined,
                   sources[selected_source_index]->ref_id,
                   sources[selected_source_index]->ip_addr,
                   &ref_time, src_offset, src_offset_sd,
                   src_frequency, src_frequency_sd, src_skew,
                   src_root_delay, src_root_dispersion);
}

/* ================================================== */
/* Force reselecting the best source */

void
SRC_ReselectSource(void)
{
  selected_source_index = INVALID_SOURCE;
  SRC_SelectSource(NULL);
}

/* ================================================== */

void
SRC_SetReselectDistance(double distance)
{
  if (reselect_distance != distance) {
    reselect_distance = distance;
    LOG(LOGS_INFO, "New reselect distance %f", distance);
  }
}

/* ================================================== */
/* This routine is registered as a callback with the local clock
   module, to be called whenever the local clock changes frequency or
   is slewed.  It runs through all the existing source statistics, and
   adjusts them to make them look as though they were sampled under
   the new regime. */

static void
slew_sources(struct timespec *raw, struct timespec *cooked, double dfreq,
             double doffset, LCL_ChangeType change_type, void *anything)
{
  int i;

  for (i=0; i<n_sources; i++) {
    if (change_type == LCL_ChangeUnknownStep) {
      SST_ResetInstance(sources[i]->stats);
    } else {
      SST_SlewSamples(sources[i]->stats, cooked, dfreq, doffset);
    }
  }

  if (change_type == LCL_ChangeUnknownStep) {
    /* Update selection status */
    SRC_SelectSource(NULL);
  }
}

/* ================================================== */
/* This routine is called when an indeterminate offset is introduced
   into the local time. */

static void
add_dispersion(double dispersion, void *anything)
{
  int i;

  for (i = 0; i < n_sources; i++) {
    SST_AddDispersion(sources[i]->stats, dispersion);
  }
}

/* ================================================== */

static int
get_dumpfile(SRC_Instance inst, char *filename, size_t len)
{
  /* Use the IP address, or reference ID with reference clocks */
  switch (inst->type) {
    case SRC_NTP:
      if (!UTI_IsIPReal(inst->ip_addr) ||
          snprintf(filename, len, "%s", source_to_string(inst)) >= len)
        return 0;
      break;
    case SRC_REFCLOCK:
      if (snprintf(filename, len, "refid:%08"PRIx32, inst->ref_id) >= len)
        return 0;
      break;
    default:
      assert(0);
  }

  return 1;
}

/* ================================================== */

static void
save_source(SRC_Instance inst)
{
  char filename[64], *dumpdir, *ntp_name;
  FILE *f;

  dumpdir = CNF_GetDumpDir();
  if (!dumpdir)
    return;

  if (!get_dumpfile(inst, filename, sizeof (filename)))
    return;

  f = UTI_OpenFile(dumpdir, filename, ".dat", 'w', 0644);
  if (!f)
    return;

  ntp_name = inst->type == SRC_NTP ? NSR_GetName(inst->ip_addr) : ".";

  if (fprintf(f, "%s%s\n%d %o %d %d %d\n",
              DUMP_IDENTIFIER, ntp_name, inst->authenticated,
              (unsigned int)inst->reachability, inst->reachability_size,
              inst->stratum, (int)inst->leap) < 0 ||
      !SST_SaveToFile(inst->stats, f)) {
    fclose(f);
    if (!UTI_RemoveFile(dumpdir, filename, ".dat"))
      ;
    return;
  }

  fclose(f);
}

/* ================================================== */
/* This is called to dump out the source measurement registers */

void
SRC_DumpSources(void)
{
  int i;

  for (i = 0; i < n_sources; i++)
    save_source(sources[i]);
}

/* ================================================== */

#define MAX_WORDS 1

static void
load_source(SRC_Instance inst)
{
  char filename[64], line[256], *dumpdir, *ntp_name, *words[MAX_WORDS];
  int auth, leap, reach_size, stratum;
  unsigned int reach;
  FILE *f;

  dumpdir = CNF_GetDumpDir();
  if (!dumpdir)
    return;

  if (!get_dumpfile(inst, filename, sizeof (filename)))
    return;

  f = UTI_OpenFile(dumpdir, filename, ".dat", 'r', 0);
  if (!f)
    return;

  ntp_name = inst->type == SRC_NTP ? NSR_GetName(inst->ip_addr) : NULL;

  if (!fgets(line, sizeof (line), f) || strcmp(line, DUMP_IDENTIFIER) != 0 ||
      !fgets(line, sizeof (line), f) || UTI_SplitString(line, words, MAX_WORDS) != 1 ||
        (inst->type == SRC_NTP && (!ntp_name || strcmp(words[0], ntp_name) != 0)) ||
      !fgets(line, sizeof (line), f) ||
        sscanf(words[0], "%d %o %d %d %d",
               &auth, &reach, &reach_size, &stratum, &leap) != 5 ||
        (!auth && inst->authenticated) ||
        stratum < 0 || stratum >= NTP_MAX_STRATUM ||
        leap < LEAP_Normal || leap >= LEAP_Unsynchronised ||
      !SST_LoadFromFile(inst->stats, f)) {
    LOG(LOGS_WARN, "Could not load dump file for %s", source_to_string(inst));
    fclose(f);
    return;
  }

  inst->reachability = reach & ((1U << SOURCE_REACH_BITS) - 1);
  inst->reachability_size = CLAMP(0, reach_size, SOURCE_REACH_BITS);
  inst->stratum = stratum;
  inst->leap = leap;

  LOG(LOGS_INFO, "Loaded dump file for %s", source_to_string(inst));

  fclose(f);
}

/* ================================================== */

void
SRC_ReloadSources(void)
{
  int i;

  for (i = 0; i < n_sources; i++) {
    load_source(sources[i]);

    /* Allow an immediate update of the reference */
    sources[i]->updates++;
  }

  /* Select sources and set the reference */
  SRC_SelectSource(NULL);
}

/* ================================================== */

void
SRC_RemoveDumpFiles(void)
{
  char pattern[PATH_MAX], name[64], *dumpdir, *s;
  IPAddr ip_addr;
  glob_t gl;
  size_t i;

  dumpdir = CNF_GetDumpDir();
  if (!dumpdir ||
      snprintf(pattern, sizeof (pattern), "%s/*.dat", dumpdir) >= sizeof (pattern))
    return;

  if (glob(pattern, 0, NULL, &gl))
    return;

  for (i = 0; i < gl.gl_pathc; i++) {
    s = strrchr(gl.gl_pathv[i], '/');
    if (!s || snprintf(name, sizeof (name), "%s", s + 1) >= sizeof (name))
      continue;

    /* Remove .dat extension */
    if (strlen(name) < 4)
      continue;
    name[strlen(name) - 4] = '\0';

    /* Check if it looks like name of an actual dump file */
    if (strncmp(name, "refid:", 6) && !UTI_StringToIP(name, &ip_addr))
      continue;

    if (!UTI_RemoveFile(NULL, gl.gl_pathv[i], NULL))
      ;
  }

  globfree(&gl);
}

/* ================================================== */

void
SRC_ResetSources(void)
{
  int i;

  for (i = 0; i < n_sources; i++)
    SRC_ResetInstance(sources[i]);

  LOG(LOGS_INFO, "Reset all sources");
}

/* ================================================== */

int
SRC_IsSyncPeer(SRC_Instance inst)
{
  if (inst->index == selected_source_index) {
    return 1;
  } else {
    return 0;
  }

}

/* ================================================== */

int
SRC_IsReachable(SRC_Instance inst)
{
  return inst->reachability != 0;
}

/* ================================================== */

int
SRC_ReadNumberOfSources(void)
{
  return n_sources;
}

/* ================================================== */

int
SRC_ActiveSources(void)
{
  int i, r;

  for (i = r = 0; i < n_sources; i++)
    if (sources[i]->active)
      r++;

  return r;
}

/* ================================================== */

static SRC_Instance
find_source(IPAddr *ip, uint32_t ref_id)
{
  int i;

  for (i = 0; i < n_sources; i++) {
    if ((ip->family != IPADDR_UNSPEC && sources[i]->type == SRC_NTP &&
         UTI_CompareIPs(ip, sources[i]->ip_addr, NULL) == 0) ||
        (ip->family == IPADDR_UNSPEC && sources[i]->type == SRC_REFCLOCK &&
         ref_id == sources[i]->ref_id))
      return sources[i];
  }

  return NULL;
}

/* ================================================== */

int
SRC_ModifySelectOptions(IPAddr *ip, uint32_t ref_id, int options, int mask)
{
  SRC_Instance inst;

  inst = find_source(ip, ref_id);
  if (!inst)
    return 0;

  if ((inst->conf_sel_options & mask) == options)
    return 1;

  inst->conf_sel_options = (inst->conf_sel_options & ~mask) | options;
  LOG(LOGS_INFO, "Source %s selection options modified", source_to_string(inst));

  update_sel_options();

  return 1;
}

/* ================================================== */

int
SRC_ReportSource(int index, RPT_SourceReport *report, struct timespec *now)
{
  SRC_Instance src;
  if ((index >= n_sources) || (index < 0)) {
    return 0;
  } else {
    src = sources[index];

    if (src->ip_addr)
      report->ip_addr = *src->ip_addr;
    else {
      /* Use refid as an address */
      report->ip_addr.addr.in4 = src->ref_id;
      report->ip_addr.family = IPADDR_INET4;
    }

    report->stratum = src->stratum;

    switch (src->status) {
      case SRC_FALSETICKER:
        report->state = RPT_FALSETICKER;
        break;
      case SRC_JITTERY:
        report->state = RPT_JITTERY;
        break;
      case SRC_WAITS_SOURCES:
      case SRC_NONPREFERRED:
      case SRC_WAITS_UPDATE:
      case SRC_DISTANT:
      case SRC_OUTLIER:
        report->state = RPT_SELECTABLE;
        break;
      case SRC_UNSELECTED:
        report->state = RPT_UNSELECTED;
        break;
      case SRC_SELECTED:
        report->state = RPT_SELECTED;
        break;
      default:
        report->state = RPT_NONSELECTABLE;
        break;
    }

    report->reachability = src->reachability;

    /* Call stats module to fill out estimates */
    SST_DoSourceReport(src->stats, report, now);

    return 1;
  }

}

/* ================================================== */

int
SRC_ReportSourcestats(int index, RPT_SourcestatsReport *report, struct timespec *now)
{ 
  SRC_Instance src;

  if ((index >= n_sources) || (index < 0)) {
    return 0;
  } else {
    src = sources[index];
    report->ref_id = src->ref_id;
    if (src->ip_addr)
      report->ip_addr = *src->ip_addr;
    else
      report->ip_addr.family = IPADDR_UNSPEC; 
    SST_DoSourcestatsReport(src->stats, report, now);
    return 1;
  }
}

/* ================================================== */

static char
get_status_char(SRC_Status status)
{
  switch (status) {
    case SRC_UNSELECTABLE:
      return 'N';
    case SRC_BAD_STATS:
      return 'M';
    case SRC_UNSYNCHRONISED:
      return 's';
    case SRC_BAD_DISTANCE:
      return 'd';
    case SRC_JITTERY:
      return '~';
    case SRC_WAITS_STATS:
      return 'w';
    case SRC_STALE:
      return 'S';
    case SRC_ORPHAN:
      return 'O';
    case SRC_UNTRUSTED:
      return 'T';
    case SRC_FALSETICKER:
      return 'x';
    case SRC_WAITS_SOURCES:
      return 'W';
    case SRC_NONPREFERRED:
      return 'P';
    case SRC_WAITS_UPDATE:
      return 'U';
    case SRC_DISTANT:
      return 'D';
    case SRC_OUTLIER:
      return 'L';
    case SRC_UNSELECTED:
      return '+';
    case SRC_SELECTED:
      return '*';
    default:
      return '?';
  }
}

/* ================================================== */

int
SRC_GetSelectReport(int index, RPT_SelectReport *report)
{
  SRC_Instance inst;

  if (index >= n_sources || index < 0)
    return 0;

  inst = sources[index];

  report->ref_id = inst->ref_id;
  if (inst->ip_addr)
    report->ip_addr = *inst->ip_addr;
  else
    report->ip_addr.family = IPADDR_UNSPEC;
  report->state_char = get_status_char(inst->status);
  report->authentication = inst->authenticated;
  report->leap = inst->leap;
  report->conf_options = inst->conf_sel_options;
  report->eff_options = inst->sel_options;
  report->last_sample_ago = inst->sel_info.last_sample_ago;
  report->score = inst->sel_score;
  report->lo_limit = inst->sel_info.lo_limit;
  report->hi_limit = inst->sel_info.hi_limit;

  return 1;
}

/* ================================================== */

SRC_Type
SRC_GetType(int index)
{
  if ((index >= n_sources) || (index < 0))
    return -1;
  return sources[index]->type;
}

/* ================================================== */
