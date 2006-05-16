/* BGP flap dampening
 * Copyright (C) 2001 IP Infusion Inc.
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>
#include <math.h>

#include "prefix.h"
#include "table.h"
#include "linklist.h"
#include "memory.h"
#include "command.h"
#include "stream.h"
#include "filter.h"
#include "str.h"
#include "log.h"
#include "routemap.h"
#include "buffer.h"
#include "sockunion.h"
#include "plist.h"
#include "thread.h"

#include "bgpd/bgp_damp.h"
#include "bgpd/bgpd.h"
#include "bgpd/bgp_route.h"

/* Time granularity for reuse lists */
#define DELTA_REUSE 15
/* Time granularity for decay arrays */
#define DELTA_T 1

#define DEFAULT_PENALTY 1000

#define DEFAULT_HARF_LIFE 15
#define DEFAULT_REUSE 750
#define DEFAULT_SUPPRESS 2000
#define REUSE_LIST_SIZE		256
#define REUSE_ARRAY_SIZE	1024

/* Global variable to access damping configuration */
struct bgp_damp_config bgp_damp_cfg;
int reuse_array_offset = 0;
struct thread *bgp_reuse_thread = NULL;
struct bgp_damp_config *prev_bgp_damp_cfg = NULL;

int bgp_reuse_timer(struct thread *);
void bgp_damp_clear_config(struct bgp_damp_config *);
void bgp_damp_clear_reuse_list();

int
bgp_damp_init(struct vty *vty, int hlife, int reuse, int sup, int maxsup)
{
  double reuse_max_ratio;
  int i;
  struct bgp_damp_config *bdc;
	
  if (hlife == -1)
    hlife = DEFAULT_HARF_LIFE * 60;
	
  if (reuse == -1)
    {
      /* Default values */
      reuse = DEFAULT_REUSE;
      sup = DEFAULT_SUPPRESS;
      maxsup = 4*hlife;
    }

  /* Clear previous configuration if any */
  if (prev_bgp_damp_cfg)
    bgp_damp_clear_config (&bgp_damp_cfg);

  prev_bgp_damp_cfg = bdc = &bgp_damp_cfg;
  bdc->suppress_value = sup;
  bdc->half_life = hlife;
  bdc->reuse_limit = reuse;
  bdc->max_suppress_time = maxsup;

  /* Initialize system-wide params */
  bdc->reuse_list_size = REUSE_LIST_SIZE;
  bdc->reuse_index_array_size = REUSE_ARRAY_SIZE;

  bdc->ceiling = (int)(bdc->reuse_limit * 
		       exp((double) (bdc->max_suppress_time/bdc->half_life)) * log(2.0));

  /* Decay-array computations */
  bdc->decay_array_size = ceil ((double)bdc->max_suppress_time/DELTA_T);
  bdc->decay_array = XMALLOC (MTYPE_BGP_DAMP_ARRAY,
			      sizeof(double) * (bdc->decay_array_size));
  bdc->decay_array[0] = 1.0;
  bdc->decay_array[1] = exp ((1.0/(bdc->half_life/DELTA_T)) * log(0.5));
	
  /* Calculate decay values for all possible times */
  for (i = 2; i < bdc->decay_array_size; i++)
    bdc->decay_array[i] = bdc->decay_array[i-1] * bdc->decay_array[1];
	
  /* Reuse-list computations */
  bdc->reuse_list_array = XMALLOC (MTYPE_BGP_DAMP_ARRAY, bdc->reuse_list_size * sizeof (struct bgp_reuse_list *));
  memset (bdc->reuse_list_array, 0x00, bdc->reuse_list_size * sizeof (struct bgp_reuse_list*));

  /* Reuse-array computations */
  bdc->reuse_index_array = XMALLOC (MTYPE_BGP_DAMP_ARRAY, 
				    sizeof(int) * bdc->reuse_index_array_size);
  reuse_max_ratio = bdc->ceiling/bdc->reuse_limit;
  i = (int)exp((1.0/((double)bdc->half_life/(bdc->reuse_list_size*DELTA_REUSE))) * log(2.0));
  if ( reuse_max_ratio > i && i != 0 )
	reuse_max_ratio = i;

  bdc->scale_factor = ceil((double)bdc->reuse_index_array_size/(reuse_max_ratio - 1));

  for (i = 0; i < bdc->reuse_index_array_size; i++)
    {
      bdc->reuse_index_array[i] = 
		ceil( (bdc->half_life/DELTA_REUSE)
		* log(1.0/(bdc->reuse_limit * ( 1.0 + ((double)i/bdc->scale_factor)))) / log(0.5) );
    }

  return CMD_SUCCESS;
}

