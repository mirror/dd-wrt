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

/* Structure maintained on a per-route basis. */
struct bgp_damp_info
{
	int penalty;
	int flap;
	
	/* Last time penalty was updated. */
	time_t t_updated;

	/* First flap time */
	time_t start_time;

	/* Reference to next damp_info in the reuse list. */
	struct bgp_damp_info *reuse_next;

	/* Back reference to bgp_info. */
	struct bgp_info *bgp_info;
};

/* Global configuration parameters. */
struct bgp_damp_config
{
	/* Configurable parameters */
	int enabled;		/* Is damping enabled? */
	int suppress_value;	/* Value over which routes suppressed */
	int reuse_limit;	/* Value below which suppressed routes reused */
	int max_suppress_time;	/* Max time a route can be suppressed */
	int half_life;		/* Time during which accumulated penalty reduces by half */

	/* Non-configurable parameters but fixed at implementation time.
	 * To change this values, init_bgp_damp() should be modified.
	 */
	int tmax;		/* Max time previous instability retained */
	int reuse_list_size;	/* Number of reuse lists */
	int reuse_index_array_size;	/* Size of reuse index array */

	/* Non-configurable parameters.  Most of these are calculated from
	 * the configurable parameters above.
	 */
	unsigned int ceiling;	/* Max value a penalty can attain */
	int decay_rate_per_tick; /* Calculated from half-life */
	int decay_array_size;	/* Calculated using config parameters */
	double *decay_array;	/* Storage for decay values */
	int	scale_factor;
	int reuse_scale_factor;	/* To scale reuse array indices */
	int *reuse_index_array;
	struct bgp_damp_info **reuse_list_array;
};

#define BGP_DAMP_CONTINUE	1
#define BGP_DAMP_DISABLED	2
#define BGP_DAMP_DISCONTINUE 3

int bgp_damp_enable(struct vty *, int, char **);
int bgp_damp_disable(struct vty *);
int bgp_config_write_damp (struct vty *);
int bgp_damp_info_print (struct vty *, struct bgp_info *);