static double
bgp_damp_get_decay(time_t tdiff)
{
  int i;
  struct bgp_damp_config *bdc;

  bdc = &bgp_damp_cfg;
  i = tdiff/DELTA_T;

  if (i >= bdc->decay_array_size)
    return 0;

  return bdc->decay_array[i];
}

static int
bgp_get_reuse_index(int penalty)
{
  int i;
  struct bgp_damp_config *bdc;

  bdc = &bgp_damp_cfg;
  i = (int)(((double)penalty / bdc->reuse_limit - 1.0) * bdc->scale_factor);

  if ( i >= bdc->reuse_index_array_size )
	  i = bdc->reuse_index_array_size - 1;
  return (bdc->reuse_index_array[i]);
}

static void
bgp_reuse_list_insert(struct bgp_damp_info *bdi)
{
  int index;
  struct bgp_damp_config *bdc;
  static int first_time_insert = 1;

  if (first_time_insert)
    {
      /* Kick off reuse timer */
      bgp_reuse_thread 
	= thread_add_timer (master, bgp_reuse_timer, NULL, DELTA_REUSE);
      first_time_insert = 0;
    }

  bdc = &bgp_damp_cfg;

  index = (reuse_array_offset + bgp_get_reuse_index(bdi->penalty)) % bdc->reuse_list_size;
  bdi->reuse_next = bdc->reuse_list_array[index];
  bdc->reuse_list_array[index] = bdi;

  return;
}

/* bgp_reuse_timer is called every DELTA_REUSE seconds.
 * Each route in the current reuse-list is evaluated and is used or requeued
 */
int
bgp_reuse_timer(struct thread *t)
{
  struct bgp_damp_info *bdi, *tbdi;
  struct bgp_damp_config *bdc;
  time_t t_now, t_diff;

  /* Restart the reuse timer */
  bgp_reuse_thread = thread_add_timer(master, bgp_reuse_timer, 
				      NULL, DELTA_REUSE);
	
  /* zlog(NULL, LOG_INFO, "DAMP:reuse timer:offset: %d", reuse_array_offset); */
  t_now = time(NULL);
  bdc = &bgp_damp_cfg;
  bdi = bdc->reuse_list_array[reuse_array_offset];
  bdc->reuse_list_array[reuse_array_offset] = NULL;
  reuse_array_offset = (reuse_array_offset + 1 ) % bdc->reuse_list_size;
	
  while (bdi)
    {
      tbdi = bdi->reuse_next;
      bdi->reuse_next = NULL;
      t_diff = t_now - bdi->t_updated;
      bdi->t_updated = t_now;

      if (bdi->bgp_info == NULL )
	free(bdi);
      else 
	{
	  bdi->penalty
	    = (int)((double)bdi->penalty * bgp_damp_get_decay(t_diff));

	  /* zlog(NULL, LOG_INFO, "DAMP:reuse timer: updated penalty: %d", bdi->penalty); */

	  if ( bdi->penalty <= bdc->reuse_limit/2 )
	    {
	      UNSET_FLAG (bdi->bgp_info->flags, BGP_INFO_DAMPED);
	      bdi->bgp_info->bgp_damp_info = NULL;
	      free(bdi);
	    }
	  else if ( bdi->penalty < bdc->reuse_limit )
	    UNSET_FLAG (bdi->bgp_info->flags, BGP_INFO_DAMPED);
	  else
	    bgp_reuse_list_insert(bdi);
	}

      bdi = tbdi;
    }
	
  return 0;
}

int
bgp_damp_withdraw(struct bgp_info *bgp_info)
{
  time_t t_now;
  struct bgp_damp_info *bdi;
  struct bgp_damp_config *bdc;
  int status;
	
  bdc = &bgp_damp_cfg;

  if (! bdc->enabled)
    return BGP_DAMP_DISABLED;

  SET_FLAG (bgp_info->flags, BGP_INFO_HISTORY);

  t_now = time(NULL);
  bdi = bgp_info->bgp_damp_info;

  status = BGP_DAMP_CONTINUE;

  if (bdi == NULL)
    {
      bdi = XMALLOC (MTYPE_BGP_DAMP_INFO, sizeof (struct bgp_damp_info));
      memset (bdi, 0, sizeof (struct bgp_damp_info));
      bgp_info->bgp_damp_info = bdi;
      bdi->penalty = DEFAULT_PENALTY;
      bdi->flap = 1;
      bdi->start_time = t_now;
      bdi->bgp_info = bgp_info;
    }
  else
    {
      bdi->penalty = (int)(bdi->penalty * bgp_damp_get_decay(t_now - bdi->t_updated)) + DEFAULT_PENALTY;
      bdi->flap++;
      if (bdi->penalty > bdc->ceiling)
	bdi->penalty = bdc->ceiling;
    }

  /* If the penalty is greater than suppress value or the route is damped,
   * no need to send withdraw to peers.
   */
  if (CHECK_FLAG (bdi->bgp_info->flags, BGP_INFO_DAMPED))
    status = BGP_DAMP_DISCONTINUE; 
  else if (bdi->penalty >= bdc->suppress_value)
    {
      bgp_reuse_list_insert(bdi);
      SET_FLAG (bdi->bgp_info->flags, BGP_INFO_DAMPED);
    }
  bdi->t_updated = t_now;

  /* zlog(NULL, LOG_ERR, "DAMP:Withdraw - penalty: %d, damped: %d", bdi->penalty, bdi->bgp_info->damped); */

  return (status);
}

int
bgp_damp_update(struct bgp_info *bgp_info)
{
  time_t t_now;
  struct bgp_damp_info *bdi;
  struct bgp_damp_config *bdc;
  int status;

  bdc = &bgp_damp_cfg;

  if (! bdc->enabled)
    return BGP_DAMP_DISABLED;

  if ((bdi = bgp_info->bgp_damp_info) == NULL)
    return BGP_DAMP_CONTINUE;

  t_now = time(NULL);
  bdi->penalty = (int) (bdi->penalty * bgp_damp_get_decay(t_now - bdi->t_updated));
	
  /* zlog(NULL, LOG_ERR, "UPDATE: penalty: %d", bdi->penalty); */
  if (! CHECK_FLAG (bgp_info->flags, BGP_INFO_DAMPED)
      && (bdi->penalty < bdc->suppress_value))
    {
      status = BGP_DAMP_CONTINUE;
    }
  else if (CHECK_FLAG (bgp_info->flags, BGP_INFO_DAMPED)
	   && (bdi->penalty < bdc->reuse_limit) )
    {
      UNSET_FLAG (bgp_info->flags, BGP_INFO_DAMPED);
      status = BGP_DAMP_CONTINUE;
    }
  else
    status = BGP_DAMP_DISCONTINUE;

  bdi->t_updated = t_now;
	
  return status;
}

int
bgp_damp_enable(struct vty *vty, int argc, char **argv)
{

  bgp_damp_cfg.enabled = 1;

  if (argc == 0)
    {
      if (prev_bgp_damp_cfg)
	{
	  if (! bgp_reuse_thread)
	    bgp_reuse_thread
	      = thread_add_timer(master, bgp_reuse_timer, NULL, DELTA_REUSE);

	  /* zlog(NULL, LOG_ERR, "Using old config"); */

	  return CMD_SUCCESS;
	}

      /* zlog(NULL, LOG_ERR, "Using new config"); */

      return bgp_damp_init(vty, -1, -1, -1, -1);
    }

  if ( argc == 1 )
    {
      return bgp_damp_init (vty, atoi(argv[0])*60, -1, -1, -1);
    }

  return bgp_damp_init (vty, atoi(argv[0]) * 60, atoi(argv[1]), atoi(argv[2]),
			atoi(argv[3]) * 60);
}

int
bgp_damp_disable( struct vty *vty )
{
  bgp_damp_cfg.enabled = 0;

  if ( bgp_reuse_thread )
    thread_cancel (bgp_reuse_thread);

  bgp_reuse_thread = NULL;
	
  /* Clear the reuse list entries */
  bgp_damp_clear_reuse_list();

  /* Clear configuration */
  bgp_damp_clear_config(&bgp_damp_cfg);

  return CMD_SUCCESS;
}

void
bgp_damp_clear_config(struct bgp_damp_config *bdc)
{
  /* Free decay array */
  free (bdc->decay_array);

  /* Free reuse index array */
  free (bdc->reuse_index_array);

  if ( bgp_reuse_thread )
    thread_cancel (bgp_reuse_thread);
  bgp_reuse_thread = NULL;

  /* Remove all entries from reuse lists */
  prev_bgp_damp_cfg = NULL;
}

void
bgp_damp_clear_reuse_list()
{
  struct bgp_damp_info *bdi, *tbdi;
  struct bgp_damp_config *bdc;
  int i;

  reuse_array_offset = 0;
  bdc = &bgp_damp_cfg;

  for (i = 0; i < bdc->reuse_list_size; i++)
    {
      if (bdc->reuse_list_array[i] == NULL)
	continue;

      bdi = bdc->reuse_list_array[i];
      bdc->reuse_list_array[i] = NULL;

      while (bdi)
	{
	  tbdi = bdi->reuse_next;

	  if (bdi->bgp_info)
	    {
	      bdi->bgp_info->bgp_damp_info = NULL;
	      UNSET_FLAG (bdi->bgp_info->flags, BGP_INFO_DAMPED);
	    }

	  free (bdi);
	  bdi = tbdi;
	}
    }
}

int
bgp_config_write_damp (struct vty *vty)
{
  if (bgp_damp_cfg.enabled)
    {
      if (bgp_damp_cfg.half_life == DEFAULT_HARF_LIFE*60
	  && bgp_damp_cfg.reuse_limit == DEFAULT_REUSE
	  && bgp_damp_cfg.suppress_value == DEFAULT_SUPPRESS
	  && bgp_damp_cfg.max_suppress_time == bgp_damp_cfg.half_life*4)
	vty_out (vty, " bgp dampening%s", VTY_NEWLINE);
      else if (bgp_damp_cfg.half_life != DEFAULT_HARF_LIFE*60
	       && bgp_damp_cfg.reuse_limit == DEFAULT_REUSE
	       && bgp_damp_cfg.suppress_value == DEFAULT_SUPPRESS
	       && bgp_damp_cfg.max_suppress_time == bgp_damp_cfg.half_life*4)
	vty_out (vty, " bgp dampening %d%s",
		 bgp_damp_cfg.half_life/60,
		 VTY_NEWLINE);
      else
	vty_out (vty, " bgp dampening %d %d %d %d%s",
		 bgp_damp_cfg.half_life/60,
		 bgp_damp_cfg.reuse_limit,
		 bgp_damp_cfg.suppress_value,
		 bgp_damp_cfg.max_suppress_time/60,
		 VTY_NEWLINE);
      return 1;
    }
  return 0;
}

#define BGP_UPTIME_LEN 25

int
bgp_damp_info_print (struct vty *vty, struct bgp_info *bgp_info)
{
  struct bgp_damp_info *bdi;
  time_t t_now;
  char timebuf[BGP_UPTIME_LEN];

  if ((bdi = bgp_info->bgp_damp_info) == NULL) 
    return CMD_WARNING;
  t_now = time (NULL);
  vty_out (vty, "      Dampinfo: penalty %d, flapped %d times in %s",
	   (int)(bdi->penalty * bgp_damp_get_decay(t_now - bdi->t_updated)),
	   bdi->flap, peer_uptime (bdi->start_time, timebuf, BGP_UPTIME_LEN));
  if (CHECK_FLAG (bdi->bgp_info->flags, BGP_INFO_DAMPED)
      && ! CHECK_FLAG (bdi->bgp_info->flags, BGP_INFO_HISTORY))
    vty_out (vty, ", reuse in 00:00:00"); 
  vty_out (vty, "%s", VTY_NEWLINE);

  return CMD_SUCCESS;
}
